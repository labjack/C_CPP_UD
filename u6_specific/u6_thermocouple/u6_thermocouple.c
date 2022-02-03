//Demonstrates using the TCVoltsToTemp function to yield temperature
//  readings for a thermocouple directly connected to a U6.
//
//  Setup:
//  TC plus to AIN0.
//  TC minus to GND.
//
//  If you do not want all the thermocouple minus leads tied together
//  at GND, you can use differential channels instead.  Connect the minus
//  to AIN1 in that case, and use a 10k resistor from AIN1 to GND to
//  provide a path for bias currents.
//
//support@labjack.com
//December 28, 2009
//Revised December 27, 2010

#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <LabJackUD.h>
//The project must also know where to find labjackud.lib.  Here we do
//that by putting the lib file in the file view to the left.  The relative
//path stored by Visual Studio might not be the same on your machine, so
//you will probably have to delete the lib in the file view and right-click
//to re-add it from the install location ...\labjack\drivers\labjackud.lib.
//Another option, besides using the .lib, is dynamic linking.  Some
//compilers might not be able to link the .lib, and dynamic linking
//is usually the solution in those cases.



//This is our simple error handling function that is called after every UD
//function call.  This function displays the errorcode and string description
//of the error.  It also has a line number input that can be used with the
//macro __LINE__ to display the line number in source code that called the
//error handler.  It also has an iteration input is useful when processing
//results in a loop (getfirst/getnext).
void ErrorHandler (LJ_ERROR lngErrorcode, long lngLineNumber, long lngIteration) {
	char err[255];

	if (lngErrorcode != LJE_NOERROR) {
		ErrorToString(lngErrorcode,err);
		printf("Error number = %d\n",lngErrorcode);
		printf("Error string = %s\n",err);
		printf("Source line number = %d\n",lngLineNumber);
		printf("Iteration = %d\n\n",lngIteration);
		if(lngErrorcode > LJE_MIN_GROUP_ERROR) {
			//Quit if this is a group error.
			getchar();
			exit(0);
		}
   }
}

main() {
	LJ_ERROR lngErrorcode;
	LJ_HANDLE lngHandle=0;

	long lngIOType=0, lngChannel=0;
	double dblValue=0;
	double valueAIN = 0;  //Analog Voltage Value
	double ainResolution = 0;	//Configure resolution of the analog inputs (pass a non-zero value for quick sampling). 
								//See section 2.6 / 3.1 for more information.
	double dblInternal;
	double range = LJ_rgBIPP01V;
	
	long time=0, j=0;
	long tempChannel = 0;  //Channel which the TC/LJTIA is on (AIN0).

	double tcVolts = 0, cjTempK = 0, pTCTempK = 0;
	long tcType = LJ_ttK;
	//Set the temperature sensor to a k type thermocouple
	//Possible Thermocouple types are:
	//LJ_ttB = 6001
	//LJ_ttE = 6002
	//LJ_ttJ = 6003
	//LJ_ttK = 6004
	//LJ_ttN = 6005
	//LJ_ttR = 6006
	//LJ_ttS = 6007
	//LJ_ttT = 6008

	//Open the first found LabJack U6 via USB.
	lngErrorcode = OpenLabJack (LJ_dtU6, LJ_ctUSB, "1", 1, &lngHandle);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//Configure the desired resolution. See section 2.6 / 3.1 
	lngErrorcode = eGet(lngHandle, LJ_ioPUT_CONFIG, LJ_chAIN_RESOLUTION, &ainResolution, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Set the range on the ananlog input channel to +/- 0.1 volts (x100 gain)
	lngErrorcode = eGet(lngHandle, LJ_ioPUT_AIN_RANGE, lngChannel, &range, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	printf("Press any key to quit\n");

	//Constantly acquire temperature readings until a key is pressed
	while(1) {

		lngErrorcode = 0;
		lngIOType = 0;
		lngChannel = 0;
		time = 0;
		tcVolts = 0;
		cjTempK = 0;
		pTCTempK = 0;
		
		//Add analog input requests.
		lngErrorcode = AddRequest (lngHandle, LJ_ioGET_AIN, tempChannel, 0, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//Add request for internal temperature reading -- Internal temp sensor uses 
		//analog input channel 14 on the U6.
		lngErrorcode = AddRequest (lngHandle, LJ_ioGET_AIN, 14, 0, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//Execute all requests on the labjack lngHandle.
		lngErrorcode = GoOne (lngHandle);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//Get all the results.  The first result should be the voltage reading of the 
		//temperature channel.
		lngErrorcode = GetFirstResult(lngHandle, &lngIOType, &lngChannel, &valueAIN, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//The second result should be the internal temp sensor.
		lngErrorcode = GetNextResult(lngHandle, &lngIOType, &lngChannel, &dblInternal, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//The cold junction is the screw-terminal block where the thermocouple
		//is connected.  As discussed in the U6 User's Guide, add 2.5 degrees C
		//to the internal temp sensor reading.  If using the CB37 rather than
		//the built-in screw terminals, just add 1.0 degrees C.
		cjTempK = dblInternal + 2.5;

		//Display Voltage Reading
		printf("Analog %i           :%.6f\n", tempChannel, valueAIN);

		//Display the internal temperature sensor reading.  This example uses
		//that value for cold junction compensation.
		printf("U6 internal sensor :%.1f deg K\n", dblInternal);

		//Convert TC voltage to temperature.
		lngErrorcode = TCVoltsToTemp(tcType, valueAIN, cjTempK, &pTCTempK);

		//Display Temperature
		printf("Thermocouple sensor:%.1f deg K\n\n", pTCTempK);

		Sleep(1500);

		if(_kbhit())
			break;
		}
	return 0;
}
