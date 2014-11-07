.data

.text
.globl main_entry

 j main_entry
# Entry Code: main
main_entry:
 move $fp $sp
# Generated Code for RA
 sw $ra 0($sp)
 addiu $sp $sp -4
# Generated Code for Temporaries
 addiu $sp $sp -16
#   %0 = add i32 10, 20
 li $a0 10
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 20
 lw $t1 4($sp)
 add $a0 $t1 $a0
 addiu $sp $sp 4
 sw $a0 -8($fp)
#   %1 = mul i32 20, 5
 li $a0 20
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 5
 lw $t1 4($sp)
 mul $a0 $t1 $a0
 addiu $sp $sp 4
 sw $a0 -12($fp)
#   %2 = mul i32 2, %1
 li $a0 2
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 -12($fp)
 lw $t1 4($sp)
 mul $a0 $t1 $a0
 addiu $sp $sp 4
 sw $a0 -16($fp)
#   %3 = add i32 %2, %0
 lw $a0 -16($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 -8($fp)
 lw $t1 4($sp)
 add $a0 $t1 $a0
 addiu $sp $sp 4
 sw $a0 -20($fp)
 li $v0 1
syscall
 li $v0 17
syscall
