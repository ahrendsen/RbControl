/*
   program to set the control temperature of the Omega CN7500

   usage $ sudo ./setOmega <float temperature>

   e.g  $sudo ./setOmega 45.5
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "interfacing.h"


int main (int argc, char* argv[]){
    // Variables for recording the time. 
	time_t rawtime;
	struct tm * timeinfo;

	float returnRes, returnTarg, returnTargSet;
	float setTemperature=0;
	float stopPoint, ccSP;

	char stringBuff[BUFSIZE];

	time(&rawtime);
	timeinfo=localtime(&rawtime);
	strftime(stringBuff,BUFSIZE,"%F %H:%M:%S",timeinfo);
	printf("%s\n===\n",stringBuff);

	initializeBoard();

	getPVCN7500(CN_RESERVE,&returnRes);
	getPVCN7500(CN_TARGET,&returnTarg);
	getSVCN7500(CN_TARGET,&returnTargSet);
	if (returnRes == 0  || returnRes > 170){ 
		getPVCN7500(CN_RESERVE,&returnRes);
		getPVCN7500(CN_TARGET,&returnTarg);
		getSVCN7500(CN_TARGET,&returnTargSet);
	}

	ccSP=200.0;

	stopPoint=159.0;
	if (returnRes + 5 < stopPoint ){
		setTemperature = returnRes + 5;
		printf("temperature %3.1f < %3.1f, setting to %3.1f and %3.1f\n",returnRes, stopPoint , ccSP, setTemperature);
	} else{
		setTemperature = stopPoint;
	}

	setSVCN7500(CN_RESERVE, setTemperature);
	setSVCN7500(CN_TARGET, ccSP);

	printf("Target Temp:%f\n",returnRes);

	return 0 ;
}
