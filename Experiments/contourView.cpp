#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>

#include "vtkStructuredPointsReader.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPoints.h" 
#include "vtkQuad.h"
#include "vtkUnstructuredGrid.h"
#include "vtkDataSetMapper.h"
#include "vtkPolyDataConnectivityFilter.h"

#include "vtkConeSource.h" 
#include "vtkInteractorStyleTrackballCamera.h" 
#include "vtkPolyDataMapper.h" 
#include <vtkContourFilter.h>
#include <vtkOutlineFilter.h>
#include <vtkQuadric.h>
#include <vtkSampleFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkNIFTIImageReader.h>


#include "MyTrackBall.h"
#include "MyGraphicsTool.h"
#include "MyPrimitiveDrawer.h"

int windowWidth = 1024;
int windowHeight = 512;
float zDistance = 1000;

MyTrackBall trackBall;

#include "Ric/RicVolume.h"
#include "Ric/RicMesh.h"

#include "CT_MSC.h"
#include "Helper.h"

vtkSmartPointer<vtkRenderer> ren;
vtkSmartPointer<vtkRenderWindow> renWin;
vtkSmartPointer<vtkNIFTIImageReader> reader;
vtkSmartPointer<vtkActor> isoActor[100];

ContourTreeNode * ContTree;
JoinSplitTreeNode* JoinTree;
JoinSplitTreeNode * SplitTree;
MorseSmaleCriticalPoint* MSCpoint;
vector<vector<int>> PairQ;
vector<vector<vector<int>>> MSCsep;
vector<vector<int>> MSCsepD;
vector<vector<int>> pairs;
vector<int> recurid;
float fMin, fMax;
int MSCriNum;
bool flag = false;
bool is_3D = true;
vector < vector <int> > DisplayNodePosition;//for tree-shape display
vector< vector <float> > TrueNodeposition;
vector < vector < vector <int> > > BrancheBox;
float **seeds, **iseeds;
int branchNum;
char * edgeCoordinator;
char * edgeActor;
int Width, Height, Center;
vector < vector <int> >  locationArray, branchLocationArray;
vector< vector < vector <int> > > BranchSep;

RicVolume downSampledTVol;


void BuildContourTree(){
	//downSampledTVol.Read("ACR_300_sub2_sub2_sub2.nii.gz");
	downSampledTVol.Read("C:/Users/GuohaoZhang/Desktop/tmpdata/compare/Patients/A0065/dti_fa.nii");
	//reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	//reader->SetFileName("ACR_300_sub2_sub2_sub2.nii.gz");
	downSampledTVol /= downSampledTVol.max;
	CT(ContTree, JoinTree, SplitTree, MSCpoint, PairQ,
		MSCsep, MSCsepD, pairs, recurid,
		downSampledTVol.vox, fMin, fMax, MSCriNum,
		downSampledTVol.get_numx(), downSampledTVol.get_numy(), downSampledTVol.get_numz(),
		flag, is_3D);
	branchNum = PairQ.size();
	seeds = f2vector(branchNum, 4);
	iseeds = f2vector(branchNum, 4);
	DisplayNodePosition = ReDrawHierarchyContTree_2(ContTree, MSCpoint, MSCriNum, MSCsep, MSCsepD,
		branchNum, pairs, recurid, PairQ, Width, Height, Center, locationArray, branchLocationArray,
		downSampledTVol.vox, fMin, fMax, seeds, BrancheBox, BranchSep);
	int seedNum = branchNum;
	TrueNodeposition = TruePosition(DisplayNodePosition, MSCriNum, Width, Height, Center);

	edgeCoordinator = new char[MSCriNum];
	edgeActor = new char[branchNum];

	for (int i = 0; i<branchNum; i++)
		edgeActor[i] = 0;

	for (int i = 0; i<MSCriNum; i++)
		edgeCoordinator[i] = 0;
}

int iseedNum = 0;

