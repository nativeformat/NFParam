#!/bin/bash

if [ $# -lt 2 ]
then
  echo "Usage: sh plot.sh <input_data_file> <output_image_file>"
  exit 0
fi

input=$1
output=$2
gpfile=$(find . | grep "params.gp")

echo Using gnuplot script $gpfile with input data = $input and output image = $output
gnuplot -e "filename='$input';output='$output'" $gpfile


