/*
 * MyTracks.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: GuohaoZhang
 */

#include "MyTracks.h"

#include "RicPoint.h"
#include "MyVec.h"
#include "Shader.h"
#include "MyGraphicsTool.h"

#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
using namespace std;

#include "GL\glew.h"
#include <GL/freeglut.h>

MyTracks::MyTracks(){
	mFaces = 6;
	mShape = TRACK_SHAPE_LINE;
}

MyTracks::MyTracks(const string& filename){
	Read(filename);
	mFaces = 6;
	mShape = TRACK_SHAPE_LINE;
}

int MyTracks::Read(const std::string& filename){
	FILE* fp;
	if((fp = fopen(filename.c_str(), "rb")) == NULL){
		printf("Error: cannot open file: %s\n", filename.c_str());
		return 0;
	}

	if( fread(&mHeader, sizeof(MyTrackHeader), 1, fp) != 1){
		printf("Error: cannot read header from: %s\n", filename.c_str());
		fclose(fp);
		return 0;
	}
	else printf("Info: %d tracts from file %s\n", mHeader.n_count, filename.c_str());

	cout << "Allocating Storage...\r";
	mTracks.clear();
	mTracks.resize(mHeader.n_count);


	for(int i = 0;i<mHeader.n_count; i++){
		if ((int)((i + 1) * 100 / (float)mHeader.n_count)
			- (int)(i * 100 / (float)mHeader.n_count) >= 1){
			cout << "Loading: " << i*100.f / mHeader.n_count << "%.\r";
		}
		MySingleTrackData& track = mTracks[i];
		fread(&track.mSize, sizeof(int), 1, fp);
		track.mPoints.resize(track.mSize);
		track.mPointScalars.resize(track.mSize);
		track.mTrackProperties.resize(track.mSize);
		for(int j = 0; j< track.mSize; j++){
			track.mPointScalars.resize(mHeader.n_scalars);
			fread(&track.mPoints[j], sizeof(Point), 1, fp);
			if(mHeader.n_scalars>0){
				fread(&track.mPointScalars[j][0], mHeader.n_scalars*sizeof(float), 1, fp);
			}
		}
		if(mHeader.n_properties>0){
			fread(&track.mTrackProperties[0], mHeader.n_properties*sizeof(float), 1, fp);
		}
	}
	cout << "Tracks loading completed.\n";
	fclose(fp);
	return 1;
}


int MyTracks::Save(const std::string& filename) const{
	FILE* fp;
	if((fp = fopen(filename.c_str(), "wb")) == NULL){
		printf("Error: cannot open file: %s\n", filename.c_str());
		return 0;
	}

	if(fwrite(&mHeader, sizeof(MyTrackHeader), 1, fp) != 1){
		printf("Error: cannot write header from: %s\n", filename.c_str());
		fclose(fp);
		return 0;
	}

	printf("Writing %d tracks...\n", mHeader.n_count);

	for(int i = 0;i<mHeader.n_count; i++){
		const MySingleTrackData& track = mTracks[i];
		fwrite(&track.mSize, sizeof(int), 1, fp);

		for(int j = 0; j< track.mSize; j++){
			fwrite(&track.mPoints[j], sizeof(Point), 1, fp);
			if(mHeader.n_scalars>0){
				fwrite(&track.mPointScalars[j][0], mHeader.n_scalars*sizeof(float), 1, fp);
			}
		}
		if(mHeader.n_properties>0){
			fwrite(&track.mTrackProperties[0], mHeader.n_properties*sizeof(float), 1, fp);
		}
	}
	fclose(fp);
	return 1;
}

MyTracks MyTracks::Subset(const std::vector<int>& trackIndices) const{
	MyTracks subset;
	subset.mHeader = this->mHeader;
	subset.mHeader.n_count = trackIndices.size();
	for(unsigned int i = 0;i<trackIndices.size(); i++){
		int trackIndex = trackIndices[i];
		subset.mTracks.push_back(this->mTracks[trackIndex]);
	}
	return subset;
}

void MyTracks::AddTracks(const MyTracks& tracks){
	mHeader.n_count += tracks.mHeader.n_count;
	for(unsigned int i = 0;i<tracks.mTracks.size(); i++){
		mTracks.push_back(tracks.mTracks[i]);
	}
}

