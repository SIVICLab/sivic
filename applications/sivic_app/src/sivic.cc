/*   
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */

#include <sivicApp.h>
#include <iostream>
#include <pthread.h>

void *print_message_function( void *ptr );
void *startApplication(void *ptr);
    
sivicApp sivic = sivicApp();

using namespace std;

/*! 
 *  A prototype MR Spectroscopy viewer. The visualization compenent of a 
 *  new stardards-based open source project aimed at promoting the 
 *  integration and use of the DICOM MR Spectroscopy object within the 
 *  imaging community to facilitate medical research and clincial science.
 */
int main(int argc, char* argv[])
{

    sivic.Build(argc, argv);
    sivic.Start(argc, argv);
    return 0;
} 
