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
		HEDriverCommandBase();
	  virtual ~HEDriverCommandBase();

		virtual void execute(HESessionInfoPtrMap& sessionMap, void* param);
	};

	class HEDriverCommandAdd : public HEDriverCommandBase
	{
	public:
		HEDriverCommandAdd(HESessionInfoPtr ptrSession);
		virtual ~HEDriverCommandAdd();

		virtual void execute(HESessionInfoPtrMap& sessionMap, void* param);

	protected:
		HESessionInfoPtr mHESessionInfoPtr;
	};

	class HEDriverCommandRemove : public HEDriverCommandBase
	{
	public:
		HEDriverCommandRemove(int iKey);
		virtual ~HEDriverCommandRemove();

		virtual void execute(HESessionInfoPtrMap& sessionMap, void* param);

	protected:
		int mDelKey;
	};
}

#endif //__DL_MULTI_RUNNER_CMD_H__