
/*
CONCEPTS USED FOR  OPTIMIZATIONS: 

EASY REORDERING
Column major -> Row major
[MUCH FASTER THAN NAIVE]
for( int j = 0; j < m; j++ )
    for( int k = 0; k < n; k++ )
        for( int i = 0; i < m; i++ ) {
            C[i+j*m] += A[i+k*m] * A[j+k*m];
              }

EASY REGISTER BLOCKING
[SAME SPEED AS NAIVE]
for( int i = 0; i < m; i++) {
    for( int k = 0; k < n; k++){
        float a = A[i + k * m];
        for( int j = 0; j < m; j++ ) {
            C[i+j*m] += a * A[j + k * m];
              }
          }
   }

REGISTER BLOCKING WITH REORDERING
[MUCH FASTER THAN NAIVE]
for( int j = 0; j < m; j++) {
    for( int k = 0; k < n; k++){
        float a = A[j + k * m];
        for( int i = 0; i < m; i++ ) {
            C[i+j*m] += A[i+k*m] * a;
              }
          }
   }
*/

//USE FOR SSE INSTRUCTIONS
#include <xmmintrin.h>

void dgemm( int m, int n, float *A, float *C ){

/*
 j is the row of mxm
 i is the column of mxm
*/
    
/*
UNROLLING - 40pts
	Unrolling 4 since it was the fastest.
	Reorder i and j.
	[MUCH FASTER]
	 Need another loop to calculate all of leftover
	 Test on MAC
	 Tried: 2, 3, 4, 5, 6, 7, 8 , 9 ,10
	 2: Faster than naive
	 3: Faster than unrolling 2 loops
	 4: Faster than unrolling 3 loops [FASTEST]
	 5: Slower than unrolling 4 loops
	 6: Faster than unrolling 5, but slower than unrolling 4 loops
	 7: Slower than unrolling 6.
	 8: Slower than unrolling 7.
	 9: Slower than unrolling 8.
	 10: Very slow and error.
*/
   //  int i, j, k, l;
   //  for(j = 0; j < m; j++) {
   //     for (k = 0; k < n - 3; k+=4) {
   //         for(i = 0; i < m; i++) {

   //             int index = i+j*m;

   //             C[index] += A[i+k*m] * A[j+k*m];
   //             C[index] += A[i+(k+1)*m] * A[j+(k+1)*m];
   //             C[index] += A[i+(k+2)*m] * A[j+(k+2)*m];
   //             C[index] += A[i+(k+3)*m] * A[j+(k+3)*m];


   //         }
   //     }
   //     //Second loop started where k left off
   //     for (l = k; l < n; l++) {
   //     		float a = A[j+l*m];
   //         for (i = 0; i < m; i++) {
   //             C[i+j*m] += A[i+l*m] * a;
   //         }
   //     }
   // }

//--------------------------------------------------------------------//

/*
SSE - 40pts

	SSE varibale __m128 (128 bits) can hold 4 floats (32 bits).
	_mm_loadu_ps() will load 4 contiguos pieces of data.
	So we can only use it for row(i). 

	0------------------------i---------------------->m
	0[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]
	|[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]
	|[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]
	k[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]
	|[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]
	|[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]
	n[128 Bits][128 Bits][128 Bits]..[32 Bits][32 Bits]

	Iterate by 4 for i. Until we reach the left overs.
	Iterate by 1 for k. Just going down the matrix row by row. 
	
	Tried to implement unrolling but caused an error.
	Use of register blocking 
*/

   int i, j, k;
   //4 since the SSE Varibale holds 4 floats. 
   const int leftOvers = m / 4 * 4;  
   for (j = 0; j < m; j++) {
   		for (k = 0; k < n; k++) {
   			//Now set all 4 floats to the same j value (column)
   			//so in next loop we can do 4 multiplcations at once
   			__m128 a_c = _mm_set_ps1(A[j+k*m]);
   			//Prefetch 32 floats of the row
   			_mm_prefetch(A + i + k * m, 32);
   			for (i = 0; i < leftOvers; i+=4) {
   				//Calculate address to load the 4 floats into varaible
   				//Twice for unrolling

   				float *a1_r_address = A + i + k * m;
				
   				__m128 a1_r = _mm_loadu_ps(a1_r_address);
   				__m128 prod_1 = _mm_mul_ps(a1_r, a_c);
   				float* c_address = C + i + j * m;
   				__m128 c = _mm_loadu_ps(c_address);
   				c = _mm_add_ps(c, prod_1);
   				_mm_storeu_ps(c_address, c);


   			}

   		}
   		for( int k = 0; k < n; k++ ){
	    	for(int i = leftOvers; i < m;i++){
	        	C[i+j*m]+=A[i+k*m] *A[j+k*m];
	        }
	    }
   }
    
//--------------------------------------------------------------------//
/*
CACHE BLOCKING - 40pts
*/	

    // int i, j, k, kk;
    // int leftOvers = n - (n % 4);
    // for (kk = 0; kk < leftOvers; kk += 4) {
    //     for (j = 0; j < m; j++) {
    //         for (i = 0; i < m; i++) {
    //             float r = C[i+j*m];
    //             for (k = kk; k < kk + 4; k++) {
    //                 r += A[j+k*m] *A[i+k*m];
    //             }
    //             C[i+j*m] = r;
    //         }
    //     }
    // }
    // for (k = leftOvers; k < n; k++) {
    //     for (j = 0; j < m; j++) {
    //         for (i = 0; i < m; i++) {
    //             C[i+j*m] += A[j+k*m] * A[i+k*m];
    //         }
    //     }
    // }

}//end of dgemm
