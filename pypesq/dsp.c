/*****************************************************************************

Perceptual Evaluation of Speech Quality (PESQ)
ITU-T Recommendations P.862, P.862.1, P.862.2.
Version 2.0 - October 2005.

              ****************************************
              PESQ Intellectual Property Rights Notice
              ****************************************

DEFINITIONS:
------------
For the purposes of this Intellectual Property Rights Notice
the terms ‘Perceptual Evaluation of Speech Quality Algorithm’
and ‘PESQ Algorithm’ refer to the objective speech quality
measurement algorithm defined in ITU-T Recommendation P.862;
the term ‘PESQ Software’ refers to the C-code component of P.862.
These definitions also apply to those parts of ITU-T Recommendation 
P.862.2 and its associated source code that are common with P.862.

NOTICE:
-------
All copyright, trade marks, trade names, patents, know-how and
all or any other intellectual rights subsisting in or used in
connection with including all algorithms, documents and manuals
relating to the PESQ Algorithm and or PESQ Software are and remain
the sole property in law, ownership, regulations, treaties and
patent rights of the Owners identified below. The user may not
dispute or question the ownership of the PESQ Algorithm and
or PESQ Software.

OWNERS ARE:
-----------

1.	British Telecommunications plc (BT), all rights assigned
      to Psytechnics Limited
2.	Royal KPN NV, all rights assigned to OPTICOM GmbH

RESTRICTIONS:
-------------

The user cannot:

1.	alter, duplicate, modify, adapt, or translate in whole or in
      part any aspect of the PESQ Algorithm and or PESQ Software
2.	sell, hire, loan, distribute, dispose or put to any commercial
      use other than those permitted below in whole or in part any
      aspect of the PESQ Algorithm and or PESQ Software

PERMITTED USE:
--------------

The user may:

1.	Use the PESQ Software to:
      i)   understand the PESQ Algorithm; or
      ii)  evaluate the ability of the PESQ Algorithm to perform
           its intended function of predicting the speech quality
           of a system; or
      iii) evaluate the computational complexity of the PESQ Algorithm,
           with the limitation that none of said evaluations or its
           results shall be used for external commercial use.

2.	Use the PESQ Software to test if an implementation of the PESQ
      Algorithm conforms to ITU-T Recommendation P.862.

3.	With the prior written permission of both Psytechnics Limited
      and OPTICOM GmbH, use the PESQ Software in accordance with the
      above Restrictions to perform work that meets all of the following
      criteria:
      i)    the work must contribute directly to the maintenance of an
            existing ITU recommendation or the development of a new ITU
            recommendation under an approved ITU Study Item; and
      ii)   the work and its results must be fully described in a
            written contribution to the ITU that is presented at a formal
            ITU meeting within one year of the start of the work; and
      iii)  neither the work nor its results shall be put to any
            commercial use other than making said contribution to the ITU.
            Said permission will be provided on a case-by-case basis.


ANY OTHER USE OR APPLICATION OF THE PESQ SOFTWARE AND/OR THE PESQ
ALGORITHM WILL REQUIRE A PESQ LICENCE AGREEMENT, WHICH MAY BE OBTAINED
FROM EITHER OPTICOM GMBH OR PSYTECHNICS LIMITED. 

EACH COMPANY OFFERS OEM LICENSE AGREEMENTS, WHICH COMBINE OEM
IMPLEMENTATIONS OF THE PESQ ALGORITHM TOGETHER WITH A PESQ PATENT LICENSE
AGREEMENT. PESQ PATENT-ONLY LICENSE AGREEMENTS MAY BE OBTAINED FROM OPTICOM.


***********************************************************************
*  OPTICOM GmbH                    *  Psytechnics Limited             *
*  Naegelsbachstr. 38,             *  Fraser House, 23 Museum Street, *
*  D- 91052 Erlangen, Germany      *  Ipswich IP1 1HN, England        *
*  Phone: +49 (0) 9131 53020 0     *  Phone: +44 (0) 1473 261 800     *
*  Fax:   +49 (0) 9131 53020 20    *  Fax:   +44 (0) 1473 261 880     *
*  E-mail: info@opticom.de,        *  E-mail: info@psytechnics.com,   *
*  www.opticom.de                  *  www.psytechnics.com             *
***********************************************************************

Further information is also available from www.pesq.org

*****************************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "dsp.h"

#ifndef TWOPI
  #define TWOPI   6.283185307179586f
#endif

unsigned long   FFTSwapInitialised = 0;
unsigned long   FFTLog2N;
unsigned long * FFTButter;
unsigned long * FFTBitSwap;
float         * FFTPhi;

long total_malloced = 0;

void *safe_malloc (unsigned long size) {
    void *result;
    total_malloced += size;
    result = malloc (size);
    if (result == NULL) {
        printf ("malloc failed!\n");
    }
    return result;
}

void safe_free (void *p) {
    free (p);
}


unsigned long nextpow2(unsigned long X)
{
    unsigned long C = 1;
    while( (C < ULONG_MAX) && (C < X) )
        C <<= 1;

    return C;
}

int ispow2(unsigned long X)
{
    unsigned long C = 1;
    while( (C < ULONG_MAX) && (C < X) )
        C <<= 1;
        
    return (C == X);
}

int intlog2(unsigned long X)
{
    return (int)floor( log( 1.0 * X ) / log( 2.0 ) + 0.5 );
}

void FFTInit(unsigned long N)
{
    unsigned long   C, L, K;
    float           Theta;
    float         * PFFTPhi;
    
    if( (FFTSwapInitialised != N) && (FFTSwapInitialised != 0) )
        FFTFree();

    if( FFTSwapInitialised == N )
    {
        return;
    }
    else
    {
        C = N;
        for( FFTLog2N = 0; C > 1; C >>= 1 )
            FFTLog2N++;

        C = 1;
        C <<= FFTLog2N;
        if( N == C )
            FFTSwapInitialised = N;

        FFTButter = (unsigned long *) safe_malloc( sizeof(unsigned long) * (N >> 1) );
        FFTBitSwap = (unsigned long *) safe_malloc( sizeof(unsigned long) * N );
        FFTPhi = (float *) safe_malloc( 2 * sizeof(float) * (N >> 1) );
    
        PFFTPhi = FFTPhi;
        for( C = 0; C < (N >> 1); C++ )
        {
            Theta = (TWOPI * C) / N;
            (*(PFFTPhi++)) = (float) cos( Theta );
            (*(PFFTPhi++)) = (float) sin( Theta );
        }
    
        FFTButter[0] = 0;
        L = 1;
        K = N >> 2;
        while( K >= 1 )
        {
            for( C = 0; C < L; C++ )
                FFTButter[C+L] = FFTButter[C] + K;
            L <<= 1;
            K >>= 1;
        }
    }
}

void FFTFree(void)
{
    if( FFTSwapInitialised != 0 )
    {
        safe_free( FFTButter );
        safe_free( FFTBitSwap );
        safe_free( FFTPhi );
        FFTSwapInitialised = 0;
    }
}

void FFT(float * x, unsigned long N)
{
    unsigned long   Cycle, C, S, NC;
    unsigned long   Step    = N >> 1;
    unsigned long   K1, K2;
    register float  R1, I1, R2, I2;
    float           ReFFTPhi, ImFFTPhi;

    if( N > 1 )
    {
        FFTInit( N );
    
        for( Cycle = 1; Cycle < N; Cycle <<= 1, Step >>= 1 )
        {
            K1 = 0;
            K2 = Step << 1;
    
            for( C = 0; C < Cycle; C++ )
            {
                NC = FFTButter[C] << 1;
                ReFFTPhi = FFTPhi[NC];
                ImFFTPhi = FFTPhi[NC+1];
                for( S = 0; S < Step; S++ )
                {
                    R1 = x[K1];
                    I1 = x[K1+1];
                    R2 = x[K2];
                    I2 = x[K2+1];
                    
                    x[K1++] = R1 + ReFFTPhi * R2 + ImFFTPhi * I2;
                    x[K1++] = I1 - ImFFTPhi * R2 + ReFFTPhi * I2;
                    x[K2++] = R1 - ReFFTPhi * R2 - ImFFTPhi * I2;
                    x[K2++] = I1 + ImFFTPhi * R2 - ReFFTPhi * I2;
                }
                K1 = K2;
                K2 = K1 + (Step << 1);
            }
        }
    
        NC = N >> 1;
        for( C = 0; C < NC; C++ )
        {
            FFTBitSwap[C] = FFTButter[C] << 1;
            FFTBitSwap[C+NC] = 1 + FFTBitSwap[C];
        }
        for( C = 0; C < N; C++ )
            if( (S = FFTBitSwap[C]) != C )
            {
                FFTBitSwap[S] = S;
                K1 = C << 1;
                K2 = S << 1;
                R1 = x[K1];
                x[K1++] = x[K2];
                x[K2++] = R1;
                R1 = x[K1];
                x[K1] = x[K2];
                x[K2] = R1;
            }
    }
}

void IFFT(float * x, unsigned long N)
{
    unsigned long   Cycle, C, S, NC;
    unsigned long   Step    = N >> 1;
    unsigned long   K1, K2;
    register float  R1, I1, R2, I2;
    float           ReFFTPhi, ImFFTPhi;

    if( N > 1 )
    {
        FFTInit( N );
    
        for( Cycle = 1; Cycle < N; Cycle <<= 1, Step >>= 1 )
        {
            K1 = 0;
            K2 = Step << 1;
    
            for( C = 0; C < Cycle; C++ )
            {
                NC = FFTButter[C] << 1;
                ReFFTPhi = FFTPhi[NC];
                ImFFTPhi = FFTPhi[NC+1];
                for( S = 0; S < Step; S++ )
                {
                    R1 = x[K1];
                    I1 = x[K1+1];
                    R2 = x[K2];
                    I2 = x[K2+1];
                    
                    x[K1++] = R1 + ReFFTPhi * R2 - ImFFTPhi * I2;
                    x[K1++] = I1 + ImFFTPhi * R2 + ReFFTPhi * I2;
                    x[K2++] = R1 - ReFFTPhi * R2 + ImFFTPhi * I2;
                    x[K2++] = I1 - ImFFTPhi * R2 - ReFFTPhi * I2;
                }
                K1 = K2;
                K2 = K1 + (Step << 1);
            }
        }
    
        NC = N >> 1;
        for( C = 0; C < NC; C++ )
        {
            FFTBitSwap[C] = FFTButter[C] << 1;
            FFTBitSwap[C+NC] = 1 + FFTBitSwap[C];
        }
        for( C = 0; C < N; C++ )
            if( (S = FFTBitSwap[C]) != C )
            {
                FFTBitSwap[S] = S;
                K1 = C << 1;
                K2 = S << 1;
                R1 = x[K1];
                x[K1++] = x[K2];
                x[K2++] = R1;
                R1 = x[K1];
                x[K1] = x[K2];
                x[K2] = R1;
            }
    
        NC = N << 1;
        for( C = 0; C < NC; )
            x[C++] /= N;
    }
}

void RealFFT(float *x, unsigned long N) 
{
    float            *y;
    unsigned long    i;

    y = (float *) safe_malloc (2 * N * sizeof (float));

    for (i = 0; i < N; i++) {
        y [2 * i] = x [i];
        y [2 * i + 1] = 0.0f;
    }

    FFT (y, N);

    for (i = 0; i <= N / 2; i++) {
        x [2 * i] = y [2 * i];
        x [2 * i + 1] = y [2 * i + 1];
    }    
    
    safe_free (y);
}

void RealIFFT(float *x, unsigned long N)
{

    float            *y;
    unsigned long    i;

    y = (float *) safe_malloc (2 * N * sizeof (float));

    for (i = 0; i <= N / 2; i++) {
        y [2 * i] = x [2 * i];
        y [2 * i + 1] = x [2 * i + 1];
    }    
    for (i = N / 2 + 1; i < N; i++) {
        int j = N - i;
        y [2 * i] = x [2 * j];
        y [2 * i + 1] = -x [2 * j + 1];
    }    
    
    IFFT (y, N);
    
    for (i = 0; i < N; i++) {
        x [i] = y [2 * i];
    }

    safe_free (y);
}

unsigned long FFTNXCorr(
  float * x1, unsigned long n1,
  float * x2, unsigned long n2,
  float * y )
{
    register float  r1, i1;
    float         * tmp1;
    float         * tmp2;
    long            C, D, Nx, Ny;

    Nx = nextpow2( max(n1, n2) );
    tmp1 = (float *) safe_malloc(sizeof(float) * (2 * Nx + 2));
    tmp2 = (float *) safe_malloc(sizeof(float) * (2 * Nx + 2));

    for( C = n1 - 1; C >= 0; C-- )
    {
        tmp1[C] = *(x1++);
    }
    for( C = n1; C < 2 * Nx; C++ )
        tmp1[C] = 0.0;

    RealFFT( tmp1, 2*Nx );
    
    for( C = 0; C < (long) n2; C++ )
    {
        tmp2[C] = x2[C];
    }
    for( C = n2; C < 2 * Nx; C++ )
        tmp2[C] = 0.0;
    
    RealFFT( tmp2, 2*Nx );

    for( C = 0; C <= Nx; C++ )
    {
        D = C << 1; r1 = tmp1[D]; i1 = tmp1[1 + D];
        tmp1[D] = r1 * tmp2[D] - i1 * tmp2[1 + D];
        tmp1[1 + D] = r1 * tmp2[1 + D] + i1 * tmp2[D];
    }

    RealIFFT( tmp1, 2*Nx );
    Ny = n1 + n2 - 1;
    for( C = 0; C < Ny; C++ )
        y[C] = tmp1[C];
    
    safe_free( tmp1 );
    safe_free( tmp2 );

    return Ny;
}

void IIRsos(
    float * x, unsigned long Nx,
    float b0, float b1, float b2, float a1, float a2,
    float * tz1, float * tz2 )
{
    register float z0;
    register float z1;
    register float z2;

    if( tz1 == NULL ) z1 = 0.0f; else z1 = *tz1;
    if( tz2 == NULL ) z2 = 0.0f; else z2 = *tz2;
    
    if( (a1 != 0.0f) || (a2 != 0.0f) )
    {
        if( (b1 != 0.0f) || (b2 != 0.0f) )
        {
            while( (Nx) > 0 )
            {
                Nx--;
                                z0 = (*x) - a1 * z1 - a2 * z2;
                *(x++) = b0 * z0 + b1 * z1 + b2 * z2;
                z2 = z1;
                z1 = z0;
            }
        }
        else
        {
            if( b0 != 1.0f )
            {
                while( (Nx) > 0 )
                                {
                                        Nx--;
                    z0 = (*x) - a1 * z1 - a2 * z2;
                    *(x++) = b0 * z0;
                    z2 = z1;
                    z1 = z0;
                }
            }
            else
            {
                while( (Nx) > 0 )
                {
                    Nx--;
                    z0 = (*x) - a1 * z1 - a2 * z2;
                    *(x++) = z0;
                    z2 = z1;
                    z1 = z0;
                }
            }
        }
    }
    else
    {
        if( (b1 != 0.0f) || (b2 != 0.0f) )
        {
            while( (Nx) > 0 )
            {
                Nx--;
                z0 = (*x);
                *(x++) = b0 * z0 + b1 * z1 + b2 * z2;
                z2 = z1;
                z1 = z0;
            }
        }
        else
        {
            if( b0 != 1.0f )
            {
                while( (Nx) > 0 )
                {
                    Nx--;
                    *x = b0 * (*x);
                    x++;
                }
            }
        }
    }

    if( tz1 != NULL ) (*tz1) = z1;
    if( tz2 != NULL ) (*tz2) = z2;
}

void IIRFilt(
    float * h, unsigned long Nsos, float * z,
    float * x, unsigned long Nx, float * y )
{
    unsigned long C;

    if( y == NULL )
        y = x;
    else
    {
        for( C = 0; C < Nx; C++ )
            y[C] = x[C];
    }


    for( C = 0; C < Nsos; C++ )
    {
        if( z != NULL )
        {
            IIRsos( y, Nx, h[0], h[1], h[2], h[3], h[4], z, z+1 );
            z += 2;
        }
        else
            IIRsos( y, Nx, h[0], h[1], h[2], h[3], h[4], NULL, NULL );
        h += 5;
    }
}
/* END OF FILE */
