#!/bin/bash
 
#wywolac z interfejsem sieciowym
 

interface=$1
received_bytes=""
old_received_bytes=""
transmitted_bytes=""
old_transmitted_bytes=""
all_received_bytes=""
all_transmitted_bytes=""
avg_rec_vel=""
avg_tran_vel=""
seconds=""
battery=""
counter=0
 
 

#Funkcja pobiera liczbe sekund dzialania systemu
#Dane sa pobierane z /proc/uptime
get_seconds(){
    local line=$(cat /proc/uptime | cut -d '.' -f 1 | awk '{print "seconds="$1}')
    eval $line
} 
#Funkcja pobiera stan baterii
#Z pliku /sys/class/power_supply/BAT0/uevent
get_battery(){
    local line=$(cat /sys/class/power_supply/BAT0/uevent | grep -m 1 POWER_SUPPLY_CAPACITY | cut -d '=' -f 2 | awk '{print "battery="$1}')
    eval $line
}
 #Funkcja pobierajaca otrzymane i wyslane bajty w pliky /proc/net/dev
get_bytes()
{
    local line=$(cat /proc/net/dev | grep $interface | cut -d ':' -f 2 | awk '{print "received_bytes="$1, "transmitted_bytes="$9}')
    eval $line #wykonanie instrukcji zapisanej w zmiennej line
}
#Funkcja obliczajaca chwilowa predkosc pobierania oraz przesylania danych na podstawie nowych i starych wartosci
#Funkcja zwraca predkosc w MB/s, KB/s i B/s w zaleznosci od rzedu wielkosci
get_velocity()
{
    value=$1    
    old_value=$2
 
    let vel=$(( value-old_value ))
    let velKB=$vel/1024
    let velMB=$velKB/1024
    if [ $velMB != 0 ]
    then
        echo -n "$velMB MB/s"
    elif [ $velKB != 0 ]
    then
        echo -n "$velKB KB/s"
    else
        echo -n "$vel B/s"
    fi
}
#Funkcja oblicza srednia predkosc pobierania i przesylania danych na podstawie wartosci poczatkowych i ostatnich pobranych
#Funkcja zwraca predkosc w MB/s, KB/s i B/s w zaleznosci od rzedu wielkosci
 calc_avarage()
 {
     value=$1
     all_value=$2
     all_value=$(( value - all_value))

     let avg_vel=$(( all_value/counter))
     let avg_velKB=$avg_vel/1024
     let avg_velMB=$avg_velKB/1024
    if [ $avg_velMB != 0 ]
    then
        echo -n "$avg_velMB MB/s"
    elif [ $avg_velKB != 0 ]
    then
        echo -n "$avg_velKB KB/s"
    else
        echo -n "$avg_vel B/s"
    fi

     
 }
 #Funkcja obliczajaca czas dzialania systemu
 #Czas jest zwracany w formacie dd:hh:mm:ss
 show_time(){
    s=$1
    days=$(( s / 86400))
    hours=$(( s/3600%24))
    (( minutes = s/60%60))
    (( sec = s%60))
    if [ $days -gt 0 ]
    then
        echo "Dni: $days: Godziny: $hours: Minuty: $minutes: Sekundy: $sec"
    elif [ $hours -gt 0 ]
    then
        echo "Godziny: $hours: Minuty: $minutes: Sekundy: $sec"
    elif [ $minutes -gt 0 ]
    then
        echo "Minuty: $minutes: Sekundy: $sec" 
    else
        echo "Sekundy: $sec" 
    fi
 }
 #Funkcja wyswietlajaca procentowy stan baterii
 show_battery(){
     echo "Stan baterii: $battery%"
 }
 #Funkcja zamieniajaca wartosci w MB/s i KB/s na B/s
 #Wykorzystywana do przesylania danych do wykresu
 transToB(){
     local value=$1
     local range=$2

     if [ $range = "MB/s" ]
     then 
        echo $(( $value * 1024 * 1024 ))
    elif [ $range = "KB/s" ]
    then
        echo $(( $value * 1024 ))
    else 
        echo $value
    fi
 }
 #Funkcja wyswietlajaca obciazenie procesora
 show_loadAvg(){
     line=$(cat /proc/loadavg)
     echo $line
 }
 #Funkcja tworzajaca wykres
 make_plot(){
     `touch data2.gnuplot`
     echo "
    set terminal png size 2000,1000 enhanced font \"Helvetica, 20\"
        set output 'output.png'

        red = \"#FF0000\"; green = \"#00FF00\"; blue = \"#0000FF\"; skyblue = \"#87CEEB\";
        set yrange [0:1900]
        set style data histogram
        set style histogram cluster gap 1
        set style fill solid
        set boxwidth 0.9
        set xtics format \"\"
        set grid ytics

        set title \"A Sample Bar Chart\"
        plot \"data2.dat\" using 2:xtic(1) title \"download\" linecolor rgb red, \\
            '' using 3 title \"upload\" linecolor rgb blue, \\
            '' using 4 title \"avg-down\" linecolor rgb green, \\
            '' using 5 title \"avg-up\" linecolor rgb skyblue" > data2.gnuplot

      # gnuplot -e "filename='data2.dat'" data2.gnuplot
      gnuplot -persist <<-EOFMarker
        load "data2.gnuplot"
EOFMarker
 }
