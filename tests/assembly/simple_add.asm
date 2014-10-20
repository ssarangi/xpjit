.data

.text
.globl main

main:
 move $t2 $sp
 addiu $sp $sp -16
 li $a0 10
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 20
 lw $t1 4($sp)
 add $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 0($t2)
 li $a0 20
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 5
 lw $t1 4($sp)
 mul $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -4($t2)
 li $a0 2
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 -4($t2)
 lw $t1 4($sp)
 mul $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -8($t2)
 lw $a0 -8($t2)
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 0($t2)
 lw $t1 4($sp)
 add $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -12($t2)
 li $v0 1
syscall
 li $v0 17
syscall
