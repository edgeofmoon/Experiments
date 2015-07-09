#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
using namespace std;

bool gluInvertMatrix(const double m[16], double invOut[16])
{
    double inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

void gluTransposeMatrix(const double inm[16], double n[16]){
	double m[16];
	memcpy(m, inm, 16*sizeof(double));
	n[0] = m[0];
        n[1] = m[4];
        n[2] = m[8];
        n[3] = m[12];
        n[4] = m[1];
        n[5] = m[5];
        n[6] = m[9];
        n[7] = m[13];
        n[8] = m[2];
        n[9] = m[6];
        n[10] = m[10];
        n[11] = m[14];
        n[12] = m[3];
        n[13] = m[7];
        n[14] = m[11];
        n[15] = m[15];
}

// m is row major
// v and outv is vertical
void matrixMultiplyVector(const double m[16], const double v[4], double outv[4]){
	for(int i = 0;i<4;i++){
		double sum = 0;
		for(int j = 0;j<4;j++){
			sum += m[i*4+j]*v[j];
		}
		outv[i] = sum;
	}	
}

int main(int argc, char* argv[]){
	if(argc<3){
		cout << "Usage: "<< argv[0] << " <matrixFile> <gradientFile> [outputFile].\n";
		return 0;
	}
	// read in the matrix
	ifstream matrixFile(argv[1]);
	if(!matrixFile.is_open()){
		cout << "Cannot open matrix file: " << argv[1] << endl;
		return 0;
	}
	double mat[16];
	for(int i = 0;i<16;i++){
		matrixFile >> mat[i];
	}
	matrixFile.close();

	// compute inverse matrix
	double invMat[16];	
	gluInvertMatrix(mat, invMat);

	// read in gradient table
	ifstream gradientFile(argv[2]);
	if(!gradientFile.is_open()){
		cout << "Cannot open gradient file: " << argv[2] << endl;
		return 0;
	}
	vector< vector<double> > gradients;	
	while(!gradientFile.eof()){
		string lineString;
		if(!getline(gradientFile, lineString)){
			break;
		}
		istringstream ss(lineString);
		vector<double> line;
		while(ss){
			string numberString;
			if(!getline(ss, numberString, ',')){
				break;
			}
			line.push_back(atof(numberString.c_str()));
		}
		if(line.size()>=3){
			gradients.push_back(line);
		}
	}
	gradientFile.close();

	// compute new gradient vectors
	vector< vector<double> > transformedGradients = gradients;
	for(unsigned int i = 0;i<gradients.size();i++){
		double grad[4];
		grad[0] = gradients[i][0];
		grad[1] = gradients[i][1];
		grad[2] = gradients[i][2];
		grad[3] = 0.0;
		double outgrad[4];
		matrixMultiplyVector(invMat, grad, outgrad);
		transformedGradients[i][0] = outgrad[0];	
		transformedGradients[i][1] = outgrad[1];	
		transformedGradients[i][2] = outgrad[2];
	}

	// debug
	cout << "gradients:" << endl;
	for(unsigned int i = 0;i<gradients.size();i++){
		for(unsigned int j = 0;j<gradients[i].size();j++){
			cout << gradients[i][j] << ", ";
		}
		cout << endl;
	}
	cout << "transformedGradients:" << endl;
	for(unsigned int i = 0;i<transformedGradients.size();i++){
		for(unsigned int j = 0;j<transformedGradients[i].size();j++){
			cout << transformedGradients[i][j] << ", ";
		}
		cout << endl;
	}

	// write transformed gradients
	string outname = "transformedGradients";
	if(argc>3){
		outname = argv[3];
	}
	ofstream outfile(outname.c_str());
	for(unsigned int i = 0;i<transformedGradients.size();i++){
		for(unsigned int j = 0;j<transformedGradients[i].size()-1;j++){
			outfile << transformedGradients[i][j] << ", ";
		}
		outfile << transformedGradients[i].back() << endl;
	}
	
	return 1;
}
