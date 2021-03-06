#!/bin/bash

if [ "$#" -ne 2 ]; then
	echo "usage: ./repeatRbQuickPolarization.sh <number of runs> <additional comments>"
else
    RBC="/home/pi/RbControl"
	RUNS=$1
	COMMENTS=$2

	for i in $(seq 1 $RUNS); do
		echo "Pausing for 30 seconds to give the opportunity to cancel."
		sleep 30
		sudo $RBC/scripts/RbQuickPolarizationScript.sh -15 "Run $i/$RUNS, $2"
	done

fi

