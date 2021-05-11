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

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "pesq.h"
#include "pesqpar.h"
#include "dsp.h"

#define        CRITERIUM_FOR_SILENCE_OF_5_SAMPLES        500.

float Sl, Sp;

int Nb;
long InIIR_Nsos;

int *nr_of_hz_bands_per_bark_band;
double *centre_of_band_bark;
double *centre_of_band_hz;
double *width_of_band_bark;
double *width_of_band_hz;
double *pow_dens_correction_factor;
double *abs_thresh_power;


void input_filter(
    SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info, float * ftmp )
{
    DC_block( (*ref_info).data, (*ref_info).Nsamples );
    DC_block( (*deg_info).data, (*deg_info).Nsamples );

    apply_filters( (*ref_info).data, (*ref_info).Nsamples );
    apply_filters( (*deg_info).data, (*deg_info).Nsamples );
}

void calc_VAD( SIGNAL_INFO * sinfo )
{
    apply_VAD( sinfo, sinfo-> data, sinfo-> VAD, sinfo-> logVAD );
}

int id_searchwindows( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info )
{
    long  Utt_num = 0;
    long  count, VAD_length;
    long  this_start;
    int   speech_flag = 0;
    float VAD_value;
    long  del_deg_start;
    long  del_deg_end;

    VAD_length = ref_info-> Nsamples / Downsample;

    del_deg_start = MINUTTLENGTH - err_info-> Crude_DelayEst / Downsample;
    del_deg_end =
        ((*deg_info).Nsamples - err_info-> Crude_DelayEst) / Downsample -
        MINUTTLENGTH;

    for (count = 0; count < VAD_length; count++)
    {
        VAD_value = ref_info-> VAD [count];

        if( (VAD_value > 0.0f) && (speech_flag == 0) ) 
        {
            speech_flag = 1;
            this_start = count;
            err_info-> UttSearch_Start [Utt_num] = count - SEARCHBUFFER;
            if( err_info-> UttSearch_Start [Utt_num] < 0 )
                err_info-> UttSearch_Start [Utt_num] = 0;
        }

        if( ((VAD_value == 0.0f) || (count == (VAD_length-1))) &&
            (speech_flag == 1) ) 
        {
            speech_flag = 0;
            err_info-> UttSearch_End [Utt_num] = count + SEARCHBUFFER;
            if( err_info-> UttSearch_End [Utt_num] > VAD_length - 1 )
                err_info-> UttSearch_End [Utt_num] = VAD_length -1;

            if( ((count - this_start) >= MINUTTLENGTH) &&
                (this_start < del_deg_end) &&
                (count > del_deg_start) )
                Utt_num++;            
        }
    }

    err_info-> Nutterances = Utt_num;
    return Utt_num;
} 

void id_utterances( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info )
{
    long  Utt_num = 0;
    long  Largest_uttsize = 0;
    long  count, VAD_length;
    int   speech_flag = 0;
    float VAD_value;
    long  this_start;
    long  last_end;
    long  del_deg_start;
    long  del_deg_end;

    VAD_length = ref_info-> Nsamples / Downsample;

    del_deg_start = MINUTTLENGTH - err_info-> Crude_DelayEst / Downsample;
    del_deg_end =
        ((*deg_info).Nsamples - err_info-> Crude_DelayEst) / Downsample -
        MINUTTLENGTH;

    for (count = 0; count < VAD_length ; count++)
    {
        VAD_value = ref_info-> VAD [count];
        if( (VAD_value > 0.0f) && (speech_flag == 0) ) 
        {
            speech_flag = 1;
            this_start = count;
            err_info-> Utt_Start [Utt_num] = count;
        }

        if( ((VAD_value == 0.0f) || (count == (VAD_length-1))) &&
            (speech_flag == 1) ) 
        {
            speech_flag = 0;
            err_info-> Utt_End [Utt_num] = count;

            if( ((count - this_start) >= MINUTTLENGTH) &&
                (this_start < del_deg_end) &&
                (count > del_deg_start) )
                Utt_num++;            
        }
    }

    err_info-> Utt_Start [0] = SEARCHBUFFER;
    err_info-> Utt_End [err_info-> Nutterances-1] = (VAD_length - SEARCHBUFFER);
    
    for (Utt_num = 1; Utt_num < err_info-> Nutterances; Utt_num++ )
    {
        this_start = err_info-> Utt_Start [Utt_num];
        last_end = err_info-> Utt_End [Utt_num - 1];
        count = (this_start + last_end) / 2;
        err_info-> Utt_Start [Utt_num] = count;
        err_info-> Utt_End [Utt_num - 1] = count;
    }

    this_start = (err_info-> Utt_Start [0] * Downsample) + err_info-> Utt_Delay [0];
    if( this_start < (SEARCHBUFFER * Downsample) )
    {
        count = SEARCHBUFFER +
                (Downsample - 1 - err_info-> Utt_Delay [0]) / Downsample;
        err_info-> Utt_Start [0] = count;
    }
    last_end = (err_info-> Utt_End [err_info-> Nutterances-1] * Downsample) +
               err_info-> Utt_Delay [err_info-> Nutterances-1];
    if( last_end > ((*deg_info).Nsamples - SEARCHBUFFER * Downsample) )
    {
        count = ( (*deg_info).Nsamples -
                  err_info-> Utt_Delay [err_info-> Nutterances-1] ) / Downsample -
                SEARCHBUFFER;
        err_info-> Utt_End [err_info-> Nutterances-1] = count;
    }

    for (Utt_num = 1; Utt_num < err_info-> Nutterances; Utt_num++ )
    {
        this_start =
            (err_info-> Utt_Start [Utt_num] * Downsample) +
            err_info-> Utt_Delay [Utt_num];
        last_end =
            (err_info-> Utt_End [Utt_num - 1] * Downsample) +
            err_info-> Utt_Delay [Utt_num - 1];
        if( this_start < last_end )
        {
            count = (this_start + last_end) / 2;
            this_start =
                (Downsample - 1 + count - err_info-> Utt_Delay [Utt_num]) / Downsample;
            last_end =
               (count - err_info-> Utt_Delay [Utt_num - 1]) / Downsample;
            err_info-> Utt_Start [Utt_num] = this_start;
            err_info-> Utt_End [Utt_num - 1] = last_end;
        }
    }

    for (Utt_num = 0; Utt_num < err_info-> Nutterances; Utt_num++ )
        if( (err_info-> Utt_End [Utt_num] - err_info-> Utt_Start [Utt_num])
             > Largest_uttsize )
            Largest_uttsize = 
                err_info-> Utt_End [Utt_num] - err_info-> Utt_Start [Utt_num];

    err_info-> Largest_uttsize = Largest_uttsize;
}

