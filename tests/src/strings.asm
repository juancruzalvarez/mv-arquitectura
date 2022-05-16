str1 EQU "string1"
str2 EQU "string2"
;por como esta diseñado el traductor, las string se guardan en orden contrario, primero guarda str2 y despues str1
MOV EAX, str2  ;el valor de str2 deberia ser 2, ya que arranca se guarda justo despues de las instrucciones y el programa tiene dos instrucciones.
MOV EBX, str1  ;el valor de str1 deberia ser 10, ya que se guarda despues de str2 (1 caracter es \0).