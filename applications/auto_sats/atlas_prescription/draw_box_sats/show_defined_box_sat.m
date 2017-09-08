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
function show_defined_box_sat
clc, clear
% This function is used to display user defined press box and sat bands on
% the atlas images where those prescription is given. An option is made availible
% for displaying 10 Octagonal sat bands as well

% Wei Bian Created in August,2014
% Sarah Nelson's Lab
% Department of Radiology UCSF

curdir = pwd;
[fName, fPath] = uigetfile('*.nii', 'Select the Atlas Volume Image');
if fPath==0
    msgbox('Must select an Atlas Volume Image', 'File Selector', 'error');
    return
end
cd(fPath);

NII_IM = load_nii(fName);
img = NII_IM.img;
img = flipdim(img,3);
img = flipdim(img,2);
img = permute(img, [2 1 3]);
img = flipdim(img, 2);
img = double(img);
[nrow ncol nslice] = size(img);
pixelSize_atlas = NII_IM.hdr.dime.pixdim(2:4);
qform_atlas = [NII_IM.hdr.hist.srow_x; NII_IM.hdr.hist.srow_y; NII_IM.hdr.hist.srow_z; 0 0 0 1];
if qform_atlas(1,1) == 1
   qform_atlas(1,1) = -1;
   qform_atlas(1,4) = -qform_atlas(1,4)-1;
end
maxDim = max([nrow ncol nslice]);
VOL = [(0:maxDim-1); (0:maxDim-1); (0:maxDim-1); ones(1,maxDim)];
RAS = qform_atlas*VOL;
x_axis = -RAS(1,1:ncol); % Now positive is L
y_axis = fliplr(-RAS(2,1:nrow)); % Now positive is P
z_axis = fliplr(RAS(3,1:nslice)); 

[fXML, fPath] = uigetfile('*.xml', 'Select the XML file');
if fPath==0
    msgbox('Must select an XML file', 'File Selector', 'error');
    return
end
cd(fPath);
[ver, box, satbands, boxsatstruct] = readBoxSatInfo(fXML);

choice = questdlg('Display OCT Sat Bands?', 'Sat Band Display', 'Yes', 'No', 'No');
if strcmp(choice,'Yes')
    if (strcmp(ver,'2.0'))
        satBands4Disp = addOctSats20( box, satbands);
    else
        satBands4Disp = addOctSats(box, satbands);        
    end
    satbands = satBands4Disp;
end


% if strcmp(choice,'Yes')
%     nSat = length(satbands);
%     octSatThickness = 40;
% %     nSat = 0;
% %     satbands = [];
%     % compute 6 sat bands that touch 6 box faces
%     for k = 1:6
%         satbands(nSat+k).normal = box(k).normal;
%         satbands(nSat+k).distance = box(k).distance+(octSatThickness/2); %*sign(box(k).distance);
%         satbands(nSat+k).thickness = octSatThickness;
%     end
%     
%     % compute 4 sat bands at corners
%     normalX = box(5).normal;
%     normalY = box(2).normal;
%     normalZ = box(3).normal;
%     R = [normalX normalY normalZ]; % rotation matrix
%     
%     if box(5).normal(1) ~= box(6).normal(1)
%         centerL = (box(5).distance -box(6).distance)/2;
%         sizeX = box(5).distance + box(6).distance;
%     else
%         centerL = (box(5).distance + box(6).distance)/2;
%         sizeX = abs(box(5).distance - box(6).distance);
%     end
%     
%     if box(1).normal(2) ~= box(2).normal(2)
%         sizeY = box(1).distance + box(2).distance;
%         centerP = (box(2).distance - box(1).distance)/2;
%     else
%         sizeY = abs(box(1).distance - box(2).distance);
%         centerP = (box(2).distance + box(1).distance)/2;
%     end
%     
%     pt_aRight = [centerL-sizeX/2 centerP-sizeY/2 0]'; % the center plane passes the corner vertex
%     pt_pRight = [centerL-sizeX/2 centerP+sizeY/2 0]';
%     pt_aLeft = [centerL+sizeX/2 centerP-sizeY/2 0]';
%     pt_pLeft = [centerL+sizeX/2 centerP+sizeY/2 0]';
%     
%     normal_aRight = [-sizeY/2 -sizeX/2  0]'; 
%     normal_aRight = normal_aRight/norm(normal_aRight); % the normal vector of the sat band before rotation
%     normal_pRight = [sizeY/2 -sizeX/2  0]';
%     normal_pRight = normal_pRight/norm(normal_pRight);
%     normal_aLeft = [-sizeY/2 sizeX/2  0]';
%     normal_aLeft = normal_aLeft/norm(normal_aLeft);
%     normal_pLeft = [sizeY/2 sizeX/2  0]';
%     normal_pLeft = normal_pLeft/norm(normal_pLeft);
%     
%     pt_aRight = R*pt_aRight; % rotate the point
%     normal_aRight = R*normal_aRight; % rotate the normal vector
%     satbands(nSat+7).normal = normal_aRight;
%     satbands(nSat+7).distance = dot(pt_aRight, normal_aRight);
%     satbands(nSat+7).thickness = octSatThickness;
%     
%     pt_pRight = R*pt_pRight;
%     normal_pRight = R*normal_pRight;
%     satbands(nSat+8).normal = normal_pRight;
%     satbands(nSat+8).distance = dot(pt_pRight, normal_pRight);
%     satbands(nSat+8).thickness = octSatThickness;
%     
%     pt_aLeft = R*pt_aLeft;
%     normal_aLeft = R*normal_aLeft;
%     satbands(nSat+9).normal = normal_aLeft;
%     satbands(nSat+9).distance = dot(pt_aLeft, normal_aLeft);
%     satbands(nSat+9).thickness = octSatThickness;
%     
%     pt_pLeft = R*pt_pLeft;
%     normal_pLeft = R*normal_pLeft;
%     satbands(nSat+10).normal = normal_pLeft;
%     satbands(nSat+10).distance = dot(pt_pLeft, normal_pLeft);
%     satbands(nSat+10).thickness = octSatThickness;
% end

if pixelSize_atlas < 1
    pixelSize_atlas(1:2) = pixelSize_atlas(1:2).*2;
    pixelSize_atlas(3) = pixelSize_atlas(3);
    display_box_satbands(ver, img(1:2:end, 1:2:end, :), box, satbands, pixelSize_atlas, x_axis(1:2:end), y_axis(1:2:end), z_axis);
else
    display_atlas_box_satbands(ver, img, box, satbands, pixelSize_atlas, x_axis, y_axis, z_axis);
end

cd(curdir);
