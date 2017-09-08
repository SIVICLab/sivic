%{
Copyright (c) 2009, Laszlo Balkay
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
%}
function orthogonalslicer(imaVOL, pixsize, colormaptype, minpix, maxpix)
% ORTHOGONALSLICER
% 
% Orthogonalslicer creates 3 figures showing the 3 orthogonal slices 
% from a scalar volume of data. Cross lines on each image represents lines of 
% intersection of the appropriate plane and the two other orthogonal plane.
% By left dragging the mouse on any plane the user can explore the volume.
% Additional mouse options:
%   - the images can be zoomed and paned with middle and right mouse dragging
%   - to restore the original state (zoom and pan) double click on the image
%   
% Usage:
%
%   orthogonalslicer; or orthogonalslicer(1); 
%   DEMO using the MATLAB 'MRI' data set.  
%
%   orthogonalslicer(2); 
%   DEMO using the MATLAB 'FLOW' data set.  
%
%   orthogonalslicer(VOLUME) 
%       - VOLUME a 3D scalar data array
%
%   orthogonalslicer(VOLUME, PIXELSIZE) 
%       - PIXELSIZE describes the size(aspect) of the pixel in 3D /default = [1 1 1]/
%
%   orthogonalslicer(VOLUME, PIXELSIZE, COLORMAPTYPE) 
%       - COLORMAPTYPE selects the colormap to be used /default = jet/
%
%   orthogonalslicer(VOLUME, PIXELSIZE, COLORMAPTYPE,MINPIX,MAXPIX)) 
%       - MINPIX,MAXPIX normalizes the values in VOLUME to the range
%       specified (same as IMAGESC does)
%

%
% University of Debrecen, PET Center/LB 2004

if nargin == 0
    mridata = load('mri');
    imaVOL = squeeze(mridata.D);
    minpix = 0; maxpix = max(imaVOL(:));
    pixsize = [1 1 1/0.4];
    colormaptype = bone(256);
elseif nargin == 1
    if sum(size(imaVOL)) == 2
        if imaVOL == 1
            mridata = load('mri');
            imaVOL = squeeze(mridata.D);
            minpix = 0; maxpix = max(imaVOL(:));
            pixsize = [1 1 1/0.4];
            colormaptype = bone(256);
        elseif imaVOL == 2
            imaVOL = permute(flow(50),[1 3 2]);
            pixsize = [1 1 1];
            colormaptype = 'jet';
            minpix = min(imaVOL(:)); maxpix = max(imaVOL(:));
        end
        mia_Start3dCursor(imaVOL, pixsize, colormaptype, minpix, maxpix);  
        return;
    end
    pixsize = [1 1 1];
    colormaptype = jet;
    minpix = min(imaVOL(:)); maxpix = max(imaVOL(:));
elseif nargin == 2
    colormaptype = jet;
    minpix = min(imaVOL(:)); maxpix = max(imaVOL(:));
elseif nargin == 3
    minpix = min(imaVOL(:)); maxpix = max(imaVOL(:));
elseif nargin == 4
    maxpix = max(imaVOL(:));
end
    
mia_Start3dCursor(imaVOL, pixsize, colormaptype, minpix, maxpix);   
    
    
    
