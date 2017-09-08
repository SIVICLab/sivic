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
function satBandsWithOct = addOctSats20(mbox, satBands)
% TODOSM
% Adapt for corner 4 sat bands

nSat = length(satBands);
octSatThickness = 40;
% compute 6 sat bands that touch 6 mbox faces
% nSat = 0; % for diagnosis only if you just want to see OCT Sat Bands
% satBands = []    
j = 1;
for k = 1:2:6
    satBands(nSat+k).id = nSat+k;
    satBands(nSat+k).label = ['OCTSat_' char(mbox(j).label) '_1'];
    satBands(nSat+k).normal_x = mbox(j).normal_x;
    satBands(nSat+k).normal_y = mbox(j).normal_y;
    satBands(nSat+k).normal_z = mbox(j).normal_z;
    satBands(nSat+k).thickness = octSatThickness;
    satBands(nSat+k).distance_from_origin = mbox(j).distance_from_origin - mbox(j).thickness/2 -(octSatThickness/2);
    
    satBands(nSat+k+1).id = nSat+k+1;    
    satBands(nSat+k+1).label = ['OCTSat_' char(mbox(j).label) '_2'];    
    satBands(nSat+k+1).normal_x = mbox(j).normal_x;
    satBands(nSat+k+1).normal_y = mbox(j).normal_y;
    satBands(nSat+k+1).normal_z = mbox(j).normal_z;
    satBands(nSat+k+1).thickness = octSatThickness;
    satBands(nSat+k+1).distance_from_origin = mbox(j).distance_from_origin + mbox(j).thickness/2 +(octSatThickness/2);
    j = j+1;
end

normalX = [mbox(1).normal_x; mbox(1).normal_y; mbox(1).normal_z];
normalY = [mbox(2).normal_x; mbox(2).normal_y; mbox(2).normal_z];
normalZ = [mbox(3).normal_x; mbox(3).normal_y; mbox(3).normal_z];

R = [normalX normalY normalZ];


sizeX = mbox(1).thickness;
sizeY = mbox(2).thickness;
centerL = mbox(1).distance_from_origin;
centerP = mbox(2).distance_from_origin;

pt_aRight = [centerL-sizeX/2 centerP-sizeY/2 0]'; % the center plane passes the corner vertex
pt_pRight = [centerL-sizeX/2 centerP+sizeY/2 0]';
pt_aLeft = [centerL+sizeX/2 centerP-sizeY/2 0]';
pt_pLeft = [centerL+sizeX/2 centerP+sizeY/2 0]';

normal_aRight = [-sizeX/2 -sizeY/2 0]';
normal_aRight = normal_aRight/norm(normal_aRight); % the normal vector of the sat band before rotation
normal_pRight = [-sizeX/2 sizeY/2 0]';
normal_pRight = normal_pRight/norm(normal_pRight);
normal_aLeft = [sizeX/2 -sizeY/2 0]';
normal_aLeft = normal_aLeft/norm(normal_aLeft);
normal_pLeft = [sizeX/2 sizeY/2 0]';
normal_pLeft = normal_pLeft/norm(normal_pLeft)


nSat = length(satBands);         
for k = 1:nSat
    if (strfind(satBands(k).label, 'sagittal_1'))
        satBands(length(satBands)+1) = satBands(k);
        satBands(length(satBands)).label = 'OCTSat_sag_m45';
        pt_pRight = R*pt_pRight;
        normal_pRight = R*normal_pRight;
        satBands(length(satBands)).normal_x = normal_pRight(1);
        satBands(length(satBands)).normal_y = normal_pRight(2);
        satBands(length(satBands)).normal_z = normal_pRight(3);        
        satBands(length(satBands)).distance_from_origin = dot(pt_pRight, normal_pRight);
    end
    if (strfind(satBands(k).label, 'sagittal_2'))
        satBands(length(satBands)+1) = satBands(k);
        satBands(length(satBands)).label = 'OCTSat_sag_p45'; 
        pt_aLeft = R*pt_aLeft;
        normal_aLeft = R*normal_aLeft;
        satBands(length(satBands)).normal_x = normal_aLeft(1);
        satBands(length(satBands)).normal_y = normal_aLeft(2);
        satBands(length(satBands)).normal_z = normal_aLeft(3);        
        satBands(length(satBands)).distance_from_origin = dot(pt_aLeft, normal_aLeft);
    end
    
    if (strfind(satBands(k).label, 'coronal_1'))
        satBands(length(satBands)+1) = satBands(k);
        satBands(length(satBands)).label = 'OCTSat_cor_m45'; 
        
        pt_aRight = R*pt_aRight; % rotate the point
        normal_aRight = R*normal_aRight; % rotate the normal vector
        
        satBands(length(satBands)).normal_x = normal_aRight(1);
        satBands(length(satBands)).normal_y = normal_aRight(2);
        satBands(length(satBands)).normal_z = normal_aRight(3);        
        satBands(length(satBands)).distance_from_origin = dot(pt_aRight, normal_aRight);
             
    end
    if (strfind(satBands(k).label, 'coronal_2'))
        satBands(length(satBands)+1) = satBands(k);
        satBands(length(satBands)).label = 'OCTSat_cor_p45'; 
        
        pt_pLeft = R*pt_pLeft;
        normal_pLeft = R*normal_pLeft;
        satBands(length(satBands)).normal_x = normal_pLeft(1);
        satBands(length(satBands)).normal_y = normal_pLeft(2);
        satBands(length(satBands)).normal_z = normal_pLeft(3);        
        satBands(length(satBands)).distance_from_origin = dot(pt_pLeft, normal_pLeft);
    end    
end    

satBandsWithOct = satBands;

