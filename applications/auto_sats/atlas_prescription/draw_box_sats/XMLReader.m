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
 
    $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/draw_box_sats/XMLReader.m $
    $Rev: 39349 $
    $Author: bolson@RADIOLOGY.UCSF.EDU $
    $Date: 2017-01-24 16:17:11 -0800 (Tue, 24 Jan 2017) $

    Authors:
        Jason C. Crane, Ph.D., 
        Beck Olson,
        Stojan Maleschlijski

%}

classdef XMLReader < handle
     properties
         fileName
         version
         xmlRootObject;
         xmlObject;
         xmlPressBox;
         xmlAutoSATs;
     end
     methods
         function obj = XMLReader(fileName)
           obj.fileName = fileName;
           try
                obj.xmlObject = xmlread(fileName);
           catch
                error('Failed to read XML file %s.', fileName);
           end
            obj.xmlRootObject = obj.xmlObject.getDocumentElement(); % svk_sat_bands
            obj.xmlPressBox = obj.xmlRootObject.getElementsByTagName('press_box').item(0);
            obj.xmlAutoSATs = obj.xmlRootObject.getElementsByTagName('auto_sats').item(0);
            
            verElement = obj.xmlRootObject.getElementsByTagName('version');
            obj.version = verElement.item(0).getTextContent();
            
            %TODO Add some version handling...
         end
         
         function result = readSimpleSATBand(obj, parent, label)
            childNodes = parent.getChildNodes();
            numChildren = childNodes.getLength(); 
            for idx = 0:numChildren-1; 
                satChild = childNodes.item(idx);
                if (satChild.getAttribute('label').strcmp(label))
                    allElements = satChild.getChildNodes();
                    nElements = allElements.getLength()
                    for i=0:nElements-1
                        currElement = allElements.item(i);
                        currentElementName = char(currElement.getNodeName());
                        eval(['result.' currentElementName]) = currElement.getTextContent();
                    end
                end         
            end
         end
         
         function pressBoxStruct = getPRESSBoxSatBands(obj)
                pressBoxStruct = obj.getAllSatBandsInParent(obj.xmlPressBox);
         end
         
         function autoSatsStruct = getAutoSatBands(obj)
                autoSatsStruct = obj.getAllSatBandsInParent(obj.xmlAutoSATs);             
         end
         
         function PRESSandSatsStruct = getPRESSandSATs(obj)
                press = obj.getPRESSBoxSatBands();
                sats =  obj.getAutoSatBands();
                sizeP = size(press,2);
                sizeS = size(sats,2);
                for (i=1:sizeP)
                    PRESSandSatsStruct(i) = press(i);
                end
                offset = sizeP;
                for (i=sizeP+1:sizeP+sizeS)
                    PRESSandSatsStruct(i) = sats(i-sizeP);
                end
         end
         
         function xmlObject = getXMLObjectForWriting(obj)
                xmlObject = obj.xmlObject;
         end
 
         function dataStruct = getAllSatBandsInParent(obj, parentNode)
             satBands = parentNode.getChildNodes();
             numSatBands = satBands.getLength();
             dataStruct = XMLReader.createEmptyDataStruct(1);
             nonEmptyElements = 1;
             for idx = 0:numSatBands-1; 
                satChild = satBands.item(idx);
                if (satChild.hasChildNodes())
                    dataStruct(nonEmptyElements).id    = satChild.getAttribute('id');
                    dataStruct(nonEmptyElements).label = satChild.getAttribute('label');                
                    
                    allElements = satChild.getChildNodes();
                    nElements = allElements.getLength();
                    for i=0:nElements-1
                        currElement = allElements.item(i);
                        if (currElement.hasChildNodes())
                            currentElementName = char(currElement.getNodeName());
                            eval(char(strcat('dataStruct(nonEmptyElements).', currentElementName, '= currElement.getTextContent();')));                      
                        end
                    end
                    nonEmptyElements = nonEmptyElements + 1;
                end
            end
         end
     end
     
     methods(Static)
        function CASorderedStruct = orderCAS(xmlStruct)
             CASorderedStruct = xmlStruct;
             for (i=1:size(xmlStruct,2))
                if (xmlStruct(i).label == 'sagittal')
                    CASorderedStruct(3) = xmlStruct(i);
                end
                if (xmlStruct(i).label == 'coronal')
                    CASorderedStruct(1) = xmlStruct(i);
                end
                if (xmlStruct(i).label == 'axial')
                    CASorderedStruct(2) = xmlStruct(i);
                end                
             end
        end
        function nStruct = convertXMLStructToNum(xmlStruct)
            nStruct = xmlStruct;
            for (i=1:size(xmlStruct,2))
                nStruct(i).id                      = str2double(xmlStruct(i).id);
                nStruct(i).normal_x                = str2double(xmlStruct(i).normal_x);
                nStruct(i).normal_y                = str2double(xmlStruct(i).normal_y);
                nStruct(i).normal_z                = str2double(xmlStruct(i).normal_z);
                nStruct(i).thickness               = str2double(xmlStruct(i).thickness);
                nStruct(i).distance_from_origin    = str2double(xmlStruct(i).distance_from_origin);
            end
        end
        function xmlStruct = convertNumStructToXML(numStruct)
            xmlStruct = numStruct;
            for (i = 1:size(numStruct,2))
                xmlStruct(i).id                      = num2str(numStruct(i).id);
                xmlStruct(i).normal_x                = num2str(numStruct(i).normal_x);
                xmlStruct(i).normal_y                = num2str(numStruct(i).normal_y);
                xmlStruct(i).normal_z                = num2str(numStruct(i).normal_z);
                xmlStruct(i).thickness               = num2str(numStruct(i).thickness);
                xmlStruct(i).distance_from_origin    = num2str(numStruct(i).distance_from_origin);   
            end
        end
        function emptyStruct = createEmptyDataStruct(structSize)
            if (structSize~=0)
                emptyStruct(structSize).id ='';
                emptyStruct(structSize).label ='';

                emptyStruct(structSize).normal_x ='';
                emptyStruct(structSize).normal_y ='';
                emptyStruct(structSize).normal_z ='';
                emptyStruct(structSize).thickness ='';
                emptyStruct(structSize).distance_from_origin ='';
            else
                emptyStruct = [];
            end
        end
        function printDataStruct(dataStruct)
             sizeStruct = size(dataStruct,2);
             for i=1:sizeStruct
                currStruct = dataStruct(i);
                structNames = fieldnames(dataStruct);
                for nElement = 1:size(structNames,1)
                    eval(char(strcat('strName = currStruct.', structNames(nElement), ';')));
                    disp(strcat(char(structNames(nElement)), ':', char(strName)))
                end
                
             end
        end
         function printSelf(xmlObject)
             xmlwrite(xmlObject)
         end
     end
         
end



















