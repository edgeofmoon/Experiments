#pragma once

#include <vector>

class MyDiffHistogramPixel
{
public:
	MyDiffHistogramPixel();
	~MyDiffHistogramPixel();

	void Build(float* d1, float* d2, const std::vector<long>& indices,
		float binWidth, float minValue, float maxValue);
	long GetNumSegs() const;
	long GetNumPixels(int segIdx) const;

	std::vector<long> mPixelRefs;
	std::vector<float> mPixelDiffValues;
	std::vector<long> mSegStartIndices;

protected:
	static std::vector<float>* static_values_for_sort;
	static bool isSmallerByIndex(long x, long y);
};

