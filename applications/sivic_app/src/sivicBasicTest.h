/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */


#ifndef SIVIC_BASIC_TEST_H 
#define SIVIC_BASIC_TEST_H 

#include <sivicTestCase.h>
#include <iostream>

using namespace std;
/*! 
 */ 
class sivicBasicTest : public sivicTestCase
{
    public:
        
        sivicBasicTest( );
        ~sivicBasicTest();
        virtual void Run();

};
#endif //SIVIC_BASIC_TEST_H 