void utterance_split( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info, float * ftmp )
{
    long Utt_id;
    long Utt_DelayEst;
    long Utt_Delay;
    float Utt_DelayConf;
    long Utt_Start;
    long Utt_End;
    long Utt_SpeechStart;
    long Utt_SpeechEnd;
    long Utt_Len;
    long step;
    long Best_ED1, Best_ED2;
    long Best_D1, Best_D2;
    float Best_DC1, Best_DC2;
    long Best_BP;
    long Largest_uttsize = 0;

    Utt_id = 0;
    while( (Utt_id < err_info-> Nutterances) &&
           (err_info-> Nutterances < MAXNUTTERANCES) )
    {
        Utt_DelayEst = err_info-> Utt_DelayEst [Utt_id];
        Utt_Delay = err_info-> Utt_Delay [Utt_id];
        Utt_DelayConf = err_info-> Utt_DelayConf [Utt_id];
        Utt_Start = err_info-> Utt_Start [Utt_id];
        Utt_End = err_info-> Utt_End [Utt_id];

        Utt_SpeechStart = Utt_Start;
        while( (Utt_SpeechStart < Utt_End) && (ref_info-> VAD [Utt_SpeechStart] <= 0.0f) )
            Utt_SpeechStart++;
        Utt_SpeechEnd = Utt_End;
        while( (Utt_SpeechEnd > Utt_Start) && (ref_info-> VAD [Utt_SpeechEnd] <= 0.0f) )
            Utt_SpeechEnd--;
        Utt_SpeechEnd++;
        Utt_Len = Utt_SpeechEnd - Utt_SpeechStart;

        if( Utt_Len >= 200 )
        {
            split_align( ref_info, deg_info, err_info, ftmp,
                Utt_Start, Utt_SpeechStart, Utt_SpeechEnd, Utt_End,
                Utt_DelayEst, Utt_DelayConf,
                &Best_ED1, &Best_D1, &Best_DC1,
                &Best_ED2, &Best_D2, &Best_DC2,
                &Best_BP );

            if( (Best_DC1 > Utt_DelayConf) && (Best_DC2 > Utt_DelayConf) )
            {
                for (step = err_info-> Nutterances-1; step > Utt_id; step-- )
                {
                    err_info-> Utt_DelayEst [step +1] = err_info-> Utt_DelayEst [step];
                    err_info-> Utt_Delay [step +1] = err_info-> Utt_Delay [step];
                    err_info-> Utt_DelayConf [step +1] = err_info-> Utt_DelayConf [step];
                    err_info-> Utt_Start [step +1] = err_info-> Utt_Start [step];
                    err_info-> Utt_End [step +1] = err_info-> Utt_End [step];
                    err_info-> UttSearch_Start [step +1] = err_info-> Utt_Start [step];
                    err_info-> UttSearch_End [step +1] = err_info-> Utt_End [step];
                }
                err_info-> Nutterances++;

                err_info-> Utt_DelayEst [Utt_id] = Best_ED1;
                err_info-> Utt_Delay [Utt_id] = Best_D1;
                err_info-> Utt_DelayConf [Utt_id] = Best_DC1;

                err_info-> Utt_DelayEst [Utt_id +1] = Best_ED2;
                err_info-> Utt_Delay [Utt_id +1] = Best_D2;
                err_info-> Utt_DelayConf [Utt_id +1] = Best_DC2;

                err_info-> UttSearch_Start [Utt_id +1] = err_info-> UttSearch_Start [Utt_id];
                err_info-> UttSearch_End [Utt_id +1] = err_info-> UttSearch_End [Utt_id];

                if( Best_D2 < Best_D1 )
                {
                    err_info-> Utt_Start [Utt_id] = Utt_Start;
                    err_info-> Utt_End [Utt_id] = Best_BP;
                    err_info-> Utt_Start [Utt_id +1] = Best_BP;
                    err_info-> Utt_End [Utt_id +1] = Utt_End;
                }
                else
                {
                    err_info-> Utt_Start [Utt_id] = Utt_Start;
                    err_info-> Utt_End [Utt_id] = Best_BP + (Best_D2 - Best_D1) / (2 * Downsample);
                    err_info-> Utt_Start [Utt_id +1] = Best_BP - (Best_D2 - Best_D1) / (2 * Downsample);
                    err_info-> Utt_End [Utt_id +1] = Utt_End;
                }

                if( (err_info-> Utt_Start [Utt_id] - SEARCHBUFFER) * Downsample + Best_D1 < 0 )
                    err_info-> Utt_Start [Utt_id] =
                        SEARCHBUFFER + (Downsample - 1 - Best_D1) / Downsample;

                if( (err_info-> Utt_End [Utt_id +1] * Downsample + Best_D2) >
                    ((*deg_info).Nsamples - SEARCHBUFFER * Downsample) )
                    err_info-> Utt_End [Utt_id +1] =
                        ((*deg_info).Nsamples - Best_D2) / Downsample - SEARCHBUFFER;

            }
            else Utt_id++;
        }
        else Utt_id++;
    }

    for (Utt_id = 0; Utt_id < err_info-> Nutterances; Utt_id++ )
        if( (err_info-> Utt_End [Utt_id] - err_info-> Utt_Start [Utt_id])
             > Largest_uttsize )
            Largest_uttsize = 
                err_info-> Utt_End [Utt_id] - err_info-> Utt_Start [Utt_id];

    err_info-> Largest_uttsize = Largest_uttsize;
}

void utterance_locate( SIGNAL_INFO * ref_info, SIGNAL_INFO * deg_info,
    ERROR_INFO * err_info, float * ftmp )
{    
    long Utt_id;
    
    id_searchwindows( ref_info, deg_info, err_info );

    for (Utt_id = 0; Utt_id < err_info-> Nutterances; Utt_id++)
    {
        crude_align( ref_info, deg_info, err_info, Utt_id, ftmp);
        time_align(ref_info, deg_info, err_info, Utt_id, ftmp );
    }

    id_utterances( ref_info, deg_info, err_info );

    utterance_split( ref_info, deg_info, err_info, ftmp );   
}


