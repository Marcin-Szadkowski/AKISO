#!/bin/bash

#zapisuje strone internetowa do pliku
lynx -dump $1 > webPage1.txt

while true;
do 
    #Czekam tyle ile podal uzytkownik
    sleep $2;

    lynx -dump $1 > webPage2.txt

    diff webPage1.txt webPage2.txt > difference.txt
    cat difference.txt >> history.txt

    cp webPage1.txt webPage2.txt

    if [ -s difference.txt ]
    then
        xmessage "Check for newly added content on website!"         
    fi
    echo "" difference.txt
done