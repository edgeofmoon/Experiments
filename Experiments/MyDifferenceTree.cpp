#include "MyDifferenceTree.h"
#include "ColorScaleTable.h"
#include <algorithm>
#include "MySpaceFillingNaive.h"
#include "MyPrimitiveDrawer.h"

MyContourTree* MyDifferenceTree::mTemplateTree = 0;
MyDifferenceTree::MyDifferenceTree()
{
	mCt0 = mCt0 = 0;
	mLabelDrawRatio = 1;
}


MyDifferenceTree::~MyDifferenceTree()
{
}

void MyDifferenceTree::SetContourTrees(MyContourTree* ct0, MyContourTree* ct1){
	mCt0 = ct0;
	mCt1 = ct1;
}

void MyDifferenceTree::UpdateArcMapping(){

	mArcMap.clear();
	for (long i = 0; i < mCt0->nValidArcs; i++){
		long ct0Arc = mCt0->valid[i];
		mArcMap[ct0Arc] = 1;
	}
	return;

	mArcMap.clear();
	vector<long> ct1ArcMapped;
	for (long i = 0; i < mCt0->mSigArcs.size(); i++){
		long ct0Arc = mCt0->mSigArcs[i];
		vector<long> smArcs;
		mCt1->FindSimilarValidArcs(mCt0, ct0Arc, smArcs);
		// 1 to 1 map only
		if (smArcs.size() == 1){
			long ct1Arc = smArcs[0];
			// if not already mapped
			if (find(ct1ArcMapped.begin(), ct1ArcMapped.end(), ct1Arc) 
				== ct1ArcMapped.end()){
				mArcMap[ct0Arc] = ct1Arc;
				ct1ArcMapped.push_back(ct1Arc);
			}
			// if already mapped
			else{
				// since it's now a many to 1 mapping, remove it
				for (map<long, long>::iterator itr = mArcMap.begin();
					itr != mArcMap.end(); itr++){
					if (itr->second == ct1Arc){
						mArcMap.erase(itr);
						cout << "Arc " << ct0Arc << 
							" is not the only arc mapped to its correspondance, so removed." << endl;
						break;
					}
				}
			}
		}
		else{
			cout << "Arc " << ct0Arc << " is mapped to multiple arcs, so removed." << endl;
		}
	}
	cout << "Total " << mArcMap.size() << " arcs have a 1 to 1 mapping." << endl;
	UpdatePathArcs();
}


void MyDifferenceTree::UpdateDifferenceHistogram(){
	mArcDiffHistogram.clear();
	mMinDiff = 10;
	mMaxDiff = -10;
	for (map<long, long>::const_iterator itr = mArcMap.begin();
		itr != mArcMap.end(); itr ++){
		long thisArc = itr->first;
		long thatArc = itr->second;

		vector<long> thisArcNodeIdx;
		vector<long> thatArcNodeIdx;
		mCt0->GetArcVoxesIndices(thisArc, thisArcNodeIdx);
		mCt1->GetArcVoxesIndices(thatArc, thatArcNodeIdx);
		sort(thisArcNodeIdx.begin(), thisArcNodeIdx.end());
		sort(thatArcNodeIdx.begin(), thatArcNodeIdx.end());

		float minValue = 999, maxValue = -999;
		const vector<float*>& thisVoxes = mCt0->mArcNodes[thisArc];
		for (int i = 0; i < thisVoxes.size(); i++){
			if (*thisVoxes[i] < minValue) minValue = *thisVoxes[i];
			if (*thisVoxes[i] > maxValue) maxValue = *thisVoxes[i];
		}
		const vector<float*>& thatVoxes = mCt1->mArcNodes[thatArc];
		for (int i = 0; i < thatVoxes.size(); i++){
			if (*thatVoxes[i] < minValue) minValue = *thatVoxes[i];
			if (*thatVoxes[i] > maxValue) maxValue = *thatVoxes[i];
		}

		vector<long> combinedNodeIdx;
		mCt0->CombineIndicesSorted(thisArcNodeIdx, thatArcNodeIdx, combinedNodeIdx);

		// tmp
		combinedNodeIdx = thisArcNodeIdx;
		// tmp

		MakeDiffHistogram(mCt0, mCt1, combinedNodeIdx, mArcDiffHistogram[thisArc]);
		mMinDiff = min(mMinDiff, mArcDiffHistogram[thisArc].mMin);
		mMaxDiff = max(mMaxDiff, mArcDiffHistogram[thisArc].mMax);
		mMaxHistogramCount = max(mMaxHistogramCount, *max_element(mArcDiffHistogram[thisArc].mKernelDensity.begin(),
			mArcDiffHistogram[thisArc].mKernelDensity.end()));
	}
}

