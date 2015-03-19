#pragma once
#include "MyColor4.h"
#include "MyString.h"

class MyImage
{
public:
	MyImage(void);
	~MyImage(void);

	virtual int Open(const MyString& filename) = 0;
	virtual int Save(const MyString& filename) = 0;

	virtual MyColor4f GetColor(int x, int y) const = 0;
	virtual int GetPixel(int x, int y) const = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;

	virtual MyColor4f* MakeColorBuffer() const = 0;

	// row prior
	virtual unsigned char* MakePixelBufferRGB() const = 0;
	virtual unsigned char* MakePixelBufferRGBA() const = 0;
	virtual const unsigned char* GetPixelBufferRGB() const {return 0;};
	virtual const unsigned char* GetPixelBufferRGBA() const {return 0;};
};

