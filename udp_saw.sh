#!/bin/bash
echo "========================= make saw ============================" >> 'build.log'
make saw #&>> 'build.log'
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

# Start the server in bkd and get its PID
./server.o &
PID_S=$!

# Run the client, which will hit the server with a datagram and expect it back
# Meanwhile, the server randomly drops packets, so it will randomly fail
echo "================ Should see [SUCCESS]/[FAILURE] ================"
./client.o -s 20 $(hostname) README.md 0
echo "================================================================"

# Cleanup
kill $PID_S

echo "[COMPLETE] Please see build.log for output from makefile"
