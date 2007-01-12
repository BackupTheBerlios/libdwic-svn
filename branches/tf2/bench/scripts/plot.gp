set xlabel "size (bytes)"
set ylabel "psnr (db)"
set logscale x
set grid mxtics

set terminal postscript color
set output "graph.ps"

set title "lena"
plot 	"lena.dwic.dat" title "dwic" with linespoints, \
		"lena.jpeg.dat" title "jpeg" with linespoints, \
		"lena.jp2k.dat" title "jp2k" with linespoints

set title "goldhill"
plot 	"goldhill.dwic.dat" title "dwic" with linespoints, \
		"goldhill.jpeg.dat" title "jpeg" with linespoints, \
		"goldhill.jp2k.dat" title "jp2k" with linespoints

set title "airfield"
plot 	"airfield.dwic.dat" title "dwic" with linespoints, \
		"airfield.jpeg.dat" title "jpeg" with linespoints, \
		"airfield.jp2k.dat" title "jp2k" with linespoints

set title "barbara"
plot 	"barbara.dwic.dat" title "dwic" with linespoints, \
		"barbara.jpeg.dat" title "jpeg" with linespoints, \
		"barbara.jp2k.dat" title "jp2k" with linespoints

set title "boats"
plot 	"boats.dwic.dat" title "dwic" with linespoints, \
		"boats.jpeg.dat" title "jpeg" with linespoints, \
		"boats.jp2k.dat" title "jp2k" with linespoints

set title "bridge"
plot 	"bridge.dwic.dat" title "dwic" with linespoints, \
		"bridge.jpeg.dat" title "jpeg" with linespoints, \
		"bridge.jp2k.dat" title "jp2k" with linespoints

set title "harbour"
plot 	"harbour.dwic.dat" title "dwic" with linespoints, \
		"harbour.jpeg.dat" title "jpeg" with linespoints, \
		"harbour.jp2k.dat" title "jp2k" with linespoints

set title "peppers"
plot 	"peppers.dwic.dat" title "dwic" with linespoints, \
		"peppers.jpeg.dat" title "jpeg" with linespoints, \
		"peppers.jp2k.dat" title "jp2k" with linespoints