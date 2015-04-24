#include "MyTracts.h"
#include <algorithm>
#define LARGENUMBER 999999

MyTracts::MyTracts(void)
	:mBox(MyVec3f(LARGENUMBER,LARGENUMBER,LARGENUMBER), MyVec3f(-LARGENUMBER,-LARGENUMBER,-LARGENUMBER))
{
	mValueMin = LARGENUMBER;
	mValueMax = -LARGENUMBER;
	mTotalValue = 0;
}


MyTracts::~MyTracts(void)
{
}

MyTracts& operator<< (MyTracts& tracts, const MyVec3f& coord){
	tracts.mCoords.back() << coord;
	tracts.mBox.Engulf(coord);
	tracts.mTotalValue++;
	return tracts;
}

MyTracts& operator<< (MyTracts& tracts, const MyColor4f& color){
	tracts.mColors.back() << color;
	tracts.mValueMin = std::min(tracts.mValueMin, color.g);
	tracts.mValueMax = std::max(tracts.mValueMax, color.g);
	tracts.mTotalValue += color.g;
	//tracts.mTotalValue++;
	return tracts;
}

void MyTracts::NewTract(int size){
	MyArray3f coords;
	MyArray<MyColor4f> colors;
	if(size >= 0){
		coords.reserve(size);
		colors.reserve(size);
	}
	mCoords << coords;
	mColors << colors;
}

const MyVec3f& MyTracts::GetCoord(int i, int j) const{
	return mCoords[i][j];
}

const MyColor4f& MyTracts::GetColor(int i, int j) const{
	return mColors[i][j];
}

const float MyTracts::GetValue(int i, int j) const{
	return mColors[i][j].g;
}

int MyTracts::GetNumTracts() const{
	return mCoords.size();
}

int MyTracts::GetNumVertices(int i) const{
	return mCoords[i].size();
}

int MyTracts::GetTotalNumVertices() const{
	int n = 0;
	for(int i = 0;i<this->GetNumTracts();i++){
		n += this->GetNumVertices(i);
	}
	return n;
}

MyArrayf MyTracts::GetValueStats() const{
	MyArrayf stats;
	int total = GetTotalNumVertices();
	stats << mValueMin
		<< (total == 0 ? 0 : mTotalValue / total)
		<< mValueMax;
	return stats;;
}

MyArrayf MyTracts::GetValueStats(const MyBoundingBox& box) const{
	float maxValue = mValueMin;
	float minValue = mValueMax;
	float nVertices = 0;
	float totalValue = 0;
	for (int i = 0; i < this->GetNumTracts(); i++){
		for (int j = 0; j < this->GetNumVertices(i); j++){
			if (box.IsIn(this->GetCoord(i, j))){
				float value = this->GetValue(i, j);
				maxValue = std::max(maxValue, value);
				minValue = std::min(minValue, value);
				totalValue += value;
				nVertices++;
			}
		}
	}
	MyArrayf stats;
	if (nVertices > 0){
		stats << minValue
			<< totalValue / nVertices
			<< maxValue;
	}
	return stats;
}


MyBoundingBox MyTracts::GetBoundingBox() const{
	return mBox;
}