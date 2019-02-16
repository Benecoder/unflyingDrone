#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define LENGTH 3
#define NEURONS 2
#define RECORDS 3
#define NONREC 1

double InLayer[RECORDS][LENGTH][NEURONS],OutLayer[RECORDS][LENGTH][NEURONS];

double Biases[LENGTH][NEURONS];
double Weights[LENGTH][NEURONS][NEURONS];

double Error[LENGTH][NEURONS]= {{0}};
double WError[LENGTH][NEURONS][NEURONS] = {{{0}}};

double oldError[LENGTH][NEURONS]= {{0}};
double oldWError[LENGTH][NEURONS][NEURONS] = {{{0}}};

double Cost= 5.0,CostSum=0;

double activation(double number){
	return 1.0/(1.0+exp(-number));
}

double Derivative(double number){
	double var = activation(number);
	return var*(1.0-var);
}

double sign(double number){
	return (number>0)-(number<0);
}

void forward(double In[]){

	int Layer,Neuron,LastNeuron,t;

	//Shifts the Record
	for(t=RECORDS-2;t>=0;t--){
		for(Layer =0;Layer<LENGTH;Layer++){
			for(Neuron = 0;Neuron<NEURONS;Neuron++){
				InLayer[t+1][Layer][Neuron] = InLayer[t][Layer][Neuron];
				OutLayer[t+1][Layer][Neuron] = OutLayer[t][Layer][Neuron];
			}
		}
	}
	
	//Introduces the new Input
	for(Neuron=0;Neuron<NONREC;Neuron++){
		OutLayer[0][0][Neuron] = In[Neuron];
	}
	//Inputs output of the last timestep
	for(Neuron =NONREC;Neuron<NEURONS;Neuron++){
		OutLayer[0][0][Neuron] = OutLayer[0][LENGTH-1][Neuron];
		InLayer[0][0][Neuron] = InLayer[0][LENGTH-1][Neuron];
	}



	for(Layer=1;Layer<LENGTH;Layer++){
		for(Neuron=0;Neuron<NEURONS;Neuron++){

			InLayer[0][Layer][Neuron] = 0;

			//Weights
			for(LastNeuron=0;LastNeuron<NEURONS;LastNeuron++){
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

	double _Error[RECORDS][LENGTH][NEURONS]= {{{0}}};
	double _WError[LENGTH][NEURONS][NEURONS] = {{{0}}};

	//Backpropagating


	//Last Layer
	for(Neuron = 0;Neuron< NONREC; Neuron ++){

		NablaC = (OutLayer[0][LENGTH-1][Neuron]-True[Neuron]);
		CostSum += pow(fabs(NablaC),2)/2;

		_Error[0][LENGTH-1][Neuron] = NablaC*Derivative(InLayer[0][LENGTH-1][Neuron]);


		for(lastNeuron = 0;lastNeuron < NEURONS;lastNeuron ++) {
			_WError[LENGTH-1][Neuron][lastNeuron] = OutLayer[0][LENGTH-2][lastNeuron]* _Error[0][LENGTH-1][Neuron];
		}
	}


	//Recurrence
	for(t=0;t<RECORDS;t++){
		//Remaining Layers
		for(Layer = LENGTH-2;Layer>=0;Layer--){

			for(Neuron = 0;Neuron < NEURONS;Neuron ++){

				//Biases
				for(nextNeuron = 0;nextNeuron < NEURONS;nextNeuron ++){
					_Error[t][Layer][Neuron] += _Error[t][Layer+1][nextNeuron] * Weights[Layer+1][nextNeuron][Neuron];
				}
				_Error[t][Layer][Neuron] *= Derivative(InLayer[t][Layer][Neuron]);

				//Weights
				for(lastNeuron = 0; lastNeuron < NEURONS;lastNeuron++){
					_WError[Layer][Neuron][lastNeuron] = OutLayer[t][Layer-1][lastNeuron]*_Error[t][Layer][Neuron];
				}
			}
		}
		
		if(t+1<RECORDS){
			for(Neuron = NONREC;Neuron<NEURONS;Neuron++){
				_Error[t+1][LENGTH-1][Neuron] = _Error[t][0][Neuron];
			}
		}
	}

	//Adding the Error

	//Biases

	for(t=0;t<RECORDS;t++){
		for(Layer = 0;Layer < LENGTH;Layer++){
			for(Neuron = 0; Neuron < NEURONS;Neuron ++) {
				Error[Layer][Neuron] += _Error[t][Layer][Neuron];
			}
		}
	}

	//Weights
	for(Layer = 1;Layer < LENGTH;Layer ++) {
		for(Neuron = 0;Neuron < NEURONS;Neuron ++){
			for(lastNeuron = 0; lastNeuron < NEURONS; lastNeuron ++){

				WError[Layer][Neuron][lastNeuron] += _WError[Layer][Neuron][lastNeuron];
			}
		}
	}
}

void correct(double LearningRate,double momentum){

	int Layer,Neuron,lastNeuron;


	for(Layer = 0;Layer<LENGTH;Layer ++) {
		for(Neuron = 0;Neuron < NEURONS;Neuron ++){

			//Biases
			Biases[Layer][Neuron] += LearningRate*sign((momentum*oldError[Layer][Neuron]) + Error[Layer][Neuron]);
			oldError[Layer][Neuron] = Error[Layer][Neuron];
			Error[Layer][Neuron] = 0;

			//Weights
			for(lastNeuron = 0; lastNeuron < NEURONS;lastNeuron ++){
				Weights[Layer][Neuron][lastNeuron] += LearningRate*sign((momentum*oldWError[Layer][Neuron][lastNeuron])+WError[Layer][Neuron][lastNeuron]);
				oldWError[Layer][Neuron][lastNeuron] = WError[Layer][Neuron][lastNeuron];
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

	for(Layer=1;Layer<LENGTH;Layer++){

		for(Neuron=0;Neuron<NEURONS;Neuron++){

			Biases[Layer][Neuron] = randomnumber();

			for(LastNeuron=0;LastNeuron<NEURONS;LastNeuron++){

				Weights[Layer][Neuron][LastNeuron] = randomnumber();
			}
		}
	}
}


int main(){

	//Sets up the network
	randomize();


	//Initializes the Training
	int Epochs = 1,samples = 5;
	int x,y,z,a;

	double In[1]={0.0},Out[1]={0.0};

	//Training Loop
	for(x=0;x<Epochs;x++){
	
		Out[0] = -1;

		//flushes the I/O States
		for(y = 0;y<RECORDS;y++){
			for(z=0;z<LENGTH;z++){
				for(a=0;a<NEURONS;a++){
					InLayer[y][z][a] = 0.0;
					OutLayer[y][z][a] = 0.0;
				}
			}
		}


		//Backprop loop
		for(y=0;y<samples;y++){

			//Setst the Input output Layers
			double goal =randomnumber();

			Out[0] = In[0];
			In[0] = goal;

			forward(In);
			backward(Out);
			
			int Layer,Neuron;


			printf("In-----------------------\n");		
			
			for(Layer=0;Layer<LENGTH;Layer++){ 
				for(Neuron = 0;Neuron < NEURONS;Neuron++){
					printf("%f	",OutLayer[0][Layer][Neuron]);
				}
				printf("\n");
			}
		/*	printf("Out:\n");		
			for(Neuron = 0;Neuron < NEURONS;Neuron++){
				printf("%f	",OutLayer[t][2][Neuron]);
			}
			printf("\n");
		*/	
		//	printf("In:%f	Out:%f	Desired:%f	\n",In[0],OutLayer[0][LENGTH-1][0],Out[0]);
		}

		//corrects the Error
		double LR;
		if(Cost > 0.5) LR = -0.001;
		else LR = -0.0001;
		correct(LR,0.1);
		printf("Lr:	%f\n",LR);


		//Calcualtes the Result
		CalcCost(samples);

		//spitts the Result
		printf("Cost:%f	",Cost);
	}

	return 0;
}

