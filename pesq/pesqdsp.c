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
#include "pesq.h"
#include "dsp.h"

void DC_block( float * data, long Nsamples )
{
    float *p;
    long count;
    float facc = 0.0f;

    long ofs = SEARCHBUFFER * Downsample;

    p = data + ofs;
    for( count = (Nsamples - 2 * ofs); count > 0L; count-- )
        facc += *(p++);
    facc /= Nsamples;

    p = data + ofs;
    for( count = (Nsamples - 2 * ofs); count > 0L; count-- )
        *(p++) -= facc;

    p = data + ofs;
    for( count = 0L; count < Downsample; count++ )
       *(p++) *= (0.5f + count) / Downsample;

    p = data + Nsamples - ofs - 1L;
    for( count = 0L; count < Downsample; count++ )
       *(p--) *= (0.5f + count) / Downsample;
}

//long InIIR_Nsos;
float *InIIR_Hsos;

void apply_filters( float * data, long Nsamples )
{
    IIRFilt( InIIR_Hsos, InIIR_Nsos, NULL,
             data, Nsamples + DATAPADDING_MSECS  * (Fs / 1000), NULL );
}

float interpolate (float    freq, 
                   double   filter_curve_db [][2],
                   int      number_of_points) {
    double  result;
    int     i;
    double  freqLow, freqHigh;
    double  curveLow, curveHigh;
    
    if (freq <= filter_curve_db [0][0]) {
        freqLow = filter_curve_db [0][0];
        curveLow = filter_curve_db [0][1];
        freqHigh = filter_curve_db [1][0];
        curveHigh = filter_curve_db [1][1];

        result = ((freq - freqLow) * curveHigh + (freqHigh - freq) * curveLow)/ (freqHigh - freqLow);
    
        return (float) result;
    }

    if (freq >= filter_curve_db [number_of_points-1][0]) {
        freqLow = filter_curve_db [number_of_points-2][0];
        curveLow = filter_curve_db [number_of_points-2][1];
        freqHigh = filter_curve_db [number_of_points-1][0];
        curveHigh = filter_curve_db [number_of_points-1][1];

        result = ((freq - freqLow) * curveHigh + (freqHigh - freq) * curveLow)/ (freqHigh - freqLow);
    
        return (float) result;
    }
        
    i = 1;
    freqHigh = filter_curve_db [i][0];
    while (freqHigh < freq) {
        i++;
        freqHigh = filter_curve_db [i][0];    
    }
    curveHigh = filter_curve_db [i][1];

    freqLow = filter_curve_db [i-1][0];
    curveLow = filter_curve_db [i-1][1];

    result = ((freq - freqLow) * curveHigh + (freqHigh - freq) * curveLow)/ (freqHigh - freqLow);

    return (float) result;
}       


void apply_filter ( float * data, long maxNsamples, int number_of_points, double filter_curve_db [][2] )
{ 
    long    n           = maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000);
    long    pow_of_2    = nextpow2 (n);
    float    *x            = (float *) safe_malloc ((pow_of_2 + 2) * sizeof (float));

    float    factorDb, factor;
    
    float   overallGainFilter = interpolate ((float) 1000, filter_curve_db, number_of_points); 
    float   freq_resolution;
    int        i;
    
    for (i = 0; i < pow_of_2 + 2; i++) {
        x [i] = 0;
    }

    for (i = 0; i < n; i++) {
        x [i] = data [i + SEARCHBUFFER * Downsample];    
    }

    RealFFT (x, pow_of_2);
    
    freq_resolution = (float) Fs / (float) pow_of_2;


    for (i = 0; i <= pow_of_2/2; i++) { 
        factorDb = interpolate (i * freq_resolution, filter_curve_db, number_of_points) - overallGainFilter;
        factor = (float) pow ((float) 10, factorDb / (float) 20); 

        x [2 * i] *= factor;       
        x [2 * i + 1] *= factor;   
    }

    RealIFFT (x, pow_of_2);

    for (i = 0; i < n; i++) {
        data [i + SEARCHBUFFER * Downsample] = x[i];    
    }

    safe_free (x);
}

