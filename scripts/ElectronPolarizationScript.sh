#!/bin/bash
#
# Usage:
#
#		./ElectronPolarizationScript <aout background> <aout Helium Excited> <dwell> <additional comments>
#

if [ "$#" -ne 5 ]; then
	echo "usage: ./ElectronPolarizationScript.sh"
	echo "				<aout energy>" 
	echo "				<dwell>" 
	echo "				<ammeter scale>" 
	echo "				<pump detuning (1.5 is max)>"
	echo "				<additional comments>"
else
    AOUT=$1
    DWELL=$2
	AMMETERSCALE=$3
    DETUNE=$4
    COMMENTS=$5
	LEAKCURRENT=0

	source LoadWaveplatePositions.sh

	NUMRUN=5

	for a in $AOUT; do 
		echo "About to start next sequence of runs..."
		sleep 5
		for i in $( seq 1 $NUMRUN ); do 
			echo "Running $a of ($AOUT)"
			echo "Checking in on detuning..."
			sudo $RBC/setPumpDetuning $DETUNE

			echo "Blocking probe beam..."
			sudo $RBC/setLaserFlag $PROBE $BLOCKED

			echo "Blocking pump beam..."
			sudo $RBC/setLaserFlag $PUMP $BLOCKED

			echo "No pump..."
			PAUSE=30
			echo "$PAUSE second pause to allow settling."
			sleep $PAUSE
			sudo $RBC/polarization "$a" "$DWELL" "$AMMETERSCALE" "$LEAKCURRENT" "$COMMENTS, AOUT->$a, pump->none, Run $i/$NUMRUN"

			echo "Unblocking pump beam..."
			sudo $RBC/setLaserFlag $PUMP $UNBLOCKED

			#echo "Setting pump to Pi..."
			#sudo $RBC/setWavePlate $PIPOS
			#sleep 10
			#sudo $RBC/polarization "$a" "$DWELL" "$AMMETERSCALE" "$LEAKCURRENT" "$COMMENTS, AOUT->$a, pump->pi"

			echo "Setting pump to S+..."
			sudo $RBC/setWavePlate $SPLUSPOS
			sleep 10
			sudo $RBC/polarization "$a" "$DWELL" "$AMMETERSCALE" "$LEAKCURRENT" "$COMMENTS, AOUT->$a, pump->s+, Run $i/$NUMRUN"

			echo "Setting pump to S-..."
			sudo $RBC/setWavePlate $SMINUSPOS
			sleep 10
			sudo $RBC/polarization "$a" "$DWELL" "$AMMETERSCALE" "$LEAKCURRENT" "$COMMENTS, AOUT->$a, pump->s-, Run $i/$NUMRUN"

			echo "Unblocking probe beam..."
			sudo $RBC/setLaserFlag $PROBE $UNBLOCKED
		done 
	done

	sudo $RBC/setHeliumTarget 0
fi
