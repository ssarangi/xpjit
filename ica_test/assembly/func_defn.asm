bits 64
section .data
section .data

section .text
global main_entry

firstmethod_entry:
push rdi
sub rsp, 16
mov rdi, rsp
mov ecx, 4
mov eax, -858993460
rep stosd
add rsp, 16
pop rdi
ret 0
secondmethod_entry:
push rdi
sub rsp, 16
mov rdi, rsp
mov ecx, 4
mov eax, -858993460
rep stosd
add rsp, 16
pop rdi
ret 0
