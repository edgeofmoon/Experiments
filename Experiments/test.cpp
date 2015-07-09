#include <GL/glut.h>
#include "RicVolume.h"
#include "MyContourTree.h"

char labelVolumeFile[] = "JHU - WhiteMatter - labels - 1mm.nii";
char labelTableFile[] = "GOBS_look_up_table.txt";
char faVolumeFile[] = "C:\\Users\\GuohaoZhang\\Desktop\\tmpdata\\dti_fa_A1021_bet.nii";

MyContourTree* ct;

void Reshape(int width, int height)
{
}

void Draw()
{
}

int main(int argc, char **argv)
{

	ct = new MyContourTree(argc, argv);
	ct->LoadLabelVolume("JHU-WhiteMatter-labels-1mm.nii");
	ct->LoadLabelTable("GOBS_look_up_table.txt");



	// GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Test");
	glutReshapeFunc(Reshape);
	glutDisplayFunc(Draw);

	// Let's start the main GLUT rendering loop
	glutMainLoop();

	return EXIT_SUCCESS;
}