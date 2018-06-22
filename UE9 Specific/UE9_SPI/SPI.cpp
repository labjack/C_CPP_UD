//---------------------------------------------------------------------------
//
//  SPI.cpp
//  
//	Demonstrates SPI communication.
//
//	You can short MOSI to MISO for testing.
//
//	MOSI    FIO2
//	MISO    FIO3
//	CLK      FIO0
//	CS        FIO1
//
//	If you short MISO to MOSI, then you will read back the same bytes that you write.  If you short
//	MISO to GND, then you will read back zeros.  If you short MISO to VS or leave it
//	unconnected, you will read back 255s.
//
//	Tested with UD driver V2.73, Comm firmware V1.40, and Control firmware V1.77.
//
//  support@labjack.com
//  Apr 26, 2007
//----------------------------------------------------------------------
//

#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include "c:\program files\labjack\drivers\LabJackUD.h"
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
void ErrorHandler (LJ_ERROR lngErrorcode, long lngLineNumber, long lngIteration)
{
	char err[255];

	if (lngErrorcode != LJE_NOERROR)
	{
		ErrorToString(lngErrorcode,err);
		printf("Error number = %d\n",lngErrorcode);
		printf("Error string = %s\n",err);
		printf("Source line number = %d\n",lngLineNumber);
		printf("Iteration = %d\n\n",lngIteration);
		if(lngErrorcode > LJE_MIN_GROUP_ERROR)
		{
			//Quit if this is a group error.
			getchar();
			exit(0);
		}
   }
}





void main()
{
	LJ_ERROR lngErrorcode;
	long lngGetNextIteration;
	long lngIOType=0, lngChannel=0;
	double dblValue=0;
	LJ_HANDLE lngHandle=0;
	double numSPIBytesToTransfer;
	unsigned char dataArray[50] = {0};
	long pdataArray = (long)&dataArray[0];


	//Open the LabJack UE9.
	lngErrorcode = OpenLabJack (LJ_dtUE9, LJ_ctUSB, "1", 1, &lngHandle);
	//errorcode = OpenLabJack (LJ_dtUE9, LJ_ctETHERNET, "192.168.1.209", 0, &Handle);
	ErrorHandler(lngErrorcode, __LINE__, 0);


	//First, configure the SPI communication.

	//Enable automatic chip-select control.
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_AUTO_CS,1,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//Do not disable automatic digital i/o direction configuration.
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_DISABLE_DIR_CONFIG,0,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//Mode A:  CPHA=1, CPOL=1.
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_MODE,0,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//125kHz clock.
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_CLOCK_FACTOR,0,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//MOSI is FIO2
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_MOSI_PIN_NUM,2,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);
	
	//MISO is FIO3
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_MISO_PIN_NUM,3,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//CLK is FIO0
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_CLK_PIN_NUM,0,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//CS is FIO1
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_CONFIG, LJ_chSPI_CS_PIN_NUM,1,0,0);
	ErrorHandler(lngErrorcode, __LINE__, 0);


	//Execute the requests on a single LabJack.  The driver will use a
    //single low-level TimerCounter command to handle all the requests above.
	lngErrorcode = GoOne(lngHandle);
	ErrorHandler(lngErrorcode, __LINE__, 0);


	//Get all the results just to check for errors.
	lngErrorcode = GetFirstResult(lngHandle, &lngIOType, &lngChannel, &dblValue, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);
	lngGetNextIteration=0;	//Used by the error handling function.
	while(lngErrorcode < LJE_MIN_GROUP_ERROR)
	{
		lngErrorcode = GetNextResult(lngHandle, &lngIOType, &lngChannel, &dblValue, 0, 0);
		if(lngErrorcode != LJE_NO_MORE_DATA_AVAILABLE)
		{
			ErrorHandler(lngErrorcode, __LINE__, lngGetNextIteration);
		}

		lngGetNextIteration++;
	}


	//This example transfers 4 test bytes.
	numSPIBytesToTransfer = 4;
	dataArray[0] = 170;
	dataArray[1] = 240;
	dataArray[2] = 170;
	dataArray[3] = 240;
	
	//Transfer the data.  The write and read is done at the same time.
	lngErrorcode = eGet(lngHandle, LJ_ioSPI_COMMUNICATION, 0, &numSPIBytesToTransfer, pdataArray);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	//Display the read data.
	printf("dataArray[0] = %d\n",dataArray[0]);
	printf("dataArray[1] = %d\n",dataArray[1]);
	printf("dataArray[2] = %d\n",dataArray[2]);
	printf("dataArray[3] = %d\n",dataArray[3]);


	getchar();
	return;

}
