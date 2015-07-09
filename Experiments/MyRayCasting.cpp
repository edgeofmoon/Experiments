#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
using namespace std;

#include "Shader.h"

#include "MyTrackBall.h"
#include "MyGraphicsTool.h"
#include "MyBitmap.h"
#include "MyMesh.h"
#include "MyTracts.h"
#include "MyTracks.h"

#include "Ric/RicVolume.h"
#include "Ric/RicMesh.h"

bool bStereo = false;
// 0: middle, 1: left, 2:right
int eyeIfNonStereo = 0;
RicVolume vol;
MyMesh mesh;
MyTracts* ptract;
MyTracks track;

bool bdraw[4] = { false, true, false, false };

MyBitmap bitmap;
GLuint colorTex;

int windowWidth = 1024;
int windowHeight = 768;
float zDistance = 1000;


MyTrackBall trackBall;
int cubeProgram;
GLuint backTexture, cubefbo, depthBuffer;
GLuint rayfbo, rayTex, rayDepth;
float cubeBufferScale = 1;
GLuint volTex;
GLuint volGradTex;

GLuint vertexArray;
GLuint vertextBuffer, indexBuffer;

int shaderProgram; 
GLuint vertexArray2;
GLuint vertextBuffer2, indexBuffer2;

int meshProgram;
GLuint vertexArray3;
GLuint vertexBuffer3, normalBuffer3, colorBuffer3, indexBuffer3;

bool isRotating = false;

// shading parameter;
float decayFactor = 1000;
float thresHigh = 1.0;
float thresLow = 0.1;
float sampeRate = 512.f;

MyVec3f sizeLow(0, 0, 0);
MyVec3f sizeHigh(0.5, 0.5, 0.5);
int sizePosIdx = 2;


int faces[6][4] = {
	{ 0, 1, 3, 2 },
	{ 2, 3, 7, 6 },
	{ 6, 7, 5, 4 },
	{ 4, 5, 1, 0 },
	{ 4, 0, 2, 6 },
	{ 1, 5, 7, 3 },
};

int CompileShader(){
	shaderProgram = InitShader("raycasting.vert", "raycasting.frag", "fragColour");
	cubeProgram = InitShader("coord.vert", "coord.frag", "fragColour");
	meshProgram = InitShader("mesh.vert", "mesh.frag", "fragColour");
	return 1;
}

MyTracts* MakeTractsFromFile(const MyString& fileName){
	ifstream inFile(fileName);
	if (inFile.is_open()){
		int numTracts;
		inFile >> numTracts;
		//numTracts = 10;;
		MyTracts* ptracts = new MyTracts;
		MyTracts& tracts = *ptracts;
		for (int i = 0; i<numTracts; i++){
			int numPoints;
			inFile >> numPoints;
			tracts.NewTract(numPoints);
			for (int j = 0; j<numPoints; j++){
				MyVec3f p;
				inFile >> p[0] >> p[1] >> p[2];
				MyColor4f c;
				inFile >> c.r >> c.g >> c.b;
				c.a = 1;
				tracts << p;
				tracts << c;
			}
		}
		inFile.close();
		return ptracts;
	}
	else{
		cout << "Cannot open file " << fileName << endl;
		return 0;
	}
}

void drawTracts(MyTracts* tract){
	static unsigned int displayList = -1;
	if (!glIsList(displayList)){
		displayList = glGenLists(1);
		glNewList(displayList, GL_COMPILE);
		for (int i = 0; i < tract->GetNumTracts(); i++){
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j < tract->GetNumVertices(i); j++){
				MyGraphicsTool::Color(tract->GetColor(i, j));
				MyGraphicsTool::Vertex(tract->GetCoord(i, j));
			}
			glEnd();
		}
		glEndList();
	}
	glPushMatrix();
	glTranslatef(0.5, 0.5, 0.5);
	glScalef(1.f / vol.get_numx(), 1.f / vol.get_numy(), 1.f / vol.get_numz());
	MyGraphicsTool::Translate(-ptract->GetBoundingBox().GetCenter());
	glCallList(displayList);
	glPopMatrix();
}

