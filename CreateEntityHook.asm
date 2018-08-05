PUBLIC hkCreateEntityThunk

EXTERN hkCreateEntity: PROC

_TEXT SEGMENT

hkCreateEntityThunk PROC

push rsp
and rsp, 0FFFFFFFFFFFFFFF7h
sub rsp, 30h
mov rdx, rsi
mov [rsp+20h], rax
call hkCreateEntity
add rsp, 30h
pop rsp
ret

hkCreateEntityThunk ENDP
_TEXT ENDS
END