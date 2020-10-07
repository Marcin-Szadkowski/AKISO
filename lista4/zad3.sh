#! /bin/bash

wget 'https://cdn2.thecatapi.com/images/cdo.jpg'
img2txt cdo.jpg
curl -s http://api.icndb.com/jokes/random | jq '.value.joke'
