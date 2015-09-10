#include <string.h>
#include <algorithm>
#include <GL/glew.h>
#include "GL/glui.h"

#include "MyContourTree.h"
#include "MyGraphicsTool.h"
#include "MyTrackBall.h"
#include "MyFrameBuffer.h"
#include "Shader.h"
#include "MyMesh.h"
#include "MyTracks.h"

#define BOARDER_X 0.05
#define BOARDER_Y 0.05
#define BOARDER_LEFT (-BOARDER_X)
#define BOARDER_RIGHT (1+BOARDER_X)
#define BOARDER_BOTTOM (-BOARDER_Y)
#define BOARDER_TOP (1+BOARDER_Y)
#define BOARDER_X_RANGE (1+BOARDER_X+BOARDER_X)
#define BOARDER_Y_RANGE (1+BOARDER_Y+BOARDER_Y)

bool showTracks = false;
bool showMesh = false;
bool showSecTree = true;
bool showDiffTree = true;
int gl_error;

/**************************************** Shared ********************/
bool isMagnifying = false;
int magnifier_xPos, magnifier_yPos;
int magnifier_radius = 100;
float magnifier_scale = 5;
float fishEye_focusRadiusRatio = 0.5;
bool useFisyeye = true;
MyTracks track;
/**************************************** GL ********************/
#define DSR_FACTOR_X 2
#define DSR_FACTOR_Y 2
int windowWidth = 1800;
int windowHeight = 800;

// for pick up region names
int* nameMap = 0;
// for contour trees
frameBuffer drawBuffer;
// for contours
frameBuffer contourBuffer;
// for fisheye
frameBuffer fishEyeFBO;
int fisheyeShader;
GLuint fisheye_vertexArray;
GLuint fisheye_vertexBuffer, fisheye_indexBuffer;

