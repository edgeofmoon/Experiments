/*
 * MyTracks.cpp
 *
 *  Created on: Dec 12, 2014
 *      Author: GuohaoZhang
 */

#include "MyTracks.h"

#include "RicPoint.h"

#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;


MyTracks::MyTracks(){

}

MyTracks::MyTracks(const string& filename){
	Read(filename);
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