int MyTracks::GetNumTracks() const{
	return mHeader.n_count;
}

int MyTracks::GetNumVertex(int trackIdx) const{
	return mTracks[trackIdx].mSize;
}

Point MyTracks::GetPoint(int trackIdx, int pointIdx) const{
	return mTracks[trackIdx].mPoints[pointIdx];
}

MyVec3f MyTracks::GetCoord(int trackIdx, int pointIdx) const{
	Point p = this->GetPoint(trackIdx, pointIdx);
	return MyVec3f(p.x, p.y, p.z);
}

void MyTracks::ComputeTubeGeometry(){
	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracks.size(); it++){
		totalPoints += mTracks[it].mSize;
	}
	totalPoints *= (mFaces + 1);

	cout << "Allocating Storage for Geometry...\r";

	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	mTexCoords.resize(totalPoints);
	mRadius.resize(totalPoints);
	mColors.resize(totalPoints);

	for (int it = 0; it < mTracks.size(); it++){
		if ((int)((it + 1) * 100 / (float)mTracks.size())
			- (int)(it * 100 / (float)mTracks.size()) >= 1){
			cout << "Computing: " << it*100.f / mTracks.size() << "%.          \r";
		}
		int npoints = mTracks[it].mSize;

		const float myPI = 3.1415926f;
		float dangle = 2 * myPI / mFaces;
		MyVec3f pole(0.6, 0.8, 0);

		MyArray3f candicates;
		candicates << MyVec3f(0, 0, 1)
			<< MyVec3f(0, 1, 1)
			<< MyVec3f(0, 1, 0)
			<< MyVec3f(1, 1, 0)
			<< MyVec3f(1, 0, 0)
			<< MyVec3f(1, 0, 1);
		float max = -1;
		int maxIdx;
		MyVec3f genDir = this->GetCoord(it, 0) - this->GetCoord(it, npoints - 1);
		genDir.normalize();
		for (int i = 0; i<candicates.size(); i++){
			float cp = (candicates[i].normalized() ^ genDir).norm();
			if (cp>max){
				max = cp;
				maxIdx = i;
			}
		}
		pole = candicates[maxIdx].normalized();

		for (int i = 0; i<npoints; i++){
			MyVec3f p = this->GetCoord(it, i);
			float size = 0.4;
			//float size = 0;
			MyVec3f d;
			if (i == npoints - 1){
				d = p - this->GetCoord(it, i - 1);
			}
			else if (i == 0){
				d = this->GetCoord(it, i + 1) - p;
			}
			else{
				d = this->GetCoord(it, i + 1) - this->GetCoord(it, i - 1);
			}

			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();
			//if ((perpend1^perpend2)*d < 0) dangle = -dangle;
			for (int is = 0; is<mFaces; is++){
				float angle = dangle*is;
				MyVec3f pt = sin(angle)*perpend1 + cos(angle)*perpend2;
				mVertices[currentIdx + i*(mFaces + 1) + is] = pt * 0.4 + p;
				mNormals[currentIdx + i*(mFaces + 1) + is] = pt;
				mTexCoords[currentIdx + i*(mFaces + 1) + is] = MyVec2f(i, is / (float)mFaces);
				mRadius[currentIdx + i*(mFaces + 1) + is] = size;
				//mColors[currentIdx + i*(mFaces + 1) + is] = mTracts->GetColor(it, i);
				mColors[currentIdx + i*(mFaces + 1) + is] = MyColor4f(0.5, 0.5, 0.5, 1);
			}
			mVertices[currentIdx + i*(mFaces + 1) + mFaces] = mVertices[currentIdx + i*(mFaces + 1)];
			mNormals[currentIdx + i*(mFaces + 1) + mFaces] = mNormals[currentIdx + i*(mFaces + 1)];
			mTexCoords[currentIdx + i*(mFaces + 1) + mFaces] = MyVec2f(i, 1);
			mRadius[currentIdx + i*(mFaces + 1) + mFaces] = mRadius[currentIdx + i*(mFaces + 1)];
			//mColors[currentIdx + i*(mFaces + 1) + mFaces] = mTracts->GetColor(it, i);
			mColors[currentIdx + i*(mFaces + 1) + mFaces] = MyColor4f(0.5, 0.5, 0.5, 1);
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints*(mFaces + 1);
	}
	// index
	/*
	mIndices.clear();
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 1; i<this->GetNumVertex(it); i++){
			for (int j = 0; j <= mFaces; j++){
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j % (mFaces + 1) + offset,
					(i)*(mFaces + 1) + j % (mFaces + 1) + offset,
					(i)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset);
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j % (mFaces + 1) + offset,
					(i)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset,
					(i - 1)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset);
			}
		}
	}
	*/
	mIndices.clear();
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 1; i<this->GetNumVertex(it); i++){
			for (int j = 0; j < mFaces; j++){
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + (j + 1) + offset);
				mIndices << MyVec3i((i - 1)*(mFaces + 1) + j + offset,
					(i)*(mFaces + 1) + (j + 1) + offset,
					(i - 1)*(mFaces + 1) + (j + 1) + offset);
			}
		}
	}
	cout << "Computing completed.\n";
}

