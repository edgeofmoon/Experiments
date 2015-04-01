#pragma once
#include "mygraphicstool.h"
#include "MyLine.h"
#include "MyBoundingBox.h"
#include <string>

template<typename T, int n>
class MyPolyline;

template<typename T, int n>
class MyLine;

class MyPrimitiveDrawer :
	public MyGraphicsTool
{
public:
	MyPrimitiveDrawer(void);
	~MyPrimitiveDrawer(void);

	static void DrawQuadAt(const MyVec3f& pos0,
		const MyVec3f& pos1,
		const MyVec3f& pos2,
		const MyVec3f& pos3);
	static void DrawQuadAtsAt(const std::vector<MyVec3f>& vecs);

	static int GetBitMapTextWidth(const MyString& str);
	static int GetBitMapTextLargeWidth(const MyString& str);
	static int GetStrokeWidth(const MyString& str);
	static void DrawLineAt(const MyVec3f& s, const MyVec3f& e);
	static void DrawLineAt(const MyVec2f& s, const MyVec2f& e);
	static void DrawLine(const MyLine<float, 3>& line);
	static void DrawLine(const MyLine<float, 2>& line);
	static void DrawSphereAt(const MyVec3f& n, float r = 1.f);
	static void DrawCircle(const MyVec3f& n, float r = 1.f, int segs = 20);
	// alignment: 0-left, 1-middle, 2-right
	static void DrawBitMapText(const MyVec3f& pos, const std::string& text, int alignment = 0);
	static void DrawBitMapTextLarge(const MyVec3f& pos, const std::string& text, int alignment = 0);
	static void DrawStrokeText(const MyVec3f& pos, const std::string& text, const MyVec3f& scale = MyVec3f(1.f,1.f,1.f));
	static void DrawStrokeTextUpDowm(const MyVec3f& pos, const std::string& text, const MyVec3f& scale = MyVec3f(1.f,1.f,1.f));
	static void Draw(const MyPolyline<float,3>& polyline);
	static void Draw(const MyPolyline<float,2>& polyline);

	static MyBoundingBox GetBitMapTextBox(const MyString& text, MyVec3f offset = MyVec3f::zero());
	static MyBoundingBox GetBitMapLargeTextBox(const MyString& text, MyVec3f offset = MyVec3f::zero());
};

