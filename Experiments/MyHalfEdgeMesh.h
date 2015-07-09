#pragma once
#include "MyMesh.h"
#include "MyArray.h"

class MyVertex;
class MyHalfEdge;
class MyFace;

class MyVertex{
public:
	long mIndex;
	long mEdge;
	MyVertex(){
		mIndex = -1;
		mEdge = -1;
	}
};

class MyFace{
public:
	long mEdge;
	MyFace(){
		mEdge = -1;
	}
};

class MyHalfEdge{
public:
	long mVertex;
	long mOpposite;
	long mNext;
	long mFace;
	MyHalfEdge(){
		mVertex = -1;
		mOpposite = -1;
		mNext = -1;
		mFace = -1;
	}
};

class MyHalfEdgeMesh :
	public MyMesh
{
public:
	MyHalfEdgeMesh();
	~MyHalfEdgeMesh();

	void ConstrucTopology();
	void CheckIntegrity();
	void FlipFaces();

protected:
	MyArray<MyVertex> mHeVetices;
	MyArray<MyFace> mHeFaces;
	MyArray<MyHalfEdge> mHeEdges;

	void flipFace(long face);
};