void MyTracks::ComputeLineGeometry(){

	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracks.size(); it++){
		totalPoints += mTracks[it].mSize;
	}

	cout << "Allocating Storage for Geometry...\r";

	mVertices.clear();
	mNormals.clear();
	mVertices.reserve(totalPoints);
	mNormals.reserve(totalPoints);
	//mVertices.resize(totalPoints);
	//mNormals.resize(totalPoints);

	mIdxOffset.clear();
	mIdxOffset.reserve(mTracks.size());
	for (int it = 0; it < mTracks.size(); it++){
		if ((int)((it + 1) * 100 / (float)mTracks.size())
			- (int)(it * 100 / (float)mTracks.size()) >= 1){
			cout << "Computing: " << it*100.f / mTracks.size() << "%.          \r";
		}
		int npoints = mTracks[it].mSize;

		const float myPI = 3.1415926f;
		float dangle = 2 * myPI / mFaces;
		MyVec3f pole(0.6, 0.8, 0);

		MyArray3f candicates;
		candicates << MyVec3f(0, 0, 1)
			<< MyVec3f(0, 1, 1)
			<< MyVec3f(0, 1, 0)
			<< MyVec3f(1, 1, 0)
			<< MyVec3f(1, 0, 0)
			<< MyVec3f(1, 0, 1);
		float max = -1;
		int maxIdx;
		MyVec3f genDir = this->GetCoord(it, 0) - this->GetCoord(it, npoints - 1);
		genDir.normalize();
		for (int i = 0; i<candicates.size(); i++){
			float cp = (candicates[i].normalized() ^ genDir).norm();
			if (cp>max){
				max = cp;
				maxIdx = i;
			}
		}
		pole = candicates[maxIdx].normalized();

		for (int i = 0; i<npoints; i++){
			MyVec3f p = this->GetCoord(it, i);
			float size = 0.4;
			//float size = 0;
			MyVec3f d;
			if (i == npoints - 1){
				d = p - this->GetCoord(it, i - 1);
			}
			else if (i == 0){
				d = this->GetCoord(it, i + 1) - p;
			}
			else{
				d = this->GetCoord(it, i + 1) - this->GetCoord(it, i - 1);
			}

			MyVec3f perpend1 = (pole^d).normalized();
			MyVec3f perpend2 = (perpend1^d).normalized();

			//mVertices[currentIdx + i] = p;
			//mNormals[currentIdx + i] = perpend1;
			mVertices.push_back(p);
			mNormals.push_back(perpend1);
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints;
	}

	mLineIndices.clear();
	mLineIndices.reserve(totalPoints);
	// index
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 0; i<this->GetNumVertex(it); i++){
			mLineIndices << i + offset;
		}
	}
	cout << "Computing completed.\n";
}

void MyTracks::ComputeGeometry(){

	if (mShape == TRACK_SHAPE_TUBE){
		this->ComputeTubeGeometry();
	}
	else{
		this->ComputeLineGeometry();
	}
}

