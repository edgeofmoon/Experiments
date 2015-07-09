#include "MyMesh.h"
#include <fstream>
#include <map>
#include <iostream>
using namespace std;

MyMesh::MyMesh()
{
}


MyMesh::~MyMesh()
{
}

int MyMesh::Read(const MyString& fileName){
	ifstream infile(fileName);
	if (!infile.is_open()){
		return -1;
	}
	char line[1000];
	char id;
	MyVec3f p;
	MyVec3i t;
	while (!infile.eof())
	{
		infile >> id;
		switch (id)
		{
		case 'v':
		case 'V':
			infile >> p[0] >> p[1] >> p[2];
			mVertices << p;
			mBox.Engulf(p);
			break;
		case 'n':
		case 'N':
			infile >> p[0] >> p[1] >> p[2];
			mNormals << p;
			break;
		case 'f':
		case 'F':
			infile >> t[0] >> t[1] >> t[2];
			t -= MyVec3i(1, 1, 1);
			mTriangles << t;
			break;
		case '#':
		default:
			infile.getline(line, 1000);
			break;
		}
	}
	return 1;
}

int MyMesh::GenPerVertexNormal(){
	MyArray4f sumNormal(this->GetNumVertex(), MyVec4f(0, 0, 0, 0));
	for (int i = 0; i < this->GetNumTriangle(); i++){
		MyVec3i triangle = this->GetTriangle(i);
		MyVec3f normal = this->ComputeTriangleNormal(triangle);
		MyVec4f normal2add = MyVec4f(normal[0], normal[1], normal[2], 1);
		sumNormal[triangle[0]] += normal2add;
		sumNormal[triangle[1]] += normal2add;
		sumNormal[triangle[2]] += normal2add;
	}
	mNormals.clear();
	mNormals.resize(this->GetNumVertex());
	for (int i = 0; i < this->GetNumVertex(); i++){
		MyVec4f normal = sumNormal[i];
		if (normal[3] != 0){
			normal /= normal[3];
		}
		mNormals[i] = MyVec3f(normal[0], normal[1], normal[2]).normalized();
	}
	return 1;
}

int MyMesh::Merge(const MyMesh& mesh){
	int vertexSize = mVertices.size();
	mVertices += mesh.mVertices;
	mNormals += mesh.mNormals;
	mTriangles.reserve(mTriangles.size() + mesh.mTriangles.size());
	MyVec3i offset(vertexSize, vertexSize, vertexSize);
	for (int i = 0; i < mesh.mTriangles.size(); i++){
		mTriangles << mesh.mTriangles[i] + offset;
	}
	mBox.Engulf(mesh.mBox);
	return 1;
}

MyVec3f MyMesh::GetVertex(int idx) const{
	return mVertices[idx];
}

MyVec3f MyMesh::GetNormal(int idx) const{
	return mNormals[idx];
}

MyVec3i MyMesh::GetTriangle(int idx) const{
	return mTriangles[idx];
}

MyBoundingBox MyMesh::GetBoundingBox() const{
	return mBox;
}

int MyMesh::GetNumVertex() const{
	return mVertices.size();
}

int MyMesh::GetNumNormal() const{
	return mNormals.size();
}
int MyMesh::GetNumTriangle() const{
	return mTriangles.size();
}


const float* MyMesh::GetVertexData() const{
	return &mVertices[0][0];
}

const float* MyMesh::GetNormalData() const{
	return &mNormals[0][0];
}

const int* MyMesh::GetTriangleData() const{
	return &mTriangles[0][0];
}

MyVec3f MyMesh::ComputeTriangleNormal(int idx) const{
	MyVec3i triangle = this->GetTriangle(idx);
	MyVec3f p0 = this->GetVertex(triangle[0]);
	MyVec3f p1 = this->GetVertex(triangle[1]);
	MyVec3f p2 = this->GetVertex(triangle[2]);
	return (p1 - p0) ^ (p2 - p1);
}

MyVec3f MyMesh::ComputeTriangleNormal(const MyVec3i& triangle) const{
	MyVec3f p0 = this->GetVertex(triangle[0]);
	MyVec3f p1 = this->GetVertex(triangle[1]);
	MyVec3f p2 = this->GetVertex(triangle[2]);
	return (p1 - p0) ^ (p2 - p1);
}

void makeEdge(MyVec2i& edge){
	if (edge[0] > edge[1]){
		int tmp = edge[0];
		edge[0] = edge[1];
		edge[1] = tmp;
	}
}


