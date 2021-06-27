PUBLIC hkCreateEntityThunk

EXTERN hkCreateEntity: PROC

_TEXT SEGMENT

hkCreateEntityThunk PROC

; i've been debugging this shit for
; like at least 6 hours, someone kill me

mov r8d, ebp
push rbp
mov rbp, rsp
and rsp, 0FFFFFFFFFFFFFFF0h
sub rsp, 30h
mov rdx, rsi
mov [rsp+20h], rax
call hkCreateEntity
mov rsp, rbp
pop rbp
ret

hkCreateEntityThunk ENDP
_TEXT ENDS
END