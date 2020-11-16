#!/usr/bin/env bash

views=( "pulsar_particles_top" "pulsar_particles_front" "pulsar_top" "pulsar_front" "pulsar_Bz_top" "pulsar_Bz_front" )

mkdir -p videos

# create videos
for view in "${views[@]}"
do
  ffmpeg -r 25 -pattern_type glob -i "${view}_*.png" -c:v libx264 -pix_fmt yuv420p videos/${view}.mp4
done

# merge videos
cd videos
ffmpeg -i pulsar_particles_top.mp4 -i pulsar_particles_front.mp4 -i pulsar_top.mp4 -i pulsar_front.mp4 -i pulsar_Bz_top.mp4 -i pulsar_Bz_front.mp4 -filter_complex "[0:v][2:v][4:v]hstack=inputs=3[top];[1:v][3:v][5:v]hstack=inputs=3[bottom];[top][bottom]vstack,format=yuv420p[v]" -map "[v]" -ac 2 merged.mp4
cd -