void MyDifferenceTree::MakeDiffHistogram(MyContourTree* ct0, MyContourTree* ct1, 
	const vector<long> voxes, SimpleDistribution& distr){
	float binWidth = 0.01;
	float sigma = 0.05;
	float* vox0 = &(ct0->height(0, 0, 0));
	float* vox1 = &(ct1->height(0, 0, 0));
	vector<float> differences(voxes.size());
	for (int i = 0; i < voxes.size(); i++){
		float v0 = *(vox0 + voxes[i]);
		float v1 = *(vox1 + voxes[i]);
		differences[i] = v0 - v1;
	}
	distr.mMin = *min_element(differences.begin(), differences.end());
	distr.mMax = *max_element(differences.begin(), differences.end());

	// convolution
	int numBins = (distr.mMax - distr.mMin) / binWidth + 1;
	distr.mKernelDensity.resize(numBins);
	for (long i = 0; i < voxes.size(); i++){
		float value = differences[i];
		// use Gaussian Kernel for density estimation
		int startBinIdx = (value - 3 * sigma - distr.mMin) / binWidth;
		startBinIdx = (startBinIdx < 0 ? 0 : startBinIdx);
		int endBinIdx = (value + 3 * sigma - distr.mMin) / binWidth;
		endBinIdx = (endBinIdx >= numBins - 1 ? numBins - 1 : endBinIdx);
		vector<float> weights;
		for (int binIdx = startBinIdx; binIdx <= endBinIdx; binIdx++){
			float dist = distr.mMin + binIdx*binWidth - value;
			float zdist = dist / sigma;
			weights.push_back(exp2f(-zdist*zdist));
		}
		float weightSum = 0;
		for (int i = 0; i < weights.size(); i++){
			weightSum += weights[i];
		}
		if (weightSum > 0){
			for (int binIdx = startBinIdx; binIdx <= endBinIdx; binIdx++){
				distr.mKernelDensity[binIdx] += weights[binIdx - startBinIdx] / weightSum;
			}
		}
	}
}

void MyDifferenceTree::UpdatePathArcs(){
	mPathArcs.clear();
	long lowestNode = mCt0->validNodes[0];
	for (long theNodeIndex = 1; theNodeIndex < mCt0->nValidNodes; theNodeIndex++){
		long theNode = mCt0->validNodes[theNodeIndex];
		if (compareHeight(mCt0->supernodes[theNode].value, mCt0->supernodes[lowestNode].value) < 0){
			lowestNode = theNode;
		}
	}
	for (map<long, long>::iterator itr = mArcMap.begin(); itr != mArcMap.end(); itr++){
		long arc =itr->first;
		std::vector<long> path;
		mCt0->FindPathDown(mCt0->superarcs[arc].bottomID, lowestNode, path);
		for (int j = 0; j < path.size() - 1; j++){
			long pathArc = mCt0->nodes2Arc(path[j], path[j + 1]);
			mPathArcs[pathArc] = true;
		}
	}
}

