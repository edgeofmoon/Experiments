#include "Helper.h"

float ***f3tensor(long l, long m, long n)
{
	float ***t;
	t=new float ** [l];
	t[0]= new float *[l*m];
	t[0][0]= new float [l*m*n];
	for(int j=1;j<m;j++) 
		t[0][j]=t[0][j-1]+n;
   
	for(int i=1;i<l;i++) 
	{
        t[i]=t[i-1]+m;
        t[i][0]=t[i-1][0]+m*n;
        for(int j=1;j<m;j++) 
			t[i][j]=t[i][j-1]+n;
    }	
	t[0][0][0]=0;
	t[1][1][0]=0;
    return t;
}

void free_f3tensor(float ***t, long l, long m, long n)
{
	if(l>=1&&m>=1&&n>=1)
	{
		delete [] t[0][0];
	    delete [] t[0];
	    delete [] t;
	}
}


char ***c3tensor(long l, long m, long n)
{
	char ***t;
	t=new char ** [l];
	t[0]= new char *[l*m];
	t[0][0]= new char [l*m*n];
	for(int j=1;j<m;j++) 
		t[0][j]=t[0][j-1]+n;
   
	for(int i=1;i<l;i++) 
	{
        t[i]=t[i-1]+m;
        t[i][0]=t[i-1][0]+m*n;
        for(int j=1;j<m;j++) 
			t[i][j]=t[i][j-1]+n;
    }	
	t[0][0][0]=0;
	t[1][1][0]=0;
    return t;
}

void free_c3tensor(char ***t, long l, long m, long n)
{
	if(l>=1&&m>=1&&n>=1)
	{
		delete [] t[0][0];
	    delete [] t[0];
	    delete [] t;
	}
}


int **i2vector(long l, long m)
{
	int **t;
	t=new int*[l];
    t[0]=new int[l*m];
    for(int i=1;i<l;i++)
    t[i]=t[0]+i*m;	
    return t;
}

void free_i2vector(int **t, long l, long m)
{
	if(l>=1&&m>=1)
	{
		delete [] t[0];  delete [] t; 
	}
}

float **f2vector(long l, long m)
{
	float **t;
	t=new float*[l];
    t[0]=new float[l*m];
    for(int i=1;i<l;i++)
    t[i]=t[0]+i*m;	
    return t;
}

void free_f2vector(float **t, long l, long m)
{
	if(l>=1&&m>=1)
	{
		delete [] t[0];  delete [] t; 
	}
}
   

void num_to_ij(int i, int j, int id, int &ni, int &nj)
{
        switch(id)
        {
             case 0: ni=i-1, nj=j; break;
                 case 1: ni=i, nj=j+1; break;
                 case 2: ni=i+1, nj=j+1; break;
                 case 3: ni=i+1, nj=j; break;
                 case 4: ni=i, nj=j-1; break;
                 case 5: ni=i-1, nj=j-1; break;
        }
}

int ij_to_num(int i, int j, int ni, int nj, int &id)
{
   switch(ni-i)
   {
       case -1: id=nj==j?0:5; break;
           case 0: id=nj>j?1:4; break;
           case 1: id=nj==j?3:2; break;
   }
   return id;
}

void binsort(float *r,int n,bool sortascending)
{
  int i;
  for(i=2;i<=n;i++)
  binpass(r,i,sortascending);
}

//(binary Insertion Sort)
void binpass(float *r, int i,bool sortascending)
{//对r[1...n]进行折半插入排序
  float x;
  int s,m,k,j;
  
  r[0]=r[i]; x=r[i];
  s=1; j=i-1;
  while(s<j)
  {
  m=int((s+j)/2);  
  if(sortascending)
  {if(x<r[m]) j=m-1;
    else s=m+1;}
  else 
  { if(x>r[m]) j=m-1;
    else s=m+1;
  }
  }
  for(k=i-1;k>=j+1;k--)
  {
  r[k+1]=r[k];
  }
  r[j+1]=r[0];
} 

float value(float n1,float n2,float hue)
{
	float v;
	if(hue > 360) hue = hue - 360;
	if(hue < 0)   hue = hue + 360;
    if(hue < 60)
		v = n1 + ( n2 - n1) * hue / 60;
	else if(hue < 180) 
		v = n2;
	else if(hue < 240)
		v = n1 + (n2 - n1) * (240 - hue) / 60;
	else 
		v = n1;
	return v;
}

void HLS_TO_RGB(float h,float l,float ss,float *r,float * g,float *b)//颜色转换
{
//L:亮度（色深）
//h:[0度,360度)颜彩）(hue)
//s:饱和度[0,1] (saturation)
	float m1,m2;
	if(l <= 0.5) m2 = l * (1 + ss);
	else m2 = l + ss - l * ss;
	m1 = 2 * l - m2;
	if(ss == 0) 
	  //if(h == 0) 
	    *r = *g = *b = l;
	   //else 
		//AfxMessageBox("error!");
	else
	{
	    *r = value(m1,m2,h + 120);
	    *g = value(m1,m2,h);
		*b = value(m1,m2,h - 120 );
	}
}




