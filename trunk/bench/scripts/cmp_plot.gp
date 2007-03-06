set xlabel "size (bytes)"
set ylabel "psnr (db)"
set logscale x
set grid mxtics

set terminal postscript color
set output "graph.ps"

set title "lena"
plot 	"results/lena.dwic.dat" title "old" with linespoints, \
		"lena.dwic.dat" title "new" with linespoints

set title "goldhill"
plot 	"results/goldhill.dwic.dat" title "old" with linespoints, \
		"goldhill.dwic.dat" title "new" with linespoints

set title "airfield"
plot 	"results/airfield.dwic.dat" title "old" with linespoints, \
		"airfield.dwic.dat" title "new" with linespoints

set title "barbara"
plot 	"results/barbara.dwic.dat" title "old" with linespoints, \
		"barbara.dwic.dat" title "new" with linespoints

set title "boats"
plot 	"results/boats.dwic.dat" title "old" with linespoints, \
		"boats.dwic.dat" title "new" with linespoints

set title "bridge"
plot 	"results/bridge.dwic.dat" title "old" with linespoints, \
		"bridge.dwic.dat" title "new" with linespoints

set title "harbour"
plot 	"results/harbour.dwic.dat" title "old" with linespoints, \
		"harbour.dwic.dat" title "new" with linespoints

set title "peppers"
plot 	"results/peppers.dwic.dat" title "old" with linespoints, \
		"peppers.dwic.dat" title "new" with linespoints