void MyTracks::LoadGeometry(){
	if (mShape == TRACK_SHAPE_LINE){
	//	return;
	}
	if (glIsVertexArray(mVertexArray)){
		glDeleteVertexArrays(1, &mVertexArray);
	}
	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);
	// vertex
	if (glIsBuffer(mVertexBuffer)){
		glDeleteBuffers(1, &mVertexBuffer);
	}
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(MyVec3f), &mVertices[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mPositionAttribute);
	glVertexAttribPointer(mPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(MyVec3f), &mNormals[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mNormalAttribute);
	glVertexAttribPointer(mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	/*
	// texCoord
	if (glIsBuffer(mTexCoordBuffer)){
		glDeleteBuffers(1, &mTexCoordBuffer);
	}
	glGenBuffers(1, &mTexCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTexCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, mTexCoords.size() * sizeof(MyVec2f), &mTexCoords[0][0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mTexCoordAttribute);
	glVertexAttribPointer(mTexCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	// radius
	if (glIsBuffer(mRadiusBuffer)){
		glDeleteBuffers(1, &mRadiusBuffer);
	}
	glGenBuffers(1, &mRadiusBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mRadiusBuffer);
	glBufferData(GL_ARRAY_BUFFER, mRadius.size() * sizeof(float), &mRadius[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mRadiusAttribute);
	glVertexAttribPointer(mRadiusAttribute, 1, GL_FLOAT, GL_FALSE, 0, 0);
	// color
	if (glIsBuffer(mColorBuffer)){
		glDeleteBuffers(1, &mColorBuffer);
	}
	glGenBuffers(1, &mColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, mColors.size() * sizeof(MyColor4f), &mColors[0].r, GL_STATIC_DRAW);
	glEnableVertexAttribArray(mColorAttribute);
	glVertexAttribPointer(mColorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
	*/
	// texture
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);
	// index
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	if (mShape == TRACK_SHAPE_TUBE && mIndices.size()>0){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_STATIC_DRAW);
	}
	else if (mShape == TRACK_SHAPE_LINE && mLineIndices.size()>0){
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLineIndices.size() * sizeof(int), &mLineIndices[0], GL_STATIC_DRAW);
	}
	else{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	}
	// unbind
	glBindVertexArray(0);

	// now free everything
	mVertices.clear();
	mNormals.clear();
	mTexCoords.clear();
	mRadius.clear();
	mColors.clear();
	mIndices.clear();
	mLineIndices.clear();
}

void MyTracks::LoadShader(){
	if (mShape == TRACK_SHAPE_LINE){
	//	return;
	}

	glDeleteProgram(mShaderProgram);
	mShaderProgram = InitShader("tracks.vert", "tracks.frag", "fragColour", "name");

	mNormalAttribute = glGetAttribLocation(mShaderProgram, "normal");
	if (mNormalAttribute < 0) {
		cerr << "Shader did not contain the 'normal' attribute." << endl;
	}
	mPositionAttribute = glGetAttribLocation(mShaderProgram, "position");
	if (mPositionAttribute < 0) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
}

void MyTracks::Show(){
	//if (mShape == TRACK_SHAPE_TUBE){
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_3D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindVertexArray(mVertexArray);
		glUseProgram(mShaderProgram);

		int mvmatLocation = glGetUniformLocation(mShaderProgram, "mvMat");
		float modelViewMat[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
		glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

		int projmatLocation = glGetUniformLocation(mShaderProgram, "projMat");
		float projMat[16];
		glGetFloatv(GL_PROJECTION_MATRIX, projMat);
		glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

		int colorLocation = glGetUniformLocation(mShaderProgram, "color");
		float color[3] = {0.5,0.5,0.5};
		glUniform3fv(colorLocation, GL_FALSE, color);

		int filterVolLocation = glGetUniformLocation(mShaderProgram, "filterVol");
		glUniform1i(filterVolLocation, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_3D, mFilterVolumeTexture);

		if (mShape == TRACK_SHAPE_TUBE){
			for (int i = 0; i < mFiberToDraw.size(); i++){
				int fiberIdx = mFiberToDraw[i];
				int offset = (mIdxOffset[fiberIdx]/(mFaces+1)-fiberIdx)*mFaces*6;
				int numVertex = (this->GetNumVertex(fiberIdx)-1)*(mFaces+0)*6;
				glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
			}
		}
		else{
			for (int i = 0; i < mFiberToDraw.size(); i++){
				int fiberIdx = mFiberToDraw[i];
				int offset = mIdxOffset[fiberIdx];
				int numVertex = this->GetNumVertex(fiberIdx);
				glDrawElements(GL_LINE_STRIP, numVertex, GL_UNSIGNED_INT, (const void *)(offset*sizeof(int)));
			}
		}

		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_3D);
	//}
	//else{
	//	int offset = 0;
	//	for (int i = 0; i < mFiberToDraw.size(); i++){
	//		int fiberIdx = mFiberToDraw[i];
	//		int numVertex = this->GetNumVertex(fiberIdx);
	//		glBegin(GL_LINE_STRIP);
	//		for (int j = 0; j < numVertex; j++){
	//			MyGraphicsTool::Normal(mNormals[mLineIndices[offset + j]]);
	//			MyGraphicsTool::Vertex(mVertices[mLineIndices[offset + j]]);
	//		}
	//		glEnd();
	//		offset += numVertex;
	//	}
	//}
}

