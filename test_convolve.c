/* Copyright (C) 2012 Henry Gomersall <heng@cantab.net> 
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organization nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY  THE AUTHOR ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#if defined(_WIN32) || defined(__APPLE__)
#define HAVE_GLIBC 0
#else
#define HAVE_GLIBC 1
#endif // _WIN32 || __APPLE__

#include <stdio.h>
#include <stdlib.h>

#if HAVE_GLIBC
#include <glib.h>
#endif // HAVE_GLIBC

#include <sys/time.h>

#include "convolve.h"
#include "convolve_2d.h"

#include "test_data.h"

#define INPUT_LENGTH 1024
#define KERNEL_LENGTH 16
#define N_LOOPS 100
#define N_TESTS 10

#define ROWS 128

#define INPUT_ARRAY input_data_1024
#define KERNEL kernel_16
#define TEST_OUTPUT_CORRECT output_in1024_kernel16

long time_delta(struct timeval* now, struct timeval* then)
{
    long delta = 0l;

    delta += (now->tv_sec - then->tv_sec) * 1000000;
    delta += (now->tv_usec - then->tv_usec);

    return delta;
}

int main()
{
    float* test_output = malloc(
            sizeof(float)*(INPUT_LENGTH-KERNEL_LENGTH+1)*ROWS);

    float* workspace = malloc(
            sizeof(float)*(INPUT_LENGTH-KERNEL_LENGTH+1)*ROWS);

    float* input_array;
    if (ROWS > 1) {
        input_array = malloc(sizeof(float)*(INPUT_LENGTH*ROWS));
        for (int col=0; col<INPUT_LENGTH; col++){
            for(int row=0; row<ROWS; row++){
                input_array[row*INPUT_LENGTH + col] = INPUT_ARRAY[col];
            }
        }
    } else {
        input_array = INPUT_ARRAY;
    }

    struct timeval now, then;

    float min_delta = -1.0;
    float delta;
    printf("Running %d tests of %d loops\n", N_TESTS, N_LOOPS);

    for (int j=0; j<N_TESTS; j++){
        gettimeofday(&then, NULL);

        convolve_sse_2d_separable_multiple(input_array, test_output, workspace, 
        //convolve_sse_in_aligned_fixed_kernel_multiple(INPUT_ARRAY, test_output,
        //convolve_sse_in_aligned_multiple(INPUT_ARRAY, test_output,
        //convolve_sse_partial_unroll_multiple(INPUT_ARRAY, test_output,
        //convolve_sse_simple_multiple(INPUT_ARRAY, test_output,
        //convolve_naive_multiple(INPUT_ARRAY, test_output,
                    INPUT_LENGTH, ROWS, KERNEL, KERNEL_LENGTH, N_LOOPS);

        gettimeofday(&now, NULL);
        delta = ((float)time_delta(&now, &then))/N_LOOPS;

        min_delta = ((min_delta == -1.0) || 
                (delta < min_delta)) ? delta : min_delta;
    }

    printf("Lowest test time: %1.3f microseconds per loop.\n", 
            min_delta);

    for (int i=0; i<(INPUT_LENGTH-KERNEL_LENGTH+1); i++)
	{
        if (TEST_OUTPUT_CORRECT[i] != test_output[i])
		{
			
#if HAVE_GLIBC
            g_error("Computed convolution is incorrect.");
#else
			fprintf(stderr, "Computed convolution is incorrect.");
#endif // HAVE_GLIBC
			
            return(-1);
        }
    }

    printf("Convolution is valid.\n");    

    free(test_output);

    return 0;
}
