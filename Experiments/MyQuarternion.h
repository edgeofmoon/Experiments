#pragma once

#include "MyVec.h"
#include "MyMatrix.h"

typedef float unit;

class MyQuarternion
{
public:
	MyQuarternion(void);
	MyQuarternion(unit angle, unit axisx, unit axisy, unit axisz);
	MyQuarternion(unit angle, MyVec<unit,3> dir);
	~MyQuarternion(void);

	friend MyQuarternion operator*(const MyQuarternion&a, const MyQuarternion&b);

	unit& operator[](int i);
	unit operator[](int i) const;

	MyMatrix<unit> GetMatrix() const;

	void normalize();

	MyVec<unit,4> _d;
};

