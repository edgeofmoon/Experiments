#pragma once

#include "MyVec.h"

template<typename T, int n>
class MyBox
{
public:
	MyBox(void);
	MyBox(const MyVec<T,n>& low, const MyVec<T,n>& high);
	~MyBox(void);

	bool IsIntersected(const MyVec<T, n>& pt) const;
	bool IsIntersected(const MyBox<T, n>& box) const;
	void Set(const MyVec<T,n>& low, const MyVec<T,n>& high);
	void SetLow(const MyVec<T,n>& low);
	void SetHigh(const MyVec<T,n>& high);
	MyVec<T,n> GetLowPos() const;
	MyVec<T,n> GetHighPos() const;
	MyVec<T,n> GetCenter() const;
	T GetSize(int idx) const;
	MyVec<T,2> GetRange(int idx) const;

	MyBox<T,n> operator+(const MyVec<T,n>& offset) const;
	MyBox<T,n> operator-(const MyVec<T,n>& negoffset) const;
	MyBox<T,n> operator*(const T& enlarge) const;
	MyBox<T,n> operator/(const T& shrink) const;
	
	MyBox<T,n>& operator+=(const MyVec<T,n>& offset);
	MyBox<T,n>& operator-=(const MyVec<T,n>& negoffset);
	MyBox<T,n>& operator*=(const T& enlarge);
	MyBox<T,n>& operator/=(const T& shrink);
	
	template<typename T, int n>
	friend MyBox<T,n> operator+(const MyVec<T,n>& offset, const MyBox<T,n>& box);
	template<typename T, int n>
	friend MyBox<T,n> operator-(const MyVec<T,n>& negoffset, const MyBox<T,n>& box);
	template<typename T, int n>
	friend MyBox<T,n> operator*(const T& enlarge, const MyBox<T,n>& box);

protected:
	MyVec<T,n> mLow;
	MyVec<T,n> mHigh;
};

template<typename T, int n>
MyBox<T,n>::MyBox(void){
	this->SetLow(MyVec<T,n>::zero());
	this->SetHigh(MyVec<T,n>::zero());
}

template<typename T, int n>
MyBox<T,n>::MyBox(const MyVec<T,n>& low, const MyVec<T,n>& high){
	this->Set(low, high);
}

template<typename T, int n>
MyBox<T,n>::~MyBox(void){
}

template<typename T, int n>
bool MyBox<T, n>::IsIntersected(const MyVec<T, n>& pt) const{
	for (int i = 0; i < n; i++){
		if (mLow[i]>pt[i] || mHigh[i] < pt[i])
			return false;
	}
	return true;
}


template<typename T, int n>
bool MyBox<T, n>::IsIntersected(const MyBox<T, n>& box) const{
	for (int i = 0; i < n; i++){
		if (mLow[i]>box.mHigh[i] || mHigh[i] < box.mLow[i])
			return false;
	}
	return true;
}

template<typename T, int n>
void MyBox<T,n>::Set(const MyVec<T,n>& low, const MyVec<T,n>& high){
	this->SetLow(low);
	this->SetHigh(high);
}

template<typename T, int n>
void MyBox<T,n>::SetLow(const MyVec<T,n>& low){
	mLow = low;
}

template<typename T, int n>
void MyBox<T,n>::SetHigh(const MyVec<T,n>& high){
	mHigh = high;
}

template<typename T, int n>
MyVec<T,n> MyBox<T,n>::GetLowPos() const{
	return mLow;
}
	
template<typename T, int n>
MyVec<T,n> MyBox<T,n>::GetHighPos() const{
	return mHigh;
}
	
template<typename T, int n>
MyVec<T,n> MyBox<T,n>::GetCenter() const{
	return (mLow+mHigh)*0.5;
}
	
template<typename T, int n>
T MyBox<T,n>::GetSize(int idx) const{
	return mHigh[idx] - mLow[idx];
}

template<typename T, int n>
MyVec<T,2> MyBox<T,n>::GetRange(int idx) const{
	return MyVec<T,2>(mLow[idx], mHigh[idx]);
}
	
template<typename T, int n>
MyBox<T,n> MyBox<T,n>::operator+(const MyVec<T,n>& offset) const{
	return MyBox<T,n>(this->GetLowPos()+offset, this->GetHighPos()+offset);
}
	
template<typename T, int n>
MyBox<T,n> MyBox<T,n>::operator-(const MyVec<T,n>& negoffset) const{
	return MyBox<T,n>(this->GetLowPos()-negoffset, this->GetHighPos()-negoffset);
}
	
template<typename T, int n>
MyBox<T,n> MyBox<T,n>::operator*(const T& enlarge) const{
	return MyBox<T,n>(this->GetLowPos()*enlarge, this->GetHighPos()*enlarge);
}
	
template<typename T, int n>
MyBox<T,n> MyBox<T,n>::operator/(const T& shrink) const{
	return MyBox<T,n>(this->GetLowPos()/shrink, this->GetHighPos()/shrink);
}

template<typename T, int n>
MyBox<T,n>& MyBox<T,n>::operator+=(const MyVec<T,n>& offset){
	mLow += offset;
	mHigh += offset;
	return *this;
}

template<typename T, int n>
MyBox<T,n>& MyBox<T,n>::operator-=(const MyVec<T,n>& negoffset){
	mLow -= negoffset;
	mHigh -= negoffset;
	return *this;
}

template<typename T, int n>
MyBox<T,n>& MyBox<T,n>::operator*=(const T& enlarge){
	mLow *= enlarge;
	mHigh *= enlarge;
	return *this;
}

template<typename T, int n>
MyBox<T,n>& MyBox<T,n>::operator/=(const T& shrink){
	mLow /= shrink;
	mHigh /= shrink;
	return *this;
}

template<typename T, int n>
MyBox<T,n> operator+(const MyVec<T,n>& offset, const MyBox<T,n>& box){
	return box+offset;
}
	
template<typename T, int n>
MyBox<T,n> operator-(const MyVec<T,n>& negoffset, const MyBox<T,n>& box){
	return MyBox<T,n>(negoffset-box.GetHighPos(), negoffset-box.GetLowPos());
}
	
template<typename T, int n>
MyBox<T,n> operator*(const T& enlarge, const MyBox<T,n>& box){
	return box*enlarge;
}

typedef MyBox<float, 3> MyBox3f;
typedef MyBox<float, 2> MyBox2f;
typedef MyBox<int, 3> MyBox3i;
typedef MyBox<int, 2> MyBox2i;
