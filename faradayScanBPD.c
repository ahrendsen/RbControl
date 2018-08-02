/*
   Program to record excitation function. This is accomplished by 
   stepping up the voltage at the target in increments and recording
   the number of counts at each of those voltages.

   RasPi connected to USB 1204LS.


*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <asm/types.h>
#include "mathTools.h"
#include "fileTools.h"
#include "interfacing/interfacing.h"

#define BUFSIZE 1024
#define WAITTIME 2

void graphData(char* fileName);
void writeFileHeader(char* fileName, char* comments);
void writeTextToFile(char* fileName, char* line);
void collectAndRecordData(char* fileName, float startvalue, float endvalue, float stepsize, float* maxes, float* mins, int* channels, int numChannels);
float stdDeviation(float* values, int numValues);
void findAndSetProbeMaxTransmission();
void findMaxMinIntensity(float* maxes,int* maxLoc, float* mins, int* minLoc, int* channels, int numChannels, int stepRange, int motor);

int main (int argc, char **argv)
{
	// Variables for finding the max and min.
	int numChannels=3;
	int channels[] = {PUMP_LASER,PROBE_LASER,REF_LASER};
	int stepRange=100;
	int motor=PROBE_MOTOR;
	float* maxes = calloc(numChannels,sizeof(float));
	int* maxLoc = calloc(numChannels,sizeof(int));
	float* mins = calloc(numChannels,sizeof(float));
	int* minLoc = calloc(numChannels,sizeof(int));

    // Variables for recording the time. 
	time_t rawtime;
	struct tm * timeinfo;
	float startvalue,endvalue,stepsize;
	char fileName[BUFSIZE],comments[BUFSIZE];
	char dataCollectionFileName[] = "/home/pi/.takingData"; 
    int err;

	FILE *dataCollectionFlagFile, *fp;

	if (argc==5) {
		startvalue=atof(argv[1]);
		endvalue=atof(argv[2]);
		stepsize=atof(argv[3]);
		strcpy(comments,argv[4]);
	} else {
		printf("Usage:\n");
		printf("$ sudo ./faradayScanBPD <begin> <end> <step>  <comments>\n");
		printf("                      (0.0 - 117.5)                   \n");
		return 0;
	}
	// Indicate that data is being collected.
	dataCollectionFlagFile=fopen(dataCollectionFileName,"w");
	if (!dataCollectionFlagFile) {
		printf("Unable to open file: %s\n",dataCollectionFileName);
		exit(1);
	}

	initializeBoard();
	initializeUSB1208();

	if (endvalue>117.5) endvalue=117.5;
	if (startvalue>117.5) endvalue=117.5;
	if (startvalue<0) startvalue=0;
	if (endvalue<0) endvalue=0;
	if (startvalue>endvalue) {
		printf("error: startvalue > endvalue.\nYeah, i could just swap them in code.. or you could just enter them in correctly. :-)\n");
		return 1;
	}

	// get file name.  use format "RbAbs"+$DATE+$TIME+".dat"
	time(&rawtime);
	timeinfo=localtime(&rawtime);
	struct stat st = {0};
	strftime(fileName,BUFSIZE,"/home/pi/RbData/%F",timeinfo);
	if (stat(fileName, &st) == -1){ // Create the directory for the Day's data 
		mkdir(fileName,S_IRWXU | S_IRWXG | S_IRWXO );
	}
	strftime(fileName,BUFSIZE,"/home/pi/RbData/%F/FScanBPD%F_%H%M%S.dat",timeinfo);

	printf("\n%s\n",fileName);

	writeFileHeader(fileName, comments);
	fp=fopen(fileName,"a");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}

    err=setMirror(0);
    if(err>0) printf("Error Occured While setting Flip Mirror: %d\n",err);


	//findMaxMinIntensity(maxes, maxLoc, mins, minLoc, channels, numChannels, stepRange, motor);
	//printf("Max Intensity PD1: %f\nMin Intensity PD1: %f\nMax Loc PD1: %d\nMin Loc PD1: %d\n",maxes[0],mins[0],maxLoc[0],minLoc[0]);

//	int equalSignalLoc=(maxLoc[0]+minLoc[0])/2;

//	homeMotor(motor);
//	stepMotor(motor,CLK,equalSignalLoc);
	setVortexPiezo(45.0); // Return Piezo to 45.0 V

	collectAndRecordData(fileName, startvalue, endvalue, stepsize, maxes, mins, channels, numChannels);

	homeMotor(PROBE_MOTOR);

	setVortexPiezo(45.0); // Return Piezo to 45.0 V

	closeUSB1208();

	graphData(fileName);

	fclose(dataCollectionFlagFile);
	remove(dataCollectionFileName);

	return 0;
}

void graphData(char* fileName){
	char fileNameBase[1024];
	char buffer[BUFSIZE];
	char* extension;
	FILE* gnuplot;
	// Create graphs for data see gnutest.c for an explanation of 
	// how this process works.
	gnuplot = popen("gnuplot","w"); 

	strcpy(fileNameBase,fileName);
	extension = strstr(fileNameBase,".dat");
	strcpy(extension,"");


	if (gnuplot != NULL){
		fprintf(gnuplot, "set terminal dumb size 80,32\n");
		fprintf(gnuplot, "set output\n");			
		
		sprintf(buffer, "set title '%s'\n", fileName);
		fprintf(gnuplot, buffer);

		fprintf(gnuplot, "set key autotitle columnheader\n");
		fprintf(gnuplot, "set xlabel 'Voltage (Detuning)'\n");			
		fprintf(gnuplot, "set ylabel 'Transmitted Current'\n");			
		fprintf(gnuplot, "set yrange [-.1:*]\n");			
		fprintf(gnuplot, "set xrange [*:*]\n");			
		//fprintf(gnuplot, "set x2range [*:*]\n");			
		fprintf(gnuplot, "set x2tics nomirror\n");
		//sprintf(buffer, "plot '%s' using 1:6:7 with errorbars, '%s' using ($1*%f+%f):6:7 axes x2y1\n",fileName,fileName,aoutConv,aoutInt);
		sprintf(buffer, "plot '%s' using 1:7:8 with errorbars\n",fileName);
		fprintf(gnuplot, buffer);
		sprintf(buffer, "plot '%s' using 1:5:6 with errorbars\n",fileName);
		fprintf(gnuplot, buffer);
		sprintf(buffer, "plot '%s' using 1:3:4 with errorbars\n",fileName);
		fprintf(gnuplot, buffer);
		fprintf(gnuplot, "unset output\n"); 
		fprintf(gnuplot, "set terminal png\n");
		sprintf(buffer, "set output '%s.png'\n", fileNameBase);
		fprintf(gnuplot, buffer);
		sprintf(buffer, "plot '%s' using 1:7:8 with errorbars,\
						 	  '%s' using 1:5:6 with errorbars,\
						 	  '%s' using 1:3:4 with errorbars\n", fileName,fileName,fileName);
		fprintf(gnuplot, buffer);
	}
	pclose(gnuplot);
}

void findAndSetProbeMaxTransmission(){
	int i;
	int numMoves=0;
	int stepsPerRevolution=350;
	int moveSize=stepsPerRevolution/4;
	int foundMax=0;
	float returnFloat;
	float maxIntensity=0;
	int numMovesBackToMax=0;
	do{
		getUSB1208AnalogIn(PROBE_LASER,&returnFloat);
		if(fabs(returnFloat)>maxIntensity){
			numMovesBackToMax=4-numMoves;
			maxIntensity=fabs(returnFloat);
		}
		stepMotor(PROBE_MOTOR,CLK,moveSize);
		numMoves++;
		if(numMoves==4){
			// Go back to the maximum
			for(i=0;i<numMovesBackToMax+1;i++){
				stepMotor(PROBE_MOTOR,CCLK,moveSize);
			}
			if(moveSize==1){
				stepMotor(PROBE_MOTOR,CLK,moveSize);
			}
			moveSize=moveSize/2;
			numMoves=0;
		}
		if(moveSize==0){
			foundMax=1;
		}
	}while(!foundMax);
}

void writeFileHeader(char* fileName, char* comments){
	FILE* fp;
	float returnFloat;
	fp=fopen(fileName,"w");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}

	fprintf(fp,"#Filename:\t%s\n",fileName);
	fprintf(fp,"#Comments:\t%s\n",comments);

    /** Record System Stats to File **/
    /** Pressure Gauges **/
	getIonGauge(&returnFloat);
	printf("IonGauge %2.2E Torr \n",returnFloat);
	fprintf(fp,"#IonGauge(Torr):\t%2.2E\n",returnFloat);

	getConvectron(GP_N2_CHAN,&returnFloat);
	printf("CVGauge(N2) %2.2E Torr\n", returnFloat);
	fprintf(fp,"#CVGauge(N2)(Torr):\t%2.2E\n", returnFloat);

	getConvectron(GP_HE_CHAN,&returnFloat);
	printf("CVGauge(He) %2.2E Torr\n", returnFloat);
	fprintf(fp,"#CVGauge(He)(Torr):\t%2.2E\n", returnFloat);

    /** Temperature Controllers **/
	getPVCN7500(CN_RESERVE,&returnFloat);
	fprintf(fp,"#CurrTemp(Res):\t%f\n",returnFloat);
	getSVCN7500(CN_RESERVE,&returnFloat);
	fprintf(fp,"#SetTemp(Res):\t%f\n",returnFloat);

	getPVCN7500(CN_TARGET,&returnFloat);
	fprintf(fp,"#CurrTemp(Targ):\t%f\n",returnFloat);
	getSVCN7500(CN_TARGET,&returnFloat);
	fprintf(fp,"#SetTemp(Targ):\t%f\n",returnFloat);

	getPVCN7500(CN_CHAMWALL,&returnFloat);
	fprintf(fp,"#CurrTemp(Res2):\t%f\n",returnFloat);
	getSVCN7500(CN_CHAMWALL,&returnFloat);
	fprintf(fp,"#SetTemp(Res2):\t%f\n",returnFloat);
    /** End System Stats Recording **/

	//fprintf(fp,"VOLT\tPUMP\tStdDev\tPROBE\tStdDev\tREF\tStdDev\n");
	fprintf(fp,"VOLT\tWAV\tPMP\tPMPsd\tPMPmax\tPMPmin\tPMPnrm\tPRB\tPRBsd\tPRBmax\tPRBmin\tPRBnrm\tREF\tREFsd\tREFmax\tREFmin\tREFnrm\tANGLE\n");
	fclose(fp);
}

