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
		// remove the hidden arc
		RemoveFromValid(arc2hide);
		// also remove the hidden node
		RemoveFromValidNodes(superarcs[arc2hide].topID);
		// store hidden arc status
		mArcAggregation[arc2hide] = ArcAggregation_HIDDEN;
	}

	// update this arc
	superarcs[arc].nextUp = arc;
	sort(mArcNodes[arc].begin(), mArcNodes[arc].end(), compareHeightLogic);
	mArcAggregation[arc] = ArcAggregation_AGGRE;
	// update its topNode
	supernodes[superarcs[arc].topID].upDegree = 0;
	supernodes[superarcs[arc].topID].upList = arc;
	supernodes[superarcs[arc].topID].value = maxValue;
	supernodes[superarcs[arc].topID].yPosn = *maxValue;
}

void MyContourTree::RestoreAggregatedArc(long arc){
	if (!this->IsArcAggregated(arc)) return;

	// restore this arc and node
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