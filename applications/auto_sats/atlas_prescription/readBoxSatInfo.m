%{  
    Copyright © 2016-2017 The Regents of the University of California.
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
%}
function [ver, boxPlane_atlas satBand_atlas BoxSatXMLStruct] = readBoxSatInfo(xml_filename)

r=XMLReader(xml_filename);
ver = r.version;
if (strcmp(ver, '2.0'))
    boxPlane_atlas  = XMLReader.convertXMLStructToNum(r.getPRESSBoxSatBands());
    for i=1:3
    boxPlane_atlas(i).normal = [boxPlane_atlas(i).normal_x;...
                                boxPlane_atlas(i).normal_y;...
                                boxPlane_atlas(i).normal_z];
    boxPlane_atlas(i).distance = boxPlane_atlas(i).distance_from_origin;                       
    end
    satBand_atlas   = XMLReader.convertXMLStructToNum(r.getAutoSatBands());
    for i=1:size(satBand_atlas,2)
    satBand_atlas(i).normal = [satBand_atlas(i).normal_x;...
                               satBand_atlas(i).normal_y;...
                               satBand_atlas(i).normal_z];
    satBand_atlas(i).distance = satBand_atlas(i).distance_from_origin;                           
    end
    BoxSatXMLStruct = XMLReader.convertXMLStructToNum(r.getPRESSandSATs());
else
    
    boxPlane_atlas = struct('normal', {}, 'distance', {}, 'thickness', {});
    satBand_atlas = struct('normal', {}, 'distance', {}, 'thickness', {});

    BoxSatXMLStruct = svk_parse_xml_sats(xml_filename,0);

    for satNumber = 1:length(BoxSatXMLStruct.pressBoxSatsStruct)
        boxPlane_atlas(satNumber).normal = [BoxSatXMLStruct.pressBoxSatsStruct(satNumber).sat_band.normal_x; ...
                                            BoxSatXMLStruct.pressBoxSatsStruct(satNumber).sat_band.normal_y; ...
                                            BoxSatXMLStruct.pressBoxSatsStruct(satNumber).sat_band.normal_z]; 
        boxPlane_atlas(satNumber).distance = BoxSatXMLStruct.pressBoxSatsStruct(satNumber).sat_band.distance_from_origin; 
        boxPlane_atlas(satNumber).thickness = BoxSatXMLStruct.pressBoxSatsStruct(satNumber).sat_band.thickness;
    end

    nSat = 1;
    for satNumber = 1:length(BoxSatXMLStruct.autoSatsStruct)
        if BoxSatXMLStruct.autoSatsStruct(satNumber).sat_band.thickness ~= 0
            satBand_atlas(nSat).normal = [BoxSatXMLStruct.autoSatsStruct(satNumber).sat_band.normal_x; ...
                                               BoxSatXMLStruct.autoSatsStruct(satNumber).sat_band.normal_y; ...
                                               BoxSatXMLStruct.autoSatsStruct(satNumber).sat_band.normal_z];
            satBand_atlas(nSat).distance = BoxSatXMLStruct.autoSatsStruct(satNumber).sat_band.distance_from_origin;
            satBand_atlas(nSat).thickness = BoxSatXMLStruct.autoSatsStruct(satNumber).sat_band.thickness;
            nSat = nSat + 1;
        end
    end
end
