//-----------------------------------------------------------------------------
//
//  u3_simple.c
//
//  Basic command-response U3 example using the UD driver.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <windows.h>
#include <LabJackUD.h>

// A simple UD error handling function. It displays the error code, string
// description of the error, line number and iteration. If the error code is
// greater than LJE_MIN_GROUP_ERROR, this function will exit the application.
// Parameters:
//     lngErrorcode: The UD error code.
//     lngLineNumber: The line number of the error. The macro __LINE__ can be
//         used to get a line number in the source code.
//     lngIteration: The iteration when running in a loop. Useful in Get loops.
void ErrorHandler(LJ_ERROR lngErrorcode, long lngLineNumber, long lngIteration)
{
	char err[255];

	if (lngErrorcode != LJE_NOERROR)
	{
		ErrorToString(lngErrorcode, err);
		printf("Error number = %d\n", lngErrorcode);
		printf("Error string = %s\n", err);
		printf("Source line number = %d\n", lngLineNumber);
		printf("Iteration = %d\n\n", lngIteration);
		if(lngErrorcode > LJE_MIN_GROUP_ERROR)
		{
			// Quit if this is a group error.
			getchar();
			exit(0);
		}
	}
}

int main()
{
	LJ_ERROR lngErrorcode = 0;
	long lngGetNextIteration = 0;
	double dblDriverVersion = 0.0;
	long lngIOType = 0;
	long lngChannel = 0;
	double dblValue = 0;
	double dblValueAin0 = 9999.0;
	double dblValueAin1 = 9999.0;
	double dblValueAin2 = 9999.0;
	double dblValueDIBit = 9999.0;
	double dblValueDIPort = 9999.0;
	double dblValueCounter1 = 9999.0;
	LJ_HANDLE lngHandle = 0;
	char ch = '\0';

	// Read and display the UD version.
	dblDriverVersion = GetDriverVersion();
	printf("UD Driver Version = %.3f\n\n", dblDriverVersion);

	// Open the first found LabJack U3.
	lngErrorcode = OpenLabJack(LJ_dtU3, LJ_ctUSB, "1", 1, &lngHandle);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Use the pin_configuration_reset IOType so that all pin assignments are
	// in the factory default condition.
	lngErrorcode = ePut(lngHandle, LJ_ioPIN_CONFIGURATION_RESET, 0, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Configuration commands. These will be done with the ePut function which
	// combines the Add-Go-Get into a single call.

	// Configure FIO0-FIO3 as analog, all else as digital. Start from channel 0
	// and update all 16 flexible bits. Pass a value of b0000000000001111 or
	// d15.
	lngErrorcode = ePut(lngHandle, LJ_ioPUT_ANALOG_ENABLE_PORT, 0, 15, 16);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Set the timer/counter pin offset to 7, which will put the first
	// timer/counter on FIO7.
	lngErrorcode = ePut(lngHandle,  LJ_ioPUT_CONFIG,
						LJ_chTIMER_COUNTER_PIN_OFFSET, 7, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Enable Counter1 (FIO7).
	lngErrorcode = ePut(lngHandle,  LJ_ioPUT_COUNTER_ENABLE, 1, 1, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// The following commands will use the Add-Go-Get method to group multiple
	// requests into a single low-level function.

	// Request a single-ended reading from AIN0.
	lngErrorcode = AddRequest(lngHandle, LJ_ioGET_AIN, 0, 0, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request a single-ended reading from AIN1.
	lngErrorcode = AddRequest(lngHandle, LJ_ioGET_AIN, 1, 0, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request a reading from AIN2 using the Special range (negative channel 32).
	lngErrorcode = AddRequest(lngHandle, LJ_ioGET_AIN_DIFF, 2, 0, 32, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request setting DAC0 to 3.5 volts.
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_DAC, 0, 3.5, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request setting digital output FIO4 to output-high.
	lngErrorcode = AddRequest(lngHandle, LJ_ioPUT_DIGITAL_BIT, 4, 1, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request a reading from digital input FIO5.
	lngErrorcode = AddRequest(lngHandle, LJ_ioGET_DIGITAL_BIT, 5, 0, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request a reading from digital inputs FIO5 through FIO6.
	lngErrorcode = AddRequest(lngHandle, LJ_ioGET_DIGITAL_PORT, 5, 0, 2, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	// Request a reading of the Counter1 (FIO7) value.
	lngErrorcode = AddRequest(lngHandle, LJ_ioGET_COUNTER, 1, 0, 0, 0);
	ErrorHandler(lngErrorcode, __LINE__, 0);

	while (1)
	{
		// Execute the requests.
		lngErrorcode = GoOne(lngHandle);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		// Get all the results. The input measurement results are stored. All
		// other results are for configuration or output requests, so we are
		// just checking whether there was an error.
		lngErrorcode = GetFirstResult(lngHandle, &lngIOType, &lngChannel,
			&dblValue, 0, 0);
		ErrorHandler(lngErrorcode, __LINE__, 0);

		lngGetNextIteration=0;  // Used by the error handling function.

		while (lngErrorcode < LJE_MIN_GROUP_ERROR)
		{
			if (lngIOType == LJ_ioGET_AIN)
			{
				if (lngChannel == 0)
				{
					dblValueAin0 = dblValue;
				}
				if (lngChannel == 1)
				{
					dblValueAin1 = dblValue;
				}
			}
			if (lngIOType == LJ_ioGET_AIN_DIFF)
			{
				dblValueAin2 = dblValue;
			}
			if (lngIOType == LJ_ioGET_DIGITAL_BIT)
			{
				dblValueDIBit = dblValue;
			}
			if (lngIOType == LJ_ioGET_DIGITAL_PORT)
			{
				dblValueDIPort = dblValue;
			}
			if (lngIOType == LJ_ioGET_COUNTER)
			{
				dblValueCounter1 = dblValue;
			}

			lngErrorcode = GetNextResult(lngHandle, &lngIOType, &lngChannel,
				&dblValue, 0, 0);
			if (lngErrorcode != LJE_NO_MORE_DATA_AVAILABLE)
			{
				ErrorHandler(lngErrorcode, __LINE__, lngGetNextIteration);
			}

			lngGetNextIteration++;
		}

		printf("AIN0 = %f\n", dblValueAin0);
		printf("AIN1 = %f\n", dblValueAin1);
		printf("AIN2 = %f\n", dblValueAin2);
		printf("FIO5 = %f\n", dblValueDIBit);
		// Will read 3 (binary 11) if both lines are pulled-high as normal.
		printf("FIO5-FIO6 = %f\n", dblValueDIPort);
		printf("Counter1 (FIO7) = %f\n", dblValueCounter1);

		printf("\nPress Enter to go again or (q) to quit\n");
		ch = getchar();
		if (ch == 'q')
		{
			return 0;
		}
	}
}
