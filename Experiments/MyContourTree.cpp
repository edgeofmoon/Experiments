#include "MyContourTree.h"
#include "RicVolume.h"
#include "PriorityIndex.h"
#include "MyBitmap.h"
#include "ColorScaleTable.h"
#include "MySpaceFillingNaive.h"
#include "MyPrimitiveDrawer.h"
#include <cstring>
#include <GL/freeglut.h>

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <map>
#include <queue>
using namespace std;

int compareAbsHeight(const float *d1, const float *d2, const float *base)								//	comparison function for all other purposes
{
	if (fabs(*d1 - *base) < fabs(*d2 - *base)) return -1;												//	and compute return values	
	if (fabs(*d1 - *base) > fabs(*d2 - *base)) return 1;
	if (d1 < d2) return -1;													//	break ties with pointer addresses (unique)	
	if (d1 > d2) return 1;
	printf("Major problem:  neither sorted higher.\n");
	printf("%p %p \n", d1, d2);
	return 0;
} // end of compareHeight()

bool compareHeightLogic(const float *d1, const float *d2)								//	comparison function for all other purposes
{
	if (*d1 < *d2) return false;												//	and compute return values	
	if (*d1 > *d2) return true;
	if (d1 < d2) return false;													//	break ties with pointer addresses (unique)	
	else return true;
} // end of compareHeight()


#define FILE_TYPE_ASCII 0
#define FILE_TYPE_VOL 1
#define FILE_TYPE_RAW 2
#define FILE_TYPE_MHA 3
#define FILE_TYPE_MIRG 4
#define FILE_TYPE_NIFTI 5

MyContourTree::MyContourTree(int argc, char** argv)
{
	{ // HeightField()
		//	HeightField() constructor needs to do the following:
		//	A.	read in the dimensions
		//	B.	set nVertices
		//	C.	allocate leafQueue to a default size, and set leafQSize
		//	D.	allocate height and heightSort
		//	E.	read in the data, setting pointers in heightSort, and finding maxHeight and minHeight in the process
		//	F.	call qsort() to sort heightSort
		//	set the first & last bytes of the buffer to '\0'
		timingStart = timingBuffer = (char *)malloc(TIMING_BUFFER_SIZE);
		timingBuffer[0] = '\0';
		timingBuffer[TIMING_BUFFER_SIZE - 1] = '\0';
		//gettimeofday(&lastFrameTime, NULL);
		//gettimeofday(&startTime, NULL);
		time(&lastFrameTime);
		time(&startTime);
		//timingBuffer += sprintf(timingBuffer, "Starting contour tree computation at: %d:%ld\n", startTime.tv_sec % 1000, (long) startTime.tv_usec);
		flushTimingBuffer();

		// 	timingBuffer += sprintf(timingBuffer, "Reading data\n");

		long fileType = FILE_TYPE_ASCII;
		char *suffix;	//	suffix of file name
		suffix = strrchr(argv[1], '.');
		if (suffix != NULL)
		{ // non-NULL suffix
			if (strcmp(suffix, ".vol") == 0)
				fileType = FILE_TYPE_VOL;
			else if (strcmp(suffix, ".raw") == 0)
				fileType = FILE_TYPE_RAW;
			else if (strcmp(suffix, ".i01") == 0)
				fileType = FILE_TYPE_RAW;
			else if (strcmp(suffix, ".img") == 0)
				fileType = FILE_TYPE_RAW;
			else if (strcmp(suffix, ".mha") == 0)
				fileType = FILE_TYPE_MHA;
			else if (strcmp(suffix, ".mirg") == 0)
				fileType = FILE_TYPE_MIRG;
			else if (strcmp(suffix, ".gz") == 0 || strcmp(suffix, ".nii") == 0)
				fileType = FILE_TYPE_NIFTI;
		} // non-NULL suffix

		FILE *inFile;
		sampleSum = 0;
		switch (fileType)
		{ // switch on filetype
		case FILE_TYPE_ASCII:
			LoadFromASCII(argv[1]);
			break;
		case FILE_TYPE_RAW:
			if (argc < 6) { printf("Unable to open RAW format file %s unless sample size and dimensions are specified.\n", argv[1]); throw 0; }
			inFile = fopen(argv[1], "rb");
			if (inFile == NULL) { printf("Unable to open RAW format fiel %s\n", argv[1]); throw 0; }
			sampleSize = atol(argv[2]); if ((sampleSize < 1) || (sampleSize > 8)) { printf("Illegal sample size %d\n", sampleSize); throw 0; }
			xDim = atol(argv[3]); if ((xDim < 1) || (xDim > 2048)) { printf("Illegal x dimension %d\n", xDim); throw 0; }
			yDim = atol(argv[4]); if ((yDim < 1) || (yDim > 2048)) { printf("Illegal y dimension %d\n", yDim); throw 0; }
			zDim = atol(argv[5]); if ((zDim < 1) || (zDim > 2048)) { printf("Illegal z dimension %d\n", zDim); throw 0; }
			LoadFromRaw(inFile);
			break;
		case FILE_TYPE_VOL:
		{
			ifstream fin(argv[1]);
			if (!fin.is_open()) { printf("Unable to open file %s\n", argv[1]); throw 0; }
			fin >> zDim >> yDim >> xDim;
			if ((xDim < 1) || (xDim > 2048)) { printf("Illegal x dimension %d\n", xDim); throw 0; }
			if ((yDim < 1) || (yDim > 2048)) { printf("Illegal y dimension %d\n", yDim); throw 0; }
			if ((zDim < 1) || (zDim > 2048)) { printf("Illegal z dimension %d\n", zDim); throw 0; }
		}
		if (argc < 3) { printf("Unable to open VOL format file %s unless sample size is specified.\n", argv[1]); throw 0; }
		sampleSize = atol(argv[2]); if ((sampleSize < 1) || (sampleSize > 8)) { printf("Illegal sample size %d\n", sampleSize); throw 0; }
		inFile = fopen(argv[1], "rb");
		if (inFile == NULL) { printf("Unable to open VOL format file %s\n", argv[1]); throw 0; }
		//	rewind to the beginning of the block
		fseek(inFile, -xDim*yDim*zDim*sampleSize, SEEK_END);
		LoadFromRaw(inFile);
		break;
		case FILE_TYPE_MHA:
		{
			FILE *dimFile = fopen(argv[1], "r");
			if (dimFile == NULL) { printf("Unable to open file %s\n", argv[1]); throw 0; }
			fscanf(dimFile, "NDims = 3\nDimSize = %d %d %d", &zDim, &yDim, &xDim);
			printf("Dimensions read: %d %d %d\n", xDim, yDim, zDim);
			if ((xDim < 1) || (xDim > 2048)) { printf("Illegal x dimension %d\n", xDim); throw 0; }
			if ((yDim < 1) || (yDim > 2048)) { printf("Illegal y dimension %d\n", yDim); throw 0; }
			if ((zDim < 1) || (zDim > 2048)) { printf("Illegal z dimension %d\n", zDim); throw 0; }
		}
		if (argc < 3) { printf("Unable to open MHA format file %s unless sample size is specified.\n", argv[1]); throw 0; }
		sampleSize = atol(argv[2]); if ((sampleSize < 1) || (sampleSize > 8)) { printf("Illegal sample size %d\n", sampleSize); throw 0; }
		inFile = fopen(argv[1], "rb");
		if (inFile == NULL) { printf("Unable to open MHA format file %s\n", argv[1]); throw 0; }
		//	rewind to the beginning of the block
		fseek(inFile, -xDim*yDim*zDim*sampleSize, SEEK_END);
		LoadFromRaw(inFile);
		break;
		case FILE_TYPE_MIRG:
		{
			FILE *dimFile = fopen(argv[1], "r");
			if (dimFile == NULL) { printf("Unable to open file %s\n", argv[1]); throw 0; }
			int nRead = fscanf(dimFile, "X =%d\nY =%d\nZ =%d", &zDim, &yDim, &xDim);
			printf("Dimensions read: %d %d %d\n", xDim, yDim, zDim);
			if ((xDim < 1) || (xDim > 2048)) { printf("Illegal x dimension %d\n", xDim); throw 0; }
			if ((yDim < 1) || (yDim > 2048)) { printf("Illegal y dimension %d\n", yDim); throw 0; }
			if ((zDim < 1) || (zDim > 2048)) { printf("Illegal z dimension %d\n", zDim); throw 0; }
		}
		sampleSize = 2;
		inFile = fopen(argv[1], "rb");
		if (inFile == NULL) { printf("Unable to open MIRG format file %s\n", argv[1]); throw 0; }
		//	rewind to the beginning of the block
		fseek(inFile, -xDim*yDim*zDim*sampleSize, SEEK_END);
		LoadFromRaw(inFile);
		break;
		case FILE_TYPE_NIFTI:
		{
			FILE *dimFile = fopen(argv[1], "r");
			if (dimFile == NULL) { printf("Unable to open file %s\n", argv[1]); throw 0; }
			fclose(dimFile);
			RicVolume vol;
			vol.Read(argv[1]);
			zDim = vol.get_numz();
			yDim = vol.get_numy();
			xDim = vol.get_numx();
			printf("Dimensions read: %d %d %d\n", xDim, yDim, zDim);
			if ((xDim < 1) || (xDim > 2048)) { printf("Illegal x dimension %d\n", xDim); throw 0; }
			if ((yDim < 1) || (yDim > 2048)) { printf("Illegal y dimension %d\n", yDim); throw 0; }
			if ((zDim < 1) || (zDim > 2048)) { printf("Illegal z dimension %d\n", zDim); throw 0; }
			LoadFromNifti(argv[1]);
			break;
		}
		} // switch on filetype      

		printf("SampleSum: %12.1f\n", sampleSum);

		// H.	invoke the routines to do the rest of construction
		CreateJoinTree();
		CreateSplitTree();
		AugmentTrees();
		CombineTrees();

		//	I.	clean up any dynamically allocated working memory	
		joinComponent.Construct(0, 0, 0);											//	reset join component to 0 logical size
		splitComponent.Construct(0, 0, 0);											//	reset split component to 0 logical size
		//	we keep this, because it lets us know which vertices belong to which superarc
		//	contourComponent.Construct(0, 0, 0);										//	reset contour component to 0 logical size	
		joinArcs.Construct(0, 0, 0); splitArcs.Construct(0, 0, 0);							//	likewise for join and split arc arrays
		free(leafQueue);
		leafQueue = NULL;
		free(heightSort);
		heightSort = NULL;

		//	J.	perform any additional processing (such as collapsing)
		SetNodeXPositions();													//	sets the x-positions of the nodes (must happen before epsilon collapse)
		CollapseEpsilonEdges();													//	collapses epsilon edges	
		collapseRecord = (long *)calloc(nNonEpsilonArcs, sizeof(long));				//	allocate array for collapse record
		collapseBounds = (long *)calloc(nNonEpsilonArcs, sizeof(long));				//	allocate array for collapse bounds
		//	collapsePriority = PRIORITY_HYPERVOLUME;
		//	CollapseLeafPruning(1, nVertices);												
		savedNextSuperarc = nextSuperarc;											//	store this for use in resetting collapse priority

		//	K.	allocate memory for variables associated with flexible isosurfaces
		active = (long *)malloc(sizeof(long) * nSuperarcs);							//	and ample room for the active set
		selected = (long *)malloc(sizeof(long) * nSuperarcs);							//	the selected set . . .
		restorable = (long *)malloc(sizeof(long) * nSuperarcs);						//	the restorable set
		selectionRoot = noContourSelected;											//	and, initially, no selection

		//	set the active, &c. lists
		nActiveArcs = nSelectedArcs = nRestorableArcs = 0;
		SetInitialColours();

		baseDisplayListID = glGenLists(nSuperarcs * 2);								//	create display lists for each superarc

		time(&thisTime);
		// 	timingBuffer += sprintf(timingBuffer, "Construction complete at: %ld:%ld\n", thisTime.tv_sec % 1000, thisTime.tv_usec);
		//timingBuffer += sprintf(timingBuffer, "Construction took %8.5f seconds\n", (float)(thisTime.tv_sec - startTime.tv_sec) + 1E-6 * (thisTime.tv_usec - startTime.tv_usec));
		timingBuffer += sprintf(timingBuffer, "Construction took %8.5f seconds\n", (float)difftime(thisTime, startTime));
		timingBuffer += sprintf(timingBuffer, "Contour tree has %ld superarcs\n", nSuperarcs);
		timingBuffer += sprintf(timingBuffer, "Memory occupied by contour tree & related structures: %ld\n",
			nSuperarcs * sizeof(Superarc) * 2 + 			//	superarc storage, including collapsed superarcs
			nSupernodes * sizeof(Supernode) + 				//	supernode storage
			nSuperarcs * sizeof(long) * 5 +				//	valid (x2), active, selected, restorable
			nSupernodes * sizeof(long) +					//	validNodes
			nNonEpsilonArcs * sizeof(long) * 2				//	collapseRecord & collapseBounds
			);
		flushTimingBuffer();

		pathLengths = (long *)malloc(sizeof(long) * 2 * nSuperarcs);
		nContourTriangles = (long *)malloc(sizeof(long) * 2 * nSuperarcs);
		extractionTime = (float *)malloc(sizeof(float) * 2 * nSuperarcs);

		for (int i = 0; i < 2 * nSuperarcs; i++)
		{
			pathLengths[i] = nContourTriangles[i] = 0;
			extractionTime[i] = 0.0;
		}
		//	register the function to dump the timing buffer to output
		atexit(printTimingBuffer);
	} // HeightField()
	mLabelVolume = NULL;
	supernodesExt = NULL;
	mMaskVolume.Construct(xDim, yDim, zDim);
	// for max height equals one to make sure height rendering consistency
	maxHeight = 1;
	mLogWidthScale = 1;
	mLinearWidthScale = 0.01;
	mScientificWidthScale = 0.1;
	mExponent_Scale = 0.1;
	mExponent_Offset = 0;
	mPruningThreshold = 10;
	mZoomLevel = 2;
	mContourTreeAlpha = 1;
	mSuperArcsBkup = NULL;
	mSuperNodesBkup = NULL;
	mSuperArcsBkup = new Superarc[2 * nSuperarcs];
	mSuperNodesBkup = new Supernode[nSupernodes];
	mValidNodes = new long[nSupernodes];
	mValidArcs = new long[nSuperarcs * 2];

	mDefaultScale = MappingScale_Sci;
	mAltScale = MappingScale_Linear;
	mHistogramSide = HistogramSide_Right;
	BackupTree();
	//SetNodeXPositionsExt();

	mSigArcThreshold_P = 0.01;
	mSigArcThreshold_VolRatio = 0.5;
	mDiffMode = false;
	mArcCombineMode = ArcCombineMode_Union;
	mNonSigArcWidth = 0.0001;
}


