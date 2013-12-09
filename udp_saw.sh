#!/bin/bash
PROTOCOL_SAW=1
PROTOCOL_GBN=2
PROTOCOL_SR=3

FILENAMES=("README.md" "PDDModel_Network99.pdf")
OUT_DIR="out"
DROP_RATES=(00 15)
PROTOCOL=$PROTOCOL_SAW
FRAME_SIZE=4096
DIFF_CODE_SUM=0

echo "####################################################################################################"
echo "# BEGIN CLIENT/SERVER DEMO                                                "
echo "# FILENAMES     = ${FILENAMES[@]}                                         "
echo "# DROP_RATES    = ${DROP_RATES[@]}                                        "
echo "# PROTOCOL      = $PROTOCOL                                               "
echo "# FRAME_SIZE    = $FRAME_SIZE                                             "
echo "###                                                                       "

# Clear the pkg directory to avoid confusion
rm out/*
rm pkg/*

echo "Compiling and dumping output to: '$OUT_DIR/build.log'..."
make udp > $OUT_DIR/build.log 2>&1
EXIT_CODE=$?
echo " '-> Make exited with code: $EXIT_CODE"
if [ $EXIT_CODE -ne 0 ]
then
	echo "EXITING! Make failed"
	exit $EXIT_CODE
fi

# Iterate over the various drop rates
for dropRate in "${DROP_RATES[@]}"
do

	serverLogName="$OUT_DIR/$dropRate.server.log"
	echo "Starting server in background and logging output to '$serverLogName'..."
	# Start the server in bkd and get its PID
	./server.o $dropRate $PROTOCOL > $serverLogName &
	PID_S=$!
	echo " '-> Server started with PID $PID_S"

	# Iterate over the input files
	for filename in "${FILENAMES[@]}"
	do
		echo " "
		echo "********************************************************************************"
		echo "* FILE: $filename, DROP_RATE: $dropRate"
		echo "***"
		echo " "
		clientLogName="$OUT_DIR/$dropRate.client.log"
		outputFilename="$OUT_DIR/$dropRate.$filename"
		echo "   Starting client in background and logging output to '$clientLogName'..."
		echo "   Saving echo file to '$outputFilename'."
		./client.o -s $FRAME_SIZE $(hostname) $filename $outputFilename $PROTOCOL > $clientLogName &
		PID_C=$!
		echo "    '-> Client started with PID $PID_C"

		echo "   Waiting for client to stop running..."
		while kill -0 $PID_C > /dev/null 2>&1; do
			echo -e "."
			sleep 0.5
		done
		echo "    '-> Client done."

		echo "   Dumping DIFF result to '$outputFilename.diff'..." 
		diff "$filename" "$outputFilename" > "$outputFilename.diff"
		DIFF_CODE=$?
		DIFF_CODE_SUM=$((DIFF_CODE_SUM + $DIFF_CODE))
		RESULT="FAILURE[         ]"
		if [ $DIFF_CODE -eq 0 ]
		then
			RESULT="SUCCESS[  YAY!   ]"
		elif [ $DIFF_CODE -eq 1 ]
		then
			RESULT="FAILURE[DIFFERENT]"
		elif [ $DIFF_CODE -eq 2 ]
		then
			RESULT="FAILURE[ TROUBLE ]"
		else
			RESULT="FAILURE[ UNKNOWN ]"
		fi
		echo "*********************************** $RESULT ************************************"
	done

	echo "Forceably stop the server..."
	kill $PID_S
	echo " '-> Server done."

done

if [ $DIFF_CODE_SUM -eq 0 ]
then
	RESULT="SUCCESS[  YAY!   ]"
	# Package the SUCCESSFUL results
	cp client.o pkg/.
	cp server.o pkg/.
	tar -zcvf pkg/$OUT_DIR.tar.gz $OUT_DIR
elif [ $DIFF_CODE_SUM -eq 1 ]
then
	RESULT="FAILURE[DIFFERENT]"
elif [ $DIFF_CODE_SUM -eq 2 ]
then
	RESULT="FAILURE[ TROUBLE ]"
else
	RESULT="FAILURE[ UNKNOWN ]"
fi

echo " "
echo "############################################# $RESULT ##############################################"
echo " "
