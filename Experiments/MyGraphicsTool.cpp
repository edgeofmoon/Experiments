#include "MyGraphicsTool.h"
#include "MyTrackBall.h"

#include "gl/glut.h"
//#include <GL/freeglut.h>


void (*MyGraphicsTool::mDisplayFunc)(void) = 0;
void (*MyGraphicsTool::mReshapeFunc)(int, int) = 0;
void (*MyGraphicsTool::mKeyReleaseFunc)(unsigned char, int, int) = 0;
void (*MyGraphicsTool::mKeyPressFunc)(unsigned char, int, int) = 0;
void (*MyGraphicsTool::mMouseKeyFunc)(int, int ,int , int) = 0;
void (*MyGraphicsTool::mMouseMoveFunc)(int, int) = 0;
void (*MyGraphicsTool::mMousePassiveMoveFunc)(int, int) = 0;
void (*MyGraphicsTool::mIdleFunc)() = 0;

int	MyGraphicsTool::mWidth = 1024;
int	MyGraphicsTool::mHeight = 768;
MyColor4f MyGraphicsTool::mBackGroundColor;

MyGraphicsTool::MyGraphicsTool(void)
{
}


MyGraphicsTool::~MyGraphicsTool(void)
{
}

int MyGraphicsTool::GetError(){
	return glGetError();
}

MyMatrixd MyGraphicsTool::GetProjectionMatrix(){
	double projMat[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMat);
	return MyMatrixd(projMat,4,4,true);
}

MyMatrixd MyGraphicsTool::GetModelViewMatrix(){
	double modelViewMat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,modelViewMat);
	return MyMatrixd(modelViewMat,4,4,true);
}

MyVec4i MyGraphicsTool::GetViewport(){
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	return MyVec4i(viewport);
}

MyVec3f MyGraphicsTool::GetWindowPosFromWorldPos(const MyVec3f& worldCoord){
	double projMat[16];
	glGetDoublev(GL_PROJECTION_MATRIX,projMat);
	double modelViewMat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX,modelViewMat);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT,viewport);
	double wx, wy, wz;
	gluProject(worldCoord[0],worldCoord[1],worldCoord[2],
		modelViewMat,projMat,viewport,
		&wx, &wy, &wz);
	return MyVec3f(wx,wy,wz);
}

MyVec3f MyGraphicsTool::GetWorldPosFromWindowPos(const MyVec3f& winCoord){
	double projMat[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projMat);
	double modelViewMat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMat);
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	double wx, wy, wz;
	gluUnProject(winCoord[0], winCoord[1], winCoord[2],
		modelViewMat, projMat, viewport,
		&wx, &wy, &wz);
	return MyVec3f(wx, wy, wz);
}

MyVec3f MyGraphicsTool::GetProjection(const MyVec3f& worldCoord, 
	const MyMatrixd& modelViewMat, const MyMatrixd& projectionMat, const MyVec4i& viewport){
	double wx, wy, wz;
	gluProject((double)worldCoord[0],(double)worldCoord[1],(double)worldCoord[2],
		modelViewMat.GetData(),projectionMat.GetData(),
		viewport.d(), &wx, &wy, &wz);
	return MyVec3f(wx,wy,wz);
}
void MyGraphicsTool::Init(int* argc, char* argv[]){
	glutInit(argc,argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1024,768);
    glutCreateWindow( "Graph Explorer" );
	
	//glewInit();
}

void MyGraphicsTool::Start(){
    glutMainLoop();
}


void MyGraphicsTool::Update(){
	glutPostRedisplay();
}

void MyGraphicsTool::SetClearColor(const MyColor4f& clearColor){
	mBackGroundColor = clearColor;
}

MyColor4f MyGraphicsTool::GetClearColor(){
	return mBackGroundColor;
}

