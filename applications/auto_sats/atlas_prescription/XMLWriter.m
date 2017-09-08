%{

    Copyright © 2009-2017 The Regents of the University of California.
    All Rights Reserved.
  
    Redistribution and use in source and binary forms, with or without 
    modification, are permitted provided that the following conditions are met:
    •   Redistributions of source code must retain the above copyright notice, 
        this list of conditions and the following disclaimer.
    •   Redistributions in binary form must reproduce the above copyright notice, 
        this list of conditions and the following disclaimer in the documentation 
        and/or other materials provided with the distribution.
    •   None of the names of any campus of the University of California, the name 
        "The Regents of the University of California," or the names of any of its 
        contributors may be used to endorse or promote products derived from this 
        software without specific prior written permission.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
    IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
    OF SUCH DAMAGE.
 
    $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/draw_box_sats/XMLWriter.m $
    $Rev: 35335 $
    $Author: smaleschlijski@RADIOLOGY.UCSF.EDU $
    $Date: 2015-07-07 14:29:01 -0700 (Tue, 07 Jul 2015) $

    Authors:
        Jason C. Crane, Ph.D., 
        Beck Olson,
        Stojan Maleschlijski

%}

classdef XMLWriter < handle
     properties
         xmlObject;
         xmlRootObject;
         xmlPressBox;
         xmlAutoSATs;
     end

     methods
         function obj = XMLWriter()
         end
         
         function Initialize(obj)
             obj.xmlObject =com.mathworks.xml.XMLUtils.createDocument('svk_sat_bands');
             obj.xmlRootObject = obj.xmlObject.getDocumentElement();
             obj.xmlRootObject.setAttribute('xmlns', 'svkSatBands');
             obj.xmlRootObject.setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance')
             obj.xmlRootObject.setAttribute('xsi:schemaLocation', 'svkSatBands svkSatBands.xsd')
             obj.writeNOD('version', '', '', '2.0');
             obj.xmlPressBox = obj.writeNOD('press_box', '', '', '');
%             obj.writeSATBand('1', 'Test', '0', '1', '2', '3', '4', obj.xmlPressBox);
             obj.xmlAutoSATs = obj.writeNOD('auto_sats', '', '', '');
%              obj.writeSATBand('1', 'Sag_Anterior', '0', '1', '2', '3', '4', obj.xmlAutoSATs);
%              obj.writeSATBand('2', 'Sag_Posterior', '0', '1', '2', '3', '4', obj.xmlAutoSATs);
%              obj.printSelf
%              obj.updateSATBand('Sag_Anterior', '4', '3', '2', '1', '0', obj.xmlAutoSATs);
%              obj.printSelf
         end
         
         function product = writeNOD(obj, nodName, attrName, attrVal, val)
            product = obj.xmlObject.createElement(nodName);
            if (~strcmp(attrName,''))
                product.setAttribute(attrName, attrVal);
            end            
            if (~strcmp(val,''))
                product.appendChild(obj.xmlObject.createTextNode(val));
            end
            obj.xmlRootObject.appendChild(product);
         end
         
         function writeSATBand(obj, id, label, nx,ny,nz,thickness, d_origin, par)
             satBand = obj.xmlObject.createElement('sat_band');
             satBand.setAttribute('id', id);
             satBand.setAttribute('label', label);
             val = obj.writeSimple('normal_x',nx);
             satBand.appendChild(val);
             satBand.appendChild(obj.writeSimple('normal_y',ny));
             satBand.appendChild(obj.writeSimple('normal_z',nz));
             satBand.appendChild(obj.writeSimple('thickness',thickness));
             satBand.appendChild(obj.writeSimple('distance_from_origin',d_origin));
             par.appendChild(satBand);
             
         end
         function writeMultipleSATsinParent(obj, dataStruct, par)
            s = size(dataStruct,2);
            for el=1:s
                writeSATBand(obj, dataStruct(el).id, dataStruct(el).label, dataStruct(el).normal_x, ...
                            dataStruct(el).normal_y,dataStruct(el).normal_z,dataStruct(el).thickness, ...
                            dataStruct(el).distance_from_origin, par);
            end
         end
         
         function writePRESSBoxSatBands(obj, dataStruct)
             obj.writeMultipleSATsinParent(dataStruct, obj.xmlPressBox);
         end
         function writeAutoSatBands(obj, dataStruct)
             obj.writeMultipleSATsinParent(dataStruct, obj.xmlAutoSATs);
         end
         
         function simple = writeSimple(obj, name, val)
             simple = obj.xmlObject.createElement(name);
             simple.appendChild(obj.xmlObject.createTextNode(val));
         end
 
         function printSelf(obj)
             xmlwrite(obj.xmlObject)
         end
     end
     methods(Static)
        function updateSATBand(label, normal_x, normal_y, normal_z, thickness, distance_from_origin, par)
            childNodes = par.getChildNodes();
            numChildren = childNodes.getLength(); 
            for idx = 0:numChildren-1; 
                satChild = childNodes.item(idx);
                if (satChild.hasChildNodes())
                    if (satChild.getAttribute('label').strcmp(label))
                        allElements = satChild.getChildNodes();
                        nElements = allElements.getLength()
                        for i=0:nElements-1
                            currElement = allElements.item(i);
                            if (currElement.hasChildNodes())
                                found = 1;
                                currElement.setTextContent(eval(char(currElement.getNodeName())));
                            end
                        end
                    end 
                end
            end
          end
         
        function writeXMLFile(xmlObject, fileName)
            try
                xmlwrite(fileName, xmlObject);
            catch
                error('Failed to generate an XML file %s.', fileName);
            end
         end
     end
 end
