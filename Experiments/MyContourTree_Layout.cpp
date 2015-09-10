#include "MyContourTree.h"
#include "Ric\RicVolume.h"
#include "MyPrimitiveDrawer.h"
#include "ColorScaleTable.h"
#include "MyBitmap.h"
#include <vector>
#include <algorithm>
using namespace std;

void MyContourTree::updateArcHistogram(long arc){
	vector<float*>& voxes = mArcNodes[arc];
	if (voxes.empty()){
		mArcHistogram[arc] = vector<float>(0);
	}
	else{
		mBinWidth = 0.005;
		mSigma = 0.01;
		float minValue = *supernodes[superarcs[arc].bottomID].value;
		float maxValue = *supernodes[superarcs[arc].topID].value;
		int numBins = (maxValue - minValue) / mBinWidth + 1;
		vector<float> histogram(numBins, 0);
		for (long i = 0; i < voxes.size(); i++){
			float value = *voxes[i];
			// use Gaussian Kernel for density estimation
			int startBinIdx = (value - 3 * mSigma - minValue) / mBinWidth;
			startBinIdx = (startBinIdx < 0 ? 0 : startBinIdx);
			int endBinIdx = (value + 3 * mSigma - minValue) / mBinWidth;
			endBinIdx = (endBinIdx >= numBins - 1 ? numBins - 1 : endBinIdx);
			vector<float> weights;
			for (int binIdx = startBinIdx; binIdx <= endBinIdx; binIdx ++){
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
					histogram[binIdx] += weights[binIdx-startBinIdx]/weightSum;
				}
			}
		}

		///remove low values
		//for (int i = 0; i < histogram.size(); i++){
		//	if (histogram[i] < 0.01){
		//		histogram[i] = 0;
		//	}
		//}

		mArcHistogram[arc] = histogram;
	}
}

void MyContourTree::updateArcHistograms(){
	mArcHistogram.clear();
	mMaxHistogramCount = 0;
	for (int arc = 0; arc < nValidArcs; arc++){
		long whichArc = valid[arc];
		updateArcHistogram(whichArc);
		if (!mArcHistogram[whichArc].empty()){
			mMaxHistogramCount = max(mMaxHistogramCount, *max_element(mArcHistogram[whichArc].begin(), mArcHistogram[whichArc].end()));
		}
	}
	cout << "Max Histogram Count: " << mMaxHistogramCount << endl;
	updateScientificHistograms();
}

MyContourTree::MappingScale MyContourTree::GetArcScale(long arc) const{
	std::map<long, char>::const_iterator it = mArcStatus.find(arc);
	if (it != mArcStatus.end()){
		if (it->second & ArcStatus_InComaprison){
			return mAltScale;
		}
	}
	return mDefaultScale;
}

float MyContourTree::GetArcZoomLevel(long arc) const{
	std::map<long, char>::const_iterator it = mArcStatus.find(arc);
	if (it != mArcStatus.end()){
		if (it->second & ArcStatus_InComaprison){
			return mZoomLevel;
		}
	}
	return 1;
}

float MyContourTree::getArcWidth(long arc) const{
	if (IsDiffMode()){
		if (IsArcSig(arc)){
			MappingScale scale = GetArcScale(arc);
			float arcZoom = GetArcZoomLevel(arc);
			std::map<long, DiffHistogram>::const_iterator itr = mArcDiffHistogram.find(arc);
			if (itr != mArcDiffHistogram.end()){
				float width1 = GetHistogramWidth(itr->second.mLeft, scale, arcZoom);
				float width2 = GetHistogramWidth(itr->second.mRight, scale, arcZoom);
				return width1 + width2;
			}
			else return mNonSigArcWidth;
		}
		else{
			return mNonSigArcWidth;
		}
	}
	else{
		MappingScale scale = GetArcScale(arc);
		return GetArcWidth(arc, scale);
	}
}

