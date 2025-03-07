/*
 *  Copyright © 2009-2017 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice, 
 *      this list of conditions and the following disclaimer in the documentation 
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name 
 *      "The Regents of the University of California," or the names of any of its 
 *      contributors may be used to endorse or promote products derived from this 
 *      software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *  OF SUCH DAMAGE.
 */



/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkByteSwap.h>


using namespace svk;


//vtkCxxRevisionMacro(svkByteSwap, "$Rev$");


/*!
 *
 */
void svkByteSwap::SwapBufferEndianness(short* data, int size)
{
    int i;
    for( i = 0; i < size; i++ ) {
        svkByteSwap::SwapEndianness( &data[i] );
    }
}


/*!
 *
 */
void svkByteSwap::SwapBufferEndianness(int* data, int size)
{
    int i;
    for( i = 0; i < size; i++ ) {
        svkByteSwap::SwapEndianness( &data[i] );
    }
}


/*!
 *
 */
void svkByteSwap::SwapBufferEndianness(float* data, int size)
{
    int i;
    for( i = 0; i < size; i++ ) {
        svkByteSwap::SwapEndianness(&data[i]);
    }
}


/*!
 *
 */
void svkByteSwap::SwapBufferEndianness(double* data, int size)
{
    int i;
    for( i = 0; i < size; i++ ) {
        svkByteSwap::SwapEndianness(&data[i]);
    }
}


/*!
 *
 */
void svkByteSwap::SwapEndianness(unsigned short* data)
{
    svkByteSwap::SwapEndianness( reinterpret_cast<short*>(data) ); 
}


/*!
 *
 */
void svkByteSwap::SwapEndianness(short* data)
{
    char* buf;
    char  c0; 
    char  c1;
    buf = (char*)data;
    c0 = buf[0];
    c1 = buf[1];
    buf[0] = c1;
    buf[1] = c0;
}


/*!
 *
 */
void svkByteSwap::SwapEndianness(unsigned int* data)
{
    svkByteSwap::SwapEndianness( reinterpret_cast<int*>(data) ); 
}


/*!
 *
 */
void svkByteSwap::SwapEndianness(int* data)
{
    char* buf;
    char c0; 
    char c1; 
    char c2; 
    char c3;
    buf = (char*)data;
    c0 = buf[0];
    c1 = buf[1];
    c2 = buf[2];
    c3 = buf[3];
    buf[0] = c3;
    buf[1] = c2;
    buf[2] = c1;
    buf[3] = c0;
}


/*!
 *
 */
void svkByteSwap::SwapEndianness(float* data)
{
    char* buf;
    char  c0; 
    char  c1; 
    char  c2; 
    char  c3; 
    buf = (char*)data;
    c0 = buf[0];
    c1 = buf[1];
    c2 = buf[2];
    c3 = buf[3];
    buf[0] = c3;
    buf[1] = c2;
    buf[2] = c1;
    buf[3] = c0;
}


/*!
 *
 */
void svkByteSwap::SwapEndianness(double* data)
{
    char* buf;
    char  c0; 
    char  c1; 
    char  c2; 
    char  c3; 
    char  c4; 
    char  c5; 
    char  c6; 
    char  c7; 
    buf = (char*)data;
    c0 = buf[0];
    c1 = buf[1];
    c2 = buf[2];
    c3 = buf[3];
    c4 = buf[4];
    c5 = buf[5];
    c6 = buf[6];
    c7 = buf[7];
    buf[0] = c7;
    buf[1] = c6;
    buf[2] = c5;
    buf[3] = c4;
    buf[4] = c3;
    buf[5] = c2;
    buf[6] = c1;
    buf[7] = c0;
}



