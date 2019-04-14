


		addiu	$a0,$0, 4198400
Label:		
		addiu	$a0, $a0, 4
		addiu	$a1, $a1, 1 
		sw	$a1, 0($a0)
		addiu	$a2, $0, 5
		beq	$a1, $a2, End
		jal	Label 
			
		addi	$0,$0,0 #unsupported instruction, terminate


End: 	
		lw	$t0, 0($a0)
		subu	$a1, $a1, 1
		addiu	$a0, $a0, -4
		bne	$a1, $0, End
		jr	$ra					
		
