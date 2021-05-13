/*
 * This is a template file to show all of the necessary components to add
 * to new data collection files that are created. 
 *
 * NOTE: For the Keithley Ammeters, I use the DDC commands.
 * For the 6485, information on these can be found in Appendix D
 * or pg. 233 of the pdf.
*/

#include "kenBoard.h"
#include "RS485Devices.h"
#include "K6485meter.h"

#define GPIBBRIDGE GPIBBRIDGE2 // the gpib bridge can have many gpib devices attached to it, so will also need the GPIB address of each


int main (int argc, char* argv[]){
	int i;
	float myTemp;
	unsigned char myStatusData[64];
	int testRange=8;

	initializeBoard();


	printf("Sending IFC to GPIB instruments on bridge %02x\n",GPIBBRIDGE);
	i=resetGPIBBridge(GPIBBRIDGE);
	printf("Status %d\n\n\n",i);
	delay(200);

	printf("Testing K6485 VERT\n===\n");

	printf("Init K6485 @ %d\n",K6485METERVERT);
	i=initializeK6485(K6485METERVERT,GPIBBRIDGE);
	printf("Status %d\n",i);
	delay(500);

	printf("Set Range  K6485 20uA\n");
	i=setRangeK6485(K6485METERVERT,GPIBBRIDGE,testRange);
	delay(500);
	printf("Status %d\n",i);
	
	printf("getting reading....\n");
	fflush(stdout);
	i = getReadingK6485(&myTemp, K6485METERVERT, GPIBBRIDGE);
	if (!i) printf("K6485 reading = %E\n",myTemp);else printf("Status %d\n",i);
	delay(400);

	i = getStatusK6485(myStatusData, K6485METERVERT, GPIBBRIDGE);
	if (!i) printf("K6485 status = %s\n",myStatusData);else printf("Status %d\n",i);

	printf("Testing K6485 HORIZ\n===\n");

	printf("Init K6485 @ %d\n",K6485METERHORIZ);
	i=initializeK6485(K6485METERHORIZ,GPIBBRIDGE);
	printf("Status %d\n",i);
	delay(500);

	printf("Set Range  K6485 20uA\n");
	i=setRangeK6485(K6485METERHORIZ,GPIBBRIDGE,testRange);
	delay(500);
	printf("Status %d\n",i);
	
	printf("getting reading....\n");
	fflush(stdout);
	i = getReadingK6485(&myTemp, K6485METERHORIZ, GPIBBRIDGE);
	if (!i) printf("K6485 reading = %E\n",myTemp);else printf("Status %d\n",i);
	delay(400);

	i = getStatusK6485(myStatusData, K6485METERHORIZ, GPIBBRIDGE);
	if (!i) printf("K6485 status = %s\n",myStatusData);else printf("Status %d\n",i);
	

	return 0;
}