void MyTracks::GetVoxelIndex(const MyVec3f vertex, long &x, long &y, long &z) const{
	x = 181-(long)vertex[0];
	y = 217-(long)vertex[1];
	z = 181-(long)vertex[2];
}

void MyTracks::MaskFiber(MyTracks* tracks, Array3D<float>* mask, int startIdx, int endIdx){
	for (int i = startIdx; i <= endIdx; i++){
		for (int j = 0; j < tracks->GetNumVertex(i); j++){
			long x, y, z;
			tracks->GetVoxelIndex(tracks->GetCoord(i, j), x, y, z);
			if (mask->operator()(x, y, z) > 0.5){
				tracks->mFiberDraw[i] = true;
				break;
			}
		}
	}
}

void MyTracks::FiberVolumeDensity(MyTracks* tracks,
	Array3D<atomic<int>>* density, const MyArrayi* indices, int startIdx, int endIdx){
	for (int i = startIdx; i <= endIdx; i++){
		int it = indices->at(i);
		long lastX = -1, lastY = -1, lastZ = -1;
		for (int j = 0; j < tracks->GetNumVertex(it); j++){
			long x, y, z;
			tracks->GetVoxelIndex(tracks->GetCoord(it, j), x, y, z);
			if (lastX != x || lastY != y || lastZ != z){
				density->operator()(x, y, z)++;
				lastX = x; lastY = y; lastZ = z;
			}
		}
	}
}

void MyTracks::FilterByVolumeMask(Array3D<float>& mask){
	if (mTracks.empty()) return;
	mFiberToDraw.clear();
	mFiberDraw = MyArrayb(mTracks.size(), false);
	// mask the fibers
	// serial edition
	/*
	for (int i = 0; i < this->GetNumTracks(); i++){
		for (int j = 0; j < this->GetNumVertex(i); j++){
			long x, y, z;
			this->GetVoxelIndex(this->GetCoord(i, j), x, y, z);
			if (mask(x, y, z) > 0.5){
				mFiberToDraw << i;
				break;
			}
		}
	}
	*/

	//MaskFiber(this, &mask, 0, mTracks.size() - 1);

	// multi-thread edition
	int numThread = std::thread::hardware_concurrency() - 1;
	numThread = min(numThread, (int)mTracks.size());
	std::thread *tt = new std::thread[numThread - 1];
	float fiberPerThread = mTracks.size() / (float)numThread;
	for (int i = 0; i < numThread-1; i++){
		int startIdx = fiberPerThread*i;
		int endIdx = fiberPerThread*(i+1)-1;
		tt[i] = std::thread(MaskFiber, this, &mask, startIdx, endIdx);
	}
	MaskFiber(this, &mask, fiberPerThread*(numThread - 1), mTracks.size() - 1);
	for (int i = 0; i < numThread - 1; i++){
		tt[i].join();
	}
	delete[] tt;
	for (int i = 0; i < mFiberDraw.size(); i++){
		if (mFiberDraw[i]){
			mFiberToDraw << i;
		}
	}

	//std::cout << "Filter: " << mFiberToDraw.size() << " fibers to be drawn.\n";
		/*
	// updating indices
	if (mShape == TRACK_SHAPE_TUBE){
		mIndices.clear();
		for (int itt = 0; itt<mFiberToDraw.size(); itt++){
			int it = mFiberToDraw[itt];
			int offset = mIdxOffset[it];
			for (int i = 1; i<this->GetNumVertex(it); i++){
				for (int j = 0; j <= mFaces; j++){
					mIndices << MyVec3i((i - 1)*(mFaces + 1) + j % (mFaces + 1) + offset,
						(i)*(mFaces + 1) + j % (mFaces + 1) + offset,
						(i)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset);
					mIndices << MyVec3i((i - 1)*(mFaces + 1) + j % (mFaces + 1) + offset,
						(i)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset,
						(i - 1)*(mFaces + 1) + (j + 1) % (mFaces + 1) + offset);
				}
			}
		}
		glBindVertexArray(mVertexArray);
		if (glIsBuffer(mIndexBuffer)){
			glDeleteBuffers(1, &mIndexBuffer);
		}
		glGenBuffers(1, &mIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		if (mIndices.size()>0){
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(MyVec3i), &mIndices[0][0], GL_DYNAMIC_DRAW);
		}
		else{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		}
		glBindVertexArray(0);
	}
	else{
		mLineIndices.clear();
		for (int itt = 0; itt<mFiberToDraw.size(); itt++){
			int it = mFiberToDraw[itt];
			int offset = mIdxOffset[it];
			for (int i = 0; i<this->GetNumVertex(it); i++){
				mLineIndices << offset + i;
			}
		}
		glBindVertexArray(mVertexArray);
		if (glIsBuffer(mIndexBuffer)){
			glDeleteBuffers(1, &mIndexBuffer);
		}
		glGenBuffers(1, &mIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		if (mLineIndices.size()>0){
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mLineIndices.size() * sizeof(int), &mLineIndices[0], GL_DYNAMIC_DRAW);
		}
		else{
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		}
		glBindVertexArray(0);
	}

		*/
}

