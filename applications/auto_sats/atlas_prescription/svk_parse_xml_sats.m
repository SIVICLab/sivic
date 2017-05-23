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

 
    function satsStruct = svk_parse_xml_sats(xml_filename, debug)

        This is a function to parse an svk sat band XML file and return a corresponding MATLAB data structure. 

        xml_filename: path to xml file representation of press box and auto sats. 
        debug. 0 or 1 to print structure results. 
 
        return:  satsStruct is a struct consisting of 2 elements: 
            satsStruct.pressBoxSatsStruct => press_box sats
            satsStruct.autoSatsStruct     => auto_sats
    
            satsStruct.pressBoxSatsStruct(i).sat_band.normal_x 
            satsStruct.pressBoxSatsStruct(i).sat_band.normal_y 
            satsStruct.pressBoxSatsStruct(i).sat_band.normal_z 
            satsStruct.pressBoxSatsStruct(i).sat_band.thickness        
            satsStruct.pressBoxSatsStruct(i).sat_band.distance_from_origin
    
            satsStruct.autoSatsStruct(i).sat_band.normal_x 
            satsStruct.autoSatsStruct(i).sat_band.normal_y 
            satsStruct.autoSatsStruct(i).sat_band.normal_z 
            satsStruct.autoSatsStruct(i).sat_band.thickness        
            satsStruct.autoSatsStruct(i).sat_band.distance_from_origin
 
 
        Each of these is in turn a struct array with each element representing a single sat band: 
 
            e.g., first press_box band: satsStruct.pressBoxSatsStruct(1) => first press_box sats
    
        Indidual sat bands params are a further sub-structure:     
 
            e.g., second press_box band parameters: 
                satsStruct.pressBoxSatsStruct(2).sat_band.normal_x
                satsStruct.pressBoxSatsStruct(2).sat_band.normal_y 
                satsStruct.pressBoxSatsStruct(2).sat_band.normal_z 
                ....
 
        EXAMPLE: 
            satBandXMLStruct = svk_parse_xml_sats('sat_bands.xml', 0);



    $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/svk_parse_xml_sats.m $
    $Rev: 39349 $
    $Author: bolson@RADIOLOGY.UCSF.EDU $
    $Date: 2017-01-24 16:17:11 -0800 (Tue, 24 Jan 2017) $

    Authors:
        Jason C. Crane, Ph.D.,
        Beck Olson,
        Wei Bian

%}


function satsStruct = svk_parse_xml_sats(xml_filename, debug)



    % PARSEXML Convert XML file to a MATLAB structure.
    try
        tree = xmlread(xml_filename);
    catch
        error('Failed to read XML file %s.', xml_filename);
    end


    sat_bands = tree.getDocumentElement; 
    parts = sat_bands.getChildNodes; 

    %   Get the structure for each node in the file (elements inside svk_sat_bands): 
    version   = parts.item(1).getChildNodes; 
    press_box = parts.item(3).getChildNodes; 
    auto_sats = parts.item(5).getChildNodes; 
    
    node_name_version   = version.getNodeName; 
    node_name_press_box = press_box.getNodeName; 
    node_name_auto_sats = auto_sats.getNodeName; 

    text_version   = str2num(version.getTextContent); 
    text_press_box = press_box.getTextContent; 
    text_auto_sats = auto_sats.getTextContent; 

    %
    %   First parse the version and verify it: 
    %
    xml_version_number = double(text_version); 
    if xml_version_number == 1 
        if (debug)
            disp(' ');  
            disp('sat_bands_xml version: 1'); 
        end
    else 
        disp('version not supported'); 
        return;
    end 
    
    %
    %   Parse the press box sat bands 
    %
    pressBoxSatsStruct = parse_sat_band_set(press_box); 
    autoSatsStruct     = parse_sat_band_set(auto_sats); 

    satsStruct.pressBoxSatsStruct = pressBoxSatsStruct; 
    satsStruct.autoSatsStruct     = autoSatsStruct; 

    if (debug) 
        disp(' ');  
        disp('PRESS BOX');  
        printSubStruct( satsStruct.pressBoxSatsStruct ); 
        disp(' ');  
        disp('AUTO SATS');  
        printSubStruct( satsStruct.autoSatsStruct ); 
        disp(' ');  
    end




return; 


function printSubStruct( satBandSet )

    for satNumber = 1:length(satBandSet)
        id          = char(satBandSet(satNumber).id); 
        label       = char(satBandSet(satNumber).label); 
        nx          = satBandSet(satNumber).sat_band.normal_x; 
        ny          = satBandSet(satNumber).sat_band.normal_y; 
        nz          = satBandSet(satNumber).sat_band.normal_z; 
        thickness   = satBandSet(satNumber).sat_band.thickness; 
        distance    = satBandSet(satNumber).sat_band.distance_from_origin; 
        sat_band = sprintf('%4s %20s %f %f %f %f %f', id, label, nx, ny, nz, thickness, distance); 
        disp(sat_band); 
    end

return; 


%
%   Parse a single node that represents a sat band: 
%
%   returns a struct: 
%       sats_struct(i).sat_band_struct
%
function sats_struct = parse_sat_band_set( satBandSetNode )

    num_sat_bands = 0; 
    sats = satBandSetNode.getChildNodes;
    sats_num_nodes = sats.getLength; 
    for count = 0:sats_num_nodes-1
        if  sats.item(count).hasChildNodes
            child = sats.item(count).getChildNodes; 
            if child.hasAttributes    
                num_sat_bands = num_sat_bands + 1; 
                numAttributes = child.getAttributes.getLength; 
                for att = 0:numAttributes-1
                    name = child.getAttributes.item(att).getName; 
                    value = child.getAttributes.item(att).getValue; 
                    if name.strcmp('id')
                        sats_struct(num_sat_bands).id = value; 
                    end
                    if name.strcmp('label')
                        sats_struct(num_sat_bands).label = value; 
                    end
                    sat_band_struct = parse_single_sat_band( child );
                    sats_struct(num_sat_bands).sat_band = sat_band_struct; 
                end
            end
        end
    end
    return; 



%
%   Parse a single node that represents a sat band: 
%    satBandNode ->     <sat_band label="Anteridor" id="1">
%                           <normal_x>0.0</normal_x>
%                           <normal_y>-0.9978</normal_y>
%                           <normal_z>0.0661</normal_z>
%                           <thickness>45.0</thickness>
%                           <distance_from_origin>57.5409</distance_from_origin>
%                       </sat_band>
%
%   returns a struct: 
%       sat_band_struct.normal_x
%       sat_band_struct.normal_y
%       sat_band_struct.normal_z
%       sat_band_struct.thickness
%       sat_band_struct.distance_from_origin
%
function sat_band_struct = parse_single_sat_band( satBandNode )

    childNodes = satBandNode.getChildNodes;

    num_nodes = childNodes.getLength; 
    for count = 0:num_nodes-1; 
        if  childNodes.item(count).hasChildNodes
            sat_child  = childNodes.item(count).getChildNodes; 
            name = sat_child.getNodeName; 
            value = sat_child.getTextContent; 
            if name.strcmp('normal_x')
                sat_band_struct.normal_x = str2num(value); 
            end
            if name.strcmp('normal_y')
                sat_band_struct.normal_y = str2num(value); 
            end
            if name.strcmp('normal_z')
                sat_band_struct.normal_z = str2num(value); 
            end
            if name.strcmp('thickness')
                sat_band_struct.thickness = str2num(value); 
            end
            if name.strcmp('distance_from_origin')
                sat_band_struct.distance_from_origin = str2num(value); 
            end
        end
    end

   return; 
    
