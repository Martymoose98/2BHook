PUBLIC ConvertFP16ToFP32

_TEXT SEGMENT

; movzx       eax,cx  
; movd        xmm0,eax  
; punpcklwd   xmm0,xmm0  
; pshufd      xmm0,xmm0,0  
; or 
; vpbroadcastw
; apparently movsx is not good movzx
; CX = value
ConvertFP16ToFP32 PROC

push rbp
mov rbp, rsp
movzx eax, cx
movd xmm2, eax
vpbroadcastw xmm1, xmm2
vcvtph2ps xmm0, xmm1
mov rsp, rbp
pop rbp
ret

ConvertFP16ToFP32 ENDP

_TEXT ENDS
END