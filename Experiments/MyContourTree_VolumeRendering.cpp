#include <GL/glew.h>
#include <GL/freeglut.h>
#include "MyContourTree.h"
#include "Shader.h"
#include <string>
#include <queue>
using namespace std;

void MyContourTree::MarkSelectedArcVoxes(long arc, float isoValue){
	vector<float*>& voxes = mArcNodes[arc];
	for (int i = 0; i < voxes.size(); i++){
		if (compareHeight(voxes[i], &isoValue) < 0) break;
		long x, y, z;
		height.ComputeIndex(voxes[i], x, y, z);
		mMaskVolume(x, y, z) = 1;
	}

	queue<long> arcQueue;
	long nextArc = supernodes[superarcs[arc].topID].upList;
	for (int i = 0; i < supernodes[superarcs[arc].topID].upDegree; i++){
		arcQueue.push(nextArc);
		nextArc = superarcs[nextArc].nextUp;
	}

	while (!arcQueue.empty()){
		long theArc = arcQueue.front();
		arcQueue.pop();
		long nextArc = supernodes[superarcs[theArc].topID].upList;
		for (int i = 0; i < supernodes[superarcs[theArc].topID].upDegree; i++){
			arcQueue.push(nextArc);
			nextArc = superarcs[nextArc].nextUp;
		}
		vector<float*>& voxes = mArcNodes[theArc];
		for (int i = 0; i < voxes.size(); i++){
			long x, y, z;
			height.ComputeIndex(voxes[i], x, y, z);
			mMaskVolume(x, y, z) = 1;
		}
	}
}

void MyContourTree::MarkSelectedVoxes(){
	ZeroMemory(&mMaskVolume(0, 0, 0), mMaskVolume.NElements()*sizeof(float));
	int theArc;
	for (theArc = 0; theArc < nActiveArcs; theArc++)
	{ // for theArc
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSuppressed)) continue;
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSelected)) continue;
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSelected))
			MarkSelectedArcVoxes(active[theArc], currentSelectionValue);
		else
			MarkSelectedArcVoxes(active[theArc], superarcs[active[theArc]].seedValue);
	} // for theArc
	for (theArc = 0; theArc < nSelectedArcs; theArc++)
	{ // for theArc
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSelected))
			MarkSelectedArcVoxes(active[theArc], currentSelectionValue);
		else
			MarkSelectedArcVoxes(active[theArc], superarcs[active[theArc]].seedValue);
	} // for theArc

	// update the volume too
	loadMarkVolumeTexture();
}

void MyContourTree::VolumeRenderingSelectedVoxes(int winWidth, int winHeight){

	// cube
	glBindFramebuffer(GL_FRAMEBUFFER, cubeFrameBuffer.frameBuffer);
	glEnable(GL_CULL_FACE);
	glClearColor(0.5, 0.5, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

	// render ray
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(shaderProgram);
	location = glGetUniformLocation(shaderProgram, "mvMat");
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, modelViewMat);

	location = glGetUniformLocation(shaderProgram, "projMat");
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);

	location = glGetUniformLocation(shaderProgram, "faVol");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_3D, volTex);

	location = glGetUniformLocation(shaderProgram, "markVol");
	glUniform1i(location, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_3D, markTex);

	location = glGetUniformLocation(shaderProgram, "backFace");
	glUniform1i(location, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, cubeFrameBuffer.colorTexture);

	location = glGetUniformLocation(shaderProgram, "colorMap");
	glUniform1i(location, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, colorTex);

	location = glGetUniformLocation(shaderProgram, "volSize");
	glUniform3f(location, height.XDim(), height.YDim(), height.ZDim());

	location = glGetUniformLocation(shaderProgram, "windowWidth");
	glUniform1f(location, winWidth);

	location = glGetUniformLocation(shaderProgram, "windowHeight");
	glUniform1f(location, winHeight);

	location = glGetUniformLocation(shaderProgram, "decayFactor");
	glUniform1f(location, 1000);

	location = glGetUniformLocation(shaderProgram, "sampeRate");
	glUniform1f(location, 1024);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glBindVertexArray(cubeVertexArray);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
	glUseProgram(0);

	glDisable(GL_BLEND);

}

