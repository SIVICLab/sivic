%{
 Copyright © 2016-2017 The Regents of the University of California.
 All Rights Reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:
 -   Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer.
 -   Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution.
 -   None of the names of any campus of the University of California, the name 
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

  
 $URL: https://intrarad.ucsf.edu/svn/rad_software/surbeck/brain/sat_placement_atlas_based/trunk/mask_bands.m $
 $Rev: 39349 $
 $Author: bolson@RADIOLOGY.UCSF.EDU $
 $Date: 2017-01-24 16:17:11 -0800 (Tue, 24 Jan 2017) $

burns the bands into the pixel data

v - image volume
A, B, D, T - vectors of alpha, beta, distance, thickness of bands
x_axis, etc. - vectors defining pixel positions of the volume on the axis
in mm
use_pattern - whether to use cross-hatched patern for sat bands
max_val - maximum pixel value


%}

% Wei Bian 08/2014
% Modified from Eugene's code

function v1 = mask_bands(ver, v, sb, x_axis, y_axis, z_axis, use_pattern, max_val,box)
if (strcmp(ver,'2.0'))
    temp = [sb.normal_x; sb.normal_y; sb.normal_z];
    if ~isempty(temp)
        a = temp(1,:);
        b = temp(2,:);
        c = temp(3,:);
    else
        a = 0;
        b = 0;
        c = 0;
    end

    %if all([sb.distance_from_origin]) && ~all([sb.thickness]) % for box
    if box % for box    
        d1 = [sb(1).distance_from_origin - sb(1).thickness/2; ... % Coronal plane (= width of sagittal)
              sb(2).distance_from_origin - sb(2).thickness/2 ; ... % Axial plane (=height of sagittal)
              sb(3).distance_from_origin - sb(3).thickness/2 ...
              ]';

        d2 = d1+[sb(1).thickness sb(2).thickness sb(3).thickness ];

    else % for sat band
        d1 = [sb.distance_from_origin]-[sb.thickness]./2;
        d2 = [sb.distance_from_origin]+[sb.thickness]./2;
    end

    n_bands = length(d1);
    [ymax, xmax, zmax] = size(v);
    v1 = v;

    for k=1:1:zmax
        for i=1:1:ymax
            for j=1:1:xmax
                if(~use_pattern || (mod(i+k,2)==0 && mod(j+k,2)==0))
                    x = x_axis(j);
                    y = y_axis(i);
                    z = z_axis(k);

                    w = 0;
                    for(l=1:n_bands)
                        s = a(l)*x+b(l)*y+c(l)*z;
                        if (box)
                           if((s>d1(l) && s<(d1(l)+4)) || (s>(d2(l)-4) && s<d2(l)))     
                                    w=1;
                            end;
                        else
                            if(s>d1(l) && s<d2(l))
                                w=1;
                            end;
                        end
                    end

                    if(w>0)
                        if(use_pattern)
                            v1(i,j,k) = max(max_val-v1(i,j,k),0);
                        else
                            v1(i,j,k) = max_val;
                        end;
                    end
                end
            end
        end
    end
else
temp = [sb.normal];
	a = temp(1,:);
	b = temp(2,:);
	c = temp(3,:);
	
	if sb(1).thickness > 0 % for sat bands
	    d1 = [sb.distance]-[sb.thickness]./2;
	    d2 = [sb.distance]+[sb.thickness]./2;
	else
	    d1 = [sb.distance]; % for box
	    d2 = d1+[5 5 5 5 5 5];
	end
	
	n_bands = length(d1);
	[ymax, xmax, zmax] = size(v);
	v1 = v;
	
	for k=1:1:zmax
	    for i=1:1:ymax
	        for j=1:1:xmax
	            if(~use_pattern || (mod(i+k,2)==0 && mod(j+k,2)==0))
	                x = x_axis(j);
	                y = y_axis(i);
	                z = z_axis(k);
	
	                w = 0;
	                for(l=1:n_bands)
	                    s = a(l)*x+b(l)*y+c(l)*z;
	                    if(s>d1(l) && s<d2(l))
	                        w=1;
	                    end;
	                end
	
	                if(w>0)
	                    if(use_pattern)
	                        v1(i,j,k) = max(max_val-v1(i,j,k),0);
	                    else
	                        v1(i,j,k) = max_val;
	                    end;
	                end
	            end
	        end
	    end
	end        
end