MyVec3f calculateNormal(Point _p1, Point _p2, Point _pole){
	MyVec3f p1(_p1.x, _p1.y, _p1.z);
	MyVec3f p2(_p2.x, _p2.y, _p2.z);
	MyVec3f pole(_pole.x, _pole.y, _pole.z);

	return (p1 - p2) ^ pole;
}

void drawTracks(MyTracks* track){
	static unsigned int displayList = -1;
	if (!glIsList(displayList)){
		cout << "Building track display list " << endl;
		displayList = glGenLists(1);
		glNewList(displayList, GL_COMPILE);
		for (int i = 0; i < track->GetNumTracks(); i++){
			Point pole = track->GetPoint(i, track->GetNumVertex(i) - 1)
				- track->GetPoint(i, 0);
			glBegin(GL_LINE_STRIP);

			for (int j = 0; j < track->GetNumVertex(i); j++){
				Point p = track->GetPoint(i, j);
				Point normal;
				if (j == 0){
					normal = (p - track->GetPoint(i, j + 1)).CrossProduct(pole);
				}
				else{
					normal = (track->GetPoint(i, j - 1) - p).CrossProduct(pole);
				}
				normal.Normalize();
				glNormal3f(normal.x, normal.y, normal.z);
				float den = vol.get_at_index(p.x, vol.get_numy()-p.y, p.z);
				MyColor4f color = bitmap.GetColor(den*(bitmap.GetWidth() - 1), 0);
				MyGraphicsTool::Color(MyColor4f(color.b, color.g, color.r));
				//glColor3f(den, den, den);
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
			if (i % 100 == 0 || i == track->GetNumTracks() - 1){
				MyString progress(100 * i / (float)(track->GetNumTracks() - 1));
				progress.resize(5);
				cout << "\r" << progress << "%";
			}
		}
		glEndList();
		cout << "Complete." << endl;
	}
	glPushMatrix();
	glTranslatef(0.5, 0.5, 0.5);
	glScalef(1, -1, 1);
	glTranslatef(-0.5, -0.5, -0.5);
	glScalef(1.f / vol.get_numx(), 1.f / vol.get_numy(), 1.f / vol.get_numz());
	//glEnable(GL_LIGHTING);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	//glEnable(GL_LIGHT0);
	glCallList(displayList);
	//glDisable(GL_LIGHTING);
	glPopMatrix();
}

void drawMesh(const MyMesh& mesh){
	
	static int displayList = -1;
	if (!glIsList(displayList)){
		//MyBoundingBox box = mesh.GetBoundingBox();
		displayList = glGenLists(1);
		glNewList(displayList, GL_COMPILE);
		glPushMatrix();
		glTranslatef(0.5, 0.5, 0.5);
		glScalef(1.f / vol.get_numx(), 1.f / vol.get_numy(), 1.f / vol.get_numz());
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < mesh.GetNumTriangle(); i++){
			MyVec3f normal = mesh.ComputeTriangleNormal(i).normalized();
			MyVec3i triangle = mesh.GetTriangle(i);
			//glBegin(GL_LINE_LOOP);
			MyGraphicsTool::Color(MyColor4f(0.5, 0.5, 0.5, 1));
			//MyGraphicsTool::Color(MyColor4f(0.8,0.8,0.8, 0.2));
			MyGraphicsTool::Normal(normal);
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[0]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[1]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[2]));
			//glEnd();
		}
		glEnd();
		glPopMatrix();
		glEndList();
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float bias = -0.01;
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	GLdouble equa1[4] = { 0, 0, -1, sizeHigh[2] + bias };
	glClipPlane(GL_CLIP_PLANE0, equa1);
	glEnable(GL_CLIP_PLANE0);
	glCallList(displayList);
	glDisable(GL_CLIP_PLANE0);

	GLdouble equa2[4] = { 0, 1, 0, -(sizeHigh[1] - bias) };
	glClipPlane(GL_CLIP_PLANE0, equa2);
	glEnable(GL_CLIP_PLANE0);
	glCallList(displayList);
	glDisable(GL_CLIP_PLANE0);

	GLdouble equa3[4] = { -1, 0, 0, sizeHigh[0] + bias };
	glClipPlane(GL_CLIP_PLANE0, equa3);
	glEnable(GL_CLIP_PLANE0);
	glCallList(displayList);
	glDisable(GL_CLIP_PLANE0);

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	/*
	glPushMatrix();

	glTranslatef(0.5, 0.5, 0.5);
	glScalef(1.f / vol.get_numx(), 1.f / vol.get_numy(), 1.f / vol.get_numz());
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	glUseProgram(meshProgram);
	int location = glGetUniformLocation(meshProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, modelViewMat);
	location = glGetUniformLocation(meshProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);
	glBindVertexArray(vertexArray3);
	glDrawElements(GL_TRIANGLES, mesh.GetNumTriangle() * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_CULL_FACE);

	glPopMatrix();
	*/
}