void MyTracks::AddVolumeFilter(RicVolume& vol){
	if (glIsTexture(mFilterVolumeTexture)){
		glDeleteTextures(1, &mFilterVolumeTexture);
	}
	glGenTextures(1, &mFilterVolumeTexture);
	glBindTexture(GL_TEXTURE_3D, mFilterVolumeTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// map x,y,z to z,y,x
	float *d = new float[vol.nvox];
	for (int i = 0; i < vol.get_numx(); i++){
		for (int j = 0; j < vol.get_numy(); j++){
			for (int k = 0; k < vol.get_numz(); k++){
				//if (vol.vox[i][j][k] > 0.1) cout << vol.vox[i][j][k] << endl;
				d[k*vol.get_numx()*vol.get_numy() + j*vol.get_numx() + i]
					= vol.vox[i][j][k];
			}

		}
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, d);
	delete[]d;

	glBindTexture(GL_TEXTURE_3D, 0);
}

void MyTracks::ToDensityVolume(float* densityVol, int x, int y, int z){
	if (mFiberToDraw.size() == 0){
		memset(densityVol, 0, x*y*z*sizeof(float));
		return;
	}
	Array3D<atomic<int>> density;
	density.Construct(x, y, z);
	for (int i = 0; i < x; i++){
		for (int j = 0; j < y; j++){
			for (int k = 0; k < z; k++){
				density(i, j, k).store(0);
			}
		}
	}
	int numThread = std::thread::hardware_concurrency() - 1;
	numThread = min(numThread, (int)mFiberToDraw.size());
	std::thread *tt = new std::thread[numThread - 1];
	float fiberPerThread = mFiberToDraw.size() / (float)numThread;
	for (int i = 0; i < numThread - 1; i++){
		int startIdx = fiberPerThread*i;
		int endIdx = fiberPerThread*(i + 1) - 1;
		tt[i] = std::thread(FiberVolumeDensity, this, &density, &mFiberToDraw, startIdx, endIdx);
	}
	FiberVolumeDensity(this, &density, &mFiberToDraw, fiberPerThread*(numThread - 1), mFiberToDraw.size() - 1);
	for (int i = 0; i < numThread - 1; i++){
		tt[i].join();
	}
	delete[] tt;

	int idx = 0;
	float maxValue = 0;
	for (int i = 0; i < x; i++){
		for (int j = 0; j < y; j++){
			for (int k = 0; k < z; k++){
				densityVol[idx] = (float)density(i, j, k).load();
				maxValue = max(maxValue, densityVol[idx]);
				idx++;
			}
		}
	}
	if (maxValue > 0){
		for (int i = 0; i < x*y*z; i++){
			densityVol[i] /= maxValue;
		}
	}
}