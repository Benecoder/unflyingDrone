#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_LENGTH 6
#define MAX_NUMOFNEURONS 30

double InLayer[MAX_LENGTH][MAX_NUMOFNEURONS],OutLayer[MAX_LENGTH][MAX_NUMOFNEURONS];

double Biases[MAX_LENGTH-1][MAX_NUMOFNEURONS];
double Weights[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS];

double Error[MAX_LENGTH-1][MAX_NUMOFNEURONS]= {{0}};
double WError[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS];

int Length,Dimension[MAX_LENGTH];
double Cost= 10.0,CostSum=0;
double Lr = -0.0003;

double activation(double number){
	return tanh(number);
}

double Derivative(double number){
	return (double) 1-pow(tanh(number),2);
}

void forward(double In[]){

	int Layer,Neuron,LastNeuron;

	for(Neuron=0;Neuron<Dimension[0];Neuron++){
		OutLayer[0][Neuron] = In[Neuron];
	}

	for(Layer=1;Layer<Length;Layer++){
		for(Neuron=0;Neuron<Dimension[Layer];Neuron++){

			InLayer[Layer][Neuron] = 0;

			//Weights
			for(LastNeuron=0;LastNeuron<Dimension[Layer-1];LastNeuron++){
				InLayer[Layer][Neuron] += OutLayer[Layer-1][LastNeuron]*Weights[Layer][Neuron][LastNeuron];
			}


			//Biases
			InLayer[Layer][Neuron] += Biases[Layer][Neuron];

			//Activation
			OutLayer[Layer][Neuron] = activation(InLayer[Layer][Neuron]);
		}
	}
}

void backward(double True[]){

	int Layer,Neuron,nextNeuron,lastNeuron;
	double NablaC;

	double _Error[MAX_LENGTH-1][MAX_NUMOFNEURONS]= {{0}};
	double _WError[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS] = {{{0}}};

	//Backpropagating


	//Last Layer
	for(Neuron = 0;Neuron< Dimension[Length-1];Neuron ++){

		NablaC = (OutLayer[Length-1][Neuron]-True[Neuron]);
		CostSum += pow(fabs(NablaC),2)/2;

		_Error[Length-1][Neuron] = NablaC*Derivative(InLayer[Length-1][Neuron]);

		for(lastNeuron = 0;lastNeuron < Dimension[Length-1];lastNeuron ++) {
			_WError[Length-1][Neuron][lastNeuron] = OutLayer[Length-2][lastNeuron]* _Error[Length-1][Neuron];
		}
	}



	//Remaining Layers
	for(Layer = Length-2;Layer>0;Layer--){

		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){

			//Biases
			for(nextNeuron = 0;nextNeuron < Dimension[Layer];nextNeuron ++){
				_Error[Layer][Neuron] += _Error[Layer +1][nextNeuron] * Weights[Layer+1][nextNeuron][Neuron];
			}
			_Error[Layer][Neuron] *= Derivative(InLayer[Layer][Neuron]);

			//Weights
			for(lastNeuron = 0; lastNeuron < Dimension[Layer-1];lastNeuron++){
				_WError[Layer][Neuron][lastNeuron] = OutLayer[Layer-1][lastNeuron]*_Error[Layer][Neuron];
			}
		}
	}

	//Adding the Error

	//Biases
	for(Layer = 0;Layer < Length-1;Layer++){
		for(Neuron = 0; Neuron < Dimension[Layer];Neuron ++) {

			Error[Layer][Neuron] += _Error[Layer][Neuron];
		}
	}


	//Weights
	for(Layer = 1;Layer < Length;Layer ++) {

		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){

			for(lastNeuron = 0; lastNeuron < Dimension[Layer-1]; lastNeuron ++){

				WError[Layer][Neuron][lastNeuron] += _WError[Layer][Neuron][lastNeuron];
			}
		}
	}
}

void correct(double index,double LearningRate){

	int Layer, Neuron,lastNeuron;

	Lr = LearningRate;


	for(Layer = Length-1;Layer>0;Layer --) {
		for(Neuron = 0;Neuron < Dimension[Layer];Neuron ++){

			//Biases
			Biases[Layer][Neuron] += (double)Lr*(Error[Layer][Neuron]/index);
			Error[Layer][Neuron] = 0;

			//Weights
			for(lastNeuron = 0; lastNeuron < Dimension[Layer-1];lastNeuron ++){
				Weights[Layer][Neuron][lastNeuron] += (double)Lr*(WError[Layer][Neuron][lastNeuron]/index);
				WError[Layer][Neuron][lastNeuron] = 0;
			}
		}
	}
}

void CalcCost(index){

	Cost = CostSum/(double)index;
	CostSum =0;
}

void randomize(){

	int Layer,Neuron,LastNeuron;

	srand(clock());

	for(Layer=1;Layer<(Length);Layer++){

		for(Neuron=0;Neuron<Dimension[Layer];Neuron++){

			Biases[Layer][Neuron] = 1.0-2.0*(double)rand()/RAND_MAX;

			for(LastNeuron=0;LastNeuron<Dimension[Layer-1];LastNeuron++){

				Weights[Layer][Neuron][LastNeuron] = 1.0-2.0*(double)rand()/RAND_MAX;

			}
		}
	}
}


void SetupNetwork(){
	Dimension[0] = 4;
	Dimension[1] = 5;
	Dimension[2] = 5;
	Dimension[3] = 4;


	Length = 4;

	randomize();

}
