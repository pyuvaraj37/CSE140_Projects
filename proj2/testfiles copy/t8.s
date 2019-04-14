	addiu	$a0, $0, 0x00400000
	addiu	$a1, $0, -3
	sw	$a1, 4($a0)
	addiu	$a1, $0, 4 
	sw	$a1, 8($a0)
	lw	$t0, 4($a0)
	lw	$t1, 8($a0)
	slt	$a0, $t0, $t1
	addiu	$a1, $0, 1
	beq	$a1, $a0, Label
Back: 
	add	$a0, $a1, $a2
	
	
	
Label:	jal	shift
	j 	Back 



shift:	addiu	$t3, $0, 0x40000000
	andi	$t3, $t3,0xf0000000
	srl	$t3, $t3, 28
	jr	$ra 
	