void increaseEdgeTriangleCount(map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)>& edgeNumTriangle, MyVec2i edge){
	map<MyVec2i, int>::iterator itr = edgeNumTriangle.find(edge);
	if (itr != edgeNumTriangle.end())
		itr->second++;
	else
		edgeNumTriangle[edge] = 1;
}

MyVec2i MyMesh::makeEdge(int i, int j){
	MyVec2i edge(i, j);
	if (i > j){
		edge[0] = j;
		edge[1] = i;
	}
	return edge;
}

bool MyMesh::compareEdge(MyVec2i edge0, MyVec2i edge1){
	if (edge0[0] < edge1[0]) return true;
	if (edge0[0] > edge1[0]) return false;
	if (edge0[1] < edge1[1]) return true;
	//if (edge0[1] > edge1[1]) return false;
	return false;
}

int MyMesh::ClearNonRegularFaces(){
	int numTriangles = mTriangles.size();
	MyArrayb removeTag(numTriangles, false);

	bool(*fn_pt)(MyVec2i, MyVec2i) = compareEdge;
	map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)> edgeNumTriangle(compareEdge);
	for (int i = 0; i < numTriangles; i++){
		if (!removeTag[i]){
			MyVec3i triangle = mTriangles[i];
			MyVec2i edge0 = makeEdge(triangle[0], triangle[1]);
			MyVec2i edge1 = makeEdge(triangle[1], triangle[2]);
			MyVec2i edge2 = makeEdge(triangle[2], triangle[0]);
			increaseEdgeTriangleCount(edgeNumTriangle, edge0);
			increaseEdgeTriangleCount(edgeNumTriangle, edge1);
			increaseEdgeTriangleCount(edgeNumTriangle, edge2);
		}
	}

	int numBadEdge = 0;
	int evilEdge = 0;
	for (map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)>::iterator itr = edgeNumTriangle.begin();
		itr != edgeNumTriangle.end(); itr++){
		if (!(itr->second > 1)){
			numBadEdge++;
		}
		if (itr->second > 2){
			evilEdge++;
		}
	}
	cout << "number bad edge: " << numBadEdge << endl;
	cout << "number evil edge: " << evilEdge << endl;

	int numRemoval = 0;
	int numItr = 0;
	do{
		numRemoval = 0;
		for (int i = 0; i < numTriangles; i++){
			if (!removeTag[i]){
				MyVec3i triangle = mTriangles[i];
				MyVec2i edge0 = makeEdge(triangle[0], triangle[1]);
				MyVec2i edge1 = makeEdge(triangle[1], triangle[2]);
				MyVec2i edge2 = makeEdge(triangle[2], triangle[0]);
				if (edgeNumTriangle[edge0] == 1){
					removeTag[i] = true;
					edgeNumTriangle[edge0] --;
					edgeNumTriangle[edge1] --;
					edgeNumTriangle[edge2] --;
					numRemoval++;
					cout << "remove triange: " << i << endl;
					continue;
				}
				if (edgeNumTriangle[edge1] == 1){
					removeTag[i] = true;
					edgeNumTriangle[edge0] --;
					edgeNumTriangle[edge1] --;
					edgeNumTriangle[edge2] --;
					numRemoval++;
					cout << "remove triange: " << i << endl;
					continue;
				}
				if (edgeNumTriangle[edge2] == 1){
					removeTag[i] = true;
					edgeNumTriangle[edge0] --;
					edgeNumTriangle[edge1] --;
					edgeNumTriangle[edge2] --;
					numRemoval++;
					cout << "remove triange: " << i << endl;
					continue;
				}
			}
		}
		cout << numRemoval << " removed in iteration " << numItr << endl;
		numItr++;
	} while (numRemoval>0);

	numBadEdge = 0;
	evilEdge = 0;
	for (map<MyVec2i, int, bool(*)(MyVec2i, MyVec2i)>::iterator itr = edgeNumTriangle.begin();
		itr != edgeNumTriangle.end(); itr++){
		if (!(itr->second > 1)){
			numBadEdge++;
		}
		if (itr->second > 2){
			evilEdge++;
		}
	}
	cout << "number bad edge: " << numBadEdge << endl;
	cout << "number evil edge: " << evilEdge << endl;

	int health = 0;
	for (int i = 0; i < numTriangles; i++){
		if (!removeTag[i]){
			mTriangles[health++] = mTriangles[i];
		}
	}
	mTriangles.resize(health);

	return 1;
}