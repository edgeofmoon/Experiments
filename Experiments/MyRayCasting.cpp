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
GLuint frontTexture, backTexture, cubefbo, cuberenderbuff;
GLuint volTex;

int shaderProgram; 
GLuint vertexArray;

float size = 3;
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

int CompileShader(char *vert, char* frag){
	shaderProgram = InitShader(vert, frag, "fragColour");
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

	glPushMatrix();
	glTranslatef(0, 0, -500);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5*size, -0.5*size, -0.5*size);
	glCullFace(GL_FRONT);
	drawCube();
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, backTexture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, windowWidth, windowHeight, 0);
	
	MyGraphicsTool::ClearFrameBuffer();
	glPushMatrix();
	glTranslatef(0, 0, -500);
	MyGraphicsTool::LoadTrackBall(&trackBall);
	glTranslatef(-0.5*size, -0.5*size, -0.5*size);
	glCullFace(GL_BACK);
	drawCube();
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, frontTexture);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, windowWidth, windowHeight, 0);

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
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, vol.get_numx(), vol.get_numy(), vol.get_numz(), 0, GL_RED, GL_FLOAT, vol.varray);
	glBindTexture(GL_TEXTURE_3D, 0);
	return 1;
}

void display(){

	RenderCubeCoords();
	MyGraphicsTool::SetBackgroundColor(MyColor4f(0, 0, 0, 0));
	MyGraphicsTool::ClearFrameBuffer();

	glPushMatrix();
	glTranslatef(0, 0, -500);
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

	location = glGetUniformLocation(shaderProgram, "frontFace");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, frontTexture);

	location = glGetUniformLocation(shaderProgram, "volSize");
	glUniform3f(location, vol.get_numx(), vol.get_numy(), vol.get_numz());

	location = glGetUniformLocation(shaderProgram, "windowWidth");
	glUniform1f(location, windowWidth);

	location = glGetUniformLocation(shaderProgram, "windowHeight");
	glUniform1f(location, windowHeight);

	glBindVertexArray(vertexArray);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	glPopMatrix();

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
	case 'x':
		trackBall.ScaleAdd(-0.05);
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
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w/200.f,w/200.f,-h/200.f,h/200.f,0.1f,1000.f);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());

	if (glIsTexture(frontTexture)) {
		glDeleteTextures(1, &backTexture);
	}
	glGenTextures(1, &frontTexture);
	glBindTexture(GL_TEXTURE_2D, frontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);


	if (glIsTexture(backTexture)) {
		glDeleteTextures(1, &backTexture);
	}
	glGenTextures(1, &backTexture);
	glBindTexture(GL_TEXTURE_2D, backTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	if (glIsRenderbuffer(cuberenderbuff)) {
		glDeleteRenderbuffers(1, &cuberenderbuff);
	}
	glGenRenderbuffers(1, &cuberenderbuff);
	glBindRenderbuffer(GL_RENDERBUFFER, cuberenderbuff);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, windowWidth, windowHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glIsFramebuffer(cubefbo)) {
		glDeleteFramebuffers(1, &cubefbo);
	}
	glGenFramebuffers(1, &cubefbo);
	glBindFramebuffer(GL_FRAMEBUFFER, cubefbo);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cuberenderbuff);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	MyGraphicsTool::SetViewport(MyVec4i(0, 0, windowWidth, windowHeight));

	//glutPostRedisplay();
}

int main(int argc, char* argv[]){
	MyGraphicsTool::Init(&argc, argv);
	glewInit();
	glutDisplayFunc(display);
	glutMouseFunc(mouseKey);
	glutMotionFunc(mouseMove);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);

	vol.Read("Control_GCC_60.nii");
	vol /= vol.max;
	CompileShader("raycasting.vert", "raycasting.frag");
	LoadVolumeTexture();


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

	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glutMainLoop();
	return 1;
}