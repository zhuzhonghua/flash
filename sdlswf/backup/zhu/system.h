#ifndef Z_SYSTEM_
#define Z_SYSTEM_

#include <boost/noncopyable.hpp>

#include "global.h"

class RenderHandler;

namespace System
{
	class Config : public boost::noncopyable
	{
	public:
		static Config*		get();
		void				setFileOpener(FileOpener opener);
		FileOpener			getFileOpener(){ return _fileOpener; }
		RenderHandler*		getRenderHandler() { return _renderHandler; }
		void				setRenderHandler(RenderHandler* r);
	protected:
		Config();
	private:
		static Config*		_s_inst;
	private:
		FileOpener			_fileOpener;
		RenderHandler*		_renderHandler;
	};
}
#endif