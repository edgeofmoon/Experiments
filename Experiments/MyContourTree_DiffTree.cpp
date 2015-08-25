#include "MyContourTree.h"
#include "Ric\RicVolume.h"
#include "MySpaceFillingNaive.h"
#include "MyPrimitiveDrawer.h"

void MyContourTree::LoadVoxSignificance(const char* fn){
	RicVolume vol(fn);
	if (vol.nvox == 0) { printf("Unable to open file %s\n", fn); }

	mVoxSignificance.Construct(xDim, yDim, zDim);

	for (long i = 0; i < xDim; i++){
		for (long j = 0; j < yDim; j++){
			for (long k = 0; k < zDim; k++){
				mVoxSignificance(i, j, k) = vol.vox[i][j][k];
			}
		}
	}	
}


void MyContourTree::UpdateSigArcList(){
	mSigArcs.clear();
	long x, y, z;
	for (long i = 0; i < nValidArcs; i++){
		long arc = valid[i];
		long numSigVoxes = 0;
		vector<float*>& voxes = mArcNodes[arc];
		for (long j = 0; j < voxes.size(); j++){
			height.ComputeIndex(voxes[j], x, y, z);
			float oneMinuesP = mVoxSignificance(x, y, z);
			if (oneMinuesP >= 1 - mSigArcThreshold_P){
				numSigVoxes++;
			}
		}
		if (numSigVoxes / (float)voxes.size() >= mSigArcThreshold_VolRatio){
			mSigArcs.push_back(arc);
		}
	}

	cout << "SigArcs: " << mSigArcs.size() << " / " << nValidArcs << endl;
}


void MyContourTree::DrawSimpleArc(long arc){
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

	glColor4f(0, 0, 0, mContourTreeAlpha);
	glBegin(GL_LINES);
	glVertex2f(xPos, yStart);
	glVertex2f(xPos, yEnd);
	glEnd();
}

void MyContourTree::DrawArcDiffHistogram(long arc){
	long thatArc = -1;
	if (mArcMap.find(arc) != mArcMap.end()){
		thatArc = mArcMap[arc];
	}
	if (thatArc < 0){
		DrawSimpleArc(arc);
	}
	else{
		//if (find(mSigArcs.begin(), mSigArcs.end(), arc) == mSigArcs.end()){
		//	cout << "Arc " << arc << " is mapped but not in the list.\n";
		//}
		if (GetArcScale(arc) == MappingScale_Sci){
			DrawArcHistogramScientific(arc);
		}
		else{
			DrawArcHistogram(arc);
		}
	}
}

