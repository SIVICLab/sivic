/*
 *  Copyright © 2009-2014 The Regents of the University of California.
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
 */
#include <sivicBasicTest.h>
#include <vtkSivicController.h>


/*! 
 *  Constructor
 */
sivicBasicTest::sivicBasicTest( )
{

}


/*! 
 *  Destructor
 */
sivicBasicTest::~sivicBasicTest()
{
    
}

void sivicBasicTest::Run() 
{
    cout << "Basic test running " << endl;
    this->sivicController->PopupMessage( "TESTING IMAGE OPEN...." );
    this->sivicController->OpenImage( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_fla.idf" );
    this->sivicController->PopupMessage( "TESTING SPECTRA OPEN...." );
    this->sivicController->Open4DImage( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_1_cor.ddf" );
    this->sivicController->PopupMessage( "TESTING METABOLTE OPEN...." );
    this->sivicController->OpenOverlay( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_1r01.idf" );
    this->sivicController->PopupMessage( "TESTING IMAGE OVERLAY OPEN...." );
    this->sivicController->OpenOverlay( "/home/bolson/Projects/visualization/4svn/visualization/trunk/libs/testing/data/t3148_adca.idf" );
    this->sivicController->PopupMessage( "TESTING SLICING...." );
    this->sivicController->SetSlice(0);
    this->sivicController->SetSlice(1);
    this->sivicController->SetSlice(2);
    this->sivicController->SetSlice(3);
    this->sivicController->SetSlice(4);
    this->sivicController->SetSlice(5);
    this->sivicController->SetSlice(6);
    this->sivicController->SetSlice(7);
    this->sivicController->PopupMessage( "TESTING SAVE AND RESTORE...." );
    this->sivicController->SaveSession();
    this->sivicController->ResetApplication();
    this->sivicController->RestoreSession();
    this->sivicController->PopupMessage( "TESTING RESET...." );
    this->sivicController->ResetApplication();

}
