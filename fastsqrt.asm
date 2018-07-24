PUBLIC fastsqrtf
PUBLIC fastsqrt

_TEXT SEGMENT

fastsqrtf PROC

FLD DWORD PTR [rsp + 8h]
FSQRT
RET 4

fastsqrtf ENDP

fastsqrt PROC

FLD QWORD PTR [rsp + 8h]
FSQRT
RET 8

fastsqrt ENDP

_TEXT ENDS
END