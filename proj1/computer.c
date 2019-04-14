#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "computer.h"
#undef mips			/* gcc already has a def for mips */

unsigned int endianSwap(unsigned int);

void PrintInfo (int changedReg, int changedMem);
unsigned int Fetch (int);
void Decode (unsigned int, DecodedInstr*, RegVals*);
int Execute (DecodedInstr*, RegVals*);
int Mem(DecodedInstr*, int, int *);
void RegWrite(DecodedInstr*, int, int *);
void UpdatePC(DecodedInstr*, int);
void PrintInstruction (DecodedInstr*);

/*Globally accessible Computer variable*/
Computer mips;
RegVals rVals;

/*
 *  Return an initialized computer with the stack pointer set to the
 *  address of the end of data memory, the remaining registers initialized
 *  to zero, and the instructions read from the given file.
 *  The other arguments govern how the program interacts with the user.
 */
void InitComputer (FILE* filein, int printingRegisters, int printingMemory,
  int debugging, int interactive) {
    int k;
    unsigned int instr;

    /* Initialize registers and memory */

    for (k=0; k<32; k++) {
        mips.registers[k] = 0;
    }
    
    /* stack pointer - Initialize to highest address of data segment */
    mips.registers[29] = 0x00400000 + (MAXNUMINSTRS+MAXNUMDATA)*4;

    for (k=0; k<MAXNUMINSTRS+MAXNUMDATA; k++) {
        mips.memory[k] = 0;
    }

    k = 0;
    while (fread(&instr, 4, 1, filein)) {
	/*swap to big endian, convert to host byte order. Ignore this.*/
        mips.memory[k] = ntohl(endianSwap(instr));
        k++;
        if (k>MAXNUMINSTRS) {
            fprintf (stderr, "Program too big.\n");
            exit (1);
        }
    }

    mips.printingRegisters = printingRegisters;
    mips.printingMemory = printingMemory;
    mips.interactive = interactive;
    mips.debugging = debugging;
}

unsigned int endianSwap(unsigned int i) {
    return (i>>24)|(i>>8&0x0000ff00)|(i<<8&0x00ff0000)|(i<<24);
}

/*
 *  Run the simulation.
 */
void Simulate () {
    char s[40];  /* used for handling interactive input */
    unsigned int instr;
    int changedReg=-1, changedMem=-1, val;
    DecodedInstr d;
    
    /* Initialize the PC to the start of the code section */
    mips.pc = 0x00400000;
    while (1) {
        if (mips.interactive) {
            printf ("> ");
            fgets (s,sizeof(s),stdin);
            if (s[0] == 'q') {
                return;
            }
        }

        /* Fetch instr at mips.pc, returning it in instr */
        instr = Fetch (mips.pc);

        printf ("Executing instruction at %8.8x: %8.8x\n", mips.pc, instr);

        /* 
	 * Decode instr, putting decoded instr in d
	 * Note that we reuse the d struct for each instruction.
	 */
        Decode (instr, &d, &rVals);

        /*Print decoded instruction*/
        PrintInstruction(&d);

        /* 
	 * Perform computation needed to execute d, returning computed value 
	 * in val 
	 */
        val = Execute(&d, &rVals);

	UpdatePC(&d,val);

        /* 
	 * Perform memory load or store. Place the
	 * address of any updated memory in *changedMem, 
	 * otherwise put -1 in *changedMem. 
	 * Return any memory value that is read, otherwise return -1.
         */
        val = Mem(&d, val, &changedMem);

        /* 
	 * Write back to register. If the instruction modified a register--
	 * (including jal, which modifies $ra) --
         * put the index of the modified register in *changedReg,
         * otherwise put -1 in *changedReg.
         */
        RegWrite(&d, val, &changedReg);

        PrintInfo (changedReg, changedMem);
    }
}

