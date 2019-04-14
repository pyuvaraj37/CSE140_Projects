#include "tips.h"
#include <stdbool.h> 

/* The following two functions are defined in util.c */

/* finds the highest 1 bit, and returns its position, else 0xFFFFFFFF */
unsigned int uint_log2(word w); 

/* return random int from 0..x-1 */
int randomint( int x );

/*
  This function allows the lfu information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lfu information
 */
char* lfu_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lfu information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].accessCount);

  return buffer;
}

/*
  This function allows the lru information to be displayed

    assoc_index - the cache unit that contains the block to be modified
    block_index - the index of the block to be modified

  returns a string representation of the lru information
 */
char* lru_to_string(int assoc_index, int block_index)
{
  /* Buffer to print lru information -- increase size as needed. */
  static char buffer[9];
  sprintf(buffer, "%u", cache[assoc_index].block[block_index].lru.value);

  return buffer;
}

/*
  This function initializes the lfu information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lfu(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].accessCount = 0;
}

/*
  This function initializes the lru information

    assoc_index - the cache unit that contains the block to be modified
    block_number - the index of the block to be modified

*/
void init_lru(int assoc_index, int block_index)
{
  cache[assoc_index].block[block_index].lru.value = 0;
}

struct addy {
	
	unsigned int offset;
	unsigned int index;
	unsigned int tag; 

};

struct addy getBytes(address addr) {
    
    unsigned int o, i, t;

    switch (block_size) {
            
        case 4:
        {
        	o = 0x00000003 & addr;
            addr = addr >> 2;
        }
            break;
               
        case 8:
        {
        	o = 0x00000007 & addr;
            addr = addr >> 3;

        }	
            break;

        case 16:
        {
        	o = 0x0000000f & addr;
            addr = addr >> 4;
            
        }
            break;

        case 32:
        {
        	o = 0x0000001f & addr;
            addr = addr >> 5;
            
        }
            
    }
    
    //No index bit
    //1 set
    if(set_count == 1) {
        i = 0; 
    }
    
    //1 index bits
    //2 sets
    if(set_count == 2) {
        i = 0x00000001 & addr;
        addr = addr >> 1;
    }
    
    //2 index bits
    //4 sets
    if (set_count == 4) {
        i = 0x00000003 & addr;
        addr = addr >> 2;
    }
    
    //3 index bits
    //8 sets
    if (set_count == 8) {
        i = 0x00000007 & addr;
        addr = addr >> 3;
    }
    //4 index bits
    //16 sets
    if (set_count == 16) {
        i = 0x0000000f & addr;
        addr = addr >> 4;
    }
    //addr only contains the tag now

    t = addr; 

    struct addy holder = {o, i, t}; 

    return holder; 
}

int replacementPolicy(unsigned int index) {

	int replace = -1; 
	for (int i = 0; i < assoc; i++) {
	     if (cache[index].block[i].valid == 0){
	        replace = i; 
	    }
	}

	//If no spot in the set with an invalid bit
	//
	//Do replacement policy
	if (replace == -1) {

	    switch(policy) {
	        //Random
	        case 0: {
	        //Randomize between 0 and assoc - 1; 

	        }
	        break;
	        //LRU
	        case 1: {
	            lru: 

	            for (int i = 0; i < assoc; i++){
					                
					if (cache[index].block[i].lru.value == 0) {
 
					    replace = i; 

					}
				}	

	        }
	        break;
	        //LFU
	        case 2: {

	            int lfu = cache[index].block[0].accessCount; 
	            replace = 0; 

	            for (int i = 1; i < assoc; i++){
					                
					if (lfu < cache[index].block[i].accessCount) {

					    lfu = cache[index].block[i].accessCount; 

					    replace = i; 
					}

				}	
				//Check for a tie
				for (int i = 0; i < assoc; i++) {
					                
					if (lfu == cache[index].block[i].accessCount && replace != i) {

					    goto lru;             		
					}
				}	

	        }
	    }

	}
	return replace;  
}

//void* data? 
void lruUpdate(int index, int block) {

	for (int i = 0; i < assoc; i++) {

		if (i != block && cache[index].block[i].lru.value != 0) {
			cache[index].block[i].lru.value--; 
		}

		if (i == block) {

			cache[index].block[i].lru.value = assoc - 1;

		}

	}


}

int getByte() {

	switch(block_size) {
		case 4:

			return 2; 

		case 8:

			return 3;

		case 16:

			return 4; 

		case 32:

			return 5; 
	}

	return 0; 
}




