PUBLIC hkUpdateModelPartsThunk

EXTERN hkUpdateModelParts: PROC

_TEXT SEGMENT

hkUpdateModelPartsThunk PROC

; push rsp
; sub rsp, 20h
; call hkUpdateModelParts
; add rsp, 20h
; pop rsp
mov [rsp + 18h], rbx 
mov [rsp + 20h], rbp
mov [rsp + 28h], rsi
ret
; jmp qword ptr[rsp]

hkUpdateModelPartsThunk ENDP
_TEXT ENDS
END	