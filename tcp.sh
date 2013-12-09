#!/bin/bash
FILENAMES=("README.md" "PDDModel_Network99.pdf")
OUT_DIR=out
DIFF_CODE_SUM=0

echo "####################################################################################################"
echo "# BEGIN CLIENT/SERVER DEMO                                                "
echo "# FILENAMES     = ${FILENAMES[@]}                                         "
echo "###                                                                       "

# Clear the pkg directory to avoid confusion
mkdir out -p
mkdir pkg -p
mkdir pkg/tcp -p
rm out/* -rf
rm pkg/tcp*

echo "Compiling and dumping output to: '$OUT_DIR/build.log'..."
make tcp > $OUT_DIR/build.log 2>&1
EXIT_CODE=$?
echo "Make exited with code: $EXIT_CODE"
if [ $EXIT_CODE -ne 0 ]
then
	echo "EXITING! Make failed"
	exit $EXIT_CODE
fi

serverLogName="$OUT_DIR/protocol-$protocol/size-$frameSize.loss-$dropRate.server.log"

echo "   Starting server in background and logging output to '$serverLogName'..."
# Start the server in bkd and get its PID
./server.o $dropRate $protocol > $serverLogName &
PID_S=$!
echo "   Server started with PID $PID_S"

# Iterate over the input files
for filename in "${FILENAMES[@]}"
do
	clientLogName="$OUT_DIR/client.log"
	outputFilename="$OUT_DIR/$filename"

	echo "      Starting client in background and logging output to '$clientLogName'..."
	echo "      Saving echo file to '$outputFilename'."
	./client.o $(hostname) $filename > $clientLogName &
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

if [ $DIFF_CODE_SUM -eq 0 ]
then
	RESULT="SUCCESS[  YAY!   ]"
	# Package the SUCCESSFUL results
	cp client.o pkg/tcp/.
	cp server.o pkg/tcp/.
	tar -zcvf pkg/tcp/$OUT_DIR.tar.gz $OUT_DIR
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

