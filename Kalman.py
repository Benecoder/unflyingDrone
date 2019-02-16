#import matplotlib
import matplotlib.pyplot as plt
#from mpl_toolkits.mplot3d import Axes3D
import numpy as N
import math
import time

Data = N.fromfile("Data_file.bin")


Data = Data.reshape(len(Data)//7,7)


#1.Dim is Time
#2. Dim [0] is Degree measured by Accelerometer
#2. Dim [1] is RotVel measured by Gyroscope
#2. Dim [2] is Time sinc last Measurement

def Integrate(List):
	# Gyroscopic RotVel Measurements integrated over time
	Integ = [0.0]
	

	for i in range(1,len(List)):
		Integ.append(Integ[-1]+ (List[i][0]*0.007))

	return Integ
def Derivate():

	Deriv = []
	for i in range(1,len(Data)):
		Deriv.append(((Data[i][0]-Data[i-1][0])/Data[i][2])*(500.0/32767.0))
	return Deriv
def SensorCV():

	DegMean = N.mean(Data[:,0])
	DegV = 0
	for i in Data[:,0]:
		DegV += (i-DegMean)**2
	DegV /= len(Data)
	
	RotVelMean = N.mean(Data[:,1])
	RotVelV = 0
	for i in Data[:,1]:
		RotVelV += (i-RotVelMean)**2
	RotVelV /= len(Data)
	
	Covariance = N.array([[DegV,math.sqrt(DegV)*math.sqrt(RotVelV)],
						  [math.sqrt(DegV)*math.sqrt(RotVelV),RotVelV]])
	
	return Covariance
def WeightAvg(Accel,Integ):

	print("length of Accel :" + str(len(Accel))+" \tLength of Gyro: "+str(len(Integ)))
	
	Weighted = []
	
	for i in range(len(Accel)):
		Weighted.append(Accel[i]*0.05+Integ[i]*0.95)
		
	return Weighted	
def singleFilter():

	t = 0.007									#Average Cycle time
	
	x1 = Data[0][0]								#Angle
	x2 = Data[0][1]								#Rotational Velocity
	
	x1_ = float(x1) 							#Transformed Angle
	x2_ = float(x2)								#Transformed Rotational Velocity
	
	u = 0										#Measured Acceleration
	
	SuCV = [10,10,10,10]											#Unrolled State uncertainty Covariance Matrix
	MuCV = [104.52623329,831.16840122,831.16840122,6609.25864646]	#Unrolled Measurement uncertainty Covariance Matrix
	
	mAngle = 0									#Measured Angle
	mRotVel = 0									#Measured Rot Vel
	
	dAngle = 0									#Angle Discrepancy
	dRotVel = 0									#Rotational Velocity Discrepancy 
	
	K = [0,0,0,0] 								#Kalman Gain
	
	
	Record = [Data[7][0]]
	
	for i in range(1,len(Data)):

		#Prediction
		u = (Data[i][1]-Data[i-1][1])/t
		x1_ = 0.5*u*t**2+x2*t+x1
		x2_ = u*t+x2
		SuCV[0] = SuCV[0]+t*(SuCV[1]+SuCV[2]+SuCV[3]*t)
		SuCV[1] = SuCV[1]+SuCV[3]*t
		SuCV[2] = SuCV[2]+SuCV[3]*t

		#Update
		mAngle = Data[i][0]
		mRotVel = Data[i][1]
		
		K[0] = SuCV[0]/(SuCV[0]+MuCV[0])
		K[1] = SuCV[1]/(SuCV[1]+MuCV[1])
		K[2] = SuCV[2]/(SuCV[2]+MuCV[2])
		K[3] = SuCV[3]/(SuCV[3]+MuCV[3])
		
		dAngle = mAngle-x1_
		dRotVel = mRotVel-x2_

		x1 = K[0]*dAngle+K[1]*dRotVel+x1_
		x2 = K[2]*dAngle*K[1]*dRotVel+x2_
		
		
		SuCV[0] = SuCV[0]-K[0]*SuCV[0]-K[1]*SuCV[2]
		SuCV[1] = SuCV[1]-K[0]*SuCV[1]-K[1]*SuCV[3]
		SuCV[2] = SuCV[3]-K[2]*SuCV[0]-K[3]*SuCV[2]
		SuCV[3] = SuCV[3]-K[2]*SuCV[1]-K[3]*SuCV[3]
		
		#Record
		Record.append(x1)

	return Record	
def MatrixFilter():

	t = N.mean(Data[:,2])
	
	x = N.array([[Data[0][0]],[Data[0][1]]])	#State Matrix; Angle and RotVel
	x_ = x										#Transformed State
	u = 0										#Measured Acceleration
	F = N.array([[1.0,t],						#Matrix that transforms state over time;
				[0.0,1.0]])				 		#based on speed
	B = N.array([[0.5*t**2],					#Matrix that transforms state over time
				 [t       ]])					#based on acceleration
	P = N.ones((2,2))*10						#State uncertainty covariance Matrix
	
	
	z = N.array([[Data[1][0]],[Data[1][1]]])	#Measured state Matrix
	R = SensorCV()								#Measurement uncertainty Covariance Matrix
	
	K = P/(P+R)									#Kalman Gain
	
	Record = [Data[7][0]]

	
	for i in range(1,len(Data)):
	
	
		#Prediction
		u = (Data[i][1]-Data[i-1][1])/t
		x_ = N.dot(F,x) + N.dot(B,u)
		P = N.dot(F,N.dot(P,N.transpose(F)))
		
		#Update
		z = [[Data[i][0]],[Data[i][1]]]
		K = P/(P+R)
		x = x_ + N.dot(K,z-x_)
		P = P - N.dot(K,P)
		
		#store
		Record.append(float(x[0]))
	
	return Record
def Display_static(Lists,name):

	colors = ["blue","green","red","cyan","magenta","yellow","black","white"]

	b = 0
	for i in Lists:
		plt.plot(i,color = colors[b])
		print(str(name[b]) + ": "+colors[b])
		b += 1
	
	plt.show()
def Display_Dynamic():

	global Filterd

	plt.ion()
	plt.plot(Data[:,0])
	plt.plot(Data[:,1],"yellow")
	graph = plt.plot(Filterd,"red")[0]

	i=0.0
	while i<10:
		i = i + 0.5
		print("Measured Error: "+str(i))
		Filterd = Filter(i)
		graph.set_ydata(Filterd)
		plt.draw()
		plt.pause(1)
def LinReg(List):

	X_Y = N.transpose(List)
	
	Slope = N.sum(N.multiply(X_Y[0],X_Y[1]))/N.sum(N.power(X_Y[0],2))
	return Slope
def Calibrate(Table):
    
    Readings = N.array(Table[:,[4,5,6]])
    Readings = (Readings - 2**15)

    x_Mean = N.mean(Readings[:,0])
    y_Mean = N.mean(Readings[:,1])
    z_Mean = N.mean(Readings[:,2])

    Readings[:,0] -= x_Mean
    Readings[:,1] -= y_Mean
    Readings[:,2] -= z_Mean
	
    XZSlope = LinReg(Readings[:,[0,2]])
    thetaXZ = math.atan(XZSlope)
    R_MatrixXZ = N.array([[math.cos(thetaXZ),-math.sin(thetaXZ)],
                          [math.sin(thetaXZ),math.cos(thetaXZ)]])

    for x in range(len(Readings)):
        Vector = N.array([Readings[x][0],Readings[x][2]])
        buffer = N.dot(R_MatrixXZ,Vector)

        Readings[x][0] = buffer[0]
        Readings[x][2] = buffer[1]
	
    YZSlope = LinReg(Readings[:,[1,2]])
    thetaYZ = math.atan(YZSlope)
							
    R_MatrixYZ = N.array([[math.cos(thetaYZ),-math.sin(thetaYZ)],
                          [math.sin(thetaYZ),math.cos(thetaYZ)]])
						
    for x in range(len(Readings)):
        Vector = N.array([Readings[x][1],Readings[x][2]])
        buffer = N.dot(R_MatrixYZ,Vector)
        Readings[x][1] = buffer[0]
        Readings[x][2] = buffer[1]

    Calibration_Data = N.array([x_Mean,y_Mean,z_Mean,
                        90.0])

#   Calibration_Data.tofile("MagnetCalibration.bin")

	
    return Readings
def MagnetRaw(Table,Shortcut):

    Parameters = N.fromfile("MagnetCalibration.bin")

    Parameters[3] = 90*(math.pi/180.0)
    
    Readings = N.array(Table[:,[4,5,6]])
    Readings = (Readings - 2**15)


    Readings[:,0] -= Parameters[0]
    Readings[:,1] -= Parameters[1]
    Readings[:,2] -= Parameters[2]


    for x in range(len(Readings)):
        thetaXZ = Parameters[3]
        
        R_MatrixXZ = N.array([[math.cos(thetaXZ),-math.sin(thetaXZ)],
                            [math.sin(thetaXZ),math.cos(thetaXZ)]])

        Vector = N.array([Readings[x][0],Readings[x][2]])
        buffer = N.dot(R_MatrixXZ,Vector)
                                      
        Readings[x][0] = buffer[0]
        Readings[x][2] = buffer[1]
   
    if Shortcut:
       return Readings
   
    for x in range(len(Readings)):
        thetaXZ = Table[x][1]*(math.pi/180.0)
        
        R_MatrixXZ = N.array([[math.cos(thetaXZ),-math.sin(thetaXZ)],
                              [math.sin(thetaXZ),math.cos(thetaXZ)]])
            
        Vector = N.array([Readings[x][0],Readings[x][2]])
        buffer = N.dot(R_MatrixXZ,Vector)
                              
        Readings[x][0] = buffer[0]
        Readings[x][2] = buffer[1]


    for x in range(len(Readings)):
        thetaYZ = Table[x][1]*(math.pi/180.0)
        
        R_MatrixYZ = N.array([[math.cos(thetaYZ),-math.sin(thetaYZ)],
                              [math.sin(thetaYZ),math.cos(thetaYZ)]])
            
        Vector = N.array([Readings[x][1],Readings[x][2]])
        buffer = N.dot(R_MatrixYZ,Vector)
                              
        Readings[x][1] = buffer[0]
        Readings[x][2] = buffer[1]

    
    return Readings
def MagnetRead(Readings):
    
    Angle = []

    for x in Readings:
        Angle.append(math.atan(x[0]/x[1])*(180.0/math.pi))

    return Angle
def Wrap(List,Threshold=90):


    for i in range(len(List)):
        if abs(List[i]) > Threshold:
            s = math.copysign(1,List[i])
            List[i:] -= Threshold*2*s

    return List

def isRotationMatrix(R) :
    Rt = N.transpose(R)
    shouldBeIdentity = N.dot(Rt, R)
    I = N.identity(3, dtype = R.dtype)
    n = N.linalg.norm(I - shouldBeIdentity)
    return n < 1e-6


def rotationMatrixToEulerAngles(R) :
     
    sy = math.sqrt(R[0,0] * R[0,0] +  R[1,0] * R[1,0])

    x = math.atan2(R[2,1] , R[2,2])
    y = math.atan2(-R[2,0], sy)
    z = math.atan2(R[1,0], R[0,0])

    return N.array([x, y, z])

# In this coordinate system, the positive z-axis is down toward Earth. 
# Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
# Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
# Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
def MagnetNew(Data):
	
	Angle = []
	

	for x in Data:
		Down = N.array([x[0],x[1],x[2]])
		East = N.cross(Down,N.array([x[3],x[4],x[5]]))
		North = N.cross(East,Down)
		
		Down /= N.linalg.norm(Down)
		East /= N.linalg.norm(East)
		North/= N.linalg.norm(North)
		
		R = N.array([Down,East,North])
		
		Euler = rotationMatrixToEulerAngles(R)

		Angle.append(Euler[2]*(180/math.pi))

	return Down,East,North
	
	

E1 = rotationMatrixToEulerAngles(R)
	
B1 = MagnetNew(Data[:,[1,2,3,4,5,6]])
plt.plot(B1)
plt.plot(Data[:,0]+B1[0])
plt.show()	
	
	
		