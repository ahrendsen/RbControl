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
#include "interfacing/interfacing.h"
#include "mathTools.h"
#include "fileTools.h"

#define BUFSIZE 1024
#define NUMCHANNELS 3

void graphData(char* fileName);
void writeFileHeader(char* fileName, char* comments, float probeOffset);
void writeTextToFile(char* fileName, char* line);
void collectAndRecordData(char* fileName, int startvalue, int endvalue, int stepsize);
float stdDeviation(float* values, int numValues);
void findAndSetProbeMaxTransmission();

int main (int argc, char **argv)
{
	int startvalue,endvalue,stepsize,piOver2StepLocation,probeStepsPerRevolution;
	time_t rawtime;
	struct tm * timeinfo;
	char fileName[BUFSIZE],comments[BUFSIZE];
	char dataCollectionFileName[] = "/home/pi/.takingData"; 

	float probeOffset;

	FILE *dataCollectionFlagFile, *fp;

	if (argc==6) {
		startvalue=atoi(argv[1]);
		endvalue=atoi(argv[2]);
		stepsize=atoi(argv[3]);
		probeOffset=atof(argv[4]);
		strcpy(comments,argv[5]);
	} else {
		printf("Usage:\n$ sudo ./RbAbsorbScan <begin> <end> <step> <probeOffset> <comments>\n");
		printf("                              ( 0 - 1023 )                   \n");
		return 0;
	}
	// Indicate that data is being collected.
	dataCollectionFlagFile=fopen(dataCollectionFileName,"w");
	if (!dataCollectionFlagFile) {
		printf("unable to open file \n");
		exit(1);
	}

	initializeBoard();
	initializeUSB1208();

	if (endvalue>1024) endvalue=1024;
	if (startvalue>1024) endvalue=1024;
	if (startvalue<1) startvalue=0;
	if (endvalue<1) endvalue=0;
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
	strftime(fileName,BUFSIZE,"/home/pi/RbData/%F/RbAbs%F_%H%M%S.dat",timeinfo);

	printf("\n%s\n",fileName);

	writeFileHeader(fileName, comments, probeOffset);
	fp=fopen(fileName,"a");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}
	
	probeStepsPerRevolution=350;
	piOver2StepLocation=probeStepsPerRevolution/4;
	printf("Homing linear polarizer...\n");
	//findAndSetProbeMaxTransmission();
	homeMotor(PROBE_MOTOR);
	writeTextToFile(fileName,"#HomeTransmission\n");
	collectAndRecordData(fileName, startvalue, endvalue, stepsize);
	/**

	stepMotor(PROBE_MOTOR,CLK,piOver2StepLocation/3);
	writeTextToFile(fileName,"#30Transmission\n");
	collectAndRecordData(fileName, startvalue, endvalue, stepsize);

	stepMotor(PROBE_MOTOR,CLK,piOver2StepLocation/3);
	writeTextToFile(fileName,"#60Transmission\n");
	collectAndRecordData(fileName, startvalue, endvalue, stepsize);

	stepMotor(PROBE_MOTOR,CLK,piOver2StepLocation/3);
	writeTextToFile(fileName,"#90Transmission\n");
	collectAndRecordData(fileName, startvalue, endvalue, stepsize);
	**/

	homeMotor(PROBE_MOTOR);

	setUSB1208AnalogOut(PROBEOFFSET,512);//sets vout such that 0 v offset at the probe laser

	closeUSB1208();

	graphData(fileName);

	fclose(dataCollectionFlagFile);
	remove(dataCollectionFileName);

	return 0;
}

