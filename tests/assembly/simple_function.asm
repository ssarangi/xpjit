.data

.text
.globl main_entry

# Entry Code: main
main_entry:
 move $fp $sp
# Generated Code for RA
 sw $ra 0($sp)
 addiu $sp $sp -4
# Generated Code for Temporaries
 addiu $sp $sp -12
#   %0 = call i32 @multiply(i32 5, i32 6)
 sw $fp 0($sp)
 addiu $sp $sp -4
 li $a0 5
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 6
 sw $a0 0($sp)
 addiu $sp $sp -4
 jal multiply_entry
 sw $a0 -8($fp)
#   %1 = call i32 @multiply(i32 5, i32 6)
 sw $fp 0($sp)
 addiu $sp $sp -4
 li $a0 5
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 6
 sw $a0 0($sp)
 addiu $sp $sp -4
 jal multiply_entry
 sw $a0 -12($fp)
#   %2 = add i32 %0, %1
 lw $a0 -8($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 -12($fp)
 lw $t1 4($sp)
 add $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -16($fp)
 li $v0 1
syscall
 li $v0 17
syscall
# Entry Code: multiply
multiply_entry:
 move $fp $sp
# Generated Code for RA
 sw $ra 0($sp)
 addiu $sp $sp -4
# Generated Code for Temporaries
 addiu $sp $sp -8
#   %0 = mul i32 %x, %y
 lw $a0 8($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 4($fp)
 lw $t1 4($sp)
 mul $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -16($fp)
#   %1 = mul i32 %0, 10
 lw $a0 -16($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 10
 lw $t1 4($sp)
 mul $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -20($fp)
 lw $ra 12($sp)
 addiu $sp $sp 24
 lw $fp 0($sp)
 jr $ra
