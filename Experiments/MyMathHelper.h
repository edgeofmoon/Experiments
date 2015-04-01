#pragma once

#include "MyMatrix.h"
#include "MyArray.h"

class MyMathHelper
{
public:
	MyMathHelper(void);
	~MyMathHelper(void);

	// memory leak alert
	// outMat and eigens are new
	static void SingularValueDecomposition(const MyMatrixf* inMat, MyMatrixf* outMat, float * eigens);

	static void MultiDimensionalProjection(const MyMatrixf* distMat, MyMatrixf* projMat, int nDim = 2);

	static int BinomialCoefficient(int n, int i);

	static int Factorial(int i);

protected:
	static MyArrayi Factorials;
};

