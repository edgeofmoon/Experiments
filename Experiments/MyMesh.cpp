#include "MyMesh.h"
#include <fstream>
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