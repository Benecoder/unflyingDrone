import matplotlib.pyplot as plt
import numpy as N
import math
import time


Data = N.fromfile("Data_file.bin")
Data = Data.reshape(len(Data)/7,7)


#Data[:,0] --> Rotation of the Sensors around the y-Axis;	Pitch;	down is negative
#Data[:,1] --> Rotation of the sensors around the x-Axis;	Roll;	looking forward positive Roll is 
#Data[:,2] --> Rotation of the Sensors around the z-Axis;	Yaw;	looking down on the sensor positive yaw is counterclockwise.


# In this coordinate system, the positive z-axis is down toward Earth. 
# Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
# Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
# Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
#to get the correct orientation the rotations must be applied in the correct order which for this configuration is yaw, pitch, and then roll.

def Integrate(List,t_):

	List = N.array(List)

	X = List[:,0]
	Y = List[:,1]
	Z = List[:,2]

	Angle = [[0.0,0.0,0.0]]
	deltaT = t_
	for i in range(1,len(Y)):
		t = [Angle[i-1][0]+X[i]*deltaT,
			Angle[i-1][1] +Y[i]*deltaT,
			Angle[i-1][2] ++Z[i]*deltaT]
	
		Angle.append(t)

	return N.array(Angle)*(180/math.pi)
class Madwick:

	def __init__(self,ax,ay,az):

		self.Q = N.array([0.01,0.01,0.01,0.01])
		self.Q_dot = N.zeros(4)
		self.Q_int = N.zeros(4)
		self.deltaT = 0.02

		self.s_om = N.zeros(4)

	def Update(self,gx,gy,gz):
	
		
	
		self.s_om = N.array([0.0,gx,gy,gz])
		self.Q_dot = Quat_mult(self.Q*0.5,self.s_om)
		self.Q_int = self.Q+self.Q_dot*self.deltaT
		
		self.Q = Norm(self.Q_int)


	def Filter(self,Records):
		
		Angles = []
		
		self.deltaT = N.mean(Data[:,9])

		
		for x in Records:
			
			self.Update(x[1],x[0],x[2])
			Pitch,Roll,Yaw = QuatToEuler(self.Q)
			
			Angles.append([Pitch,Roll,Yaw])
			
		return N.array(Angles)

def Display_static(Lists,name):

	colors = ["blue","green","red","cyan","magenta","yellow","black","white"]

	b = 0
	for i in Lists:
		plt.plot(i,color = colors[b])
		print(str(name[b]) + ": "+colors[b])
		b += 1
	
	plt.show()


def MagnetNew(Data):
	
	Angle = []
	

	for x in Data:
		Down = N.array([x[0],x[1],x[2]])
		East = N.cross(Down,N.array([x[3],x[4],x[5]]))
		North = N.cross(East,Down)
		
		Down /= N.linalg.norm(Down)
		East /= N.linalg.norm(East)
		North/= N.linalg.norm(North)
		
def Norm(Vector):
	return Vector/N.linalg.norm(Vector)
		
def Quat_mult(Q1,Q2):

	Q1_Q2 = []
	Q1_Q2.append(Q1[0]*Q2[0]-Q1[1]*Q2[1]-Q1[2]*Q2[2]-Q1[3]*Q2[3])
	Q1_Q2.append(Q1[0]*Q2[1]+Q1[1]*Q2[0]+Q1[2]*Q2[3]-Q1[3]*Q2[2])
	Q1_Q2.append(Q1[0]*Q2[2]-Q1[1]*Q2[3]+Q1[2]*Q2[0]+Q1[3]*Q2[1])
	Q1_Q2.append(Q1[0]*Q2[3]+Q1[1]*Q2[2]-Q1[2]*Q2[1]+Q1[3]*Q2[0])

	Q1_Q2 = Norm(N.array(Q1_Q2))

	return Q1_Q2

def QuatToEuler(Q):


	Roll = math.atan2(2*Q[1]*Q[2]-2*Q[0]*Q[3],2*Q[0]*Q[0]+2*Q[1]*Q[1]-1)
	Pitch = -math.asin(2*Q[1]*Q[3]+2*Q[0]*Q[2])
	Yaw  = math.atan2(2*Q[2]*Q[3]-2*Q[0]*Q[1],2*Q[0]*Q[0]+2*Q[3]*Q[3]-1)
	
	return Pitch,Roll,Yaw

def inv(Q):

	return Q*(N.array([1.0,-1.0,-1.0-1.0])

def Convert(Data):

	Angles = []
	
	for x in Data:
		Pitch,Roll,Yaw = 2*Quat
		Angles.append([Pitch,Roll,Yaw])
		
	return N.array(Angles)

B1 = Integrate(Convert(Data[:,[0,1,2,3]]),0.007)
B2 = Data[:,[4,5,6]]

plt.plot(B1)
#plt.plot(B2)

plt.show()