#!/bin/bash
# Zamiana snakow
file="$PWD"
nazwa="-ABC DEF"
nazwa=`echo $nazwa | tr '[A-Z]' '[a-z]'`
echo $nazwa