void extractISeeds(float value)
{
	iseedNum = 0;
	for (int i = 0; i<branchNum; i++)//
	{

		int saddle = PairQ[i][0], end = PairQ[i][1];
		if (edgeCoordinator[saddle] == 1 && edgeCoordinator[end] == 1 && (ContTree[saddle].value - value)*(ContTree[end].value - value) <= 0)
		{
			float temp1, temp2;
			//int *vect[3];
			//int i1=BranchSep[i][1][0], i2=BranchSep[i][1][1], i3=BranchSep[i][1][2];
			if (BranchSep[i].size() == 0)
				continue;

			temp2 = BranchSep[i][0][0]>-1 ? downSampledTVol.vox[BranchSep[i][0][0]][BranchSep[i][0][1]][BranchSep[i][0][2]] : -100000;
			for (int j = 1; j<BranchSep[i].size(); j++)
			{
				temp1 = temp2;
				temp2 = BranchSep[i][j][0]>-1 ? downSampledTVol.vox[BranchSep[i][j][0]][BranchSep[i][j][1]][BranchSep[i][j][2]] : -100000;
				if ((value - temp1)*(value - temp2) <= 0)
				{
					if (fabs(value - temp1)<fabs(value - temp2))
					{
						iseeds[i][0] = BranchSep[i][j - 1][0];
						iseeds[i][1] = BranchSep[i][j - 1][1];
						iseeds[i][2] = BranchSep[i][j - 1][2];
						iseeds[i][3] = temp1;
					}
					else
					{
						iseeds[i][0] = BranchSep[i][j][0];
						iseeds[i][1] = BranchSep[i][j][1];
						iseeds[i][2] = BranchSep[i][j][2];
						iseeds[i][3] = temp2;
					}
					iseedNum++;
					break;
				}
			}
		}

	}
}

void VTKContour(float value, float **seeds, int SeedNum, int fieldNum, int contourType)
{//contourType=0: exact all the contour of the given iso-value
	//contourType=1: exact only the largest contour of contour of given seed with the given isovalue
	//contourType=2: exact only the closest contour of given seed
	int maxNum = 80;
	vtkContourFilter *iso[80];
	vtkSmartPointer<vtkPolyDataMapper> isoMapper[80];
	float I = downSampledTVol.get_numx(), J = downSampledTVol.get_numy(), K = downSampledTVol.get_numz();

	vtkSmartPointer<vtkPolyDataConnectivityFilter> connectivityFilter[80];

	if (contourType != 2)
		SeedNum = 1;

	for (int i = 0; i<maxNum; i++)
	{
		ren->RemoveActor(isoActor[i]);
	}

	int c = 0;
	for (int j = 0; j<fieldNum; j++)
	{
		for (int i = 0; i<SeedNum; i++)
		{
			iso[c] = vtkContourFilter::New();
			//void vtkPolyDataAlgorithm::SetInput(vtkDataObject *)' : cannot convert parameter 1 from 'vtkStructuredPoints
			iso[c]->SetInputConnection(reader->GetOutputPort());
			iso[c]->SetValue(c, value); // for brain

			connectivityFilter[c] = vtkSmartPointer<vtkPolyDataConnectivityFilter>::New();
			connectivityFilter[c]->SetInputConnection(iso[c]->GetOutputPort());

			if (contourType == 1)
			{
				connectivityFilter[i]->SetExtractionModeToLargestRegion();
				connectivityFilter[c]->Update();
			}
			else if (contourType == 2)
			{
				connectivityFilter[c]->SetClosestPoint(float(seeds[i][0]), float(seeds[i][1]), float(seeds[i][2])); //break;	 
				connectivityFilter[c]->SetExtractionModeToClosestPointRegion();
				connectivityFilter[c]->Update();
			}


			//PPP
			isoMapper[c] = vtkSmartPointer<vtkPolyDataMapper>::New();
			//if(flag)	 
			isoMapper[c]->SetInputConnection(connectivityFilter[c]->GetOutputPort());
			//else 
			//isoMapper[i]->SetInput(iso[i]->GetOutput());
			isoMapper[c]->ScalarVisibilityOff();
			isoActor[c] = vtkSmartPointer<vtkActor>::New();
			isoActor[c]->SetMapper(isoMapper[c]);

			//extractedMapper->SetInputConnection(connectivityFilter->GetOutputPort());
			//extractedMapper->Update();
			switch (j)
			{
			case 0: isoActor[c]->GetProperty()->SetColor(141 / 255.0, 211 / 255.0, 199 / 255.0);   break;
			case 1: isoActor[c]->GetProperty()->SetColor(255 / 255.0, 255 / 255.0, 179 / 255.0);   break;
			case 2: isoActor[c]->GetProperty()->SetColor(190 / 255.0, 186 / 255.0, 218 / 255.0);   break;
			case 3: isoActor[c]->GetProperty()->SetColor(251 / 255.0, 128 / 255.0, 114 / 255.0);   break;
			case 4: isoActor[c]->GetProperty()->SetColor(128 / 255.0, 177 / 255.0, 211 / 255.0);   break;
			case 5: isoActor[c]->GetProperty()->SetColor(253 / 255.0, 180 / 255.0, 98 / 255.0);   break;
			case 6: isoActor[c]->GetProperty()->SetColor(179 / 255.0, 222 / 255.0, 105 / 255.0);   break;
			case 7: isoActor[c]->GetProperty()->SetColor(252 / 255.0, 205 / 255.0, 229 / 255.0);   break;
			}
			isoActor[c]->GetProperty()->SetOpacity(1);
			ren->AddActor(isoActor[c]);
			c++;
			if (c >= maxNum)
				break;
		}
		if (c >= maxNum)
			break;
	}

}

