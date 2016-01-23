#ifndef Z_FILE_
#define Z_FILE_

#include "global.h"

class ZFile
{
public:
	~ZFile(){}
	virtual int	getPosition() const = 0;
	virtual void setPosition(int pos) = 0;
	virtual int readBytes(void* bytes, int numBytes) = 0;
	UInt32	readUI32() { UInt32 u; readBytes(&u, 4); return u; }
	UInt16	readUI16() { UInt16 u; readBytes(&u, 2); return u; }
	UInt8	readUI8() { UInt8 u; readBytes(&u, 1);; return u; }
};
#endif