
#include <GL/glew.h>
#include "MyContourTree.h"
#include "FollowCubeTables.h"
#include "Shader.h"
#include "ColorScaleTable.h"

#include <queue>
#include <math.h>
#include <iostream>

class CellSurface
{
public:
	long x, y, z, entryFaceEdge;
	CellSurface(long X, long Y, long Z, long EntryFaceEdge) { x = X; y = Y; z = Z; entryFaceEdge = EntryFaceEdge; }
};

void MyContourTree::RenderContour(long arc){
	if (superarcs[arc].CheckFlag(Superarc::isSelected))
		FollowHierarchicalPathSeed(arc, currentSelectionValue);
	else
		FollowHierarchicalPathSeed(arc, superarcs[arc].seedValue);
}

void MyContourTree::FollowHierarchicalPathSeedUnagregated(int sArc, float ht){
	long whichArc = sArc;
	while (superarcs[whichArc].topArc != NO_SUPERARC){
		if (*(supernodes[superarcs[superarcs[whichArc].topArc].bottomID].value) <= ht)
			whichArc = superarcs[whichArc].topArc;
		else
			whichArc = superarcs[whichArc].bottomArc;
	}

//	glBegin(GL_TRIANGLES);
	FollowContour(ht, superarcs[whichArc]);	
//	glEnd();

	pathLengths[sArc] = pathLength;
	nContourTriangles[sArc] = triangleCount;
}

// need special treatment for aggregated arcs
void MyContourTree::FollowHierarchicalPathSeed(int sArc, float ht){
	long whichArc = sArc;
	if (this->IsArcAggregated(whichArc)){
		queue<long> arc2visit;
		arc2visit.push(whichArc);
		while (!arc2visit.empty()){
			long thisArc = arc2visit.front();
			arc2visit.pop();
			if (*(GetUnaggregatedNode(GetUnaggregatedArc(thisArc).topID).value) <= ht){
				long topNodeId = GetUnaggregatedArc(thisArc).topID;
				long theArc = GetUnaggregatedNode(topNodeId).upList;;
				for (long i = 0; i < GetUnaggregatedNode(topNodeId).upDegree; i++){
					arc2visit.push(theArc);
					theArc = GetUnaggregatedArc(theArc).nextUp;
				}
			}
			// if thisArc intersects, its upper children won't
			else if (*(GetUnaggregatedNode(GetUnaggregatedArc(thisArc).bottomID).value) <= ht){
				//FollowContour(ht, GetUnaggregatedArc(thisArc));
				FollowHierarchicalPathSeedUnagregated(thisArc, ht);
			}
		}
	}
	else {
		//FollowContour(ht, superarcs[whichArc]);
		FollowHierarchicalPathSeedUnagregated(sArc, ht);
	}
}

void MyContourTree::FollowContour(float ht, Superarc& theArc){
	long x, y, z;
	float *neighbour;
	float *thisEnd, *thatEnd;
	long nSteps = 1;

	if (theArc.seedFromLo != NULL){
		thisEnd = theArc.seedFromLo;
		thatEnd = theArc.seedToLo;
		while (*thatEnd <= ht){
			nSteps++;
			thisEnd = thatEnd;
			height.ComputeIndex(thisEnd, x, y, z);
			QueueJoinNeighbours(x, y, z);
			for (int i = 0; i < nNeighbours; i++){
				neighbour = &(height(neighbourQueue[i][0], neighbourQueue[i][1], neighbourQueue[i][2]));
				if (neighbour == thatEnd) continue;	
				if (compareHeight(neighbour, thatEnd) > 0)
					thatEnd = neighbour;
			}
		}
	}
	else if (theArc.seedFromHi != NULL){
		thisEnd = theArc.seedFromHi;
		thatEnd = theArc.seedToHi;	
		while (*thatEnd >= ht){ 
			nSteps++;
			thisEnd = thatEnd;
			height.ComputeIndex(thisEnd, x, y, z);
			QueueSplitNeighbours(x, y, z);
			for (int i = 0; i < nNeighbours; i++){
				neighbour = &(height(neighbourQueue[i][0], neighbourQueue[i][1], neighbourQueue[i][2]));
				if (neighbour == thatEnd) continue;	
				if (compareHeight(neighbour, thatEnd) < 0)
					thatEnd = neighbour;
			}
		}
	}
	else{
		printf("Major error at %s: %d: no seed available\n", __FILE__, __LINE__);
		return;
	} 
	pathLength = nSteps;
	triangleCount = 0;
	FollowSurface(ht, thisEnd, thatEnd);
}


