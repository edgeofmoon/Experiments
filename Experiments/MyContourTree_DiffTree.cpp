#include "MyContourTree.h"
#include "Ric\RicVolume.h"
#include "MySpaceFillingNaive.h"
#include "MyPrimitiveDrawer.h"
#include "ColorScaleTable.h"

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

void MyContourTree::DrawDistribution(float xPos, const SimpleDistribution& distribution,
	HistogramSide side, MappingScale scale){
	const vector<float>& histogram = distribution.mKernelDensity;
	if (histogram.empty()) return;

	float yStart = distribution.mMin;
	float yEnd = distribution.mMax;
	float yStep = (yEnd - yStart) / histogram.size();

	MappingScale arcScale = scale;
	float arcZoom = 1;
	glLineWidth(1);

	float leftHeightScale;
	float rightHeightScale;
	switch (side){
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
	float alplaModulation = 0.7;
	glDepthFunc(GL_ALWAYS);
	if (scale == MappingScale_Sci){
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
				exponentPos[i] = (exponent - mMinExponent) / exponentRange * 7 + 0.5;
			}
			else{
				exponentPos[i] = 0;
			}
		}
		for (int i = 0; i < histogram.size(); i++){
			float expHeight = exponentHeights[i];
			float expHeightNext = (i == histogram.size() - 1 ? exponentHeights[i] : exponentHeights[i + 1]);
			float posY = yStart + i*yStep;

			glBegin(GL_QUADS);
			//glColor3ubv((const GLubyte*)colorBrewer_sequential_8_multihue_9[exponentPos[i]]);
			glColor4f(colorBrewer_sequential_8_multihue_9[exponentPos[i]][0] / 255.f,
				colorBrewer_sequential_8_multihue_9[exponentPos[i]][1] / 255.f,
				colorBrewer_sequential_8_multihue_9[exponentPos[i]][2] / 255.f, mContourTreeAlpha*alplaModulation);
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
	}
	else{
		float maxBaseHeight = GetDrawingHeight(mMaxHistogramCount, mDefaultScale);
		for (int i = 0; i < histogram.size(); i++){
			float baseHeight = GetDrawingHeight(histogram[i], arcScale);
			float leftHeight = baseHeight * leftHeightScale*arcZoom;
			float rightHeight = baseHeight * rightHeightScale*arcZoom;
			glBegin(GL_QUADS);
			int color = GetDrawingHeight(histogram[i], mDefaultScale)
				/ GetDrawingHeight(mMaxHistogramCount, mDefaultScale) * 7 + 0.5;
			//float color = 0.5;
			glColor4f(colorBrewer_sequential_8_multihue_9[color][0] / 255.f,
				colorBrewer_sequential_8_multihue_9[color][1] / 255.f,
				colorBrewer_sequential_8_multihue_9[color][2] / 255.f, mContourTreeAlpha*alplaModulation);
			glVertex2f(xPos - leftHeight, yStart + i*yStep);
			glVertex2f(xPos + rightHeight, yStart + i*yStep);
			glVertex2f(xPos + rightHeight, yStart + (i + 1)*yStep);
			glVertex2f(xPos - leftHeight, yStart + (i + 1)*yStep);
			glEnd();

			glColor4f(0, 0, 0, mContourTreeAlpha);
			glBegin(GL_LINES);
			if (i == 0){
				glVertex2f(xPos - leftHeight, yStart);
				glVertex2f(xPos + rightHeight, yStart);
			}
			if (i == histogram.size() - 1){
				glVertex2f(xPos - leftHeight, yEnd);
				glVertex2f(xPos + rightHeight, yEnd);
			}
			else{
				float baseHeightNext = GetDrawingHeight(histogram[i + 1], arcScale);
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
	}
	glDepthFunc(GL_LESS);
}

void MyContourTree::DrawArcDiffHistogram(long arc){
	long thatArc = -1;
	if (mArcMap.find(arc) != mArcMap.end()){
		thatArc = mArcMap[arc];
	}
	if (thatArc < 0){
		if (mPathArcs[arc]){
			DrawSimpleArc(arc);
		}
	}
	else{
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;
		float xPos;
		if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
			xPos = supernodes[topNode].xPosn;
		}
		else{
			xPos = supernodes[bottomNode].xPosn;
		}
		const DiffHistogram& diffHist = mArcDiffHistogram[arc];
		DrawDistribution(xPos, diffHist.mLeft, HistogramSide_Left, mDefaultScale);
		DrawDistribution(xPos, diffHist.mRight, HistogramSide_Right, mDefaultScale);
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
		if (!IsArcSig(arc) && !mPathArcs[arc]) continue;
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
	for (long whichArc = 0; whichArc < mSigArcs.size(); whichArc++)	
	{ // loop through superarcs
		long arc = mSigArcs[whichArc];
		long topNode = superarcs[arc].topID, bottomNode = superarcs[arc].bottomID;

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

void MyContourTree::UpdatePathArcs(){
	mPathArcs.clear();
	long lowestNode = validNodes[0];
	for (long theNodeIndex = 1; theNodeIndex < nValidNodes; theNodeIndex++){
		long theNode = validNodes[theNodeIndex];
		if (compareHeight(supernodes[theNode].value, supernodes[lowestNode].value) < 0){
			lowestNode = theNode;
		}
	}
	for (int i = 0; i < mSigArcs.size(); i++){
		long arc = mSigArcs[i];
		std::vector<long> path;
		FindPathDown(superarcs[arc].bottomID, lowestNode, path);
		for (int j = 0; j < path.size()-1; j++){
			long pathArc = nodes2Arc(path[j], path[j + 1]);
			mPathArcs[pathArc] = true;
		}
	}
}

long MyContourTree::GetContrastArc(long arc) const{
	map<long, long>::const_iterator itr = mArcMap.find(arc);
	if (itr != mArcMap.end()){
		return itr->second;
	}
	else return -1;
}

void MyContourTree::CombineIndicesSorted(const vector<long>& thisIndices,
	const vector<long>& thatIndices, vector<long>& combinedIndices){
	combinedIndices.clear();
	int i = 0, j = 0;
	if (mArcCombineMode == ArcCombineMode_Intersection){
		while (i < thisIndices.size() && j < thatIndices.size()){
			if (thisIndices[i] == thatIndices[j]){
				combinedIndices.push_back(thisIndices[i]);
				i++; j++;
			}
			else if (thisIndices[i] > thatIndices[j]) j++;
			else i++;
		}
	}
	else if (mArcCombineMode == ArcCombineMode_Union){
		while (i < thisIndices.size() && j < thatIndices.size()){
			if (thisIndices[i] == thatIndices[j]){
				combinedIndices.push_back(thisIndices[i]);
				i++; j++;
			}
			else if (thisIndices[i] > thatIndices[j]) j++;
			else i++;
		}
		while (i < thisIndices.size()) combinedIndices.push_back(thisIndices[i++]);
		while (j < thatIndices.size()) combinedIndices.push_back(thatIndices[j++]);
	}
	else if (mArcCombineMode == ArcCombineMode_Complement){
		while (i < thisIndices.size() && j < thatIndices.size()){
			if (thisIndices[i] == thatIndices[j]){
				i++; j++;
			}
			else if (thisIndices[i] > thatIndices[j]){
				combinedIndices.push_back(thatIndices[j]);
				j++;
			}
			else{
				combinedIndices.push_back(thisIndices[i]);
				i++;
			}
		}
		while (i < thisIndices.size()) combinedIndices.push_back(thisIndices[i++]);
		while (j < thatIndices.size()) combinedIndices.push_back(thatIndices[j++]);
	}
}

void MyContourTree::UpdateDiffHistogram(){
	mArcDiffHistogram.clear();
	for (int i = 0; i < mSigArcs.size(); i++){
		long thisArc = mSigArcs[i];
		long thatArc = this->GetContrastArc(thisArc);

		vector<long> thisArcNodeIdx;
		vector<long> thatArcNodeIdx;
		this->GetArcVoxesIndices(thisArc, thisArcNodeIdx);
		mContrastContourTree->GetArcVoxesIndices(thatArc, thatArcNodeIdx);
		sort(thisArcNodeIdx.begin(), thisArcNodeIdx.end());
		sort(thatArcNodeIdx.begin(), thatArcNodeIdx.end());

		float minValue = 999, maxValue = -999;
		const vector<float*>& thisVoxes = this->mArcNodes[thisArc];
		for (int i = 0; i < thisVoxes.size(); i++){
			if (*thisVoxes[i] < minValue) minValue = *thisVoxes[i];
			if (*thisVoxes[i] > maxValue) maxValue = *thisVoxes[i];
		}
		const vector<float*>& thatVoxes = mContrastContourTree->mArcNodes[thatArc];
		for (int i = 0; i < thatVoxes.size(); i++){
			if (*thatVoxes[i] < minValue) minValue = *thatVoxes[i];
			if (*thatVoxes[i] > maxValue) maxValue = *thatVoxes[i];
		}

		vector<long> combinedNodeIdx;
		CombineIndicesSorted(thisArcNodeIdx, thatArcNodeIdx, combinedNodeIdx);

		ComputeDistribution(this->height, combinedNodeIdx, 
			mArcDiffHistogram[thisArc].mLeft, minValue, maxValue);
		ComputeDistribution(mContrastContourTree->height, combinedNodeIdx,
			mArcDiffHistogram[thisArc].mRight, minValue, maxValue);
	}
}


bool MyContourTree::IsArcSig(long arc) const{
	for (int i = 0; i < mSigArcs.size(); i++){
		if (mSigArcs[i] == arc) return true;
	}
	return false;
}

void MyContourTree::ComputeDistribution(Array3D<float>& vol, const vector<long>& indices,
	SimpleDistribution& distribution, float minValue, float maxValue){
	distribution.mMin = minValue;
	distribution.mMax = maxValue;
	if (indices.empty()){
		distribution.mKernelDensity = vector<float>(0);
	}
	else{
		int numBins = (maxValue - minValue) / mBinWidth + 1;
		distribution.mKernelDensity = vector<float>(numBins, 0);
		float* startPtr = &vol(0, 0, 0);
		for (long i = 0; i < indices.size(); i++){
			float value = *(startPtr+indices[i]);
			// use Gaussian Kernel for density estimation
			int startBinIdx = (value - 3 * mSigma - minValue) / mBinWidth;
			startBinIdx = (startBinIdx < 0 ? 0 : startBinIdx);
			int endBinIdx = (value + 3 * mSigma - minValue) / mBinWidth;
			endBinIdx = (endBinIdx >= numBins - 1 ? numBins - 1 : endBinIdx);
			vector<float> weights;
			for (int binIdx = startBinIdx; binIdx <= endBinIdx; binIdx++){
				float dist = minValue + binIdx*mBinWidth - value;
				float zdist = dist / mSigma;
				weights.push_back(exp2f(-zdist*zdist));
			}
			float weightSum = 0;
			for (int i = 0; i < weights.size(); i++){
				weightSum += weights[i];
			}
			if (weightSum > 0){
				for (int binIdx = startBinIdx; binIdx <= endBinIdx; binIdx++){
					distribution.mKernelDensity[binIdx] += weights[binIdx - startBinIdx] / weightSum;
				}
			}
		}
	}
}


float MyContourTree::GetHistogramWidth(
	const SimpleDistribution& hist, MappingScale scale, float arcZoom) const{
	const vector<float>& histogram = hist.mKernelDensity;
	if (histogram.empty()){
		return GetDrawingHeight(0, scale)*arcZoom;
	}
	else{
		if (scale == MappingScale_Sci){
			float maxWidth = 0;
			for (int i = 0; i < histogram.size(); i++){
				float width = GetDrawingHeight(histogram[i], MappingScale_Sci);
				maxWidth = max(width, maxWidth);
			}
			return maxWidth*arcZoom;
		}
		else{
			return GetDrawingHeight(*max_element(histogram.begin(), histogram.end()), scale)*arcZoom;
		}
	}
}

void MyContourTree::SyncSigArcsTo(MyContourTree* ct){
	mContrastContourTree = ct;
	mSigArcs.clear();
	UpdateArcMapping();
	mDiffMode = true;
	for (std::map<long, long>::iterator itr = mArcMap.begin();
		itr != mArcMap.end(); itr ++){
		if (itr->second >= 0){
			mSigArcs.push_back(itr->first);
		}
	}
	UpdatePathArcs();
	UpdateDiffHistogram();
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