void DrawContourTree(){
	//int ** NodePosition=app->GUIDataManager()->NodePosition;
	vector< vector <float> > NodePosition = TrueNodeposition;
	//vector< vector<int> > NodePosition = NodePosition;

	float x0, y0, x1, y1;
	int count;

	//if(PairQ.size()<1)
	//     return;
	if (branchNum <= 0)
		return;

	/*x0=NodePosition[PairQ[0][0]][0]+center;
	//fprintf(fp,"x0%d,",x0);


	y0=NodePosition[PairQ[0][0]][1];

	x1=NodePosition[PairQ[0][1]][0]+center;
	y1=NodePosition[PairQ[0][1]][1];*/

	for (int i = 0; i<branchNum; i++)
	{
		edgeCoordinator[PairQ[i][0]] = edgeCoordinator[PairQ[i][1]] = 0;
	}

	int KK = branchNum>100 ? 100 : branchNum;
	if (KK>0)
	{
		//glColor3f(0.0, 0.0, 0.0);
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);

		//glVertex2f(x0*scale_x-dx, y0*scale_y-dy);
		//glVertex2f(x1*scale_x-dx, y0*scale_y-dy);

		glVertex2f(NodePosition[PairQ[0][0]][0], NodePosition[PairQ[0][0]][1] + 0.05);
		glVertex2f(NodePosition[PairQ[0][1]][0], NodePosition[PairQ[0][0]][1] + 0.05);

		//glVertex2f(x1*scale_x-dx, y0*scale_y-dy);
		//glVertex2f(x1*scale_x-dx, y1*scale_y-dy);

		glVertex2f(NodePosition[PairQ[0][1]][0], NodePosition[PairQ[0][0]][1] + 0.05);
		glVertex2f(NodePosition[PairQ[0][1]][0], NodePosition[PairQ[0][1]][1] + 0.05);

		glEnd();

		glPointSize(3);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
		glColor3f(0.0, 1.0, 0.0);
		//glVertex2f(x0*scale_x-dx, y0*scale_y-dy);
		glVertex2f(NodePosition[PairQ[0][0]][0], NodePosition[PairQ[0][0]][1] + 0.05);

		glColor3f(0.0, 0.0, 1.0);
		//glVertex2f(x1*scale_x-dx, y1*scale_y-dy);
		glVertex2f(NodePosition[PairQ[0][1]][0], NodePosition[PairQ[0][1]][1] + 0.05);
		glEnd();
		edgeCoordinator[PairQ[0][0]] = edgeCoordinator[PairQ[0][1]] = 1;
	}

	//count=app->GUIDataManager()->branchNum;
	//fprintf(fp,"count=%d\n",count);

	for (int i = 1; i<KK; i++)
		//for(vector< vector <int> >::iterator it=++PairQ.begin(); it!=PairQ.end(); it++)
	{
		//vector <int> point=*it;
		int point0, point1, point3;
		point0 = PairQ[i][0];
		point1 = PairQ[i][1];
		//point[2]=PairQ[i][2];
		point3 = PairQ[i][3];

		x0 = NodePosition[point0][0], y0 = NodePosition[point0][1];
		x1 = NodePosition[point1][0], y1 = NodePosition[point1][1];

		//fprintf(fp,"%f,%f,%f,%f\n",x0,y0,x1,y1);


		//app->GUIDataManager()->edgeCoordinator[point0]=app->GUIDataManager()->edgeCoordinator[point1]=0;

		//if(ContTree[PairQ[point[3]][1]].Pro[0]==0)
		if (ContTree[PairQ[point3][1]].Pro[0] == 0)
		{
			ContTree[point1].Pro[0] = 0;
		}
		else ContTree[point1].Pro[0] = 1;

		if (ContTree[point0].Pro[0] == 2 && ContTree[point1].Pro[0] == 1)
		{
			ContTree[point1].Pro[0] = 0;

			//x0=NodePosition[point0][0]+center;
			//y0=NodePosition[point0][1];


			glPointSize(6);
			glDisable(GL_POINT_SMOOTH);
			glColor3f(0.0, 0.0, 0.0);
			glBegin(GL_POINTS);
			//glVertex2f(x0*scale_x-dx-0.001, y0*scale_y-dy);
			glVertex2f(x0, y0 + 0.05);
			glEnd();

			glPointSize(4);
			glColor3f(1.0, 0.0, 0.0);
			glBegin(GL_POINTS);
			//glVertex2f(x0*scale_x-dx-0.001, y0*scale_y-dy);
			glVertex2f(x0, y0 + 0.05);
			glEnd();

			//glColor3f(0.0, 0.0, 0.0);
			glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_LINES);
			//glVertex2f(x0*scale_x-dx-0.006, y0*scale_y-dy);
			//glVertex2f(x0*scale_x-dx+0.006, y0*scale_y-dy);
			glVertex2f(x0 - 0.006, y0 + 0.05);
			glVertex2f(x0 + 0.006, y0 + 0.05);
			glVertex2f(x0, y0 + 0.08);
			glVertex2f(x0, y0 + 0.02);

			//glVertex2f(x0*scale_x-dx, y0*scale_y-dy+0.03);
			//glVertex2f(x0*scale_x-dx, y0*scale_y-dy-0.03);
			glEnd();

		}
		else if (ContTree[point1].Pro[0] == 1)
		{
			//x0=x0+center;
			//y0=y0;

			//x1=x1+center;
			//y1=y1;

			edgeCoordinator[point0] = edgeCoordinator[point1] = 1;

			//glColor3f(0.0, 0.0, 0.0);
			glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_LINES);

			//glVertex2f(x0*scale_x-dx, y0*scale_y-dy);
			glVertex2f(x0, y0 + 0.05);
			//glVertex2f(x1*scale_x-dx, y0*scale_y-dy);
			glVertex2f(x1, y0 + 0.05);

			//glVertex2f(x1*scale_x-dx, y0*scale_y-dy);
			glVertex2f(x1, y0 + 0.05);
			//glVertex2f(x1*scale_x-dx, y1*scale_y-dy);
			glVertex2f(x1, y1 + 0.05);
			glEnd();


			glColor3f(1.0, 0.0, 0.0);
			glPointSize(3);

			glDisable(GL_POINT_SMOOTH);
			glBegin(GL_POINTS);
			//glVertex2f(x0*scale_x-dx, y0*scale_y-dy);
			glVertex2f(x0, y0 + 0.05);
			glEnd();

			glEnable(GL_POINT_SMOOTH);
			glBegin(GL_POINTS);
			if (MSCpoint[point1].type == 1)
				glColor3f(0.0, 0.0, 1.0);
			else glColor3f(0.0, 1.0, 0.0);
			glVertex2f(x1, y1 + 0.05); //glVertex2f(x1*scale_x-dx, y1*scale_y-dy);
			glEnd();
		}
	}
}

