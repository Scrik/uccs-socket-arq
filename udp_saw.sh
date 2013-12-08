#!/bin/bash
IN_FILENAME='README.md'
OUT_FILENAME='out/README.md'
DROP_RATE=1
PROTOCOL=1
FRAME_SIZE=1024

echo "#############################################################################################"
echo "# BEGIN CLIENT/SERVER DEMO                                               "
echo "# IN_FILENAME  = $IN_FILENAME                                            "
echo "# OUT_FILENAME = $OUT_FILENAME                                           "
echo "# DROP_RATE    = $DROP_RATE                                              "
echo "# PROTOCOL     = $PROTOCOL                                               "
echo "# FRAME_SIZE   = $FRAME_SIZE                                             "
echo "###                                                                      "

# Clear the pkg directory to avoid confusion
rm pkg/*

echo "========================= make saw ============================" >> 'build.log'
make saw > out/build.log
EXIT_CODE=$?
echo "make exited with code: $EXIT_CODE"
if [ $EXIT_CODE -ne 0 ]
then
	echo "EXITING! Make failed"
	exit $EXIT_CODE
fi
echo "===============================================================" >> 'build.log'

echo "========== Should see up-to-date client.o and server.o ========="
ls *.o -l
echo "================================================================"


# Run the client, which will hit the server with a datagram and expect it back
# Meanwhile, the server randomly drops packets, so it will randomly fail
echo "======= Logging to 'out/client.log', 'out/server.log'  ========="

# Start the server in bkd and get its PID
./server.o $DROP_RATE $PROTOCOL > out/server.log &
PID_S=$!

./client.o -s $FRAME_SIZE $(hostname) "$IN_FILENAME" 0 > out/client.log &
PID_C=$!

echo "PROCESSES ==> SERVER: $PID_S    CLIENT: $PID_C"

echo "================================================================"

echo "================= Politely kill the client PID ================="
while kill -0 $PID_C > /dev/null 2>&1; do
	echo -e "."
	sleep 0.5
done
echo "================================================================"
echo "================ Forcefully kill the server PID ================"
kill $PID_S
echo "================================================================"

echo "[COMPLETE] Please see build.log for output from makefile"
echo ""

echo "======= Dumping DIFF result to '$OUT_FILENAME.diff' ========="
diff "$IN_FILENAME" "$OUT_FILENAME" > "$OUT_FILENAME.diff"
DIFF_CODE=$?
RESULT="FAILURE[         ]"
if [ $DIFF_CODE -eq 0 ]
then
	RESULT="SUCCESS[  YAY!   ]"

	# Package the SUCCESSFUL results
	cp client.o pkg/.
	cp server.o pkg/.
	tar -zcvf pkg/out.tar.gz out
elif [ $DIFF_CODE -eq 1 ]
then
	RESULT="FAILURE[DIFFERENT]"
elif [ $DIFF_CODE -eq 2 ]
then
	RESULT="FAILURE[ TROUBLE ]"
else
	RESULT="FAILURE[ UNKNOWN ]"
fi
echo "================================================================"

echo " "
echo "################################### $RESULT ######################################"
echo " "
