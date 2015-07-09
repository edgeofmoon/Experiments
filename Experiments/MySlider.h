#pragma once

template<typename Type>
class MySlider
{
public:
	MySlider();
	~MySlider();

	void Show();
	void SetValueRange(const Type&start, const Type&end);
	void SetSteps(int n);
	Type GetValue() const;
	void BindValue(Type* valuePtr);
	void BindDirty(bool* dirtyPtr);
	void SetQuad(int left, int bottom, int width, int height);

	void MouseKey(int button, int state, int x, int y);
	void MouseMove(int x, int y);

protected:
	Type mStart;
	Type mEnd;
	int mSteps;

	void updateValues(const Type& newValue);
	Type mCurrentValue;
	Type* mValuePtr;
	bool* mDirtyPtr;
	bool mBeingDrag;

	int mLeft, mBottom, mWidth, mHeight;
};

