/*

- Angle and RotVel as Input
- Stores Result

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
#include "NeuralNet.h"

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


	ScaledInData[0] = (Data[0]*0.98+Data[1]*0.02)/30.0;
	ScaledInData[1] = Data[2]/20.0;
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
	endStandby();

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

		for(n=0;n<(int)(NOS/parts);n++){

			ReadData(file);

			double Delta,D;

			D = 0.02*Data[1] + 0.98*Data[0];

			Delta = D * 0.12+Data[2]*0.23;
			BaseThrust = 20;
			printf("%d	|",(int)BaseThrust);

			Settings[0] = (int)BaseThrust +  Delta;
			Settings[1] = (int)BaseThrust -  Delta;
			Settings[2] = (int)BaseThrust -  Delta;
			Settings[3] = (int)BaseThrust +  Delta;

			for(i=0;i<4;i++) ScaledOutData[i] = ((Settings[i]-20.0)/20.0);

			//Sends Motor Signal
			SetMotors(Settings);

			//Set Recordings
			for(i=0;i<2;i++) InData[x][i] = ScaledInData[i];

			for(i=0;i<4;i++){
				OutData[x][i] = ScaledOutData[i];
				printf("%d	",Settings[i]);
			}
//			printf("\n");
			printf("RealRotVel:%f	Degree:%f	RotVel:%f\n",Data[2],InData[x][0],InData[x][1]);
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
	int sampleSize = 30000,index;

	while(i <= 400 && delta > -0.1){

		index = (int) (NOS-sampleSize)*(rand()/RAND_MAX);

		for(j=index;j<(index+sampleSize);j++){
			forward(InData[j]);
			backward(OutData[j]);
		}

		correct(sampleSize,LearningRate);
		CalcCost(sampleSize);
		printf("cost:%f\n",Cost);

		if(i%20 == 0){
			delta = Cost;


			for(j=0;j<NOS;j++){
				forward(InData[j]);
				backward(OutData[j]);
			}

			CalcCost(NOS);

			delta -= Cost;

			correct(NOS,LearningRate);
			printf("Cost:%f		Improvment:%f		Index:%d\n",Cost,delta,i);
		}
	i++;
	}
	//Stores the Parameters
	//Opens the File
	FILE *Weights_ptr;
	FILE *Biases_ptr;

	Weights_ptr = fopen("Angles-Weights.bin","wb");
	Biases_ptr = fopen("Angles-Biases.bin","wb");

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

	n = 0;
	endStandby();

	int Standart[4];
	double D,Delta;

	for(i=0;i<500;i++){
		running = 1;
                RawAccel(file);
                Data[0] = AccelRead('p');
		while(running){

			BaseThrust = 20.0;

			ReadData(file);

			forward(ScaledInData);

			//Standart
			D = 0.02*Data[1] + 0.98*Data[0];

                        Delta = D * 0.12+Data[2]*0.21;

                        Standart[0] = (int)BaseThrust +  Delta;
                        Standart[1] = (int)BaseThrust -  Delta;
                        Standart[2] = (int)BaseThrust -  Delta;
                        Standart[3] = (int)BaseThrust +  Delta;


			//Neural Network
			for(j=0;j<4;j++){

				double value = 20.0 +(OutLayer[3][j]*20.0);
				Settings[j] = (int) value;
			}
			SetMotors(Settings);

			printf("%d	%d\n",Standart[0],Settings[0]);


//			printf("%f	|M1:%d	M2:%d	M3:%d	M4:%d|	Degree:%f	RotVel:%f\n",BaseThrust,Settings[0],Settings[1],Settings[2],Settings[3],ScaledInData[0],ScaledInData[1]);
			if(fabs(Data[0])> 50.0) running = 0;
			n++;
		}
		SetMotors(low);
		sleep(4);
	}
	end();
	return 0;
}
