/*

- Using the preproccesed input Data
- Learning based on PD Controller

*/
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pigpio.h>
#include <string.h>
#include <assert.h>
#include "Sensors.h"
#include "Motor.h"
#include "NeuralNet2.h"

#define DC 0.0003
#define PI 3.141592653589793

//GLobal Variabels
int NoS = 40000;  //When Changing NOS, change Size of InData and OutData accordingly
double Data[3] ={0};
double InData[40000][3] = {{0.0}},OutData[40000][4]={{0.0}};
double ScaledInData[3],ScaledOutData[4];

double scale(double content,double factor) {
	return ((2*content)/factor)-1.0;
}


double InScale(double  content,double factor) {
	return (content+1.0)*(factor/2.0);
}

void ReadData(file){
	double Gyrobuffer = GyroRead('p',file);
	Data[0] += (Gyrobuffer*DC)*(180.0/PI)-0.002;
	Data[1] = AccelRead('p',file);
	Data[2] = Gyrobuffer;

	ScaledInData[0] = Data[0]/60.0;
	ScaledInData[1] = Data[1]/60.0;
	ScaledInData[2] = Data[2]/80.0;
}

int main(){
	int i,j=0,n,x=0;


	//Setup for the Motors
	int  Settings[4]={0};
	int low[4] = {0};
	start();

	//Setup for the Data Input
	int file = setupSensors();

	//Setup for the Neural Network
	SetupNetwork();

	srand(5);

	//Gathering the Data
	while(j<14){
		Data[0] = AccelRead('p',file);

		for(n=0;n<(int)(NoS/14);n++){

			ReadData(file);

			double Delta,D;

			D = 0.02*Data[1] + 0.98*Data[0];

			Delta = D*0.1+Data[2]*0.09+0.3;

			Settings[0] = 40.0 +  Delta;
			Settings[1] = 40.0 +  Delta;
			Settings[2] = 40.0 -  Delta;
			Settings[3] = 40.0 -  Delta;


			for(i=0;i<4;i++) ScaledOutData[i] = ((Settings[i]-41.0)/10.0);

			//Sends Motor Signal
			SetMotors(Settings);

			//Set Recordings
			InData[x][0] = (double)ScaledInData[0];
			InData[x][1] = (double)ScaledInData[1];
			InData[x][2] = (double)ScaledInData[2];

			for(i=0;i<4;i++){
				OutData[x][i] = ScaledOutData[i];
				printf("%d	",Settings[i]);
			}
			printf("\n");
//			printf("ScaledGyro:%f	Gyro: %f			ScaledAccel:%f	Accel:%f	\n",InData[x][0],Data[0],InData[x][1],Data[1]);
			x++;
		}

		printf("-------------------reseting!-----------------\n");
		SetMotors(low);
//		sleep(3);

		j++;

	}
	end();


	//Backpropagation
	printf("---------------------Backpropagating---------------------\n");

	double LearningRate = -0.04,delta = 0;
	i = 0;
	int sampleSize = 20000,index;

	while(i <= 500 && Cost > 0.001 && delta > -0.2){

		index = (int) (NoS-sampleSize)*(rand()/RAND_MAX);

		for(j=index;j<(index+sampleSize);j++){
			forward(InData[j]);
			backward(OutData[j]);
		}

		correct(sampleSize,LearningRate);

		if(i%20 == 0){
			delta = Cost;

			CalcCost(1);

			for(j=0;j<NoS;j++){
				forward(InData[j]);
				backward(OutData[j]);
			}

			CalcCost(NoS);

			delta -= Cost;

			correct(NoS,LearningRate);
			printf("Cost:%f		Improvment:%f		Index:%d\n",Cost,delta,i);
		}
	i++;
	}

	printf("---------------------Sleeping-----------------------------\n");

	sleep(10);

	printf("--------------------------USING NEURAL NETWORK-----------------\n");

	int running = 1;
	start();

	for(i=0;i<500;i++){
		Data[0] = AccelRead('p',file);
		running = 1;
		while(running){
			ReadData(file);

			forward(ScaledInData);

			for(j=0;j<4;j++){
				double value = 41.0+(OutLayer[3][j]*10.0);
				printf("%f	",value);
				Settings[j] = (int) value;
			}
			printf("\n");
			SetMotors(Settings);

//			printf("|M1:%f	M2:%f	M3:%f	M4:%f\n",OutLayer[Length-1][0],OutLayer[Length-1][1],OutLayer[Length-1][2],OutLayer[Length-1][3]);
			if(fabs(Data[0])> 50.0) running = 0;
		}
		SetMotors(low);
		sleep(4);
	}
	end();
	return 0;
}
