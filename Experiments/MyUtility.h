#ifndef MYUTILITY_H
#define MYUTILITY_H

#define SafeFreeObject(x) \
	if(x){					\
		delete x;			\
		x = 0;				\
	}						

#define SafeFreeArray(x)	\
	if(x){					\
		delete[] x;			\
		x = 0;				\
	}						

#define Clamp(x, a, b)		\
	if(x<a) x = a;			\
	else if(x>b) x = b;		\

#define MY_PI 3.141592653589793238462643383279502884197169399375105820974944592307816406286f

#define MY_INF_FLOAT 3.4028e+38
#define MY_LARGE_FLOAT 3.4028e+10

#define SIGN(x) (x>0?1:(x<0?-1:0))

#endif