void writeTextToFile(char* fileName, char* line){
	FILE* fp;
	fp=fopen(fileName,"a");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}
	fprintf(fp,"%s",line);
	fclose(fp);
}

void collectAndRecordData(char* fileName, float startvalue, float endvalue, float stepsize, float* maxes, float* mins, int* channels, int numChannels){
	float value;
	FILE* fp;
	int k=0,i;
	float angle;
	int nSamples;
	float normSignal;
    int count=0;
	float involts[numChannels];
	float kensWaveLength;
	float returnFloat;

	fp=fopen(fileName,"a");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}
	// Allocate some memory to store measurements for calculating
	// error bars.
	nSamples = 50;
	float* measurement = malloc(nSamples*sizeof(float));

	value=startvalue;
	//setVortexPiezo(value);
	delay(10000);

	for (value=startvalue;value < endvalue && value >= startvalue;value+=stepsize){
	//returnFloat=1;
	//while(returnFloat!=0){
        if(count%15==0) printf("          \t       \t\t\tHORIZ      |        PERP	|  REF   |  ANGLE\n");
		setVortexPiezo(value);
		printf("VOLT %3.1f \t",value);
		fprintf(fp,"%f\t",value);

		// delay to allow transients to settle
		delay(10000);
		//scanf("%f",&returnFloat);
		kensWaveLength = getWaveMeter();// Getting the wavelength invokes a significant delay
                                        // So we no longer need the previous delay statement. 
		//kensWaveLength = -1;
		fprintf(fp,"%03.4f\t",kensWaveLength);
		printf("%03.4f\t",kensWaveLength);
		for(k=0;k<numChannels;k++){
			involts[k]=0.0;	
		}

		// grab several readings and average
		for(k=0;k<numChannels;k++){
			for (i=0;i<nSamples;i++){
				getUSB1208AnalogIn(channels[k],&measurement[i]);
				involts[k]=involts[k]+measurement[i];
				delay(10);
			}
			involts[k]=fabs(involts[k])/(float)nSamples;

			normSignal=(involts[k]-mins[k])/(maxes[k]-mins[k]);
			fprintf(fp,"%0.4f\t%0.4f\t%0.4f\t%0.4f\t%0.4f\t",involts[k],stdDeviation(measurement,nSamples),maxes[k],mins[k],normSignal);
			printf("  %0.4f %0.4f  ",involts[k],stdDeviation(measurement,nSamples));
            if(k<numChannels) printf(" | ");
		}
		//angle=atan((involts[0]-mins[0])/(maxes[0]-mins[0])/(involts[1]-mins[1])*(maxes[1]-mins[1]));
		angle=atan(sqrt((involts[0])/(involts[1])));
		fprintf(fp,"%02.3f\t",angle);
		fprintf(fp,"\n");
		printf("%02.3f\n",angle);
        count++;
	}
	fprintf(fp,"\n");
	fclose(fp);
	free(measurement);
}//end while