float MyContourTree::GetArcWidth(long arc, MappingScale scale) const{
	std::map<long, std::vector<float>>::const_iterator it
		= mArcHistogram.find(arc);
	const vector<float>& histogram = it->second;
	float arcZoom = GetArcZoomLevel(arc);
	if (histogram.empty()){
		cout << "Arc " << arc << " has no vertex.\n";
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

void MyContourTree::getSubArcs(long rootNode, long parentArc, std::vector<long>& subArcs){
	// up
	if (supernodes[rootNode].upDegree > 0){
		long theArc = supernodes[rootNode].upList;
		for (int i = 0; i < supernodes[rootNode].upDegree; i++){
			long otherNode = superarcs[theArc].topID;
			if (theArc != parentArc){
				subArcs.push_back(theArc);
			}
			theArc = superarcs[theArc].nextUp;
		}
	}
	// down
	if (supernodes[rootNode].downDegree > 0){
		long theArc = supernodes[rootNode].downList;
		for (int i = 0; i < supernodes[rootNode].downDegree; i++){
			long otherNode = superarcs[theArc].bottomID;
			if (theArc != parentArc){
				subArcs.push_back(theArc);
			}
			theArc = superarcs[theArc].nextDown;
		}
	}
}

float MyContourTree::getSubTreeWidth(long rootNode, long parentNode){
	vector<long> subArcs;
	long parentArc = nodes2Arc(rootNode, parentNode);
	if (parentArc < 0){
		cout << "SubTree Defined by node " << rootNode << " and node " << parentNode << " is invalid.\n";
	}
	float parentArcWidth = getArcWidth(parentArc);
	getSubArcs(rootNode, parentArc, subArcs);
	if (subArcs.empty()){
		return parentArcWidth;
	}
	else{
		float widthSum = 0;
		for (int i = 0; i < subArcs.size(); i++){
			float subWidth = getArcWidth(subArcs[i]);
			widthSum += subWidth;
		}
		return max(parentArcWidth, widthSum);
	}
}

bool MyContourTree::IsNameLeft(std::string name) const{
	if (name.size() < 2) return false;
	char hyp = name[name.size() - 2];
	if (hyp == '-'){
		if (name.back() == 'L'){
			return true;
		}
	}
	return false;
}

bool MyContourTree::IsNameRight(std::string name) const{
	if (name.size() < 2) return false;
	char hyp = name[name.size() - 2];
	if (hyp == '-'){
		if (name.back() == 'R'){
			return true;
		}
	}
	return false;
}

float MyContourTree::subTreeLayoutWidth(long rootNode, long parentNode){
	//int numLeaves = supernodesExt[rootNode].numLeaves;
	if (supernodesExt[rootNode].subTreeLayoutWidth > 0){
		return supernodesExt[rootNode].subTreeLayoutWidth;
	}
	vector<long> centerPath;
	//FindHighestPath(rootNode, parentNode, centerPath, supernodes[rootNode].value);
	if (parentNode >= 0){
		if (supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf()){
			long arc = nodes2Arc(rootNode, parentNode);
			return getArcWidth(arc);
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

	vector<bool> isLeft(rootNodes.size(), false);
	bool t_isLeft = false;
	map<long, float> brunchStartX;
	// decide left and right
	for (int i = 0; i < brunchNodes.size(); i++){
		isLeft[i] = isBrunchLeft(brunchNodes[i], rootNodes[i]);
	}

	float centerPathWidth = getPathWidth(centerPath, parentNode);
	// assuming 0 offset for now
	vector<MyVec2f> leftUpperBottomFilled(1, MyVec2f(centerPathWidth / 2, MinHeight()));
	vector<MyVec2f> rightUpperBottomFilled(1, MyVec2f(centerPathWidth / 2, MinHeight()));;
	vector<MyVec2f> leftDowmTopFilled(1, MyVec2f(centerPathWidth / 2, MaxHeight()));;
	vector<MyVec2f> rightDownTopFilled(1, MyVec2f(centerPathWidth / 2, MaxHeight()));;

	float maxLeft = centerPathWidth / 2;
	float maxRight = centerPathWidth / 2;

	// upper slots
	// from high to low
	for (int i = 0; i < downStart; i++){
		long brunchNode = brunchNodes[i];
		float subTreeWidth = subTreeLayoutWidth(brunchNode, rootNodes[i]);
		if (brunchNode>0){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (isLeft[i]){
					float xPos = fillUpper(leftUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
				else{
					float xPos = fillUpper(rightUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxRight = max(maxRight, xPos + subTreeWidth);
				}
			}
			else {
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
					float xPos = fillUpper(leftUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
			}
		}
	}
	// dowm slots
	// from low to high
	for (int i = brunchNodes.size() - 1; i >= mixStart; i--){
		long brunchNode = brunchNodes[i];
		float subTreeWidth = subTreeLayoutWidth(brunchNode, rootNodes[i]);
		if (brunchNode>0){
			if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
				}
				else{
					float xPos = fillBottom(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxRight = max(maxRight, xPos + subTreeWidth);
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (isLeft[i]){
					float xPos = fillBottom(leftDowmTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
				else{
					float xPos = fillBottom(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
			}
		}
	}
	float totalWidth = maxLeft + maxRight;
	supernodesExt[rootNode].subTreeLayoutWidth = totalWidth;
	return totalWidth;
}

bool MyContourTree::isBrunchLeft(long brunchNode, long rootNode){
	float *value = supernodes[brunchNode].value;
	long x, y, z;
	height.ComputeIndex(value, x, y, z);
	int label = mLabelVolume->get_at_index(x, y, z);
	string name = mArcName[nodes2Arc(brunchNode, rootNode)];
	if (IsNameLeft(name)) return true;
	else if (IsNameRight(name)) return false;
	else if (label % 2 == 1){
		return false;
	}
	else{
		return true;
	}
}

float MyContourTree::UpdateSubTreeLayout(long rootNode, long parentNode, float xStart, float xEnd){
	int numLeaves = supernodesExt[rootNode].numLeaves;
	vector<long> centerPath;
	//FindHighestPath(rootNode, parentNode, centerPath, supernodes[rootNode].value);
	if (parentNode >= 0){
		if (supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf()){
			//supernodes[rootNode].xPosn = (xStart+xEnd)/2;
			switch (mHistogramSide){
				case HistogramSide_Sym:
					supernodes[rootNode].xPosn = (xStart+xEnd)/2;
					break;
				case HistogramSide_Left:
					supernodes[rootNode].xPosn = xEnd;
					break;
				case HistogramSide_Right:
					supernodes[rootNode].xPosn = xStart;
					break;
			}
			return (xEnd-xStart)/getArcWidth(nodes2Arc(rootNode, parentNode));
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

	vector<bool> isLeft(rootNodes.size(), false);
	bool t_isLeft = false;
	map<long, float> brunchStartX;
	// decide left and right
	for (int i = 0; i < brunchNodes.size(); i++){
		isLeft[i] = isBrunchLeft(brunchNodes[i], rootNodes[i]);
	}

	float centerPathWidth = getPathWidth(centerPath, parentNode);

	float maxLeft;
	float maxRight;
	switch (mHistogramSide){
	case HistogramSide_Sym:
		maxLeft = centerPathWidth / 2;
		maxRight = centerPathWidth / 2;
		break;
	case HistogramSide_Left:
		// then the brunch line is on the right side of the histogram
		maxLeft = centerPathWidth;
		maxRight = 0;
		break;
	case HistogramSide_Right:
		// then the brunch line is on the left side of the histogram
		maxLeft = 0;
		maxRight = centerPathWidth;
		break;
	}

	// assuming 0 offset for now
	vector<MyVec2f> leftUpperBottomFilled(1, MyVec2f(maxLeft, MinHeight()));
	vector<MyVec2f> rightUpperBottomFilled(1, MyVec2f(maxRight, MinHeight()));;
	vector<MyVec2f> leftDowmTopFilled(1, MyVec2f(maxLeft, MaxHeight()));;
	vector<MyVec2f> rightDownTopFilled(1, MyVec2f(maxRight, MaxHeight()));;

	// upper slots
	// from high to low
	for (int i = 0; i < downStart; i++){
		long brunchNode = brunchNodes[i];
		//float subTreeWidth = getSubTreeWidth(brunchNode, rootNodes[i]);
		float subTreeWidth = subTreeLayoutWidth(brunchNode, rootNodes[i]);
		long arc = nodes2Arc(brunchNode, rootNodes[i]);
		if (brunchNode>0){
			if (i < mixStart){
				// up-zone
				// left-right inter-changable
				if (isLeft[i]){
					 float xPos = fillUpper(leftUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						subTreeWidth);
					 brunchStartX[brunchNode] = xPos;
					 maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
				else{
					float xPos = fillUpper(rightUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxRight = max(maxRight, xPos + subTreeWidth);
				}
			}
			else {
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
					float xPos = fillUpper(leftUpperBottomFilled,
						*supernodes[rootNodes[i]].value,
						*supernodesExt[brunchNode].maxHeight,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
			}
		}
	}
	// dowm slots
	// from low to high
	for (int i = brunchNodes.size() - 1; i >= mixStart; i--){
		long brunchNode = brunchNodes[i];
		//float subTreeWidth = getSubTreeWidth(brunchNode, rootNodes[i]);
		float subTreeWidth = subTreeLayoutWidth(brunchNode, rootNodes[i]);
		if (brunchNode>0){
			if (i < downStart){
				// mix-zone
				// up brunch at left
				if (compareHeight(supernodes[rootNodes[i]].value, supernodes[brunchNode].value)<0){
				}
				else{
					float xPos = fillBottom(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxRight = max(maxRight, xPos + subTreeWidth);
				}
			}
			else {
				// down-zone
				// left-right inter-changable
				if (isLeft[i]){
					float xPos = fillBottom(leftDowmTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
				else{
					float xPos = fillBottom(rightDownTopFilled,
						*supernodesExt[brunchNode].maxHeight,
						*supernodes[rootNodes[i]].value,
						subTreeWidth);
					brunchStartX[brunchNode] = xPos;
					maxLeft = max(maxLeft, xPos + subTreeWidth);
				}
			}
		}
	}

	// calculate offset and scale
	float totalWidth = maxLeft + maxRight;
	float scaleWidth = (xEnd - xStart) / totalWidth;
	//cout << "scale width = " << scaleWidth << endl;
	float centerLine = maxLeft* scaleWidth + xStart;
	/*
	switch (mHistogramSide){
		case HistogramSide_Sym:
			centerLine = maxLeft* scaleWidth + xStart;
			break;
		case HistogramSide_Left:
			// then the brunch line is on the right side of the histogram
			centerLine = (maxLeft + centerPathWidth / 2)* scaleWidth + xStart;
			break;
		case HistogramSide_Right:
			// then the brunch line is on the left side of the histogram
			centerLine = (maxLeft - centerPathWidth / 2)* scaleWidth + xStart;
			break;
	}
	*/
	for (int i = 0; i < centerPath.size(); i++){
		long thisNode = centerPath[i];
		supernodes[thisNode].xPosn = centerLine;
	}

	// layout brunches
	for (int i = 0; i < brunchNodes.size(); i++){
		long brunchNode = brunchNodes[i];
		//float subTreeWidth = getSubTreeWidth(brunchNode, rootNodes[i]);
		float subTreeWidth = subTreeLayoutWidth(brunchNode, rootNodes[i]);
		if (brunchNode>0){
			if (isLeft[i]){
				float xPosEnd = centerLine - brunchStartX[brunchNode] * scaleWidth;
				float xPosStart = xPosEnd - subTreeWidth*scaleWidth;
				UpdateSubTreeLayout(brunchNode, rootNodes[i], xPosStart, xPosEnd);
			}
			else{
				float xPosStart = centerLine + brunchStartX[brunchNode] * scaleWidth;
				float xPosEnd = xPosStart + subTreeWidth*scaleWidth;
				UpdateSubTreeLayout(brunchNode, rootNodes[i], xPosStart, xPosEnd);
			}
		}
	}
	return scaleWidth;
}

long MyContourTree::nodes2Arc(long node1, long node2){
	// up
	if (supernodes[node1].upDegree > 0){
		long theArc = supernodes[node1].upList;
		for (int i = 0; i < supernodes[node1].upDegree; i++){
			long otherNode = superarcs[theArc].topID;
			if (otherNode == node2){
				return theArc;
			}
			theArc = superarcs[theArc].nextUp;
		}
	}
	// down
	if (supernodes[node1].downDegree > 0){
		long theArc = supernodes[node1].downList;
		for (int i = 0; i < supernodes[node1].downDegree; i++){
			long otherNode = superarcs[theArc].bottomID;
			if (otherNode == node2){
				return theArc;
			}
			theArc = superarcs[theArc].nextDown;
		}
	}
	return -1;
}

float MyContourTree::getPathWidth(vector<long>& pathNodes, long parentNode){
	float width = 0;
	if (pathNodes.size() == 0) return 0;
	if (parentNode > 0){
		long arc = nodes2Arc(pathNodes[0], parentNode);
		if (arc < 0){
			arc = nodes2Arc(pathNodes.back(), parentNode);
		}
		if (arc < 0){
			cout << "Cannot Find Arc Connecting Node " << pathNodes.back() << " & Node " << parentNode << endl;
		}
		width = getArcWidth(arc);
	}
	for (int i = 1; i < pathNodes.size(); i++){
		long arc = nodes2Arc(pathNodes[i - 1], pathNodes[i]);
		float wid = getArcWidth(arc);
		if (wid>width) width = wid;
	}
	return width;
}

// assume upperFilled is sorted as y goes down
float MyContourTree::fillUpper(vector<MyVec2f>& upperFilled, float bottom, float top, float width){
	if (upperFilled.size() == 0){
		upperFilled.push_back(MyVec2f(width, bottom));
		return 0;
	}
	float xPos = upperFilled.front()[0];
	for (int i = upperFilled.size() - 1; i >= 0; i--){
		if (upperFilled[i][0] <= xPos) continue;
		if (upperFilled[i][1] > top){
			upperFilled.push_back(MyVec2f(xPos + width, bottom));
			return xPos;
		}
		xPos = max(xPos, upperFilled[i][0]);
	}
	upperFilled.push_back(MyVec2f(xPos + width, bottom));
	return xPos;
}

// assume upperFilled is sorted as y goes up
float MyContourTree::fillBottom(vector<MyVec2f>& bottomFilled, float bottom, float top, float width){
	if (bottomFilled.size() == 0){
		bottomFilled.push_back(MyVec2f(width, top));
		return 0;
	}
	float xPos = bottomFilled.front()[0];
	for (int i = bottomFilled.size() - 1; i >= 0; i--){
		if (bottomFilled[i][0] <= xPos) continue;
		if (bottomFilled[i][1] < bottom){
			bottomFilled.push_back(MyVec2f(xPos + width, top));
			return xPos;
		}
		xPos = max(xPos, bottomFilled[i][0]);
	}
	bottomFilled.push_back(MyVec2f(xPos + width, top));
	return xPos;
}

void MyContourTree::DrawLegend(MyVec2f lowPos, MyVec2f highPos){
	vector<float> histogram;
	switch (mDefaultScale)
	{
	case MyContourTree::MappingScale_Linear:
		for (int i = 0; i <= 10; i++){
			histogram.push_back((10 - i) / 10.f*mMaxHistogramCount/7);
		}
		break;
	case MyContourTree::MappingScale_Sci:
		for (int i = 0; i <= 10; i++){
			histogram.push_back(pow(mMaxHistogramCount, (10 - i) / 10.f));
		}
		break;
	case MyContourTree::MappingScale_Log:
	default:
		for (int i = 0; i <= 10; i++){
			histogram.push_back(pow(mMaxHistogramCount, (10 - i) / 10.f));
		}
		break;
	}
	float xPos;
	switch (mHistogramSide)
	{
	case MyContourTree::HistogramSide_Sym:
		xPos = (lowPos[0] + highPos[0]) / 2;
		break;
	case MyContourTree::HistogramSide_Left:
		xPos = highPos[0];
		break;
	case MyContourTree::HistogramSide_Right:
		xPos = lowPos[0];
		break;
	default:
		break;
	}

	float yStart = lowPos[1];
	float yEnd = highPos[1];
	float yStep = (yEnd - yStart) / histogram.size();

	MappingScale arcScale = mDefaultScale;
	float arcZoom = 1;

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

	if (mDefaultScale == MappingScale_Sci){
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
				exponentPos[i] = exponent - mMinExponent;
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
			glColor4f(0, 0, 0, 1);
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

			glColor4f(0, 0, 0, 1);
			glRasterPos2f(xPos + exponentHeights.front()* rightHeightScale + 0.01, yStart + i*yStep);
			string str = to_string(histogram[i]);
			str.resize(6);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)str.c_str());
		}
	}
	else{
		glDepthFunc(GL_ALWAYS);
		float maxBaseHeight = GetDrawingHeight(histogram.front(), mDefaultScale);
		for (int i = 0; i < histogram.size(); i++){
			float baseHeight = GetDrawingHeight(histogram[i], arcScale);
			float leftHeight = baseHeight * leftHeightScale*arcZoom;
			float rightHeight = baseHeight * rightHeightScale*arcZoom;
			glBegin(GL_QUADS);
			//float color = 1 - baseHeight / maxBaseHeight;
			//float color = 0.5;
			//glColor4f(color, color, color, mContourTreeAlpha);
			int color = GetDrawingHeight(histogram[i], mDefaultScale)
				/ GetDrawingHeight(mMaxHistogramCount, mDefaultScale) * 7 + 0.5;
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


			glColor4f(0, 0, 0, 1);
			glRasterPos2f(xPos + maxBaseHeight*rightHeightScale*arcZoom+0.01, yStart + i*yStep);
			string str = to_string(histogram[i]);
			str.resize(6);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)str.c_str());
		}
		glDepthFunc(GL_LESS);

	}
}

void MyContourTree::DrawLegendSimple(MyVec2f lowPos, MyVec2f highPos){
	float tickHeight = 0.01;
	float height;
	float height1;
	float count = mMaxHistogramCount;
	if (mDefaultScale != MyContourTree::MappingScale_Sci){
		if (mDefaultScale == MyContourTree::MappingScale_Linear){
			count /= 7;
		}
		height = GetDrawingHeight(count, mDefaultScale);

		glColor4f(0, 0, 0, 1);
		glBegin(GL_LINE_STRIP);
		glVertex2f(lowPos[0], lowPos[1] + tickHeight);
		glVertex2f(lowPos[0], lowPos[1]);
		glVertex2f(lowPos[0] + height, lowPos[1]);
		glVertex2f(lowPos[0] + height, lowPos[1] + tickHeight);
		glEnd();
		glRasterPos2f(lowPos[0], lowPos[1] + tickHeight);
		string str = to_string(count);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)str.c_str());
	}
	else{
		height = GetDrawingHeight(count, mDefaultScale);
		count = mMaxMantissa*pow(10, mMaxExponent);
		float exponentHeight;
		float mantissaHeight;
		GetDrawingHeightScientific(count, exponentHeight, mantissaHeight);


		int viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		glColor4f(0, 0, 1, 1);
		glBegin(GL_LINE_STRIP);
		glVertex2f(lowPos[0], lowPos[1] + tickHeight);
		glVertex2f(lowPos[0], lowPos[1]);
		glVertex2f(lowPos[0] + mantissaHeight, lowPos[1]);
		glVertex2f(lowPos[0] + mantissaHeight, lowPos[1] + tickHeight);
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(lowPos[0] + mScientificWidthScale, lowPos[1]);
		glVertex2f(lowPos[0] + mScientificWidthScale, lowPos[1] + tickHeight);
		glEnd();
		glRasterPos2f(lowPos[0], lowPos[1] + tickHeight);
		// set max mantissa = 10;
		string str = to_string(10);
		str.resize(2);
		str = "Mantissa";
		glRasterPos2f(lowPos[0], lowPos[1] + tickHeight+0.01);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)str.c_str());
		glRasterPos2f(lowPos[0] 
			- glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char *)"0") / (float)viewport[2],
			lowPos[1] - 0.025);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"0");
		glRasterPos2f(lowPos[0] + mantissaHeight, lowPos[1] - 0.025);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)"10");

		float offsety = (highPos[1] - lowPos[1]) / 2;
		float offsetx = (highPos[0] - lowPos[0]) / 2;
		offsetx = 0;
		glColor4f(0, 0, 0, 1);
		glBegin(GL_LINE_STRIP);
		glVertex2f(lowPos[0] + offsetx, lowPos[1] + offsety + tickHeight);
		glVertex2f(lowPos[0] + offsetx, lowPos[1] + offsety);
		glVertex2f(lowPos[0] + offsetx + exponentHeight, lowPos[1] + offsety);
		glVertex2f(lowPos[0] + offsetx + exponentHeight, lowPos[1] + offsety + tickHeight);
		glEnd();
		glBegin(GL_LINES);
		glVertex2f(lowPos[0] + offsetx + mScientificWidthScale, lowPos[1] + offsety);
		glVertex2f(lowPos[0] + offsetx + mScientificWidthScale, lowPos[1] + offsety + tickHeight);
		glEnd();
		str = "Exponent";
		glRasterPos2f(lowPos[0] + offsetx, lowPos[1] + offsety + tickHeight+0.01);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)str.c_str());
		glRasterPos2f(lowPos[0] + offsetx - 
			glutBitmapLength(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char *)to_string(mMinExponent).c_str())/(float)viewport[2],
			lowPos[1] + offsety - 0.05);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)to_string(mMinExponent).c_str());
		glRasterPos2f(lowPos[0] + offsetx + mantissaHeight, lowPos[1] + offsety - 0.05);
		glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)to_string(mMaxExponent).c_str());

		// color quads
		float quadOffsetY = -0.025;
		float quadHeight = 0.020;
		for (int i = 0; i < 8; i++){
			glBegin(GL_QUADS);
			glColor4ub(colorBrewer_sequential_8_multihue_9[i][0], 
				colorBrewer_sequential_8_multihue_9[i][1], 
				colorBrewer_sequential_8_multihue_9[i][2], 255);
			glVertex2f(lowPos[0] + offsetx + exponentHeight / 8.f * i, lowPos[1] + offsety + quadOffsetY);
			glVertex2f(lowPos[0] + offsetx + exponentHeight / 8.f * (i + 1), lowPos[1] + offsety + quadOffsetY);
			glVertex2f(lowPos[0] + offsetx + exponentHeight / 8.f * (i + 1), lowPos[1] + offsety + quadOffsetY + quadHeight);
			glVertex2f(lowPos[0] + offsetx + exponentHeight / 8.f * i, lowPos[1] + offsety + quadOffsetY + quadHeight);
			glEnd();
		}
	}
}

