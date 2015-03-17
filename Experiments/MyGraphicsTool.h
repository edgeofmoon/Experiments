#pragma once

#include "MyVec.h"
#include "MyMatrix.h"
#include "MyColor4.h"
#include "MyArray.h"

class MyTrackBall;

// warp some handy openGL code
class MyGraphicsTool
{
public:
	MyGraphicsTool(void);
	~MyGraphicsTool(void);

	static int GetError();
	// openGL query
	static MyMatrixd GetProjectionMatrix();
	static MyMatrixd GetModelViewMatrix();
	static MyVec4i GetViewport();
	static MyVec3f GetWindowPosFromWorldPos(const MyVec3f& worldCoord);
	static MyVec3f GetWorldPosFromWindowPos(const MyVec3f& winCoord);

	// this one requires column major matrix
	static MyVec3f GetProjection(const MyVec3f& worldCoord, 
		const MyMatrixd& modelViewMat, const MyMatrixd& projectionMat,
		const MyVec4i& viewport);

	static void Init(int* argc, char* argv[]);
	static void Start();
	static void Update();

	static void SetClearColor(const MyColor4f& clearColor);
	static MyColor4f GetClearColor();
	static void ClearFrameBuffer();
	static void FreshScreen();
	static void SetFullScreen();

	static void StartList(int& list);
	static void EndList(int& list);
	static void ShowList(int list);
	static void DeleteList(int& list);

	static int ToSelectMode(); 
	static int ToRenderMode();

	static void FillPolygon();
	static void WirePolygon();
	static void PointPolygon();
	static void SetPointSize(float psize);
	static void SetLineWidth(float lineWidth);

	static void BeginPoints();
	static void BeginQuads();
	static void BeginQuadStrip();
	static void BeginLines();
	static void BeginLineStrip();
	static void BeginLineLoop();
	static void BeginTriangleFan();
	//static void Vertex(const MyVec4f& vec);
	static void Vertex(const MyVec3f& vec);
	static void Vertex(const MyVec2f& vec);
	static void Vertices(const MyArray3f& vec);
	static void Vertices(const MyArray2f& vec);
	static void Normal(const MyVec3f& vec);
	static void TextureCoordinate(const MyVec3f& vec);
	static void TextureCoordinate(const MyVec2f& vec);
	static void Color(const MyColor4f& color);
	static void EndPrimitive();

	static void EnableAlplaBlending();
	static void DisableAlplaBlending();
	static void SetReverseColorAlphaBlending();
	static void SetNormalAlphaBlending();
	// must call hit buffer before change to select mode
	static void InitHitBuffer(int n, unsigned int* buffer);
	static void InitNameStack();
	static void PushName(int name);
	static void PopName();
	static void LoadName(int name);

	static void LoadTrackBall(const MyTrackBall* tractBall);
	static void LoadPickMatrix(int x, int y, int w, int h, const MyMatrixf* perspectiveMat, const MyVec4i& viewport);
	static void LoadProjectionMatrix(const MyMatrixf* mat);
	static void LoadModelViewMatrix(const MyMatrixf* mat);
	static void SetToByPixelMatrix(int width, int height, float near, float far);
	static void Translate(const MyVec3f& offset);
	static void Rotate(float angle, const MyVec3f& axis);
	static void Scale(float s);
	static void Scale(const MyVec3f& s);
	static void MultiplyMatrix(const MyMatrixf* mat);
	static void PushMatrix();
	static void PopMatrix();
	static void PushProjectionMatrix();
	static void PopProjectionMatrix();

	static void RasterPos(const MyVec3f& pos);
	
	static void SetSize(int w, int h);
	static void SetViewport(const MyVec4i& viewport);

	// from glut
	static void SetBackgroundColor(MyColor4f backGroundColor);
	static void RegisterDisplayFunction(void (*displayFunc)(void));
	static void RegisterReshapeFunction(void (*reshapeFunc)(int, int));
	static void RegisterKeyReleaseFunction(void (*keyReleaseFunc)(unsigned char, int, int));
	static void RegisterKeyPressFunction(void (*keyPressFunc)(unsigned char, int, int));
	static void RegisterMouseKeyFunction(void (*mouseKeyFunc)(int, int ,int , int));
	static void RegisterMouseMoveFunction(void (*mouseMoveFunc)(int, int));
	static void RegisterMousePassiveMoveFunction(void (*mousePassiveMoveFunc)(int,int));
	static void RegisterIdleFunction(void (*idleFunc)(void));

	static void Sphere(float r = 1, int slices = 10, int stacks = 10);

	static void BitmapChar(char c);
	static void BitmapCharLarge(char c);
	static int GetBitmapWidth(int character);
	static int GetBitmapLargeWidth(int character);
	static int GetBitmapHeight(int character = '0');
	static int GetBitmapLargeHeight(int character = '0');
	static void StrokeChar(char c);
	static int GetStrokeWidth(int character);
	static void StrokeCharMono(char c);

	static void DrawCurve(const MyArray3f& ctrlpoints, int segs = 10);

	// texture
	static void EnableTexture2D();
	static void DisableTexture2D();
	static bool IsTexture(int tex);
	static int GenerateTexture();
	static MyArrayi GenerateTextures(int n = 1);
	static void DeleteTexture(int tex);
	static void DeleteTextures(const MyArrayi& texs);
	static void BindTexture2D(int tex);
	static void UnbindTexture2D(int tex);
	static void SpecifyTextureImage2D(int width, int height, const float* data = 0);
	static void CopyToTextureImage2D(int x, int y, int width, int height);
	static void AutoSpecifyTexutreParamters(int idx = 0);
	static void GenerateMipMap2D();

	//clipPlan
	static void EnableClipPlane(int idx);
	static void DisableClipPlane(int idx);
	static void SetClipPlane(int idx, MyVec4f equa);

protected:
	static void (*mDisplayFunc)(void);
	static void (*mReshapeFunc)(int, int);
	static void (*mKeyReleaseFunc)(unsigned char, int, int);
	static void (*mKeyPressFunc)(unsigned char, int, int);
	static void (*mMouseKeyFunc)(int, int ,int , int);
	static void (*mMouseMoveFunc)(int, int);
	static void (*mMousePassiveMoveFunc)(int, int);
	static void (*mIdleFunc)(void);

	static int mWidth, mHeight;
	static MyColor4f mBackGroundColor;
};

