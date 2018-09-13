set terminal png
set print "-"
out = sprintf("%s/stretch_plot.png",dir)
print out
set datafile separator ","
set output out
set style fill solid border -1
set style histogram rowstacked
set style data histograms
set boxwidth 0.75
set xlabel 'NodeID'
set ylabel 'Average Stretch Factor'
set yrange [0:5]
set xtics font ',10'
file = sprintf("%s/stretch_merge.csv",dir)
print file
plot file using 2:xticlabels(1) notitle
pause -1