void short_term_fft (int Nf, SIGNAL_INFO *info, float *window, long start_sample, float *hz_spectrum, float *fft_tmp) {
    int n, k;        

    for (n = 0; n < Nf; n++ )
    {
        fft_tmp [n] = info-> data [start_sample + n] * window [n];
    }
    RealFFT(fft_tmp, Nf);

    for (k = 0; k < Nf / 2; k++ ) 
    {
        hz_spectrum [k] = fft_tmp [k << 1] * fft_tmp [k << 1] + fft_tmp [1 + (k << 1)] * fft_tmp [1 + (k << 1)];
    }    

    hz_spectrum [0] = 0;
}

void freq_warping (int number_of_hz_bands, float *hz_spectrum, int Nb, float *pitch_pow_dens, long frame) {

    int        hz_band = 0;
    int        bark_band;
    double    sum;

    for (bark_band = 0; bark_band < Nb; bark_band++) {
        int n = nr_of_hz_bands_per_bark_band [bark_band];
        int i;

        sum = 0;
        for (i = 0; i < n; i++) {
            sum += hz_spectrum [hz_band++];
        }
        
        sum *= pow_dens_correction_factor [bark_band];
        sum *= Sp;
        pitch_pow_dens [frame * Nb + bark_band] = (float) sum;
    }
}

float total_audible (int frame, float *pitch_pow_dens, float factor) {
    int        band;
    float     h, threshold;
    double  result;
    
    result = 0.;
    for (band= 1; band< Nb; band++) {
        h = pitch_pow_dens [frame * Nb + band];
        threshold = (float) (factor * abs_thresh_power [band]);
        if (h > threshold) {
            result += h;
        }
    }
    return (float) result;
}

void time_avg_audible_of (int number_of_frames, int *silent, float *pitch_pow_dens, float *avg_pitch_pow_dens, int total_number_of_frames) 
{
    int    frame;
    int    band;

    for (band = 0; band < Nb; band++) {
        double result = 0;
        for (frame = 0; frame < number_of_frames; frame++) {
            if (!silent [frame]) {
                float h = pitch_pow_dens [frame * Nb + band];
                if (h > 100 * abs_thresh_power [band]) {
                    result += h;
                }
            }
        }

        avg_pitch_pow_dens [band] = (float) (result / total_number_of_frames);
    }
}            

void freq_resp_compensation (int number_of_frames, float *pitch_pow_dens_ref, float *avg_pitch_pow_dens_ref, float *avg_pitch_pow_dens_deg, float constant)
{
    int band;

    for (band = 0; band < Nb; band++) {
        float    x = (avg_pitch_pow_dens_deg [band] + constant) / (avg_pitch_pow_dens_ref [band] + constant);
        int        frame;

        if (x > (float) 100.0) {x = (float) 100.0;} 
        if (x < (float) 0.01) {x = (float) 0.01;}   

        for (frame = 0; frame < number_of_frames; frame++) {        
            pitch_pow_dens_ref [frame * Nb + band] *= x;
        }        
    }
}

#define ZWICKER_POWER       0.23 

void intensity_warping_of (float *loudness_dens, int frame, float *pitch_pow_dens)
{
    int        band;
    float    h;
    double    modified_zwicker_power;

    for (band = 0; band < Nb; band++) {
        float threshold = (float) abs_thresh_power [band];
        float input = pitch_pow_dens [frame * Nb + band];

        if (centre_of_band_bark [band] < (float) 4) {
            h =  (float) 6 / ((float) centre_of_band_bark [band] + (float) 2);
        } else {
            h = (float) 1;
        }
        if (h > (float) 2) {h = (float) 2;}
        h = (float) pow (h, (float) 0.15); 
        modified_zwicker_power = ZWICKER_POWER * h;

        if (input > threshold) {
            loudness_dens [band] = (float) (pow (threshold / 0.5, modified_zwicker_power)
                                                    * (pow (0.5 + 0.5 * input / threshold, modified_zwicker_power) - 1));
        } else {
            loudness_dens [band] = 0;
        }

        loudness_dens [band] *= (float) Sl;
    }    
}

float pseudo_Lp (int n, float *x, float p) {   
    double totalWeight = 0;
    double result = 0;
    int    band;

    for (band = 1; band < Nb; band++) {
        float h = (float) fabs (x [band]);        
        float w = (float) width_of_band_bark [band];
        float prod = h * w;

        result += pow (prod, p);
        totalWeight += w;
    }

    result /= totalWeight;
    result = pow (result, 1/p);
    result *= totalWeight;
    
    return (float) result;
}  
void multiply_with_asymmetry_factor (float      *disturbance_dens, 
                                     int         frame, 
                                     const float   * const pitch_pow_dens_ref, 
                                     const float   * const pitch_pow_dens_deg) 
{
    int   i;
    float ratio, h;

    for (i = 0; i < Nb; i++) {
        ratio = (pitch_pow_dens_deg [frame * Nb + i] + (float) 50)
                  / (pitch_pow_dens_ref [frame * Nb + i] + (float) 50);

        h = (float) pow (ratio, (float) 1.2);    
        if (h > (float) 12) {h = (float) 12;}
        if (h < (float) 3) {h = (float) 0.0;}

        disturbance_dens [i] *= h;
    }
}

double pow_of (const float * const x, long start_sample, long stop_sample, long divisor) {
    assert(start_sample >= 0);
    assert(start_sample <= stop_sample);

    long    i;
    double  power = 0;

    for (i = start_sample; i < stop_sample; i++) {
        float h = x [i];
        power += h * h;        
    }
    
    power /= divisor;
    return power;
}


