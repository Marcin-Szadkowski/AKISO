    ;w dosbox
    ; nasm -fbin zad4.asm -o zad4.com
    ; zad4.com
    org 100h

    mov ax, 13h
	int 10h     ;przejscie w tryb graficzny

	mov ax, 0a000h
	mov es, ax

	;mov cx, 360 ; tyle iteracji N

    mov dword [CntrA], -510*256  ;X0
@@LoopHoriz:
    mov dword [CntrB], -270*256 ;Y0
    mov word [Y], 200 ;Y=0
@@LoopVert:
    xor ecx, ecx    ;x=0
    xor edx, edx    ;y=0
    mov si, 32-1    ;kolor
@@LoopFractal:
    mov eax, ecx
    imul eax, eax   ;x^2
    mov ebx, edx
    imul ebx, ebx   ;y^2
    sub eax, ebx    
    add eax, dword [CntrA]   ; xtemp=x^2-y^2+X0
    mov ebx, ecx
    imul ebx, edx   ;x*y
    sal ebx, 1      ;2*x*y
    add ebx, dword [CntrB]   ;ytemp=2*x*y+Y0
    sar eax, 8
    sar ebx, 8
    mov ecx, eax    ; x=xtemp
    mov edx, ebx    ; y=ytemp
    imul eax, eax   ;x^2
    imul ebx, ebx   ;y^2
    add  eax, ebx   ;x^2+y^2
    sar eax, 8
    cmp eax, 1024   ;if(x^2 + y^2)> 4 then -tu trzeba wyskalowac
    jg  Break;
    dec si      ;kolor--
    jnz @@LoopFractal ;jesli si !=0 to iteruj dalej

Break:
    ;teraz trzeba obliczyc piksel do pokolorowania
    mov ax, [Y] ; wiersz =Y
    mov bx, [Y]
    shl ax, 8   ;ax*256
    shl bx, 6   ;bx*64
    add ax, bx  ;ax = ax +bx =320
    add ax, [X] ; +kolumna

    mov di, ax 
    mov byte al, [si]

    mov [es:di], al ;narysuj piksel
    add dword [CntrB], 720
    dec word [Y]
    jnz @@LoopVert ;Jesli Y != 0 to lec dalej w tej kolumnie
    add dword [CntrA], 568
    inc word [X]
    cmp word [X], 320
    jnz @@LoopHoriz
    ;ret
    xor ah, ah  ;oczekiwanie na nacisniecie klawisza
	int 16h

    mov ax, 3 ;zmiana na tryb tekstowy
	int 10h

	mov ax, 4c00h
	int 21h

X       dw 0
CntrA   dd 0    ;double word
CntrB   dd 0    ; double word
Y       dw 0
szer    dw 320
wys     dw 200