void BuildGLBuffers(frameBuffer& fb, bool hasName = false){
	// COLOR
	if (glIsTexture(fb.colorTexture)) {
		glDeleteTextures(1, &(fb.colorTexture));
	}
	glGenTextures(1, &(fb.colorTexture));
	glBindTexture(GL_TEXTURE_2D, fb.colorTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

	// name
	if (glIsTexture(fb.nameTexture)) {
		glDeleteTextures(1, &(fb.nameTexture));
	}
	if (hasName){
		glGenTextures(1, &(fb.nameTexture));
		glBindTexture(GL_TEXTURE_2D, fb.nameTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, fb.width, fb.height, 0, GL_RGBA_INTEGER, GL_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// FRAMEBUFFER ASSEMBLE
	if (glIsFramebuffer(fb.frameBuffer)) {
		glDeleteFramebuffers(1, &(fb.frameBuffer));
	}
	glGenFramebuffers(1, &(fb.frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, (fb.frameBuffer));
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fb.colorTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fb.depthTexture, 0);
	if (hasName){
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, fb.nameTexture, 0);
		GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0+1 };
		glDrawBuffers(2, DrawBuffers);
	}
	else{
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

MyMesh mesh;
frameBuffer meshfbo;
int meshProgram;
float transparencyExponent = 5;
GLuint meshVertexArray;
GLuint meshVertexBuffer;
GLuint meshNormalBuffer;
GLuint meshIndexBuffer;

bool drawInPlaceHist = false;
float inPlaceHistX, inPlaceHistY;
int inPlaceHistCtId, inPlaceHistArcId;
bool drawHoverContur = false;
int hoverCtID, hoverArcID;

int LoadMeshShaderData(){
	glGenVertexArrays(1, &meshVertexArray);
	glBindVertexArray(meshVertexArray);
	// vertices
	glGenBuffers(1, &meshVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh.GetNumVertex() * sizeof(MyVec3f), mesh.GetVertexData(), GL_STATIC_DRAW);
	int location = glGetAttribLocation(meshProgram, "position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normals
	glGenBuffers(1, &meshNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshNormalBuffer);
	glBufferData(GL_ARRAY_BUFFER, mesh.GetNumNormal() * sizeof(MyVec3f), mesh.GetNormalData(), GL_STATIC_DRAW);
	location = glGetAttribLocation(meshProgram, "normal");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_TRUE, 0, 0);
	//index
	glGenBuffers(1, &meshIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.GetNumTriangle() * sizeof(MyVec3i), mesh.GetTriangleData(), GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return 1;
}


void drawMeshGlass(const MyMesh& mesh){
	glPushMatrix();
	glTranslatef(0.5, 0.5, 0.5);
	//glScalef(1.f / faVol.get_numx(), 1.f / faVol.get_numy(), 1.f / faVol.get_numz());
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(meshProgram);
	int location = glGetUniformLocation(meshProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, modelViewMat);
	location = glGetUniformLocation(meshProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(location, 1, GL_FALSE, projMat);
	location = glGetUniformLocation(meshProgram, "cutCone");
	glUniform3f(location, 1, 1, 1);
	location = glGetUniformLocation(meshProgram, "transExp");
	glUniform1f(location, transparencyExponent);
	glBindVertexArray(meshVertexArray);
	glDrawElements(GL_TRIANGLES, mesh.GetNumTriangle() * 3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glDisable(GL_CULL_FACE);
	glPopMatrix();
	glDisable(GL_BLEND);
}

void RenderTexture(int texture, int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
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

void RenderZoomedTexture(int texture, int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(0, 1, 0, 1, 1, 10));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	MyGraphicsTool::EnableTexture2D();
	MyGraphicsTool::BindTexture2D(texture);
	float xCenter = magnifier_xPos / (float)windowWidth;
	float yCenter = magnifier_yPos / (float)windowHeight;
	float xRadius = magnifier_radius / (float)windowWidth;
	float yRadius = magnifier_radius / (float)windowHeight;

	MyGraphicsTool::BeginTriangleFan();
	MyGraphicsTool::TextureCoordinate(MyVec2f(xCenter, yCenter));
	MyGraphicsTool::Color(MyColor4f(1, 1, 1));
	MyGraphicsTool::Vertex(MyVec3f(xCenter, yCenter, -5));
	for (int i = 0; i <= 100; i++){
		float angle = i*3.1415926 * 2 / 100.f;
		MyGraphicsTool::TextureCoordinate(MyVec2f(xCenter + xRadius*cos(angle)/magnifier_scale, 
			yCenter + yRadius*sin(angle)/magnifier_scale));
		MyGraphicsTool::Vertex(MyVec3f(xCenter + xRadius*cos(angle), yCenter + yRadius*sin(angle), -5));

	}
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::Color(MyColor4f(0, 0, 0));
	MyGraphicsTool::SetLineWidth(2);
	MyGraphicsTool::BeginLineLoop();
	for (int i = 0; i <= 100; i++){
		float angle = i*3.1415926 * 2 / 100.f;
		MyGraphicsTool::Vertex(MyVec3f(xCenter + xRadius*cos(angle), yCenter + yRadius*sin(angle), -4));

	}
	MyGraphicsTool::EndPrimitive();
	MyGraphicsTool::SetLineWidth(1);
	MyGraphicsTool::BindTexture2D(0);
	MyGraphicsTool::DisableTexture2D();
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();
}

void CompileShader(){
	if (glIsProgram(fisheyeShader)){
		glDeleteProgram(fisheyeShader);
	}
	fisheyeShader = InitShader("fisheye.vert", "fisheye.frag", "fragColour");

	if (glIsProgram(meshProgram)){
		glDeleteProgram(meshProgram);
	}
	meshProgram = InitShader("mesh.vert", "mesh.frag", "fragColour");

}

void LoadFisheyeVertexData(){
	float fisheye_vertices[][2] = {
		{ -1, -1 }, { 1, -1 },
		{ 1, 1 }, { -1, 1 }
	};
	int fisheye_index[] = {
			0, 1, 2, 3
	};
	if (glIsVertexArray(fisheye_vertexArray)){
		glDeleteVertexArrays(1, &fisheye_vertexArray);
	}
	glGenVertexArrays(1, &fisheye_vertexArray);
	glBindVertexArray(fisheye_vertexArray);
	if (glIsBuffer(fisheye_vertexBuffer)){
		glDeleteBuffers(1, &fisheye_vertexBuffer);
	}
	glGenBuffers(1, &fisheye_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, fisheye_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), fisheye_vertices[0], GL_STATIC_DRAW);
	int location = glGetAttribLocation(fisheyeShader, "in_position");
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	if (glIsBuffer(fisheye_indexBuffer)){
		glDeleteBuffers(1, &fisheye_indexBuffer);
	}
	glGenBuffers(1, &fisheye_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fisheye_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(int), &fisheye_index, GL_STATIC_DRAW);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void RenderFisheyeTexture(int texture, int width, int height){
	glViewport(0, 0, width, height);
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(fisheyeShader);

	int location = glGetUniformLocation(fisheyeShader, "colorTexture");
	glUniform1i(location, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, texture);

	location = glGetUniformLocation(fisheyeShader, "center");
	glUniform2f(location, (magnifier_xPos-windowWidth/3)/((float)windowWidth*2/3), 
		magnifier_yPos/(float)windowHeight);

	location = glGetUniformLocation(fisheyeShader, "radius");
	glUniform2f(location, magnifier_radius / ((float)windowWidth * 2 / 3),
		magnifier_radius / (float)windowHeight);

	location = glGetUniformLocation(fisheyeShader, "focusRadius");
	glUniform1f(location, fishEye_focusRadiusRatio);

	location = glGetUniformLocation(fisheyeShader, "scale");
	glUniform1f(location, magnifier_scale);

	glBindVertexArray(fisheye_vertexArray);
	glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
	glUseProgram(0);
}

/**************************************** Data ********************/
MyContourTree *fields[] = { 0, 0 };
//float offsetX[] = { 0, 1.04 };
//float offsetY[] = { 0, 0 };
float offsetX[] = { 0, 0 };
float offsetY[] = { 0, 1.04 };
float lastOffsetX, lastOffsetY;
float isovalue = 0.8;
int pruningThreshold = 10;
float sigArcP = 0.01;
float sigArcVr = 0.5;
int sourceCt = -1;
int targetCt = -1;
int frontField = 0;
long viewArc[] = { -1, -1 };
float contourColor[][3] = {
	{ 1, 0, 0 }, 
	{ 0, 0, 1 }
};

enum TreeLayout{
	LAYOUT_VERTICAL = 1,
	LAYOUT_HORIZONTAL = 2
};
TreeLayout treelayout = LAYOUT_VERTICAL;
MyContourTree::MappingScale defaultScale = MyContourTree::MappingScale_Sci;
MyContourTree::MappingScale altScale = MyContourTree::MappingScale_Sci;
MyContourTree::HistogramSide histogramSide = MyContourTree::HistogramSide_Right;
MyContourTree::ArcSnapPosition snapPos = MyContourTree::ArcSnapPosition_Bottom_Y;

#define DRAG_FOCUS_NONE 0
#define DRAG_FOCUS_VOL_0 3
#define DRAG_FOCUS_TREE_0 4
#define DRAG_FOCUS_TREE_1 5
int dragFocus;


/**************************************** UI ********************/
GLUI* glui;
GLUI_Rollout* snap_panel;
GLUI_Rollout* altZoom_panel;
int   main_window;
MyTrackBall trackBall;
bool isRotating = false;
float zoomScale = 2;
float altViewScaleX = 2;
float altViewScaleY = 2;
int UI_mappingScale = 0;
int UI_mappingScale_alt = 0;
int UI_histogramSide = 2;
int UI_comparison = 0;
int UI_snapPos = 2;
float UI_contourTreeAlpha_front = 1;
float UI_contourTreeAlpha_back = 1;
int UI_drawTracks = 1;
int UI_drawMesh = 1;
int UI_drawROI = 0;
GLUI_Spinner* roiRatioSpinner;
float UI_roiRatio = 0.9;
int UI_diff = 1;

#define UI_ALTSCALE_ID 4

/**************************************** Action Functions********************/

void clamp(float& val, float minValue, float maxValue){
	val = max(minValue, val);
	val = min(maxValue, val);
}

void screen2ctspace(int x, int y, float& ctx, float& cty){
	if (treelayout == LAYOUT_HORIZONTAL){
		ctx = BOARDER_LEFT + BOARDER_X_RANGE * ((float)x / (windowWidth / 3));
		cty = BOARDER_BOTTOM + BOARDER_Y_RANGE * ((float)y / windowHeight);
	}
	else{
		ctx = BOARDER_LEFT + BOARDER_X_RANGE * ((float)x / (windowWidth / 3 * 2));
		cty = BOARDER_BOTTOM + BOARDER_Y_RANGE * ((float)y / (windowHeight / 2));
	}
}

int GetFieldIdx(int x, int y){
	if (fields[1] == 0) return 0;
	float ctx, cty;
	screen2ctspace(x, y, ctx, cty);
	float dist1 = pow(offsetX[0] + 0.5 - ctx, 2) + pow(offsetY[0] + 0.5 - cty, 2);
	float dist2 = pow(offsetX[1] + 0.5 - ctx, 2) + pow(offsetY[1] + 0.5 - cty, 2);
	if (dist1 < dist2) return 0;
	else return 1;
}

void selectArcSnap(int x, int y, int field){
	float ctx, cty;
	screen2ctspace(x, y, ctx, cty);
	long arcID = fields[field]->PickArc(ctx - offsetX[field], cty - offsetY[field]);
	if (arcID == -1) return;
	fields[field]->ClickSnapArc(arcID);
	cout << arcID << " snapping.\n";
}

void selectArcCompare(int x, int y, int field){
	float ctx, cty;
	screen2ctspace(x, y, ctx, cty);
	long arcID = fields[field]->PickArc(ctx - offsetX[field], cty - offsetY[field]);
	if (arcID == -1) return;
	fields[field]->ClickComparedArc(arcID);
}

void selectArcView(int x, int y, int field){
	float ctx, cty;
	screen2ctspace(x, y, ctx, cty);
	long arcID = fields[field]->PickArc(ctx - offsetX[field], cty - offsetY[field]);
	if (arcID == -1) {
		viewArc[field] = -1;
		return;
	}
	viewArc[field] = arcID;
}

void selectArc(int x, int y, int field){
	float ctx, cty;
	screen2ctspace(x, y, ctx, cty);
	long arcID = fields[field]->PickArc(ctx - offsetX[field], cty - offsetY[field]);
	if (arcID == -1){
		if (ctx - offsetX[field] < 0){
			float value = cty - offsetY[field];
			fields[field]->SetIsosurface(value);
			fields[field]->UpdateContours();
		}
	}
	else{
		float newHt = cty - offsetY[field];
		fields[field]->SelectComponent(arcID, newHt);
		fields[field]->UpdateContours();
	}

}

void aggregateArc(int x, int y, int field){
	float ctx, cty;
	screen2ctspace(x, y, ctx, cty);
	long arc = fields[field]->PickArc(ctx - offsetX[field], cty - offsetY[field]);
	if (arc != -1){
		if (!fields[field]->IsArcAggregated(arc)){
			fields[field]->AggregateArcFromAbove(arc);
		}
		else{
			fields[field]->RestoreAggregatedArc(arc);
		}
		fields[field]->ComputeArcNames();
		fields[field]->SetNodeXPositionsExt(defaultScale);
	}
	else{
		if (ctx - offsetX[field] < 0){
			float value = cty - offsetY[field];
			fields[field]->SetIsosurface(value);
			fields[field]->UpdateContours();
		}
	}
}

void updateScaleWidth(){
	float scaleWidth = 1;
	float scaleWidth1 = 0;
	float scaleWidth2 = 0;
	if (defaultScale == MyContourTree::MappingScale_Sci){
		if (fields[0] != 0){
			scaleWidth1 = fields[0]->GetScientificWidthScale();
		}
		if (scaleWidth1 != 0) scaleWidth = scaleWidth1;
		if (fields[1] != 0){
			scaleWidth2 = fields[1]->GetScientificWidthScale();
		}
		if (scaleWidth2 != 0) scaleWidth = min(scaleWidth, scaleWidth2);
		if (scaleWidth != 0){
			fields[0]->SetScientificWidthScale(scaleWidth);
			if (fields[1] != 0){
				fields[1]->SetScientificWidthScale(scaleWidth);
			}
		}
	}
	else if (defaultScale == MyContourTree::MappingScale_Linear){
		if (fields[0] != 0){
			scaleWidth1 = fields[0]->GetLinearScaleWidth();
		}
		if (scaleWidth1 != 0) scaleWidth = scaleWidth1;
		if (fields[1] != 0){
			scaleWidth2 = fields[1]->GetLinearScaleWidth();
		}
		if (scaleWidth2 != 0) scaleWidth = min(scaleWidth, scaleWidth2);
		if (scaleWidth != 0){
			fields[0]->SetLinearScaleWidth(scaleWidth);
			if (fields[1] != 0){
				fields[1]->SetLinearScaleWidth(scaleWidth);
			}
		}
	}
	else{
		if (fields[0] != 0){
			scaleWidth1 = fields[0]->GetLogScaleWidth();
		}
		if (scaleWidth1 != 0) scaleWidth = scaleWidth1;
		if (fields[1] != 0){
			scaleWidth2 = fields[1]->GetLogScaleWidth();
		}
		if (scaleWidth2 != 0) scaleWidth = min(scaleWidth, scaleWidth2);
		if (scaleWidth != 0){
			fields[0]->SetLogScaleWidth(scaleWidth);
			if (fields[1] != 0){
				fields[1]->SetLogScaleWidth(scaleWidth);
			}
		}
	}
}

void updateAltScale(){
	float scale0 = fields[0]->SuggestAltMappingWidthScaleModifier()
		* fields[0]->GetAltMappingWidthScale();
	float scale1 = scale0;
	if (fields[1]){
		scale1 = fields[1]->SuggestAltMappingWidthScaleModifier()
			* fields[1]->GetAltMappingWidthScale();
	}
	if (scale0 > 0 && scale1 > 0){
		float scale = min(scale0, scale1);
		fields[0]->SetAltMappingScale(scale);
		if (fields[1]) fields[1]->SetAltMappingScale(scale);
	}
	else if (scale0 > 0 || scale1 > 0){
		float scale = max(scale0, scale1);
		fields[0]->SetAltMappingScale(scale);
		if (fields[1]) fields[1]->SetAltMappingScale(scale);
	}
}

void updateLayout(int control);

void handleRightClick(int x, int y, int field, int otherField){
	sourceCt = -1;
	targetCt = -1;
	switch (UI_comparison)
	{
		// in place
	case 0:
		selectArcCompare(x, y, field);
		if (fields[otherField]) fields[otherField]->CompareArcs(fields[field]);
		updateLayout(-1);
		break;
		// Snap
	case 1:
		fields[field]->ClearSnapArcs();
		if (fields[otherField]) fields[otherField]->ClearSnapArcs();
		selectArcSnap(x, y, field);
		if (fields[otherField]) fields[otherField]->SyncSnapArcsTo(fields[field]);
		sourceCt = field;
		targetCt = otherField;
		frontField = field;
		targetCt = otherField;
		screen2ctspace(x, y, lastOffsetX, lastOffsetY);
		break;
		// Snap
	case 2:
		selectArcView(x, y, field);
		break;
	default:
		break;
	}
}

void handleSnapRelease(int field, int otherField){
	if (fields[field] && fields[otherField]){
		float ox, oy;
		if (fields[field]->ShouldWeSnap(fields[otherField], ox, oy)){
			if (field == 0){
				offsetX[0] = offsetX[1] + ox;
				offsetY[0] = offsetY[1] + oy;
			}
			else{
				offsetX[1] = offsetX[0] + ox;
				offsetY[1] = offsetY[0] + oy;
			}
			cout << "Snapped!.\n";
		}
	}
}

void drawContourTrees(int x, int y, int width, int height){

	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); {
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glLoadIdentity();
			//glOrtho(-0.02, 1.02, -0.02, 1.02, -1.0, 1.0);
			if (treelayout == LAYOUT_HORIZONTAL){
				glOrtho(BOARDER_LEFT, BOARDER_RIGHT + BOARDER_X_RANGE, BOARDER_BOTTOM, BOARDER_TOP, -1.0, 1.0);
			}
			else{
				glOrtho(BOARDER_LEFT, BOARDER_RIGHT, BOARDER_BOTTOM, BOARDER_TOP + BOARDER_Y_RANGE, -1.0, 1.0);
			}
			glPushMatrix();
			glTranslatef(offsetX[0], offsetY[0], 0);
			fields[0]->DrawPlanarContourTree();
			glPopMatrix();
			if (showDiffTree && fields[1] != 0){
				glPushMatrix();
				glTranslatef(offsetX[1], offsetY[1], 0);
				fields[1]->RenderSigDiffTree();
				glPopMatrix();
			}
			else if (fields[1] != 0){
				glPushMatrix();
				glTranslatef(offsetX[1], offsetY[1], 0);
				fields[1]->DrawPlanarContourTree();
				glPopMatrix();
			}
			// draw legend
			fields[0]->DrawLegendSimple(MyVec2f(0.95, 1.9), MyVec2f(1, 2.1));

			// draw arc view
			if (UI_comparison == 2){
				glLineWidth(3);
				glColor4f(0, 0, 0, 1);
				glBegin(GL_LINE_LOOP);
				glVertex2f(0.8, 0.75);
				glVertex2f(1, 0.75);
				glVertex2f(1, 1.25);
				glVertex2f(0.8, 1.25);
				glEnd();
				if (viewArc[0] >= 0){
					fields[0]->DrawArcHistogramAt(viewArc[0], 0.85, 0.75, altViewScaleX, altViewScaleY);
				}
				if (viewArc[1] >= 0){
					fields[1]->DrawArcHistogramAt(viewArc[1], 0.95, 0.75, altViewScaleX, altViewScaleY);
				}
			}

			glDisable(GL_BLEND);
		}glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	}glPopMatrix();
}

void drawContours(int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	glPushMatrix(); {
		MyGraphicsTool::LoadTrackBall(&trackBall);
		glScalef(-1, -1, -1);
		glTranslatef(-fields[0]->GetVolume().XDim() / 2,
			-fields[0]->GetVolume().YDim() / 2,
			-fields[0]->GetVolume().ZDim() / 2);
		//fields[0]->FlexibleContours();
		fields[0]->RenderContours();
		//fields[0]->ShowSelectedVoxes();
		if (UI_drawROI){
			fields[0]->RenderAllUpperLeaveContours(UI_roiRatio);
		}
		if (fields[1] != 0) {
			float color2[] = { 0, 0, 0.6, 1 };
			//glColor4fv(color2);
			//fields[1]->FlexibleContours(true, false);
			//fields[1]->FlexibleContours();
			fields[1]->RenderContours();
		}
		if (drawHoverContur){
			fields[hoverCtID]->DrawArcContour(hoverArcID);
		}
	}glPopMatrix();

	// draw color patch
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); {
		glLoadIdentity();
		glOrtho(BOARDER_LEFT, BOARDER_RIGHT, BOARDER_BOTTOM, BOARDER_TOP, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); {
			glLoadIdentity();
			/********* Draw Color Patch ***********/
			//glDepthFunc(GL_ALWAYS);
			float leftBottomQuad1[] = { 0.7, 0.02 };
			float leftBottomQuad2[] = { 0.7, 0.05 };
			float sizeQuad = 0.02;
			glBegin(GL_QUADS);
			//glColor4f(1, 0, 0, 1);
			glColor3fv(contourColor[0]);
			glVertex2f(leftBottomQuad1[0], leftBottomQuad1[1]);
			glVertex2f(leftBottomQuad1[0] + sizeQuad, leftBottomQuad1[1]);
			glVertex2f(leftBottomQuad1[0] + sizeQuad, leftBottomQuad1[1] + sizeQuad);
			glVertex2f(leftBottomQuad1[0], leftBottomQuad1[1] + sizeQuad);
			glEnd();
			glColor4f(0, 0, 0, 1);
			// give them some outline
			glBegin(GL_LINE_LOOP);
			glVertex2f(leftBottomQuad1[0], leftBottomQuad1[1]);
			glVertex2f(leftBottomQuad1[0] + sizeQuad, leftBottomQuad1[1]);
			glVertex2f(leftBottomQuad1[0] + sizeQuad, leftBottomQuad1[1] + sizeQuad);
			glVertex2f(leftBottomQuad1[0], leftBottomQuad1[1] + sizeQuad);
			glEnd();
			glRasterPos2f(leftBottomQuad1[0] + sizeQuad + 0.005, leftBottomQuad1[1]);
			glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char *)fields[0]->GetName().c_str());

			if (fields[1]){
				glBegin(GL_QUADS);
				//glColor4f(0, 0, 1, 1);
				glColor3fv(contourColor[1]);
				glVertex2f(leftBottomQuad2[0], leftBottomQuad2[1]);
				glVertex2f(leftBottomQuad2[0] + sizeQuad, leftBottomQuad2[1]);
				glVertex2f(leftBottomQuad2[0] + sizeQuad, leftBottomQuad2[1] + sizeQuad);
				glVertex2f(leftBottomQuad2[0], leftBottomQuad2[1] + sizeQuad);
				glEnd();
				glColor4f(0, 0, 0, 1);
				glBegin(GL_LINE_LOOP);
				glVertex2f(leftBottomQuad2[0], leftBottomQuad2[1]);
				glVertex2f(leftBottomQuad2[0] + sizeQuad, leftBottomQuad2[1]);
				glVertex2f(leftBottomQuad2[0] + sizeQuad, leftBottomQuad2[1] + sizeQuad);
				glVertex2f(leftBottomQuad2[0], leftBottomQuad2[1] + sizeQuad);
				glEnd();
				glRasterPos2f(leftBottomQuad2[0] + sizeQuad + 0.005, leftBottomQuad2[1]);
				glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char *)fields[1]->GetName().c_str());
			}
			//glDepthFunc(GL_LESS);
		}
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	}
	glPopMatrix();
}

