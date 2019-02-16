#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_LENGTH 6
#define MAX_NUMOFNEURONS 30

int Length,Dimension[MAX_LENGTH];

double InLayer[MAX_LENGTH][MAX_NUMOFNEURONS],OutLayer[MAX_LENGTH][MAX_NUMOFNEURONS];

double Biases[MAX_LENGTH-1][MAX_NUMOFNEURONS];
double Weights[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS];
double RecWeights[MAX_LENGTH-1][MAX_NUMOFNEURONS][MAX_NUMOFNEURONS];

double activation(double number){
	return tanh(number);
}

double Derivative(double number){
	return (double) 1-pow(tanh(number),2);
}

void forward(double In[]){

	int Layer,Neuron,lastNeuron;
	
	for(Neuron = 0; Neuron<Dimension[0];Neuron ++){
		OutLayer[0][Neuron] = In[Neuron];
	}
	
	
	for(Layer = 1;Layer<Length;Layer++){
		for(Neuron = 0;Neuron<Dimension[Layer];Neuron ++){

			InLayer[Layer][Neuron] = 0;

			//Weights
			for(lastNeuron = 0;lastNeuron < Dimension[Layer-1];lastNeuron++){
				InLayer[Layer][Neuron] += OutLayer[Layer-1][Neuron]*Weights[Layer][Neuron][lastNeuron];
			}
			
			//Recurrent Inputs
			for(lastNeuron = 0;lastNeuron < Dimension[Layer];lastNeuron++){
				InLayer[Layer][Neuron] += OutLayer[Layer][Neuron]*RecWeights[Layer][Neuron][lastNeuron];
			}
			
			//Biases
			InLayer[Layer][Neuron] += Biases[Layer][Neuron];
			
			
			OutLayer[Layer][Neuron] = activation(InLayer[Layer][Neuron]);
			
		}
	}
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

int main(){
	
	Dimension[0] = 3;
	Dimension[1] = 5;
	Dimension[2] = 4;

	Length = 3;

	randomize();
	return 0;
}
			