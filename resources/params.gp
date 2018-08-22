set term png
set title "Param values"
set output output
set xlabel "Time (s)"
set ylabel "Value"
set xtics 0, 0.1, 1
set ytics 0, 0.1, 1
set grid ytics lt 0 lw 1 lc rgb "#bbbbbb"
set grid xtics lt 0 lw 1 lc rgb "#bbbbbb"
plot filename title "" with lines lt 1 lw 2
