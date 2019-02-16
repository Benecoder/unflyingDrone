/*

- Numerical Gradient decent reinforcment PID Tuning

*/

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <pigpio.h>
#include <string.h>
#include <assert.h>
#include "Sensors.h"
#include "Motor.h"

#define DT 0.005
#define RAD_TO_DEG 57.2957795131

//GLobal Variabels
double Data[9] ={0.0};
int K[3][3];

//returns millisecond accurate time readings
int mymillis()
{
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}



//Reads the Sensor Data
void ReadData(file){

	RawAccel(file);
	RawMagnet(file);

	//Roll
	double Gyrobuffer = GyroRead('r',file);
	Data[0] += Gyrobuffer*DT*RAD_TO_DEG-0.032;
	Data[1] = AccelRead('r');
	Data[2] = Gyrobuffer;

	//Pitch
	Gyrobuffer = GyroRead('p',file);
	Data[3] += Gyrobuffer*DT*RAD_TO_DEG-0.0063;
	Data[4] = AccelRead('p')+6.5;
	Data[5] = Gyrobuffer;

	//Yaw
	Gyrobuffer = GyroRead('y',file);
	Data[6] += Gyrobuffer*DT*RAD_TO_DEG-0.029;
	Data[7] = MagnetRead(file);
	Data[8] = Gyrobuffer;

}

int main(){

	//Setup for the Motors
	int  Settings[4]={0};
	double BaseThrust = 40.0;
	int MaxThrust = 35;

	//Setup for the Data Input
	int file = setupSensors();
	int startInt = mymillis();

	//Defines Test length
	int Epochs = 100;
	int Test_Length = 1000;
	int Test,Iteration;
	int axis,index;

	//Sets up the reward measures
	double oldReward=0,newReward=0;
	double instantReward;
	double h = 0.005;
	double alpha = 0.001;
	int State;

	//initiates the Konstants randomly
	srand(clock());
	for(axis = 0;axis<3;axis++){
		for(index = 0;index<3;index++){
			K[axis][index] = 1.0-2.0*(double)rand()/RAND_MAX;
		}
	}

	//Training
	for(Test =0;Test<Epochs;Test++){
                for(axis =0;axis<3;axis++){
                        for(index = 0;index<3;index++){
				for(State = 0;State<2;State++){

					//Adjusts Konstants
					if(State == 1){
						K[axis][index] += h;
						printf("Increased the Parameter!\n");
						sleep(3);
					}

					//Restes the Gyrotrackervariables
					RawAccel(file);
					RawMagnet(file);
					Data[0] = AccelRead('r');
					Data[3] = AccelRead('p')+6.5;
					Data[6] = 0;

					//starts the Motors
					endStandby();

					//zeros the Basethrust
					BaseThrust = 0;

					//zeros the clock
					startInt = mymillis();

					for(Iteration=1;Iteration<(int)(Test_Length);Iteration++){

						ReadData(file);

						double PitchDelta,Pitch;
						double RollDelta,Roll;
						double YawDelta,Yaw;

						//Combines the Angle Measumrments
						Pitch = 0.02*Data[4] + 0.98*Data[3];
						Roll  = 0.02*Data[1] + 0.98*Data[0];
						Yaw   = Data[6];

						//stores reward
						//reward: Angle(squared)+half the rotaional Velocity
						instantReward = pow(Pitch,2.0)+pow(Roll,2.0)+pow(Yaw,2.0)+(Data[5]/2.0)+(Data[2]/2.0)+(Data[8]/2.0);
						if(State == 0){
							oldReward += ((instantReward-oldReward)/Iteration);
						} else {
							newReward += ((instantReward-newReward)/Iteration);
						}

						//PID Deltas
						PitchDelta = K[0][0]*Pitch + K[0][1]*Data[5] + K[0][2]*PitchDelta;
						RollDelta  = K[1][0]*Roll  + K[1][1]*Data[2] + K[1][2]*RollDelta;
						YawDelta   = K[2][0]*Yaw   + K[2][1]*Data[8] + K[2][2]*YawDelta;

						if(BaseThrust<MaxThrust&&Iteration<(Test_Length)-MaxThrust){
							BaseThrust += 1;
						}
						else if(Iteration>(Test_Length)-MaxThrust){
							BaseThrust -= 1;
						}

						//Sets the Motrs Settings
						Settings[0] = (int)BaseThrust +  PitchDelta + RollDelta - YawDelta;
						Settings[1] = (int)BaseThrust +  PitchDelta - RollDelta + YawDelta;
						Settings[2] = (int)BaseThrust -  PitchDelta - RollDelta - YawDelta;
						Settings[3] = (int)BaseThrust -  PitchDelta + RollDelta + YawDelta;

						//Sends Motor Signal
						SetMotors(Settings);

						printf("Test:%d\tAxis:%d\tindex:%d\tState:%d\tIteration:%d\n",Test,axis,index,State,Iteration);

						//ensures constant cycle length
						while(mymillis()-startInt<DT*1000.0){
                					usleep(100);
						}
                				startInt = mymillis();
					}

					printf("-------------------reseting!-----------------\n");

					end();

					if(State == 1){
						K[axis][index] -= h;
						printf("Decreased the parameter!\n");
					}
					sleep(3);
				}
				//Adjusts the Konstants
				K[axis][index] += alpha*((oldReward-newReward)/h);
				printf("Adjusted the Constants!\n");
				sleep(3);
				oldReward = 0;
				newReward = 0;
			}
		}
	}
	return 0;
}