/*
 *  Print relevant information about the state of the computer.
 *  changedReg is the index of the register changed by the instruction
 *  being simulated, otherwise -1.
 *  changedMem is the address of the memory location changed by the
 *  simulated instruction, otherwise -1.
 *  Previously initialized flags indicate whether to print all the
 *  registers or just the one that changed, and whether to print
 *  all the nonzero memory or just the memory location that changed.
 */
void PrintInfo ( int changedReg, int changedMem) {
    int k, addr;
    printf ("New pc = %8.8x\n", mips.pc);
    if (!mips.printingRegisters && changedReg == -1) {
        printf ("No register was updated.\n");
    } else if (!mips.printingRegisters) {
        printf ("Updated r%2.2d to %8.8x\n",
        changedReg, mips.registers[changedReg]);
    } else {
        for (k=0; k<32; k++) {
            printf ("r%2.2d: %8.8x  ", k, mips.registers[k]);
            if ((k+1)%4 == 0) {
                printf ("\n");
            }
        }
    }
    if (!mips.printingMemory && changedMem == -1) {
        printf ("No memory location was updated.\n");
    } else if (!mips.printingMemory) {
        printf ("Updated memory at address %8.8x to %8.8x\n",
        changedMem, Fetch (changedMem));
    } else {
        printf ("Nonzero memory\n");
        printf ("ADDR	  CONTENTS\n");
        for (addr = 0x00400000+4*MAXNUMINSTRS;
             addr < 0x00400000+4*(MAXNUMINSTRS+MAXNUMDATA);
             addr = addr+4) {
            if (Fetch (addr) != 0) {
                printf ("%8.8x  %8.8x\n", addr, Fetch (addr));
            }
        }
    }
}

/*
 *  Return the contents of memory at the given address. Simulates
 *  instruction fetch. 
 */
unsigned int Fetch ( int addr) {
	
	//Might need to add unsupported instruction exit 

    return mips.memory[(addr-0x00400000)/4];
}

