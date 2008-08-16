/** 
 * @file llfft.cpp
 * @brief FFT function implementations
 *
 * Copyright (c) 2003-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

/*
 *  Fast Fourier Transform
 *
 */
#include "llviewerprecompiledheaders.h"

#include "llfft.h"
#include "llerror.h"

/*
**
*********************************************************************
** Forward and inverse discrete Fourier transforms on complex data **
*********************************************************************
**
**
** forward_fft(array, rows, cols)
**               COMPLEX *array;
**               S32 rows, cols;
**
** inverse_fft(array, rows, cols)
**               COMPLEX *array;
**               S32 rows, cols;
**
** These entry points compute the forward and inverse DFT's, respectively, 
** of a single-precision COMPLEX array.
**
** The result is a COMPLEX array of the same size, returned in 
** the same space as the input array.  That is, the original array is
** overwritten and destroyed.
**
** Rows and columns must each be an integral power of 2.
**
** These routines return integer value -1 if an error was detected,
** 0 otherwise
**
** This implementation of the DFT uses the transform pair defined as follows.
**
** Let there be two COMPLEX arrays each with n rows and m columns
** Index them as 
** f(x,y):    0 <= x <= m - 1,  0 <= y <= n - 1
** F(u,v):    -m/2 <= u <= m/2 - 1,  -n/2 <= v <= n/2 - 1
**
** Then the forward and inverse transforms are related as
**
** Forward:
**
** F(u,v) = \sum_{x=0}^{m-1} \sum_{y=0}^{n-1} 
**                      f(x,y) \exp{-2\pi i (ux/m + vy/n)}
**
**
** Inverse:
**
** f(x,y) = 1/(mn) \sum_{u=-m/2}^{m/2-1} \sum_{v=-n/2}^{n/2-1} 
**                      F(u,v) \exp{2\pi i (ux/m + vy/n)}
**  
** Therefore, the transforms have these properties:
** 1.  \sum_x \sum_y  f(x,y) = F(0,0)
** 2.  m n \sum_x \sum_y |f(x,y)|^2 = \sum_u \sum_v |F(u,v)|^2
**
*/


//DPCOMPLEX 	*stageBuff;  /* buffer to hold a row or column at a time */
//COMPLEX 	*bigBuff;    /* a pointer to the input array */


/*
 *	These macros move complex data between bigBuff and
 *	stageBuff
 */

inline void LoadRow(DPCOMPLEX* stageBuff, COMPLEX* bigBuff, U32 row, U32 cols)
{
    for (U32 j = row*cols, k = 0 ; k < cols ; j++, k++)
	{
		stageBuff[k].re = bigBuff[j].re;
		stageBuff[k].im = bigBuff[j].im;
	}
}

inline void StoreRow(DPCOMPLEX* stageBuff, COMPLEX* bigBuff, U32 row, U32 cols)
{
    for (U32 j = row*cols, k = 0 ; k < cols ; j++, k++)
	{
		bigBuff[j].re = (F32)stageBuff[k].re;
		bigBuff[j].im = (F32)stageBuff[k].im;
	}
}

inline void LoadCol(DPCOMPLEX* stageBuff, COMPLEX* bigBuff, U32 col, U32 rows, U32 cols)
{
    for (U32 j = col,k = 0 ; k < rows ; j+=cols, k++)
	{
		stageBuff[k].re = bigBuff[j].re;
		stageBuff[k].im = bigBuff[j].im;
	}
}

inline void StoreCol(DPCOMPLEX* stageBuff, COMPLEX* bigBuff, U32 col, U32 rows, U32 cols)
{
    for (U32 j = col,k = 0 ; k < rows ; j+=cols, k++)
	{
		bigBuff[j].re = (F32)stageBuff[k].re;
		bigBuff[j].im = (F32)stageBuff[k].im;
	}
}


/* do something with an error message */
inline void handle_error(S8* msg)
{
	llerrs << msg << llendl;
}


