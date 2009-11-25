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
 *  License: TBD
 *
 *
 *
 *  Test driver for svkImageData types.
 *  
 *  The following classes are utilized in this driver. 
 *      svkImageData
 *      svkMrsImage
 *      svkMriImage
 *
 */


#include <svkImageData.h>
#include <svkMrsImageData.h>
#include <svkMriImageData.h>

using namespace svk;

int main (int argc, char** argv)
{

    //  Turn debuggin on (1) or of (0): 
    vtkObject::SetGlobalWarningDisplay(1);
    svkImageData* mrs = svkMrsImageData::New();
    mrs->Delete();

    svkImageData* mri = svkMriImageData::New();
    mri->Delete();

    return 0; 
}


