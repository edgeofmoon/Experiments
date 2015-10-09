#pragma once

namespace ColorScaleTable{
	extern unsigned char colorBrewer_sequential_8_singlehue_3[8][3];

	extern unsigned char colorBrewer_sequential_8_multihue_9[8][3];

	void DiffValueToColor(float diff, float minDiff, float maxDiff, float color_rgba[4]);

};