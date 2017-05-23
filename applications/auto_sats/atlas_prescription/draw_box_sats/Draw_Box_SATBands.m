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
function varargout = Draw_Box_SATBands(varargin)
% DRAW_BOX_SATBANDS MATLAB code for Draw_Box_SATBands.fig
%      DRAW_BOX_SATBANDS, by itself, creates a new DRAW_BOX_SATBANDS or raises the existing
%      singleton*.
%
%      H = DRAW_BOX_SATBANDS returns the handle to a new DRAW_BOX_SATBANDS or the handle to
%      the existing singleton*.
%
%      DRAW_BOX_SATBANDS('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in DRAW_BOX_SATBANDS.M with the given input arguments.
%
%      DRAW_BOX_SATBANDS('Property','Value',...) creates a new DRAW_BOX_SATBANDS or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Draw_Box_SATBands_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Draw_Box_SATBands_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help Draw_Box_SATBands

% Last Modified by GUIDE v2.5 10-Oct-2014 14:14:47

% Wei Bian Created in August,2014
% Sarah Nelson's Lab
% Department of Radiology UCSF


% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Draw_Box_SATBands_OpeningFcn, ...
                   'gui_OutputFcn',  @Draw_Box_SATBands_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before Draw_Box_SATBands is made visible.
function Draw_Box_SATBands_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Draw_Box_SATBands (see VARARGIN)

% Choose default command line output for Draw_Box_SATBands
handles.output = hObject;
handles.atlas_dir = pwd;
   
handles.axialAx = findobj(hObject, 'Tag','axialAxes');
handles.saggitalAx = findobj(hObject, 'Tag','saggitalAxes');
handles.coronalAx = findobj(hObject, 'Tag','coronalAxes');

handles.axialSlider = findobj(hObject, 'Tag','axialSlider');
handles.saggitalSlider = findobj(hObject, 'Tag','saggitalSlider');
handles.coronalSlider = findobj(hObject, 'Tag','coronalSlider');

% handles.coordR = findobj(hObject, 'Tag','coordR'); % may be used in the future
% handles.coordL = findobj(hObject, 'Tag','coordL');
% handles.coordA = findobj(hObject, 'Tag','coordA');
% handles.coordP = findobj(hObject, 'Tag','coordP');
% handles.coordS = findobj(hObject, 'Tag','coordS');
% handles.coordI = findobj(hObject, 'Tag','coordI');

handles.BoxTog = findobj(hObject, 'Tag','togglebutton_Press_Box');
handles.SatAnterior1Tog = findobj(hObject, 'Tag','togglebutton_SatAnterior1');
handles.SatAnterior2Tog = findobj(hObject, 'Tag','togglebutton_SATAnterior2');
handles.SatPosterior1Tog = findobj(hObject, 'Tag','togglebutton_SatPosterior1');
handles.SatPosterior2Tog = findobj(hObject, 'Tag','togglebutton_SATPosterior2');
handles.SatPSuperiorTog = findobj(hObject, 'Tag','togglebutton_SatPSuperior');
handles.SatASuperiorTog = findobj(hObject, 'Tag','togglebutton_SatASuperior');
handles.SatInferiorTog = findobj(hObject, 'Tag','togglebutton_SatInferior');
handles.SatSuperiorTog = findobj(hObject, 'Tag','togglebutton_SATSuperior');
handles.SatARightTog = findobj(hObject, 'Tag','togglebutton_SatARight');
handles.SatRSuperiorTog = findobj(hObject, 'Tag','togglebutton_SATRSuperior');
handles.SatLSuperiorTog = findobj(hObject, 'Tag','togglebutton_SATLSuperior');
handles.SatRightTog = findobj(hObject, 'Tag','togglebutton_SATRight');
handles.SatPRightTog = findobj(hObject, 'Tag','togglebutton_SatPRight');
handles.SatALeftTog = findobj(hObject, 'Tag','togglebutton_SatALeft');
handles.SatLeftTog = findobj(hObject, 'Tag','togglebutton_SATLeft');
handles.SatPLeftTog = findobj(hObject, 'Tag','togglebutton_SatPLeft');

handles.showRL = findobj(hObject, 'Tag','ShowRL');
handles.showAP = findobj(hObject, 'Tag','ShowAP');
handles.showSI = findobj(hObject, 'Tag','ShowSI');

handles.color = [0.5 0.26 0.26];

handles.axialImg = [];
handles.saggitalImg = [];
handles.coronalImg = [];

handles.axialShow = [];
handles.saggitalShow = [];
handles.coronalShow = [];

handles.axialBoxPos = [];
handles.axialBoxDeleted = true;
handles.saggitalBoxPos = [];
handles.saggitalBoxDeleted = true;
handles.coronalBoxPos = [];
handles.coronalBoxDeleted = true;
handles.indexLine = [];
handles.PressBox_Saggital = [];
handles.PressBox_Axial = [];
handles.PressBox_Coronal = [];

handles.SatBand_Anterior1_Saggital = [];
handles.SatBand_Anterior1_Axial = [];
handles.SatBand_Anterior1_Coronal = [];
handles.saggitalSatAnterior1Pos = [];

handles.SatBand_Anterior2_Saggital = [];
handles.SatBand_Anterior2_Axial = [];
handles.SatBand_Anterior2_Coronal = [];
handles.saggitalSatAnterior2Pos = [];

handles.SatBand_Posterior1_Saggital = [];
handles.SatBand_Posterior1_Axial = [];
handles.SatBand_Posterior1_Coronal = [];
handles.saggitalSatPosterior1Pos = [];

handles.SatBand_Posterior2_Saggital = [];
handles.SatBand_Posterior2_Axial = [];
handles.SatBand_Posterior2_Coronal = [];
handles.saggitalSatPosterior2Pos = [];

handles.SatBand_ASuperior_Saggital = [];
handles.SatBand_ASuperior_Axial = [];
handles.SatBand_ASuperior_Coronal = [];
handles.saggitalSatASuperiorPos = [];

handles.SatBand_PSuperior_Saggital = [];
handles.SatBand_PSuperior_Axial = [];
handles.SatBand_PSuperior_Coronal = [];
handles.saggitalSatPSuperiorPos = [];

handles.SatBand_Inferior_Saggital = [];
handles.SatBand_Inferior_Axial = [];
handles.SatBand_Inferior_Coronal = [];
handles.saggitalSatInferiorPos = [];

handles.SatBand_Superior_Saggital = [];
handles.SatBand_Superior_Axial = [];
handles.SatBand_Superior_Coronal = [];
handles.saggitalSatSuperiorPos = [];

handles.SatBand_ARight_Saggital = [];
handles.SatBand_ARight_Axial = [];
handles.SatBand_ARight_Coronal = [];
handles.axialSatARightPos = [];

handles.SatBand_RSuperior_Saggital = [];
handles.SatBand_RSuperior_Axial = [];
handles.SatBand_RSuperior_Coronal = [];
handles.coronalSatRSuperiorPos = [];

handles.SatBand_LSuperior_Saggital = [];
handles.SatBand_LSuperior_Axial = [];
handles.SatBand_LSuperior_Coronal = [];
handles.coronalSatLSuperiorPos = [];

handles.SatBand_Right_Saggital = [];
handles.SatBand_Right_Axial = [];
handles.SatBand_Right_Coronal = [];
handles.axialSatRightPos = [];

handles.SatBand_PRight_Saggital = [];
handles.SatBand_PRight_Axial = [];
handles.SatBand_PRight_Coronal = [];
handles.axialSatPRightPos = [];

handles.SatBand_ALeft_Saggital = [];
handles.SatBand_ALeft_Axial = [];
handles.SatBand_ALeft_Coronal = [];
handles.axialSatALeftPos = [];

handles.SatBand_Left_Saggital = [];
handles.SatBand_Left_Axial = [];
handles.SatBand_Left_Coronal = [];
handles.axialSatLeftPos = [];

handles.SatBand_PLeft_Saggital = [];
handles.SatBand_PLeft_Axial = [];
handles.SatBand_PLeft_Coronal = [];
handles.axialSatPLeftPos = [];

handles.SatARightCenter = [];
handles.SatPRightCenter = [];
handles.SatALeftCenter = [];
handles.SatLeftCenter = [];
handles.SatPLeftCenter = [];
handles.SatRightCenter = [];

handles.SatARightRot = 0;
handles.SatPRightRot = 0;
handles.SatALeftRot = 0;
handles.SatLeftRot = 0;
handles.SatPLeftRot = 0;
handles.SatRightRot = 0;

handles.axisX = [];
handles.axisY = [];
handles.axisZ = [];

handles.originX = [];
handles.originY = [];
handles.originZ = [];

handles.pixelSize = [];
% Update handles structure
guidata(hObject, handles);

% UIWAIT makes Draw_Box_SATBands wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = Draw_Box_SATBands_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on slider movement.
function axialSlider_Callback(hObject, eventdata, handles)
% hObject    handle to axialSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider
slice = get(handles.axialSlider,'Value'); % returns position of slider
slice = round(slice);
set(handles.axialShow, 'CData',handles.axialImg(:,:,slice));

interPlane = compute_interplane(handles.saggitalBoxPos(1:4,:));
idx = find(interPlane(:,1)==slice);
if ~isempty(idx)
    if ~handles.axialBoxDeleted
        delete(handles.PressBox_Axial);
    end
    boxHeight = interPlane(idx,3) - interPlane(idx,2);
    boxPos = [handles.axialBoxPos(1,1) interPlane(idx,2)  handles.axialBoxPos(6,1) boxHeight handles.axialBoxPos(8,1)];
    [handles.PressBox_Axial api]=imRectRot('hParent', handles.axialAx, 'pos', boxPos , 'ellipse', 0,'rotate',0,'color','y','colorc','y'  );
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc, w, h] = rectToCorners( api.getPos());
    handles.axialBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_axial(pos));
    handles.axialBoxDeleted = false;
else
    if ~handles.axialBoxDeleted
        delete(handles.PressBox_Axial);
        handles.axialBoxDeleted = true;
    end
end