MyBox2f MyDifferenceTree::GetArcBox(long arc) const{
	map<long, SimpleDistribution>::const_iterator itr = mArcDiffHistogram.find(arc);
	if (itr == mArcDiffHistogram.end()) return MyBox2f(MyVec2f(0, 0), MyVec2f(0, 0));
	long topNode = mCt0->superarcs[arc].topID, bottomNode = mCt0->superarcs[arc].bottomID;
	float xPos;
	if (mCt0->supernodesExt[topNode].numLeaves <= mCt0->supernodesExt[bottomNode].numLeaves){
		xPos = mCt0->supernodes[topNode].xPosn;
	}
	else{
		xPos = mCt0->supernodes[bottomNode].xPosn;
	}
	float yPos = mCt0->supernodes[bottomNode].yPosn;
	float yEnd = itr->second.mMax - itr->second.mMin + yPos;

	float leftHeightScale;
	float rightHeightScale;
	switch (mCt0->mHistogramSide){
	case MyContourTree::HistogramSide_Sym:
		leftHeightScale = 0.5;
		rightHeightScale = 0.5;
		break;
	case MyContourTree::HistogramSide_Left:
		leftHeightScale = 1;
		rightHeightScale = 0;
		break;
	case MyContourTree::HistogramSide_Right:
		leftHeightScale = 0;
		rightHeightScale = 1;
		break;
	}

	float width;
	const vector<float>& histogram = itr->second.mKernelDensity;
	if (mCt0->mDefaultScale == MyContourTree::MappingScale_Sci){
		float maxWidth = 0;
		for (int i = 0; i < histogram.size(); i++){
			float width = mCt0->GetDrawingHeight(histogram[i], MyContourTree::MappingScale_Sci);
			maxWidth = max(width, maxWidth);
		}
		width = maxWidth;
	}
	else{
		width = mCt0->GetDrawingHeight(*max_element(histogram.begin(), histogram.end()), mCt0->mDefaultScale);
	}
	return MyBox2f(MyVec2f(xPos, yPos), MyVec2f(xPos+width, yEnd));
}

void MyDifferenceTree::DiffValueToColor(float diff, float color_rgba[4]){
	if (diff <= 0){
		color_rgba[0] = 1;
		color_rgba[1] = 1 - diff / mMinDiff;
		color_rgba[2] = 1 - diff / mMinDiff;
	}
	else{
		color_rgba[2] = 1;
		color_rgba[1] = 1 - diff / mMaxDiff;
		color_rgba[0] = 1 - diff / mMaxDiff;
	}
	color_rgba[3] = 1;
}

bool MyDifferenceTree::IsArcMapped(long arc) const{
	return mArcMap.find(arc) != mArcMap.end();
}