//	CombineTrees()
void MyContourTree::CombineTrees()											//	combines join & split trees to produce contour tree
{ // CombineTrees()
	//	CombineTrees() needs to do the following:
	//	C.	Loop through the leaves on the leaf queue
	//		i.	if upper leaf
	//			a.	add to contour tree
	//			b.	delete from join tree
	//			c.	delete from split tree
	//			d.	test other end to see if it should be added to leaf queue
	//		ii.	if lower leaf
	//			a.	add to contour tree
	//			b.	delete from split tree
	//			c.	delete from join tree
	//			d.	test other end to see if it should be added to leaf queue
	//	D.	Clean up excess memory, &c.
	long x, y, z;														//	coordinates of vertex
	long xNext, yNext, zNext;											//	for walking down the arcs
	float *theVertex;													//	pointer to vertex in data set
	Component *jComp, *sComp;											//	pointers to component in join & split trees
	float *otherEnd;													//	other end of the arc added
	long newSuperarc;													//	the arc we just added
	long upArc, downArc;												//	indices for walks around vertices

	//	JoinArcsToDotFile("joinArcs", dotFileNo++);
	//	SplitArcsToDotFile("splitArcs", dotFileNo++);

#ifdef DEBUG_COMBINE_TREES
	//	printf("About to start merge loop.\n");
	//	printf("Join Tree: \n");
	//	PrintJoinTree();
	//	printf("Join Components: \n");
	//	PrintJoinComponents();
	//	printf("Split Tree: \n");
	//	PrintSplitTree();
	//	printf("Split Components: \n");
	//	PrintSplitComponents();
	//	printf("Leaf Queue: \n");
	//	PrintLeafQueue();
#endif
	time(&thatTime);
	//timingBuffer += sprintf(timingBuffer, "Starting to merge join and split trees at %ld: %ld\n", (long)thatTime.tv_sec % 1000, (long)thatTime.tv_usec);
	timingBuffer += sprintf(timingBuffer, "Starting to merge join and split trees now.\n");
	flushTimingBuffer();

	nodeLookup.Construct(xDim, yDim, zDim);									//	build array of supernode ID's
	for (int i = 0; i < xDim; i++)										//	walk through each dimension
		for (int j = 0; j < yDim; j++)
			for (int k = 0; k < zDim; k++)
			{
				nodeLookup(i, j, k) = NO_SUPERNODE;						//	setting the ID to predictable value
				visitFlags(i, j, k) = 1;									//	and set the "visit" flag, too
			}

	//	C.	Loop through the leaves on the leaf queue
	for (vertex = 0; vertex < nSuperarcs; vertex++)							//	keep going until we've added n-1 edges
	{ // C.
		//		CheckContourTree();
		//		printf("%ld\n", vertex);
		//		if (vertex % 10 == 0) { printf("."); fflush(stdout); }

#ifdef DEBUG_COMBINE_TREES
		printf("Loop %ld\n", vertex);
		//	printf("Join Tree: \n");
		//	PrintJoinTree();
		//	printf("Split Tree: \n");
		//	PrintSplitTree();
		//	printf("Leaf Queue: \n");
		//	PrintLeafQueue();
		//	printf("Contour Components: \n");
		//	PrintContourComponents();
		//	getc(stdin);
		//	BAILOUT;
#endif
		theVertex = leafQueue[vertex];									//	retrieve the vertex from the leaf queue
		height.ComputeIndex(theVertex, x, y, z);							//	compute its indices
		jComp = joinComponent(x, y, z);									//	retrieve the corresponding join component
		sComp = splitComponent(x, y, z);									//	and the corresponding split component
		//		i.	if upper leaf
		if (jComp->nextHi == jComp)										//	since we already know that it's a leaf, this suffices
		{ // C. i.
			//			a.	add to contour tree
			otherEnd = jComp->loEnd;										//	grab a pointer to the other end
			if (jComp->seedFrom == NULL)									//	if there wasn't a seed stored on the edge
				newSuperarc = AddSuperarc(theVertex, otherEnd, theVertex, FindDescendingNeighbour(theVertex), NULL, NULL);
			else
				newSuperarc = AddSuperarc(theVertex, otherEnd, NULL, NULL, jComp->seedFrom, jComp->seedTo);
			//	now compute the sum of the nodes this side of the top end: initial -1 excludes the vertex proper
			superarcs[newSuperarc].nodesThisSideOfTop = superarcs[newSuperarc].nodesThisSideOfBottom = nVertices - 1;
			superarcs[newSuperarc].sampleSumTop = sampleSum - *theVertex;
			for (downArc = superarcs[newSuperarc].nextDown; downArc != newSuperarc; downArc = superarcs[downArc].nextDown)
			{ // other downarcs exist
				superarcs[newSuperarc].nodesThisSideOfTop -= superarcs[downArc].nodesThisSideOfTop;
				superarcs[newSuperarc].sampleSumTop -= superarcs[downArc].sampleSumTop;
				//				ContourTreeToDotFile("building", dotFileNo++);
			} // other downarcs exist
			//	up arcs: first we have to check if any exist
			upArc = supernodes[superarcs[newSuperarc].topID].upList;
			if (upArc != NO_SUPERARC)
			{ // there are uparcs
				do
				{ // walk around them
					superarcs[newSuperarc].nodesThisSideOfTop -= superarcs[upArc].nodesThisSideOfBottom;
					superarcs[newSuperarc].sampleSumTop -= superarcs[upArc].sampleSumBottom;
					upArc = superarcs[upArc].nextUp;
				} // walk around them					
				while (upArc != supernodes[superarcs[newSuperarc].topID].upList);
			} // there are uparcs

			visitFlags(x, y, z) = 0;										//	set flag for the vertex to mark "visited"
			for (float *nextNode = joinArcs(x, y, z); nextNode != otherEnd; nextNode = joinArcs(xNext, yNext, zNext))
			{ // nextNode											//	walk along join arcs to transfer nodes
				height.ComputeIndex(nextNode, xNext, yNext, zNext);			//	find the x, y, z indices for the next step
				//				printf("Walking past (%1d, %1d, %1d): %8.5f\n", xNext, yNext, zNext, *nextNode);
				if (visitFlags(xNext, yNext, zNext) == 1)					//	1 => vertex is not yet on a superarc
				{ // first time this vertex has been processed
					//					printf(": counting.");
					visitFlags(xNext, yNext, zNext) = 0;					//	set to 0 to mark that its used, and reset flags for rendering
					height.ComputeIndex(nextNode, xNext, yNext, zNext);		//	find the x, y, z indices
					superarcs[newSuperarc].nodesOnArc++;					//	increment the superarcs node count
					superarcs[newSuperarc].sampleSumOnArc += height(xNext, yNext, zNext);
					mArcNodes[newSuperarc].push_back(nextNode);
				} // first time this vertex has been processed
				//				printf("\n");
			} // nextNode
			//	compute the count of nodes this side of the bottom
			superarcs[newSuperarc].nodesThisSideOfBottom = superarcs[newSuperarc].nodesOnArc + (nVertices - superarcs[newSuperarc].nodesThisSideOfTop);
			superarcs[newSuperarc].sampleSumBottom = sampleSum - superarcs[newSuperarc].sampleSumTop + superarcs[newSuperarc].sampleSumOnArc;
			//			b.	delete from join tree
			//				note that otherEnd is degree 2 or higher (i.e. it is guaranteed to have a downarc, even for last edge, because of existence of -infinity
			if (jComp->nextLo->hiEnd == otherEnd)							//	if the next edge at low end is downwards
				jComp->nextLo->lastHi = jComp->lastLo;						//	reset its lastHi pointer
			else	jComp->nextLo->lastLo = jComp->lastLo;						//	otherwise reset the lastLo pointer
			if (jComp->lastLo->hiEnd == otherEnd)							//	if the last edge at low end is downwards
				jComp->lastLo->nextHi = jComp->nextLo;						//	reset its nextHi pointer
			else	jComp->lastLo->nextLo = jComp->nextLo;						//	otherwise reset the nextLo pointer
			delete jComp;												//	get rid of the jComp edge
			joinComponent(x, y, z) = NULL;									//	get rid of it in the jComponent array as well
			nJoinSupernodes--;
			//			c.	delete from split tree
			//				since we have +infinity, there will always be an up and a down arc
			//				this is true even for the last edge, since we start at an upper leaf U.  This means that there is a down-arc to the other remaining node, L.
			//				and since sComp is the departing edge travelling upwards, UL must be downwards in the split tree.
			//				all we do is collapse sComp & sComp->nextLo onto sComp->nextLo
			if (sComp->nextHi == sComp)									//	i.e. sComp leads to +inf
				sComp->nextLo->nextHi = sComp->nextLo->lastHi = sComp->nextLo;	//	set the upper arcs to itself (i.e. no higher neighbours)
			else	//	not an edge to +inf
			{ // not edge to +inf
				sComp->nextLo->nextHi = sComp->nextHi;						//	set the upper arcs
				sComp->nextLo->lastHi = sComp->lastHi;
			} //  not edge to +inf
			sComp->nextLo->hiEnd = sComp->hiEnd;							//	transfer the high end
			sComp->nextLo->seedFrom = sComp->seedFrom;						//	transfer the seed as well
			sComp->nextLo->seedTo = sComp->seedTo;							//	transfer the seed as well
			if (sComp->nextHi->loEnd == sComp->hiEnd)						//	if nextHi is an up-arc
				sComp->nextHi->lastLo = sComp->nextLo;						//	adjust the low end
			else	sComp->nextHi->lastHi = sComp->nextLo;						//	otherwise the high end
			if (sComp->lastHi->loEnd == sComp->hiEnd)						//	if lastHi is an up-arc
				sComp->lastHi->nextLo = sComp->nextLo;						//	adjust the low end
			else	sComp->lastHi->nextHi = sComp->nextLo;						//	otherwise the high end
			delete sComp;												//	delete the now-useless edge
			splitComponent(x, y, z) = NULL;									//	get rid of it in the sComponent array as well
			nSplitSupernodes--;
			//			d.	test other end to see if it should be added to leaf queue
			//			we have reduced the up-degree of otherEnd in the join tree, and haven't changed the degrees in the split tree
			height.ComputeIndex(otherEnd, x, y, z);							//	compute indices
			sComp = splitComponent(x, y, z);								//	retrieve the split component
			jComp = joinComponent(x, y, z);									//	and the join component
			if (((jComp->nextHi == jComp)								//	two ways otherEnd can be a leaf:  upper 
				&& (sComp->nextLo->nextHi == sComp))				//		(jComp has no "higher end" nbr, sComp has one each way)
				|| (((sComp->nextLo == sComp)							//	or lower
				&& (jComp->nextHi->nextLo == jComp))))		//		(sComp has no "lower end" nbr, jComp has one each way)
			{
				//				printf("Adding leaf %2ld (%p) to leaf queue\n", leafQSize, otherEnd);
				leafQueue[leafQSize++] = otherEnd;							//	so add it already
			}
		} // C. i.
		else // i.e. a lower leaf
		{ // C. ii.
			//			a.	add to contour tree
			otherEnd = sComp->hiEnd;										//	grab a pointer to the other end
			if (sComp->seedFrom == NULL)									//	if there wasn't a seed stored on the edge
				newSuperarc = AddSuperarc(otherEnd, theVertex, otherEnd, FindDescendingNeighbour(otherEnd), NULL, NULL);
			else
				newSuperarc = AddSuperarc(otherEnd, theVertex, sComp->seedFrom, sComp->seedTo, NULL, NULL);
			//	add the superarc to the contour tree
			//			if (newSuperarc == 175)
			//				ContourTreeToDotFile("building", dotFileNo++);
			//	now compute the sum of the nodes this side of the top end: initial -1 excludes the vertex proper
			superarcs[newSuperarc].nodesThisSideOfTop = superarcs[newSuperarc].nodesThisSideOfBottom = nVertices - 1;
			superarcs[newSuperarc].sampleSumBottom = sampleSum - *theVertex;
			//			printf("Bot: %8.5f\n", superarcs[newSuperarc].sampleSumBottom);
			for (upArc = superarcs[newSuperarc].nextUp; upArc != newSuperarc; upArc = superarcs[upArc].nextUp)
			{ // other uparcs exist
				//				printf("Up arc %d\n", upArc);
				superarcs[newSuperarc].nodesThisSideOfBottom -= superarcs[upArc].nodesThisSideOfBottom;
				superarcs[newSuperarc].sampleSumBottom -= superarcs[upArc].sampleSumBottom;
				//				printf("Bot: %8.5f \n", superarcs[newSuperarc].sampleSumBottom);
			} // other uparcs exist
			//	down arcs: first we have to check if any exist
			downArc = supernodes[superarcs[newSuperarc].bottomID].downList;
			if (downArc != NO_SUPERARC)
			{ // there are downarcs
				do
				{ // walk around them
					superarcs[newSuperarc].nodesThisSideOfBottom -= superarcs[downArc].nodesThisSideOfTop;
					superarcs[newSuperarc].sampleSumBottom -= superarcs[downArc].sampleSumTop;
					//					printf("Bot: %8.5f\n ", superarcs[newSuperarc].sampleSumBottom);
					downArc = superarcs[downArc].nextDown;
				} // walk around them					
				while (downArc != supernodes[superarcs[newSuperarc].bottomID].downList);
			} // there are downarcs

			//			printf("On: %8.5f\n", superarcs[newSuperarc].sampleSumOnArc);
			visitFlags(x, y, z) = 0;										//	set flag for the vertex to mark "visited"
			for (float *nextNode = splitArcs(x, y, z); nextNode != otherEnd; nextNode = splitArcs(xNext, yNext, zNext))
			{ // nextNode											//	walk along split arcs to transfer nodes
				height.ComputeIndex(nextNode, xNext, yNext, zNext);			//	find the x, y, z indices
				//				printf("Walking past (%1d, %1d, %1d): %8.5f\n", xNext, yNext, zNext, *nextNode);
				if (visitFlags(xNext, yNext, zNext))						//	1 => vertex is not yet on a superarc
				{ // first time this vertex has been processed
					//					printf(": counting.");
					visitFlags(xNext, yNext, zNext) = 0;					//	set to 0 to mark that its used, and reset flags for rendering
					height.ComputeIndex(nextNode, xNext, yNext, zNext);		//	find the x, y, z indices
					superarcs[newSuperarc].nodesOnArc++;					//	increment the superarcs node count
					superarcs[newSuperarc].sampleSumOnArc += height(xNext, yNext, zNext);
					mArcNodes[newSuperarc].push_back(nextNode);
					//					printf("On: %8.5f\n", superarcs[newSuperarc].sampleSumOnArc);
				} // first time this vertex has been processed
				//				printf("\n");
			} // nextNode

			//	compute the count of nodes this side of the top
			superarcs[newSuperarc].nodesThisSideOfTop = superarcs[newSuperarc].nodesOnArc + (nVertices - superarcs[newSuperarc].nodesThisSideOfBottom);
			superarcs[newSuperarc].sampleSumTop = sampleSum - superarcs[newSuperarc].sampleSumBottom + superarcs[newSuperarc].sampleSumOnArc;
			//			printf("Top: %8.5f\n", superarcs[newSuperarc].sampleSumTop);
			//			b.	delete from split tree
			//				note that otherEnd is degree 2 or higher (i.e. it is guaranteed to have a up arc, even for last edge, because of existence of -infinity)
			if (sComp->nextHi->loEnd == otherEnd)							//	if the next edge at high end is upwards
				sComp->nextHi->lastLo = sComp->lastHi;						//	reset its lastLo pointer
			else	sComp->nextHi->lastHi = sComp->lastHi;						//	otherwise reset the lastHi pointer
			if (sComp->lastHi->loEnd == otherEnd)							//	if the last edge at high end is upwards
				sComp->lastHi->nextLo = sComp->nextHi;						//	reset its nextLo pointer
			else	sComp->lastHi->nextHi = sComp->nextHi;						//	otherwise reset the nextHi pointer
			delete sComp;												//	get rid of the sComp edge
			splitComponent(x, y, z) = NULL;									//	get rid of it in the sComponent array as well
			nSplitSupernodes--;
			//			c.	delete from join tree
			//				since we have -infinity, there will always be an up and a down arc
			//				this is true even for the last edge, since we start at a lower leaf L.  This means that there is an up-arc to the other remaining node, U.
			//				and since jComp is the departing edge travelling downwards, LU must be upwards in the split tree.
			//				all we do is collapse jComp & jComp->nextHi onto jComp->nextHi
			if (jComp->nextLo == jComp)									//	i.e. jComp leads to -inf
				jComp->nextHi->nextLo = jComp->nextHi->lastLo = jComp->nextHi;	//	set the lower arcs to itself (i.e. no higher neighbours)
			else	//	not an edge to -inf
			{ // not edge to -inf
				jComp->nextHi->nextLo = jComp->nextLo;						//	set the upper arcs
				jComp->nextHi->lastLo = jComp->lastLo;
			} //  not edge to -inf
			jComp->nextHi->loEnd = jComp->loEnd;							//	transfer the low end
			jComp->nextHi->seedFrom = jComp->seedFrom;						//	transfer the seed
			jComp->nextHi->seedTo = jComp->seedTo;							//	transfer the seed
			if (jComp->nextLo->hiEnd == jComp->loEnd)						//	if nextLo is a down-arc
				jComp->nextLo->lastHi = jComp->nextHi;						//	adjust the high end
			else	jComp->nextLo->lastLo = jComp->nextHi;						//	otherwise the low end
			if (jComp->lastLo->hiEnd == jComp->loEnd)							//	if lastLo is a down-arc
				jComp->lastLo->nextHi = jComp->nextHi;						//	adjust the high end
			else	jComp->lastLo->nextLo = jComp->nextHi;						//	otherwise the low end
			delete jComp;												//	delete the now-useless edge
			joinComponent(x, y, z) = NULL;									//	get rid of it in the jComponent array as well
			nJoinSupernodes--;
			//			d.	test other end to see if it should be added to leaf queue
			//			we have reduced the up-degree of otherEnd in the join tree, and haven't changed the degrees in the split tree
			height.ComputeIndex(otherEnd, x, y, z);							//	compute indices
			sComp = splitComponent(x, y, z);								//	retrieve the split component
			jComp = joinComponent(x, y, z);								//	and the join component
			if (((jComp->nextHi == jComp)								//	two ways otherEnd can be a leaf:  upper 
				&& (sComp->nextLo->nextHi == sComp))				//		(jComp has no "higher end" nbr, sComp has one each way)
				|| (((sComp->nextLo == sComp)								//	or lower
				&& (jComp->nextHi->nextLo == jComp))))		//		(sComp has no "lower end" nbr, jComp has one each way)
			{
				//				printf("Adding leaf %2ld (%p) to leaf queue\n", leafQSize, otherEnd);
				leafQueue[leafQSize++] = otherEnd;							//	so add it already
			}
		} // C. ii.
	} // C.
	// (x, y, z) i now set for the last of the "other ends", so set the visitFlag to 0
	visitFlags(x, y, z) = 0;
	//	D.	Clean up excess memory, &c.
	//	start off by getting rid of the two remaining edges in the join and split tree
	nodeLookup.Construct(0, 0, 0);
	//	delete joinRoot;														//	delete the root (remaining arc) in join tree
	//	CheckContourTree();
	//	delete splitRoot;														//	delete the root (remaining arc) in split tree
	joinRoot = splitRoot = NULL;											//	and reset to something predictable
	time(&thisTime);
	timingBuffer += sprintf(timingBuffer, "Finished merging trees mow.\n");
	timingBuffer += sprintf(timingBuffer, "Merging took %8.5f seconds to compute: contour tree has %1d supernodes\n",
		//(float)(thisTime.tv_sec - thatTime.tv_sec) + 1E-6 * ((long)thisTime.tv_usec - (long)thatTime.tv_usec), nSupernodes);
		(float)difftime(thisTime, thatTime), nSupernodes);
	flushTimingBuffer();
	//	PrintContourTree();
} // end of CombineTrees()

