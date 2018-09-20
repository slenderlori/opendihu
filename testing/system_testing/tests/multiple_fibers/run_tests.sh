#!/bin/bash

echo "running example $(pwd) with $1 processes"

workdir=$(pwd)
variant="debug"
variant="release"

# copy fibre results from fibers system test to this folder
#cd input
#. copy.sh
cd $workdir

mkdir -p build_${variant}
cd build_${variant}

# remove old output data
rm -rf out
export output_path=/data/scratch/maierbn/multiple_fibres/out2
mkdir -p $output_path
ln -s $output_path out

export OMP_NUM_THREADS=1

# arguments: <fibre no> <cellml_file> <end_time>
#./single_fibre ../single_fibre_settings.py 1 "../input/hodgkin_huxley_1952.c" 10
mpirun -n $1 ./multiple_fibers ../multiple_fibers_settings.py

cd $workdir
