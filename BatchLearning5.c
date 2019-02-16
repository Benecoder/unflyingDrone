/*

Based on presaved Data
Stochastig instead of Batch

*/
#include <stdio.h>
#include <stdlib.h>
//#include <linux/i2c-dev.h>
//#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
//#include <pigpio.h>
#include <string.h>
#include <assert.h>
//#include "Sensors.h"
//#include "Motor.h"
#include "NeuralNet2.h"

double scale(double content,double factor) {
	return (2*content)/factor-1;
}

 
double InScale(double  content,double factor) {
	return (content+1)*(factor/2);
} 

int main(){
	int running=1,i,j;

	//Setup for the Motors
	int Settings[4]={0};
	int low[4] = {0};

	//Setup for the Data Input
	//int file = setupSensors();
	double Data[3]  = {0};
	double begin = clock(),DutyCycle,D;


	//Setup for the Neural Network
	double ScaledInData[3];
	SetupNetwork();

	//Sets up the Recordings
	int NoS = 40000;

	//Readin the binary file
	FILE *In_ptr;
	FILE *Out_ptr;

	In_ptr = fopen("In2.bin","rb");
	Out_ptr = fopen("Out2.bin","rb");

	if(!In_ptr) printf("OPENING IN DATA FAILED!!\n");
	if(!Out_ptr) printf("OPENING OUT DATA FAILED!!!\n");

	double _In[NoS*3];
	double _Out[NoS*4];

	fread(&_In,sizeof(double),NoS*3,In_ptr);
	fread(&_Out,sizeof(double),NoS*4,Out_ptr);

	fclose(In_ptr);
	fclose(Out_ptr);

	//Formating the Data
	double In[NoS][3],Out[NoS][4];
	for(i=0;i<NoS;i++){
		for(j=0;j<3;j++){
			In[i][j] = _In[(i*3)+j];
		}
	}

	for(i=0;i<NoS;i++){
		for(j=0;j<4;j++){
			Out[i][j] = _Out[(i*4)+j];
		}
	} 

	srand(5);

	//Backpropagation
	printf("---------------------Backpropagating---------------------\n");
	i = 0;
	
	double prevCost = 2,delta= 1,LearningRate = -0.08; 


	while(i < 1000){


		
		int  index =  (NoS-10000.0)*((double)rand()/RAND_MAX);


		for(j=index;j<(index+10000);j++){ 
			forward(In[j]);
			backward(Out[j]);
		}
	
		
		correct(10000,LearningRate);

		if(i%50 == 0){
		
			prevCost = Cost;
		
			CalcCost(NoS);
		
			for(j=0;j<NoS;j++){
				forward(In[j]);
				backward(Out[j]);
		
			} 
			
			CalcCost(NoS);
	
			correct(NoS,LearningRate);
					
			delta = prevCost - Cost;
 
			printf("Cost:%f		Improvement:%f 	Lr: %f	Index:%d\n",Cost,delta,LearningRate,i);
		}

		i++;
	}

	printf("---------------------Sleeping-----------------------------\n");

	//sleep(10);


	printf("--------------------------USING NEURAL NETWORK-----------------\n");

	//start();


	for(j=0;j<5;j++){ 
		//Data[0] = AccelRead('p',file);
		begin = clock();
		running = 1;
		
		double count = 0;


		while(running){
		
					/*
	                Gathers the Data
	                0-->Gyrospcope Degree
	                1-->Accelerometer Degree
	                2-->Gyrospcope Rotational Velocity
	                */

	                DutyCycle = (double) (clock()-begin)/CLOCKS_PER_SEC;
	                Data[2] = 34*cos(count);//GyroRead('p',file);
	                begin = clock();

        	        Data[0] =  34*sin((double)count); //+= degrees(Data[2]*DutyCycle)-0.001;

        	        Data[1] = Data[0] + (0.01-0.02*(rand()/RAND_MAX));//AccelRead('p',file);

        	        D =  34*sin((double)count);//Data[1] *0.05+Data[0]*0.95;

        	        //Scaling in Data
        	        ScaledInData[0] = scale(Data[0],45);
        	        ScaledInData[1] = scale(Data[1],45);
        	        ScaledInData[2] = scale(Data[2],60);

        	        //Motor Settings
        	        forward(ScaledInData);

			for(i=0;i<4;i++){
				Settings[i] = (int)InScale(OutLayer[Length-1][i],20.0)+30;
			}
			//SetMotors(Settings);

//			for(i=0;i<4;i++) printf("%f\n",L3out[i]);
//			printf("----------------\n"); 

			printf("Gyro:%f		Accel:%f		RotVel:%f	",Data[0],Data[1],Data[2]);
			printf("M1:%d		M2:%d		M3:%d		M4:%d\n",Settings[0],Settings[1],Settings[2],Settings[3]);

			if(fabs(D)>35)running = 0;
			
			count += 0.001;
		}
		//SetMotors(low);
		sleep(4);
	}

	//end();*/

	return 0;
}