/* 
** compute DFT: forward if direction==0, inverse if direction==1
** array must be COMPLEX 
*/ 
BOOL fft(const LLFFTPlan& plan, COMPLEX *array, S32 rows, S32 cols, S32 direction)
{
	S32 i;
	
	if (!plan.valid() || plan.rows() != rows || plan.cols() != cols)
		return FALSE;
	
	/* compute transform row by row */
	
	if(cols>1)
	{
		for(i=0;i<rows;i++) 
		{
			LoadRow(plan.buffer(), array, i, cols);
			FFT842(direction, cols, plan.buffer());
			StoreRow(plan.buffer(), array, i, cols);
		}
	}
	
	if(rows<2) /* done */
	{
		//freeBuffer();
		return TRUE;
	}
	
	
	/* compute transform column by column */
	
	for(i=0;i<cols;i++) 
	{
		LoadCol(plan.buffer(), array, i, rows, cols);
		FFT842(direction, rows, plan.buffer());
		StoreCol(plan.buffer(), array, i, rows, cols);
	}
	
	//freeBuffer();
	
	return TRUE;
}



/*
** FFT842
** This routine replaces the input DPCOMPLEX vector by its
** finite discrete complex fourier transform if in==0.
** It replaces the input DPCOMPLEX vector by its
** finite discrete complex inverse fourier transform if in==1.
**
**     in - FORWARD or INVERSE
**     n - length of vector
**     b - input vector
**
** It performs as many base 8 iterations as possible and
** then finishes with a base 4 iteration or a base 2 
** iteration if needed.
**
** Ported from the FORTRAN code in Programming for Digital Signal Processing,
** IEEE Press 1979, Section 1, by G. D. Bergland and M. T. Dolan
**
*/
void FFT842(S32 in, S32 n, DPCOMPLEX *b)
{
	F64 fn, r, fi;
	
	S32 L[16],L1,L2,L3,L4,L5,L6,L7,L8,L9,L10,L11,L12,L13,L14,L15;
	S32 j1,j2,j3,j4,j5,j6,j7,j8,j9,j10,j11,j12,j13,j14;
	S32 i, j, ij, ji, ij1, ji1;
	S32 n2pow, n8pow, nthpo, ipass, nxtlt, lengt;
	
	n2pow = fastlog2(n);
	nthpo = n;
	fn = 1.0 / (F64)nthpo;
	
	
	if(in==FORWARD)
	{
		/* take conjugate  */
		for(i=0;i<n;i++)
		{
			b[i].im *= -1.0;
		}
	}
	
	if(in==INVERSE)
	{
		/* scramble inputs */
		for(i=0,j=n/2;j<n;i++,j++) 
		{
			r  = b[j].re;
			fi = b[j].im;
			b[j].re = b[i].re;
			b[j].im = b[i].im;
			b[i].re = r;
			b[i].im = fi;
		}
	}
	
	n8pow = n2pow/3;
	
	if(n8pow) 
    {
		/* radix 8 iterations */
		for(ipass=1;ipass<=n8pow;ipass++) 
		{
			nxtlt = 0x1 << (n2pow - 3*ipass);
			lengt = 8*nxtlt;
			R8TX(nxtlt,nthpo,lengt,
				b,b+nxtlt,b+2*nxtlt,
				b+3*nxtlt,b+4*nxtlt,b+5*nxtlt,
				b+6*nxtlt,b+7*nxtlt);
		}
    }
	
	if(n2pow%3 == 1) 
    {
		/* radix 2 iteration needed */
		R2TX(nthpo,b,b+1); 
    }
	
	
	if(n2pow%3 == 2)  
    {
		/* radix 4 iteration needed */
		R4TX(nthpo,b,b+1,b+2,b+3); 
    }
	
	
	
	for(j=1;j<=15;j++) 
    {
		L[j] = 1;
		if(j-n2pow <= 0) L[j] = 0x1 << (n2pow + 1 - j);
    }
	L15=L[1];L14=L[2];L13=L[3];L12=L[4];L11=L[5];L10=L[6];L9=L[7];
	L8=L[8];L7=L[9];L6=L[10];L5=L[11];L4=L[12];L3=L[13];L2=L[14];L1=L[15];
	
	ij = 1;
    
	for(j1=1;j1<=L1;j1++)
		for(j2=j1;j2<=L2;j2+=L1)
			for(j3=j2;j3<=L3;j3+=L2)
				for(j4=j3;j4<=L4;j4+=L3)
					for(j5=j4;j5<=L5;j5+=L4)
						for(j6=j5;j6<=L6;j6+=L5)
							for(j7=j6;j7<=L7;j7+=L6)
								for(j8=j7;j8<=L8;j8+=L7)
									for(j9=j8;j9<=L9;j9+=L8)
										for(j10=j9;j10<=L10;j10+=L9)
											for(j11=j10;j11<=L11;j11+=L10)
												for(j12=j11;j12<=L12;j12+=L11)
													for(j13=j12;j13<=L13;j13+=L12)
														for(j14=j13;j14<=L14;j14+=L13)
															for(ji=j14;ji<=L15;ji+=L14) 
															{
																ij1 = ij-1;
																ji1 = ji-1;
																
																if(ij-ji<0) 
																{
																	r = b[ij1].re;
																	b[ij1].re = b[ji1].re;
																	b[ji1].re = r;
																	fi = b[ij1].im;
																	b[ij1].im = b[ji1].im;
																	b[ji1].im = fi;
																}
																ij++;
															}
															
															if(in==FORWARD)  // take conjugates & unscramble outputs
															{
																for(i=0,j=n/2;j<n;i++,j++)
																{
																	r = b[j].re; 
																	fi = b[j].im;
																	b[j].re = b[i].re;
																	b[j].im = -b[i].im;
																	b[i].re = r;
																	b[i].im = -fi;
																}
															}

															if(in==INVERSE) // scale outputs
															{
																for(i=0;i<nthpo;i++) 
																{
																	b[i].re *= fn;
																	b[i].im *= fn;
																}
															}
}