void apply_VAD( SIGNAL_INFO * pinfo, float * data, float * VAD, float * logVAD )
{
    float g;
    float LevelThresh;
    float LevelNoise;
    float StDNoise;
    float LevelSig;
    float LevelMin;
    long  count;
    long  iteration;
    long  length;
    long  start;
    long  finish;
    long  Nwindows = (*pinfo).Nsamples / Downsample;

    for( count = 0L; count < Nwindows; count++ )
    {
        VAD[count] = 0.0f;
        for( iteration = 0L; iteration < Downsample; iteration++ )
        {
            g = data[count * Downsample + iteration];
            VAD[count] += (g * g);
        }
        VAD[count] /= Downsample;
    }

    LevelThresh = 0.0f;
    for( count = 0L; count < Nwindows; count++ )
        LevelThresh += VAD[count];
    LevelThresh /= Nwindows;

    LevelMin = 0.0f;
    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] > LevelMin )
            LevelMin = VAD[count];
    if( LevelMin > 0.0f )
        LevelMin *= 1.0e-4f;
    else
        LevelMin = 1.0f;
    
    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] < LevelMin )
            VAD[count] = LevelMin;

    for( iteration = 0L; iteration < 12L; iteration++ )
    {
        LevelNoise = 0.0f;
        StDNoise = 0.0f;
        length = 0L;
        for( count = 0L; count < Nwindows; count++ )
            if( VAD[count] <= LevelThresh )
            {
                LevelNoise += VAD[count];
                length++;
            }
        if( length > 0L )
        {
            LevelNoise /= length;
            for( count = 0L; count < Nwindows; count++ )
                if( VAD[count] <= LevelThresh )
                {
                    g = VAD[count] - LevelNoise;
                    StDNoise += g * g;
                }
            StDNoise = (float)sqrt(StDNoise / length);
        }

        LevelThresh = 1.001f * (LevelNoise + 2.0f * StDNoise);
    }

    LevelNoise = 0.0f;
    LevelSig = 0.0f;
    length = 0L;
    for( count = 0L; count < Nwindows; count++ )
    {
        if( VAD[count] > LevelThresh )
        {
            LevelSig += VAD[count];
            length++;
        }
        else
            LevelNoise += VAD[count];
    }
    if( length > 0L )
        LevelSig /= length;
    else
        LevelThresh = -1.0f;
    if( length < Nwindows )
        LevelNoise /= (Nwindows - length);
    else
        LevelNoise = 1.0f;

    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] <= LevelThresh )
            VAD[count] = -VAD[count];

    VAD[0] = -LevelMin;
    VAD[Nwindows-1] = -LevelMin;

    start = 0L;
    finish = 0L;
    for( count = 1; count < Nwindows; count++ )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
            start = count;
        if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
        {
            finish = count;
            if( (finish - start) <= MINSPEECHLGTH )
                for( iteration = start; iteration < finish; iteration++ )
                    VAD[iteration] = -VAD[iteration];
        }
    }

    if( LevelSig >= (LevelNoise * 1000.0f) )
    {
        for( count = 1; count < Nwindows; count++ )
        {
            if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
                start = count;
            if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
            {
                finish = count;
                g = 0.0f;
                for( iteration = start; iteration < finish; iteration++ )
                    g += VAD[iteration];
                if( g < 3.0f * LevelThresh * (finish - start) )
                    for( iteration = start; iteration < finish; iteration++ )
                        VAD[iteration] = -VAD[iteration];
            }
        }
    }

    start = 0L;
    finish = 0L;
    for( count = 1; count < Nwindows; count++ )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
        {
            start = count;
            if( (finish > 0L) && ((start - finish) <= JOINSPEECHLGTH) )
                for( iteration = finish; iteration < start; iteration++ )
                    VAD[iteration] = LevelMin;
        }
        if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
            finish = count;
    }

    start = 0L;
    for( count = 1; count < Nwindows; count++ )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-1] <= 0.0f) )
            start = count;
    }
    if( start == 0L )
    {
        for( count = 0L; count < Nwindows; count++ )
            VAD[count] = (float)fabs(VAD[count]);
        VAD[0] = -LevelMin;
        VAD[Nwindows-1] = -LevelMin;
    }

    count = 3;
    while( count < (Nwindows-2) )
    {
        if( (VAD[count] > 0.0f) && (VAD[count-2] <= 0.0f) )
        {
            VAD[count-2] = VAD[count] * 0.1f;
            VAD[count-1] = VAD[count] * 0.3f;
            count++;
        }
        if( (VAD[count] <= 0.0f) && (VAD[count-1] > 0.0f) )
        {
            VAD[count] = VAD[count-1] * 0.3f;
            VAD[count+1] = VAD[count-1] * 0.1f;
            count += 3;
        }
        count++;
    }

    for( count = 0L; count < Nwindows; count++ )
        if( VAD[count] < 0.0f ) VAD[count] = 0.0f;
    
    if( LevelThresh <= 0.0f )
        LevelThresh = LevelMin;
    for( count = 0L; count < Nwindows; count++ )
    {
        if( VAD[count] <= LevelThresh )
            logVAD[count] = 0.0f;
        else
            logVAD[count] = (float)log( VAD[count]/LevelThresh );
    }
}