void drawCube(){
	/*
	float vertices[8][3] = {
		{ 0, 0, 1 }, { 1, 0, 1 },
		{ 0, 1, 1 }, { 1, 1, 1 },
		{ 0, 0, 0 }, { 1, 0, 0 },
		{ 0, 1, 0 }, { 1, 1, 0 },
	};
	*/
	float vertices[8][4] = {
		{ 0, 0, 1, 1 }, { 1, 0, 1, 1 },
		{ 0, 1, 1, 1 }, { 1, 1, 1, 1 },
		{ 0, 0, 0, 1 }, { 1, 0, 0, 1 },
		{ 0, 1, 0, 1 }, { 1, 1, 0, 1 },
	};

	for (int i = 0; i < 6; i++){
		//glBegin(GL_TRIANGLE_FAN);
		glBegin(GL_LINE_LOOP);
		for (int j = 0; j < 4; j++){
			int idx = faces[i][j];
			glColor4fv(vertices[idx]);
			glVertex4fv(vertices[idx]);
		}
		glEnd();
	}

	/*
	glBegin(GL_TRIANGLE_FAN);
	glColor4fv(vertices[0]);
	glVertex4fv(vertices[0]);
	glColor4fv(vertices[3]);
	glVertex4fv(vertices[3]);
	glColor4fv(vertices[7]);
	glVertex4fv(vertices[7]);
	glColor4fv(vertices[4]);
	glVertex4fv(vertices[4]);
	glEnd();
	*/
}

MyVec3f* MakeGradients(){
	int xmax = vol.get_numx() - 1;
	int ymax = vol.get_numy() - 1;
	int zmax = vol.get_numz() - 1;
	MyVec3f* gradients = new MyVec3f[vol.nvox];
	for (int i = 0; i < vol.get_numx(); i++){
		for (int j = 0; j < vol.get_numy(); j++){
			for (int k = 0; k < vol.get_numz(); k++){
				float x0 = vol.vox[max(i - 1, 0)][j][k];
				float x1 = vol.vox[min(i + 1, xmax)][j][k];
				float y0 = vol.vox[i][max(j - 1, 0)][k];
				float y1 = vol.vox[i][min(j + 1, ymax)][k];
				float z0 = vol.vox[i][j][max(k - 1, 0)];
				float z1 = vol.vox[i][j][min(k + 1, zmax)];
				int idx = i*vol.get_numy()*vol.get_numz() + j * vol.get_numz() + k;
				gradients[idx] = MyVec3f(x1 - x0, y1 - y0, z1 - z0);
			}
		}
	}
	return gradients;
}

int RenderCubeCoords(){

	glBindFramebuffer(GL_FRAMEBUFFER, cubefbo);
	glEnable(GL_CULL_FACE);
	MyGraphicsTool::SetBackgroundColor(MyColor4f(0.5, 0.5, 0.5, 0));
	MyGraphicsTool::ClearFrameBuffer();
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth*cubeBufferScale, windowHeight*cubeBufferScale));

	glPushMatrix();
	glTranslatef(0, 0, -zDistance);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5, -0.5, -0.5);
	glCullFace(GL_FRONT);
	glUseProgram(cubeProgram);
	int location = glGetUniformLocation(cubeProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, modelViewMat);

	location = glGetUniformLocation(cubeProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);

	glBindVertexArray(vertexArray2);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glPopMatrix();

	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 1;
}