MyContourTree::~MyContourTree()
{
	delete[] supernodesExt;
	delete[] mSuperArcsBkup;
	delete[] mSuperNodesBkup;
	delete[] mValidNodes;
	delete[] mValidArcs;
	delete mLabelVolume;
}

void MyContourTree::LoadLabelVolume(char* fileName){
	if (mLabelVolume != NULL) delete mLabelVolume;
	mLabelVolume = new RicVolume;
	mLabelVolume->Read(fileName);
}

void MyContourTree::LoadLabelTable(char* fileName){
	ifstream infile;
	infile.open(fileName);
	if (infile.is_open()){
		mLabelName.clear();
		while (!infile.eof()){
			char line[1000];
			infile.getline(line, 1000);
			stringstream ss(line);
			int label;
			string name;
			ss >> label >> name;
			if (!name.empty()){
				mLabelName[label] = name;
			}
		}
	}
	for (int i = 0; i < 51; i++){
		if (mLabelName[i].empty()) {
			mLabelName[i] = "R" + to_string(i);
		}
	}
}

void MyContourTree::SetNodeXPositionsExt(MappingScale scale){
	long highestNode = validNodes[0];
	long lowestNode = validNodes[0];
	for (long theNodeIndex = 1; theNodeIndex < nValidNodes; theNodeIndex++){
		long theNode = validNodes[theNodeIndex];
		if (compareHeight(supernodes[theNode].value, supernodes[highestNode].value) > 0){
			highestNode = theNode;
		}
		if (compareHeight(supernodes[theNode].value, supernodes[lowestNode].value) < 0){
			lowestNode = theNode;
		}
	}

	if (supernodesExt){
		delete[] supernodesExt;
	}
	supernodesExt = new SuperNodeExt[nSupernodes];
	int numNodes = FindHeightRangeAndLeaves(highestNode, -1);
	cout << "nodes accounted for layout: " << numNodes << endl;
	//LayoutSubTree2(highestNode, -1, 0, 1);
	updateArcHistograms();
	mDefaultScale = scale;
	float rescale = UpdateSubTreeLayout(highestNode, -1, 0, 1);;
	switch (scale)
	{
	case MyContourTree::MappingScale_Linear:
		mLinearWidthScale *= IsDiffMode() ? 1 : rescale;
		cout << "Linear Scale updated: " << mLinearWidthScale << endl;
		break;
	case MyContourTree::MappingScale_Sci:
		mScientificWidthScale *= IsDiffMode() ? 1 : rescale;
		cout << "Scientific Scale updated: " << mScientificWidthScale << endl;
		break;
	case MyContourTree::MappingScale_Log:
		mLogWidthScale *= IsDiffMode() ? 1 : rescale;
		cout << "Log Scale updated: " << mLogWidthScale << endl;
		break;
	default:
		break;
	}
}

/*
vector<long> MyContourTree::FindPathDown2(long highNode, long lowNode, std::vector<long> currentPath){
	if (currentPath.size() == 0 && highNode != lowNode){
		currentPath.push_back(highNode);
	}
	if (highNode == lowNode){
		currentPath.push_back(highNode);
		return currentPath;
	}
	else if (supernodes[highNode].downDegree == 0){
		return currentPath;
	}
	else{
		vector<long> path;
		vector<long> tmpPath = currentPath;
		tmpPath.push_back(-1);
		// downDegree >= 1
		long theArc = supernodes[highNode].downList;
		do{
			long otherNode = superarcs[theArc].bottomID;
			tmpPath.back()=otherNode;
			path = FindPathDown2(otherNode, lowNode, tmpPath);
			if (path.back() == lowNode) break;
			theArc = superarcs[theArc].nextDown;
		} while (theArc != supernodes[highNode].downList);
		return path;
	}
}
*/

bool MyContourTree::FindPathDown(long highNode, long lowNode, std::vector<long>& path){
	if (highNode == lowNode){
		return true;
	}
	else if (supernodes[highNode].downDegree == 0){
		return highNode == lowNode;
	}
	else{
		// downDegree >= 1
		long theArc = supernodes[highNode].downList;
		do{
			long otherNode = superarcs[theArc].bottomID;
			if (FindPathDown(otherNode, lowNode, path)){
				path.push_back(otherNode);
				return true;
			};
			theArc = superarcs[theArc].nextDown;
		} while (theArc != supernodes[highNode].downList);
		return false;
	}
}

bool MyContourTree::FindPath(long sourceNode, long targetNode, std::vector<long>& path, long parent){
	if (sourceNode == targetNode){
		path.push_back(sourceNode);
		return true;
	}
	else{
		vector<long> neighbors;
		GetNeighbors(sourceNode, neighbors);
		for (unsigned int i = 0; i < neighbors.size(); i++){
			long nbr = neighbors[i];
			if (nbr != parent){
				path.push_back(sourceNode);
				if (FindPath(nbr, targetNode, path, sourceNode)){
					return true;
				}
				path.pop_back();
			}
		}
		return false;
	}
}

float* MyContourTree::FindHighestPath(long rootNode, long parentNode, std::vector<long>& path, float *baseHeight){
	float* height = supernodes[rootNode].value;
	if ((supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf())
		&& parentNode>0){
		path.push_back(rootNode);
		return height;
	}
	list<long> neighbors;
	GetNeighbors(rootNode, neighbors);
	long theNode = -1;
	path.push_back(rootNode);
	vector<long> pathTmp;
	while (neighbors.size()>0){
		theNode = neighbors.front();
		neighbors.pop_front();
		if (theNode != parentNode){
			vector<long> pathCopy = path;
			float *tmpHeight = FindHighestPath(theNode, rootNode, pathCopy, baseHeight);
			if (compareAbsHeight(tmpHeight, height, baseHeight) > 0){
				height = tmpHeight;
				pathTmp = pathCopy;
			}
		}
	}
	path = pathTmp;
	return height;
}

float* MyContourTree::FindHighestUpPath(long rootNode, vector<long>& path){
	long leave = FindMonotoneUpNode(rootNode);
	FindMonotoneUpPath(rootNode, leave, path);
	return supernodes[leave].value;
}

float* MyContourTree::FindHighestDownPath(long rootNode, std::vector<long>& path){
	/*
	if (supernodes[rootNode].downDegree == 0){
		path.push_back(rootNode);
		return supernodes[rootNode].value;
	}
	else{
		// downDegree >= 1
		long theArc = supernodes[rootNode].downList;
		float *height = supernodes[rootNode].value;
		path.push_back(rootNode);
		list<long> tmpPath;
		list<long> thisPath;
		do{
			long otherNode = superarcs[theArc].bottomID;
			thisPath = path;
			float *thisHeight = FindHighestDownPath(otherNode, thisPath);
			if (compareHeight(thisHeight, height)<0){
				height = thisHeight;
				tmpPath = thisPath;
			};
			theArc = superarcs[theArc].nextDown;
		} while (theArc != supernodes[rootNode].downList);
		path = tmpPath;
		return height;
	}
	*/
	long leave = FindMonotoneDownNode(rootNode);
	FindMonotoneDownPath(rootNode, leave, path);
	return supernodes[leave].value;
}

void MyContourTree::LayoutSubTree(long rootNode, long parentNode, float xStart, float xEnd){
	int numLeaves = supernodesExt[rootNode].numLeaves;
	vector<long> centerPath;
	//FindHighestPath(rootNode, parentNode, centerPath, supernodes[rootNode].value);
	if (parentNode >= 0){
		if (supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf()){
			supernodes[rootNode].xPosn = (xStart + xEnd) / 2;
			return;
		}
		if (compareHeight(supernodes[rootNode].value, supernodes[parentNode].value) > 0){
			FindHighestUpPath(rootNode, centerPath);
		}
		else{
			FindHighestDownPath(rootNode, centerPath);
		}
	}
	else{
		FindHighestDownPath(rootNode, centerPath);
	}

	// get up-zone, mix-zone and down-zone starting index
	int upStart = 1;
	int mixStart = -1;
	int downStart = -1;
	for (int i = 1; i < centerPath.size() - 1; i++){
		long brunchNode = GetBrunchNode(centerPath[i], centerPath[i - 1], centerPath[i + 1]);
		if (brunchNode){
			// mix-zone starts at first down-arc
			if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value) > 0){
				mixStart = i;
				break;
			}
		}
	}

	for (int i = centerPath.size() - 2; i > 0; i--){
		long brunchNode = GetBrunchNode(centerPath[i], centerPath[i - 1], centerPath[i + 1]);
		if (brunchNode){
			// down-zone starts when all rest are down-arc
			if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value) < 0){
				downStart = i+1;
				break;
			}
		}
	}

	long leftCount = 0;
	long rightCount = 0;
	bool isLeft = false;
	for (unsigned int i = 1; i < centerPath.size()-1; i++){
		long brunchNode = GetBrunchNode(centerPath[i], centerPath[i - 1], centerPath[i + 1]);
		if (brunchNode){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (isLeft){
					leftCount += supernodesExt[brunchNode].numLeaves;
				}
				else{
					rightCount += supernodesExt[brunchNode].numLeaves;
				}
				isLeft = !isLeft;
			}
			else if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value)<0){
					//leftCount += GetNumLeaves(brunchNode, centerPath[i]);
					leftCount += supernodesExt[brunchNode].numLeaves;
				}
				else{
					//rightCount += GetNumLeaves(brunchNode, centerPath[i]);
					rightCount += supernodesExt[brunchNode].numLeaves;
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (isLeft){
					leftCount += supernodesExt[brunchNode].numLeaves;
				}
				else{
					rightCount += supernodesExt[brunchNode].numLeaves;
				}
				isLeft = !isLeft;
			}
		}
	}

	// layout center path
	float intv = (xEnd - xStart) / (leftCount+rightCount+1);
	float centerLine = xStart + (leftCount + 0.5)*intv;
	for (unsigned int i = 0; i < centerPath.size(); i++){
		long thisNode = centerPath[i];
		supernodes[thisNode].xPosn = centerLine;
	}

	// layout brunches
	float leftRightBound = centerLine - 0.5*intv;
	float rightLeftBound = centerLine + 0.5*intv;
	float rightRightBound = xEnd;
	isLeft = false;
	for (unsigned int i = 1; i < centerPath.size() - 1; i++){
		long brunchNode = GetBrunchNode(centerPath[i], centerPath[i - 1], centerPath[i + 1]);
		if (brunchNode){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (isLeft){
					int numLeaves = supernodesExt[brunchNode].numLeaves;
					LayoutSubTree(brunchNode, centerPath[i], leftRightBound - numLeaves*intv, leftRightBound);
					leftRightBound -= numLeaves*intv;
				}
				else{
					int numLeaves = supernodesExt[brunchNode].numLeaves;
					LayoutSubTree(brunchNode, centerPath[i], rightLeftBound, rightLeftBound + numLeaves*intv);
					rightLeftBound += numLeaves*intv;
				}
				isLeft = !isLeft;
			}
			else if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value)<0){
					int numLeaves = supernodesExt[brunchNode].numLeaves;
					LayoutSubTree(brunchNode, centerPath[i], leftRightBound - numLeaves*intv, leftRightBound);
					leftRightBound -= numLeaves*intv;
				}
				else{
					int numLeaves = supernodesExt[brunchNode].numLeaves;
					LayoutSubTree(brunchNode, centerPath[i], rightLeftBound, rightLeftBound + numLeaves*intv);
					rightLeftBound += numLeaves*intv;
					rightRightBound = rightLeftBound;
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (isLeft){
					int numLeaves = supernodesExt[brunchNode].numLeaves;
					LayoutSubTree(brunchNode, centerPath[i], leftRightBound - numLeaves*intv, leftRightBound);
					leftRightBound -= numLeaves*intv;
				}
				else{
					// layout from right end
					int numLeaves = supernodesExt[brunchNode].numLeaves;
					LayoutSubTree(brunchNode, centerPath[i], rightRightBound - numLeaves*intv, rightRightBound);
					//LayoutSubTree(brunchNode, centerPath[i], rightLeftBound, rightLeftBound + numLeaves*intv);
					rightRightBound -= numLeaves*intv;
				}
				isLeft = !isLeft;
			}
		}
	}
}