void MyContourTree::FollowSurface(float ht, float *p1, float *p2){
	long x1, y1, z1, x2, y2, z2;															
	long xm, ym, zm, xc, yc, zc;															
	int i1, i2;																		
	long whichCubeEdge, theSurface, whichFaceEdge;											
	long theCase = 0;																	
	long whichVertex;																	
	float cubeVert[8];																	
	
	height.ComputeIndex(p1, x1, y1, z1); height.ComputeIndex(p2, x2, y2, z2);						
	
	xm = x1 < x2 ? x1 : x2;		ym = y1 < y2 ? y1 : y2;		zm = z1 < z2 ? z1 : z2;				
	xc = xm; 					yc = ym; 					zc = zm;							
	if (xc == xDim - 1) xc--;	if (yc == yDim - 1) yc--;	if (zc == zDim - 1) zc--;			
	
	i1 = ((x1 - xc) << 2) + ((y1 - yc) << 1) + (z1 - zc);		i2 = ((x2 - xc) << 2) + ((y2 - yc) << 1) + (z2 - zc);
		
	whichCubeEdge = vertex2edge[i1][i2];													
	if (whichCubeEdge == -1){
		printf("Major problem: %ld to %ld is not a valid seed edge\n", i1, i2); return;
	}

	
	if ((xc < 0) || (yc < 0) || (zc < 0)) return;											
	if ((xc > xDim - 2) || (yc > yDim - 2) || (zc > zDim - 2)) return;								

	cubeVert[0] = height(xc, yc, zc);
	cubeVert[1] = height(xc, yc, zc + 1);
	cubeVert[2] = height(xc, yc + 1, zc);
	cubeVert[3] = height(xc, yc + 1, zc + 1);
	cubeVert[4] = height(xc + 1, yc, zc);
	cubeVert[5] = height(xc + 1, yc, zc + 1);
	cubeVert[6] = height(xc + 1, yc + 1, zc);
	cubeVert[7] = height(xc + 1, yc + 1, zc + 1);

	theCase = 0;
	for (whichVertex = 0; whichVertex < 8; whichVertex++)										
		if (ht < cubeVert[whichVertex])													
			theCase |= (1 << whichVertex);												

	theSurface = seedEdge2Surface[theCase][whichCubeEdge];										
	if (theSurface == -1){
		printf("Major problem: Edge %ld in case %ld is not a valid seed edge\n", whichCubeEdge, theCase); return;
	}

	whichFaceEdge = surface2exitEdges[theCase][theSurface][0];									

	IntersectSurface(ht, xc, yc, zc, whichFaceEdge);											
	UnFlagSurface(ht, xc, yc, zc, whichFaceEdge);											
} 

