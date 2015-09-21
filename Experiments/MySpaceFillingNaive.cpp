#include "MySpaceFillingNaive.h"


MySpaceFillingNaive::MySpaceFillingNaive()
{
}


MySpaceFillingNaive::~MySpaceFillingNaive()
{
}

void MySpaceFillingNaive::Clear(){
	mBoxes.clear();
}

MyBox2f MySpaceFillingNaive::PushBoxFromTop(const MyBox2f& box, float intv){
	bool settled;
	MyBox2f currentBox = box;
	float boxHeight = currentBox.GetHighPos()[1] - currentBox.GetLowPos()[1];
	do{
		settled = true;
		for (int i = 0; i < mBoxes.size(); i++){
			if (currentBox.IsIntersected(mBoxes[i])){
				currentBox.SetLow(MyVec2f(currentBox.GetLowPos()[0], mBoxes[i].GetHighPos()[1]+intv));
				currentBox.SetHigh(MyVec2f(currentBox.GetHighPos()[0], mBoxes[i].GetHighPos()[1] + intv + boxHeight));
				settled = false;
			}
		}
	} while (!settled);
	mBoxes.push_back(currentBox);
	return currentBox;
}

MySpaceFillingSpiral::MySpaceFillingSpiral(){
}

MySpaceFillingSpiral::~MySpaceFillingSpiral(){
}

void MySpaceFillingSpiral::Clear(){
	mBoxes.clear();
}

MyBox2f MySpaceFillingSpiral::PushBox(const MyBox2f& box, const MyVec2f& boarder){
	MyBox2f theBox = MyBox2f(box.GetLowPos() - boarder, box.GetHighPos() + boarder);
	if (IsBoxEmpty(theBox)){
		mBoxes.push_back(box);
		return box;
	}
	else{
		MySpiralWalker spiralWalker;
		spiralWalker.SetOrigin(MyVec2f(0,0));
		spiralWalker.SetPhase(90);
		spiralWalker.SetStep(box.GetSize(0), box.GetSize(1), 10);
		MyBox2f currentBox;
		do{
			currentBox = theBox + spiralWalker.Next();
		} while (!IsBoxEmpty(currentBox));
		mBoxes.push_back(box + spiralWalker.Last());
		return box + spiralWalker.Last();
	}
}

void MySpaceFillingSpiral::ForceAddBox(const MyBox2f& box){
	mBoxes.push_back(box);
}

bool MySpaceFillingSpiral::IsBoxEmpty(const MyBox2f& box) const{
	// a naive and inefficient one for now
	for (int i = 0; i < mBoxes.size(); i++){
		if (box.IsIntersected(mBoxes[i])){
			return false;
		}
	}
	return true;
}

MySpiralWalker::MySpiralWalker(){
	mOrigin = MyVec2f(0, 0);
	mX = mY = mA = mP = 0;
	mStepX = mStepY = mAngle = 0.1;
}

MySpiralWalker::~MySpiralWalker(){

}

void MySpiralWalker::SetOrigin(const MyVec2f& origin){
	mOrigin = origin;
}

void MySpiralWalker::SetPhase(const float phase){
	mP = phase / 180 * 3.141592657;
}

void MySpiralWalker::SetStep(float xStep, float yStep, float angle){
	mAngle = angle / 180 * 3.141592657;
	mStepX = xStep*(angle / 360);
	mStepY = yStep*(angle / 360);
}

// use definition for Archimedean spiral
// source: https://en.wikipedia.org/wiki/Spiral
MyVec2f MySpiralWalker::Next(){
	mA += mAngle;
	mX = mStepX*mA;
	mY = mStepY*mA;
	return mOrigin + MyVec2f(mX*cos(mA + mP), mY*sin(mA + mP));
}


MyVec2f MySpiralWalker::Last() const{
	return mOrigin + MyVec2f(mX*cos(mA + mP), mY*sin(mA + mP));
}