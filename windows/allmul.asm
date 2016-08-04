.686
.MMX
.XMM
.MODEL FLAT, C

.DATA

conv_tab      dq 0.0, 4294967296.0
MaxUInt64     dq 043f0000000000000h
MaxInt64      dq 043e0000000000000h
MaxFP32       dq 047effffff0000000h
MinFP32       dq 0380ffffff0000000h
MinInt64      dq 0c3e0000000000000h
MinSubInexact dq 04560000000000000h

MantissaMask dq 0000fffffffffffffh
IntegerBit   dq 00010000000000000h
i1075        dd 000000433h

.CODE

public _allmul
_allmul PROC NEAR USES ESI, multiplicand:QWORD, multiplier:QWORD

 MA EQU DWORD PTR multiplier [4]
 MB EQU DWORD PTR multiplier
 MC EQU DWORD PTR multiplicand [4]
 MD EQU DWORD PTR multiplicand

 mov eax, MA
 mov ecx, MC
 or  ecx, eax    ; both zero?
 mov ecx, MD
 .if zero?      ; yes, use shortcut.
   mov eax, MB
   mul ecx      ; EDX:EAX = DB[0:63].
 .else
   mov eax, MA
   mul ecx      ; EDX:EAX = DA[0:63].
   mov esi, eax ; ESI = DA[0:31].

   mov eax, MB
   mul MC       ; EDX:EAX = CB[0:63]
   add esi, eax ; ESI = DA[0:31] + CB[0:31]


   mov eax, MB
   mul ecx      ; EDX:EAX = BD[0:63]
   add edx, esi ; EDX = DA[0:31] + CB[0:31] + DB[31:63]
                ; EAX = DB[0:31]
 .endif


 ret 16 ; callee clears the stack.
_allmul ENDP

public _aulldiv
_aulldiv PROC

        push    ebx
        push    esi

;; Set up the local stack and save the index registers.  When this is done
;; the stack frame will look as follows (assuming that the expression a/b will
;; generate a call to uldiv(a, b)):
;;
;;               -----------------
;;               |               |
;;               |---------------|
;;               |               |
;;               |--divisor (b)--|
;;               |               |
;;               |---------------|
;;               |               |
;;               |--dividend (a)-|
;;               |               |
;;               |---------------|
;;               | return addr** |
;;               |---------------|
;;               |      EBX      |
;;               |---------------|
;;       ESP---->|      ESI      |
;;               -----------------
;;

DVNDLO EQU  DWORD PTR [esp + 12]       ; stack address of dividend (a)
DVNDHI EQU  DWORD PTR [esp + 16]       ; stack address of dividend (a)
DVSRLO EQU  DWORD PTR [esp + 20]      ; stack address of divisor (b)
DVSRHI EQU  DWORD PTR [esp + 24]      ; stack address of divisor (b)

;;
;; Now do the divide.  First look to see if the divisor is less than 4194304K.
;; If so, then we can use a simple algorithm with word divides, otherwise
;; things get a little more complex.
;;

        mov     eax,DVSRHI ; check to see if divisor < 4194304K
        or      eax,eax
        jnz     short L1        ; nope, gotta do this the hard way
        mov     ecx,DVSRLO ; load divisor
        mov     eax,DVNDHI ; load high word of dividend
        xor     edx,edx
        div     ecx             ; get high order bits of quotient
        mov     ebx,eax         ; save high bits of quotient
        mov     eax,DVNDLO ; edx:eax <- remainder:lo word of dividend
        div     ecx             ; get low order bits of quotient
        mov     edx,ebx         ; edx:eax <- quotient hi:quotient lo
        jmp     short L2        ; restore stack and return

;;
;; Here we do it the hard way.  Remember, eax contains DVSRHI
;;

L1:
        mov     ecx,eax         ; ecx:ebx <- divisor
        mov     ebx,DVSRLO
        mov     edx,DVNDHI ; edx:eax <- dividend
        mov     eax,DVNDLO
