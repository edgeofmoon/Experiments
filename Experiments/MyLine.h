#pragma once

#include "MyVec.h"
#include "MyArray.h"

template<typename T = float, int n = 2>
class MyLine{
public:
	MyLine();
	MyLine(const MyVec<T, n>& start, const MyVec<T, n>& end);
	~MyLine(){};

	void SetLine( const MyVec<T, n>& start, const MyVec<T, n>& end);

	void SetStart(const MyVec<T, n>& start);
	void SetEnd(const MyVec<T, n>& end);

	const MyVec<T, n>& GetStart() const;
	const MyVec<T, n>& GetEnd() const;

	MyVec<T, n> GetUnitVector() const;

	virtual const MyVec<T, n>& GetPoint(int i) const;
	virtual int GetNumPoints() const;
	virtual T GetLength() const;

	virtual const MyArray<MyVec<T, n>>& GetVertices() const{
		return mVertices;
	}

	// this only works for n == 2
	static bool Intersected(const MyLine<T, 2>& line1, const MyLine<T, 2>& line2);
	bool Intersected(const MyLine<T, 2>& line) const;
	static MyVec<T,2> GetIntersection(const MyLine<T, 2>& line1, const MyLine<T, 2>& line2);
	MyVec<T,2> GetIntersection(const MyLine<T,2>& line) const;
	T GetSlope() const;
	T GetOffset() const;
	

protected:
	MyArray<MyVec<T, n>> mVertices;
};

template<typename T, int n>
MyLine<T,n>::MyLine(){
	mVertices.resize(2);
}

template<typename T, int n>
MyLine<T,n>::MyLine(const MyVec<T, n>& start, const MyVec<T, n>& end){
	this->SetLine(start,end);
}

template<typename T, int n>
void MyLine<T,n>::SetLine( const MyVec<T, n>& start, const MyVec<T, n>& end){
	mVertices.resize(2);
	mVertices[0] = start;
	mVertices[1] = end;
}
	
template<typename T, int n>
void MyLine<T,n>::SetStart(const MyVec<T, n>& start){
	mVertices.front() = start;
}
	
template<typename T, int n>
void MyLine<T,n>::SetEnd(const MyVec<T, n>& end){
	mVertices.back() = end;
}
	
template<typename T, int n>
const MyVec<T, n>& MyLine<T,n>::GetStart() const{
	return mVertices.front();
}
	
template<typename T, int n>
const MyVec<T, n>& MyLine<T,n>::GetEnd() const{
	return mVertices.back();
}

template<typename T, int n>
MyVec<T, n> MyLine<T,n>::GetUnitVector() const{
	return (this->GetEnd() - this->GetStart()).normalized();
}

template<typename T, int n>
const MyVec<T, n>& MyLine<T,n>::GetPoint(int i) const{
	return mVertices[i];
}

template<typename T, int n>
int MyLine<T,n>::GetNumPoints() const{
	return mVertices.size();
}

template<typename T, int n>
T MyLine<T,n>::GetLength() const{
	return (this->GetEnd()-this->GetStart()).norm();
}

template<typename T, int n>
bool MyLine<T,n>::Intersected(const MyLine<T, 2>& line1, const MyLine<T, 2>& line2){
	MyVec3f a1(line1.GetStart()[0], line1.GetStart()[1], 0);
	MyVec3f a2(line1.GetEnd()[0], line1.GetEnd()[1], 0);
	MyVec3f b1(line2.GetStart()[0], line2.GetStart()[1], 0);
	MyVec3f b2(line2.GetEnd()[0], line2.GetEnd()[1], 0);

	if(((a2-a1)^(b1-a1))*((a2-a1)^(b2-a1))>0){
		return false;
	}
	
	if(((b2-b1)^(a1-b1))*((b2-b1)^(a2-b1))>0){
		return false;
	}

	return true;
}

template<typename T, int n>
bool MyLine<T,n>::Intersected(const MyLine<T, 2>& line) const{
	return MyLine<T,2>::Intersected(*this, line);
}

template<typename T, int n>
MyVec<T,2> MyLine<T,n>::GetIntersection(const MyLine<T, 2>& line1, const MyLine<T, 2>& line2){
	if(line1.GetEnd()[0] == line1.GetStart()[0]){
		T a2 = line2.GetSlope();
		T b2 = line2.GetOffset();
		T y = a2*line1.GetEnd()[0] + b2;
		return MyVec<T,2>(line1.GetEnd()[0],y);
	}
	else if (line2.GetEnd()[0] == line2.GetStart()[0]){
		T a1 = line1.GetSlope();
		T b1 = line1.GetOffset();
		T y = a1*line2.GetEnd()[0] + b1;
		return MyVec<T,2>(line2.GetEnd()[0],y);
	}
	else{
		T a1 = line1.GetSlope();
		T a2 = line2.GetSlope();
		if(a1 == a2){
			T length1 = line1.GetLength();
			T length2 = line2.GetLength();
			if(length1 >= length2){
				return (line2.GetStart()+line2.GetEnd())/2;
			}
			else{
				return (line1.GetStart()+line1.GetEnd())/2;
			}
		}
		T b1 = line1.GetOffset();
		T b2 = line2.GetOffset();

		T y = (b1*a2-b2*a1)/(a2-a1);
		T x;
		if(a1 == 0){
			x = (b1-b2)/a2;
		}
		else{
			x = (y-b1)/a1;
		}

		return MyVec<T,2>(x,y);
	}
}

template<typename T, int n>
MyVec<T,2> MyLine<T,n>::GetIntersection(const MyLine<T,2>& line) const{
	return MyLine<T,n>::GetIntersection(*this, line);
}

template<typename T, int n>
T MyLine<T,n>::GetSlope() const{
	T dis = this->GetEnd()[1] - this->GetStart()[1];
	T div = this->GetEnd()[0] - this->GetStart()[0];
	return dis/div;
}

template<typename T, int n>
T MyLine<T,n>::GetOffset() const{
	//return this->GetStart[1] - this->GetSlope()*this->GetSlope();
	T dis = this->GetStart()[1]*this->GetEnd()[0]
		- (this->GetEnd()[1]*this->GetStart()[0]);
	T div = this->GetEnd()[0] - this->GetStart()[0];
	return dis/div;
}

typedef MyLine<float, 3> MyLine3f;
typedef MyLine<float, 2> MyLine2f;