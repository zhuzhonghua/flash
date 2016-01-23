#ifndef Z_ZLIBFILE_
#define Z_ZLIBFILE_

#include "file.h"
#include "zlib.h"
#include "global.h"

const int	ZBUF_SIZE = 4096;

class ZLibFile : public ZFile
{
public:
	ZLibFile(ZFile* file);
	~ZLibFile();
	virtual int	getPosition() const;
	virtual void setPosition(int pos);
	virtual int readBytes(void* bytes, int numBytes);
			void	reset();
private:
	ZFile*		_file;

	int			_initialPos;
	int			_logicalPos;

	z_stream	_zstream;

	Byte		_rawdata[ZBUF_SIZE];

	bool		_atEof;
	int			_error;
};
#endif