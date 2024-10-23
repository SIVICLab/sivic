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


#ifndef SVK_MRS_TOPO_GENERATOR_H
#define SVK_MRS_TOPO_GENERATOR_H


#include </usr/include/vtk/vtkObjectFactory.h>
#include <svkMrsImageData.h>
#include <svkImageTopologyGenerator.h>
#include <svkSatBandSet.h>


namespace svk {


using namespace std;


class svkMrsImageData;


/*!
 *  Class that generates topologies for svkMrsImageData objects. Currently it can generate
 *  two topology types: a grid that represents the cell structure of a data set and a 
 *  hexahdron that represents the selection box.  
 */
class svkMrsTopoGenerator: public svkImageTopologyGenerator 
{

    public:

        static svkMrsTopoGenerator* New();
        vtkTypeMacro( svkMrsTopoGenerator, vtkObject );

        svkMrsTopoGenerator();
        ~svkMrsTopoGenerator();

        virtual vtkActorCollection* GetTopoActorCollection( svkImageData* data, int actorIndex = 0);

};


}   //svk


#endif //SVK_MRS_TOPO_GENERATOR2_H
