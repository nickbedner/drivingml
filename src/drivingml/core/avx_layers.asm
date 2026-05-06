.intel_syntax noprefix

.text

.globl linear_layer_avx2
.globl relu_activation_avx2

# linear_layer_avx2
# rcx = weights
# rdx = bias
# r8d = input_size
# r9d = output_size
# [rsp + 40] = input
# [rsp + 48] = output

linear_layer_avx2:
mov r10, qword ptr [rsp + 40]
mov r11, qword ptr [rsp + 48]

movsxd r8, r8d

test r9d, r9d
jle done_linear

outer_loop:
vxorps ymm0, ymm0, ymm0

xor eax, eax
mov dword ptr [rsp + 8], r8d
and dword ptr [rsp + 8], -8

inner_vec_loop:
cmp eax, dword ptr [rsp + 8]
jge horizontal_sum

vmovups ymm1, ymmword ptr [rcx + rax * 4]
vmovups ymm2, ymmword ptr [r10 + rax * 4]

vmulps ymm1, ymm1, ymm2
vaddps ymm0, ymm0, ymm1

add eax, 8
jmp inner_vec_loop

horizontal_sum:
vextractf128 xmm1, ymm0, 1
vaddps xmm0, xmm0, xmm1
vhaddps xmm0, xmm0, xmm0
vhaddps xmm0, xmm0, xmm0

vaddss xmm0, xmm0, dword ptr [rdx]

tail_loop:
cmp eax, r8d
jge store_output

vmovss xmm1, dword ptr [rcx + rax * 4]
vmulss xmm1, xmm1, dword ptr [r10 + rax * 4]
vaddss xmm0, xmm0, xmm1

inc eax
jmp tail_loop

store_output:
vmovss dword ptr [r11], xmm0

lea rcx, [rcx + r8 * 4]
add rdx, 4
add r11, 4

dec r9d
jnz outer_loop

done_linear:
vzeroupper
ret


# relu_activation_avx2
# rcx = values
# edx = count

relu_activation_avx2:
test edx, edx
jle done_relu

vxorps ymm1, ymm1, ymm1

xor eax, eax
mov r8d, edx
and r8d, -8

relu_vec_loop:
cmp eax, r8d
jge relu_tail_loop

vmovups ymm0, ymmword ptr [rcx + rax * 4]
vmaxps ymm0, ymm0, ymm1
vmovups ymmword ptr [rcx + rax * 4], ymm0

add eax, 8
jmp relu_vec_loop

relu_tail_loop:
cmp eax, edx
jge done_relu

vmovss xmm0, dword ptr [rcx + rax * 4]
vmaxss xmm0, xmm0, xmm1
vmovss dword ptr [rcx + rax * 4], xmm0

inc eax
jmp relu_tail_loop

done_relu:
vzeroupper
ret
