
double yAccel,xAccel,zAccel;


//Converts the Raw Data into doubles and scales the apropriatly for the Accelerometer
double twos_compA(char arg1,char arg2) {

	//twos complement operation
	int result = ((arg1 &  0x03)*256 + (arg2 & 0xFF));
	if (result > 511) result -= 1024;

	//Scales the Data
	double Scaled = (double)result * 0.0039;

	return Scaled;
}

//Converts from radians to degrees
double degrees(double radians) {
	return radians*180.0/M_PI;
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
	yAccel = twos_compA(data[3],data[2]);
	xAccel = twos_compA(data[1],data[0]);
	zAccel = twos_compA(data[5],data[4]);
}

//Pulls the Data from the Accelerometer and Calculates the Degrees
double AccelRead(char axis) {

	//Converts the Accelaration to angels
	double result;
	if (axis == 'p') {		//pitch axis
			result =  yAccel/zAccel;
			result = atan(result);
	} else if (axis =='r') {	//roll axis
			result = xAccel/zAccel;
			result = atan(result);
	} else {
			printf("Axis Selection invalid! 'p': pitch, 'r': roll \n");
			exit(1);
	}

	return degrees(result);
}


char GyroRaw(int file,char Reg) {
	char reg[1] = {Reg};
	char data[1] = {0};

	write(file,reg,1);
	if(read(file,data,1) != 1) printf("Gyro did not answer!\n");

	return data[0];
}

//Functions that Pulls the Data from the Gyroscope and converst it into Rotational Velocity
double GyroRead(char axis, int file) {

	//selects the device
	ioctl(file,I2C_SLAVE,0x69);

	//Reads the most and least significant bit of the register
	char lsb;
	char msb;


	if (axis == 'p') {
		lsb = GyroRaw(file,0x28);
		msb = GyroRaw(file,0x29);
	} else if (axis == 'r') {
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
	RotVel *= 0.00875;

	return RotVel;
}


int setupSensors(){

	//opens the bus
	int file;
	char *bus = "/dev/i2c-1";

	if ((file = open(bus,O_RDWR)) < 0) {

		printf("Failed to open bus \n");
		exit(1);
	}

	//Accelerometer:

	//selects the device
	ioctl(file,I2C_SLAVE,0x53);

	//Select Bandwidth rate register(0x20)
	//normal mode ,Output Data Rate = 100Hz(0x0a)
	char config[2]={0};
	config[0] = 0x2C;
	config[1] = 0x0A;
	write(file,config,2);

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

	//Enable X,Y,Z-Axis and disable Power down mode(0x0F)
	config[0] = 0x20;
	config[1] = 0x0F;
	write(file,config,2);

	//Fullscale Range, 2000 dps(0x30)
	config[0] = 0x23;
	config[1] = 0x30;
	write(file,config,2);

	sleep(1.5);
	return file;
}
