#ifndef MYTRACK_H
#define MYTRACK_H

#include <string>
#include <vector>

#include "RicPoint.h"

class MySingleTrackData
{
public:
	int mSize;
	std::vector<Point> mPoints;
	std::vector<std::vector<float> > mPointScalars;
	std::vector<float> mTrackProperties;
};

class MyTracks
{
public:
	MyTracks();

	MyTracks(const std::string& filename);
	
	int Read(const std::string& filename);
	
	int Save(const std::string& filename) const;

	MyTracks Subset(const std::vector<int>& trackIndices) const;
	void AddTracks(const MyTracks& tracks);

	int GetNumTracks() const;
	int GetNumVertex(int trackIdx) const;
	
	Point GetPoint(int trackIdx, int pointIdx) const;

protected:
	struct MyTrackHeader
	{
		char id_string[6];
		short dim[3];
		float voxel_size[3];
		float origin[3];
		short n_scalars;
		char scalar_name[10][20];
		short n_properties; 
		char property_name[10][20];
		float vox_to_ras[4][4];
		char reserved[444];
		char voxel_order[4];
		char pad2[4];
		float image_orientation_patient[6];
		char pad1[2];
		unsigned char invert_x;
		unsigned char invert_y;
		unsigned char invert_z;
		unsigned char swap_xy;
		unsigned char swap_yz;
		unsigned char swap_zx;
		int n_count;
		int version;
		int hdr_size; 
	} mHeader;
	
	std::vector<MySingleTrackData> mTracks;
};
	

#endif
