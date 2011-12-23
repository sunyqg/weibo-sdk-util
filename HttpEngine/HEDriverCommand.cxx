#include "HEDriverCommand.hxx"
#include "HEDriver.hxx"

using namespace httpengine;

////////////////////////////////////////////////////////////////////////
// http engine driver base command

HEDriverCommandBase::HEDriverCommandBase(void* pMap)
: mSessionMapPtr(pMap)
{

}

HEDriverCommandBase::~HEDriverCommandBase()
{

}

void HEDriverCommandBase::execute(void* param)
{

}

////////////////////////////////////////////////////////////////////////
// http engine driver command - add

HEDriverCommandAdd::HEDriverCommandAdd(void* pMap, HESessionInfoPtr ptrSession)
: HEDriverCommandBase(pMap)
, mHESessionInfoPtr(ptrSession)
{
}

HEDriverCommandAdd::~HEDriverCommandAdd()
{
}

void HEDriverCommandAdd::execute(void* param)
{
	HEDriver::HESessionInfoPtrMap* pSessionMap = 
		reinterpret_cast<HEDriver::HESessionInfoPtrMap*>(mSessionMapPtr);

	if(!pSessionMap)
	{
		assert(false);
		return ;
	}

	if( pSessionMap->empty())
	{
		pSessionMap->insert(std::make_pair(mHESessionInfoPtr->details()->mKey, mHESessionInfoPtr));
	}
	else
	{
		HEDriver::HESessionInfoPtrMap::iterator it = pSessionMap->find(mHESessionInfoPtr->details()->mKey);
		if(it == pSessionMap->end())
		{
			pSessionMap->insert(std::make_pair(mHESessionInfoPtr->details()->mKey,mHESessionInfoPtr));
		}
	}
}

////////////////////////////////////////////////////////////////////////
// http engine driver command - remove

HEDriverCommandRemove::HEDriverCommandRemove(void *pMap,const int iKey)
: HEDriverCommandBase(pMap)
, mDelKey(iKey) 
{

}

HEDriverCommandRemove::~HEDriverCommandRemove()
{

}

void HEDriverCommandRemove::execute(void *param)
{
	HEDriver::HESessionInfoPtrMap* pSessionMap = 
		reinterpret_cast<HEDriver::HESessionInfoPtrMap*>(mSessionMapPtr);

	if (!pSessionMap)
	{
		assert(false);
		return ;
	}
	if (!pSessionMap->empty()) 
	{
		HEDriver::HESessionInfoPtrMap::iterator it = pSessionMap->find(mDelKey);
		if( it != pSessionMap->end())
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
			pSessionMap->erase(it);
		}
	}
}