void MyContourTree::SetupVolomeRenderingBuffers(int width, int height){
	ReCompileShaders();
	cubeFrameBuffer.width = width;
	cubeFrameBuffer.height = height;
	setupCubeFrameBuffer();
	loadCubeShaderData();
	loadVolumeTexture();
}

void MyContourTree::loadCubeShaderData(){
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
	if (glIsVertexArray(cubeVertexArray)){
		glDeleteVertexArrays(1, &cubeVertexArray);
	}
	glGenVertexArrays(1, &cubeVertexArray);
	glBindVertexArray(cubeVertexArray);
	if (glIsBuffer(cubeVertexBuffer)){
		glDeleteBuffers(1, &cubeVertexBuffer);
	}
	glGenBuffers(1, &cubeVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
	int location = glGetAttribLocation(cubeProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if (glIsBuffer(cubeIndexBuffer)){
		glDeleteBuffers(1, &cubeIndexBuffer);
	}
	glGenBuffers(1, &cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(int), &index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void MyContourTree::setupCubeFrameBuffer(){
	if (glIsTexture(cubeFrameBuffer.colorTexture)) {
		glDeleteTextures(1, &(cubeFrameBuffer.colorTexture));
	}
	glGenTextures(1, &(cubeFrameBuffer.colorTexture));
	glBindTexture(GL_TEXTURE_2D, cubeFrameBuffer.colorTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cubeFrameBuffer.width, cubeFrameBuffer.height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// DEPTH
	if (glIsTexture(cubeFrameBuffer.depthTexture)) {
		glDeleteTextures(1, &(cubeFrameBuffer.depthTexture));
	}
	glGenTextures(1, &(cubeFrameBuffer.depthTexture));
	glBindTexture(GL_TEXTURE_2D, cubeFrameBuffer.depthTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, cubeFrameBuffer.width, cubeFrameBuffer.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// FRAMEBUFFER ASSEMBLE
	if (glIsFramebuffer(cubeFrameBuffer.frameBuffer)) {
		glDeleteFramebuffers(1, &(cubeFrameBuffer.frameBuffer));
	}
	glGenFramebuffers(1, &(cubeFrameBuffer.frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, (cubeFrameBuffer.frameBuffer));
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cubeFrameBuffer.colorTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, cubeFrameBuffer.depthTexture, 0);

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyContourTree::loadVolumeTexture(){
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
	float *d = new float[height.NElements()];
	for (int i = 0; i < height.XDim(); i++){
		for (int j = 0; j < height.YDim(); j++){
			for (int k = 0; k < height.ZDim(); k++){
				d[k*height.XDim()*height.YDim() + j*height.ZDim() + i]
					= height(i,j,k);
			}

		}
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, height.ZDim(), height.YDim(), height.XDim(), 0, GL_RED, GL_FLOAT, d);
	delete[]d;
	glBindTexture(GL_TEXTURE_3D, 0);
}


void MyContourTree::loadMarkVolumeTexture(){
	if (glIsTexture(markTex)){
		glDeleteTextures(1, &markTex);
	}
	glGenTextures(1, &markTex);
	glBindTexture(GL_TEXTURE_3D, markTex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	// map x,y,z to z,y,x
	float *d = new float[mMaskVolume.NElements()];
	for (int i = 0; i < mMaskVolume.XDim(); i++){
		for (int j = 0; j < mMaskVolume.YDim(); j++){
			for (int k = 0; k < mMaskVolume.ZDim(); k++){
				d[k*mMaskVolume.XDim()*mMaskVolume.YDim() + j*mMaskVolume.ZDim() + i]
					= mMaskVolume(i, j, k);
			}

		}
	}
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, mMaskVolume.ZDim(), mMaskVolume.YDim(), mMaskVolume.XDim(), 0, GL_RED, GL_FLOAT, d);
	delete[]d;
	glBindTexture(GL_TEXTURE_3D, 0);
}

void MyContourTree::ReCompileShaders(){

	if (glIsProgram(shaderProgram)){
		glDeleteProgram(shaderProgram);
	}
	shaderProgram = InitShader("selVol.vert", "selVol.frag", "fragColour");

	if (glIsProgram(cubeProgram)){
		glDeleteProgram(cubeProgram);
	}
	cubeProgram = InitShader("coord.vert", "coord.frag", "fragColour");
}