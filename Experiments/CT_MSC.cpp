#include <math.h>
#include "CT_MSC.h"
#include "helper.h"

bool compare_2(struct CriticalPoint first, struct CriticalPoint second)
{
		if (first.value > second.value) { return true; }
		else { return false; }
}
bool fccomp_2(vector<float> lhs, vector<float> rhs)  //sort increasingly
{
		for(int i=0;i<lhs.size()&&i<rhs.size();i++)
		{    
			if(lhs[i]>rhs[i]) { return true; }
			else if(lhs[i]<rhs[i]) { return false; }
		}
		printf("In the function fccomp_2, in ClusterStreamlineView.cpp, all the values of the two vectors being compared are the same. This is bad. Quitting now.");
		exit(-1);
}


void getBranchSep(struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<vector<int>>> MSCsep,  vector < vector <int> > PairQ, vector<vector<int>> MSCsepD, vector<vector<vector<int>>> &BranchSep)
{
	int id,saddle,end,sepid,pnum;	
	bool mark;
	BranchSep.resize(PairQ.size());
	for(int j=0;j<PairQ.size();j++)
	{
		if(j>=554)
			j=j;

		if(j>=555)
			j=j;

		if(j>=560)
			j=j;

		if(j>=565)
			j=j;

		if(j>=570)
			j=j;

		if(j>=575)
			j=j;

		if(j>=580)
			j=j;

		
		saddle=PairQ[j][0];
		end=PairQ[j][1];
		id=saddle;
		mark=false;
		pnum = 0;
		vector<int> SepIDs;
	   	if(MSCpoint[end].type == 2)
		{
			if(saddle == 0) 
			{ 
				mark = true; 
			}	
			while(ContTree[id].HighIn.size()>0)
			{       int id1=id;
					for(set<int>::iterator it=ContTree[id].HighIn.begin(); it!=ContTree[id].HighIn.end(); it++)
					{
							if((mark && ContTree[*it].Pro[1] == 0) || (!mark && ContTree[*it].Pro[1] == 1))	   	  
							{	
			  					    mark = true;
									// The code checks if either of the following two things match id.
									// I stuck them in local variables to make it more legible. --Julia Ford
									int checkIfMatchesId0 = MSCsepD[*ContTree[*it].LowInSep.begin()][0];
									int checkIfMatchesId1 = MSCsepD[*ContTree[*it].LowInSep.begin()][1];
									if(checkIfMatchesId0 == id || checkIfMatchesId1 == id)
									{ 
										sepid=*ContTree[*it].LowInSep.begin(); 
									}
									else
									{   
										sepid=*ContTree[*it].LowInSep.rbegin(); 
									}
									SepIDs.push_back(sepid);
									pnum += MSCsep[sepid].size();
									id = *it;	
									break;
							}
					}
					if(id1==id)
						break;
			}			
		}
		else
		{
		 	 while(ContTree[id].LowIn.size() > 0)
			 {
				    int id1=id;
					for(set<int>::iterator it = ContTree[id].LowIn.begin(); it != ContTree[id].LowIn.end(); it++)
					{
							if((mark && ContTree[*it].Pro[1] == 0) || (!mark && ContTree[*it].Pro[1] == 1))	   	  
							{	
									mark = true;
									// The code checks if either of the following two things match id.
									// I stuck them in local variables to make it more legible. --Julia Ford
									int checkIfMatchesId0 = MSCsepD[*ContTree[*it].HighInSep.begin()][0];
									int checkIfMatchesId1 = MSCsepD[*ContTree[*it].HighInSep.begin()][1];

									if(checkIfMatchesId0 == id || checkIfMatchesId1 == id)
									{
									 	sepid=*ContTree[*it].HighInSep.begin();								  
									}
									else
									{
										sepid=*ContTree[*it].HighInSep.rbegin();								  
									}	
									SepIDs.push_back(sepid);
									pnum+=MSCsep[sepid].size();
									id=*it;	
									break;
							}
					}
					if(id1==id)
						break;
			 }				
		}	
		pnum = 0;
		for(int i=0;i<SepIDs.size();i++)
		{
			
			sepid=SepIDs[i];
			if(MSCpoint[end].type==2)
			{
				// initilize iterator for the following for loop
				vector<vector<int>>::iterator it = MSCsep[sepid].begin();
				if(i<SepIDs.size()-1&&SepIDs.size()>1) { it++; }
				for(/*use iterator initilized above*/; it!=MSCsep[sepid].end(); it++) 
				{
					vector<int> point=*it;	
					//fwrite(&point[0], sizeof(int), 1, pspFile);
					//fwrite(&point[1], sizeof(int), 1, pspFile);
					//fwrite(&point[2], sizeof(int), 1, pspFile);
					BranchSep[i].push_back(point);
					//BranchSep[i][j]=point;
					pnum++;
				}
			}
	  	    else
		    {
			 	 // initilize iterator for the following for loop
				vector<vector<int>>::reverse_iterator it = MSCsep[sepid].rbegin();
				 if((i < (SepIDs.size() - 1)) && (SepIDs.size() > 1)) { it++; }
				 for(/*use iterator initilized above*/; it != MSCsep[sepid].rend(); it++) 
				 {
				 	  vector<int> point=*it;	
					  //fwrite(&point[0], sizeof(int), 1, pspFile);
					  //fwrite(&point[1], sizeof(int), 1, pspFile);
					  //fwrite(&point[2], sizeof(int), 1, pspFile);
					  BranchSep[i].push_back(point);
					  //BranchSep[i][j]=point;					  
					  pnum++;
				 }	
			}
		}
		SepIDs.clear();
	}	
}		

	/*fclose(pspFile);

	FILE *TestFile = fopen(fileName, "rb");	
	int pnum;
	struct BranchSep_str* BranchSep = new struct BranchSep_str [PairQ.size()];
	for(i = 0; i < PairQ.size(); i++) //PairQ[n].size = 5  //for major pair record only PairQ[n][0]  PairQ[n][1]
	{
		fread(&pnum, sizeof(int), 1, pspFile);	
		BranchSep[i].sep=imatrix(0, pnum+1,0, 3);
		BranchSep[i].sep[0][0]=pnum;
		for(int j = 1; j <= pnum; j++)
		{
			fread(BranchSep[i].sep[j], sizeof(int), 3, pspFile);
		}			 
	}
	fclose(TestFile);*/


//vector< vector <int> > ReDrawHierarchyContTree_2(struct Tree_str * &ContTree, struct MScripoint *MSCpoint, int  MSCriNum, vector< vector<int> > &pairs, vector<int> &recurid, vector< vector <int> > PairQ, int &Width, int &Height, int &Center, vector< vector <int> > &locationArray, vector< vector <int> > &branchLocationArray, float fMin, float fMax)
vector<vector<int>> ReDrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int  MSCriNum, vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, int branchNum, vector< vector <int> > &pairs, vector <int>  &recurid, vector<vector<int>> PairQ, int &Width, int &Height, int &Center, vector<vector<int>> &locationArray, vector<vector<int>> &branchLocationArray, float ***Vs, float fMin, float fMax, float ** &seeds, vector<vector<vector<int>>> &BrancheBox, vector< vector < vector <int> > > &BranchSep)
//int** ReDrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MScripoint *MSCpoint, int  MSCriNum, int ** &pairs, int *&recurid, int ** PairQ, int &Width, int &Height, int &Center, vector< vector <int> > &locationArray, vector< vector <int> > &branchLocationArray, float fMin, float fMax)
{
        vector<vector <int> > NodePosition_1;//= i2vector(MSCriNum,4);
	    if( branchNum<1 ) 
			return NodePosition_1;	    
		RecordHierarchyContTree_2(ContTree, MSCpoint, MSCriNum, branchNum, pairs, recurid, NodePosition_1, PairQ, Width, Height, Center, fMin, fMax);
        locationArray=LocateNodePosition(ContTree, MSCpoint, MSCriNum, branchNum, NodePosition_1, fMin, fMax);
        branchLocationArray= LocateBranchPosition(ContTree, PairQ, NodePosition_1, branchNum, Width, Center);
		BrancheBox = LocateBoxPosition(ContTree, PairQ,  NodePosition_1,  branchNum,  Width, Center, 10);
		//LocateBranchPoints(ContTree, PairQ, BranchSep, MSCsepD, Vs, branchNum, seeds, fMin);
	

		getBranchSep(ContTree, MSCpoint, MSCsep, PairQ, MSCsepD, BranchSep);

		return NodePosition_1;
}

// used in JoinSplitTree --> CT --> OnDraw
void  RemoveSinNode(struct JoinSplitTreeNode *Tree, int m)
{   //remove a node in a tree, the node has at most one up degree and at most one down degree
	//single linked

	int i,u,w;
	if(Tree[m].LowIn.size()==0)
	{
		if(Tree[m].HighIn.size()>0)
		{
			i = *Tree[m].HighIn.begin();
			Tree[i].LowIn.erase(m);	   
		}
	}
	else if(Tree[m].HighIn.size() == 0)
	{
		if(Tree[m].LowIn.size()>0)
		{
			i = *Tree[m].LowIn.begin();
			Tree[i].HighIn.erase(m);	
		}
	}
	else if(Tree[m].HighIn.size()==1&&Tree[m].LowIn.size()==1)
	{
		if(Tree[m].LowIn.size()>0 && Tree[m].HighIn.size()>0)
		{
			u = *Tree[m].HighIn.begin();
			w = *Tree[m].LowIn.begin(); 
			Tree[u].LowIn.erase(m);
			Tree[w].HighIn.erase(m);	
			Tree[u].LowIn.insert(w);
			Tree[w].HighIn.insert(u);
		}
	}
	Tree[m].LowIn.clear();
	Tree[m].HighIn.clear();
}

// used in CT_2 --> ProcessDataMembers --> OnDraw
void SolveDegeneration(float ***Values, float &fMin, float &fMax, int I, int J, int K, bool is_3D)
{
		int sign[18],minm,num;  //type, 0: regular; 1,2: minima, maxima, 3, 4: saddle, monkey saddle, 5: unknown;
		float check[18],temp;
		int v;

		if(is_3D) { num = 18; }
		else { num = 6; }

		for(int k=0;k<K;k++)
			for(int i=0;i<I;i++)
				for(int j=0;j<J;j++)
				{
					bool flag = false;

					for(int l=0;l<num;l++) {
						check[l] = 0; }

					if(is_3D)
					{
						v=k*I*J+i*J+j+1;

						//x,y plan
						sign[0]=i==0?0:((check[0]=Values[i-1][j][k]-Values[i][j][k])==0?-1:1);
						sign[1]=j==J-1?0:((check[1]=Values[i][j+1][k]-Values[i][j][k])==0?-1:1);
						sign[2]=(i==I-1||j==J-1)?0:((check[2]=Values[i+1][j+1][k]-Values[i][j][k])==0?-1:1);
						sign[3]=i==I-1?0:((check[3]=Values[i+1][j][k]-Values[i][j][k])==0?-1:1);
						sign[4]=j==0?0:(check[4]=Values[i][j-1][k]-Values[i][j][k])==0?-1:1;
						sign[5]=(i==0||j==0)?0:(check[5]=Values[i-1][j-1][k]-Values[i][j][k])==0?-1:1;	

						//y,z plan
						sign[6]=j==J-1?0:((check[6]=Values[i][j+1][k]-Values[i][j][k])==0?-1:1);
						sign[7]=(j==J-1||k==K-1)?0:((check[7]=Values[i][j+1][k+1]-Values[i][j][k])==0?-1:1);
						sign[8]=k==K-1?0:((check[8]=Values[i][j][k+1]-Values[i][j][k])==0?-1:1);
						sign[9]=j==0?0:((check[9]=Values[i][j-1][k]-Values[i][j][k])==0?-1:1);
						sign[10]=(j==0||k==0)?0:(check[10]=Values[i][j-1][k-1]-Values[i][j][k])==0?-1:1;
						sign[11]=k==0?0:(check[11]=Values[i][j][k-1]-Values[i][j][k])==0?-1:1;

						//x,z plan
						sign[12]=i==0?0:((check[12]=Values[i-1][j][k]-Values[i][j][k])==0?-1:1);
						sign[13]=k==K-1?0:((check[13]=Values[i][j][k+1]-Values[i][j][k])==0?-1:1);
						sign[14]=(i==I-1||k==K-1)?0:((check[14]=Values[i+1][j][k+1]-Values[i][j][k])==0?-1:1);
						sign[15]=i==I-1?0:((check[15]=Values[i+1][j][k]-Values[i][j][k])==0?-1:1);
						sign[16]=k==0?0:(check[16]=Values[i][j][k-1]-Values[i][j][k])==0?-1:1;
						sign[17]=(i==0||k==0)?0:(check[17]=Values[i-1][j][k-1]-Values[i][j][k])==0?-1:1;	
					}
					else
					{			
						sign[0]=i==0?0:((check[0]=Values[i-1][j][0]-Values[i][j][0])==0?-1:1);
						sign[1]=j==J-1?0:((check[1]=Values[i][j+1][0]-Values[i][j][0])==0?-1:1);
						sign[2]=(i==I-1||j==J-1)?0:((check[2]=Values[i+1][j+1][0]-Values[i][j][0])==0?-1:1);
						sign[3]=i==I-1?0:((check[3]=Values[i+1][j][0]-Values[i][j][0])==0?-1:1);
						sign[4]=j==0?0:(check[4]=Values[i][j-1][0]-Values[i][j][0])==0?-1:1;
						sign[5]=(i==0||j==0)?0:(check[5]=Values[i-1][j-1][0]-Values[i][j][0])==0?-1:1;	
					}

					for(int l=0;l<num;l++)
					{
						if(sign[l]==-1)
						{
							if(Values[i][j][k]!=0&&Values[i][j][k]>-9999.000)
							{
								temp=10000000;
								minm=0;
								for(int m=0;m<num;m++)
								{
									if(check[m]!=0.0)
										if(temp>fabs(check[m]))
										{
											temp = fabs(check[m]);	
											minm = m;						
										}
								}
								Values[i][j][k] += check[minm] / 2.0f;
								break;	
							}
						}
					}
				}
				fMin=100000000;
				fMax=-100000000;  
				for(int k=0;k<K;k++) {
					for(int i=0;i<I;i++) {
						for(int j=0;j<J;j++)
						{
							if(Values[i][j][k]==0.0||Values[i][j][k]<-9999.000)
								Values[i][j][k]=-9999.0000;
							else 
							{
								if(fMin>Values[i][j][k]) fMin=Values[i][j][k];
								if(fMax<Values[i][j][k]) fMax=Values[i][j][k];  
							}
						}
					}
				}
	}

int FindLow_Highest_2(struct ContourTreeNode *Tree, int id, bool flag)
	{
		if(flag)
		{
			while(Tree[id].LowIn.size()!=0&&Tree[id].LowIn.size()!=2)
			{
				id=*Tree[id].LowIn.begin();
			}	   		  
		}
		else
		{
			while(Tree[id].HighIn.size()!=0&&Tree[id].HighIn.size()!=2)
			{
				id=*Tree[id].HighIn.begin();		
			}	 	        		
		}	
		return id;	
	}

int FindLow_Highest(JoinSplitTreeNode *Tree, int id, bool flag)
	{
		if(flag)
		{
			while(Tree[id].LowIn.size()!=0)
			{
				id=*Tree[id].LowIn.begin();
			}	   		  
		}
		else
		{
			while(Tree[id].HighIn.size()!=0)
			{
				id=*Tree[id].HighIn.begin();		
			}	 	        		
		}	
		return id;	
	}

	
