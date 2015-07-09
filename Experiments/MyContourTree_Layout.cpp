#include "MyContourTree.h"
#include "Ric\RicVolume.h"
#include "MyPrimitiveDrawer.h"
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
		mArcHistogram[arc] = histogram;
	}
}

void MyContourTree::updateArcHistograms(){
	mArcHistogram.clear();
	for (int arc = 0; arc < nValidArcs; arc++){
		long whichArc = valid[arc];
		updateArcHistogram(whichArc);
	}
}

float MyContourTree::countToHeight(int count, char compareByte) const{
	switch (compareByte)
	{
	case 1:
		//return count;
		return count * mComparedScaleWidth;
		break;
	case 0:
	default:
		//return log(count + 1.f);
		return log(count + 1.f) * mScaleWidth;
		break;
	}
}

float MyContourTree::getArcWidth(long arc){
	vector<float>& histogram = mArcHistogram[arc];
	if (histogram.empty()){
		cout << "Arc " << arc << " has no vertex.\n";
		return countToHeight(0);
	}
	else{
		return countToHeight(*max_element(histogram.begin(), histogram.end()), mArcCompared[arc]);
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
	float parentArcWidth = getArcWidth(parentArc);;
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
		long brunchNode = brunchNodes[i];
		float *value = supernodes[brunchNode].value;
		long x, y, z;
		height.ComputeIndex(value, x, y, z);
		int label = mLabelVolume->get_at_index(x, y, z);
		long brunchArc = nodes2Arc(brunchNode, rootNodes[i]);
		string name = mArcName[brunchArc];
		if (IsNameLeft(name)) isLeft[i] = true;
		else if (IsNameRight(name)) isLeft[i] = false;
		else if (label % 2 == 1){
			isLeft[i] = false;
		}
		else{
			isLeft[i] = true;
		}
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

float MyContourTree::UpdateSubTreeLayout(long rootNode, long parentNode, float xStart, float xEnd){
	int numLeaves = supernodesExt[rootNode].numLeaves;
	vector<long> centerPath;
	//FindHighestPath(rootNode, parentNode, centerPath, supernodes[rootNode].value);
	if (parentNode >= 0){
		if (supernodes[rootNode].IsUpperLeaf() || supernodes[rootNode].IsLowerLeaf()){
			supernodes[rootNode].xPosn = (xStart + xEnd) / 2;
			return 0.1;
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
		long brunchNode = brunchNodes[i];
		float *value = supernodes[brunchNode].value;
		long x, y, z;
		height.ComputeIndex(value, x, y, z);
		int label = mLabelVolume->get_at_index(x, y, z);
		string name = mArcName[nodes2Arc(brunchNode, rootNodes[i])];
		if (supernodes[rootNodes[i]].downDegree == 0)
		{
			int debug = 1;
		}
		if (IsNameLeft(name)) isLeft[i] = true;
		else if (IsNameRight(name)) isLeft[i] = false;
		else if (label % 2 == 1){
			isLeft[i] = false;
		}
		else{
			isLeft[i] = true;
		}
	}

	float centerPathWidth = getPathWidth(centerPath, parentNode);
	// assuming 0 offset for now
	vector<MyVec2f> leftUpperBottomFilled(1, MyVec2f(centerPathWidth / 2, MinHeight()));
	vector<MyVec2f> rightUpperBottomFilled(1, MyVec2f(centerPathWidth / 2, MinHeight()));;
	vector<MyVec2f> leftDowmTopFilled(1, MyVec2f(centerPathWidth / 2, MaxHeight()));;
	vector<MyVec2f> rightDownTopFilled(1, MyVec2f(centerPathWidth / 2, MaxHeight()));;

	float maxLeft = centerPathWidth/2;
	float maxRight = centerPathWidth/2;

	// upper slots
	// from high to low
	for (int i = 0; i < downStart; i++){
		long brunchNode = brunchNodes[i];
		//float subTreeWidth = getSubTreeWidth(brunchNode, rootNodes[i]);
		float subTreeWidth = subTreeLayoutWidth(brunchNode, rootNodes[i]);
		long arc = nodes2Arc(brunchNode, rootNodes[i]);
		if (supernodes[brunchNode].downList == 37762){
			int debut = 1;
		}
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
	MyBitmap& bitmap = *colorMap;

	int numSteps = 1 / mBinWidth + 1;
	float step = (highPos[1] - lowPos[1]) / numSteps;
	float xMid = (lowPos[0] + highPos[0]) / 2;
	
	glBegin(GL_QUAD_STRIP);
	int countHere = nVertices;
	for (int i = 0; i <= numSteps; i++){
		if (countHere <= 1) break;
		float height = GetDrawingHeight(countHere, 0) / 2;
		glColor3f(bitmap.GetColor(i, 0).b, bitmap.GetColor(i, 0).g, bitmap.GetColor(i, 0).r);
		glVertex2f(xMid - height, lowPos[1] + step*i);
		glColor3f(bitmap.GetColor(i, 1).b, bitmap.GetColor(i, 0).g, bitmap.GetColor(i, 0).r);
		glVertex2f(xMid + height, lowPos[1] + step*i);
		countHere *= 0.9;
	}
	glEnd();

	glColor3f(0, 0, 0);
	countHere = nVertices;
	for (int i = 0; i <= numSteps; i++){
		if (countHere <= 1) break;
		if (i % 20 != 0) {
			countHere *= 0.9;
			continue;
		}
		glRasterPos2f(highPos[0], lowPos[1] + step*i);
		glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)MyString(countHere).c_str());
		countHere *= 0.9;
	}
}

bool MyContourTree::IsArcCompared(long arc) const{
	/*
	for (int i = 0; i < mArcCompared.size(); i++){
		if (mArcCompared[i] == arc) return true;
	}
	return false;
	*/
	if (mArcCompared.find(arc) != mArcCompared.end()){
		return mArcCompared.at(arc) != 0;
	}
	return false;
}

void MyContourTree::AddComparedArc(long arc){
	mArcCompared[arc] = 1;
}

void MyContourTree::RemoveComparedArc(long arc){
	mArcCompared[arc] = 0;
}

void MyContourTree::ClickComparedArc(long arc){
	if (IsArcCompared(arc)){
		RemoveComparedArc(arc);
	}
	else{
		AddComparedArc(arc);
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

float MyContourTree::GetDrawingHeight(float count, char compareByte) const{
	switch (compareByte)
	{
	case 1:
		return count*mComparedScaleWidth;
		break;
	case 0:
	default:
		return log(count + 1)*mScaleWidth;
		break;
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
	mArcCompared.clear();
	for (std::map<long, char>::iterator it = ct->mArcCompared.begin();
		it != ct->mArcCompared.end(); ++it){
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

float MyContourTree::MaxComparedArcWidth() const{
	float maxHeight = 0;
	for (std::map<long, char>::const_iterator it = mArcCompared.begin();
		it != mArcCompared.end(); ++it){
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

float MyContourTree::SuggestComparedArcWidthScale() const{
	float maxHeight = MaxComparedArcWidth();
	if (maxHeight != 0){
		return countToHeight(maxHeight) / maxHeight;
	}
	return 0;
}