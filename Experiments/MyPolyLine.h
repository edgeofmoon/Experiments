#pragma once

#include "MyLine.h"
#include "MyBoundingBox.h"
#include "MyUtility.h"
#include "MyMathHelper.h"
#include <algorithm>

template<typename T = float, int n = 2>
class MyPolyline
	:public MyLine<T,n>{
public:
	MyPolyline();
	MyPolyline(const MyArray<MyVec<T,n>>& ctlPoints);
	~MyPolyline(){};

	void SetLine(const MyArray<MyVec<T,n>>& ctlPoints);
	void SetLoop(bool loop = true);
	bool GetLoop() const;
	int GetNumLineSegments() const;
	MyLine<T,n> GetLineSegment(int i) const;
	
	static bool Intersected(const MyLine<T, 2>& line, const MyPolyline<T, 2>& polyline);
	static bool Intersected(const MyPolyline<T, 2>& line1, const MyPolyline<T, 2>& line2);
	bool Intersected(const MyLine<T, 2>& line);
	bool Intersected(const MyPolyline<T, 2>& line);

	int GetNumIntersections(const MyLine<T, 2>& line) const;
	MyArray<MyVec<T,2>>* MakeIntersectionArray(const MyLine<T, 2>& line) const;
	MyArray<MyLine<T,2>>* MakeClippedLineArray(const MyLine<T,2>& line);
	MyPolyline<T,n>* MakeBezierCurve(int nPoints) const;

	bool IsPointIn(const MyVec<T,2>& p) const;

protected:
	bool mLoop;
};

template<typename T, int n>
MyPolyline<T,n>::MyPolyline(){
	mLoop = false;
}

template<typename T, int n>
MyPolyline<T,n>::MyPolyline(const MyArray<MyVec<T,n>>& ctlPoints){
	this->SetLine(ctlPoints);
	mLoop = false;
}


template<typename T, int n>
void MyPolyline<T,n>::SetLine(const MyArray<MyVec<T,n>>& ctlPoints){
	mVertices = ctlPoints;
}

template<typename T, int n>
void MyPolyline<T,n>::SetLoop(bool loop){
	mLoop = loop;
}

template<typename T, int n>
bool MyPolyline<T,n>::GetLoop() const{
	return mLoop;
}

template<typename T, int n>
int MyPolyline<T,n>::GetNumLineSegments() const{
	int modify = (this->GetLoop()?1:0);
	return this->GetNumPoints()-1+modify;
}
template<typename T, int n>
MyLine<T,n> MyPolyline<T,n>::GetLineSegment(int i) const{
	if(!mLoop)
		return MyLine<T,n>(mVertices[i],mVertices[i+1]);
	else
		return MyLine<T,n>(mVertices[i%(mVertices.size())],mVertices[(i+1)%(mVertices.size())]);
}

template<typename T, int n>
bool MyPolyline<T,n>::Intersected(const MyLine<T, 2>& line, const MyPolyline<T, 2>& polyline){
	for(int i = 0;i<polyline.GetNumLineSegments();i++){
		if(MyLine2f::Intersected(line,polyline.GetLineSegment(i))){
			return true;
		}
	}
	return false;
}

template<typename T, int n>
bool MyPolyline<T,n>::Intersected(const MyPolyline<T, 2>& line1, const MyPolyline<T, 2>& line2){
	for(int i = 0;i<line1.GetNumLineSegments();i++){
		if(MyLine2f::Intersected(line1.GetLineSegment(i),line2)){
			return true;
		}
	}
	return false;
}

template<typename T, int n>
bool MyPolyline<T,n>::Intersected(const MyLine<T, 2>& line){
	return MyPolyline<T,n>::Intersected(line, *this);
}
	
template<typename T, int n>
bool MyPolyline<T,n>::Intersected(const MyPolyline<T, 2>& line){
	return MyPolyline<T,n>::Intersected(line, *this);
}

template<typename T, int n>
int MyPolyline<T,n>::GetNumIntersections(const MyLine<T, 2>& line) const{
	int n = 0;
	for(int i = 0;i<this->GetNumLineSegments();i++){
		if(this->GetLineSegment(i).Intersected(line)){
			n++;
		}
	}
	return n;
}

template<typename T, int n>
MyArray<MyVec<T,2>>* MyPolyline<T,n>::MakeIntersectionArray(const MyLine<T, 2>& line) const{
	MyArray<MyVec<T,2>>* rst = new MyArray<MyVec<T,2>>;
	for(int i = 0;i<this->GetNumLineSegments();i++){
		MyLine<T,2> lineSeg = this->GetLineSegment(i);
		if(lineSeg.Intersected(line)){
			MyVec<T,2> intersection = lineSeg.GetIntersection(line);
			rst->push_back(intersection);
		}
	}
	return rst;
}

template<typename T, int n>
MyArray<MyLine<T,2>>* MyPolyline<T,n>::MakeClippedLineArray(const MyLine<T,2>& line){
	MyArray<MyLine<T,2>>* rst = new MyArray<MyLine<T,2>>;
	
	MyArray<MyVec<T,2>>* intersections = this->MakeIntersectionArray(line);

	MyArray<T> ts;
	MyVec<T,2> lineDir = (line.GetEnd()-line.GetStart()).normalized();
	for(int i = 0;i<intersections->size();i++){
		T t = (intersections->at(i)-line.GetStart())*lineDir;
		ts.push_back(t);
	}

	std::sort(ts.begin(), ts.end());

	bool lineIn = this->IsPointIn(line.GetStart());

	MyVec<T,2> start = line.GetStart();
	MyVec<T,2> next;
	for(int i = 0;i<ts.size();i++){
		next = line.GetStart() + lineDir*ts[i];
		if(lineIn){
			rst->PushBack(MyLine<T,2>(start, next));
		}
		
		lineIn = !lineIn;
		start = next;
	}

	return rst;
}

template<typename T, int n>
bool MyPolyline<T,n>::IsPointIn(const MyVec<T,2>& p) const{
	//if(!mLoop) return false;
	MyVec<T,2> inf(MY_LARGE_FLOAT,MY_LARGE_FLOAT);
	MyLine<T,2> line(p, inf);
	if(this->GetNumIntersections(line) %2 == 0){
		return false;
	}
}

template<typename T, int n>
MyPolyline<T,n>* MyPolyline<T,n>::MakeBezierCurve(int nPoints) const{
	MyArray<MyVec<T,n>> vertices(nPoints);
	vertices.front() = mVertices.front();
	int np = mVertices.size();
	for(int ip = 1;ip<nPoints-1;ip++){
		float t = (float)ip/(nPoints-1);
		MyVec<T,n> p = MyVec<T,n>::zero();
		for(int i = 0;i<np;i++){
			int bc = MyMathHelper::BinomialCoefficient(np, i);
			T bINT = bc*_Pow_int(t,i)*_Pow_int(1-t,np-i);
			p += bINT*mVertices[i];
		}
		vertices[ip] = p;
	}
	vertices.back() = mVertices.back();
	return new MyPolyline<T,n>(vertices);
}

typedef MyPolyline<float,2> MyPolyline2f;
typedef MyPolyline<float,3> MyPolyline3f;