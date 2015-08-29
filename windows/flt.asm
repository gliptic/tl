   .386
   .xmm
   .model flat
   
   
public  __fltused
__fltused        equ     9876h

   .code

; __declspec(naked) long __cdecl _ftol2(double x)
; {
; 	__asm
; 	{
; 		fstp        qword ptr [esp-8]
; 		cvttsd2si   eax, mmword ptr [esp-8]
; 		ret
; 	}
; }

public __ftol2_sse
__ftol2_sse proc
	fstp        qword ptr [esp-8]
	cvttsd2si   eax, mmword ptr [esp-8]
	ret
__ftol2_sse endp

public __aulldvrm
__aulldvrm proc

		push    esi

; Set up the local stack and save the index registers.  When this is done
; the stack frame will look as follows (assuming that the expression a/b will
; generate a call to aulldvrm(a, b)):
;
;               -----------------
;               |               |
;               |---------------|
;               |               |
;               |--divisor (b)--|
;               |               |
;               |---------------|
;               |               |
;               |--dividend (a)-|
;               |               |
;               |---------------|
;               | return addr** |
;               |---------------|
;       ESP---->|      ESI      |
;               -----------------
;


;
; Now do the divide.  First look to see if the divisor is less than 4194304K.
; If so, then we can use a simple algorithm with word divides, otherwise
; things get a little more complex.
;

		mov     eax,[esp + 12] ; check to see if divisor < 4194304K
		or      eax,eax
		jnz     short L1        ; nope, gotta do this the hard way
		mov     ecx,[esp + 16] ; load divisor
		mov     eax,[esp + 12] ; load high word of dividend
		xor     edx,edx
		div     ecx             ; get high order bits of quotient
		mov     ebx,eax         ; save high bits of quotient
		mov     eax,[esp + 8] ; edx:eax <- remainder:lo word of dividend
		div     ecx             ; get low order bits of quotient
		mov     esi,eax         ; ebx:esi <- quotient

;
; Now we need to do a multiply so that we can compute the remainder.
;
        mov     eax,ebx         ; set up high word of quotient
        mul     dword ptr [esp + 16] ; HIWORD(QUOT) * DVSR
        mov     ecx,eax         ; save the result in ecx
        mov     eax,esi         ; set up low word of quotient
        mul     dword ptr [esp + 16] ; LOWORD(QUOT) * DVSR
        add     edx,ecx         ; EDX:EAX = QUOT * DVSR
        jmp     short L2        ; complete remainder calculation

;
; Here we do it the hard way.  Remember, eax contains DVSRHI
;

L1:
        mov     ecx,eax         ; ecx:ebx <- divisor
        mov     ebx,[esp + 16]
        mov     edx,[esp + 12] ; edx:eax <- dividend
        mov     eax,[esp + 8]
L3:
        shr     ecx,1           ; shift divisor right one bit; hi bit <- 0
        rcr     ebx,1
        shr     edx,1           ; shift dividend right one bit; hi bit <- 0
        rcr     eax,1
        or      ecx,ecx
        jnz     short L3        ; loop until divisor < 4194304K
        div     ebx             ; now divide, ignore remainder
        mov     esi,eax         ; save quotient

;
; We may be off by one, so to check, we will multiply the quotient
; by the divisor and check the result against the orignal dividend
; Note that we must also check for overflow, which can occur if the
; dividend is close to 2**64 and the quotient is off by 1.
;

        mul     dword ptr [esp + 20] ; QUOT * HIWORD(DVSR)
        mov     ecx,eax
        mov     eax,[esp + 16]
        mul     esi             ; QUOT * LOWORD(DVSR)
        add     edx,ecx         ; EDX:EAX = QUOT * DVSR
        jc      short L4        ; carry means Quotient is off by 1

;
; do long compare here between original dividend and the result of the
; multiply in edx:eax.  If original is larger or equal, we are ok, otherwise
; subtract one (1) from the quotient.
;

        cmp     edx,[esp + 12] ; compare hi words of result and original
        ja      short L4        ; if result > original, do subtract
        jb      short L5        ; if result < original, we are ok
        cmp     eax,[esp + 8] ; hi words are equal, compare lo words
        jbe     short L5        ; if less or equal we are ok, else subtract
L4:
        dec     esi             ; subtract 1 from quotient
        sub     eax,[esp + 16] ; subtract divisor from result
        sbb     edx,[esp + 20]
L5:
        xor     ebx,ebx         ; ebx:esi <- quotient

L2:
;
; Calculate remainder by subtracting the result from the original dividend.
; Since the result is already in a register, we will do the subtract in the
; opposite direction and negate the result.
;

        sub     eax,[esp + 8] ; subtract dividend from result
        sbb     edx,[esp + 12]
        neg     edx             ; otherwise, negate the result
        neg     eax
        sbb     edx,0

;
; Now we need to get the quotient into edx:eax and the remainder into ebx:ecx.
;
        mov     ecx,edx
        mov     edx,ebx
        mov     ebx,ecx
        mov     ecx,eax
        mov     eax,esi
;
; Just the cleanup left to do.  edx:eax contains the quotient.
; Restore the saved registers and return.
;

        pop     esi

        ret     16
__aulldvrm endp

end