void MyDifferenceTree::DrawDiffHistogram(float xPos, float yPos, SimpleDistribution& distr,
	MyContourTree::MappingScale scale, MyContourTree::HistogramSide side, float alpha){
	const vector<float>& histogram = distr.mKernelDensity;
	if (histogram.empty()) return;

	float yStart = yPos;
	float yEnd = distr.mMax-distr.mMin + yPos;
	float yStep = (yEnd - yStart) / histogram.size();

	float arcZoom = 1;
	glLineWidth(1);

	float leftHeightScale;
	float rightHeightScale;
	switch (side){
	case MyContourTree::HistogramSide_Sym:
		leftHeightScale = 0.5;
		rightHeightScale = 0.5;
		break;
	case MyContourTree::HistogramSide_Left:
		leftHeightScale = 1;
		rightHeightScale = 0;
		break;
	case MyContourTree::HistogramSide_Right:
		leftHeightScale = 0;
		rightHeightScale = 1;
		break;
	}
	float alplaModulation = alpha;
	glDepthFunc(GL_ALWAYS);
	if (scale == MyContourTree::MappingScale_Sci){
		vector<float> exponentHeights(histogram.size());
		vector<float> mantissaHeights(histogram.size());
		vector<int> exponentPos(histogram.size());
		float exponentRange = mCt0->mMaxExponent - mCt0->mMinExponent;
		for (int i = 0; i < histogram.size(); i++){
			float exponentHeight;
			float mantissaHeight;
			mCt0->GetDrawingHeightScientific(histogram[i], exponentHeight, mantissaHeight);
			exponentHeights[i] = exponentHeight*arcZoom;
			mantissaHeights[i] = mantissaHeight*arcZoom;
			if (exponentRange != 0){
				int exponent;
				float mantissa;
				mCt0->floatToScientific(histogram[i], exponent, mantissa);
				exponentPos[i] = (exponent - mCt0->mMinExponent) / exponentRange * 7 + 0.5;
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
			//glColor4f(colorBrewer_sequential_8_multihue_9[exponentPos[i]][0] / 255.f,
			//	colorBrewer_sequential_8_multihue_9[exponentPos[i]][1] / 255.f,
			//	colorBrewer_sequential_8_multihue_9[exponentPos[i]][2] / 255.f, alplaModulation);
			float color[4];
			DiffValueToColor(distr.mMin+i*yStep,color);
			glColor4f(color[0], color[1], color[2], alplaModulation);
			glVertex2f(xPos - expHeight*leftHeightScale, yStart + i*yStep);
			glVertex2f(xPos + expHeight*rightHeightScale, yStart + i*yStep);
			glVertex2f(xPos + expHeightNext*rightHeightScale, yStart + (i + 1)*yStep);
			glVertex2f(xPos - expHeightNext*leftHeightScale, yStart + (i + 1)*yStep);
			glEnd();


			glBegin(GL_LINES);
			// exponent part
			glColor4f(0, 0, 0, alplaModulation);
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
			glColor4f(0, 0, 1, alplaModulation);
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
		for (int i = 0; i < histogram.size(); i++){
			float baseHeight = mCt0->GetDrawingHeight(histogram[i], scale);
			float leftHeight = baseHeight * leftHeightScale*arcZoom;
			float rightHeight = baseHeight * rightHeightScale*arcZoom;
			glBegin(GL_QUADS);
			int color = mCt0->GetDrawingHeight(histogram[i], scale)
				/ mCt0->GetDrawingHeight(mMaxHistogramCount, scale) * 7 + 0.5;
			//float color = 0.5;
			glColor4f(colorBrewer_sequential_8_multihue_9[color][0] / 255.f,
				colorBrewer_sequential_8_multihue_9[color][1] / 255.f,
				colorBrewer_sequential_8_multihue_9[color][2] / 255.f, alplaModulation);
			glVertex2f(xPos - leftHeight, yStart + i*yStep);
			glVertex2f(xPos + rightHeight, yStart + i*yStep);
			glVertex2f(xPos + rightHeight, yStart + (i + 1)*yStep);
			glVertex2f(xPos - leftHeight, yStart + (i + 1)*yStep);
			glEnd();

			glColor4f(0, 0, 0, alplaModulation);
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
				float baseHeightNext = mCt0->GetDrawingHeight(histogram[i + 1], scale);
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

void MyDifferenceTree::DrawDiffHistogram(long arc){
	long topNode = mCt0->superarcs[arc].topID, bottomNode = mCt0->superarcs[arc].bottomID;
	float xPos;
	if (mCt0->supernodesExt[topNode].numLeaves <= mCt0->supernodesExt[bottomNode].numLeaves){
		xPos = mCt0->supernodes[topNode].xPosn;
	}
	else{
		xPos = mCt0->supernodes[bottomNode].xPosn;
	}
	float yPos = mCt0->supernodes[bottomNode].yPosn;
	float alpha = 0.3;
	if (find(mCt0->mSigArcs.begin(), mCt0->mSigArcs.end(), arc) != mCt0->mSigArcs.end()){
		alpha = 1;
	}
	DrawDiffHistogram(xPos, yPos, mArcDiffHistogram[arc], mCt0->mDefaultScale, mCt0->mHistogramSide, alpha);
}

void MyDifferenceTree::Show(){
	mCt0->DrawContourTreeFrame();
	for (long i = 0; i < mCt0->nValidArcs; i++){
		long arc = mCt0->valid[i];
		if (this->IsArcMapped(arc)){
			this->DrawDiffHistogram(arc);
		}
		//else if(mCt0->mPathArcs[arc]){
		else{
			if (mCt0->GetArcScale(arc) == MyContourTree::MappingScale_Sci){
				mCt0->DrawArcHistogramScientific(arc);
			}
			else{
				mCt0->DrawArcHistogram(arc);
			}
		}
	}

	// lines
	glColor4f(0, 0, 0, 1);
	glLineWidth(1.0);
	glBegin(GL_LINES);

	for (long whichArc = 0; whichArc < mCt0->nValidArcs; whichArc++)
	{ // loop through superarcs
		long arc = mCt0->valid[whichArc];
		//if (!IsArcMapped(arc) && !mPathArcs[arc]) continue;
		long topNode = mCt0->superarcs[arc].topID, bottomNode = mCt0->superarcs[arc].bottomID;
		if (mCt0->supernodesExt[topNode].numLeaves <= mCt0->supernodesExt[bottomNode].numLeaves){
			// horizontal line from parent always
			glVertex2f(mCt0->supernodes[bottomNode].xPosn, mCt0->supernodes[bottomNode].yPosn);
			glVertex2f(mCt0->supernodes[topNode].xPosn, mCt0->supernodes[bottomNode].yPosn);
			//if (!mPathArcs[arc]) continue;
			glVertex2f(mCt0->supernodes[topNode].xPosn, mCt0->supernodes[bottomNode].yPosn);
			glVertex2f(mCt0->supernodes[topNode].xPosn, mCt0->supernodes[topNode].yPosn);
		}
		else{
			// horizontal line from parent always
			glVertex2f(mCt0->supernodes[topNode].xPosn, mCt0->supernodes[topNode].yPosn);
			glVertex2f(mCt0->supernodes[bottomNode].xPosn, mCt0->supernodes[topNode].yPosn);
			//if (!mPathArcs[arc]) continue;
			glVertex2f(mCt0->supernodes[bottomNode].xPosn, mCt0->supernodes[topNode].yPosn);
			glVertex2f(mCt0->supernodes[bottomNode].xPosn, mCt0->supernodes[bottomNode].yPosn);
		}
	} // loop through superarcs
	glEnd();

	// label
	float cutSize = 0.005;
	//MySpaceFillingNaive spaceFill;
	MySpaceFillingSpiral spaceFill;
	for (int i = 0; i<mArcLabelSorted.size()*mLabelDrawRatio; i++){
		long arc = mArcLabelSorted[i];
		MyBox2f box = this->GetArcBox(arc);
		spaceFill.ForceAddBox(box);
	}
	// draw the leader lines
	for (int i = 0; i<mArcLabelSorted.size()*mLabelDrawRatio; i++){
		long arc = mArcLabelSorted[i];
		long topNode = mCt0->superarcs[arc].topID, bottomNode = mCt0->superarcs[arc].bottomID;
		MyBox2f box = mLabelPos[arc];
		MyPrimitiveDrawer::DrawLineAt(MyVec2f(mCt0->supernodes[topNode].xPosn, mCt0->supernodes[bottomNode].yPosn), box.GetLowPos());
	}
	void * font = GLUT_BITMAP_HELVETICA_18;
	//void * font = GLUT_BITMAP_TIMES_ROMAN_24;
	for (int i = 0; i<mArcLabelSorted.size()*mLabelDrawRatio; i++){
		long arc = mArcLabelSorted[i];

		long topNode = mCt0->superarcs[arc].topID, bottomNode = mCt0->superarcs[arc].bottomID;
		MyBox2f box = mLabelPos[arc];
		string name = mCt0->mArcName[arc];
		glColor4f(1, 1, 1, 1);
		glBegin(GL_QUADS);
		glVertex2f(box.GetLowPos()[0], box.GetLowPos()[1]);
		glVertex2f(box.GetHighPos()[0], box.GetLowPos()[1]);
		glVertex2f(box.GetHighPos()[0], box.GetHighPos()[1]);
		glVertex2f(box.GetLowPos()[0], box.GetHighPos()[1]);
		glEnd();
		glColor4f(0, 0, 0, 1);
		glBegin(GL_LINE_LOOP);
		glVertex2f(box.GetLowPos()[0], box.GetLowPos()[1]);
		glVertex2f(box.GetHighPos()[0], box.GetLowPos()[1]);
		glVertex2f(box.GetHighPos()[0], box.GetHighPos()[1]);
		glVertex2f(box.GetLowPos()[0], box.GetHighPos()[1]);
		//glVertex2f(box.GetLowPos()[0]-0.0025, box.GetLowPos()[1]-0.01);
		//glVertex2f(box.GetHighPos()[0] + 0.0025, box.GetLowPos()[1] - 0.01);
		//glVertex2f(box.GetHighPos()[0] + 0.0025, box.GetHighPos()[1]+0.01);
		//glVertex2f(box.GetLowPos()[0] - 0.0025, box.GetHighPos()[1] + 0.01);
		glEnd();
		glRasterPos2f(box.GetLowPos()[0], box.GetLowPos()[1] + 0.01);
		glutBitmapString(font, (const unsigned char*)name.c_str());
	}

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	float pixelWidth = 1.1f / viewport[2];
	float pixelHeight = 1.1f / (viewport[3] / 3);
	glColor4f(0, 0, 0, 1);
	glRasterPos2f(0.01, 1.02);
	string diffName("Difference Tree");
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)diffName.c_str());
	glRasterPos2f(0.01 + pixelWidth / 2, 1.02 + pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)diffName.c_str());
	glRasterPos2f(0.01 - pixelWidth / 2, 1.02 - pixelHeight / 2);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)diffName.c_str());
}