int LoadVolumeTexture(){
	if (glIsTexture(volTex)){
		glDeleteTextures(1, &volTex);
	}
	glGenTextures(1, &volTex);
	glBindTexture(GL_TEXTURE_3D, volTex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	// map x,y,z to z,y,x
	float *d = new float[vol.nvox];
	for (int i = 0; i < vol.get_numx(); i++){
		for (int j = 0; j < vol.get_numx(); j++){
			for (int k = 0; k < vol.get_numx(); k++){
				d[k*vol.get_numx()*vol.get_numy() + j*vol.get_numx()+i]
					= vol.vox[i][j][k];
			}

		}
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, d);
	delete[]d;
	
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, vol.varray);
	
	//glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);
	return 1;
}

int LoadVolumeGradientTexture(){
	if (glIsTexture(volGradTex)){
		glDeleteTextures(1, &volGradTex);
	}
	glGenTextures(1, &volGradTex);
	MyVec3f *grad = MakeGradients();
	glBindTexture(GL_TEXTURE_3D, volGradTex);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RGB, GL_FLOAT, grad);
	glBindTexture(GL_TEXTURE_3D, 0);
	delete grad;
	return 1;
}

int LoadTestVolumeTexture(){
	if (glIsTexture(volTex)){
		glDeleteTextures(1, &volTex);
	}
	glGenTextures(1, &volTex);
	glBindTexture(GL_TEXTURE_3D, volTex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	float* densitySphere = new float[vol.get_numx()*vol.get_numy()*vol.get_numz()];
	for (int i = 0; i < vol.get_numx()*vol.get_numy()*vol.get_numz(); i++){
		float z = i / (vol.get_numx()*vol.get_numy());
		float y = (i % (vol.get_numx()*vol.get_numy())) / vol.get_numx();
		float x = i % vol.get_numx();
		z = z / vol.get_numz() - 0.5;
		y = y / vol.get_numy() - 0.5;
		x = x / vol.get_numx() - 0.5;
		float radius =sqrtf( x*x + y*y + z*z);
		if (radius < 0.25 || radius > 0.35) densitySphere[i] = 0;
		else densitySphere[i] = max(1 - 2 * radius, 0.f);
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, densitySphere);
	//glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);
	delete[] densitySphere;
	return 1;
}

int LoadColorMap(){
	if (glIsTexture(colorTex)){
		glDeleteTextures(1, &colorTex);
	}
	glGenTextures(1, &colorTex);
	glBindTexture(GL_TEXTURE_2D, colorTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap.GetWidth(), bitmap.GetHeight(), 0, GL_BGR, GL_UNSIGNED_BYTE, bitmap.GetPixelBufferRGB());
	glBindTexture(GL_TEXTURE_2D, 0);
	return 1;
}

int BuildGLBuffers(){

	int w = windowWidth*cubeBufferScale;
	int h = windowHeight*cubeBufferScale;


	// cube
	if (glIsTexture(backTexture)) {
		glDeleteTextures(1, &backTexture);
	}
	glGenTextures(1, &backTexture);
	glBindTexture(GL_TEXTURE_2D, backTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (glIsFramebuffer(cubefbo)) {
		glDeleteFramebuffers(1, &cubefbo);
	}
	glGenFramebuffers(1, &cubefbo);
	glBindFramebuffer(GL_FRAMEBUFFER, cubefbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, backTexture, 0);
	if (glIsRenderbuffer(depthBuffer)){
		glDeleteRenderbuffers(1, &depthBuffer);
	}
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// render
	if (glIsTexture(rayTex)) {
		glDeleteTextures(1, &rayTex);
	}
	glGenTextures(1, &rayTex);
	glBindTexture(GL_TEXTURE_2D, rayTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	if (glIsFramebuffer(rayfbo)) {
		glDeleteFramebuffers(1, &rayfbo);
	}
	glGenFramebuffers(1, &rayfbo);
	glBindFramebuffer(GL_FRAMEBUFFER, rayfbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rayTex, 0);
	if (glIsRenderbuffer(rayDepth)){
		glDeleteRenderbuffers(1, &rayDepth);
	}
	glGenRenderbuffers(1, &rayDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rayDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rayDepth);
	glDrawBuffers(1, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return 1;
}

void RenderTexture(int texture){

	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));

	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(-1, 1, -1, 1, 1, 10));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());

	MyGraphicsTool::EnableTexture2D();
	MyGraphicsTool::BindTexture2D(texture);
	MyGraphicsTool::BeginTriangleFan();
	MyGraphicsTool::TextureCoordinate(MyVec2f(0, 0));
	//MyGraphicsTool::Color(MyColor4f(0, 0, 0));
	MyGraphicsTool::Vertex(MyVec3f(-1, -1, -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(1, 0));
	//MyGraphicsTool::Color(MyColor4f(1, 0, 0));
	MyGraphicsTool::Vertex(MyVec3f(1, -1, -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(1, 1));
	//MyGraphicsTool::Color(MyColor4f(1, 1, 0));
	MyGraphicsTool::Vertex(MyVec3f(1, 1, -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(0, 1));
	//MyGraphicsTool::Color(MyColor4f(0, 1, 0));
	MyGraphicsTool::Vertex(MyVec3f(-1, 1, -5));
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::BindTexture2D(0);
	MyGraphicsTool::DisableTexture2D();

	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();


}

void RenderRay(){

	//glBindFramebuffer(GL_FRAMEBUFFER, rayfbo);

	MyGraphicsTool::SetBackgroundColor(MyColor4f(0, 0, 0, 0));
	MyGraphicsTool::ClearFrameBuffer();
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth*cubeBufferScale, windowHeight*cubeBufferScale));
	//MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));
	glPushMatrix();
	glTranslatef(0, 0, -zDistance);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5, -0.5, -0.5);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (bdraw[3]){
		drawCube();
	}
	glUseProgram(shaderProgram);
	int location = glGetUniformLocation(shaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, modelViewMat);

	location = glGetUniformLocation(shaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);

	location = glGetUniformLocation(shaderProgram, "densityVol");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, volTex);

	if (glIsTexture(volGradTex)){
		location = glGetUniformLocation(shaderProgram, "gradVol");
		glUniform1i(location, 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_3D, volGradTex);
	}

	location = glGetUniformLocation(shaderProgram, "backFace");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, backTexture);

	location = glGetUniformLocation(shaderProgram, "colorMap");
	glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, colorTex);

	location = glGetUniformLocation(shaderProgram, "volSize");
	glUniform3f(location, vol.get_numx(), vol.get_numy(), vol.get_numz());

	location = glGetUniformLocation(shaderProgram, "cutCone");
	glUniform3f(location, sizeHigh[0], sizeHigh[1], sizeHigh[2]);

	location = glGetUniformLocation(shaderProgram, "windowWidth");
	glUniform1f(location, windowWidth);

	location = glGetUniformLocation(shaderProgram, "windowHeight");
	glUniform1f(location, windowHeight);

	location = glGetUniformLocation(shaderProgram, "decayFactor");
	glUniform1f(location, decayFactor);

	location = glGetUniformLocation(shaderProgram, "thresHigh");
	glUniform1f(location, thresHigh);

	location = glGetUniformLocation(shaderProgram, "thresLow");
	glUniform1f(location, thresLow);

	location = glGetUniformLocation(shaderProgram, "sampeRate");
	if (isRotating){
		glUniform1f(location, min(sampeRate,256.f));
	}
	else{
		glUniform1f(location, sampeRate);
	}

	if (bdraw[0]){
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glBindVertexArray(vertexArray);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDisable(GL_CULL_FACE);
	}
	glUseProgram(0);

	if (bdraw[2]){
		drawTracks(&track);
	}
	if (bdraw[1]){
		drawMesh(mesh);
	}
	glDisable(GL_BLEND);
	glPopMatrix();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void updateShaderData(){
	/*
	float vertices[8][3] = {
		{ sizeLow[0], sizeLow[1], sizeHigh[2] }, { sizeHigh[0], sizeLow[1], sizeHigh[2] },
		{ sizeLow[0], sizeHigh[1], sizeHigh[2] }, { sizeHigh[0], sizeHigh[1], sizeHigh[2] },
		{ sizeLow[0], sizeLow[1], sizeLow[2] }, { sizeHigh[0], sizeLow[1], sizeLow[2] },
		{ sizeLow[0], sizeHigh[1], sizeLow[2] }, { sizeHigh[0], sizeHigh[1], sizeLow[2] },
	};
	*/
	float vertices[8][3] = {
		{ 0, 0, 1 }, { 1, 0, 1 },
		{ 0, 1, 1 }, { 1, 1, 1 },
		{ 0, 0, 0 }, { 1, 0, 0 },
		{ 0, 1, 0 }, { 1, 1, 0 },
	};
	glBindVertexArray(vertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindVertexArray(vertexArray2);
	glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer2);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void loadShaderData(){
	// index
	int index[36];
	for (int i = 0; i < 6; i++){
		index[i * 6 + 0] = faces[i][0];
		index[i * 6 + 1] = faces[i][1];
		index[i * 6 + 2] = faces[i][2];
		index[i * 6 + 3] = faces[i][0];
		index[i * 6 + 4] = faces[i][2];
		index[i * 6 + 5] = faces[i][3];
	}
	float vertices[8][3] = {
		{ sizeLow[0], sizeLow[1], sizeHigh[2] }, { sizeHigh[0], sizeLow[1], sizeHigh[2] },
		{ sizeLow[0], sizeHigh[1], sizeHigh[2] }, { sizeHigh[0], sizeHigh[1], sizeHigh[2] },
		{ sizeLow[0], sizeLow[1], sizeLow[2] }, { sizeHigh[0], sizeLow[1], sizeLow[2] },
		{ sizeLow[0], sizeHigh[1], sizeLow[2] }, { sizeHigh[0], sizeHigh[1], sizeLow[2] },
	};

	// vertex
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertextBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
	int location = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// vertex2
	glGenVertexArrays(1, &vertexArray2);
	glBindVertexArray(vertexArray2);
	//glGenBuffers(1, &vertextBuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer2);
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
	//location = glGetAttribLocation(cubeProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//glGenBuffers(1, &indexBuffer2);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// vertex3
	// mesh
	glGenVertexArrays(1, &vertexArray3);
	glBindVertexArray(vertexArray3);
	// vertices
	glGenBuffers(1, &vertexBuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer3);
	glBufferData(GL_ARRAY_BUFFER, mesh.GetNumVertex() * sizeof(MyVec3f), mesh.GetVertexData(), GL_STATIC_DRAW);
	location = glGetAttribLocation(meshProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normals
	glGenBuffers(1, &normalBuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer3);
	glBufferData(GL_ARRAY_BUFFER, mesh.GetNumNormal() * sizeof(MyVec3f), mesh.GetNormalData(), GL_STATIC_DRAW);
	location = glGetAttribLocation(meshProgram, "normal");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_TRUE, 0, 0);
	// color
	glGenBuffers(1, &colorBuffer3);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer3);
	glBufferData(GL_ARRAY_BUFFER, mesh.GetNumNormal() * sizeof(MyVec3f), mesh.GetNormalData(), GL_STATIC_DRAW);
	location = glGetAttribLocation(meshProgram, "color");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_TRUE, 0, 0);

	glGenBuffers(1, &indexBuffer3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.GetNumTriangle() * sizeof(MyVec3i), mesh.GetTriangleData(), GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void display(){

	//glDrawBuffer(GL_BACK);
	MyGraphicsTool::SetBackgroundColor(MyColor4f(0, 0, 0, 0));
	MyGraphicsTool::ClearFrameBuffer();
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth*cubeBufferScale, windowHeight*cubeBufferScale));


	float caperture = 90;
	float cnear = 50;
	float cfar = zDistance*2;
	float displayDistance = zDistance;
	float eyesep = 65;
	float ratio = windowWidth / (double)windowHeight;
	float vwidth = 500.f;
	float vheight = 500 / ratio;

	if (bStereo){
		glDrawBuffer(GL_BACK_LEFT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float left = -(cnear * ((vwidth / 2.f - eyesep / 2.f) / displayDistance));
		float right = (cnear * ((vwidth / 2.f + eyesep / 2.f) / displayDistance));
		float top = (cnear * ((vheight / 2.f) / displayDistance));
		float bottom = -(cnear * ((vheight / 2.f) / displayDistance));
		glFrustum(left, right, bottom, top, cnear, cfar);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		gluLookAt(-eyesep / 2, 0, 0, -eyesep / 2, 0, -zDistance, 0, 1, 0);
		RenderCubeCoords();
		RenderRay();
		glPopMatrix();

		glDrawBuffer(GL_BACK_RIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -(cnear * ((vwidth / 2.f + eyesep / 2.f) / displayDistance));
		right = (cnear * ((vwidth / 2.f - eyesep / 2.f) / displayDistance));
		glFrustum(left, right, bottom, top, cnear, cfar);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		gluLookAt(eyesep / 2, 0, 0, eyesep / 2, 0, -zDistance, 0, 1, 0);
		RenderCubeCoords();
		RenderRay();
		glPopMatrix();
	}
	else{
		if (eyeIfNonStereo == 0){
			// middle eye
			eyesep = 0;
		}
		else if (eyeIfNonStereo == 1){
			// left eye
		}
		else{
			// right eye
			eyesep = -eyesep;
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		float left = -(cnear * ((vwidth / 2.f - eyesep / 2.f) / displayDistance));
		float right = (cnear * ((vwidth / 2.f + eyesep / 2.f) / displayDistance));
		float top = (cnear * ((vheight / 2.f) / displayDistance));
		float bottom = -(cnear * ((vheight / 2.f) / displayDistance));
		glFrustum(left, right, bottom, top, cnear, cfar);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		gluLookAt(-eyesep / 2, 0, 0, -eyesep / 2, 0, -zDistance, 0, 1, 0);
		/*
		//////insert exp
		glTranslatef(0, 0, -zDistance);
		//MyGraphicsTool::LoadTrackBall(&trackBall);
		glTranslatef(-0.5, -0.5, -0.5);
		glBindFramebuffer(GL_FRAMEBUFFER, cubefbo);
		MyGraphicsTool::ClearFrameBuffer();
		drawMesh(mesh);
		glutSolidSphere(100, 10, 10);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//return;
		//////end exp
		*/
		RenderCubeCoords();
		RenderRay();
		glPopMatrix();
	}

	glutSwapBuffers();
}

void mouseKey(int button, int state, int x, int y){
	if (state == GLUT_DOWN){
		trackBall.StartMotion(x, y);
		isRotating = true;
	}
	else if (state == GLUT_UP){
		trackBall.EndMotion(x, y);
		isRotating = false;
	}
	glutPostRedisplay();
}

void mouseMove(int x, int y){
	trackBall.RotateMotion(x, y);
	glutPostRedisplay();
}

void key(unsigned char c, int x, int y){
	switch (c)
	{
	case 27:
		exit(1);
		break;
	case '!':
		// superior
		trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(0, 0, 1, 0));
		glutPostRedisplay();
		break;
	case '@':
		// inferior
		trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(180, 0, 1, 0));
		glutPostRedisplay();
		break;
	case '#':
		// left
		trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(90, 0, 0, 1)*MyMatrixf::RotateMatrix(90, 0, 1, 0));
		glutPostRedisplay();
		break;
	case '$':
		// right
		trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(-90, 0, 0, 1)*MyMatrixf::RotateMatrix(-90, 0, 1, 0));
		glutPostRedisplay();
		break;
	case '%':
		// posterior
		trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(90, 1, 0, 0));
		glutPostRedisplay();
		break;
	case '^':
		// anterior
		trackBall.SetRotationMatrix(MyMatrixf::RotateMatrix(180, 0, 0, 1)*MyMatrixf::RotateMatrix(-90, 1, 0, 0));
		glutPostRedisplay();
		break;
	case '1':
	case '2':
	case '3':
	case '4':
		bdraw[c - '1'] = !bdraw[c - '1'];
		glutPostRedisplay();
		break;
	case 'z':
		trackBall.ScaleAdd(5);
		glutPostRedisplay();
		break;
	case 'Z':
		trackBall.ScaleAdd(-5);
		glutPostRedisplay();
		break;
	case 'd':
		decayFactor *= 1.2;
		glutPostRedisplay();
		break;
	case 'D':
		decayFactor /= 1.2;
		glutPostRedisplay();
		break;
	case 's':
		sampeRate = min(sampeRate*1.1f, 16384.f);
		glutPostRedisplay();
		break;
	case 'S':
		sampeRate = max(sampeRate/1.1f, 4.f);
		glutPostRedisplay();
		break;
	case 'h':
		thresHigh = min(thresHigh+0.05f, 1.0f);
		glutPostRedisplay();
		break;
	case 'H':
		thresHigh = max(thresHigh-0.05f, thresLow);
		glutPostRedisplay();
		break;
	case 'l':
		thresLow = min(thresLow + 0.05f, thresHigh);
		glutPostRedisplay();
		break;
	case 'L':
		thresLow = max(thresLow-0.05f, 0.f);
		glutPostRedisplay();
		break;
	case 'v':
	case 'V':
		sizePosIdx = (sizePosIdx + 1) % 3;
		cout << "size pos index: " << sizePosIdx << endl;
		glutPostRedisplay();
		break;
	case 'b':
		sizeLow[sizePosIdx] = min(sizeLow[sizePosIdx]+0.05f, sizeHigh[sizePosIdx]);
		updateShaderData();
		glutPostRedisplay();
		break;
	case 'B':
		sizeLow[sizePosIdx] = max(sizeLow[sizePosIdx] - 0.05f, 0.f);
		updateShaderData();
		glutPostRedisplay();
		break;
	case 'n':
		sizeHigh[sizePosIdx] = min(sizeHigh[sizePosIdx] + 0.05f, 1.f);
		updateShaderData();
		glutPostRedisplay();
		break;
	case 'N':
		sizeHigh[sizePosIdx] = max(sizeHigh[sizePosIdx] - 0.05f, sizeLow[sizePosIdx]);
		updateShaderData();
		glutPostRedisplay();
		break;
	case 'e':
	case 'E':
		eyeIfNonStereo = (eyeIfNonStereo + 1) % 3;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