void MyContourTree::IntersectSurface(float ht, long x, long y, long z, int theEntryFaceEdge){
	long theCase = 0;																	
	long theSurface;																	
	long whichExitFaceEdge;																
	long theExitFaceEdge;																
	float cubeVert[8];																	
	long whichVertex;																	
	long whichTri;																		
	queue<CellSurface> theQueue;															
	CellSurface theCellSurface = CellSurface(x, y, z, theEntryFaceEdge);							

	theQueue.push(theCellSurface);														

	while (!theQueue.empty()){ 
		theCellSurface = theQueue.front();													
		theQueue.pop();																
		if ((theCellSurface.x < 0) || (theCellSurface.y < 0) || (theCellSurface.z < 0)) continue;		
		if ((theCellSurface.x > xDim - 2) || (theCellSurface.y > yDim - 2) || (theCellSurface.z > zDim - 2)) continue;

		cubeVert[0] = height(theCellSurface.x, theCellSurface.y, theCellSurface.z);				
		cubeVert[1] = height(theCellSurface.x, theCellSurface.y, theCellSurface.z + 1);
		cubeVert[2] = height(theCellSurface.x, theCellSurface.y + 1, theCellSurface.z);
		cubeVert[3] = height(theCellSurface.x, theCellSurface.y + 1, theCellSurface.z + 1);
		cubeVert[4] = height(theCellSurface.x + 1, theCellSurface.y, theCellSurface.z);
		cubeVert[5] = height(theCellSurface.x + 1, theCellSurface.y, theCellSurface.z + 1);
		cubeVert[6] = height(theCellSurface.x + 1, theCellSurface.y + 1, theCellSurface.z);
		cubeVert[7] = height(theCellSurface.x + 1, theCellSurface.y + 1, theCellSurface.z + 1);

		theCase = 0;
		for (whichVertex = 0; whichVertex < 8; whichVertex++)									
			if (ht < cubeVert[whichVertex])												
				theCase |= (1 << whichVertex);											

		theSurface = entryEdge2Surface[theCase][theCellSurface.entryFaceEdge];					
		if (Visited(theCellSurface.x, theCellSurface.y, theCellSurface.z, theSurface)) continue;		
		Visit(theCellSurface.x, theCellSurface.y, theCellSurface.z, theSurface);	

		for (whichTri = 0; whichTri < 3 * nTriangles[theCase][theSurface]; whichTri += 3)				
			RenderTriangle(ht, theCellSurface.x, theCellSurface.y, theCellSurface.z, cubeVert,
			mcFollowTriangles[theCase][theSurface][whichTri + 0],
			mcFollowTriangles[theCase][theSurface][whichTri + 1],
			mcFollowTriangles[theCase][theSurface][whichTri + 2]);
		
		for (whichExitFaceEdge = 0; whichExitFaceEdge < nExitEdges[theCase][theSurface]; whichExitFaceEdge++)
		{ 
			theExitFaceEdge = surface2exitEdges[theCase][theSurface][whichExitFaceEdge];				

			theQueue.push(CellSurface(
				theCellSurface.x + exitDirection[theExitFaceEdge][0], 								
				theCellSurface.y + exitDirection[theExitFaceEdge][1],								
				theCellSurface.z + exitDirection[theExitFaceEdge][2], 								
				exit2entryEdge[theExitFaceEdge]));												
		} 
	} 
} 

void InterpolatePoint(float x1, float y1, float z1, float h1, float x2, float y2, float z2, float h2, float ht, float* result);

void MyContourTree::InterpolateVertex(long x, long y, long z, int edge, float *cubeVert, float ht){
	int v0 = edgeVertices[edge][0], v1 = edgeVertices[edge][1];
	int x0 = x + mcFollowVertexCoords[v0][0], y0 = y + mcFollowVertexCoords[v0][1], z0 = z + mcFollowVertexCoords[v0][2];
	int x1 = x + mcFollowVertexCoords[v1][0], y1 = y + mcFollowVertexCoords[v1][1], z1 = z + mcFollowVertexCoords[v1][2];
	float height0 = cubeVert[v0], height1 = cubeVert[v1];

	GLfloat norm0[3], norm1[3], norm[3], vertex[3];
	float normNorm;																	

	CentralDifferenceNormal(x0, y0, z0, norm0);
	CentralDifferenceNormal(x1, y1, z1, norm1);
	InterpolatePoint(norm0[0], norm0[1], norm0[2], height0, norm1[0], norm1[1], norm1[2], height1, ht, norm);

	normNorm = norm[0] * norm[0] + norm[1] * norm[1] + norm[2] * norm[2];								
	normNorm = 1.0 / sqrt(normNorm);
	norm[0] *= normNorm; norm[1] *= normNorm; norm[2] *= normNorm;								

	//glNormal3fv(norm);
	mNormals.push_back(norm[0]);
	mNormals.push_back(norm[1]);
	mNormals.push_back(norm[2]);

	InterpolatePoint(x0, y0, z0, cubeVert[v0], x1, y1, z1, cubeVert[v1], ht, vertex);
	//glVertex3fv(vertex);
	mVertices.push_back(vertex[0]);
	mVertices.push_back(vertex[1]);
	mVertices.push_back(vertex[2]);

	mNames.push_back(0);
	mNames.push_back(1);
	mNames.push_back(mCurrentArcName);
	mNames.push_back(mIndex);
} 

