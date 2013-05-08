/*
 *  Copyright © 2009-2011 The Regents of the University of California.
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
 *
 */


#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkImageData.h>
#include <svkMriImageData.h>
#include <svkDcmHeader.h>

using namespace std; 
using namespace svk; 

int main (int argc, char** argv)
{

    if ( argc != 2 ) {
        std::cout << " test_svkDcmQuickRead fileName " << endl;
        exit(1); 
    }
    string fileName(argv[1]); 
  /* 
    svkImageReaderFactory* readerFactory = svkImageReaderFactory::New();
    svkImageReader2* reader = readerFactory->CreateImageReader2(fileName.c_str());
    readerFactory->Delete();

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << fileName << endl;
        exit(1);
    }

    reader->SetFileName( fileName.c_str() );
    reader->Update();
*/ 
    svkImageData* tmp = svkMriImageData::New();
    tmp->GetDcmHeader()->ReadDcmFile(  fileName  );
    //tmp->GetDcmHeader()->PrintDcmHeader(); 

    cout << "check" << endl;
    //reader->GetOutput()->GetDcmHeader()->PrintDcmHeader(); 

    cout << "PD EXISTS: " << tmp->GetDcmHeader()->ElementExists("PixelData") << endl;;  
    //cout << "PD EXISTS: " << reader->GetOutput()->GetDcmHeader()->ElementExists("PixelData") << endl;;  


    svkDcmHeader::DimensionVector dimensionVector = tmp->GetDcmHeader()->GetDimensionIndexVector();
    int rows = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::ROW_INDEX) + 1;
    int columns = svkDcmHeader::GetDimensionValue( &dimensionVector, svkDcmHeader::COL_INDEX) + 1;
    int numFrames = tmp->GetDcmHeader()->GetIntValue( "NumberOfFrames" );

    int numPixels = rows * columns * numFrames; 
    int numBytesPerPixel = 2; 
    int pixelBytes = numPixels * numBytesPerPixel; 

    cout << "numPixels: " << numPixels << endl;



    for (int i = 0; i < numPixels/100; i++ ) {
        short pixel = tmp->GetDcmHeader()->GetPixelValue( i ); 
        cout << "PIXEL: " << i << " " << pixel << endl;
    }



    //reader->GetOutput()->GetDcmHeader()->GetShortValue( 
            //"PixelData", 
            //((short *)pixelBuffer), 
           //1//numPixels 
    //); 


    return 0; 
}


