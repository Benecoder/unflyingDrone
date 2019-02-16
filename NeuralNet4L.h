#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define Max 30

int L1,L2,L3,L4;

double Cost= 100000.0;
double CostSum=0;
double Lr = -0.0000000000006;

int x,y;

double L1out[Max],L2out[Max],L3out[Max],L4out[Max];
double L2in[Max],L3in[Max],L4in[Max];

double wL2_[Max*Max],wL3_[Max*Max],wL4_[Max*Max];
double wL2[Max][Max],wL3[Max][Max],wL4[Max][Max];
double bL2[Max],bL3[Max],bL4[Max];

double True[Max] = {0};

double L2er[Max],L3er[Max],L4er[Max];
double L2wer[Max][Max],L3wer[Max][Max],L4wer[Max][Max];

double activation(double Input){
	return tanh(Input);
}

double activationDer(double Input){
	double value =  (double)1-pow(tanh(Input),2);
	return value;
}

void forward(double L1in[]){
	int x,y;

	//Layer 1
	for(x=0;x<L1;x++){
		L1out[x] = L1in[x];
	}

	//Layer 2
	for(x=0;x<L2;x++){
		L2in[x] = 0;
		for(y=0;y<L1;y++){
			L2in[x] += L1out[y]*wL2[x][y];
		}
	}
	for(x=0;x<L2;x++) {
		L2out[x] = activation(bL2[x]+L2in[x]);
	}

	//Layer 3
	for(x=0;x<L3;x++){
		L3in[x] = 0;
		for(y=0;y<L2;y++){
			L3in[x] += L2out[y]*wL3[x][y];
		}
	}

	for(x=0;x<L3;x++){
		L3out[x] = activation(bL3[x]+L3in[x]);
	}
	
	//Layer 4
	for(x=0;x<L4;x++){
		L4in[x] = 0;
		for(y=0;y<L3;y++){
				L4in[x] += L3out[y]*wL4[x][y];
		}
	}
	
	for(x=0;x<L4;x++){
		L4out[x] = activation(bL4[x]+L4in[x]);
	}
}

void CalcCost(){
	Cost = CostSum;
	CostSum =0;
}

void backward(double Motors[]){
	int i,j;

	double L2der[L2],L3der[L3],L4der[L4],value;

	for(i=0;i<4;i++){
		CostSum += pow(fabs(L4out[i]-Motors[i]),2)/2;
	}


	for(i=0;i<L2;i++){
		L2der[i] = activationDer(L2in[i]+bL2[i]);
	}

	for(i=0;i<L3;i++){
		L3der[i] = activationDer(L3in[i]+bL3[i]);
	}
	
	for(i=0;i<L4;i++){
		L4der[i] = activationDer(L3in[i]+bL3[i]);
	}

	//Backpropagating the Error
	//Layer 4
	for(i=0;i<L4;i++){
		L4er[i] += L4der[i] * (L4out[i]-Motors[i]);
	}
	for(i=0;i<L3;i++){
		for(j=0;j<L4;j++){
			L4wer[j][i] += L4er[j]*L3out[i];
		}
	}


	//Layer 3
	for(i=0;i<L3;i++){
		value = 0;
		for(j=0;j<L4;j++){
			value += L4er[j] * wL4[j][i];
		}
		value *= L3der[i];
		L3er[i] += value;
	}
	
	for(i=0;i<L3;i++){
		for(j=0;j<L2;j++){
			L3wer[i][j] += L3er[i]*L2out[j];
		}
	}
	

	//Layer 2
	for(i=0;i<L2;i++){
		value = 0;
		for(j=0;j<L3;j++){
			value += L3er[j]*wL3[j][i];
		}
		value *= L2der[i];
		L2er[i] += value;
	}
	for(i=0;i<L2;i++){
		for(j=0;j<L1;j++){
			L2wer[i][j] += L2er[i]*L1out[j];
		}
	}
}

void Correct(int index){
	int i,j;
	
	for(i=0;i<L4;i++){
		bL4[i] += (double)Lr*(L4er[i]/index);
		L4er[i] = 0;
	}
	
	for(i=0;i<L4;i++){
		for(j=0;j<L3;j++){
			wL4[i][j] = (double)Lr* (L4wer[i][j]/index);
			L4wer[i][j] = 0;
		}	
	}

	for(i=0;i<L3;i++){
		bL3[i] += (double)Lr*(L3er[i]/index);
		L3er[i] = 0;
	}

	for(i=0;i<L3;i++){
		for(j=0;j<L2;j++){
			wL3[i][j] +=(double) Lr*(L3wer[i][j]/index);
			L3wer[i][j] = 0;
		}
	}

	for(i=0;i<L2;i++){
		bL2[i] += (double)Lr*(L2er[i]/index);
		L2er[i] =0;
	}

	for(i=0;i<L2;i++){
		for(j=0;j<L1;j++){
			wL2[i][j] += (double)Lr*(L2wer[i][j]/index);
			L2wer[i][j] = 0;
		}
	}
}



void randomize(double (*list)[],int len){
	double value;
	int x;
	for(x=0;x<len;x++) {
		value =(double)1.0- 2.0*rand()/RAND_MAX;
		(*list)[x] = value;
	}
}

void SetupNetwork(double  LearningRate,int Dimensions[4],int seed){
	L1 = Dimensions[0];
	L2 = Dimensions[1];
	L3 = Dimensions[2];
	L4 = Dimensions[3];

	Lr = LearningRate;

	srand(seed);

	int i,j;

	randomize(&wL2_,L2*L1);
	randomize(&wL3_,L3*L2);
	randomize(&wL4_,L4*L2);
	
	randomize(&bL2,L2);
	randomize(&bL3,L3);
	randomize(&bL4,L4);
	
	for(i=0;i<L2;i++){
		for(j=0;j<L1;j++){
			wL2[i][j] = wL2_[(i*L1)+j];
		}
	}

	for(i=0;i<L3;i++){
		for(j=0;j<L2;j++){
			wL3[i][j] = wL3_[(i*L2)+j];
		}
	}
	
	for(i=0;i<L4;i++){
		for(j=0;j<L3;j++){
			wL4[i][j] = wL4_[(i*L3)+j];
		}
	}

}
