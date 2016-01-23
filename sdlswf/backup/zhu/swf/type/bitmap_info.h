#ifndef _BITMAP_INFO_H
#define _BITMAP_INFO_H

class BitmapInfo
{
public:
	virtual void layout() {};
	virtual void activate() {};
	virtual int getWidth() const { return 0; }
	virtual int getHeight() const { return 0; }
	virtual unsigned char* getData() const { return 0; }
	virtual int getBPP() const { return 0; }	// byte per pixel
};

#endif