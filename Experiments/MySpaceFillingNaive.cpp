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