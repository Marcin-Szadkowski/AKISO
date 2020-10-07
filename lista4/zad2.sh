#!/bin/bash

echo -e "PID\tPPid\tState\tCMD\t\tFILES_OPENED"
for process in $( ls /proc | grep -v [a-z] | sort -n)
do
     cmd=""
    ppid=""
     time=""
     state=""
     files_opened=""
     numberOfFiles="";   

    #jesli status istnieje, to podejmuje dzialanie
    if [ -f "/proc/$process/status" ]
    then
        line=$( cat /proc/$process/status | grep Name | cut -d ':' -f 2 | awk '{print "cmd="$1}' )
        eval $line
        line=$( cat /proc/$process/status | grep State | cut -d ':' -f 2 | awk '{print "state="$1}')
        eval $line
        line=$( cat /proc/$process/status | grep PPid | cut -d ':' -f 2 | awk '{print "ppid="$1}' )
        eval $line
        #Zliczam liczbe otwartych plikow przez proces
        numberOfFiles=$(sudo ls -l /proc/$process/fdinfo | wc -l)
        
    else
        #przechodze do nastepnej iteracji
        continue
    fi
    #wyswietlam dane
     echo -e "$process\t$ppid\t$state\t$cmd\t$numberOfFiles"

done