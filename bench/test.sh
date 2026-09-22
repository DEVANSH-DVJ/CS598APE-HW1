#!/bin/bash
# Run one test on one built commit REPS times and print the times in seconds.
#   bench/test.sh opt3 piano
# Tests: piano, globe, sphere, elephant.
# The image is saved as bench/results/<name>-<test>.ppm.
# Environment: REPS (default 3), THREADS (default 1), FRAMES (default 1)
cd "$(dirname "$0")/.."
name=$1
test=$2
case $test in
   piano)    args="-i inputs/pianoroom.ray    -W 1000 -H 1000" ;;
   globe)    args="-i inputs/globe.ray        -W 1000 -H 1000 -a inputs/globe.animate" ;;
   sphere)   args="-i inputs/elephant.ray     -W 100  -H 100  -a inputs/elephant.animate" ;;
   elephant) args="-i inputs/realelephant.ray -W 100  -H 100  -a inputs/elephant.animate" ;;
esac
mkdir -p bench/results
out=$PWD/bench/results/$name-$test.ppm

cd bench/build/$name
echo -n "$name $test:"
for r in $(seq ${REPS:-3}); do
   OMP_NUM_THREADS=${THREADS:-1} timeout 120 ./main.exe $args -F ${FRAMES:-1} --ppm --no-movie -o $out > run.log 2>&1
   if [ $? -ne 0 ]; then echo " timed out or failed"; exit 1; fi
   echo -n " $(grep -o 'images=[0-9.]*' run.log | cut -d= -f2)"
done
echo