L3:
        shr     ecx,1           ; shift divisor right one bit; hi bit <- 0
        rcr     ebx,1
        shr     edx,1           ; shift dividend right one bit; hi bit <- 0
        rcr     eax,1
        or      ecx,ecx
        jnz     short L3        ; loop until divisor < 4194304K
        div     ebx             ; now divide, ignore remainder
        mov     esi,eax         ; save quotient

;;
;; We may be off by one, so to check, we will multiply the quotient
;; by the divisor and check the result against the orignal dividend
;; Note that we must also check for overflow, which can occur if the
;; dividend is close to 2**64 and the quotient is off by 1.
;;

        mul     dword ptr DVSRHI ; QUOT * DVSRHI
        mov     ecx,eax
        mov     eax,DVSRLO
        mul     esi             ; QUOT * DVSRLO
        add     edx,ecx         ; EDX:EAX = QUOT * DVSR
        jc      short L4        ; carry means Quotient is off by 1

;;
;; do long compare here between original dividend and the result of the
;; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
;; subtract one (1) from the quotient.
;;

        cmp     edx,DVNDHI ; compare hi words of result and original
        ja      short L4        ; if result > original, do subtract
        jb      short L5        ; if result < original, we are ok
        cmp     eax,DVNDLO ; hi words are equal, compare lo words
        jbe     short L5        ; if less or equal we are ok, else subtract
L4:
        dec     esi             ; subtract 1 from quotient
L5:
        xor     edx,edx         ; edx:eax <- quotient
        mov     eax,esi

;;
;; Just the cleanup left to do.  edx:eax contains the quotient.
;; Restore the saved registers and return.
;;

L2:

        pop     esi
        pop     ebx

        ret     16

_aulldiv ENDP



public _ltod3
_ltod3 PROC

xorps       xmm1, xmm1  
cvtsi2sd    xmm1, edx  
xorps       xmm0, xmm0  
cvtsi2sd    xmm0, ecx  
shr         ecx, 1Fh  
mulsd       xmm1, mmword ptr [conv_tab]   ; f64(2^32)
addsd       xmm0, mmword ptr [ecx*8+conv_tab]  
addsd       xmm0, xmm1  
ret  

_ltod3 ENDP

_dtoul3 PROC

mov    ecx, 4
movsd  xmm5,mmword ptr MaxUInt64 ; 0x43f0000000000000   MaxUInt64
jmp L1
mov    ecx, 5
movsd  xmm5,mmword ptr MaxInt64 ; 0x43e0000000000000   MaxInt64
L1:
movdqa      xmm1,xmm0  
psrldq      xmm1,4  
movd        eax,xmm1  
and         eax,7FFFFFFFh  
cmp         eax,7FF00000h  
jae         _ftol3_arg_error
cmp         ecx,1  
je          _ftol3_work
movdqa      xmm1,xmm0  
psllq       xmm1,1  
psrlq       xmm1,1

movsd       xmm2,mmword ptr MaxFP32  ; 0x47effffff0000000   MaxFP32
comisd      xmm1,xmm2
jbe         _dtol3_underflow
cmp         ecx,4  
je          _dtoul3_overflow
mov         edx,1  
call        _ftol3_except
mov         edx,10h  
call        _ftol3_except
jmp         _dtol3_underflow

_dtoul3_overflow:
mov         edx,10h  
call        _ftol3_except
mov         edx,1
call        _ftol3_except

_dtol3_underflow:
movsd       xmm2,mmword ptr MinFP32  ; 0x380ffffff0000000   MinFP32
comisd      xmm1,xmm2  
jae         _dtol3_inexact
xorpd       xmm2,xmm2  
comisd      xmm1,xmm2  
je          _ftol3_common
mov         edx,2  
call        _ftol3_except
mov         edx,10h  
call        _ftol3_except
jmp         _ftol3_common

