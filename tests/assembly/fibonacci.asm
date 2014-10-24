.data

.text
.globl main_entry

# Entry Code: fib
fib_entry:
 move $fp $sp
# Generated Code for RA
 sw $ra 0($sp)
 addiu $sp $sp -4
# Generated Code for Temporaries
 addiu $sp $sp -28
fib_condblock:
 lw $a0 4($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 0
 lw $t1 4($sp)
 beq $a0 $t1 fib_codeblock
 j fib_postif
fib_codeblock:
 lw $ra 32($sp)
 addiu $sp $sp 40
 lw $fp 0($sp)
 jr $ra
fib_postif:
fib_condblock1:
 lw $a0 4($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 1
 lw $t1 4($sp)
 beq $a0 $t1 fib_codeblock2
 j fib_postif3
fib_codeblock2:
 lw $ra 32($sp)
 addiu $sp $sp 40
 lw $fp 0($sp)
 jr $ra
fib_postif3:
#   %2 = sub i32 %x, 1
 lw $a0 4($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 1
 lw $t1 4($sp)
 sub $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -12($fp)
#   %3 = call i32 @fib(i32 %2)
 sw $fp 0($sp)
 addiu $sp $sp -4
 lw $a0 -12($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 jal fib_entry
 sw $a0 -16($fp)
#   %4 = sub i32 %x, 2
 lw $a0 4($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 li $a0 2
 lw $t1 4($sp)
 sub $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -20($fp)
#   %5 = call i32 @fib(i32 %4)
 sw $fp 0($sp)
 addiu $sp $sp -4
 lw $a0 -20($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 jal fib_entry
 sw $a0 -24($fp)
#   %6 = add i32 %3, %5
 lw $a0 -16($fp)
 sw $a0 0($sp)
 addiu $sp $sp -4
 lw $a0 -24($fp)
 lw $t1 4($sp)
 add $a0 $a0 $t1
 addiu $sp $sp 4
 sw $a0 -28($fp)
 lw $ra 32($sp)
 addiu $sp $sp 40
 lw $fp 0($sp)
 jr $ra
# Entry Code: main
main_entry:
 move $fp $sp
# Generated Code for RA
 sw $ra 0($sp)
 addiu $sp $sp -4
# Generated Code for Temporaries
 addiu $sp $sp -4
#   %0 = call i32 @fib(i32 6)
 sw $fp 0($sp)
 addiu $sp $sp -4
 li $a0 6
 sw $a0 0($sp)
 addiu $sp $sp -4
 jal fib_entry
 sw $a0 -8($fp)
 li $v0 1
syscall
 li $v0 17
syscall