/* Decode instr, returning decoded instruction. */
void Decode ( unsigned int instr, DecodedInstr* d, RegVals* rVals) {
    /* Your code goes here */

	unsigned int mod = instr;

	//Shifting mode to the right 26 bits to get first 6 bits
	mod = mod >> 26;

	//Setting int op in DecodedInstr *d to the first 6 bits(Opcode)
	d->op = mod; 


	if (mod == 0) {
		//R Instruction 
		d->type = 0; 
		//Redefining mod as instruction 
		mod = instr; 

		//Extracts the funct bitsby mod AND 0000 0000 0000 0000 0000 0000 0011 1111
		mod = mod & 0x0000003f; 
		d->regs.r.funct = mod; 

		switch(mod) {

			case 33:
			case 35:
			case 0:
			case 2:
			case 36:
			case 37:
			case 42:
			case 8: {
				//Redefining mod as instruction 
				mod = instr; 

				//Extracts the rs bits by mod AND 0000 0011 1110 0000 0000 0000 0000 0000
				mod = mod & 0x03e00000;

				//Isolate the rs bits.
				mod = mod >> 21; 
				d->regs.r.rs = mod; 
				rVals->R_rs = mips.registers[mod]; 

				//Redefining mod as instruction 
				mod = instr; 

				//Extracts the rt bits by mod AND 0000 0000 0001 1111 0000 0000 0000 0000
				mod = mod & 0x001f0000;

				//Isolate the rt bits.
				mod = mod >> 16; 

				d->regs.r.rt = mod; 
				rVals->R_rt = mips.registers[mod];

				//Redefining mod as instruction 
				mod = instr; 

				//Extracts the rd bits by mod AND 0000 0000 0000 0000 1111 1000 0000 0000
				mod = mod & 0x0000f800;

				//Isolate the rd bits.
				mod = mod >> 11; 

				d->regs.r.rd = mod; 
				rVals->R_rd = mips.registers[mod];

				//Redefining mod as instruction 
				mod = instr; 

				//Extracts the shamt bits by mod AND 0000 0000 0000 0000 0000 0111 1100 0000
				mod = mod & 0x000007c0;

				//Isolate the rd bits.
				mod = mod >> 6; 

				d->regs.r.shamt = mod; 

				break; 
			}
			default: {
				//Ask daniel about 
				exit(0);
			}

		}


	} else if (mod == 2 || mod == 3) {
		//J Instruction 
		d->type = 2; 

		//Redefining mod as instruction 
		mod = instr; 

		//Extracts the Address bits by mod AND 0000 0011 1111 1111 1111 1111 1111 1111
		//Need to zero extend 
		mod = mod & 0x03fffff; 

		d->regs.j.target = mod; 


	} else {

		//I Instruction 
		switch(d->op) { 

			case 9:
			case 12:
			case 13:
			case 15: 
			case 4:
			case 5:
			case 35:
			case 43: 
			{	
				d->type = 1; 
				//Redefining mod as instruction 
				mod = instr; 

				//Extracts the rs bits by mod AND 0000 0011 1110 0000 0000 0000 0000 0000
				mod = mod & 0x03e00000;

				//Isolate the rs bits.
				mod = mod >> 21; 
				d->regs.i.rs = mod; 
				rVals->R_rs = mips.registers[mod]; 

				//Redefining mod as instruction 
				mod = instr; 

				//Extracts the rt bits by mod AND 0000 0000 0001 1111 0000 0000 0000 0000
				mod = mod & 0x001f0000;

				//Isolate the rt bits.
				mod = mod >> 16; 

				d->regs.i.rt = mod; 
				rVals->R_rt = mips.registers[mod];

				//Redefining mod as instruction 
				mod = instr; 

				if (d->op == 4 || d->op == 5 || d->op == 9) {

					//Sign Extentions 
					unsigned int smod = instr;
					smod = smod & 0x0000ffff;
					if (instr & 0x00008000) {

						smod = smod + 0xffff0000;

					} else {
						//Zero Extend?
						smod = smod + 0x00000000; 

					}

					d->regs.i.addr_or_immed = smod;

				} else {

					//Extracts the Immediate bits by mod AND 0000 0000 0000 0000 1111 1111 1111 1111
					mod = mod & 0x0000ffff;

					//Isolate the rt bits. 
					d->regs.i.addr_or_immed = mod;
				}
				break; 
			}	
			
			default: {
				exit(0);
			}
			//End Program nots a supported instruction
		}
	}

    
}

/*
 *  Print the disassembled version of the given instruction
 *  followed by a newline.
 */
void PrintInstruction ( DecodedInstr* d) {
    /* Your code goes here */

	switch(d->type) {
		//For R instuctions 
		case 0: {
			unsigned int rs = d->regs.r.rs;
			unsigned int rt = d->regs.r.rt;
			unsigned int rd = d->regs.r.rd;

			switch(d->regs.r.funct) {

				case 33: {
				//addu
					printf("addu\t $%d, $%d, $%d\n", rd, rs, rt);
					break;
				}
				case 35: {
				//subu
					printf("subu\t $%d, $%d, $%d\n", rd, rs, rt);
					break;
				}
				case 0: {
				//sll
					printf("sll\t $%d, $%d, %d\n", rd, rs, d->regs.r.shamt);
					break;
				}
				case 2: {
				//srl
					printf("srl\t $%d, $%d, %d\n", rd, rs, d->regs.r.shamt);
					break;
				}
				case 36: {
				//and
					printf("and\t $%d, $%d, %d\n", rd, rs, rt);
					break;
				}
				case 37: {
				//or
					printf("or\t $%d, $%d, $%d\n", rd, rs, rt);
				}
				case 42: {
				//slt	
					printf("slt\t $%d, $%d, $%d\n", rd, rs, rt);
					break;	
				}
				case 8: {
				//jr
					printf("jr\t $%d\n", rs);
					break;
				}

			}
			break;

		}

		//For I instructions 
		case 1: {

			unsigned int rs = d->regs.i.rs; 
			unsigned int rt = d->regs.i.rt; 
			unsigned int imm = d->regs.i.addr_or_immed; 

			switch(d->op) {

				case 9: {
				//addiu
					printf("addiu\t $%d, $%d, %d\n", rt, rs, imm);	
					break;
				}
				case 12: {
				//andi
					printf("andi\t $%d, $%d, %d\n", rt, rs, imm);	
					break;				
				}
				case 13: {
				//ori
					printf("ori\t $%d, $%d, 0x%x\n", rt, rs, imm);
					break;
				}
				case 15: {
				//lui
					printf("lui\t $%d, 0x%x\n", rt, imm);
					break;	
				}
				case 4: {
				//beq
					printf("beq\t $%d, $%d, 0x%08x\n", rt, rs, imm);
					break;
				}
				case 5: {
				//bne
					printf("bne\t $%d, $%d, 0x%08x\n", rt, rs, imm);
					break;		
				}
				case 35: {
				//lw
					printf("lw\t $%d, %d($%d)\n", rt, imm, rs);
					break;
				}	
				case 43: {
				//sw
					printf("sw\t $%d, %d($%d)\n", rt, imm, rs);
					break;	
				}
			}
			break; 	
		}

		//For J instuctions
		case 2: {

			unsigned int addr = d->regs.j.target; 
			if (d->op == 2) {
				printf("j\t 0x%x\n", addr);
			} else {
				printf("jal\t 0x%x\n", addr);
			}

		}
	}


}