prepare_file(){
    touch dane.txt
    echo "" > dane.txt
    sed -i '1d' dane.txt
    for i in 1 2 3 4 5 6
    do
        echo -e "0\t0\t0\t0" >> dane.txt
    done
}

scale(){
    value=$1
    echo $(( value/100 ))
}
show_data(){
    column=$1
   # tput setab 4
    for value in $( sed 's/|//' dane.txt | awk -v c=$column '{print $c}' )
    do
       # tput cud1 #move the cursor down 1 line
        count=0
        
        value=$( scale $value )
        while [ $count !=  $value ]
        do 
            echo -n "|"            
            (( count++ ))
        done
        echo ""
    done
    #tput setab 0
}
linearScale(){
    pos=5
    step=5
    value=500
    counter=1
    tput cup 11 0
    echo -n "0"
    while [ 100 -gt $pos  ]
    do
        tput cup 11 $pos 
        echo -e $(( $value*$counter))
        (( counter++))
        (( pos+=$step))
    done
}
# Przypisanie poczatkowych wartosci
get_bytes
old_received_bytes=$received_bytes
old_transmitted_bytes=$transmitted_bytes
all_received_bytes=$received_bytes
all_transmitted_bytes=$transmitted_bytes
 
#Informacja o uruchomieniu skryptu
echo "Please wait...";
sleep 1;
echo "";
 
#Tworze folder, w ktorym beda przechowywane dane do wykresu 
touch data2.dat
echo -e "time\tDOWNLOAD\tUPLOAD\tAVG_DOWN\tAVG_UP" > data2.dat

#Tworze plik, z ktorego bedzie rysowany wykres w terminalu
prepare_file
#Glowna petla, ktora jest wykonywana do czasu zakonczenia programu przez uzytkownika
tput civis
while true; 
do
 
    #Update transmisji danych, czasu dzialania systemu
    get_bytes
    get_seconds
    get_battery
    (( counter++))   
    # Obliczenie predkosci przesylu
    vel_recv=$(get_velocity $received_bytes $old_received_bytes)
    vel_trans=$(get_velocity $transmitted_bytes $old_transmitted_bytes)
    # Obliczenie srednich predkosci
    avg_rec_vel=$(calc_avarage $received_bytes $all_received_bytes)
    avg_tran_vel=$(calc_avarage $transmitted_bytes $all_transmitted_bytes)

    #zapisuje dane w pliku   
    echo -e "$counter\t$(transToB $vel_recv)\t$(transToB $vel_trans)\t$(transToB $avg_rec_vel)\t$(transToB $avg_tran_vel)" >> data2.dat

    
    #Z tego pliku bedzie tworzony wykres w terminalu
    echo -e "$(transToB $vel_recv)\t$(transToB $vel_trans)\t$(transToB $avg_rec_vel)\t$(transToB $avg_tran_vel)" >> dane.txt
    #I usuwam pierwszy od gory wiersz danych tak zeby w pliku bylo 6 ostatnisz pomiarow
    sed -i '1d' dane.txt

    #Wyswietlenie wartosci
    tput clear
    echo -e "$interface DOWN:$vel_recv\tUP:$vel_trans\r"
    echo -e "$interface AVG_DOWN:$avg_rec_vel\tAVG_UP:$avg_tran_vel\r"
    echo ====================================================
    show_time $seconds 
    show_battery
    echo ====================================================
    echo   Avarage load
    echo -e "1   5 \t15\tRUN/EXIST   recently_PID"
    echo `show_loadAvg`

    #wywolanie funkcji towrzacej wykres    
    make_plot

    
    tput bold
    echo "=============================== Wykres w KB/s ==================================="
    linearScale
    tput setaf 1
 
    echo "Download"
    show_data 1
  
    tput setaf 2
    echo "Upload"
    show_data 2
    echo "Avarage download"
    tput setaf 3
    show_data 3
    tput setaf 4
    echo "Avarage upload" 
    show_data 4

    tput setaf 7

    #Przepisanie nowych danych na stare dane
    old_received_bytes=$received_bytes
    old_transmitted_bytes=$transmitted_bytes
    
 
    #Dane sa aktualizowane co sekunde
    sleep 1;
 
done
