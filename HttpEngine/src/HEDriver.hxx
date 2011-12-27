#pragma once
#ifndef HTTPENGINE_HEDRIVER_HXX
#define HTTPENGINE_HEDRIVER_HXX


#include <map>
#include <curl/curl.h>
#include <util/common/Fifo.hxx>
#include <util/threading/ThreadIf.hxx>

#include "HESessionInfo.hxx"
#include "IHEDriverCommand.hxx"


namespace httpengine
{
	/**
	*  Http engine driver,this class will run work thread.
	*
	* @athor welbon
	*
	* @date 2011-04-14
	*/
	class HEDriver : public Util::ThreadIf
	{
	public:
		typedef std::map<unsigned int, HESessionInfoPtr> HESessionInfoPtrMap;

		HEDriver(void);
		virtual ~HEDriver(void);

		void addSession(HESessionInfoPtr ptr,const unsigned int &sessionId);
		void removeSession(const unsigned int sessionId);
		const HESessionInfoPtr getSession(const unsigned int sessionId);
		const unsigned int getCounts() const;

		///////////////////////////////////////////////////////////////////////
		// Util::ThreadIf interface
	private:
		virtual void thread();
		bool threadScanQueue(CURLM* multi_curl);
		void threadPerformCommand(CURLM* multi_curl);
		bool threadMultiURLRoop(CURLM* multi_curl,bool bHasHandle);
		void threadDispatchSession(CURLM* multi_curl);
		void threadQueueCleanup(CURLM* multi_curl);
		void threadFifoCleanup(void);
		void threadNotificationProgress();

	private:
		HESessionInfoPtrMap mSessionInfoPtrMap;
		Util::Fifo<IHEDriverCommand> mCmdFifo;
	};
}

#endif //HTTPENGINE_HEDRIVER_HXX