#pragma once

#include "MyFrameBuffer.h"
#include "MyVec.h"

class RicVolume;

class MyVolumeRenderer
{
public:
	MyVolumeRenderer();
	~MyVolumeRenderer();

	void Resize(int width, int height);
	void CompileShader();
	void Show(int winWidth, int winHeight);
	void LoadVolume(RicVolume& vol);
	void LoadVolume(float* vol, int x, int y, int z);
	void SetDensityThreshold(float thres){ mThreshold = thres; };
	void SetSampleRate(float sr){ mSampleRate = sr; };
	void SetDecayFactor(float df){ mDecayFactor = df; };

protected:
	// paremeter
	int mVolX, mVolY, mVolZ;
	float mThreshold;
	float mSampleRate;
	float mDecayFactor;

	unsigned int mPreIntegralLUT;
	unsigned int mVolumeTexture;
	int mShaderProgram;
	int mCubeProgram;

	frameBuffer mCubeFrameBuffer;
	unsigned int mCubeVertexArray;
	unsigned int mCubeVertexBuffer;
	unsigned int mCubeIndexBuffer;
	void LoadCubeData();
	void ComputePreIntegralLUT(int nDim = 256);
	void SetupCubeFrameBuffer();

	float ExtinctionCoefficent(float density) const;
	MyVec3f ColorFromDensity(float density) const;

	float ExtinctionCoefficentIntegral(float st, float ed, 
		float densityIn, float densityOut, float itgrStep = 0.01) const;
	MyVec4f ComputeLUTEntry(float densityIn, float densityOut) const;
};