void MyDifferenceTree::ShowLegend(){
	glBegin(GL_QUAD_STRIP);
	int nSteps = 50;
	for (int i = 0; i < nSteps; i++){
		float value = (mMaxDiff - mMinDiff) / nSteps * i + mMinDiff;
		float color[4];
		DiffValueToColor(value, color);
		glColor4fv(color);
		glVertex2f(i / (float)nSteps, 0);
		glVertex2f(i / (float)nSteps, 1);
	}
	glEnd();

	glColor4f(0, 0, 0, 1);
	glRasterPos2f(0, -1);
	string vstr = to_string(mMinDiff);
	vstr.resize(5);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)vstr.c_str());
	glRasterPos2f(1, -1);
	vstr = to_string(mMaxDiff);
	vstr.resize(5);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)vstr.c_str());


	glRasterPos2f(0, 1);
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"Difference");
}

void MyDifferenceTree::UpdateLabels(int width, int height){
	mLabelPos.clear();
	mArcLabelSorted.clear();
	MySpaceFillingSpiral spaceFill;
	for (map<long, long>::iterator itr = mArcMap.begin(); itr != mArcMap.end(); itr++){
		long arc = itr->first;
		if (mCt0->IsArcSig(arc)){
			mArcLabelSorted.push_back(arc);
		}
	}

	mTemplateTree = mCt0;
	sort(mArcLabelSorted.begin(), mArcLabelSorted.end(), compareArcMore);

	for (map<long, long>::iterator itr = mArcMap.begin(); itr != mArcMap.end(); itr++){
		long arc = itr->first;
		MyBox2f box = this->GetArcBox(arc);
		spaceFill.ForceAddBox(box);
	}

	// label
	float cutSize = 0.005;
	float pixelWidth = 1.1f / width;
	float pixelHeight = 1.1f / height * 2; // why times 2?
	void * font = GLUT_BITMAP_HELVETICA_18;
	for (int i = 0; i<mArcLabelSorted.size()*mLabelDrawRatio; i++){
		long arc = mArcLabelSorted[i];

		long topNode = mCt0->superarcs[arc].topID, bottomNode = mCt0->superarcs[arc].bottomID;

		// draw label
		if (mCt0->mLabelVolume){
			string name = mCt0->mArcName[arc];
			//if (supernodes[topNode].IsLowerLeaf() || supernodes[topNode].IsUpperLeaf()){
			float length = 0;
			for (int i = 0; i < name.size(); i++){
				length += glutBitmapWidth(font, name[i]);
			}
			MyVec2f lowPos(mCt0->supernodes[topNode].xPosn - length / 2 * pixelWidth,
				mCt0->supernodes[bottomNode].yPosn + cutSize + mArcDiffHistogram[arc].mMax - mArcDiffHistogram[arc].mMin);
			MyVec2f highPos = lowPos + MyVec2f(length * pixelWidth, glutBitmapHeight(font)*pixelHeight/2);
			//MyBox2f box = spaceFill.PushBoxFromTop(MyBox2f(lowPos, highPos), 0.0001);
			MyBox2f box = spaceFill.PushBox(MyBox2f(lowPos, highPos), MyVec2f(0.005, 0));
			mLabelPos[arc] = box;
		}
	}
}

bool MyDifferenceTree::compareArcMore(long arc0, long arc1){
	return mTemplateTree->mArcNodes[arc0].size() > mTemplateTree->mArcNodes[arc1].size();
}