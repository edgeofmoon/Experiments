#include "MyMathHelper.h"

#include "svd.c"

#include <vector>
#include <algorithm>

// more than 40 is hardly expressable
MyArrayi MyMathHelper::Factorials = MyArrayi(40, -1);

MyMathHelper::MyMathHelper(void)
{
}


MyMathHelper::~MyMathHelper(void)
{
}

void MyMathHelper::SingularValueDecomposition(const MyMatrixf* inMat, MyMatrixf* outMat, float * eigens){
	int m = inMat->GetNumRows();
	int n = inMat->GetNumCols();
	float** a = new float*[m];
	float** v = new float*[m];
	for (int i = 0;i<m;i++){
		a[i] = new float[n];
		v[i] = new float[n];
		for (int j = 0;j<n;j++){
			a[i][j] = inMat->At(i,j);
		}
	}
	eigens = new float[n];
	
	dsvd(a,m,n,eigens,v);

	outMat = new MyMatrixf((const float **)a,m,n);

	delete[] a;
	delete[] v;
}

float *w;
bool largerThan(int i,int j){
	return w[i]>w[j];
};

void MyMathHelper::MultiDimensionalProjection(const MyMatrixf* distMat, MyMatrixf* projMat, int nDim){
	MyMatrixf* transMat;
	float *eigens;
	MyMathHelper::SingularValueDecomposition(distMat,transMat,eigens);

	
	std::vector<int> evs;
	for (int i = 0;i<transMat->GetNumCols();i++){
		evs.push_back(i);
	}

	w = eigens;

	std::sort(evs.begin(),evs.end(),largerThan);
	evs.resize(nDim);

	projMat = new MyMatrixf(transMat->GetRows(evs));
}


int MyMathHelper::BinomialCoefficient(int n, int i){
	if(i == 0 || i == n) return 1;
	return Factorial(n)/(Factorial(i)*Factorial(n-i));
}

int MyMathHelper::Factorial(int i){
	if(i == 0) return 1;
	if(Factorials[i] > 0){
		return Factorials[i];
	}
	else{
		int rst = i*Factorial(i-1);
		Factorials[i] = rst;
		return rst;
	}
}