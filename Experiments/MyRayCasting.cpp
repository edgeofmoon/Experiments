#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shader.h"

#include "MyTrackBall.h"
#include "MyGraphicsTool.h"

#include "Ric/RicVolume.h"

RicVolume vol;

int windowWidth = 1024;
int windowHeight = 768;

MyTrackBall trackBall;

int CompileShader(char *vert, char* frag){
	int shaderProgram = InitShader(vert, frag, "fragColour");

	return 1;
}

void drawCube(){

	float vertices[8][3] =
	{ { 0, 0, 0 }, { 0, 0, 1 },
	{ 0, 1, 0 }, { 0, 1, 1 },
	{ 1, 0, 0 }, { 1, 0, 1 },
	{ 1, 1, 0 }, { 1, 1, 1 }, };

	int faces[6][4] = {
		{ 0, 1, 3, 2 },
		{ 2, 3, 7, 6 },
		{ 6, 7, 5, 4 },
		{ 4, 5, 1, 0 },
		{ 4, 0, 2, 6 },
		{ 1, 5, 7, 3 },
	};

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
	GLuint fbo, render_buf;
	glGenFramebuffers(1, &fbo);
	glGenRenderbuffers(1, &render_buf);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, windowWidth, windowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buf);


	glBindFramebuffer(GL_RENDERBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	return 1;
}

int LoadVolumeTexture(){
	GLuint volTex;
	glGenTextures(1, &volTex);
	glBindTexture(GL_TEXTURE_3D, volTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16UI, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_UNSIGNED_INT, vol.varray);
}

void display(){
	MyGraphicsTool::ClearFrameBuffer();
	glEnable(GL_CULL_FACE);
	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));

	// cube back
	glPushMatrix();
	glTranslatef(-2, 0, -5);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5, -0.5, -0.5);
	glCullFace(GL_FRONT);
	drawCube();
	glPopMatrix();

	// cube front
	glPushMatrix();
	glTranslatef(2, 0, -5);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5, -0.5, -0.5);
	glCullFace(GL_BACK);
	drawCube();
	glPopMatrix();

	glDisable(GL_CULL_FACE);
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

void reshape(int w, int h){
	windowWidth = w;
	windowHeight = h;
	trackBall.Reshape(w, h);
	//MyMatrixf projectionMatrix = MyMatrixf::PerspectiveMatrix(60, w / (float)h, 1, 10000);
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w/200.f,w/200.f,-h/200.f,h/200.f,0.1f,1000.f);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	glutPostRedisplay();
}

int main(int argc, char* argv[]){
	vol.Read("Control_GCC_60.nii");
	MyGraphicsTool::Init(&argc, argv);
	glEnable(GL_DEPTH_TEST);
	//glutInitWindowSize(windowWidth, windowHeight);
	//glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutMouseFunc(mouseKey);
	glutMotionFunc(mouseMove);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 1;
}