float MyContourTree::SuggestAltMappingWidthScaleModifier() const{
	float minScale = -1;
	for (std::map<long, char>::const_iterator it = mArcStatus.begin();
		it != mArcStatus.end(); it++){
		if (it->second & ArcStatus_InComaprison){
			float height = GetArcWidth(it->first, mDefaultScale);
			float altHeight = GetArcWidth(it->first, mAltScale);
			float scale = height / altHeight;
			if (minScale <= 0){
				minScale = scale;
			}
			else minScale = min(minScale, scale);
		}
	}
	return minScale;
}

float MyContourTree::GetAltMappingWidthScale() const{
	switch (mAltScale)
	{
	case MappingScale_Linear:
		return mLinearWidthScale;
		break;
	case MappingScale_Log:
		return mLogWidthScale;
		break;
	default:
	case MappingScale_Sci:
		return mScientificWidthScale;
		break;
	}
}

void MyContourTree::SetAltMappingScale(float scale){
	switch (mAltScale)
	{
	case MappingScale_Linear:
		mLinearWidthScale = scale;
		break;
	case MappingScale_Log:
		mLogWidthScale = scale;
		break;
	case MappingScale_Sci:
		mScientificWidthScale = scale;
		break;
	default:
		break;
	}
}

bool MyContourTree::IsArcCompared(long arc) const{
	/*
	for (int i = 0; i < mArcCompared.size(); i++){
		if (mArcCompared[i] == arc) return true;
	}
	return false;
	*/
	std::map<long, char>::const_iterator it = mArcStatus.find(arc);
	if (it != mArcStatus.end()){
		return it->second & ArcStatus_InComaprison;
	}
	return false;
}

