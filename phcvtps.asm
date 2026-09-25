PUBLIC ConvertFP16ToFP32
PUBLIC ConvertFP16ToFP32Extra

_TEXT SEGMENT

; movzx       eax,cx  
; movd        xmm0,eax  
; punpcklwd   xmm0,xmm0  
; pshufd      xmm0,xmm0,0  
; or 
; vpbroadcastw
; apparently movsx is not good movzx
; CX = value
ConvertFP16ToFP32Extra PROC

push rbp
mov rbp, rsp
movzx eax, cx
vmovd xmm1, eax
vpbroadcastw xmm0, xmm1
vcvtph2ps xmm0, xmm0
mov rsp, rbp
pop rbp
ret

ConvertFP16ToFP32Extra ENDP

ConvertFP16ToFP32 PROC
    vmovd     xmm0, ecx        ; half in the low word, lanes 1..3 zero
    vcvtph2ps xmm0, xmm0       ; lane 0 is the answer, already in the return reg
    ret
ConvertFP16ToFP32 ENDP

_TEXT ENDS
END