#ifndef Z_STREAM_
#define Z_STREAM_

#include "global.h"
#include "enum.h"
#include <vector>

struct TagInfo
{
	SWF::Tag	tagType;
	int			tagOffset;
	int			tagLength;
	int			tagDataOffset;
};

class Stream
{
public:
	Stream(ZFile* file);
	~Stream();

	virtual int	getPosition() const;
	virtual void setPosition(int pos);
	virtual int readBytes(void* bytes, int numBytes);

	UInt32	readUI32() { UInt32 u; readBytes(&u, 4); return u; }
	UInt16	readUI16() { UInt16 u; readBytes(&u, 2); return u; }
	UInt8	readUI8() { UInt8 u; readBytes(&u, 1);; return u; }
	SInt32	readSI32() { SInt32 u; readBytes(&u, 4); return u; }
	SInt16	readSI16() { SInt16 u; readBytes(&u, 2); return u; }
	SInt8	readSI8() { SInt8 u; readBytes(&u, 1);; return u; }
	Byte	readByte() { return readUI8(); }

	UInt32	readUInt(int bitCount);
	SInt32	readSInt(int bitCount);
	void	align() { _unusedBits = 0;}

	SWF::Tag		openTag(TagInfo* info);
	void			closeTag();
private:
	ZFile*			_file;

	UInt8			_unusedBits;
	UInt8			_unusedData;

	std::vector<int>	_tagStack;
};
#endif