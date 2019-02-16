#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "NeuralNet3L2.h"

int LrEpoch,x,y,z;
int Seeds[] = {4,5,6};
int Length = 40000;

int main(){
	//Reading the binary file
	FILE *In_ptr;
	FILE *Out_ptr;

	In_ptr = fopen("In.bin","rb");
	Out_ptr = fopen("Out.bin","rb");

	if(!In_ptr) printf("READING INDATA FAILED!!\n");
	if(!Out_ptr) printf("READING OUTDATA FAILED!!\n");

	double _In[Length*L1];
	double _Out[Length*L3];

	fread(&_In,sizeof(double),Length*L1,In_ptr);
	fread(&_Out,sizeof(double),Length*L3,Out_ptr);

	fclose(In_ptr);
	fclose(Out_ptr);
	
	
	//Formating the Data
	double In[Length][L1];
	double Out[Length][L3];
	
	for(x=0;x<Length;x++){
		for(y=0;y<L1;y++){
			In[x][y] = _In[(x*L1)+y];
		}
		for(y=0;y<L3;y++){
			Out[x][y] = _Out[(x*L3)+y];
		}
	}

	//Prepares target file
	FILE *result;
	
	result = fopen("Result.bin","ab");
	

	//Testing
	printf("Testing for %d Hidden Layers\n",L2);
	
	for(LrEpoch=0;LrEpoch<5;LrEpoch++){
		double Value = 0;
		for(x=0;x<3;x++){
			double LearningRate = -1*pow(10,-5-LrEpoch);
			SetupNetwork(Seeds[x],LearningRate);
		
			for(y=0;y<2000;y++){
				for(z=0;z<Length;z++){
					forward(In[z]);
					backward(Out[z]);
				}
				
				CalcCost();
				Correct(Length);
			}
			Value += Cost;
			printf("Lr: %d		Try:%d	\n",LrEpoch,x);
		}
		Value /= 3;
		fwrite(&Value,sizeof(double),1,result);
	}
	fclose(result);

	return 0;
}