void graphData(char* fileName){
	char buffer[BUFSIZE];
	FILE* gnuplot;
	// Create graphs for data see gnutest.c for an explanation of 
	// how this process works.
	gnuplot = popen("gnuplot","w"); 

	//getCommentLineFromFile(fileName,"#ProbeOffset:",buffer);
	//probeOffset=atof(buffer);
	//aoutConv=9e-6*pow(probeOffset,2)+.0012*probeOffset-.0651;
	//aoutInt=.6516*probeOffset-22.851;

	if (gnuplot != NULL){
		fprintf(gnuplot, "set terminal dumb size 158,32\n");
		fprintf(gnuplot, "set output\n");			
		
		sprintf(buffer, "set title '%s'\n", fileName);
		fprintf(gnuplot, buffer);

		fprintf(gnuplot, "set key autotitle columnheader\n");
		fprintf(gnuplot, "set xlabel 'Aout (Detuning)'\n");			
		fprintf(gnuplot, "set ylabel 'Transmitted Current'\n");			
		fprintf(gnuplot, "set yrange [-.1:*]\n");			
		fprintf(gnuplot, "set xrange [*:*] reverse\n");			
		//fprintf(gnuplot, "set x2range [*:*] reverse\n");			
		fprintf(gnuplot, "set x2tics nomirror\n");
		//sprintf(buffer, "plot '%s' using 1:6:7 with errorbars, '%s' using ($1*%f+%f):6:7 axes x2y1\n",fileName,fileName,aoutConv,aoutInt);
		sprintf(buffer, "plot '%s' using 1:6:7 with errorbars\n",fileName);
		fprintf(gnuplot, buffer);
		sprintf(buffer, "plot '%s' using 1:4:5 with errorbars\n",fileName);
		fprintf(gnuplot, buffer);
		sprintf(buffer, "plot '%s' using 1:2:3 with errorbars\n",fileName);
		fprintf(gnuplot, buffer);
		fprintf(gnuplot, "unset output\n"); 
		fprintf(gnuplot, "set terminal png\n");
		sprintf(buffer, "set output '%s.png'\n", fileName);
		fprintf(gnuplot, buffer);
		sprintf(buffer, "plot '%s' using 1:6:7 with errorbars,\
						 	  '%s' using 1:4:5 with errorbars,\
						 	  '%s' using 1:2:3 with errorbars\n", fileName,fileName,fileName);
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

void writeFileHeader(char* fileName, char* comments, float probeOffset){
	FILE* fp;
	float returnFloat;
	fp=fopen(fileName,"w");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}

	fprintf(fp,"#Filename:\t%s\n",fileName);
	fprintf(fp,"#Comments:\t%s\n",comments);
	getPVCN7500(CN_RESERVE,&returnFloat);
	fprintf(fp,"#CellTemp(Res):\t%f\n",returnFloat);
	getPVCN7500(CN_TARGET,&returnFloat);
	fprintf(fp,"#CellTemp(Targ):\t%f\n",returnFloat);
	fprintf(fp,"#ProbeOffset:\t%f\n",probeOffset);
	fprintf(fp,"Aout\tPUMP\tStdDev\tPROBE\tStdDev\tREF\tStdDev\n");
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

void collectAndRecordData(char* fileName, int startvalue, int endvalue, int stepsize){
	__u16 value;
	FILE* fp;
	int k=0,i;
	int nSamples;
	float involts[NUMCHANNELS];

	fp=fopen(fileName,"a");
	if (!fp) {
		printf("unable to open file: %s\n",fileName);
		exit(1);
	}
	// Allocate some memory to store measurements for calculating
	// error bars.
	nSamples = 32;
	float* measurement = malloc(nSamples*sizeof(float));

	value=startvalue;
	setUSB1208AnalogOut(PROBEOFFSET,value);
	//delay(60000);
	/** Reverse the start and end point **/
	for (value=startvalue;value < endvalue && value >= startvalue;value+=stepsize){
//	for (value=endvalue;value > startvalue && value <= endvalue;value-=stepsize){
		setUSB1208AnalogOut(PROBEOFFSET,value);
		printf("Aout %d \t",value);
		fprintf(fp,"%d\t",value);

		// delay to allow transients to settle
		//delay(10000);
		delay(100);
		for(k=0;k<NUMCHANNELS;k++){
			involts[k]=0.0;	
		}

		// grab several readings and average
		for(k=1;k<NUMCHANNELS+1;k++){
			for (i=0;i<nSamples;i++){
				getUSB1208AnalogIn(k,&measurement[i]);
				involts[k-1]=involts[k-1]+measurement[i];
				delay(1);
			}
			involts[k-1]=fabs(involts[k-1])/(float)nSamples;
			fprintf(fp,"%f\t%f\t",involts[k-1],stdDeviation(measurement,nSamples));
			printf("%f\t%f\t",involts[k-1],stdDeviation(measurement,nSamples));
		}
		fprintf(fp,"\n");
		printf("\n");
	}
	fprintf(fp,"\n");
	fclose(fp);
	free(measurement);
}
