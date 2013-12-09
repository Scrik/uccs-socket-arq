#!/bin/bash
PROTOCOL_SAW=1
PROTOCOL_GBN=2
PROTOCOL_SR=3

FILENAMES=("README.md" "PDDModel_Network99.pdf")
OUT_DIR="out"
DROP_RATES=(05 25)
PROTOCOLS=($PROTOCOL_SAW $PROTOCOL_GBN)
FRAME_SIZES=(512 2048)
DIFF_CODE_SUM=0

echo "####################################################################################################"
echo "# BEGIN CLIENT/SERVER DEMO                                                "
echo "# FILENAMES     = ${FILENAMES[@]}                                         "
echo "# DROP_RATES    = ${DROP_RATES[@]}                                        "
echo "# PROTOCOLS     = ${PROTOCOLS[@]}                                         "
echo "# FRAME_SIZES   = ${FRAME_SIZES[@]}                                       "
echo "###                                                                       "

# Clear the pkg directory to avoid confusion
mkdir out -p
mkdir pkg -p
mkdir pkg/udp -p
rm out/* -rf
rm pkg/*
mkdir out/protocol-1 -p	# SAW
mkdir out/protocol-2 -p # GBN

echo "Compiling and dumping output to: '$OUT_DIR/build.log'..."
make udp > $OUT_DIR/build.log 2>&1
EXIT_CODE=$?
echo "Make exited with code: $EXIT_CODE"
if [ $EXIT_CODE -ne 0 ]
then
	echo "EXITING! Make failed"
	exit $EXIT_CODE
fi

# Iterate over the various protocols
for frameSize in "${FRAME_SIZES[@]}"
do
echo "Frame Size: $frameSize"

	# Iterate over the various protocols
	for protocol in "${PROTOCOLS[@]}"
	do

	echo "Protocol: $protocol"

		# Iterate over the various drop rates
		for dropRate in "${DROP_RATES[@]}"
		do
			serverLogName="$OUT_DIR/protocol-$protocol/size-$frameSize.loss-$dropRate.server.log"

			echo "   Starting server in background and logging output to '$serverLogName'..."
			# Start the server in bkd and get its PID
			./server.o $dropRate $protocol > $serverLogName &
			PID_S=$!
			echo "   Server started with PID $PID_S"

			# Iterate over the input files
			for filename in "${FILENAMES[@]}"
			do
				clientLogName="$OUT_DIR/protocol-$protocol/size-$frameSize.loss-$dropRate.client.log"
				outputFilename="$OUT_DIR/protocol-$protocol/size-$frameSize.loss-$dropRate.$filename"

				echo "      Starting client in background and logging output to '$clientLogName'..."
				echo "      Saving echo file to '$outputFilename'."
				./client.o -s $frameSize $(hostname) $filename $outputFilename $protocol > $clientLogName &
				PID_C=$!
				echo "      Client started with PID $PID_C"

				echo "      Waiting for client to stop running @$PID_C..."
				echo -n "      "
				while kill -0 $PID_C > /dev/null 2>&1; do
					echo -n "."
					sleep 0.5
				done
				echo " "
				echo "      Client done."

				echo "      Dumping DIFF result to '$outputFilename.diff'..." 
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
				echo "   '-> $RESULT"
			done

			echo "   Forceably stop the server @$PID_S..."
			kill $PID_S
			echo "   Server done."

		done
	done
done

if [ $DIFF_CODE_SUM -eq 0 ]
then
	RESULT="SUCCESS[  YAY!   ]"
	# Package the SUCCESSFUL results
	cp client.o pkg/udp/.
	cp server.o pkg/udp/.
	tar -zcvf pkg/udp/$OUT_DIR.tar.gz $OUT_DIR
elif [ $DIFF_CODE_SUM -eq 1 ]
then
	RESULT="FAILURE[DIFFERENT]"
elif [ $DIFF_CODE_SUM -eq 2 ]
then
	RESULT="FAILURE[ TROUBLE ]"
else
	RESULT="FAILURE[ UNKNOWN ]"
fi

echo " '-> $RESULT"

