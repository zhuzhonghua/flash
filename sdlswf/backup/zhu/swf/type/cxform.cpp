#include "cxform.h"

CXForm CXForm::identity;

CXForm::CXForm()
{
	_m[0][0] = 1;
	_m[1][0] = 1;
	_m[2][0] = 1;
	_m[3][0] = 1;
	_m[0][1] = 0;
	_m[1][1] = 0;
	_m[2][1] = 0;
	_m[3][1] = 0;
}

void CXForm::concatenate(const CXForm& c)
{
	_m[0][1] += _m[0][0] * c._m[0][1];
	_m[1][1] += _m[1][0] * c._m[1][1];
	_m[2][1] += _m[2][0] * c._m[2][1];
	_m[3][1] += _m[3][0] * c._m[3][1];

	_m[0][0] *= c._m[0][0];
	_m[1][0] *= c._m[1][0];
	_m[2][0] *= c._m[2][0];
	_m[3][0] *= c._m[3][0];
}

RGBA CXForm::transform(const RGBA in) const
{
	RGBA	result;

	result._r = (UInt8)Base::fclamp(in._r * _m[0][0] + _m[0][1], 0, 255);
	result._g = (UInt8)Base::fclamp(in._g * _m[1][0] + _m[1][1], 0, 255);
	result._b = (UInt8)Base::fclamp(in._b * _m[2][0] + _m[2][1], 0, 255);
	result._a = (UInt8)Base::fclamp(in._a * _m[3][0] + _m[3][1], 0, 255);

	return result;
}

void CXForm::readRGB(Stream* in)
{
	in->align();

	int	has_add = in->readUInt(1);
	int	has_mult = in->readUInt(1);
	int	nbits = in->readUInt(4);

	if (has_mult) 
	{
		_m[0][0] = in->readSInt(nbits) / 255.0f;
		_m[1][0] = in->readSInt(nbits) / 255.0f;
		_m[2][0] = in->readSInt(nbits) / 255.0f;
		_m[3][0] = 1;
	}
	else 
	{
		for (int i = 0; i < 4; i++) 
		{ 
			_m[i][0] = 1; 
		}
	}

	if (has_add) 
	{
		_m[0][1] = (float)in->readSInt(nbits);
		_m[1][1] = (float)in->readSInt(nbits);
		_m[2][1] = (float)in->readSInt(nbits);
		_m[3][1] = 1;
	}
	else 
	{
		for (int i = 0; i < 4; i++) 
		{ 
			_m[i][1] = 0; 
		}
	}
}

void CXForm::readRGBA(Stream* in)
{
	in->align();

	int	has_add = in->readUInt(1);
	int	has_mult = in->readUInt(1);
	int	nbits = in->readUInt(4);

	if (has_mult) 
	{
		_m[0][0] = in->readSInt(nbits) / 256.0f;
		_m[1][0] = in->readSInt(nbits) / 256.0f;
		_m[2][0] = in->readSInt(nbits) / 256.0f;
		_m[3][0] = in->readSInt(nbits) / 256.0f;
	}
	else 
	{
		for (int i = 0; i < 4; i++) 
		{ 
			_m[i][0] = 1; 
		}
	}
	if (has_add) 
	{
		_m[0][1] = (float)in->readSInt(nbits);
		_m[1][1] = (float)in->readSInt(nbits);
		_m[2][1] = (float)in->readSInt(nbits);
		_m[3][1] = (float)in->readSInt(nbits);
	}
	else 
	{
		for (int i = 0; i < 4; i++) 
		{ 
			_m[i][1] = 0; 
		}
	}
}

void CXForm::clamp()  // Force component values to be in range.
{
	_m[0][0] = Base::fclamp(_m[0][0], 0, 1);
	_m[1][0] = Base::fclamp(_m[1][0], 0, 1);
	_m[2][0] = Base::fclamp(_m[2][0], 0, 1);
	_m[3][0] = Base::fclamp(_m[3][0], 0, 1);

	_m[0][1] = Base::fclamp(_m[0][1], -255.0f, 255.0f);
	_m[1][1] = Base::fclamp(_m[1][1], -255.0f, 255.0f);
	_m[2][1] = Base::fclamp(_m[2][1], -255.0f, 255.0f);
	_m[3][1] = Base::fclamp(_m[3][1], -255.0f, 255.0f);
}