void  JoinSplitTree(struct ContourTreeNode * &ContTree, struct JoinSplitTreeNode * &JoinTree, struct JoinSplitTreeNode * &SplitTree, float ***Values, int I, int J, int K, bool flag, bool Is_Members, bool is_3D)
{//flag=1 record edge span
		list<struct CriticalPoint> criticalPointList; 
		//bool is_3D;
        vector<float> IsoValues;	
        vector<vector<int>> IsoEdge;			// used in JoinSplitTree
		//union find
		vector<int> Union, UnionNode;
		

		struct CriticalPoint current;	   	   
		int i,j,k,u,v,n,m,r,test,test2;

		UnionNode.push_back(0);
		Union.push_back(0);
		for(k = 0; k < K; k++) {
			for(i = 0; i < I; i++) {
				for(j = 0; j < J; j++) {
					current.C1 = i;
					current.C2 = j;
					current.id = k;
					current.value = Values[i][j][k];
					criticalPointList.push_back(current);	
					v = k*I*J + i*J + j + 1;
					Union.push_back(v);
					UnionNode.push_back(v);
				}
			}
		}
		int count = 0;
		//1. build join tree
		criticalPointList.sort(compare_2); //descending;
		JoinTree = new JoinSplitTreeNode[K*I*J + 1];   
		for(list<CriticalPoint>::iterator it = criticalPointList.begin(); it!=criticalPointList.end(); it++) {
			current = *it;
			if(current.value == 0) { current.value = current.value; }
			else { current.value=current.value; }
		} // end for

		for(list<struct CriticalPoint>::iterator it=criticalPointList.begin(); it != criticalPointList.end(); it++) {
			count++;
			current = *it;
			i = current.C1;
			j = current.C2;
			k = current.id;	
			if(current.value != -9999.0000)
			{
				//DrawPoint(float(i*DX), float(j*DY + 300), RGB(0,0,0));
				vector<int> node;
				if(is_3D)
				{
					v=k*I*J+i*J+j+1;

					//x,y plan
					node.push_back(i-1); node.push_back(j);  node.push_back(k);
					node.push_back(i);  node.push_back(j+1); node.push_back(k);
					node.push_back(i+1); node.push_back(j+1); node.push_back(k);
					node.push_back(i+1); node.push_back(j); node.push_back(k);
					node.push_back(i); node.push_back(j-1); node.push_back(k);
					node.push_back(i-1); node.push_back(j-1); node.push_back(k);	

					//y,z plan
					node.push_back(i); node.push_back(j+1); node.push_back(k);
					node.push_back(i); node.push_back(j+1); node.push_back(k+1);
					node.push_back(i); node.push_back(j); node.push_back(k+1);
					node.push_back(i); node.push_back(j-1); node.push_back(k);
					node.push_back(i); node.push_back(j-1); node.push_back(k-1);
					node.push_back(i); node.push_back(j); node.push_back(k-1);

					//x,z plan
					node.push_back(i-1); node.push_back(j); node.push_back(k);
					node.push_back(i); node.push_back(j); node.push_back(k+1);
					node.push_back(i+1); node.push_back(j); node.push_back(k+1);
					node.push_back(i+1); node.push_back(j); node.push_back(k);
					node.push_back(i); node.push_back(j); node.push_back(k-1);
					node.push_back(i-1); node.push_back(j); node.push_back(k-1);	

					for(int k=0;k<54;k=k+3)
					{
						if(node[k]>=0&&node[k]<I&&node[k+1]>=0&&node[k+1]<J&&node[k+2]>=0&&node[k+2]<K)
						{
							if(Values[node[k]][node[k+1]][node[k+2]] != -9999.0000) {
								if(Values[node[k]][node[k+1]][node[k+2]] >= current.value) {
									n = node[k+2]*J*I + node[k]*J + node[k+1] + 1;

									test  = Union[n];
									test2 = UnionNode[test];
									while(JoinTree[test2].HighIn.size() > 1 && JoinTree[test2].LowIn.size() > 0) {
										test  = Union[*JoinTree[test2].LowIn.begin()];
										test2 = UnionNode[test];					
									}
									UnionNode[test] = v;
									Union[v] = test;

									u = test2;

									if(u != v) {
										int size = JoinTree[v].HighIn.size();
										if(size > 1) {
											int V = v;

											do {
												V = *JoinTree[V].HighIn.rbegin();
												size = JoinTree[V].HighIn.size(); }
											while(size > 1);			
											JoinTree[u].LowIn.insert(V);
											JoinTree[V].HighIn.insert(u);
										}
										else {
											JoinTree[u].LowIn.insert(v);
											JoinTree[v].HighIn.insert(u); }
									}

									if(flag) {
										for(unsigned int l = 0; l < IsoValues.size(); l++) {
											if(current.value <= IsoValues[l] && Values[node[k]][node[k+1]][node[k+2]] >= IsoValues[l]) {
												IsoEdge[l].push_back(v);
												IsoEdge[l].push_back(n); }
										} // end for
									} // end if(flag)
								}
							}
						}			
					}
				}
				else {
					v = i*J + j + 1;		

					node.push_back(i-1); node.push_back(j);
					node.push_back(i);   node.push_back(j+1);
					node.push_back(i+1); node.push_back(j+1);
					node.push_back(i+1); node.push_back(j);
					node.push_back(i);   node.push_back(j-1);
					node.push_back(i-1); node.push_back(j-1);

					for(int k = 0; k < 12; k += 2) {
						if(node[k]>=0&&node[k]<I&&node[k+1]>=0&&node[k+1]<J) {
							if(Values[node[k]][node[k+1]][0] != -9999.0000) 
								if(Values[node[k]][node[k+1]][0] >= current.value) {
									n = node[k]*J+node[k+1]+1;
									u = FindLow_Highest(JoinTree,n,1);
									if(u != v) {
										if(JoinTree[v].HighIn.size() > 1) {
											int size = JoinTree[v].HighIn.size();
											int V = v;
											do {
												V =   *JoinTree[V].HighIn.rbegin();
												size = JoinTree[V].HighIn.size(); }
											while(size > 1);			
											JoinTree[u].LowIn.insert(V);
											JoinTree[V].HighIn.insert(u);
										} // end if(JoinTree[v].HighIn.size() > 1)

										else {
											JoinTree[u].LowIn.insert(v);
											JoinTree[v].HighIn.insert(u); }
									} // end if(u != v)

									if(flag) {
										for(unsigned int l = 0; l < IsoValues.size(); l++) {
											if(current.value <= IsoValues[l] && Values[node[k]][node[k+1]][0] >= IsoValues[l]) {
												IsoEdge[l].push_back(v);
												IsoEdge[l].push_back(n); }
										} // end for
									} // end if(flag)
								}
						}			
					}
				}
			}
		}
		//virtual minimum
		u=FindLow_Highest(JoinTree,v,1);
		JoinTree[u].LowIn.insert(0);
		JoinTree[0].HighIn.insert(u);

		Union.clear();
		UnionNode.clear();
		UnionNode.push_back(0);
		Union.push_back(0);
		for(k=0;k<K;k++) {
			for(i=0;i<I;i++) {
				for(j=0;j<J;j++)
				{
					v=k*I*J+i*J+j+1;
					Union.push_back(v);
					UnionNode.push_back(v);
				}
			}
		}

		//2. build split tree  
		SplitTree=new struct JoinSplitTreeNode[K*I*J+1];
		int highest = 0;
		count = 0;
		for(list<struct CriticalPoint>::reverse_iterator rit=criticalPointList.rbegin(); rit!=criticalPointList.rend(); rit++ )
		{  
			current = *rit;
			i = current.C1;
			j = current.C2;
			k = current.id;	
			if(current.value != -9999.0000)
			{
				//DrawPoint(float(i*DX), float(j*DY + 300.0f), RGB(155,155,155));
				vector<int> node;

				count++;

				if(is_3D) // global variable
				{
					v=k*I*J+i*J+j+1;
					//x,y plan
					node.push_back(i-1); node.push_back(j);   node.push_back(k);
					node.push_back(i);   node.push_back(j+1); node.push_back(k);
					node.push_back(i+1); node.push_back(j+1); node.push_back(k);
					node.push_back(i+1); node.push_back(j);   node.push_back(k);
					node.push_back(i);   node.push_back(j-1); node.push_back(k);
					node.push_back(i-1); node.push_back(j-1); node.push_back(k);	

					//y,z plan
					node.push_back(i); node.push_back(j+1); node.push_back(k);
					node.push_back(i); node.push_back(j+1); node.push_back(k+1);
					node.push_back(i); node.push_back(j);   node.push_back(k+1);
					node.push_back(i); node.push_back(j-1); node.push_back(k);
					node.push_back(i); node.push_back(j-1); node.push_back(k-1);
					node.push_back(i); node.push_back(j);   node.push_back(k-1);

					//x,z plan
					node.push_back(i-1); node.push_back(j); node.push_back(k);
					node.push_back(i);   node.push_back(j); node.push_back(k+1);
					node.push_back(i+1); node.push_back(j); node.push_back(k+1);
					node.push_back(i+1); node.push_back(j); node.push_back(k);
					node.push_back(i);   node.push_back(j); node.push_back(k-1);
					node.push_back(i-1); node.push_back(j); node.push_back(k-1);	

					for(int k = 0; k < 54; k += 3)
					{
						if(node[k] >= 0 && node[k] < I && node[k + 1] >= 0 && node[k + 1] < J && node[k + 2] >= 0 && node[k + 2] < K) {
							if(Values[node[k]][node[k+1]][node[k+2]] != -9999.0000) {
								if(Values[node[k]][node[k + 1]][node[k + 2]] < current.value) {
									n = node[k + 2]*I*J + node[k]*J + node[k + 1] + 1;

									test  = Union[n];
									test2 = UnionNode[test];
									while(SplitTree[test2].HighIn.size() > 0) {
										test  = Union[*SplitTree[test2].HighIn.begin()];
										test2 = UnionNode[test]; }
									UnionNode[test] = v;
									Union[v] = test;

									u = test2;
									UnionNode[test] = v;
									Union[v] = test; 

									if(u != v) {
										int size=SplitTree[v].LowIn.size();
										if(SplitTree[v].LowIn.size() > 1 || JoinTree[v].HighIn.size() + SplitTree[v].LowIn.size() > 2)
										{
											int size1 = SplitTree[v].LowIn.size();
											int size2 = JoinTree[v].HighIn.size();
											int V = v;
											do {
												V = *SplitTree[V].LowIn.rbegin();
												size1 = SplitTree[V].LowIn.size(); 
												size2 = JoinTree[V].HighIn.size(); }
											while(size1 > 1 || size1 + size2 > 2);

											SplitTree[u].HighIn.insert(V);
											SplitTree[V].LowIn.insert(u); }
										else {
											SplitTree[u].HighIn.insert(v);
											SplitTree[v].LowIn.insert(u); }
									}

									if(flag) {
										for(unsigned int l = 0; l < IsoValues.size(); l++) {
											if(current.value >= IsoValues[l] && Values[node[k]][node[k+1]][node[k+2]] <= IsoValues[l]) {
												IsoEdge[l].push_back(n);
												IsoEdge[l].push_back(v); }
										} // end for
									} // end if(flag)
								} // end if
							} // end if
						} // end if
						else //reach virtual minimum
						{
							test  = Union[highest];
							test2 = UnionNode[test];
							while(SplitTree[test2].HighIn.size() > 0)
							{
								test  = Union[*SplitTree[test2].HighIn.begin()];
								test2 = UnionNode[test];					
							}
							UnionNode[test] = v;
							Union[v] = test;

							u = test2;

							highest = u;
							if(u != v) {
								SplitTree[u].HighIn.insert(v);
								SplitTree[v].LowIn.insert(u); }			
						}
					}			
				}
				else
				{
					v=i*J+j+1;      				
					node.push_back(i-1); node.push_back(j);
					node.push_back(i);   node.push_back(j+1);
					node.push_back(i+1); node.push_back(j+1);
					node.push_back(i+1); node.push_back(j);
					node.push_back(i);   node.push_back(j-1);
					node.push_back(i-1); node.push_back(j-1);

					for(int k=0;k<12;k=k+2)
					{
						if(node[k]>=0&&node[k]<I&&node[k+1]>=0&&node[k+1]<J)
						{
							if(Values[node[k]][node[k+1]][0]!=-9999.0000) 
								if(Values[node[k]][node[k + 1]][0]<current.value) {
									n = node[k]*J + node[k + 1] + 1;
									u = FindLow_Highest(SplitTree, n, false);

									if(u != v) {
										if(SplitTree[v].LowIn.size() > 1 || JoinTree[v].HighIn.size() + SplitTree[v].LowIn.size() > 2) {
											int size1 = SplitTree[v].LowIn.size();
											int size2 = JoinTree[v].HighIn.size();
											int V = v;
											do {
												V = *SplitTree[V].LowIn.rbegin();
												size1=SplitTree[V].LowIn.size(); 
												size2=JoinTree[V].HighIn.size(); }
											while(size1 > 1 || size1 + size2 > 2);

											SplitTree[u].HighIn.insert(V);
											SplitTree[V].LowIn.insert(u);
										} // end if
										else {
											SplitTree[u].HighIn.insert(v);
											SplitTree[v].LowIn.insert(u); }
									} // end if(u != v)

									if(flag) {
										for(unsigned int l = 0; l < IsoValues.size(); l++) {
											if(current.value >= IsoValues[l] && Values[node[k]][node[k+1]][0] <= IsoValues[l]) {
												IsoEdge[l].push_back(n);
												IsoEdge[l].push_back(v); }
										} // end for
									} // end if(flag)
								}
						}	
						else //reach virtual minimum
						{
							u=FindLow_Highest(SplitTree,highest,0);
							highest=u;
							if(u!=v)
							{ 
								SplitTree[u].HighIn.insert(v);
								SplitTree[v].LowIn.insert(u);
							}			
						}
					}
				}
			}
		}
		criticalPointList.clear(); 
		Union.clear();
		UnionNode.clear();

		//3. merge join and Join1 tree
		ContTree = new struct ContourTreeNode[K*I*J+1]; 
		/*float v1=ContTree[11895].value;
		int lsize1=ContTree[11895].LowIn.size();
		float v2=ContTree[11896].value;
		int lsize2=ContTree[11896].LowIn.size();*/

		for(i=0;i<K*I*J+1;i++)  
		{
			vector<int> Pro(6,-1);	   
			ContTree[i].Pro=Pro;
		}

		for(k=0;k<K;k++) {
			for(i=0;i<I;i++) {
				for(j=0;j<J;j++)
				{
					if(is_3D) { ContTree[k*I*J+i*J+j+1].value = Values[i][j][k]; }
					else { ContTree[i*J+j+1].value = Values[i][j][k]; }
				}
			}
		}
		ContTree[0].value=MinFloat;

		list<int> Q;
		vector<int> Saddle;

		for(k=0;k<K;k++) {
			for(i=0;i<I;i++) {
				for(j=0;j<J;j++)
				{	
					m=k*I*J+i*J+j+1;
					if(SplitTree[m].LowIn.size()==0&&JoinTree[m].HighIn.size()==1)  
					{
						if(m==46646)
					           m=m;
						Q.push_back(m);		
					}
					else if(SplitTree[m].LowIn.size()==1&&JoinTree[m].HighIn.size()==0)
					{
						if(m==46646)
					           m=m;
						Q.push_back(m);		
					}		
				}
			}
		}

		//virtual minimum
		m=0;
		if((SplitTree[m].LowIn.size()==0&&JoinTree[m].HighIn.size()==1)||(SplitTree[m].LowIn.size()==1&&JoinTree[m].HighIn.size()==0))  
		{
			Q.push_back(m);		
		}

		int testid = FindLow_Highest(SplitTree, 0, false);
        set<int> testID;
		testid=0;
		while(SplitTree[testid].HighIn.size()!=0)
		{
			testid=*SplitTree[testid].HighIn.begin();		
			testID.insert(testid);
		}	 

		int mark;// mark saddles
		
		while(Q.size()!=1)
		{
			r=-1;

			m=*Q.begin();
          
			if(testID.find(m)!=testID.end())
				m=m;

			if(m==234)
				m=234;

			if(SplitTree[m].LowIn.size()+ContTree[m].LowIn.size()>1||JoinTree[m].HighIn.size()+ContTree[m].HighIn.size()>1)
				mark=m;
			else if(SplitTree[m].LowIn.size()+ContTree[m].LowIn.size()==0||JoinTree[m].HighIn.size()+ContTree[m].HighIn.size()==0)
				mark=-1;

			if(SplitTree[m].LowIn.size()==0&&JoinTree[m].HighIn.size()==1 && SplitTree[m].HighIn.size()!=0)
			{		
				r=*SplitTree[m].HighIn.begin();

				if(r>40*40*40)
					r=r;
				
				ContTree[m].HighIn.insert(r); 

				int hr=ContTree[r].HighIn.size()>0?ContTree[r].Pro[4]:r;
				int lm=ContTree[m].LowIn.size()>0?ContTree[m].Pro[0]:(ContTree[m].Pro[0]=m);
				if(lm<0)
					lm=lm;

				if(mark>=0)
				{
					if(SplitTree[r].HighIn.size()==0) //extrema
					{
						if(ContTree[mark].HighIn.size()>1)
						{
							if(ContTree[ContTree[mark].Pro[4]].value<ContTree[r].value)
							{
								ContTree[mark].Pro[2]=ContTree[mark].Pro[4];
								ContTree[mark].Pro[4]=r;
							}
							else ContTree[mark].Pro[2]=r;
						}
						else ContTree[mark].Pro[4]=r;
					}
					else if(SplitTree[r].LowIn.size()+ContTree[r].LowIn.size()>1||JoinTree[r].HighIn.size()+ContTree[r].HighIn.size()>1) //saddle 
					{
						if(ContTree[mark].HighIn.size()>1)
						{
							if(ContTree[ContTree[mark].Pro[4]].value<ContTree[r].value)
							{
								ContTree[mark].Pro[2]=ContTree[mark].Pro[4];
								ContTree[mark].Pro[4]=r;
							}
							else { ContTree[mark].Pro[2]=r;	}			
						}
						else { ContTree[mark].Pro[4]=r; }
					}
				}
			
				ContTree[r].LowIn.insert(m); 			

				if(ContTree[r].HighIn.size()+ContTree[r].LowIn.size()>=3) 			
				{				
					int lowConnect=FindLow_Highest_2(ContTree,m,true);
					if(ContTree[r].LowIn.size()<1) { ContTree[lowConnect].Pro[4]=r; }
					else { ContTree[lowConnect].Pro[4]=ContTree[r].Pro[4]; }               
				}

				if(ContTree[m].HighIn.size()==1)
				{
					ContTree[m].Pro[4]=hr;
					ContTree[m].Pro[2]=-1;
				}
				else 
				{
					if(ContTree[ContTree[m].Pro[4]].value<ContTree[hr].value)
					{
						ContTree[m].Pro[2]=ContTree[m].Pro[4];
						ContTree[m].Pro[4]=hr;
					}
					else ContTree[m].Pro[2]=hr;			
				}
				if(ContTree[r].LowIn.size()==1)
				{
					ContTree[r].Pro[0]=lm;
					if(lm<0)
						lm=lm;
					ContTree[r].Pro[3]=-1;
				}
				else 
				{
					if(ContTree[ContTree[r].Pro[0]].value>ContTree[lm].value)
					{
						ContTree[r].Pro[3]=ContTree[r].Pro[0];
						ContTree[r].Pro[0]=lm;
						if(lm<0)
						lm=lm;
					}
					else ContTree[r].Pro[3]=lm;						
				}
				if(ContTree[m].HighIn.size()+ContTree[m].LowIn.size()>=3) 			
				{
					Saddle.push_back(m);		
				}            		    
			}
			else if( SplitTree[m].LowIn.size()==1 && JoinTree[m].HighIn.size()==0 && JoinTree[m].LowIn.size()!=0 )  
			{
				r = *JoinTree[m].LowIn.begin();

				if(r>40*40*40)
					r=r;

				ContTree[m].LowIn.insert(r);

				

				int lr, hm;
				lr=ContTree[r].LowIn.size()>0?ContTree[r].Pro[0]:r;
				hm=ContTree[m].HighIn.size()>0?ContTree[m].Pro[4]:(ContTree[m].Pro[4]=m);

				if(ContTree[r].HighIn.size()+ContTree[r].LowIn.size()>=3) 			
				{
					int highConnect = FindLow_Highest_2(ContTree,m,false);
					if(ContTree[r].LowIn.size()<1) 
					{ 
						ContTree[highConnect].Pro[0]=r; 
						if(r<0)
						   r=r;
					}
					else
					{ 
						ContTree[highConnect].Pro[0] = ContTree[r].Pro[0]; 
						if(ContTree[r].Pro[0]<0)
						   ContTree[r].Pro[0]=ContTree[r].Pro[0];
					}
				}


				if(mark>=0)
				{
					if(JoinTree[r].LowIn.size()==0)
					{
						if(ContTree[mark].LowIn.size()>1)
						{
							if(ContTree[ContTree[mark].Pro[0]].value>ContTree[r].value)
							{
								ContTree[mark].Pro[3]=ContTree[mark].Pro[0];
								ContTree[mark].Pro[0]=r;
								if(r<0)
						              r=r;
							}
							else { ContTree[mark].Pro[3]=r; }
						}
						else { 
							ContTree[mark].Pro[0]=r; 
						    if(r<0)
						      r=r;
						}
					}
					else if(SplitTree[r].LowIn.size()+ContTree[r].LowIn.size()>1||JoinTree[r].HighIn.size()+ContTree[r].HighIn.size()>1) //saddle 
					{
						if(ContTree[mark].LowIn.size()>1)
						{
							if(ContTree[ContTree[mark].Pro[0]].value>ContTree[r].value)
							{
								ContTree[mark].Pro[3]=ContTree[mark].Pro[0];
								ContTree[mark].Pro[0]=r;
								if(r<0)
						              r=r;
							}
							else { ContTree[mark].Pro[3]=r; }
						}
						else { ContTree[mark].Pro[0]=r;
						if(r<0)
						   r=r;
						}
					}	
				}
				ContTree[r].HighIn.insert(m); 

				if(ContTree[m].LowIn.size()==1)
				{
					
					ContTree[m].Pro[0]=lr;
					
					if(lr<0)
						   lr=lr;
					ContTree[m].Pro[3]=-1;
				}
				else 
				{
					if(ContTree[ContTree[m].Pro[0]].value>ContTree[lr].value)
					{
						ContTree[m].Pro[3]=ContTree[m].Pro[0];
						ContTree[m].Pro[0]=lr;
						if(lr<0)
						   lr=lr;
					}
					else ContTree[m].Pro[3]=lr;			
				}
				if(ContTree[r].HighIn.size()==1)
				{
					ContTree[r].Pro[4]=hm; // Pro[4] positionid, pro[2] upleaf 
					ContTree[r].Pro[2]=-1;
				}    
				else 
				{
					if(ContTree[ContTree[r].Pro[4]].value<ContTree[hm].value)
					{
						ContTree[r].Pro[2]=ContTree[r].Pro[4];
						ContTree[r].Pro[4]=hm;
					}
					else ContTree[r].Pro[2]=hm;			
				}			
				if(ContTree[m].HighIn.size() + ContTree[m].LowIn.size() >= 3) 
				{ 
					Saddle.push_back(m); 
				}			
			}
			else  
			{

			}
			//remove m,e in J		     
			RemoveSinNode(SplitTree,m);
			RemoveSinNode(JoinTree,m);	

			
			if(*Q.begin() == 234)			    
				m=m;


			Q.pop_front();

			if(r>=0)
			if((SplitTree[r].LowIn.size()==0&&JoinTree[r].HighIn.size()==1)||(SplitTree[r].LowIn.size()==1&&JoinTree[r].HighIn.size()==0))  
			{
				if(r<0)
					r=r;
				if(r==46646)
					r=r;
				Q.push_front(r);
			}
            //if(ContTree[46646].Pro[0]<-1)
			//		   m=m;
		}
		int s[4];
		for(int i=Saddle.size()-2; i>=0; i--)
		{
			s[0]=ContTree[Saddle[i]].Pro[0];
			s[1]=ContTree[Saddle[i]].Pro[3];
			s[2]=ContTree[Saddle[i]].Pro[4];
			s[3]=ContTree[Saddle[i]].Pro[2];

			for(int j=0;j<4;j++)
			{
				if(s[j] >= 0)
					if(ContTree[s[j]].HighIn.size()+ContTree[s[j]].LowIn.size() > 2) //saddle
					{
						if(j < 2) 
						{
							if(ContTree[Saddle[i]].LowIn.size()==1) { ContTree[Saddle[i]].Pro[0]=ContTree[s[j]].Pro[0]; }
							else 
							{
								if(ContTree[ContTree[Saddle[i]].Pro[0]].value>ContTree[ContTree[s[j]].Pro[0]].value)
								{
									ContTree[Saddle[i]].Pro[3]=ContTree[Saddle[i]].Pro[0];
									ContTree[Saddle[i]].Pro[0]=ContTree[s[j]].Pro[0];
									if(ContTree[s[j]].Pro[0]<0)
						               ContTree[s[j]].Pro[0]=ContTree[s[j]].Pro[0];
								}
								else { ContTree[Saddle[i]].Pro[3]=ContTree[s[j]].Pro[0]; }
							}
						}	
						else
						{
							if(ContTree[Saddle[i]].HighIn.size() == 1) { ContTree[Saddle[i]].Pro[4]=ContTree[s[j]].Pro[4]; } //positionid       				
							else 
							{
								if(ContTree[ContTree[Saddle[i]].Pro[4]].value<ContTree[ContTree[s[j]].Pro[4]].value)
								{
									ContTree[Saddle[i]].Pro[2]=ContTree[Saddle[i]].Pro[4];
									ContTree[Saddle[i]].Pro[4]=ContTree[s[j]].Pro[4];
								}
								else { ContTree[Saddle[i]].Pro[2]=ContTree[s[j]].Pro[4]; }
							}				
						}
						break;
					}
			}
		}

		delete [] JoinTree;
		delete [] SplitTree;
		
	}



