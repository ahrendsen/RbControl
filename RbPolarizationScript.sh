#!/bin/bash

# script to take faraday scan data and absoption data
# ALL OF THE INFORMATION GATHERED BY THIS SCRIPT IS USED TO CALCUATE NUMBER DENSITY AND Rb POLARIZATION
# Usage:
#
#		sudo ./RbPolarizationScript <aout start> <aout end> <step size> <RbScan start> <RbScan end> <additional comments>

if [ "$#" -ne 4 ]; then
	echo "usage: sudo ./RbPolarizationScript.sh <probeOffset> <magnet1> <magnet2> <additional comments>"
else

	# Take a picture of the chamber
	DATE=$(date +'%Y-%m-%d')
	FILENAME=$(date +'%Y-%m-%d_%H%M%S').jpg
#	mkdir -p /home/pi/RbPictures/$DATE
	# take Picture takes the filename and a subfolder to store the 
	# file in within the RbPicture folder
#	ssh pi@irpi "/home/pi/karlCode/takeRemotePicture.sh $FILENAME $DATE"

	#set laser flag to unblock the beam.
	echo "Unblocking Pump Beam..."
	sudo /home/pi/RbControl/setLaserFlag 1 0

	# set QWP for pi light
	echo "Setting QWP for Pi light..."
	sudo /home/pi/RbControl/setWavePlate 66

	echo "Faraday Scan: pi pump..."
	sudo /home/pi/RbControl/faradayScan "$1" "$2" "$3" "$4, pi pump"

	# Unblock pump beam for sure
	echo "Unblocking pump beam..."
	sudo /home/pi/RbControl/setLaserFlag 1 0

	#echo "Set magnets to ~10 G"
	#read -p "Input Magnet Voltage 1" magVolt1
	#read -p "Input Magnet Voltage 2" magVolt2

	# set QWP for s+ light
	echo "Setting QWP for S+ light..."
	sudo /home/pi/RbControl/setWavePlate 12
	echo "Faraday Scan: S+ pump..."
    sudo /home/pi/RbControl/faradayScan "$1" "$2" "$3" "$4, s+ pump"

	# set QWP for s- light
	echo "Setting QWP for S- light..."
	sudo /home/pi/RbControl/setWavePlate 99
	echo "Faraday Scan: S- pump..."
    sudo /home/pi/RbControl/faradayScan "$1" "$2" "$3" "$4, s- pump"
fi
