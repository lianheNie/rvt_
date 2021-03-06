/* --COPYRIGHT--,BSD
 * Copyright (c) 2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

#include <stdint.h>
#include <stdbool.h>

#include <vibration/DSPLib.h>

/*
 * Perform an in-place inverse DFT of the complex input signal of length N/2 to
 * obtain a real result of length N. This is done using the inverse split
 * operation and then performing an inverse DFT of length N/2. Fixed scaling by
 * 2 is performed at each stage.
 */
msp_status msp_ifft_q15(const msp_ifft_q15_params *params, _q15 *src)
{
    msp_status status;                          // Status of the operations
    msp_cmplx_ifft_q15_params  paramsCmplxIFFT; // Complex IFFT params structure
    msp_isplit_q15_params  paramsISplit;        // Inverse split operation params structure
    
    /* Initialize split operation params structure. */
    paramsISplit.length = params->length;
    paramsISplit.splitTable = params->splitTable;
    
    /* Perform the inverse split operation. */
    status = msp_isplit_q15(&paramsISplit, src);
        
    /* Check if the operation was not successful. */
    if (status !=  MSP_SUCCESS) {
        return status;
    }
    
    /* Initialize complex IFFT params structure with size N/2. */
    paramsCmplxIFFT.length = params->length >> 1;
    paramsCmplxIFFT.bitReverse = params->bitReverse;
    paramsCmplxIFFT.bitReverseTable = params->bitReverseTable;
    paramsCmplxIFFT.twiddleTable = params->twiddleTable;
    
    /*
     * Perform an N/2 complex IFFT on the complex input to obtain N real
     * results.
     */
    return msp_cmplx_ifft_q15(&paramsCmplxIFFT, src);
}
