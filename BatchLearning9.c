/*

- Using the raw input Data
- Learning based on PD Controller
- Includes the Thrst setting in the training
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
#include "NeuralNet4.h"

#define DC 0.0003
#define PI 3.141592653589793
#define NOS 40000

//GLobal Variabels
double Data[3] ={0.0};
double InData[NOS][4] = {{0.0}},OutData[NOS][4]={{0.0}};
double ScaledInData[4],ScaledOutData[4];

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
	double BaseThrust = 40.0;
	int low[4] = {0};
	start();

	//Setup for the Data Input
	int file = setupSensors();

	//Setup for the Neural Network
	SetupNetwork();

	srand(5);
	
	int parts = 10;

	//Gathering the Data
	while(j<parts){
		RawAccel(file);
		Data[0] = AccelRead('p');

		for(n=0;n<(int)(NoS/parts);n++){

			ReadData(file);

			double Delta,D;

			D = 0.02*Data[1] + 0.98*Data[0];

			Delta = (D * 0.2+Data[2]*0.3-Delta*0.2)+5;
			BaseThrust = 15.0*sin(x/1500.0)+20.0;
			printf("%d	|",(int)BaseThrust);

			Settings[0] = (int)BaseThrust +  Delta;
			Settings[1] = (int)BaseThrust +  Delta;
			Settings[2] = (int)BaseThrust -  Delta;
			Settings[3] = (int)BaseThrust -  Delta;
			ScaledInData[3] = (BaseThrust-20.0)/40.0;

			for(i=0;i<4;i++) ScaledOutData[i] = ((Settings[i]-20.0)/40.0);

			//Sends Motor Signal
			SetMotors(Settings);

			//Set Recordings
			for(i=0;i<4;i++) InData[x][i] = ScaledInData[i];

			for(i=0;i<4;i++){
				OutData[x][i] = ScaledOutData[i];
				printf("%d	",Settings[i]);
			}
//			printf("\n");
			printf("Gyro: %f	Accel:%f	Delta:%f\n",Data[0],Data[1],Delta);
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

	while(i <= 300 && delta > -0.5&&Cost>0.007){

		if(i%1 == 0){
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

	Weights_ptr = fopen("TWeights.bin","wb");
	Biases_ptr = fopen("TBiases.bin","wb");

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

			BaseThrust = 15.0*sin(x/1500.0)+20.0;

			RawData(file);
			ScaledInData[3] = (BaseThrust-20.0)/40.0;

			forward(ScaledInData);

			for(j=0;j<4;j++){

				double value = 20.0 +(OutLayer[3][j]*40.0);
//				printf("%f	",value);
				Settings[j] = (int) value;
			}
//			printf("\n");
			SetMotors(Settings);

			printf("%f	|M1:%d	M2:%d	M3:%d	M4:%d\n",BaseThrust,Settings[0],Settings[1],Settings[2],Settings[3]);
			if(fabs(Data[0])> 20.0) running = 0;
			n++;
		}
		SetMotors(low);
		sleep(4);
	}
	end();
	return 0;
}
