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

#ifndef min
  #define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
  #define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef DSP_INCLUDED
  #define DSP_INCLUDED
   void *safe_malloc (unsigned long);
   void safe_free (void *);

  void IIRFilt(
    float * h, unsigned long Nsos, float * z,
    float * x, unsigned long Nx, float * y );

  unsigned long nextpow2(unsigned long X);
  int ispow2(unsigned long X);
  int intlog2(unsigned long X);
  void FFTInit(unsigned long N);
  void FFTFree(void);
  void RealFFT(float * x, unsigned long N);
  void RealIFFT(float * x, unsigned long N);
  unsigned long FFTNXCorr(
    float * x1, unsigned long n1, float * x2, unsigned long n2, float * y );
  void IIRsos(
    float * x, unsigned long Nx,
    float b0, float b1, float b2, float a1, float a2,
    float * tz1, float * tz2 );
  void IIRFilt(
    float * h, unsigned long Nsos, float * z,
    float * x, unsigned long Nx, float * y );
#endif

/* END OF FILE */
