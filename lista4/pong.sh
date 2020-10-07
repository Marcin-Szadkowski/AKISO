#!/bin/bash

#Poruszanie:
#Gracz lewy: q a
#Gracz prawy: p l

time=.1
ARG=$1
[ ${ARG} ] && [ ${ARG} -gt 0 ] && [ ${ARG} -le 20 ] && time=$(echo "2/${ARG}"|bc -l)

#Gdy zmienna globalna GAMEOVER przyjmie wartos 1 to gra sie zakonczy
GAMEOVER=0

leftBatLoc=0 	#gorna wspolrzedna lewej rakietki
rightBatLoc=0	#gorna wspolrzedna prawej rakietki
ballXLoc=45 	#wspolrzedna X pileczki
ballYLoc=20		#wspolrzedna Y pileczki
B_SPEED=1 	
DIRECTION_X=-1 	#Wspolrzedna kierunku, jestli wynosi -1 to pileczka porusza sie lewo
DIRECTION_Y=1	#Wspolrzedna kierunku, jesli wynosi 1 to pileczka porusza sie do gory

#Wymiary rakietek
batWidth=2
batHeight=5
#Wymiary pilki
ballWidth=4
ballHeight=2

#Przypisanie wymiarow okna
YX=$(/bin/stty -a |grep row |awk '{print $5$7}'|awk -F';' '{print $1" "$2}')
winWidth=${YX##* }
winHeight=${YX%% *}


#Aby znalezc strefe po, ktorej pilka moze sie poruszac musimy
#Odjac od wymiarow okna wymiary pilki
W_SAFE_X=$[${winWidth}-${ballWidth}]
W_SAFE_Y=$[${winHeight}-${ballHeight}]

#Funkcja rysujaca pilke
printBallAtXY(){
	local X=$1
	local Y=$2
	echo -ne "\033[$[${Y}+0];${X}f ## \033[$[${Y}+1];${X}f####\033[$[${Y}+2];${X}f####\033[$[${Y}+3];${X}f ## "
	
}
#Funkcja rysujaca lewa rakietke
printLbat(){
	local Y=$1
	echo -ne "\033[$[${Y}+0];0f||\033[$[${Y}+1];0f||\033[$[${Y}+2];0f||\033[$[${Y}+3];0f||\033[$[${Y}+4];0f||"
}
#Funkcja rysujaca prawa rakietke
printRbat(){
        local Y=$1
	local X=$[${winWidth}-1]
        echo -ne "\033[$[${Y}+0];${X}f||\033[$[${Y}+1];${X}f||\033[$[${Y}+2];${X}f||\033[$[${Y}+3];${X}f||\033[$[${Y}+4];${X}f||"
}

echo -e "\033c"
while [ ${GAMEOVER} -eq 0 ] ; do
	
	#Wczytaj 1 znak
	#W czasie time i wczytaj do direction
	read -n 1 -t ${time} direction
	case "${direction}" in
		q) leftBatLoc=$[${leftBatLoc}-1];;
		a) leftBatLoc=$[${leftBatLoc}+1];;
		p) rightBatLoc=$[${rightBatLoc}-1];;
		l) rightBatLoc=$[${rightBatLoc}+1];;
	esac
	#Jesli lewa rakietka wchodzi poza plansze to ustaw ja w granicach
	{ [ ${leftBatLoc} -lt 0 ] && leftBatLoc=0 ; } || { [ ${leftBatLoc} -gt ${winHeight} ] && leftBatLoc=${winHeight} ; }
	#Jesli prawa rakietka wchodzi poza plansze to ustaw ja w granicach
	{ [ ${rightBatLoc} -lt 0 ] && rightBatLoc=0 ; } || { [ ${rightBatLoc} -gt ${winHeight} ] && rightBatLoc=${winHeight} ; }

	#Jezeli pilka dotyka ktorejs ze scianek pionowych to zmien jej kierunek w tej osi
	{ [ ${ballYLoc} -eq 0 ] || [ ${ballYLoc} -eq ${W_SAFE_Y} ] ; } && { DIRECTION_Y=$[${DIRECTION_Y}*(-1)] ; }
	#Jezeli pilka dotyka ktorejs ze scianek poziomych to zmien jej kierunek w tej osi

	#Jezeli pilka dotarla do ktorej ze stron gracza
	if [ ${ballXLoc} -eq ${W_SAFE_X} ] || [ ${ballXLoc} -eq ${batWidth} ] ; then 
		if #Ktora to strona
			#Czy to lewa strona?
			[ ${ballXLoc} -eq ${batWidth} ] && 	
			#Czy to pod gornym krancem rakietki
			[ ${ballYLoc} -ge ${leftBatLoc}   ] && 	
			#Czy to ponad dolnym krancem rakietki
			[ ${ballYLoc} -le $[${leftBatLoc}+${batHeight}] ] ; 

		then #Jesli warunki sa spelnione to znaczy, ze gracz odbil pilke
			#Wiec pilka zmienia kierunek
			echo "bump"
			DIRECTION_X=$[${DIRECTION_X}*(-1)] ;
			TEMPVAL=AAAA
		elif #Jesli to nie lewa rakietka

			[ ${ballXLoc} -eq ${W_SAFE_X}  ] && 		
			[ ${ballYLoc} -ge ${rightBatLoc}   ] && 		
			[ ${ballYLoc} -le $[${rightBatLoc}+${batHeight}] ] ; 

		then 
			#Jesli warunki sa spelnione to prawy gracz odbil pilke
			echo "bump"
			#Zmien kierunek pilki
			DIRECTION_X=$[${DIRECTION_X}*(-1)] ; 
		else
			#Zaden z warunkow nie zostal speliony wiec KONIEC GRY
			GAMEOVER=1
		fi ; 
	fi;

	echo -e "\033c"
	ballXLoc=$[ ${ballXLoc} + $[ ${B_SPEED} * ${DIRECTION_X} ] ]
	ballYLoc=$[ ${ballYLoc} + $[ ${B_SPEED} * ${DIRECTION_Y} ] ]
	
	printBallAtXY ${ballXLoc} ${ballYLoc}
	printLbat ${leftBatLoc}
	printRbat ${rightBatLoc}
	
done
echo -e "\033c"
echo KONIEC GRY