int compute_delay (long              start_sample, 
                   long                 stop_sample, 
                   long                 search_range, 
                   float            *time_series1, 
                   float            *time_series2,
                   float            *max_correlation) {

    double            power1, power2, normalization;
    long            i;
    float           *x1, *x2, *y;
    double            h;
    long            n = stop_sample - start_sample;   
    long            power_of_2 = nextpow2 (2 * n);
    long            best_delay;

    power1 = pow_of (time_series1, start_sample, stop_sample, stop_sample - start_sample) * (double) n/(double) power_of_2;
    power2 = pow_of (time_series2, start_sample, stop_sample, stop_sample - start_sample) * (double) n/(double) power_of_2;
    normalization = sqrt (power1 * power2);

    if ((power1 <= 1E-6) || (power2 <= 1E-6)) {
        *max_correlation = 0;
        return 0;
    }

    x1 = (float *) safe_malloc ((power_of_2 + 2) * sizeof (float));;
    x2 = (float *) safe_malloc ((power_of_2 + 2) * sizeof (float));;
    y = (float *) safe_malloc ((power_of_2 + 2) * sizeof (float));;
    
    for (i = 0; i < power_of_2 + 2; i++) {
        x1 [i] = 0.;
        x2 [i] = 0.;
        y [i] = 0.;
    }

    for (i = 0; i < n; i++) {
        x1 [i] = (float) fabs (time_series1 [i + start_sample]);
        x2 [i] = (float) fabs (time_series2 [i + start_sample]);
    }

    RealFFT (x1, power_of_2);
    RealFFT (x2, power_of_2);

    for (i = 0; i <= power_of_2 / 2; i++) { 
        x1 [2 * i] /= power_of_2;
        x1 [2 * i + 1] /= power_of_2;                
    }

    for (i = 0; i <= power_of_2 / 2; i++) { 
        y [2*i] = x1 [2*i] * x2 [2*i] + x1 [2*i + 1] * x2 [2*i + 1];
        y [2*i + 1] = -x1 [2*i + 1] * x2 [2*i] + x1 [2*i] * x2 [2*i + 1];
    }    
  
    RealIFFT (y, power_of_2);

    best_delay = 0;
    *max_correlation = 0;

    for (i = -search_range; i <= -1; i++) {
        h = (float) fabs (y [(i + power_of_2)]) / normalization;
        if (fabs (h) > (double) *max_correlation) {
            *max_correlation = (float) fabs (h);
            best_delay= i;
        }
    }

    for (i = 0; i < search_range; i++) {
        h = (float) fabs (y [i]) / normalization;
        if (fabs (h) > (double) *max_correlation) {
            *max_correlation = (float) fabs (h);
            best_delay= i;
        }
    }

    safe_free (x1);
    safe_free (x2);
    safe_free (y);
    
    return best_delay;
}

    
#define NUMBER_OF_PSQM_FRAMES_PER_SYLLABE       20
 

float Lpq_weight (int         start_frame,
                  int         stop_frame,
                  float         power_syllable,
                  float      power_time,
                  float        *frame_disturbance,
                  float        *time_weight) {

    double    result_time= 0;
    double  total_time_weight_time = 0;
    int        start_frame_of_syllable;
    
    for (start_frame_of_syllable = start_frame; 
         start_frame_of_syllable <= stop_frame; 
         start_frame_of_syllable += NUMBER_OF_PSQM_FRAMES_PER_SYLLABE/2) {

        double  result_syllable = 0;
        int     count_syllable = 0;
        int     frame;

        for (frame = start_frame_of_syllable;
             frame < start_frame_of_syllable + NUMBER_OF_PSQM_FRAMES_PER_SYLLABE;
             frame++) {
            if (frame <= stop_frame) {
                float h = frame_disturbance [frame];
                result_syllable +=  pow (h, power_syllable); 
            }
            count_syllable++;                
        }

        result_syllable /= count_syllable;
        result_syllable = pow (result_syllable, (double) 1/power_syllable);        
    
        result_time+=  pow (time_weight [start_frame_of_syllable - start_frame] * result_syllable, power_time); 
        total_time_weight_time += pow (time_weight [start_frame_of_syllable - start_frame], power_time);
    }

    result_time /= total_time_weight_time;
    result_time= pow (result_time, (float) 1 / power_time);

    return (float) result_time;
}

void set_to_sine (SIGNAL_INFO *info, float amplitude, float omega) {
    long i;

    for (i = 0; i < info-> Nsamples; i++) {
        info-> data [i] = amplitude * (float) sin (omega * i);
    }
}

float maximum_of (float *x, long start, long stop) {
    long i;
    float result = -1E20f;

    for (i = start; i < stop; i++) {
        if (result < x [i]) {
            result = x [i];
        }
    }

    return result;
}

float integral_of (float *x, long frames_after_start) {
    double result = 0;
    int    band;

    for (band = 1; band < Nb; band++) {
        result += x [frames_after_start * Nb + band] * width_of_band_bark [band];        
    }
    return (float) result;    


    return (float) result;
}

#define DEBUG_FR    0

