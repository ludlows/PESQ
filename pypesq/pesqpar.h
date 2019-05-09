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

long Fs = 0L;

long Fs_16k = 16000L;

long Fs_8k = 8000L;

long Downsample;

long Downsample_16k = 64;

long Downsample_8k = 32;

long Align_Nfft;

long Align_Nfft_16k = 1024;

long Align_Nfft_8k = 512;

long InIIR_Nsos;

long InIIR_Nsos_8k = 8L;
float InIIR_Hsos_8k[LINIIR] =
 { 0.885535424f,        -0.885535424f,  0.000000000f,   -0.771070709f,  0.000000000f,
    0.895092588f,       1.292907193f,   0.449260174f,   1.268869037f,   0.442025372f,
    4.049527940f,       -7.865190042f,  3.815662102f,   -1.746859852f,  0.786305963f,
    0.500002353f,       -0.500002353f,  0.000000000f,   0.000000000f,   0.000000000f,
    0.565002834f,       -0.241585934f,  -0.306009671f,  0.259688659f,   0.249979657f,
    2.115237288f,       0.919935084f,   1.141240051f,   -1.587313419f,  0.665935315f,
    0.912224584f,       -0.224397719f,  -0.641121413f,  -0.246029464f,  -0.556720590f,
    0.444617727f,       -0.307589321f,  0.141638062f,   -0.996391149f,  0.502251622f };

long InIIR_Nsos_16k = 12L;
float InIIR_Hsos_16k[LINIIR] =
 { 0.325631521f,        -0.086782860f,  -0.238848661f,  -1.079416490f,  0.434583902f,
   0.403961804f,        -0.556985881f,  0.153024077f,   -0.415115835f,  0.696590244f,
   4.736162769f,        3.287251046f,   1.753289019f,   -1.859599046f,  0.876284034f,
   0.365373469f,        0.000000000f,   0.000000000f,   -0.634626531f,  0.000000000f,
   0.884811506f,        0.000000000f,   0.000000000f,   -0.256725271f,  0.141536777f,
   0.723593055f,        -1.447186099f,  0.723593044f,   -1.129587469f,  0.657232737f,
   1.644910855f,        -1.817280902f,  1.249658063f,   -1.778403899f,  0.801724355f,
   0.633692689f,        -0.284644314f,  -0.319789663f,  0.000000000f,   0.000000000f,
   1.032763031f,        0.268428979f,   0.602913323f,   0.000000000f,   0.000000000f,
   1.001616361f,        -0.823749013f,  0.439731942f,   -0.885778255f,  0.000000000f,
   0.752472096f,        -0.375388990f,  0.188977609f,   -0.077258216f,  0.247230734f,
   1.023700575f,        0.001661628f,   0.521284240f,   -0.183867259f,  0.354324187f };


int nr_of_hz_bands_per_bark_band_8k [42] = { 1,    1,    1,    1,    1,    
                                                1,    1,    1,    2,    1,    
                                                1,    1,    1,    1,    2,    
                                                1,    1,    2,    2,    2,    
                                                2,    2,    2,    2,    2,    
                                                3,    3,    3,    3,    4,    
                                                3,    4,    5,    4,    5,    
                                                6,    6,    7,    8,    9,    
                                                9,    11,    };

double centre_of_band_bark_8k [42] = { 0.078672,     0.316341,     0.636559,     0.961246,     1.290450,     
                                        1.624217,     1.962597,     2.305636,     2.653383,     3.005889,     
                                        3.363201,     3.725371,     4.092449,     4.464486,     4.841533,     
                                        5.223642,     5.610866,     6.003256,     6.400869,     6.803755,     
                                        7.211971,     7.625571,     8.044611,     8.469146,     8.899232,     
                                        9.334927,     9.776288,     10.223374,     10.676242,     11.134952,     
                                        11.599563,     12.070135,     12.546731,     13.029408,     13.518232,     
                                        14.013264,     14.514566,     15.022202,     15.536238,     16.056736,     
                                        16.583761,     17.117382};

double centre_of_band_hz_8k [42] = { 7.867213,     31.634144,     63.655895,     96.124611,     129.044968,     
                                    162.421738,     196.259659,     230.563568,     265.338348,     300.588867,     
                                    336.320129,     372.537140,     409.244934,     446.448578,     484.568604,     
                                    526.600586,     570.303833,     619.423340,     672.121643,     728.525696,     
                                    785.675964,     846.835693,     909.691650,     977.063293,     1049.861694,     
                                    1129.635986,     1217.257568,     1312.109497,     1412.501465,     1517.999390,     
                                    1628.894165,     1746.194336,     1871.568848,     2008.776123,     2158.979248,     
                                    2326.743164,     2513.787109,     2722.488770,     2952.586670,     3205.835449,     
                                    3492.679932,     3820.219238};