/*
void MyContourTree::LayoutSubTree2(long rootNode, long parentNode, float xStart, float xEnd){
	int numLeaves = supernodesExt[rootNode].numLeaves;
	vector<long> centerPath;
	//FindHighestPath(rootNode, parentNode, centerPath, supernodes[rootNode].value);
	if (parentNode >= 0){
		if (supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf()){
			supernodes[rootNode].xPosn = (xStart + xEnd) / 2;
			return;
		}
		if (compareHeight(supernodes[rootNode].value, supernodes[parentNode].value) > 0){
			FindHighestUpPath(rootNode, centerPath);
			reverse(centerPath.begin(),centerPath.end());
		}
		else{
			FindHighestDownPath(rootNode, centerPath);
		}
	}
	else{
		FindHighestDownPath(rootNode, centerPath);
	}
	// get up-zone, mix-zone and down-zone starting index
	int mixStart = centerPath.size();
	for (int i = 0; i < centerPath.size(); i++){
		long brunchNode = GetBrunchNode(centerPath[i], i > 0 ? centerPath[i - 1] : parentNode, i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode);
		if (brunchNode>0){
			// mix-zone starts at first down-arc
			if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value) > 0){
				mixStart = i;
				break;
			}
		}
	}
	int downStart = mixStart;
	for (int i = centerPath.size() - 1; i >= 0; i--){
		long brunchNode = GetBrunchNode(centerPath[i], i > 0 ? centerPath[i - 1] : parentNode, i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode);
		if (brunchNode>0){
			// down-zone starts when all rest are down-arc
			if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value) < 0){
				downStart = i + 1;
				break;
			}
		}
	}

	vector<float> leftUpperBottomFilled;
	vector<float> rightUpperBottomFilled;
	vector<float> leftDowmTopFilled;
	vector<float> rightDownTopFilled;
	vector<bool> isLeft(centerPath.size(), false);
	bool t_isLeft = false;
	map<long, int> brunchSlotIdx;
	// decide left and right
	for (int i = 0; i < centerPath.size(); i++){
		long brunchNode = GetBrunchNode(centerPath[i], i > 0 ? centerPath[i - 1] : parentNode, i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode);
		if (brunchNode>0){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (t_isLeft){
					isLeft[i] = true;
				}
				else{
					isLeft[i] = false;
				}
				t_isLeft = !t_isLeft;
			}
			else if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value)<0){
					isLeft[i] = true;
				}
				else{
					isLeft[i] = false;
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (t_isLeft){
					isLeft[i] = true;
				}
				else{
					isLeft[i] = false;
				}
				t_isLeft = !t_isLeft;
			}
		}
	}
	// upper slots
	// from high to low
	for (int i = 0; i < downStart; i++){
		long brunchNode = GetBrunchNode(centerPath[i], i > 0 ? centerPath[i - 1] : parentNode, i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode);
		if (brunchNode>0){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (isLeft[i]){
					brunchSlotIdx[brunchNode] = FillUpperSlots(leftUpperBottomFilled,
						*supernodes[centerPath[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						supernodesExt[brunchNode].numLeaves);
				}
				else{
					brunchSlotIdx[brunchNode] = FillUpperSlots(rightUpperBottomFilled,
						*supernodes[centerPath[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						supernodesExt[brunchNode].numLeaves);
				}
			}
			else {
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value)<0){
					brunchSlotIdx[brunchNode] = FillUpperSlots(leftUpperBottomFilled,
						*supernodes[centerPath[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						supernodesExt[brunchNode].numLeaves);
				}
			}
		}
	}
	// dowm slots
	// from low to high
	for (int i = centerPath.size() - 1; i >= mixStart; i--){
		long brunchNode = GetBrunchNode(centerPath[i], i > 0 ? centerPath[i - 1] : parentNode, i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode);
		if (brunchNode>0){
			if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[centerPath[i]].value, supernodes[brunchNode].value)<0){
				}
				else{
					brunchSlotIdx[brunchNode] = FillDownSlots(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[centerPath[i]].value,
						supernodesExt[brunchNode].numLeaves);
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (isLeft[i]){
					brunchSlotIdx[brunchNode] = FillDownSlots(leftDowmTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[centerPath[i]].value,
						supernodesExt[brunchNode].numLeaves);
				}
				else{
					brunchSlotIdx[brunchNode] = FillDownSlots(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[centerPath[i]].value,
						supernodesExt[brunchNode].numLeaves);
				}
			}
		}
	}

	// layout center path
	int leftWidth = max(leftUpperBottomFilled.size(), leftDowmTopFilled.size());
	int rightWidth = max(rightUpperBottomFilled.size(), rightDownTopFilled.size());
	float intv = (xEnd - xStart) / (leftWidth + rightWidth + 1);
	float centerLine = xStart + (leftWidth + 0.5)*intv;
	for (int i = 0; i < centerPath.size(); i++){
		long thisNode = centerPath[i];
		supernodes[thisNode].xPosn = centerLine;
	}

	// layout brunches
	for (int i = 0; i < centerPath.size(); i++){
		long brunchNode = GetBrunchNode(centerPath[i], i > 0 ? centerPath[i - 1] : parentNode, i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode);
		if (brunchNode>0){
			if (isLeft[i]){
				int numLeaves = supernodesExt[brunchNode].numLeaves;
				LayoutSubTree2(brunchNode, centerPath[i],
					centerLine - (numLeaves + 0.5 + brunchSlotIdx[brunchNode])*intv,
					centerLine - (0.5 + brunchSlotIdx[brunchNode])*intv);
			}
			else{
				int numLeaves = supernodesExt[brunchNode].numLeaves;
				LayoutSubTree2(brunchNode, centerPath[i],
					centerLine + (0.5 + brunchSlotIdx[brunchNode])*intv,
					centerLine + (numLeaves + 0.5 + brunchSlotIdx[brunchNode])*intv);
			}
		}
	}
}
*/

void MyContourTree::LayoutSubTree2(long rootNode, long parentNode, float xStart, float xEnd){
	int numLeaves = supernodesExt[rootNode].numLeaves;
	vector<long> centerPath;
	//FindHighestPath(rootNode, parentNode, centerPath, supernodes[rootNode].value);
	if (parentNode >= 0){
		if (supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf()){
			supernodes[rootNode].xPosn = (xStart + xEnd) / 2;
			return;
		}
		if (compareHeight(supernodes[rootNode].value, supernodes[parentNode].value) > 0){
			FindHighestUpPath(rootNode, centerPath);
			reverse(centerPath.begin(), centerPath.end());
		}
		else{
			FindHighestDownPath(rootNode, centerPath);
		}
	}
	else{
		FindHighestDownPath(rootNode, centerPath);
	}

	vector<long> rootNodes;
	vector<long> parentNodes;
	vector<long> childNodes;
	vector<long> brunchNodes;
	for (int i = 0; i < centerPath.size(); i++){
		long thisParentNode = i > 0 ? centerPath[i - 1] : parentNode;
		long childNode = i < centerPath.size() - 1 ? centerPath[i + 1] : parentNode;
		vector<long> neighbors;
		GetNeighbors(centerPath[i], neighbors);
		for (int j = 0; j < neighbors.size(); j++){
			if (neighbors[j] != thisParentNode && neighbors[j] != childNode){
				rootNodes.push_back(centerPath[i]);
				parentNodes.push_back(thisParentNode);
				childNodes.push_back(childNode);
				brunchNodes.push_back(neighbors[j]);
			}
		}
	}

	// get up-zone, mix-zone and down-zone starting index
	int mixStart = brunchNodes.size();
	for (int i = 0; i < brunchNodes.size(); i++){
		long brunchNode = brunchNodes[i];
		if (brunchNode>0){
			// mix-zone starts at first down-arc
			if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value) > 0){
				mixStart = i;
				break;
			}
		}
	}
	int downStart = mixStart;
	for (int i = brunchNodes.size() - 1; i >= 0; i--){
		long brunchNode = brunchNodes[i];
		if (brunchNode>0){
			// down-zone starts when all rest are down-arc
			if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value) < 0){
				downStart = i + 1;
				break;
			}
		}
	}

	vector<float> leftUpperBottomFilled;
	vector<float> rightUpperBottomFilled;
	vector<float> leftDowmTopFilled;
	vector<float> rightDownTopFilled;
	vector<bool> isLeft(rootNodes.size(), false);

	bool t_isLeft = false;
	map<long, int> brunchSlotIdx;
	// decide left and right
	for (int i = 0; i < brunchNodes.size(); i++){
		long brunchNode = brunchNodes[i];
		if (brunchNode>0){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (t_isLeft){
					isLeft[i] = true;
				}
				else{
					isLeft[i] = false;
				}
				t_isLeft = !t_isLeft;
			}
			else if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
					isLeft[i] = true;
				}
				else{
					isLeft[i] = false;
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (t_isLeft){
					isLeft[i] = true;
				}
				else{
					isLeft[i] = false;
				}
				t_isLeft = !t_isLeft;
			}
		}
	}

	// overwrite
	for (int i = 0; i < brunchNodes.size(); i++){
		long brunchNode = brunchNodes[i];
		float *value = supernodes[brunchNode].value;
		long x, y, z;
		height.ComputeIndex(value, x, y, z);
		int label = mLabelVolume->get_at_index(x, y, z);
		if (label % 2==0){
			isLeft[i] = true;
		}
		else{
			isLeft[i] = false;
		}
	}
	// upper slots
	// from high to low
	for (int i = 0; i < downStart; i++){
		long brunchNode = brunchNodes[i];
		if (brunchNode>0){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (isLeft[i]){
					brunchSlotIdx[brunchNode] = FillUpperSlots(leftUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						supernodesExt[brunchNode].numLeaves);
				}
				else{
					brunchSlotIdx[brunchNode] = FillUpperSlots(rightUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						supernodesExt[brunchNode].numLeaves);
				}
			}
			else {
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
					brunchSlotIdx[brunchNode] = FillUpperSlots(leftUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						supernodesExt[brunchNode].numLeaves);
				}
			}
		}
	}
	// dowm slots
	// from low to high
	for (int i = brunchNodes.size() - 1; i >= mixStart; i--){
		long brunchNode = brunchNodes[i];
		if (brunchNode>0){
			if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
				}
				else{
					brunchSlotIdx[brunchNode] = FillDownSlots(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						supernodesExt[brunchNode].numLeaves);
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (isLeft[i]){
					brunchSlotIdx[brunchNode] = FillDownSlots(leftDowmTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						supernodesExt[brunchNode].numLeaves);
				}
				else{
					brunchSlotIdx[brunchNode] = FillDownSlots(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						supernodesExt[brunchNode].numLeaves);
				}
			}
		}
	}

	// layout center path
	int leftWidth = max(leftUpperBottomFilled.size(), leftDowmTopFilled.size());
	int rightWidth = max(rightUpperBottomFilled.size(), rightDownTopFilled.size());
	float intv = (xEnd - xStart) / (leftWidth + rightWidth + 1);
	float centerLine = xStart + (leftWidth + 0.5)*intv;
	for (int i = 0; i < centerPath.size(); i++){
		long thisNode = centerPath[i];
		supernodes[thisNode].xPosn = centerLine;
	}

	// layout brunches
	for (int i = 0; i < brunchNodes.size(); i++){
		long brunchNode = brunchNodes[i];
		if (brunchNode>0){
			if (isLeft[i]){
				int numLeaves = supernodesExt[brunchNode].numLeaves;
				LayoutSubTree2(brunchNode, rootNodes[i],
					centerLine - (numLeaves + 0.5 + brunchSlotIdx[brunchNode])*intv,
					centerLine - (0.5 + brunchSlotIdx[brunchNode])*intv);
			}
			else{
				int numLeaves = supernodesExt[brunchNode].numLeaves;
				LayoutSubTree2(brunchNode, rootNodes[i],
					centerLine + (0.5 + brunchSlotIdx[brunchNode])*intv,
					centerLine + (numLeaves + 0.5 + brunchSlotIdx[brunchNode])*intv);
			}
		}
	}
}

int MyContourTree::FillUpperSlots(vector<float>& upperBottomFilled, float minHeight, float maxHeight, int numLeaves){
	// use only maxHeight
	for (int i = 0; i < upperBottomFilled.size(); i++){
		bool usable = true;
		for (int j = i; j < upperBottomFilled.size() && j < i + numLeaves; j++){
			if (upperBottomFilled[j] < maxHeight){
				usable = false;
				break;
			}
		}
		if (usable){
			for (int j = 0; j < i + numLeaves; j++){
				if (j < upperBottomFilled.size()){
					upperBottomFilled[j] = minHeight;
				}
				else{
					upperBottomFilled.push_back(minHeight);
				}
			}
			return i;
		}
	}
	for (int i = 0; i < upperBottomFilled.size(); i++){
			upperBottomFilled[i] = minHeight;
	}
	for (int j = 0; j < numLeaves; j++){
		upperBottomFilled.push_back(minHeight);
	}
	return upperBottomFilled.size() - numLeaves;
}

int MyContourTree::FillDownSlots(vector<float>& dowmTopFilled, float minHeight, float maxHeight, int numLeaves){
	// use only minHeight
	for (int i = 0; i < dowmTopFilled.size(); i++){
		bool usable = true;
		for (int j = i; j < dowmTopFilled.size() && j < i + numLeaves; j++){
			if (dowmTopFilled[j] > minHeight){
				usable = false;
				break;
			}
		}
		if (usable){
			for (int j = 0; j < i + numLeaves; j++){
				if (j < dowmTopFilled.size()){
					dowmTopFilled[j] = maxHeight;
				}
				else{
					dowmTopFilled.push_back(maxHeight);
				}
			}
			return i;
		}
	}
	for (int i = 0; i < dowmTopFilled.size(); i++){
		dowmTopFilled[i] = maxHeight;
	}
	for (int j = 0; j < numLeaves; j++){
		dowmTopFilled.push_back(maxHeight);
	}
	return dowmTopFilled.size() - numLeaves;
}

