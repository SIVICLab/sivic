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

    $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/applyTransMatrix.m $
    $Rev: 39349 $
    $Author: bolson@RADIOLOGY.UCSF.EDU $
    $Date: 2017-01-24 16:17:11 -0800 (Tue, 24 Jan 2017) $

    Authors:
        Jason C. Crane, Ph.D., 
        Beck Olson,
        Wei Bian,
        Stojan Maleschlijski

%}


% README: This function is inherited from Wei. It applies the tranformation
% matrix to the press box and sat bands. It handles 3 and 6 plane versions
% differently. In addition, the funciton outputs a .dat file which can be
% forwarded to the sscanner directly or compared with the output of the
% PSD. In addition its main output is the XML file which is used as an
% input by the psd and converted to .DAT. 
% In the case of 3 plane, the function transforms the normals of each of
% the 3 planes defining the box and updates the distance to origin.
% Furthermore, the size of the box is scaled according to the transform
% matrix using QR decomposition.
% SM, Feb 2016

function [boxPlane_patient, satBand_patient, boxDatName, satDatName] = applyTransMatrix(version, xml_filename, boxPlane_atlas, satBand_atlas, handles)

% Transform the normal vector of each plane of the predefined PRESS box to
% the patient space

boxPlane_patient = boxPlane_atlas;

if (strcmp(version, '2.0'))
    for k = 1:3
        [temp, boxPlane_patient(k).distance_from_origin] = ...
                                                    atlas2patient(  [boxPlane_atlas(k).normal_x, ...
                                                                    boxPlane_atlas(k).normal_y, ...
                                                                    boxPlane_atlas(k).normal_z]', ...
                                                                    boxPlane_atlas(k).distance_from_origin, ...
                                                                    handles.pixelSize_atlas, ...
                                                                    handles.pixelSize_patient, ...
                                                                    handles.qform_atlas, ...
                                                                    handles.qform_patient, ...
                                                                    handles.tmatrix);
        
        boxPlane_patient(k).normal_x = temp(1);
        boxPlane_patient(k).normal_y = temp(2);
        boxPlane_patient(k).normal_z = temp(3);
        
        boxPlane_patient(k).normal = temp;

    end
else
    for k = 1:6 
        [boxPlane_patient(k).normal boxPlane_patient(k).distance] = atlas2patient(boxPlane_atlas(k).normal, boxPlane_atlas(k).distance, handles.pixelSize_atlas, handles.pixelSize_patient, handles.qform_atlas, handles.qform_patient, handles.tmatrix); 
    end
%   Sort the 6 box elements according to the psd required convention:
%       *          1 => most Anterior
%       *          2 => most Posterior 
%       *          3 => most Superior  
%       *          4 => most Inferior  
%       *          5 => most Left  
%       *          6 => most Right 
%   LPS normals: 
    posteriorLPSNorm = [ 0,  1,  0]; %2
    superiorLPSNorm  = [ 0,  0,  1]; %3
    leftLPSNorm      = [ 1,  0,  0]; %5
    rightLPSNorm     = [-1,  0,  0]; %6
    anteriorLPSNorm  = [ 0, -1,  0]; %1
    inferiorLPSNorm  = [ 0,  0, -1]; %4
%   pressBoxFace 2 should have maximum projection along posteriorLPSNorm
%   pressBoxFace 3 should have maximum projection along superiorLPSNorm
%   pressBoxFace 5 should have maximum projection along leftLPSNorm
    maxPIndex = 1;  
    maxSIndex = 1;  
    maxRIndex = 1;  
    maxAIndex = 1;  
    maxIIndex = 1;  
    maxLIndex = 1;  
    maxDotP = 0; 
    maxDotS = 0; 
    maxDotR = 0; 
    maxDotA = 0; 
    maxDotI = 0; 
    maxDotL = 0; 


