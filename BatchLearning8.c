/*

- Using the raw input Data
- Learning based on PD Controller
- Stores the trained Network

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
#include "NeuralNet3.h"

#define DC 0.0003
#define PI 3.141592653589793

//GLobal Variabels
int NoS = 40000;  //When Changing NOS, change Size of InData and OutData accordingly
double Data[3] ={0.0};
double InData[40000][3] = {{0.0}},OutData[40000][4]={{0.0}};
double ScaledInData[3],ScaledOutData[4];

double scale(double content,double factor) {
	return ((2*content)/factor)-1.0;
}


double InScale(double  content,double factor) {
	return (content+1.0)*(factor/2.0);
}
void ReadData(file){

	RawAccel(file);
	double Gyrobuffer = GyroRead('p',file);
	Data[0] += (Gyrobuffer*DC)*(180/PI)-0.002;
	Data[1] = AccelRead('p');
	Data[2] = Gyrobuffer;


	ScaledInData[0] = Gyrobuffer/80.0;
	ScaledInData[1] = yAccel;
	ScaledInData[2] = zAccel;
}

void RawData(file){

	RawAccel(file);

	ScaledInData[0] = GyroRead('p',file)/80.0;
	ScaledInData[1] = yAccel;
	ScaledInData[2] = zAccel;
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
		RawAccel(file);
		Data[0] = AccelRead('p');

		for(n=0;n<(int)(NoS/14);n++){

			ReadData(file);

			double Delta,D;

			D = 0.02*Data[1] + 0.98*Data[0];

			Delta = D*0.1+Data[2]*0.09+0.3;

			Settings[0] = 40 +  Delta;
			Settings[1] = 40 +  Delta;
			Settings[2] = 40 -  Delta;
			Settings[3] = 40 -  Delta;


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
		sleep(3);

		j++;

	}
	end();


	//Backpropagation
	printf("---------------------Backpropagating---------------------\n");

	double LearningRate = -0.08,delta = 0;
	i = 0;
	int sampleSize = 20000,index;

	while(i <= 400 && delta > -0.1){

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

	//Stores the Parameters
	//Opens the File
	FILE *Weights_ptr;
	FILE *Biases_ptr;

	Weights_ptr = fopen("Weights.bin","wb");
	Biases_ptr = fopen("Biases.bin","wb");

	int Layer,Neuron,LastNeuron;

	//Writes the Data
	for(Layer=1;Layer<Length;Layer++){
		for(Neuron=0;Neuron<Dimension[Layer];Neuron++){
			double value = Biases[Layer][Neuron];
			fwrite(&value,sizeof(double),1,Biases_ptr);
		}
	}

	for(Layer=1;Layer<Length;Layer++){
		for(Neuron=0;Neuron<Dimension[Layer];Neuron++){
			for(LastNeuron=0;LastNeuron<Dimension[Layer-1];LastNeuron++){
				double value = Weights[Layer][Neuron][LastNeuron];
				fwrite(&value,sizeof(double),1,Weights_ptr);
			}
		}
	}

	//Closes the Files
	fclose(Weights_ptr);
	fclose(Biases_ptr);

	printf("---------------------Sleeping-----------------------------\n");

	sleep(10);

	printf("--------------------------USING NEURAL NETWORK-----------------\n");

	int running = 1;
	start();

	n = 0;

	for(i=0;i<500;i++){
		running = 1;
		while(running){
			RawData(file);

			forward(ScaledInData);

			for(j=0;j<4;j++){
				double thrust = 41;
				double value = thrust +(OutLayer[3][j]*10.0);
//				printf("%f	",value);
				Settings[j] = (int) value;
			}
//			printf("\n");
			SetMotors(Settings);

			printf("|M1:%d	M2:%d	M3:%d	M4:%d\n",Settings[0],Settings[1],Settings[2],Settings[3]);
			if(fabs(Data[0])> 50.0) running = 0;
			n++;
		}
		SetMotors(low);
		sleep(4);
	}
	end();
	return 0;
}
