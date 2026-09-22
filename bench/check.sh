#!/bin/bash
# After bench/run.sh: check that every commit rendered the same images as base.
# base and opt1 cannot finish the elephant, so it is checked against opt2 instead.
cd "$(dirname "$0")/results"

for name in opt1 opt2 opt3 opt4 opt5 opt6 opt7 opt8; do
   for test in piano globe sphere; do
      cmp -s base-$test.ppm $name-$test.ppm && echo "$name $test: same as base" || echo "$name $test: DIFFERENT from base"
   done
done
for name in opt3 opt4 opt5 opt6 opt7 opt8; do
   cmp -s opt2-elephant.ppm $name-elephant.ppm && echo "$name elephant: same as opt2" || echo "$name elephant: DIFFERENT from opt2"
done
