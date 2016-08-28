.DATA

abs_mask   dq 07fffffffffffffff, 07fffffffffffffff

.CODE

public _tl_sincos_fixed3
_tl_sincos_fixed3 PROC


mov       ecx, eax
shl       rcx, 32
add       eax, ecx
add       eax, (32 << 16)
movq      xmm1, rax
pslld     xmm1, 9
psrad     xmm1, 9      ; clamp_ang(x), clamp_ang(x + cos_step)
cvtdq2pd  xmm1, xmm1


_tl_sincos_fixed3 ENDP
