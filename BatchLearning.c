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
#include "NeuralNet3L2.h"

double scale(double content,double factor) {
	return (2*content)/factor-1;
}

double InScale(double  content,double factor) {
	return (content+1)*(factor/2);
}

int main(){
	int running=1,i,j;

	//Setup for the Motors
	start();
	int Settings[4]={0};
	int low[4] = {0};
	double Delta;

	//Setup for the Data Input
	int file = setupSensors();
	double Data[3]  = {0};
	double begin = clock(),DutyCycle,D;


	//Setup for the Neural Network
	double ScaledInData[3],ScaledOutData[4];
	double **In,**Out;
	int NoC = 0,Length;
	SetupNetwork();


	printf("Data is reset!\n");
	//Sets up the Recordings
	Length = 40000;

	In = calloc(3*Length,sizeof(double*));
	if(In==NULL) printf("ALLOCATING IN MEORY FAILED!");
	for(i=0;i<Length;i++){
		In[i] = calloc(3,sizeof(double));
		if(In[i]==NULL) printf("MEMORY ALLOCATION WITH IN DATA %d FAILED!",i);
	}


	Out = calloc(4*Length,sizeof(double*));
	if(Out==NULL) printf("ALLOCATING OUT MEMORY FAILED!");
	for(i=0;i<Length;i++){
		Out[i] = calloc(4,sizeof(double));
		if(Out[i] == NULL) printf("MEMORY ALLOCATION WITH OUT DATA %d FAILED!",i);
	}

	printf("---------------------------testing--------------------------\n");

	while (NoC<Length){
		while(running) {

			/*Gathers the Data
			0-->Gyrospcope Degree
			1-->Accelerometer Degree
			2-->Gyrospcope Rotational Velocity
			*/

			DutyCycle = (double) (clock()-begin)/CLOCKS_PER_SEC;
			Data[2] = GyroRead('p',file);
			begin = clock();

			Data[0] += degrees(Data[2]*DutyCycle)-0.001;

			Data[1] = AccelRead('p',file);

			D = Data[1] *0.05+Data[0]*0.95;

			//Scaling in Data
			ScaledInData[0] = scale(Data[0],45);
			ScaledInData[1] = scale(Data[1],45);
			ScaledInData[2] = scale(Data[2],60);

			//Motor Settings
			Delta = D*0.2+1.5+Data[2]*0.09;
			Settings[0] = (int)40 + Delta;
			Settings[1] = (int)40 + Delta;
			Settings[2] = (int)40 - Delta;
			Settings[3] = (int)40 - Delta;
			for(i=0;i<4;i++){
				ScaledOutData[i] = scale(Settings[i],100);
			}


			SetMotors(Settings);

			//Degree Telemetry
			printf("M1:%d	M2:%d	M3:%d	M4:%d	Index: %d\n",Settings[0],Settings[1],Settings[2],Settings[3],NoC);
//			printf("Gyro: %f	Accel:%f	RotVel:%f\n",Data[0],Data[1],Data[2]);
			//Records the Data
			if(NoC < Length){
				for(i=0;i<3;i++){
					In[NoC][i] = ScaledInData[i];
				}
				for(i=0;i<4;i++){
					Out[NoC][i]= ScaledOutData[i];
				}
			}

			NoC++;

			//Checks wether to abort the session
			if(fabs(D)>35 ||NoC>Length) running = 0;

		}
		SetMotors(low);
		running = 1;
		printf("Terminated!\n");
		sleep(4);
		Data[0] = AccelRead('p',file);
		begin = clock();
	}

	//Backpropagation
	printf("---------------------Backpropagating---------------------\n");
	i = 0;
	while(Cost >200){
		for(j=0;j<Length;j++){
			forward(In[j]);
			backward(Out[j]);
		}
		CalcCost();
		Correct(Length);
		printf("Idx:%d		Cost:%f \n",i,Cost);

		i++;
	}

	printf("---------------------Sleeping-----------------------------\n");

	//Freeing the memory

	for(i=0;i<Length;i++){
		free(In[i]);
		free(Out[i]);
	}
	free(In);
	free(Out);

	sleep(10);





	printf("--------------------------USING NEURAL NETWORK-----------------\n");
	for(j=0;j<5;j++){
		Data[0] = AccelRead('p',file);
		begin = clock();
		running = 1;


		while(running){
	                /*Gathers the Data
	                0-->Gyrospcope Degree
	                1-->Accelerometer Degree
	                2-->Gyrospcope Rotational Velocity
	                */

	                DutyCycle = (double) (clock()-begin)/CLOCKS_PER_SEC;
	                Data[2] = GyroRead('p',file);
	                begin = clock();

        	        Data[0] += degrees(Data[2]*DutyCycle)-0.001;

        	        Data[1] = AccelRead('p',file);

        	        D = Data[1] *0.05+Data[0]*0.95;

        	        //Scaling in Data
        	        ScaledInData[0] = scale(Data[0],45);
        	        ScaledInData[1] = scale(Data[1],45);
        	        ScaledInData[2] = scale(Data[2],60);

        	        //Motor Settings
        	        forward(ScaledInData);

			for(i=0;i<4;i++){
				Settings[i] = (int)InScale(L3out[i],100.0);
			}
			SetMotors(Settings);

//			for(i=0;i<4;i++) printf("%f\n",L3out[i]);
//			printf("----------------\n");

//			printf("Gyro:%f		Accel:%f		RotVel:%f\n",Data[0],Data[1],Data[2]);
			printf("M1:%d		M2:%d		M3:%d		M4:%d\n",Settings[0],Settings[1],Settings[2],Settings[3]);

			if(fabs(D)>35)running = 0;

		}
		SetMotors(low);
		sleep(4);
	}

	end();

	return 0;
}