% trying to identify which plane is which face of the prism (since labels
% are not present). Not needed currently
    for k = 1:6

        %   multiply by the distance to get the correct directionality along the normal (e.g. R vs L)
        dotP = dot( posteriorLPSNorm, boxPlane_patient(k).normal ) 
        %dotP = boxPlane_patient(k).distance * dot( posteriorLPSNorm, boxPlane_patient(k).normal ) 

        dotS = dot( superiorLPSNorm,  boxPlane_patient(k).normal ) 
        %dotS = boxPlane_patient(k).distance * dot( superiorLPSNorm,  boxPlane_patient(k).normal ) 

        dotR = dot( rightLPSNorm,     boxPlane_patient(k).normal ) 
        %dotR = boxPlane_patient(k).distance * dot( rightLPSNorm,     boxPlane_patient(k).normal ) 

        dotA = dot( anteriorLPSNorm,  boxPlane_patient(k).normal ) 
        %dotA = boxPlane_patient(k).distance * dot( anteriorLPSNorm,  boxPlane_patient(k).normal ) 

        dotI = dot( inferiorLPSNorm,  boxPlane_patient(k).normal ) 
        %dotI = boxPlane_patient(k).distance * dot( inferiorLPSNorm,  boxPlane_patient(k).normal ) 

        dotL = dot( leftLPSNorm,      boxPlane_patient(k).normal ) 
        %dotL = boxPlane_patient(k).distance * dot( leftLPSNorm,      boxPlane_patient(k).normal ) 

        if ( dotP > maxDotP ) 
            maxDotP = dotP; 
            maxPIndex = k; 
        end

        if ( dotS > maxDotS ) 
            maxDotS = dotS; 
            maxSIndex = k; 
        end

        if ( dotR > maxDotR ) 
            maxDotR = dotR; 
            maxRIndex = k; 
        end

        if ( dotA > maxDotA ) 
            maxDotA = dotA; 
            maxAIndex = k; 
        end

        if ( dotI > maxDotI ) 
            maxDotI = dotI; 
            maxIIndex = k; 
        end 

        if ( dotL > maxDotL ) 
            maxDotL = dotL; 
            maxLIndex = k; 
        end
    end
% maxPIndex
% maxSIndex
% maxLIndex
% maxAIndex
% maxIIndex
% maxRIndex

% % Now order the 6 faces into boxPlane_patient_tmp
    boxPlane_patient_tmp(2) = boxPlane_patient(maxPIndex); 
    boxPlane_patient_tmp(3) = boxPlane_patient(maxSIndex); 
    boxPlane_patient_tmp(5) = boxPlane_patient(maxLIndex); 
    boxPlane_patient_tmp(1) = boxPlane_patient(maxAIndex); 
    boxPlane_patient_tmp(4) = boxPlane_patient(maxIIndex); 
    boxPlane_patient_tmp(6) = boxPlane_patient(maxRIndex); 

    for k = 1:6
        boxPlane_patient(k) = boxPlane_patient_tmp(k);
    end
end




if (strcmp(version, '2.0'))
    % orthogonize the transformed normal vector (keep normal vetors in SI directions unchanged)
    for (k=1:3)
        if (strcmp(boxPlane_patient(k).label, 'axial'))
            idxAxial = k;
        end
        if (strcmp(boxPlane_patient(k).label, 'coronal'))
            idxCoronal = k;
        end
        if (strcmp(boxPlane_patient(k).label, 'sagittal'))
            idxSaggital = k;
        end    
    end

    normalZ = [boxPlane_patient(idxAxial).normal_x, boxPlane_patient(idxAxial).normal_y, boxPlane_patient(idxAxial).normal_z];
    normalZ = normalZ';
    
    normalX = [boxPlane_patient(idxSaggital).normal_x, boxPlane_patient(idxSaggital).normal_y, boxPlane_patient(idxSaggital).normal_z];
    normalX = normalX';
    
    normalY = [boxPlane_patient(idxCoronal).normal_x, boxPlane_patient(idxCoronal).normal_y, boxPlane_patient(idxCoronal).normal_z];
    normalY = normalY'; 
else
    % orthoganize the transformed normal vector (keep normal vetors in SI directions unchanged) 
    if boxPlane_patient(3).normal(3) > 0 
        normalZ = boxPlane_patient(3).normal; 
    else 
        normalZ = -boxPlane_patient(3).normal; 
    end 

    if boxPlane_patient(2).normal(2) > 0 
    normalX = -cross(normalZ, boxPlane_patient(2).normal); 
    else 
        normalX = -cross(normalZ, -boxPlane_patient(2).normal); 
    end 
    normalY = cross(normalZ, normalX); 



