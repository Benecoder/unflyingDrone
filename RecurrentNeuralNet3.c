#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_LENGTH 6
#define MAX_NUMOFNEURONS 30
#define RECORDS 5

double InLayer[RECORDS][MAX_LENGTH][MAX_NUMOFNEURONS],OutLayer[RECORDS][MAX_LENGTH][MAX_NUMOFNEURONS];

double Biases[MAX_LENGTH-1][MAX_NUMOFNEURONS];
double Weights[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS];

double Error[MAX_LENGTH-1][MAX_NUMOFNEURONS]= {{0}};
double WError[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS];

int Length,Dimension[MAX_LENGTH];
int NonRecIn,NonRecOut;
double Cost= 5.0,CostSum=0;
double Lr = -0.0003;

double activation(double number){
	return tanh(number);
}

double Derivative(double number){
	return (double) 1-pow(tanh(number),2);
}

double sign(double number){
	return (number>0)-(number<0);
}

void forward(double In[]){

	int Layer,Neuron,LastNeuron,t;

	//Introduces the new Input
	for(Neuron=0;Neuron<NonRecIn;Neuron++){
		OutLayer[0][0][Neuron] = In[Neuron];
	}
	//Inputs output of the last timestep
	for(Neuron =NonRecIn;Neuron<Dimension[0];Neuron++){
		OutLayer[0][0][Neuron] = OutLayer[0][Length-1][Neuron-NonRecIn+NonRecOut];
		InLayer[0][0][Neuron] = InLayer[0][Length-1][Neuron-NonRecIn+NonRecOut];
	}

	//Shifts the Record
	for(t=RECORDS-2;t>=0;t--){
		for(Layer =0;Layer<Length;Layer++){
			for(Neuron = 0;Neuron<Dimension[Layer];Neuron++){
				InLayer[t+1][Layer][Neuron] = InLayer[t][Layer][Neuron];
				OutLayer[t+1][Layer][Neuron] = OutLayer[t][Layer][Neuron];
			}
		}
	}

	for(Layer=1;Layer<Length;Layer++){
		for(Neuron=0;Neuron<Dimension[Layer];Neuron++){

			InLayer[0][Layer][Neuron] = 0;

			//Weights
			for(LastNeuron=0;LastNeuron<Dimension[Layer-1];LastNeuron++){
				InLayer[0][Layer][Neuron] += OutLayer[0][Layer-1][LastNeuron]*Weights[Layer][Neuron][LastNeuron];
			}

			//Biases
			InLayer[0][Layer][Neuron] += Biases[Layer][Neuron];

			//Activation
			OutLayer[0][Layer][Neuron] = activation(InLayer[0][Layer][Neuron]);
		}
	}
}

void backward(double True[]){

	int Layer,Neuron,nextNeuron,lastNeuron,t;
	double NablaC;

	double _Error[RECORDS][MAX_LENGTH-1][MAX_NUMOFNEURONS]= {{{0}}};
	double _WError[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS] = {{{0}}};

	//Backpropagating


	//Last Layer
	for(Neuron = 0;Neuron< NonRecOut;Neuron ++){

		NablaC = (OutLayer[0][Length-1][Neuron]-True[Neuron]);
		CostSum += pow(fabs(NablaC),2)/2;

		_Error[0][Length-1][Neuron] = NablaC*Derivative(InLayer[0][Length-1][Neuron]);


		for(lastNeuron = 0;lastNeuron < Dimension[Length-1];lastNeuron ++) {
			_WError[Length-1][Neuron][lastNeuron] = OutLayer[0][Length-1][lastNeuron]* _Error[0][Length-1][Neuron];
		}
	}



	//Remaining Layers
	for(Layer = Length-2;Layer>=0;Layer--){

		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){

			//Biases
			for(nextNeuron = 0;nextNeuron < Dimension[Layer];nextNeuron ++){
				_Error[0][Layer][Neuron] += _Error[0][Layer+1][nextNeuron] * Weights[Layer+1][nextNeuron][Neuron];
			}
			_Error[0][Layer][Neuron] *= Derivative(InLayer[0][Layer][Neuron]);

			//Weights
			for(lastNeuron = 0; lastNeuron < Dimension[Layer-1];lastNeuron++){
				_WError[Layer][Neuron][lastNeuron] = OutLayer[0][Layer-1][lastNeuron]*_Error[0][Layer][Neuron];
			}
		}
	}



	//Recurrence
	for(t=1;t<RECORDS;t++){

		//Last Layer
		for(Neuron = NonRecOut;Neuron<Dimension[Length-1];Neuron++){
			_Error[t][Length-1][Neuron] = _Error[t-1][0][Neuron-NonRecOut+NonRecIn];

	                for(lastNeuron = 0;lastNeuron < Dimension[Length-1];lastNeuron ++) {
        	                _WError[Length-1][Neuron][lastNeuron] = OutLayer[t][Length-1][lastNeuron]* _Error[t][Length-1][Neuron];
                	}
		}


        	//Remaining Layers
	        for(Layer = Length-2;Layer>=0;Layer--){

               		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){

                	        //Biases
                	        for(nextNeuron = 0;nextNeuron < Dimension[Layer];nextNeuron ++){
                	                _Error[t][Layer][Neuron] += _Error[t][Layer+1][nextNeuron] * Weights[Layer+1][nextNeuron][Neuron];
        	                }
	                        _Error[t][Layer][Neuron] *= Derivative(InLayer[t][Layer][Neuron]);

                        	//Weights
                    	    for(lastNeuron = 0; lastNeuron < Dimension[Layer-1];lastNeuron++){
                        	        _WError[Layer][Neuron][lastNeuron] = OutLayer[t][Layer-1][lastNeuron]*_Error[t][Layer][Neuron];
                	        }
        	        }
	        }
	}

	//Adding the Error

	//Biases

	for(t=0;t<RECORDS;t++){
		for(Layer = 0;Layer < Length;Layer++){
			for(Neuron = 0; Neuron < Dimension[Layer];Neuron ++) {
				Error[Layer][Neuron] += _Error[t][Layer][Neuron];
			}
		}
	}

	//Weights
	for(Layer = 1;Layer < Length;Layer ++) {
		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){
			for(lastNeuron = 0; lastNeuron < Dimension[Layer-1]; lastNeuron ++){

				WError[Layer][Neuron][lastNeuron] += (_WError[Layer][Neuron][lastNeuron]);
			}
		}
	}
}