void drawTracks(int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	if (showTracks){
		glPushMatrix(); {
			MyGraphicsTool::LoadTrackBall(&trackBall);
			glTranslatef(-fields[0]->GetVolume().XDim() / 2,
				-fields[0]->GetVolume().YDim() / 2,
				-fields[0]->GetVolume().ZDim() / 2);
			float color3[] = { 0.5, 0.5, 0.6, 1 };
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color3);
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			track.Show();
			glDisable(GL_LIGHTING);
			glDisable(GL_LIGHT0);
		}glPopMatrix();
	}
}

void drawMesh(int x, int y, int width, int height){
	MyGraphicsTool::SetViewport(MyVec4i(x, y, width, height));
	if (showMesh){
		glPushMatrix(); {
			MyGraphicsTool::LoadTrackBall(&trackBall);
			drawMeshGlass(mesh);
		}glPopMatrix();
	}
}

void draw(int width, int height){
	glClearColor(1, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawContourTrees(width / 3, 0, width / 3 * 2, height);
	drawContours(0, 0, width / 3, height);




	/*
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); {
		glLoadIdentity();
		if (treelayout == LAYOUT_HORIZONTAL){
			glOrtho(BOARDER_LEFT, BOARDER_RIGHT, BOARDER_BOTTOM, BOARDER_TOP, -1.0, 1.0);
		}
		else{
			glOrtho(BOARDER_LEFT, BOARDER_LEFT+BOARDER_X_RANGE/2, BOARDER_BOTTOM, BOARDER_TOP + BOARDER_Y_RANGE, -1.0, 1.0);
		}
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix(); {
			glLoadIdentity();
			if (treelayout == LAYOUT_HORIZONTAL){
				//fields[0]->DrawLegend(MyVec2f(0.9, 0), MyVec2f(1, 1));
				fields[0]->DrawLegendSimple(MyVec2f(0.9, 0), MyVec2f(1, 1));
			}
			else {
				fields[0]->DrawLegendSimple(MyVec2f(0.45, 0.8), MyVec2f(0.5, 1.2));
			}
		}glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	}glPopMatrix();
	*/
}
/**************************************** UI Callback********************/

void updateAlpha(int control){
	if (fields[frontField]) fields[frontField]->SetContourTreeAlpha(UI_contourTreeAlpha_front);
	if (fields[1 - frontField]) fields[1 - frontField]->SetContourTreeAlpha(UI_contourTreeAlpha_back);
}

void updateComponent(int control){
	showTracks = (UI_drawTracks == 1);
	showMesh = (UI_drawMesh == 1);
}

void updateDiff(int control){
	if (fields[1]){
		switch (UI_diff)
		{
		case 0:
			fields[1]->SetArcCombineMode(MyContourTree::ArcCombineMode_Intersection);
			break;
		case 2:
			fields[1]->SetArcCombineMode(MyContourTree::ArcCombineMode_Complement);
			break;
		case 1:
		default:
			fields[1]->SetArcCombineMode(MyContourTree::ArcCombineMode_Union);
			break;
		}
		fields[1]->SyncSigArcsTo(fields[0]);
	}
}

void updateROI(int control){
	if (!UI_drawROI){
		roiRatioSpinner->disable();
	}
	else{
		roiRatioSpinner->enable();
	}
}

void updateSnapPos(int control){
	switch (UI_snapPos)
	{
	case 0:
		snapPos = MyContourTree::ArcSnapPosition_Top_Y;
		break;
	case 1:
		snapPos = MyContourTree::ArcSnapPosition_Center_Y;
		break;
	case 2:
	default:
		snapPos = MyContourTree::ArcSnapPosition_Bottom_Y;
		break;
	}
	fields[0]->SetSnapPosition(snapPos);
	if (fields[1]){
		fields[1]->SetSnapPosition(snapPos);
	}
}

void updateComparison(int control){
	switch (UI_comparison)
	{
	case 0:
		// in place
		fields[0]->ClearSnapArcs();
		if (fields[1]) fields[1]->ClearSnapArcs();
		snap_panel->disable();
		altZoom_panel->disable();
		break;
	case 1:
		snap_panel->enable();
		altZoom_panel->disable();
		break;
	case 2:
	default:
		fields[0]->ClearSnapArcs();
		if (fields[1]) fields[1]->ClearSnapArcs();
		snap_panel->disable();
		altZoom_panel->enable();
		break;
	}
}

void updateLayout(int control){

	defaultScale = MyContourTree::MappingScale_Linear;
	if (UI_mappingScale == 1) defaultScale = MyContourTree::MappingScale_Sci;
	else  if (UI_mappingScale == 2) defaultScale = MyContourTree::MappingScale_Log;

	altScale = MyContourTree::MappingScale_Linear;
	if (UI_mappingScale_alt == 1) altScale = MyContourTree::MappingScale_Sci;
	else  if (UI_mappingScale_alt == 2) altScale = MyContourTree::MappingScale_Log;

	histogramSide = MyContourTree::HistogramSide_Right;
	if (UI_histogramSide == 0) histogramSide = MyContourTree::HistogramSide_Left;
	else  if (UI_histogramSide == 1) histogramSide = MyContourTree::HistogramSide_Sym;

	//return;
	if (fields[0]){
		fields[0]->SetDefaultMappingScale(defaultScale);
		fields[0]->SetZoomLevel(zoomScale);
		fields[0]->SetAltMappingScale(altScale);
		fields[0]->SetHistogramSide(histogramSide);
	}
	if (fields[1]){
		fields[1]->SetDefaultMappingScale(defaultScale);
		fields[1]->SetZoomLevel(zoomScale);
		fields[1]->SetAltMappingScale(altScale);
		fields[1]->SetHistogramSide(histogramSide);
	}

	// set alt scale
	//if (control == UI_ALTSCALE_ID){
		updateAltScale();
	//}

	fields[0]->SetNodeXPositionsExt(defaultScale);
	if (fields[1]){
		fields[1]->SetNodeXPositionsExt(defaultScale);
		updateScaleWidth();
	}
}

void updatePruning(int control){
	if (control == 2){
		//return;
		if (fields[0]){
			fields[0]->ClearComparedArcs();
			fields[0]->SetPruningThreshold(pruningThreshold);
			fields[0]->PruneNoneROIs();
			fields[0]->ComputeArcNames();
			fields[0]->SetIsosurface(isovalue);
			fields[0]->SetNodeXPositionsExt(defaultScale);
			if (showDiffTree) fields[0]->UpdateSigArcList();
		}
		if (fields[1]){
			fields[1]->ClearComparedArcs();
			fields[1]->SetPruningThreshold(pruningThreshold);
			fields[1]->PruneNoneROIs();
			fields[1]->ComputeArcNames();
			fields[1]->SetIsosurface(isovalue);
			fields[1]->SetNodeXPositionsExt(defaultScale);

			if (showDiffTree) fields[1]->SyncSigArcsTo(fields[0]);

			updateScaleWidth();
		}
	}
}


void updateSigArcs(int control){
	if (control == 2){
		//return;
		if (fields[0]){
			fields[0]->SetSigArcThreshold_P(sigArcP);
			fields[0]->SetSigArcThreshold_VolRatio(sigArcVr);
			if (showDiffTree) fields[0]->UpdateSigArcList();
		}
		if (fields[1]){
			fields[1]->SetSigArcThreshold_P(sigArcP);
			fields[1]->SetSigArcThreshold_VolRatio(sigArcVr);
			if (showDiffTree) fields[1]->SyncSigArcsTo(fields[0]);
		}
	}
}


/**************************************** GLUT Callback ********************/

void myGlutKeyboard(unsigned char Key, int x, int y)
{
	switch (Key)
	{
	case 27:
	case 'q':
		exit(0);
		break;
	case 'r':
	case 'R':
		if(showTracks) track.LoadShader();
		fields[0]->ReCompileShaders();
		fields[0]->CompileContourShader();
		CompileShader();
		LoadFisheyeVertexData();
		break;
	case 'i':
		isovalue = min(1.0, isovalue + 0.1);
		fields[0]->SetIsosurface(isovalue);
	case 'I':
		isovalue = max(0.0, isovalue - 0.1);
		fields[0]->SetIsosurface(isovalue);
		fields[0]->MarkSelectedVoxes();
		track.FilterByVolumeMask(fields[0]->GetMaskVolume());
		break;
	};

	glutPostRedisplay();
}

void myGlutMenu(int value)
{
	myGlutKeyboard(value, 0, 0);
}

void myGlutMouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN){
		if (x < windowWidth / 3){
			trackBall.StartMotion(x, y);
			isRotating = true;
			dragFocus = DRAG_FOCUS_VOL_0;

			// test read pixels
			glBindFramebuffer(GL_FRAMEBUFFER, contourBuffer.frameBuffer);
			glReadBuffer(GL_COLOR_ATTACHMENT0 + 1);
			glReadPixels(0, 0, contourBuffer.width, contourBuffer.height,
				GL_RGBA_INTEGER, GL_INT, nameMap);
			//glReadPixels(0, 0, contourBuffer.width, contourBuffer.height,
			//	GL_RGBA, GL_FLOAT, pixels);
			//for (int i = 0; i < contourBuffer.width*contourBuffer.height * 4; i++) if(pixels[i]>0) cout << pixels[i];
			cout << "Clicked Color: " 
				<< nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4] << ", "
				<< nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 1] << ", "
				<< nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 2] << ", "
				<< nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 3] << endl;
			long arcID = nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 2];
			long ctID = nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 3];
			if (arcID < 999999 && ctID<=1 && ctID >=0){
				if (fields[ctID]){
					fields[ctID]->ClickComparedArc(arcID);
					if (fields[1 - ctID]) fields[1 - ctID]->CompareArcs(fields[ctID]);
				}
				updateLayout(-1);
			}
		}
		else {
			int fieldIdx = GetFieldIdx(x - windowWidth / 3, windowHeight - y);
			if (button == GLUT_LEFT_BUTTON){
				if (glutGetModifiers() == GLUT_ACTIVE_ALT){
					aggregateArc(x - windowWidth / 3, windowHeight - y, fieldIdx);
				}
				else{
					selectArc(x - windowWidth / 3, windowHeight - y, fieldIdx);
					fields[0]->MarkSelectedVoxes();
					track.FilterByVolumeMask(fields[0]->GetMaskVolume());
				}
				dragFocus = DRAG_FOCUS_TREE_0 + fieldIdx;
			}
			else if (button == GLUT_RIGHT_BUTTON){
				handleRightClick(x - windowWidth / 3, windowHeight - y, fieldIdx, 1-fieldIdx);
				dragFocus = DRAG_FOCUS_TREE_0 + fieldIdx;
			}
			else if (button == GLUT_MIDDLE_BUTTON){
				isMagnifying = true;
				magnifier_xPos = x;
				magnifier_yPos = windowHeight - y;
				dragFocus = DRAG_FOCUS_NONE;
			}
		}
	}
	else if (state == GLUT_UP){
		if (isRotating){
			trackBall.EndMotion(std::min(x, windowWidth / 3), y);
			isRotating = false;
		}
		dragFocus = DRAG_FOCUS_NONE;
		isMagnifying = false;
		if (sourceCt >= 0 && targetCt >= 0){
			handleSnapRelease(sourceCt, targetCt);
			sourceCt = targetCt = -1;
		}
	}
	glutPostRedisplay();
}

