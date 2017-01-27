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

 
    function svk_write_xml_sats(xml_filename_in, xml_filename_out, xmlStruct)

        Function to write out a modified version of a SatBand XML file. 
        xml_filename_in:  this is the original template XML file. 
        xml_filename_out: this is the name of the file the modified XML will be written to. 
        xmlStruct:        this is the data structure representing the modified sat band information.  
                          It is obtained in unmodified form as the output from svk_parse_xml_sats. 

        EXAMPLE: 
        example usage for creating a modified xml file (modify the value of normal_x for pressBoxSat number 1):
            satBandXMLStruct = svk_parse_xml_sats('sat_bands.xml', 0);
            satBandXMLStruct.pressBoxSatsStruct(1).sat_band.normal_x = 123456789; 
            svk_write_xml_sats('sat_bands.xml', 'modified_sat_bands.xml', satBandXMLStruct);

 
    $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/svk_write_xml_sats.m $
    $Rev: 39349 $
    $Author: bolson@RADIOLOGY.UCSF.EDU $
    $Date: 2017-01-24 16:17:11 -0800 (Tue, 24 Jan 2017) $

    Authors:
        Jason C. Crane, Ph.D., 
        Beck Olson,
        Wei Bian

%}

function svk_write_xml_sats(xml_filename_in, xml_filename_out, xmlStruct)

    %   Read in template XML to create a DOMNode that will be modified and written
    %   out to xml_filename_out.  This is suitable since the purpose is to register
    %   the sat bands from the input XML file to the patient space and therefore the 
    %   number of sats will be the same.  
    try
        tree_in = xmlread(xml_filename_in);
    catch
        error('Failed to read XML file %s.', xml_filename_in);
    end

    %   
    sat_bands = tree_in.getDocumentElement; 
    parts = sat_bands.getChildNodes; 

    %   Get the structure for each node in the file (elements inside svk_sat_bands): 
    version   = parts.item(1).getChildNodes; 
    press_box = parts.item(3).getChildNodes; 
    auto_sats = parts.item(5).getChildNodes; 
    
    %
    %   Modify the press box and auto sat bands 
    %
    modify_sat_band_set(press_box, xmlStruct.pressBoxSatsStruct ); 
    modify_sat_band_set(auto_sats, xmlStruct.autoSatsStruct ); 

    % PARSEXML Convert XML file to a MATLAB structure.
    try
        xmlwrite(xml_filename_out, tree_in);
    catch
        error('Failed to read XML file %s.', xml_filename_out);
    end

return; 



%
%   Parse a single node that represents a sat band: 
%
%   returns a struct: 
%       sats_struct(i).sat_band_struct
%
function modify_sat_band_set( satBandSetNode, xmlStruct )

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
                    %   Pass the specific sub-structure/sat band to modify
                    modify_single_sat_band( child, xmlStruct(num_sat_bands) );
                end
            end
        end
    end
    return; 



%
%   Modify a single node that represents a sat band.  Input is the DOMNode to be modified, 
%   together with the dataStructure containing the new data to be used: 
%       satBandNode -> <sat_band label="Anteridor" id="1">
%                           <normal_x>0.0</normal_x>
%                           <normal_y>-0.9978</normal_y>
%                           <normal_z>0.0661</normal_z>
%                           <thickness>45.0</thickness>
%                           <distance_from_origin>57.5409</distance_from_origin>
%                       </sat_band>
%
%   input struct: 
%       sat_band_struct.normal_x
%       sat_band_struct.normal_y
%       sat_band_struct.normal_z
%       sat_band_struct.thickness
%       sat_band_struct.distance_from_origin
%
function modify_single_sat_band( satBandNode, satBandStruct )

    childNodes = satBandNode.getChildNodes;

    num_nodes = childNodes.getLength; 
    for count = 0:num_nodes-1; 
        if  childNodes.item(count).hasChildNodes
            sat_child  = childNodes.item(count).getChildNodes; 
            name = sat_child.getNodeName; 
            value = sat_child.getTextContent; 
            if name.strcmp('normal_x')
                sat_child.setTextContent( num2str(satBandStruct.sat_band.normal_x) ); 
            end
            if name.strcmp('normal_y')
                sat_child.setTextContent( num2str(satBandStruct.sat_band.normal_y) ); 
            end
            if name.strcmp('normal_z')
                sat_child.setTextContent( num2str(satBandStruct.sat_band.normal_z) ); 
            end
            if name.strcmp('thickness')
                sat_child.setTextContent( num2str(satBandStruct.sat_band.thickness) ); 
            end
            if name.strcmp('distance_from_origin')
                sat_child.setTextContent( num2str(satBandStruct.sat_band.distance_from_origin) ); 
            end
        end
    end

   return; 
    