% % There is probaly a much simpler way to do followings
    if boxPlane_patient(1).normal(2) > 0
        boxPlane_patient(1).normal = normalY;
    else
        boxPlane_patient(1).normal = -normalY;
    end
    if boxPlane_patient(2).normal(2) > 0
        boxPlane_patient(2).normal = normalY;
    else
        boxPlane_patient(2).normal = -normalY;
    end

    if boxPlane_patient(5).normal(1) > 0
        boxPlane_patient(5).normal = normalX;
    else
        boxPlane_patient(5).normal = -normalX;
    end
    if boxPlane_patient(6).normal(1) > 0
        boxPlane_patient(6).normal = normalX;
    else
        boxPlane_patient(6).normal = -normalX;
    end
end


if (strcmp(version, '2.0'))
    cL = boxPlane_patient(idxCoronal).distance_from_origin*normalY;
    cP = boxPlane_patient(idxSaggital).distance_from_origin*normalX;
    cS = boxPlane_patient(idxAxial).distance_from_origin*normalZ;
else
    centerL = (boxPlane_patient(5).distance*sign(boxPlane_patient(5).normal(1)) + boxPlane_patient(6).distance*sign(boxPlane_patient(6).normal(1)))/2;
    centerP = (boxPlane_patient(1).distance*sign(boxPlane_patient(1).normal(2)) + boxPlane_patient(2).distance*sign(boxPlane_patient(2).normal(2)))/2;
    centerS = (boxPlane_patient(3).distance*sign(boxPlane_patient(3).normal(3)) + boxPlane_patient(4).distance*sign(boxPlane_patient(4).normal(3)))/2;
end

if (strcmp(version, '2.0'))
    % Needed in order to account for the directions of the normals
    if (normalX(1)< 0)
            normalX(1) = -normalX(1);    
    end
    if (normalY(2) < 0)    
         normalY(2) = -normalY(2);
         normalY(3) = -normalY(3);
    end
    if (normalZ(3) < 0)    
         normalZ(2) = -normalZ(2);
         normalZ(3) = -normalZ(3);
    end
end

% construct the rotation matrix
R = [normalX normalY normalZ]; 

if R(3,1) ~= 1 && R(3,1) ~= -1
    %BETA = pi + asin(R(3,1) );  % To match High order shim ROI rotation
    BETA = -asin(R(3,1));
    ALPHA = atan2(R(3,2)/cos(BETA), R(3,3)/cos(BETA));
    GAMMA = atan2(R(2,1)/cos(BETA), R(1,1)/cos(BETA));
else
    GAMMA = 0;
    if R(3,1) == 1
        BETA = pi/2;
        ALPHA = atan2(R(1,2), R(1,3));
    else
        BETA = -pi/2;
        ALPHA = atan2(-R(1,2), -R(1,3));
    end
end


if (strcmp(version, '2.0'))
    if get(handles.checkbox_FixBoxSize, 'value') == 0
        % Scale Box with scale factor from the tmatrix. To obtain the scale
        % we can use SVD to factor the RotMatrix = U*S*V' components
        % representing basically a Rot*Scale*Rot matrix. The diagonal S
        % component is the scale.
        tmat = handles.tmatrix;
        [q,r] = qr(tmat(1:3,1:3));

        
        tmp = [ boxPlane_patient(idxSaggital).thickness; ...
                boxPlane_patient(idxCoronal).thickness; ...
                boxPlane_patient(idxAxial).thickness];
                
        sizes = (eye(3).*abs(r))*tmp;
        
        sizeX = sizes(1); 
        sizeY = sizes(2); 
        sizeZ = sizes(3); 
        
        % Update the XML struct
        boxPlane_patient(idxSaggital).thickness = sizes(1);
        boxPlane_patient(idxCoronal).thickness  = sizes(2);
        boxPlane_patient(idxAxial).thickness    = sizes(3);
          
        
    else
        sizeX = handles.boxSizeRL;
        sizeY = handles.boxSizeAP;
        sizeZ = handles.boxSizeSI;
     end
  
    box_center = cP+cS+cL;
    box_center = box_center';

