PUBLIC hkSaveFileIOThunk

EXTERN hkSaveFileIO: PROC

_TEXT SEGMENT

hkSaveFileIOThunk PROC

push rcx
mov rcx, qword ptr [rsp + 18h] ; mov rbx, qword ptr[rbp + 0F0h] then we pushed it on the stack
call hkSaveFileIO
pop rcx
ret

hkSaveFileIOThunk ENDP
_TEXT ENDS
END