.DATA
EXTERN g_FuncPointer:QWORD;

.CODE

__asm_hv_dispatcher_handler PROC

mov r9,rdi
mov rax,112233344556677h
jmp rax

__asm_hv_dispatcher_handler ENDP

__asm_set_hv_handler PROC

mov g_FuncPointer,rcx
ret

__asm_set_hv_handler ENDP



__asm_dispatch_iCall PROC FRAME

sub rsp, 0128h
.allocstack 0128h
.endprolog
mov qword ptr[rsp + 0120h],00h
mov [rsp + 0118h],r15
mov [rsp + 0110h],r14
mov [rsp + 0108h],r13
mov [rsp + 0100h],r12
mov [rsp + 00F8h],r11
mov [rsp + 00F0h],r10
mov [rsp + 00E8h],r9
mov [rsp + 00E0h],r8
mov [rsp + 00D8h],rdi
mov [rsp + 00D0h],rsi
mov [rsp + 00C8h],rbp
mov [rsp + 00C0h],rsp
mov [rsp + 00B8h],rbx
mov [rsp + 00B0h],rdx
mov [rsp + 00A8h],rcx
mov [rsp + 00A0h],rax

lea rsi,[rsp + 0150h]
lea rdi,[rsp + 028h]
mov rcx,0Eh
rep movsq
mov [rsp + 020h],r9
mov r9,r8
mov r8,rdx
mov rdx,[rsp + 00A8h] ; get rcx value
lea rcx,[rsp+0A0h]

call rax

mov r15,[rsp + 0118h]
mov r14,[rsp + 0110h]
mov r13,[rsp + 0108h]
mov r12,[rsp + 0100h]
mov r11,[rsp + 00F8h]
mov r10,[rsp + 00F0h]
mov r9,[rsp + 00E8h]
mov r8,[rsp + 00E0h]
mov rdi,[rsp + 00D8h]
mov rsi,[rsp + 00D0h]
mov rbp,[rsp + 00C8h]
mov rbx,[rsp + 00B8h]
mov rdx,[rsp + 00B0h]
mov rcx,[rsp + 00A8h]
mov rax,[rsp + 00A0h]

cmp qword ptr[rsp + 0120h],00h
je $_no_fix_rip
lea rsp, [rsp + 0120h]
ret

$_no_fix_rip:
add rsp, 0128h
ret

__asm_dispatch_iCall ENDP

End