void myGlutMotion(int x, int y)
{
	if (x < windowWidth / 3){
		if (dragFocus == DRAG_FOCUS_VOL_0){
			trackBall.RotateMotion(x, y);
		}
		glutPostRedisplay();
	}
	else if (dragFocus == DRAG_FOCUS_TREE_0){
		if (sourceCt == 0){
			// is Snapping
			float offx, offy;
			screen2ctspace(x - windowWidth / 3, windowHeight - y, offx, offy);
			offsetX[0] += offx - lastOffsetX;
			offsetY[0] += offy - lastOffsetY;
			if (treelayout == LAYOUT_HORIZONTAL){
				clamp(offsetX[0], BOARDER_LEFT, BOARDER_RIGHT+BOARDER_X);
				clamp(offsetY[0], BOARDER_BOTTOM, BOARDER_TOP);
			}
			else{
				clamp(offsetX[0], BOARDER_LEFT, BOARDER_RIGHT);
				clamp(offsetY[0], BOARDER_BOTTOM, BOARDER_TOP+BOARDER_Y);
			}
			lastOffsetX = offx;
			lastOffsetY = offy;
			if (fields[1]){
				fields[0]->UpdateSnapArcStatus(fields[1], offsetX[0] - offsetX[1], offsetY[0] - offsetY[1]);
			}
		}
		else{
			// dragging arc isovalue
			if (fields[0]->selectionRoot != -1){
				float ctx, cty;
				screen2ctspace(x, windowHeight - y, ctx, cty);
				float newHt = cty - offsetY[0];
				fields[0]->UpdateSelection(newHt);
				fields[0]->UpdateContours();
				fields[0]->MarkSelectedVoxes();
				track.FilterByVolumeMask(fields[0]->GetMaskVolume());
			}
		}
	}
	else if (dragFocus == DRAG_FOCUS_TREE_1){
		if (fields[1] != 0){
			if (sourceCt == 1){
				// is Snapping
				float offx, offy;
				screen2ctspace(x - windowWidth / 3, windowHeight - y, offx, offy);
				offsetX[1] += offx - lastOffsetX;
				offsetY[1] += offy - lastOffsetY;
				if (treelayout == LAYOUT_HORIZONTAL){
					clamp(offsetX[1], BOARDER_LEFT, BOARDER_RIGHT + BOARDER_X);
					clamp(offsetY[1], BOARDER_BOTTOM, BOARDER_TOP);
				}
				else{
					clamp(offsetX[1], BOARDER_LEFT, BOARDER_RIGHT);
					clamp(offsetY[1], BOARDER_BOTTOM, BOARDER_TOP + BOARDER_Y);
				}
				lastOffsetX = offx;
				lastOffsetY = offy;
				fields[1]->UpdateSnapArcStatus(fields[0], offsetX[1] - offsetX[0], offsetY[1] - offsetY[0]);
			}
			else{
				// dragging arc isovalue
				if (fields[1]->selectionRoot != -1){
					float ctx, cty;
					screen2ctspace(x, windowHeight - y, ctx, cty);
					float newHt = cty - offsetY[1];
					fields[1]->UpdateSelection(newHt);
					fields[1]->UpdateContours();
					//fields[1]->MarkSelectedVoxes();
				}
			}
		}
	}
	if (isMagnifying){
		magnifier_xPos = x;
		magnifier_yPos = windowHeight - y;
	}
	glutPostRedisplay();
}

