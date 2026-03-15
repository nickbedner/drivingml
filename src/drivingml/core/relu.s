.intel_syntax noprefix
.text
.globl relu
.p2align 4

relu:
    # int3
    # rcx = float* x
    # edx = int n

    test edx, edx
    jle .done

    vxorps ymm0, ymm0, ymm0
    mov r8d, edx
    shr r8d, 3            # r8d = n / 8
    jz .scalar

.vec_loop:
    vmovups ymm1, [rcx]
    vmaxps  ymm1, ymm1, ymm0
    vmovups [rcx], ymm1
    add rcx, 32
    dec r8d
    jnz .vec_loop

.scalar:
    and edx, 7
    jz .done

.scalar_loop:
    vmovss xmm1, dword ptr [rcx]
    vmaxss xmm1, xmm1, xmm0
    vmovss dword ptr [rcx], xmm1
    add rcx, 4
    dec edx
    jnz .scalar_loop

.done:
    vzeroupper
    ret