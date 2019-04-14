# Short test case for your project.
#
# Note that this is by no means a comprehensive test!
#

		.text
		addiu	$a0,$0,1
		addiu   $a1, $0, 5
		
Loop:
		beq	$a0,$a1,Done
		addiu	$a0,$a0,1
		j Loop
Done:		
		add $a0, $a1, $a3