/*
long MyContourTree::GetNumLeaves(long rootNode, long parentNode){

	if ((supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf())
		&& parentNode >=0){
		supernodesExt[rootNode].parent = parentNode;
		supernodesExt[rootNode].numLeaves = 1;
		return 1;
	}
	int numLeaves = 0;
	// up-leaves
	if (supernodes[rootNode].upDegree > 0){
		long theArc = supernodes[rootNode].upList;
		do{
			long otherNode = superarcs[theArc].topID;
			if (otherNode != parentNode){
				numLeaves += GetNumLeaves(otherNode, rootNode);
			}
			theArc = superarcs[theArc].nextUp;
		} while (theArc != supernodes[rootNode].upList);
	}
	// down-leaves
	if (supernodes[rootNode].downDegree > 0){
		long theArc = supernodes[rootNode].downList;
		do{
			long otherNode = superarcs[theArc].bottomID;
			if (otherNode != parentNode){
				numLeaves += GetNumLeaves(otherNode, rootNode);
			}
			theArc = superarcs[theArc].nextDown;
		} while (theArc != supernodes[rootNode].downList);
	}
	supernodesExt[rootNode].parent = parentNode;
	supernodesExt[rootNode].numLeaves = numLeaves;
	return numLeaves;
}
*/

long MyContourTree::GetNumLeaves(long rootNode, long parentNode){
	list<long> nodes_visited;
	list<long> nodes_visited_parent;
	list<long> nodes_to_visit;
	list<long> nodes_to_visit_parent;
	nodes_visited.push_front(rootNode);
	nodes_to_visit.push_front(rootNode);
	nodes_to_visit_parent.push_front(parentNode);
	nodes_visited_parent.push_front(parentNode);
	while (!nodes_visited.empty()){
		// width first search
		long thisNode = nodes_visited.front();
		nodes_visited.pop_front();
		long thisNodeParent = nodes_visited_parent.front();
		nodes_visited_parent.pop_front();
		vector<long> neighbors;
		GetNeighbors(thisNode, neighbors);
		for (int i = 0; i < neighbors.size(); i++){
			if (neighbors[i] != thisNodeParent){
				nodes_visited.push_front(neighbors[i]);
				nodes_visited_parent.push_front(thisNode);
				nodes_to_visit.push_front(neighbors[i]);
				nodes_to_visit_parent.push_front(thisNode);
			}
		}
	}
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		long thisNodeParent = nodes_to_visit_parent.front();
		if ((supernodes[thisNode].IsLowerLeaf() || supernodes[thisNode].IsUpperLeaf())
			&& thisNodeParent >= 0){
			supernodesExt[thisNode].numLeaves = 1;
		}
		else{
			supernodesExt[thisNode].numLeaves = 0;
			vector<long> neighbors;
			GetNeighbors(thisNode, neighbors);
			for (int i = 0; i < neighbors.size(); i++){
				if (neighbors[i] != thisNodeParent){
					if (supernodesExt[neighbors[i]].numLeaves <= 0){
						assert(0);
					}
					supernodesExt[thisNode].numLeaves += supernodesExt[neighbors[i]].numLeaves;
				}
			}
		}
		supernodesExt[thisNode].parent = thisNodeParent;
		nodes_to_visit.pop_front();
		nodes_to_visit_parent.pop_front();
	}

	return supernodesExt[rootNode].numLeaves;
}


long MyContourTree::GetBrunchNode(long rootNode, long parentNode, long childNode){
	list<long> neighbors;
	GetNeighbors(rootNode, neighbors);
	for (list<long>::iterator neighbor = neighbors.begin();
		neighbor != neighbors.end();
		neighbor++){
		if (*neighbor != parentNode && *neighbor != childNode){
			return *neighbor;
		}
	}
	return -1;
}

void MyContourTree::GetNeighbors(long rootNode, std::list<long>& neighbors){
	// up
	if (supernodes[rootNode].upDegree > 0){
		long theArc = supernodes[rootNode].upList;
		for (int i = 0; i < supernodes[rootNode].upDegree; i++){
			long otherNode = superarcs[theArc].topID;
			neighbors.push_back(otherNode);
			theArc = superarcs[theArc].nextUp;
		}
	}
	// down
	if (supernodes[rootNode].downDegree > 0){
		long theArc = supernodes[rootNode].downList;
		for (int i = 0; i < supernodes[rootNode].downDegree; i++){
			long otherNode = superarcs[theArc].bottomID;
			neighbors.push_back(otherNode);
			theArc = superarcs[theArc].nextDown;
		}
	}
}

void MyContourTree::GetNeighbors(long rootNode, std::vector<long>& neighbors){
	// up
	if (supernodes[rootNode].upDegree > 0){
		long theArc = supernodes[rootNode].upList;
		for (int i = 0; i < supernodes[rootNode].upDegree; i++){
			long otherNode = superarcs[theArc].topID;
			neighbors.push_back(otherNode);
			theArc = superarcs[theArc].nextUp;
		}
	}
	// down
	if (supernodes[rootNode].downDegree > 0){
		long theArc = supernodes[rootNode].downList;
		for (int i = 0; i < supernodes[rootNode].downDegree; i++){
			long otherNode = superarcs[theArc].bottomID;
			neighbors.push_back(otherNode);
			theArc = superarcs[theArc].nextDown;
		}
	}
}

long MyContourTree::FindMonotoneDownNode(long rootNode){
	long targetNode = rootNode;
	list<long> nodes_to_visit;
	nodes_to_visit.push_front(rootNode);
	std::vector<long> tmpPath;
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		tmpPath.push_back(thisNode);
		nodes_to_visit.pop_front();
		// push all down nodes into the list
		long theArc = supernodes[thisNode].downList;
		for (long i = 0; i < supernodes[thisNode].downDegree; i++){
			long theNode = superarcs[theArc].bottomID;
			nodes_to_visit.push_front(theNode);
			theArc = superarcs[theArc].nextDown;
		}
		if (supernodes[thisNode].downDegree == 0){
			if (compareHeight(supernodes[thisNode].value, supernodes[targetNode].value) < 0){
				targetNode = thisNode;
			}
		}
	}
	return targetNode;
}


bool MyContourTree::FindMonotoneDownPath(long sourceNode, long targetNode, std::vector<long>& path){
	list<long> nodes_to_visit;
	list<long> nodes_to_visit_parents;
	nodes_to_visit.push_front(sourceNode);
	nodes_to_visit_parents.push_front(-1);
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		nodes_to_visit.pop_front();
		nodes_to_visit_parents.pop_front();
		path.push_back(thisNode);

		// push all down nodes into the list
		long theArc = supernodes[thisNode].downList;
		for (long i = 0; i < supernodes[thisNode].downDegree; i++){
			long theNode = superarcs[theArc].bottomID;
			nodes_to_visit.push_front(theNode);
			nodes_to_visit_parents.push_front(thisNode);
			theArc = superarcs[theArc].nextDown;
		}

		// check find
		if (thisNode == targetNode){
			return true;
		}

		// path retract
		while (path.back() != nodes_to_visit_parents.front()){
			path.pop_back();
		}
	}
	return false;
}


long MyContourTree::FindMonotoneUpNode(long rootNode){
	long targetNode = rootNode;
	list<long> nodes_to_visit;
	nodes_to_visit.push_front(rootNode);
	std::vector<long> tmpPath;
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		tmpPath.push_back(thisNode);
		nodes_to_visit.pop_front();
		// push all down nodes into the list
		long theArc = supernodes[thisNode].upList;
		for (long i = 0; i < supernodes[thisNode].upDegree; i++){
			long theNode = superarcs[theArc].topID;
			nodes_to_visit.push_front(theNode);
			theArc = superarcs[theArc].nextUp;
		}
		if (supernodes[thisNode].upDegree == 0){
			if (compareHeight(supernodes[thisNode].value, supernodes[targetNode].value) > 0){
				targetNode = thisNode;
			}
		}
	}
	return targetNode;
}


bool MyContourTree::FindMonotoneUpPath(long sourceNode, long targetNode, std::vector<long>& path){
	list<long> nodes_to_visit;
	list<long> nodes_to_visit_parents;
	nodes_to_visit.push_front(sourceNode);
	nodes_to_visit_parents.push_front(-1);
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		nodes_to_visit.pop_front();
		nodes_to_visit_parents.pop_front();
		path.push_back(thisNode);

		// push all down nodes into the list
		long theArc = supernodes[thisNode].upList;
		for (long i = 0; i < supernodes[thisNode].upDegree; i++){
			long theNode = superarcs[theArc].topID;
			nodes_to_visit.push_front(theNode);
			nodes_to_visit_parents.push_front(thisNode);
			theArc = superarcs[theArc].nextUp;
		}

		// check find
		if (thisNode == targetNode){
			return true;
		}

		// path retract
		while (path.back() != nodes_to_visit_parents.front()){
			path.pop_back();
		}
	}
	return false;
}

void MyContourTree::FindHeightRange(long rootNode, long parentNode, float*&minHeight, float*&maxHeight){
	list<long> nodes_to_visit;
	list<long> nodes_to_visit_parents;
	nodes_to_visit.push_front(rootNode);
	nodes_to_visit_parents.push_front(parentNode);
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		long thisNodeParent = nodes_to_visit_parents.front();
		nodes_to_visit.pop_front();
		nodes_to_visit_parents.pop_front();
		vector<long> neighbors;
		GetNeighbors(thisNode, neighbors);
		for (int i = 0; i < neighbors.size(); i++){
			long neighbor = neighbors[i];
			if (neighbor != thisNodeParent){
				nodes_to_visit.push_front(neighbor);
				nodes_to_visit_parents.push_front(thisNode);
			}
		}
		float* value = supernodes[thisNode].value;
		if (compareHeight(value, minHeight) < 0){
			minHeight = value;
		}
		if (compareHeight(value, maxHeight) > 0){
			maxHeight = value;
		}
	}
}

int MyContourTree::FindHeightRangeAndLeaves(long rootNode, long parentNode){
	list<long> nodes_visited;
	list<long> nodes_visited_parent;
	list<long> nodes_to_visit;
	list<long> nodes_to_visit_parent;
	nodes_visited.push_front(rootNode);
	nodes_to_visit.push_front(rootNode);
	nodes_to_visit_parent.push_front(parentNode);
	nodes_visited_parent.push_front(parentNode);
	while (!nodes_visited.empty()){
		// width first search
		long thisNode = nodes_visited.front();
		nodes_visited.pop_front();
		long thisNodeParent = nodes_visited_parent.front();
		nodes_visited_parent.pop_front();
		vector<long> neighbors;
		GetNeighbors(thisNode, neighbors);
		for (int i = 0; i < neighbors.size(); i++){
			if (neighbors[i] != thisNodeParent){
				nodes_visited.push_front(neighbors[i]);
				nodes_visited_parent.push_front(thisNode);
				nodes_to_visit.push_front(neighbors[i]);
				nodes_to_visit_parent.push_front(thisNode);
			}
		}
	}
	int numNodes = nodes_to_visit.size();
	while (!nodes_to_visit.empty()){
		long thisNode = nodes_to_visit.front();
		long thisNodeParent = nodes_to_visit_parent.front();
		supernodesExt[thisNode].minHeight = supernodes[thisNode].value;
		supernodesExt[thisNode].maxHeight = supernodes[thisNode].value;
		if ((supernodes[thisNode].IsLowerLeaf() || supernodes[thisNode].IsUpperLeaf())
			&& thisNodeParent >= 0){
			supernodesExt[thisNode].numLeaves = 1;
		}
		else{
			supernodesExt[thisNode].numLeaves = 0;
			vector<long> neighbors;
			GetNeighbors(thisNode, neighbors);
			for (int i = 0; i < neighbors.size(); i++){
				if (neighbors[i] != thisNodeParent){
					if (supernodesExt[neighbors[i]].numLeaves <= 0){
						assert(0);
					}
					supernodesExt[thisNode].numLeaves += supernodesExt[neighbors[i]].numLeaves;
					if (supernodesExt[neighbors[i]].maxHeight == 0 || supernodesExt[neighbors[i]].minHeight == 0){
						assert(0);
					}
					if (compareHeight(supernodesExt[thisNode].maxHeight, supernodesExt[neighbors[i]].maxHeight) < 0){
						supernodesExt[thisNode].maxHeight = supernodesExt[neighbors[i]].maxHeight;
					}
					if (compareHeight(supernodesExt[thisNode].minHeight, supernodesExt[neighbors[i]].minHeight) > 0){
						supernodesExt[thisNode].minHeight = supernodesExt[neighbors[i]].minHeight;
					}
				}
			}
		}
		supernodesExt[thisNode].parent = thisNodeParent;
		nodes_to_visit.pop_front();
		nodes_to_visit_parent.pop_front();
	}
	return numNodes;
}

/*
void MyContourTree::DrawArcHistogram(long arc){
	long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;
	float xPos;
	if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
		xPos = supernodes[topNode].xPosn;
	}
	else{
		xPos = supernodes[bottomNode].xPosn;
	}
	vector<float*>& nodes = mArcNodes[arc];
	if (!nodes.empty()){
		float minValue = *supernodes[superarcs[arc].bottomID].value;
		float maxValue = *supernodes[superarcs[arc].topID].value;
		int numBins = sqrt(nodes.size())+1;
		float binSize = (maxValue - minValue) / numBins;
		vector<int> count(numBins,0);
		for (int i = 0; i < nodes.size(); i++){
			float value = *nodes[i];
			if (value>maxValue || value < minValue){
				cout << "Arc [" << arc << "] FA value out of nodes' bound.\n";
			}
			int bin = (value - minValue) / binSize;
			if (bin >= numBins) bin = numBins - 1;
			count[bin]++;
		}

		float perCountSize = 0.01 / (*(max_element(count.begin(), count.end())));
		float yStart = min(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
		float yEnd = max(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
		float yStep = (yEnd - yStart) / numBins;
		for (int i = 0; i < count.size(); i++){
			float height = count[i]*perCountSize;

			glColor3f(0, 0, 0);
			glBegin(GL_LINES);
			glVertex2f(xPos + height, yStart + i*yStep);
			glVertex2f(xPos + height, yStart + (i + 1)*yStep);

			glVertex2f(xPos - height, yStart + i*yStep);
			glVertex2f(xPos - height, yStart + (i + 1)*yStep);

			if (i == 0){
				glVertex2f(xPos + height, yStart + i*yStep);
				glVertex2f(xPos - height, yStart + i*yStep);
			}
			if (i == count.size() - 1){
				glVertex2f(xPos + height, yStart + (i + 1)*yStep);
				glVertex2f(xPos - height, yStart + (i + 1)*yStep);
			}
			else{
				float heightNext = count[i + 1]*perCountSize;
				glVertex2f(xPos + height, yStart + (i + 1)*yStep);
				glVertex2f(xPos + heightNext, yStart + (i + 1)*yStep);

				glVertex2f(xPos - height, yStart + (i + 1)*yStep);
				glVertex2f(xPos - heightNext, yStart + (i + 1)*yStep);
			}
			glEnd();

			MyColor4f color1, color2;
			float posY;
			glColor3f(0.2, 0.2, 0.2);
			glBegin(GL_QUADS);

			posY = yStart + i*yStep;
			color1 = colorMap->GetColor(posY*colorMap->GetWidth(), 0);
			glColor3f(color1.b, color1.g, color1.r);
			glVertex2f(xPos + height, yStart + i*yStep);

			posY = yStart + (i + 1)*yStep;
			color2 = colorMap->GetColor(posY*colorMap->GetWidth(), 0);
			glColor3f(color2.b, color2.g, color2.r);
			glVertex2f(xPos + height, yStart + (i + 1)*yStep);
			glVertex2f(xPos - height, yStart + (i + 1)*yStep);

			glColor3f(color1.b, color1.g, color1.r);
			glVertex2f(xPos - height, yStart + i*yStep);
			glEnd();
		}
	}
}
*/


