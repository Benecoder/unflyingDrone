In my defence: I did work on this project alone and so there was never really anybody that I had to make the code readable for. If it makes a difference, this is a short explantion of what the code does under the right conditions:

<h3>komplette Arbeit/Anhang</h3>
The scientific paper where all the results are gathered.

<h3>Sensors</h3>
Reads the Sensor Data via i2c and calculates orientation in degrees.

<h3>Motor.h</h3>
Generates the pulse width modulated signal that controles the motors.

<h3>Matrix.h</h3>
All the matrix operations that are needed for the neural network.

<h3>BatchLearning</h3>
Main loop that put everything together. Also based on varying input datasets. Runs the motors for a while, then tries to predict the motor settings. after having fitted the NN.

<h3>Hyperparameters</h3>
Automated hyperparameter search.

<h3>PID / Kalman </h3>
As a areference I also tried to implement a standart PID algorithm using Kalman sensor fusuion. But then I did not know how to tune the parameters, so I had it look for them by itself. 

<h3>RecurrentNeuralNet</h3>
Because the results where slightly dissapointing, I tried to upgrade to a recurrent Neural Net, but somewhere in the derivaties I got lost.


