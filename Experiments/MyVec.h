#pragma once

#include <cmath>

template<typename T, int n>
class MyVec{
public:
    MyVec(T _x,T _y,T _z, T _w){
		_d[0] = _x;
		_d[1] = _y;
		_d[2] = _z;
		_d[3] = _w;
		//for(int i = 4;i<n;i++){
		//	_d[i] = 0;
		//}
	};
    MyVec(T _x,T _y,T _z){
		_d[0] = _x;
		_d[1] = _y;
		_d[2] = _z;
		//for(int i = 3;i<n;i++){
		//	_d[i] = 0;
		//}
	};
	MyVec(T _x,T _y){
		_d[0] = _x;
		_d[1] = _y;
		//for(int i = 2;i<n;i++){
		//	_d[i] = 0;
		//}
	};
    MyVec() {
		//for(int i = 0;i<n;i++){
		//	_d[i] = 0;
		//}
	};
	MyVec(T *indata){
		for(int i = 0;i<n;i++){
			_d[i] = indata[i];
		}
	}
	
	static MyVec<T,n> zero(){
		MyVec<T,n> t;
		for(int i = 0;i<n;i++){
			t[i] = 0;
		}
		return t;
	}

	const T* d() const{
		return _d;
	}

	const T& operator[](int i) const{
		return _d[i];
	}
		
	T& operator[](int i){
		return _d[i];
	}
	/*
	const T& x() const{
		return _d[0];
	}
		
	T& x(){
		return _d[0];
	}
	
	const T& y() const{
		return _d[1];
	}
		
	T& y(){
		return _d[1];
	}

	const T& z() const{
		return _d[2];
	}
		
	T& z(){
		return _d[2];
	}
	
	const T& w() const{
		return _d[3];
	}
		
	T& w(){
		return _d[3];
	}
	*/

	template<int m>
	MyVec<T,m> toDim() const{
		MyVec<T,m> tmp;
		for(int i = 0;i<m && i<n; i++){
			tmp[i] = _d[i];
		}
		return tmp;
	}

	template<int m>
	MyVec<T,m> toDim(T t) const{
		MyVec<T,m> tmp;
		int i;
		for(i = 0;i<m && i<n; i++){
			tmp[i] = _d[i];
		}
		for(i; i<m || i<n;i++){
			tmp[i] = t;
		}
		return tmp;
	}

    friend MyVec<T, n> operator+(const MyVec<T, n> &a,const MyVec<T, n> &b){
        MyVec<T, n> t;
		for(int i = 0;i<n;i++){
			t[i] = a[i]+b[i];
		}
        return t;
    }
	
    friend MyVec<T, n> operator-(const MyVec<T, n> &a,const MyVec<T, n> &b){
        MyVec<T, n> t;
		for(int i = 0;i<n;i++){
			t[i] = a[i]-b[i];
		}
        return t;
    }
	
    friend T operator*(const MyVec<T, n> &a,const MyVec<T, n> &b){
        return a.dotMultiply(b);
    }
	
    friend MyVec<T, 3> operator^(const MyVec<T, n> &a,const MyVec<T, n> &b){
        return a.crossMultiply(b);
    }
	
    friend MyVec<T, n> operator*(const MyVec<T, n> &a,const T b){
        MyVec<T, n> t;
		for(int i = 0;i<n;i++){
			t[i] = a[i]*b;
		}
        return t;
    }
	
    friend MyVec<T, n> operator*(const T b, const MyVec<T, n> &a){
        MyVec<T, n> t;
		for(int i = 0;i<n;i++){
			t[i] = a[i]*b;
		}
        return t;
    }
	
    friend MyVec<T, n> operator/(const MyVec<T, n> &a,const T b){
        MyVec<T, n> t;
		for(int i = 0;i<n;i++){
			t[i] = a[i]/b;
		}
        return t;
    }

