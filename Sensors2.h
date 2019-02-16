//Final States
double Pitch,Yaw,Roll;
//Raw States
double yAccel,xAccel,zAccel;

// Kalman Filtering					Pitch,Yaw,Roll ->  0,1,2
//Sensor Covarinace matrix
double x1[3] = {0.0,0.0,0.0};                                     //Best estimate of Angles
double x2[3] = {0.0,0.0,0.0};                                     //Best estimate of Rotational Velocities

double vOld[3] = {0.0,0.0,0.0};                                 	//Velocities Dump

double meanRotVel[3] = {0.0,0.0,0.0};
double meanDeg[3] = {0.0,0.0,0.0};

double DegVar[3] = {65.05,65.05,65.05};          	            	//Variance of the Degree Measurment
double RotVelVar[3]= {2000,2000,2000};;                   		//Variance of the RotVBel Measurment

double MuCVrange = 1000;                             //Calculation range of the Measurement uncertainty Covarinace Matrix

double SuCV[3][4] = {{1,1,1,1},				 //Unrolled State uncertainty Covarianve Matrix
		{1,1,1,1},
		{1,1,1,1}};



//Converts the Raw Data into doubles and scales the apropriatly for the Accelerometer
double twos_compAccel(char arg1,char arg2) {

	//twos complement operation
	int result = ((arg1 &  0x03)*256 + (arg2 & 0xFF));
	if (result > 511) result -= 1024;

	//Scales the Data
	double Scaled = (double)result * 0.0039;

	return Scaled;
}


//Pulls the Data from the Accelerometer and Calculates the Degrees
double AccelRead(char axis) {

        //Converts the Accelaration to angels
        double result;
        if (axis == 'p') {              //pitch axis
                        result =  xAccel/zAccel;
                        result = atan(result);
        } else if (axis =='r') {        //roll axis
                        result = yAccel/zAccel;
                        result = atan(result);
        } else {
                        printf("Axis Selection invalid! 'p': pitch, 'r': roll \n");
                        exit(1);
        }

        return (180.0*result)/M_PI;
}
//Pulls the Data from the Accelerometer and Returns Acceleration Data
void RawAccel(int file) {

	//selects the device
	ioctl(file,I2C_SLAVE,0x53);

	//reads the raw Data
	char reg[1] = {0x32};
	write(file,reg,1);
	char data[6] = {9};
	if(read(file,data,6)!=6) printf("Problems with the Acclerometer Readings. \n");

	//Converts the Raw Data to Acceleration Readings
	yAccel = twos_compAccel(data[3],data[2]);
	xAccel = twos_compAccel(data[1],data[0]);
	zAccel = twos_compAccel(data[5],data[4]);


}

//Pulls the Data from the Accelerometer and Calculates the Degrees
double AccelRead(char axis) {

	//Converts the Accelaration to angels
	double result;
	if (axis == 'p') {		//pitch axis
			result =  xAccel/zAccel;
			result = atan(result);
	} else if (axis =='r') {	//roll axis
			result = yAccel/zAccel;
			result = atan(result);
	} else {
			printf("Axis Selection invalid! 'p': pitch, 'r': roll \n");
			exit(1);
	}

	return (180.0*result)/M_PI;
}

//Gyroscope
char GyroRaw(int file,char Reg) {
	char reg[1] = {Reg};
	char data[1] = {0};

	write(file,reg,1);
	if(read(file,data,1) != 1)printf("Gyro did not answer!\n");

	return data[0];
}

//Functions that Pulls the Data from the Gyroscope and converst it into Rotational Velocity
double GyroRead(char axis, int file) {

	//selects the device
	ioctl(file,I2C_SLAVE,0x69);

	//Reads the most and least significant bit of the register
	char lsb;
	char msb;


	if (axis == 'r') {
		lsb = GyroRaw(file,0x28);
		msb = GyroRaw(file,0x29);
	} else if (axis == 'p') {
		lsb = GyroRaw(file,0x2A);
		msb = GyroRaw(file,0x2B);
	} else if (axis == 'y') {
		lsb = GyroRaw(file,0x2C);
		msb = GyroRaw(file,0x2D);
	} else {
		printf("Axis Selection invalid! 'p': pitch, 'r': roll, 'y':yaw\n");
		exit(1);
	}

	//Twos Complement of the Data; Converst binary data into doubles
	double RotVel = (msb*256+lsb);
	if(RotVel > 32767) RotVel-= 65536;

	//scales the Rotational Velocity Data
	//Page 10 of the Datasheet
	RotVel *= -0.0175;

	return RotVel;
}

