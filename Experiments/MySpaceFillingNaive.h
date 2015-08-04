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