void myGlutPassiveMotion(int x, int y){
	if (x < windowWidth / 3){
		if (drawHoverContur){
			drawHoverContur = false;
			glutPostRedisplay();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, contourBuffer.frameBuffer);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + 1);
		glReadPixels(0, 0, contourBuffer.width, contourBuffer.height,
			GL_RGBA_INTEGER, GL_INT, nameMap);
		long arcID = nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 2];
		long ctID = nameMap[(windowHeight - y - 1)*contourBuffer.width * DSR_FACTOR_Y * 4 + x*DSR_FACTOR_X * 4 + 3];
		if (arcID < 999999 && ctID <= 1 && ctID >= 0){
			drawInPlaceHist = true;
			//cout << "CtID: " << ctID << ", ArcID: " << arcID << endl;
			inPlaceHistX = x;
			inPlaceHistY = windowHeight - y;
			inPlaceHistCtId = ctID;
			inPlaceHistArcId = arcID;
			glutPostRedisplay();
		}
		else if (drawInPlaceHist){
			drawInPlaceHist = false;
			glutPostRedisplay();
		}
	}
	else {
		if (drawInPlaceHist){
			drawInPlaceHist = false;
			glutPostRedisplay();
		}

		hoverCtID = -1;
		int fieldIdx = GetFieldIdx(x - windowWidth / 3, windowHeight - y);
		float ctx, cty;
		screen2ctspace(x - windowWidth / 3, windowHeight - y, ctx, cty);
		long arcID = fields[fieldIdx]->PickArc(ctx - offsetX[fieldIdx], cty - offsetY[fieldIdx], false);
		if (arcID >= 0){
			drawHoverContur = true;
			hoverCtID = fieldIdx;
			hoverArcID = arcID;
			glutPostRedisplay();
		}
		else if (drawHoverContur){
			drawHoverContur = false;
			glutPostRedisplay();
		}
		//cout << "Hover ct: " << hoverCtID << ", Arc: " << hoverArcID << endl;
	}
}

