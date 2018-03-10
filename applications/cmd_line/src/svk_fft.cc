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
 *
 *  Utility application for converting between supported file formats. 
 *
 */

#include <vtkSmartPointer.h>

#include <svkImageReaderFactory.h>
#include <svkImageReader2.h>
#include <svkDdfVolumeReader.h>
#include <svkDcmVolumeReader.h>
#include <svkImageWriterFactory.h>
#include <svkImageWriter.h>
#include <svkDICOMMRSWriter.h>
#include <svkDdfVolumeWriter.h>
#include <svkCorrectDCOffset.h>
#include <svkDcmHeader.h>
#include <svkGEPFileReader.h>
#include <svkGEPFileMapper.h>
#include <svkImageAlgorithm.h>
#include <svkMrsImageFFT.h>
#ifdef WIN32
extern "C" {
#include <getopt.h>
}
#else
#include <getopt.h>
#endif
#define UNDEFINED_TEMP -1111

using namespace svk;



int main (int argc, char** argv)
{

    string usemsg("\n") ; 
    usemsg += "Version " + string(SVK_RELEASE_VERSION) +                                           "\n";
    usemsg += "svk_fft -i input_file_name -o output_root_name [ -t output_data_type ]               \n";
    usemsg += "        [ --spec ] [ --spatial ] [ --single ]                                        \n";
    usemsg += "        [                                                                            \n";
    usemsg += "             ( --vsx sihftX  --vsy shiftY  --vsz shiftZ )                            \n";
    usemsg += "          || (--ctrL ctrL --ctrP ctrP --ctrS ctrS )                                  \n";
    //usemsg += "          || (--voxL voxL --P voxP --S voxS )                                        \n";
    usemsg += "          || ( --maxVoxels )                                                         \n";
    usemsg += "        )                                                                            \n";
    usemsg += "        [ -h ]                                                                       \n";
    usemsg += "                                                                                     \n";
    usemsg += "   -i            name        Name of file to convert.                                \n";
    usemsg += "   -o            root        Root name of outputfile.                                \n";
    usemsg += "   -t            type        Target data type:                                       \n";
    usemsg += "                                 2 = UCSF DDF                                        \n";
    usemsg += "                                 4 = DICOM_MRS (default)                             \n";
    usemsg += "   --spec                    transform spectral domain only                          \n";
    usemsg += "   --spatial                 transform spatial domain only                           \n";
    usemsg += "   --vsx         shiftX      Fractional voxel shift in X                             \n";
    usemsg += "   --vsy         shiftY      Fractional voxel shift in Y                             \n";
    usemsg += "   --vsz         shiftZ      Fractional voxel shift in Z                             \n";
    usemsg += "   --ctrL        ctrL        center of volume (L position)                           \n";
    usemsg += "   --ctrP        ctrP        center of volume (P position)                           \n";
    usemsg += "   --ctrS        ctrS        center of volume (S position)                           \n";
    //usemsg += "   --voxL        voxL        L position to center nearest voxel on                   \n";
    //usemsg += "   --voxP        voxP        P position to center nearest voxel on                   \n";
    //usemsg += "   --voxS        voxS        S position to center nearest voxel on                   \n";
    usemsg += "   --nst                     apply factor of N normalization to spatial IFFT         \n";
    usemsg += "   --maxVoxels               Maximize num voxels > .5 in selection box               \n";
    usemsg += "   --single                  Only transform specified file if multiple in series     \n";
    usemsg += "   -b                        Only transform data in selection box, only valid for    \n";
    usemsg += "                             --spec transforms. Ignored otherwise                    \n";
    usemsg += "   -h                        Print this help mesage.                                 \n";
    usemsg += "                                                                                     \n";
    usemsg += "Performs spatial and spectral FFTs by default.  If --spec or --spatial is specified, \n";
    usemsg += "will transform only the specified domain.                                            \n";
    usemsg += "\n";
    usemsg += "Voxel Shift options:                                                                 \n";
    usemsg += "\tFractional voxel shifts can be specified (--vs*) to reconstruct at a shifted position. \n";
    usemsg += "\t\tShifts are given in fractions of a voxel along the data axes (cols, rows, slices). \n";
    usemsg += "\tThe LPS position of the MRS grid can be specified wiht the --ctrL/P/S flags.         \n";
    //usemsg += "\tThe LPS position to center a voxel on can be specified with the --voxL/P/S flags.    \n";
    usemsg += "\tTo maximize the number of whole voxels in the selction box use the --maxVoxels flag. \n";
    usemsg += "\n";


    string inputFileName; 
    string outputFileName;
    bool transformSpecDomain = true; 
    bool transformSpatialDomain = true; 
    bool onlyTransformSingle = false; 
    bool onlyTransformSelectionBox = false;
    bool normalizeSpatialTransform = false; 

    double voxelShift[3]; 
    voxelShift[0] = 0.; 
    voxelShift[1] = 0.; 
    voxelShift[2] = 0.; 
    double centerLPS[3]; 
    centerLPS[0] = VTK_DOUBLE_MIN;
    centerLPS[1] = VTK_DOUBLE_MIN;
    centerLPS[2] = VTK_DOUBLE_MIN;
    double voxelLPS[3];
    voxelLPS[0] = VTK_DOUBLE_MIN;
    voxelLPS[1] = VTK_DOUBLE_MIN;
    voxelLPS[2] = VTK_DOUBLE_MIN;
    bool maximizeVoxelsInBox = false;

    svkImageWriterFactory::WriterType dataTypeOut = svkImageWriterFactory::DICOM_MRS;

    string cmdLine = svkProvenance::GetCommandLineString( argc, argv ); 

    enum FLAG_NAME {
        FLAG_TRANSFORM_SPEC_DOMAIN = 0, 
        FLAG_TRANSFORM_SPATIAL_DOMAIN, 
        FLAG_VOXEL_SHIFT_X, 
        FLAG_VOXEL_SHIFT_Y, 
        FLAG_VOXEL_SHIFT_Z, 
        FLAG_CENTER_L, 
        FLAG_CENTER_P, 
        FLAG_CENTER_S,
        FLAG_MAX_VOXELS,
        FLAG_NST,
        FLAG_SINGLE
    }; 


    static struct option long_options[] =
    {
        /* This option sets a flag. */
        {"spec",      no_argument,       NULL,  FLAG_TRANSFORM_SPEC_DOMAIN},
        {"spatial",   no_argument,       NULL,  FLAG_TRANSFORM_SPATIAL_DOMAIN},
        {"vsx",       required_argument, NULL,  FLAG_VOXEL_SHIFT_X},
        {"vsy",       required_argument, NULL,  FLAG_VOXEL_SHIFT_Y},
        {"vsz",       required_argument, NULL,  FLAG_VOXEL_SHIFT_Z},
        {"ctrL",      required_argument, NULL,  FLAG_CENTER_L},
        {"ctrP",      required_argument, NULL,  FLAG_CENTER_P},
        {"ctrS",      required_argument, NULL,  FLAG_CENTER_S},
        {"maxVoxels", no_argument,       NULL,  FLAG_MAX_VOXELS},
        {"single",    no_argument,       NULL,  FLAG_SINGLE},
        {"nst",       no_argument,       NULL,  FLAG_NST},
        {0, 0, 0, 0}
    };



    // ===============================================  
    //  Process flags and arguments
    // ===============================================  
    int i;
    int option_index;
    option_index = 0;
    while ( ( i = getopt_long(argc, argv, "i:o:t:bh", long_options, &option_index) ) != EOF) {
        switch (i) {
            case 'i':
                inputFileName.assign( optarg );
                if( ! svkUtils::FilePathExists( inputFileName.c_str() ) ) {
                    cerr << endl << "Input file can not be loaded (may not exist) " << inputFileName << endl << endl;
                    exit(1);
                }
                break;
            case 'o':
                outputFileName.assign(optarg);
                break;
            case 't':
                dataTypeOut = static_cast<svkImageWriterFactory::WriterType>( atoi(optarg) );
                break;
            case 'b':
                onlyTransformSelectionBox = true; 
                break;    
            case FLAG_TRANSFORM_SPEC_DOMAIN:
                transformSpecDomain = true;
                transformSpatialDomain = false; 
                break;
            case FLAG_TRANSFORM_SPATIAL_DOMAIN:
                transformSpecDomain = false;
                transformSpatialDomain = true; 
                break;
            case FLAG_VOXEL_SHIFT_X:
                voxelShift[0] = atof( optarg );
                break;
            case FLAG_VOXEL_SHIFT_Y:
                voxelShift[1] = atof( optarg );
                break;
            case FLAG_VOXEL_SHIFT_Z:
                voxelShift[2] = atof( optarg );
                break;
            case FLAG_CENTER_L:
                centerLPS[0] = atof( optarg );
                break;
            case FLAG_CENTER_P:
                centerLPS[1] = atof( optarg );
                break;
            case FLAG_CENTER_S:
                centerLPS[2] = atof( optarg );
                break;
            case FLAG_MAX_VOXELS:
                maximizeVoxelsInBox = true;
                break;
            case FLAG_SINGLE:
                onlyTransformSingle = true;
                break;
            case FLAG_NST:
                normalizeSpatialTransform = true;
                break;
            case 'h':
                cout << usemsg << endl;
                exit(1);  
                break;
            default:
                ;
        }
    }

    argc -= optind;
    argv += optind;

    // ===============================================  
    //  validate that: 
    //      an output name was supplied
    //      that not suppresses and unsuppressed were both specified 
    //      that only the supported output types was requested. 
    //      
    // ===============================================  
    if ( argc != 0 ||  inputFileName.length() == 0  
         || outputFileName.length() == 0 
         || ( dataTypeOut != svkImageWriterFactory::DICOM_MRS && dataTypeOut != svkImageWriterFactory::DDF ) 
         
    ) {
        cout << usemsg << endl;
        exit(1); 
    }

    cout << "file name: " << inputFileName << endl;

    // ===============================================  
    //  Use a reader factory to get the correct reader  
    //  type . 
    // ===============================================  
    vtkSmartPointer< svkImageReaderFactory > readerFactory = vtkSmartPointer< svkImageReaderFactory >::New(); 
    svkImageReader2* reader = readerFactory->CreateImageReader2(inputFileName.c_str());

    if (reader == NULL) {
        cerr << "Can not determine appropriate reader for: " << inputFileName << endl;
        exit(1);
    }

    reader->SetFileName( inputFileName.c_str() );
    if ( onlyTransformSingle == true ) {
        reader->OnlyReadOneInputFile();
    }
    reader->Update(); 

    // ===============================================  
    //  fft
    // ===============================================  

    svkDcmHeader* hdr = reader->GetOutput()->GetDcmHeader();
    if ( transformSpecDomain ) {

        svkMrsImageFFT* imageFFT = svkMrsImageFFT::New();
        imageFFT->SetInputData( reader->GetOutput() );

        imageFFT->SetFFTDomain( svkMrsImageFFT::SPECTRAL ); 
        string specDomain = hdr->GetStringValue( "SignalDomainColumns"); 
        if ( specDomain.compare("TIME") == 0 ) {
            //  time to frequency: 
            imageFFT->SetFFTMode( svkMrsImageFFT::FORWARD ); 
        } else {
            //  frequency to time: 
            imageFFT->SetFFTMode( svkMrsImageFFT::REVERSE ); 
        }
        
        if ( onlyTransformSelectionBox == true) { 
            imageFFT->OnlyUseSelectionBox();
        }

        if (normalizeSpatialTransform == true) {
            imageFFT->NormalizeTransform();
        }

        imageFFT->Update();
        imageFFT->Delete();
    }

    if (transformSpatialDomain ) { 

        svkMrsImageFFT* spatialFFT = svkMrsImageFFT::New();

        spatialFFT->SetInputData( reader->GetOutput() );
        spatialFFT->SetFFTDomain( svkMrsImageFFT::SPATIAL );

        string domainCol = hdr->GetStringValue( "SVK_ColumnsDomain");
        if ( domainCol.compare("SPACE") == 0) {
            spatialFFT->SetFFTMode( svkMrsImageFFT::FORWARD);
        } else {
            spatialFFT->SetFFTMode( svkMrsImageFFT::REVERSE );
        }
        spatialFFT->SetPreCorrectCenter( true );
        spatialFFT->SetPostCorrectCenter( true );

        //  Voxel Shift Options
        //  use following bool to ensure that only 1 method is used
        bool voxelShiftAlreadySpecified = false;
        if ( voxelShift[0] != 0 ||  voxelShift[1] != 0 || voxelShift[2] != 0 ) {
            spatialFFT->SetVoxelShift( voxelShift );
            voxelShiftAlreadySpecified = true;
        }
        if ( centerLPS[0] != VTK_DOUBLE_MIN ||  centerLPS[1] != VTK_DOUBLE_MIN || centerLPS[2] != VTK_DOUBLE_MIN ) {
            if ( voxelShiftAlreadySpecified == true ) {
                cout << "ERROR, only specify one voxel shift method!" << endl;
                cout << usemsg << endl;
                exit(1);
            } else {
                spatialFFT->SetVolumeCenter(centerLPS);
                voxelShiftAlreadySpecified = true;
            }
        }
        if ( maximizeVoxelsInBox == true ) {
            if ( voxelShiftAlreadySpecified == true ) {
                cout << "ERROR, only specify one voxel shift method!" << endl;
                cout << usemsg << endl;
                exit(1);
            } else {
                spatialFFT->MaximizeVoxelsInSelectionBox();
                voxelShiftAlreadySpecified = true;
            }
        }
        spatialFFT->Update();
        spatialFFT->Delete();

    }


    // ===============================================  
    //  Write the data out to the specified file type.  
    //  Use an svkImageWriterFactory to obtain the
    //  correct writer type. 
    // ===============================================  
    vtkSmartPointer< svkImageWriterFactory > writerFactory = vtkSmartPointer< svkImageWriterFactory >::New(); 
    svkImageWriter* writer = static_cast<svkImageWriter*>( writerFactory->CreateImageWriter( dataTypeOut ) ); 

    if ( writer == NULL ) {
        cerr << "Can not determine writer of type: " << dataTypeOut << endl;
        exit(1);
    }

    writer->SetFileName( outputFileName.c_str() );
    writer->SetInputData( svkMrsImageData::SafeDownCast( reader->GetOutput() ) );

    // ===============================================  
    //  Set the input command line into the data set 
    //  provenance: 
    // ===============================================  
    reader->GetOutput()->GetProvenance()->SetApplicationCommand( cmdLine );

    // ===============================================  
    //  Write data to file: 
    // ===============================================  
    writer->Write();

    // ===============================================  
    //  Clean up: 
    // ===============================================  
    writer->Delete();
    reader->Delete();

    return 0; 
}

