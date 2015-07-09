#include "MyHalfEdgeMesh.h"
#include "map"
#include <iostream>
#include <list>
using namespace std;

MyHalfEdgeMesh::MyHalfEdgeMesh()
{
}


MyHalfEdgeMesh::~MyHalfEdgeMesh()
{
}

void MyHalfEdgeMesh::ConstrucTopology(){
	mHeVetices.resize(mVertices.size());
	for (int i = 0; i < mVertices.size(); i++){
		mHeVetices[i].mIndex = i;
	}

	map<MyVec2i, long, bool(*)(MyVec2i, MyVec2i)> edgeMap(compareEdge);

	mHeFaces.resize(mTriangles.size());
	for (int i = 0; i < mTriangles.size(); i++){
		MyVec3i triangle = mTriangles[i];
		int edgeIdx = mHeEdges.size();
		for (int j = 0; j < 3; j++){
			MyHalfEdge halfEdge;
			halfEdge.mVertex = triangle[(j + 1) % 3];
			halfEdge.mFace = i;
			mHeEdges.push_back(halfEdge);

			MyVec2i edge = makeEdge(triangle[j], triangle[(j + 1) % 3]);
			map<MyVec2i, long, bool(*)(MyVec2i, MyVec2i)>::iterator itr = edgeMap.find(edge);
			if (itr != edgeMap.end()){
				mHeEdges[itr->second].mOpposite = mHeEdges.size() - 1;
				mHeEdges.back().mOpposite = itr->second;
			}
			else{
				edgeMap[edge] = mHeEdges.size() - 1;
			}
		}
		for (int j = 0; j < 3; j++){
			mHeEdges[edgeIdx + j].mNext = edgeIdx + (j + 1)%3;
			mHeVetices[mHeEdges[edgeIdx + j].mVertex].mEdge = mHeEdges[edgeIdx + j].mNext;
		}
		mHeFaces[i].mEdge = edgeIdx;
	}
}

void MyHalfEdgeMesh::CheckIntegrity(){
	for (int i = 0; i < mHeVetices.size(); i++){
		if (mHeVetices[i].mIndex < 0){
			cout << "HeVertex " << i << "has no index" << endl;
		}
		if (mHeVetices[i].mEdge < 0){
			cout << "HeVertex " << i << "has no edge" << endl;
		}
	}

	for (int i = 0; i < mHeFaces.size(); i++){
		if (mHeFaces[i].mEdge < 0){
			cout << "HeFace " << i << "has no edge" << endl;
		}
	}

	for (int i = 0; i < mHeEdges.size(); i++){
		if (mHeEdges[i].mVertex < 0){
			cout << "HeEdges " << i << "has no vertex" << endl;
		}
		if (mHeEdges[i].mOpposite < 0){
			cout << "HeEdges " << i << "has no opposite" << endl;
		}
		if (mHeEdges[i].mNext < 0){
			cout << "HeEdges " << i << "has no next" << endl;
		}
		if (mHeEdges[i].mFace < 0){
			cout << "HeEdges " << i << "has no face" << endl;
		}
	}
}

void MyHalfEdgeMesh::flipFace(long face){
	long heEdge = mHeFaces[face].mEdge;
	long nextEdge = heEdge;
	vector<long> heEdges;
	vector<long> heVertices;
	do{
		heEdges.push_back(nextEdge);
		heVertices.push_back(mHeEdges[nextEdge].mVertex);
		nextEdge = mHeEdges[nextEdge].mNext;
	} while (nextEdge != heEdge);
	for (int i = 0; i < heEdges.size(); i++){
		mHeEdges[heEdges[i]].mVertex = heVertices[(i + heVertices.size() - 1) % heVertices.size()];
		mHeEdges[heEdges[i]].mNext = heEdges[(i + heEdges.size() - 1) % heEdges.size()];
	}

	int tmp = mTriangles[face][0];
	mTriangles[face][0] = mTriangles[face][1];
	mTriangles[face][1] = tmp;
}

void MyHalfEdgeMesh::FlipFaces(){

	if (mHeFaces.size() == 0) return;

	int numFaceFlipped = 0;
	int numFaceVisited = 0;
	vector<bool> faceVisited(mHeFaces.size(), false);
	list<long> face2visit;
	list<long> faceSupEdge;
	face2visit.push_back(0);
	faceSupEdge.push_back(-1);
	while (!face2visit.empty()){
		long face = face2visit.front();
		numFaceVisited++;
		face2visit.pop_front();
		long superEdge = faceSupEdge.front();
		faceSupEdge.pop_front();
		long heEdge = mHeFaces[face].mEdge;
		long nextEdge = heEdge;
		do{
			long oppositeEdge = mHeEdges[nextEdge].mOpposite;
			if (oppositeEdge >= 0){
				long theFace = mHeEdges[oppositeEdge].mFace;
				if (!faceVisited[theFace]){
					face2visit.push_back(theFace);
					faceSupEdge.push_back(nextEdge);
					faceVisited[theFace] = true;
				}
			}
			nextEdge = mHeEdges[nextEdge].mNext;
		} while (nextEdge != heEdge);

		// checkFace
		if (superEdge >= 0){
			long thisFaceNeighborEdge = mHeEdges[superEdge].mOpposite;
			if (mHeEdges[thisFaceNeighborEdge].mVertex 
				== mHeEdges[superEdge].mVertex){
				flipFace(face);
				numFaceFlipped++;
			}
		}
	}
	cout << "Face flipped: " << numFaceFlipped << endl;
	cout << "Face visited: " << numFaceVisited << " / " << mHeFaces.size() << endl;
}