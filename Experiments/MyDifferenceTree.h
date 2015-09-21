#pragma once

#include "MyContourTree.h"
#include <vector>
#include <map>

#include "MyBox.h"

class MyDifferenceTree
{
public:
	MyDifferenceTree();
	~MyDifferenceTree();

	void SetContourTrees(MyContourTree* ct0, MyContourTree* ct1);
	void UpdateArcMapping();
	void UpdateDifferenceHistogram();
	void UpdateLabels(int width, int height);
	void Show();
	void ShowLegend();
	void SetLabelDrawRatio(float ratio) {
		mLabelDrawRatio = ratio;
	};

protected:
	MyContourTree* mCt0, *mCt1;
	std::map<long, long> mArcMap;
	std::map<long, bool> mPathArcs;
	std::map<long, SimpleDistribution> mArcDiffHistogram;
	std::map<long, MyBox2f> mLabelPos;
	std::vector<long> mArcLabelSorted;
	float mLabelDrawRatio;
	float mMinDiff, mMaxDiff;
	float mMaxHistogramCount;

	MyBox2f GetArcBox(long arc) const;
	void UpdatePathArcs();
	void DiffValueToColor(float diff, float color_rgba[4]);
	bool IsArcMapped(long arc) const;
	void DrawDiffHistogram(long arc);
	void DrawDiffHistogram(float xPos, float yPos, SimpleDistribution& distr,
		MyContourTree::MappingScale scale, MyContourTree::HistogramSide side, float alpha = 1);
	void MakeDiffHistogram(MyContourTree* ct0, MyContourTree* ct1, 
		const vector<long> voxes, SimpleDistribution& distr);

	static MyContourTree* mTemplateTree;
	static bool compareArcMore(long arc0, long arc1);
};