// used in CT --> OnDraw
void  ReduceTree(struct ContourTreeNode *ContTree, int I, int J, int K) //reduce a full augmented tree to un
{
		int i,j,k,m,u,v;
		for(k=0;k<K;k++)
			for(i=0;i<I;i++)
				for(j=0;j<J;j++)
				{	
					m=k*I*J+i*J+j+1;
					if(ContTree[m].HighIn.size()==1&&ContTree[m].LowIn.size()==1)
					{
						u=*ContTree[m].HighIn.begin();
						v=*ContTree[m].LowIn.begin();

						ContTree[u].LowIn.insert(v);
						ContTree[u].LowIn.erase(m);

						ContTree[v].HighIn.insert(u);
						ContTree[v].HighIn.erase(m);

						ContTree[m].LowIn.clear();
						ContTree[m].HighIn.clear();
						ContTree[m].Pro.clear();
					}		
				}			
}

// used in CT, which is used in OnDraw
	void RecordCritical(struct ContourTreeNode * &ContTree, float ***Values, struct MorseSmaleCriticalPoint* &MSCpoint, int &MSCriNum, int I, int J, int K)
	{	int i,j,k,m,num;

	vector<int> i1,j1,k1,type1;
	vector<float> value1;

	//virtual minimum
	i1.push_back(-1);
	j1.push_back(-1);
	k1.push_back(-1);
	type1.push_back(1);
	value1.push_back(MinFloat);

	ContTree[0].Pro[2]=ContTree[0].Pro[3]=0;  //upLeaf and downLeaf
	ContTree[0].Pro[5]=0;

	MSCriNum = 1;

	for(k=0;k<K;k++)
		for(j=0;j<J;j++)
			for(i=0;i<I;i++)	
			{	
				m=k*I*J+i*J+j+1;	    
				num=ContTree[m].HighIn.size()+ContTree[m].LowIn.size();		
				if(num>3)
				{
					num=num;

				}
				if(num>0)
				{
					//MSCpoint[MSCriNum].i=i, MSCpoint[MSCriNum].j=j, MSCpoint[MSCriNum].k=k;			
					//MSCpoint[MSCriNum].value=Values[i][j][k];
					i1.push_back(i);  j1.push_back(j);  k1.push_back(k);  
					value1.push_back(Values[i][j][k]);

					if(num==1&&ContTree[m].HighIn.size())
						type1.push_back(1); //MSCpoint[MSCriNum].type=1;
					else if(num==1&&ContTree[m].LowIn.size()) 
						type1.push_back(2); //MSCpoint[MSCriNum].type=2;     
					else 
						type1.push_back(num); //MSCpoint[MSCriNum].type=num;

					ContTree[m].Pro[5]=MSCriNum;
					ContTree[m].Pro[2]=ContTree[m].Pro[3]=0;
					//ContTree[m].Pro[0]=1;

					MSCriNum++;

				}	
			}	

			MSCpoint=new struct MorseSmaleCriticalPoint[MSCriNum];
			for(int i=0; i<MSCriNum;i++)
				MSCpoint[i].i=i1[i], MSCpoint[i].j=j1[i],MSCpoint[i].k=k1[i],MSCpoint[i].type=type1[i],MSCpoint[i].value=value1[i];


			i1.clear(); j1.clear(); k1.clear(); type1.clear(); 
			value1.clear();
	}   

