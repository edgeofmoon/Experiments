#ifndef CT_MSC_H
#define CT_MSC_H

#include <list>
#include <set>
#include <vector>
#include <queue>
using namespace std;
#define MaxFloat 100000000.0f
#define MinFloat -10000000.0f

struct ContourTreeNode
{
	// HighIn and LowIn are edges, or pointers, toward other nodes in the contour tree.
	// I'm not sure how this is possible yet.
	set<int> HighIn;
	set<int> LowIn;

	// "Sep" is short for "separatrix" or "separatrices"
	set<int> HighInSep; //nodes' connecting node and sep
	set<int> LowInSep;

	// value stores the contour tree's actual data,
	// be it heights, or intensities, or whatever.
	float value;

	// Pro is a strange variable that controls many things. It is an array of
	// integers, and each integer in the array contols something different.
	// I have summarized my understanding of its values below.
	vector<int> Pro;
	// Index 0: How to Display This Node
	//				- value of 0: do not display this node
	//				- value of 1: display this node normally
	//				- value of 2: display this node as a "working node"
	// Index 1: The Level of This Node
	//				- the value corresponds directly to the node's level
	// Index 2: "upLeaf"
	//				- unknown; still researching
	// Index 3: "downLeaf"
	//				- unknown; still researching
	// Index 4: Does Not Exist? No Documentation available.
	// Index 5: Positionid
	//				- unsure, but I think this may allow nodes to be identified based on their positions
	// Index 6: criID
	//				- unsure, but "cri" often has to do with critical points.
	//				- ID of critical point that corresponds to it?
};

struct JoinSplitTreeNode
{
	// As in ContourTreeNode, these are edges/pointers to other nodes.
	// I still don't know how.
	set<int> HighIn;
	set<int> LowIn;
}; 

// I renamed this struct to make it make more sense.
// It didn't help much.
/*struct MScripoint
{
	short i,j,k;
	//float value;
	char type;
};*/

// I renamed this struct to make it make more sense.
// It didn't help much.
struct MorseSmaleCriticalPoint
{
	short i,j,k;
	float value;
	char type;
};

/// <summary> Represents a critical point. </summary>
/// <remarks> Used in compare_2, which is used in JoinSplitTree, which is used in CT, which is used in OnDraw </remarks>
struct CriticalPoint
{
	int id;
    int C1, C2;
	float value;
};

/*struct Tree_str
{
  //int id,SpanEdge; //tree's id
  set<int> HighIns;
  set<int> LowIns;
  set<int> HighInSeps; //nodes' connecting node and sep
  set<int> LowInSeps;
  //char High,Low;
  //short HighIn[2];//HighInSep[2]; //nodes' connecting node and sep  
  //short LowIn[2];//LowInSep[2];
  //struct MSSep_str *LowSP[4],*HighSP[4];
  float value;
  short Pro[6]; //0:
  //int 0:display; //0: do not show, 1: shown as normal, 2: shown as working node
  //int 1:level,2:upLeaf,3:downLeaf;
  ////int 5: Positionid,6:criID;  
};  //record contour tree
*/

/// <remarks> used in CopyContTree, and possibly other places </remarks>
struct CTtemp
{
   bool mark; //label as traveled
   int branch1,branch2;
   vector<vector<int>> childset1, childset2;
   float longest1, longest2; 
};

struct MSCsep_str
{
    int **sep;
};

struct MSC_str  //how many node on a vertical slot?
{
    int *node;
};

struct Box_str  
{   int num;    //how many node on a vertical slot?
    int *node;
};

