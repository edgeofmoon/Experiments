#pragma once
#include "HeightField.h"
#include "MyFrameBuffer.h"
#include "MyVec.h"
#include "MyBox.h"
#include <list>
#include <vector>
#include <map>

class RicVolume;
class MyDifferenceTree;

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

class SimpleDistribution{
public:
	float mMin, mMax;
	std::vector<float> mKernelDensity;
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

public:
	float GetValue(long x, long y, long z) { return height(x, y, z); };
	float GetDimX() { return height.XDim(); };
	float GetDimY() { return height.YDim(); };
	float GetDimZ() { return height.ZDim(); };

protected:
	MappingScale mDefaultScale;
	MappingScale mAltScale;
	HistogramSide mHistogramSide;
	float mContourTreeAlpha;
	std::string mName;
public:
	void SetContourTreeAlpha(float alpha){ mContourTreeAlpha = alpha; };
	float GetContourTreeAlpha() const { return mContourTreeAlpha; };
	void SetDefaultMappingScale(MappingScale scale){ mDefaultScale = scale; };
	void SetAltMappingScale(MappingScale scale){ mAltScale = scale; };
	void SetHistogramSide(HistogramSide side){ mHistogramSide = side; };
	void SetName(const std::string& name){ mName = name; };
	std::string GetName() const{ return mName; };
	Array3D<float>& GetVolume() { return height; };
	void LoadLabelVolume(char* fileName);
	void LoadLabelTable(char* fileName);
	void SetNodeXPositionsExt(MappingScale scale);
	void GetArcBottomPos(long arc, float& x, float& y);
	void DrawArcHistogramAt(long arc, float x, float y, float scaleX = 1, float scaleY = 1);
	void DrawArcSnappingPoint(long arc);
	void DrawArcHistogram(long arc);
	void DrawArcHistogramScientific(long arc);
	void DrawArcBackLight(long arc);
	void DrawContourTreeFrame();
	void DrawArcLabels();
	void DrawArcLabelHighlight(long arc);
	void DrawArcLabelsUnoccluded();
	void DrawPlanarContourTree();
	void DrawSelectedVoxes(bool useDisplayLists, bool pickColours);
	void DrawSelectedArcVoxes(long arc, bool useDisplayLists, bool pickColours);
	void DrawSelectedArcVoxes(long arc, float isoValue);
	void PruneNoneROIs();
	void UpdateArcNodes();
	void UpdateLabels(int width, int height);
	void SingleCollapse(long whichArc);
	long CollapseVertex(long whichSupernode);
	std::string ComputeArcName(long arc);
	void ComputeArcNames();
	long GetArcRoiCount(long arc);
	int PickArc(float x, float y, bool printInfo = true);
	int PickArcFromLabel(float x, float y);

	void SetPruningThreshold(int thres){ mPruningThreshold = thres; };
	void SetLabelDrawRatio(float ratio) {
		mLabelDrawRatio = ratio;
	};

	enum LabelStyleBit{
		LabelStyle_BOARDER = 1,
		LabelStyle_SOLID = 2
	};
	void SetLabelStyle(unsigned char style){ mLabelStyle = style; };

protected:
	int mPruningThreshold;
	SuperNodeExt* supernodesExt;
	RicVolume* mLabelVolume;
	std::map<int, std::string> mLabelName;
	std::map<long, vector<float*>> mArcNodes;
	std::map<long, std::string> mArcName;
	// for label layout
	std::map<long, MyBox2f> mLabelPos;
	std::vector<long> mArcLabelSorted;
	float mLabelDrawRatio;

	unsigned char mLabelStyle;

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
	Array3D<float>& GetMaskVolume(){ return mMaskVolume; };

	void ShowSelectedVoxes();

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
	MyBox2f GetArcBox(long arc) const;
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
	void DrawLegendSimple(MyVec2f lowPos, MyVec2f highPos);
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

// for contour drawing
protected:
	int mCurrentArcName;
	int mIndex;
	float mContourColour[3];
	std::vector<float> mNormals;
	std::vector<float> mVertices;
	std::vector<int> mNames;
	int mNumVertices;

	void RenderContour(long arc);
	void FollowHierarchicalPathSeedUnagregated(int sArc, float ht);
	void FollowHierarchicalPathSeed(int sArc, float ht);
	void FollowContour(float ht, Superarc &theArc);
	void FollowSurface(float ht, float *p1, float *p2);
	void IntersectSurface(float ht, long x, long y, long z, int theEntryFaceEdge);
	void RenderTriangle(float ht, long x, long y, long z, float *cubeVert, int edge1, int edge2, int edge3);
	void InterpolateVertex(long x, long y, long z, int edge, float *cubeVert, float ht);

	void LoadContourGeometry();

	// shader data
	void DestoryContourDrawingBuffer();
	unsigned int mVertexArray;
	unsigned int mVertexBuffer;
	unsigned int mNormalBuffer;
	unsigned int mNameBuffer;
	unsigned int mIndexBuffer;
	unsigned int mPositionAttribute;
	unsigned int mNormalAttribute;
	unsigned int mNameAttribute;
	unsigned int mContourShaderProgram;