/*
** radix 2 iteration subroutine
*/
void R2TX(S32 nthpo, DPCOMPLEX *c0, DPCOMPLEX *c1)
{
	S32 k,kk;
	F64 *cr0, *ci0, *cr1, *ci1, r1, fi1;
	
	cr0 = &(c0[0].re);
	ci0 = &(c0[0].im);
	cr1 = &(c1[0].re);
	ci1 = &(c1[0].im);
	
	for(k = 0; k < nthpo; k += 2) 
    {
		kk = k*2;
		
		r1 = cr0[kk] + cr1[kk];
		cr1[kk] = cr0[kk] - cr1[kk];
		cr0[kk] = r1;
		fi1 = ci0[kk] + ci1[kk];
		ci1[kk] = ci0[kk] - ci1[kk];
		ci0[kk] = fi1;
    }
}


/*
** radix 4 iteration subroutine
*/
void R4TX(S32 nthpo, DPCOMPLEX *c0, DPCOMPLEX *c1, DPCOMPLEX *c2, DPCOMPLEX *c3)
{
	S32 k,kk;
	F64 *cr0, *ci0, *cr1, *ci1, *cr2, *ci2, *cr3, *ci3;
	F64 r1,r2,r3,r4,i1,i2,i3,i4;
	
	cr0 = &(c0[0].re);
	cr1 = &(c1[0].re);
	cr2 = &(c2[0].re);
	cr3 = &(c3[0].re);
	ci0 = &(c0[0].im);
	ci1 = &(c1[0].im);
	ci2 = &(c2[0].im);
	ci3 = &(c3[0].im);
	
	for(k = 1; k <= nthpo; k += 4) 
    {
		kk = (k-1)*2;  /* real and imag parts alternate */
		
		r1 = cr0[kk] + cr2[kk];
		r2 = cr0[kk] - cr2[kk];
		r3 = cr1[kk] + cr3[kk];
		r4 = cr1[kk] - cr3[kk];
		i1 = ci0[kk] + ci2[kk];
		i2 = ci0[kk] - ci2[kk];
		i3 = ci1[kk] + ci3[kk];
		i4 = ci1[kk] - ci3[kk];
		cr0[kk] = r1 + r3;
		ci0[kk] = i1 + i3;
		cr1[kk] = r1 - r3;
		ci1[kk] = i1 - i3;
		cr2[kk] = r2 - i4;
		ci2[kk] = i2 + r4;
		cr3[kk] = r2 + i4;
		ci3[kk] = i2 - r4;
    }
}


	
/*
** radix 8 iteration subroutine
*/
void R8TX(S32 nxtlt, S32 nthpo, S32 lengt, DPCOMPLEX *cc0, DPCOMPLEX *cc1, DPCOMPLEX *cc2,
		  DPCOMPLEX *cc3, DPCOMPLEX *cc4, DPCOMPLEX *cc5, DPCOMPLEX *cc6, DPCOMPLEX *cc7)
{
	S32 j,k,kk;
	F64 scale, arg, tr, ti;
	F64 c1,c2,c3,c4,c5,c6,c7;
	F64 s1,s2,s3,s4,s5,s6,s7;
	F64 ar0,ar1,ar2,ar3,ar4,ar5,ar6,ar7;
	F64 ai0,ai1,ai2,ai3,ai4,ai5,ai6,ai7;
	F64 br0,br1,br2,br3,br4,br5,br6,br7;
	F64 bi0,bi1,bi2,bi3,bi4,bi5,bi6,bi7;
	
	F64 *cr0,*cr1,*cr2,*cr3,*cr4,*cr5,*cr6,*cr7;
	F64 *ci0,*ci1,*ci2,*ci3,*ci4,*ci5,*ci6,*ci7;
	
	cr0 = &(cc0[0].re);
	cr1 = &(cc1[0].re);
	cr2 = &(cc2[0].re);
	cr3 = &(cc3[0].re);
	cr4 = &(cc4[0].re);
	cr5 = &(cc5[0].re);
	cr6 = &(cc6[0].re);
	cr7 = &(cc7[0].re);
	
	ci0 = &(cc0[0].im);
	ci1 = &(cc1[0].im);
	ci2 = &(cc2[0].im);
	ci3 = &(cc3[0].im);
	ci4 = &(cc4[0].im);
	ci5 = &(cc5[0].im);
	ci6 = &(cc6[0].im);
	ci7 = &(cc7[0].im);
	
	
	scale = F_TWO_PI/lengt;
	
	for(j = 1; j <= nxtlt; j++) 
    {
		arg = (j-1)*scale;
		c1 = cos(arg);
		s1 = sin(arg);
		c2 = c1*c1 - s1*s1;
		s2 = c1*s1 + c1*s1;
		c3 = c1*c2 - s1*s2;
		s3 = c2*s1 + s2*c1;
		c4 = c2*c2 - s2*s2;
		s4 = c2*s2 + c2*s2;
		c5 = c2*c3 - s2*s3;
		s5 = c3*s2 + s3*c2;
		c6 = c3*c3 - s3*s3;
		s6 = c3*s3 + c3*s3;
		c7 = c3*c4 - s3*s4;
		s7 = c4*s3 + s4*c3;
		
		for(k = j; k <= nthpo; k += lengt) 
		{
			kk = (k-1)*2; /* index by twos; re & im alternate */
			
			ar0 = cr0[kk] + cr4[kk];
			ar1 = cr1[kk] + cr5[kk];
			ar2 = cr2[kk] + cr6[kk];
			ar3 = cr3[kk] + cr7[kk];
			ar4 = cr0[kk] - cr4[kk];
			ar5 = cr1[kk] - cr5[kk];
			ar6 = cr2[kk] - cr6[kk];
			ar7 = cr3[kk] - cr7[kk];
			ai0 = ci0[kk] + ci4[kk];
			ai1 = ci1[kk] + ci5[kk];
			ai2 = ci2[kk] + ci6[kk];
			ai3 = ci3[kk] + ci7[kk];
			ai4 = ci0[kk] - ci4[kk];
			ai5 = ci1[kk] - ci5[kk];
			ai6 = ci2[kk] - ci6[kk];
			ai7 = ci3[kk] - ci7[kk];
			br0 = ar0 + ar2;
			br1 = ar1 + ar3;
			br2 = ar0 - ar2;
			br3 = ar1 - ar3;
			br4 = ar4 - ai6;
			br5 = ar5 - ai7;
			br6 = ar4 + ai6;
			br7 = ar5 + ai7;
			bi0 = ai0 + ai2;
			bi1 = ai1 + ai3;
			bi2 = ai0 - ai2;
			bi3 = ai1 - ai3;
			bi4 = ai4 + ar6;
			bi5 = ai5 + ar7;
			bi6 = ai4 - ar6;
			bi7 = ai5 - ar7;
			cr0[kk] = br0 + br1;
			ci0[kk] = bi0 + bi1;
			if(j > 1) 
			{
				cr1[kk] = c4*(br0-br1) - s4*(bi0-bi1);
				cr2[kk] = c2*(br2-bi3) - s2*(bi2+br3);
				cr3[kk] = c6*(br2+bi3) - s6*(bi2-br3);
				ci1[kk] = c4*(bi0-bi1) + s4*(br0-br1);
				ci2[kk] = c2*(bi2+br3) + s2*(br2-bi3);
				ci3[kk] = c6*(bi2-br3) + s6*(br2+bi3);
				tr = OO_SQRT2*(br5-bi5);
				ti = OO_SQRT2*(br5+bi5);
				cr4[kk] = c1*(br4+tr) - s1*(bi4+ti);
				ci4[kk] = c1*(bi4+ti) + s1*(br4+tr);
				cr5[kk] = c5*(br4-tr) - s5*(bi4-ti);
				ci5[kk] = c5*(bi4-ti) + s5*(br4-tr);
				tr = -OO_SQRT2*(br7+bi7);
				ti = OO_SQRT2*(br7-bi7);
				cr6[kk] = c3*(br6+tr) - s3*(bi6+ti);
				ci6[kk] = c3*(bi6+ti) + s3*(br6+tr);
				cr7[kk] = c7*(br6-tr) - s7*(bi6-ti);
				ci7[kk] = c7*(bi6-ti) + s7*(br6-tr);
			}
			else 
			{
				cr1[kk] = br0 - br1;
				cr2[kk] = br2 - bi3;
				cr3[kk] = br2 + bi3;
				ci1[kk] = bi0 - bi1;
				ci2[kk] = bi2 + br3;
				ci3[kk] = bi2 - br3;
				tr = OO_SQRT2*(br5-bi5);
				ti = OO_SQRT2*(br5+bi5);
				cr4[kk] = br4 + tr;
				ci4[kk] = bi4 + ti;
				cr5[kk] = br4 - tr;
				ci5[kk] = bi4 - ti;
				tr = -OO_SQRT2*(br7+bi7);
				ti = OO_SQRT2*(br7-bi7);
				cr6[kk] = br6 + tr;
				ci6[kk] = bi6 + ti;
				cr7[kk] = br6 - tr;
				ci7[kk] = bi6 - ti;
			}
		}
    }
}
	

/* see if exactly one bit is set in integer argument */
S32 power_of_2(S32 n)
{
	S32 bits=0;
	while(n)
	{
		bits += n & 1;
		n >>= 1;
	}
	return(bits==1);
}

/* get binary log of integer argument; exact if n a power of 2 */
S32 fastlog2(S32 n)
{
	S32 log = -1;
	while(n)
	{
		log++;
		n >>= 1;
	}
	return(log);
}
