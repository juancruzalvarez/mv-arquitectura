;archivo asembler para probrar errores del traductor
;segmento demaciado grande.
\\DATA 10000000
;segmento con valor negativo
\\EXTRA -300

BASE EQU 5618f          ; define un simbolo con un valor invalido.
MOV  EAX, BASE          ; utiliza un simbolo con valor invalido.

DUP EQU 32
DUP EQU 26235           ; simbolo duplicado

MOV  EAX, t             ; t es un simbolo desconocido.
MOV  EAX, [EAX+]        ; offset faltante .
MOV  EAX, 34fe          ; 34fe no es numero valido.
MOV  EAX, [P+3]         ; p no es ningun registro.
MOV  EAX,               ; falta un operando.
MOB  EAX, 65            ; mob no es un mnemonico.
MOV  EAX, [EBX+43       ; falta ]
MOV  EAX, 5000000       ; advertencia truncado de operando
MOV  EAX, -5000000      ; advertencia truncado de operando

