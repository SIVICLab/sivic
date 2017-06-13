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

% Code modified from: https://www.mathworks.com/matlabcentral/fileexchange/5934-orthogonalslicer
% License below:

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

function mia_Stop3dCursor()
%
% destroy the 3 figures showing the 3 perpendicular slices
%
% Matlab library function for mia_gui utility. 
% University of Debrecen, PET Center/LB 2003

figureHandle = gcbo;
FigDataIn = get(figureHandle,'Userdata');

if isempty(findobj('tag','mia_figure1')) && isempty(findobj('tag','PetAnalSimulatorFigure'))
    xyplane_name = 'X-Y plane';
    yzplane_name = 'Y-Z plane';
    xzplane_name = 'X-Z plane';
else
    xyplane_name = 'Axial slice';
    yzplane_name = 'Coronal slice';
    xzplane_name = 'Sagital slice';
end

if strcmp(get(figureHandle,'name'),xyplane_name);
    if ishandle(FigDataIn.FigHandlerX); delete(FigDataIn.FigHandlerX); end
    if ishandle(FigDataIn.FigHandlerY); delete(FigDataIn.FigHandlerY); end
elseif strcmp(get(figureHandle,'name'),yzplane_name);
    if ishandle(FigDataIn.FigHandlerY); delete(FigDataIn.FigHandlerY); end
    if ishandle(FigDataIn.FigHandlerZ); delete(FigDataIn.FigHandlerZ); end
elseif strcmp(get(figureHandle,'name'),xzplane_name);
    if ishandle(FigDataIn.FigHandlerZ); delete(FigDataIn.FigHandlerZ); end
    if ishandle(FigDataIn.FigHandlerY); delete(FigDataIn.FigHandlerY); end
end

figh = findobj('tag','mia_3dcursor_figure');
if ~isempty(figh);
    mia_3dcursor_figure_handles = guidata(figh);
    miafigh = findobj('tag','mia_figure1');
    if isempty(miafigh); return;end
    mia_mainfigure_handles = guidata(miafigh);
    if isfield(mia_3dcursor_figure_handles,'NewClim')
        NewClim = mia_3dcursor_figure_handles.NewClim;
        if get(mia_mainfigure_handles.ColorBarMaxSlider,'Min') > NewClim(1)
        set(mia_mainfigure_handles.ColorBarMaxSlider, 'Min', 0.8*NewClim(1));
        set(mia_mainfigure_handles.ColorBarMinSlider, 'Min', 0.8*NewClim(1));
        end
        if get(mia_mainfigure_handles.ColorBarMaxSlider, 'Max') <  NewClim(2)
            set(mia_mainfigure_handles.ColorBarMaxSlider, 'Max', 1.2*NewClim(2));
            set(mia_mainfigure_handles.ColorBarMinSlider, 'Max' , 1.2*NewClim(2));
        end
        set(mia_mainfigure_handles.ColorBarMaxSlider, 'Value', NewClim(2));
        set(mia_mainfigure_handles.ColorBarMinSlider, 'Value',  NewClim(1));

        % change the image type from intensity  to RGB
        SliderPosMax = NewClim(2);
        SliderPosMin = NewClim(1);
        CurrentImage = double(mia_mainfigure_handles.imaVOL(:,:,mia_mainfigure_handles.CurrentImgIdx));
        CurrentImage_RGB = ...
            ImageRefresh(CurrentImage,SliderPosMax,SliderPosMin,...
            mia_mainfigure_handles.ColorRes,mia_mainfigure_handles.ColormapIn1);
        set(mia_mainfigure_handles.ImaHandler,'CData',CurrentImage_RGB);
        newlabels = num2cell(fixround(linspace(SliderPosMin,SliderPosMax,...
            mia_mainfigure_handles.NumOfYtickOfColorbar),mia_mainfigure_handles.decimal_prec))';
        set(mia_mainfigure_handles.CmapAxes,'Yticklabel',newlabels);
    end
    delete(figh);
    set(miafigh,'visible','on');
end



% --- Executes the RBG image generation.
function Image_RGB = CreateRGBImage(ImageIn,ImageMinMax,ColorRes,ColormapIn)
%
%CurrentImage_Rescaled = int16( fix((ColorRes-1)*double(ImageIn) / ( double(ImageMinMax(2))-double(ImageMinMax(1))  ) )+1);
CurrentImage_Rescaled = uint16( fix( (ColorRes-1) * ...
    ( (double(ImageIn)-double(ImageMinMax(1))) / (double(ImageMinMax(2))-double(ImageMinMax(1))) )   )+1);
range_zeros = (find(CurrentImage_Rescaled == 0));
CurrentImage_Rescaled(range_zeros) = 1;
Current_cmap = ColormapIn;
Image_RGB = (zeros(size(ImageIn,1),size(ImageIn,2),3));
for RGB_dim = 1:3
    colour_slab_vals = Current_cmap(CurrentImage_Rescaled, RGB_dim);
    Image_RGB(:,:,RGB_dim) = reshape( colour_slab_vals, size(ImageIn));
end

% --- Executes the image refresh.
function Image_out = ImageRefresh(CurrentImage,SliderPosMax,SliderPosMin,ColorRes,ColormapIn)
%
CurrentImage( find(CurrentImage > SliderPosMax )) = SliderPosMax;
CurrentImage( find(CurrentImage < SliderPosMin)) = min(CurrentImage(:));
            Image_out = ... 
CreateRGBImage(CurrentImage,[SliderPosMin SliderPosMax], ColorRes, ColormapIn);

function valout = fixround(valin,precision)
% simple rounding function to cut off the necessary decimal digits
valout = round(valin*10^precision)/10^precision; 