/** This function is used to find the maximum and minimum intensity 
 * on the two photodetectors for a balanced photodetector.
 *
 * maxes    is an empty array in which to store the maximum values of the two photodetectors
 * mins     is an empty array in which to store the minimum values of the two photodetectors
 * channels is an integer array containing the channel numbers to use with getUSB1208 analog in
 *          to obtain the photodetector signals.
 * numChannels is the number of channels that maxes and mins will be obtained for (2 for balanced
 *          photodetector).
 * stepRange is the number of steps to take with the motor that will for sure go through a maximum
 *          and a minimum. For a balanced photodetector with a half wave plate being rotated, this
 *          will be 1/4 of the total number of steps (350) so at least 87.5 steps. We'll do 100. 
 **/
void findMaxMinIntensity(float* maxes,int* maxLoc, float* mins, int* minLoc, int* channels, int numChannels, int stepRange, int motor){
	int steps;
	int i,j;
    int stepSize=1;
    float involts=0;
    float measurement=0;
    int nSamples=8;

	homeMotor(motor);

	for(j=0;j<numChannels;j++){ // numPhotoDet1
		maxes[j]=0;
		mins[j]=100;
	}

    for (steps=0;steps < stepRange;steps+=stepSize){ // steps
        // (STEPSPERREV) in increments of STEPSIZE
        delay(150); // watching the o-scope, it looks like it takes ~100ms for the ammeter to settle after a change in LP

        //get samples and average
        for(j=0;j<numChannels;j++){ // numPhotoDet1
            involts=0;
            for (i=0;i<nSamples;i++){ // nSamples
                getUSB1208AnalogIn(channels[j],&measurement);
                involts=involts+fabs(measurement);
                delay(WAITTIME);
            } // nSamples
            involts=involts/(float)nSamples;
            if(maxes[j]<involts){
                maxes[j]=involts;
				maxLoc[j]=steps;
            }
            if(mins[j]>involts){
                mins[j]=involts;
				minLoc[j]=steps;
            }
        } // numPhotoDet1


        stepMotor(motor,CLK,stepSize);
    } // steps
}
