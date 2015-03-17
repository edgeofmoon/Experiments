#ifndef MyColor4_h
#define MyColor4_h

template <typename T>
class MyColor4{
public:
	MyColor4(){r = g = b = 0.0; a = 1.0;};
	MyColor4(T _r,T _g, T _b, T _a = 1.0)
		:r(_r),g(_g),b(_b),a(_a){};

	friend MyColor4<T> operator*(const MyColor4<T> &c,const T t){
		return MyColor4<T>(c.r*t,c.g*t,c.b*t,c.a*t);
	}

	friend MyColor4<T> operator*(const T t,const MyColor4<T> &c){
		return MyColor4<T>(c.r*t,c.g*t,c.b*t,c.a*t);
	}

	friend MyColor4<T> operator*(const MyColor4<T> &c,const MyColor4<T> &d){
		return MyColor4<T>(c.r*d.r,c.g*d.g,c.b*d.b,c.a*d.a);
	}
	
	friend MyColor4<T> operator+(const MyColor4<T> &ca, const MyColor4<T> &cb){
		T rr = ca.r+cb.r;
		T gg = ca.g+cb.g;
		T bb = ca.b+cb.b;
		T aa = ca.a+cb.a;

		return MyColor4<T>(rr,gg,bb,aa);
	}
	

	template<typename T2>
	friend MyColor4<T> pow(MyColor4<T> c, T2 ga){
		return MyColor4<T>(pow(c.r,ga), pow(c.g,ga),pow(c.b,ga),c.a);
	}

	friend MyColor4<T> operator+(const MyColor4<T> &ca, const T cb){
		return ca+MyColor4<T>(cb,cb,cb,1);
	}

	friend MyColor4<T> operator+( const T cb, const MyColor4<T> &ca){
		return ca+MyColor4<T>(cb,cb,cb,1);
	}

	void operator +=(const MyColor4<T> &ca){
                T rr = ca.r*ca.a+r*a;
                if (rr>1) rr = 1;
                T gg = ca.g*ca.a+g*a;
                if (gg>1) gg = 1;
                T bb = ca.b*ca.a+b*a;
                if (bb>1) bb = 1;
                T aa = 1-(1-ca.a)*(1-a);
		r = rr;
		g = gg;
		b = bb;
		a = aa;
	}

	int getMaxIndex() const{
		if (r>g && r>b){
			return 0;
		}
		if (g>r &&g>b){
			return 1;
		}
		return 2;
	}

	int getMinIndex() const{
		if (r<g && r<b){
			return 0;
		}
		if (g<r &&g<b){
			return 1;
		}
		return 2;
	}

	T getMax() const{
		if (r>g && r>b){
			return r;
		}
		if (g>r &&g>b){
			return g;
		}
		return b;
	}

	T getMin() const{
		if (r<g && r<b){
			return r;
		}
		if (g<r &&g<b){
			return g;
		}
		return b;
	}

	T getH() const{
		T c = getMax()-getMin();
		if (c==0){
			return 0;
		}
		int maxIdx = getMaxIndex();
		T rt;
		if (maxIdx == 0){
			return ((int)((g-b)/c))%6 *60;
		}
		if (maxIdx == 1){
			return ((b-r)/c+2)*60;
		}
		return ((r-g)/c + 4)*60;
	}

	T getC() const{
		return getMax()-getMin();
	}
	T getI() const{
		return (r+g+b)/3;
	}

	T getV() const{
		return getMax();
	}

	T getL() const{
		return (getMax()+getMin())/2;
	}

	T getSaturation() const{
		return getC()/getV();
	}

	void fromHSV(const T h, const T s, const T v){
		a = 1;
		T c = v*s;
		T hp = h/60;
		T t = hp;
		while(t>=2) t-=2;
		T tmp = t-1;
		T m = v-c;
		tmp=tmp>0?tmp:-tmp;
		T x = c*(1-tmp);
		if (0<=hp && hp<1){
			r = c;
			g = x;
			b = 0;
		}
		else if (1<=hp && hp<2){
			r = x;
			g = c;
			b = 0;
		}
		else if (2<=hp && hp<3){
			r = 0;
			g = c;
			b = x;
		}
		else if (3<=hp && hp<4){
			r = 0;
			g = x;
			b = c;
		}
		else if (4<=hp && hp<5){
			r = x;
			g = 0;
			b = c;
		}
		else if (5<=hp && hp<6){
			r = c;
			g = 0;
			b = x;
		}
		else{
			r = g = b = 0;
		}
		r+=m;
		g+=m;
		b+=m;
	}
	static const MyColor4<T> black(){return MyColor4<T>(0,0,0,1);};
	static const MyColor4<T> white(){return MyColor4<T>(1,1,1,1);};
	static const MyColor4<T> yellow(){return MyColor4<T>(1,1,0,1);};

public:
	T r;
	T g;
	T b;
	T a;
};

typedef MyColor4<float> MyColor4f;
#endif //MyColor4<T>_h
