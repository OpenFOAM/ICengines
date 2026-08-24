#!/usr/bin/env gnuplot

set terminal pngcairo enhanced size 1200,700 font "Arial,12"
set output "chamberPressureComparison.png"

set title "Chamber Pressure Comparison"
set xlabel "Crank Angle (deg)"
set xrange [250:470]
set ylabel "Pressure (bar)"
set datafile missing NaN
set grid
set border lw 1.5
set tics out
set key top right

# Ignore OpenFOAM header lines beginning with '#'
set datafile commentschars "#"

# Number of cycles
nCycles = 3

plot \
    "constant/engineData/chamberPressure.txt" \
        using 1:($2/1e6) \
        with lines lw 3 lc rgb "#0060ad" \
        title "Experiment", \
    for [i=0:nCycles-1] \
        "postProcessing/chamber_pTAvg/0/volFieldValue.dat" \
	using ( ($1 >= 720*i && $1 <= 720*(i+1)) ? \
       	(($1 - 720*i + 360) - 720*floor(($1 - 720*i + 360)/720)) : 1/0 ):($2/1e6) \
        with lines lw 2 lc rgb "#d62728" \
        title (i==0 ? "Simulation" : "")
