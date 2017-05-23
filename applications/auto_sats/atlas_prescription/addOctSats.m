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

function satBandsWithOct = addOctSats(mbox, satBands)

nSat = length(satBands);
octSatThickness = 40;
% compute 6 sat bands that touch 6 mbox faces
% nSat = 0; % for diagnosis only if you just want to see OCT Sat Bands
% satBands = []
for k = 1:6
    satBands(nSat+k).normal = mbox(k).normal;
    %satBands(nSat+k).distance = mbox(k).distance+(octSatThickness/2); % bug fixed see below
    satBands(nSat+k).thickness = octSatThickness;
end

% Very messy code, but works
if mbox(1).normal(2) < 0
    satBands(nSat+1).distance = mbox(1).distance+(octSatThickness/2);
else
    satBands(nSat+1).distance = mbox(1).distance-(octSatThickness/2);
end
if mbox(2).normal(2) > 0
    satBands(nSat+2).distance = mbox(2).distance+(octSatThickness/2);
else
    satBands(nSat+2).distance = mbox(2).distance-(octSatThickness/2);
end

if mbox(3).normal(3) > 0
    satBands(nSat+3).distance = mbox(3).distance+(octSatThickness/2);
else
    satBands(nSat+3).distance = mbox(3).distance-(octSatThickness/2);
end
if mbox(4).normal(3) < 0
    satBands(nSat+4).distance = mbox(4).distance+(octSatThickness/2);
else
    satBands(nSat+4).distance = mbox(4).distance-(octSatThickness/2);
end

if mbox(5).normal(1) > 0
    satBands(nSat+5).distance = mbox(5).distance+(octSatThickness/2);
else
    satBands(nSat+5).distance = mbox(5).distance-(octSatThickness/2);
end
if mbox(6).normal(1) < 0
    satBands(nSat+6).distance = mbox(6).distance+(octSatThickness/2);
else
    satBands(nSat+6).distance = mbox(6).distance-(octSatThickness/2);
end

% compute 4 sat bands at corners
normalX = mbox(5).normal;
normalY = mbox(2).normal;
normalZ = mbox(3).normal;
R = [normalX normalY normalZ]; % rotation matrix

if mbox(5).normal(1) ~= mbox(6).normal(1)
    centerL = (mbox(5).distance -mbox(6).distance)/2;
    sizeX = mbox(5).distance + mbox(6).distance;
else
    centerL = (mbox(5).distance + mbox(6).distance)/2;
    sizeX = abs(mbox(5).distance - mbox(6).distance);
end

if mbox(1).normal(2) ~= mbox(2).normal(2)
    sizeY = mbox(1).distance + mbox(2).distance;
    centerP = (mbox(2).distance - mbox(1).distance)/2;
else
    sizeY = abs(mbox(1).distance - mbox(2).distance);
    centerP = (mbox(2).distance + mbox(1).distance)/2;
end

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
normal_pLeft = normal_pLeft/norm(normal_pLeft);

pt_aRight = R*pt_aRight; % rotate the point
normal_aRight = R*normal_aRight; % rotate the normal vector
satBands(nSat+7).normal = normal_aRight;
satBands(nSat+7).distance = dot(pt_aRight, normal_aRight);
satBands(nSat+7).thickness = octSatThickness;

pt_pRight = R*pt_pRight;
normal_pRight = R*normal_pRight;
satBands(nSat+8).normal = normal_pRight;
satBands(nSat+8).distance = dot(pt_pRight, normal_pRight);
satBands(nSat+8).thickness = octSatThickness;

pt_aLeft = R*pt_aLeft;
normal_aLeft = R*normal_aLeft;
satBands(nSat+9).normal = normal_aLeft;
satBands(nSat+9).distance = dot(pt_aLeft, normal_aLeft);
satBands(nSat+9).thickness = octSatThickness;

pt_pLeft = R*pt_pLeft;
normal_pLeft = R*normal_pLeft;
satBands(nSat+10).normal = normal_pLeft;
satBands(nSat+10).distance = dot(pt_pLeft, normal_pLeft);
satBands(nSat+10).thickness = octSatThickness;



satBandsWithOct = satBands;
