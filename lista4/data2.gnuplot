
    set terminal png size 2000,1000 enhanced font "Helvetica, 20"
        set output 'output.png'

        red = "#FF0000"; green = "#00FF00"; blue = "#0000FF"; skyblue = "#87CEEB";
        set yrange [0:1900]
        set style data histogram
        set style histogram cluster gap 1
        set style fill solid
        set boxwidth 0.9
        set xtics format ""
        set grid ytics

        set title "A Sample Bar Chart"
        plot "data2.dat" using 2:xtic(1) title "download" linecolor rgb red, \
            '' using 3 title "upload" linecolor rgb blue, \
            '' using 4 title "avg-down" linecolor rgb green, \
            '' using 5 title "avg-up" linecolor rgb skyblue
