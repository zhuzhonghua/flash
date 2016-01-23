#include "system.h"
#include "swf/tag_loader.h"

using namespace System;

Config* Config::_s_inst;

Config* Config::get()
{
	if (!_s_inst)
	{
		_s_inst = new Config();
	}
	return _s_inst;
}

Config::Config()
{
	_fileOpener = NULL;
	_renderHandler = NULL;
}

void Config::setFileOpener(FileOpener opener)
{
	_fileOpener = opener;
}

void Config::setRenderHandler(RenderHandler* r)
{
	_renderHandler = r;
}