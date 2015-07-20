#pragma once

#include "HeightField.h"
#include "MyFrameBuffer.h"
#include "MyVec.h"
#include <list>
#include <vector>
#include <map>

class RicVolume;

class SuperNodeExt{
public:
	long parent;
	long numLeaves;
	float subTreeLayoutWidth;
	float *minHeight;
	float *maxHeight;

	SuperNodeExt(){
		parent = -1;
		numLeaves = -1;
		maxHeight = 0;
		minHeight = 0;
		subTreeLayoutWidth = -1;
	}
};

class MyBitmap;
class MyContourTree
	:public HeightField
{
public:
	MyContourTree(int argc, char** argv);
	void CombineTrees();
	~MyContourTree();

public:
	enum MappingScale{
		MappingScale_Linear = 1,
		MappingScale_Sci = 2,
		MappingScale_Log = 3,
	};

	enum HistogramSide{
		HistogramSide_Sym = 0,
		HistogramSide_Left = 1,
		HistogramSide_Right = 2,
	};

protected:
	MappingScale mDefaultScale;
	MappingScale mAltScale;
	HistogramSide mHistogramSide;
	float mContourTreeAlpha;
public:
	void SetContourTreeAlpha(float alpha){ mContourTreeAlpha = alpha; };
	float GetContourTreeAlpha() const { return mContourTreeAlpha; };
	void SetDefaultMappingScale(MappingScale scale){ mDefaultScale = scale; };
	void SetAltMappingScale(MappingScale scale){ mAltScale = scale; };
	void SetHistogramSide(HistogramSide side){ mHistogramSide = side; };
	Array3D<float>& GetVolume() { return height; };
	void LoadLabelVolume(char* fileName);
	void LoadLabelTable(char* fileName);
	void SetNodeXPositionsExt(MappingScale scale);
	void GetArcBottomPos(long arc, float& x, float& y);
	void DrawArcHistogramAt(long arc, float x, float y, float scaleX = 1, float scaleY = 1);
	void DrawArcSnappingPoint(long arc);
	void DrawArcHistogram(long arc);
	void DrawArcHistogramScientific(long arc);
	void DrawPlanarContourTree();
	void DrawSelectedVoxes(bool useDisplayLists, bool pickColours);
	void DrawSelectedArcVoxes(long arc, bool useDisplayLists, bool pickColours);
	void DrawSelectedArcVoxes(long arc, float isoValue);
	void PruneNoneROIs();
	void UpdateArcNodes();
	void SingleCollapse(long whichArc);
	long CollapseVertex(long whichSupernode);
	std::string ComputeArcName(long arc);
	void ComputeArcNames();
	long GetArcRoiCount(long arc);
	int PickArc(float x, float y);
	void SetPruningThreshold(int thres){ mPruningThreshold = thres; };

protected:
	int mPruningThreshold;
	SuperNodeExt* supernodesExt;
	RicVolume* mLabelVolume;
	std::map<int, std::string> mLabelName;
	std::map<long, vector<float*>> mArcNodes;
	std::map<long, std::string> mArcName;

	bool IsNameLeft(std::string name) const;
	bool IsNameRight(std::string name) const;
	//std::vector<long> FindPathDown2(long highNode, long lowNode, std::vector<long> currentPath = std::vector<long>(0));
	bool FindPathDown(long highNode, long lowNode, std::vector<long>& path);
	bool FindPath(long sourceNode, long targetNode, std::vector<long>& path, long parent = -1);
	float* FindHighestPath(long rootNode, long parentNode, std::vector<long>& path, float *baseHeight);
	// return leave height
	float* FindHighestUpPath(long rootNode, std::vector<long>& path);
	// return leave height
	float* FindHighestDownPath(long rootNode, std::vector<long>& path);
	void LayoutSubTree(long rootNode, long parentNode, float xStart, float xEnd);
	void LayoutSubTree2(long rootNode, long parentNode, float xStart, float xEnd);
	long GetNumLeaves(long rootNode, long parentNode);
	// contour tree is a binary tree, three neighbors at most
	// return the other neighbor
	long GetBrunchNode(long rootNode, long parentNode, long childNode);
	void GetNeighbors(long rootNode, std::list<long>& neighbors);
	void GetNeighbors(long rootNode, std::vector<long>& neighbors);

	long FindMonotoneDownNode(long rootNode);
	bool FindMonotoneDownPath(long sourceNode, long targetNode, std::vector<long>& path);
	long FindMonotoneUpNode(long rootNode);
	bool FindMonotoneUpPath(long sourceNode, long targetNode, std::vector<long>& path);

	void FindHeightRange(long rootNode, long parentNode, float*&minHeight, float*&maxHeight);
	int FindHeightRangeAndLeaves(long rootNode, long parentNode);

	int FillUpperSlots(vector<float>& upperBottomFilled, float minHeight, float maxHeight, int numLeaves);
	int FillDownSlots(vector<float>& dowmTopFilled, float minHeight, float maxHeight, int numLeaves);

	// rendering component
protected:
	Array3D<float> mMaskVolume;
	GLuint cubeVertexArray, cubeVertexBuffer, cubeIndexBuffer, cubeProgram;
	void loadCubeShaderData();
	frameBuffer cubeFrameBuffer;
	void setupCubeFrameBuffer();
	GLuint volTex;
	void loadVolumeTexture();
	GLuint markTex;
	void loadMarkVolumeTexture();
	int shaderProgram;
	void CompileShaders();
	void MarkSelectedArcVoxes(long arc, float isoValue); GLuint colorTex;
	MyBitmap* colorMap;
public:
	void MarkSelectedVoxes();
	void ReCompileShaders();
	void VolumeRenderingSelectedVoxes(int width, int height);
	void SetupVolomeRenderingBuffers(int width, int height);
	void SetColorTexture(GLuint tex){ colorTex = tex; };
	void SetColorMap(MyBitmap* bitmap){ colorMap = bitmap; };

	// layout component
protected:
	std::map<long, std::vector<float>> mArcHistogram;
	float mLogWidthScale;
	float mLinearWidthScale;
	float mBinWidth;
	float mSigma;
	float mMaxHistogramCount;
	float mZoomLevel;
	void updateArcHistogram(long arc);
	MappingScale GetArcScale(long arc) const;
	float GetArcZoomLevel(long arc) const;
	float getArcWidth(long arc) const;
	float GetArcWidth(long arc, MappingScale scale) const;
	void getSubArcs(long rootNode, long parentArc, std::vector<long>& subArcs);
	float getSubTreeWidth(long rootNode, long parentNode);
	float subTreeLayoutWidth(long rootNode, long parentNode);
	bool isBrunchLeft(long brunchNode, long rootNode);
	long nodes2Arc(long node1, long node2);
	float getPathWidth(vector<long>& pathNodes, long parentNode);
	float fillUpper(vector<MyVec2f>& upperFilled, float bottom, float top, float width);
	float fillBottom(vector<MyVec2f>& bottomFilled, float bottom, float top, float width);
	void updateArcHistograms();
public:
	float GetZoomLevel() const { return mZoomLevel; };
	void SetZoomLevel(float zoom){ mZoomLevel = zoom; };
	float GetLogScaleWidth() const{ return mLogWidthScale; };
	void SetLogScaleWidth(float w) { mLogWidthScale = w; };
	float GetLinearScaleWidth() const{ return mLinearWidthScale; };
	void SetLinearScaleWidth(float w) { mLinearWidthScale = w; };
	float GetBinWidth() const { return mBinWidth; };
	float UpdateSubTreeLayout(long rootNode, long parentNode, float xStart, float xEnd);
	void DrawLegend(MyVec2f lowPos, MyVec2f highPos);
	float SuggestAltMappingWidthScaleModifier() const;
	float GetAltMappingWidthScale() const;
	void SetAltMappingScale(float scale);

// comparison component
// part of layout component
protected:
	enum ArcStatus{
		ArcStatus_Default = 0x00,
		ArcStatus_InComaprison = 0x01,
		ArcStatus_InSnapping = 0x02,
		ArcStatus_SnapAnchoring = 0x04,
		ArcStatus_SnapAnchored = 0x08,
	};
	std::map<long, char> mArcStatus;
	void ClearAllArcStatus(ArcStatus status);
	int CountSameElementSorted(vector<long>& a1, vector<long>& a2);
	bool IsArcCompared(long arc) const;
	void AddComparedArc(long arc);
	void RemoveComparedArc(long arc);
	void FindSimilarValidArcs(MyContourTree* ct, long arc, vector<long>& arcs);
	void GetArcVoxesIndices(long arc, vector<long>& idx);
	float GetDrawingHeight(float count, MappingScale scale) const;
public:
	void ClickComparedArc(long arc);
	float MaxComparedArcWidth() const;
	int CompareArcs(MyContourTree* ct);
	void ClearComparedArcs();

//scientific notation histogram
protected:
	int mMinExponent, mMaxExponent;
	float mMinMantissa, mMaxMantissa;
	float mScientificWidthScale;
	// this linear mapping maps exponent to the mantissa range
	float mExponent_Scale;
	float mExponent_Offset;
	float mCountClamp;
	void floatToScientific(float num, int& exp, float&manti) const;
	void updateScientificHistograms();
	void GetDrawingHeightScientific(float count, float& expHeight, float&mantissaHeight) const;
public:
	int GetMinExponent() const{ return mMinExponent; };
	int GetMaxExponent() const{ return mMaxExponent; };
	float GetMinMantissa() const{ return mMinMantissa; };
	float GetMaxMantissa() const{ return mMaxMantissa; };
	float GetExponentScaleWidth() const{ return mExponent_Scale; };
	float GetExponnetScaleOffset() const{ return mExponent_Offset; };
	float GetScientificWidthScale() const { return mScientificWidthScale; };
	void SetScientificWidthScale(float w) { mScientificWidthScale = w; };

// prune
protected:
	Superarc* mSuperArcsBkup;
	Supernode* mSuperNodesBkup;
	long *mValidNodes, *mValidArcs;
	long mNumValidNodes, mNumValidArcs;
	long mNumSuperArcsBkup, mNumSuperNodesBkup;
	long mNextSuperarcBkup, mSavedNextSuperarcBkup;
	long mNextSupernodeBkup;
	void BackupTree();
	void RestoreTree();

// for snapping
public:
	enum ArcSnapPosition{
		ArcSnapPosition_Bottom_Y = 0,
		ArcSnapPosition_Top_Y = 1,
		ArcSnapPosition_Center_Y = 2,
	};

protected:
	ArcSnapPosition mSnapPosition;
public:
	void SetSnapPosition(ArcSnapPosition pos){ mSnapPosition = pos; };
	void ClickSnapArc(long arc);
	void ClearSnapArcs();
	void AddSnapArc(long arc);
	void GetArcSnapPosition(long arc, float& xpos, float& ypos) const;
	bool IsArcSnapped(long arc) const;
	int SyncSnapArcsTo(MyContourTree* ct);
	void UpdateSnapArcStatus(MyContourTree* ct, float offsetX, float offsetY, float range = 0.02);
	long GetAnchoringArc() const;
	bool ShouldWeSnap(MyContourTree* ct, float& offsetX, float& offsetY) const;

// for renaming
public:
	static void RenameLeaveArcsBySimilarity(MyContourTree* ct0, MyContourTree* ct1);
};

