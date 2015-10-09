#include "ColorScaleTable.h"

unsigned char ColorScaleTable::colorBrewer_sequential_8_singlehue_3[8][3] = {
	{ 0xff, 0xff, 0xff },
	{ 0xf0, 0xf0, 0xf0 },
	{ 0xd9, 0xd9, 0xd9 },
	{ 0xbd, 0xbd, 0xbd },
	{ 0x96, 0x96, 0x96 },
	{ 0x73, 0x73, 0x73 },
	{ 0x52, 0x52, 0x52 },
	{ 0x25, 0x25, 0x25 },
};

/*
unsigned char colorBrewer_sequential_8_multihue_9[8][3] = {
	{ 0xff, 0xff, 0xe5 },
	{ 0xf7, 0xfc, 0xb9 },
	{ 0xd9, 0xf0, 0xa3 },
	{ 0xad, 0xdd, 0x8e },
	{ 0x78, 0xc6, 0x79 },
	{ 0x41, 0xab, 0x5d },
	{ 0x23, 0x84, 0x43 },
	{ 0x00, 0x5a, 0x32 },
};
*/

// this is actually faked from sequential_9_multihue
unsigned char ColorScaleTable::colorBrewer_sequential_8_multihue_9[8][3] = {
//	{ 0xf7, 0xfc, 0xf5 },
	{ 0xe5, 0xf5, 0xe0 },
	{ 0xc7, 0xe9, 0xc0 },
	{ 0xa1, 0xd9, 0x9b },
	{ 0x74, 0xc4, 0x76 },
	{ 0x41, 0xab, 0x5d },
	{ 0x23, 0x8b, 0x45 },
	{ 0x00, 0x6d, 0x2c },
	{ 0x00, 0x44, 0x1b },
};

void ColorScaleTable::DiffValueToColor(float diff, float minDiff, float maxDiff, float color_rgba[4]){
	if (diff <= 0){
		color_rgba[0] = 1;
		color_rgba[1] = 1 - diff / minDiff;
		color_rgba[2] = 1 - diff / minDiff;
	}
	else{
		color_rgba[2] = 1;
		color_rgba[1] = 1 - diff / maxDiff;
		color_rgba[0] = 1 - diff / maxDiff;
	}
	color_rgba[3] = 1;
}

