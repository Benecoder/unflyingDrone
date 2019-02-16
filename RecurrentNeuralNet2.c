#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAX_NEURONS 10
#define MAX_LENGTH 10
#define RECORDS 5

/*Matrix Operations*/

void dot(int Neurons1,int Neurons2,double *Values,double (*Weights)[Neurons1],double *result){
//	Neurons of Layer 1/Layer2,InputValues,	WeightMarix		     Pointer to result list

	int Neuron,lastNeuron;

	for(Neuron = 0;Neuron<Neurons2;Neuron++){
		result[Neuron] = 0.0;
		for(lastNeuron = 0;lastNeuron<Neurons1;lastNeuron++){
			result[Neuron] += Values[lastNeuron]*Weights[Neuron][lastNeuron];
		}
	}
}

void activation(int len,double *values,double *result){
	int Neuron;

	for(Neuron = 0;Neuron<len;Neuron++){
		result[Neuron] = tanh(values[Neuron]);
	}
}

void derivative(int len,double *values,double *result){
	 int Neuron;

	for(Neuron = 0;Neuron<len;Neuron++){
                result[Neuron] = 1-pow(tanh(values[Neuron]),2);
        }
}

void add(int len,double *A,double*B){

	int i;

	for(i=0;i<len;i++){
		A[i] += B[i];
	}
}

void subtract(int len,double *A,double*B,double *C){

        int i;

        for(i=0;i<len;i++){
                C[i] = A[i] - B[i];
        }
}

double  sum(int len,double *list){
	int i;
	double result = 0;

	for(i=0;i<len;i++){
		result += fabs(list[i]);
	}
	return result;
}

void multiply(int len,double (*A),double(*B),double(*result)){

	int i;

	for(i=0;i<len;i++){
		result[i] = A[i]*B[i];
	}
}






/*Network operations*/

int length;
int Dimension[MAX_LENGTH];

double Weights[MAX_LENGTH][MAX_NEURONS][MAX_NEURONS]={{{0.0}}};
double RecWeights[MAX_LENGTH][MAX_NEURONS][MAX_NEURONS]={{{0.0}}};
double Biases[MAX_LENGTH][MAX_NEURONS] = {{0.0}};

double InStates[RECORDS][MAX_LENGTH][MAX_NEURONS];
double OutStates[RECORDS][MAX_LENGTH][MAX_NEURONS];

double Error[MAX_LENGTH][MAX_NEURONS] = {{0.0}};

double Cost;

//Random Parameter aquisiton
double randomnumber(){
        return 1.0-2.0*(double)rand()/RAND_MAX;
}

void randomize(){

        int Layer,Neuron,LastNeuron;
        srand(clock());

        for(Layer=1;Layer<(length);Layer++){
                for(Neuron=0;Neuron<Dimension[Layer];Neuron++){

                        Biases[Layer][Neuron] = randomnumber();

                        for(LastNeuron=0;LastNeuron<Dimension[Layer-1];LastNeuron++){
                                Weights[Layer][Neuron][LastNeuron] = randomnumber();
                        }
                        for(LastNeuron = 0;LastNeuron<Dimension[Layer];LastNeuron++){
                                RecWeights[Layer][Neuron][LastNeuron] = randomnumber();
                        }
               }
        }
}
void forward(){

	int Layer,Neuron,t;


	//Shifting the Reocrded Data
	for(t = RECORDS-2;t>=0;t--){
		for(Layer = 0;Layer < length;Layer++){

			for(Neuron =0;Neuron < Dimension[Layer];Neuron++){
				InStates[t+1][Layer][Neuron] = InStates[t][Layer][Neuron];
				OutStates[t+1][Layer][Neuron] = OutStates[t][Layer][Neuron];
			}
		}
	}



	//Forwarding the Data
	for(Layer=1;Layer<length;Layer++){
		for(Neuron=0;Neuron<Layer;Neuron++)InStates[0][Layer][Neuron] = 0.0;

		//Input from the last Layer
		dot(Dimension[Layer-1],Dimension[Layer],OutStates[0][Layer-1],Weights[Layer],InStates[0][Layer]);
		add(Dimension[Layer],InStates[0][Layer],Biases[Layer]);

		//Activation
		activation(Dimension[Layer],InStates[0][Layer],OutStates[0][Layer]);
	}
}


void backward(double *True){

	int Layer,t;


	double _Error[RECORDS][MAX_LENGTH][MAX_NEURONS] = {{{0.0}}};
	double _WError[MAX_LENGTH][MAX_NEURONS][MAX_NEURONS] ={{{0.0}}};

	double NablaC[Dimension[length-1]];
	double derivative_buffer[MAX_NEURONS];
	double dot_buffer[MAX_NEURONS];

	//last Layer 
	subtract(Dimension[length-1],True,OutStates[0][length-1],NablaC);
	derivative(Dimension[length-1],InStates[0][length-1],derivative_buffer);
	multiply(Dimension[length-1],NablaC,derivative_buffer,_Error[0][length-1]);

	//remaining Layers
	


	//Cost
    Cost += pow(sum(Dimension[length-1],NablaC),2)/2.0;


	//Adding the Error
	for(t=0;t<RECORDS;t++){
		for(Layer =0;Layer <length;Layer++){
			add(Dimension[Layer],Error[Layer],_Error[t][Layer]);
		}
	}
}

int main(){

	int i,n;

	length =3;
	Dimension[0] = 4;
	Dimension[1] = 3;
	Dimension[2] = 4;



	randomize();
	double Derivative=0.0;
	double simulation=0.0;

        double True[4] = {0.2,0.2,0.2,0.2};

	for(n=0;n<10;n++){

		for(i=0;i<4;i++)OutStates[0][0][i] = 5.0;

		forward();
		backward(True);

		Derivative += Error[1][0];
		printf("-----------------------------------\n");
	}

        for(i=0;i<4;i++)Error[1][i] = 0.0;
	Cost /= 10.0;

	printf("oldCost:%f	Error:%f	|",Cost,Derivative/10.0);

	double h = 0.000001,newCost=0.0;
	Biases[1][0] += h;


	for(n=0;n<10;n++){

                for(i=0;i<4;i++)OutStates[0][0][i] = 5.0;

		forward();

		double NablaC[Dimension[length-1]];
	        subtract(Dimension[length-1],True,OutStates[0][length-1],NablaC);

		newCost += pow(sum(Dimension[length-1],NablaC),2.0)/2.0;
	}


	simulation = (Cost-(newCost/10.0))/(h);
	double Delta = fabs((newCost/10)-Cost);

	printf("newCost:%f	numericalError:%f	|Delta:%f\n",(newCost/10.0),simulation,Delta);
	return 0;
}