void display(){

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	MyGraphicsTool::SetViewport(MyVec4i(windowWidth / 2, 0, windowWidth / 2, windowHeight));
	MyGraphicsTool::PushProjectionMatrix();
	MyGraphicsTool::PushMatrix();
	MyGraphicsTool::LoadProjectionMatrix(&MyMatrixf::OrthographicMatrix(-1, 1, -1, 1, 1, 10));
	MyGraphicsTool::LoadModelViewMatrix(&MyMatrixf::IdentityMatrix());
	glTranslatef(0, 0, -5);
	DrawContourTree();
	MyGraphicsTool::PopMatrix();
	MyGraphicsTool::PopProjectionMatrix();

	//renWin->Render();
	//ren->GetActiveCamera()->Azimuth(1);

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

	//renWin->SetSize(w, h);
}


void mouseKey(int button, int state, int x, int y){
	//renWin->GetInteractor()->SetEventInformation(x, y);
	//if (state == GLUT_DOWN){
	//	renWin->GetInteractor()->LeftButtonPressEvent();
	//}
	//else if (state == GLUT_UP){
	//	renWin->GetInteractor()->LeftButtonReleaseEvent();
	//}
	glutPostRedisplay();
}

void mouseMove(int x, int y){
	//renWin->GetInteractor()->SetEventInformation(x, y);
	//renWin->GetInteractor()->MouseMoveEvent();
	glutPostRedisplay();
}

void key(unsigned char c, int x, int y){
}

int main(int argc, char* argv[]){


	glutInit(&argc, argv);
	glutInitWindowSize(windowWidth, windowHeight);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	glutCreateWindow("Graph Explorer");
	//glewInit();

	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutMouseFunc(mouseKey);
	glutMotionFunc(mouseMove);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);

	//ren = vtkSmartPointer<vtkRenderer>::New();
	//renWin = vtkSmartPointer<vtkRenderWindow>::New();
	//vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();

	//iren->SetRenderWindow(renWin);

	//renWin->AddRenderer(ren);

	// Here is the trick: we ask the RenderWindow to join the current OpenGL context created by GLUT
	//renWin->InitializeFromCurrentContext();


	cout << "Building Contour Tree ..." << endl;
	BuildContourTree();
	//float isoValue = 0.8*(fMax - fMin) + fMin;
	//extractISeeds(isoValue);
	//VTKContour(isoValue, iseeds, iseedNum, 1, 0);
	//renWin->Render();
	//iren->Start();

	glutMainLoop();
	return 1;
}