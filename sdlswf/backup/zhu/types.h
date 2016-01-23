#ifndef Z_TYPES
#define Z_TYPES

class ZFile;
typedef ZFile* (*FileOpener)(const char* path);

typedef char                SInt8;
typedef unsigned char       UInt8;
typedef short               SInt16;
typedef unsigned short      UInt16;
typedef int					SInt32;
typedef unsigned int		UInt32;

typedef UInt8				Byte;


#ifndef NULL
#define NULL	0
#endif

class ZFloat
{
public:
	ZFloat()	{ _float = 0.0f; }
	ZFloat(float x)	{ operator=(x); }
	ZFloat(double x)	{ operator=((float)x); }

	operator float() const { return _float; }
	inline void	operator=(float x)
	{
		_float = x >= -3.402823466e+38F && x <= 3.402823466e+38F ? x : 0.0f;
	}
	void	operator+=(const float x) { operator=(_float + x); }
	void	operator-=(const float x) { operator=(_float - x); }
	void	operator*=(const float x) { operator=(_float * x); }
	void	operator/=(const float x) { operator=(_float / x); }

private:
	float _float;
};

#endif