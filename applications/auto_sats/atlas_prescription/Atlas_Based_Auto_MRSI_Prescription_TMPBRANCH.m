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
function varargout = Atlas_Based_Auto_MRSI_Prescription_TMPBRANCH(varargin)
% ATLAS_BASED_AUTO_MRSI_PRESCRIPTION MATLAB code for Atlas_Based_Auto_MRSI_Prescription.fig
%      ATLAS_BASED_AUTO_MRSI_PRESCRIPTION, by itself, creates a new ATLAS_BASED_AUTO_MRSI_PRESCRIPTION or raises the existing
%      singleton*.
%
%      H = ATLAS_BASED_AUTO_MRSI_PRESCRIPTION returns the handle to a new ATLAS_BASED_AUTO_MRSI_PRESCRIPTION or the handle to
%      the existing singleton*.
%
%      ATLAS_BASED_AUTO_MRSI_PRESCRIPTION('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in ATLAS_BASED_AUTO_MRSI_PRESCRIPTION.M with the given input arguments.
%
%      ATLAS_BASED_AUTO_MRSI_PRESCRIPTION('Property','Value',...) creates a new ATLAS_BASED_AUTO_MRSI_PRESCRIPTION or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before Atlas_Based_Auto_MRSI_Prescription_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to Atlas_Based_Auto_MRSI_Prescription_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help Atlas_Based_Auto_MRSI_Prescription

% Last Modified by GUIDE v2.5 10-Nov-2015 15:15:03

% Wei Bian Created in August,2014
% Sarah Nelson's Lab
% Department of Radiology UCSF

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @Atlas_Based_Auto_MRSI_Prescription_OpeningFcn, ...
                   'gui_OutputFcn',  @Atlas_Based_Auto_MRSI_Prescription_OutputFcn, ...
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


% --- Executes just before Atlas_Based_Auto_MRSI_Prescription is made visible.
function Atlas_Based_Auto_MRSI_Prescription_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to Atlas_Based_Auto_MRSI_Prescription (see VARARGIN)

% Choose default command line output for Atlas_Based_Auto_MRSI_Prescription
handles.output = hObject;

curdir = pwd;
p = inputParser;
addOptional(p,'atlas_dir', curdir);
addOptional(p,'regist_dir', curdir);
addOptional(p,'output_dir', curdir);
addOptional(p,'flirt_dir', '');

parse(p, varargin{:});
handles.atlas_dir = p.Results.atlas_dir;
handles.regist_dir = p.Results.regist_dir;
handles.output_dir = p.Results.output_dir;
handles.flirt_dir = p.Results.flirt_dir;

handles.edit_atlasDir = findobj(hObject, 'Tag','edit_atlas_dir');
handles.listbox_existFiles = findobj(hObject, 'Tag','listbox_exist_files');
handles.listbox_selectedFiles = findobj(hObject, 'Tag','listbox_selected_files');
handles.listbox_outputFiles = findobj(hObject, 'Tag','listbox_output_files');
handles.text_progressStatus =  findobj(hObject, 'Tag','text_progress_status');
handles.pushbutton_remove = findobj(hObject, 'Tag','pushbutton_remove');
set(handles.pushbutton_remove, 'Enable','off');
handles.edit_boxSizeRL = findobj(hObject, 'Tag','edit_BoxSizeRL');
set(handles.edit_boxSizeRL, 'Enable','off');
handles.edit_boxSizeAP = findobj(hObject, 'Tag','edit_BoxSizeAP');
set(handles.edit_boxSizeAP, 'Enable','off');
handles.edit_boxSizeSI = findobj(hObject, 'Tag','edit_BoxSizeSI');
set(handles.edit_boxSizeSI, 'Enable','off');
handles.edit_satThickness = findobj(hObject, 'Tag','edit_SATThickness');
set(handles.edit_satThickness, 'Enable','off');
handles.pushbutton_register = findobj(hObject, 'Tag','pushbutton_register');
set(handles.pushbutton_register, 'Enable','off');
handles.pushbutton_applyTMaxtrix = findobj(hObject, 'Tag','pushbutton_apply_tmatrix');
set(handles.pushbutton_applyTMaxtrix, 'Enable','off');
handles.pushbutton_export = findobj(hObject, 'Tag','pushbutton_export');
set(handles.pushbutton_export, 'Enable','off');

handles.selected_file_names = [];
handles.output_file_names = [];
handles.registDone = false;
handles.atlas_images = 'Atlas_default.nii';
set(handles.edit_atlasImages,'String',handles.atlas_images);

% Update handles structure
guidata(hObject, handles);
dir_path = handles.atlas_dir;
load_listbox(dir_path, handles);

function load_listbox(dir_path, handles)
cd (dir_path)
dir_struct = dir(dir_path);
[sorted_names,sorted_index] = sortrows({dir_struct.name}');
handles.exist_file_names = sorted_names;
handles.is_dir = [dir_struct.isdir];
handles.sorted_index = sorted_index;
handles.atlas_dir = pwd;
set(handles.listbox_existFiles,'String',handles.exist_file_names, 'Value',1)
set(handles.edit_atlasDir,'String',handles.atlas_dir);
guidata(handles.figure1,handles)

% UIWAIT makes Atlas_Based_Auto_MRSI_Prescription wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = Atlas_Based_Auto_MRSI_Prescription_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in pushbutton_add.
function pushbutton_add_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_add (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

index_selected = get(handles.listbox_existFiles,'Value');
file_list = get(handles.listbox_existFiles,'String');
filename = file_list{index_selected};
if strcmpi(filename(end-2:end), 'xml')
    if isempty(handles.selected_file_names) || ~any(strncmpi(filename, handles.selected_file_names, 100))
        handles.selected_file_names = [{filename} ; handles.selected_file_names];
        set(handles.listbox_selectedFiles,'String',handles.selected_file_names, 'Value',1)
    else
        errordlg({'The file is already on the list'},' ')
    end
else
    errordlg({'Must select an .xml file'},'Looking for an xml file')
end
set(handles.pushbutton_remove, 'Enable','on');
set(handles.pushbutton_register, 'Enable','on');
guidata(hObject, handles);


% --- Executes on button press in pushbutton_remove.
function pushbutton_remove_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_remove (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

index_selected = get(handles.listbox_selectedFiles,'Value');
file_list = get(handles.listbox_selectedFiles,'String');
if index_selected == 1
    file_list = file_list(2:end);
elseif index_selected == length(file_list)
    file_list = file_list(1:end-1);
else
    file_list = file_list([1:index_selected-1 index_selected+1:end]);
end
handles.selected_file_names = file_list;
set(handles.listbox_selectedFiles,'String',handles.selected_file_names, 'Value',1)
if isempty(file_list)
    set(handles.pushbutton_remove, 'Enable','off');
    set(handles.pushbutton_register, 'Enable','off');
end
guidata(hObject, handles);


function edit_atlas_dir_Callback(hObject, eventdata, handles)
% hObject    handle to edit_atlas_dir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_atlas_dir as text
%        str2double(get(hObject,'String')) returns contents of edit_atlas_dir as a double

dir_path = get(hObject,'String');
load_listbox(dir_path, handles);


% --- Executes during object creation, after setting all properties.
function edit_atlas_dir_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_atlas_dir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in listbox_exist_files.
function listbox_exist_files_Callback(hObject, eventdata, handles)
% hObject    handle to listbox_exist_files (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns listbox_exist_files contents as cell array
%        contents{get(hObject,'Value')} returns selected item from listbox_exist_files
get(handles.figure1,'SelectionType');
% If double click
index_selected = get(handles.listbox_existFiles,'Value');
file_list = get(handles.listbox_existFiles,'String');
filename = file_list{index_selected};
if strcmp(get(handles.figure1,'SelectionType'),'open') && handles.is_dir(handles.sorted_index(index_selected))
    cd (filename)
    load_listbox(pwd,handles)
end


% --- Executes during object creation, after setting all properties.
function listbox_exist_files_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listbox_exist_files (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in listbox_selected_files.
function listbox_selected_files_Callback(hObject, eventdata, handles)
% hObject    handle to listbox_selected_files (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns listbox_selected_files contents as cell array
%        contents{get(hObject,'Value')} returns selected item from listbox_selected_files


% --- Executes during object creation, after setting all properties.
function listbox_selected_files_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listbox_selected_files (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in listbox_output_files.
function listbox_output_files_Callback(hObject, eventdata, handles)
% hObject    handle to listbox_output_files (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns listbox_output_files contents as cell array
%        contents{get(hObject,'Value')} returns selected item from listbox_output_files


% --- Executes during object creation, after setting all properties.
function listbox_output_files_CreateFcn(hObject, eventdata, handles)
% hObject    handle to listbox_output_files (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: listbox controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton_apply_tmatrix.
function pushbutton_apply_tmatrix_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_apply_tmatrix (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set(handles.text_progressStatus, 'String', 'Read the xml file of Box and SAT bands');
drawnow
index_selected = get(handles.listbox_selectedFiles,'Value');
file_list = get(handles.listbox_selectedFiles,'String');
xml_filename = file_list{index_selected};
system(['/bin/cp -f ', strcat(handles.atlas_dir, '/', xml_filename), ' ', handles.regist_dir]);
[ver, boxPlane_atlas satBand_atlas BoxSatXMLStruct] = readBoxSatInfo(xml_filename);

%% Apply the transformation matrix to PRESS Box

set(handles.text_progressStatus, 'String', 'Apply the transformation matrix to PRESS Box and SAT Bands');
drawnow
[handles.boxPlane_patient, handles.satBand_patient, boxDatName, satDatName] = applyTransMatrix(ver, xml_filename, boxPlane_atlas, satBand_atlas, handles);

set(handles.text_progressStatus, 'String', 'Displaying the transformed PRESS box and SAT bands...');
drawnow

satBands4Disp = handles.satBand_patient;
if get(handles.checkbox_DisplayOCTSat, 'value') == 1
    if (strcmp(ver,'2.0'))
        satBands4Disp = addOctSats20(handles.boxPlane_patient, handles.satBand_patient);
    else
        satBands4Disp = addOctSats(handles.boxPlane_patient, handles.satBand_patient);        
    end
end

if handles.pixelSize_patient(1) <= 1
    pixelSize_patient(1:2) = handles.pixelSize_patient(1:2).*2;
    pixelSize_patient(3) = handles.pixelSize_patient(3);
    display_box_satbands(ver, handles.img(1:2:end, 1:2:end, :), handles.boxPlane_patient, satBands4Disp, pixelSize_patient, handles.x_axis(1:2:end), handles.y_axis(1:2:end), handles.z_axis);
else
    display_box_satbands(ver, handles.img, handles.boxPlane_patient, satBands4Disp, handles.pixelSize_patient, handles.x_axis, handles.y_axis, handles.z_axis);
end

% copy files to the output direcotry
if ~strncmpi(boxDatName, handles.output_file_names, 100)
    handles.output_file_names = [{boxDatName} ; handles.output_file_names];
    set(handles.listbox_outputFiles,'String',handles.output_file_names, 'Value',1)
    if ~isempty(satDatName)
        handles.output_file_names = [{satDatName} ; handles.output_file_names];
        set(handles.listbox_outputFiles,'String',handles.output_file_names, 'Value',1)
    end
end

% Commented because people may NOT want to overwrite previous output files
% Instead use the button 'Export' on the GUI to copy following files to
% the output diretory

% system(['/bin/cp -f ', boxDatName, ' ', handles.output_dir, '/press_box.dat']);
% if ~isempty(satDatName)
%     system(['/bin/cp -f ', satDatName, ' ', handles.output_dir, '/sat_bands.dat']);
% end

% Write XML file

idx1 = strfind(xml_filename, '_');
idx2 =  strfind(xml_filename, '.')-1;
xml_postfix = xml_filename(idx1(end)+1:idx2);
file_name = strcat('box_satbands_', xml_postfix, '.dat');

if (strcmp(ver, '2.0'))
    
    w = XMLWriter();
    w.Initialize(); % write header
    w.writePRESSBoxSatBands(XMLReader.convertNumStructToXML(handles.boxPlane_patient));
    w.writeAutoSatBands(XMLReader.convertNumStructToXML(handles.satBand_patient));
    XMLWriter.writeXMLFile(w.xmlObject, file_name);
else
    for k = 1:6
        BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.normal_x = handles.boxPlane_patient(k).normal(1);
        BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.normal_y = handles.boxPlane_patient(k).normal(2);
        BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.normal_z = handles.boxPlane_patient(k).normal(3);
        BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.distance_from_origin = handles.boxPlane_patient(k).distance;
        BoxSatXMLStruct.pressBoxSatsStruct(k).sat_band.thickness = 0;
    end
    for k = 1:length(handles.satBand_patient)
        BoxSatXMLStruct.autoSatsStruct(k).sat_band.normal_x = handles.satBand_patient(k).normal(1);
        BoxSatXMLStruct.autoSatsStruct(k).sat_band.normal_y = handles.satBand_patient(k).normal(2);
        BoxSatXMLStruct.autoSatsStruct(k).sat_band.normal_z = handles.satBand_patient(k).normal(3);
        BoxSatXMLStruct.autoSatsStruct(k).sat_band.distance_from_origin = handles.satBand_patient(k).distance;
        if get(handles.checkbox_FixSATThickness, 'value') == 0
            BoxSatXMLStruct.autoSatsStruct(k).sat_band.thickness = handles.satBand_patient(k).thickness;
        else
            BoxSatXMLStruct.autoSatsStruct(k).sat_band.thickness = handles.satThickness;
        end
    end
    svk_write_xml_sats(xml_filename, file_name, BoxSatXMLStruct);
end


% Commented because people may NOT want to overwrite previous output files
% Instead use the button 'Export' on the GUI to copy following files to
% the output directory
% system(['/bin/cp -f ', file_name, ' ', handles.output_dir, '/box_satbands_atlasbased.dat']);

if ~strncmpi(file_name, handles.output_file_names, 100)
    handles.output_file_names = [{file_name} ; handles.output_file_names];
    set(handles.listbox_outputFiles,'String',handles.output_file_names, 'Value',1)
end

guidata(hObject, handles);


% --- Executes on button press in pushbutton_register.
function pushbutton_register_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_register (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if handles.registDone
    choice = questdlg('Registration has been done. Redo it?', ' ', 'Yes', 'No', 'No');
    if strcmp(choice,'No')
        return
    end
else
    handles.registDone = true;
end

%% copy the Atlas file to the directory where the image registration happens
cd(handles.regist_dir)
filename = strcat(handles.atlas_dir, '/', handles.atlas_images);
system(['/bin/cp -f ', filename, ' ', handles.regist_dir]);

index_selected = get(handles.listbox_selectedFiles,'Value');
file_list = get(handles.listbox_selectedFiles,'String');
filename = strcat(handles.atlas_dir, '/', file_list{index_selected});
system(['/bin/cp -f ', filename, ' ', handles.regist_dir]);


%% read atlas NIFTI header

hdr = load_nii_hdr(handles.atlas_images);

% Matrix that transforms voxel coordinates to RAS coodinates defined in the Atlas NIFTI header
handles.qform_atlas = [hdr.hist.srow_x ; hdr.hist.srow_y; hdr.hist.srow_z; 0 0 0 1];

handles.pixelSize_atlas = hdr.dime.pixdim(2:4); 

%% Convert DICOM to NIFTI

% Check DCM file directory
%temp = '/data/brain_work/stojan/development/sat_placement_atlas_based/trunk/atlasPrescription';
%handles.regist_dir = temp;
DCM_files = dir(strcat(handles.regist_dir,'/*.DCM'));
if isempty(DCM_files)
    DCM_files = dir(strcat(handles.regist_dir,'/*.dcm'));
end;

if isempty(DCM_files)
    errordlg({'No T1 DICOM images found'},'Looking for T1 DICOM images')
    return
else
    set(handles.text_progressStatus, 'string', 'Converting the patient DICOM images to a NIFTI volume image...')
    drawnow
    file_name = DCM_files(1,1).name;
    if isempty(handles.flirt_dir) % Empty only if NOT ran on the scanner, so dcm2nii should be called directly no /../
        command = strcat(handles.flirt_dir, 'dcm2nii -f y -d n -e n -g n -i n -p n -r n');
    else
        command = strcat(handles.flirt_dir, '/../dcm2nii -f y -d n -e n -g n -i n -p n -r n');
    end
    system([command ' ' handles.regist_dir '/' file_name]);
end

%% Load NIFTI image volume

file_name = [handles.regist_dir '/' file_name(1:end-3) 'nii']; % NOTE: check DICOM naming style if used for different scanners
% %TODOSM
% file_name = [temp '/Atlas_default.nii'];
NII_IM = load_nii(file_name);
img = NII_IM.img;
img = flipdim(img,3);
img = flipdim(img,2);
img = permute(img, [2 1 3]);
img = flipdim(img, 2);
handles.img = double(img);
[nrow ncol nslice] = size(handles.img);
handles.pixelSize_patient = NII_IM.hdr.dime.pixdim(2:4);

% Form the matrix that transform voxel coordinates to RAS coodinates defined in the patient NIFTI headerswitch NII_IM.hdr.hist.rot_orient(1)
handles.qform_patient = [];
switch NII_IM.hdr.hist.rot_orient(1)
    case 1 % Axial acquisition
        handles.qform_patient = [NII_IM.hdr.hist.srow_x; NII_IM.hdr.hist.srow_y; NII_IM.hdr.hist.srow_z];
    case 2 % Coronal acquisition
        qform = [NII_IM.hdr.hist.srow_x; NII_IM.hdr.hist.srow_y; NII_IM.hdr.hist.srow_z];
        originPixel = qform*[0 0 nrow-1 1]'; % potential bug (haven't tested for coronal acquisition)
        handles.qform_patient(NII_IM.hdr.hist.rot_orient,:) = qform;
        handles.qform_patient(:,4) = originPixel';
    case 3 % Saggital acquisition
        qform = [NII_IM.hdr.hist.srow_x; NII_IM.hdr.hist.srow_y; NII_IM.hdr.hist.srow_z];
        originPixel = qform*[nslice-1 0 ncol-1 1]';
        handles.qform_patient(NII_IM.hdr.hist.rot_orient,:) = qform;
        handles.qform_patient(:,4) = originPixel';
end
handles.qform_patient = [handles.qform_patient; 0 0 0 1];

%% Resample the atlas and patient images
set(handles.text_progressStatus, 'String', 'Resampling the NIFTI image...');
drawnow
if get(handles.checkbox_regist_quality, 'value') == 0
    reslice_nii(file_name, 'patient_resampled.nii', [2 2 2], 0);
    reslice_nii(handles.atlas_images, 'Atlas_resampled.nii', [2 2 2], 0);
else
    reslice_nii(file_name, 'patient_resampled.nii', [1 1 1], 0);
end

atlPath = '/Atlas_resampled.nii';
patPath = '/patient_resampled.nii';
%% Add BET to the pipeline
if get(handles.checkbox_BET, 'value') == 1
    set(handles.text_progressStatus, 'String', 'Performing brain extraction......');
    drawnow
    if isempty(handles.flirt_dir) % Empty only if NOT ran on the scanner, so dcm2nii should be called directly no /../
        fslEnvCommand = '';
    else
        fslEnvCommand = 'setenv FSLDIR /export/home/sdc/svk/console/packages/Atlas_Based_Auto_MRSI_Prescription; setenv FSLOUTPUTTYPE NIFTI_GZ; ';
    end
    system([fslEnvCommand handles.flirt_dir 'bet ' handles.regist_dir atlPath ' ' handles.regist_dir '/Atlas_resampled_bet.nii'])
    system([fslEnvCommand handles.flirt_dir 'bet ' handles.regist_dir patPath ' ' handles.regist_dir '/patient_resampled_bet.nii'])

    atlPath = '/Atlas_resampled_bet.nii.gz';
    patPath = '/patient_resampled_bet.nii.gz';
end
%% Do registration using FSL 

set(handles.text_progressStatus, 'String', 'Registration is now starting and will take a while...');
drawnow
command = strcat(handles.flirt_dir, 'flirt');
command = strcat('setenv FSLOUTPUTTYPE NIFTI_GZ; ', command);
if get(handles.checkbox_regist_quality, 'value') == 0
    system([command ' -in ' handles.regist_dir atlPath ' -ref ' handles.regist_dir patPath ' -out AtlasToPatient -omat transMatrix -cost normmi -dof 12 -forcescaling -searchrx -20 20 -searchry -20 20 -searchrz -20 20 ']);
else
    system([command ' -in ' handles.regist_dir atlPath ' -ref ' handles.regist_dir patPath ' -out AtlasToPatient -omat transMatrix -cost normmi -dof 12 -forcescaling -searchrx -20 20 -searchry -20 20 -searchrz -20 20 ']);
end

% Get the transformation matrix from Atlas space to Patient space
[status tmatrix] = system('cat transMatrix');
if status
    errordlg({'No transformation matrix found'},'Check registration!')
    return
end
handles.tmatrix = str2num(tmatrix);
% %TODOSM
% handles.tmatrix = [1 0 0 0; 0 1 0 0; 0 0 1 0; 0 0 0 1];
set(handles.text_progressStatus, 'String', 'Registration completed!');
guidata(hObject, handles);

%% Read the xml file that contain info of the predefined PRESS box and SAT bands on Atlas
set(handles.text_progressStatus, 'String', 'Read the xml files of Box and SAT bands');
drawnow
index_selected = get(handles.listbox_selectedFiles,'Value');
file_list = get(handles.listbox_selectedFiles,'String');
xml_filename = file_list{index_selected};

% returned structures are XMLReader structures with num values
[ver, boxPlane_atlas satBand_atlas BoxSatXMLStruct] = readBoxSatInfo(xml_filename); % includes the new version of XMLReader


%% Apply the transformation matrix to PRESS Box

set(handles.text_progressStatus, 'String', 'Apply the transformation matrix to PRESS Box and SAT Bands');
drawnow
[handles.boxPlane_patient, handles.satBand_patient, boxDatName, satDatName] = applyTransMatrix(ver, xml_filename, boxPlane_atlas, satBand_atlas, handles);

% display box and/or satbands
set(handles.text_progressStatus, 'String', 'Displaying the transformed PRESS box and SAT bands...');
drawnow
maxDim = max([nrow ncol nslice]);
VOL = [(0:maxDim-1); (0:maxDim-1); (0:maxDim-1); ones(1,maxDim)];
RAS = handles.qform_patient*VOL;
handles.x_axis = -RAS(1,1:ncol); % Now positive is L
handles.y_axis = fliplr(-RAS(2,1:nrow)); % Now positive is P
handles.z_axis = fliplr(RAS(3,1:nslice));

satBands4Disp = handles.satBand_patient;
if get(handles.checkbox_DisplayOCTSat, 'value') == 1
    %TODOSM
    if (strcmp(ver,'2.0'))
        satBands4Disp = addOctSats20(handles.boxPlane_patient, handles.satBand_patient);
    else
        satBands4Disp = addOctSats(handles.boxPlane_patient, handles.satBand_patient);        
    end
end
    
if handles.pixelSize_patient(1) <= 1
    pixelSize_patient(1:2) = handles.pixelSize_patient(1:2).*2;
    pixelSize_patient(3) = handles.pixelSize_patient(3);
    display_box_satbands(ver, handles.img(1:2:end, 1:2:end, :), handles.boxPlane_patient, satBands4Disp, pixelSize_patient, handles.x_axis(1:2:end), handles.y_axis(1:2:end), handles.z_axis);
else
    display_box_satbands(ver, handles.img, handles.boxPlane_patient, satBands4Disp, handles.pixelSize_patient, handles.x_axis, handles.y_axis, handles.z_axis);
end
set(handles.pushbutton_applyTMaxtrix, 'Enable','on');
set(handles.pushbutton_export, 'Enable','on');

% copy files to the output directory
system(['/bin/cp -f ', boxDatName, ' ', handles.output_dir, '/press_box.dat']);
if ~isempty(satDatName)
    system(['/bin/cp -f ', satDatName, ' ', handles.output_dir, '/sat_bands.dat']);
end
if ~strncmpi(boxDatName, handles.output_file_names, 100)
    handles.output_file_names = [{boxDatName} ; handles.output_file_names];
    set(handles.listbox_outputFiles,'String',handles.output_file_names, 'Value',1)
    if ~isempty(satDatName)
        handles.output_file_names = [{satDatName} ; handles.output_file_names];
        set(handles.listbox_outputFiles,'String',handles.output_file_names, 'Value',1)
    end
end

idx1 = strfind(file_name, 'S');
idx2 = strfind(file_name, 'I');
examNumb = file_name(1:idx1-1);
serialNumb = file_name(idx1:idx2-1);
fid = fopen('transMatrix','a');
fprintf(fid, '\n');
fprintf(fid,'Transformation Version: 1');
fprintf(fid,'Exam Number: %s\n', examNumb);
fprintf(fid,'Input image: %s\n', handles.atlas_images);
fprintf(fid,'Target image (Series Number): %s\n', serialNumb);
fprintf(fid,'Template prescription xml: %s\n', xml_filename);
fclose(fid);
system(['/bin/cp -f ', 'transMatrix', ' ', handles.output_dir, '/transMatrix.dat']);

if get(handles.checkbox_FixSATThickness, 'value') ~= 0
    for k = 1:length(handles.satBand_patient)
        handles.satBand_patient(k).thickness = handles.satThickness;
    end
end

idx1 = strfind(xml_filename, '_');
idx2 =  strfind(xml_filename, '.')-1;
xml_postfix = xml_filename(idx1(end)+1:idx2);
file_name = strcat('box_satbands_', xml_postfix, '.dat');
if (strcmp(ver, '2.0'))
    writer = XMLWriter();
    writer.Initialize(); % write header
    writer.writePRESSBoxSatBands(XMLReader.convertNumStructToXML(handles.boxPlane_patient))
    writer.writeAutoSatBands(XMLReader.convertNumStructToXML(handles.satBand_patient));
    XMLWriter.writeXMLFile(writer.xmlObject, file_name)
else
    svk_write_xml_sats(xml_filename, file_name, BoxSatXMLStruct); 
end
system(['/bin/cp -f ', file_name, ' ', handles.output_dir, '/box_satbands_atlasbased.dat']);
if ~strncmpi(file_name, handles.output_file_names, 100)
    handles.output_file_names = [{file_name} ; handles.output_file_names];
    set(handles.listbox_outputFiles,'String',handles.output_file_names, 'Value',1)
end

%to make the stupid compiler not miss that function (per Eugene)
if(0)
    mia_Stop3dCursor();
end;

guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function popupmenu_regist_quality_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu_regist_quality (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton_display.
% comment now but may be used in the future
% function pushbutton_display_Callback(hObject, eventdata, handles)
% % hObject    handle to pushbutton_display (see GCBO)
% % eventdata  reserved - to be defined in a future version of MATLAB
% % handles    structure with handles and user data (see GUIDATA)
% set(handles.text_progressStatus, 'String', 'Displaying the transformed PRESS box and SAT bands...');
% drawnow
% if handles.pixelSize_patient(1) <= 1
%     pixelSize_patient(1:2) = handles.pixelSize_patient(1:2).*2;
%     pixelSize_patient(3) = handles.pixelSize_patient(3);
%     display_box_satbands(handles.img(1:2:end, 1:2:end, :), handles.boxPlane_patient, handles.satBand_patient, pixelSize_patient, handles.x_axis(1:2:end), handles.y_axis(1:2:end), handles.z_axis);
% else
%     display_box_satbands(handles.img, handles.boxPlane_patient, handles.satBand_patient, handles.pixelSize_patient, handles.x_axis, handles.y_axis, handles.z_axis);
% end


% --- Executes on button press in pushbutton_export.
function pushbutton_export_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_export (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

index_selected = get(handles.listbox_outputFiles,'Value');
file_list = get(handles.listbox_outputFiles,'String');
filename = file_list{index_selected};
%handles.regist_dir
if strcmp(filename(1:3), 'sat');
    system(['/bin/cp -f ', handles.regist_dir '/' filename, ' ', handles.output_dir, '/sat_bands.dat']);
elseif strcmp(filename(1:3), 'pre');
    system(['/bin/cp -f ', handles.regist_dir '/' filename, ' ', handles.output_dir, '/press_box.dat']);
else
    system(['/bin/cp -f ', handles.regist_dir '/' filename, ' ', handles.output_dir, '/box_satbands_atlasbased.dat']);
end
set(handles.text_progressStatus, 'String', ['The file copied to ' handles.output_dir]);


% --- Executes on button press in pushbutton_close.
function pushbutton_close_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_close (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
choice = questdlg('Quit the GUI?', ' ', 'Yes', 'No', 'No');
if strcmp(choice,'Yes')
    delete(handles.figure1)
end
    

% --- Executes on button press in checkbox_regist_quality.
function checkbox_regist_quality_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_regist_quality (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_regist_quality

handles.checkbox_registQuality = get(hObject,'Value');
guidata(hObject, handles);


function edit_atlasImages_Callback(hObject, eventdata, handles)
% hObject    handle to edit_atlasImages (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_atlasImages as text
%        str2double(get(hObject,'String')) returns contents of edit_atlasImages as a double

handles.atlas_images = get(hObject,'String');
if ~exist(handles.atlas_images, 'file');
    errordlg({'The atlas image does NOT exist'},'Check the directory!')
end
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function edit_atlasImages_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_atlasImages (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in pushbutton_update.
function pushbutton_update_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton_update (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
index_selected = get(handles.listbox_existFiles,'Value');
file_list = get(handles.listbox_existFiles,'String');
filename = file_list{index_selected};
if strcmp(filename(end-2:end),'nii')
    set(handles.edit_atlasImages,'String',filename);
    handles.atlas_images = filename;
else
    errordlg({'Atlas images must be a NIFTI file'},'Select a NIFTI file!')
end
guidata(hObject, handles);


% --- Executes on button press in checkbox_FixBoxSize.
function checkbox_FixBoxSize_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_FixBoxSize (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_FixBoxSize
checked = get(hObject,'Value');
if checked
    set(handles.edit_boxSizeRL, 'Enable','on');
    handles.boxSizeRL = str2num(get(handles.edit_boxSizeRL,'String'));
    set(handles.edit_boxSizeAP, 'Enable','on');
    handles.boxSizeAP = str2num(get(handles.edit_boxSizeAP,'String'));
    set(handles.edit_boxSizeSI, 'Enable','on');
    handles.boxSizeSI = str2num(get(handles.edit_boxSizeSI,'String'));
else
    set(handles.edit_boxSizeRL, 'Enable','off');
    set(handles.edit_boxSizeAP, 'Enable','off');
    set(handles.edit_boxSizeSI, 'Enable','off');
end
guidata(hObject, handles);

function edit_BoxSizeRL_Callback(hObject, eventdata, handles)
% hObject    handle to edit_BoxSizeRL (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_BoxSizeRL as text
%        str2double(get(hObject,'String')) returns contents of edit_BoxSizeRL as a double

handles.boxSizeRL = str2num(get(hObject,'String'));
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function edit_BoxSizeRL_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_BoxSizeRL (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit_BoxSizeSI_Callback(hObject, eventdata, handles)
% hObject    handle to edit_BoxSizeSI (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_BoxSizeSI as text
%        str2double(get(hObject,'String')) returns contents of edit_BoxSizeSI as a double

handles.boxSizeSI = str2num(get(hObject,'String'));
guidata(hObject, handles);


% --- Executes during object creation, after setting all properties.
function edit_BoxSizeSI_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_BoxSizeSI (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit_BoxSizeAP_Callback(hObject, eventdata, handles)
% hObject    handle to edit_BoxSizeAP (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_BoxSizeAP as text
%        str2double(get(hObject,'String')) returns contents of edit_BoxSizeAP as a double

handles.boxSizeAP = str2num(get(hObject,'String'));
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function edit_BoxSizeAP_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_BoxSizeAP (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in checkbox_FixSATThickness.
function checkbox_FixSATThickness_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_FixSATThickness (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_FixSATThickness

checked = get(hObject,'Value');
if checked
    set(handles.edit_satThickness, 'Enable','on');
    handles.satThickness = str2num(get(handles.edit_satThickness,'String'));
else
    set(handles.edit_satThickness, 'Enable','off');
end
guidata(hObject, handles);


% --- Executes on button press in checkbox_DisplayOCTSat.
function checkbox_DisplayOCTSat_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_DisplayOCTSat (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_DisplayOCTSat

handles.DisplayOCTSat = get(hObject,'Value');
guidata(hObject, handles);

function edit_SATThickness_Callback(hObject, eventdata, handles)
% hObject    handle to edit_SATThickness (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit_SATThickness as text
%        str2double(get(hObject,'String')) returns contents of edit_SATThickness as a double

handles.satThickness = str2num(get(hObject,'String'));
guidata(hObject, handles);

% --- Executes during object creation, after setting all properties.
function edit_SATThickness_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit_SATThickness (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in checkbox_BET.
function checkbox_BET_Callback(hObject, eventdata, handles)
% hObject    handle to checkbox_BET (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of checkbox_BET


% --- If Enable == 'on', executes on mouse press in 5 pixel border.
% --- Otherwise, executes on mouse press in 5 pixel border or over pushbutton_register.
function pushbutton_register_ButtonDownFcn(hObject, eventdata, handles)
% hObject    handle to pushbutton_register (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