void MyContourTree::RenderSigDiffTree(){
	DrawContourTreeFrame();
	for (long i = 0; i < nValidArcs; i++){
		long arc = valid[i];
		DrawArcDiffHistogram(arc);
	}

	// lines
	glColor4f(0,0,0,mContourTreeAlpha);															//	set the colour for nodes and arcs
	glLineWidth(1.0);
	glBegin(GL_LINES);																	//	we will generate a bunch of lines

	for (long whichArc = 0; whichArc < nValidArcs; whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		long arc = valid[whichArc];															//	grab an edge from the list

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
	glEnd();

	// label
	float cutSize = 0.005;																//	relative length of cuts
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float pixelWidth = 1.f / viewport[2];
	float pixelHeight = 1.f / (viewport[3] / 2);
	MySpaceFillingNaive spaceFill;
	void * font = GLUT_BITMAP_HELVETICA_18;
	//void * font = GLUT_BITMAP_TIMES_ROMAN_24;
	for (long whichArc = 0; whichArc < mSigArcs.size(); whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		long arc = mSigArcs[whichArc];															//	grab an edge from the list
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends

		// draw label
		if (mLabelVolume){
			string name = mArcName[arc];
			//if (supernodes[topNode].IsLowerLeaf() || supernodes[topNode].IsUpperLeaf()){
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
				MyPrimitiveDrawer::DrawLineAt(MyVec2f(supernodes[topNode].xPosn, supernodes[topNode].yPosn), box.GetLowPos());
			//}
			//else if (supernodes[bottomNode].IsLowerLeaf() || supernodes[bottomNode].IsUpperLeaf()){
			//}
		}
	}

	glColor4f(0, 0, 0, mContourTreeAlpha);
	glRasterPos2f(0.01, 1.02);
	string diffName("Difference Tree");
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)diffName.c_str());
	glRasterPos2f(0.01 + pixelWidth / 2, 1.02 + pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)diffName.c_str());
	glRasterPos2f(0.01 - pixelWidth / 2, 1.02 - pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)diffName.c_str());

	// ticks
	float heightUnit = (MaxHeight() - MinHeight()) * 0.001;

	glDepthFunc(GL_ALWAYS);
	glBegin(GL_QUADS);
	for (long whichArc = 0; whichArc < nActiveArcs; whichArc++)										//	walk through the array from low to high
	{ // loop through superarcs
		long arc = active[whichArc];															//	grab an edge from the list
		if (find(mSigArcs.begin(), mSigArcs.end(), arc) == mSigArcs.end()) continue;
		//		printf("arc %d\n", arc);
		if (!superarcs[arc].CheckFlag(Superarc::isValid)) { printf("Yowch! %d \n", arc); continue; }
		if (superarcs[arc].CheckFlag(Superarc::isSuppressed)) continue;							//	skip suppressed edges

		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends
		float xDiff = supernodes[topNode].xPosn - supernodes[bottomNode].xPosn;						//	compute difference in x
		float xHeight = (*(supernodes[topNode].value) - *(supernodes[bottomNode].value));				//	compute the x-height

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

	for (long whichArc = 0; whichArc < nSelectedArcs; whichArc++)									//	walk through the array from low to high
	{ // loop through superarcs
		long arc = selected[whichArc];														//	grab an edge from the list
		if (find(mSigArcs.begin(), mSigArcs.end(), arc) == mSigArcs.end()) continue;
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;					//	grab the two ends
		float xDiff = supernodes[topNode].xPosn - supernodes[bottomNode].xPosn;						//	compute difference in x
		float xHeight = (*(supernodes[topNode].value) - *(supernodes[bottomNode].value));				//	compute the x-height

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
		glVertex2f(xPosn - 3 * cutSize, currentSelectionValue - 3 * heightUnit);
		glVertex2f(xPosn + 3 * cutSize, currentSelectionValue - 3 * heightUnit);
		glVertex2f(xPosn + 3 * cutSize, currentSelectionValue + 3 * heightUnit);
		glVertex2f(xPosn - 3 * cutSize, currentSelectionValue + 3 * heightUnit);
	} // loop through superarcs
	glEnd();

	glDepthFunc(GL_LESS);
}

void MyContourTree::UpdateArcMapping(){
	mArcMap.clear();
	for (long i = 0; i < mContrastContourTree->mSigArcs.size(); i++){
		long ctArc = mContrastContourTree->mSigArcs[i];
		vector<long> smArcs;
		FindSimilarValidArcs(mContrastContourTree, ctArc, smArcs);
		for (int is = 0; is < smArcs.size(); is++){
			// if not already added
			if (find(mSigArcs.begin(), mSigArcs.end(), smArcs[is])
				== mSigArcs.end()){
				mArcMap[smArcs[is]] = ctArc;
			}
			else{
				// multiple mapping results in no mapping
				mArcMap[smArcs[is]] = -1;
			}
		}
	}
}

void MyContourTree::SyncSigArcsTo(MyContourTree* ct){
	mContrastContourTree = ct;
	mSigArcs.clear();
	UpdateArcMapping();
	for (std::map<long, long>::iterator itr = mArcMap.begin();
		itr != mArcMap.end(); itr ++){
		if (itr->second >= 0){
			mSigArcs.push_back(itr->first);
		}
	}

	/*
	for (long i = 0; i < ct->mSigArcs.size(); i++){
		long ctArc = ct->mSigArcs[i];
		vector<long> smArcs;
		FindSimilarValidArcs(ct, ctArc, smArcs);
		for (int is = 0; is < smArcs.size(); is++){
			// if not already added
			if (find(mSigArcs.begin(), mSigArcs.end(), smArcs[is])
				== mSigArcs.end()){
				mSigArcs.push_back(smArcs[is]);
			}
		}
	}
	*/
}