#include "config.h"
#include "HEDriver.hxx"
#include "HashHelper.hxx"
#include <util/threading/Lock.hxx>
#include "HEDriverCommand.hxx"

#ifdef LOG_SUPPORT
#	define DEFAULT_SUBSYSTEM "HttpEngine"
#include <util/log/Logger.hxx>
#else
#	define CerrLog(args_)
#	define StackLog(args_)
#	define DebugLog(args_)
#	define InfoLog(args_)
#	define WarningLog(args_)
#	define ErrLog(args_)
#	define CritLog(args_)
#endif

using namespace httpengine;

HEDriver::HEDriver(void)
{
}

HEDriver::~HEDriver(void)
{
}

void HEDriver::addSession(HESessionInfoPtr ptr, const unsigned int &sessionId)
{
	HEDriverCommandPtr cmdPtr(new HEDriverCommandAdd(&mSessionInfoPtrMap, ptr));
	mCmdFifo.add(cmdPtr);
}

void HEDriver::removeSession(const unsigned int sessionId)
{
	HEDriverCommandPtr cmdPtr(new HEDriverCommandRemove(&mSessionInfoPtrMap, sessionId));
	mCmdFifo.add(cmdPtr);
}

const HESessionInfoPtr HEDriver::getSession(const unsigned int sessionId)
{
	HESessionInfoPtr ptr;
	HESessionInfoPtrMap::iterator it = mSessionInfoPtrMap.find(sessionId);
	if(it != mSessionInfoPtrMap.end())
	{
		ptr = it->second;
	}
    return ptr;
}

const unsigned int HEDriver::getCounts() const
{
	return mSessionInfoPtrMap.size();
}

void HEDriver::thread()
{
	curl_global_init(CURL_GLOBAL_ALL);
	CURLM* multi_curl = curl_multi_init();
	if( !multi_curl)
	{
		assert("Critical Error:initialize multi curl handle failed.");
		return ;
	}

	while(!isShutdown()) 
	{
		threadPerformCommand(multi_curl);

		//queue handle
		bool bHasHandle = threadScanQueue(multi_curl);
		
		if (!bHasHandle)
		{
			// TODO(welbon): This will going to be optimize,
			// We have no handler, wait a signle.
			sleep(100);
			continue;
		}

		int still_running = 0;
		while(curl_multi_perform(multi_curl, &still_running) == CURLM_CALL_MULTI_PERFORM);

		//curl handle
		threadMultiURLRoop(multi_curl, bHasHandle);
		threadNotificationProgress();

		//Dispatch
		threadDispatchSession(multi_curl);

		//sleep(10);
	}
	threadFifoCleanup();
	threadQueueCleanup(multi_curl);
	curl_multi_cleanup(multi_curl);
	curl_global_cleanup();
}

void HEDriver::threadPerformCommand(CURLM* multi_curl)
{
	if( mCmdFifo.size() > 0 )
	{
		HEDriverCommandPtr ptr = mCmdFifo.getNext();
		ptr->execute(multi_curl);
	}
}

bool HEDriver::threadScanQueue(CURLM* multi_curl)
{
	if( !multi_curl)
	{
		assert(false);
		return false;
	}

	// If empty,do noting.
	if( mSessionInfoPtrMap.empty())
	{
		return false;
	}

	bool bHasHandle = false;
	HESessionInfoPtrMap::iterator it = mSessionInfoPtrMap.begin();
	while(it != mSessionInfoPtrMap.end()) 
	{
		bool isNeedRelease = false;
		if(it->second)
		{
			switch(it->second->getCurrentStatus())
			{
			case E_HTTPSTATUS_CLOSED:
				{
					// Initialize status, do noting now.
				}
				break;

			case E_HTTPSTATUS_START:
				{
					if (it->second->mCURL == NULL)
					{
						it->second->mCURL = curl_easy_init();
						it->second->initSessionInfo();
						CURLMcode code = curl_multi_add_handle(multi_curl, it->second->mCURL);

						if(code == CURLM_OK)
						{
							it->second->onStart();
						}
						else
						{
							ErrLog(<< __FUNCTION__ << " | curl_multi_add_handle return not ok, result code :" << code);
							it->second->setCurrentStatus(E_HTTPSTATUS_ERROR);
							if (it->second->details())
							{
								it->second->details()->mErrorCode = HE_CURL_ERROR;
								it->second->details()->mErrorSubCode = code;
							}
						}
					}
					// else do nothing!
				}
				break;

			case E_HTTPSTATUS_ABORT:
				{
					isNeedRelease = true;
					it->second->onAbort();
				}
				break;

			case E_HTTPSTATUS_WAITCOMPLATE:
				{
					it->second->setCurrentStatus(E_HTTPSTATUS_COMPLETE);
				}
				break;

			case E_HTTPSTATUS_COMPLETE:
				{
					isNeedRelease = true;
					it->second->onComplate();
				}
				break;

			case E_HTTPSTATUS_ERROR:
				{
					isNeedRelease = true;
					it->second->onError();
				}
				break;

			default:
				break;
			}

			if (isNeedRelease)
			{
				curl_multi_remove_handle(multi_curl, it->second->mCURL);
				it->second->uninitSessionInfo();

				// Notify release
				it->second->onRelease();
				//
				mSessionInfoPtrMap.erase(it ++);

				// Must continue , cos 'it' aready ++.
				continue ;
			}
		}
		++ it;
		bHasHandle = true;
	}
	return bHasHandle;
}

