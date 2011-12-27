#pragma once
#ifndef HTTPENGINE_HEDRIVER_COMMAND_HXX
#define HTTPENGINE_HEDRIVER_COMMAND_HXX

#include "IHEDriverCommand.hxx"
#include "HESessionInfo.hxx"

namespace httpengine
{
	class HEDriverCommandBase : public IHEDriverCommand
	{
	public:
		HEDriverCommandBase(void* pMap);
	    virtual ~HEDriverCommandBase();

		virtual void execute(void* param);
	protected:
		void* mSessionMapPtr;
	};

	class HEDriverCommandAdd : public HEDriverCommandBase
	{
	public:
		HEDriverCommandAdd(void* pMap, HESessionInfoPtr ptrSession);
		virtual ~HEDriverCommandAdd();

		virtual void execute(void* param);

	protected:
		HESessionInfoPtr mHESessionInfoPtr;
	};

	class HEDriverCommandRemove : public HEDriverCommandBase
	{
	public:
		HEDriverCommandRemove(void* pMap,const int iKey);
		virtual ~HEDriverCommandRemove();

		virtual void execute(void* param);

	protected:
		int mDelKey;
	};
}

#endif //__DL_MULTI_RUNNER_CMD_H__