//=======================================================================//
// CT stuff                                                              //
//=======================================================================//
// used in OnDraw	

	// used in CT, which is used in OnDraw
	void RecordSuperArcs(struct ContourTreeNode *ContTree, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, int I, int J, int K, int is_3D)
	{
		int i,j,k,m;	
		if(ContTree[0].HighIn.size()>0)
		GetSuperArc(ContTree, MSCsep, MSCsepD, ContTree[0].HighIn.begin(), 0, I, J, K, 1, is_3D);
		for(k=0;k<K;k++) //keqin changed
		{
			for(i=0;i<I;i++)
			{
				for(j=0;j<J;j++)
				{	
					m=k*I*J+i*J+j+1;
					int hsize=ContTree[m].HighIn.size();	
					int lsize=ContTree[m].LowIn.size();	
					if(ContTree[m].LowIn.size()==0||(ContTree[m].HighIn.size()+ContTree[m].LowIn.size())>=3)
					{
						if(ContTree[m].HighIn.size()!=0)
						for(set<int>::iterator it=ContTree[m].HighIn.begin(); it!=ContTree[m].HighIn.end(); it++)
						{
							GetSuperArc(ContTree, MSCsep, MSCsepD, it, m, I, J, K, 1, is_3D);
						}
					}
				}
			}
		}
	}
	// used in CT, which is used in OnDraw
	void RecordSuperArcs(vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, struct ContourTreeNode *&ContTree) 
	{
		for(unsigned int m = 0; m < MSCsepD.size(); m++)	   
		{
			int start = MSCsepD[m][0];
			int end   = MSCsepD[m][1];
			int d     = MSCsepD[m][2];

			if(d > 0) {
				ContTree[start].HighInSep.insert(m);				
				ContTree[end].LowInSep.insert(m); }
			else {
				ContTree[start].LowInSep.insert(m);				
				ContTree[end].HighInSep.insert(m); }
		} // end for
	}
	// used in RecordSuperArcs, which is used in CT, which is used in OnDraw
	void GetSuperArc(struct ContourTreeNode *ContTree, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, set<int>::iterator it, int id, int I, int J, int K, int flag, int is_3D) //reduce a full augmented tree to un
	{   
		vector<vector<int>> superArc;
		vector<int> node;
		if(id==0)
			node.push_back(-1), node.push_back(-1), node.push_back(-1);  
		else 
		{
			if(is_3D)
				node.push_back((id-1)/J%I), node.push_back((id-1)%J),  node.push_back((id-1)/(J*I));  	
			else 
			{
				node.push_back((id-1)/J), node.push_back((id-1)%J), node.push_back(0);  
			}
		}
		superArc.push_back(node);

		int sepid=MSCsep.size();
		int i=id;

		if(flag)
		{
			vector<int> sepd;
			sepd.push_back(i);
			//ContTree[i].Pro[4]=sepid;		 //arcid
			do
			{
				if(i==id)
					i=*it;
				else
					i=*ContTree[i].HighIn.begin();

				vector<int> node;
				if(i==0)
					node.push_back(-1), node.push_back(-1), node.push_back(-1);  
				else
				{
					if(is_3D)  node.push_back((i-1)/J%I), node.push_back((i-1)%J),  node.push_back((i-1)/(J*I));  	
					else node.push_back((i-1)/J), node.push_back((i-1)%J), node.push_back(0);  
				}
				//ContTree[i].Pro[4]=sepid;
				superArc.push_back(node);
			}
			while(ContTree[i].HighIn.size()!=0&&ContTree[i].HighIn.size()<2&&ContTree[i].LowIn.size()<2);	
			sepd.push_back(i);
			sepd.push_back(1);
			MSCsep.push_back(superArc);	
			MSCsepD.push_back(sepd);		
			ContTree[id].HighInSep.insert(sepid), ContTree[i].LowInSep.insert(sepid);
		}
		else
		{
			vector<int> sepd;
			sepd.push_back(i);
			//ContTree[i].Pro[4]=sepid;		//arcid
			do
			{
				if(i==id)
					i=*it;
				else
					i=*ContTree[i].LowIn.begin();
				vector<int> node;
				if(i==0)
					node.push_back(-1), node.push_back(-1), node.push_back(-1);  
				else
				{
					if(is_3D)  node.push_back((i-1)/J%I), node.push_back((i-1)%J),  node.push_back((i-1)/(J*I));  	
					else node.push_back((i-1)/J), node.push_back((i-1)%J), node.push_back(0);  
				}
				//ContTree[i].Pro[4]=sepid;           
				superArc.push_back(node);		
			}
			while(ContTree[i].LowIn.size()!=0&&ContTree[i].HighIn.size()<2&&ContTree[i].LowIn.size()<2);	
			MSCsep.push_back(superArc);	
			sepd.push_back(i);
			sepd.push_back(-1);
			MSCsepD.push_back(sepd);
			ContTree[id].LowInSep.insert(sepid), ContTree[i].HighInSep.insert(sepid);
		}
	}

void CT(struct ContourTreeNode * &ContTree, struct JoinSplitTreeNode * &JoinTree, struct JoinSplitTreeNode * &SplitTree, struct MorseSmaleCriticalPoint* &MSCpoint, vector<vector<int>> &PairQ, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, vector<vector<int>> &pairs, vector<int> &recurid, float ***Values, float &fMin, float &fMax, int &MSCriNum, int I, int J, int K, bool flag, bool is_3D)//for build CT with all the vertex
{   //flag=1, record edge span , 0 do not record
		//static int mark = 0;
	    //struct MorseSmaleCriticalPoint* MSCpoint;
		//int MSCriNum;
		SolveDegeneration(Values, fMin, fMax, I, J, K, is_3D);	
		JoinSplitTree(ContTree, JoinTree, SplitTree, Values, I, J, K, flag, false, is_3D); //build CT and MScomplex  		
    	RecordSuperArcs(ContTree, MSCsep, MSCsepD, I, J, K, is_3D);

		RecordSuperArcs(MSCsep, MSCsepD, ContTree); 

		ReduceTree(ContTree, I, J, K);   
        
		RecordCritical(ContTree,Values, MSCpoint,MSCriNum,I, J, K); 
		ReduceCT(ContTree,MSCpoint,MSCriNum,MSCsep,MSCsepD, I, J, K, 1, is_3D);

		/*if(flag)  //use augmented CT for contour-based uncertainty
		{ 
			IsoNum = 30;
			RecordContInfo(ContTree, MSCsep, sepWatch, sepWatch1, sepWatch2, nodeWatch, nodeWatch1, errorWatch, errorWatch1, errorWatch2, errorWatch3, sepSample, IsoNum, Values);
		}*/		
		
		PairQ = CTSimplification_UpDown_2(ContTree,MSCpoint,MSCsep,MSCsepD,MSCriNum);  //new version according to the algorithm in the paper.
		//PairQ //0: saddle, 1: extremum, 2:upward or downward, 3:parent branch id, 4: record the best matched pair, 5 - ... : childs  

		//DrawHierarchyContTree_2
		vector<vector<int>> NodePosition = DrawHierarchyContTree_2(ContTree,MSCpoint,pairs, recurid, PairQ, MSCriNum, fMin, fMax);	   
        
		//mark++;
}

	/// <summary>
	/// This function compares three floats and returns the largest of the three.
	/// </summary>
	/// <remarks>
	/// used in Traversal, which is used in CTSimplification_UpDown_2, which is used in CT, which is used in OnDraw
	/// </remarks>
	/// <param name='longest1'>The first of the three floats to be compared.</param>
	/// <param name='longest2'>The second of the three floats to be compared.</param>
	/// <param name='longest3'>The third of the three floats to be compared.</param>
	/// <returns>
	/// The largest of the three parameters.
	/// </returns>
	float MaxPersistence(float longest1, float longest2, float longest3)
	{
		float max;
		max = longest1;
		if(max < longest2) { max = longest2; }
		if(max < longest3) { max = longest3; }
		return max;
	}

// used in CTSimplification_UpDown_2, which is used in CT, which is used in OnDraw
	void Traversal(struct ContourTreeNode *&ContTree, struct CTtemp * &cT, vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, int u, int type)
	{	//type: 1, height persistence, 2, area persistence.
		//label u as explored
		vector<int> ui,si,ti;
		int u0=-1,v1,v2,d;
		vector<int> v,s;
		vector<int> branch(2,-1);
		cT[u].mark=true;
		// if u is not the virtual minimum and u is a leaf then
		if(u != 0 && (ContTree[u].HighIn.size() == 0 || ContTree[u].LowIn.size() == 0))
		{
			cT[u].branch1=u;
			cT[u].branch2=u;
			if(type==1)
			{
				cT[u].longest1=0;
				cT[u].longest2=0;
			}
			else if(type == 2)
			{
				cT[u].longest1=1;
				cT[u].longest2=1;
			}
		}
		else 
		{   //for each neighbour ui of u do
			for(set <int> ::iterator it= ContTree[u].LowIn.begin(); it!=ContTree[u].LowIn.end(); it++)
			{  ui.push_back(*it); if(cT[*it].mark) d=1; }

			for(set <int> ::iterator it= ContTree[u].HighIn.begin(); it!=ContTree[u].HighIn.end(); it++)
			{  ui.push_back(*it); if(cT[*it].mark) d=-1; }

			for(set <int> ::iterator it= ContTree[u].LowInSep.begin(); it!=ContTree[u].LowInSep.end(); it++)
				si.push_back(*it);

			for(set <int> ::iterator it= ContTree[u].HighInSep.begin(); it!=ContTree[u].HighInSep.end(); it++)
				si.push_back(*it);

			u0 = -1;
			for(unsigned int i = 0; i < ui.size(); i++) {
				if(!cT[ui[i]].mark) { //if ui is unexplored then
					Traversal(ContTree, cT, MSCsep, MSCsepD, ui[i], type);
					v.push_back(ui[i]);
					/*for(unsigned int j = 0; j < si.size(); j++) {  
						if(MSCsepD[si[j]][0] + MSCsepD[si[j]][1] == ui[i] + u) {
							ti.push_back(si[j]);
							break; }
					} *////keqin??? // end for loop
				} // end "ui is unexplored"
				else { u0 = ui[i]; }
			} // end for loop

			//Let u0 be explored one among ui, v1 and v1 the other two
			if(u0 != -1) {
				if(type == 1) {                    
					if(d > 0) {    
						//Let v1 be with the lower farthest_end, v2 be the other
						if(ContTree[cT[v[0]].branch2].value < ContTree[cT[v[1]].branch2].value) {
							v1 = v[0];
							v2 = v[1]; }
						else {
							v1 = v[1];
							v2 = v[0]; }
					} // end "d > 0"
					else /* d > 0 */ {
						//Let v1 be with higher farthest_end, v2 be the other
						if(ContTree[cT[v[0]].branch2].value > ContTree[cT[v[1]].branch2].value) {
							v1 = v[0];
							v2 = v[1]; }
						else {
							v1 = v[1];
							v2 = v[0]; }
					}
					cT[u].branch1=cT[v1].branch2;
					cT[u].branch2=cT[v2].branch2;         
					cT[u].childset1=cT[v1].childset2;
					if(v1 != cT[v1].branch1) {
						branch[0] = v1;
						branch[1] = cT[v1].branch1;
						cT[u].childset1.push_back(branch); }

					cT[u].childset2 = cT[v2].childset2;
					if(v2!=cT[v2].branch1) {
						branch[0] = v2;
						branch[1] = cT[v2].branch1;
						cT[u].childset2.push_back(branch); }

					cT[u].longest1 = MaxPersistence(cT[v1].longest2, cT[v1].longest1, fabs(ContTree[cT[u].branch1].value-ContTree[u].value));     
					cT[u].longest2 = MaxPersistence(cT[v2].longest2, cT[v2].longest1, fabs(ContTree[cT[u].branch2].value-ContTree[u].value));			
				}
				else if(type == 2) {
					int sum1 = int(cT[v[0]].longest1 + cT[v[0]].longest2 + MSCsep[ti[0]].size() - 2);
					int sum2 = int(cT[v[1]].longest1 + cT[v[1]].longest2 + MSCsep[ti[1]].size() - 2);
					if(sum1 < sum2) {
						v1 = v[0];
						cT[u].longest1 = (float)sum1;
						cT[u].longest2 = (float)sum2;
						cT[u].branch1 = cT[v[0]].branch2;
						cT[u].branch2 = cT[v[1]].branch2;         		    			  

						cT[u].childset1 = cT[v[0]].childset2;
						if(v[0] != cT[v[0]].branch1) {
							branch[0] = v[0];
							branch[1] = cT[v[0]].branch1;
							cT[u].childset1.push_back(branch); }

						cT[u].childset2 = cT[v[1]].childset2;
						if(v[1] != cT[v[1]].branch1) {
							branch[0] = v[1];
							branch[1] = cT[v[1]].branch1;
							cT[u].childset1.push_back(branch); }
					}
					else {
						v1 = v[1];
						cT[u].longest1 = (float)sum2;
						cT[u].longest2 = (float)sum1;
						cT[u].branch1 = cT[v[1]].branch2;
						cT[u].branch2 = cT[v[0]].branch2;  

						cT[u].childset1 = cT[v[1]].childset2;		          
						if(v[1] != cT[v[1]].branch1) {
							branch[0] = v[1];
							branch[1] = cT[v[1]].branch1;
							cT[u].childset1.push_back(branch); }

						cT[u].childset2=cT[v[0]].childset2;
						if(v[0]!=cT[v[0]].branch1) {
							branch[0] = v[0];
							branch[1] = cT[v[0]].branch1;
							cT[u].childset1.push_back(branch); }
					}
				} // end "type == 2"
				for(set<int>::iterator it = ContTree[u].LowIn.begin(); it!=ContTree[u].LowIn.end(); it++)
					if(*it==v1)  ContTree[*it].Pro[1]=1;

				for(set<int>::iterator it = ContTree[u].HighIn.begin(); it != ContTree[u].HighIn.end(); it++) {
					if(*it == v1) { ContTree[*it].Pro[1]=1; } }
			} // end "u0 != -1"
			else
			{
				if(v.size()>0)
				{
					cT[u].branch1 = cT[v[0]].branch2;
					cT[u].branch2 = -1;
					cT[u].childset1 = cT[v[0]].childset2;
					branch[0] = v[0];
					branch[1] = cT[v[0]].branch1;
					cT[u].childset1.push_back(branch);
					cT[u].longest1 = 1000000;     
					cT[u].longest2 = 0;
				}
			}
		}
	}

	// used in CTSimplification_UpDown_2, which is used in CT, which is used in OnDraw
	void  CopyContTree(struct ContourTreeNode *ContTree, struct CTtemp * &cT, int MSCriNum)
	{//copy ContTree to cT
		cT = new struct CTtemp[MSCriNum];   	
		for(int i = 0; i<MSCriNum; i++)
		{
			cT[i].mark = false;	
			cT[i].branch1=-1;
			cT[i].branch2=-1;
			ContTree[i].Pro[1]=0;
			ContTree[i].Pro[0]=1;
		}
	}

// used in CT --> OnDraw "new simplification by area"
	vector<vector<int>> CTSimplification_UpDown_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<vector<int>>> MSCsep, vector<vector<int>> MSCsepD, int MSCriNum)
	{   //output pair in top down order
		//move Contour Tree to CTtemp
		struct CTtemp *cT;
		CopyContTree(ContTree,cT,MSCriNum);
		vector<vector<vector<int>>> sep;
		vector<vector<int>> sepD;
		Traversal(ContTree,cT,sep, sepD, 0,1);
		return(TopDown(cT, MSCpoint, 0));
	}


	// used in CTSimplification_UpDown_2 --> CT --> OnDraw 
	vector<vector<int>> TopDown(struct CTtemp * &cT, struct MorseSmaleCriticalPoint *MSCpoint, int u)
	{
		//Priority queue Q ={v.branch1};      
		int v, count;
		vector<vector<int>> nodes;
		bool(*fn_pt)(vector<float>, vector<float>) = fccomp_2;
		set<vector<float>, bool(*)(vector<float>, vector<float>)>  Q(fn_pt);
		vector<float> branch(4,0);
		vector<float> Di(4,0);
		vector<int>   pair(5,-1);
		branch[1] = float(u);
		branch[2] = float(cT[u].branch1);
		branch[0] = cT[u].longest1;
		branch[3] = -1.0f;
		Q.insert(branch);
		count = 0;
		while(!Q.empty())   
		{           
			branch = *(Q.begin()); 
			Q.erase(Q.begin());
			u = int(branch[1]);
			v = int(branch[2]);
			if(u>=0)
			for(unsigned int i = 0; i < cT[u].childset1.size(); i++) {
				Di[0] = float(cT[cT[u].childset1[i][0]].longest1);
				Di[1] = float(cT[u].childset1[i][0]);
				Di[2] = float(cT[u].childset1[i][1]);
				Di[3] = float(count);
				Q.insert(Di);		  
			}
			if(v>=0)
			if(u != v) {
				pair[0] = u;
				pair[1] = v;
				pair[2] = (MSCpoint[v].type == 2) ? 1 : -1;  
				pair[3] = (int)(branch[3]);
				nodes.push_back(pair);             
			}		
			count++;
		} 
		return(nodes);
	}