void MyContourTree::AddComparedArc(long arc){
	mArcStatus[arc] |= ArcStatus_InComaprison;
}

void MyContourTree::RemoveComparedArc(long arc){
	mArcStatus[arc] &= ~ArcStatus_InComaprison;
}

void MyContourTree::ClickComparedArc(long arc){
	if (IsArcCompared(arc)){
		RemoveComparedArc(arc);
	}
	else{
		AddComparedArc(arc);
	}
}

void MyContourTree::ClearAllArcStatus(ArcStatus status){
	for (std::map<long, char>::iterator it = mArcStatus.begin();
		it != mArcStatus.end(); ++it){
		it->second &= ~status;
	}
}

int MyContourTree::CountSameElementSorted(vector<long>& a1, vector<long>& a2){
	int i = 0;
	int j = 0;
	int count = 0;
	while (i < a1.size() && j < a2.size()){
		if (a1[i] == a2[j]){
			i++;
			j++;
			count++;
		}
		else if (a1[i] > a2[j]){
			j++;
		}
		else{
			i++;
		}
	}
	return count;
}

void MyContourTree::GetArcVoxesIndices(long arc, vector<long>& idx){
	vector<float*>& arcVoxes = mArcNodes[arc];
	float* startVoxes = &height(0, 0, 0);
	idx.resize(arcVoxes.size());
	for (int i = 0; i < arcVoxes.size(); i++){
		idx[i] = arcVoxes[i] - startVoxes;
	}
}