void pesq_psychoacoustic_model(SIGNAL_INFO * ref_info, 
                               SIGNAL_INFO * deg_info,
                               ERROR_INFO  * err_info,
                               long        * Error_Flag,
                               char       ** Error_Type,
                               float       * ftmp)
{

    long    maxNsamples = max (ref_info-> Nsamples, deg_info-> Nsamples);
    long    Nf = Downsample * 8L;
    long    start_frame, stop_frame;
    long    samples_to_skip_at_start, samples_to_skip_at_end;
    float   sum_of_5_samples;
    long    n, i;
    float   power_ref, power_deg;
    long    frame;
    float   *fft_tmp;
    float    *hz_spectrum_ref, *hz_spectrum_deg;
    float   *pitch_pow_dens_ref, *pitch_pow_dens_deg;
    float    *loudness_dens_ref, *loudness_dens_deg;
    float   *avg_pitch_pow_dens_ref, *avg_pitch_pow_dens_deg;
    float    *deadzone;
    float   *disturbance_dens, *disturbance_dens_asym_add;
    float     total_audible_pow_ref, total_audible_pow_deg;
    int        *silent;
    float    oldScale, scale;
    int     *frame_was_skipped;
    float   *frame_disturbance;
    float   *frame_disturbance_asym_add;
    float   *total_power_ref;
    int         utt;
    
#ifdef CALIBRATE
    int     periodInSamples;
    int     numberOfPeriodsPerFrame;
    float   omega; 
#endif

    float   peak;

#define    MAX_NUMBER_OF_BAD_INTERVALS        1000

    int        *frame_is_bad; 
    int        *smeared_frame_is_bad; 
    int         start_frame_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         stop_frame_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         start_sample_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         stop_sample_of_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];   
    int         number_of_samples_in_bad_interval [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         delay_in_samples_in_bad_interval  [MAX_NUMBER_OF_BAD_INTERVALS];    
    int         number_of_bad_intervals= 0;
    int         search_range_in_samples;
    int         bad_interval;
    float *untweaked_deg = NULL;
    float *tweaked_deg = NULL;
    float *doubly_tweaked_deg = NULL;
    int         there_is_a_bad_frame = FALSE;
    float    *time_weight;
    float    d_indicator, a_indicator;
    int      nn;

    float Whanning [Nfmax];

    for (n = 0L; n < Nf; n++ ) {
        Whanning [n] = (float)(0.5 * (1.0 - cos((TWOPI * n) / Nf)));
    }

    switch (Fs) {
    case 8000:
        Nb = 42;
        Sl = (float) Sl_8k;
        Sp = (float) Sp_8k;
        nr_of_hz_bands_per_bark_band = nr_of_hz_bands_per_bark_band_8k;
        centre_of_band_bark = centre_of_band_bark_8k;
        centre_of_band_hz = centre_of_band_hz_8k;
        width_of_band_bark = width_of_band_bark_8k;
        width_of_band_hz = width_of_band_hz_8k;
        pow_dens_correction_factor = pow_dens_correction_factor_8k;
        abs_thresh_power = abs_thresh_power_8k;
        break;
    case 16000:
        Nb = 49;
        Sl = (float) Sl_16k;
        Sp = (float) Sp_16k;
        nr_of_hz_bands_per_bark_band = nr_of_hz_bands_per_bark_band_16k;
        centre_of_band_bark = centre_of_band_bark_16k;
        centre_of_band_hz = centre_of_band_hz_16k;
        width_of_band_bark = width_of_band_bark_16k;
        width_of_band_hz = width_of_band_hz_16k;
        pow_dens_correction_factor = pow_dens_correction_factor_16k;
        abs_thresh_power = abs_thresh_power_16k;
        break;
    default:
        *Error_Flag = PESQ_ERROR_INVALID_SAMPLE_RATE;
        *Error_Type = "Invalid sample frequency!\n";
        return;
    }

    samples_to_skip_at_start = 0;
    do {
        sum_of_5_samples= (float) 0;
        for (i = 0; i < 5; i++) {
            sum_of_5_samples += (float) fabs (ref_info-> data [SEARCHBUFFER * Downsample + samples_to_skip_at_start + i]);
        }
        if (sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) {
            samples_to_skip_at_start++;         
        }        
    } while ((sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) 
            && (samples_to_skip_at_start < maxNsamples / 2));
    
    samples_to_skip_at_end = 0;
    do {
        sum_of_5_samples= (float) 0;
        for (i = 0; i < 5; i++) {
            sum_of_5_samples += (float) fabs (ref_info-> data [maxNsamples - SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000) - 1 - samples_to_skip_at_end - i]);
        }
        if (sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) {
            samples_to_skip_at_end++;         
        }        
    } while ((sum_of_5_samples< CRITERIUM_FOR_SILENCE_OF_5_SAMPLES) 
        && (samples_to_skip_at_end < maxNsamples / 2));
       
    start_frame = samples_to_skip_at_start / (Nf /2);
    stop_frame = (maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000) - samples_to_skip_at_end) / (Nf /2) - 1; 

    power_ref = (float) pow_of (ref_info-> data, 
                                SEARCHBUFFER * Downsample, 
                                maxNsamples - SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000),
                                maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000)); 
    power_deg = (float) pow_of (deg_info-> data, 
                                SEARCHBUFFER * Downsample, 
                                maxNsamples - SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000),
                                maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000));

    fft_tmp                = (float *) safe_malloc ((Nf + 2) * sizeof (float));
    hz_spectrum_ref        = (float *) safe_malloc ((Nf / 2) * sizeof (float));
    hz_spectrum_deg        = (float *) safe_malloc ((Nf / 2) * sizeof (float));
    
    frame_is_bad        = (int *) safe_malloc ((stop_frame + 1) * sizeof (int)); 
    smeared_frame_is_bad=(int *) safe_malloc ((stop_frame + 1) * sizeof (int)); 
    
    silent                = (int *) safe_malloc ((stop_frame + 1) * sizeof (int));
    
    pitch_pow_dens_ref    = (float *) safe_malloc ((stop_frame + 1) * Nb * sizeof (float));
    pitch_pow_dens_deg    = (float *) safe_malloc ((stop_frame + 1) * Nb * sizeof (float));
    
    frame_was_skipped    = (int *) safe_malloc ((stop_frame + 1) * sizeof (int));

    frame_disturbance    = (float *) safe_malloc ((stop_frame + 1) * sizeof (float));
    frame_disturbance_asym_add    = (float *) safe_malloc ((stop_frame + 1) * sizeof (float));

    avg_pitch_pow_dens_ref = (float *) safe_malloc (Nb * sizeof (float));
    avg_pitch_pow_dens_deg = (float *) safe_malloc (Nb * sizeof (float));
    loudness_dens_ref    = (float *) safe_malloc (Nb * sizeof (float));
    loudness_dens_deg    = (float *) safe_malloc (Nb * sizeof (float));;
    deadzone                = (float *) safe_malloc (Nb * sizeof (float));;
    disturbance_dens    = (float *) safe_malloc (Nb * sizeof (float));
    disturbance_dens_asym_add = (float *) safe_malloc (Nb * sizeof (float));    

    time_weight            = (float *) safe_malloc ((stop_frame + 1) * sizeof (float));
    total_power_ref     = (float *) safe_malloc ((stop_frame + 1) * sizeof (float));
        
#ifdef CALIBRATE
    periodInSamples = Fs / 1000;
    numberOfPeriodsPerFrame = Nf / periodInSamples;
    omega = (float) (TWOPI / periodInSamples);    
    peak;

    set_to_sine (ref_info, (float) 29.54, (float) omega);    
