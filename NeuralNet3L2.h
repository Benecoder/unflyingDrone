#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define L1 3
#define L2 20
#define L3 4


int l1 = 3;
int l2 = 10;

double L1out[L1],L2out[L2],L3out[L3];
double L2in[L2],L3in[L3];

double wL2_[L2*L1],wL3_[L3*L2];
double wL2[L2][L1],wL3[L3][L2];
double bL2[L2],bL3[L3];
double Cost= 100000.0,CostSum=0,Lr = -0.00000000001;
double True[L3] = {0};

double L2er[L2],L3er[L3];
double L2wer[L2][L1],L3wer[L3][L2];

int Epoch = 0;

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
}

void CalcCost(){
	Cost = CostSum;
	CostSum =0;
}

void backward(double Motors[]){
	int i,j;

	double L2der[L2],L3der[L3],value;

	for(i=0;i<4;i++){
		CostSum += pow(fabs(L3out[i]-Motors[i]),2)/2;
	}


	for(i=0;i<L2;i++){
		L2der[i] = activationDer(L2in[i]+bL2[i]);
	}

	for(i=0;i<L3;i++){
		L3der[i] = activationDer(L3in[i]+bL3[i]);
	}

	//Backpropagating the Error
	//Layer 3
	for(i=0;i<L3;i++){
		L3er[i] += L3der[i] * (L3out[i]-Motors[i]);
	}
	for(i=0;i<L2;i++){
		for(j=0;j<L3;j++){
			L3wer[j][i] += L3er[j]*L2out[i];
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

void SetupNetwork(int seed, double LearningRate) {

	srand(seed);

	Lr = LearningRate;

	int i,j;

	randomize(&wL2_,L2*L1);
	randomize(&wL3_,L3*L2);

	randomize(&bL2,L2);
	randomize(&bL3,L3);

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

/*	// To train the Network

	double In[23];
	for(i=0;i<23;i++){
		In[i] = 0.5;
	}
	for(i=0;i<4;i++){
		True[i] = -0.1;
	}

	for(Epoch=0;Epoch<40;Epoch++){
		for(i=0;i<10;i++){
			forward(In);
			CalcCost();
			backward(True);
		}
		printf("M1:%f		M2:%f		M3:%f		M4:%f		Cost:%f\n",L3out[0],L3out[1],L3out[2],L3out[3],Cost);
		Correct(3);
	}*/

}
