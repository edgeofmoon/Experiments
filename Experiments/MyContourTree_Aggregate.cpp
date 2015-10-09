#include "MyContourTree.h"
#include "Ric\RicVolume.h"
#include <queue>
#include <algorithm>


bool MyContourTree::IsArcAggregated(long arc) const{
	map<long, ArcAggregation>::const_iterator itr = mArcAggregation.find(arc);
	if (itr != mArcAggregation.end()){
		return itr->second == ArcAggregation_AGGRE;
	}
	else return false;
}

void MyContourTree::AggregateArcFromAbove(long arc){
	map<long, ArcAggregation>::const_iterator itr = mArcAggregation.find(arc);
	if (itr != mArcAggregation.end()){
		if (itr->second != ArcAggregation_NONE) return;
	}
	if (supernodes[superarcs[arc].topID].IsUpperLeaf()) return;
	mArcAggregateRestore[arc] = superarcs[arc];
	mNodeAggregateRestore[superarcs[arc].topID] = supernodes[superarcs[arc].topID];
	mArcAggregateHistogramRestore[arc] = mArcNodes[arc];

	// record all sub arcs that are to be hidden
	std::vector<long> subArcs;
	queue<long> arc2visit;
	arc2visit.push(arc);
	while (!arc2visit.empty()){
		long thisArc = arc2visit.front();
		arc2visit.pop();
		long topNodeId = superarcs[thisArc].topID;
		long theArc = supernodes[topNodeId].upList;;
		for (long i = 0; i < supernodes[topNodeId].upDegree; i++){
			arc2visit.push(theArc);
			subArcs.push_back(theArc);
			theArc = superarcs[theArc].nextUp;
		}
	}

	float *maxValue = supernodes[superarcs[arc].topID].value;
	for (int i = 0; i < subArcs.size(); i++){
		long arc2hide = subArcs[i];
		// transfer hidden arc data
		mArcNodes[arc].insert(mArcNodes[arc].end(), mArcNodes[arc2hide].begin(), mArcNodes[arc2hide].end());
		if (compareHeight(mArcNodes[arc2hide].front(), maxValue) > 0){
			maxValue = mArcNodes[arc2hide].front();
		}
		if (superarcs[arc2hide].CheckFlag(Superarc::isActive)){
			RemoveFromActive(arc2hide);
		}
		// remove the hidden arc
		RemoveFromValid(arc2hide);
		// also remove the hidden node
		RemoveFromValidNodes(superarcs[arc2hide].topID);
		// store hidden arc status
		mArcAggregation[arc2hide] = ArcAggregation_HIDDEN;
	}

	// update this arc
	sort(mArcNodes[arc].begin(), mArcNodes[arc].end(), compareHeightLogic);
	mArcAggregation[arc] = ArcAggregation_AGGRE;
	// update its topNode
	supernodes[superarcs[arc].topID].upDegree = 0;
	supernodes[superarcs[arc].topID].upList = -1;
	supernodes[superarcs[arc].topID].value = maxValue;
	supernodes[superarcs[arc].topID].yPosn = *maxValue;
}

void MyContourTree::RestoreAggregatedArc(long arc){
	if (!this->IsArcAggregated(arc)) return;

	// restore this arc and node
	if (superarcs[arc].CheckFlag(Superarc::isActive)){
		RemoveFromActive(arc);
	}
	superarcs[arc] = mArcAggregateRestore[arc];
	mArcAggregateRestore.erase(arc);
	supernodes[superarcs[arc].topID] = mNodeAggregateRestore[superarcs[arc].topID];
	mNodeAggregateRestore.erase(superarcs[arc].topID);
	mArcNodes[arc] = mArcAggregateHistogramRestore[arc];
	mArcAggregateHistogramRestore.erase(arc);

	// restore the status of this arc
	mArcAggregation.erase(arc);

	// record all sub arcs that are to be restored
	std::vector<long> subArcs;
	queue<long> arc2visit;
	arc2visit.push(arc);
	while (!arc2visit.empty()){
		long thisArc = arc2visit.front();
		arc2visit.pop();
		long topNodeId = superarcs[thisArc].topID;
		long theArc = supernodes[topNodeId].upList;;
		for (long i = 0; i < supernodes[topNodeId].upDegree; i++){
			arc2visit.push(theArc);
			subArcs.push_back(theArc);
			theArc = superarcs[theArc].nextUp;
		}
	}

	for (int i = 0; i < subArcs.size(); i++){
		long arc2restore = subArcs[i];
		// remove the hidden arc
		AddToValid(arc2restore);
		// also remove the hidden node
		AddToValidNodes(superarcs[arc2restore].topID);
		// restore arc status
		mArcAggregation.erase(arc2restore);
	}
}