void MyContourTree::GetArcBottomPos(long arc, float& x, float& y){
	long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;
	if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
		x = supernodes[topNode].xPosn;
	}
	else{
		x = supernodes[bottomNode].xPosn;
	}

	y = min(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
}

void MyContourTree::DrawArcHistogramAt(long arc, float x, float y, float scaleX, float scaleY){
	float histx, histy;
	GetArcBottomPos(arc, histx, histy);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(scaleX, scaleY, 1);
	glTranslatef(-histx, -histy, 0);
	if (GetArcScale(arc) == MappingScale_Sci){
		DrawArcHistogramScientific(arc);
	}
	else{
		DrawArcHistogram(arc);
	}
	glPopMatrix();
}

void MyContourTree::DrawArcSnappingPoint(long arc){
	map<long, char>::const_iterator it = mArcStatus.find(arc);
	if (it != mArcStatus.end()){
		char arcStatus = it->second;
		if (arcStatus & ArcStatus_SnapAnchoring){
			float anchorX, anchorY;
			GetArcSnapPosition(arc, anchorX, anchorY);
			glPointSize(30);
			glBegin(GL_POINTS);
			glColor4f(0, 1, 0, 1);
			glVertex2f(anchorX, anchorY);
			glEnd();
		}
		else if (arcStatus & ArcStatus_InSnapping){
			float anchorX, anchorY;
			GetArcSnapPosition(arc, anchorX, anchorY);
			glPointSize(20);
			glBegin(GL_POINTS);
			glColor4f(1, 0, 0, 1);
			glVertex2f(anchorX, anchorY);
			glEnd();
		}
	}
}

void MyContourTree::DrawArcHistogram(long arc){
	vector<float>& histogram = mArcHistogram[arc];
	if (histogram.empty()) return;
	long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;
	float xPos;
	if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
		xPos = supernodes[topNode].xPosn;
	}
	else{
		xPos = supernodes[bottomNode].xPosn;
	}

	float yStart = min(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
	float yEnd = max(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
	float yStep = (yEnd - yStart) / histogram.size();

	MappingScale arcScale = GetArcScale(arc);
	float arcZoom = GetArcZoomLevel(arc);
	if (IsArcCompared(arc)){
		glLineWidth(2);
	}
	else{
		glLineWidth(1);
	}

	if (IsArcAggregated(arc)){
		glLineWidth(4);
	}

	float leftHeightScale;
	float rightHeightScale;
	switch (mHistogramSide){
	case HistogramSide_Sym:
		leftHeightScale = 0.5;
		rightHeightScale = 0.5;
		break;
	case HistogramSide_Left:
		leftHeightScale = 1;
		rightHeightScale = 0;
		break;
	case HistogramSide_Right:
		leftHeightScale = 0;
		rightHeightScale = 1;
		break;
	}
	glDepthFunc(GL_ALWAYS);
	float maxBaseHeight = GetDrawingHeight(mMaxHistogramCount, mDefaultScale);
	for (int i = 0; i < histogram.size(); i++){
		float baseHeight = GetDrawingHeight(histogram[i], arcScale);
		float leftHeight = baseHeight * leftHeightScale*arcZoom;
		float rightHeight = baseHeight * rightHeightScale*arcZoom;
		glBegin(GL_QUADS);
		//float color = 1 - baseHeight / maxBaseHeight;
		//float color = 0.5;
		//glColor4f(color, color, color, mContourTreeAlpha);
		int color = GetDrawingHeight(histogram[i], mDefaultScale)
			/ GetDrawingHeight(mMaxHistogramCount, mDefaultScale) * 7+0.5;
		//float color = 0.5;
		glColor4f(colorBrewer_sequential_8_multihue_9[color][0] / 255.f,
			colorBrewer_sequential_8_multihue_9[color][1] / 255.f,
			colorBrewer_sequential_8_multihue_9[color][2] / 255.f, mContourTreeAlpha);
		glVertex2f(xPos - leftHeight, yStart + i*yStep);
		glVertex2f(xPos + rightHeight, yStart + i*yStep);
		glVertex2f(xPos + rightHeight, yStart + (i + 1)*yStep);
		glVertex2f(xPos - leftHeight, yStart + (i + 1)*yStep);
		glEnd();

		glColor4f(0, 0, 0, mContourTreeAlpha);
		glBegin(GL_LINES);
		if (i == 0){
			glVertex2f(xPos - leftHeight, yStart );
			glVertex2f(xPos + rightHeight, yStart);
		}
		if (i == histogram.size() - 1){
			glVertex2f(xPos - leftHeight, yEnd);
			glVertex2f(xPos + rightHeight, yEnd);
		}
		else{
			float baseHeightNext = GetDrawingHeight(histogram[i+1], arcScale);
			float leftHeightNext = baseHeightNext * leftHeightScale*arcZoom;
			float rightHeightNext = baseHeightNext * rightHeightScale*arcZoom;
			glVertex2f(xPos - leftHeight, yStart + (i + 1)*yStep);
			glVertex2f(xPos - leftHeightNext, yStart + (i + 1)*yStep);
			glVertex2f(xPos + rightHeight, yStart + (i + 1)*yStep);
			glVertex2f(xPos + rightHeightNext, yStart + (i + 1)*yStep);
		}
		glVertex2f(xPos - leftHeight, yStart + i*yStep);
		glVertex2f(xPos - leftHeight, yStart + (i + 1)*yStep);
		glVertex2f(xPos + rightHeight, yStart + i*yStep);
		glVertex2f(xPos + rightHeight, yStart + (i + 1)*yStep);
		glEnd();
	}
	glDepthFunc(GL_LESS);
}

void MyContourTree::DrawArcHistogramScientific(long arc){
	vector<float>& histogram = mArcHistogram[arc];
	if (histogram.empty()) return;
	long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;
	float xPos;
	if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
		xPos = supernodes[topNode].xPosn;
	}
	else{
		xPos = supernodes[bottomNode].xPosn;
	}

	float yStart = min(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
	float yEnd = max(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
	float yStep = (yEnd - yStart) / histogram.size();

	MappingScale arcScale = GetArcScale(arc);
	float arcZoom = GetArcZoomLevel(arc);
	if (IsArcCompared(arc)){
		glLineWidth(2);
	}
	else{
		glLineWidth(1);
	}
	if (IsArcAggregated(arc)){
		glLineWidth(4);
	}


	float leftHeightScale;
	float rightHeightScale;
	switch (mHistogramSide){
	case HistogramSide_Sym:
		leftHeightScale = 0.5;
		rightHeightScale = 0.5;
		break;
	case HistogramSide_Left:
		leftHeightScale = 1;
		rightHeightScale = 0;
		break;
	case HistogramSide_Right:
		leftHeightScale = 0;
		rightHeightScale = 1;
		break;
	}
	vector<float> exponentHeights(histogram.size());
	vector<float> mantissaHeights(histogram.size());
	vector<int> exponentPos(histogram.size());
	float exponentRange = mMaxExponent - mMinExponent;
	for (int i = 0; i < histogram.size(); i++){
		float exponentHeight;
		float mantissaHeight;
		GetDrawingHeightScientific(histogram[i], exponentHeight, mantissaHeight);
		exponentHeights[i] = exponentHeight*arcZoom;
		mantissaHeights[i] = mantissaHeight*arcZoom;
		if (exponentRange != 0){
			int exponent;
			float mantissa;
			floatToScientific(histogram[i], exponent, mantissa);
			exponentPos[i] = (exponent - mMinExponent)/exponentRange*7+0.5;
		}
		else{
			exponentPos[i] = 0;
		}
	}

	glDepthFunc(GL_ALWAYS);
	for (int i = 0; i < histogram.size(); i++){
		float expHeight = exponentHeights[i];
		float expHeightNext = (i == histogram.size() - 1 ? exponentHeights[i] : exponentHeights[i + 1]);
		float posY = yStart + i*yStep;

		glBegin(GL_QUADS);
		//glColor3ubv((const GLubyte*)colorBrewer_sequential_8_multihue_9[exponentPos[i]]);
		glColor4f(colorBrewer_sequential_8_multihue_9[exponentPos[i]][0] / 255.f,
			colorBrewer_sequential_8_multihue_9[exponentPos[i]][1] / 255.f,
			colorBrewer_sequential_8_multihue_9[exponentPos[i]][2] / 255.f, mContourTreeAlpha);
		glVertex2f(xPos - expHeight*leftHeightScale, yStart + i*yStep);
		glVertex2f(xPos + expHeight*rightHeightScale, yStart + i*yStep);
		glVertex2f(xPos + expHeightNext*rightHeightScale, yStart + (i + 1)*yStep);
		glVertex2f(xPos - expHeightNext*leftHeightScale, yStart + (i + 1)*yStep);
		glEnd();


		glBegin(GL_LINES);
		// exponent part
		glColor4f(0, 0, 0, mContourTreeAlpha);
		glVertex2f(xPos + expHeight*rightHeightScale, yStart + i*yStep);
		glVertex2f(xPos + expHeightNext*rightHeightScale, yStart + (i + 1)*yStep);
		glVertex2f(xPos - expHeight*leftHeightScale, yStart + i*yStep);
		glVertex2f(xPos - expHeightNext*leftHeightScale, yStart + (i + 1)*yStep);
		if (i == 0){
			glVertex2f(xPos + expHeight*rightHeightScale, yStart);
			glVertex2f(xPos - expHeightNext*leftHeightScale, yStart);
		}
		if (i == histogram.size() - 1){
			glVertex2f(xPos + expHeight*rightHeightScale, yEnd);
			glVertex2f(xPos - expHeightNext*leftHeightScale, yEnd);
		}

		// mantissa part
		float mantissaHeight = mantissaHeights[i];
		float mantHeightNext = (i == histogram.size() - 1 ? mantissaHeights[i] : mantissaHeights[i + 1]);
		glColor4f(0, 0, 1, mContourTreeAlpha);
		glVertex2f(xPos + mantissaHeight*rightHeightScale, yStart + i*yStep);
		glVertex2f(xPos + mantHeightNext*rightHeightScale, yStart + (i + 1)*yStep);
		glVertex2f(xPos - mantissaHeight*leftHeightScale, yStart + i*yStep);
		glVertex2f(xPos - mantHeightNext*leftHeightScale, yStart + (i + 1)*yStep);

		if (i == 0){
			glVertex2f(xPos + mantissaHeight*rightHeightScale, yStart);
			glVertex2f(xPos - mantissaHeight*leftHeightScale, yStart);
		}
		if (i == histogram.size() - 1){
			glVertex2f(xPos + mantissaHeight*rightHeightScale, yEnd);
			glVertex2f(xPos - mantissaHeight*leftHeightScale, yEnd);
		}
		glEnd();
	}
	glDepthFunc(GL_LESS);
}

void MyContourTree::DrawContourTreeFrame(){
	float boarder_x = 0.035;
	glColor4f(0, 0, 0, mContourTreeAlpha);
	glBegin(GL_LINE_LOOP);
	glVertex2f(-boarder_x, 0);
	glVertex2f(1 + boarder_x, 0);
	glVertex2f(1 + boarder_x, 1);
	glVertex2f(-boarder_x, 1);
	glEnd();

	glBegin(GL_LINES);
	for (int i = 1; i <= 49; i++){
		float height = i*0.02;
		if (i % 5 == 0){
			glColor4f(0.5, 0.5, 0.5, mContourTreeAlpha);
			glLineWidth(2);
		}
		else{
			glColor4f(0.8, 0.8, 0.8, mContourTreeAlpha);
			glLineWidth(0.5);
		}
		glVertex2f(-boarder_x, height);
		glVertex2f(1 + boarder_x, height);
	}
	glEnd();

	glColor4f(0, 0, 0, mContourTreeAlpha);
	for (int i = 0; i <= 10; i++){
		glRasterPos2f(-boarder_x-0.015, i*0.1);
		string str = to_string(i*0.1);
		str.resize(3);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)str.c_str());
	}
}

void MyContourTree::DrawArcLabels(){
	float cutSize = 0.005;																//	relative length of cuts
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float pixelWidth = 1.f / viewport[2];
	for (long whichArc = 0; whichArc < nValidArcs; whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		long arc = valid[whichArc];															//	grab an edge from the list
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends

		// draw label
		if (mLabelVolume){
			string name = mArcName[arc];
			if (supernodes[topNode].IsLowerLeaf() || supernodes[topNode].IsUpperLeaf()){
				float length = 0;
				for (int i = 0; i < name.size(); i++){
					length += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, name[i]);
				}
				glColor4f(0, 0, 0, mContourTreeAlpha);
				glRasterPos2f(supernodes[topNode].xPosn - length / 2 * pixelWidth, supernodes[topNode].yPosn + cutSize);
				glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)name.c_str());
			}
			else if (supernodes[bottomNode].IsLowerLeaf() || supernodes[bottomNode].IsUpperLeaf()){
				float length = 0;
				for (int i = 0; i < name.size(); i++){
					length += glutBitmapWidth(GLUT_BITMAP_TIMES_ROMAN_24, name[i]);
				}
				glColor4f(0, 0, 0, mContourTreeAlpha);
				glRasterPos2f(supernodes[bottomNode].xPosn - length / 2 * pixelWidth, supernodes[bottomNode].yPosn + cutSize);
				glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)name.c_str());
			}
			else{
				//glRasterPos2f(supernodes[bottomNode].xPosn + cutSize, 
				//	(supernodes[bottomNode].yPosn + supernodes[topNode].yPosn)/2);
			}
		}
	} // loop through superarcs
	float pixelHeight = 1.f / (viewport[3] / 2);
	glColor4f(0, 0, 0, mContourTreeAlpha);
	glRasterPos2f(0.01, 1.02);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)mName.c_str());
	glRasterPos2f(0.01 + pixelWidth / 2, 1.02 + pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)mName.c_str());
	glRasterPos2f(0.01 - pixelWidth / 2, 1.02 - pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)mName.c_str());
}

void MyContourTree::DrawArcLabelsUnoccluded(){
	float cutSize = 0.005;																//	relative length of cuts
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float pixelWidth = 1.f / viewport[2];
	float pixelHeight = 1.f / (viewport[3]/2);
	MySpaceFillingNaive spaceFill;
	void * font = GLUT_BITMAP_HELVETICA_18;
	//void * font = GLUT_BITMAP_TIMES_ROMAN_24;
	for (long whichArc = 0; whichArc < nValidArcs; whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		long arc = valid[whichArc];															//	grab an edge from the list
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends

		// draw label
		if (mLabelVolume){
			string name = mArcName[arc];
			if (supernodes[topNode].IsLowerLeaf() || supernodes[topNode].IsUpperLeaf()){
				float length = 0;
				for (int i = 0; i < name.size(); i++){
					length += glutBitmapWidth(font, name[i]);
				}
				glColor4f(0, 0, 0, mContourTreeAlpha);
				MyVec2f lowPos(supernodes[topNode].xPosn - length / 2 * pixelWidth, supernodes[topNode].yPosn + cutSize);
				MyVec2f highPos = lowPos + MyVec2f(length * pixelWidth, glutBitmapHeight(font)*pixelHeight);
				MyBox2f box = spaceFill.PushBoxFromTop(MyBox2f(lowPos, highPos), 0.0001);
				glRasterPos2f(box.GetLowPos()[0], box.GetLowPos()[1]);
				glutBitmapString(font, (const unsigned char*)name.c_str());
				MyPrimitiveDrawer::DrawLineAt(MyVec2f(supernodes[topNode].xPosn , supernodes[topNode].yPosn ), box.GetLowPos());
			}
			else if (supernodes[bottomNode].IsLowerLeaf() || supernodes[bottomNode].IsUpperLeaf()){
			}
		}
	}

	glColor4f(0, 0, 0, mContourTreeAlpha);
	glRasterPos2f(0.01, 1.02);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)mName.c_str());
	glRasterPos2f(0.01 + pixelWidth / 2, 1.02 + pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)mName.c_str());
	glRasterPos2f(0.01 - pixelWidth / 2, 1.02 - pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)mName.c_str());
}

