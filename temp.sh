#!/bin/bash/gnuplot -persist

set title "Location data"
set xlabel "location"
set ylabel "count"
set grid
plot "dane.txt" u (column(0)):2:xtic(1) w l title "","dane.txt" u (column(0)):3:xtic(1) w l title ""