float MyContourTree::GetDrawingHeight(float count, MappingScale scale) const{
	if (scale == MappingScale_Linear){
		// highlighted, linear
		return count*mLinearWidthScale;
	}
	else if (scale == MappingScale_Sci){
		// scientific
		int exponent;
		float mantissa;
		floatToScientific(count, exponent, mantissa);
		return max(mantissa, (exponent + mExponent_Offset)*mExponent_Scale)*mScientificWidthScale;
	}
	else{
		// default, log scale
		return log(count + 1)*mLogWidthScale;
	}
}

void MyContourTree::FindSimilarValidArcs(MyContourTree* ct, long arc, vector<long>& arcs){
	vector<long> nodeIdx;
	ct->GetArcVoxesIndices(arc, nodeIdx);
	sort(nodeIdx.begin(), nodeIdx.end());
	for (int i = 0; i < nValidArcs; i++){
		long whichArc = valid[i];
		vector<long> nodeIdx2;
		GetArcVoxesIndices(whichArc, nodeIdx2);
		sort(nodeIdx2.begin(), nodeIdx2.end());
		int overlap = CountSameElementSorted(nodeIdx, nodeIdx2);
		if (overlap >= min(nodeIdx.size(), nodeIdx2.size())*0.5 ){
			arcs.push_back(whichArc);
		}
	}
}

