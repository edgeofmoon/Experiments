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

	mTracks.resize(mHeader.n_count);


	for(int i = 0;i<mHeader.n_count; i++){
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
	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	mTexCoords.resize(totalPoints);
	mRadius.resize(totalPoints);
	mColors.resize(totalPoints);

	for (int it = 0; it < mTracks.size(); it++){
		int npoints = mTracks[it].mSize;

		const float PI = 3.1415926f;
		float dangle = 2 * PI / mFaces;
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
}

void MyTracks::ComputeLineGeometry(){

	int currentIdx = 0;
	mIdxOffset.clear();
	int totalPoints = 0;
	for (int it = 0; it < mTracks.size(); it++){
		totalPoints += mTracks[it].mSize;
	}
	mVertices.resize(totalPoints);
	mNormals.resize(totalPoints);
	//mTexCoords.resize(totalPoints);
	//mRadius.resize(totalPoints);
	//mColors.resize(totalPoints);

	for (int it = 0; it < mTracks.size(); it++){
		int npoints = mTracks[it].mSize;

		const float PI = 3.1415926f;
		float dangle = 2 * PI / mFaces;
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

			mVertices[currentIdx + i] = p;
			mNormals[currentIdx + i] = perpend1;
			//mTexCoords[currentIdx + i] = MyVec2f(i, 1);
			//mRadius[currentIdx + i] = 0.4;
			//mColors[currentIdx + i] = mTracts->GetColor(it, i);
			//mColors[currentIdx + i] = MyColor4f(0.5, 0.5, 0.5, 1);
		}

		mIdxOffset << currentIdx;
		currentIdx += npoints;
	}

	mLineIndices.clear();
	// index
	for (int it = 0; it<this->GetNumTracks(); it++){
		int offset = mIdxOffset[it];
		for (int i = 0; i<this->GetNumVertex(it); i++){
			mLineIndices << i + offset;
		}
	}
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
	mShaderProgram = InitShader("contour.vert", "contour.frag", "fragColour", "name");

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

		int boxLowLocation1 = glGetUniformLocation(mShaderProgram, "boxLow1");
		int boxHighLocation1 = glGetUniformLocation(mShaderProgram, "boxHigh1");
		//MyVec3f lowPos1 = box[0].GetLowPos() + mTracts->GetBoundingBox().GetCenter();
		//MyVec3f highPos1 = box[0].GetHighPos() + mTracts->GetBoundingBox().GetCenter();
		MyVec3f lowPos1(0, 0, 0);
		MyVec3f highPos1(0, 0, 0);
		glUniform3f(boxLowLocation1, lowPos1[0], lowPos1[1], lowPos1[2]);
		glUniform3f(boxHighLocation1, highPos1[0], highPos1[1], highPos1[2]);

		int boxLowLocation2 = glGetUniformLocation(mShaderProgram, "boxLow2");
		int boxHighLocation2 = glGetUniformLocation(mShaderProgram, "boxHigh2");
		//MyVec3f lowPos2 = box[1].GetLowPos() + mTracts->GetBoundingBox().GetCenter();
		//MyVec3f highPos2 = box[1].GetHighPos() + mTracts->GetBoundingBox().GetCenter();
		MyVec3f lowPos2(0, 0, 0);
		MyVec3f highPos2(0, 0, 0);
		glUniform3f(boxLowLocation2, lowPos2[0], lowPos2[1], lowPos2[2]);
		glUniform3f(boxHighLocation2, highPos2[0], highPos2[1], highPos2[2]);

		int screenSpaceLocation = glGetUniformLocation(mShaderProgram, "screenSpace");
		//int screenSpace = mbScreenSpace ? 1 : 0;
		int screenSpace = 0;
		glUniform1i(screenSpaceLocation, screenSpace);

		if (mShape == TRACK_SHAPE_TUBE){
			//glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);
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

		glDisable(GL_BLEND);
		glUseProgram(0);
		glBindVertexArray(0);
		glDisable(GL_CULL_FACE);
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

void MyTracks::FilterByVolumeMask(Array3D<float>& mask){
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
	int numThread = std::thread::hardware_concurrency()-1;
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

	std::cout << "Filter: " << mFiberToDraw.size() << " fibers to be drawn.\n";
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