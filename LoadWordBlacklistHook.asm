PUBLIC hkLoadWordBlacklistThunk

EXTERN hkLoadWordBlacklist: PROC

_TEXT SEGMENT

hkLoadWordBlacklistThunk PROC

; stack is still fucked but it's good. make sure to save rcx,rdx,r8,r9
; we're 0x20 short on rsp

mov     [rsp+28h], rbx  ; mov     [rsp+18h], rbx
push    rsi
push    rdi
push    r14
push    rcx
push    rdx
push    r8
push    r9
sub     rsp, 10h
mov     rax, HOOK
push    rax
sub     rsp, 40h ; sub     rsp, 20h
mov     r14, 14147B490h
mov     r14, [r14]
jmp	    qword ptr[rsp + 90h] ; call	qword ptr[rsp + 58h]
HOOK:
add     rsp, 10h
test    eax, eax
pop     r9
pop     r8
pop     rdx
pop     rcx
jz      DONE
call    hkLoadWordBlacklist
add     rsp, 28h
DONE:
ret
hkLoadWordBlacklistThunk  ENDP
_TEXT ENDS
END