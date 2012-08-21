#include "HEDriverCommand.hxx"
#include "HEDriver.hxx"

using namespace httpengine;

////////////////////////////////////////////////////////////////////////
// http engine driver base command

HEDriverCommandBase::HEDriverCommandBase()
{

}

HEDriverCommandBase::~HEDriverCommandBase()
{

}

void HEDriverCommandBase::execute(HESessionInfoPtrMap& sessionMap, void* param)
{

}

////////////////////////////////////////////////////////////////////////
// http engine driver command - add

HEDriverCommandAdd::HEDriverCommandAdd(HESessionInfoPtr ptrSession)
: mHESessionInfoPtr(ptrSession)
{
}

HEDriverCommandAdd::~HEDriverCommandAdd()
{
}

void HEDriverCommandAdd::execute(HESessionInfoPtrMap& sessionMap, void* param)
{
	if( sessionMap.empty())
	{
		sessionMap.insert(std::make_pair(mHESessionInfoPtr->details()->mKey, mHESessionInfoPtr));
	}
	else
	{
		HESessionInfoPtrMap::iterator it = sessionMap.find(mHESessionInfoPtr->details()->mKey);
		if(it == sessionMap.end())
		{
			sessionMap.insert(std::make_pair(mHESessionInfoPtr->details()->mKey,mHESessionInfoPtr));
		}
	}
}

////////////////////////////////////////////////////////////////////////
// http engine driver command - remove

HEDriverCommandRemove::HEDriverCommandRemove(int iKey)
: mDelKey(iKey) 
{

}

HEDriverCommandRemove::~HEDriverCommandRemove()
{

}

void HEDriverCommandRemove::execute(HESessionInfoPtrMap& sessionMap, void* param)
{
	if (!sessionMap.empty()) 
	{
		HESessionInfoPtrMap::iterator it = sessionMap.find(mDelKey);
		if( it != sessionMap.end())
		{
			HESessionInfoPtr pMultiSession = it->second;
			if( pMultiSession) 
			{
				CURLM* multi_curl = (CURLM*)param;
				if(pMultiSession->mCURL != NULL && multi_curl)
				{
					curl_multi_remove_handle(multi_curl,pMultiSession->mCURL);
					pMultiSession->uninitSessionInfo();
				}
				pMultiSession->onRelease();
				pMultiSession.reset();
			}
			sessionMap.erase(it);
		}
	}
}