#endif

    for (frame = 0; frame <= stop_frame; frame++) {
        int start_sample_ref = SEARCHBUFFER * Downsample + frame * Nf / 2;
        int start_sample_deg;
        int delay;    

        short_term_fft (Nf, ref_info, Whanning, start_sample_ref, hz_spectrum_ref, fft_tmp);
        
        if (err_info-> Nutterances < 1) {
            *Error_Flag = PESQ_ERROR_NO_UTTERANCES_DETECTED;
            *Error_Type = "No utterances!\n";
            return;
        }

        utt = err_info-> Nutterances - 1;
        while ((utt >= 0) && (err_info-> Utt_Start [utt] * Downsample > start_sample_ref)) {
            utt--;
        }
        if (utt >= 0) {
            delay = err_info-> Utt_Delay [utt];
        } else {
            delay = err_info-> Utt_Delay [0];        
        }
        start_sample_deg = start_sample_ref + delay;         

        if ((start_sample_deg > 0) && (start_sample_deg + Nf < maxNsamples + DATAPADDING_MSECS  * (Fs / 1000))) {
            short_term_fft (Nf, deg_info, Whanning, start_sample_deg, hz_spectrum_deg, fft_tmp);            
        } else {
            for (i = 0; i < Nf / 2; i++) {
                hz_spectrum_deg [i] = 0;
            }
        }

        freq_warping (Nf / 2, hz_spectrum_ref, Nb, pitch_pow_dens_ref, frame);

        peak = maximum_of (pitch_pow_dens_ref, 0, Nb);    

        freq_warping (Nf / 2, hz_spectrum_deg, Nb, pitch_pow_dens_deg, frame);
    
        total_audible_pow_ref = total_audible (frame, pitch_pow_dens_ref, 1E2);
        total_audible_pow_deg = total_audible (frame, pitch_pow_dens_deg, 1E2);        

        silent [frame] = (total_audible_pow_ref < 1E7);     
    }    

    time_avg_audible_of (stop_frame + 1, silent, pitch_pow_dens_ref, avg_pitch_pow_dens_ref, (maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000)) / (Nf / 2) - 1);
    time_avg_audible_of (stop_frame + 1, silent, pitch_pow_dens_deg, avg_pitch_pow_dens_deg, (maxNsamples - 2 * SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000)) / (Nf / 2) - 1);
    
#ifndef CALIBRATE
    freq_resp_compensation (stop_frame + 1, pitch_pow_dens_ref, avg_pitch_pow_dens_ref, avg_pitch_pow_dens_deg, 1000);
