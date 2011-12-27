#ifndef HTTPENGINE_IHEDRIVERCOMMAND_HXX
#define HTTPENGINE_IHEDRIVERCOMMAND_HXX

#include <boost/shared_ptr.hpp>

namespace httpengine
{
	/** Session command for multi runner
	*
	* @author welbon
	*
	* @date 2011-04-19
	*/
	class IHEDriverCommand
	{
	public:
		virtual void execute(void *param) = 0;
	};
	typedef boost::shared_ptr<IHEDriverCommand> HEDriverCommandPtr;
}

#endif //__DL_MULTI_RUNNER_CMD_INTERFACE_H__