#include "global.h"
#include "base/file.h"
#include "stream.h"

Stream::Stream(ZFile* file)
{
	_file = file;
	_unusedBits = 0;
	_unusedData = 0;
}
Stream::~Stream()
{

}

int	Stream::getPosition() const
{
	return _file->getPosition();
}

void Stream::setPosition(int pos)
{
	_file->setPosition(pos);
}

int Stream::readBytes(void* bytes, int numBytes)
{
	return _file->readBytes(bytes, numBytes);
}

UInt32 Stream::readUInt(int bitCount)
{
	Assert(bitCount <= 32 && bitCount >= 0);

	UInt32	value = 0;

	int	bitsNeeded = bitCount;
	while (bitsNeeded > 0)
	{
		if (_unusedBits) 
		{
			if (bitsNeeded >= _unusedBits) 
			{
				// Consume all the unused bits.
				value |= (_unusedData << (bitsNeeded - _unusedBits));

				bitsNeeded -= _unusedBits;

				_unusedData = 0;
				_unusedBits = 0;
			}
			else 
			{
				// Consume some of the unused bits.
				value |= (_unusedData >> (_unusedBits - bitsNeeded));

				// mask off the bits we consumed.
				_unusedData &= ((1 << (_unusedBits - bitsNeeded)) - 1);

				_unusedBits -= bitsNeeded;

				// We're done.
				bitsNeeded = 0;
			}
		}
		else 
		{
			_unusedData = this->readByte();
			_unusedBits = 8;
		}
	}

	Assert(bitsNeeded == 0);

	return value;
}

SInt32 Stream::readSInt(int bitCount)
{
	Assert(bitCount <= 32 && bitCount >= 0);

	SInt32	value = (SInt32)this->readUInt(bitCount);

	// Sign extend...
	if (value & (1 << (bitCount - 1))) 
	{
		value |= -1 << bitCount;
	}

	return value;
}

SWF::Tag Stream::openTag(TagInfo* info)
{
	align();
	int	tagHeader = readUI16();
	info->tagType = (SWF::Tag)(tagHeader >> 6);
	info->tagLength = tagHeader & 0x3F;
	
	if (info->tagLength == 0x3F) 
	{
		info->tagLength = readSI32();
	}

	// Remember where the end of the tag is, so we can
	// fast-forward past it when we're done reading it.
	_tagStack.push_back(getPosition()+info->tagLength);

	return info->tagType;
}

void Stream::closeTag()
{
	Assert(_tagStack.size() > 0);
	int	endPos = _tagStack.back();
	_tagStack.pop_back();

	_file->setPosition(endPos);

	_unusedBits = 0;
}