#ifndef _Z_CXFORM_H
#define _Z_CXFORM_H

#include "global.h"
#include "rgba.h"

class Stream;

struct CXForm
{
	ZFloat	_m[4][2];	// [RGBA][mult, add]

	CXForm();
	void	concatenate(const CXForm& c);
	RGBA	transform(const RGBA in) const;
	void	readRGB(Stream* in);
	void	readRGBA(Stream* in);
	void	clamp();  // Force component values to be in range.
	void	print() const;

	static CXForm	identity;
};

#endif