_dtol3_inexact:
movdqa      xmm3,xmm0  
cmp         ecx,4  
jne         _dtol3_inexact_1
movsd       xmm2,mmword ptr MaxInt64  ; 0x43e0000000000000   MaxInt64
comisd      xmm3,xmm2  
jb          _dtol3_inexact_1
subpd       xmm3,xmm2  

_dtol3_inexact_1:
movdqa      xmm4,xmm3  
psllq       xmm4,23h  
psrlq       xmm4,23h  
movd        eax,xmm4  
or          eax,eax  
je          _dtol3_exact
mov         edx,10h
call        _ftol3_except

_dtol3_exact:
comisd      xmm0,xmm5  
jae         _ftol3_arg_error

movsd       xmm2,mmword ptr MinInt64  ; 0xc3e0000000000000   MinInt64
comisd      xmm0,xmm2  
jb          _ftol3_arg_error
jmp         _ftol3_common

_ftol3_common:
xorpd       xmm6,xmm6
comisd      xmm1,xmm6  
je          _ftol3_exact
movsd       xmm3,mmword ptr MantissaMask  ; 0x000fffffffffffff    MantissaMask
movsd       xmm4,mmword ptr IntegerBit    ; 0x0010000000000000    IntegerBit
movd        xmm5,dword ptr i1075          ; 0x00000433          i1075
movdqa      xmm6,xmm0  
cmpeqsd     xmm6,xmm1  
movdqa      xmm2,xmm1  
pand        xmm2,xmm3  
por         xmm2,xmm4  
psrlq       xmm1,34h  
movdqa      xmm4,xmm1  
psubq       xmm4,xmm5  
movdqa      xmm3,xmm2  
psllq       xmm3,xmm4  
movdqa      xmm4,xmm5  
psubq       xmm4,xmm1  
movdqa      xmm7,xmm2  
psrlq       xmm7,xmm4  
pcmpgtd     xmm1,xmm5  
punpckldq   xmm1,xmm1  
pand        xmm3,xmm1  
pandn       xmm1,xmm7  
por         xmm1,xmm3  
pxor        xmm5,xmm5  
psubq       xmm5,xmm1  
pand        xmm1,xmm6  
pandn       xmm6,xmm5  
por         xmm6,xmm1  
movd        eax,xmm4  
cmp         eax,0  
jle         _ftol3_exact
psllq       xmm7,xmm4  
pxor        xmm2,xmm7  
movd        eax,xmm2  
psrldq      xmm2,4  
movd        edx,xmm2  
or          eax,edx  
je          _ftol3_exact
mov         edx,10h  
call        _ftol3_except

_ftol3_exact:
movd        eax,xmm6  
psrldq      xmm6,4
movd        edx,xmm6
ret

_ftol3_arg_error:
mov         edx,8  
call        _ftol3_except

_ftol3_except:
ret

_ftol3_work:
cmp         ecx,2  
jne         _ftol3_non_ftoul3
movsd       xmm2,mmword ptr MinSubInexact  
comisd      xmm0,xmm2  
jbe         _ftol3_non_ftoul3
mov         edx,10h  
call        _ftol3_except

_ftol3_non_ftoul3:
comisd      xmm0,xmm5  
jae         _ftol3_arg_error
movsd       xmm6,mmword ptr MinInt64
comisd      xmm0,xmm6  
jb          _ftol3_arg_error
movdqa      xmm1,xmm0  
psllq       xmm1,1  
psrlq       xmm1,1  
cmp         ecx,1  
je          _ftol3_common 
movsd       xmm2,mmword ptr MinFP32
comisd      xmm1,xmm2  
ja          _ftol3_common  
xorpd       xmm2,xmm2  
comisd      xmm1,xmm2  
je          _ftol3_common  
push        eax  
wait  
fnstcw      word ptr [esp]  
mov         ax,word ptr [esp]  
test        ax,10h  
pop         eax  
jne         _ftol3_common
mov         edx,2  
call        _ftol3_except
xor         eax,eax  
mov         edx,80000000h  
ret  

_dtoul3 ENDP

END