void MyContourTree::RenderTriangle(float ht, long x, long y, long z, float *cubeVert, int edge1, int edge2, int edge3){
	InterpolateVertex(x, y, z, edge1, cubeVert, ht);
	InterpolateVertex(x, y, z, edge2, cubeVert, ht);
	InterpolateVertex(x, y, z, edge3, cubeVert, ht);
	triangleCount++;
} 


void MyContourTree::UpdateContours(){
	int theArc;
	int nContours = 0;
	mNormals.clear();
	mVertices.clear();
	mNames.clear();
	for (theArc = 0; theArc < nActiveArcs; theArc++){
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSuppressed)) continue;
		if (superarcs[active[theArc]].CheckFlag(Superarc::isSelected)) continue;
		mCurrentArcName = active[theArc];
		RenderContour(active[theArc]);
		nContours++;
	}
	for (theArc = 0; theArc < nSelectedArcs; theArc++){
		mCurrentArcName = selected[theArc];
		RenderContour(selected[theArc]);
		nContours++;
	}

	LoadContourGeometry();
}

void MyContourTree::FlexibleContours(){
	UpdateContours();
	RenderContours();
}

void MyContourTree::DestoryContourDrawingBuffer(){
	if (glIsVertexArray(mVertexArray)){
		glDeleteVertexArrays(1, &mVertexArray);
	}
	if (glIsBuffer(mVertexBuffer)){
		glDeleteBuffers(1, &mVertexBuffer);
	}
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	if (glIsBuffer(mNameBuffer)){
		glDeleteBuffers(1, &mNameBuffer);
	}
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glDeleteProgram(mContourShaderProgram);
	if (glIsTexture(mDiffColorTexture)){
		glDeleteTextures(1, &mDiffColorTexture);
	}
}

