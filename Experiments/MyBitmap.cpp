#include "MyBitmap.h"
#include "MyUtility.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>




#define RGB(r, g, b) ((((r)&0xFF)<<16) + (((g)&0xFF)<<8) + ((b)&0xFF))

#define RGBA(r, g, b, a) ((((r)&0xFF)<<24) + (((g)&0xFF)<<16) + (((b)&0xFF)<<8) + ((a)&0xFF))

#define CEIL4(x) ((((x)+3)/4)*4)


MyBitmap::MyBitmap(void)
{
	mData = 0;
	mWidth = mHeight = 0;
}


MyBitmap::~MyBitmap(void)
{
	mWidth = mHeight = 0;
	SafeFreeArray(mData);
}

int MyBitmap::Open(const MyString& filename){


	FILE *fp;
	const char* fname = filename.c_str();

	if ((fp = fopen(fname, "rb")) == NULL){
		printf("Error: can't open bmp file \"%s\".\n", fname);
		return 1;
	}

	/* read head, and check its format */
	if (fgetc(fp) != 'B' || fgetc(fp) != 'M') {
		printf("Error: bmp file \"%s\" format error.", fname);
		goto ERR_EXIT;
	}

	/* read its infomation */
	fread(&mHeader, sizeof(MyBmpHeader), 1, fp);
	if (mHeader.sizeStruct != 40 || mHeader.reserved != 0){
		printf("Error: bmp file \"%s\" format error.", fname);
		goto ERR_EXIT;
	}

	/* check format */
	if (mHeader.bitCount != 24){
		printf("Sorry: bmp file \"%s\" bit count != 24", fname);
		goto ERR_EXIT;
	}

	/* close old bitmap */
	SafeFreeArray(mData);

	/* fill the bitmap struct */
	mWidth = mHeader.width;
	mHeight = mHeader.height;
	//mbmp.size = head.sizeImage;
	if (mHeader.sizeImage != CEIL4(mWidth * 3) * mHeight){ /* check size */
		printf("Error: bmp file \"%s\" size do not match!\n", fname);
		goto ERR_EXIT;
	}

	/* allocate memory */
	if ((mData = (unsigned char *)realloc(mData, mHeader.sizeImage)) == NULL){
		printf("Error: alloc fail!");
		goto ERR_EXIT;
	}
	/* read data into memory */
	if (fread(mData, 1, mHeader.sizeImage, fp) != mHeader.sizeImage){
		printf("Error: read data fail!");
		goto ERR_EXIT;
	}

	fclose(fp);
	return 1;

ERR_EXIT:
	fclose(fp);
	return 0;
}

int MyBitmap::Save(const MyString& filename){
	FILE *fp;
	MyBmpHeader head = {0, 0, 54, 40, 0, 0, 1, 24, 0, 0}; /* BMP file header */

	const char* fname = filename.c_str();

	if ((fp = fopen(fname, "wb")) == NULL) {
		printf("Error: can't save to BMP file \"%s\".\n", fname);
		return 1;
	}

	fputc('B', fp); fputc('M', fp); /* write type */
	/* fill BMP file header */
	head.width = mWidth;
	head.height = mHeight;
	head.sizeImage = mHeader.sizeImage;
	head.sizeFile = mHeader.sizeImage + head.offbits;
	fwrite(&head, sizeof head, 1, fp); /* write header */
	if (fwrite(mData, 1, mHeader.sizeImage, fp) != mHeader.sizeImage) {
		fclose(fp);
		return 1; /* write bitmap infomation */
	}

	fclose(fp);
	return 0;
}

MyColor4f MyBitmap::GetColor(int x, int y) const{
	float r = mData[(y*mWidth+x)*3+0]/(float)255;
	float g = mData[(y*mWidth+x)*3+1]/(float)255;
	float b = mData[(y*mWidth+x)*3+2]/(float)255;
	return MyColor4f(r,g,b,1);
}

int MyBitmap::GetPixel(int x, int y) const{
	unsigned char r = mData[(y*mWidth+x)*3+0];
	unsigned char g = mData[(y*mWidth+x)*3+1];
	unsigned char b = mData[(y*mWidth+x)*3+2];
	unsigned char a = 255;
	return RGBA(r,g,b,a);
}

int MyBitmap::GetWidth() const{
	return mWidth;
}

int MyBitmap::GetHeight() const{
	return mHeight;
}

MyColor4f* MyBitmap::MakeColorBuffer() const{
	MyColor4f* rst = new MyColor4f[mWidth*mHeight];
	for(int j = 0;j<mHeight;j++){
		for(int i = 0;i<mWidth;i++){
			rst[j*mWidth+i].r = mData[(j*mWidth+i)*3+0]/(float)255;
			rst[j*mWidth+i].g = mData[(j*mWidth+i)*3+1]/(float)255;
			rst[j*mWidth+i].b = mData[(j*mWidth+i)*3+2]/(float)255;
			rst[j*mWidth+i].a = 1;
		}
	}
	return rst;
}

unsigned char* MyBitmap::MakePixelBufferRGB() const{
	unsigned char* rst = new unsigned char[mWidth*mHeight*3];
	memcpy(rst, mData,mWidth*mHeight*3*sizeof(unsigned char));
	return rst;
}

unsigned char* MyBitmap::MakePixelBufferRGBA() const{
	unsigned char* rst = new unsigned char[mWidth*mHeight*4];
	for(int j = 0;j<mHeight;j++){
		for(int i = 0;i<mWidth;i++){
			rst[(j*mWidth+i)*4+0] = mData[(j*mWidth+i)*3+0];
			rst[(j*mWidth+i)*4+1] = mData[(j*mWidth+i)*3+1];
			rst[(j*mWidth+i)*4+2] = mData[(j*mWidth+i)*3+2];
			rst[(j*mWidth+i)*4+3] = 255;
		}
	}
	return rst;
}

const unsigned char* MyBitmap::GetPixelBufferRGB() const{
	return mData;
}