#endif
    
    oldScale = 1;
    for (frame = 0; frame <= stop_frame; frame++) {
        int band;

        total_audible_pow_ref = total_audible (frame, pitch_pow_dens_ref, 1);
        total_audible_pow_deg = total_audible (frame, pitch_pow_dens_deg, 1);        
        total_power_ref [frame] = total_audible_pow_ref;

        scale = (total_audible_pow_ref + (float) 5E3) / (total_audible_pow_deg + (float) 5E3);
                
        if (frame > 0) {
            scale = (float) 0.2 * oldScale + (float) 0.8*scale;
        }
        oldScale = scale;

#define MAX_SCALE   5.0
    
        if (scale > (float) MAX_SCALE) scale = (float) MAX_SCALE;

#define MIN_SCALE   3E-4
    
        if (scale < (float) MIN_SCALE) {
            scale = (float) MIN_SCALE;            
        }

        for (band = 0; band < Nb; band++) {
            pitch_pow_dens_deg [frame * Nb + band] *= scale;
        }

        intensity_warping_of (loudness_dens_ref, frame, pitch_pow_dens_ref); 
        intensity_warping_of (loudness_dens_deg, frame, pitch_pow_dens_deg); 
    
        for (band = 0; band < Nb; band++) {
            disturbance_dens [band] = loudness_dens_deg [band] - loudness_dens_ref [band];
        }
        
        for (band = 0; band < Nb; band++) {
            deadzone [band] = min (loudness_dens_deg [band], loudness_dens_ref [band]);    
            deadzone [band] *= 0.25;
        }
        
        for (band = 0; band < Nb; band++) {
            float d = disturbance_dens [band];
            float m = deadzone [band];
                
            if (d > m) {
                disturbance_dens [band] -= m;
            } else {
                if (d < -m) {
                    disturbance_dens [band] += m;
                } else {
                    disturbance_dens [band] = 0;
                }
            }
        }    

        frame_disturbance [frame] = pseudo_Lp (Nb, disturbance_dens, D_POW_F);    

#define THRESHOLD_BAD_FRAMES   30

        if (frame_disturbance [frame] > THRESHOLD_BAD_FRAMES) 
        {
            there_is_a_bad_frame = TRUE;
        }

        multiply_with_asymmetry_factor (disturbance_dens, frame, pitch_pow_dens_ref, pitch_pow_dens_deg);
    
        frame_disturbance_asym_add [frame] = pseudo_Lp (Nb, disturbance_dens, A_POW_F);    
    }

    for (frame = 0; frame <= stop_frame; frame++) {
        frame_was_skipped [frame] = FALSE;
    }

    for (utt = 1; utt < err_info-> Nutterances; utt++) {
        int frame1 = (int) floor (((err_info-> Utt_Start [utt] - SEARCHBUFFER ) * Downsample + err_info-> Utt_Delay [utt]) / (Nf / 2));
        int j = (int) floor ((err_info-> Utt_End [utt-1] - SEARCHBUFFER) * Downsample + err_info-> Utt_Delay [utt-1]) / (Nf / 2);
        int delay_jump = err_info-> Utt_Delay [utt] - err_info-> Utt_Delay [utt-1];

        if (frame1 > j) {
            frame1 = j;
        }
        
        if (frame1 < 0) {
            frame1 = 0;
        }
            
        if (delay_jump < -(int) (Nf / 2)) {

            int frame2 = (int) ((err_info-> Utt_Start [utt] - SEARCHBUFFER) * Downsample + max (0, abs (delay_jump))) / (Nf / 2) + 1; 
            
            for (frame = frame1; frame <= frame2; frame++)  {
                if (frame < stop_frame) {
                    frame_was_skipped [frame] = TRUE;

                    frame_disturbance [frame] = 0;
                    frame_disturbance_asym_add [frame] = 0;
                }
            } 
        }    
    }
    
    nn = DATAPADDING_MSECS  * (Fs / 1000) + maxNsamples;

    tweaked_deg = (float *) safe_malloc (nn * sizeof (float));

    for (i = 0; i < nn; i++) {
        tweaked_deg [i] = 0;
    } 

    for (i = SEARCHBUFFER * Downsample; i < nn - SEARCHBUFFER * Downsample; i++) {
        int  utt = err_info-> Nutterances - 1;
        long delay, j;

        while ((utt >= 0) && (err_info-> Utt_Start [utt] * Downsample > i)) {
            utt--;
        }
        if (utt >= 0) {
            delay = err_info-> Utt_Delay [utt];
        } else {
            delay = err_info-> Utt_Delay [0];        
        }
            
        j = i + delay;
        if (j < SEARCHBUFFER * Downsample) {
            j = SEARCHBUFFER * Downsample;
        }
        if (j >= nn - SEARCHBUFFER * Downsample) {
            j = nn - SEARCHBUFFER * Downsample - 1;
        }
        tweaked_deg [i] = deg_info-> data [j];
    }

    if (there_is_a_bad_frame) {        
        
        for (frame = 0; frame <= stop_frame; frame++) 
        {  
            frame_is_bad [frame] = (frame_disturbance [frame] > THRESHOLD_BAD_FRAMES);       

            smeared_frame_is_bad [frame] = FALSE;
        }
        frame_is_bad [0] = FALSE;

    #define SMEAR_RANGE 2
        
        for (frame = SMEAR_RANGE; frame < stop_frame - SMEAR_RANGE; frame++) {    
            long max_itself_and_left = frame_is_bad [frame];
            long max_itself_and_right = frame_is_bad [frame];
            long mini, i;

            for (i = -SMEAR_RANGE; i <= 0; i++) {
                if (max_itself_and_left < frame_is_bad [frame  + i]) {
                    max_itself_and_left = frame_is_bad [frame  + i];
                }
            }
        
            for (i = 0; i <= SMEAR_RANGE; i++) {
                if (max_itself_and_right < frame_is_bad [frame + i]) {
                    max_itself_and_right = frame_is_bad [frame + i];
                }
            }

            mini = max_itself_and_left;
            if (mini > max_itself_and_right) {
                mini = max_itself_and_right;
            }

            smeared_frame_is_bad [frame] = mini;
        }
   
#define MINIMUM_NUMBER_OF_BAD_FRAMES_IN_BAD_INTERVAL    5

        number_of_bad_intervals = 0;    
        frame = 0; 
        while (frame <= stop_frame) {

            while ((frame <= stop_frame) && (!smeared_frame_is_bad [frame])) {
                frame++; 
            }

            if (frame <= stop_frame) { 
                start_frame_of_bad_interval [number_of_bad_intervals] = frame;

                while ((frame <= stop_frame) && (smeared_frame_is_bad [frame])) {
                    frame++; 
                }
            
                if (frame <= stop_frame) {
                    stop_frame_of_bad_interval [number_of_bad_intervals] = frame; 

                    if (stop_frame_of_bad_interval [number_of_bad_intervals] - start_frame_of_bad_interval [number_of_bad_intervals] >= MINIMUM_NUMBER_OF_BAD_FRAMES_IN_BAD_INTERVAL) {
                        number_of_bad_intervals++; 
                    }
                }
            }
        }

        for (bad_interval = 0; bad_interval < number_of_bad_intervals; bad_interval++) {
            start_sample_of_bad_interval [bad_interval] =  start_frame_of_bad_interval [bad_interval] * (Nf / 2) + SEARCHBUFFER * Downsample;
            stop_sample_of_bad_interval [bad_interval] =  stop_frame_of_bad_interval [bad_interval] * (Nf / 2) + Nf + SEARCHBUFFER* Downsample;
            if (stop_frame_of_bad_interval [bad_interval] > stop_frame) {
                stop_frame_of_bad_interval [bad_interval] = stop_frame; 
            }

            number_of_samples_in_bad_interval [bad_interval] =  stop_sample_of_bad_interval [bad_interval] - start_sample_of_bad_interval [bad_interval];
        }

        

    #define SEARCH_RANGE_IN_TRANSFORM_LENGTH    4

        search_range_in_samples= SEARCH_RANGE_IN_TRANSFORM_LENGTH * Nf;

        for (bad_interval= 0; bad_interval< number_of_bad_intervals; bad_interval++) {
            float  *ref = (float *) safe_malloc ( (2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval]) * sizeof (float));
            float  *deg = (float *) safe_malloc ( (2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval]) * sizeof (float));
            int        i;
            float    best_correlation;
            int        delay_in_samples;

            for (i = 0; i < search_range_in_samples; i++) {
                ref[i] = 0.0f;
            }
            for (i = 0; i < number_of_samples_in_bad_interval [bad_interval]; i++) {
                ref [search_range_in_samples + i] = ref_info-> data [start_sample_of_bad_interval [bad_interval] + i];
            }
            for (i = 0; i < search_range_in_samples; i++) {
                ref [search_range_in_samples + number_of_samples_in_bad_interval [bad_interval] + i] = 0.0f;
            }
        
            for (i = 0; 
                 i < 2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval];
                 i++) {
                
                int j = start_sample_of_bad_interval [bad_interval] - search_range_in_samples + i;
                int nn = maxNsamples - SEARCHBUFFER * Downsample + DATAPADDING_MSECS  * (Fs / 1000);

                if (j < SEARCHBUFFER * Downsample) {
                    j = SEARCHBUFFER * Downsample;
                }
                if (j >= nn) {
                    j = nn - 1;
                }
                deg [i] = tweaked_deg [j]; 
            }

            delay_in_samples= compute_delay (0, 
                                             2 * search_range_in_samples + number_of_samples_in_bad_interval [bad_interval], 
                                             search_range_in_samples,
                                             ref, 
                                             deg,
                                             &best_correlation);

            delay_in_samples_in_bad_interval [bad_interval] =  delay_in_samples;

            if (best_correlation < 0.5) {
                delay_in_samples_in_bad_interval  [bad_interval] = 0;
            } 

            safe_free (ref);
            safe_free (deg);
        }

        if (number_of_bad_intervals > 0) {
            doubly_tweaked_deg = (float *) safe_malloc ((maxNsamples + DATAPADDING_MSECS  * (Fs / 1000)) * sizeof (float));

            for (i = 0; i < maxNsamples + DATAPADDING_MSECS  * (Fs / 1000); i++) {
                doubly_tweaked_deg [i] = tweaked_deg [i];
            }
        
            for (bad_interval= 0; bad_interval< number_of_bad_intervals; bad_interval++) {
                int delay = delay_in_samples_in_bad_interval  [bad_interval];
                int i;
    
                for (i = start_sample_of_bad_interval [bad_interval]; i < stop_sample_of_bad_interval [bad_interval]; i++) {
                    float h;
                    int j = i + delay;
                    if (j < 0) {
                        j = 0;
                    }
                    if (j >= maxNsamples) {
                        j = maxNsamples - 1;
    
                    }
                    doubly_tweaked_deg [i] = h = tweaked_deg [j];        
                }
            }
    
            untweaked_deg = deg_info-> data;
            deg_info-> data = doubly_tweaked_deg;
    
            for (bad_interval= 0; bad_interval < number_of_bad_intervals; bad_interval++) {
    
                 for (frame = start_frame_of_bad_interval [bad_interval]; 
                     frame < stop_frame_of_bad_interval [bad_interval]; 
                     frame++) {
    
                     int start_sample_ref = SEARCHBUFFER * Downsample + frame * Nf / 2;
                    int start_sample_deg = start_sample_ref;
                    
                    short_term_fft (Nf, deg_info, Whanning, start_sample_deg, hz_spectrum_deg, fft_tmp);            
    
                    freq_warping (Nf / 2, hz_spectrum_deg, Nb, pitch_pow_dens_deg, frame);
                }    
    
                oldScale = 1;
                for (frame = start_frame_of_bad_interval [bad_interval]; 
                     frame < stop_frame_of_bad_interval [bad_interval]; 
                     frame++) {
                    int band;
    
                    total_audible_pow_ref = total_audible (frame, pitch_pow_dens_ref, 1);
                    total_audible_pow_deg = total_audible (frame, pitch_pow_dens_deg, 1);        
    
                    scale = (total_audible_pow_ref + (float) 5E3) / (total_audible_pow_deg + (float) 5E3);
                    
                    if (frame > 0) {
                        scale = (float) 0.2 * oldScale + (float) 0.8*scale;
                    }
                    oldScale = scale;
    
                    if (scale > (float) MAX_SCALE) scale = (float) MAX_SCALE;
    
                    if (scale < (float) MIN_SCALE) {
                        scale = (float) MIN_SCALE;            
                    }
    
                    for (band = 0; band < Nb; band++) {
                        pitch_pow_dens_deg [frame * Nb + band] *= scale;
                    }
    
                    intensity_warping_of (loudness_dens_ref, frame, pitch_pow_dens_ref); 
                    intensity_warping_of (loudness_dens_deg, frame, pitch_pow_dens_deg); 
        
                    for (band = 0; band < Nb; band++) {
                        disturbance_dens [band] = loudness_dens_deg [band] - loudness_dens_ref [band];
                    }
    
                    for (band = 0; band < Nb; band++) {
                        deadzone [band] = min (loudness_dens_deg [band], loudness_dens_ref [band]);    
                        deadzone [band] *= 0.25;
                    }
                    
                    for (band = 0; band < Nb; band++) {
                        float d = disturbance_dens [band];
                        float m = deadzone [band];
                    
                        if (d > m) {
                            disturbance_dens [band] -= m;
                        } else {
                            if (d < -m) {
                                disturbance_dens [band] += m;
                            } else {
                                disturbance_dens [band] = 0;
                            }
                        }
                    }    
    
                    frame_disturbance [frame] = min (frame_disturbance [frame] , pseudo_Lp (Nb, disturbance_dens, D_POW_F));    
    
                    multiply_with_asymmetry_factor (disturbance_dens, frame, pitch_pow_dens_ref, pitch_pow_dens_deg);
        
                    frame_disturbance_asym_add [frame] = min (frame_disturbance_asym_add [frame], pseudo_Lp (Nb, disturbance_dens, A_POW_F));    
                }
            }    
            safe_free (doubly_tweaked_deg);
            deg_info->data = untweaked_deg;
        }
    }
    

    for (frame = 0; frame <= stop_frame; frame++) {
        float h = 1;
        
        if (stop_frame + 1 > 1000) {
            long n = (maxNsamples - 2 * SEARCHBUFFER * Downsample) / (Nf / 2) - 1;
            double timeWeightFactor = (n - (float) 1000) / (float) 5500;
            if (timeWeightFactor > (float) 0.5) timeWeightFactor = (float) 0.5;
            h = (float) (((float) 1.0 - timeWeightFactor) + timeWeightFactor * (float) frame / (float) n);
        }

        time_weight [frame] = h;
    }

    for (frame = 0; frame <= stop_frame; frame++) {

        float h = (float) pow ((total_power_ref [frame] + 1E5) / 1E7, 0.04); 

        frame_disturbance [frame] /= h;
        frame_disturbance_asym_add [frame] /= h;

        if (frame_disturbance [frame] > 45) {
            frame_disturbance [frame] = 45;
        }
        if (frame_disturbance_asym_add [frame] > 45) {
            frame_disturbance_asym_add [frame] = 45;
        }            
    }
        
    d_indicator = Lpq_weight (start_frame, stop_frame, D_POW_S, D_POW_T, frame_disturbance, time_weight);    
    a_indicator = Lpq_weight (start_frame, stop_frame, A_POW_S, A_POW_T, frame_disturbance_asym_add, time_weight);       
    
    err_info-> pesq_mos = (float) (4.5 - D_WEIGHT * d_indicator - A_WEIGHT * a_indicator); 

    FFTFree();
    safe_free (fft_tmp);
    safe_free (hz_spectrum_ref);
    safe_free (hz_spectrum_deg);
    safe_free (silent);
    safe_free (pitch_pow_dens_ref);
    safe_free (pitch_pow_dens_deg);
    safe_free (frame_was_skipped);
    safe_free (avg_pitch_pow_dens_ref);
    safe_free (avg_pitch_pow_dens_deg);
    safe_free (loudness_dens_ref);
    safe_free (loudness_dens_deg);
    safe_free (deadzone);
    safe_free (disturbance_dens);
    safe_free (disturbance_dens_asym_add);
    safe_free (total_power_ref);

    safe_free (frame_is_bad);
    safe_free (smeared_frame_is_bad);
    
    safe_free (time_weight);
    safe_free (frame_disturbance);
    safe_free (frame_disturbance_asym_add);
    safe_free (tweaked_deg);

    return;
}

/* END OF FILE */
