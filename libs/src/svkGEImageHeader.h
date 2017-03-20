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
 *      Don C. Bigler, Ph.D.
 */

#ifndef SVK_GE_IMAGE_HEADER_H
#define SVK_GE_IMAGE_HEADER_H


namespace svk {


struct GEImageMainHeader {
    int Magic_Number;
    int Header_Length;
    int Width;
    int Height;
    int Depth;
    int Compressed;
    int Dwindow;
    int Dlevel;
    int Background_Shade;
    int Overflow;
    int Underflow;
    int Top_Offset;
    int Bottom_Offset;
    short Version;
    unsigned short Checksum;
    int Pointer_ID;
    int Length_ID;
    int Pointer_Unpack;
    int Length_Unpack;
    int Pointer_Compress;
    int Length_Compress;
    int Pointer_Histogram;  // Appears to point to suite/exam/series/image info in version 2.
    int Length_Histogram;
    int Pointer_Text;
    int Length_Text;
    int Pointer_Graphics;
    int Length_Graphics;
    int Pointer_DBHeader;
    int Length_DBHeader;
    int Level_Offset;
    int Pointer_User;
    int Length_User;
    int Pointer_Suite;  // Not present in version 2.
    int Length_Suite;   // Not present in version 2.
    int Pointer_Exam;   // Not present in version 2.
    int Length_Exam;    // Not present in version 2.
    int Pointer_Series; // Not present in version 2.
    int Length_Series;  // Not present in version 2.
    int Pointer_Image;  // Not present in version 2.
    int Length_Image;   // Not present in version 2.
};

struct GESignaHeader {
    GEImageMainHeader mainHeader;
    char Suite_Product_ID[13];                    // Product ID from Suite header.
    char Exam_System_ID[5];                       // System ID stored in Exam header (plus 1 for string terminator).
    unsigned short Exam_Number;                   // Exam number from Exam header.
    char Exam_Hospital_Name[34];                  // Version 3 only uses 34 bytes (not sure for version 2).
    unsigned int Exam_Magnet_Strength;            // MR magnet strength extracted from Exam header.
    char Exam_Patient_ID[13];                     // Patient ID stored in Exam header. 
    char Exam_Patient_Name[25];                   // Patient name stored in Exam header.
    unsigned short Exam_Patient_Age;              // Patient age stored in Exam header.
    unsigned short Exam_Patient_Age_Notation;     // Patient age notation (years, etc.) stored in Exam header.
    unsigned short Exam_Patient_Sex;              // Patient sex.
    unsigned int Exam_Patient_Weight_In_Grams;    // Patient weight in grams stored in Exam header.
    char Exam_Patient_History[62];                // Version 2 uses 62 bytes & version 3 61.
    char Exam_Requisition_Number[13];             // Requisition Number from Exam header.
    int Exam_Time_Stamp;                          // Study time stamp from Exam header.
    char Exam_Time_Stamp_Date[9];                 // Study time stamp date as a string.
    char Exam_Time_Stamp_Time[7];                 // Study time stamp time as a string.
    char Exam_Referring_Physician[33];            // Referring physician from Exam header.
    char Exam_Radiologist[33];                    // Radiologist performing study from Exam header.
    char Exam_Operator[4];                        // Operator from Exam header.
    char Exam_Description[23];                    // Study description from Exam header.
    char Exam_Type[3];                            // Type of Exam (MR or CT).
    char Exam_Unique_System_ID[17];               // Unique system ID from Exam header (plus 1 for string terminator).
    char Exam_Software_Version[3];                // Software version stored in Exam header (plus 1 for string terminator).
    unsigned short Series_Number;                 // Series number from Series header.
    int Series_Time_Stamp;                        // Series time stamp from Series header.
    char Series_Time_Stamp_Date[9];               // Series time stamp date as a string.
    char Series_Time_Stamp_Time[7];               // Series time stamp time as a string.
    char Series_Description[30];                  // Series description from Series header.
    unsigned short Series_Type;                   // Series type value from Series header.
    unsigned int Series_Patient_Position;         // Patient position from Series header.
    unsigned int Series_Patient_Entry;            // Patient entry from Series header.
    char Series_Scan_Potocol_Name[26];            // Scan protocol name string from Series header.
    unsigned short MR_Image_Number;               // Image number from MR header.
    int MR_Time_Stamp;                            // Acquisition time stamp from MR header.
    char MR_Time_Stamp_Date[9];                   // Acquisition time stamp date as string.
    char MR_Time_Stamp_Time[7];                   // Acquisition time stamp time as string.
    float MR_Scan_Duration;                       // Duration of scan in seconds from MR header.
    float MR_Slice_Thickness;                     // Slice thickness in mm from MR header.
    unsigned short MR_Image_Matrix_Size_X;        // Image x matrix size from MR header.
    unsigned short MR_Image_Matrix_Size_Y;        // Image y matrix size from MR header.
    float MR_FOV_X;                               // Image FOV in the x dimension from MR header.
    float MR_FOV_Y;                               // Image FOV in the y dimension from MR header.
    float MR_Image_Dimension_X;                   // Image x dimension from MR header.
    float MR_Image_Dimension_Y;                   // Image y dimension from MR header.
    float MR_Image_Pixel_Size_X;                  // Image pixel spacing in the x dimension from MR header.
    float MR_Image_Pixel_Size_Y;                  // Image pixel spacing in the y dimension from MR header.
    char MR_IV_Contrast_Agent[14];                // IV contrast agent from MR header.
    char MR_Oral_Contrast_Agent[17];              // Oral contrast agent from MR header.
    unsigned short MR_Image_Contrast_Mode;        // Image contrast mode used from MR header.
    unsigned short MR_Plane_Type;                 // Imaging plane from MR header.
    float MR_Slice_Spacing;                       // Spacing between slices from MR header.
    float MR_Image_Location;                      // Spatial location of slice from MR header.
    float MR_R_Normal;                            // Right coordinate normal vector value.
    float MR_A_Normal;                            // Anterior coordinate normal vector value.
    float MR_S_Normal;                            // Superior coordinate normal vector value.
    float MR_R_Top_Left_Corner;                   // Right coordinate of top left hand corner from MR header.
    float MR_A_Top_Left_Corner;                   // Anterior coordinate of top left hand corner from MR header.
    float MR_S_Top_Left_Corner;                   // Superior coordinate of top left hand corner from MR header.
    float MR_R_Top_Right_Corner;                  // Right coordinate of top right hand corner from MR header.
    float MR_A_Top_Right_Corner;                  // Anterior coordinate of top right hand corner from MR header.
    float MR_S_Top_Right_Corner;                  // Superior coordinate of top right hand corner from MR header.
    float MR_R_Bottom_Right_Corner;               // Right coordinate of bottom right hand corner from MR header.
    float MR_A_Bottom_Right_Corner;               // Anterior coordinate of bottom right hand corner from MR header.
    float MR_S_Bottom_Right_Corner;               // Superior coordinate of bottom right hand corner from MR header.
    unsigned int MR_Pulse_Repetition_Time;        // Pulse repetition time (TR) in usec from MR header.
    unsigned int MR_Pulse_Inversion_Time;         // Pulse inversion time (TI) in usec from MR header.
    unsigned int MR_Pulse_Echo_Time;              // Pulse echo time (TE) in usec from MR header.
    unsigned short MR_Number_Of_Echoes;           // Number of echoes from MR header.
    unsigned short MR_Echo_Number;                // Echo number from MR header.
    float MR_Number_Of_Averages;                  // Number of averages from MR header.
    unsigned short MR_Cardiac_Heart_Rate;         // Cardiac heart rate in BPM from MR header.
    float MR_Average_SAR;                         // Average SAR from MR header.
    unsigned short MR_Trigger_Window;             // Trigger window from MR header.
    unsigned short MR_Images_Per_Cardiac_Cycle;   // Images per cardiac cycle from MR header.
    short MR_Flip_Angle;                          // Flip angle of sequence in degrees from MR header.
    int MR_Center_Frequency;                      // Center frequency of MR scanner in 0.1 HZ from MR header.
    unsigned short MR_Imaging_Mode;               // MR imaging mode from MR header.
    unsigned int MR_Imaging_Options;              // MR imaging options from MR header.
    unsigned short MR_Pulse_Sequence;             // Pulse sequence number from MR header.
    char MR_Pulse_Sequence_Name[34];              // Pulse sequence name from MR header.
    char MR_Receive_Coil_Name[18];                // Receive coil name from MR header.
    int MR_Raw_Data_Run_Number;                   // Acquisition number from MR header.
    unsigned short MR_Fat_Water_SAT;              // Fat/Water saturation value from MR header.
    float MR_Variable_Bandwidth;                  // Variable bandwidth in Hz from MR header.
    unsigned short MR_Number_Of_Slices;           // Number of slices in this scan group from MR header.
    int MR_Timestamp_Of_Last_Change;              // Time stamp of last change from MR header.
    char MR_Timestamp_Of_Last_Change_Date[9];     // Time stamp date of last change as string.
    char MR_Timestamp_Of_Last_Change_Time[7];     // Time stamp time of last change as string.
    unsigned short MR_Bitmap_Of_Saturation_Selections; // Bitmap of saturation selectioins used from MR header.
    unsigned short MR_Surface_Coil_Intensity_Correction; // Surface coil intensity correction value from MR header.
    unsigned short MR_Image_Type;                 // MR image type (magnitude, phase, real, imaginary, spectroscopy).
    unsigned short MR_Vascular_Collapse;          // Collapse image value from MR header.
    unsigned short MR_Projection_Algorithm;       // Projection algorithm from MR header.
    unsigned short MR_Echo_Train_Length;          // Echo train length from MR header.
    unsigned short MR_Fractional_Echo_Flag;       // Fractional echo flag from MR header.
    unsigned short MR_Preparatory_Pulse_Option;   // Prep. pulse option from MR header.
    unsigned short MR_Frequency_Direction;        // Frequency direction from the MR header.
};


}   //svk
#endif //SVK_GE_IMAGE_HEADER_H
