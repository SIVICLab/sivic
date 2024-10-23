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


#ifndef SVK_VARIABLE_FLIP_DAT_READER_H
#define SVK_VARIABLE_FLIP_DAT_READER_H

#include </usr/include/vtk/vtkInformation.h>
#include <map>
#include </usr/include/vtk/vtkStringArray.h>
#include <string>

#include <svkImageReader2.h>


namespace svk {


/*! 
 *      Reads .dat files that containg scaling information about variable flip angle
 *      acquisitions. 
 *
 *      Authors: 
 *          Jason C. Crane
 *          Peder E.Z. Larson
 *          Beck(Marram) P. Olson 
 *
 *
 *      Example V1 format: 
 *          dat_type: variable_flip_angle
 *          dat_version: 1
 *          dat_content_name: brain_2d_17-Feb-2016
 *          num_time_pts: 20
 *          profile_bandwidth: 543.478261
 *          profile_num_pts: 256
 *          time_point: 1
 *          signal_scaling: space_separated_float_values(256)
 *          mz_scaling: space_separated_float_values(256)
 *          time_point: 2
 *          signal_scaling: space_separated_float_values(256)
 *          mz_scaling: space_separated_float_values(256)
 *          ...
 *          time_point: 20 
 *          signal_scaling: space_separated_float_values(256)
 *          mz_scaling: space_separated_float_values(256)
 *
 *      Thsigna_scaling arrays are array of scaling factors for each dynamic time point. 
 *      The scaling factors are to be applied to the reconstructed data in 
 *      frequency domain by dividing the complex intensities by the scaling factor at the 
 *      corresponding frequency point.  The bandwidth is defined (GetProfileBandwidth) with 0 hz at 
 *      the center point ( GetNumTimePoints()/2 , e.g. 128 for 256 point profile.
 *
 *      The time index starts at 1 
 *      returns status 0 on success 

 */
class svkVariableFlipDatReader : public svkImageReader2 
{

    public:

        static svkVariableFlipDatReader* New();
        vtkTypeMacro( svkVariableFlipDatReader, svkImageReader2);

        // Description: 
        // A descriptive name for this format
        virtual const char* GetDescriptiveName() {
            return "Variable Flip Dat Reader";
        }

        virtual svkImageReader2::ReaderType GetReaderType()
        {
            return svkImageReader2::VARIABLE_FLIP_DAT; 
        }


        //  Methods:
        virtual int                     CanReadFile( const char* fname );
        svkImageReader2::ReaderType     InitDatReader(); 
        int                             GetNumTimePoints(); 
        float                           GetProfileBandwidth();
        int                             GetProfileNumPoints(); 
        int                             GetSignalScaling( int timePt, vtkFloatArray* signalScale ); 
        int                             GetMzScaling( int timePt, vtkFloatArray* mzScale );  


    protected:

        svkVariableFlipDatReader();
        ~svkVariableFlipDatReader();

        //virtual int                                 FillInputPortInformation( int port, vtkInformation* info ); 
        virtual int                                 FillOutputPortInformation( int vtkNotUsed(port), vtkInformation* info ); 

        virtual void                                ExecuteInformation();
        virtual svkDcmHeader::DcmPixelDataFormat    GetFileType();

    private:

        //  Methods:
        virtual void                                InitDcmHeader();
        void                                        ParseDatFile();
        svkImageReader2::ReaderType                 datType; 
        ifstream*                                   datFp;
        map <string, string>                        datMap; 
        int                                         OpenDatFile(const char* fname); 
        int                                         ParseDatMetaData(); 
        void                                        PrintKeyValuePairs(); 
        int                                         GetScaling( string type, int timePt, vtkFloatArray* signalScale ); 


};


}   //svk


#endif //SVK_VARIABLE_FLIP_DAT_READER_H