    friend bool operator==(const MyVec<T, n> &a, const MyVec<T, n> &b){
		for(int i = 0;i<n;i++){
			if(a[i]!=b[i]){
				return false;
			}
		}
        return true;
    }
	
    friend bool operator!=(const MyVec<T, n> &a, const MyVec<T, n> &b){
		for(int i = 0;i<n;i++){
			if(a[i]!=b[i]){
				return true;
			}
		}
        return false;
    }

    friend bool operator>=(const MyVec<T, n> &a, const MyVec<T, n> &b){
		for(int i = 0;i<n;i++){
			if(a[i]<b[i]){
				return false;
			}
		}
        return true;
    }
	
    friend bool operator<=(const MyVec<T, n> &a, const MyVec<T, n> &b){
		for(int i = 0;i<n;i++){
			if(a[i]>b[i]){
				return false;
			}
		}
        return true;
    }

    MyVec<T, n> operator-() const{
        MyVec<T, n> t;
		for(int i = 0;i<n;i++){
			t[i] = -_d[i];
		}
        return t;
    }

    void operator+=(const MyVec<T, n> &a){
		for(int i = 0;i<n;i++){
			_d[i] += a[i];
		}
    };
	
    void operator-=(const MyVec<T, n> &a){
		for(int i = 0;i<n;i++){
			_d[i] -= a[i];
		}
    };

    void operator*=(const T a){
		for(int i = 0;i<n;i++){
			_d[i] *= a;
		}
    }

    void operator/=(const T a){
		for(int i = 0;i<n;i++){
			_d[i] /= a;
		}
    }

    MyVec<T, n>& operator=(const MyVec<T, n> &a){
		for(int i = 0;i<n;i++){
			_d[i] = a[i];
		}
		return *this;
    };
	
    T dotMultiply(const MyVec<T, n> &a) const{
		T sum = 0;
		for(int i = 0;i<n;i++){
			sum += _d[i]*a[i];
		}
		return sum;
    }
	
    MyVec<T, 3> crossMultiply(const MyVec<T, n> &other) const{
		MyVec<T,3> tmp1 = this->toDim<3>(0);
		MyVec<T,3> tmp2 = other.toDim<3>(0);
        MyVec<T, 3> t;
        t[0] = tmp1[1]*tmp2[2]-tmp1[2]*tmp2[1];
        t[1] = -(tmp1[0]*tmp2[2]-tmp1[2]*tmp2[0]);
        t[2] = tmp1[0]*tmp2[1]-tmp1[1]*tmp2[0];
        return t;
    }
	
    void scale(const MyVec<T, n> &a){
		for(int i = 0;i<n;i++){
			_d[i] *= a[i];
		}
    }

    void normalize(){
        T a = norm();
        if(a != 0) (*this)/=a;
    }

    MyVec<T, n> normalized() const{
        T a = norm();
        if(a != 0) return (*this)/a;
        return (*this);
    }

    T norm() const{
        return sqrt(squared());
    }

    T squared() const{
		T sum = 0;
		for(int i = 0;i<n;i++){
			sum += _d[i]*_d[i];
		}
		return sum;
    }

	static bool Intersected(MyVec<T,2>& a1, MyVec<T,2>& a2, MyVec<T,2>& b1, MyVec<T,2>& b2){
		if(((a2-a1)^(b1-a1))*((a2-a1)^(b2-a1))>0){
			return false;
		}
	
		if(((b2-b1)^(a1-b1))*((b2-b1)^(a2-b1))>0){
			return false;
		}

		return true;
	}

protected:
    T _d[n];
};

typedef MyVec<double,4> MyVec4d;
typedef MyVec<double,3> MyVec3d;
typedef MyVec<double,2> MyVec2d;
typedef MyVec<float,4> MyVec4f;
typedef MyVec<float,3> MyVec3f;
typedef MyVec<float,2> MyVec2f;
typedef MyVec<int,2> MyVec2i;
typedef MyVec<int,3> MyVec3i;
typedef MyVec<int,4> MyVec4i;