void crude_align(
    SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, ERROR_INFO * err_info,
    long Utt_id, float * ftmp)
{
    long  nr;
    long  nd;
    long  startr;
    long  startd;
    long  count;
    long  I_max;
    float max;
    float * ref_VAD = (*ref_info).logVAD;
    float * deg_VAD = (*deg_info).logVAD;
    float * Y;

    if( Utt_id == WHOLE_SIGNAL )
    {
        nr = (*ref_info).Nsamples / Downsample;
        nd = (*deg_info).Nsamples / Downsample;
        startr = 0L;
        startd = 0L;
    }
    else if( Utt_id == MAXNUTTERANCES )
    {
        startr = (*err_info).UttSearch_Start[MAXNUTTERANCES-1];
        startd = startr + (*err_info).Utt_DelayEst[MAXNUTTERANCES-1] / Downsample;

        if ( startd < 0L )
        {
            startr = -(*err_info).Utt_DelayEst[MAXNUTTERANCES-1] / Downsample;
            startd = 0L;
        }

        nr = (*err_info).UttSearch_End[MAXNUTTERANCES-1] - startr;
        nd = nr;

        if( startd + nd > (*deg_info).Nsamples / Downsample )
            nd = (*deg_info).Nsamples / Downsample - startd;
    }
    else
    {
        startr = (*err_info).UttSearch_Start[Utt_id];
        startd = startr + (*err_info).Crude_DelayEst / Downsample;

        if ( startd < 0L )
        {
            startr = -(*err_info).Crude_DelayEst / Downsample;
            startd = 0L;
        }

        nr = (*err_info).UttSearch_End[Utt_id] - startr;
        nd = nr;

        if( startd + nd > (*deg_info).Nsamples / Downsample )
            nd = (*deg_info).Nsamples / Downsample - startd;
    }

    Y  = ftmp;
        
    if( (nr > 1L) && (nd > 1L) )
        FFTNXCorr( ref_VAD + startr, nr, deg_VAD + startd, nd, Y );

    max = 0.0f;
    I_max = nr - 1;
    if( (nr > 1L) && (nd > 1L) )
        for( count = 0L; count < (nr+nd-1); count++ )
            if( Y[count] > max )
            {
                max = Y[count];
                I_max = count;
            }

    if( Utt_id == WHOLE_SIGNAL )
    {
        (*err_info).Crude_DelayEst = (I_max - nr + 1) * Downsample;
        (*err_info).Crude_DelayConf = 0.0f;
    }
    else if( Utt_id == MAXNUTTERANCES )
    {
        (*err_info).Utt_Delay[MAXNUTTERANCES-1] =
            (I_max - nr + 1) * Downsample + (*err_info).Utt_DelayEst[MAXNUTTERANCES-1];
    }
    else
    {
        (*err_info).Utt_DelayEst[Utt_id] =
            (I_max - nr + 1) * Downsample + (*err_info).Crude_DelayEst;
    }

    FFTFree();
}

