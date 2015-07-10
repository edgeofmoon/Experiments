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

	void LoadLabelVolume(char* fileName);
	void LoadLabelTable(char* fileName);
	void SetNodeXPositionsExt();
	void DrawArcHistogram(long arc);
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
	void MarkSelectedArcVoxes(long arc, float isoValue);GLuint colorTex;
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
	float mScaleWidth;
	float mBinWidth;
	float mSigma;
	void updateArcHistogram(long arc);
	float countToHeight(int count, char compareByte = 0) const;
	float getArcWidth(long arc);
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
	float GetScaleWidth() const{ return mScaleWidth; };
	void SetScaleWidth(float w) { mScaleWidth = w; };
	float GetBinWidth() const { return mBinWidth; };
	float UpdateSubTreeLayout(long rootNode, long parentNode, float xStart, float xEnd);
	void DrawLegend(MyVec2f lowPos, MyVec2f highPos);

// comparison component
// part of layout component
protected:
	std::map<long, char> mArcCompared;
	float mComparedScaleWidth;
	int CountSameElementSorted(vector<long>& a1, vector<long>& a2);
	bool IsArcCompared(long arc) const;
	void AddComparedArc(long arc);
	void RemoveComparedArc(long arc);
	void FindSimilarValidArcs(MyContourTree* ct, long arc, vector<long>& arcs);
	void GetArcVoxesIndices(long arc, vector<long>& idx);
	float GetDrawingHeight(float count, char compareByte) const;
public:
	void SetComparedArcWidthScale(float w) { mComparedScaleWidth = w; };
	float GetComparedArcScaleWidth() const{ return mComparedScaleWidth; };
	void ClickComparedArc(long arc);
	float MaxComparedArcWidth() const;
	float SuggestComparedArcWidthScale() const;
	int CompareArcs(MyContourTree* ct);
	void ClearComparedArcs() { mArcCompared.clear(); };

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
};

