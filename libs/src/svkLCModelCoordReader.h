/*
 *  Copyright © 2009-2015 The Regents of the University of California.
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


#ifndef SVK_LCMODEL_COORD_READER_H
#define SVK_LCMODEL_COORD_READER_H

#include <vtkInformation.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

#include <svkUtils.h>
#include <svkLCModelReader.h>


namespace svk {


/*! 
 *  
 */
class svkLCModelCoordReader : public svkLCModelReader
{

    public:

        static svkLCModelCoordReader* New();
        vtkTypeMacro( svkLCModelCoordReader, svkLCModelReader );

        //  Enum reader type
        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::LC_MODEL_COORD;
        }
        

        //  Methods:
        virtual int             CanReadFile( const char* fname );
        void                    SetDataStartDelimiter( string delimiterString ); 

    protected:

        svkLCModelCoordReader();
        ~svkLCModelCoordReader();

        virtual int                              FillOutputPortInformation(int port, vtkInformation* info);
        virtual void                             ExecuteDataWithInformation(vtkDataObject *output, vtkInformation* outInfo);


    private:

        //  Methods:
        void            ParseCoordFiles();
        void            GetDataRow(vtkTable* table, int rowID, string* rowString); 
        void            InitPPMRangeFromCoordFile(); 
        void            InitNumFreqPointsFromCoordFile(); 
        void            CheckOutputPtsPerPPM(); 
        int             FindStartOfIntensityData( vtkTable* coordDataTable ); 
        void            ParseIntensityValuesFromCoord( 
                            vtkTable* coordDataTable, 
                            int rowID, 
                            float* coordIntensities 
                        ); 


        //  Members:
        void*                                   pixelData; 
        vtkDoubleArray*                         coordPixelValues; 
        vtkIntArray*                            coordColIndex; 
        vtkIntArray*                            coordRowIndex; 
        string                                  dataStartDelimiter; 
        float                                   ppmStart; 
        float                                   ppmEnd; 
        int                                     numCoordFreqPoints;


};


}   //svk


#endif //SVK_LCMODEL_COORD_READER_H

