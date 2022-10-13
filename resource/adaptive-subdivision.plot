set xlabel "Thresh"
set multiplot layout 1, 2;
plot 'adaptive-subdivision.dat' using 2:1 with linespoints title columnheader
plot 'adaptive-subdivision.dat' using 2:3 with linespoints title columnheader
unset multiplot
