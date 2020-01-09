;Kompilacja: nasm -felf zad3.asm
;           : gcc -m32 zad3.o -o zad3

section .data
  message: db "Podaj parametr dla funkcji : ", 0
  input: db "%lf", 0
  output: db "Wynik sinh : %lf", 10, 0
  output2: db "Wynik sinh^-1: %lf", 10, 0

section .bss
  x resb 8
  y resb 8
  
section .text
  global main
  extern printf
  extern scanf

main:
  push message
  call printf
  add esp, 2

  push x
  push input
  call scanf
  add esp, 6

  ;licze sinh
  finit ;resets the coprocessor and restores all the default conditions in the control and status words
  fldl2e    ;zaladuj log2(e)
  fmul qword [x]    ;mnozenie, skladnia jak w dodawaniu
  fld1  ;zaladuj do st0 liczbe 1
  fld st1
  fprem ;reszta z dzielenia, st0 = st0 mod st1
  f2xm1
  faddp ;  st1 += st0 i zdejmij ze stosu
  fscale    ;st0*2^st1, jesli st1 nie jest calkowita to jest obcinana
  fld st0   ;duplikacja na stosie
  fld1
  fdivr ;dzielenie, skladnia jak w odejmowaniu odwrotnym
  fsub  ; st0 -= st1
  fld1
  fld1
  faddp
  fdiv  ;dzielenie, skladnia jak w dodawaniu

  fst qword [y] ;zapisuje st0 w pamieci
  push dword [y+4]
  push dword [y]
  push output
  call printf
  add esp, 4 
 
  ;licze sinh^-1
  finit
  fld qword [x] ;zaladuj x
  fld st0 ;duplikacja x na stosie
  fmulp ;oblicz x^2
  fld1  ;zaladuj 1 do st0
  faddp ;policz x^2+1
  fsqrt ;policz z tego pierwiastek
  fld qword [x] ;zaladuj x
  faddp ; dodaj x do policzonego pierwiastka
  fld1  ;zaladuj 1 do st0
  fld st1 ; 
  fyl2x ;st1 = log_2(x+sqrt(x^2+1)) i zdejmij
  fldl2e ;zaladuj log2(e)
  fdivr st0, st1 ; st1/st0

  fst qword [x]
  push dword [x+4]
  push dword [x]
  push output2
  call printf
  add esp, 4

  mov ebx, 0
  mov eax, 1
  int 0x80
 