void reshape(int w, int h){
	windowWidth = w;
	windowHeight = h;
	trackBall.Reshape(w, h);
	//MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(30, w / (float)h, 0.1f, 2000);
	//zDistance = 2;
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w/800.f,w/800.f,-h/800.f,h/800.f,0.1f,2000.f);
	zDistance = 500;
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	BuildGLBuffers();

	//glutPostRedisplay();
}

int main(int argc, char* argv[]){
	cout << "Loading volume ..." << endl;
	//vol.Read("average_s1.nii");
	vol.Read("target.nii.gz");
	vol /= vol.max;

	cout << "Loading mesh ..." << endl;
	int read = mesh.Read("lh.pial.obj");
	MyMesh mesh2;
	read = mesh2.Read("rh.pial.obj");
	mesh.Merge(mesh2);

	cout << "Calculating mesh normals ..." << endl;
	mesh.GenPerVertexNormal();

	cout << "Loading tracts ..." << endl;
	//ptract = MakeTractsFromFile("region_s5.data");
	//track.Read("ACR.trk");
	track.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti.trk");
	//MyGraphicsTool::Init(&argc, argv);
	glutInit(&argc, argv);


	glutInitWindowSize(1024, 768);
	
	if (bStereo){
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STEREO);
	}
	else{
		glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	}
	glutCreateWindow("Graph Explorer");
	glewInit();

	//glutFullScreen();

	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutMouseFunc(mouseKey);
	glutMotionFunc(mouseMove);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);

	CompileShader();
	LoadVolumeTexture();


	bitmap.Open("colorScale3.bmp");
	LoadColorMap();

	loadShaderData();
	trackBall.ScaleAdd(80);

	glutMainLoop();
	return 1;
}