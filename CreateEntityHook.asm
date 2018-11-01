PUBLIC hkCreateEntityThunk

EXTERN hkCreateEntity: PROC

_TEXT SEGMENT

hkCreateEntityThunk PROC

; i've been debugging this shit for
; like at least 6 hours, someone kill me

; old

push rsp
and rsp, 0FFFFFFFFFFFFFFF7h
sub rsp, 30h
mov rdx, rsi
mov [rsp+20h], rax
call hkCreateEntity
add rsp, 30h 
xor rdi, rdi
pop rsp
ret

; new

;push rsp
;and rsp, 0FFFFFFFFFFFFFFF7h
;sub rsp, 30h
;mov rax, [rcx+8]
;mov r9d, [rsp+40h]
;mov r8d, ebp
;mov rdx, rsi
;mov [rsp+20h], rax
;call hkCreateEntity
;add rsp, 30h
;pop rsp
;ret


hkCreateEntityThunk ENDP
_TEXT ENDS
END