#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
using namespace std;

#include "Shader.h"

#include "MyTrackBall.h"
#include "MyGraphicsTool.h"
#include "MyPrimitiveDrawer.h"
#include "MyBitmap.h"
#include "MyMesh.h"
#include "MyTracts.h"
#include "MyTracks.h"

#include "Ric/RicVolume.h"
#include "Ric/RicMesh.h"

bool bStereo = false;
// 0: middle, 1: left, 2:right
int eyeIfNonStereo = 0;

int windowWidth = 1024;
int windowHeight = 768;
float zDistance = 1000;

MyTrackBall trackBall;
RicVolume trackVol;
GLuint trackVolTex;
RicVolume tVol;
GLuint tVolTex;
float tmin, tmax;
RicVolume faVol;
GLuint faVolTex;
MyMesh mesh;
MyTracks track;
MyBitmap bitmap;
GLuint colorTex;

bool isRotating = false;
// shading parameter;
float decayFactor = 1000;
float thresHigh = 1.0;
float thresLow = 0.1;
float sampeRate = 512.f;
bool bdraw[4] = { false, true, false, false };
MyVec3f sizeLow(0, 0, 0);
MyVec3f sizeHigh(0.5, 0.5, 0.5);
int sizePosIdx = 2;

typedef struct{
	GLuint width;
	GLuint height;
	GLuint frameBuffer;
	GLuint colorTexture;
	GLuint depthTexture;
}frameBuffer;

frameBuffer meshFbo;
frameBuffer cubeFbo;
int shaderProgram;
int meshProgram;
int cubeProgram;
int planeProgram;

GLuint planeVertexArray;
GLuint planeVertexBuffer, planeIndexBuffer;

GLuint cubeVertexArray;
GLuint cubeVertexBuffer, cubeIndexBuffer;

int CompileShader(){
	if (glIsProgram(shaderProgram)){
		glDeleteProgram(shaderProgram);
	}
	shaderProgram = InitShader("raycasting.vert", "raycasting.frag", "fragColour");
	if (glIsProgram(cubeProgram)){
		glDeleteProgram(cubeProgram);
	}
	cubeProgram = InitShader("coord.vert", "coord.frag", "fragColour");
	//meshProgram = InitShader("mesh.vert", "mesh.frag", "fragColour");
	if (glIsProgram(planeProgram)){
		glDeleteProgram(planeProgram);
	}
	planeProgram = InitShader("plane.vert", "plane.frag", "fragColour");
	return 1;
}

