#pragma once
#include "myimage.h"
#include "MyString.h"

class MyBitmap :
	public MyImage
{
public:
	MyBitmap(void);
	~MyBitmap(void);

	virtual int Open(const MyString& filename);
	virtual int Save(const MyString& filename);

	virtual MyColor4f GetColor(int x, int y) const;
	virtual int GetPixel(int x, int y) const;
	virtual int GetWidth() const;
	virtual int GetHeight() const;

	virtual MyColor4f* MakeColorBuffer() const;

	virtual unsigned char* MakePixelBufferRGB() const;
	virtual unsigned char* MakePixelBufferRGBA() const;

	virtual const unsigned char* GetPixelBufferRGB() const;


protected:
	int mWidth, mHeight;
	unsigned char* mData;

	struct MyBmpHeader {
	//	char  type[2];       /* = "BM", it is matipulated separately to make the size of this structure the multiple of 4, */
		unsigned long sizeFile;      /* = offbits + sizeImage */
		unsigned long reserved;      /* == 0 */
		unsigned long offbits;       /* offset from start of file = 54 + size of palette */
		unsigned long sizeStruct;    /* 40 */
		unsigned long width, height;
		unsigned short  planes;        /* 1 */
		unsigned short  bitCount;      /* bits of each pixel, 256color it is 8, 24 color it is 24 */
		unsigned long compression;   /* 0 */
		unsigned long sizeImage;     /* (width+?)(till multiple of 4) * height£¬in bytes */
		unsigned long xPelsPerMeter; /* resolution in mspaint */
		unsigned long yPelsPerMeter;
		unsigned long colorUsed;     /* if use all, it is 0 */
		unsigned long colorImportant;/*  */
	} mHeader;
};

