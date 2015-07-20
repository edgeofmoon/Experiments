#include "MySlider.h"
#include "MyGraphicsTool.h"

template<typename Type>
MySlider<Type>::MySlider()
{
	mValuePtr = NULL;
	mDirtyPtr = NULL;
	mBeingDrag = false;
	mSteps = 100;
}


template<typename Type>
MySlider<Type>::~MySlider()
{
}

template<typename Type>
void MySlider<Type>::Show(){
	MyGraphicsTool::SetViewport(MyVec4i(mLeft, mBottom, mWidth, mHeight));
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::IdentityMatrix());
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 0, 1));
	MyGraphicsTool::Color(MyColor4f(0, 0, 0));
	MyGraphicsTool::WirePolygon();
	MyGraphicsTool::BeginQuads();
	float unitPerPixelX = 1.f / mWidth;
	float unitPerPixelY = 1.f / mHeight;
	MyGraphicsTool::Vertex(MyVec3f(0 + unitPerPixelX, 0.5 - unitPerPixelY, -0.5));
	MyGraphicsTool::Vertex(MyVec3f(1 - unitPerPixelX, 0.5 - unitPerPixelY, -0.5));
	MyGraphicsTool::Vertex(MyVec3f(1 - unitPerPixelX, 0.5 + unitPerPixelY, -0.5));
	MyGraphicsTool::Vertex(MyVec3f(0 + unitPerPixelX, 0.5 + unitPerPixelY, -0.5));
	MyGraphicsTool::EndPrimitive();
	float xPos = (mCurrentValue - mStart) / (float)(mEnd - mStart);
	float sliderHalfWidth = 4.5 * unitPerPixelX;
	float sliderHalfHeight = 0.4;
	MyGraphicsTool::BeginQuads();
	MyGraphicsTool::Vertex(MyVec3f(xPos - sliderHalfWidth, 0.5 - sliderHalfHeight, -0.5));
	MyGraphicsTool::Vertex(MyVec3f(xPos + sliderHalfWidth, 0.5 - sliderHalfHeight, -0.5));
	MyGraphicsTool::Vertex(MyVec3f(xPos + sliderHalfWidth, 0.5 + sliderHalfHeight, -0.5));
	MyGraphicsTool::Vertex(MyVec3f(xPos - sliderHalfWidth, 0.5 + sliderHalfHeight, -0.5));
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::FillPolygon();
	MyGraphicsTool::Color(MyColor4f(1, 1, 1));
	sliderHalfWidth = 4 * unitPerPixelX;
	sliderHalfHeight = 0.4 - 0.5*unitPerPixelY;
	MyGraphicsTool::BeginQuads();
	MyGraphicsTool::Vertex(MyVec3f(xPos - sliderHalfWidth, 0.5 - sliderHalfHeight, - 0.4));
	MyGraphicsTool::Vertex(MyVec3f(xPos + sliderHalfWidth, 0.5 - sliderHalfHeight, -0.4));
	MyGraphicsTool::Vertex(MyVec3f(xPos + sliderHalfWidth, 0.5 + sliderHalfHeight, -0.4));
	MyGraphicsTool::Vertex(MyVec3f(xPos - sliderHalfWidth, 0.5 + sliderHalfHeight, -0.4));
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::PopProjectionMatrix();
	MyGraphicsTool::PopMatrix();
}


template<typename Type>
void MySlider<Type>::SetValueRange(const Type&start, const Type&end){
	mStart = start;
	mEnd = end;
}

template<typename Type>
void MySlider<Type>::SetSteps(int n){
	mSteps = n;
}

template<typename Type>
void MySlider<Type>::SetValue(Type value){
	mCurrentValue = value;
}

template<typename Type>
Type MySlider<Type>::GetValue() const{
	return mCurrentValue;
}

template<typename Type>
void MySlider<Type>::BindValue(Type* valuePtr){
	mValuePtr = valuePtr;
}

template<typename Type>
void MySlider<Type>::BindDirty(bool* dirtyPtr){
	mDirtyPtr = dirtyPtr;
}

template<typename Type>
void MySlider<Type>::SetQuad(int left, int bottom, int width, int height){
	mLeft = left;
	mBottom = bottom;
	mWidth = width;
	mHeight = height;
}

template<typename Type>
void MySlider<Type>::MouseKey(int button, int state, int x, int y){
	if (state == GLUT_DOWN){
		if (x - mLeft >= 0 && x - mLeft <= mWidth 
			&& y - mBottom >= 0 && y - mBottom < mHeight){
			int step = (x - mLeft) / (float)mWidth*mSteps;
			Type newValue = mStart + (mEnd - mStart)*step / (float)mSteps;
			updateValues(newValue);
			mBeingDrag = true;
		}
	}
	else if (state == GLUT_UP){
		mBeingDrag = false;
	}
}

template<typename Type>
void MySlider<Type>::MouseMove(int x, int y){
	if (mBeingDrag){
		if (x - mLeft >= 0 && x - mLeft <= mWidth
			&& y - mBottom >= 0 && y - mBottom < mHeight){
			int step = (x - mLeft) / (float)mWidth*mSteps;
			Type newValue = mStart + (mEnd - mStart)*step / (float)mSteps;
			updateValues(newValue);
		}
	}
}

template<typename Type>
void MySlider<Type>::updateValues(const Type& newValue){
	if (newValue != mCurrentValue){
		mCurrentValue = newValue;
		if (mValuePtr != NULL){
			(*mValuePtr) = mCurrentValue;
		}
		if (mDirtyPtr != NULL){
			(*mDirtyPtr) = true;
		}
	}
}