//=======================================================================//
	// Draw Hierarchy Contour Tree                                           //
	//=======================================================================//
	// used in OnDraw
	vector<vector<int>> DrawHierarchyContTree_2(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector<vector<int>> &pairs, vector<int> &recurid, vector<vector<int>> PairQ, int MSCriNum, float fMin, float fMax)
	{   // no recurrence
		vector<int> point(4,0); // TODO: why start at point (4,0)? What is the significance?
		vector<vector<int>> NodePosition_1;

		// clean global variables
		pairs.clear();
		recurid.clear();

		RecordHierarchyContTree_1(ContTree, MSCpoint, pairs, recurid, PairQ, -1, 0, 1, 0, 0);

		// note: MSCriNum is a global variable
		int Width, Center;
		RecordHierarchyContTree_2(ContTree, MSCpoint, MSCriNum, pairs, recurid, NodePosition_1, PairQ, Width, Center, fMin, fMax);

		// set some more global variables
		vector<vector<int>> locationArray = LocateNodePosition(ContTree, MSCpoint, MSCriNum, PairQ.size(), NodePosition_1, fMin, fMax);
		//temp keqin
		//int Width=100, Center=50;
		vector<vector<int>> branchLocationArray = LocateBranchPosition(ContTree, PairQ, NodePosition_1, PairQ.size(), Width, Center);
        
		return NodePosition_1;
	}

	
	//=======================================================================//
	// Record Hierarchy Contour Tree                                         //
	//=======================================================================//
	// used in DrawHierarchyContTree_2, which is used in OnDraw
	void RecordHierarchyContTree_1(
		struct ContourTreeNode* ContTree, struct MorseSmaleCriticalPoint *MSCpoint,
		vector<vector<int>> &pairs, vector<int> &recurid, vector<vector<int>> PairQ,
		int start, int id, int d, int precount, int flag  )
	{
		//d: up or down, no recurrence
		vector<int> nodes;

		vector<int> branch(8,-1);

		bool mark;
		int order = 0;
		int i;
		int count;
		int childcount;

		// create a thing named Q. This actually does initialize an empty list.
		list<vector<int>> listOfVectorsOfInts;
		// create a new vector with space for 8 integers, each of which is initialized to -1.
		// this is the type of thing that listOfVectorsOfInts holds.
		vector<int> branch1(8,-1);
		// initialize branch1's values
		branch1[0] = start;
		branch1[1] = id;
		branch1[2] = d;
		branch1[3] = flag; 
		branch1[4] = -1; //order
		branch1[5] = -1; //pole's end
		// store branch1 at the end of listOfVectorsOfInts
		listOfVectorsOfInts.push_back(branch1);
		// keep looping until listOfVectorsOfInts has been emptied
		while(!listOfVectorsOfInts.empty()) {
			// set branch1 to be the first vector in listOfVectorsOfInts
			branch1 = *(listOfVectorsOfInts.begin());
			// retreive values from branch1 and store them in the parameters
			start = branch1[0];
			id    = branch1[1];
			d     = branch1[2];
			flag  = branch1[3];

			// clean nodes, which is a local variable
			nodes.clear();
			// store start, which is branch1[0]
			nodes.push_back(start);
			branch1[6] = SearchBranch(ContTree, id, d, nodes);

			if(id == 0) { childcount = branch1[6] - 1; }
			else { childcount = branch1[6]; }

			if(branch1[6] == 0 && branch1[7] == 0)
			{
				listOfVectorsOfInts.erase(listOfVectorsOfInts.begin());
				branch1[7]--;
				listOfVectorsOfInts.push_front(branch1);   	
			}
			else 
			{
				listOfVectorsOfInts.erase(listOfVectorsOfInts.begin());

				if(branch1[6] == 0)	{ recurid.push_back(order); }
				if(branch1[4]>=0)
				{
					pairs[branch1[4]][6]--;
					if(branch1[6] == 0)
					{
						int pre=branch1[4];
						while(pre>=0&&pairs[pre][6]==0)
						{
							recurid.push_back(pre);
							pre=pairs[pre][4];		
						}
					}
				}
				branch1[1]=*nodes.rbegin();
				int exam=branch1[0];
				pairs.push_back(branch1);
				if(branch1[5]==48&&(branch1[1]==70||branch1[1]==64||branch1[1]==11||branch1[1]==52||branch1[1]==85||branch1[1]==36))
					branch1[0]=branch1[0];
				//debuge
				int exam1=branch1[1];
				exam1=branch[0];

				list<vector<int>> tempListOfVectorsOfInts;

				count = 0;
				mark = false;	
				for((d > 0) ? (i = 1) : (i = nodes.size() - 2); (d > 0) ? (i < nodes.size() - 1) : (i > 0); (d > 0) ? i++ : i--)
				{
					int indexOfCurrentContourTreeNode = nodes[i];
					ContourTreeNode currentContourTreeNode = ContTree[indexOfCurrentContourTreeNode];
					// loop through the integers in the "HighIn" set, starting at the first one, and ending at the second-to-last one
					for(set<int>::iterator it = currentContourTreeNode.HighIn.begin(); it != currentContourTreeNode.HighIn.end(); it++) {
						// check if any of the integers in "HighIn" match the index of the previous node or the index of the next node.
						// if any integer in "HighIn" fails to match both indeces, set mark to true.
						if(*it != nodes[i-1] && *it != nodes[i+1]) { mark = true; } }

					// loop through the integers in the "LowIn" set, statting at the first one, and ending at the second-to-last one
					for(set<int>::iterator it = currentContourTreeNode.LowIn.begin(); it != currentContourTreeNode.LowIn.end(); it++) {
						// if a given integer in "LowIn" fails to match both the index of the previous node and the index of the next node,
						if(*it != nodes[i-1] && *it != nodes[i+1]) {
							// if no such integer was previously found in "HighIn",
							if(!mark) {
								// flag is a parameter passed to this function. It is them modified, it value set to "branch1[3]"
								if((flag > 0) ? (count % 2 == 1) : (count % 2 == 0))
								{
									branch[0] = nodes[i];
									branch[1] = *it; 
									branch[2] = -1;
									branch[3] = -1;
									branch[4] = order;
									branch[5] = *nodes.rbegin();

									if(--childcount == 0) { branch[7] = 0; }
									else { branch[7] = -1; }
				  
									tempListOfVectorsOfInts.push_back(branch);	
								}
								else 
								{
									branch[0] = nodes[i];
									branch[1] = *it;
									branch[2] = -1;
									branch[3] = 1;
									branch[4] = order;
									branch[5] = *nodes.rbegin();

									if(--childcount == 0) { branch[7] = 0; }
									else { branch[7] = -1; }

									tempListOfVectorsOfInts.push_back(branch);
								}
								count++;
							}
							else 
							{
								branch[0]=nodes[i], branch[1]=*it, branch[2]=-1, branch[3]=2, branch[4]=order, branch[5]=*nodes.rbegin();

								if(--childcount == 0) { branch[7] = 0; }
								else { branch[7] = -1; }

								tempListOfVectorsOfInts.push_back(branch);
							}
						}
					}
				}
				count = 0;
				mark = false;
				for( d>0?i=nodes.size()-2:i=1; d>0?i>0:(i<nodes.size()-1);d>0?i--:i++)
				{
					for(set<int>::iterator it=ContTree[nodes[i]].HighIn.begin(); it!=ContTree[nodes[i]].HighIn.end(); it++)
					{

						if(*it!=nodes[i-1]&&*it!=nodes[i+1])
						{
							if(!mark)
							{
								if(flag>0?count%2==0:count%2==1)
								{
									branch[0]=nodes[i], branch[1]=*it, branch[2]=1, branch[3]=1, branch[4]=order, branch[5]=*nodes.rbegin();

									if(branch[5]==48&&(branch[1]==70||branch[1]==64||branch[1]==11||branch[1]==52||branch[1]==85||branch[1]==36))
										i=i;

									if(--childcount==0) { branch[7]=0; }
									else { branch[7]=-1; }

									tempListOfVectorsOfInts.push_back(branch);
								}
								else 
								{

									branch[0]=nodes[i], branch[1]=*it, branch[2]=1, branch[3]=-1, branch[4]=order, branch[5]=*nodes.rbegin();

									if(branch[5]==48&&(branch[1]==70||branch[1]==64||branch[1]==11||branch[1]==52||branch[1]==85||branch[1]==36))
										i=i;

									if(--childcount==0) { branch[7] = 0; }
									else { branch[7] = -1; }

									tempListOfVectorsOfInts.push_back(branch);					 
								}
								count++;
							}
							else 
							{
								branch[0]=nodes[i], branch[1]=*it, branch[2]=1, branch[3]=-2, branch[4]=order, branch[5]=*nodes.rbegin();

								if(--childcount==0) { branch[7]=0; }
								else { branch[7] = -1; }
				
								tempListOfVectorsOfInts.push_back(branch);
							}				
						}
					}
					for(set<int>::iterator it=ContTree[nodes[i]].LowIn.begin(); it!=ContTree[nodes[i]].LowIn.end(); it++) {
						if(*it!=nodes[i-1]&&*it!=nodes[i+1]) { mark=true; } }
				}
				order++;
				for(list<vector<int>>::reverse_iterator iq=tempListOfVectorsOfInts.rbegin(); iq!=tempListOfVectorsOfInts.rend(); iq++) {
					listOfVectorsOfInts.push_front(*iq); }
			}
		}
	}

	// used in stuff that leads back to OnDraw
	int SearchBranch(struct ContourTreeNode* ContTree, int id, int d, vector<int> &nodes)
	{
		// Things I know so far:
		// - parameter "ContTree" is an array of type "ContourTreeNode"
		//      - parameter "id" is an index that is accessed within "ContTree"
		int count = 0;
		nodes.push_back(id);
		if(d > 0)
		{
			while(ContTree[id].HighIn.size() > 0)
			{
				for(set<int>::iterator it = ContTree[id].HighIn.begin(); it != ContTree[id].HighIn.end(); it++)
				{
					if(ContTree[*it].Pro[1] == 0)	   	  
					{			  
						nodes.push_back(*it);			
						id = *it;
						count++;
						break;
					}
				}
			}	
		}
		else 
		{
			while(ContTree[id].LowIn.size() > 0)
			{
				for(set<int>::iterator it = ContTree[id].LowIn.begin(); it!=ContTree[id].LowIn.end(); it++)
				{
					if(ContTree[*it].Pro[1] == 0)	   	  
					{
						nodes.push_back(*it);			
						id = *it;
						count++;
						break;
					}
				}
			}	
		}  
		return count;
	}

	//=======================================================================//
	// Slots                                                                 //
	//=======================================================================//
	// used in RecordHierarchyContTree_2, which is used in ReDrawHierarchyContTree_2, which is used in OnDraw
	int searchSlot1(vector<float> SlotHeight, float height, float itv)
	{
		for(unsigned int i = 0; i < SlotHeight.size(); i++) {
			if(SlotHeight[i] - itv >= height) { return i; }
		} // end for
		return SlotHeight.size();
	}
	// used in RecordHierarchyContTree_2, which is used in ReDrawHierarchyContTree_2, which is used in OnDraw
	int searchSlot2(vector<float> SlotHeight, float height, float itv)
	{
		for(unsigned int i = 0; i < SlotHeight.size(); i++) {
			if(SlotHeight[i] + itv <= height) { return i; }
		} // end for
		return SlotHeight.size();;
	}
	// used in RecordHierarchyContTree_2, which is used in ReDrawHierarchyContTree_2, which is used in OnDraw
	int searchSlot3(vector<float> SlotHeight, float height, float itv)
	{
		for(unsigned int i = 0; i < SlotHeight.size(); i++) {
			if(SlotHeight[i]-itv>=height) { return i; }
		} // end for
		return SlotHeight.size();
	}
	// used in RecordHierarchyContTree_2, which is used in ReDrawHierarchyContTree_2, which is used in OnDraw
	int searchSlot4(vector<float> SlotHeight, float height, float itv)
	{
		for(unsigned int i = 0; i < SlotHeight.size(); i++) {
			if(SlotHeight[i] + itv <= height) { return i; }
		} // end for
		return SlotHeight.size();
	}
	// used in RecordHierarchyContTree_2, which is used in ReDrawHierarchyContTree_2, which is used in OnDraw
	void fillSlots(vector<float> &SlotHeight, float height, int start, int end)
	{
		int size = SlotHeight.size();
		for(int i = start; i < end; i++)
		{
			if(i<size) { SlotHeight[i] = height; }
			else SlotHeight.push_back(height);
		}
	}

	
	// used in ReDrawHierarchyContTree_2, which is used in OnDraw
	void RecordHierarchyContTree_2(struct ContourTreeNode* ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int MSCriNum,  vector< vector<int> > pairs, vector<int> recurid, vector< vector<int> > &NodePosition, vector < vector <int> > PairQ, int &Width, int &Center, float fMin, float fMax)
	{	
		int interval_H = 40;
		//int Width;				// used in RecordHierarchyContTree_2
        //int Center;	

		vector< vector<int> > NodePosition_1;
		vector< vector<float> > NodeHeight;
		vector< vector<float> > SlotHeight1,SlotHeight2,SlotHeight3,SlotHeight4;
		vector <int> NodePosition_2;

		vector<int> point9(9,0),point(3,0);
		vector<float> point0(2,0);
		vector<float> point1,point2,point3,point4;
		float itv=(fMax-fMin)*0.05;

		for(int i=0;i<MSCriNum;i++)
		{	
			NodePosition_1.push_back(point9); 
			NodePosition_2.push_back(0);
			NodePosition.push_back(point);
			NodeHeight.push_back(point0); 
			SlotHeight1.push_back(point1);
			SlotHeight2.push_back(point2);
			SlotHeight3.push_back(point3);
			SlotHeight4.push_back(point4);		
		}

		NodeHeight[pairs[0][1]][0]=fMin-(fMax-fMin)*0.1, NodeHeight[pairs[0][1]][1]=ContTree[pairs[0][1]].value;	

		for(int j=1; j<pairs.size(); j++)
		{
			vector<int> branch=pairs[j];
			int D=branch[1]; //its end
			int S=branch[0]; //its start
			if(ContTree[D].value<ContTree[S].value)
			{
				NodeHeight[D][0]=ContTree[D].value;
				NodeHeight[D][1]=ContTree[S].value;
			}
			else
			{
				NodeHeight[D][1]=ContTree[D].value;
				NodeHeight[D][0]=ContTree[S].value;
			}
		}

		for(int j=0; j<recurid.size(); j++)
		{
			vector<int> branch = pairs[recurid[j]];
			int start,start1;
			if(branch[0]>=0)
			if(ContTree[branch[0]].Pro[0]==1)
			{
				int D=branch[1];  //its end
				int N=branch[5];  //its pole end	
				int Leftcount=NodePosition_1[D][1]>NodePosition_1[D][2]?NodePosition_1[D][1]:NodePosition_1[D][2];
				int Rightcount=NodePosition_1[D][3]>NodePosition_1[D][4]?NodePosition_1[D][3]:NodePosition_1[D][4];
				int total=Leftcount+Rightcount+1;
				int leftcount, rightcount;

				if(branch[3]==-2||branch[3]==2)  // in the middle session
				{
					if(branch[3]>0 && branch[2]>0)//up left
					{
						start=searchSlot1(SlotHeight1[N], NodeHeight[D][1], itv);
						total+=start;
						NodePosition_1[N][1]=total>NodePosition_1[N][1]?total:NodePosition_1[N][1];
						fillSlots(SlotHeight1[N],NodeHeight[D][0],0,total);
						NodePosition_1[D][5]=start+Rightcount+1;
					}
					else if(branch[3]>0 && branch[2]<0) //down left
					{
						start=searchSlot2(SlotHeight2[N], NodeHeight[D][0], itv);
						total+=start;
						NodePosition_1[N][2]=total>NodePosition_1[N][2]?total:NodePosition_1[N][2];
						fillSlots(SlotHeight2[N],NodeHeight[D][1],0,total);
						NodePosition_1[D][6]=start+Rightcount+1;
					}
					else if(branch[3]<0 && branch[2]>0) //up right
					{
						start=searchSlot3(SlotHeight3[N], NodeHeight[D][1], itv);
						total+=start;
						NodePosition_1[N][3]=total>NodePosition_1[N][3]?total:NodePosition_1[N][3];
						fillSlots(SlotHeight3[N],NodeHeight[D][0],0,total);
						NodePosition_1[D][7]=start+Leftcount+1;
					}
					else if(branch[3]<0 && branch[2]<0)
					{
						if(NodeHeight[N][9]+itv<=NodeHeight[D][0])
							NodePosition_1[N][4]=total>NodePosition_1[N][4]?total:NodePosition_1[N][4];
						else { NodePosition_1[N][4] += total; }
						start = searchSlot4(SlotHeight4[N], NodeHeight[D][0], itv);
						total += start;
						NodePosition_1[N][4] = total>NodePosition_1[N][4]?total:NodePosition_1[N][4];
						fillSlots(SlotHeight4[N],NodeHeight[D][1],0,total);
						NodePosition_1[D][8] = start+Leftcount+1;
					}
				}
				else  // in the top and down session
				{
					if(branch[2]>0)//up
					{
						start=searchSlot1(SlotHeight1[N], NodeHeight[D][1], itv);			   
						leftcount=total+start;
						start1=searchSlot3(SlotHeight3[N], NodeHeight[D][1], itv);
						rightcount=total+start1;

						if(leftcount==rightcount)
						{
							if(branch[3]>0)//left
							{
								NodePosition_1[N][1]=leftcount;	
								fillSlots(SlotHeight1[N],NodeHeight[D][0],0,total+start);
								NodePosition_1[D][5]=start+Rightcount+1;
							}
							else //right
							{
								NodePosition_1[N][3]=rightcount;
								fillSlots(SlotHeight3[N],NodeHeight[D][0],0,total+start1);
								NodePosition_1[D][7]=start1+Leftcount+1;
							}
						}
						else if(leftcount<rightcount)
						{   
							NodePosition_1[N][1]=leftcount;  //up left
							fillSlots(SlotHeight1[N],NodeHeight[D][0],0,total+start);
							NodePosition_1[D][5]=start+Rightcount+1;
							pairs[recurid[j]][3]=1;
						}
						else 
						{
							NodePosition_1[N][3]=rightcount;
							fillSlots(SlotHeight3[N],NodeHeight[D][0],0,total+start1);
							NodePosition_1[D][7]=start1+Leftcount+1;
							pairs[recurid[j]][3]=-1;
						}
					}
					else //down
					{
						start=searchSlot2(SlotHeight2[N], NodeHeight[D][0], itv);			    
						leftcount=total+start;
						start1=searchSlot4(SlotHeight4[N], NodeHeight[D][0], itv);
						rightcount=total+start1;

						if(leftcount==rightcount)
						{
							if(branch[3]>0)
							{ 
								NodePosition_1[N][2]=leftcount;
								NodePosition_1[D][6]=start+Rightcount+1;
								fillSlots(SlotHeight2[N],NodeHeight[D][1],0,total+start);					   
							}		          
							else //right down
							{
								NodePosition_1[N][4]=rightcount;
								NodePosition_1[D][8]=start1+Leftcount+1;
								fillSlots(SlotHeight4[N],NodeHeight[D][1],0,total+start1);					   
							}	
						}
						else if(leftcount<rightcount)  //left down
						{ 
							NodePosition_1[N][2]=leftcount;
							NodePosition_1[D][6]=start+Rightcount+1;
							fillSlots(SlotHeight2[N],NodeHeight[D][1],0,total+start);					
							pairs[recurid[j]][3]=1;
						}
						else //right down
						{
							NodePosition_1[N][4]=rightcount;
							NodePosition_1[D][8]=start1+Leftcount+1;
							fillSlots(SlotHeight4[N],NodeHeight[D][1],0,total+start1);
							pairs[recurid[j]][3]=-1;
						}
					}
				}	
			}		
		}

		NodePosition_2[pairs[0][1]] = NodePosition_2[pairs[0][1]] = 0;

		for(int j=1; j<pairs.size(); j++)
		{
			vector<int> branch=pairs[j];
			if(branch[0]>=0)
			if(ContTree[branch[0]].Pro[0]==1)
			{ 
				int D=branch[1]; //its end
				int N=branch[5]; //its pole end
				int S=branch[0]; //its start
				int center=NodePosition_2[N];
				ContTree[branch[0]].Pro[0]=1, ContTree[branch[1]].Pro[0]=1;

				if(branch[3]>0&&branch[2]>0)//up left
				{
					NodePosition_2[D]=center-NodePosition_1[D][5];
				}
				else if(branch[3]>0&&branch[2]<0) //down left
				{
					NodePosition_2[D]=center-NodePosition_1[D][6];
				}
				else if(branch[3]<0&&branch[2]>0) //up right
				{
					NodePosition_2[D]=center+NodePosition_1[D][7];
				}
				else if(branch[3]<0&&branch[2]<0) 
				{
					NodePosition_2[D]=center+NodePosition_1[D][8];
				}
				NodePosition_2[S]=center;	   
			}
		}

		float temp; //if set interval>1, then no vertical overlay
		int x0, y0, x1, y1;		
		int length=1000;//J*DY*5.33;
		int centerx=0;//I*DX*3.5;//keep constant with others

		int leftcount = MaxFloat;
		int rightcount = MinFloat;	


		int count1=0;
		if(PairQ.size()>0)
		for(vector< vector <int> >::iterator it=++PairQ.begin(); it!=PairQ.end(); it++)
		{
			vector <int> point=*it;	
			int start=point[0], end=point[1];
			if(ContTree[point[0]].Pro[0]==1)
			{
				if(NodePosition_2[end]<leftcount)
					leftcount=NodePosition_2[end];

				if(NodePosition_2[end]>rightcount)
					rightcount=NodePosition_2[end];

				x0=NodePosition_2[start]+centerx;
				temp=ContTree[point[0]].value-fMin;		
				y0=length*temp/(fMax-fMin);	

				x1=NodePosition_2[end]+centerx;
				temp=ContTree[point[1]].value-fMin;		
				y1=length*temp/(fMax-fMin);	

				NodePosition[end][0]=x1; 
				NodePosition[end][1]=y1;
				NodePosition[end][2]=point[1];

				NodePosition[start][0]=x0; 
				NodePosition[start][1]=y0;
				NodePosition[start][2]=point[0];  //its node is in CT
				if(NodePosition_1[end][0]<NodePosition_1[start][0]) //left
					NodePosition[start].push_back(NodePosition_1[start][1]>NodePosition_1[start][2]?NodePosition_1[start][1]:NodePosition_1[start][2]);   //most left node if it it at left, its most right node if it is at right
				else NodePosition[start].push_back(NodePosition_1[start][3]>NodePosition_1[start][4]?NodePosition_1[start][3]:NodePosition_1[start][4]);   //most left node if it it at left, its most right node if it is at right

				NodePosition[start].push_back(count1);
			}
			else if(ContTree[point[0]].Pro[0]==2)
			{
				int N;
				N=PairQ[point[3]][1];  //its pole end
				start=point[0];
				x0=NodePosition_2[N]*interval_H+centerx;		
				temp=ContTree[point[0]].value-fMin;		
				y0=length*temp/(fMax-fMin);	

				NodePosition[start][0]=x0; 
				NodePosition[start][1]=y0;
				NodePosition[start][2]=point[0];

				NodePosition[start].push_back(count1);
			}
			count1++;
		}
		if(leftcount>0) { leftcount = 0; }
		Width = rightcount - leftcount + 1;
		Center = -leftcount;

		//first pair
		NodePosition[0][0]=centerx; 
		NodePosition[0][1]=-10;
		NodePosition[0][2]=0;
		NodePosition[0].push_back(0);

		if(PairQ.size()>0)
		{
			NodePosition[PairQ[0][1]][0]=centerx; 
			NodePosition[PairQ[0][1]][1]=length;
			NodePosition[PairQ[0][1]][2]=PairQ[0][1];
			NodePosition[PairQ[0][1]].push_back(0);
		}

		NodePosition_2.clear();
		NodePosition_1.clear();
	}
	

	// used in CT --> OnDraw
	void ReduceCT(struct ContourTreeNode *&ContTree, struct MorseSmaleCriticalPoint* MSCpoint, int MSCriNum, vector<vector<vector<int>>> &MSCsep, vector<vector<int>> &MSCsepD, int I, int J, int K, bool flag, bool is_3D)
	{   //flag=1, also rewrite the SepD for CT
		//flag=1, do not rewrite the sepD for CT (datamembers)
		//do 
		//reduce tree to new copy with only critical points;
		struct ContourTreeNode *NewTree=new struct ContourTreeNode[MSCriNum];   
		int v,sepid,start,end;
		int sepnum=MSCsep.size();
		vector<bool> record(sepnum,true);
		static int mark=0;
        mark++;
		for(int i=0;i<MSCriNum;i++)  
		{
			

			if(is_3D)
				v=MSCpoint[i].k*I*J+MSCpoint[i].i*J+MSCpoint[i].j+1;
			else v=MSCpoint[i].i*J+MSCpoint[i].j+1;

			if(v<0) v=0;
			else if(flag) //rewrite MSsepD as well
			{
				//HighInSep, LowInSep
				for(set<int>::iterator it=ContTree[v].HighInSep.begin(); it!=ContTree[v].HighInSep.end(); it++)
				{
					sepid=*it;
					if(record[sepid])
					{
						record[sepid]=false;
						start=MSCsepD[sepid][0];
						end=MSCsepD[sepid][1];

						/* if(ContTree[start].Pro[5]==16||ContTree[end].Pro[5]==16)
						start=start;

						if(ContTree[start].Pro[5]==24||ContTree[end].Pro[5]==24)
						start=start;

						int test1=ContTree[end].Pro[5];
						int test2=ContTree[start].Pro[5];*/

						MSCsepD[sepid][0]=ContTree[start].Pro[5];
						MSCsepD[sepid][1]=ContTree[end].Pro[5];     
					} 				
				}
				if(v==1)
					v=v;
				for(set<int>::iterator it=ContTree[v].LowInSep.begin(); it!=ContTree[v].LowInSep.end(); it++)
				{
					sepid=*it;
					if(record[sepid])
					{
						record[sepid]=false;

						start=MSCsepD[sepid][0];
						end=MSCsepD[sepid][1];

						/*if(ContTree[start].Pro[5]==16||ContTree[end].Pro[5]==16)
						start=start;

						if(ContTree[start].Pro[5]==24||ContTree[end].Pro[5]==24)
						start=start;

						int test1=ContTree[end].Pro[5];
						int test2=ContTree[start].Pro[5];*/
						int test1=ContTree[start].Pro.size();
						int test2=ContTree[end].Pro.size();

						if(start==4)
			            {
							int i1,i2,i3,i4;
							i1=MSCsepD[sepid][0];
							i2=MSCsepD[sepid][1];
							i3=ContTree[start].Pro.size();
							i4=ContTree[end].Pro[5];     
							mark=3;
						}

						MSCsepD[sepid][0]=ContTree[start].Pro[5];
						MSCsepD[sepid][1]=ContTree[end].Pro[5];     
					}
				}	

			}


			//0,2,3,5
			NewTree[i].Pro=ContTree[v].Pro;
			//if(v==0||MSCpoint[i].type>2)
			{
				if(NewTree[i].Pro[0]>=0&&ContTree[NewTree[i].Pro[0]].Pro.size()>0)
					NewTree[i].Pro[0]=ContTree[NewTree[i].Pro[0]].Pro[5];

				if(NewTree[i].Pro[2]>=0&&ContTree[NewTree[i].Pro[2]].Pro.size()>0)
					NewTree[i].Pro[2]=ContTree[NewTree[i].Pro[2]].Pro[5];

				if(NewTree[i].Pro[3]>=0&&ContTree[NewTree[i].Pro[3]].Pro.size()>0)
					NewTree[i].Pro[3]=ContTree[NewTree[i].Pro[3]].Pro[5];
				if(NewTree[i].Pro[4]>=0&&ContTree[NewTree[i].Pro[4]].Pro.size()>0)
					NewTree[i].Pro[4]=ContTree[NewTree[i].Pro[4]].Pro[5];	       
			}

			NewTree[i].HighInSep=ContTree[v].HighInSep;
			NewTree[i].LowInSep=ContTree[v].LowInSep;	   
			NewTree[i].value=ContTree[v].value;	 

			if(ContTree[v].HighIn.size()==1)
				NewTree[i].HighIn.insert(ContTree[*ContTree[v].HighIn.begin()].Pro[5]);
			if(ContTree[v].HighIn.size()==2)
			{
				NewTree[i].HighIn.insert(ContTree[*ContTree[v].HighIn.begin()].Pro[5]);
				NewTree[i].HighIn.insert(ContTree[*ContTree[v].HighIn.rbegin()].Pro[5]);
			}

			if(ContTree[v].LowIn.size()==1)
				NewTree[i].LowIn.insert(ContTree[*ContTree[v].LowIn.begin()].Pro[5]);
			if(ContTree[v].LowIn.size()==2)
			{
				NewTree[i].LowIn.insert(ContTree[*ContTree[v].LowIn.begin()].Pro[5]);
				NewTree[i].LowIn.insert(ContTree[*ContTree[v].LowIn.rbegin()].Pro[5]);
			}

			//ContTree[i].Pro.clear();
			ContTree[v].HighIn.clear();
			ContTree[v].LowIn.clear();
			ContTree[v].HighInSep.clear();
			ContTree[v].LowInSep.clear();	   

		}
		for(int i=0;i<MSCriNum;i++)  
		{
			if(is_3D)
				v=MSCpoint[i].k*I*J+MSCpoint[i].i*J+MSCpoint[i].j+1;
			else v=MSCpoint[i].i*J+MSCpoint[i].j+1;	
			if(v<0)
				v=0;

			ContTree[v].Pro.clear();
		}

		delete [] ContTree;
		ContTree=NewTree;	 

	}

 vector< vector <float> > TruePosition(vector<vector<int>> NodePosition, int MSCriNum, int Width, int Height, int Center)
{
   vector< vector <float> > NodePosition_1;//=f2vector(MSCriNum,2);
   NodePosition_1.resize(MSCriNum);
   float dx=1.9/2.0,dy=1.75/2.0;
   float scale_x=1.9/(Width==0?1:Width);///app->GUIDataManager()->I/;
   float scale_y=1.75/Height;///app->GUIDataManager()->I/;
   for(int i=0; i<MSCriNum && i < NodePosition.size() ; i++)
   {
	   NodePosition_1[i].push_back((NodePosition[i][0]+Center)*scale_x-dx);
	   NodePosition_1[i].push_back(NodePosition[i][1]*scale_y-dy);
       
   }   
   return NodePosition_1;  
}