//about contour tree generation
float MaxPersistence(float longest1, float longest2, float longest3);
void ReduceCT(struct ContourTreeNode *&ContTree, struct MorseSmaleCriticalPoint* MSCpoint, int MSCriNum, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, int I, int J, int K, bool flag, bool is_3D);
void RecordSuperArcs(struct ContourTreeNode *ContTree, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, int I, int J, int K, int is_3D);
void RecordSuperArcs(vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, struct ContourTreeNode *&ContTree); 
void GetSuperArc(struct ContourTreeNode *ContTree, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, set<int>::iterator it, int id, int I, int J, int K, int flag, int is_3D); //reduce a full augmented tree to un
void ReduceTree(struct ContourTreeNode *JoinTree, int I, int J, int K); 
void RecordCritical(struct ContourTreeNode * &ContTree, float ***Values, struct MorseSmaleCriticalPoint* &MSCpoint, int &MSCriNum, int I, int J, int K, int flag);
bool compare_2(struct CriticalPoint first, struct CriticalPoint second);
bool fccomp_2(vector<float> lhs, vector<float> rhs);  //sort increasingly	
void RemoveSinNode(struct JoinSplitTreeNode *Tree, int m);
void JoinSplitTree(struct ContourTreeNode * &ContTree, struct JoinSplitTreeNode * &JoinTree, struct JoinSplitTreeNode * &SplitTree, float ***Values, int I, int J, int K, bool flag, bool Is_Members, bool is_3D);
void CT(struct ContourTreeNode * &ContTree, struct JoinSplitTreeNode * &JoinTree, struct JoinSplitTreeNode * &SplitTree, struct MorseSmaleCriticalPoint* &MSCpoint, vector<vector<int>> &PairQ, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, vector<vector<int>> &pairs, vector<int> &recurid, float ***Values, float &fMin, float &fMax, int &MSCriNum, int I, int J, int K, bool flag, bool is_3D);//for build CT with all the vertex	
void SolveDegeneration(float ***Values,float &fMin, float &fMax, int I, int J, int K, bool is_3D);
int FindLow_Highest  (struct JoinSplitTreeNode *Tree, int id, bool flag);
int FindLow_Highest_2(struct ContourTreeNode *Tree, int id, bool flag);

//about contour tree drawing and simplification
vector<vector<int>> DrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<int>> &pairs, vector<int> &recurid, vector<vector<int>> PairQ, int MSCriNum, float fMin, float fMax);
//vector<vector<int>> DrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<int>> &pairs, vector<int> &recurid, vector<vector<int>> PairQ, int MSCriNum);

vector<vector<int>> CTSimplification_UpDown_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, int MSCriNum);

void CopyContTree(struct ContourTreeNode *ContTree, struct CTtemp * &cT, int MSCriNum);
void Traversal(struct ContourTreeNode *&ContTree, struct CTtemp * &cT, vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, int u, int type);
void RecordHierarchyContTree_2(struct ContourTreeNode* ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int MSCriNum,  vector<vector<int>> pairs, vector<int> recurid, vector< vector<int> > &NodePosition, vector < vector <int> > PairQ, int &Width, int &Center, float fMin, float fMax);
void RecordHierarchyContTree_1(struct ContourTreeNode* ContTree, struct MorseSmaleCriticalPoint *MSCpoint,vector<vector<int>> &pairs, vector<int> &recurid, vector<vector<int>> PairQ,int start, int id, int d, int precount, int flag  );
vector<vector<int>> TopDown(struct CTtemp * &cT, struct MorseSmaleCriticalPoint *MSCpoint, int u);//, int MSCriNum);
int searchSlot1(vector<float> SlotHeight, float height, float itv); // used in RecordHierarchyContTree_2
int searchSlot2(vector<float> SlotHeight, float height, float itv); // used in RecordHierarchyContTree_2
int searchSlot3(vector<float> SlotHeight, float height, float itv); // used in RecordHierarchyContTree_2
int searchSlot4(vector<float> SlotHeight, float height, float itv); // used in RecordHierarchyContTree_2


// Search Branch
int SearchBranch(  struct ContourTreeNode * ContTree, int id, int d, vector<int> &nodes);
void getBranchSep(struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<vector<int>>> MSCsep,  vector < vector <int> > PairQ, vector<vector<int>> MSCsepD, vector<vector<vector<int>>> &BranchSep);
///////


//void GetIsoValues(vector<float> &isoValue, vector< vector <int> > &isoEdge, float fMin, float fMax, int num);
//void GetIsoValues(float * &isoValue, int ** &isoEdge, float fMin, float fMax, int num);