int MyContourTree::CompareArcs(MyContourTree* ct){
	long count = 0;
	ClearComparedArcs();
	for (std::map<long, char>::iterator it = ct->mArcStatus.begin();
		it != ct->mArcStatus.end(); ++it){
		long ctArc = it->first;
		if (ct->IsArcCompared(ctArc)){
			vector<long> smArcs;
			FindSimilarValidArcs(ct, ctArc, smArcs);
			for (int i = 0; i < smArcs.size(); i++){
				AddComparedArc(smArcs[i]);
				count++;
			}
		}
	}
	return count;
}

void MyContourTree::ClearComparedArcs() { 
	ClearAllArcStatus(ArcStatus_InComaprison);
};


float MyContourTree::MaxComparedArcWidth() const{
	float maxHeight = 0;
	for (std::map<long, char>::const_iterator it = mArcStatus.begin();
		it != mArcStatus.end(); ++it){
		long ctArc = it->first;
		if (IsArcCompared(ctArc)){
			const vector<float>& histogram = mArcHistogram.at(ctArc);
			if (!histogram.empty()){
				maxHeight = max(maxHeight, *max_element(histogram.begin(), histogram.end()));
			}
		}
	}
	return maxHeight;
}

void MyContourTree::floatToScientific(float num, int& exp, float&manti) const{
	float testNum = fabs(num);
	if (testNum < mCountClamp) testNum = mCountClamp;
	exp = floor(log10(testNum));
	manti = testNum / pow(10, exp);
	if (num < 0){
		manti = -manti;
	}
}