void RecordHierarchyContTree_2(struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int MSCriNum, int branchNum, vector< vector<int> > pairs, vector<int> recurid, vector< vector<int> > &NodePosition, vector < vector <int> > PairQ, int &Width, int &Height,int &Center, float fMin, float fMax)
//void RecordHierarchyContTree_2(struct ContourTreeNode * ContTree, struct MScripoint *MSCpoint, int MSCriNum, vector< vector<int> > pairs, vector<int> recurid, vector< vector<int> > &NodePosition, vector < vector <int> > PairQ, int &Width, int &Height, int &Center, float fMin, float fMax)
{
        vector< vector<int> > NodePosition_1;
	    //int **NodePosition_1;
        vector <int> NodePosition_2;
		//int *NodePosition_2;

        vector<int> point1(9,0),point(4,0);
		//int point1[9]={0,0,0,0,0,0,0,0,0};
		//int point[3]={0,0,0};

		//NodePosition_1=i2vector(MSCriNum,9);
		//NodePosition_1.resize(MSCriNum);
		//NodePosition_2 = new int [MSCriNum]; 
		//NodePosition_2.resize(MSCriNum);
        for(int i=0;i<MSCriNum;i++)
		{    
			 //NodePosition_1[i][0]=NodePosition_1[i][1]=NodePosition_1[i][2]=NodePosition_1[i][3]
			 //=NodePosition_1[i][4]=NodePosition_1[i][5]=NodePosition_1[i][6]=NodePosition_1[i][7]=NodePosition_1[i][8]=0;
			 
			 //NodePosition[i][0]=NodePosition[i][1]=NodePosition[i][2]=NodePosition[i][3]=0;

			 NodePosition_1.push_back(point1),
             NodePosition_2.push_back(0),
             NodePosition.push_back(point);
		}
		
        //NodePosition= i2vector(MSCriNum,4);
             //NodePosition
        //int branchNum=10; //recurid.size()
    	 
	    for(int j=0; j<branchNum-1; j++)
        {
                vector<int> branch=pairs[recurid[j]];
			    //fprintf(fp,"j=%d\n",j); 
				//fclose(fp);

				if(ContTree[branch[0]].Pro[0]==1)
                {
                int D=branch[1]; //its end
                int N=branch[5];  //its pole end
				
                int leftcount=NodePosition_1[D][1]>NodePosition_1[D][2]?NodePosition_1[D][1]:NodePosition_1[D][2];
                int rightcount=NodePosition_1[D][3]>NodePosition_1[D][4]?NodePosition_1[D][3]:NodePosition_1[D][4];
                int total=leftcount+rightcount+1;

                if(branch[3]>0&&branch[2]>0)
                   NodePosition_1[N][1]+=total;
                else if(branch[3]>0&&branch[2]<0)
                   NodePosition_1[N][2]+=total;
                else if(branch[3]<0&&branch[2]>0)
                   NodePosition_1[N][3]+=total;
                else if(branch[3]<0&&branch[2]<0)
                   NodePosition_1[N][4]+=total;
				//fprintf(fp,"j=%d,NodePosition_1[N][1]=%d,NodePosition_1[N][2]=%d,NodePosition_1[N][3]=%d,NodePosition_1[N][4]=%d\n",j,NodePosition_1[N][1],NodePosition_1[N][2],NodePosition_1[N][3],NodePosition_1[N][4]); 
                }				
        }
        //record node position relative to their closest matched branch
        NodePosition_2[pairs[0][1]]=NodePosition_2[pairs[0][1]]=0;

        for(int j=1; j<branchNum; j++)//pairs.size()
        {
           vector<int> branch=pairs[j];
		   //int *branch=pairs[j];
           if(ContTree[branch[0]].Pro[0]==1)
           {
           int D=branch[1]; //its end
           int N=branch[5]; //its pole end
           int S=branch[0]; //its start
           int center=NodePosition_2[N];
           int leftcount=NodePosition_1[D][1]>NodePosition_1[D][2]?NodePosition_1[D][1]:NodePosition_1[D][2];
           int rightcount=NodePosition_1[D][3]>NodePosition_1[D][4]?NodePosition_1[D][3]:NodePosition_1[D][4];
           int total=leftcount+rightcount+1;
           ContTree[branch[0]].Pro[0]=1, ContTree[branch[1]].Pro[0]=1;

                 if(branch[3]>0&&branch[2]>0)
                {
                        NodePosition_2[D]=center-(NodePosition_1[N][5]+rightcount+1);
                        NodePosition_1[N][5]+=total;
                }
                else if(branch[3]>0&&branch[2]<0)
                {
                        NodePosition_2[D]=center-(NodePosition_1[N][6]+rightcount+1);
                        NodePosition_1[N][6]+=total;
                }
                else if(branch[3]<0&&branch[2]>0)
                {
                        NodePosition_2[D]=center+(NodePosition_1[N][7]+leftcount+1);
                        NodePosition_1[N][7]+=total;
                }
                else if(branch[3]<0&&branch[2]<0)
                {
                        NodePosition_2[D]=center+(NodePosition_1[N][8]+leftcount+1);
                        NodePosition_1[N][8]+=total;
                }
                NodePosition_2[S]=center;
				//fprintf(fp,"j=%d, NodePosition_2[S]=%d\n",j,NodePosition_2[S]); 
           }
		   //fprintf(fp,"j=%d, recurid[j]=%d\n",j, recurid[j]);
        }

        float temp; //if set interval>1, then no vertical overlay
        int x0, y0, x1, y1;
        int leftcount,rightcount;
        int length,centerx;
        length=1000,centerx=0;

        leftcount=1000000, rightcount=-10000000;
        //for(vector< vector <int> >::iterator it=++PairQ.begin(); it!=PairQ.end(); it++)
		for(int i=0; i<branchNum; i++)//
        {
                //vector <int> point=*it;
			    vector <int> point=PairQ[i];
                if(ContTree[point[0]].Pro[0]==1)
                {
                    int start=point[0], end=point[1];

                    if(NodePosition_2[end]<leftcount)
                         leftcount=NodePosition_2[end];

                    if(NodePosition_2[end]>rightcount)
                        rightcount=NodePosition_2[end];

                    x0=NodePosition_2[start]+centerx;
                    temp=ContTree[point[0]].value-fMin;
                    y0=length*temp/(fMax-fMin);

                    x1=NodePosition_2[end]+centerx;
                    temp=ContTree[point[1]].value-fMin;
                    y1=length*temp/(fMax-fMin);

                    NodePosition[end][0]=x1;
                    NodePosition[end][1]=y1;
                    NodePosition[end][2]=point[1];

                    NodePosition[start][0]=x0;
                    NodePosition[start][1]=y0;
                    NodePosition[start][2]=point[0];  //its node is in CT
                    if(NodePosition_1[end][0]<NodePosition_1[start][0]) //left
                        // NodePosition[start].push_back(NodePosition_1[start][1]>NodePosition_1[start][2]?NodePosition_1[start][1]:NodePosition_1[start][2]);   //most left node if it it at left, its most right node if it is at right
					    NodePosition[start][3]=NodePosition_1[start][1]>NodePosition_1[start][2]?NodePosition_1[start][1]:NodePosition_1[start][2];   //most left node if it it at left, its most right node if it is at right
                    else 
						NodePosition[start][3]=NodePosition_1[start][1]>NodePosition_1[start][2]?NodePosition_1[start][1]:NodePosition_1[start][2];   //most left node if it it at left, its most right node if it is at right
					    //NodePosition[start].push_back(NodePosition_1[start][3]>NodePosition_1[start][4]?NodePosition_1[start][3]:NodePosition_1[start][4]);   //most left node if it it at left, its most right node if it is at right
					
					//fprintf(fp,"i=%d, start=%d, end=%d, NodePosition[start][0]=%d, NodePosition[end][0]=%d\n", i, start, end, NodePosition[start][0],NodePosition[end][0]);
                }
                else if(ContTree[point[0]].Pro[0]==2)
                {
                   int start,N;
                   N=PairQ[point[3]][1];  //its pole end
                   start=point[0];
                   x0=NodePosition_2[N]+centerx;
                   temp=ContTree[point[0]].value-fMin;
                   y0=length*temp/(fMax-fMin);

                   NodePosition[start][0]=x0;
                    NodePosition[start][1]=y0;
                   NodePosition[start][2]=point[0];
                }
        }
        Height=length;
        Width=rightcount-leftcount+1;
        Center=-leftcount;

        //first pair
        NodePosition[0][0]=centerx;
        NodePosition[0][1]=-10;
        NodePosition[0][2]=0;
        //NodePosition[0].push_back(0);
		NodePosition[0][3]=0;

        NodePosition[PairQ[0][1]][0]=centerx;
        NodePosition[PairQ[0][1]][1]=length;
        NodePosition[PairQ[0][1]][2]=PairQ[0][1];
        //NodePosition[PairQ[0][1]].push_back(0);
		NodePosition[PairQ[0][1]][3]=0;
        //DrawABranch(centerx, -10, centerx,length,RGB(0,0,0));

		//delete [] NodePosition_2;
		//free_i2vector(NodePosition_1,MSCriNum,9); 
		
		//fclose(fp);
        NodePosition_2.clear();
        NodePosition_1.clear();
}
vector<vector<vector<int>>> LocateBoxPosition(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int branchNum, int Width, int Center, int scale)
//vector<vector<vector<int>>> LocateBoxPosition(struct ContourTreeNode * ContTree, int ** PairQ, int ** NodePosition, int branchNum, int Width, int Center, int scale)
//vector< vector <int> > LocateBranchPosition(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int Width, int Center)
{
   //vector<int> list;
   //int centerx=I*DX*1.5;
	//struct Box_str **branchesBox; 
	vector<vector<vector<int>>> branchesBox; 
	float x0,y0,x1,y1;
	/*branchesBox= new struct Box_str *[scale];
	branchesBox[0]= new struct Box_str [scale*scale];
	for(int i=1;i<10;i++)
	    branchesBox[i] = branchesBox[i-1] + scale;
	*/
	branchesBox.resize(scale);
	for(int i=0;i<scale;i++)
		branchesBox[i].resize(scale);

	int **count;
	count= new int *[scale];
	count[0]= new int [scale*scale];
	for(int i=1;i<10;i++)
	    count[i] = count[i-1] + scale;

	/*for(int i=0;i<10;i++)
	for(int j=0;j<10;j++)
	{
	    //branchesBox[i][j].num=0;	
		//count[i][j]=0;
	}*/

    for(int i=0; i< branchNum; i++)
    {
       //int *point=PairQ[i];
	   x0=NodePosition[PairQ[i][0]][0]+Center;
       y0=NodePosition[PairQ[i][0]][1];

       x1=NodePosition[PairQ[i][1]][0]+Center;
       y1=NodePosition[PairQ[i][1]][1];	  
   
       int inter_y,inter_x;
  	   inter_x=float(Width)/scale-int(float(Width)/scale)>0?int(float(Width)/scale)+1:int(float(Width)/scale);
       inter_y=1000.0/scale-int(1000.0/scale)>0?int(1000.0/scale)+1:int(1000.0/scale);

	   int ix0,ix1,iy0,iy1,temp;
	   ix0=x0/inter_x;  if(ix0>=scale) ix0=scale-1;   
	   ix1=x1/inter_x;  if(ix1>=scale) ix1=scale-1;   

	   iy0=y0/inter_y;  if(iy0>=scale) iy0=scale-1;   
	   iy1=y1/inter_y;  if(iy1>=scale) iy1=scale-1;   
			  
 	   if(ix0>ix1) temp=ix0, ix0=ix1, ix1=temp;
	   if(iy0>iy1) temp=iy0, iy0=iy1, iy1=temp;

	   // 1. x0 y0 - x1 y0
	   for(int k=ix0; k<=ix1; k++)
	   {
	       //branchesBox[k][iy0].num++;
		   branchesBox[k][iy0].push_back(0);
	   }
	   // 2. x1 y0 - x1 y1
	   for(int j=iy0; j<=iy1; j++)
	   {
		   //branchesBox[ix1][j].num++;
		   branchesBox[ix1][j].push_back(0);
	   }	 
   }

	for(int i=0;i<scale;i++)
	for(int j=0;j<scale;j++)
	{
	    //branchesBox[i][j].node=new int [branchesBox[i][j].num];	
		//branchesBox[i][j].resize(branchesBox[i][j]);
		//keqin ???
		count[i][j]=0;
	}
  
   for(int i=0; i< branchNum; i++)
   {
       //int *point=PairQ[i];
	   x0=NodePosition[PairQ[i][0]][0]+Center;
       y0=NodePosition[PairQ[i][0]][1];

       x1=NodePosition[PairQ[i][1]][0]+Center;
       y1=NodePosition[PairQ[i][1]][1];
   
       int inter_y,inter_x;
  	   inter_x=float(Width)/scale-int(float(Width)/scale)>0?int(float(Width)/scale)+1:int(float(Width)/scale);
       inter_y=1000/scale-int(1000/scale)>0?int(1000/scale)+1:int(1000/scale);

	   int ix0,ix1,iy0,iy1,temp;
	   ix0=x0/inter_x;  if(ix0>=scale) ix0=scale-1;   
	   ix1=x1/inter_x;  if(ix1>=scale) ix1=scale-1;   

	   iy0=y0/inter_y;  if(iy0>=scale) iy0=scale-1;   
	   iy1=y1/inter_y;  if(iy1>=scale) iy1=scale-1;   
			  
 	   if(ix0>ix1) temp=ix0, ix0=ix1, ix1=temp;
	   if(iy0>iy1) temp=iy0, iy0=iy1, iy1=temp;

	   // 1. x0 y0 - x1 y0
	   for(int k=ix0; k<ix1; k++)
	   {
	       //branchesBox[k][iy0].node[count[k][iy0]]=i;
		   branchesBox[k][iy0][count[k][iy0]]=i;
		   count[k][iy0]++;
	   }
	   // 2. x1 y0 - x1 y1
	   for(int j=iy0; j<=iy1; j++)
	   {
		   //branchesBox[ix1][j].node[count[ix1][j]]=i;
		   int sizex=branchesBox[ix1][j].size();
		   int sizec=count[ix1][j];
		   branchesBox[ix1][j][count[ix1][j]]=i;
		   count[ix1][j]++;
	   }	 
   }
 
   delete [] count[0];
   delete [] count;
   return branchesBox;
}