/* Perform computation needed to execute d, returning computed value */
int Execute ( DecodedInstr* d, RegVals* rVals) {
    /* Your code goes here */

	//Function Code Decoding
	//R Instructions 
	 
	if(d->op == 0){
		//R Instructions 
		switch(d->regs.r.funct ) {
		    
			case 33: {
			//addu



				rVals->R_rd = rVals->R_rs + rVals->R_rt;
				return rVals->R_rd; 

			}
			case 35: {
			//subu
				rVals->R_rd = rVals->R_rs - rVals->R_rt;
				return rVals->R_rd; 
				
			}
			case 0: {
			//sll
				rVals->R_rd = rVals->R_rt << d->regs.r.shamt;
				return rVals->R_rd; 
				
			}
			case 2: {
			//srl
				rVals->R_rd = rVals->R_rt >> d->regs.r.shamt;
				return rVals->R_rd; 
				
			}
			case 36: {
			//and
				rVals->R_rd = rVals->R_rs & rVals->R_rt;
				return rVals->R_rd; 
				
			}
			case 37: {
			//or
				rVals->R_rd = rVals->R_rs | rVals->R_rt;
				return rVals->R_rd; 
				
			}
			case 42: {
			//slt
				if (rVals->R_rs < rVals->R_rt) {
					rVals->R_rd = 1;
				} else {
					rVals->R_rd = 0;
				}
				return rVals->R_rd; 
				
			}
			case 8: {
			//Doesnt use execute
				return d->regs.r.rs;

			}
		 }

	} else if (d->op == 2 || d->op == 3){
			//Doesnt use execute
			return d->regs.j.target; 

	} else {
		// I Instructions s 

		switch(d->op) {

			case 9: 
			//addiu
				rVals->R_rt = rVals->R_rs + d->regs.i.addr_or_immed;
				return rVals->R_rt; 

			case 8:
			//andi
				rVals->R_rt = rVals->R_rs & d->regs.i.addr_or_immed;
				return rVals->R_rt; 
				
			case 13: 
			//ori
				rVals->R_rt= rVals->R_rs | d->regs.i.addr_or_immed;
				return rVals->R_rt; 
				
			case 15:
			//lui
				rVals->R_rt = d->regs.i.addr_or_immed << 16; 
				return rVals->R_rt; 
			
			case 4:
			//beq
				if(rVals->R_rs - rVals->R_rt == 0) {
					return d->regs.i.addr_or_immed;
				} else {
					return 0;
				}

			case 5:
			//bne
				if(rVals->R_rs - rVals->R_rt != 0) {

					return d->regs.i.addr_or_immed;

				} else {

					return 0;

				}

			case 35: 
			{
			//lw(35)

			int val = rVals->R_rs + d->regs.i.addr_or_immed; 
			return val; 

			}

			case 43:
			{
			//sw(43)

			int val = rVals->R_rs + d->regs.i.addr_or_immed; 
			return val; 

			}

		}

	}
  return 0;
}