void MyContourTree::updateScientificHistograms(){
	mCountClamp = 9999;
	for (std::map<long, std::vector<float>>::const_iterator it = mArcHistogram.begin();
		it != mArcHistogram.end(); ++it){
		const std::vector<float>& histogram = it->second;
		if (!histogram.empty()){
			for (int i = 0; i < histogram.size(); i++){
				if (histogram[i] != 0){
					mCountClamp = min(mCountClamp, histogram[i]);
				}
			}
		}
	}
	float clampExp = floor(log10(mCountClamp));
	mCountClamp = pow(10, clampExp);

	// added to ensure consistency
	// bad idea though
	mCountClamp = pow(10, -4);

	mMinExponent = 99999;
	mMaxExponent = -9999;
	mMinMantissa = 10;
	mMaxMantissa = -10;
	float maxHeight = 0;
	for (std::map<long, std::vector<float>>::const_iterator it = mArcHistogram.begin();
		it != mArcHistogram.end(); ++it){
		const std::vector<float>& histogram = it->second;
		if (!histogram.empty()){
			for (int i = 0; i < histogram.size(); i++){
				int exponent;
				float mantissa;
				floatToScientific(histogram[i], exponent, mantissa);
				mMinExponent = min(mMinExponent, exponent);
				mMaxExponent = max(mMaxExponent, exponent);
				mMinMantissa = min(mMinMantissa, mantissa);
				mMaxMantissa = max(mMaxMantissa, mantissa);
			}
		}
	}
	float mantissaRange = mMaxMantissa - mMinMantissa;
	float exponentRange = mMaxExponent - mMinExponent;
	if (exponentRange == 0){
		mExponent_Scale = (mMaxMantissa + mMinMantissa)/2 / mMaxExponent;
		mExponent_Offset = 0;

	}
	else{
		//mExponent_Scale = mantissaRange / exponentRange;
		//mExponent_Offset = mMinMantissa / mExponent_Scale - mMinExponent;
		mExponent_Offset = 1 - mMinExponent;
		mExponent_Scale = 1;
	}

	cout << "Exponent range: " << mMinExponent << ',' << mMaxExponent << endl;
	cout << "Mantissa range: " << mMinMantissa << ',' << mMaxMantissa << endl;
	cout << "Exponent offset & scale: " << mExponent_Offset << ',' << mExponent_Scale << endl;
}

void MyContourTree::GetDrawingHeightScientific(float count, float& expHeight, float&mantissaHeight) const{
	int exponent;
	float mantissa;
	floatToScientific(count, exponent, mantissa);
	expHeight = (exponent + mExponent_Offset)*mExponent_Scale*mScientificWidthScale;
	mantissaHeight = mantissa*mScientificWidthScale;
}

void MyContourTree::ClickSnapArc(long arc){
	std::map<long, char>::iterator it = mArcStatus.find(arc);
	if (it != mArcStatus.end()){
		mArcStatus[arc] ^= ArcStatus_InSnapping;
	}
	else{
		mArcStatus[arc] = ArcStatus_InSnapping;
	}
}

void MyContourTree::ClearSnapArcs() {
	for (std::map<long, char>::iterator it = mArcStatus.begin();
		it != mArcStatus.end(); ++it){
		it->second &= ~ArcStatus_InSnapping;
	}
};

void MyContourTree::GetArcSnapPosition(long arc, float& xpos, float& ypos) const{
	long topID = superarcs[arc].topID;
	long bottomID = superarcs[arc].bottomID;
	if (supernodes[topID].IsUpperLeaf()){
		xpos = supernodes[topID].xPosn;
	}
	else{
		xpos = supernodes[bottomID].xPosn;
	}
	switch (mSnapPosition)
	{
		case MyContourTree::ArcSnapPosition_Top_Y:
			ypos = (*supernodes[topID].value - minHeight) / (maxHeight - minHeight);
			break;
		case MyContourTree::ArcSnapPosition_Center_Y:
			ypos = ((*supernodes[topID].value + *supernodes[bottomID].value)/2 - minHeight) / (maxHeight - minHeight);
			break;
		case MyContourTree::ArcSnapPosition_Bottom_Y:
		default:
			ypos = (*supernodes[bottomID].value - minHeight) / (maxHeight - minHeight);
			break;
	}
}

