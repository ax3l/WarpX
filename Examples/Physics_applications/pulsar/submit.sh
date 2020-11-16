#!/bin/bash
#BSUB -J warpx_pulsar_ascent
#BSUB -q batch-hm
#BSUB -W 07:00
#BSUB -nnodes 1
#BSUB -e warp_ascent.err
#BSUB -o warp_ascent.out
#BSUB -P APH114

#module load gcc
#module load cuda
#module load hdf5/1.10.4

export OMP_NUM_THREADS=1

jsrun -r 6 -a 1 -g 1 -c 7 -l GPU-CPU -d packed -b rs --smpiargs="-gpu" ./warpx inputs.corotating.3d.PM  > output.txt