int setupSensors(){

	//opens the bus
	int file;
	char *bus = "/dev/i2c-1";

	if ((file = open(bus,O_RDWR)) < 0) {

		printf("Failed to open bus! \n Unable to establish I2C connection\n");
		exit(1);
	}

	//Accelerometer:

	//selects the device
	ioctl(file,I2C_SLAVE,0x53);


	//Select Bandwidth rate register(0x20)
	//normal mode ,Output Data Rate = 100Hz(0x0a)
	char config[]={0};
	config[0] = 0x2C;
	config[1] = 0x0A;
	if(write(file,config,2)!=2){							//ERROR HANDLING TURNED OFF. ERROR CODE !=1
		printf("UNABLE TO CONFIGURE THE ACCELEROMETER!\n");
		exit(1);
	}

	//Select Power Control register (0x2d)
	//Auto Sleep disable(0x88)
	config[0] = 0x2D;
	config[1] = 0x08;
	write(file,config,2);

	//Select Data  format register(0x31)
	//Self test disabled, 4-wire interface,full resolution,range +/- 2g
	config[0] = 0x31;
	config[1] = 0x08;
	write(file,config,2);

	//Gyroscope:

	//selects the device
	ioctl(file,I2C_SLAVE,0x69);

	//Enable X,Y,Z-Axis and disable Power down mode(0x8F)
	config[0] = 0x20;
	config[1] = 0x0F;
	if(write(file,config,2)!= 2 ){
		printf("UNABLE TO CONFIGURE THE GYROSCOPE!\n");
		exit(1);
	}

	//Fullscale Range, 2000 dps(0x30)
	config[0] = 0x23;
	config[1] = 0x30;
	write(file,config,2);

	sleep(1.5);
	return file;
}



void  Filter(double t)
{
	//Initilazation
	double mdeg[3];
	mdeg[0] = Accelread('p');
	mdeg[1] = Yaw;
	mdeg[2] = AccelRead('r');

	double mRotVel[3];
	mRotVel[0] = GyroRead('p');
	mRotVel[1] = GyroRead('y');
	mRotVel[2] = GyroRead('r');


	double x1_[3],x2_[3];					//Transformed Angle and Velocity
	double u[3];							//Former Velocity and Calculated Velocity
	double MuCV[4];							//Unrolled Measurement uncertainty Covarinace Matrix
	double dAngle[3],dRotVel[3];			//Angle and Rotational Velocity Discrepany (Estimate vs. Measurment)
	double K[3][4];							//Kalman Gain


	int i;						//Three axis
	for(i=0;i<3;i++)
	{
		//Calcualting the Measurment Uncertainty Covariance Matrix
		meanDeg[i] += (mdeg[i]-meanDeg[i])/MuCVrange;
		meanRotVel[i] += (RotVel[i]-meanRotVel[i])/MuCVrange;

		DegVar[i] += pow((mdeg[i]-meanDeg[i]),2)/MuCVrange;
		RotVelVar[i] += pow((mRotVel[i]-meanRotVel[i]),2)/MuCVrange;

		MuCV[i][0] = DegVar[i];
		MuCV[i][1] = sqrt(DegVar[i])*sqrt(RotVelVar[i]);
		MuCV[i][2] = MuCV[i][1];
		MuCV[i][3] = RotVelVar[i];

		//Prediction
		u[i] = (mRotVel[i]-vOld[i])/t;
		x1_[i] = 0.5*u[i]*t*t+x2[i]*t+x1[i];
		x2_[i] = u[i]*t+x2[i];

		SuCV[i][0] = SuCV[i][0]+t*(SuCV[i][1]+SuCV[i][2]+SuCV[i][3]*t);
		SuCV[i][1] = SuCV[i][1]+SuCV[i][3]*t;
		SuCV[i][2] = SuCV[i][2]+SuCV[i][3]*t;

		//Update
		K[i][0] = SuCV[i][0]/(SuCV[i][0]+MuCV[i][0]);
		K[i][1] = SuCV[i][1]/(SuCV[i][1]+MuCV[i][1]);
		K[i][2] = SuCV[i][2]/(SuCV[i][2]+MuCV[i][2]);
		K[i][3] = SuCV[i][3]/(SuCV[i][3]+MuCV[i][3]);

		dAngle[i] = mdeg[i]-x1_[i];
		dRotVel[i] = mRotVel[i]-x2_[i];

		x1[i] = K[i][0]*dAngle[i]+K[i][1]*dRotVel[i]+x1_[i];
		x2[i] = K[i][2]*dAngle[i]*K[i][1]*dRotVel[i]+x2_[i];

		SuCV[i][0] = SuCV[i][0]-K[i][0]*SuCV[i][0]-K[i][1]*SuCV[i][2];
		SuCV[i][1] = SuCV[i][1]-K[i][0]*SuCV[i][1]-K[i][1]*SuCV[i][3];
		SuCV[i][2] = SuCV[i][3]-K[i][2]*SuCV[i][0]-K[i][3]*SuCV[i][2];
		SuCV[i][3] = SuCV[i][3]-K[i][2]*SuCV[i][1]-K[i][3]*SuCV[i][3];

		vOld[i] = RotVel[i];
	}

	Pitch = x1[0]
	Yaw = 	x1[1]
	Roll = 	x1[2]
}