bool MyContourTree::IsArcSnapped(long arc) const{
	std::map<long, char>::const_iterator it = mArcStatus.find(arc);
	if (it != mArcStatus.end()){
		return (it->second & ArcStatus_InSnapping) 
			|| (it->second & ArcStatus_SnapAnchoring);
	}
	return false;
}

void MyContourTree::AddSnapArc(long arc){
	mArcStatus[arc] |= ArcStatus_InSnapping;
}

int MyContourTree::SyncSnapArcsTo(MyContourTree* ct){
	long count = 0;
	ClearSnapArcs();
	for (std::map<long, char>::iterator it = ct->mArcStatus.begin();
		it != ct->mArcStatus.end(); ++it){
		long ctArc = it->first;
		if (ct->IsArcSnapped(ctArc)){
			vector<long> smArcs;
			FindSimilarValidArcs(ct, ctArc, smArcs);
			for (int i = 0; i < smArcs.size(); i++){
				AddSnapArc(smArcs[i]);
				count++;
			}
		}
	}
	return count;
}


void MyContourTree::UpdateSnapArcStatus(MyContourTree* ct, float offsetX, float offsetY, float range){
	ClearAllArcStatus(ArcStatus_SnapAnchoring);
	ct->ClearAllArcStatus(ArcStatus_SnapAnchoring);
	float minDistSqad = range*range;
	long thatArc = -1;
	long thisArc = -1;
	for (std::map<long, char>::iterator it_i = ct->mArcStatus.begin();
		it_i != ct->mArcStatus.end(); ++it_i){
		long ctArc = it_i->first;
		float x_i, y_i;
		ct->GetArcSnapPosition(ctArc, x_i, y_i);
		x_i -= offsetX;
		y_i -= offsetY;
		if (ct->IsArcSnapped(ctArc)){
			for (std::map<long, char>::iterator it_j = this->mArcStatus.begin();
				it_j != this->mArcStatus.end(); ++it_j){
				long arc = it_j->first;
				if (this->IsArcSnapped(arc)){
					float x_j, y_j;
					this->GetArcSnapPosition(arc, x_j, y_j);
					float distSqad = pow(x_i - x_j, 2) + pow(y_i - y_j, 2);
					if (distSqad < minDistSqad){
						thatArc = ctArc;
						thisArc = arc;
						minDistSqad = distSqad;
					}
				}
			}
		}
	}
	if (thisArc >= 0 && thatArc >= 0){
		ct->mArcStatus[thatArc] |= ArcStatus_SnapAnchoring;
		this->mArcStatus[thisArc] |= ArcStatus_SnapAnchoring;
		cout << "Anchoring.\n";
	}
}

long MyContourTree::GetAnchoringArc() const{
	for (std::map<long, char>::const_iterator it = mArcStatus.begin();
		it != mArcStatus.end(); ++it){
		if (it->second & ArcStatus_SnapAnchoring){
			return it->first;
		}
	}
	return -1;
}

bool MyContourTree::ShouldWeSnap(MyContourTree* ct, float& offsetX, float& offsetY) const{
	long thisAnchor = this->GetAnchoringArc();
	long thatAnchor = ct->GetAnchoringArc();
	if (thisAnchor >= 0 && thatAnchor >= 0){
		float thatX, thatY, thisX, thisY;
		ct->GetArcSnapPosition(thatAnchor, thatX, thatY);
		this->GetArcSnapPosition(thisAnchor, thisX, thisY);
		offsetX = thatX - thisX;
		offsetY = thatY - thisY;
		return true;
	}
	return false;
}

void MyContourTree::RenameLeaveArcsBySimilarity(MyContourTree* ct0, MyContourTree* ct1){
	map<string, long> label_current_Idx;
	map<long, int> ct0_arc_suffix;
	map<long, int> ct1_arc_suffix;
	for (int i = 0; i < ct0->nValidArcs; i++){
		long ct0_arc = ct0->valid[i];

		// find current idx for this label
		if (label_current_Idx.find(ct0->mArcName[ct0_arc]) != label_current_Idx.end()){
			label_current_Idx[ct0->mArcName[ct0_arc]] ++;
			ct0->mArcName[ct0_arc] += ('_'+to_string(label_current_Idx[ct0->mArcName[ct0_arc]]));
		}
		else{
			label_current_Idx[ct0->mArcName[ct0_arc]] = 0;;
		}

		// rename the label of the other tree arcs
		if (ct0->supernodes[ct0->superarcs[0].topID].IsUpperLeaf()
			|| ct0->supernodes[ct0->superarcs[0].bottomID].IsLowerLeaf()){
			vector<long> smArcs1;
			ct1->FindSimilarValidArcs(ct0, ct0_arc, smArcs1);
			for (int j = 0; j < smArcs1.size(); j++){
				ct1->mArcName[smArcs1[j]] = ct0->mArcName[ct0_arc];
			}
		}
	}
}