void myGlutDisplay(void)
{
	glClearColor(1, 1, 1, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, drawBuffer.frameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawContourTrees(0, 0, drawBuffer.width, drawBuffer.height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (isMagnifying){
		if (useFisyeye){
			glBindFramebuffer(GL_FRAMEBUFFER, fishEyeFBO.frameBuffer);
			RenderFisheyeTexture(drawBuffer.colorTexture, 
				fishEyeFBO.width, fishEyeFBO.height);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			RenderTexture(fishEyeFBO.colorTexture, windowWidth / 3, 0, windowWidth * 2 / 3, windowHeight);
			//RenderFisheyeTexture(drawBuffer.colorTexture,
			//	windowWidth, windowHeight);
		}
		else{
			RenderTexture(drawBuffer.colorTexture, windowWidth / 3, 0, windowWidth * 2 / 3, windowHeight);
			RenderZoomedTexture(drawBuffer.colorTexture, windowWidth / 3, 0, windowWidth * 2 / 3, windowHeight);
		}
	}
	else{
		RenderTexture(drawBuffer.colorTexture, windowWidth / 3, 0, windowWidth * 2 / 3, windowHeight);
	}
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, contourBuffer.frameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLint initNames[] = { -1, -1, -1, -1 };
	glClearBufferiv(GL_COLOR, 1, initNames);

	drawContours(0, 0, contourBuffer.width, contourBuffer.height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//RenderTexture(contourBuffer.colorTexture, 0, 0, windowWidth / 3, windowHeight);
	drawContours(0, 0, windowWidth / 3, windowHeight);

	drawTracks(0, 0, windowWidth / 3, windowHeight);
	drawMesh(0, 0, windowWidth / 3, windowHeight);
	glDisable(GL_DEPTH_TEST);

	// draw inplace histogram
	glViewport(0, 0, windowWidth / 3, windowHeight);
	glDepthFunc(GL_ALWAYS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	if (drawInPlaceHist){
		fields[inPlaceHistCtId]->DrawArcHistogramAt(
			inPlaceHistArcId, inPlaceHistX / (float)(windowWidth / 3),
			inPlaceHistY / (float)windowHeight, 2, 0.5);
	}
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glDepthFunc(GL_LESS);

	glutSwapBuffers();
}

void myGlutReshape(int x, int y)
{
	int tx, ty, tw, th;
	GLUI_Master.get_viewport_area(&tx, &ty, &tw, &th);
	glViewport(tx, ty, tw, th);
	windowWidth = tw;
	windowHeight = th;
	trackBall.Reshape(tw / 3, th);
	//MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-w / 2 / 80.f, w / 80.f, -h / 80.f, h / 80.f, 0.1f, 2000.f);
	MyMatrixf projectionMatrix = MyMatrixf::OrthographicMatrix(-100, 100, -100, 100, -200, 200.f);
	MyGraphicsTool::LoadProjectionMatrix(&projectionMatrix);
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	//fields[0]->SetupVolomeRenderingBuffers(windowWidth / 3, windowHeight);
	//fields[0]->MarkSelectedVoxes();
	//if (fields[1] != 0){
	//	fields[1]->SetupVolomeRenderingBuffers(windowWidth / 3, windowHeight);
	//	fields[1]->MarkSelectedVoxes();
	//}
	drawBuffer.width = tw*DSR_FACTOR_X * 2.0 / 3;
	drawBuffer.height = th*DSR_FACTOR_Y;
	fishEyeFBO.width = tw*DSR_FACTOR_X * 2.0 / 3;
	fishEyeFBO.height = th*DSR_FACTOR_Y;
	contourBuffer.width = tw*DSR_FACTOR_X / 3;
	contourBuffer.height = th*DSR_FACTOR_Y;
	meshfbo.width = tw / 3 * DSR_FACTOR_X;
	meshfbo.height = th * DSR_FACTOR_Y;
	BuildGLBuffers(drawBuffer);
	BuildGLBuffers(fishEyeFBO);
	BuildGLBuffers(contourBuffer, true);
	if (nameMap) delete[] nameMap;
	nameMap = new int[contourBuffer.width*contourBuffer.height * 4];
	if(showMesh) BuildGLBuffers(meshfbo);

	//fields[0]->SetupVolomeRenderingBuffers(tw / 3 * DSR_FACTOR_X, th * DSR_FACTOR_Y);

	glutPostRedisplay();
}


/**************************************** main() ********************/

int main(int argc, char* argv[])
{
	/****************************************/
	/*   Initialize GLUT and create window  */
	/****************************************/

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(windowWidth, windowHeight);
	main_window = glutCreateWindow("Contour Tree Based Cohort Brain Comparison");
	glewInit();
	glutDisplayFunc(myGlutDisplay);
	GLUI_Master.set_glutDisplayFunc(myGlutDisplay);
	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	GLUI_Master.set_glutKeyboardFunc(myGlutKeyboard);
	GLUI_Master.set_glutSpecialFunc(NULL);
	GLUI_Master.set_glutMouseFunc(myGlutMouse);
	GLUI_Master.set_glutReshapeFunc(myGlutReshape);
	glutMotionFunc(myGlutMotion);
	glutPassiveMotionFunc(myGlutPassiveMotion);

	/****************************************/
	/*         Here's the GLSL code         */
	/****************************************/

	CompileShader();
	LoadFisheyeVertexData();


	/****************************************/
	/*               Loading Data           */
	/****************************************/

	if (showTracks){
		track.Read("C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti.trk");
		//track.Read("ACR.trk");
		track.SetShape(MyTracks::TRACK_SHAPE_LINE);
		//track.SetShape(MyTracks::TRACK_SHAPE_TUBE);
		track.ComputeGeometry();
		track.LoadShader();
		track.LoadGeometry();
	}

	int nArg = argc - 1;
	char* argv1[] = { argv[0], argv[1] };
	fields[0] = new MyContourTree(nArg, argv1);
	fields[0]->LoadLabelVolume("JHU-WhiteMatter-labels-1mm.nii");
	fields[0]->LoadLabelTable("GOBS_look_up_table.txt");
	fields[0]->SetName("Control");
	fields[0]->SetPruningThreshold(pruningThreshold);
	fields[0]->PruneNoneROIs();
	fields[0]->ComputeArcNames();
	fields[0]->SetIsosurface(isovalue);
	fields[0]->SetNodeXPositionsExt(defaultScale);
	fields[0]->SetContourColour(contourColor[0]);
	fields[0]->SetIndex(0);
	fields[0]->CompileContourShader();
	fields[0]->BuildContourGeomeryBuffer();
	fields[0]->UpdateContours();

	if (showTracks){
		fields[0]->MarkSelectedVoxes();
		track.FilterByVolumeMask(fields[0]->GetMaskVolume());
	}

	if (showDiffTree){
		fields[0]->LoadVoxSignificance("h16e0d1c26_stats_scz122ctr127_tfce_p_tstat4.nii.gz");
		fields[0]->UpdateSigArcList();
	}

	if (showSecTree){
		if (argc > 2){
			char* argv2[] = { argv[0], argv[2] };
			fields[1] = new MyContourTree(nArg, argv2);
			fields[1]->LoadLabelVolume("JHU-WhiteMatter-labels-1mm.nii");
			fields[1]->LoadLabelTable("GOBS_look_up_table.txt");
			fields[1]->SetName("Schizophrenia");
			fields[1]->SetPruningThreshold(pruningThreshold);
			fields[1]->PruneNoneROIs();
			fields[1]->ComputeArcNames();
			fields[1]->SetIsosurface(isovalue);
			fields[1]->SetNodeXPositionsExt(defaultScale);
			fields[1]->SetContourColour(contourColor[1]);
			fields[1]->SetIndex(1);
			fields[1]->CompileContourShader();
			fields[1]->BuildContourGeomeryBuffer();
			fields[1]->UpdateContours();

			if (showDiffTree) fields[1]->SyncSigArcsTo(fields[0]);

			updateScaleWidth();


			fields[1]->SetNodeXPositionsExt(defaultScale);
			MyContourTree::RenameLeaveArcsBySimilarity(fields[0], fields[1]);
		}
	}

	if (showMesh){
		cout << "Loading mesh ..." << endl;
		int read = mesh.Read("lh.pial.obj");
		MyMesh mesh2;
		read = mesh2.Read("rh.pial.obj");
		mesh.Merge(mesh2);
		cout << "Calculating mesh normals ..." << endl;
		mesh.GenPerVertexNormal();
		LoadMeshShaderData();
	}

	if (treelayout == LAYOUT_HORIZONTAL)
	{	
		offsetX[1] = BOARDER_RIGHT+BOARDER_X;
		offsetY[1] = 0;
	}
	else{
		offsetX[1] = 0 ;
		offsetY[1] = BOARDER_TOP+BOARDER_Y;
	}

	/****************************************/
	/*         Here's the GLUI code         */
	/****************************************/

	printf("GLUI version: %3.2f\n", GLUI_Master.get_version());

	/*** Create the side subwindow ***/
	glui = GLUI_Master.create_glui_subwindow(main_window,
		GLUI_SUBWINDOW_RIGHT);

	/***** Control for object params *****/
	GLUI_Panel* obj_panel = new GLUI_Panel(glui, "Properties");
	obj_panel->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_Spinner *pruneSpinner = new GLUI_Spinner(obj_panel, "Pruning: ", &pruningThreshold, 2, updatePruning);
	pruneSpinner->set_int_limits(1, 60);
	pruneSpinner->set_int_val(pruningThreshold);
	pruneSpinner->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_Spinner *sigArcPSpinner = new GLUI_Spinner(obj_panel, "p value: ", &sigArcP, 2, updateSigArcs);
	sigArcPSpinner->set_float_limits(0, 1);
	sigArcPSpinner->set_float_val(sigArcP);
	sigArcPSpinner->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_Spinner *sigArcVrSpinner = new GLUI_Spinner(obj_panel, "Vol Overlap: ", &sigArcVr, 2, updateSigArcs);
	sigArcVrSpinner->set_float_limits(0, 1);
	sigArcVrSpinner->set_float_val(sigArcVr);
	sigArcVrSpinner->set_alignment(GLUI_ALIGN_LEFT);

	/***** Control for layout params *****/
	GLUI_Panel* layoutPanel_base = new GLUI_Panel(glui, "Layout");
	layoutPanel_base->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_Panel* layoutPanel_main = new GLUI_Panel(layoutPanel_base, "Mapping Scale");
	layoutPanel_main->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_RadioGroup* radioGroup_main = new GLUI_RadioGroup(layoutPanel_main, &UI_mappingScale, 3, updateLayout);
	new GLUI_RadioButton(radioGroup_main, "Linear");
	new GLUI_RadioButton(radioGroup_main, "Scientific");
	new GLUI_RadioButton(radioGroup_main, "Logorithm");
	radioGroup_main->set_int_val(1);
	radioGroup_main->set_alignment(GLUI_ALIGN_LEFT);

	GLUI_Rollout* layoutPanel_alt = new GLUI_Rollout(layoutPanel_base, "Alt Mapping Scale", 0);
	layoutPanel_alt->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_RadioGroup* radioGroup_alt = new GLUI_RadioGroup(layoutPanel_alt, &UI_mappingScale_alt, UI_ALTSCALE_ID, updateLayout);
	new GLUI_RadioButton(radioGroup_alt, "Linear");
	new GLUI_RadioButton(radioGroup_alt, "Scientific");
	new GLUI_RadioButton(radioGroup_alt, "Logorithm");
	radioGroup_alt->set_int_val(1);
	radioGroup_alt->set_alignment(GLUI_ALIGN_LEFT);

	GLUI_Spinner *scale_spinner = new GLUI_Spinner(layoutPanel_alt, "Zoom:", &zoomScale, 5, updateLayout);
	scale_spinner->set_float_limits(.2f, 50.0);
	scale_spinner->set_alignment(GLUI_ALIGN_LEFT);

	GLUI_Rollout* histoSide = new GLUI_Rollout(layoutPanel_base, "Histogram Side", 0);
	histoSide->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_RadioGroup* radioGroup1 = new GLUI_RadioGroup(histoSide, &UI_histogramSide, 6, updateLayout);
	new GLUI_RadioButton(radioGroup1, "Left");
	new GLUI_RadioButton(radioGroup1, "Symmetric");
	new GLUI_RadioButton(radioGroup1, "Right");
	radioGroup1->set_int_val(2);
	radioGroup1->set_alignment(GLUI_ALIGN_LEFT);

	GLUI_Rollout* comparison_panel = new GLUI_Rollout(glui, "Comparison", 0);
	comparison_panel->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_RadioGroup* radioGroup_comp = new GLUI_RadioGroup(comparison_panel, &UI_comparison, 7, updateComparison);
	new GLUI_RadioButton(radioGroup_comp, "In Place");
	new GLUI_RadioButton(radioGroup_comp, "Snapping");
	new GLUI_RadioButton(radioGroup_comp, "Alt View");
	radioGroup_comp->set_int_val(0);
	radioGroup_comp->set_alignment(GLUI_ALIGN_LEFT);


	snap_panel = new GLUI_Rollout(glui, "Snap Position", 0);
	snap_panel->set_alignment(GLUI_ALIGN_LEFT);
	snap_panel->disable();
	GLUI_RadioGroup* radioGroup_snapPos = new GLUI_RadioGroup(snap_panel, &UI_snapPos, 8, updateSnapPos);
	new GLUI_RadioButton(radioGroup_snapPos, "Top");
	new GLUI_RadioButton(radioGroup_snapPos, "Middle");
	new GLUI_RadioButton(radioGroup_snapPos, "Bottom");
	radioGroup_snapPos->set_int_val(2);
	radioGroup_snapPos->set_alignment(GLUI_ALIGN_LEFT);


	altZoom_panel = new GLUI_Rollout(glui, "Alt View Zoom", 0);
	altZoom_panel->set_alignment(GLUI_ALIGN_LEFT);
	altZoom_panel->disable();
	GLUI_Spinner *scaleX_spinner = new GLUI_Spinner(altZoom_panel, "x scale:", &altViewScaleX);
	scaleX_spinner->set_float_limits(.2f, 20.0);
	scaleX_spinner->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_Spinner *scaleY_spinner = new GLUI_Spinner(altZoom_panel, "y scale:", &altViewScaleY);
	scaleY_spinner->set_float_limits(.2f, 20.0);
	scaleY_spinner->set_alignment(GLUI_ALIGN_LEFT);

	new GLUI_StaticText(comparison_panel, "Front Alpha");
	GLUI_Scrollbar* frontAlphaSlider = new GLUI_Scrollbar
		(comparison_panel, "Front Alpha", GLUI_SCROLL_HORIZONTAL, &UI_contourTreeAlpha_front, 9, updateAlpha);
	frontAlphaSlider->set_float_limits(0, 1);
	frontAlphaSlider->set_float_val(1);
	new GLUI_StaticText(comparison_panel, "Back Alpha");
	GLUI_Scrollbar* backAlphaSlider = new GLUI_Scrollbar
		(comparison_panel, "Back Alpha", GLUI_SCROLL_HORIZONTAL, &UI_contourTreeAlpha_back, 10, updateAlpha);
	backAlphaSlider->set_float_limits(0, 1);
	backAlphaSlider->set_float_val(1);


	GLUI_Rollout *magnifier_panel = new GLUI_Rollout(glui, "Magnifier", 0);
	magnifier_panel->set_alignment(GLUI_ALIGN_LEFT);
	new GLUI_StaticText(magnifier_panel, "Radius");
	GLUI_Scrollbar* magnifier_radiusSlider = new GLUI_Scrollbar
		(magnifier_panel, "Magnify Radius", GLUI_SCROLL_HORIZONTAL, &magnifier_radius);
	magnifier_radiusSlider->set_int_limits(0, 600);
	magnifier_radiusSlider->set_int_val(magnifier_radius);
	magnifier_radiusSlider->set_alignment(GLUI_ALIGN_LEFT);
	new GLUI_StaticText(magnifier_panel, "Scale");
	GLUI_Scrollbar* magnifier_scaleSlider = new GLUI_Scrollbar
		(magnifier_panel, "Magnify Scale", GLUI_SCROLL_HORIZONTAL, &magnifier_scale);
	magnifier_scaleSlider->set_float_limits(1, 10);
	magnifier_scaleSlider->set_float_val(magnifier_scale);
	magnifier_scaleSlider->set_alignment(GLUI_ALIGN_LEFT);
	new GLUI_StaticText(magnifier_panel, "Focus");
	GLUI_Scrollbar* magnifier_focusRatioSlider = new GLUI_Scrollbar
		(magnifier_panel, "Focus Radius", GLUI_SCROLL_HORIZONTAL, &fishEye_focusRadiusRatio);
	magnifier_focusRatioSlider->set_float_limits(0, 1);
	magnifier_focusRatioSlider->set_float_val(fishEye_focusRadiusRatio);
	magnifier_focusRatioSlider->set_alignment(GLUI_ALIGN_LEFT);


	GLUI_Rollout *component_panel = new GLUI_Rollout(glui, "Component", 0);
	component_panel->set_alignment(GLUI_ALIGN_LEFT);
	UI_drawTracks = showTracks ? 1 : 0;
	UI_drawMesh = showMesh ? 1 : 0;
	GLUI_Checkbox* trackCheckbox = new GLUI_Checkbox(component_panel, "Show Tracks", &UI_drawTracks, -1, updateComponent);
	GLUI_Checkbox* meshCheckbox = new GLUI_Checkbox(component_panel, "Show Mesh", &UI_drawMesh, -1, updateComponent);
	if (!showTracks) trackCheckbox->disable();
	if (!showMesh) meshCheckbox->disable();

	GLUI_Rollout *roi_panel = new GLUI_Rollout(glui, "ROI", 0);
	roi_panel->set_alignment(GLUI_ALIGN_LEFT);
	GLUI_Checkbox* roiCheckbox = new GLUI_Checkbox(roi_panel, "Show ROI", &UI_drawROI, -1, updateROI);
	roiRatioSpinner = new GLUI_Spinner(roi_panel, "ROI Ratio: ", &UI_roiRatio, 2, updateROI);
	roiRatioSpinner->set_float_limits(0, 1);
	roiRatioSpinner->disable();


	GLUI_Rollout* diff_panel = new GLUI_Rollout(glui, "Diff Tree", 0);
	diff_panel->set_alignment(GLUI_ALIGN_LEFT);
	if(!showDiffTree) diff_panel->disable();
	GLUI_RadioGroup* radioGroup_diff = new GLUI_RadioGroup(diff_panel, &UI_diff, 8, updateDiff);
	new GLUI_RadioButton(radioGroup_diff, "Intersection");
	new GLUI_RadioButton(radioGroup_diff, "Union");
	new GLUI_RadioButton(radioGroup_diff, "Complement");
	radioGroup_diff->set_alignment(GLUI_ALIGN_LEFT);

	glui->set_main_gfx_window(main_window);

	glutMainLoop();

	return EXIT_SUCCESS;
}