double width_of_band_bark_8k [42] = { 0.157344,     0.317994,     0.322441,     0.326934,     0.331474,     
                                        0.336061,     0.340697,     0.345381,     0.350114,     0.354897,     
                                        0.359729,     0.364611,     0.369544,     0.374529,     0.379565,     
                                        0.384653,     0.389794,     0.394989,     0.400236,     0.405538,     
                                        0.410894,     0.416306,     0.421773,     0.427297,     0.432877,     
                                        0.438514,     0.444209,     0.449962,     0.455774,     0.461645,     
                                        0.467577,     0.473569,     0.479621,     0.485736,     0.491912,     
                                        0.498151,     0.504454,     0.510819,     0.517250,     0.523745,     
                                        0.530308,     0.536934};

double width_of_band_hz_8k [42] = { 15.734426,     31.799433,     32.244064,     32.693359,     33.147385,     
                                    33.606140,     34.069702,     34.538116,     35.011429,     35.489655,     
                                    35.972870,     36.461121,     36.954407,     37.452911,     40.269653,     
                                    42.311859,     45.992554,     51.348511,     55.040527,     56.775208,     
                                    58.699402,     62.445862,     64.820923,     69.195374,     76.745667,     
                                    84.016235,     90.825684,     97.931152,     103.348877,     107.801880,     
                                    113.552246,     121.490601,     130.420410,     143.431763,     158.486816,     
                                    176.872803,     198.314697,     219.549561,     240.600098,     268.702393,     
                                    306.060059,     349.937012};

double pow_dens_correction_factor_8k [42] = { 100.000000,     99.999992,     100.000000,     100.000008,     100.000008, 
                                                100.000015,     99.999992,     99.999969,     50.000027,     100.000000,     
                                                99.999969,     100.000015,     99.999947,     100.000061,     53.047077,     
                                                110.000046,     117.991989,     65.000000,     68.760147,     69.999931,     
                                                71.428818,     75.000038,     76.843384,     80.968781,     88.646126,     
                                                63.864388,     68.155350,     72.547775,     75.584831,     58.379192,     
                                                80.950836,     64.135651,     54.384785,     73.821884,     64.437073,     
                                                59.176456,     65.521278,     61.399822,     58.144047,     57.004543,     
                                                64.126297,     59.248363};

double abs_thresh_power_8k [42] = {51286152.000000,     2454709.500000,     70794.593750,     4897.788574,     1174.897705,     
                                    389.045166,     104.712860,     45.708820,     17.782795,     9.772372,     
                                    4.897789,     3.090296,     1.905461,     1.258925,     0.977237,     
                                    0.724436,     0.562341,     0.457088,     0.389045,     0.331131,     
                                    0.295121,     0.269153,     0.257040,     0.251189,     0.251189,     
                                    0.251189,     0.251189,     0.263027,     0.288403,     0.309030,     
                                    0.338844,     0.371535,     0.398107,     0.436516,     0.467735,     
                                    0.489779,     0.501187,     0.501187,     0.512861,     0.524807,     
                                    0.524807,     0.524807};

int nr_of_hz_bands_per_bark_band_16k [49] = { 1,    1,    1,    1,    1,    
                                                1,    1,    1,    2,    1,    
                                                1,    1,    1,    1,    2,    
                                                1,    1,    2,    2,    2,    
                                                2,    2,    2,    2,    2,    
                                                3,    3,    3,    3,    4,    
                                                3,    4,    5,    4,    5,    
                                                6,    6,    7,    8,    9,    
                                                9,    12,    12,    15,    16,    
                                                18,    21,    25,    20};

double centre_of_band_bark_16k [49] = { 0.078672,     0.316341,     0.636559,     0.961246,     1.290450,     
                                        1.624217,     1.962597,     2.305636,     2.653383,     3.005889,     
                                        3.363201,     3.725371,     4.092449,     4.464486,     4.841533,     
                                        5.223642,     5.610866,     6.003256,     6.400869,     6.803755,     
                                        7.211971,     7.625571,     8.044611,     8.469146,     8.899232,     
                                        9.334927,     9.776288,     10.223374,     10.676242,     11.134952,     
                                        11.599563,     12.070135,     12.546731,     13.029408,     13.518232,     
                                        14.013264,     14.514566,     15.022202,     15.536238,     16.056736,     
                                        16.583761,     17.117382,     17.657663,     18.204674,     18.758478,     
                                        19.319147,     19.886751,     20.461355,     21.043034};

