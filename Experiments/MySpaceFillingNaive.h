#pragma once

#include "MyBox.h"
#include "MyArray.h"
class MySpaceFillingNaive
{
public:
	MySpaceFillingNaive();
	~MySpaceFillingNaive();

	void Clear();
	MyBox2f PushBoxFromTop(const MyBox2f& box, float intv);

protected:
	MyArray<MyBox2f> mBoxes;
};

class MySpiralWalker{
public:
	MySpiralWalker();
	~MySpiralWalker();

	// center of spiral
	// will not be return from Next()
	void SetOrigin(const MyVec2f& origin = MyVec2f(0, 0));
	// pass in degree for phase and angle
	void SetPhase(const float phase);
	// xStep is for a radius increase in x direction for 1 circle
	// the same goes for yStep
	void SetStep(float xStep, float yStep, float angle);

	MyVec2f Last() const;
	MyVec2f Next();

protected:
	MyVec2f mOrigin;
	// mAngle and mP(phase) stored in arc
	float mStepX, mStepY, mAngle;
	float mX, mY, mA, mP;
};

class MySpaceFillingSpiral
{
public:
	MySpaceFillingSpiral();
	~MySpaceFillingSpiral();

	void Clear();
	MyBox2f PushBox(const MyBox2f& box, const MyVec2f& boarder = MyVec2f(0,0));
	void ForceAddBox(const MyBox2f& box);

protected:
	bool IsBoxEmpty(const MyBox2f& box) const;
	MyArray<MyBox2f> mBoxes;
};
