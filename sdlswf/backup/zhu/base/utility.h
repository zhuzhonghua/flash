#ifndef Z_UTILITY_
#define Z_UTILITY_

#define Assert(x) do{if(!(x)) *(int*)0 = 0; }while(0);
#define UNUSED(x) (void) (x)

#define TWIPS_TO_PIXELS(x)	((x) / 20.f)
#define PIXELS_TO_TWIPS(x)	((x) * 20.f)

namespace Base
{
	template <typename T>
	const T Min(const T a, const T b)
	{
		return (a < b) ? a : b;
	}
	template <typename T>
	const T Max(const T a, const T b)
	{
		return (a < b) ? b : a;
	}

	inline int		fchop(float f) { return (int)f; }	// replace w/ inline asm if desired
	inline int		frnd(float f) { return fchop(f + 0.5f); }	// replace with inline asm if desired
	inline float	flerp(float a, float b, float f) { return (b - a) * f + a; }
	inline int		iclamp(int i, int min, int max) 
	{
		Assert(min <= max);
		return Max(min, Min(i, max));
	}

	inline float	fclamp(float f, float xmin, float xmax) {
		Assert(xmin <= xmax);
		return Max(xmin, Min(f, xmax));
	}

	inline float sqr(float number) {
		long i;
		float x, y;
		const float f = 1.5F;
		x = number * 0.5F;
		y = number;
		i = *(long *)&y;
		i = 0x5f3759df - (i >> 1);
		y = *(float *)&i;
		y = y * (f - (x * y * y));
		y = y * (f - (x * y * y));
		return number * y;
	}

	inline size_t	bernsteinHash(const void* data_in, int size, unsigned int seed = 5381)
		// Computes a hash of the given data buffer.
		// Hash function suggested by http://www.cs.yorku.ca/~oz/hash.html
		// Due to Dan Bernstein.  Allegedly very good on strings.
		//
		// One problem with this hash function is that e.g. if you take a
		// bunch of 32-bit ints and hash them, their hash values will be
		// concentrated toward zero, instead of randomly distributed in
		// [0,2^32-1], because of shifting up only 5 bits per byte.
	{
		const unsigned char*	data = (const unsigned char*)data_in;
		unsigned int	h = seed;
		while (size > 0) {
			size--;
			h = ((h << 5) + h) ^ (unsigned)data[size];
		}

		return h;
	}

}

#define ZNEW	new(__FILE__, __LINE__)

#endif