void MyContourTree::RestoreAllAggregated(){
	while (!mArcAggregation.empty()){
		long arc = mArcAggregation.begin()->first;
		if (mArcAggregation.begin()->second == ArcAggregation_AGGRE){
			RestoreAggregatedArc(arc);
		}
		else mArcAggregation.erase(mArcAggregation.begin());
	}
}

void MyContourTree::AggregateAllBranches(){
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
	long rootNode = highestNode;
	int numLeaves = supernodesExt[rootNode].numLeaves;
	vector<long> centerPath;
	FindHighestDownPath(rootNode, centerPath);

	vector<long> branchArcs;
	bool centerPathBranchAdded = false;
	for (int i = 0; i < centerPath.size(); i++){
		long thisParentNode = i > 0 ? centerPath[i - 1] : -1;
		long childNode = i < centerPath.size() - 1 ? centerPath[i + 1] : -1;
		vector<long> neighbors;
		GetNeighbors(centerPath[i], neighbors);
		for (int j = 0; j < neighbors.size(); j++){
			if (neighbors[j] != thisParentNode && neighbors[j] != childNode){
				long branchArc = nodes2Arc(neighbors[j], centerPath[i]);
				branchArcs.push_back(branchArc);
				//if (!centerPathBranchAdded){
				//	centerPathBranchAdded = true;
				//	if (thisParentNode >= 0){
				//		long centerPathBranchArc = nodes2Arc(thisParentNode, centerPath[i]);
				//		if (centerPathBranchArc >= 0){
				//			branchArcs.push_back(centerPathBranchArc);
				//			cout << "***CenterPathArc: " << centerPathBranchArc << endl;
				//		}
				//	}
				//}
			}
		}
	}

	for (int i = 0; i < branchArcs.size(); i++){
		long topNode = superarcs[branchArcs[i]].topID;
		long bottomNode = superarcs[branchArcs[i]].bottomID;
		// aggregate upper branch
		if (supernodesExt[topNode].numLeaves <= supernodesExt[bottomNode].numLeaves){
			this->AggregateArcFromAbove(branchArcs[i]);
		}
	}
}

Superarc MyContourTree::GetUnaggregatedArc(long arc) const{
	std::map<long, Superarc>::const_iterator itr = mArcAggregateRestore.find(arc);
	if (itr != mArcAggregateRestore.end()){
		return itr->second;
	}
	else{
		return superarcs[arc];
	}
}

Supernode MyContourTree::GetUnaggregatedNode(long node) const{
	std::map<long, Supernode>::const_iterator itr = mNodeAggregateRestore.find(node);
	if (itr != mNodeAggregateRestore.end()){
		return itr->second;
	}
	else{
		return supernodes[node];
	}
}

/*
void MyContourTree::SetIsosurface(float ht){
	int sArc, validArc;
	ClearAllArrays();
	for (validArc = 0; validArc < nValidArcs; validArc++){
		sArc = valid[validArc];
		if (this->IsArcAggregated(sArc)){
			queue<long> arc2visit;
			arc2visit.push(sArc);
			while (!arc2visit.empty()){
				long thisArc = arc2visit.front();
				arc2visit.pop();
				if (*(GetUnaggregatedNode(GetUnaggregatedArc(sArc).topID).value) <= ht){
					long topNodeId = GetUnaggregatedArc(thisArc).topID;
					long theArc = GetUnaggregatedNode(topNodeId).upList;;
					for (long i = 0; i < GetUnaggregatedNode(topNodeId).upDegree; i++){
						arc2visit.push(theArc);
						theArc = GetUnaggregatedArc(theArc).nextUp;
					}
				}
				// if thisArc intersects, its upper children won't
				else if (*(GetUnaggregatedNode(GetUnaggregatedArc(sArc).bottomID).value) <= ht){
					AddToActive(thisArc, ht);
				}
			}
		}
		else 
			if (*(supernodes[superarcs[sArc].topID].value) > ht
			&& *(supernodes[superarcs[sArc].bottomID].value) <= ht){
			AddToActive(sArc, ht);
		}
	}
}
*/