#!/bin/bash
echo "========================= make tcp ============================" >> 'build.log'
make tcp &>> 'build.log'
echo "===============================================================" >> 'build.log'

echo "========== Should see up-to-date client.o and server.o ========="
ls *.o -l
echo "================================================================"

# Start the server in bkd and get its PID
./server.o &
PID_S=$!

# Save current dir to change back to it at end
REPO=$(pwd)
cp client.o ~/Downloads
cd ~/Downloads

# Run the client, downloading the README
./client.o $(hostname) README.md
echo "================== Should see README.md below =================="
ls README.md -l
echo "================================================================"

# Cleanup
cd $REPO
kill $PID_S

echo "[COMPLETE] Please see build.log for output from makefile"