void MyGraphicsTool::ClearFrameBuffer(){
	glClearColor(mBackGroundColor.r,mBackGroundColor.g,mBackGroundColor.b,mBackGroundColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void MyGraphicsTool::FreshScreen(){
	glutSwapBuffers();
}

void MyGraphicsTool::SetFullScreen(){
	glutFullScreen();
}

void MyGraphicsTool::StartList(int& list){
	if(glIsList(list)){
		glDeleteLists(list,1);
	}
	list = glGenLists(1);
	glNewList(list,GL_COMPILE);
}
void MyGraphicsTool::EndList(int& list){
	glEndList();
}

void MyGraphicsTool::ShowList(int list){
	glCallList(list);
}

void MyGraphicsTool::DeleteList(int& list){
	if(glIsList(list)){
		glDeleteLists(list,1);
		list = -1;
	}
}

int MyGraphicsTool::ToSelectMode (){
	return glRenderMode(GL_SELECT);
}
int MyGraphicsTool::ToRenderMode (){
	return glRenderMode(GL_RENDER);
}

void MyGraphicsTool::FillPolygon(){
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void MyGraphicsTool::WirePolygon(){
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void MyGraphicsTool::PointPolygon(){
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

void MyGraphicsTool::SetPointSize(float psize){
	glPointSize(psize);
}

void MyGraphicsTool::SetLineWidth(float lineWidth){
	glLineWidth(lineWidth);
}

void MyGraphicsTool::BeginPoints(){
	glBegin(GL_POINTS);
}

void MyGraphicsTool::BeginQuads(){
	glBegin(GL_QUADS);
}

void MyGraphicsTool::BeginQuadStrip(){
	glBegin(GL_QUAD_STRIP);
}

void MyGraphicsTool::BeginLines(){
	glBegin(GL_LINES);
}

void MyGraphicsTool::BeginLineStrip(){
	glBegin(GL_LINE_STRIP);
}

void MyGraphicsTool::BeginLineLoop(){
	glBegin(GL_LINE_LOOP);
}

void MyGraphicsTool::BeginTriangleFan(){
	glBegin(GL_TRIANGLE_FAN);
}

//void MyGraphicsTool::Vertex(const MyVec4f& vec){
//	glVertex4f(vec[0],vec[1],vec[2],vec[3]);
//}

void MyGraphicsTool::Vertex(const MyVec3f& vec){
	glVertex3f(vec[0],vec[1],vec[2]);
}

void MyGraphicsTool::Vertex(const MyVec2f& vec){
	glVertex2f(vec[0],vec[1]);
}

void MyGraphicsTool::Vertices(const MyArray3f& vec){
	for(int i = 0;i<vec.size();i++){
		Vertex(vec[i]);
	}
}

void MyGraphicsTool::Vertices(const MyArray2f& vec){
	for(int i = 0;i<vec.size();i++){
		Vertex(vec[i]);
	}
}

void MyGraphicsTool::Normal(const MyVec3f& vec){
	glNormal3f(vec[0],vec[1],vec[2]);
}

void MyGraphicsTool::TextureCoordinate(const MyVec3f& vec){
	glTexCoord3f(vec[0],vec[1],vec[2]);
}

void MyGraphicsTool::TextureCoordinate(const MyVec2f& vec){
	glTexCoord2f(vec[0],vec[1]);
}

void MyGraphicsTool::Color(const MyColor4f& color){
	glColor4f(color.r,color.g,color.b,color.a);
}

void MyGraphicsTool::EndPrimitive(){
	glEnd();
}

void MyGraphicsTool::EnableAlplaBlending(){
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyGraphicsTool::DisableAlplaBlending(){
	glDisable (GL_BLEND);
}

void MyGraphicsTool::SetReverseColorAlphaBlending(){
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
}

void MyGraphicsTool::SetNormalAlphaBlending(){
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyGraphicsTool::InitHitBuffer(int n, unsigned int* buffer){
	glSelectBuffer(n, buffer);
}
void MyGraphicsTool::InitNameStack(){
	glInitNames();
}
void MyGraphicsTool::PushName(int name){
	glPushName(name);
}
void MyGraphicsTool::PopName(){
	glPopName();
}
void MyGraphicsTool::LoadName(int name){
	glLoadName(name);
}

void MyGraphicsTool::LoadTrackBall(const MyTrackBall* tractBall){
	// convert from row major to column major
	MyMatrixf mat = tractBall->Matrix().Transpose();
	//glLoadMatrixf(mat.GetData());
	glMultMatrixf(mat.GetData());
}

void MyGraphicsTool::LoadPickMatrix(int x, int y,  int w, int h, const MyMatrixf* perspectiveMat, const MyVec4i& viewport){
	MyMatrixf matTrans = perspectiveMat->Transpose();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	int v[] = {viewport[0],viewport[1],viewport[2],viewport[3]};
	gluPickMatrix(x, viewport[3] - y, w, h, v);
	glMultMatrixf(matTrans.GetData());
	glMatrixMode(GL_MODELVIEW);
}

void MyGraphicsTool::LoadProjectionMatrix(const MyMatrixf* mat){
    glMatrixMode(GL_PROJECTION);
	MyMatrixf matTrans = mat->Transpose();
	glLoadMatrixf(matTrans.GetData());
    glMatrixMode(GL_MODELVIEW);
}
void MyGraphicsTool::LoadModelViewMatrix(const MyMatrixf* mat){
    glMatrixMode(GL_MODELVIEW);
	MyMatrixf matTrans = mat->Transpose();
	glLoadMatrixf(matTrans.GetData());
}

void MyGraphicsTool::SetToByPixelMatrix(int width, int height, float _near, float _far){
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(
		0,width,0,height,_near,_far);
	MyMatrixf modelViewMatrix = MyMatrixf::IdentityMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&modelViewMatrix);

}

void MyGraphicsTool::Translate(const MyVec3f& offset){
	glTranslatef(offset[0], offset[1], offset[2]);
}

void MyGraphicsTool::Rotate(float angle, const MyVec3f& axis){
	glRotatef(angle,axis[0],axis[1],axis[2]);
}

void MyGraphicsTool::Scale(float s){
	glScalef(s,s,s);
}
void MyGraphicsTool::Scale(const MyVec3f& s){
	glScalef(s[0],s[1],s[2]);
}

void MyGraphicsTool::MultiplyMatrix(const MyMatrixf* mat){
	glMultMatrixf(mat->GetData());
}

void MyGraphicsTool::PushMatrix(){
	glPushMatrix();
}
void MyGraphicsTool::PopMatrix(){
	glPopMatrix();
}
void MyGraphicsTool::PushProjectionMatrix(){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
}
void MyGraphicsTool::PopProjectionMatrix(){
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void MyGraphicsTool::RasterPos(const MyVec3f& pos){
	glRasterPos3f(pos[0], pos[1], pos[2]);
}

void MyGraphicsTool::SetSize(int w, int h){
	mWidth = w;
	mHeight = h;
	glViewport(0,0,w,h);
}

void MyGraphicsTool::SetViewport(const MyVec4i& viewport){
	glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);
}

void MyGraphicsTool::SetBackgroundColor(MyColor4f backGroundColor){
	mBackGroundColor = backGroundColor;
}

void MyGraphicsTool::RegisterDisplayFunction(void (*displayFunc)(void)){
	mDisplayFunc = displayFunc;
	glutDisplayFunc( mDisplayFunc );
}
void MyGraphicsTool::RegisterReshapeFunction(void (*reshapeFunc)(int, int)){
	mReshapeFunc = reshapeFunc;
    glutReshapeFunc( mReshapeFunc );
}
void MyGraphicsTool::RegisterKeyReleaseFunction(void (*keyReleaseFunc)(unsigned char, int, int)){
	mKeyReleaseFunc = keyReleaseFunc;
	glutKeyboardUpFunc( mKeyReleaseFunc );
}
void MyGraphicsTool::RegisterKeyPressFunction(void (*keyPressFunc)(unsigned char, int, int)){
	mKeyPressFunc = keyPressFunc;
    glutKeyboardFunc( mKeyPressFunc );
}
void MyGraphicsTool::RegisterMouseKeyFunction(void (*mouseKeyFunc)(int, int ,int , int)){
	mMouseKeyFunc = mouseKeyFunc;
    glutMouseFunc( mMouseKeyFunc );
}
void MyGraphicsTool::RegisterMouseMoveFunction(void (*mouseMoveFunc)(int, int)){
	mMouseMoveFunc = mouseMoveFunc;
    glutMotionFunc( mMouseMoveFunc );
}

void MyGraphicsTool::RegisterMousePassiveMoveFunction(void (*mousePassiveMoveFunc)(int,int)){
	mMousePassiveMoveFunc = mousePassiveMoveFunc;
	glutPassiveMotionFunc( mousePassiveMoveFunc );
}

void MyGraphicsTool::RegisterIdleFunction(void (*idleFunc)(void)){
	mIdleFunc = idleFunc;
    glutIdleFunc( mIdleFunc );
}

void MyGraphicsTool::Sphere(float r, int slices, int stacks){
	glutSolidSphere(r,slices,stacks);
}

void MyGraphicsTool::BitmapChar(char c){
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12 , c);
}

void MyGraphicsTool::BitmapCharLarge(char c){
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18 , c);
}

int MyGraphicsTool::GetBitmapWidth(int character){
	return glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, character);
}

int MyGraphicsTool::GetBitmapLargeWidth(int character){
	return glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, character);
}

int MyGraphicsTool::GetBitmapHeight(int character){
	return 12;
}

int MyGraphicsTool::GetBitmapLargeHeight(int character){
	return 18;
}

void MyGraphicsTool::StrokeChar(char c){
	glutStrokeCharacter(GLUT_STROKE_ROMAN , c);
}

int MyGraphicsTool::GetStrokeWidth(int character){
	return glutStrokeWidth(GLUT_STROKE_ROMAN, character);
}

void MyGraphicsTool::StrokeCharMono(char c){
	glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN , c);
}

void MyGraphicsTool::DrawCurve(const MyArray3f& ctrlpoints, int segs){
	glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, ctrlpoints.size(), &(ctrlpoints[0][0]));
	glEnable(GL_MAP1_VERTEX_3);
	glBegin(GL_LINE_STRIP);
	for (int is = 0; is <= segs; is++) 
			glEvalCoord1f((GLfloat) is/segs);
	glEnd();
	glDisable(GL_MAP1_VERTEX_3);
}

