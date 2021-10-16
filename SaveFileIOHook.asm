PUBLIC hkSaveFileIOThunk

EXTERN hkSaveFileIO: PROC

_TEXT SEGMENT

hkSaveFileIOThunk PROC

push rcx
mov rcx, rbx
call hkSaveFileIO
pop rcx
ret

hkSaveFileIOThunk ENDP
_TEXT ENDS
END