//MOTOR CONTROL

int PwmPercent(int per) {
	return per*10 + 1000;
}

void SetMotors(int thrust[]){
	int var[4],i;

	for(i=0;i<4;i++) {
		if(thrust[i]>100){
			thrust[i] = 100;
		} else if (thrust[i] <0) {
			thrust[i] =0;
		}
	}

	var[0] = gpioServo(17,PwmPercent(thrust[0]));	//Front Left
	var[1] = gpioServo(18,PwmPercent(thrust[1]));	//Front Right
	var[2] = gpioServo(27,PwmPercent(thrust[2]));	//Rear	Left
	var[3] = gpioServo(4 ,PwmPercent(thrust[3]));	//Rear	Right

	//Error Handling

	int Problem =0;
	for (i=0;i<4;i++){
		if (var[i]!=0){
			printf("Problem with Motor %d \n",i);
			Problem = 1;
		}
	}
	if (Problem) exit(1);
}

void end(){
	int cut[] = {0,0,0,0};
	int zero[] = {-100,-100,-100,-100};

	SetMotors(cut);
	sleep(1);
	SetMotors(zero);
	gpioTerminate();
}
void start(){
	assert(gpioInitialise() >=0);

	printf("Motors Initialized !!! \n");
	int full[] = {100,100,100,100};
	int cut[] = {0,0,0,0};
	int low[] = {10,10,10,10};

	SetMotors(full);
	sleep(3);
	SetMotors(cut);
	sleep(3);
	SetMotors(low);
}
void endStandby(){
	assert(gpioInitialise() >=0);

	int low[] = {5,5,5,5};
	SetMotors(low);
}

