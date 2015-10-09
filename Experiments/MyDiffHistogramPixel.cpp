#include "MyDiffHistogramPixel.h"
#include <algorithm>
using namespace std;

std::vector<float>* MyDiffHistogramPixel::static_values_for_sort = 0;

MyDiffHistogramPixel::MyDiffHistogramPixel()
{
}


MyDiffHistogramPixel::~MyDiffHistogramPixel()
{
}


long MyDiffHistogramPixel::GetNumSegs() const{
	return mSegStartIndices.size();
}

long MyDiffHistogramPixel::GetNumPixels(int segIdx) const{
	if (segIdx < mSegStartIndices.size() - 1){
		return mSegStartIndices[segIdx + 1] - mSegStartIndices[segIdx];
	}
	else if (segIdx == mSegStartIndices.size()-1){
		return mPixelRefs.size() - mSegStartIndices[segIdx];
	}
	else{
		return 0;
	}
}

void MyDiffHistogramPixel::Build(float* d1, float* d2, 
	const std::vector<long>& indices, float binWidth, float minValue, float maxValue){
	if (indices.empty()) return;
	mPixelRefs = indices;
	std::vector<float> values(indices.size());
	std::vector<long> valueIdx(indices.size());
	for (int i = 0; i < indices.size(); i++){
		values[i] = *(d1 + indices[i]);
		valueIdx[i] = i;
	}
	static_values_for_sort = &values;
	sort(valueIdx.begin(), valueIdx.end(), isSmallerByIndex);

	int idx = 0;
	while (values[valueIdx[idx]] < minValue) idx++;
	for (float binStart = minValue; binStart < maxValue; binStart += binWidth){
		mSegStartIndices.push_back(idx);
		if (values[valueIdx[idx]] >= binStart){
			while (idx < valueIdx.size()-1){
				if (values[valueIdx[++idx]] >= binStart + binWidth){
					break;
				}
			}
		}
	}

	std::vector<float> diffValues(indices.size());
	diffValues.resize(indices.size());
	for (int i = 0; i < indices.size(); i++){
		float v0 = *(d1 + indices[i]);
		float v1 = *(d2 + indices[i]);
		diffValues[i] = v0 - v1;
	}
	static_values_for_sort = &diffValues;
	std::vector<long>::iterator itr = valueIdx.begin();
	for (int i = 0; i < mSegStartIndices.size() - 1; i++){
		sort(itr + mSegStartIndices[i], itr + mSegStartIndices[i + 1], isSmallerByIndex);
	}
	sort(itr + mSegStartIndices.back(), valueIdx.end(), isSmallerByIndex);

	mPixelDiffValues.resize(valueIdx.size());
	mPixelRefs.resize(valueIdx.size());
	for (int i = 0; i < valueIdx.size(); i++){
		mPixelDiffValues[i] = diffValues[valueIdx[i]];
		mPixelRefs[i] = indices[valueIdx[i]];
	}
}

bool MyDiffHistogramPixel::isSmallerByIndex(long x, long y){
	return static_values_for_sort->at(x) < static_values_for_sort->at(y);
}