#!/bin/bash
# Build one commit from bench/commits.txt into bench/build/<name>.
#   bench/build.sh opt3
cd "$(dirname "$0")/.."
name=$1
sha=$(grep "^$name " bench/commits.txt | awk '{print $2}')
dir=bench/build/$name

rm -rf $dir
mkdir -p $dir
git archive $sha | tar -x -C $dir
cp inputs/realelephant.ray $dir/inputs/    # this scene was only committed with opt5
make -C $dir -j > $dir/build.log 2>&1 || echo "$name: build failed, see $dir/build.log"