vector<vector<int>> LocateNodePosition(struct ContourTreeNode * &ContTree, struct MorseSmaleCriticalPoint *MSCpoint, int MSCriNum, int branchNum, vector< vector <int> > NodePosition, float fMin, float fMax)
//int ** LocateNodePosition(struct ContourTreeNode * &ContTree, struct MScripoint *MSCpoint, int MSCriNum, int ** NodePosition, float fMin, float fMax)
//vector< vector <int> > LocateNodePosition(struct ContourTreeNode * &ContTree, struct MScripoint *MSCpoint, int MSCriNum, vector< vector <int> > NodePosition, float fMin, float fMax)
{
   int height=sqrt(float(MSCriNum))+2;
   //vector<int> list;
   //int list;
   float inter;
   int left=100000,right=-100000,up=-1000000;
   inter=(fMax-fMin)/height;
   //vector<vector <int> > locationArray(height,list);
   //int count=0;
   //int **locationArray=i2vector(height,Width);
   vector<vector<int>> locationArray;//=new struct MSC_str[height+1];
   locationArray.resize(height+1);
   //locationArray[0].node=new int [1];
   locationArray[0].resize(1);

   int *count=new int [height];
   for(int i=0;i<height; i++)
	   count[i]=0;

  

   for(int i=1; i<MSCriNum; i++)
   {
           int j=int((ContTree[i].value-fMin)/inter);
           if(j>=height)
               j=height-1;
		   count[j]++;
   }
   for(int i=0;i<height; i++)
   {
	   //locationArray[i].node=new int [count[i]+1];
	   locationArray[i].resize(count[i]+1);
	   locationArray[i][0]=count[i];
	   //locationArray[i].node[0]=count[i];
	   count[i]=1;
   }
   //locationArray[height].node=new int [4];
   locationArray[height].resize(4);

    

	
   for(int i=0; i<MSCriNum; i++)
   {
	       //fprintf(fp,"PairQ[0][0]%d,NodePosition[PairQ[0][0]][0]%d",PairQ[0][0],NodePosition[PairQ[0][0]][0]);		   
           int j;
           if(i==0)  
			   j=0;
           else
           {
                j=int((ContTree[i].value-fMin)/inter);
                if(j>=height)
                    j=height-1;
           }
		  
		   //fprintf(fp,"j=%d,count[j]=%d,NodePosition[i][0]=%d\n",j,count[j], NodePosition[i][0]);
		   //if(i==69)			  
		   //{
		   //	fprintf(fp,"locationArray[j].node[count[j]]=%d\n",locationArray[j].node[count[j]]);
		   //	fclose(fp);
		   // }
           locationArray[j].push_back(i);		   
		   //locationArray[j].node[count[j]]=i;		   
		   //fprintf(fp,"j=%d,count[j]=%d,NodePosition[i][0]=%d\n",j,count[j], NodePosition[i][0]);
		   //if(i==69)			  
		   //{
			//   fprintf(fp,"i=%d,locationArray[j].node[count[j]]=%d\n",i,locationArray[j].node[count[j]]);
			   //fclose(fp);
		  // }

		   count[j]++;
           if(NodePosition[i][0]<=left)  left=NodePosition[i][0];
           else if(NodePosition[i][0]>=right)  right=NodePosition[i][0];
           if(NodePosition[i][1]>=up)  up=NodePosition[i][1];

		   //fprintf(fp,"j=%d,heightcount[j]=%d,NodePosition[i][0]=%d\n",j,count[j], NodePosition[i][0]);
		   
		  
   }
   vector<int> list;
   list.push_back(left),list.push_back(0),list.push_back(right),list.push_back(up);

   locationArray.push_back(list); 
   //locationArray[height].node[0]=left, locationArray[height].node[1]=0, locationArray[height].node[2]=right, locationArray[height].node[3]=up;

 

   return locationArray;
    //return NULL;
}
vector<vector<int>> LocateBranchPosition(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int branchNum, int Width, int Center)
//vector<vector<int>> LocateBranchPosition(struct ContourTreeNode * ContTree, int ** PairQ, vector< vector <int> > NodePosition, int branchNum, int Width, int Center)
//vector< vector <int> > LocateBranchPosition(struct ContourTreeNode * ContTree, vector< vector <int> > PairQ, vector< vector <int> > NodePosition, int Width, int Center)
{
   //vector<int> list;
   //int centerx=I*DX*1.5;
   if(Width<0)
	   Width=branchNum; //maybe a bug here
   if(Width<=0)
	   Width=1;

   vector<vector<int>> branchLocationArray;//= new struct MSC_str [Width]; 
   branchLocationArray.resize(Width);
   int *count= new int [Width];
   //vector<vector <int> > branchLocationArray(Width,list);
    /*QString output= "output3.txt";
		QByteArray ba = output.toLocal8Bit();  
		FILE *fp = fopen(ba.data(),"w");
		fprintf(fp,"Width=%d\n",Width);	
		fclose(fp);*/

   for(int i=0; i< Width; i++)
       count[i]=0;

   for(int i=0; i< branchNum; i++)
   {
       if(ContTree[PairQ[i][0]].Pro[0]==1&&ContTree[PairQ[i][1]].Pro[0]==1)              
           count[NodePosition[PairQ[i][1]][0]+Center]++;		  
   }

   for(int i=0; i< Width; i++)
   {
	    //branchLocationArray[i].node=new int [count[i]+1];
	   branchLocationArray[i].resize(count[i]+1);
	   int sizei=count[i]+1;
	   //branchLocationArray[i].push_back(count[i]);
		branchLocationArray[i][0]=count[i];
	    count[i]=0;		
   }

   for(int i=0; i< branchNum; i++)
   {
       if(ContTree[PairQ[i][0]].Pro[0]==1&&ContTree[PairQ[i][1]].Pro[0]==1)              
	   {
		   int id=NodePosition[PairQ[i][1]][0]+Center;
		   //branchLocationArray[id].node[count[id]]=i;
		   branchLocationArray[id][count[id]]=i;
		   count[id]++;
	   }
   }
   return branchLocationArray;
   //return NULL;
}
/*
int *FindBranchBox(float x, float y, struct ContourTreeNode * ContTree, struct MScripoint *MSCpoint, struct MSC_str * branchLocationArray, int ** NodePosition, int ** PairQ, float ***Vs, struct MSCsep_str * MSCsep, int ** MSCsepD, int Width, float fMin)
{
    if(nArraySize==0) return -1;
	
	float left=locationArray[nArraySize].node[0],down=locationArray[nArraySize].node[1],right=locationArray[nArraySize].node[2], up=locationArray[nArraySize].node[3];
    int inter=10;

    if(x<left-inter||x>right+inter||y<down-inter||y>up+inter)
           return(-1);

	int height=(y-down)/(up-down)*(nArraySize);
	int grade=int(nArraySize*0.05)+1;
	
	for(int i=height-grade; i<=height+grade; i++)
    {
           if(i>=0&&i<nArraySize-1)
           {
				for(int j=1; j<=locationArray[i].node[0]; j++)  //assum locationarray[i][0] store the number of the rest item
                {
                	int k=locationArray[i].node[j];
					X=NodePosition[k][0], Y=NodePosition[k][1];
                    temp=pow(float(X-x)/Width*1000,2)+pow(float(Y-y)/Height*1000,2);
                    if(temp<radii)
                    {
                         if(temp<min) min=temp, mini=NodePosition[k][2];
                    }
                }
           }
     }     
     return(mini);
}*/