//vector<vector<int>> ReDrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int  MSCriNum, vector<vector<int>> MSCsep, vector<vector<int>> MSCsepD, int branchNum, int ** &pairs, int *&recurid, vector<vector<int>> PairQ, int &Width, int &Height, int &Center, vector<vector<int>> &locationArray, vector<vector<int>> &branchLocationArray, float ***Vs, float fMin, float fMax, float ** &seeds, vector<vector<vector<int>>> &BrancheBox, vector< vector < vector <int> > > BranchSep);
vector<vector<int>> ReDrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int  MSCriNum, vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, int branchNum, vector<vector<int>> &pairs, vector<int>  &recurid, vector<vector<int>> PairQ, int &Width, int &Height, int &Center, vector<vector<int>> &locationArray, vector<vector<int>> &branchLocationArray, float ***Vs, float fMin, float fMax, float ** &seeds, vector<vector<vector<int>>>  &BrancheBox, vector< vector < vector <int> > > &BranchSep);
//void RecordHierarchyContTree_2(struct Tree_str * ContTree, struct MScripoint *MSCpoint, int MSCriNum, vector< vector<int> > pairs, vector<int> recurid, vector< vector<int> > &NodePosition, vector < vector <int> > PairQ, int &Width, int &Height,int &Center, float fMin, float fMax);
void RecordHierarchyContTree_2(struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int MSCriNum,int branchNum,  vector< vector<int> > pairs, vector<int> recurid, vector< vector <int> > &NodePosition, vector< vector <int> > PairQ, int &Width, int &Height,int &Center, float fMin, float fMax);
//vector< vector <int> > LocateNodePosition(struct Tree_str * &ContTree, struct MScripoint *MSCpoint, int MSCriNum, vector< vector <int> > NodePosition, float fMin, float fMax);
vector<vector<int>> LocateNodePosition(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int MSCriNum, int branchNum, vector< vector <int> > NodePosition, float fMin, float fMax);
//int FindContTreeNode(float x, float y, vector< vector <int> > locationArray, vector<vector <int> > NodePosition, int Width, int Height); //found clicked node
int FindContTreeNode(float x, float y, vector< vector <int> > locationArray, vector< vector <int> >  NodePosition, int nArraySize, int Width, int Height, char * edgeCoordinator, struct ContourTreeNode * &ContTree); //found clicked node

//vector< vector <int> > LocateBranchPosition(struct Tree_str * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int Width, int Center);
//vector <int> FindAugmemtedContTreeNode(float x, float y,struct Tree_str * ContTree, struct MScripoint *MSCpoint, vector<vector <int> > branchLocationArray, vector<vector <int> > NodePosition, vector< vector <int> > PairQ, float ***Vs, vector< list < vector <int> > > MSCsep, vector< vector <int> > MSCsepD, float fMin);
vector<vector<int>> LocateBranchPosition(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int branchNum, int Width, int Center);
int * FindAugmemtedContTreeNode(float x, float y, struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint,  vector< vector <int> > branchLocationArray, vector< vector <int> > NodePosition, vector< vector <int> > PairQ, float ***Vs, vector< vector< vector <int> > > MSCsep, vector<vector<int>> MSCsepD, vector<vector<vector<int>>>  BranchSep, int Width, float fMin);
vector<vector<float>> TruePosition( vector< vector <int> > NodePosition, int MSCriNum, int Width, int Height, int Center);
int *findMidNode(struct MSCsep_str * MSCsep, vector<vector<int>> MSCsepD, float ***Vs, int sepid, float value);
//int findSep(struct Tree_str * ContTree, int ** MSCsepD, int saddle, int end);
void LocateBranchPoints(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector<vector<vector<int>>> BranchSep, vector<vector<int>> MSCsepD, float ***Vs, int branchNum, float ** &seeds, float fMin);
vector<vector<vector<int>>> LocateBoxPosition(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int branchNum, int Width, int Center, int scale);



#endif // CT_MSC_H