/* 
 * Update the program counter based on the current instruction. For
 * instructions other than branches and jumps, for example, the PC
 * increments by 4 (which we have provided).
 */
void UpdatePC ( DecodedInstr* d, int val) {

	switch (d->op) {

		case 0: {
			if (d->regs.r.funct == 8) {
				mips.pc = mips.registers[val];
			} else {
				mips.pc+=4;
			}
			
			break;
		}

		case 4: 
		case 5:
		{
		//beq or bne PC Update by addition [PC + 4 + val]: If no branch val = 0; 
		//Must be able to branch up or down 
		//Checking if up or down 
			if ((val & 0x80000000) < 0) {

				mips.pc = mips.pc + 4 - (val * 4); 

			} else {

				mips.pc = mips.pc + 4 + (val * 4); 
			}

			break; 
		}
		case 3: 

			mips.registers[31] = mips.pc + 4; 

		case 2:  {
		//jump
			unsigned int pcHolder = mips.pc; 

			//Shift val by 2 to the left to add 00 
			val = val << 2; 

			//Isolating the first 4 bits of the PC
			pcHolder = pcHolder & 0xf0000000;


			//Grab the 4 bits from pcHolder, the 26 from val plus the 00 
			mips.pc = val + pcHolder;

			break;
		}
		
		default: 
			mips.pc+=4;
		
	}



    
    /* Your code goes here */
}

/*
 * Perform memory load or store. Place the address of any updated memory 
 * in *changedMem, otherwise put -1 in *changedMem. Return any memory value 
 * that is read, otherwise return -1. 
 *
 * Remember that we're mapping MIPS addresses to indices in the mips.memory 
 * array. mips.memory[0] corresponds with address 0x00400000, mips.memory[1] 
 * with address 0x00400004, and so forth.
 *
 */
int Mem( DecodedInstr* d, int val, int *changedMem) {
    /* Your code goes here */

	if (d->op == 35) {
		//Changed MEM
		*changedMem = -1;
		val = val - 0x00400000;
		val = val / 4; 
		rVals.R_rt = mips.memory[val]; 
	    return rVals.R_rt; 
	} else if (d->op == 43) {
		//Changed MEM
		*changedMem = val;
		val  = val - 0x00400000;
		val = val / 4;  
	    mips.memory[val] = rVals.R_rt;
	    return *changedMem;
	} else {
		*changedMem = -1; 
	}

	return -1; 
}

/* 
 * Write back to register. If the instruction modified a register--
 * (including jal, which modifies $ra) --
 * put the index of the modified register in *changedReg,
 * otherwise put -1 in *changedReg.
 */
void RegWrite( DecodedInstr* d, int val, int *changedReg) {
    /* Your code goes here */
		switch(d->type) {

			//R Instruction write back to RD
			case 0: {
				if (d->regs.r.funct != 8) {//Checks for jr
					mips.registers[d->regs.r.rd] = rVals.R_rd; 
					*changedReg = d->regs.r.rd; 
				} else {
					*changedReg = -1;
				}
				break;
			}

			//I Instruction write back to RT
			case 1: { 
				if (d->op != 43 && d->op !=4 && d->op != 5) {//Checks for sw, beq, and bne
					mips.registers[d->regs.i.rt] = rVals.R_rt; 
					*changedReg = d->regs.i.rt;
				} else {
					*changedReg = -1;
				}
				break; 
			}

			case 2: {
				//Jump doesnt write to registers 
				*changedReg = -1; 
			}

		}
	

}