void MyContourTree::LoadContourGeometry(){
	if (mVertices.size() > 0){
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), &mVertices[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), &mNormals[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, mNameBuffer);
		glBufferData(GL_ARRAY_BUFFER, mNames.size() * sizeof(int), &mNames[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		mNumVertices = mVertices.size() / 3;
		int* index = new int[mNumVertices];
		for (int i = 0; i < mNumVertices; i++) index[i] = i;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, mNumVertices * sizeof(int), index, GL_DYNAMIC_DRAW);
		delete[] index;
	}
	else{
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, mNameBuffer);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MyContourTree::RenderContours(){
	glDisable(GL_BLEND);
	//glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	glUseProgram(mContourShaderProgram);
	glBindVertexArray(mVertexArray);

	int mvmatLocation = glGetUniformLocation(mContourShaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

	int projmatLocation = glGetUniformLocation(mContourShaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

	int colorLocation = glGetUniformLocation(mContourShaderProgram, "color");
	glUniform3fv(colorLocation, 1, mContourColour);

	if (glIsTexture(mDiffColorTexture)){
		int diffLocation = glGetUniformLocation(mContourShaderProgram, "colorTexture");
		glUniform1i(diffLocation, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_3D, mDiffColorTexture);

		diffLocation = glGetUniformLocation(mContourShaderProgram, "volSize");
		glUniform3f(diffLocation, height.XDim(), height.YDim(), height.ZDim());

		diffLocation = glGetUniformLocation(mContourShaderProgram, "bUseTextureColor");
		glUniform1i(diffLocation, 1);
	}
	else{
		int diffLocation = glGetUniformLocation(mContourShaderProgram, "bUseTextureColor");
		glUniform1i(diffLocation, 0);
	}

	glDrawElements(GL_TRIANGLES, mNumVertices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyContourTree::BuildContourGeomeryBuffer(){
	if (glIsVertexArray(mVertexArray)){
		glDeleteVertexArrays(1, &mVertexArray);
	}
	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);
	// vertex
	if (glIsBuffer(mVertexBuffer)){
		glDeleteBuffers(1, &mVertexBuffer);
	}
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), &mVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mPositionAttribute);
	glVertexAttribPointer(mPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(mNormalBuffer)){
		glDeleteBuffers(1, &mNormalBuffer);
	}
	glGenBuffers(1, &mNormalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNormalBuffer);
	//glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), &mNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mNormalAttribute);
	glVertexAttribPointer(mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// name
	if (glIsBuffer(mNameBuffer)){
		glDeleteBuffers(1, &mNameBuffer);
	}
	glGenBuffers(1, &mNameBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mNameBuffer);
	//glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), &mNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(mNameAttribute);
	glVertexAttribIPointer(mNameAttribute, 4, GL_INT, 0, 0);
	// index
	if (glIsBuffer(mIndexBuffer)){
		glDeleteBuffers(1, &mIndexBuffer);
	}
	glGenBuffers(1, &mIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
	mNumVertices = 0;
	glBindVertexArray(0);

	/*
	// framebuffer
	// color
	if (glIsTexture(mColorTexture)) {
		glDeleteTextures(1, &mColorTexture);
	}
	glGenTextures(1, &mColorTexture);
	glBindTexture(GL_TEXTURE_2D, mColorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// DEPTH
	if (glIsTexture(mDepthTexture)) {
		glDeleteTextures(1, &mDepthTexture);
	}
	glGenTextures(1, &mDepthTexture);
	glBindTexture(GL_TEXTURE_2D, mDepthTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// name
	if (glIsTexture(mNameTexture)) {
		glDeleteTextures(1, &mNameTexture);
	}
	glGenTextures(1, &mNameTexture);
	glBindTexture(GL_TEXTURE_2D, mNameTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, width, height, 0, GL_BGRA_INTEGER, GL_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// FRAMEBUFFER ASSEMBLE
	if (glIsFramebuffer(mFrameBuffer)) {
		glDeleteFramebuffers(1, &mFrameBuffer);
	}
	glGenFramebuffers(1, &mFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mColorTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+1, mNameTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mDepthTexture, 0);

	GLenum DrawBuffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0+1 };
	glDrawBuffers(2, DrawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	*/
}

void MyContourTree::LoadDiffColorTexture(Array3D<float>& diffs, float minDiff, float maxDiff){
	float *texColors = new float[diffs.NElements() * 4];
	// be careful with different arrangement
	// between opengl and Array3D
	int idx = 0;
	for (int k = 0; k < diffs.ZDim(); k++){
		for (int j = 0; j < diffs.YDim(); j++){
			for (int i = 0; i < diffs.XDim(); i++){
				float rgba[4];
				float diff = diffs(i,j,k);
				ColorScaleTable::DiffValueToColor(diff, minDiff, maxDiff, rgba);
				memcpy(texColors + idx * 4, rgba, 4 * sizeof(float));
				idx++;
			}
		}
	}

	if (glIsTexture(mDiffColorTexture)){
		glDeleteTextures(1, &mDiffColorTexture);
	}
	glGenTextures(1, &mDiffColorTexture);
	glBindTexture(GL_TEXTURE_3D, mDiffColorTexture);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, diffs.XDim(), diffs.YDim(), diffs.ZDim(), 0, GL_RGBA, GL_FLOAT, texColors);
	delete[] texColors;

	glBindTexture(GL_TEXTURE_3D, 0);
}

void MyContourTree::RemoveDiffColorTexture(){
	if (glIsTexture(mDiffColorTexture)){
		glDeleteTextures(1, &mDiffColorTexture);
	}
	mDiffColorTexture = -1;
}

void MyContourTree::SetContourColour(float color[3]){
	mContourColour[0] = color[0];
	mContourColour[1] = color[1];
	mContourColour[2] = color[2];
}

void MyContourTree::SetIndex(int idx){
	mIndex = idx;
}

int MyContourTree::GetIndex() const{
	return mIndex;
}

void MyContourTree::CompileContourShader(){
	glDeleteProgram(mContourShaderProgram);
	mContourShaderProgram = InitShader("contour.vert", "contour.frag", "fragColour", "name");

	mNormalAttribute = glGetAttribLocation(mContourShaderProgram, "normal");
	mPositionAttribute = glGetAttribLocation(mContourShaderProgram, "position");
	mNameAttribute = glGetAttribLocation(mContourShaderProgram, "name");
}

MyContourTree::ContourGeometryDataBuffer::ContourGeometryDataBuffer(MyContourTree* ct){
	if (glIsVertexArray(vertexArray)){
		glDeleteVertexArrays(1, &vertexArray);
	}
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	// vertex
	if (glIsBuffer(vertexBuffer)){
		glDeleteBuffers(1, &vertexBuffer);
	}
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	//glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), &mVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(ct->mPositionAttribute);
	glVertexAttribPointer(ct->mPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// normal
	if (glIsBuffer(normalBuffer)){
		glDeleteBuffers(1, &normalBuffer);
	}
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	//glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), &mNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(ct->mNormalAttribute);
	glVertexAttribPointer(ct->mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// name
	if (glIsBuffer(nameBuffer)){
		glDeleteBuffers(1, &nameBuffer);
	}
	glGenBuffers(1, &nameBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, nameBuffer);
	//glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), &mNormals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(ct->mNameAttribute);
	glVertexAttribIPointer(ct->mNameAttribute, 4, GL_INT, 0, 0);
	// index
	if (glIsBuffer(indexBuffer)){
		glDeleteBuffers(1, &indexBuffer);
	}
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	numVertices = 0;
	glBindVertexArray(0);
}

MyContourTree::ContourGeometryDataBuffer::~ContourGeometryDataBuffer(){
	if (glIsBuffer(vertexBuffer)){
		glDeleteBuffers(1, &vertexBuffer);
	}
	if (glIsBuffer(normalBuffer)){
		glDeleteBuffers(1, &normalBuffer);
	}
	if (glIsBuffer(nameBuffer)){
		glDeleteBuffers(1, &nameBuffer);
	}
	if (glIsBuffer(indexBuffer)){
		glDeleteBuffers(1, &indexBuffer);
	}
	if (glIsVertexArray(vertexArray)){
		glDeleteVertexArrays(1, &vertexArray);
	}
}

void MyContourTree::ContourGeometryDataBuffer::LoadGeometry(MyContourTree* ct){
	if (ct->mVertices.size() > 0){
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, ct->mVertices.size() * sizeof(float), &ct->mVertices[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, ct->mNormals.size() * sizeof(float), &ct->mNormals[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, nameBuffer);
		glBufferData(GL_ARRAY_BUFFER, ct->mNames.size() * sizeof(int), &ct->mNames[0], GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		numVertices = ct->mVertices.size() / 3;
		int* index = new int[numVertices];
		for (int i = 0; i < numVertices; i++) index[i] = i;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numVertices * sizeof(int), index, GL_DYNAMIC_DRAW);
		delete[] index;
	}
	else{
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, nameBuffer);
		glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
		numVertices = 0;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/*
void MyContourTree::ClearArcContourGeometry(){
	for (std::map<long, ContourGeometryDataBuffer*>::const_iterator itr
		= mArcContourGeometry.begin();
		itr != mArcContourGeometry.end(); itr++){
		delete itr->second;
	}
	mArcContourGeometry.clear();
}
*/

void MyContourTree::DrawArcContour(long arc){
	/*
	std::map<long, ContourGeometryDataBuffer*>::const_iterator itr
		= mArcContourGeometry.find(arc);

	unsigned int vertexArray, numVertices;
	if (itr != mArcContourGeometry.end()){
		vertexArray = itr->second->vertexArray;
		numVertices = itr->second->numVertices;
	}
	else{
		mNormals.clear();
		mVertices.clear();
		mNames.clear();
		FollowHierarchicalPathSeed(arc, *supernodes[superarcs[arc].bottomID].value);
		mArcContourGeometry[arc] = new ContourGeometryDataBuffer(this);
		mArcContourGeometry[arc]->LoadGeometry(this);
		vertexArray = mArcContourGeometry[arc]->vertexArray;
		numVertices = mArcContourGeometry[arc]->numVertices;
	}
	*/
	unsigned int vertexArray, numVertices;
	mNormals.clear();
	mVertices.clear();
	mNames.clear();
	mCurrentArcName = arc;
	FollowHierarchicalPathSeed(arc, max(*supernodes[superarcs[arc].bottomID].value,0.01f));
	ContourGeometryDataBuffer* geoBuffer = new ContourGeometryDataBuffer(this);
	geoBuffer->LoadGeometry(this);
	vertexArray = geoBuffer->vertexArray;
	numVertices = geoBuffer->numVertices;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(mContourShaderProgram);
	glBindVertexArray(vertexArray);

	int mvmatLocation = glGetUniformLocation(mContourShaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

	int projmatLocation = glGetUniformLocation(mContourShaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

	int colorLocation = glGetUniformLocation(mContourShaderProgram, "color");
	glUniform3fv(colorLocation, 1, mContourColour);

	if (glIsTexture(mDiffColorTexture)){
		int diffLocation = glGetUniformLocation(mContourShaderProgram, "colorTexture");
		glUniform1i(diffLocation, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_3D, mDiffColorTexture);

		diffLocation = glGetUniformLocation(mContourShaderProgram, "volSize");
		glUniform3f(diffLocation, height.XDim(), height.YDim(), height.ZDim());

		diffLocation = glGetUniformLocation(mContourShaderProgram, "bUseTextureColor");
		glUniform1i(diffLocation, 1);
	}
	else{
		int diffLocation = glGetUniformLocation(mContourShaderProgram, "bUseTextureColor");
		glUniform1i(diffLocation, 0);
	}

	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);


	delete geoBuffer;
}

void MyContourTree::RenderAllUpperLeaveContours(float voxelRatio){
	mNormals.clear();
	mVertices.clear();
	mNames.clear();
	for (long i = 0; i < nValidArcs; i++){
		long arc = valid[i];
		if (supernodes[superarcs[arc].topID].IsUpperLeaf()){
			vector<float*>& voxes = mArcNodes[arc];
			int idx = voxes.size()*voxelRatio + 0.5;
			if (idx <= 0) continue;
			if (idx >= voxes.size() - 1) idx = voxes.size() - 1;
			mCurrentArcName = arc;
			FollowHierarchicalPathSeed(arc, *voxes[idx]);
		}
	}
	unsigned int vertexArray, numVertices;
	ContourGeometryDataBuffer* geoBuffer = new ContourGeometryDataBuffer(this);
	geoBuffer->LoadGeometry(this);
	vertexArray = geoBuffer->vertexArray;
	numVertices = geoBuffer->numVertices;

	glUseProgram(mContourShaderProgram);
	glBindVertexArray(vertexArray);

	int mvmatLocation = glGetUniformLocation(mContourShaderProgram, "mvMat");
	float modelViewMat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMat);
	glUniformMatrix4fv(mvmatLocation, 1, GL_FALSE, modelViewMat);

	int projmatLocation = glGetUniformLocation(mContourShaderProgram, "projMat");
	float projMat[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projMat);
	glUniformMatrix4fv(projmatLocation, 1, GL_FALSE, projMat);

	int colorLocation = glGetUniformLocation(mContourShaderProgram, "color");
	glUniform3fv(colorLocation, 1, mContourColour);

	if (glIsTexture(mDiffColorTexture)){
		int diffLocation = glGetUniformLocation(mContourShaderProgram, "colorTexture");
		glUniform1i(diffLocation, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_3D, mDiffColorTexture);

		diffLocation = glGetUniformLocation(mContourShaderProgram, "volSize");
		glUniform3f(diffLocation, height.XDim(), height.YDim(), height.ZDim());

		diffLocation = glGetUniformLocation(mContourShaderProgram, "bUseTextureColor");
		glUniform1i(diffLocation, 1);
	}
	else{
		int diffLocation = glGetUniformLocation(mContourShaderProgram, "bUseTextureColor");
		glUniform1i(diffLocation, 0);
	}

	glDrawElements(GL_TRIANGLES, numVertices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);


	delete geoBuffer;
}