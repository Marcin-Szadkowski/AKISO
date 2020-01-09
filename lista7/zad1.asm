   ;kompilowac: ./xasm zad1.asm
   ;uruchamiac: ./emu6502 zad1.obx -v
       ;Szybkie mnozenie
        opt f-g-h+l+o+
        org $1000

start   equ *
        LDA #20  ;mnozone zmieenne
        LDX #20
        JSR MULU
        BRK

MULU    EOR #$ff ;XOR na A i liczbe 11111111 - czyli tam gdzie byly '1' sa '0'
        STA BTA ;zapisuje wartosc A pod adres BTA 
        STX BTB ; zapisuje wartosc X pod BTB 
        LDX #0
        STX BTB+1   ;pod adresem BTB+1 zapisuje wartosc X - czyli 0
        STX RSL     ; pod RSL zapisuje 0
        STX RSL+1   ; pod RSL+1 zapisuje 0
        LDX #8      ;do X laduje 8
MUL_LOP LSR BTA ;przesuniecie bitowe w prawo
        BCS MUL_NXT ; Branch on Carry Set, C =1 
        LDA BTB
        ADC RSL
        STA RSL
        LDA BTB+1
        ADC RSL+1
        STA RSL+1
MUL_NXT ASL BTB ; lewy shift, bit najbardziej znaczacy idzie do carrfy flag, 
        ; jesli powyzsza instrukcja ustawila carry flag to ponizsza instrukcja ustawi carry flag na najmniej znaczacym miejscu
        ROL BTB+1 ; przesuniecie w lewo i najbardziej znaczcy bit ustawiony na miejscu najmniej znaczacym
        DEX         ;decrement X
        BNE MUL_LOP ;Branch on Result not Zero, Z=0 - if(X != 0) then jump
        LDA RSL
        LDX RSL+1
        RTS     ;return from subroutine

BTA     DTA b(0)    ;rezerwuje bajt pamiec
BTB     DTA a(0)    ;rezerwuje 2 bajty pamieci
RSL     DTA a(0)

        org $2E0
        dta a(start)

        end of file