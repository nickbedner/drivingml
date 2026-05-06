.intel_syntax noprefix

.text

.globl linear_layer_avx2
.globl relu_activation_avx2

# linear_layer_avx2
# rcx = weights
# rdx = bias
# r8d = input_size   ; assumed exactly 8
# r9d = output_size
# [rsp + 40] = input
# [rsp + 48] = output

linear_layer_avx2:
mov r10, qword ptr [rsp + 40]
mov r11, qword ptr [rsp + 48]

test r9d, r9d
jle done_linear

outer_loop:
vmovups ymm0, ymmword ptr [rcx]
vmulps ymm0, ymm0, ymmword ptr [r10]

# horizontal sum of 8 floats in ymm0
vextractf128 xmm1, ymm0, 1
vaddps xmm0, xmm0, xmm1
vhaddps xmm0, xmm0, xmm0
vhaddps xmm0, xmm0, xmm0

# add bias
vaddss xmm0, xmm0, dword ptr [rdx]

# store output
vmovss dword ptr [r11], xmm0

# next output neuron
add rcx, 32        # 8 floats * 4 bytes
add rdx, 4
add r11, 4

dec r9d
jnz outer_loop

done_linear:
vzeroupper
ret


# relu_activation_avx2
# rcx = values
# edx = count        ; assumed exactly 8

relu_activation_avx2:
vxorps ymm1, ymm1, ymm1

vmovups ymm0, ymmword ptr [rcx]
vmaxps ymm0, ymm0, ymm1
vmovups ymmword ptr [rcx], ymm0

done_relu:
vzeroupper
ret
