#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
using namespace std;

#include "Shader.h"

#include "MyTrackBall.h"
#include "MyGraphicsTool.h"

#include "Ric/RicVolume.h"

RicVolume vol;

int windowWidth = 1024;
int windowHeight = 768;

MyTrackBall trackBall;
int cubeProgram;
GLuint backTexture, cubefbo, depthBuffer;
float cubeBufferScale = 1;
GLuint volTex;
GLuint vertexArray2;

int shaderProgram; 
GLuint vertexArray;

// shading parameter;
float decayFactor = 10;
float thresHigh = 1.0;
float thresLow = 0.0;
float sampeRate = 100.f;

float size = 1;
float vertices[8][3] = {
	{ 0, 0, size }, { 0, size, size },
	{ size, 0, size }, { size, size, size },
	{ 0, 0, 0 }, { 0, size, 0 },
	{ size, 0, 0 }, { size, size, 0 },
};

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
	return 1;
}

void drawCube(){


	for (int i = 0; i < 6; i++){
		glBegin(GL_TRIANGLE_FAN);
		for (int j = 0; j < 4; j++){
			int idx = faces[i][j];
			glColor3fv(vertices[idx]);
			glVertex3fv(vertices[idx]);
		}
		glEnd();
	}
}

int RenderCubeCoords(){

	glBindFramebuffer(GL_FRAMEBUFFER, cubefbo);
	glEnable(GL_CULL_FACE);
	MyGraphicsTool::SetBackgroundColor(MyColor4f(0.5, 0.5, 0.5, 0));
	MyGraphicsTool::ClearFrameBuffer();
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth*cubeBufferScale, windowHeight*cubeBufferScale));

	glPushMatrix();
	glTranslatef(0, 0, -1000);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5*size, -0.5*size, -0.5*size);
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

	//MyGraphicsTool::Color(MyColor4f(1, 1, 1));
	//glutSolidSphere(20, 10, 10);
	
	glPopMatrix();

	//glBindTexture(GL_TEXTURE_2D, backTexture);
	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, windowWidth*cubeBufferScale, windowHeight*cubeBufferScale, 0);
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
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, vol.varray);
	glBindTexture(GL_TEXTURE_3D, 0);
	return 1;
}

int BuildGLBuffers(){

	int w = windowWidth*cubeBufferScale;
	int h = windowHeight*cubeBufferScale;

	if (glIsTexture(backTexture)) {
		glDeleteTextures(1, &backTexture);
	}
	glGenTextures(1, &backTexture);
	glBindTexture(GL_TEXTURE_2D, backTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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

void display(){


	RenderCubeCoords();

	MyGraphicsTool::SetBackgroundColor(MyColor4f(0, 0, 0, 0));
	MyGraphicsTool::ClearFrameBuffer();
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));

	
	glPushMatrix();
	glTranslatef(0, 0, -1000);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5*size, -0.5*size, -0.5*size);

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

	location = glGetUniformLocation(shaderProgram, "backFace");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, backTexture);

	location = glGetUniformLocation(shaderProgram, "volSize");
	glUniform3f(location, vol.get_numx(), vol.get_numy(), vol.get_numz());

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
	glUniform1f(location, sampeRate);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindVertexArray(vertexArray);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);

	glPopMatrix();
	//RenderTexture(backTexture);

	glutSwapBuffers();
}

void mouseKey(int button, int state, int x, int y){
	if (state == GLUT_DOWN){
		trackBall.StartMotion(x, y);
	}
	else if (state == GLUT_UP){
		trackBall.EndMotion(x, y);
	}
}

void mouseMove(int x, int y){
	trackBall.RotateMotion(x, y);
	glutPostRedisplay();
}

void key(unsigned char c, int x, int y){
	switch (c)
	{
	case 'z':
		trackBall.ScaleAdd(0.05);
		glutPostRedisplay();
		break;
	case 'Z':
		trackBall.ScaleAdd(-0.05);
		glutPostRedisplay();
		break;
	case 'd':
		decayFactor *= 2;
		glutPostRedisplay();
		break;
	case 'D':
		decayFactor /= 2;
		glutPostRedisplay();
		break;
	case 's':
		sampeRate = min(sampeRate*1.1f, 512.f);
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
	default:
		break;
	}
}

void reshape(int w, int h){
	windowWidth = w;
	windowHeight = h;
	trackBall.Reshape(w, h);
	//MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, w / (float)h, 1, 10000);
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w/800.f,w/800.f,-h/800.f,h/800.f,0.1f,2000.f);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	BuildGLBuffers();

	//glutPostRedisplay();
}

int main(int argc, char* argv[]){
	MyGraphicsTool::Init(&argc, argv);
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutMouseFunc(mouseKey);
	glutMotionFunc(mouseMove);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);

	//vol.Read("Control_GCC_60.nii");
	vol.Read("dti_fa.nii");
	vol /= vol.max;
	CompileShader();
	LoadVolumeTexture();

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


	// vertex
	GLuint vertextBuffer, indexBuffer;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	glGenBuffers(1, &vertextBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
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
	GLuint vertextBuffer2, indexBuffer2;
	glGenVertexArrays(1, &vertexArray2);
	glBindVertexArray(vertexArray2);
	glGenBuffers(1, &vertextBuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertextBuffer2);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
	location = glGetAttribLocation(cubeProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &indexBuffer2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	trackBall.ScaleAdd(0.6);

	glutMainLoop();
	return 1;
}