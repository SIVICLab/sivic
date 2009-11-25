/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 */


#ifndef SIVIC_TEST_CASE_H 
#define SIVIC_TEST_CASE_H 

class vtkSivicController;

/*! 
 *  Abstact base class for tests cases.
 */ 
class sivicTestCase
{
    public:
        
        sivicTestCase( );
        ~sivicTestCase();
        virtual void SetSivicController( vtkSivicController* sivicController );
        
        virtual void Run() = 0;
    
    protected: 
        vtkSivicController* sivicController;
        
};

#endif //SIVIC_TEST_CASE_H 