/*
  This is the primary function you are filling out,
  You are free to add helper functions if you need them

  @param addr 32-bit byte address
  @param data a pointer to a SINGLE word (32-bits of data)
  @param we   if we == READ, then data used to return
              information back to CPU

              if we == WRITE, then data used to
              update Cache/DRAM
*/
void accessMemory(address addr, word* data, WriteEnable we)
{
  /* Declare variables here */
    
      /* handle the case of no cache at all - leave this in */
      if(assoc == 0) {
        accessDRAM(addr, (byte*)data, WORD_SIZE, we);
        return;
      }
    struct addy theAddr = getBytes(addr);
    unsigned int offset = theAddr.offset;
    unsigned int index = theAddr.index;
    unsigned int tag = theAddr.tag;
    bool hit = false; 
    TransferUnit b = getByte();

    switch (we) {
        //Read
        case 0:
            {
            	
                //Cache[index] -> block[association] -> cacheBlock
                for (int i = 0; i < assoc; i++) {

                	//Read HIT
                	//Valid bit to see if data is valid (Not valid of start up)
                	//Note: Dirty bit is used for write back. 
                	if(cache[index].block[i].valid == 1) {
                    	
                    	//The tags match
                		if (tag == cache[index].block[i].tag) {

                			//Find correct Block(Use offset)
                			//chznge to memcpy
                			highlight_offset(index, i, offset, HIT);
                			memcpy(data, cache[index].block[i].data + offset, 4);
                			cache[index].block[i].accessCount += 1;
                			lruUpdate(index, i);
                			hit = true; 
                		} 
                	}
            	}
            	//Read MISS
                //Need to go to main memory to cache
                //
                if (!hit) {

					int replace = replacementPolicy(index);
					highlight_offset(index, replace, offset, MISS);
                	//MAKE INTO A FUNCTION WILL USE FOR WRITE
	                //Decide how much memory to get from the physical memory (Block size)
	                //Search for an invalid bit in the set to physical memory to
	                cache[index].block[replace].accessCount += 1;
                	lruUpdate(index, replace);
                	cache[index].block[replace].valid = 1;
                	cache[index].block[replace].tag = tag;
	                //Access main memory 
	                accessDRAM(addr,(byte *)cache[index].block[replace].data, 0, READ);
	                highlight_block(index, replace);
	                //Then read cache 
	                memcpy(data, cache[index].block[replace].data + offset, 4);

                }

        	}
        	break; 
        //Write
        case 1:
            {

            	int replace = replacementPolicy(index);

            	switch (memory_sync_policy) {

            		//write back
            		//CACHE -> DIRTY (wait for change) -> MEM
            		case 0: {

            			if (cache[index].block[replace].dirty == 1) {
            				accessDRAM(addr, cache[index].block[replace].data, b, WRITE);
            			} 

            			//USE memcpy(); 
            			memcpy(cache[index].block[replace].data + offset, data, 4);
            			cache[index].block[replace].accessCount += 1;
                		lruUpdate(index, replace);
                		cache[index].block[replace].tag = tag;
                		cache[index].block[replace].valid = 1;
                		cache[index].block[replace].dirty = 1; 

            		}

            		//write through
            		//CACHE -> MEM
            		case 1: {
						//USE memcpy(); 
            			memcpy(cache[index].block[replace].data + offset, data, 4);
            			cache[index].block[replace].accessCount += 1;
                		lruUpdate(index, replace);
                		cache[index].block[replace].tag = tag;
                		cache[index].block[replace].valid = 1;
                		cache[index].block[replace].dirty = 1; 
						accessDRAM(addr, cache[index].block[replace].data, b, WRITE);


            		}


            	}

/*            	//check writh back
				if(memory_sync_policy == 0) {
					//check replacement policy
					if(polilcy == 0) { //Random
						
					}
					if(policy == 1) { //LRU

					}
					if(policy == 2) { //LFU
						int blk = LFU(index);
					}
				}
				//check write-through
				if(memory_sync_policy == 1) {
					//check replacement policy
					if(polilcy == 0) { 
						
					}
					if(policy == 1) {

					}
					if(policy == 2) {
						int blk = LFU(index);
					}
				}

*/
            }

            
    }

  /*
  You need to read/write between memory (via the accessDRAM() function) and
  the cache (via the cache[] global structure defined in tips.h)

  Remember to read tips.h for all the global variables that tell you the
  cache parameters

  The same code should handle random, LFU, and LRU policies. Test the policy
  variable (see tips.h) to decide which policy to execute. The LRU policy
  should be written such that no two blocks (when their valid bit is VALID)
  will ever be a candidate for replacement. In the case of a tie in the
  least number of accesses for LFU, you use the LRU information to determine
  which block to replace.

  Your cache should be able to support write-through mode (any writes to
  the cache get immediately copied to main memory also) and write-back mode
  (and writes to the cache only gets copied to main memory when the block
  is kicked out of the cache.

  Also, cache should do allocate-on-write. This means, a write operation
  will bring in an entire block if the block is not already in the cache.

  To properly work with the GUI, the code needs to tell the GUI code
  when to redraw and when to flash things. Descriptions of the animation
  functions can be found in tips.h
  */

  /* Start adding code here */


  /* This call to accessDRAM occurs when you modify any of the
     cache parameters. It is provided as a stop gap solution.
     At some point, ONCE YOU HAVE MORE OF YOUR CACHELOGIC IN PLACE,
     THIS LINE SHOULD BE REMOVED.
  */
  accessDRAM(addr, (byte*)data, WORD_SIZE, we);
}




