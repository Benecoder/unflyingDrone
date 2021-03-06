#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "NeuralNet.h"

int LrEpoch,x,y,z;
int Seeds[3];
int NoS = 40000;

int main(){
	//Reading the binary file
	FILE *In_ptr;
	FILE *Out_ptr;

	In_ptr = fopen("In2.bin","rb");
	Out_ptr = fopen("Out2.bin","rb");

	if(!In_ptr) printf("READING INDATA FAILED!!\n");
	if(!Out_ptr) printf("READING OUTDATA FAILED!!\n");

	double _In[NoS*3];
	double _Out[NoS*4];

	fread(&_In,sizeof(double),NoS*3,In_ptr);
	fread(&_Out,sizeof(double),NoS*4,Out_ptr);

	fclose(In_ptr);
	fclose(Out_ptr);
	
	
	//Formating the Data
	double In[NoS][3];
	double Out[NoS][4];
	
	for(x=0;x<NoS;x++){
		for(y=0;y<3;y++){
			In[x][y] = _In[(x*3)+y];
		}
		for(y=0;y<4;y++){
			Out[x][y] = _Out[(x*4)+y];
		}
	}
	
	
	//Prepares target file
	FILE *result;	
	int Neurons,LR;
	double value = 0,LearningRate,Percentage = 0;
	
	for(Neurons = 2;Neurons<20;Neurons ++){
	
		result = fopen("Result3.bin","ab");
		if(!result) printf("OPENING TARGET FILE FAILED!!!\n");
		
		Seeds[0] = clock();
		Seeds[1] = clock()+500;
		Seeds[2] = clock()-500;
		
		for(LR = -5;LR>(-15);LR -- ){
		
			LearningRate = (-1.0)*pow(10.0,(double) LR);
		
			value = 0;
		
			for(x = 0;x<3;x++){
				SetupNetwork(LearningRate,Neurons,Seeds[x]);
				
				for(y=0;y<3000;y++){

					for(z = 0;z<NoS;z++){
					
						forward(In[z]);
						backward(Out[z]);
					
					}
					
					CalcCost();
					correct(NoS);
					
					Percentage += 0.00018519;
					
					printf("Cost: %f		Try:%d		LR: %d		Neurons:%d		Percentage:%f\n",Cost,x,-LR,Neurons,Percentage);
					
				}
				
				value += Cost;
			
			}
			
			value /= 3;
			
			fwrite(&value,sizeof(double),1,result);
				
			value = 0;
			
		}
		
		fclose(result);
	}
	

	return 0;
}
