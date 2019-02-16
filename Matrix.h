#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define COLUMNS 2
#define ROWS 2

/*  -----------------Vector Operations --------------*/

void Vector_add(double Vector1[2],double Vector2[2],double targetVector[2]){

	targetVector[0] = Vector1[0]+Vector2[0];
	targetVector[1] = Vector1[1]+Vector2[1];
}

void Vector_multiply_scalar(double Vector[2],double Scalar){

	Vector[0] *= Scalar;
	Vector[1] *= Scalar;
}
void print_Vector(double Vector[2]){

	printf("----\n%f\n%f\n----",Vector[0],Vector[1]);
}

/*  ----------------Matrix Operations ------------ */


//Scalar Multiplication for 2 Column Maticies
void Matrix_multiply_scalar(double Array[ROWS][COLUMNS],double Scalar){

 	int i,j;
	for(i = 0;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			Array[i][j] *= Scalar;
		}
	}
}

//Adding a single number to a 2x2 matrix
void Matrix_add_single(double Array[ROWS][COLUMNS],double number){

        int i,j;
        for(i = 0;i<ROWS;i++){
                for(j=0;j<COLUMNS;j++){
                        Array[i][j] += number;
                }
        }
}

//Piece by piece Matrix Addition
void Matrix_add(double Array1[ROWS][COLUMNS],double Array2[ROWS][COLUMNS], double targetArray[ROWS][COLUMNS]){

	int i,j;
	for(i= 0;i<ROWS;i++){
                for(j=0;j<COLUMNS;j++){
			targetArray[i][j] = Array1[i][j]+Array2[i][j];
		}
	}
}

//Piece by piece Matrix Multiplication
void Matrix_multiply(double Array1[ROWS][COLUMNS],double Array2[ROWS][COLUMNS], double targetArray[ROWS][COLUMNS]){

        int i,j;
        for(i= 0;i<ROWS;i++){
                for(j=0;j<COLUMNS;j++){
                        targetArray[i][j] = Array1[i][j]*Array2[i][j];
                }
        }
}


//Piece by piece Matrix Division
void Matrix_divide(double Array1[ROWS][COLUMNS],double Array2[ROWS][COLUMNS], double targetArray[ROWS][COLUMNS]){

        int i,j;
        for(i= 0;i<ROWS;i++){
                for(j=0;j<COLUMNS;j++){
                        targetArray[i][j] = Array1[i][j]/Array2[i][j];
                }
        }
}

//Prints a 2x2 double Matrix
void print_Matrix(double Array[ROWS][COLUMNS]){
	int i,j;
	for(i=0;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			printf("%f \t",Array[i][j]);
		}
		printf("\n");
	}
}

// Matrix/Matrix  Dot Product
void Matrix_Matrix_Dot(double Array1[ROWS][COLUMNS],double Array2[ROWS][COLUMNS],double targetArray[ROWS][COLUMNS]){

	targetArray[0][0] = Array1[0][0]*Array2[0][0]+Array1[0][1]*Array2[1][0];
	targetArray[0][1] = Array1[0][0]*Array2[0][1]+Array1[0][1]*Array2[1][1];
	targetArray[1][0] = Array1[1][0]*Array2[0][0]+Array1[1][1]*Array2[1][0];
	targetArray[1][1] = Array1[1][0]*Array2[0][1]+Array1[1][1]*Array2[1][1];
}


//Vector/Matrix Dot Product
//FAILS IF VECTOR == TARGETVECTOR
void Vector_Matrix_Dot(double Array[ROWS][COLUMNS],double Vector[2],double targetVector[2]){

	int i,j;
	double runningSum;
	for(i=0;i<ROWS;i++){
		runningSum = 0.0;
		for(j=0;j<COLUMNS;j++){
			runningSum += Array[i][j]*Vector[j];
		}
		targetVector[i] = runningSum;
	}
}

//Matrix Transpose
void Transpose(double Array[ROWS][COLUMNS],double targetArray[ROWS][COLUMNS]){

	int i,j;
	for(i=0;i<ROWS;i++){
		for(j=0;j<COLUMNS;j++){
			targetArray[j][i] = Array[i][j];
		}
	}
}