void time_align(
    SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, ERROR_INFO * err_info,
    long Utt_id, float * ftmp )
{
    long  count;
    long  I_max;
    float v_max;
    long  estdelay;
    long  startr;
    long  startd;
    float * X1;
    float * X2;
    float * H;
    float * Window;
    float r1, i1;
    long  kernel;
    float Hsum;

    estdelay = (*err_info).Utt_DelayEst[Utt_id];

    X1 = ftmp;
    X2 = ftmp + Align_Nfft + 2;
    H  = (ftmp + 4 + 2 * Align_Nfft);
    for( count = 0L; count < Align_Nfft; count++ )
        H[count] = 0.0f;
    Window = ftmp + 5 * Align_Nfft;

    for( count = 0L; count < Align_Nfft; count++ )
         Window[count] = (float)(0.5 * (1.0 - cos((TWOPI * count) / Align_Nfft)));

    startr = (*err_info).UttSearch_Start[Utt_id] * Downsample;
    startd = startr + estdelay;

    if ( startd < 0L )
    {
        startr = -estdelay;
        startd = 0L;
    }

    while( ((startd + Align_Nfft) <= (*deg_info).Nsamples) &&
           ((startr + Align_Nfft) <= ((*err_info).UttSearch_End[Utt_id] * Downsample)) )
    {
        for( count = 0L; count < Align_Nfft; count++ )
        {
            X1[count] = (*ref_info).data[count + startr] * Window[count];
            X2[count] = (*deg_info).data[count + startd] * Window[count];
            
        }
        RealFFT( X1, Align_Nfft );
        RealFFT( X2, Align_Nfft );

        for( count = 0L; count <= Align_Nfft / 2; count++ )
        {
            r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
            X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
            X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
        }

        RealIFFT( X1, Align_Nfft );

        v_max = 0.0f;
        for( count = 0L; count < Align_Nfft; count++ )
        {
            r1 = (float) fabs(X1[count]);
            X1[count] = r1;
            if( r1 > v_max ) v_max = r1;
        }
        v_max *= 0.99f;
        for( count = 0L; count < Align_Nfft; count++ )
            if( X1[count] > v_max )
                H[count] += (float) pow( v_max, 0.125 );

        startr += (Align_Nfft / 4);
        startd += (Align_Nfft / 4);
    }

    Hsum = 0.0f;
    for( count = 0L; count < Align_Nfft; count++ )
    {
        Hsum += H[count];
        X1[count] = H[count];
        X2[count] = 0.0f;        
    }

    X2[0] = 1.0f;
    kernel = Align_Nfft / 64;
    for( count = 1; count < kernel; count++ )
    {
        X2[count] = 1.0f - ((float)count) / ((float)kernel);
        X2[(Align_Nfft - count)] = 1.0f - ((float)count) / ((float)kernel);
    }
    RealFFT( X1, Align_Nfft );
    RealFFT( X2, Align_Nfft );

    for( count = 0L; count <= Align_Nfft / 2; count++ )
    {
        r1 = X1[count * 2]; i1 = X1[1 + (count * 2)];
        X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
        X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
    }
    RealIFFT( X1, Align_Nfft );

    for( count = 0L; count < Align_Nfft; count++ )
    {
        if( Hsum > 0.0 )
            H[count] = (float) fabs(X1[count]) / Hsum;
        else
            H[count] = 0.0f;
    }

    v_max = 0.0f;
    I_max = 0L;
    for( count = 0L; count < Align_Nfft; count++ )
        if( H[count] > v_max )
        {
            v_max = H[count];
            I_max = count;
        }
    if( I_max >= (Align_Nfft/2) )
        I_max -= Align_Nfft;

    (*err_info).Utt_Delay[Utt_id] = estdelay + I_max;
    (*err_info).Utt_DelayConf[Utt_id] = v_max;

    FFTFree();
}

