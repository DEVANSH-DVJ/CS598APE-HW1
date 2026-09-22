#!/bin/bash
# Build each commit and run the four tests on it.
#   bench/run.sh              all commits in bench/commits.txt
#   bench/run.sh opt3 opt4    only these
cd "$(dirname "$0")/.."
names=${*:-$(grep -v '^#' bench/commits.txt | cut -d' ' -f1)}

for name in $names; do
   bench/build.sh $name
   for test in piano globe sphere elephant; do
      bench/test.sh $name $test
   done
done
