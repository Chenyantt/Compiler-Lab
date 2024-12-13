.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
_read:
li $v0, 4
la $a0, _prompt
syscall
li $v0, 5
syscall
jr $ra
_write:
li $v0, 1
syscall
li $v0, 4
la $a0, _ret
syscall
move $v0, $0
jr $ra
_squaRever:
addi $sp,$sp,-4
sw $fp,0($sp)
move $fp,$sp
addi $sp,$sp,-308
li $t2,0
sw $t2, -4($fp)
lw $t2,-4($fp)
sw $t2, -8($fp)
main:
addi $sp,$sp,-4
sw $fp,0($sp)
move $fp,$sp
addi $sp,$sp,-52
li $t2,100
sw $t2, -4($fp)
lw $t2,-4($fp)
sw $t2, -8($fp)
label12:
lw $t2,-8($fp)
sw $t2, -12($fp)
li $t2,150
sw $t2, -16($fp)
lw $t1,-12($fp)
lw $t2,-16($fp)
blt $t1,$t2,label13
j label14
label13:
lw $t2,-8($fp)
sw $t2, -20($fp)
lw $t1,-20($fp)
addi $sp,$sp,-4
sw $t1, 0($sp)
addi $sp,$sp,-4
sw $ra,0($sp)
jal _squaRever
lw $ra,0($sp)
addi $sp,$sp,4
sw $v0, -24($fp)
li $t2,1
sw $t2, -28($fp)
lw $t1,-24($fp)
lw $t2,-28($fp)
beq $t1,$t2,label15
j label16
label15:
lw $t2,-8($fp)
sw $t2, -32($fp)
lw $a0, -32($fp)
addi $sp,$sp,-4
sw $ra,0($sp)
jal _write
lw $ra,0($sp)
addi $sp,$sp,4
li $t2,0
sw $t2, -36($fp)
label16:
lw $t2,-8($fp)
sw $t2, -40($fp)
li $t2,1
sw $t2, -44($fp)
lw $t2,-40($fp)
lw $t3,-44($fp)
add $t1,$t2,$t3
sw $t1, -48($fp)
lw $t2,-48($fp)
sw $t2, -8($fp)
j label12
label14:
li $t2,0
sw $t2, -52($fp)
lw $v0,-52($fp)
move $sp,$fp
lw $fp,0($sp)
addi $sp,$sp,4
jr $ra
