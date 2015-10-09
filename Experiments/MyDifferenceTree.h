#pragma once

#include "MyContourTree.h"
#include <vector>
#include <map>

#include "MyBox.h"
#include "MyDiffHistogramPixel.h"

class MyDifferenceTree
{
public:
	MyDifferenceTree();
	~MyDifferenceTree();

	void SetContourTrees(MyContourTree* ct0, MyContourTree* ct1);
	void UpdateArcMapping();
	void UpdateDifferenceHistogram();
	void UpdateDifferenceHistogramPixel();
	void UpdateLayout();
	void UpdateLabels(int width, int height);
	void Show();
	void ShowLegend();
	void DrawArcLabelHighlight(long arc);
	int PickArc(float x, float y, bool printInfo = false);
	int PickArcFromLabel(float x, float y);
	void SetLabelDrawRatio(float ratio) {
		mLabelDrawRatio = ratio;
	};
	void SetUseOutsidePlacement(bool outp){ mUseOutsidePlacement = outp; };
	float GetMinDiff() const{ return mMinDiff; };
	float GetMaxDiff() const{ return mMaxDiff; };
	void SetLesserHistogramAlpha(float alpha){ mLesserHistogramAlpha = alpha; };

protected:
	MyContourTree* mCt0, *mCt1;
	std::map<long, long> mArcMap;
	std::map<long, bool> mPathArcs;
	std::map<long, SimpleDistribution> mArcDiffHistogram;
	std::map<long, MyBox2f> mLabelPos;
	std::vector<long> mArcLabelSorted;
	std::map<long, MyBox2f> mArcPosBox;
	float mLabelDrawRatio;
	float mMinDiff, mMaxDiff;
	float mMaxHistogramCount;
	float mLesserHistogramAlpha;
	bool mUseOutsidePlacement;

	MyBox2f GetArcRawBox(long arc) const;
	MyVec2f GetArcBasePos(long arc) const;
	MyVec2f GetArcRawBasePos(long arc) const;
	void UpdatePathArcs();
	void DiffValueToColor(float diff, float color_rgba[4]);
	bool IsArcMapped(long arc) const;
	void DrawDiffHistogram(long arc);
	void DrawDiffHistogram(float xPos, float yPos, SimpleDistribution& distr,
		MyContourTree::MappingScale scale, MyContourTree::HistogramSide side, float alpha = 1);
	void MakeDiffHistogram(MyContourTree* ct0, MyContourTree* ct1, 
		const vector<long>& voxes, SimpleDistribution& distr);

// diff histogram pixel
	MyBox2f GetArcRawBoxPixel(long arc) const;
	std::map<long, MyDiffHistogramPixel> mArcDiffHistogramPixel;
	void MakeDiffHistogramPixel(MyContourTree* ct0, MyContourTree* ct1,
		const vector<long>& voxes, long arc);
	float CountHistogramPixels(MyContourTree* ct0, long arc);
	void DrawDiffHistogramPixel(long arc);
	void DrawDiffHistogramPixel(float xPos, float yPos, float height, const std::vector<float>& tempDistr,
		const MyDiffHistogramPixel& diffDistr, MyContourTree::MappingScale scale, MyContourTree::HistogramSide side, float alpha = 1);
};