void BuildGLBuffers(frameBuffer& fb){
	// COLOR
	if (glIsTexture(fb.colorTexture)) {
		glDeleteTextures(1, &(fb.colorTexture));
	}
	glGenTextures(1, &(fb.colorTexture));
	glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb.width, fb.height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// DEPTH
	if (glIsTexture(fb.depthTexture)) {
		glDeleteTextures(1, &(fb.depthTexture));
	}
	glGenTextures(1, &(fb.depthTexture));
	glBindTexture(GL_TEXTURE_2D, fb.depthTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, fb.width, fb.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// FRAMEBUFFER ASSEMBLE
	if (glIsFramebuffer(fb.frameBuffer)) {
		glDeleteFramebuffers(1, &(fb.frameBuffer));
	}
	glGenFramebuffers(1, &(fb.frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, (fb.frameBuffer));
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fb.colorTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fb.depthTexture, 0);

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

int LoadVolumeTexture(GLuint &volTex, RicVolume& vol){
	if (glIsTexture(volTex)){
		glDeleteTextures(1, &volTex);
	}
	glGenTextures(1, &volTex);
	glBindTexture(GL_TEXTURE_3D, volTex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// map x,y,z to z,y,x
	float *d = new float[vol.nvox];
	for (int i = 0; i < vol.get_numx(); i++){
		for (int j = 0; j < vol.get_numx(); j++){
			for (int k = 0; k < vol.get_numx(); k++){
				d[k*vol.get_numx()*vol.get_numy() + j*vol.get_numx() + i]
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

void updateShaderData(){
	float size = 1;
	MyVec3f vertices[3][4] = {
		{ sizeHigh, sizeHigh, sizeHigh, sizeHigh },
		{ sizeHigh, sizeHigh, sizeHigh, sizeHigh },
		{ sizeHigh, sizeHigh, sizeHigh, sizeHigh }
	};
	vertices[0][1][0] = size;
	vertices[0][2][0] = size;
	vertices[0][2][1] = size;
	vertices[0][3][1] = size;

	vertices[1][1][1] = size;
	vertices[1][2][1] = size;
	vertices[1][2][2] = size;
	vertices[1][3][2] = size;

	vertices[2][1][2] = size;
	vertices[2][2][2] = size;
	vertices[2][2][0] = size;
	vertices[2][3][0] = size;

	glBindVertexArray(planeVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(MyVec3f), &vertices[0][0][0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int LoadPlaneShaderData(){
	float size = 1;
	MyVec3f vertices[3][4] = {
		{ sizeHigh, sizeHigh, sizeHigh, sizeHigh },
		{ sizeHigh, sizeHigh, sizeHigh, sizeHigh },
		{ sizeHigh, sizeHigh, sizeHigh, sizeHigh }
	};
	vertices[0][1][0] = size;
	vertices[0][2][0] = size;
	vertices[0][2][1] = size;
	vertices[0][3][1] = size;

	vertices[1][1][1] = size;
	vertices[1][2][1] = size;
	vertices[1][2][2] = size;
	vertices[1][3][2] = size;

	vertices[2][1][2] = size;
	vertices[2][2][2] = size;
	vertices[2][2][0] = size;
	vertices[2][3][0] = size;

	int index[18] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11
	};

	glGenVertexArrays(1, &planeVertexArray);
	glBindVertexArray(planeVertexArray);
	glGenBuffers(1, &planeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(MyVec3f), &vertices[0][0][0], GL_DYNAMIC_DRAW);
	int location = glGetAttribLocation(planeProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glGenBuffers(1, &planeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 18 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return 1;
}

int LoadCubeShaderData(){
	float vertices[8][3] = {
		{ 0, 0, 1 }, { 1, 0, 1 },
		{ 0, 1, 1 }, { 1, 1, 1 },
		{ 0, 0, 0 }, { 1, 0, 0 },
		{ 0, 1, 0 }, { 1, 1, 0 },
	};
	int faces[6][4] = {
		{ 0, 1, 3, 2 },
		{ 2, 3, 7, 6 },
		{ 6, 7, 5, 4 },
		{ 4, 5, 1, 0 },
		{ 4, 0, 2, 6 },
		{ 1, 5, 7, 3 },
	};
	int index[36];
	for (int i = 0; i < 6; i++){
		index[i * 6 + 0] = faces[i][0];
		index[i * 6 + 1] = faces[i][1];
		index[i * 6 + 2] = faces[i][2];
		index[i * 6 + 3] = faces[i][0];
		index[i * 6 + 4] = faces[i][2];
		index[i * 6 + 5] = faces[i][3];
	}
	glGenVertexArrays(1, &cubeVertexArray);
	glBindVertexArray(cubeVertexArray);
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
	int location = glGetAttribLocation(cubeProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return 1;
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
				float tScore = tVol.get_at_index(p.x + 0.5f, tVol.get_numy() - p.y - 1.5f, p.z + 0.5f);
				if (tScore < 0.1) continue;
				MyColor4f color = bitmap.GetColor(tScore*(bitmap.GetWidth() - 1), 0);
				float den = trackVol.get_at_index(p.x + 0.5f, tVol.get_numy() - p.y - 1.5f, p.z + 0.5f);
				MyGraphicsTool::Color(MyColor4f(color.b, color.g, color.r, den));
				//glColor3f(den, den, den);
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
			if (i % 1000 == 0 || i == track->GetNumTracks() - 1){
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
	glScalef(1.f / tVol.get_numx(), 1.f / tVol.get_numy(), 1.f / tVol.get_numz());
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
		glScalef(1.f / faVol.get_numx(), 1.f / faVol.get_numy(), 1.f / faVol.get_numz());
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < mesh.GetNumTriangle(); i++){
			MyVec3i triangle = mesh.GetTriangle(i);
			MyGraphicsTool::Color(MyColor4f(0.1, 0.1, 0.1, 1));
			MyGraphicsTool::Normal(mesh.GetNormal(triangle[0]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[0]));
			MyGraphicsTool::Normal(mesh.GetNormal(triangle[1]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[1]));
			MyGraphicsTool::Normal(mesh.GetNormal(triangle[2]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[2]));

			/*
			MyVec3f normal = mesh.ComputeTriangleNormal(i).normalized();
			MyGraphicsTool::Normal(normal);
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[0]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[1]));
			MyGraphicsTool::Vertex(mesh.GetVertex(triangle[2]));
			*/
			
		}
		glEnd();
		glPopMatrix();
		glEndList();
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float diffuse[] = { 0.1, 0.06, 0.05, 1 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, diffuse);

	float bias = -0.00;
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	GLdouble equa1[4] = { 0, 0, -1, sizeHigh[2] + bias };
	glClipPlane(GL_CLIP_PLANE0, equa1);
	glEnable(GL_CLIP_PLANE0);
	glCallList(displayList);
	glDisable(GL_CLIP_PLANE0);

	GLdouble equa2[4] = { 0, -1, 0, (sizeHigh[1] + bias) };
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
	MyGraphicsTool::Color(MyColor4f(1, 1, 1));
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

void RenderTexture(int texture, MyVec2f lowPos, MyVec2f highPos){
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(-1, 1, -1, 1, 1, 10));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	MyGraphicsTool::EnableTexture2D();
	MyGraphicsTool::BindTexture2D(texture);
	MyGraphicsTool::BeginTriangleFan();
	MyGraphicsTool::TextureCoordinate(MyVec2f(0, 0));
	MyGraphicsTool::Color(MyColor4f(1, 1, 1));
	MyGraphicsTool::Vertex(MyVec3f(lowPos[0], lowPos[1], -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(1, 0));
	//MyGraphicsTool::Color(MyColor4f(1, 0, 0));
	MyGraphicsTool::Vertex(MyVec3f(highPos[0], lowPos[1], -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(1, 1));
	//MyGraphicsTool::Color(MyColor4f(1, 1, 0));
	MyGraphicsTool::Vertex(MyVec3f(highPos[0], highPos[1], -5));
	MyGraphicsTool::TextureCoordinate(MyVec2f(0, 1));
	//MyGraphicsTool::Color(MyColor4f(0, 1, 0));
	MyGraphicsTool::Vertex(MyVec3f(lowPos[0], highPos[1], -5));
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::BindTexture2D(0);
	MyGraphicsTool::DisableTexture2D();

	MyString lowValue(tmin);
	MyString highValue(tmax);
	MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(lowPos[0], highPos[1], -5), lowValue, 1);
	MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(highPos[0], highPos[1], -5), highValue, 1);
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
}

void RenderLegend(MyVec2f lowPos, MyVec2f highPos){
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(-1, 1, -1, 1, 1, 10));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	float step = (highPos[0]-lowPos[0]) / (bitmap.GetWidth()-1);
	MyGraphicsTool::BeginQuadStrip();
	for (int i = 0; i < bitmap.GetWidth(); i++){
		MyGraphicsTool::Color(bitmap.GetColor(i, 0));
		MyGraphicsTool::Vertex(MyVec3f(lowPos[0] + step*i, lowPos[1], -5));
		MyGraphicsTool::Color(bitmap.GetColor(i, 1));
		MyGraphicsTool::Vertex(MyVec3f(lowPos[0] + step*i, highPos[1], -5));
	}
	MyGraphicsTool::EndPrimitive();

	MyString lowValue(tmin);
	MyString highValue(tmax);
	MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(lowPos[0], highPos[1], -5), lowValue, 1);
	MyPrimitiveDrawer::DrawBitMapTextLarge(MyVec3f(highPos[0], highPos[1], -5), highValue, 1);
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
}

void drawPlane(){

	glUseProgram(planeProgram);

	int location = glGetUniformLocation(planeProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, modelViewMat);

	location = glGetUniformLocation(planeProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);

	location = glGetUniformLocation(planeProgram, "densityVol");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, faVolTex);

	location = glGetUniformLocation(planeProgram, "meshDepthTex");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, meshFbo.depthTexture);

	location = glGetUniformLocation(planeProgram, "meshColorTex");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, meshFbo.colorTexture);

	location = glGetUniformLocation(planeProgram, "windowWidth");
	glUniform1f(location, meshFbo.width);

	location = glGetUniformLocation(planeProgram, "windowHeight");
	glUniform1f(location, meshFbo.height);

	glBindVertexArray(planeVertexArray);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

int RenderCubeCoords(){

	glBindFramebuffer(GL_FRAMEBUFFER, cubeFbo.frameBuffer);
	glEnable(GL_CULL_FACE);
	MyGraphicsTool::SetBackgroundColor(MyColor4f(0.5, 0.5, 0.5, 0));
	MyGraphicsTool::ClearFrameBuffer();
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));

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

	glBindVertexArray(cubeVertexArray);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 1;
}


void RenderRay(){

	//glBindFramebuffer(GL_FRAMEBUFFER, rayfbo);

	//MyGraphicsTool::SetBackgroundColor(MyColor4f(0, 0, 0, 0));
	//MyGraphicsTool::ClearFrameBuffer();
	//MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	glBindTexture(GL_TEXTURE_3D, trackVolTex);

	location = glGetUniformLocation(shaderProgram, "attriVol");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_3D, tVolTex);

	location = glGetUniformLocation(shaderProgram, "backFace");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, cubeFbo.colorTexture);

	location = glGetUniformLocation(shaderProgram, "colorMap");
	glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, colorTex);

	location = glGetUniformLocation(shaderProgram, "volSize");
	glUniform3f(location, trackVol.get_numx(), trackVol.get_numy(), trackVol.get_numz());

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
		glUniform1f(location, min(sampeRate, 256.f));
	}
	else{
		glUniform1f(location, sampeRate);
	}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindVertexArray(cubeVertexArray);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);

	glDisable(GL_BLEND);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void display(){

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glTranslatef(0, 0, -zDistance);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5, -0.5, -0.5);
	if (bdraw[0]){
		glBindFramebuffer(GL_FRAMEBUFFER, meshFbo.frameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawMesh(mesh);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		drawPlane();
		glDisable(GL_CULL_FACE);
	}
	if (bdraw[1]){
		//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawMesh(mesh);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	}
	if (bdraw[2]){
		//glEnable(GL_ALPHA_TEST);
		//glAlphaFunc(GL_GREATER, thresLow);
		drawTracks(&track);
		//glDisable(GL_ALPHA_TEST);

	}
	if (bdraw[3]){
		RenderCubeCoords();
		RenderRay();
	}
	RenderLegend(MyVec2f(-0.8, -1.f), MyVec2f(0.8, -0.9f));
	glPopMatrix();

	glutSwapBuffers();
}

void reshape(int w, int h){
	windowWidth = w;
	windowHeight = h;
	trackBall.Reshape(w, h);
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w / 80.f, w / 80.f, -h / 80.f, h / 80.f, 0.1f, 2000.f);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	meshFbo.width = w;
	meshFbo.height = h;
	cubeFbo.width = w;
	cubeFbo.height = h;
	BuildGLBuffers(meshFbo);
	BuildGLBuffers(cubeFbo);
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
		trackBall.ScaleMultiply(1.2);
		glutPostRedisplay();
		break;
	case 'Z':
		trackBall.ScaleMultiply(1/1.2);
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
		sampeRate = max(sampeRate / 1.1f, 4.f);
		glutPostRedisplay();
		break;
	case 'h':
		thresHigh = min(thresHigh + 0.05f, 1.0f);
		glutPostRedisplay();
		break;
	case 'H':
		thresHigh = max(thresHigh - 0.05f, thresLow);
		glutPostRedisplay();
		break;
	case 'l':
		thresLow = min(thresLow + 0.05f, thresHigh);
		glutPostRedisplay();
		break;
	case 'L':
		thresLow = max(thresLow - 0.05f, 0.f);
		glutPostRedisplay();
		break;
	case 'v':
	case 'V':
		sizePosIdx = (sizePosIdx + 1) % 3;
		cout << "size pos index: " << sizePosIdx << endl;
		glutPostRedisplay();
		break;
	case 'b':
		sizeLow[sizePosIdx] = min(sizeLow[sizePosIdx] + 0.05f, sizeHigh[sizePosIdx]);
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
	case 'r':
	case 'R':
		CompileShader();
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

int main(int argc, char* argv[]){
	cout << "Loading volumes ..." << endl;
	//vol.Read("average_s1.nii");
	faVol.Read("target.nii.gz");
	faVol /= faVol.max;
	tVol.Read("average.nii");
	tmin = tVol.min;
	tmax = tVol.max;
	tVol /= tVol.max;
	trackVol.Read("ACR_300.nii");
	trackVol /= trackVol.max;

	cout << "Loading mesh ..." << endl;
	int read = mesh.Read("lh.pial.obj");
	MyMesh mesh2;
	read = mesh2.Read("rh.pial.obj");
	mesh.Merge(mesh2);
	cout << "Calculating mesh normals ..." << endl;
	mesh.GenPerVertexNormal();

	cout << "Loading tracts ..." << endl;
	//track.Read("ACR.trk");
	track.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\ACR_10.trk");

	bitmap.Open("colorScale3.bmp");

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

	CompileShader();
	LoadPlaneShaderData();
	LoadCubeShaderData();
	LoadColorMap();
	LoadVolumeTexture(faVolTex, faVol);
	LoadVolumeTexture(tVolTex, tVol);
	LoadVolumeTexture(trackVolTex, trackVol);

	//glutFullScreen();

	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutMouseFunc(mouseKey);
	glutMotionFunc(mouseMove);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutMainLoop();
	return 1;
}