void MyContourTree::DrawPlanarContourTree()		//	draws a planar version of the contour tree
{ // DrawPlanarContourTree()
	float heightUnit = (MaxHeight() - MinHeight()) * 0.001;									//	compute unit of height for scaling
	float cutSize = 0.005;																//	relative length of cuts
	int arc, whichArc;
	float xDiff, xHeight;
	long x, y, z;

	/*
	// draw compared arc highlight
	for (std::map<long, char>::const_iterator it = mArcStatus.begin();
		it != mArcStatus.end(); ++it){
		long ctArc = it->first;
		if (IsArcCompared(ctArc)){
			vector<float>& histogram = mArcHistogram[ctArc];
			if (histogram.empty()) return;
			long topNode = superarcs[ctArc].topID, bottomNode = superarcs[ctArc].bottomID;
			float xPos;
			if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
				xPos = supernodes[topNode].xPosn;
			}
			else{
				xPos = supernodes[bottomNode].xPosn;
			}
			float height = GetDrawingHeight(*max_element(histogram.begin(), histogram.end()), GetArcScale(ctArc)) / 2;

			float yStart = min(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
			float yEnd = max(supernodes[bottomNode].yPosn, supernodes[topNode].yPosn);
			float yRange = (yEnd - yStart)/2;
			float yCenter = (yEnd + yStart) / 2;

			glBegin(GL_TRIANGLE_FAN);
			glColor4f(0, 0, 0, 0.5);
			glVertex2f(xPos, yCenter);
			glColor4f(1, 1, 1, 0.5);
			for (int i = 0; i < 21; i++){
				float angle = i*3.1415926 / 10;
				glVertex2f(xPos + height*cos(angle) * 1.5, yCenter + yRange*sin(angle) * 1.3);
			}
			glEnd();
		}
	}
	*/
	glLineWidth(1.0);
	DrawContourTreeFrame();
	GLfloat edge_colour[4] = { 0.0, 0.0, 0.0, mContourTreeAlpha };						//	colour for edges if not coloured	

	for (whichArc = 0; whichArc < nValidArcs; whichArc++){
		arc = valid[whichArc];
		//DrawArcHistogram(arc);
		if (GetArcScale(arc) == MappingScale_Sci){
			DrawArcHistogramScientific(arc);
		}
		else{
			DrawArcHistogram(arc);
		}
	}

	glColor4fv(edge_colour);															//	set the colour for nodes and arcs
	glLineWidth(1.0);
	glBegin(GL_LINES);																	//	we will generate a bunch of lines

	for (whichArc = 0; whichArc < nValidArcs; whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		arc = valid[whichArc];															//	grab an edge from the list

		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends
		if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
			// horizontal line from parent always
			glVertex2f(supernodes[bottomNode].xPosn, supernodes[bottomNode].yPosn);
			glVertex2f(supernodes[topNode].xPosn, supernodes[bottomNode].yPosn);
			glVertex2f(supernodes[topNode].xPosn, supernodes[bottomNode].yPosn);
			glVertex2f(supernodes[topNode].xPosn, supernodes[topNode].yPosn);
		}
		else{
			// horizontal line from parent always
			glVertex2f(supernodes[topNode].xPosn, supernodes[topNode].yPosn);
			glVertex2f(supernodes[bottomNode].xPosn, supernodes[topNode].yPosn);
			glVertex2f(supernodes[bottomNode].xPosn, supernodes[topNode].yPosn);
			glVertex2f(supernodes[bottomNode].xPosn, supernodes[bottomNode].yPosn);
		}
	} // loop through superarcs
	glEnd();																			//	end the set of lines
	glLineWidth(1.0);

	glDepthFunc(GL_ALWAYS);
	// draw label
	//DrawArcLabels();
	DrawArcLabelsUnoccluded();

	glPointSize(5.0);																	//	use a point radius of 5
	glBegin(GL_POINTS);																	//	now draw some points

	glColor4f(0, 0, 0, mContourTreeAlpha);
	for (int whichNode = 0; whichNode < nValidNodes; whichNode++)								//	walk through the nodes
	{ // for each node
		int node = validNodes[whichNode];													//	grab an active node from the list
		glVertex2f(supernodes[node].xPosn, supernodes[node].yPosn);								//	place the point
	} // for each node
	glEnd();																			//	end the set of points
	glPointSize(1.0);																	//	use a point radius of 1

	glBegin(GL_QUADS);																	//	one quad per edge
	for (whichArc = 0; whichArc < nActiveArcs; whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		arc = active[whichArc];															//	grab an edge from the list
		//		printf("arc %d\n", arc);
		if (!superarcs[arc].CheckFlag(Superarc::isValid)) { printf("Yowch! %d \n", arc); continue; }
		if (superarcs[arc].CheckFlag(Superarc::isSuppressed)) continue;							//	skip suppressed edges

		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends
		xDiff = supernodes[topNode].xPosn - supernodes[bottomNode].xPosn;						//	compute difference in x
		xHeight = (*(supernodes[topNode].value) - *(supernodes[bottomNode].value));				//	compute the x-height

		if (differentColouredContours)
			//glColor3fv(surface_colour[superarcs[arc].colour]);
			glColor4f(surface_colour[superarcs[arc].colour][0], surface_colour[superarcs[arc].colour][0],
			surface_colour[superarcs[arc].colour][0], mContourTreeAlpha);
		else
			glColor4f(basic_colour[0], basic_colour[0], basic_colour[0], mContourTreeAlpha);			//	set the colour for this contour
		//float xPosn = supernodes[bottomNode].xPosn + xDiff * (superarcs[arc].seedValue - *(supernodes[bottomNode].value)) / xHeight;
		float xPosn;
		if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
			// horizontal line from parent always
			xPosn = supernodes[topNode].xPosn;
		}
		else{
			// horizontal line from parent always
			xPosn = supernodes[bottomNode].xPosn;
		}
		//	compute x position
		glColor4f(1, 0.5, 0.5, mContourTreeAlpha);
		glVertex2f(xPosn - 2 * cutSize, superarcs[arc].seedValue - 2 * heightUnit);
		glVertex2f(xPosn + 2 * cutSize, superarcs[arc].seedValue - 2 * heightUnit);
		glVertex2f(xPosn + 2 * cutSize, superarcs[arc].seedValue + 2 * heightUnit);
		glVertex2f(xPosn - 2 * cutSize, superarcs[arc].seedValue + 2 * heightUnit);
	} // loop through superarcs

	for (whichArc = 0; whichArc < nSelectedArcs; whichArc++)									//	walk through the array from low to high
	{ // loop through superarcs
		arc = selected[whichArc];														//	grab an edge from the list
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends
		xDiff = supernodes[topNode].xPosn - supernodes[bottomNode].xPosn;						//	compute difference in x
		xHeight = (*(supernodes[topNode].value) - *(supernodes[bottomNode].value));				//	compute the x-height

		if (differentColouredContours)
			glColor3fv(basic_colour);								//	set the colour for this contour
		else
			glColor3fv(select_colour);													//	set the colour for this contour
		//float xPosn = supernodes[bottomNode].xPosn + xDiff * (currentSelectionValue - *(supernodes[bottomNode].value)) / xHeight;
		float xPosn;
		if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
			// horizontal line from parent always
			xPosn = supernodes[topNode].xPosn;
		}
		else{
			// horizontal line from parent always
			xPosn = supernodes[bottomNode].xPosn;
		}
		//	compute x position
		glColor4f(1, 0.5, 0.5, mContourTreeAlpha);
		glVertex2f(xPosn - 3*cutSize, currentSelectionValue - 3 * heightUnit);
		glVertex2f(xPosn + 3*cutSize, currentSelectionValue - 3 * heightUnit);
		glVertex2f(xPosn + 3*cutSize, currentSelectionValue + 3 * heightUnit);
		glVertex2f(xPosn - 3*cutSize, currentSelectionValue + 3 * heightUnit);
	} // loop through superarcs

	glEnd();																			//	end the set of quads

	for (whichArc = 0; whichArc < nValidArcs; whichArc++){
		arc = valid[whichArc];
		DrawArcSnappingPoint(arc);
	}

	glDepthFunc(GL_LESS);
} // DrawPlanarContourTree()

void MyContourTree::DrawSelectedArcVoxes(long arc, float isoValue){
	vector<float*>& voxes = mArcNodes[arc];
	for (int i = 0; i < voxes.size(); i++){
		if (compareHeight(voxes[i], &isoValue) < 0) break;
		long x, y, z;
		height.ComputeIndex(voxes[i], x, y, z);
		float value = height(x, y, z);
		MyColor4f color = colorMap->GetColor(value*colorMap->GetWidth(), 0);
		glColor3f(color.r, color.g, color.b);
		glPushMatrix();
		glTranslatef(x, y, z);
		glutSolidCube(1);
		glPopMatrix();
	}

	queue<long> arcQueue;
	long nextArc = supernodes[superarcs[arc].topID].upList;
	for (int i = 0; i < supernodes[superarcs[arc].topID].upDegree; i++){
		arcQueue.push(nextArc);
		nextArc = superarcs[nextArc].nextUp;
	}

	while (!arcQueue.empty()){
		long theArc = arcQueue.front();
		arcQueue.pop();
		long nextArc = supernodes[superarcs[theArc].topID].upList;
		for (int i = 0; i < supernodes[superarcs[theArc].topID].upDegree; i++){
			arcQueue.push(nextArc);
			nextArc = superarcs[nextArc].nextUp;
		}
		vector<float*>& voxes = mArcNodes[theArc];
		for (int i = 0; i < voxes.size(); i++){
			long x, y, z;
			height.ComputeIndex(voxes[i], x, y, z);
			float value = height(x,y,z);
			MyColor4f color = colorMap->GetColor(value*colorMap->GetWidth(), 0);
			glColor3f(color.r, color.g, color.b);
			glPushMatrix();
			glTranslatef(x, y, z);
			glutSolidCube(1);
			glPopMatrix();
		}
	}
}

void MyContourTree::DrawSelectedArcVoxes(long arc, bool useDisplayLists, bool pickColours){
	{ // RenderContour()
		GLubyte pick_colour[3] = { 0, 0, 0 };													//	for use picking colours
		//	GLfloat shell_colour[3] = { 0, 0, 0};													//	for use picking colours
		if (pickColours)																	//	i.e. we want to pick something
		{ // pickColours	
			pick_colour[0] = arc % 256;														//	chop up into bytes
			pick_colour[1] = (arc / 256) % 256;
			pick_colour[2] = ((arc / 256) / 256);
			glColor3ubv(pick_colour);														//	and set the colour
		} // pickColours
		else // i.e. not pick colours
		{ // not pick colours
			if (superarcs[arc].CheckFlag(Superarc::isSelected))
				if (differentColouredContours)
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, basic_colour);
				else
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, select_colour);
			else
				if (differentColouredContours)
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, surface_colour[superarcs[arc].colour]);	
				else
					glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, basic_colour);
		} // not pick colours
		if (useDisplayLists)
		{ // display lists turned off
			if (superarcs[arc].CheckFlag(Superarc::isDirty))
			{ // dirty arc needs re-extraction
				superarcs[arc].ClearFlag(Superarc::isDirty);
				glNewList(baseDisplayListID + arc, GL_COMPILE_AND_EXECUTE);							//	start creating its display list
				if (superarcs[arc].CheckFlag(Superarc::isSelected))
					DrawSelectedArcVoxes(arc, currentSelectionValue);
				else
					DrawSelectedArcVoxes(arc, superarcs[arc].seedValue);
			} // dirty arc needs re-extraction
			glEndList();																	//	end the list
			glCallList(baseDisplayListID + arc);													//	call the stored list for this contour
		} // display lists turned on
		else
		{ // display lists turned off
			if (superarcs[arc].CheckFlag(Superarc::isSelected))
				DrawSelectedArcVoxes(arc, currentSelectionValue);
			else
				DrawSelectedArcVoxes(arc, superarcs[arc].seedValue);
		} // display lists turned off
	} // RenderContour()
}

void MyContourTree::DrawSelectedVoxes(bool useDisplayLists, bool pickColours){
	/*
	for (int i = 0; i < mMaskVolume.XDim(); i++){
		for (int j = 0; j < mMaskVolume.YDim(); j++){
			for (int k = 0; k < mMaskVolume.ZDim(); k++){
				if (mMaskVolume(i, j, k) > 0.5){
					float value = height(i, j, k);
					MyColor4f color = colorMap->GetColor(value*colorMap->GetWidth(), 0);
					glColor3f(color.r, color.g, color.b);
					glPushMatrix();
					glTranslatef(i, j, k);
					glutSolidCube(1);
					glPopMatrix();
				}
			}
		}
	}
	return;
	*/
	int theArc;																			//	count of how many contours were generated

	for (theArc = 0; theArc < nActiveArcs; theArc++)											//	walk along array of superarcs
	{ // for theArc
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSuppressed)) continue;					//	ignore suppressed arcs
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSelected)) continue;					//	ignore selected flags (SHOULD be redundant)
		DrawSelectedArcVoxes(active[theArc], useDisplayLists, pickColours);
	} // for theArc
	for (theArc = 0; theArc < nSelectedArcs; theArc++)										//	walk along array of superarcs
	{ // for theArc
		DrawSelectedArcVoxes(selected[theArc], useDisplayLists, pickColours);
	} // for theArc

	return;
}