double centre_of_band_hz_16k [49] = { 7.867213,     31.634144,     63.655895,     96.124611,     129.044968,     
                                        162.421738,     196.259659,     230.563568,     265.338348,     300.588867,     
                                        336.320129,     372.537140,     409.244934,     446.448578,     484.568604,     
                                        526.600586,     570.303833,     619.423340,     672.121643,     728.525696,     
                                        785.675964,     846.835693,     909.691650,     977.063293,     1049.861694,     
                                        1129.635986,     1217.257568,     1312.109497,     1412.501465,     1517.999390,     
                                        1628.894165,     1746.194336,     1871.568848,     2008.776123,     2158.979248,     
                                        2326.743164,     2513.787109,     2722.488770,     2952.586670,     3205.835449,     
                                        3492.679932,     3820.219238,     4193.938477,     4619.846191,     5100.437012,     
                                        5636.199219,     6234.313477,     6946.734863,     7796.473633};

double width_of_band_bark_16k [49] = { 0.157344,     0.317994,     0.322441,     0.326934,     0.331474,     
                                        0.336061,     0.340697,     0.345381,     0.350114,     0.354897,     
                                        0.359729,     0.364611,     0.369544,     0.374529,     0.379565,     
                                        0.384653,     0.389794,     0.394989,     0.400236,     0.405538,     
                                        0.410894,     0.416306,     0.421773,     0.427297,     0.432877,     
                                        0.438514,     0.444209,     0.449962,     0.455774,     0.461645,     
                                        0.467577,     0.473569,     0.479621,     0.485736,     0.491912,     
                                        0.498151,     0.504454,     0.510819,     0.517250,     0.523745,     
                                        0.530308,     0.536934,     0.543629,     0.550390,     0.557220,     
                                        0.564119,     0.571085,     0.578125,     0.585232};

double width_of_band_hz_16k [49] = { 15.734426,     31.799433,     32.244064,     32.693359,     33.147385,     
                                    33.606140,     34.069702,     34.538116,     35.011429,     35.489655,     
                                    35.972870,     36.461121,     36.954407,     37.452911,     40.269653,     
                                    42.311859,     45.992554,     51.348511,     55.040527,     56.775208,     
                                    58.699402,     62.445862,     64.820923,     69.195374,     76.745667,     
                                    84.016235,     90.825684,     97.931152,     103.348877,     107.801880,     
                                    113.552246,     121.490601,     130.420410,     143.431763,     158.486816,     
                                    176.872803,     198.314697,     219.549561,     240.600098,     268.702393,     
                                    306.060059,     349.937012,     398.686279,     454.713867,     506.841797,     
                                    564.863770,     637.261230,     794.717285,     931.068359};

double pow_dens_correction_factor_16k [49] = { 100.000000,     99.999992,     100.000000,     100.000008,     100.000008,     
                                                100.000015,     99.999992,     99.999969,     50.000027,     100.000000,     
                                                99.999969,     100.000015,     99.999947,     100.000061,     53.047077,     
                                                110.000046,     117.991989,     65.000000,     68.760147,     69.999931,     
                                                71.428818,     75.000038,     76.843384,     80.968781,     88.646126,     
                                                63.864388,     68.155350,     72.547775,     75.584831,     58.379192,     
                                                80.950836,     64.135651,     54.384785,     73.821884,     64.437073,     
                                                59.176456,     65.521278,     61.399822,     58.144047,     57.004543,     
                                                64.126297,     54.311001,     61.114979,     55.077751,     56.849335,     
                                                55.628868,     53.137054,     54.985844,     79.546974};
double abs_thresh_power_16k [49] = {51286152.000000,     2454709.500000,     70794.593750,     4897.788574,     1174.897705,     
                                    389.045166,     104.712860,     45.708820,     17.782795,     9.772372,     
                                    4.897789,     3.090296,     1.905461,     1.258925,     0.977237,     
                                    0.724436,     0.562341,     0.457088,     0.389045,     0.331131,     
                                    0.295121,     0.269153,     0.257040,     0.251189,     0.251189,     
                                    0.251189,     0.251189,     0.263027,     0.288403,     0.309030,     
                                    0.338844,     0.371535,     0.398107,     0.436516,     0.467735,     
                                    0.489779,     0.501187,     0.501187,     0.512861,     0.524807,     
                                    0.524807,     0.524807,     0.512861,     0.478630,     0.426580,     
                                    0.371535,     0.363078,     0.416869,     0.537032};


/* END OF FILE */