	unsigned int mFrameBuffer;
	unsigned int mColorTexture;
	unsigned int mDepthTexture;
	unsigned int mNameTexture;

	unsigned int mDiffColorTexture;
public:
	void SetContourColour(float color[3]);
	void SetIndex(int idx);
	int GetIndex() const;
	void CompileContourShader();
	void BuildContourGeomeryBuffer();
	void LoadDiffColorTexture(Array3D<float>& diffs, float minDiff, float maxDiff);
	void RemoveDiffColorTexture();
	void FlexibleContours();
	void UpdateContours();
	void RenderContours();

// for mouse hover quick draw
protected:
	class ContourGeometryDataBuffer{
	public:
		ContourGeometryDataBuffer(MyContourTree* ct);
		~ContourGeometryDataBuffer();
		void LoadGeometry(MyContourTree* ct);
		unsigned int vertexArray;
		unsigned int vertexBuffer;
		unsigned int normalBuffer;
		unsigned int nameBuffer;
		unsigned int indexBuffer;
		unsigned int numVertices;
	};
	//std::map<long, ContourGeometryDataBuffer*> mArcContourGeometry;
	//void ClearArcContourGeometry();
	//void BuildGeometryDataBuffer();
	long mContourHoveredArc;

public:
	void SetContourHoveredArc(long arc){ mContourHoveredArc = arc; };
	void DrawArcContour(long arc);
	void RenderAllUpperLeaveContours(float voxelRatio = 0.9);

// for difference tree
public:
	enum ArcCombineMode{
		ArcCombineMode_Intersection = 1,
		ArcCombineMode_Union = 2,
		ArcCombineMode_Complement = 3,
	};
protected:

	class DiffHistogram{
	public:
		SimpleDistribution mLeft, mRight;
	};
	ArcCombineMode mArcCombineMode;
	Array3D<float> mVoxSignificance;
	std::vector<long> mSigArcs;
	std::map<long, bool> mPathArcs;
	float mSigArcThreshold_P;
	float mSigArcThreshold_VolRatio;
	MyContourTree* mContrastContourTree;
	// map this arc to that arc
	// -1 means no corresponding arc mapped
	std::map<long, long> mArcMap;
	std::map<long, DiffHistogram> mArcDiffHistogram;
	bool mDiffMode;
	void UpdateArcMapping();
	void UpdatePathArcs();
	void UpdateDiffHistogram();
	void DrawArcDiffHistogram(long arc);
	void DrawSimpleArc(long arc);
	long GetContrastArc(long arc) const;
	// input arrays shall be sorted
	void CombineIndicesSorted(const vector<long>& thisIndices, const vector<long>& thatIndices,
		vector<long>& combinedIndices);
	void ComputeDistribution(Array3D<float>& vol, const vector<long>& indices, 
		SimpleDistribution& distribution, float minValue, float maxValue);
	void DrawDistribution(float xPos, const SimpleDistribution& distribution, 
		HistogramSide side, MappingScale scale);
	bool IsArcSig(long arc) const;
	bool IsDiffMode() const { return mDiffMode;};
	float mNonSigArcWidth;
	float GetHistogramWidth(const SimpleDistribution& hist, MappingScale scale, float arcZoom = 1) const;
public:
	void SetSigArcThreshold_P(float p){ mSigArcThreshold_P = p; };
	void SetSigArcThreshold_VolRatio(float vr){ mSigArcThreshold_VolRatio = vr; };
	void SetArcCombineMode(ArcCombineMode mode){ mArcCombineMode = mode; };
	void LoadVoxSignificance(RicVolume* vol);
	void UpdateSigArcList();
	void RenderSigDiffTree();
	void SyncSigArcsTo(MyContourTree* ct);

// for aggregation
protected:
	enum ArcAggregation{
		ArcAggregation_NONE = 1,
		ArcAggregation_HIDDEN = 2,
		ArcAggregation_AGGRE = 3,
	};
	std::map<long, ArcAggregation> mArcAggregation;
	std::map<long, Superarc> mArcAggregateRestore;
	std::map<long, Supernode> mNodeAggregateRestore;
	std::map<long, std::vector<float*>> mArcAggregateHistogramRestore;

	Superarc GetUnaggregatedArc(long arc) const;
	Supernode GetUnaggregatedNode(long node) const;

public:
	bool IsArcAggregated(long arc) const;
	void AggregateArcFromAbove(long arc);
	void RestoreAggregatedArc(long arc);
	void RestoreAllAggregated();
	void AggregateAllBranches();

	friend class MyDifferenceTree;

// statif functions
	static MyContourTree* TemplateTree;
	static bool compareArcMore(long arc0, long arc1);
};

int compareAbsHeight(const float *d1, const float *d2, const float *base);

bool compareHeightLogic(const float *d1, const float *d2);