else
    if get(handles.checkbox_FixBoxSize, 'value') == 0
        sizeX = abs(boxPlane_patient(5).distance*sign(boxPlane_patient(5).normal(1)) - boxPlane_patient(6).distance*sign(boxPlane_patient(6).normal(1))); 
        sizeY = abs(boxPlane_patient(1).distance*sign(boxPlane_patient(1).normal(2)) - boxPlane_patient(2).distance*sign(boxPlane_patient(2).normal(2))); 
        sizeZ = abs(boxPlane_patient(3).distance*sign(boxPlane_patient(3).normal(3)) - boxPlane_patient(4).distance*sign(boxPlane_patient(4).normal(3))); 
    else
        sizeX = handles.boxSizeRL;
        sizeY = handles.boxSizeAP;
        sizeZ = handles.boxSizeSI;
        boxPlane_patient(1).distance = (centerP - sizeY/2)*sign(boxPlane_patient(1).normal(2)); 
        boxPlane_patient(2).distance = (centerP + sizeY/2)*sign(boxPlane_patient(2).normal(2)); 
        boxPlane_patient(3).distance = (centerS + sizeZ/2)*sign(boxPlane_patient(3).normal(3)); 
        boxPlane_patient(4).distance = (centerS - sizeZ/2)*sign(boxPlane_patient(4).normal(3)); 
        boxPlane_patient(5).distance = (centerL + sizeX/2)*sign(boxPlane_patient(5).normal(1)); 
        boxPlane_patient(6).distance = (centerL - sizeX/2)*sign(boxPlane_patient(6).normal(1)); 
    end 
    box_center = [centerL centerP centerS]'; 
    box_center = R*box_center; 

end


% export box info to press_box.dat
idx1 = strfind(xml_filename, '_');
idx2 =  strfind(xml_filename, '.')-1;
xml_postfix = xml_filename(idx1(end)+1:idx2);
filename = strcat('press_box_', xml_postfix, '.dat');
fid = fopen(filename,'w');
fprintf(fid,'%f %f %f\n', box_center(1), box_center(2), box_center(3));
fprintf(fid,'%f %f %f\n',sizeX, sizeY, sizeZ);
fprintf(fid,'%f %f %f\n', ALPHA, BETA, GAMMA+pi); % NOTE: plus pi means from LPS to RAS. Alternatively this can be done in PSD.
fclose(fid);

boxDatName = filename;

%% Apply transformation matrix to SAT Bands
% Transform the normal vector of the center plane of each predefined SAT band to
% the patient space

satBand_patient = satBand_atlas;
nSat = length(satBand_patient);

if nSat >= 1
    
    filename = strcat('sat_bands_', xml_postfix, '.dat');
    fid = fopen(filename,'w');
    fprintf(fid,'%d\n',nSat);
    
    for k = 1:nSat
        if (strcmp(version, '2.0'))
            [temp,...
             satBand_patient(k).distance_from_origin, ...
             satBand_patient(k).thickness] = ...
                atlas2patient([satBand_atlas(k).normal_x, satBand_atlas(k).normal_y, satBand_atlas(k).normal_z]', ...
                              satBand_atlas(k).distance_from_origin, ...
                              handles.pixelSize_atlas, ...
                              handles.pixelSize_patient, ...
                              handles.qform_atlas, ...
                              handles.qform_patient, ...
                              handles.tmatrix, ...
                              satBand_atlas(k).thickness);

            satBand_patient(k).normal_x = temp(1);
            satBand_patient(k).normal_y = temp(2);
            satBand_patient(k).normal_z = temp(3);
        else
            [satBand_patient(k).normal satBand_patient(k).distance satBand_patient(k).thickness] = atlas2patient(satBand_atlas(k).normal, satBand_atlas(k).distance, handles.pixelSize_atlas, handles.pixelSize_patient, handles.qform_atlas, handles.qform_patient, handles.tmatrix, satBand_atlas(k).thickness); 
        end
         
        if get(handles.checkbox_FixSATThickness, 'value') == 1
            satBand_patient(k).thickness = handles.satThickness;
        end
        if (strcmp(version, '2.0'))
            % Compute the rotation angles of the SAT band, which will be read by the pulse sequence
            BETA = -acos(satBand_patient(k).normal_z);
            GAMMA = -atan2(satBand_patient(k).normal_y, -satBand_patient(k).normal_x);

            % NOTE: clockwise rotation in the left-handed LPS coordinates. Also PSD takes only
            % two rotation angles for each SAT bands
            fprintf(fid,'%f %f %f %f 0\n', GAMMA, BETA, satBand_patient(k).distance_from_origin, satBand_patient(k).thickness);
        else
            BETA = -acos(satBand_patient(k).normal(3)); 
	 	    GAMMA = -atan2(satBand_patient(k).normal(2), -satBand_patient(k).normal(1));
            fprintf(fid,'%f %f %f %f 0\n', GAMMA, BETA, satBand_patient(k).distance, satBand_patient(k).thickness); 
        end
    end
    fclose(fid);
    satDatName = filename;
else
    satDatName = [];
end
