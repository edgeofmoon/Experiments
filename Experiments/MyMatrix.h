#pragma once

#include <vector>
#include <iostream>

// stored as row prior
template<typename T>
class MyMatrix
{
public:
	MyMatrix();
	MyMatrix(const MyMatrix<T> &mat);
	MyMatrix(int _m, int _n, T val = 0);
	MyMatrix(const T ** indata,int _m, int _n);
	MyMatrix(const T * indata, int _m, int _n, int _columnPrior = false);

	~MyMatrix(void);

	void Print() const;

	inline int GetNumRows() const {return m;};
	inline int GetNumCols() const {return n;};
	inline T& At(int i,int j) {return d[i*n+j];};
	inline const T& At(int i,int j) const {return d[i*n+j];};
	inline const T* GetData() const {return d;};
	inline bool IsSquare() const {return m == n;};

	MyMatrix<T>& operator=(const MyMatrix<T> &mat);
	MyMatrix<T> operator*(const MyMatrix<T> &mat) const;
	MyMatrix<T> operator*(const float x) const;
	MyMatrix<T> operator+(const MyMatrix<T> &mat) const;
	MyMatrix<T> operator-(const MyMatrix<T> &mat) const;

	MyMatrix<T> Transpose() const;
	MyMatrix<T> DotMultiply(const MyMatrix<T> &mat) const;
	MyMatrix<T> GetMeanOfCol() const;
	MyMatrix<T> GetMeanOfRow() const;
	MyMatrix<T> Duplicated(int nRow, int nCol) const;
	MyMatrix<T> GetRows(const std::vector<int>& colIdx) const;
	MyMatrix<T> GetCols(const std::vector<int>& rowIdx) const;
	MyMatrix<T> RemoveRow(int rowIdx) const;
	MyMatrix<T> RemoveCol(int colIdx) const;
	MyMatrix<T> ColMultiply(const MyMatrix<T>& colFactors) const;
	MyMatrix<T> GetNormalizedByCol() const;
	MyMatrix<T> GetEqualVarByCol() const;

	//void SvdDcp(MyMatrix &left, MyMatrix &mid) const;
	void FindRangeByCol(int colIdx, T& min, T& max) const;
	T FindVarianceByCol(int colIdx) const;

	// openGL
	// note openGL uses column prior
	// need to transpose before load to openGL
	static MyMatrix<T> IdentityMatrix(int m = 4, int n = 4);
	static MyMatrix<T> RotateMatrix(T angle, T x, T y, T z);
	static MyMatrix<T> TranslateMatrix(T x, T y, T z);
	static MyMatrix<T> ZeroMatrix(int m = 4, int n = 4);
	static MyMatrix<T> PerspectiveMatrix(T fov, T aspect, T znear, T zfar);
	static MyMatrix<T> OrthographicMatrix(T left, T right, T bottom, T top, T near, T far);
	static MyMatrix<T> ScaleMatrix(T x, T y, T z);

protected:
	int m;
	int n;
	T *d;
};


// definition
// c++ requires templete class be defined in .h

#include "MyMatrix.h"
#include "memory.h"
#include <cassert>

template<typename T>
MyMatrix<T>::MyMatrix()
{
	d = 0;
	m=n=0;
}

template<typename T>
MyMatrix<T>::MyMatrix( int _m, int _n, T val = 0 )
{
	m = _m;
	n = _n;
	d = new T[m*n];
	memset(d,val,m*n*sizeof(T));
	for(int i = 0;i<m*n;i++){
		d[i] = val;
	}
}

template<typename T>
MyMatrix<T>::MyMatrix(const MyMatrix<T> &mat)
{
	m = mat.GetNumRows();
	n = mat.GetNumCols();
	d = new T[m*n];
	memcpy(d,mat.GetData(),m*n*sizeof(T));
}
template<typename T>
MyMatrix<T>::MyMatrix( const T ** indata,int _m, int _n )
{
	m = _m;
	n = _n;
	d = new T[m*n];
	for (int i = 0;i<m;i++){
		for(int j = 0;j<n;j++){
			At(i,j) = indata[i][j];
		}
	}
}

template<typename T>
MyMatrix<T>::MyMatrix(const T * indata, int _m, int _n, int _columnPrior){
	m = _m;
	n = _n;
	d = new T[m*n];
	if(_columnPrior){
		// if is column prior, transpose it
		for(int i = 0;i<_m;i++){
			for(int j = 0;j<_n;j++){
				At(i,j) = indata[j*_m+i];
			}
		}
	}
	else{
		for(int i = 0;i<_m;i++){
			for(int j = 0;j<_n;j++){
				At(i,j) = indata[i*_n+j];
			}
		}
	}

}
template<typename T>
MyMatrix<T>::~MyMatrix(void)
{
	if (d != 0){
		delete[] d;
	}
}