void MyGraphicsTool::EnableTexture2D(){
	glEnable(GL_TEXTURE_2D);
}

void MyGraphicsTool::DisableTexture2D(){
	glDisable(GL_TEXTURE_2D);
}
bool MyGraphicsTool::IsTexture(int tex){
	return glIsTexture(tex);
}

MyArrayi MyGraphicsTool::GenerateTextures(int n){
	int* ids = new int[n];
	glGenTextures(n, (GLuint*)ids);
	MyArrayi rst(n);
	for(unsigned int i = 0;i<n;i++){
		rst[i] = ids[i];
	}
	delete ids;
	return rst;
}

void MyGraphicsTool::DeleteTextures(const MyArrayi& texs){
	if(texs.size() <= 0) return;
	glDeleteTextures(texs.size(), (const GLuint*)&texs[0]);
}

int MyGraphicsTool::GenerateTexture(){
	GLuint tex;
	glGenTextures(1, &tex);
	return tex;
}

void MyGraphicsTool::DeleteTexture(int tex){
	glDeleteTextures(1, (const GLuint*)&tex);
}

void MyGraphicsTool::BindTexture2D(int tex){
	glBindTexture(GL_TEXTURE_2D, tex);
}

void MyGraphicsTool::UnbindTexture2D(int tex){
	glBindTexture(GL_TEXTURE_2D, 0);
}

void MyGraphicsTool::SpecifyTextureImage2D(int width, int height, const float* data){
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void MyGraphicsTool::CopyToTextureImage2D(int x, int y, int width, int height){
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, width, height, 0);
}

void MyGraphicsTool::AutoSpecifyTexutreParamters(int idx){
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void MyGraphicsTool::GenerateMipMap2D(){
	//glGenerateMipmap(GL_TEXTURE_2D); 
}

void MyGraphicsTool::EnableClipPlane(int idx){
	glEnable(GL_CLIP_PLANE0+idx);
}

void MyGraphicsTool::DisableClipPlane(int idx){
	glDisable(GL_CLIP_PLANE0+idx);
}

void MyGraphicsTool::SetClipPlane(int idx, MyVec4f equa){
	GLdouble d[] = {equa[0],equa[1],equa[2],equa[3]};
	glClipPlane(GL_CLIP_PLANE0+idx, d);
}