if get(handles.SatAnterior1Tog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatAnterior1Pos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Anterior1_Axial);
    [handles.SatBand_Anterior1_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatAnterior2Tog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatAnterior2Pos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Anterior2_Axial);
    [handles.SatBand_Anterior2_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPosterior1Tog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatPosterior1Pos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Posterior1_Axial);
    [handles.SatBand_Posterior1_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPosterior2Tog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatPosterior2Pos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Posterior2_Axial);
    [handles.SatBand_Posterior2_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatASuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatASuperiorPos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_ASuperior_Axial);
    [handles.SatBand_ASuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPSuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatPSuperiorPos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_PSuperior_Axial);
    [handles.SatBand_PSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatInferiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatInferiorPos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Inferior_Axial);
    [handles.SatBand_Inferior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatSuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.saggitalSatSuperiorPos(1:4,:),handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Superior_Axial);
    [handles.SatBand_Superior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end


if get(handles.SatARightTog,'Value') == 1 && ~isempty(handles.SatARightCenter);
    idx = find(handles.SatARightCenter(3,:)==handles.originZ-slice);
    delete(handles.SatBand_ARight_Axial);
    w = handles.axialSatARightPos(6,1);
    h = handles.axialSatARightPos(7,1);
    satPos = [handles.SatARightCenter(1,idx(1))+handles.originX-w/2 handles.originY-handles.SatARightCenter(2,idx(1))-h/2 w h handles.axialSatARightPos(8,1)];
    [handles.SatBand_ARight_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatRSuperiorTog,'Value') == 1
    interPlane = compute_sat_interplane_axial(handles.coronalSatRSuperiorPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_RSuperior_Axial);
    [handles.SatBand_RSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatLSuperiorTog,'Value') == 1
    interPlane = compute_sat_interplane_axial(handles.coronalSatLSuperiorPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_LSuperior_Axial);
    [handles.SatBand_LSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatRightTog,'Value') == 1 && ~isempty(handles.SatRightCenter);
    idx = find(handles.SatRightCenter(3,:)==handles.originZ-slice);
    delete(handles.SatBand_Right_Axial);
    w = handles.axialSatRightPos(6,1);
    h = handles.axialSatRightPos(7,1);
    satPos = [handles.SatRightCenter(1,idx(1))+handles.originX-w/2 handles.originY-handles.SatRightCenter(2,idx(1))-h/2 w h handles.axialSatRightPos(8,1)];
    [handles.SatBand_Right_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end


if get(handles.SatALeftTog,'Value') == 1 && ~isempty(handles.SatALeftCenter);
    idx = find(handles.SatALeftCenter(3,:)==handles.originZ-slice);
    delete(handles.SatBand_ALeft_Axial);
    w = handles.axialSatALeftPos(6,1);
    h = handles.axialSatALeftPos(7,1);
    satPos = [handles.SatALeftCenter(1,idx(1))+handles.originX-w/2 handles.originY-handles.SatALeftCenter(2,idx(1))-h/2 w h handles.axialSatALeftPos(8,1)];
    [handles.SatBand_ALeft_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatLeftTog,'Value') == 1 && ~isempty(handles.SatLeftCenter);
    idx = find(handles.SatLeftCenter(3,:)==handles.originZ-slice);
    delete(handles.SatBand_Left_Axial);
    w = handles.axialSatLeftPos(6,1);
    h = handles.axialSatLeftPos(7,1);
    satPos = [handles.SatLeftCenter(1,idx(1))+handles.originX-w/2 handles.originY-handles.SatLeftCenter(2,idx(1))-h/2 w h handles.axialSatLeftPos(8,1)];
    [handles.SatBand_Left_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPRightTog,'Value') == 1 && ~isempty(handles.SatPRightCenter);
    idx = find(handles.SatPRightCenter(3,:)==handles.originZ-slice);
    delete(handles.SatBand_PRight_Axial);
    w = handles.axialSatPRightPos(6,1);
    h = handles.axialSatPRightPos(7,1);
    satPos = [handles.SatPRightCenter(1,idx(1))+handles.originX-w/2 handles.originY-handles.SatPRightCenter(2,idx(1))-h/2 w h handles.axialSatPRightPos(8,1)];
    [handles.SatBand_PRight_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPLeftTog,'Value') == 1 && ~isempty(handles.SatPLeftCenter);
    idx = find(handles.SatPLeftCenter(3,:)==handles.originZ-slice);
    delete(handles.SatBand_PLeft_Axial);
    w = handles.axialSatPLeftPos(6,1);
    h = handles.axialSatPLeftPos(7,1);
    satPos = [handles.SatPLeftCenter(1,idx(1))+handles.originX-w/2 handles.originY-handles.SatPLeftCenter(2,idx(1))-h/2 w h handles.axialSatPLeftPos(8,1)];
    [handles.SatBand_PLeft_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function axialSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to axialSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes on button press in LoadImages.
function LoadImages_Callback(hObject, eventdata, handles)
% hObject    handle to LoadImages (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% Read B1 Maps

if get(handles.BoxTog,'Value') == 1;
    if ~handles.axialBoxDeleted
        delete(handles.PressBox_Axial);
        handles.axialBoxDeleted = true;
    end
    if ~handles.saggitalBoxDeleted
        delete(handles.PressBox_Saggital);
        handles.saggitalBoxDeleted = true;
    end
    if ~handles.coronalBoxDeleted
        delete(handles.PressBox_Coronal);
        handles.coronalBoxDeleted = true;
    end
    set(handles.BoxTog,'Value',0);
    set(handles.showRL, 'String', '');
    set(handles.showAP, 'String', '');
    set(handles.showSI, 'String', '');
end

if get(handles.SatPSuperiorTog,'Value') == 1;
    delete(handles.SatBand_PSuperior_Saggital);
    delete(handles.SatBand_PSuperior_Axial);
    delete(handles.SatBand_PSuperior_Coronal);
    set(handles.SatPSuperiorTog,'Value',0);
end

if get(handles.SatAnterior1Tog,'Value') == 1;
    delete(handles.SatBand_Anterior1_Saggital);
    delete(handles.SatBand_Anterior1_Axial);
    delete(handles.SatBand_Anterior1_Coronal);
    set(handles.SatAnterior1Tog,'Value',0)
end

if get(handles.SatAnterior2Tog,'Value') == 1;
    delete(handles.SatBand_Anterior2_Saggital);
    delete(handles.SatBand_Anterior2_Axial);
    delete(handles.SatBand_Anterior2_Coronal);
    set(handles.SatAnterior2Tog,'Value',0)
end

if get(handles.SatPosterior1Tog,'Value') == 1;
    delete(handles.SatBand_Posterior1_Saggital);
    delete(handles.SatBand_Posterior1_Axial);
    delete(handles.SatBand_Posterior1_Coronal);
    set(handles.SatPosterior1Tog,'Value',0)
end

if get(handles.SatPosterior2Tog,'Value') == 1;
    delete(handles.SatBand_Posterior2_Saggital);
    delete(handles.SatBand_Posterior2_Axial);
    delete(handles.SatBand_Posterior2_Coronal);
    set(handles.SatPosterior2Tog,'Value',0)
end

if get(handles.SatInferiorTog,'Value') == 1;
    delete(handles.SatBand_Inferior_Saggital);
    delete(handles.SatBand_Inferior_Axial);
    delete(handles.SatBand_Inferior_Coronal);
    set(handles.SatInferiorTog,'Value',0);
end

if get(handles.SatSuperiorTog,'Value') == 1;
    delete(handles.SatBand_Superior_Saggital);
    delete(handles.SatBand_Superior_Axial);
    delete(handles.SatBand_Superior_Coronal);
    set(handles.SatSuperiorTog,'Value',0);
end


if get(handles.SatASuperiorTog,'Value') == 1;
    delete(handles.SatBand_ASuperior_Saggital);
    delete(handles.SatBand_ASuperior_Axial);
    delete(handles.SatBand_ASuperior_Coronal);
    set(handles.SatASuperiorTog,'Value',0);
end

if get(handles.SatARightTog,'Value') == 1;
    delete(handles.SatBand_ARight_Saggital);
    delete(handles.SatBand_ARight_Axial);
    delete(handles.SatBand_ARight_Coronal);
    set(handles.SatARightTog,'Value',0)
    handles.SatARightCenter = [];
    handles.SatARightRot = 0;
end

if get(handles.SatRSuperiorTog,'Value') == 1;
    delete(handles.SatBand_RSuperior_Saggital);
    delete(handles.SatBand_RSuperior_Axial);
    delete(handles.SatBand_RSuperior_Coronal);
    set(handles.SatRSuperiorTog,'Value',0)
end

if get(handles.SatLSuperiorTog,'Value') == 1;
    delete(handles.SatBand_LSuperior_Saggital);
    delete(handles.SatBand_LSuperior_Axial);
    delete(handles.SatBand_LSuperior_Coronal);
    set(handles.SatLSuperiorTog,'Value',0)
end

if get(handles.SatRightTog,'Value') == 1;
    delete(handles.SatBand_Right_Saggital);
    delete(handles.SatBand_Right_Axial);
    delete(handles.SatBand_Right_Coronal);
    set(handles.SatRightTog,'Value',0)
    handles.SatRightCenter = [];
    handles.SatRightRot = 0;
end


if get(handles.SatPLeftTog,'Value') == 1;
    delete(handles.SatBand_PLeft_Saggital);
    delete(handles.SatBand_PLeft_Axial);
    delete(handles.SatBand_PLeft_Coronal);
    set(handles.SatPLeftTog,'Value',0)
    handles.SatPLeftCenter = [];
    handles.SatPLeftRot = 0;
end

if get(handles.SatPRightTog,'Value') == 1;
    delete(handles.SatBand_PRight_Saggital);
    delete(handles.SatBand_PRight_Axial);
    delete(handles.SatBand_PRight_Coronal);
    set(handles.SatPRightTog,'Value',0)
    handles.SatPRightCenter = [];
    handles.SatPRightRot = 0;
end

if get(handles.SatALeftTog,'Value') == 1;
    delete(handles.SatBand_ALeft_Saggital);
    delete(handles.SatBand_ALeft_Axial);
    delete(handles.SatBand_ALeft_Coronal);
    set(handles.SatALeftTog,'Value',0);
    handles.SatALeftCenter = [];
    handles.SatALeftRot = 0;
end

if get(handles.SatLeftTog,'Value') == 1;
    delete(handles.SatBand_Left_Saggital);
    delete(handles.SatBand_Left_Axial);
    delete(handles.SatBand_Left_Coronal);
    set(handles.SatLeftTog,'Value',0);
    handles.SatLeftCenter = [];
    handles.SatLeftRot = 0;
end

if ~isempty(handles.axialShow)
    delete(handles.axialShow);
    delete(handles.saggitalShow);
    delete(handles.coronalShow);
end

[fName, fPath] = uigetfile('*.nii', 'Select the Atlas Volume Image', ...
                          handles.atlas_dir);
if fPath==0
    msgbox('Must select an Atlas Volume Image', 'File Selector', 'error');
    return
end
handles.atlas_dir = fPath;
cd(handles.atlas_dir);
NII_IM = load_nii(fName);
img = NII_IM.img;
img = flipdim(img,3);
img = flipdim(img,2);
img = permute(img, [2 1 3]);
img = flipdim(img, 2);
[handles.nrow handles.ncol handles.nslice] = size(img);
handles.axialImg = img;

handles.qform_atlas = [NII_IM.hdr.hist.srow_x; NII_IM.hdr.hist.srow_y; NII_IM.hdr.hist.srow_z; 0 0 0 1];
if handles.qform_atlas(1,1) == 1
    handles.qform_atlas(1,1) = -1;
    handles.qform_atlas(1,4) = -handles.qform_atlas(1,4)-1;
end
handles.pixelSize = NII_IM.hdr.dime.pixdim(2:4);
maxDim = max([handles.nrow handles.ncol handles.nslice]);
VOL = [(0:maxDim-1); (0:maxDim-1); (0:maxDim-1); ones(1,maxDim)];
RAS = handles.qform_atlas*VOL;
handles.axisX = -RAS(1,1:handles.ncol); % Now positive is L
handles.axisY = fliplr(-RAS(2,1:handles.nrow)); % Now positive is P
handles.axisZ = fliplr(RAS(3,1:handles.nslice)); 

handles.originX = find(handles.axisX>=0, 1);
handles.originY = find(handles.axisY>=0, 1);
handles.originZ= find(handles.axisZ<=0, 1);

axes(handles.axialAx);
set(handles.axialSlider,'Max',handles.nslice);
set(handles.axialSlider,'Min',1);
set(handles.axialSlider,'SliderStep', [1, 1]/(handles.nslice-1));
set(handles.axialSlider, 'Enable','on')
initialSlice = round(handles.nslice/2);
set(handles.axialSlider,'value',initialSlice);
handles.axialShow = imshow(handles.axialImg(:,:,initialSlice), []);

handles.saggitalImg = permute(img, [3 1 2]);
axes(handles.saggitalAx);
set(handles.saggitalSlider, 'Enable','on')
set(handles.saggitalSlider,'Max',handles.ncol);
set(handles.saggitalSlider,'Min',1);
set(handles.saggitalSlider,'SliderStep', [1, 1]/(handles.ncol-1));
initialSlice = round(handles.ncol/2);
set(handles.saggitalSlider,'value',initialSlice);
handles.saggitalShow = imshow(handles.saggitalImg(:,:,initialSlice), []);

handles.coronalImg = permute(img, [3 2 1]);
axes(handles.coronalAx);
set(handles.coronalSlider, 'Enable','on')
set(handles.coronalSlider,'Max',handles.nrow);
set(handles.coronalSlider,'Min',1);
set(handles.coronalSlider,'SliderStep', [1, 1]/(handles.nrow-1));
initialSlice = round(handles.nrow/2);
set(handles.coronalSlider,'value',initialSlice);
handles.coronalShow = imshow(handles.coronalImg(:,:,initialSlice), []);

guidata(hObject, handles);


% --- Executes on slider movement.
function saggitalSlider_Callback(hObject, eventdata, handles)
% hObject    handle to saggitalSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

slice = get(handles.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
set(handles.saggitalShow, 'CData',handles.saggitalImg(:,:,slice));

if slice<handles.axialBoxPos(1,1) || slice>handles.axialBoxPos(2,1)
    if ~handles.saggitalBoxDeleted
        delete(handles.PressBox_Saggital);
        handles.saggitalBoxDeleted = true;
    end
else
    if handles.saggitalBoxDeleted
        w = handles.saggitalBoxPos(6,1);
        h = handles.saggitalBoxPos(7,1);
        boxPos = [handles.saggitalBoxPos(5,1)-w/2 handles.saggitalBoxPos(5,2)-h/2 w h handles.saggitalBoxPos(8,1)];
        [handles.PressBox_Saggital,api]=imRectRot('hParent', handles.saggitalAx, 'pos', boxPos,'ellipse', 0,'rotate',1,'color','y','colorc','y'  );
        api.setPosChnCb( @(pos) redrawBox_saggital(pos));
        handles.saggitalBoxDeleted = false;
    end
end

if get(handles.SatARightTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.axialSatARightPos(1:4,:),handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_ARight_Saggital);
    [handles.SatBand_ARight_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatRSuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.coronalSatRSuperiorPos(1:4,:), handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satHeight= interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
    delete(handles.SatBand_RSuperior_Saggital);
    [handles.SatBand_RSuperior_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatLSuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.coronalSatLSuperiorPos(1:4,:), handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satHeight= interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
    delete(handles.SatBand_LSuperior_Saggital);
    [handles.SatBand_LSuperior_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatRightTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.axialSatRightPos(1:4,:),handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_Right_Saggital);
    [handles.SatBand_Right_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatALeftTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.axialSatALeftPos(1:4,:),handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_ALeft_Saggital);
    [handles.SatBand_ALeft_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatLeftTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.axialSatLeftPos(1:4,:),handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_Left_Saggital);
    [handles.SatBand_Left_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPRightTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.axialSatPRightPos(1:4,:),handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_PRight_Saggital);
    [handles.SatBand_PRight_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPLeftTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.axialSatPLeftPos(1:4,:),handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    delete(handles.SatBand_PLeft_Saggital);
    [handles.SatBand_PLeft_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function saggitalSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to saggitalSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


function boxDef3Plane = generate3PlaneRepresenation(boxLoc)
    A = 1, P = 2, S = 3, I = 4, L = 5, R = 6;
    normal_x1 = boxLoc(A,1);
    normal_y1 = boxLoc(A,2);
    normal_z1 = boxLoc(A,3);
    
    normal_x2 = boxLoc(S,1);
    normal_y2 = boxLoc(S,2);
    normal_z2 = boxLoc(S,3);
    
    distanceOrigin1 = boxLoc(A,4);
    distanceOrigin2 = boxLoc(S,4);
    cross([normal_x1, normal_y1, normal_z1],[normal_x2, normal_y2, normal_z2]);

    % To get the thickness calculate the distance between the antiparallel
    % normals: e.g. normal A and Normal S and so on..
    
    % To calculate the center of the box and its distance to origin 
    % add the three vector components corrensponding to xyz, e.g.
    % dx = (d1+d2)/2 x normal1..3
    
    
    
    % to calculate the angles use source

function nSatBands = getNumberofSatBands(handles)
nSatBands = 0
if get(handles.SatAnterior1Tog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatAnterior2Tog,'Value') == 1
   nSatBands = nSatBands + 1;
end
if get(handles.SatPosterior1Tog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatPosterior2Tog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatSuperiorTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatInferiorTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatASuperiorTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatPSuperiorTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatRightTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatARightTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatPRightTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatLeftTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatALeftTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatPLeftTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatRSuperiorTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
if get(handles.SatLSuperiorTog,'Value') == 1
    nSatBands = nSatBands + 1;
end
    
% --- Executes on button press in SaveBoxBands.
function SaveBoxBands_Callback(hObject, eventdata, handles)
% hObject    handle to SaveBoxBands (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
globalDefs
boxLoc = get_Box_loc(handles.saggitalBoxPos, handles.axialBoxPos, handles.qform_atlas, handles.nrow, handles.nslice);

[boxParam, boxStruct] = getBoxAsThreePlanes(handles.saggitalBoxPos, handles.axialBoxPos, handles.qform_atlas, handles.nrow, handles.nslice)
% No 6 Plane saving
% templateXML = 'box_sat_template4all.xml';
% BoxSatXMLStruct = svk_parse_xml_sats(templateXML, 0);
% for k = 1:length(BoxSatXMLStruct.pressBoxSatsStruct)
%     BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.normal_x = boxLoc(k,1);
%     BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.normal_y = boxLoc(k,2);
%     BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.normal_z = boxLoc(k,3);
%     BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.distance_from_origin = boxLoc(k,4);
%     BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.thickness = 0;
% end

nSatBands = getNumberofSatBands(handles);
SATBandsStruct = XMLReader.createEmptyDataStruct(nSatBands);
satNotUsed = 0;
cntr = 1;
if get(handles.SatAnterior1Tog,'Value') == 1;
    pts = handles.saggitalSatAnterior1Pos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(1).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(1).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(1).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(1).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(1).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatAnterior1';
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;
else
%     BoxSatXMLStruct.autoSatsStruct(1) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatAnterior2Tog,'Value') == 1;
    pts = handles.saggitalSatAnterior2Pos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
        
    handles.saggitalSatAnterior2Pos = pts;
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(2-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(2-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(2-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(2-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(2-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatAnterior2'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;    
else
%     BoxSatXMLStruct.autoSatsStruct(2-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatPosterior1Tog,'Value') == 1;
    pts = handles.saggitalSatPosterior1Pos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
% No 6 Plane saving    
%     BoxSatXMLStruct.autoSatsStruct(3-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(3-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(3-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(3-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(3-satNotUsed).sat_band.thickness = thickness;
%     
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatPosterior1'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;    
else
%     BoxSatXMLStruct.autoSatsStruct(3-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatPosterior2Tog,'Value') == 1;
    pts = handles.saggitalSatPosterior2Pos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
  % No 6 Plane saving       
%     BoxSatXMLStruct.autoSatsStruct(4-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(4-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(4-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(4-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(4-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatPosterior2'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;   
else
%     BoxSatXMLStruct.autoSatsStruct(4-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatSuperiorTog,'Value') == 1;
    pts = handles.saggitalSatSuperiorPos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
   % No 6 Plane saving  
%     BoxSatXMLStruct.autoSatsStruct(5-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(5-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(5-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(5-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(5-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatSuperior'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;   
else
%     BoxSatXMLStruct.autoSatsStruct(5-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatInferiorTog,'Value') == 1;
    pts = handles.saggitalSatInferiorPos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
% No 6 Plane saving         
%     BoxSatXMLStruct.autoSatsStruct(6-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(6-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(6-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(6-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(6-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatInferior'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;  
else
%     BoxSatXMLStruct.autoSatsStruct(6-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatASuperiorTog,'Value') == 1;
    pts = handles.saggitalSatASuperiorPos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving 
%     BoxSatXMLStruct.autoSatsStruct(7-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(7-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(7-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(7-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(7-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatASuperior'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;  
else
%     BoxSatXMLStruct.autoSatsStruct(7-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatPSuperiorTog,'Value') == 1;
    pts = handles.saggitalSatPSuperiorPos(1:7,:);
    [normal distance] = get_saggitalSat_loc(pts, handles.qform_atlas, handles.nrow, handles.nslice);
    normal = [0 normal];
    thickness = min(pts(6,1), pts(7,1));
   % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(8-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(8-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(8-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(8-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(8-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatPSuperior'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;  
else
%     BoxSatXMLStruct.autoSatsStruct(8-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end




if get(handles.SatRightTog,'Value') == 1;
    pts = handles.axialSatRightPos(1:7,:);
    [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
    normal = [normal 0]';
    center = [center 0]';
    BETA = handles.SatRightRot;
    Ry = [cosd(BETA)    0   sind(BETA)
        0           1      0
        -sind(BETA)    0   cosd(BETA)];
    normal = Ry*normal;
    center = Ry*center;
    distance = dot(center, normal);
    thickness = min(pts(6,1), pts(7,1));
% No 6 Plane saving
    %     BoxSatXMLStruct.autoSatsStruct(9-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(9-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(9-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(9-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(9-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatRight'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1; 
else
%     BoxSatXMLStruct.autoSatsStruct(9-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatLeftTog,'Value') == 1;
    pts = handles.axialSatLeftPos(1:7,:);
    [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
    normal = [normal 0]';
    center = [center 0]';
    BETA = handles.SatLeftRot;
    Ry = [cosd(BETA)    0   sind(BETA)
        0           1      0
        -sind(BETA)    0   cosd(BETA)];
    normal = Ry*normal;
    center = Ry*center;
    distance = dot(center, normal);
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(10-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(10-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(10-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(10-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(10-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatLeft'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;     
else
%     BoxSatXMLStruct.autoSatsStruct(10-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatARightTog,'Value') == 1;
    pts = handles.axialSatARightPos(1:7,:);
    [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
    normal = [normal 0]';
    center = [center 0]';
    BETA = handles.SatARightRot;
    Ry = [cosd(BETA)    0   sind(BETA)
        0           1      0
        -sind(BETA)    0   cosd(BETA)];
    normal = Ry*normal;
    center = Ry*center;
    distance = dot(center, normal);
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(11-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(11-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(11-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(11-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(11-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatARight'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;      
else
%     BoxSatXMLStruct.autoSatsStruct(11-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatPRightTog,'Value') == 1;
    pts = handles.axialSatPRightPos(1:7,:);
    [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
    normal = [normal 0]';
    center = [center 0]';
    BETA = handles.SatPRightRot;
    Ry = [cosd(BETA)    0   sind(BETA)
        0           1      0
        -sind(BETA)    0   cosd(BETA)];
    normal = Ry*normal;
    center = Ry*center;
    distance = dot(center, normal);
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(12-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(12-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(12-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(12-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(12-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatPRight'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;     
else
%     BoxSatXMLStruct.autoSatsStruct(12-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatALeftTog,'Value') == 1;
    pts = handles.axialSatALeftPos(1:7,:);
    [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
    normal = [normal 0]';
    center = [center 0]';
    BETA = handles.SatALeftRot;
    Ry = [cosd(BETA)    0   sind(BETA)
        0           1      0
        -sind(BETA)    0   cosd(BETA)];
    normal = Ry*normal;
    center = Ry*center;
    distance = dot(center, normal);
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(13-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(13-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(13-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(13-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(13-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatALeft'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;      
else
%     BoxSatXMLStruct.autoSatsStruct(13-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatPLeftTog,'Value') == 1;
    pts = handles.axialSatPLeftPos(1:7,:);
    [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
    normal = [normal 0]';
    center = [center 0]';
    BETA = handles.SatPLeftRot;
    Ry = [cosd(BETA)    0   sind(BETA)
        0           1      0
        -sind(BETA)    0   cosd(BETA)];
    normal = Ry*normal;
    center = Ry*center;
    distance = dot(center, normal);
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(14-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(14-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(14-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(14-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(14-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatPLeft'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;     
else
%     BoxSatXMLStruct.autoSatsStruct(14-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatRSuperiorTog,'Value') == 1;
    pts = handles.coronalSatRSuperiorPos(1:7,:);
    [normal distance] = get_coronalSat_loc(pts, handles.qform_atlas, handles.nslice);
    normal = [normal(1), 0, normal(2)];
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(15-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(15-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(15-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(15-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(15-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatRSuperior'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1;     
else
%     BoxSatXMLStruct.autoSatsStruct(15-satNotUsed) = [];
    satNotUsed = satNotUsed + 1;
end

if get(handles.SatLSuperiorTog,'Value') == 1;
    pts = handles.coronalSatLSuperiorPos(1:7,:);
    [normal distance] = get_coronalSat_loc(pts, handles.qform_atlas, handles.nslice);
    normal = [normal(1), 0, normal(2)];
    thickness = min(pts(6,1), pts(7,1));
    % No 6 Plane saving
%     BoxSatXMLStruct.autoSatsStruct(16-satNotUsed).sat_band.normal_x = normal(1);
%     BoxSatXMLStruct.autoSatsStruct(16-satNotUsed).sat_band.normal_y = normal(2);
%     BoxSatXMLStruct.autoSatsStruct(16-satNotUsed).sat_band.normal_z = normal(3);
%     BoxSatXMLStruct.autoSatsStruct(16-satNotUsed).sat_band.distance_from_origin = distance;
%     BoxSatXMLStruct.autoSatsStruct(16-satNotUsed).sat_band.thickness = thickness;
    
    SATBandsStruct(cntr).id = num2str(cntr);
    SATBandsStruct(cntr).label = 'SatLSuperior'
    SATBandsStruct(cntr).normal_x = num2str(normal(1));
    SATBandsStruct(cntr).normal_y = num2str(normal(2));
    SATBandsStruct(cntr).normal_z = num2str(normal(3));
    SATBandsStruct(cntr).thickness = num2str(thickness);
    SATBandsStruct(cntr).distance_from_origin = num2str(distance);

    cntr = cntr + 1; 
else
    ;
    %BoxSatXMLStruct.autoSatsStruct(16-satNotUsed) = [];
end

ACTIVATE_OLD_XML_VERSION = 0;
if (ACTIVATE_OLD_XML_VERSION == 1)

    [templateXML, fPath] = uigetfile('*.xml', 'Select Your Template XML file', ...
                              handles.atlas_dir);
    if fPath==0
        msgbox('Must select a template XML file', 'File Selector', 'error');
        return
    end
    cd(fPath);
    outputName = uiputfile('*.xml','Save Your Current Prescription As');
    svk_write_xml_sats(templateXML, outputName, BoxSatXMLStruct);

end

outputName = uiputfile('*.xml','Save Your Prescription As 3Plane-XML');
writer = XMLWriter();
writer.Initialize(); % write header

writer.writePRESSBoxSatBands(boxStruct)
writer.writeAutoSatBands(SATBandsStruct);
XMLWriter.writeXMLFile(writer.xmlObject, outputName)




% save box and sat band position in a .mat file, which will be used when retrieve the prescription in MatLab only

% Box position
axialBoxPos = handles.axialBoxPos;
saggitalBoxPos = handles.saggitalBoxPos;
coronalBoxPos = handles.coronalBoxPos;

% Sat bands defined on the saggital plane (total number: 8)
saggitalSatAnterior1Pos = handles.saggitalSatAnterior1Pos;
saggitalSatAnterior2Pos = handles.saggitalSatAnterior2Pos;
saggitalSatPosterior1Pos = handles.saggitalSatPosterior1Pos;
saggitalSatPosterior2Pos = handles.saggitalSatPosterior2Pos;
saggitalSatSuperiorPos = handles.saggitalSatSuperiorPos;
saggitalSatASuperiorPos = handles.saggitalSatASuperiorPos;
saggitalSatPSuperiorPos = handles.saggitalSatPSuperiorPos;
saggitalSatInferiorPos = handles.saggitalSatInferiorPos;

% Sat bands defined on the axial plane (total number: 4)
axialSatRightPos = handles.axialSatRightPos;
axialSatARightPos = handles.axialSatARightPos;
axialSatPRightPos = handles.axialSatPRightPos ;
axialSatLeftPos = handles.axialSatLeftPos;
axialSatALeftPos = handles.axialSatALeftPos;
axialSatPLeftPos = handles.axialSatPLeftPos;

% Sat bands defined on the coronal plane (total number: 2)
coronalSatRSuperiorPos = handles.coronalSatRSuperiorPos;
coronalSatLSuperiorPos = handles.coronalSatLSuperiorPos;

% Save prescription to a .mat file
% % matfile = outputName(1:end-4);
% % save(matfile, 'axialBoxPos','saggitalBoxPos','coronalBoxPos', 'saggitalSatAnterior1Pos', 'saggitalSatAnterior2Pos',...
% %     'saggitalSatPosterior1Pos', 'saggitalSatPosterior2Pos', 'saggitalSatASuperiorPos', 'saggitalSatPSuperiorPos', ...
% %     'saggitalSatInferiorPos', 'saggitalSatSuperiorPos', 'axialSatRightPos', 'axialSatARightPos', 'axialSatPRightPos', ...
% %     'axialSatLeftPos', 'axialSatALeftPos', 'axialSatPLeftPos', 'coronalSatRSuperiorPos','coronalSatLSuperiorPos');
% % cd(handles.atlas_dir);

% --- Executes on button press in togglebutton_Press_Box.
function togglebutton_Press_Box_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_Press_Box (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_Press_Box

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    % toggle button is pressed
     
    % Display the center axial slice 
    axes(handles.axialAx);
    centerSlice = round(handles.nslice/2);
    set(handles.axialSlider,'value', centerSlice);
    handles.axialShow = imshow(handles.axialImg(:,:,centerSlice), []);
    
    % Display the center saggital slice
    axes(handles.saggitalAx);
    centerSlice = round(handles.ncol/2);
    set(handles.saggitalSlider,'value',centerSlice);
    handles.saggitalShow = imshow(handles.saggitalImg(:,:,centerSlice), []);

    % Display the center coronal slice
    axes(handles.coronalAx);
    centerSlice = round(handles.nrow/2);
    set(handles.coronalSlider,'value',centerSlice);
    handles.coronalShow = imshow(handles.coronalImg(:,:,centerSlice), []);
    
    [handles.PressBox_Saggital,api]=imRectRot('hParent', handles.saggitalAx, 'pos',[handles.nrow/2-60 handles.nslice/2-30 120 60 0],'ellipse', 0,'rotate',1,'color','y','colorc','y'  );
    [pts,angle,pc,w,h] = rectToCorners( api.getPos());
    handles.saggitalBoxPos = [pts ; pc ; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_saggital(pos));
    handles.saggitalBoxDeleted = false;
    
    [handles.PressBox_Axial,api]=imRectRot('hParent', handles.axialAx, 'pos',[handles.ncol/2-60 handles.nrow/2-60 120 120 0],'ellipse', 0,'rotate',0,'color','y','colorc','y'  );
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc,w,h] = rectToCorners( api.getPos());
    handles.axialBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_axial(pos));
    handles.axialBoxDeleted = false;
    
    [handles.PressBox_Coronal,api]=imRectRot('hParent', handles.coronalAx, 'pos',[handles.axialBoxPos(1,1) handles.saggitalBoxPos(1,2) 120 60 0],'ellipse', 0,'rotate',0,'color','y','colorc','y'  );
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc,w,h] = rectToCorners( api.getPos());
    handles.coronalBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_coronal(pos));
    handles.coronalBoxDeleted = false;

    set(handles.showRL, 'String', num2str(handles.axialBoxPos(6,1)*handles.pixelSize(1), '%5.1f'));
    set(handles.showAP, 'String', num2str(handles.saggitalBoxPos(6,1)*handles.pixelSize(2), '%5.1f'));
    set(handles.showSI, 'String', num2str(handles.saggitalBoxPos(7,1)*handles.pixelSize(3), '%5.1f'));

elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    if ~handles.saggitalBoxDeleted
        delete(handles.PressBox_Saggital);
        handles.saggitalBoxPos = [];
        handles.saggitalBoxDeleted = true;
    end
    if ~handles.axialBoxDeleted
        delete(handles.PressBox_Axial);
        handles.axialBoxPos = [];
        handles.axialBoxDeleted = true;
    end
    if ~handles.coronalBoxDeleted
        delete(handles.PressBox_Coronal);
        handles.coronalBoxPos = [];
        handles.CoronalBoxDeleted = true;
    end
    set(handles.showRL, 'String', '');
    set(handles.showAP, 'String', '');
    set(handles.showSI, 'String', '');
end
guidata(hObject, handles);




% --- Executes on button press in togglebutton_SatPSuperior.
function togglebutton_SatPSuperior_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatPSuperior (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatPSuperior

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow-80 handles.nslice/2-550 45 1000 -60];
    [handles.SatBand_PSuperior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatPSuperior(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatPSuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_PSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_PSuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_PSuperior_Saggital);
    delete(handles.SatBand_PSuperior_Axial);
    delete(handles.SatBand_PSuperior_Coronal);
    handles.saggitalSatPSuperiorPos = [];
end
guidata(hObject, handles);


% --- Executes on button press in togglebutton_SatAnterior1.
function togglebutton_SatAnterior1_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatAnterior1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatAnterior1

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow/2-80 handles.nslice/2-500 45 1000 20];
    [handles.SatBand_Anterior1_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatAnterior1(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatAnterior1Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior1_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior1_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Anterior1_Saggital);
    delete(handles.SatBand_Anterior1_Axial);
    delete(handles.SatBand_Anterior1_Coronal);
    handles.saggitalSatAnterior1Pos = [];
end
   
guidata(hObject, handles);



% --- Executes on button press in togglebutton_SatPosterior1.
function togglebutton_SatPosterior1_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatPosterior1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatPosterior1
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow-80 handles.nslice/2-500 45 1000 -20];
    [handles.SatBand_Posterior1_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatPosterior1(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatPosterior1Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior1_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior1_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Posterior1_Saggital);
    delete(handles.SatBand_Posterior1_Axial);
    delete(handles.SatBand_Posterior1_Coronal);
    handles.saggitalSatPosterior1Pos = [];
end
guidata(hObject, handles);


% --- Executes on button press in togglebutton_SatARight.
function togglebutton_SatARight_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatARight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatARight
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol/2-80 handles.nrow/2-550 45 1000 45];
    [handles.SatBand_ARight_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatARight(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatARightPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_ARight_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    %satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_ARight_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatARight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_ARight_Saggital);
    delete(handles.SatBand_ARight_Axial);
    delete(handles.SatBand_ARight_Coronal);
    handles.axialSatARightPos = [];
    handles.SatARightCenter = [];
    handles.SatARightRot = 0;
end
guidata(hObject, handles);



% --- Executes on button press in togglebutton_SatPLeft.
function togglebutton_SatPLeft_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatPLeft (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatPLeft

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol-80 handles.nrow/2-450 45 1000 45];
    [handles.SatBand_PLeft_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatPLeft(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatPLeftPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_PLeft_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_PLeft_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatPLeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_PLeft_Saggital);
    delete(handles.SatBand_PLeft_Axial);
    delete(handles.SatBand_PLeft_Coronal);
    handles.axialSatPLeftPos = [];
    handles.SatPLeftCenter = [];
    handles.SatPLeftRot = 0;
end
guidata(hObject, handles);

% --- Executes on button press in togglebutton_SatPRight.
function togglebutton_SatPRight_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatPRight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatPRight

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol/2-80 handles.nrow/2-450 45 1000 -45];
    [handles.SatBand_PRight_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatPRight(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatPRightPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_PRight_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_PRight_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatPRight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_PRight_Saggital);
    delete(handles.SatBand_PRight_Axial);
    delete(handles.SatBand_PRight_Coronal);
    handles.axialSatPRightPos = [];
    handles.SatPRightCenter = [];
    handles.SatPRightRot = 0;
end
guidata(hObject, handles);

% --- Executes on button press in togglebutton_SatALeft.
function togglebutton_SatALeft_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatALeft (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatALeft

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol-80 handles.nrow/2-550 45 1000 -45];
    [handles.SatBand_ALeft_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatALeft(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatALeftPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_ALeft_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_ALeft_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatALeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_ALeft_Saggital);
    delete(handles.SatBand_ALeft_Axial);
    delete(handles.SatBand_ALeft_Coronal);
    handles.axialSatALeftPos = [];
    handles.SatALeftCenter = [];
    handles.SatALeftRot = 0;
end
guidata(hObject, handles);

% --- Executes on button press in togglebutton_SatInferior.
function togglebutton_SatInferior_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatInferior (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatInferior

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow/2-20 handles.nslice/2-450 45 1000 90];
    [handles.SatBand_Inferior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatInferior(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatInferiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Inferior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Inferior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Inferior_Saggital);
    delete(handles.SatBand_Inferior_Axial);
    delete(handles.SatBand_Inferior_Coronal);
    handles.saggitalSatInferiorPos = [];
end
guidata(hObject, handles);


% --- Executes on button press in togglebutton_SatASuperior.
function togglebutton_SatASuperior_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SatASuperior (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SatASuperior

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow/2-80 handles.nslice/2-550 45 1000 60];
    [handles.SatBand_ASuperior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatASuperior(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatASuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_ASuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_ASuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_ASuperior_Saggital);
    delete(handles.SatBand_ASuperior_Axial);
    delete(handles.SatBand_ASuperior_Coronal);
    handles.saggitalSatASuperiorPos = [];
end
guidata(hObject, handles);



% --- Executes on slider movement.
function coronalSlider_Callback(hObject, eventdata, handles)
% hObject    handle to coronalSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

slice = get(handles.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
set(handles.coronalShow, 'CData',handles.coronalImg(:,:,slice));

interPlane = compute_box_interplane_coronal(handles.saggitalBoxPos(1:4,:));
idx = find(interPlane(:,1)==slice);
if ~isempty(idx)
    if ~handles.coronalBoxDeleted
        delete(handles.PressBox_Coronal);
    end
    boxHeight = interPlane(idx,3) - interPlane(idx,2);
    boxPos = [handles.coronalBoxPos(1,1) interPlane(idx,2)  handles.coronalBoxPos(6,1) boxHeight handles.coronalBoxPos(8,1)];
    [handles.PressBox_Coronal api]=imRectRot('hParent', handles.coronalAx, 'pos', boxPos , 'ellipse', 0,'rotate',0,'color','y','colorc','y'  );
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc, w, h] = rectToCorners( api.getPos());
    handles.coronalBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_coronal(pos));
    handles.coronalBoxDeleted = false;
else
    if ~handles.coronalBoxDeleted
        delete(handles.PressBox_Coronal);
        handles.coronalBoxDeleted = true;
    end
end

if get(handles.SatAnterior1Tog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatAnterior1Pos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Anterior1_Coronal);
    [handles.SatBand_Anterior1_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatAnterior2Tog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatAnterior2Pos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Anterior2_Coronal);
    [handles.SatBand_Anterior2_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPosterior1Tog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatPosterior1Pos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Posterior1_Coronal);
    [handles.SatBand_Posterior1_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPosterior2Tog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatPosterior2Pos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Posterior2_Coronal);
    [handles.SatBand_Posterior2_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatASuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatASuperiorPos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_ASuperior_Coronal);
    [handles.SatBand_ASuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatPSuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatPSuperiorPos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_PSuperior_Coronal);
    [handles.SatBand_PSuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatInferiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatInferiorPos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Inferior_Coronal);
    [handles.SatBand_Inferior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatSuperiorTog,'Value') == 1;
    interPlane = compute_sat_interplane_saggital(handles.saggitalSatSuperiorPos(1:4,:),handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    delete(handles.SatBand_Superior_Coronal);
    [handles.SatBand_Superior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
end

if get(handles.SatARightTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.axialSatARightPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500 satWidth 1000 handles.SatARightRot];
    delete(handles.SatBand_ARight_Coronal);
    [handles.SatBand_ARight_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatARight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
end

if get(handles.SatRightTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.axialSatRightPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500 satWidth 1000 handles.SatRightRot];
    delete(handles.SatBand_Right_Coronal);
    [handles.SatBand_Right_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatRight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
end


if get(handles.SatPRightTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.axialSatPRightPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 handles.SatPRightRot];
    delete(handles.SatBand_PRight_Coronal);
    [handles.SatBand_PRight_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatPRight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
end

if get(handles.SatALeftTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.axialSatALeftPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 handles.SatALeftRot];
    delete(handles.SatBand_ALeft_Coronal);
    [handles.SatBand_ALeft_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatALeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
end

if get(handles.SatLeftTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.axialSatLeftPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 handles.SatLeftRot];
    delete(handles.SatBand_Left_Coronal);
    [handles.SatBand_Left_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatLeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
end

if get(handles.SatPLeftTog,'Value') == 1;
    interPlane = compute_sat_interplane_axial(handles.axialSatPLeftPos(1:4,:), handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 handles.SatPLeftRot];
    delete(handles.SatBand_PLeft_Coronal);
    [handles.SatBand_PLeft_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatPLeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
end


% Update handles structure
guidata(gcbo, handles);

% --- Executes during object creation, after setting all properties.
function coronalSlider_CreateFcn(hObject, eventdata, handles)
% hObject    handle to coronalSlider (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end


% --- Executes on button press in LoadBoxBands.
function LoadBoxBands_Callback(hObject, eventdata, handles)
% hObject    handle to LoadBoxBands (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if get(handles.BoxTog,'Value') == 1;
    if ~handles.axialBoxDeleted
        delete(handles.PressBox_Axial);
        handles.axialBoxDeleted = true;
    end
    if ~handles.saggitalBoxDeleted
        delete(handles.PressBox_Saggital);
        handles.saggitalBoxDeleted = true;
    end
    if ~handles.coronalBoxDeleted
        delete(handles.PressBox_Coronal);
        handles.coronalBoxDeleted = true;
    end
    set(handles.BoxTog,'Value',0);
    set(handles.showRL, 'String', '');
    set(handles.showAP, 'String', '');
    set(handles.showSI, 'String', '');
end

if get(handles.SatPSuperiorTog,'Value') == 1;
    delete(handles.SatBand_PSuperior_Saggital);
    delete(handles.SatBand_PSuperior_Axial);
    delete(handles.SatBand_PSuperior_Coronal);
    set(handles.SatPSuperiorTog,'Value',0);
end

if get(handles.SatAnterior1Tog,'Value') == 1;
    delete(handles.SatBand_Anterior1_Saggital);
    delete(handles.SatBand_Anterior1_Axial);
    delete(handles.SatBand_Anterior1_Coronal);
    set(handles.SatAnterior1Tog,'Value',0)
end

if get(handles.SatAnterior2Tog,'Value') == 1;
    delete(handles.SatBand_Anterior2_Saggital);
    delete(handles.SatBand_Anterior2_Axial);
    delete(handles.SatBand_Anterior2_Coronal);
    set(handles.SatAnterior2Tog,'Value',0)
end

if get(handles.SatPosterior1Tog,'Value') == 1;
    delete(handles.SatBand_Posterior1_Saggital);
    delete(handles.SatBand_Posterior1_Axial);
    delete(handles.SatBand_Posterior1_Coronal);
    set(handles.SatPosterior1Tog,'Value',0)
end

if get(handles.SatPosterior2Tog,'Value') == 1;
    delete(handles.SatBand_Posterior2_Saggital);
    delete(handles.SatBand_Posterior2_Axial);
    delete(handles.SatBand_Posterior2_Coronal);
    set(handles.SatPosterior2Tog,'Value',0)
end

if get(handles.SatInferiorTog,'Value') == 1;
    delete(handles.SatBand_Inferior_Saggital);
    delete(handles.SatBand_Inferior_Axial);
    delete(handles.SatBand_Inferior_Coronal);
    set(handles.SatInferiorTog,'Value',0);
end

if get(handles.SatSuperiorTog,'Value') == 1;
    delete(handles.SatBand_Superior_Saggital);
    delete(handles.SatBand_Superior_Axial);
    delete(handles.SatBand_Superior_Coronal);
    set(handles.SatSuperiorTog,'Value',0);
end


if get(handles.SatASuperiorTog,'Value') == 1;
    delete(handles.SatBand_ASuperior_Saggital);
    delete(handles.SatBand_ASuperior_Axial);
    delete(handles.SatBand_ASuperior_Coronal);
    set(handles.SatASuperiorTog,'Value',0);
end

if get(handles.SatARightTog,'Value') == 1;
    delete(handles.SatBand_ARight_Saggital);
    delete(handles.SatBand_ARight_Axial);
    delete(handles.SatBand_ARight_Coronal);
    set(handles.SatARightTog,'Value',0)
    handles.SatARightCenter = [];
    handles.SatARightRot = 0;
end

if get(handles.SatRSuperiorTog,'Value') == 1;
    delete(handles.SatBand_RSuperior_Saggital);
    delete(handles.SatBand_RSuperior_Axial);
    delete(handles.SatBand_RSuperior_Coronal);
    set(handles.SatRSuperiorTog,'Value',0)
end

if get(handles.SatLSuperiorTog,'Value') == 1;
    delete(handles.SatBand_LSuperior_Saggital);
    delete(handles.SatBand_LSuperior_Axial);
    delete(handles.SatBand_LSuperior_Coronal);
    set(handles.SatLSuperiorTog,'Value',0)
end

if get(handles.SatRightTog,'Value') == 1;
    delete(handles.SatBand_Right_Saggital);
    delete(handles.SatBand_Right_Axial);
    delete(handles.SatBand_Right_Coronal);
    set(handles.SatRightTog,'Value',0)
    handles.SatRightCenter = [];
    handles.SatRightRot = 0;
end


if get(handles.SatPLeftTog,'Value') == 1;
    delete(handles.SatBand_PLeft_Saggital);
    delete(handles.SatBand_PLeft_Axial);
    delete(handles.SatBand_PLeft_Coronal);
    set(handles.SatPLeftTog,'Value',0)
    handles.SatPLeftCenter = [];
    handles.SatPLeftRot = 0;
end

if get(handles.SatPRightTog,'Value') == 1;
    delete(handles.SatBand_PRight_Saggital);
    delete(handles.SatBand_PRight_Axial);
    delete(handles.SatBand_PRight_Coronal);
    set(handles.SatPRightTog,'Value',0)
    handles.SatPRightCenter = [];
    handles.SatPRightRot = 0;
end

if get(handles.SatALeftTog,'Value') == 1;
    delete(handles.SatBand_ALeft_Saggital);
    delete(handles.SatBand_ALeft_Axial);
    delete(handles.SatBand_ALeft_Coronal);
    set(handles.SatALeftTog,'Value',0);
    handles.SatALeftCenter = [];
    handles.SatALeftRot = 0;
end

if get(handles.SatLeftTog,'Value') == 1;
    delete(handles.SatBand_Left_Saggital);
    delete(handles.SatBand_Left_Axial);
    delete(handles.SatBand_Left_Coronal);
    set(handles.SatLeftTog,'Value',0);
    handles.SatLeftCenter = [];
    handles.SatLeftRot = 0;
end

% [fName, fPath] = uigetfile('.mat');
% load(strcat(fPath,fName));

[fName, fPath] = uigetfile('.xml');
r=XMLReader(strcat(fPath,fName));
             
saggitalSatAnterior1Pos = [];
saggitalSatAnterior2Pos = [];
saggitalSatPosterior1Pos = [];
saggitalSatPosterior2Pos = [];
saggitalSatSuperiorPos = [];
saggitalSatASuperiorPos = [];
saggitalSatPSuperiorPos = [];
saggitalSatInferiorPos = [];

axialSatRightPos = [];
axialSatARightPos = [];
axialSatPRightPos = [];
axialSatLeftPos = [];
axialSatALeftPos = [];
axialSatPLeftPos = [];

coronalSatRSuperiorPos = [];
coronalSatLSuperiorPos = [];

CASpressBox = XMLReader.orderCAS(r.getPRESSBoxSatBands());
[saggitalBoxPos, axialBoxPos, coronalBoxPos] = ...
            getBoxPositionsFromXML(CASpressBox, handles.qform_atlas, ...
                                   handles.nrow, handles.nslice)




AutoSatsStruct = r.getAutoSatBands();
% saggitalSatAnterior2Pos = f(nx,ny,nz,th,d);               
for (i=1:size(AutoSatsStruct,2))
    currEl = AutoSatsStruct(i);
    normal = [str2num(currEl.normal_x); str2num(currEl.normal_y); str2num(currEl.normal_z)];
    distance = str2num(currEl.distance_from_origin); 
    thickness = str2num(currEl.thickness);
        
    if     (strcmp(currEl.label, 'SatAnterior1'))
        saggitalSatAnterior1Pos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);
    elseif (strcmp(currEl.label, 'SatAnterior2'))
        saggitalSatAnterior2Pos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);
    elseif (strcmp(currEl.label, 'SatPosterior1'))
        saggitalSatPosterior1Pos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);                                                  
    elseif (strcmp(currEl.label, 'SatPosterior2'))
        saggitalSatPosterior2Pos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);
    elseif (strcmp(currEl.label, 'SatSuperior'))
        saggitalSatSuperiorPos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);                                              
    elseif (strcmp(currEl.label, 'SatASuperior'))
        saggitalSatASuperiorPos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);
    elseif (strcmp(currEl.label, 'SatPSuperior'))
        saggitalSatPSuperiorPos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);
    elseif (strcmp(currEl.label, 'SatInferior'))
        saggitalSatInferiorPos = get_saggitalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);                                              
    elseif (strcmp(currEl.label, 'SatRSuperior'))
        coronalSatRSuperiorPos = get_coronalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice);
    elseif (strcmp(currEl.label, 'SatLSuperior'))
        coronalSatLSuperiorPos = get_coronalSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice); 
                                              
    elseif (strcmp(currEl.label, 'SatRight'))
        axialSatRightPos = get_axialSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice, pi/2); 
%     [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
%     normal = [normal 0]';
%     center = [center 0]';
%     BETA = handles.SatALeftRot;
%     Ry = [cosd(BETA)    0   sind(BETA)
%         0           1      0
%         -sind(BETA)    0   cosd(BETA)];
%     normal = Ry*normal;
%     center = Ry*center;
%     distance = dot(center, normal);
%     thickness = min(pts(6,1), pts(7,1));
    elseif (strcmp(currEl.label, 'SatARight'))
        axialSatARightPos = get_axialSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice,0);
    elseif (strcmp(currEl.label, 'SatPRight'))
        axialSatPRightPos = get_axialSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice,0);
    elseif (strcmp(currEl.label, 'SatLeft'))
        axialSatLeftPos = get_axialSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice,pi/2);  
%     [normal center] = get_axialSat_loc(pts, handles.qform_atlas, handles.nrow);
%     normal = [normal 0]';
%     center = [center 0]';
%     BETA = handles.SatALeftRot;
%     Ry = [cosd(BETA)    0   sind(BETA)
%         0           1      0
%         -sind(BETA)    0   cosd(BETA)];
%     normal = Ry*normal;
%     center = Ry*center;
%     distance = dot(center, normal);
%     thickness = min(pts(6,1), pts(7,1));                                              
    elseif (strcmp(currEl.label, 'SatALeft'))
        axialSatALeftPos = get_axialSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice,0);
    elseif (strcmp(currEl.label, 'SatPLeft'))
        axialSatPLeftPos = get_axialSatPosXML(  normal, distance, ...
                                                  thickness, handles.qform_atlas, ...
                                                  handles.nrow, handles.nslice,0);                                               
    end
    
end
      


% Display the center axial slice
axes(handles.axialAx);
[~, I] = sort(saggitalBoxPos(1:4,2));
centerSlice_axial = round((saggitalBoxPos(I(1),2)+saggitalBoxPos(I(4),2))/2);
set(handles.axialSlider,'value', centerSlice_axial);
handles.axialShow = imshow(handles.axialImg(:,:,centerSlice_axial), []);

% Display the center saggital slice
axes(handles.saggitalAx);
[~, I] = sort(axialBoxPos(1:4,1));
centerSlice_saggital = round((axialBoxPos(I(1),1)+axialBoxPos(I(4),1))/2);
set(handles.saggitalSlider,'value',centerSlice_saggital);
handles.saggitalShow = imshow(handles.saggitalImg(:,:,centerSlice_saggital), []);

% Display the center coronal slice
axes(handles.coronalAx);
[~, I] = sort(saggitalBoxPos(1:4,1));
centerSlice_coronal = round((saggitalBoxPos(I(1),1)+saggitalBoxPos(I(4),1))/2);
set(handles.coronalSlider,'value',centerSlice_coronal);
handles.coronalShow = imshow(handles.coronalImg(:,:,centerSlice_coronal), []);

w = saggitalBoxPos(6,1);
h = saggitalBoxPos(7,1);
boxPos = [saggitalBoxPos(5,1)-w/2 saggitalBoxPos(5,2)-h/2 w h saggitalBoxPos(8,1)];
[handles.PressBox_Saggital,api]=imRectRot('hParent', handles.saggitalAx, 'pos', boxPos,'ellipse', 0,'rotate',1,'color','y','colorc','y'  );
[pts,angle,pc,w,h] = rectToCorners( api.getPos());
handles.saggitalBoxPos = [pts ; pc ; w w ; h h; angle angle];
api.setPosChnCb( @(pos) redrawBox_saggital(pos));
handles.saggitalBoxDeleted = false;

interPlane = compute_interplane(pts);
idx = find(interPlane(:,1)==centerSlice_axial);
if ~isempty(idx)
    boxHeight = interPlane(idx,3) - interPlane(idx,2);
    boxPos = [axialBoxPos(1,1) interPlane(idx,2)  axialBoxPos(6,1) boxHeight axialBoxPos(8,1)];
    [handles.PressBox_Axial api]=imRectRot('hParent', handles.axialAx, 'pos', boxPos , 'ellipse', 0,'rotate',0,'color','y', 'colorc','y');
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc, w, h] = rectToCorners( api.getPos());
    handles.axialBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_axial(pos));
    handles.axialBoxDeleted = false;
end

interPlane = compute_box_interplane_coronal(handles.saggitalBoxPos(1:4,:));
idx = find(interPlane(:,1)==centerSlice_coronal);
if ~isempty(idx)
    boxHeight = interPlane(idx,3) - interPlane(idx,2);
    boxPos = [coronalBoxPos(1,1) interPlane(idx,2)  coronalBoxPos(6,1) boxHeight coronalBoxPos(8,1)];
    [handles.PressBox_Coronal api]=imRectRot('hParent', handles.coronalAx, 'pos', boxPos , 'ellipse', 0,'rotate',0,'color','y', 'colorc','y');
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc, w, h] = rectToCorners( api.getPos());
    handles.coronalBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_coronal(pos));
    handles.coronalBoxDeleted = false;
end


if (isempty(handles.axialBoxPos))
    handles.axialBoxPos = axialBoxPos;
end
if (isempty(handles.saggitalBoxPos))
    handles.saggitalBoxPos = saggitalBoxPos;
end
set(handles.showRL, 'String', num2str(handles.axialBoxPos(6,1)*handles.pixelSize(1), '%5.1f'));
set(handles.showAP, 'String', num2str(handles.saggitalBoxPos(6,1)*handles.pixelSize(2), '%5.1f'));
set(handles.showSI, 'String', num2str(handles.saggitalBoxPos(7,1)*handles.pixelSize(3), '%5.1f'));
set(handles.BoxTog,'Value',1);






if ~isempty(saggitalSatAnterior1Pos)
    w = saggitalSatAnterior1Pos(6,1);
    h = saggitalSatAnterior1Pos(7,1);
    satPos = [saggitalSatAnterior1Pos(5,1)-w/2 saggitalSatAnterior1Pos(5,2)-h/2 w h saggitalSatAnterior1Pos(8,1)];
    [handles.SatBand_Anterior1_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatAnterior1(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatAnterior1Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior1_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior1_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatAnterior1Tog,'Value',1);
end

if ~isempty(saggitalSatAnterior2Pos)
    w = saggitalSatAnterior2Pos(6,1);
    h = saggitalSatAnterior2Pos(7,1);
    % just needs the center and angles
    satPos = [saggitalSatAnterior2Pos(5,1)-w/2 saggitalSatAnterior2Pos(5,2)-h/2 w h saggitalSatAnterior2Pos(8,1)];
    [handles.SatBand_Anterior2_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatAnterior2(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatAnterior2Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior2_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior2_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatAnterior2Tog,'Value',1);
end

if ~isempty(saggitalSatPosterior1Pos)
    w = saggitalSatPosterior1Pos(6,1);
    h = saggitalSatPosterior1Pos(7,1);
    satPos = [saggitalSatPosterior1Pos(5,1)-w/2 saggitalSatPosterior1Pos(5,2)-h/2 w h saggitalSatPosterior1Pos(8,1)];
    [handles.SatBand_Posterior1_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatPosterior1(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatPosterior1Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior1_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior1_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatPosterior1Tog,'Value',1);
end

if ~isempty(saggitalSatPosterior2Pos)
    w = saggitalSatPosterior2Pos(6,1);
    h = saggitalSatPosterior2Pos(7,1);
    satPos = [saggitalSatPosterior2Pos(5,1)-w/2 saggitalSatPosterior2Pos(5,2)-h/2 w h saggitalSatPosterior2Pos(8,1)];
    [handles.SatBand_Posterior2_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatPosterior2(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatPosterior2Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior2_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior2_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatPosterior2Tog,'Value',1);
end

if ~isempty(saggitalSatASuperiorPos)
    w = saggitalSatASuperiorPos(6,1);
    h = saggitalSatASuperiorPos(7,1);
    satPos = [saggitalSatASuperiorPos(5,1)-w/2 saggitalSatASuperiorPos(5,2)-h/2 w h saggitalSatASuperiorPos(8,1)];
    [handles.SatBand_ASuperior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatASuperior(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatASuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_ASuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_ASuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatASuperiorTog,'Value',1);
end

if ~isempty(saggitalSatPSuperiorPos)
    w = saggitalSatPSuperiorPos(6,1);
    h = saggitalSatPSuperiorPos(7,1);
    satPos = [saggitalSatPSuperiorPos(5,1)-w/2 saggitalSatPSuperiorPos(5,2)-h/2 w h saggitalSatPSuperiorPos(8,1)];
    [handles.SatBand_PSuperior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatPSuperior(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatPSuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_PSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_PSuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatPSuperiorTog,'Value',1);
end

if ~isempty(saggitalSatInferiorPos)
    w = saggitalSatInferiorPos(6,1);
    h = saggitalSatInferiorPos(7,1);
    satPos = [saggitalSatInferiorPos(5,1)-w/2 saggitalSatInferiorPos(5,2)-h/2 w h saggitalSatInferiorPos(8,1)];
    [handles.SatBand_Inferior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatInferior(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatInferiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Inferior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Inferior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1);
    set(handles.SatInferiorTog,'Value',1);
end

if ~isempty(saggitalSatSuperiorPos)
    w = saggitalSatSuperiorPos(6,1);
    h = saggitalSatSuperiorPos(7,1);
    satPos = [saggitalSatSuperiorPos(5,1)-w/2 saggitalSatSuperiorPos(5,2)-h/2 w h saggitalSatSuperiorPos(8,1)];
    [handles.SatBand_Superior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatSuperior(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatSuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Superior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Superior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1);
    set(handles.SatSuperiorTog,'Value',1);
end

if ~isempty(axialSatARightPos)
    w = axialSatARightPos(6,1);
    h = axialSatARightPos(7,1);
    satPos = [axialSatARightPos(5,1)-w/2 axialSatARightPos(5,2)-h/2 w h axialSatARightPos(8,1)];
    [handles.SatBand_ARight_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatARight(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatARightPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_ARight_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_ARight_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatARight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    set(handles.SatARightTog,'Value',1);
end

if ~isempty(coronalSatRSuperiorPos)
    w = coronalSatRSuperiorPos(6,1);
    h = coronalSatRSuperiorPos(7,1);
    satPos = [coronalSatRSuperiorPos(5,1)-w/2 coronalSatRSuperiorPos(5,2)-h/2 w h coronalSatRSuperiorPos(8,1)];
    [handles.SatBand_RSuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawCoronalSatRSuperior(pos));

    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.coronalSatRSuperiorPos = [pts ; pc; w w; h h; angle angle];
    
    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satHeight= interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
    [handles.SatBand_RSuperior_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_RSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatRSuperiorTog,'Value',1);
end

if ~isempty(coronalSatLSuperiorPos)
    w = coronalSatLSuperiorPos(6,1);
    h = coronalSatLSuperiorPos(7,1);
    satPos = [coronalSatLSuperiorPos(5,1)-w/2 coronalSatLSuperiorPos(5,2)-h/2 w h coronalSatLSuperiorPos(8,1)];
    [handles.SatBand_LSuperior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawCoronalSatLSuperior(pos));

    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.coronalSatLSuperiorPos = [pts ; pc; w w; h h; angle angle];
    
    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satHeight= interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
    [handles.SatBand_LSuperior_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_axial);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_LSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    set(handles.SatLSuperiorTog,'Value',1);
end

if ~isempty(axialSatRightPos)
    w = axialSatRightPos(6,1);
    h = axialSatRightPos(7,1);
    satPos = [axialSatRightPos(5,1)-w/2 axialSatRightPos(5,2)-h/2 w h axialSatRightPos(8,1)];
    [handles.SatBand_Right_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatRight(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatRightPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_Right_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_Right_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatRight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    set(handles.SatRightTog,'Value',1);
end

if ~isempty(axialSatPRightPos)
    w = axialSatPRightPos(6,1);
    h = axialSatPRightPos(7,1);
    satPos = [axialSatPRightPos(5,1)-w/2 axialSatPRightPos(5,2)-h/2 w h axialSatPRightPos(8,1)];
    [handles.SatBand_PRight_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatPRight(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatPRightPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_PRight_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_PRight_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatPRight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    set(handles.SatPRightTog,'Value',1);
end

if ~isempty(axialSatALeftPos)
    w = axialSatALeftPos(6,1);
    h = axialSatALeftPos(7,1);
    satPos = [axialSatALeftPos(5,1)-w/2 axialSatALeftPos(5,2)-h/2 w h axialSatALeftPos(8,1)];
    [handles.SatBand_ALeft_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatALeft(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatALeftPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_ALeft_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_ALeft_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatALeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    set(handles.SatALeftTog,'Value',1);
end

if ~isempty(axialSatLeftPos)
    w = axialSatLeftPos(6,1);
    h = axialSatLeftPos(7,1);
    satPos = [axialSatLeftPos(5,1)-w/2 axialSatLeftPos(5,2)-h/2 w h axialSatLeftPos(8,1)];
    [handles.SatBand_Left_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatLeft(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatLeftPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_Left_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_Left_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatLeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    set(handles.SatLeftTog,'Value',1);
end

if ~isempty(axialSatPLeftPos)
    w = axialSatPLeftPos(6,1);
    h = axialSatPLeftPos(7,1);
    satPos = [axialSatPLeftPos(5,1)-w/2 axialSatPLeftPos(5,2)-h/2 w h axialSatPLeftPos(8,1)];
    [handles.SatBand_PLeft_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatPLeft(pos));
    
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatPLeftPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==centerSlice_saggital);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_PLeft_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==centerSlice_coronal);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_PLeft_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatPLeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    set(handles.SatPLeftTog,'Value',1);
end

% Update handles structure
guidata(gcbo, handles);


% --- Executes on button press in togglebutton_SATLeft.
function togglebutton_SATLeft_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATLeft (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATLeft
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol-80 handles.nrow/2-550 45 1000 0];
    [handles.SatBand_Left_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatLeft(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatLeftPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_Left_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_Left_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatLeft(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Left_Saggital);
    delete(handles.SatBand_Left_Axial);
    delete(handles.SatBand_Left_Coronal);
    handles.axialSatLeftPos = [];
    handles.SatLeftCenter = [];
    handles.SatLeftRot = 0;
end
guidata(hObject, handles);


% --- Executes on button press in togglebutton_SATRight.
function togglebutton_SATRight_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATRight (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATRight

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol/2-80 handles.nrow/2-550 45 1000 0];
    [handles.SatBand_Right_Axial,api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawSaggitalSatRight(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.axialSatRightPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_Right_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.originZ-500, satWidth, 1000 0];
    [handles.SatBand_Right_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', handles.color);
    api.setPosChnCb( @(pos) redrawAxialSatRight(pos));
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Right_Saggital);
    delete(handles.SatBand_Right_Axial);
    delete(handles.SatBand_Right_Coronal);
    handles.axialSatRightPos = [];
    handles.SatRightCenter = [];
    handles.SatRightRot = 0;
end
guidata(hObject, handles);

% --- Executes on button press in togglebutton_SATSuperior.
function togglebutton_SATSuperior_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATSuperior (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATSuperior
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow/2-20 handles.nslice/2-550 45 1000 90];
    [handles.SatBand_Superior_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatSuperior(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatSuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Superior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Superior_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Superior_Saggital);
    delete(handles.SatBand_Superior_Axial);
    delete(handles.SatBand_Superior_Coronal);
    handles.saggitalSatSuperiorPos = [];
end
guidata(hObject, handles);


% --- Executes on button press in togglebutton_SATPosterior2.
function togglebutton_SATPosterior2_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATPosterior2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATPosterior2
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow-80 handles.nslice/2-500 45 1000 0];
    [handles.SatBand_Posterior2_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatPosterior2(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatPosterior2Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior2_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Posterior2_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Posterior2_Saggital);
    delete(handles.SatBand_Posterior2_Axial);
    delete(handles.SatBand_Posterior2_Coronal);
    handles.saggitalSatPosterior2Pos = [];
end
guidata(hObject, handles);

% --- Executes on button press in togglebutton_SATAnterior2.
function togglebutton_SATAnterior2_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATAnterior2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATAnterior2

button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.nrow/2-40 handles.nslice/2-500 45 1000 0];
    [handles.SatBand_Anterior2_Saggital,api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawAxialSatAnterior2(pos));
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.saggitalSatAnterior2Pos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_axial(pts, handles.nslice);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior2_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.coronalSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_saggital(pts,handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satHeight = interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
    [handles.SatBand_Anterior2_Coronal api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_Anterior2_Saggital);
    delete(handles.SatBand_Anterior2_Axial);
    delete(handles.SatBand_Anterior2_Coronal);
    handles.saggitalSatAnterior2Pos = [];
end

guidata(hObject, handles);

% --- Executes on button press in togglebutton_SATLSuperior.
function togglebutton_SATLSuperior_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATLSuperior (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATLSuperior
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol-80 handles.nslice/2-550 45 1000 -45];
    [handles.SatBand_LSuperior_Coronal,api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawCoronalSatLSuperior(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.coronalSatLSuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satHeight= interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
    [handles.SatBand_LSuperior_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_LSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_LSuperior_Saggital);
    delete(handles.SatBand_LSuperior_Axial);
    delete(handles.SatBand_LSuperior_Coronal);
    handles.coronalSatLSuperiorPos = [];
end
guidata(hObject, handles);

% --- Executes on button press in togglebutton_SATRSuperior.
function togglebutton_SATRSuperior_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton_SATRSuperior (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton_SATRSuperior
button_state = get(hObject,'Value');
if button_state == get(hObject,'Max')
    satPos = [handles.ncol/2-80 handles.nslice/2-550 45 1000 45];
    [handles.SatBand_RSuperior_Coronal,api]=imRectRot_Sat('hParent', handles.coronalAx, 'pos', satPos,'ellipse', 0,'rotate',1,'color', handles.color, 'colorc', handles.color );
    api.setPosChnCb( @(pos) redrawCoronalSatRSuperior(pos));
    
    slice = get(handles.saggitalSlider,'Value'); % returns position of slider
    slice = round(slice);
    [pts,angle,pc,w,h] = rectToCorners(satPos);
    handles.coronalSatRSuperiorPos = [pts ; pc; w w; h h; angle angle];

    interPlane = compute_sat_interplane_saggital(pts, handles.ncol);
    idx = find(interPlane(:,1)==slice);
    satHeight= interPlane(idx,3) - interPlane(idx,2);
    satPos = [handles.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
    [handles.SatBand_RSuperior_Saggital api]=imRectRot_Sat('hParent', handles.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
    slice = get(handles.axialSlider,'Value'); % returns position of slider
    slice = round(slice);
    interPlane = compute_sat_interplane_axial(pts, handles.nrow);
    idx = find(interPlane(:,1)==slice);
    satWidth= interPlane(idx,3) - interPlane(idx,2);
    satPos = [interPlane(idx,2) handles.ncol/2-500, satWidth, 1000 0];
    [handles.SatBand_RSuperior_Axial api]=imRectRot_Sat('hParent', handles.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', handles.color);
    api.setSidLock([1 1 1 1]');
    api.setDrgLock(1)
    
elseif button_state == get(hObject,'Min')
    % toggle button is not pressed
    delete(handles.SatBand_RSuperior_Saggital);
    delete(handles.SatBand_RSuperior_Axial);
    delete(handles.SatBand_RSuperior_Coronal);
    handles.coronalSatRSuperiorPos = [];
end
guidata(hObject, handles);

function redrawBox_axial(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialBoxPos = [pts ; pc; w w; h h; angle angle];

if slice<pts(1,1) || slice>pts(2,1)
    if ~data.saggitalBoxDeleted
        delete(data.PressBox_Saggital);
        data.saggitalBoxDeleted = true;
    end
else
    if data.saggitalBoxDeleted
        boxPos = [data.saggitalBoxPos(1,1) data.saggitalBoxPos(1,2) data.saggitalBoxPos(6,1) data.saggitalBoxPos(7,1) data.saggitalBoxPos(8,1)];
        [data.PressBox_Saggital,api]=imRectRot('hParent', data.saggitalAx, 'pos', boxPos,'ellipse', 0,'rotate',1,'color','y','colorc','y'  );
        api.setPosChnCb( @(pos) redrawBox_saggital(pos));
        data.saggitalBoxDeleted = false;
    end
end

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
if ~data.coronalBoxDeleted
    delete(data.PressBox_Coronal);
    data.coronalBoxDeleted = true;
end
data.coronalBoxPos(1,1) = data.axialBoxPos(1,1);
data.coronalBoxPos(6,:) = data.axialBoxPos(6,:);

if slice>=pts(1,2) && slice<=pts(3,2)
        boxPos = [data.axialBoxPos(1,1) data.coronalBoxPos(1,2) data.coronalBoxPos(6,1) data.coronalBoxPos(7,1) data.coronalBoxPos(8,1)];
        [data.PressBox_Coronal,api]=imRectRot('hParent', data.coronalAx, 'pos', boxPos,'ellipse', 0,'rotate',0,'color','y','colorc','y'  );
        api.setPosChnCb( @(pos) redrawBox_coronal(pos));
        api.setSidLock([1 0 1 0]');
        api.setDrgLock(1)
        data.coronalBoxDeleted = false;
end

set(data.showRL, 'String', num2str(data.axialBoxPos(6,1)*data.pixelSize(1), '%5.1f'));
set(data.showAP, 'String', num2str(data.saggitalBoxPos(6,1)*data.pixelSize(2), '%5.1f'));
set(data.showSI, 'String', num2str(data.saggitalBoxPos(7,1)*data.pixelSize(3), '%5.1f'));

% Update handles structure
guidata(gcbo, data);


function redrawBox_coronal(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.coronalBoxPos = [pts ; pc; w w; h h; angle angle];

if slice<pts(1,1) || slice>pts(2,1)
    if ~data.saggitalBoxDeleted
        delete(data.PressBox_Saggital);
        data.saggitalBoxDeleted = true;
    end
else
    if data.saggitalBoxDeleted
        boxPos = [data.saggitalBoxPos(1,1) data.saggitalBoxPos(1,2) data.saggitalBoxPos(6,1) data.saggitalBoxPos(7,1) data.saggitalBoxPos(8,1)];
        [data.PressBox_Saggital,api]=imRectRot('hParent', data.saggitalAx, 'pos', boxPos,'ellipse', 0,'rotate',1,'color','y','colorc','y'  );
        api.setPosChnCb( @(pos) redrawBox_saggital(pos));
        data.saggitalBoxDeleted = false;
    end
end

slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
if ~data.axialBoxDeleted
    delete(data.PressBox_Axial);
    data.axialBoxDeleted = true;
end
data.axialBoxPos(1,1) = data.coronalBoxPos(1,1);
data.axialBoxPos(6,:) = data.coronalBoxPos(6,:);

if slice>=pts(1,2) && slice<=pts(3,2)
        boxPos = [data.coronalBoxPos(1,1) data.axialBoxPos(1,2) data.coronalBoxPos(6,1) data.axialBoxPos(7,1) data.axialBoxPos(8,1)];
        [data.PressBox_Axial,api]=imRectRot('hParent', data.axialAx, 'pos', boxPos,'ellipse', 0,'rotate',0,'color','y','colorc','y'  );
        api.setPosChnCb( @(pos) redrawBox_axial(pos));
        api.setSidLock([1 0 1 0]');
        api.setDrgLock(1)
        data.axialBoxDeleted = false;
end

set(data.showRL, 'String', num2str(data.axialBoxPos(6,1)*data.pixelSize(1), '%5.1f'));
set(data.showAP, 'String', num2str(data.saggitalBoxPos(6,1)*data.pixelSize(2), '%5.1f'));
set(data.showSI, 'String', num2str(data.saggitalBoxPos(7,1)*data.pixelSize(3), '%5.1f'));

% Update handles structure
guidata(gcbo, data);


function redrawBox_saggital(pos)

data = guidata(gcbo);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalBoxPos = [pts ; pc; w w; h h; angle angle];

slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
if ~data.axialBoxDeleted
    delete(data.PressBox_Axial);
    data.axialBoxDeleted = true;
end

interPlane = compute_interplane(pts);
idx = find(interPlane(:,1)==slice);
if ~isempty(idx)
    boxHeight = interPlane(idx,3) - interPlane(idx,2);
    boxPos = [data.axialBoxPos(1,1) interPlane(idx,2)  data.axialBoxPos(6,1) boxHeight data.axialBoxPos(8,1)];
    [data.PressBox_Axial api]=imRectRot('hParent', data.axialAx, 'pos', boxPos , 'ellipse', 0,'rotate',0,'color','y', 'colorc','y');
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc, w, h] = rectToCorners( api.getPos());
    data.axialBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_axial(pos));
    data.axialBoxDeleted = false;
end

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
if ~data.coronalBoxDeleted
    delete(data.PressBox_Coronal);
    data.coronalBoxDeleted = true;
end

interPlane = compute_box_interplane_coronal(data.saggitalBoxPos(1:4,:));
idx = find(interPlane(:,1)==slice);
if ~isempty(idx)
    boxHeight = interPlane(idx,3) - interPlane(idx,2);
    boxPos = [data.coronalBoxPos(1,1) interPlane(idx,2)  data.coronalBoxPos(6,1) boxHeight data.coronalBoxPos(8,1)];
    [data.PressBox_Coronal api]=imRectRot('hParent', data.coronalAx, 'pos', boxPos , 'ellipse', 0,'rotate',0,'color','y', 'colorc','y');
    api.setSidLock([1 0 1 0]');
    api.setDrgLock(1)
    [pts,angle,pc, w, h] = rectToCorners( api.getPos());
    data.coronalBoxPos = [pts ; pc; w w ; h h; angle angle];
    api.setPosChnCb( @(pos) redrawBox_coronal(pos));
    data.coronalBoxDeleted = false;
end

set(data.showRL, 'String', num2str(data.axialBoxPos(6,1)*data.pixelSize(1), '%5.1f'));
set(data.showAP, 'String', num2str(data.saggitalBoxPos(6,1)*data.pixelSize(2), '%5.1f'));
set(data.showSI, 'String', num2str(data.saggitalBoxPos(7,1)*data.pixelSize(3), '%5.1f'));

% Update handles structure
guidata(gcbo, data);


function redrawAxialSatAnterior1(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatAnterior1Pos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Anterior1_Axial);
[data.SatBand_Anterior1_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Anterior1_Coronal);
[data.SatBand_Anterior1_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatAnterior2(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatAnterior2Pos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Anterior2_Axial);
[data.SatBand_Anterior2_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Anterior2_Coronal);
[data.SatBand_Anterior2_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatPosterior1(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatPosterior1Pos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Posterior1_Axial);
[data.SatBand_Posterior1_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)


slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Posterior1_Coronal);
[data.SatBand_Posterior1_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatPosterior2(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatPosterior2Pos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Posterior2_Axial);
[data.SatBand_Posterior2_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)


slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Posterior2_Coronal);
[data.SatBand_Posterior2_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatASuperior(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatASuperiorPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_ASuperior_Axial);
[data.SatBand_ASuperior_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_ASuperior_Coronal);
[data.SatBand_ASuperior_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatPSuperior(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatPSuperiorPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_PSuperior_Axial);
[data.SatBand_PSuperior_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_PSuperior_Coronal);
[data.SatBand_PSuperior_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatInferior(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatInferiorPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Inferior_Axial);
[data.SatBand_Inferior_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Inferior_Coronal);
[data.SatBand_Inferior_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawCoronalSatRSuperior(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.coronalSatRSuperiorPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_RSuperior_Axial);
[data.SatBand_RSuperior_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate', 0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satHeight= interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
delete(data.SatBand_RSuperior_Saggital);
[data.SatBand_RSuperior_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function redrawCoronalSatLSuperior(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.coronalSatLSuperiorPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_LSuperior_Axial);
[data.SatBand_LSuperior_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate', 0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satHeight= interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500, interPlane(idx,2), 1000, satHeight, 0];
delete(data.SatBand_LSuperior_Saggital);
[data.SatBand_LSuperior_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatSuperior(pos)

data = guidata(gcbo);
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.saggitalSatSuperiorPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_axial(pts, data.nslice);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Superior_Axial);
[data.SatBand_Superior_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_saggital(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satHeight = interPlane(idx,3) - interPlane(idx,2);
satPos = [data.ncol/2-500 interPlane(idx,2) 1000 satHeight 0];
delete(data.SatBand_Superior_Coronal);
[data.SatBand_Superior_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function redrawSaggitalSatARight(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialSatARightPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_ARight_Saggital);
[data.SatBand_ARight_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
satPos = [interPlane(idx,2) data.originZ-500, satWidth, 1000 0];
delete(data.SatBand_ARight_Coronal);
[data.SatBand_ARight_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', data.color);
api.setPosChnCb( @(pos) redrawAxialSatARight(pos));
api.setSidLock([1 0 1 0]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawSaggitalSatRight(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialSatRightPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_Right_Saggital);
[data.SatBand_Right_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.originZ-500, satWidth, 1000 0];
delete(data.SatBand_Right_Coronal);
[data.SatBand_Right_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', data.color);
api.setPosChnCb( @(pos) redrawAxialSatRight(pos));
api.setSidLock([1 0 1 0]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function redrawSaggitalSatPRight(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialSatPRightPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_PRight_Saggital);
[data.SatBand_PRight_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.originZ-500, satWidth, 1000 0];
delete(data.SatBand_PRight_Coronal);
[data.SatBand_PRight_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', data.color);
api.setPosChnCb( @(pos) redrawAxialSatPRight(pos));
api.setSidLock([1 0 1 0]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawSaggitalSatALeft(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialSatALeftPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_ALeft_Saggital);
[data.SatBand_ALeft_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.originZ-500, satWidth, 1000 0];
delete(data.SatBand_ALeft_Coronal);
[data.SatBand_ALeft_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', data.color);
api.setPosChnCb( @(pos) redrawAxialSatALeft(pos));
api.setSidLock([1 0 1 0]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawSaggitalSatLeft(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialSatLeftPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_Left_Saggital);
[data.SatBand_Left_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.originZ-500, satWidth, 1000 0];
delete(data.SatBand_Left_Coronal);
[data.SatBand_Left_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', data.color);
api.setPosChnCb( @(pos) redrawAxialSatLeft(pos));
api.setSidLock([1 0 1 0]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawSaggitalSatPLeft(pos)

data = guidata(gcbo);
slice = get(data.saggitalSlider,'Value'); % returns position of slider
slice = round(slice);
[pts,angle,pc,w,h] = rectToCorners(pos);
data.axialSatPLeftPos = [pts ; pc; w w; h h; angle angle];

interPlane = compute_sat_interplane_saggital(pts, data.ncol);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.ncol/2-500, satWidth, 1000 0];
delete(data.SatBand_PLeft_Saggital);
[data.SatBand_PLeft_Saggital api]=imRectRot_Sat('hParent', data.saggitalAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color',data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

slice = get(data.coronalSlider,'Value'); % returns position of slider
slice = round(slice);
interPlane = compute_sat_interplane_axial(pts, data.nrow);
idx = find(interPlane(:,1)==slice);
satWidth= interPlane(idx,3) - interPlane(idx,2);
satPos = [interPlane(idx,2) data.originZ-500, satWidth, 1000 0];
delete(data.SatBand_PLeft_Coronal);
[data.SatBand_PLeft_Coronal api]=imRectRot_Sat('hParent', data.coronalAx, 'pos', satPos , 'ellipse', 0,'rotate',1,'color', data.color);
api.setPosChnCb( @(pos) redrawAxialSatPLeft(pos));
api.setSidLock([1 0 1 0]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function redrawAxialSatARight(pos)

data = guidata(gcbo);
data.SatARightRot = pos(5);
BETA = pos(5);
Ry = [cosd(BETA)    0   sind(BETA)
         0          1      0
      -sind(BETA)   0   cosd(BETA)];

pts = [data.axialSatARightPos(5,1)-data.originX data.originY-data.axialSatARightPos(5,2)]';
pts = repmat(pts,[1,data.nslice+100]);
pts = [pts ; (data.originZ-data.nslice)-49:data.originZ+50];
pts = Ry*pts;
pts(3,:) = round(pts(3,:));
data.SatARightCenter = pts;
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = data.originZ-round(slice);
idx = find(pts(3,:)==slice);
delete(data.SatBand_ARight_Axial);
w = data.axialSatARightPos(6,1);
h = data.axialSatARightPos(7,1);
satPos = [pts(1,idx(1))+data.originX-w/2 data.originY-pts(2,idx(1))-h/2 w h data.axialSatARightPos(8,1)];
[data.SatBand_ARight_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function redrawAxialSatPRight(pos)

data = guidata(gcbo);
data.SatPRightRot = pos(5);
BETA = pos(5);
Ry = [cosd(BETA)    0   sind(BETA)
         0          1      0
      -sind(BETA)   0   cosd(BETA)];

pts = [data.axialSatPRightPos(5,1)-data.originX data.originY-data.axialSatPRightPos(5,2)]';
pts = repmat(pts,[1,data.nslice+100]);
pts = [pts ; (data.originZ-data.nslice)-49:data.originZ+50];
pts = Ry*pts;
pts(3,:) = round(pts(3,:));
data.SatPRightCenter = pts;
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = data.originZ-round(slice);
idx = find(pts(3,:)==slice);
delete(data.SatBand_PRight_Axial);
w = data.axialSatPRightPos(6,1);
h = data.axialSatPRightPos(7,1);
satPos = [pts(1,idx(1))+data.originX-w/2 data.originY-pts(2,idx(1))-h/2 w h data.axialSatPRightPos(8,1)];
[data.SatBand_PRight_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function redrawAxialSatALeft(pos)

data = guidata(gcbo);
data.SatALeftRot = pos(5);
BETA = pos(5);
Ry = [cosd(BETA)    0   sind(BETA)
         0          1      0
      -sind(BETA)   0   cosd(BETA)];

pts = [data.axialSatALeftPos(5,1)-data.originX data.originY-data.axialSatALeftPos(5,2)]';
pts = repmat(pts,[1,data.nslice+100]);
pts = [pts ; (data.originZ-data.nslice)-49:data.originZ+50];
pts = Ry*pts;
pts(3,:) = round(pts(3,:));
data.SatALeftCenter = pts;
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = data.originZ-round(slice);
idx = find(pts(3,:)==slice);
delete(data.SatBand_ALeft_Axial);
w = data.axialSatALeftPos(6,1);
h = data.axialSatALeftPos(7,1);
satPos = [pts(1,idx(1))+data.originX-w/2 data.originY-pts(2,idx(1))-h/2 w h data.axialSatALeftPos(8,1)];
[data.SatBand_ALeft_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatLeft(pos)

data = guidata(gcbo);
data.SatLeftRot = pos(5);
BETA = pos(5);
Ry = [cosd(BETA)    0   sind(BETA)
         0          1      0
      -sind(BETA)   0   cosd(BETA)];

pts = [data.axialSatLeftPos(5,1)-data.originX data.originY-data.axialSatLeftPos(5,2)]';
pts = repmat(pts,[1,data.nslice+100]);
pts = [pts ; (data.originZ-data.nslice)-49:data.originZ+50];
pts = Ry*pts;
pts(3,:) = round(pts(3,:));
data.SatLeftCenter = pts;
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = data.originZ-round(slice);
idx = find(pts(3,:)==slice);
delete(data.SatBand_Left_Axial);
w = data.axialSatLeftPos(6,1);
h = data.axialSatLeftPos(7,1);
satPos = [pts(1,idx(1))+data.originX-w/2 data.originY-pts(2,idx(1))-h/2 w h data.axialSatLeftPos(8,1)];
[data.SatBand_Left_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatRight(pos)

data = guidata(gcbo);
data.SatRightRot = pos(5);
BETA = pos(5);
Ry = [cosd(BETA)    0   sind(BETA)
         0          1      0
      -sind(BETA)   0   cosd(BETA)];

pts = [data.axialSatRightPos(5,1)-data.originX data.originY-data.axialSatRightPos(5,2)]';
pts = repmat(pts,[1,data.nslice+100]);
pts = [pts ; (data.originZ-data.nslice)-49:data.originZ+50];
pts = Ry*pts;
pts(3,:) = round(pts(3,:));
data.SatRightCenter = pts;
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = data.originZ-round(slice);
idx = find(pts(3,:)==slice);
delete(data.SatBand_Right_Axial);
w = data.axialSatRightPos(6,1);
h = data.axialSatRightPos(7,1);
satPos = [pts(1,idx(1))+data.originX-w/2 data.originY-pts(2,idx(1))-h/2 w h data.axialSatRightPos(8,1)];
[data.SatBand_Right_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);

function redrawAxialSatPLeft(pos)

data = guidata(gcbo);
data.SatPLeftRot = pos(5);
BETA = pos(5);
Ry = [cosd(BETA)    0   sind(BETA)
         0          1      0
      -sind(BETA)   0   cosd(BETA)];

pts = [data.axialSatPLeftPos(5,1)-data.originX data.originY-data.axialSatPLeftPos(5,2)]';
pts = repmat(pts,[1,data.nslice+100]);
pts = [pts ; (data.originZ-data.nslice)-49:data.originZ+50];
pts = Ry*pts;
pts(3,:) = round(pts(3,:));
data.SatPLeftCenter = pts;
slice = get(data.axialSlider,'Value'); % returns position of slider
slice = data.originZ-round(slice);
idx = find(pts(3,:)==slice);
delete(data.SatBand_PLeft_Axial);
w = data.axialSatPLeftPos(6,1);
h = data.axialSatPLeftPos(7,1);
satPos = [pts(1,idx(1))+data.originX-w/2 data.originY-pts(2,idx(1))-h/2 w h data.axialSatPLeftPos(8,1)];
[data.SatBand_PLeft_Axial api]=imRectRot_Sat('hParent', data.axialAx, 'pos', satPos , 'ellipse', 0,'rotate',0,'color', data.color);
api.setSidLock([1 1 1 1]');
api.setDrgLock(1)

% Update handles structure
guidata(gcbo, data);


function [pc,rs,R] = rectInfo( pos0 )
% return rectangle center, radii, and rotation matrix
t=pos0(5); c=cosd(t); s=sind(t); R=[c -s; s c];
rs=pos0(3:4)/2; pc=pos0(1:2)+rs;
        
        
function [pts,angle, pc, w, h, xs,ys] = rectToCorners( pos0 )
% return 4 rect corners in real world coords
[pc,rs,R]=rectInfo( pos0 );
x0=-rs(1); x1=rs(1); y0=-rs(2); y1=rs(2);
pts=[x0 y0; x1 y0; x1 y1; x0 y1]*R'+pc(ones(4,1),:);
xs=pts(:,1); ys=pts(:,2);
angle=pos0(5);
w = pos0(3);
h = pos0(4);

function [normal center distance] = get_axialSat_loc(pts, qform, nrow)

if norm([pts(3,1)-pts(2,1) pts(3,2)-pts(2,2)]) > norm([pts(2,1)-pts(1,1) pts(2,2)-pts(1,2)])
     x1 = pts(2,1); x2 = pts(3,1);
     y1 = pts(2,2); y2 = pts(3,2);
 else
     x1 = pts(1,1); x2 = pts(2,1);
     y1 = pts(1,2); y2 = pts(2,2);
end

Coord_RAS =  qform*[x1-1, nrow-y1, 0 , 1]';
x1 = -Coord_RAS(1);
y1 = -Coord_RAS(2);

Coord_RAS =  qform*[x2-1, nrow-y2, 0 , 1]';
x2 = -Coord_RAS(1);
y2 = -Coord_RAS(2);

centerX = pts(5,1);
centerY = pts(5,2);
Coord_RAS =  qform*[centerX-1, nrow-centerY, 0 , 1]';
centerX = -Coord_RAS(1);
centerY = -Coord_RAS(2);
center = [centerX centerY];
[normal distance] = get_normal_distance(x1, x2, y1, y2, centerX, centerY);

function [normal distance] = get_saggitalSat_loc(pts, qform, nrow, nslice)
% obtain the longer side of the rectangular
if norm([pts(3,1)-pts(2,1) pts(3,2)-pts(2,2)]) > norm([pts(2,1)-pts(1,1) pts(2,2)-pts(1,2)])
     x1 = pts(2,1); x2 = pts(3,1);
     y1 = pts(2,2); y2 = pts(3,2);
 else
     x1 = pts(1,1); x2 = pts(2,1);
     y1 = pts(1,2); y2 = pts(2,2);
end

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

centerX = pts(5,1);
centerY = pts(5,2);
Coord_RAS =  qform*[0, nrow-centerX, nslice-centerY, 1]';
centerX = -Coord_RAS(2)
centerY = Coord_RAS(3)
[normal distance] = get_normal_distance(x1, x2, y1, y2, centerX, centerY);


function [dx, dy] = getRectPositionInXY(rotAngle, deltaX, deltaY)

dx = (deltaX - deltaY*sin(rotAngle))*sin(rotAngle);
dy = (deltaX - deltaY*sin(rotAngle))*cos(rotAngle);

function [x2,y2] = RASConvert(in, qform, nrow, nslice)
Coord_RAS =  inv(qform)*[0, -in(1), in(2), 1]';
x2 = nrow-Coord_RAS(2);
y2 = nslice-Coord_RAS(3);

function [x2,y2] = RASConvertB(in, qform, nrow, nslice)
Coord_RAS =  inv(qform)*[0, -in(1), in(2), 1]';
x2 = nrow-Coord_RAS(2);
y2 = nslice-Coord_RAS(3);

function [x2,y2] = RASConvertS(in, qform, nrow, nslice)
Coord_RAS =  inv(qform)*[-in(1), -in(2), 0 , 1]';
x2 = 1+Coord_RAS(1);
y2 = nrow-Coord_RAS(2);

function [x2,y2] = RASConvertC(in, qform, nrow, nslice)
Coord_RAS =  (qform)*[0, -in(1), in(2), 1]';
y2 = nslice-Coord_RAS(3);
Coord_RAS =  (qform)*[-in(1), -in(2), 0 , 1]';
x2 = 1+Coord_RAS(1);

function [x2,y2] = RASConvertCoronal(in, qform, nrow, nslice)
Coord_RAS =  qform*[-in(1), 0, in(2), 1]';
x2 = 1+Coord_RAS(1);
y2 = nslice-Coord_RAS(3);



function pts = get_saggitalSatPosXML(normal, distance, thicknessY, qform, nrow, nslice)
% sag plane is defined in the zy direction
%
Y = 1;
Z = 2;
otherThickness = 1000; % per definition

nd = normal*distance;

% define the box
centerY = nd(2);
centerZ = nd(3);

temp = zeros(8,2);

temp(5,Y) = centerY;
temp(5,Z) = centerZ;

temp(2,Y) = centerY - thicknessY / 2;
temp(1,Y) = centerY + thicknessY / 2;
temp(4,Y) = centerY + thicknessY / 2;
temp(3,Y) = centerY - thicknessY / 2;
temp(2,Z) = centerZ + otherThickness / 2;
temp(1,Z) = centerZ + otherThickness / 2;
temp(4,Z) = centerZ - otherThickness / 2;
temp(3,Z) = centerZ - otherThickness / 2;

% temp(:,Y) = temp(:,Y)+ distance;


% rotate the box
rotAng = pi-atan2(normal(3), normal(2));
RMAT = [cos(rotAng) -sin(rotAng);...
        sin(rotAng)  cos(rotAng)];

pts = temp*RMAT

pts(5,:) = temp(5,:);
tpts = pts;
% adapt to RAS
[pts(1,Y) pts(1,Z)] = RASConvert(tpts(1,:), qform, nrow, nslice)
[pts(2,Y) pts(2,Z)] = RASConvert(tpts(2,:), qform, nrow, nslice)
[pts(3,Y) pts(3,Z)] = RASConvert(tpts(3,:), qform, nrow, nslice)
[pts(4,Y) pts(4,Z)] = RASConvert(tpts(4,:), qform, nrow, nslice)
[pts(5,Y) pts(5,Z)] = RASConvert(tpts(5,:), qform, nrow, nslice)

% assign to pts
pts(6,1:2) = thicknessY;
pts(7,1:2) = otherThickness;
pts(8,1:2) = 180/pi*rotAng;

function pts = get_axialSatPosXML(normal, distance, thicknessY, qform, nrow, nslice, factor)

% sag plane is defined in the zy direction
%
Y = 1;
Z = 2;
otherThickness = 1000; % per definition

nd = normal*distance;

% define the box
centerY = nd(1);
centerZ = nd(2);

temp = zeros(8,2);

temp(5,Y) = centerY;
temp(5,Z) = centerZ;

temp(2,Y) = centerY - thicknessY / 2;
temp(1,Y) = centerY + thicknessY / 2;
temp(4,Y) = centerY + thicknessY / 2;
temp(3,Y) = centerY - thicknessY / 2;
temp(2,Z) = centerZ + otherThickness / 2;
temp(1,Z) = centerZ + otherThickness / 2;
temp(4,Z) = centerZ - otherThickness / 2;
temp(3,Z) = centerZ - otherThickness / 2;

% temp(:,Y) = temp(:,Y)+ distance;


% rotate the box
if (factor~=0)
    rotAng = factor-atan2(normal(1), normal(2));
else
    rotAng = atan2(normal(1), normal(2));
end
RMAT = [cos(rotAng) -sin(rotAng);...
        sin(rotAng)  cos(rotAng)];

pts = temp*RMAT

pts(5,:) = temp(5,:);
tpts = pts;
% adapt to RAS
[pts(1,Y) pts(1,Z)] = RASConvertS(tpts(1,:), qform, nrow, nslice)
[pts(2,Y) pts(2,Z)] = RASConvertS(tpts(2,:), qform, nrow, nslice)
[pts(3,Y) pts(3,Z)] = RASConvertS(tpts(3,:), qform, nrow, nslice)
[pts(4,Y) pts(4,Z)] = RASConvertS(tpts(4,:), qform, nrow, nslice)
[pts(5,Y) pts(5,Z)] = RASConvertS(tpts(5,:), qform, nrow, nslice)

% assign to pts
pts(6,1:2) = thicknessY;
pts(7,1:2) = otherThickness;
pts(8,1:2) = 180/pi*rotAng;




function pts = get_coronalSatPosXML(normal, distance, thicknessY, qform, nrow, nslice)
% sag plane is defined in the zy direction
%
Y = 1;
Z = 2;
otherThickness = 1000; % per definition

nd = normal*distance;

% define the box
centerY = nd(1);
centerZ = nd(3);

temp = zeros(8,2);

temp(5,Y) = centerY;
temp(5,Z) = centerZ;

temp(2,Y) = centerY - thicknessY / 2;
temp(1,Y) = centerY + thicknessY / 2;
temp(4,Y) = centerY + thicknessY / 2;
temp(3,Y) = centerY - thicknessY / 2;
temp(2,Z) = centerZ + otherThickness / 2;
temp(1,Z) = centerZ + otherThickness / 2;
temp(4,Z) = centerZ - otherThickness / 2;
temp(3,Z) = centerZ - otherThickness / 2;

% temp(:,Y) = temp(:,Y)+ distance;


% rotate the box
rotAng = pi-atan2(normal(3), normal(1));
RMAT = [cos(rotAng) -sin(rotAng);...
        sin(rotAng)  cos(rotAng)];

pts = temp*RMAT

pts(5,:) = temp(5,:);
tpts = pts;
% adapt to RAS
[pts(1,Y) pts(1,Z)] = RASConvertCoronal(tpts(1,:), qform, nrow, nslice)
[pts(2,Y) pts(2,Z)] = RASConvertCoronal(tpts(2,:), qform, nrow, nslice)
[pts(3,Y) pts(3,Z)] = RASConvertCoronal(tpts(3,:), qform, nrow, nslice)
[pts(4,Y) pts(4,Z)] = RASConvertCoronal(tpts(4,:), qform, nrow, nslice)
[pts(5,Y) pts(5,Z)] = RASConvertCoronal(tpts(5,:), qform, nrow, nslice)

% assign to pts
pts(6,1:2) = thicknessY;
pts(7,1:2) = otherThickness;
pts(8,1:2) = 180/pi*rotAng;


function [normal distance] = get_coronalSat_loc(pts, qform, nslice)

if norm([pts(3,1)-pts(2,1) pts(3,2)-pts(2,2)]) > norm([pts(2,1)-pts(1,1) pts(2,2)-pts(1,2)])
     x1 = pts(2,1); x2 = pts(3,1);
     y1 = pts(2,2); y2 = pts(3,2);
 else
     x1 = pts(1,1); x2 = pts(2,1);
     y1 = pts(1,2); y2 = pts(2,2);
end

Coord_RAS =  qform*[x1-1, 0, nslice-y1, 1]';
x1 = -Coord_RAS(1);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[x2-1, 0, nslice-y2, 1]';
x2 = -Coord_RAS(1);
y2 = Coord_RAS(3);

centerX = pts(5,1);
centerY = pts(5,2);
Coord_RAS =  qform*[centerX-1, 0, nslice-centerY, 1]';
centerX = -Coord_RAS(1);
centerY = Coord_RAS(3);
[normal distance] = get_normal_distance(x1, x2, y1, y2, centerX, centerY);

function [normal distance] = get_normal_distance(x1, x2, y1, y2, x, y)

slope = (y2-y1)/(x2-x1);
if slope == inf || slope == -inf
    normal = [1*sign(x) 0];
    distance = abs(x);
else
    distance = (slope*x-y)/norm([-slope 1]);
    normal = [-slope 1]./norm([-slope 1]);
    normal = -normal*sign(distance);
    distance = abs(distance);
end

function [normal distance signedDistance] = get_normal_distancesigned(x1, x2, y1, y2, x, y)

slope = (y2-y1)/(x2-x1);
if slope == inf || slope == -inf
    normal = [1*sign(x) 0];
    signedDistance =x;
    distance = abs(x);
    
else
    distance = (slope*x-y)/norm([-slope 1]);
    normal = [-slope 1]./norm([-slope 1]);
    normal = -normal*sign(distance);
    signedDistance = distance;
    distance = abs(distance);

end


function boxLoc = get_Box_loc(pts_saggital, pts_axial, qform, nrow, nslice)

boxLoc = zeros(6,4);
[~, idx] = sort(pts_saggital(1:4,1));

x1 = pts_saggital(idx(1),1); x2 = pts_saggital(idx(2),1);
y1 = pts_saggital(idx(1),2); y2 = pts_saggital(idx(2),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normal distance] = get_normal_distance(x1, x2, y1, y2, x1, y1);
normal = [0 normal];
boxLoc(1,:) = [normal distance];

x1 = pts_saggital(idx(3),1); x2 = pts_saggital(idx(4),1);
y1 = pts_saggital(idx(3),2); y2 = pts_saggital(idx(4),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normal distance] = get_normal_distance(x1, x2, y1, y2, x1, y1);
normal = [0 normal];
boxLoc(2,:) = [normal distance];


[~, idx] = sort(pts_saggital(1:4,2));
x1 = pts_saggital(idx(1),1); x2 = pts_saggital(idx(2),1);
y1 = pts_saggital(idx(1),2); y2 = pts_saggital(idx(2),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normal distance] = get_normal_distance(x1, x2, y1, y2, x1, y1);
normal = [0 normal];
boxLoc(3,:) = [normal distance];

x1 = pts_saggital(idx(3),1); x2 = pts_saggital(idx(4),1);
y1 = pts_saggital(idx(3),2); y2 = pts_saggital(idx(4),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normal distance] = get_normal_distance(x1, x2, y1, y2, x1, y1);
normal = [0 normal];
boxLoc(4,:) = [normal distance];

[~, idx] = sort(pts_axial(1:4,1));
x1 = pts_axial(idx(3),1); x2 = pts_axial(idx(4),1);
y1 = pts_axial(idx(3),2); y2 = pts_axial(idx(4),2);

Coord_RAS =  qform*[x1-1, nrow-y1, 0 , 1]';
x1 = -Coord_RAS(1);
y1 = -Coord_RAS(2);

Coord_RAS =  qform*[x2-1, nrow-y2, 0 , 1]';
x2 = -Coord_RAS(1);
y2 = -Coord_RAS(2);

[normal distance] = get_normal_distance(x1, x2, y1, y2, x1, y1);
normal = [normal 0];
boxLoc(5,:) = [normal distance];


x1 = pts_axial(idx(1),1); x2 = pts_axial(idx(2),1);
y1 = pts_axial(idx(1),2); y2 = pts_axial(idx(2),2);

Coord_RAS =  qform*[x1-1, nrow-y1, 0 , 1]';
x1 = -Coord_RAS(1);
y1 = -Coord_RAS(2);

Coord_RAS =  qform*[x2-1, nrow-y2, 0 , 1]';
x2 = -Coord_RAS(1);
y2 = -Coord_RAS(2);

[normal distance] = get_normal_distance(x1, x2, y1, y2, x1, y1);
normal = [normal 0];
boxLoc(6,:) = [normal distance];

% Define a box around the coordinate origin with 3 thicknesses
function boxParam = defineABox( thS, thC, thA) % thickness of Saggital, Coronal, Axial
globalDefs;
boxParam = zeros(8,3);

boxParam(TLF,C) = -thC/2;
boxParam(TLF,A) = thA/2;
boxParam(TLF,S) = thS/2;

boxParam(TRF,C) = thC/2;
boxParam(TRF,A) = thA/2;
boxParam(TRF,S) = thS/2;

boxParam(BLF,C) = -thC/2;
boxParam(BLF,A) = -thA/2;
boxParam(BLF,S) = thS/2;

boxParam(BRF,C) = thC/2;
boxParam(BRF,A) = -thA/2;
boxParam(BRF,S) = thS/2;

boxParam(TLB,C) = -thC/2;
boxParam(TLB,A) = thA/2;
boxParam(TLB,S) = -thS/2;

boxParam(TRB,C) = thC/2;
boxParam(TRB,A) = thA/2;
boxParam(TRB,S) = -thS/2;

boxParam(BLB,C) = -thC/2;
boxParam(BLB,A) = -thA/2;
boxParam(BLB,S) = -thS/2;

boxParam(BRB,C) = thC/2;
boxParam(BRB,A) = -thA/2;
boxParam(BRB,S) = -thS/2;

% define a box and rotate it
function [saggitalBoxPos, axialBoxPos, coronalBoxPos] = ...
                getBoxPositionsFromXML(PRESSBoxStruct, qform, nrow, nslice)

% The distances to the origin are absolute values and the direction is
% saved in the normal

globalDefs;           

thx = str2num(PRESSBoxStruct(S).thickness); % thickness of saggital defines limits of x
thy = str2num(PRESSBoxStruct(C).thickness); % thickness of coronal defines limits of y
thz = str2num(PRESSBoxStruct(A).thickness); % thickness of axial defines limits of z

dx = str2num(PRESSBoxStruct(S).distance_from_origin);
dy = str2num(PRESSBoxStruct(C).distance_from_origin); 
dz = str2num(PRESSBoxStruct(A).distance_from_origin); 

% define a box
boxPara1 = defineABox(thx, thy, thz);

% our xyz order is YZX
nrmC = [str2num(PRESSBoxStruct(C).normal_y), str2num(PRESSBoxStruct(C).normal_z), str2num(PRESSBoxStruct(C).normal_x)];
nrmS = [str2num(PRESSBoxStruct(S).normal_y), str2num(PRESSBoxStruct(S).normal_z), str2num(PRESSBoxStruct(S).normal_x)];

nrmA = [str2num(PRESSBoxStruct(A).normal_y), str2num(PRESSBoxStruct(A).normal_z), str2num(PRESSBoxStruct(A).normal_x)];


boxCenterC = (dy)*nrmC;
boxCenterS = (dx)*nrmS;
boxCenterA = (dz)*nrmA;

boxCenter = boxCenterC+boxCenterS+boxCenterA;
boxPara(:,S) = boxPara1(:,S) + boxCenter(S);
boxPara(:,C) = boxPara1(:,C) + boxCenter(C);
boxPara(:,A) = boxPara1(:,A) + boxCenter(A);


normMatrix = [circshift(nrmS,1,2); circshift(nrmC,1,2); circshift(nrmA,1,2)];
[a1, a2, a3] = obtainEulerAnglesFromNormals(normMatrix)


ps = 0;%a2;
th = 0;%a3;

if a1<-pi/2
    a1 = (a1 + pi);
elseif a1>pi/2
    a1 = (a1 - pi);
end

phi = sign(nrmA(1))*sign(nrmC(2))*a1;

RX = [1 0 0; ... our X
      0 cos(ps) -sin(ps); ...
      0 sin(ps)  cos(ps)];
  

RY = [cos(th) 0 sin(th); ... our Y
      0 1 0; ...
      -sin(th) 0 cos(th)];  


RZ = [cos(phi) -sin(phi) 0; ... our Z
      sin(phi)  cos(phi) 0; ...
      0 0 1];

RR = RX*RY*RZ;  
  
% rotate the box
rotBoxParam = boxPara*RR;
rotBoxCenter = boxCenter;

% extract the saggital and axial rectangulars
angleA = 180/pi*phi; % from coronal
angleC = 180/pi*th; % from axial normally 0
angleS = 180/pi*ps; %from sag normally 0

% TODOSM update Center Points
saggitalBoxPos = [rotBoxParam([TLF,BLF, BRF, TRF],[C,A]); rotBoxCenter(C) rotBoxCenter(A); thy thy; thz thz;  angleA angleA];
axialBoxPos = [rotBoxParam([TLB,TLF, TRF, TRB],[S,C]); rotBoxCenter(S) rotBoxCenter(C) ; thx thx; thy thy; angleS angleS];

coronalBoxPos = [rotBoxParam([BRB, BRF, TRF,TRB],[S,A]); rotBoxCenter(S) rotBoxCenter(A) ; thx thx; thz thz; angleC angleC];


ss = saggitalBoxPos;
% % transform from RAS

[ss(1,1), ss(1,2)] = RASConvertB(saggitalBoxPos(1,:), qform, nrow, nslice);
[ss(2,1), ss(2,2)] = RASConvertB(saggitalBoxPos(2,:), qform, nrow, nslice);
[ss(3,1), ss(3,2)] = RASConvertB(saggitalBoxPos(3,:), qform, nrow, nslice);
[ss(4,1), ss(4,2)] = RASConvertB(saggitalBoxPos(4,:), qform, nrow, nslice);
[ss(5,1), ss(5,2)] = RASConvertB(saggitalBoxPos(5,:), qform, nrow, nslice);
saggitalBoxPos = ss;

aa = axialBoxPos;
[aa(1,1), aa(1,2)] = RASConvertS(axialBoxPos(1,:), qform, nrow, nslice);
[aa(2,1), aa(2,2)] = RASConvertS(axialBoxPos(2,:), qform, nrow, nslice);
[aa(3,1), aa(3,2)] = RASConvertS(axialBoxPos(3,:), qform, nrow, nslice);
[aa(4,1), aa(4,2)] = RASConvertS(axialBoxPos(4,:), qform, nrow, nslice);
[aa(5,1), aa(5,2)] = RASConvertS(axialBoxPos(5,:), qform, nrow, nslice);
axialBoxPos = aa;

cc = coronalBoxPos;
[cc(1,1), cc(1,2)] = RASConvertC(coronalBoxPos(1,:), qform, nrow, nslice);
[cc(2,1), cc(2,2)] = RASConvertC(coronalBoxPos(2,:), qform, nrow, nslice);
[cc(3,1), cc(3,2)] = RASConvertC(coronalBoxPos(3,:), qform, nrow, nslice);
[cc(4,1), cc(4,2)] = RASConvertC(coronalBoxPos(4,:), qform, nrow, nslice);
[cc(5,1), cc(5,2)] = RASConvertC(coronalBoxPos(5,:), qform, nrow, nslice);
coronalBoxPos = cc;


function [boxPara, boxStruct] = getBoxAsThreePlanes(pts_saggital, pts_axial, qform, nrow, nslice)
globalDefs;
boxPara = zeros(3,6); % normalx,y,z, distance (face closer to origin), thickness
boxStruct = XMLReader.createEmptyDataStruct(3);

%% CORONAL
[~, idx] = sort(pts_saggital(1:4,1));

% anterior plane
x1 = pts_saggital(idx(1),1); x2 = pts_saggital(idx(2),1);
y1 = pts_saggital(idx(1),2); y2 = pts_saggital(idx(2),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normal distance1, d1] = get_normal_distancesigned(x1, x2, y1, y2, x1, y1);
normal = [0 normal];



% Posterior plane
x1 = pts_saggital(idx(3),1); x2 = pts_saggital(idx(4),1);
y1 = pts_saggital(idx(3),2); y2 = pts_saggital(idx(4),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

% distance 2 with respect to Z
[normalTemp distance2, d2] = get_normal_distancesigned(x1, x2, y1, y2, x1, y1);
normalTemp = [0 normalTemp];

dirC = [0 1 0]; % P is positive

% if ((dot(dirC, normal)) >= (dot(dirC, normalTemp)))
if (abs(d1) >= abs(d2))  
    boxPara(C,1:3) = normal; % normalX, normalY, normalZ
else
    boxPara(C,1:3) = normalTemp; % normalX, normalY, normalZ
end

boxPara(C,4  ) = abs(d1 + d2)/2;

if (d2*d1<0)
    thickness = abs(d2)+abs(d1); 
else
    thickness = max(distance1, distance2) - min(distance1, distance2);
end
boxPara(C,5) = thickness;

k=2; % to be conform with PSD
boxStruct(k).id = num2str(k);
boxStruct(k).label = 'coronal';
boxStruct(k).normal_x = num2str(boxPara(C,1));
boxStruct(k).normal_y = num2str(boxPara(C,2));
boxStruct(k).normal_z = num2str(boxPara(C,3));
boxStruct(k).thickness = num2str(thickness);
boxStruct(k).distance_from_origin = num2str(boxPara(C,4  )); % with respect to Z


%% AXIAL
[~, idx] = sort(pts_saggital(1:4,2));

% inf
x1 = pts_saggital(idx(1),1); x2 = pts_saggital(idx(2),1);
y1 = pts_saggital(idx(1),2); y2 = pts_saggital(idx(2),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normal distance1, d1] = get_normal_distancesigned(x1, x2, y1, y2, x1, y1);
normal = [0 normal];



% post
x1 = pts_saggital(idx(3),1); x2 = pts_saggital(idx(4),1);
y1 = pts_saggital(idx(3),2); y2 = pts_saggital(idx(4),2);

Coord_RAS =  qform*[0, nrow-x1, nslice-y1, 1]';
x1 = -Coord_RAS(2);
y1 = Coord_RAS(3);

Coord_RAS =  qform*[0, nrow-x2, nslice-y2, 1]';
x2 = -Coord_RAS(2);
y2 = Coord_RAS(3);

[normalTemp distance2, d2] = get_normal_distancesigned(x1, x2, y1, y2, x1, y1);
normalTemp = [0 normalTemp];

dirA = [0 0 1]; % S is positive

% if ((dot(dirA, normal)) >= (dot(dirA, normalTemp)))
if (abs(d1) >= abs(d2))      
    boxPara(A,1:3) = normal; % normalX, normalY, normalZ
else
    boxPara(A,1:3) = normalTemp; % normalX, normalY, normalZ
end
% with respect to Y
boxPara(A,4  ) = abs(d1 + d2)/2;



if (d2*d1<0)
    thickness = abs(d2)+abs(d1); 
else
    thickness = max(distance1, distance2) - min(distance1, distance2);
end
boxPara(A,5) = thickness;

k=3;
boxStruct(k).id = num2str(k);
boxStruct(k).label = 'axial';
boxStruct(k).normal_x = num2str(boxPara(A,1));
boxStruct(k).normal_y = num2str(boxPara(A,2));
boxStruct(k).normal_z = num2str(boxPara(A,3));
boxStruct(k).thickness = num2str(thickness);
boxStruct(k).distance_from_origin = num2str(boxPara(A,4  )); % with respect to Y

%% SAGGITAL
[~, idx] = sort(pts_axial(1:4,1));

% r
x1 = pts_axial(idx(3),1); x2 = pts_axial(idx(4),1);
y1 = pts_axial(idx(3),2); y2 = pts_axial(idx(4),2);

Coord_RAS =  qform*[x1-1, nrow-y1, 0 , 1]';
x1 = -Coord_RAS(1);
y1 = -Coord_RAS(2);

Coord_RAS =  qform*[x2-1, nrow-y2, 0 , 1]';
x2 = -Coord_RAS(1);
y2 = -Coord_RAS(2);

[normal distance1, d1] = get_normal_distancesigned(x1, x2, y1, y2, x1, y1);
normal = [normal 0];

%boxPara(S,1:3) = normal; % normalX, normalY, normalZ

% l
x1 = pts_axial(idx(1),1); x2 = pts_axial(idx(2),1);
y1 = pts_axial(idx(1),2); y2 = pts_axial(idx(2),2);

Coord_RAS =  qform*[x1-1, nrow-y1, 0 , 1]';
x1 = -Coord_RAS(1);
y1 = -Coord_RAS(2);

Coord_RAS =  qform*[x2-1, nrow-y2, 0 , 1]';
x2 = -Coord_RAS(1);
y2 = -Coord_RAS(2);

[normalTemp distance2, d2] = get_normal_distancesigned(x1, x2, y1, y2, x1, y1);
normalTemp = [normalTemp 0];

dirS = [1 0 0]; % L is positive

% if ((dot(dirS, normal)) >= (dot(dirS, normalTemp)))
if (abs(d1) >= abs(d2))      
    boxPara(S,1:3) = normal; % normalX, normalY, normalZ
else
    boxPara(S,1:3) = normalTemp; % normalX, normalY, normalZ
end
% with respect to X
boxPara(S,4  ) = abs(d1 + d2)/2;

if (d2*d1<0)
    thickness = abs(d2)+abs(d1); 
else
    thickness = max(distance1, distance2) - min(distance1, distance2);
end
boxPara(S,5) = thickness;

k=1;
boxStruct(k).id = num2str(k);
boxStruct(k).label = 'sagittal';
boxStruct(k).normal_x = num2str(boxPara(S,1));
boxStruct(k).normal_y = num2str(boxPara(S,2));
boxStruct(k).normal_z = num2str(boxPara(S,3));
boxStruct(k).thickness = num2str(thickness);
boxStruct(k).distance_from_origin = num2str(boxPara(S,4  )); % with respect to X