int MyContourTree::PickArc(float x, float y, bool printInfo)								//	picks an arc: returns -1 if it fails
{ // PickArc()
	y = MinHeight() + y * (MaxHeight() - MinHeight());					//	convert to the range of heights actually in use
	float bestDeltaX = 1.0;											//	best distance laterally
	long whichArc = NO_SUPERARC;										//	which arc has this distance

	for (int theArc = 0; theArc < nValidArcs; theArc++)					//	again, walk through all the arcs
	{ // arc loop
		int arc = valid[theArc];										//	grab the arc from the active list
		float deltaHt = *(supernodes[superarcs[arc].topID].value) - *(supernodes[superarcs[arc].bottomID].value);
		//	height range of arc
		if (*(supernodes[superarcs[arc].topID].value) <= y) continue;		//	discard arcs outside the range
		if (*(supernodes[superarcs[arc].bottomID].value) > y) continue;
		if (deltaHt == 0.0) continue;									//	discard horizontal arcs
		//	this way, no divide by zero for vertical arcs
		float arcX;
		if (supernodesExt[superarcs[arc].bottomID].numLeaves <= supernodesExt[superarcs[arc].topID].numLeaves){
			// horizontal line from parent always
			arcX = supernodes[superarcs[arc].bottomID].xPosn;
		}
		else{
			// horizontal line from parent always
			arcX = supernodes[superarcs[arc].topID].xPosn;
		}
		//	compute x value at fixed height on arc
		float deltaX = fabs(arcX - x);								//	compute delta from point given	
		if ((deltaX < 0.02) && (deltaX < bestDeltaX))					//	check for tolerance & best
		{
			whichArc = arc; bestDeltaX = deltaX;
		};					//	and update "best" so far
	} // arc loop
	if (whichArc != NO_SUPERARC && printInfo){
		float maxFA = -1;
		float minFA = -1;
		if (mArcNodes[whichArc].size() > 0){
			maxFA = *mArcNodes[whichArc].front();
			minFA = *mArcNodes[whichArc].back();
		}
		if (superarcs[whichArc].seedFromLo != NULL){
			printf("Arc [%d] Info: %d vertices, %d nodesOnArc,\nseed %f to %f,\nfa %f to %f\nbottomFA %f, topFA %f\n", whichArc,
				mArcNodes[whichArc].size(), superarcs[whichArc].nodesOnArc,
				*(superarcs[whichArc].seedFromLo), *(superarcs[whichArc].seedToLo),
				minFA, maxFA,
				*supernodes[superarcs[whichArc].bottomID].value, *supernodes[superarcs[whichArc].topID].value);
		}
		else if (superarcs[whichArc].seedFromHi != NULL){
			printf("Arc [%d] Info: %d vertices, %d nodesOnArc,\nseed %f to %f,\nfa %f to %f\nbottomFA %f, topFA %f\n", whichArc,
				mArcNodes[whichArc].size(), superarcs[whichArc].nodesOnArc,
				*(superarcs[whichArc].seedFromHi), *(superarcs[whichArc].seedToHi),
				minFA, maxFA,
				*supernodes[superarcs[whichArc].bottomID].value, *supernodes[superarcs[whichArc].topID].value);
		}
		else{
			printf("Arc [%d] Info: %d vertices, %d nodesOnArc,\nno seed available,\nfa %f to %f\nbottomFA %f, topFA %f\n", whichArc,
				mArcNodes[whichArc].size(), superarcs[whichArc].nodesOnArc,
				minFA, maxFA,
				*supernodes[superarcs[whichArc].bottomID].value, *supernodes[superarcs[whichArc].topID].value);
		}
	}
	return whichArc;												//	return the one we found (if any)
} // PickArc()

long MyContourTree::GetArcRoiCount(long arc){
	//return superarcs[arc].nodesOnArc;
	long count = 0;
	vector<float*>& voxes = mArcNodes[arc];
	for (int i = 0; i < voxes.size(); i++){
		long x, y, z;
		height.ComputeIndex(voxes[i], x, y, z);
		if (mLabelVolume->get_at_index(x,y,z)>0){
			count++;
		}
	}
	return count;
}


std::string MyContourTree::ComputeArcName(long arc){
	vector<float*>& voxes = mArcNodes[arc];
	map<long, long> labelCount;
	for (int i = 0; i < voxes.size(); i++){
		long x, y, z;
		height.ComputeIndex(voxes[i], x, y, z);
		int label = mLabelVolume->get_at_index(x, y, z);
		if (label > 0){
			if (labelCount.find(label) != labelCount.end()){
				labelCount[label]++;
			}
			else{
				labelCount[label] = 1;
			}
		}
	}

	class PriorityCompareLess
	{ // class PriorityCompare
	public:
		bool operator () (PriorityIndex index1, PriorityIndex index2)			//	comparison operator
		{ // comparison operator
			return index1.priority < index2.priority;						//	pretty straightforward
		} // comparison operator
	}; // class PriorityCompare

	priority_queue<PriorityIndex, vector<PriorityIndex>, PriorityCompareLess> pQueue;
	for (map<long, long>::iterator itr = labelCount.begin();
		itr != labelCount.end(); itr++){
		pQueue.push(PriorityIndex(itr->first, itr->second));
	}

	if (pQueue.empty()){
		return "";
	}
	else{
		// check first two elements in the pQueue
		PriorityIndex majorEle = pQueue.top();
		pQueue.pop();
		if (majorEle.priority <= voxes.size() / 5){
			// mostly zero
			return "*"+mLabelName[majorEle.index];
		}
		else{
			if (pQueue.empty()){
				return mLabelName[majorEle.index];
			}
			else{
				PriorityIndex minorEle = pQueue.top();
				if (minorEle.priority >= majorEle.priority / 5){
					return mLabelName[majorEle.index] + "&" + mLabelName[minorEle.index];
				}
				else{
					return mLabelName[majorEle.index];
				}
			}
		}
	}
}

void MyContourTree::ComputeArcNames(){
	//for (long arc = 0; arc < nSuperarcs; arc++){
	//	mArcName[arc] = ComputeArcName(arc);
	//}
	for (long arc = 0; arc < nValidArcs; arc++){
		long whichArc = valid[arc];
		mArcName[whichArc] = ComputeArcName(whichArc);
	}
}

void MyContourTree::PruneNoneROIs(){
	RestoreTree();

	int pruned;
	int numPruned = 0;

	queue<long> leaveQueue;
	//for (int arc = 0; arc < nSuperarcs; arc++){
	for (int arc = 0; arc < nValidArcs; arc++){
		// Use top nodes
		long whichArc = valid[arc];
		if (supernodes[superarcs[whichArc].topID].IsUpperLeaf()){
			leaveQueue.push(whichArc);
		}
		else if (supernodes[superarcs[whichArc].bottomID].IsLowerLeaf()){
			leaveQueue.push(whichArc);
		}
	}
	/*
	while (!leaveQueue.empty()){
		long whichArc = leaveQueue.front();
		leaveQueue.pop();
		float* value = supernodes[superarcs[whichArc].topID].value;
		long x, y, z;
		height.ComputeIndex(value, x, y, z);
		int label = mLabelVolume->get_at_index(x, y, z);
		if (label == 0 || superarcs[whichArc].nodesOnArc < 20){
			SingleCollapse(whichArc);
			numPruned++;
			if (supernodes[superarcs[whichArc].bottomID].IsUpperLeaf()){
				leaveQueue.push(supernodes[superarcs[whichArc].bottomID].downList);
			}
		}
	}
	*/
	while (!leaveQueue.empty()){
		long whichArc = leaveQueue.front();
		leaveQueue.pop();
		long roiCount = GetArcRoiCount(whichArc);
		if (roiCount < mPruningThreshold){
		//if (mArcName[whichArc].empty()){
			SingleCollapse(whichArc);
			numPruned++;
			if (supernodes[superarcs[whichArc].bottomID].IsUpperLeaf()){
				leaveQueue.push(supernodes[superarcs[whichArc].bottomID].downList);
			}
			else if (supernodes[superarcs[whichArc].topID].IsLowerLeaf()){
				leaveQueue.push(supernodes[superarcs[whichArc].topID].upList);
			}
		}
	}
	cout << "ROI-based Pruning: " << numPruned << " arcs pruned." << endl;

	numPruned = 0;
	do{
		pruned = false;
		for (long node = 0; node < nSupernodes; node++){
			if (supernodes[node].IsRegular()){
				CollapseVertex(node);
				pruned = true;
				numPruned++;
			}
		}
	} while (pruned);
	cout << "ROI-based Pruning: " << numPruned << " nodes pruned." << endl;
}

void MyContourTree::UpdateArcNodes(){
	for (long arc = 0; arc < nValidArcs; arc++){
		long whichArc = valid[arc];
		vector<float*>& nodes = mArcNodes[whichArc];
		nodes.clear();
		Array3D<char> visited;
		visited.Construct(xDim, yDim, zDim);


		// if superarc, retrive proper seed
		long topArc = superarcs[whichArc].topArc;
		long bottomArc = superarcs[whichArc].bottomArc;
		while (topArc != NO_SUPERARC){
			topArc = superarcs[topArc].topArc;
		}
		while (bottomArc != NO_SUPERARC){
			whichArc = bottomArc;
			bottomArc = superarcs[bottomArc].bottomArc;
		}

		Superarc theArc = superarcs[whichArc];
		if (theArc.seedToLo != NULL){
			queue<float*> vertexQueue;
			vertexQueue.push(theArc.seedToLo);
			while (!vertexQueue.empty()){
				float *thisEnd = vertexQueue.front();
				vertexQueue.pop();
				long x, y, z;
				height.ComputeIndex(thisEnd, x, y, z);
				QueueJoinNeighbours(x, y, z);
				visited(x, y, z) = 1;
				for (int i = 0; i < nNeighbours; i++){
					long nbrX = neighbourQueue[i][0];
					long nbrY = neighbourQueue[i][1];
					long nbrZ = neighbourQueue[i][2];
					if (visited(nbrX, nbrY, nbrZ) == 0){
						float* neighbour = &(height(nbrX, nbrY, nbrZ));
						visited(nbrX, nbrY, nbrZ) = 1;
						if (neighbour == theArc.seedFromLo) continue;
						if (compareHeight(neighbour, theArc.seedFromLo) > 0){
							vertexQueue.push(neighbour);
						}
					}
				}
				nodes.push_back(thisEnd);
			}
		}
		else if (theArc.seedToHi != NULL){
			queue<float*> vertexQueue;
			vertexQueue.push(theArc.seedToHi);
			while (!vertexQueue.empty()){
				float *thisEnd = vertexQueue.front();
				vertexQueue.pop();
				long x, y, z;
				height.ComputeIndex(thisEnd, x, y, z);
				QueueSplitNeighbours(x, y, z);
				visited(x, y, z) = 1;
				for (int i = 0; i < nNeighbours; i++){
					long nbrX = neighbourQueue[i][0];
					long nbrY = neighbourQueue[i][1];
					long nbrZ = neighbourQueue[i][2];
					if (visited(nbrX, nbrY, nbrZ) == 0){
						float* neighbour = &(height(nbrX, nbrY, nbrZ));
						visited(nbrX, nbrY, nbrZ) = 1;
						if (neighbour == theArc.seedFromHi) continue;
						if (compareHeight(neighbour, theArc.seedFromHi) < 0){
							vertexQueue.push(neighbour);
						}
					}
				}
				nodes.push_back(thisEnd);
			}
		}
		sort(nodes.begin(), nodes.end(), compareHeightLogic);
	}

}

void MyContourTree::SingleCollapse(long whichArc)											//	collapses a single node after constructing collapse tree
{ // SingleCollapse()
	for (int arc = 0; arc < nActiveArcs; arc++)
		if (!superarcs[active[arc]].CheckFlag(Superarc::isActive))
			printf("%20s:%5d: At collapse step %5d, arc %3d is on active list but active flag is not set\n",
			__FILE__, __LINE__, nextSuperarc - savedNextSuperarc, active[arc]);
		else if (!superarcs[active[arc]].CheckFlag(Superarc::isValid))
			printf("%20s:%5d: At collapse step %5d, arc %3d is on active list but valid flag is not set\n",
			__FILE__, __LINE__, nextSuperarc - savedNextSuperarc, active[arc]);
	if (nValidArcs <= 1) return;
	ClearRestorable();													//	simplify life by having no "restorables"
	if (superarcs[whichArc].CheckFlag(Superarc::isValid))						//	if the next to collapse is currently valid
	{ // leaf-prune
		if (selectionRoot == whichArc)									//	it's the selection root
			CommitSelection();											//	so transfer the selection to active status
		else
			ClearSelection();											//	otherwise, clear selection (for later recomputation)
		if (superarcs[whichArc].CheckFlag(Superarc::isActive))					//	if it is active
		{ // active arc
			RemoveFromActive(whichArc);									//	make it inactive
		} // active arc
		if (supernodes[superarcs[whichArc].topID].IsUpperLeaf())
		{ // upper leaf
			RemoveArc(whichArc);										//	and remove it from the tree
			RemoveFromValidNodes(superarcs[whichArc].topID);					//	remove the bottom node as well
		} // upper leaf
		else
		{ // lower leaf
			RemoveArc(whichArc);										//	and remove it from the tree
			RemoveFromValidNodes(superarcs[whichArc].bottomID);				//	remove the bottom node as well
		} // lower leaf						
	} // leaf-prune
	else
		CollapseToSuperarc(whichArc);										//	collapse known top & bottom arcs to it
	if (selectionRoot != NO_SUPERARC)										//	if there is a selection
		UpdateSelection(currentSelectionValue);								//	reset it
	logTreeSize = log((float)nValidArcs) / log((float)nNonEpsilonArcs);			//	reset the tree size
	logPriorityBound = log((float)collapseBounds[nValidArcs]) / log((float)nVertices);//	and the priority bound
} // SingleCollapse()

long MyContourTree::CollapseVertex(long whichSupernode)							//	collapses a vertex: returns the new superarcs ID
	{ // CollapseVertex()
	if (! supernodes[whichSupernode].IsRegular())
		{ // not a regular point
		printf("Major error: attempt to collapse a vertex with up-degree %1d and down-degree %1d. (both should be 1).\n",
			supernodes[whichSupernode].upDegree, supernodes[whichSupernode].downDegree);
		return NO_SUPERARC;
		} // not a regular point

	//	grab the existing arcs & nodes
	long topArc = supernodes[whichSupernode].upList, bottomArc = supernodes[whichSupernode].downList;
	long topNode = superarcs[topArc].topID, bottomNode = superarcs[bottomArc].bottomID;

//	printf("Collapsing edges %d and %d\n", topArc, bottomArc);

	//	first remove the existing arcs
	RemoveArc(topArc);	RemoveArc(bottomArc);
	
	//	and remove the vertex
	RemoveFromValidNodes(whichSupernode);
	
	//	now add the new arc
	long newSArc = AddSuperarc(topNode, bottomNode, NULL, NULL, NULL, NULL);

	//	now fill in the hierarchical fields
	superarcs[newSArc].topArc = topArc; superarcs[newSArc].bottomArc = bottomArc;
	superarcs[newSArc].nodesThisSideOfTop = superarcs[topArc].nodesThisSideOfTop;
	superarcs[newSArc].nodesThisSideOfBottom = superarcs[bottomArc].nodesThisSideOfBottom;
	//	this one could equally be the sum of the two parts, but this value is also defensible
	superarcs[newSArc].nodesOnArc = superarcs[newSArc].nodesThisSideOfTop - (nVertices - superarcs[newSArc].nodesThisSideOfBottom);
	mArcNodes[newSArc] = mArcNodes[topArc];
	mArcNodes[newSArc].insert(mArcNodes[newSArc].end(), mArcNodes[bottomArc].begin(), mArcNodes[bottomArc].end());
	sort(mArcNodes[newSArc].begin(), mArcNodes[newSArc].end(), compareHeightLogic);
	//	now do the same for the Riemann sum
	superarcs[newSArc].sampleSumTop = superarcs[topArc].sampleSumTop;
	superarcs[newSArc].sampleSumBottom = superarcs[bottomArc].sampleSumBottom;
	superarcs[newSArc].sampleSumOnArc = superarcs[newSArc].sampleSumTop + superarcs[newSArc].sampleSumBottom - sampleSum;
	
//	printf("top: %8.1f, bot: %8.1f, on: %8.1f\n", superarcs[newSArc].sampleSumTop, superarcs[newSArc].sampleSumBottom, superarcs[newSArc].sampleSumOnArc);
	
	return newSArc;
	} // CollapseVertex()