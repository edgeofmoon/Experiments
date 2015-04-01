#include "MyQuarternion.h"

/*
http://www.cprogramming.com/tutorial/3d/quaternions.html
*/

// please keep it normalized all time
MyQuarternion::MyQuarternion(void)
{
	_d = MyVec<unit,4>(1,0,0,0);
}

MyQuarternion::MyQuarternion(unit angle, unit axisx, unit axisy, unit axisz){
	/*local_rotation.w  = cosf( fAngle/2)
	local_rotation.x = axis.x * sinf( fAngle/2 )
	local_rotation.y = axis.y * sinf( fAngle/2 )
	local_rotation.z = axis.z * sinf( fAngle/2 )
	*/
	_d[0] = cosf(angle/2);
	unit half_sin = sinf(angle/2);
	_d[1] = axisx*half_sin;
	_d[2] = axisy*half_sin;
	_d[3] = axisz*half_sin;
}

MyQuarternion::MyQuarternion(unit angle, MyVec<unit,3> dir){
	_d[0] = cosf(angle/2);
	unit half_sin = sinf(angle/2);
	_d[1] = dir[0]*half_sin;
	_d[2] = dir[1]*half_sin;
	_d[3] = dir[2]*half_sin;
}

MyQuarternion::~MyQuarternion(void)
{
}

MyQuarternion operator*(const MyQuarternion&a, const MyQuarternion&b){
	/*
	(Q1 * Q2).w = (w1w2 - x1x2 - y1y2 - z1z2)
	(Q1 * Q2).x = (w1x2 + x1w2 + y1z2 - z1y2)
	(Q1 * Q2).y = (w1y2 - x1z2 + y1w2 + z1x2)
	(Q1 * Q2).z = (w1z2 + x1y2 - y1x2 + z1w2
	*/
	MyQuarternion rst;
	rst[0] = a[0]*b[0] - a[1]*b[1] - a[2]*b[2] - a[3]*b[3];
	rst[1] = a[0]*b[1] + a[1]*b[0] + a[2]*b[3] - a[3]*b[2];
	rst[2] = a[0]*b[2] - a[1]*b[3] + a[2]*b[0] + a[3]*b[1];
	rst[3] = a[0]*b[3] + a[1]*b[2] - a[2]*b[1] + a[3]*b[0];
	return rst;
}

void MyQuarternion::normalize(){
	_d.normalize();
}

unit& MyQuarternion::operator[](int i){
	return _d[i];
}

unit MyQuarternion::operator[](int i) const{
	return _d[i];
}

MyMatrix<unit> MyQuarternion::GetMatrix() const{
	unit w = _d[0];
	unit x = _d[1];
	unit y = _d[2];
	unit z = _d[3];
	unit eles[4][4] = {
		{1-2*y*y-2*z*z,	2*x*y-2*w*z,	2*x*z+2*w*y,	0},
		{2*x*y+2*w*z,	1-2*x*x-2*z*z,	2*y*z+2*w*x,	0},
		{2*x*z-2*w*y,	2*y*z-2*w*x,	1-2*x*x-2*y*y,	0},
		{0, 0,	0,	1}
	};
	return MyMatrix<unit>(&eles[0][0],4,4);
}