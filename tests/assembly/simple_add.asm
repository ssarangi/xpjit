.data

.text
.globl main

main:
 li $a0 10
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 20
 lw $t1 4($sp)
 add $a0 $a0 $t1
 addiu $sp $sp 4
 li $v0 1
syscall
 li $v0 17
syscall
