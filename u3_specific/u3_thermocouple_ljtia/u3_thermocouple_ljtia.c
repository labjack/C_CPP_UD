//---------------------------------------------------------------------------
//
//  thermocouple_ljtia.cpp
// 
//	Demonstrates using the TCVoltsToTemp to yield temperature readings for
//	a thermocouple connected to a U3 via a LJTick-InAmp
//
//  Setup:
//  TC plus to INA+.
//	TC minus to INA-.
//	10k resistor from INA- to GND on LJTIA (a short is also acceptable in most cases).
//	LJTIA connected to FIO4/FIO5 block on U3.
//	LJTIA offset set to 0.4 volts.
//	LJTIA channel A gain set to 51.
//
//  For the best accuracy, we recommend doing a quick offset calibration as
//	described in the comments below, and using an external cold-junction
//	temperature sensor rather than the internal U3 temp sensor.  The internal
//	temp sensor is often sufficient, but an external sensor, such as 
//	the LM34CAZ (national.com) or the EI-1034 (labjack.com), placed near
//	the LJTIA, can provide better CJC, particularly when the U3 itself
//	is subjected to varying temperatures.
//
//  support@labjack.com
//  Jul 21, 2008
//----------------------------------------------------------------------

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
	
	long time=0, j=0;
	long tempChannel = 4;  //Channel which the TC/LJTIA is on (FIO4).

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


	//Offset calibration:  The nominal voltage offset of the LJTick is
	//0.4 volts.  For improved accuracy, though, you should measure the
	//overall system offset.  We know that if the end of the TC is at the
	//same temperature as the cold junction, the voltage should be zero.
	//Put the end of the TC near the LJTIA to make sure they are at the same
	//temperature, and note the voltage measured by FIO4.  This is the actual
	//offset that can be entered below.
	double offsetVoltage = 0.4;

	//Open the first found LabJack U3 via USB.
	lngErrorcode = OpenLabJack (LJ_dtU3, LJ_ctUSB, "1", 1, &lngHandle);
	ErrorHandler(lngErrorcode, __LINE__, 0);


	//Reset all U3 ports to their default configuration.  
	//This is recommended prior to using the U3 to ensure the U3's ports are 
	//in the proper state.
	lngErrorcode = ePut (lngHandle, LJ_ioPIN_CONFIGURATION_RESET, 0, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	
	lngErrorcode = ePut (lngHandle, LJ_ioPUT_ANALOG_ENABLE_BIT, tempChannel, 1, 0);
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
		//analog input channel 30 on the U3.
		lngErrorcode = AddRequest (lngHandle, LJ_ioGET_AIN, 30, 0, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//Execute all requests on the labjack lngHandle.
		lngErrorcode = GoOne (lngHandle);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//Get all the results.  The first result should be the voltage reading of the 
		//temperature channel.
		lngErrorcode = GetFirstResult(lngHandle, &lngIOType, &lngChannel, &valueAIN, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		//The second result should be the internal temp sensor.
		lngErrorcode = GetNextResult(lngHandle, &lngIOType, &lngChannel, &cjTempK, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);


		//Display Voltage Reading
		printf("Analog %i           :%.6f\n", tempChannel, valueAIN);

		//Display the internal temperature sensor reading.  This example uses
		//that value for cold junction compensation.
		printf("U3 internal sensor :%.1f deg K\n", cjTempK);

		//To get the thermocouple voltage we subtract the offset from the AIN
		//voltage and divide by the LJTIA gain.
		tcVolts = (valueAIN - offsetVoltage)/51;

		//Convert TC voltage to temperature.
		lngErrorcode = TCVoltsToTemp(tcType, tcVolts, cjTempK, &pTCTempK);

		//Display Temperature
		printf("Thermocouple sensor:%.1f deg K\n\n", pTCTempK);

		Sleep(1500);

		if(_kbhit())
			break;
		}
	return 0;
}