void correct(double LearningRate){

	int Layer, Neuron,lastNeuron;

	Lr = LearningRate;


	for(Layer = Length-1;Layer>0;Layer --) {
		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){

			//Biases
			Biases[Layer][Neuron] += Lr*sign(Error[Layer][Neuron]);
			Error[Layer][Neuron] = 0;

			//Weights
			for(lastNeuron = 0; lastNeuron < Dimension[Layer-1];lastNeuron ++){
				Weights[Layer][Neuron][lastNeuron] += Lr*sign(WError[Layer][Neuron][lastNeuron]);
				WError[Layer][Neuron][lastNeuron] = 0;
			}
		}
	}
}

void CalcCost(index){

	Cost = CostSum/(double)index;
	CostSum =0;
}

double randomnumber(){
	return 1.0-2.0*(double)rand()/RAND_MAX;
}

void randomize(){

	int Layer,Neuron,LastNeuron;

	srand(clock());

	for(Layer=1;Layer<(Length);Layer++){

		for(Neuron=0;Neuron<Dimension[Layer];Neuron++){

			Biases[Layer][Neuron] = randomnumber();

			for(LastNeuron=0;LastNeuron<Dimension[Layer-1];LastNeuron++){

				Weights[Layer][Neuron][LastNeuron] = randomnumber();

			}
		}
	}
}


void SetupNetwork(int Topo[]){

	Length = 3;

	int i;
	for(i=0;i<Length;i++) Dimension[i] = Topo[i];
	randomize();

}
int main(){

	//Sets up the network
	int Structure[3] = {2,2,2};
	NonRecIn = 1;
	NonRecOut = 1;
	SetupNetwork(Structure);
	randomize();


	//Initializes the Training
	int Epochs = 100,samples = 5000;
	int x,y,z,a;

	double In[1]={0.0},Out[2]={0.0};

	//Training Loop
	for(x=0;x<Epochs;x++){

		//flushes the I/O States
		for(y = 0;y<RECORDS;y++){
			for(z=0;z<Length;z++){
				for(a=0;a<Dimension[z];z++){
					InLayer[y][z][a] = 0.0;
					OutLayer[y][z][a] = 0.0;
				}
			}
		}

		//Backprop loop
		for(y=0;y<samples;y++){

			//Setst the Input output Layers
			double goal =1-2*randomnumber();

			for(z=0;z<NonRecOut;z++)Out[z] = In[z];
			for(z=0;z<NonRecIn;z++)In[z] = goal;


			forward(In);
			backward(Out);
			
			if(y == samples-1){
				printf("In:%f	Out:%f	Desired:%f\n",In[z],OutLayer[0][Length-1][0],goal);
			}
		}

		//corrects the Error
		correct(-0.01);


		//Calcualtes the Result
		CalcCost(samples);

		//spitts the Result
		printf("Cost:%f	",Cost);
	}

	return 0;
}

