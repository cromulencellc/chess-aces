# riverway


## Description

"Riverway" is a simulator for planetary landers and was developed around the logic used in the Mars Polar Lander (MPL) that famously had a bug that caused the lander to crash into the planet at high velocity in 1999. That bug does not exist in this implementation. The main goal of the simulation is to find appropriate values for the PID coefficients (Kp, Ki, Kd) that control the firing of the lander's thrusters to achieve a smooth touchdown. At startup the simulator is configured with initial conditions that match the MPL planned Entry, Descent, and Landing (EDL) and will succeed in touching down at a low enough velocity. Users are free to play with all the parameters of both the PID controller and the simulation's starting conditions to see what affect they have on the outcome. No limitations have been placed on the various parameters and unrealistic simulations can be attempted that will no doubt end in nonsensical results or take a very long time to simulate to conclusion, i.e. making contact with the planetary surface. 

The simulation is controlled by a Web UI that the simulator serves up as an HTTP server. THe UI is how parameters are changed and simulation results are viewed. Due to the amount of data produced in simulation, only 10% is sent back to the browser for display, but the full data can be configured to be logged into a file that can be retrieved for analysis. 


### Scope for Analysis

The UI code that is both HTML and Javascript are provided for convenience and to make the service interesting, but are not where the bug exists. Analyzing the UI code does of course give some understanding of the underlying websocket protocol used to configure and run the simulation, but the same information can be gleened from the server side code.


### Running It

Riverway is a nodejs application that is started with 'nodejs app.js'

The challenge has several environment variables for configuration:

* `PORT`: this environment variable sets the listening port number. If this variable is not defined the service defaults to TCP port 3000.


### Persistent Data

No persistent data is required at the start of the service, but detailed simulation data is stored in CSV files in the Logs directory, if logging is enabled.  Logging is turned off by default and must be explicity turned on. The logging directory is never cleared and will grow to be quite large unless the Docker container is restarted.


## Poller Operation

The poller is a python script that connects to the simulation service using a client websocket. It performs a random sequence of operations using the JSON based protocol to ensure the simulator performs as expected.  It will specifically ensure a simulation succeeds using known good parameters, that known bad parameters fail, that logging can be enabled and disabled, and that all configuration items can be set and retrieved accurately.

THe Poller makes use of several environment variables:

* 'HOST' and 'PORT' are used to specificy where the simulation service is running
* 'LENGTH' controlls the number of random actions performed during the poll.  This value is capped at 800, which results in an approximately 1 minute poller run
* 'SEED' is used to seed the random number generator used to send random configuration settings




