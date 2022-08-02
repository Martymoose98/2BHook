PUBLIC hkCreateEntityThunk

EXTERN hkCreateEntity: PROC

_TEXT SEGMENT

hkCreateEntityThunk PROC

; i've been debugging this shit for
; like at least 6 hours, someone kill me
; this assembly is for the old denuvo build

; mov r8d, ebp
; push rbp
; mov rbp, rsp
; and rsp, 0FFFFFFFFFFFFFFF0h
; sub rsp, 30h
; mov rdx, rsi
; mov [rsp+20h], rax
; call hkCreateEntity
; mov rsp, rbp
; pop rbp
; ret

; this assembly is for the new build

mov rcx, qword ptr [rcx+8h]   ; load 1st param into rcx
mov qword ptr [rdi+58h], rcx  ; store rcx in entityinfo struct 
mov r9d, dword ptr [rsp+0D8h] ; load uGroupId into r9d (+018h cause of the hook asm)
mov r8d, dword ptr [rsp+0C8h] ; load uObjectId into r8d (+018h cause of the hook asm)
mov rdx, rdi                  ; load pEntityInfo with entityinfo (rdi)
push rbp
mov rbp, rsp
and rsp, 0FFFFFFFFFFFFFFF0h  ; hkCreateEntity needs the stack to be 16 byte aligned
sub rsp, 20h                 ; allocate shadow space not nessesary anymore
call hkCreateEntity
add rsp, 20h                 ; deallocate shadow space not nessesary anymore
mov rsp, rbp
pop rbp
ret 

hkCreateEntityThunk ENDP
_TEXT ENDS
END