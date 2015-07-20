#include "MyContourTree.h"

#include <cstring>

void MyContourTree::BackupTree(){
	memcpy(mSuperArcsBkup, superarcs, nSuperarcs*sizeof(Superarc));
	memcpy(mValidArcs, valid, 2 * nSuperarcs*sizeof(long));
	memcpy(mSuperNodesBkup, supernodes, nSupernodes*sizeof(Supernode));
	memcpy(mValidNodes, validNodes, nSupernodes*sizeof(long));
	mNumSuperArcsBkup = nSuperarcs;
	mNumSuperNodesBkup = nSupernodes;
	mNumValidArcs = nValidArcs;
	mNumValidNodes = nValidNodes;

	mNextSuperarcBkup = nextSuperarc;
	mSavedNextSuperarcBkup = savedNextSuperarc;
	mNextSupernodeBkup = nextSupernode;
}

void MyContourTree::RestoreTree(){
	memcpy(superarcs, mSuperArcsBkup, nSuperarcs*sizeof(Superarc));
	memcpy(valid, mValidArcs, 2 * nSuperarcs*sizeof(long));
	memcpy(supernodes, mSuperNodesBkup, nSupernodes*sizeof(Supernode));
	memcpy(validNodes, mValidNodes, nSupernodes*sizeof(long));
	selectionRoot = noContourSelected;
	nActiveArcs = nSelectedArcs = nRestorableArcs = 0;
	nValidNodes = mNumValidNodes;
	nValidArcs = mNumValidArcs;

	nextSuperarc = mNextSuperarcBkup;
	savedNextSuperarc = mSavedNextSuperarcBkup;
	nextSupernode = mNextSupernodeBkup;
}
