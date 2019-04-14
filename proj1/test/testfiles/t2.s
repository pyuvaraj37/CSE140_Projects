# Short test case for your project.
#
# Note that this is by no means a comprehensive test!
#

		.text
		addiu	$a0, $0, 5
		addiu	$a1, $0, 1
Label2: 	jal 	Label1
		bne	$a0, $0, Label2 
		j	Label3
		
Label1: 	subu	$a0, $a0, $a1
		jr	$ra
		
Label3: 	add	$a0, $a1, $a2