void split_align( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info, float * ftmp,
    long Utt_Start, long Utt_SpeechStart, long Utt_SpeechEnd, long Utt_End,
    long Utt_DelayEst, float Utt_DelayConf,
    long * Best_ED1, long * Best_D1, float * Best_DC1,
    long * Best_ED2, long * Best_D2, float * Best_DC2,
    long * Best_BP )
{
    long count, bp, k;
    long Utt_Len = Utt_SpeechEnd - Utt_SpeechStart;
    long Utt_Test = MAXNUTTERANCES - 1;

    long N_BPs;
    long Utt_BPs[41];
    long Utt_ED1[41], Utt_ED2[41];
    long Utt_D1[41], Utt_D2[41];
    float Utt_DC1[41], Utt_DC2[41];

    long Delta, Step, Pad;

    long  estdelay;
    long  I_max;
    float v_max, n_max;
    long  startr;
    long  startd;
    float * X1;
    float * X2;
    float * H;
    float * Window;
    float r1, i1;
    long  kernel;
    float Hsum;

    *Best_DC1 = 0.0f;
    *Best_DC2 = 0.0f;

    X1 = ftmp;
    X2 = ftmp + 2 + Align_Nfft;
    H  = (ftmp + 4 + 2 * Align_Nfft);
    Window = ftmp + 6 + 3 * Align_Nfft;
    for( count = 0L; count < Align_Nfft; count++ )
         Window[count] = (float)(0.5 * (1.0 - cos((TWOPI * count) / Align_Nfft)));
    kernel = Align_Nfft / 64;

    Delta = Align_Nfft / (4 * Downsample);

    Step = (long) ((0.801 * Utt_Len + 40 * Delta - 1)/(40 * Delta));
    Step *= Delta;

    Pad = Utt_Len / 10;
    if( Pad < 75 ) Pad = 75;
    Utt_BPs[0] = Utt_SpeechStart + Pad;
    N_BPs = 0;
    do {
        N_BPs++;
        Utt_BPs[N_BPs] = Utt_BPs[N_BPs-1] + Step;
    } while( (Utt_BPs[N_BPs] <= (Utt_SpeechEnd - Pad)) && (N_BPs < 40) );

    if( N_BPs <= 0 ) return;  

    for( bp = 0; bp < N_BPs; bp++ )
    {
        (*err_info).Utt_DelayEst[Utt_Test] = Utt_DelayEst;
        (*err_info).UttSearch_Start[Utt_Test] = Utt_Start;
        (*err_info).UttSearch_End[Utt_Test] = Utt_BPs[bp];

        crude_align( ref_info, deg_info, err_info, MAXNUTTERANCES, ftmp);
        Utt_ED1[bp] = (*err_info).Utt_Delay[Utt_Test];

        (*err_info).Utt_DelayEst[Utt_Test] = Utt_DelayEst;
        (*err_info).UttSearch_Start[Utt_Test] = Utt_BPs[bp];
        (*err_info).UttSearch_End[Utt_Test] = Utt_End;

        crude_align( ref_info, deg_info, err_info, MAXNUTTERANCES, ftmp);
        Utt_ED2[bp] = (*err_info).Utt_Delay[Utt_Test];
    }

    for( bp = 0; bp < N_BPs; bp++ )
        Utt_DC1[bp] = -2.0f;
    while( 1 )
    {
        bp = 0;
        while( (bp < N_BPs) && (Utt_DC1[bp] > -2.0) )
            bp++;
        if( bp >= N_BPs )
            break;

        estdelay = Utt_ED1[bp];

        for( count = 0L; count < Align_Nfft; count++ )
            H[count] = 0.0f;
        Hsum = 0.0f;

        startr = Utt_Start * Downsample;
        startd = startr + estdelay;

        if ( startd < 0L )
        {
            startr = -estdelay;
            startd = 0L;
        }

        while( ((startd + Align_Nfft) <= (*deg_info).Nsamples) &&
               ((startr + Align_Nfft) <= (Utt_BPs[bp] * Downsample)) )
        {
            for( count = 0L; count < Align_Nfft; count++ )
            {
                X1[count] = (*ref_info).data[count + startr] * Window[count];
                X2[count] = (*deg_info).data[count + startd] * Window[count];                
            }
            RealFFT( X1, Align_Nfft );
            RealFFT( X2, Align_Nfft );

            for( count = 0L; count <= Align_Nfft / 2; count++ )
            {
                r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
            }

            RealIFFT( X1, Align_Nfft );

            v_max = 0.0f;
            for( count = 0L; count < Align_Nfft; count++ )
            {
                r1 = (float) fabs(X1[count]);
                X1[count] = r1;
                if( r1 > v_max ) v_max = r1;
            }
            v_max *= 0.99f;
            n_max = (float) pow( v_max, 0.125 ) / kernel;

            for( count = 0L; count < Align_Nfft; count++ )
                if( X1[count] > v_max )
                {
                    Hsum += n_max * kernel;
                    for( k = 1-kernel; k < kernel; k++ )
                        H[(count + k + Align_Nfft) % Align_Nfft] +=
                            n_max * (kernel - (float) labs(k));
                }

            startr += (Align_Nfft / 4);
            startd += (Align_Nfft / 4);
        }

        v_max = 0.0f;
        I_max = 0L;
        for( count = 0L; count < Align_Nfft; count++ )
            if( H[count] > v_max )
            {
                v_max = H[count];
                I_max = count;
            }
        if( I_max >= (Align_Nfft/2) )
            I_max -= Align_Nfft;

        Utt_D1[bp] = estdelay + I_max;
        if( Hsum > 0.0 )
            Utt_DC1[bp] = v_max / Hsum;
        else
            Utt_DC1[bp] = 0.0f;

        while( bp < (N_BPs - 1) )
        {
            bp++;
            if( (Utt_ED1[bp] == estdelay) && (Utt_DC1[bp] <= -2.0) )
            {
                while( ((startd + Align_Nfft) <= (*deg_info).Nsamples) &&
                       ((startr + Align_Nfft) <= (Utt_BPs[bp] * Downsample)) )
                {
                    for( count = 0L; count < Align_Nfft; count++ )
                    {
                        X1[count] = (*ref_info).data[count + startr] * Window[count];
                        X2[count] = (*deg_info).data[count + startd] * Window[count];                        
                    }
                    RealFFT( X1, Align_Nfft );
                    RealFFT( X2, Align_Nfft );

                    for( count = 0L; count <= Align_Nfft/2; count++ )
                    {
                        r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                        X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                        X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
                    }

                    RealIFFT( X1, Align_Nfft );

                    v_max = 0.0f;
                    for( count = 0L; count < Align_Nfft; count++ )
                    {
                        r1 = (float) fabs(X1[count]);
                        X1[count] = r1;
                        if( r1 > v_max ) v_max = r1;
                    }
                    v_max *= 0.99f;
                    n_max = (float) pow( v_max, 0.125 ) / kernel;

                    for( count = 0L; count < Align_Nfft; count++ )
                        if( X1[count] > v_max )
                        {
                            Hsum += n_max * kernel;
                            for( k = 1-kernel; k < kernel; k++ )
                                H[(count + k + Align_Nfft) % Align_Nfft] +=
                                    n_max * (kernel - (float) labs(k));
                        }

                    startr += (Align_Nfft / 4);
                    startd += (Align_Nfft / 4);
                }

                v_max = 0.0f;
                I_max = 0L;
                for( count = 0L; count < Align_Nfft; count++ )
                    if( H[count] > v_max )
                    {
                        v_max = H[count];
                        I_max = count;
                    }
                if( I_max >= (Align_Nfft/2) )
                    I_max -= Align_Nfft;

                Utt_D1[bp] = estdelay + I_max;
                if( Hsum > 0.0 )
                    Utt_DC1[bp] = v_max / Hsum;
                else
                    Utt_DC1[bp] = 0.0f;
            }
        }
    }

    for( bp = 0; bp < N_BPs; bp++ )
    {
        if( Utt_DC1[bp] > Utt_DelayConf )
            Utt_DC2[bp] = -2.0f;
        else
            Utt_DC2[bp] = 0.0f;
    }
    while( 1 )
    {
        bp = N_BPs - 1;
        while( (bp >= 0) && (Utt_DC2[bp] > -2.0) )
            bp--;
        if( bp < 0 )
            break;

        estdelay = Utt_ED2[bp];

        for( count = 0L; count < Align_Nfft; count++ )
            H[count] = 0.0f;
        Hsum = 0.0f;

        startr = Utt_End * Downsample - Align_Nfft;
        startd = startr + estdelay;

        if ( (startd + Align_Nfft) > (*deg_info).Nsamples )
        {
            startd = (*deg_info).Nsamples - Align_Nfft;
            startr = startd - estdelay;
        }

        while( (startd >= 0L) &&
               (startr >= (Utt_BPs[bp] * Downsample)) )
        {
            for( count = 0L; count < Align_Nfft; count++ )
            {
                X1[count] = (*ref_info).data[count + startr] * Window[count];
                X2[count] = (*deg_info).data[count + startd] * Window[count];                
            }
            RealFFT( X1, Align_Nfft );
            RealFFT( X2, Align_Nfft );

            for( count = 0L; count <= Align_Nfft/2; count++ )
            {
                r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
            }

            RealIFFT( X1, Align_Nfft );

            v_max = 0.0f;
            for( count = 0L; count < Align_Nfft; count++ )
            {
                r1 = (float) fabs(X1[count]);
                X1[count] = r1;
                if( r1 > v_max ) v_max = r1;
            }
            v_max *= 0.99f;
            n_max = (float) pow( v_max, 0.125 ) / kernel;

            for( count = 0L; count < Align_Nfft; count++ )
                if( X1[count] > v_max )
                {
                    Hsum += n_max * kernel;
                    for( k = 1-kernel; k < kernel; k++ )
                        H[(count + k + Align_Nfft) % Align_Nfft] +=
                            n_max * (kernel - (float) labs(k));
                }

            startr -= (Align_Nfft / 4);
            startd -= (Align_Nfft / 4);
        }

        v_max = 0.0f;
        I_max = 0L;
        for( count = 0L; count < Align_Nfft; count++ )
            if( H[count] > v_max )
            {
                v_max = H[count];
                I_max = count;
            }
        if( I_max >= (Align_Nfft/2) )
            I_max -= Align_Nfft;

        Utt_D2[bp] = estdelay + I_max;
        if( Hsum > 0.0 )
            Utt_DC2[bp] = v_max / Hsum;
        else
            Utt_DC2[bp] = 0.0f;

        while( bp > 0 )
        {
            bp--;
            if( (Utt_ED2[bp] == estdelay) && (Utt_DC2[bp] <= -2.0) )
            {
                while( (startd >= 0L) &&
                       (startr >= (Utt_BPs[bp] * Downsample)) )
                {
                    for( count = 0L; count < Align_Nfft; count++ )
                    {
                        X1[count] = (*ref_info).data[count + startr] * Window[count];
                        X2[count] = (*deg_info).data[count + startd] * Window[count];                        
                    }
                    RealFFT( X1, Align_Nfft );
                    RealFFT( X2, Align_Nfft );

                    for( count = 0L; count <= Align_Nfft / 2; count++ )
                    {
                        r1 = X1[count * 2]; i1 = -X1[1 + (count * 2)];
                        X1[count * 2] = (r1 * X2[count * 2] - i1 * X2[1 + (count * 2)]);
                        X1[1 + (count * 2)] = (r1 * X2[1 + (count * 2)] + i1 * X2[count * 2]);
                    }

                    RealIFFT( X1, Align_Nfft );

                    v_max = 0.0f;
                    for( count = 0L; count < Align_Nfft; count++ )
                    {
                        r1 = (float) fabs(X1[count]);
                        X1[count] = r1;
                        if( r1 > v_max ) v_max = r1;
                    }
                    v_max *= 0.99f;
                    n_max = (float) pow( v_max, 0.125 ) / kernel;

                    for( count = 0L; count < Align_Nfft; count++ )
                        if( X1[count] > v_max )
                        {
                            Hsum += n_max * kernel;
                            for( k = 1-kernel; k < kernel; k++ )
                                H[(count + k + Align_Nfft) % Align_Nfft] +=
                                    n_max * (kernel - (float) labs(k));
                        }

                    startr -= (Align_Nfft / 4);
                    startd -= (Align_Nfft / 4);
                }

                v_max = 0.0f;
                I_max = 0L;
                for( count = 0L; count < Align_Nfft; count++ )
                    if( H[count] > v_max )
                    {
                        v_max = H[count];
                        I_max = count;
                    }
                if( I_max >= (Align_Nfft/2) )
                    I_max -= Align_Nfft;

                Utt_D2[bp] = estdelay + I_max;
                if( Hsum > 0.0 )
                    Utt_DC2[bp] = v_max / Hsum;
                else
                    Utt_DC2[bp] = 0.0f;
            }
        }
    }

    for( bp = 0; bp < N_BPs; bp++ )
    {
        if( (labs(Utt_D2[bp] - Utt_D1[bp]) >= Downsample) &&
            ((Utt_DC1[bp] + Utt_DC2[bp]) > ((*Best_DC1) + (*Best_DC2))) &&
            (Utt_DC1[bp] > Utt_DelayConf) && (Utt_DC2[bp] > Utt_DelayConf) )
            {
                *Best_ED1 = Utt_ED1[bp]; *Best_D1 = Utt_D1[bp]; *Best_DC1 = Utt_DC1[bp];
                *Best_ED2 = Utt_ED2[bp]; *Best_D2 = Utt_D2[bp]; *Best_DC2 = Utt_DC2[bp];
                *Best_BP = Utt_BPs[bp];
            }
    }

    FFTFree();
}

/* END OF FILE */
