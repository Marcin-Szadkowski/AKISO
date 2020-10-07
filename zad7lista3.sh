#!/bin/bash
# Zamiana snakow
OIFS="$IFS"
IFS=$'\n'
for i in $( ls | grep [A-Z])
do 
	file="$PWD"
	new=`echo $i | tr '[A-Z]' '[a-z]'`
	

	mv  "$file/"$i"" "$file/"$new""	
done
