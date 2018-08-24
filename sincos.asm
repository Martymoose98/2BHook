PUBLIC sincos

_TEXT SEGMENT

sincos PROC

movd ebx, xmm0
fld dword ptr [ebx]
fsincos
fstp dword ptr[ecx]
fstp dword ptr[edx]

sincos ENDP

_TEXT ENDS
END