template<typename T>
void MyMatrix<T>::Print() const{
	for(int i = 0;i<m;i++){
		for(int j = 0;j<n;j++){
			std::cerr << At(i,j) << ' ';
		}
		std::cerr << std::endl;
	}
}

template<typename T>
MyMatrix<T> MyMatrix<T>::operator*(const MyMatrix<T> &mat) const
{
	int p = mat.GetNumRows();
	int q = mat.GetNumCols();
	assert(n == p);
	MyMatrix tmp(m,q);

	for (int i = 0;i<m;i++){
		for (int j = 0;j<q;j++){
			tmp.At(i,j) = 0;
			for (int k = 0;k<p;k++){
				tmp.At(i,j) += At(i,k)*mat.At(k,j);
			}
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::operator*( const float x ) const
{
	MyMatrix<T> tmp(m,n);

	for (int i = 0;i<m;i++){
		for (int j = 0;j<n;j++){
			tmp.At(i,j) = At(i,j)*x;
		}
	}
	return tmp;

}

template<typename T>
MyMatrix<T>& MyMatrix<T>::operator=( const MyMatrix<T> &mat )
{
	if (this == &mat){
		return *this;
	}

	if (d != 0){
		delete[] d;
	}
	m = mat.GetNumRows();
	n = mat.GetNumCols();

	d = new T[m*n];

	memcpy(d,mat.GetData(),m*n*sizeof(T));

	return *this;
}


template<typename T>
MyMatrix<T> MyMatrix<T>::Transpose() const
{
	MyMatrix<T> tmp(n,m);

	for (int i = 0;i<n;i++){
		for (int j = 0;j<m;j++){
			tmp.At(i,j) = At(j,i);
		}
	}

	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::DotMultiply(const MyMatrix<T> &mat) const{
	 assert(m == mat.GetNumRows() && n == mat.GetNumCols());
	 MyMatrix<T> tmp(m,n);

	 for (int i = 0;i<m;i++){
		 for (int j = 0;j<n;j++){
			 tmp.At(i,j) = At(i,j)*mat.At(i,j);
		 }
	 }
	 return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::operator+( const MyMatrix<T> &mat ) const
{
	 assert(m == mat.GetNumRows() && n == mat.GetNumCols());
	 MyMatrix<T> tmp(m,n);

	 for (int i = 0;i<m;i++){
		 for (int j = 0;j<n;j++){
			 tmp.At(i,j) = At(i,j)+mat.At(i,j);
		 }
	 }
	 return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::operator-( const MyMatrix<T> &mat ) const
{
	assert(m == mat.GetNumRows() && n == mat.GetNumCols());
	MyMatrix<T> tmp(m,n);

	for (int i = 0;i<m;i++){
		for (int j = 0;j<n;j++){
			tmp.At(i,j) = At(i,j)-mat.At(i,j);
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::GetMeanOfCol() const
{
	MyMatrix<T> mean(1,n);
	for(int j = 0;j<n;j++){
		T sum = 0;
		for (int i = 0;i<m;i++){
			sum += At(i,j);
			if (sum <0){
				T kk = At(i,j);
			}
		}
		mean.At(0,j) = sum/m;
	}
	return mean;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::GetMeanOfRow() const
{
	MyMatrix<T> mean(m,1);
	for(int i = 0;i<m;i++){
		T sum = 0;
		for (int j = 0;j<n;j++){
			sum += At(i,j);
		}
		mean.At(i,0) = sum/n;
	}
	return mean;
}


template<typename T>
MyMatrix<T> MyMatrix<T>::Duplicated( int nRow, int nCol ) const
{
	MyMatrix<T> tmp(m*nRow,n*nCol);
	for(int i = 0;i<m*nRow;i++){
		unsigned oldi = i%m;
		for (int j = 0;j<n*nCol;j++){
			unsigned oldj = j%n;
			tmp.At(i,j) = At(oldi,oldj);
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::GetCols( const std::vector<int>& colIdx ) const
{
	MyMatrix<T> tmp(m,colIdx.size());
	for (int i = 0;i<m;i++){
		for (int cId = 0;cId<colIdx.size();cId++){
			int j = colIdx[cId] ;
			tmp.At(i,cId) = At(i,j);
		}
	}
	return tmp;
}
template<typename T>
MyMatrix<T> MyMatrix<T>::GetRows( const std::vector<int>& rowIdx ) const
{
	MyMatrix<T> tmp(rowIdx.size(),n);
	for (int rId = 0;rId<rowIdx.size();rId++){
		for (int j = 0;j<n;j++){
			int i = rowIdx[rId] ;
			tmp.At(rId,j) = At(i,j);
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::RemoveRow(int rowIdx) const{
	MyMatrix<T> tmp(this->GetNumRows() - 1, this->GetNumCols());
	for (int rId = 0; rId<rowIdx; rId++){
		for (int j = 0; j<this->GetNumCols(); j++){
			tmp.At(rId, j) = At(rId, j);
		}
	}
	for (int rId = rowIdx; rId<this->GetNumRows()-1; rId++){
		for (int j = 0; j<this->GetNumCols(); j++){
			tmp.At(rId, j) = At(rId+1, j);
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::RemoveCol(int colIdx) const{
	MyMatrix<T> tmp(this->GetNumRows(), this->GetNumCols() - 1);
	for (int i = 0; i<this->GetNumRows(); i++){
		for (int cId = 0; cId<colIdx; cId++){
			tmp.At(i, cId) = At(i, cId);
		}
	}
	for (int i = 0; i<this->GetNumRows(); i++){
		for (int cId = colIdx; cId<this->GetNumCols() - 1; cId++){
			tmp.At(i, cId) = At(i, cId+1);
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::ColMultiply( const MyMatrix<T>& colFactors ) const
{
	assert(colFactor.GetNumRows() == 1&& colFactor.GetNumCols() == n)

	MyMatrix<T> tmp(m,n);
	for (int j = 0;j<n;j++){
		T colFactor = colFactors.At(j,0);
		for (int i = 0;i<m;i++){
			tmp.At(i,j) = At(i,j)*colFactor;
		}
	}
}

template<typename T>
MyMatrix<T>  MyMatrix<T>::GetNormalizedByCol() const
{
	MyMatrix<T> tmp(m,n);
	for (unsigned j = 0;j<n;j++){
		T min, max;
		FindRangeByCol(j,min,max);
		for (int i = 0;i<m;i++){
			T v = At(i,j);
			T range = max-min;
			if (range != 0){
				tmp.At(i,j) = (v-min)/range;
			}
			else tmp.At(i,j) = 0;
		}
	}
	return tmp;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::GetEqualVarByCol() const{
	MyMatrix<T> tmp(m, n);
	for (unsigned j = 0; j<n; j++){
		T var = FindVarianceByCol(j);
		for (int i = 0; i<m; i++){
			if (var != 0){
				tmp.At(i, j) = At(i, j)/var;
			}
			else tmp.At(i, j) = 0;
		}
	}
	return tmp;
}

template<typename T>
void MyMatrix<T>::FindRangeByCol(int colIdx, T& min, T& max) const
{
	max = min = At(0,colIdx);
	for (int i = 1;i<m;i++){
		T ele = At(i,colIdx);
		if (ele>max){
			max = ele;
		}
		if (ele<min){
			min = ele;
		}
	}
}

template<typename T>
T MyMatrix<T>::FindVarianceByCol(int colIdx) const{
	T var = 0;
	for (int i = 1; i<m; i++){
		T ele = At(i, colIdx);
		var += ele*ele;
	}
	var /= m;
	var = sqrt(var);
	return var;
}

/*
template<typename T>
void MyMatrix<T>::SvdDcp( MyMatrix &left, MyMatrix &mid ) const
{
	
}
*/
template<typename T>
MyMatrix<T> MyMatrix<T>::RotateMatrix(T angle, T x, T y, T z){
	if (angle == 0){
		return IdentityMatrix(4, 4);
	}
	T sinAngle, cosAngle;
	T mag = sqrtf(x * x + y * y + z * z);
 
	const T PI = 3.1415926f;
	sinAngle = sinf ( angle * PI / 180.0f );
	cosAngle = cosf ( angle * PI / 180.0f );

	MyMatrix<T> result(4,4);
	if ( mag > 0.0f ){
		T xx, yy, zz, xy, yz, zx, xs, ys, zs;
		T oneMinusCos;
 
		x /= mag;
		y /= mag;
		z /= mag;
 
		xx = x * x;
		yy = y * y;
		zz = z * z;
		xy = x * y;
		yz = y * z;
		zx = z * x;
		xs = x * sinAngle;
		ys = y * sinAngle;
		zs = z * sinAngle;
		oneMinusCos = 1.0f - cosAngle;
 
		result.At(0,0) = (oneMinusCos * xx) + cosAngle;
		result.At(1,0) = (oneMinusCos * xy) - zs;
		result.At(2,0) = (oneMinusCos * zx) + ys;
		result.At(3,0) = 0.0F;
 
		result.At(0,1) = (oneMinusCos * xy) + zs;
		result.At(1,1) = (oneMinusCos * yy) + cosAngle;
		result.At(2,1) = (oneMinusCos * yz) - xs;
		result.At(3,1) = 0.0F;
 
		result.At(0,2) = (oneMinusCos * zx) - ys;
		result.At(1,2) = (oneMinusCos * yz) + xs;
		result.At(2,2) = (oneMinusCos * zz) + cosAngle;
		result.At(3,2) = 0.0F;
 
		result.At(0,3) = 0.0F;
		result.At(1,3) = 0.0F;
		result.At(2,3) = 0.0F;
		result.At(3,3) = 1.0F;
	}
	return result;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::IdentityMatrix(int m = 4, int n = 4){
	MyMatrix<T> result(m,n);
	for(int i = 0;i<m;i++){
		for(int j = 0;j<n;j++){
			result.At(i,j) = (i==j?1.f:0);
		}
	}
	return result;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::TranslateMatrix(T x, T y, T z){
	MyMatrix<T> result(4, 4);
	for (int i = 0; i<4; i++){
		for (int j = 0; j<3; j++){
			result.At(i, j) = (i == j ? 1.f : 0);
		}
	}
	result.At(0, 3) = x;
	result.At(1, 3) = y;
	result.At(2, 3) = z;
	result.At(3, 3) = 1;
	return result;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::ZeroMatrix(int m = 4, int n = 4){
	MyMatrix<T> result(m, n);
	for (int i = 0; i<m; i++){
		for (int j = 0; j<n; j++){
			result.At(i, j) = 0;
		}
	}
	return result;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::PerspectiveMatrix(T fov, T aspect, T zNear, T zFar){
	const T PI = 3.1415926f;
	const T h = 1.0f/tan(fov*PI/360.f);
	T neg_depth = zNear-zFar;
	MyMatrix<T> m(4,4);
	m.At(0,0) = h / aspect;
	m.At(1,0) = 0;
	m.At(2,0) = 0;
	m.At(3,0) = 0;
 
	m.At(0,1) = 0;
	m.At(1,1) = h;
	m.At(2,1) = 0;
	m.At(3,1) = 0;
 
	m.At(0,2) = 0;
	m.At(1,2) = 0;
	m.At(2,2) = (zFar + zNear)/neg_depth;
	m.At(3,2) = -1;
 
	m.At(0,3) = 0;
	m.At(1,3) = 0;
	m.At(2,3) = 2.0f*(zNear*zFar)/neg_depth;
	m.At(3,3) = 0;
	return m;
}

template<typename T>
MyMatrix<T> MyMatrix<T>::OrthographicMatrix(T left, T right, T bottom, T top, T _near, T _far){
	MyMatrix<T> m(4,4);
	m.At(0,0) = 2.f/(right-left);
	m.At(1,0) = 0;
	m.At(2,0) = 0;
	m.At(3,0) = 0;
 
	m.At(0,1) = 0;
	m.At(1,1) = 2.f/(top-bottom);
	m.At(2,1) = 0;
	m.At(3,1) = 0;
 
	m.At(0,2) = 0;
	m.At(1,2) = 0;
	m.At(2,2) = -2.f/(_far-_near);
	m.At(3,2) = 0;
 
	m.At(0,3) = -(right+left)/(right-left);
	m.At(1,3) = -(top+bottom)/(top-bottom);
	m.At(2,3) = -(_far+_near)/(_far-_near);
	m.At(3,3) = 1;
	return m;
}
template<typename T>
MyMatrix<T> MyMatrix<T>::ScaleMatrix(T x, T y, T z){
	MyMatrix<T> result(4, 4);
	for (int i = 0; i<4; i++){
		for (int j = 0; j<4; j++){
			result.At(i, j) = 0;
		}
	}
	result.At(0, 0) = x;
	result.At(1, 1) = y;
	result.At(2, 2) = z;
	result.At(3, 3) = 1;
	return result;
}

typedef MyMatrix<float>  MyMatrixf;
typedef MyMatrix<double> MyMatrixd;
typedef MyMatrix<int>    MyMatrixi;