bool HEDriver::threadMultiURLRoop(CURLM* multi_curl, bool bHasHandle)
{
	if(!bHasHandle)
	{
		return false;
	}

	if(!multi_curl)
	{
		return false;
	}
	int still_running = 0;

	/* we start some action by calling perform right away */ 
	CURLMcode code = curl_multi_perform(multi_curl, &still_running);

	if/*while*/(still_running /*&& !isShutdown()*/) 
	{
		struct timeval timeout;
		int rc; /* select() return code */ 

		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd = -1;

		long curl_timeo = -1;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		/* set a suitable timeout to play around with */ 
		timeout.tv_sec = 0;
		timeout.tv_usec = 100 * 1000;

		curl_multi_timeout(multi_curl, &curl_timeo);
		if(curl_timeo >= 0)
		{
			timeout.tv_sec = curl_timeo / 1000;
			if( timeout.tv_sec > 1)
			{
				timeout.tv_sec = 1;
			}
			else
			{
				timeout.tv_usec = (curl_timeo % 1000) * 1000;
			}
		}

		/* get file descriptors from the transfers */ 
		curl_multi_fdset(multi_curl, &fdread, &fdwrite, &fdexcep, &maxfd);

		/* In a real-world program you OF COURSE check the return code of the
		function calls.  On success, the value of maxfd is guaranteed to be
		greater or equal than -1.  We call select(maxfd + 1, ...), specially in
		case of (maxfd == -1), we call select(0, ...), which is basically equal
		to sleep. */ 

		rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

		//
		switch(rc) 
		{
		case -1:
			{
				/* select error */ 
				sleep(100);
			}
			break;
		case 0: /* timeout */
		default: /* action */ 
			curl_multi_perform(multi_curl, &still_running);
			break;
		}
		return true;
	}
	return false;
}

void HEDriver::threadDispatchSession(CURLM* multi_curl)
{
	// Handle Map
	int msgs_left = 0;
	CURLMsg* msg = NULL;
	while((msg = curl_multi_info_read(multi_curl, &msgs_left))) 
	{
		HESessionInfoPtrMap::iterator it = mSessionInfoPtrMap.begin();
		while( it != mSessionInfoPtrMap.end()) 
		{
			if (it->second
				&& it->second->mCURL == msg->easy_handle
				&& it->second->getCurrentStatus() == E_HTTPSTATUS_START
				&& msg->msg == CURLMSG_DONE) 
			{
				if (CURLE_OK != msg->data.result)
				{
					it->second->details()->mErrorCode = HE_CURL_ERROR;
					it->second->details()->mErrorSubCode = msg->data.result;
				}
				it->second->setCurrentStatus((HE_OK == it->second->details()->mErrorCode) ? E_HTTPSTATUS_WAITCOMPLATE : E_HTTPSTATUS_ERROR);
			}
			++ it;
		}
	}
}

void HEDriver::threadQueueCleanup(CURLM* multi_curl)
{
	if(!multi_curl)
	{
		assert(false);
		return ;
	}
	HESessionInfoPtrMap::iterator it = mSessionInfoPtrMap.begin();
	while( it != mSessionInfoPtrMap.end())
	{
		if(it->second)
		{
			if(it->second->mCURL)
			{
				curl_multi_remove_handle(multi_curl, it->second->mCURL);
				it->second->uninitSessionInfo();
			}
			it->second->onRelease();
		}
		++it;
	}
	mSessionInfoPtrMap.clear();
}

void HEDriver::threadFifoCleanup(void)
{
	mCmdFifo.clear();
}

void HEDriver::threadNotificationProgress()
{
	HESessionInfoPtrMap::iterator it = mSessionInfoPtrMap.begin();
	while(it != mSessionInfoPtrMap.end())
	{
		if(it->second && it->second->getCurrentStatus() == E_HTTPSTATUS_START)
		{
			it->second->notificationProgress();
		}
		++it;
	}
}

