/*
  Copyright (c) 2010-2011, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIEvals, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

#define NOMINMAX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <algorithm>
using std::max;

#include "../timing.h"

#include "transcendentals_ispc.h"
using namespace ispc;

// val1 = exp(val) * sin(val) * atan(val)
// val2 = log(val1);

void transcendental_serial(float vals[], float result[], int count) {
  for (int i = 0; i < count; i++) {
    float val = vals[i];
    result[i] = log(exp(val) * exp(2*val) * sin(val) * atan(val));
  }
}

static void usage() {
    printf("usage: options [--count=<num elements>]\n");
}


int main(int argc, char *argv[]) {
    const int nRounds = 100;
    int nElements = 1024*1024;

    for (int i = 1; i < argc; ++i) {
        if (strncmp(argv[i], "--count=", 8) == 0) {
            nElements = atoi(argv[i] + 8);
            if (nElements <= 0) {
                usage();
                exit(1);
            }
        }
    }

    float *vals = new float[nElements];
    float *result = new float[nElements];

    for (int i = 0; i < nElements; ++i) {
      vals[i] = 1.0;
    }

    double sum;
    //
    // Transcendental ispc implementation
    //
    double bs_ispc = 1e30;
    for (int i = 0; i < 3; ++i) {
        reset_and_start_timer();
        transcendental_ispc(vals, result, nElements);
        double dt = get_elapsed_mcycles();
        sum = 0.;
        for (int i = 0; i < nElements; ++i)
            sum += result[i];
        bs_ispc = std::min(bs_ispc, dt);
    }
    printf("[transcendental ispc, 1 thread]:\t[%.3f] million cycles (avg %f)\n", 
           bs_ispc, sum / nElements);

    //
    // Transcendental serial implementation
    //
    double transcendental_serial_n = 1e30;
    for (int i = 0; i < 3; ++i) {
        reset_and_start_timer();
        transcendental_serial(vals, result, nElements);
        double dt = get_elapsed_mcycles();
        sum = 0.;
        for (int i = 0; i < nElements; ++i)
            sum += result[i];
        transcendental_serial_n = std::min(transcendental_serial_n, dt);
    }
    printf("[transcendental serial]:\t\t[%.3f] million cycles (avg %f)\n", transcendental_serial_n, 
           sum / nElements);

    printf("\t\t\t\t(%.2fx speedup from ISPC)\n", 
           transcendental_serial_n / bs_ispc);

    return 0;
}