int FindContTreeNode(float x, float y, vector<vector<int>>  locationArray, vector<vector<int>> NodePosition,int nArraySize, int Width, int Height, char * edgeCoordinator, struct ContourTreeNode * &ContTree) //found clicked node
{
	    
	    if(nArraySize==0)
			return -1;

        int X,Y,temp,inter=20,radii=400,min,mini=-1;
        float left=locationArray[nArraySize][0],down=locationArray[nArraySize][1],right=locationArray[nArraySize][2], up=locationArray[nArraySize][3];

		
        if(x<left-inter||x>right+inter||y<down-inter||y>up+inter)
           return(-1);

       	int height=(y-down)/(up-down)*(nArraySize);//,saddle,end;

        min=100000;
        for(int i=height-2; i<=height+2; i++)
        {
                if(i>=0&&i<nArraySize-1)
                {
					    for(int j=1; j<=locationArray[i][0]; j++)  //assum locationarray[i][0] store the number of the rest item
                        {
                    		int k=locationArray[i][j];
							X=NodePosition[k][0], Y=NodePosition[k][1];
							temp=pow(float(X-x)/Width*1000,2)+pow(float(Y-y)/Height*1000,2);                       
                            if(temp<radii&&(edgeCoordinator[NodePosition[k][2]]==1||ContTree[NodePosition[k][2]].Pro[0]==2))
                            {
                                if(temp<min) min=temp, mini=NodePosition[k][2];
                            }
                        }
                }
        }
        return(mini);
}


int * FindAugmemtedContTreeNode(float x, float y, struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint,  vector< vector <int> > branchLocationArray, vector< vector <int> > NodePosition, vector< vector <int> > PairQ, float ***Vs, vector< vector< vector <int> > > MSCsep, vector<vector<int>> MSCsepD, vector<vector<vector<int>>>  BranchSep, int Width, float fMin)
//int *FindAugmemtedContTreeNode(float x, float y, struct ContourTreeNode * ContTree, struct MorseSmaleCriticalPoint *MSCpoint, vector< vector <int> > branchLocationArray, int ** NodePosition, int ** PairQ, float ***Vs, vector< vector < vector <int> > > BranchSep, vector<vector<int>> MSCsepD, int Width, float fMin)
//vector <int> FindAugmemtedContTreeNode(float x, float y,struct Tree_str * ContTree, struct MScripoint *MSCpoint, vector<vector <int> > branchLocationArray, vector<vector <int> > NodePosition, vector< vector <int> > PairQ, float ***Vs, vector< list < vector <int> > > MSCsep, vector< vector <int> > MSCsepD, float fMin)
{
	    //vector<int> vect(3,-1);
	    int *vect=new int [5];
	    vect[0]=vect[1]=vect[2]=vect[3]=vect[4]=0;
    	
	    if(Width==0)
             return NULL;

        if(x<-0.5||x>=Width+0.5)
             return NULL;
	
        //find  branch first then node
        int id,ID=int(x+0.5),bid;
        //int bid;
        int saddle, end;
		//for(int i=1; i<=branchLocationArray[ID].node[0]; i++)
		for(int i=0; i<=branchLocationArray[ID].size(); i++)
        {
            saddle=PairQ[branchLocationArray[ID][i]][0], end=PairQ[branchLocationArray[ID][i]][1];
			bid=branchLocationArray[ID][i];
            id=saddle;
			float svalue,evalue,temp1,temp2,value;
		    if((y-NodePosition[saddle][1])*(y-NodePosition[end][1])<=0)
            {
			   svalue=ContTree[saddle].value; 
			   evalue=ContTree[end].value;
               if(saddle!=0)
                  value=(y-NodePosition[saddle][1])*(evalue-svalue)/(NodePosition[end][1]-NodePosition[saddle][1])+svalue;
               else value=(evalue-fMin)*(y-0)/(NodePosition[end][1]-0)+fMin;

			   temp2=BranchSep[bid][1][0]>-1?Vs[BranchSep[bid][1][0]][BranchSep[bid][1][1]][BranchSep[bid][1][2]]:-100000;
               for(int i=1;i<BranchSep[bid][0].size();i++)
               {   
				   temp1=temp2;
                   temp2=BranchSep[bid][i][0]>-1?Vs[BranchSep[bid][i][0]][BranchSep[bid][i][1]][BranchSep[bid][i][2]]:-100000;
                   if((value-temp1)*(value-temp2)<=0)
             	   {
					   if(fabs(value-temp2)<fabs(value-temp1))
					   {
						   vect[0]=BranchSep[bid][i][0],
					  vect[1]=BranchSep[bid][i][1],
					  vect[2]=BranchSep[bid][i][2],
					  vect[3]=temp2,
					  vect[4]=bid;
					   }
					   else
						{
						   vect[0]=BranchSep[bid][i-1][0],
					  vect[1]=BranchSep[bid][i-1][1],
					  vect[2]=BranchSep[bid][i-1][2],
					  vect[3]=temp1,
					  vect[4]=bid;
					   }
					   break;

				   }

			   }
			   
			   break;
            }
	   }
       return vect;         
}

void LocateBranchPoints(struct ContourTreeNode * ContTree, vector<vector<int>> PairQ, vector<vector<vector<int>>>  BranchSep, vector<vector<int>> MSCsepD, float ***Vs, int branchNum, float ** &seeds, float fMin)
{

   for(int i=0; i<branchNum; i++)
   {
       /*int sepid=findSep(ContTree, MSCsepD, PairQ[i][0], PairQ[i][1]);    
	   if(sepid==-1)
	       break;

	   float value=i==0?ContTree[PairQ[i][1]].value/2.0:(ContTree[PairQ[i][0]].value+ContTree[PairQ[i][1]].value)/2.0;
	   int *vect=findMidNode(MSCsep, MSCsepD, Vs, sepid, value);*/
	   
	   int saddle=PairQ[i][0],end=PairQ[i][1];
	   float temp1,temp2;
	   int *vect[3];
	   float value=i==0?(ContTree[end].value-fMin)/2.0:(ContTree[saddle].value+ContTree[end].value)/2.0;	  
       {
			float temp2=BranchSep[i][1].size()!=0?Vs[BranchSep[i][1][0]][BranchSep[i][1][1]][BranchSep[i][1][2]]:-100000;
            for(int j=1;j<BranchSep[i].size();j++)
            {   
				 if(i>=9)
					i=i;
				 int sizeB = BranchSep[i].size();
			     temp1=temp2;
                 temp2=BranchSep[i][j].size()!=0?Vs[BranchSep[i][j][0]][BranchSep[i][j][1]][BranchSep[i][j][2]]:-100000;
                 if((value-temp1)*(value-temp2)<=0)
                 {
					 if(fabs(value-temp2)<fabs(value-temp1))
					 {
						 seeds[i][0]=BranchSep[i][j][0];
					  seeds[i][1]=BranchSep[i][j][1];
					  seeds[i][2]=BranchSep[i][j][2];
					  seeds[i][3]=temp2;
					 }
					 else
					 {
						 seeds[i][0]=BranchSep[i][j-1][0];
					  seeds[i][1]=BranchSep[i][j-1][1];
					  seeds[i][2]=BranchSep[i][j-1][2];
					  seeds[i][3]=temp1;
					 }
					 break;

			     }
			}	
       }	      
   }
           
}


/*int findSep(struct Tree_str * ContTree, int **MSCsepD, int saddle, int end)
//vector <int> FindAugmemtedContTreeNode(float x, float y,struct Tree_str * ContTree, struct MScripoint *MSCpoint, vector<vector <int> > branchLocationArray, vector<vector <int> > NodePosition, vector< vector <int> > PairQ, float ***Vs, vector< list < vector <int> > > MSCsep, vector< vector <int> > MSCsepD, float fMin)
{	
	 int id=saddle,sepid=-1;
     float y=(ContTree[saddle].value+ContTree[end].value)/2.0;
	 int flag=0;
	 if(ContTree[saddle].value<ContTree[end].value)
     {
	      while(ContTree[id].High>0)
          {
	           for(int j=0; j<ContTree[id].High; j++)
               {
			         int dj=ContTree[id].HighIn[j];
                     if(ContTree[dj].Pro[1]==0)
                     {
                          if(ContTree[dj].value>=y)
                          {
                                flag=1;
                                if(MSCsepD[ContTree[dj].LowInSep[0]][0]==id||MSCsepD[ContTree[dj].LowInSep[0]][1]==id)
                                {
                                    sepid=ContTree[dj].LowInSep[0];
                                }
                                else
                                {
                                    sepid=ContTree[dj].LowInSep[1];
                                }
                         }
                         id=dj;
                         break;
                      }
                 }
                 if(flag)
                   break;
             }         
	 }
	 else
     {
          while(ContTree[id].Low>0)
          {
              //for(set<int>::iterator it=ContTree[id].LowIn.begin(); it!=ContTree[id].LowIn.end(); it++)
			  for(int j=0; j<ContTree[id].Low; j++)
              {
			       int dj=ContTree[id].LowIn[j];
                   if(ContTree[dj].Pro[1]==0)
                   {
                       if(ContTree[dj].value<=y)
                       {
						   flag=1;
                          if(MSCsepD[ContTree[dj].HighInSep[0]][0]==id||MSCsepD[ContTree[dj].HighInSep[0]][1]==id)
                           {
                                sepid=ContTree[dj].HighInSep[0];
                           }
                           else
                           {
                                sepid=ContTree[dj].HighInSep[1];
                           }                           
                       }
                       id=dj;
                       break;
                   }
			  }
			  if(flag)
                 break;
          }
	 }  // particular
	 return sepid;
}
*/

int *findMidNode(struct MSCsep_str * MSCsep, vector<vector<int>> MSCsepD, float ***Vs, int sepid, float value)
{
	   int *vect=new int [4];
	   vect[0]=vect[1]=vect[2]=vect[3]=0;
	   for(int i=1;i<=MSCsep[sepid].sep[0][0];i++)
	   {
           	 float temp=MSCsep[sepid].sep[i][0]>-1?Vs[MSCsep[sepid].sep[i][0]][MSCsep[sepid].sep[i][1]][MSCsep[sepid].sep[i][2]]:-100000;
             if((MSCsepD[sepid][2]>0&&value<temp)||(MSCsepD[sepid][2]<0&&value>temp))
             {
                   if(i!=1)
                   {
                          i--;
                          if(fabs(value-temp)<fabs(value-MSCsep[sepid].sep[i][0]>-1?Vs[MSCsep[sepid].sep[i][0]][MSCsep[sepid].sep[i][1]][MSCsep[sepid].sep[i][2]]:-1000000))
						  {
							     return MSCsep[sepid].sep[i+1];
						  }
                          else 
						  {
							  
						     return MSCsep[sepid].sep[i];
						  }
                   }
                   else vect=MSCsep[sepid].sep[i];
            }

         }		 
         return vect;         
}
