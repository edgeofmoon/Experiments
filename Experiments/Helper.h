#pragma once

struct f1d_ID //带编号的浮点数{
{
  int id;
  float x;
};

float ***f3tensor(long l, long m, long n);
void free_f3tensor(float ***t, long l, long m, long n);
char ***c3tensor(long l, long m, long n);
void free_c3tensor(char ***t, long l, long m, long n);

int **i2vector(long l, long m);
void free_i2vector(int **t, long l, long m);
void free_f2vector(float **t, long l, long m);
float **f2vector(long l, long m);

void num_to_ij(int i, int j, int id, int &ni, int &nj);
int ij_to_num(int i, int j, int ni, int nj, int &id);
void binsort(float *r,int n,bool sortascending);
void binpass(float *r, int i,bool sortascending);

float value(float n1,float n2,float hue);
void HLS_TO_RGB(float h,float l,float ss,float *r,float * g,float *b); //颜色转换

