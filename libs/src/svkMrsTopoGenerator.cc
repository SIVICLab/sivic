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


#include <svkMrsTopoGenerator.h>


using namespace svk;


//vtkCxxRevisionMacro(svkMrsTopoGenerator, "$Rev$");
vtkStandardNewMacro(svkMrsTopoGenerator);


/*!
 * Constructor.
 */
svkMrsTopoGenerator::svkMrsTopoGenerator()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif
    
}


/*!
 * Destructor.
 */
svkMrsTopoGenerator::~svkMrsTopoGenerator()
{
    vtkDebugMacro(<<"svkMrsTopoGenerator::~svkMrsTopoGenerator");
}


/*!
 *  Method chooses the topology to return.
 *
 *  \param data the svkImageData object who's topology you wish to generate
 *
 *  \param actorIndex the index of the topology type you wish to generate
 *
 *  \return a vtkActorCollection of the requested topology
 *
 *  TODO: Implement a sat bands topology.
 */
vtkActorCollection* svkMrsTopoGenerator::GetTopoActorCollection( svkImageData* data, int actorIndex )
{
    vtkActorCollection* topology = NULL;
    switch( actorIndex ) {
        case svkMrsImageData::PLOT_GRID:
            topology = GenerateVoxelGrid( static_cast<svkMrsImageData*>(data) );
            break;
        case svkMrsImageData::VOL_SELECTION:
            topology = GenerateSelectionBox( static_cast<svkMrsImageData*>(data) );
            break;
        case svkMrsImageData::SAT_BANDS:
            svkSatBandSet* satBands = svkSatBandSet::New();
            satBands->SetInput( static_cast<svkMrsImageData*>(data));
            topology = vtkActorCollection::New();
            topology->AddItem(satBands->GetSatBandsActor( ));
            break;
    }
    return topology; 
}
