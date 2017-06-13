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

function [n_patient D T] = atlas2patient(n_atlas, distance, pSize_atlas, pSize_patient, qform_atlas, qform_patient, tmatrix, thickness)

% find a point on the plane
p_atlas = n_atlas*distance; % distance for box: from LPS origin to each face for box
                            % distance for satbands:  from LPS origin to the center plane of each band

if exist('thickness', 'var') % the step only for sat band transformation
    % find a point on the inner plane of the sat band
    p_atlas_inner = n_atlas*(distance-thickness/2);
end

% transfer the point on the plane
p_atlas = p_atlas.*[-1, -1, 1]'; % now in RAS
p_atlas = [p_atlas ; 1]; % homogeneous coordinates by adding '1'
p_atlas = inv(qform_atlas)*p_atlas; % from RAS to Voxel coordinates
p_patient = tmatrix*(p_atlas.*[pSize_atlas 1]')./[pSize_patient 1]'; % apply transformation
p_patient = qform_patient*p_patient; % from Voxel to RAS coordinates
p_patient = p_patient(1:3).*[-1, -1, 1]'; % now in LPS

% Transfer the normal vector of the plane
n_atlas = n_atlas.*[-1, -1, 1]'; % now in RAS
n_atlas = inv(qform_atlas(1:3 , 1:3) )*n_atlas; % from RAS to Voxel coordinates
n_patient = transpose(inv(tmatrix(1:3 , 1:3)))*(n_atlas.*pSize_atlas')./pSize_patient'; % apply transformation
n_patient = qform_patient(1:3 , 1:3)*n_patient; % from Voxel to RAS coordinates
n_patient = n_patient/norm(n_patient); % normalization
n_patient = n_patient.*[-1, -1, 1]'; % now in LPS

% distance from the origin to the transformed plane
D = dot(p_patient, n_patient);

if exist('thickness', 'var') % the step only for sat band transformation
    
    % transfer the point on the plane
    p_atlas_inner = p_atlas_inner.*[-1, -1, 1]'; % now in RAS
    p_atlas_inner = [p_atlas_inner ; 1]; % homogeneous coordinates by adding '1'
    p_atlas_inner = inv(qform_atlas)*p_atlas_inner; % from RAS to Voxel coordinates
    p_patient_inner = tmatrix*(p_atlas_inner.*[pSize_atlas 1]')./[pSize_patient 1]'; % apply transformation
    p_patient_inner = qform_patient*p_patient_inner; % from Voxel to RAS coordinates
    p_patient_inner = p_patient_inner(1:3).*[-1, -1, 1]'; % now in LPS
    
    D_inner = dot(p_patient_inner, n_patient);
    T = 2*abs(D-D_inner); % sat band thickness in the patient space
end

   
