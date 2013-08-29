#!/bin/sh

DIR=`dirname $0`
if [ $DIR != "." ]; then
  echo "Error: run script from same directory"
  exit -1
fi

if [ -d output ]; then
  rm -rf output/
fi
rsync -avrc --files-from=manifest.txt cube2/ output/
if [ -f bananabread.zip ]; then
  rm bananabread.zip
fi
pushd output && zip -r ../bananabread.zip * && popd
