;Program pobiera wartosc z ekranu i ja wypisuje
;kompilacja: nasm -f elf zad2.asm
;linkowanie: ld -m elf_i386 zad2.o -o zad2
;uruchomienie: ./zad2
global _start

section .text
_start:
        ;pobieram wartosc
        mov     eax, 3
        mov     ebx, 2
        mov     ecx, input
        mov     edx, 1
        int     0x80

        mov     byte [counter], '1'; ;counter =1
L2:     mov     byte [var], '1' ;var= 1
L1:     mov     ecx, var
        call    print
        mov     ecx, space
        call    print
        inc     byte [var] ; var++
        mov     al, [counter] ;laduje zmienne do rejestrow 8-bitowych
        mov     ah, [var]
        cmp     al, ah ;porownuje
        jge     L1 ;jesli counter jest wiekszy od var to skocz do L1
        mov     ecx, newLine 
        call    print ;wypisuje znak nowej linii
        inc     byte [counter] ;counter++

        mov     al, [input]
        mov     ah, [counter]
        cmp     al, ah 
        jge     L2      ;jesli input jest wiekszy niz counter to skocz do L2

        mov     ebx, 0
        mov     eax, 1
        int     0x80
;Aby cos wypisac, najpierw zaladuj to do ecx, a potem wywolaj print       
print:
        mov     eax, 4
        mov     ebx, 2
       ; mov     ecx, input
        mov     edx, 1
        int     0x80
        ret

section .data
        newLine:        db  0ah
        space:          db ' '
        counter:        db '0'
        var:            db '0'
section .bss
input:   resb 1 ;deklaruje byte pamieci
