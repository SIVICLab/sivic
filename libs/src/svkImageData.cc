/*
 *  Copyright © 2009-2010 The Regents of the University of California.
 *  All Rights Reserved.
 *
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met:
 *  •   Redistributions of source code must retain the above copyright notice, 
 *      this list of conditions and the following disclaimer.
 *  •   Redistributions in binary form must reproduce the above copyright notice, 
 *      this list of conditions and the following disclaimer in the documentation 
 *      and/or other materials provided with the distribution.
 *  •   None of the names of any campus of the University of California, the name 
 *      "The Regents of the University of California," or the names of any of its 
 *      contributors may be used to endorse or promote products derived from this 
 *      software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 *  OF SUCH DAMAGE.
 */



/*
 *  $URL$
 *  $Rev$
 *  $Author$
 *  $Date$
 *
 *  Authors:
 *      Jason C. Crane, Ph.D.
 *      Beck Olson
 */


#include <svkImageData.h>


using namespace svk;


vtkCxxRevisionMacro(svkImageData, "$Rev$");


/*!
 *  Constructor. Creates a default directional cosine matrix.
 *  dcos = +x, +y, -z. The order is Ux, Vx, Wx, Uy, Vy, Wy, Uz, Vz, Wz.
 */
svkImageData::svkImageData()
{

#if VTK_DEBUG_ON
    this->DebugOn();
#endif

    dcos[0][0] = 99;
    dcos[0][1] = 99;
    dcos[0][2] = 99;
    dcos[1][0] = 99;
    dcos[1][1] = 99;
    dcos[1][2] = 99;
    dcos[2][0] = 99;
    dcos[2][1] = 99;
    dcos[2][2] = 99;

    this->dcmHeader = NULL;
    if ( svkDcmHeader::adapter_type == svkDcmtkAdapter::DCMTK_API ) {
        this->dcmHeader = svkDcmtkAdapter::New();
    }
 
    this->provenance = svkProvenance::New(); 

    this->range[0][0] = VTK_DOUBLE_MAX;
    this->range[0][1] = -VTK_DOUBLE_MAX;
    this->range[1][0] = VTK_DOUBLE_MAX;
    this->range[1][1] = -VTK_DOUBLE_MAX;
    this->range[2][0] = VTK_DOUBLE_MAX;
    this->range[2][1] = -VTK_DOUBLE_MAX;

    this->lastUpdateTime = this->GetMTime();

}


/*!
 * Destructor.
 */
svkImageData::~svkImageData()
{
    vtkDebugMacro(<<"svkImageData::~svkImageData");

    if (this->dcmHeader != NULL) {
        this->dcmHeader->Delete();
        this->dcmHeader = NULL;
    }

    if (this->provenance != NULL) {
        this->provenance->Delete();
        this->provenance = NULL;
    }

    if (this->topoGenerator != NULL ) {
        this->topoGenerator->Delete();
        this->topoGenerator = NULL;
    }
}


/*!
 *  PrintSelf method calls parent class PrintSelf, then prints the dcos.
 *
 */
void svkImageData::PrintSelf( ostream &os, vtkIndent indent )
{
    Superclass::PrintSelf( os, indent );
    os << "  dcos: " << this->dcos[0][0] << " " << this->dcos[0][1] << " " << this->dcos[0][2] << endl;
    os << "        " << this->dcos[1][0] << " " << this->dcos[1][1] << " " << this->dcos[1][2] << endl;
    os << "        " << this->dcos[2][0] << " " << this->dcos[2][1] << " " << this->dcos[2][2] << endl;
}


/*!
 *  DeepCopy creates a deep copy of svkImageData, including orientation information.
 *  First calls vtkImageData's deep copy, then also copies the dcos, but does NOT copy the 
 *  DICOM header.
 */
void svkImageData::DeepCopy( vtkDataObject* src, svkDcmHeader::DcmPixelDataFormat castToFormat)
{
    this->Superclass::DeepCopy( src );
    if( src->IsA("svkImageData") ) {
        svkImageData::SafeDownCast(src)->GetDcmHeader()->MakeDerivedDcmHeader(this->GetDcmHeader(), ""); 
    }
    this->CopyDcos( src );
    if( castToFormat != svkDcmHeader::UNDEFINED ) {
        this->CastDataFormat( castToFormat );
    }
}


/*!
 *  First calls vtkImageData's shallow copy, then also copies the dcos, but does NOT copy the 
 *  DICOM header.
 */
void svkImageData::ShallowCopy( vtkDataObject* src )
{
    this->vtkImageData::ShallowCopy( src );
    this->CopyDcos( src );
}


/*!
 *  First calls CopyStructure, then also copies the dcos, but does NOT copy the 
 *  DICOM header.
 */
void svkImageData::ZeroCopy( vtkImageData* src, svkDcmHeader::DcmPixelDataFormat castToFormat)
{
    // First we copy the basic structure, this copies no data
    this->CopyStructure( src );

    // Next lets figure out the vtk equivalent data type for the target
    int dataTypeVtk;
    switch( castToFormat ) {
        case svkDcmHeader::UNSIGNED_INT_1:
            dataTypeVtk = VTK_UNSIGNED_CHAR;
            break; 
        case svkDcmHeader::UNSIGNED_INT_2:
            dataTypeVtk = VTK_UNSIGNED_SHORT;
            break; 
        case svkDcmHeader::SIGNED_FLOAT_4:
            dataTypeVtk = VTK_FLOAT;
            break; 
        case svkDcmHeader::SIGNED_FLOAT_8:
            dataTypeVtk = VTK_DOUBLE;
            break; 
    }

    // Now lets get the cell data structure 
    int numCellArrays = src->GetCellData()->GetNumberOfArrays();
    int numCellTuples = 0;
    int numCellComponents = 0;
    if( numCellArrays > 0 ) {
        vtkDataArray* firstCellArray = src->GetCellData()->GetArray(0);
        numCellTuples = firstCellArray->GetNumberOfTuples();
        numCellComponents = firstCellArray->GetNumberOfComponents();
    }

    // And create new arrays of the appropriate type 
    for( int i = 0; i < numCellArrays; i++ ) {
        vtkDataArray* emptyArray = vtkDataArray::CreateDataArray( dataTypeVtk );
        emptyArray->SetNumberOfComponents( numCellComponents );
        emptyArray->SetNumberOfTuples( numCellTuples );
        emptyArray->SetName( src->GetCellData()->GetArray(i)->GetName() );
        for( int j = 0; j < emptyArray->GetNumberOfComponents(); j++ ) {
            emptyArray->FillComponent( j, 0 );
        }
        this->GetCellData()->AddArray( emptyArray );
    }
    // Lastly make sure the scalars are set
    if( src->GetCellData()->GetScalars()) {
        this->GetCellData()->SetActiveScalars( src->GetCellData()->GetScalars()->GetName() );
    }

    // Now lets get the point data structure 
    int numPointArrays = src->GetPointData()->GetNumberOfArrays();
    int numPointTuples = 0;
    int numPointComponents = 0;
    if( numPointArrays > 0 ) {
        vtkDataArray* firstPointArray = src->GetPointData()->GetArray(0);
        numPointTuples = firstPointArray->GetNumberOfTuples();
        numPointComponents = firstPointArray->GetNumberOfComponents();
    }

    // And create new arrays of the appropriate type 
    for( int i = 0; i < numPointArrays; i++ ) {
        vtkDataArray* emptyArray = vtkDataArray::CreateDataArray( dataTypeVtk );
        emptyArray->SetNumberOfComponents( numPointComponents );
        emptyArray->SetNumberOfTuples( numPointTuples );
        emptyArray->SetName( src->GetPointData()->GetArray(i)->GetName());
        for( int j = 0; j < emptyArray->GetNumberOfComponents(); j++ ) {
            emptyArray->FillComponent( j, 0 );
        }
        this->GetPointData()->AddArray( emptyArray );
    }
    // Lastly make sure the scalars are set
    if( src->GetPointData()->GetScalars()) {
        this->GetPointData()->SetActiveScalars( src->GetPointData()->GetScalars()->GetName() );
    }
}


/*!
 *  Cast the pixel format to a different type. Only certain casts will be allowed,
 *  if you request an unsupported cast, no cast will be performed.
 */
void svkImageData::CastDataFormat( svkDcmHeader::DcmPixelDataFormat castToFormat )
{
    
    if ( castToFormat != svkDcmHeader::UNDEFINED ) {
        int dataTypeVtk; 

        // Lets grab the first array to make sure we identify the data type correctly
        vtkDataArray* firstPointArray = this->GetPointData()->GetArray(0);
        vtkDataArray* firstCellArray = this->GetCellData()->GetArray(0);

        int cellArrayType = -1;
        int pointArrayType = -1;

        // Get the point type
        if( firstPointArray != NULL ) {
            pointArrayType = firstPointArray->GetDataType(); 
        }

        // Get the cell type
        if( firstCellArray != NULL ) {
            cellArrayType = firstCellArray->GetDataType(); 
        }

        switch( castToFormat ) {

            case svkDcmHeader::UNSIGNED_INT_2:
                dataTypeVtk = VTK_UNSIGNED_SHORT;
                
                // We only accept certain casts
                if( pointArrayType != VTK_UNSIGNED_CHAR ) {
                    pointArrayType = -1;
                }

                /* Point and cell arrays are seperated because there could
                 * potentially be two different types.
                 */
                if( cellArrayType != VTK_UNSIGNED_CHAR ) {
                    cellArrayType = -1;
                }
                break;

            case svkDcmHeader::SIGNED_FLOAT_4:
                dataTypeVtk = VTK_FLOAT;

                // We only accept certain casts
                if( pointArrayType != VTK_UNSIGNED_CHAR && pointArrayType != VTK_UNSIGNED_SHORT  ) {
                    pointArrayType = -1;
                } 

                /* Point and cell arrays are seperated because there could
                 * potentially be two different types.
                 */
                if( cellArrayType != VTK_UNSIGNED_CHAR && cellArrayType != VTK_UNSIGNED_SHORT  ) {
                    cellArrayType = -1;
                }
                break;

            case svkDcmHeader::SIGNED_FLOAT_8:
                dataTypeVtk = VTK_DOUBLE;

                // We only accept certain casts
                if( pointArrayType != VTK_UNSIGNED_CHAR && pointArrayType != VTK_UNSIGNED_SHORT && pointArrayType != VTK_FLOAT) {
                    pointArrayType = -1;
                } 

                /* Point and cell arrays are seperated because there could
                 * potentially be two different types.
                 */
                if( cellArrayType != VTK_UNSIGNED_CHAR && cellArrayType != VTK_UNSIGNED_SHORT && cellArrayType != VTK_FLOAT ) {
                    cellArrayType = -1;
                }
                break;

            default:
                vtkErrorWithObjectMacro(this, "Can't perform requested downcast cast to: " << castToFormat);
                exit(1); 
        }

        // If the cell data cast is legitimate
        if ( cellArrayType != -1 ) {
            this->CastDataArrays(dataTypeVtk, this->GetCellData()); 
        }

        // If the point data cast is legitimate
        if ( pointArrayType != -1 ) {
            this->CastDataArrays(dataTypeVtk, this->GetPointData()); 
        }
    } else {
        cerr << "ERROR: You must define the format to which you wish to cast" << endl; 
    }
      
}


/*!
 *  CastDataArrays takes a vtkDataSetAttributes object (parent of vtkCell/PointData)
 *  depending on which you wish to operate on, then casts all data arrays in that
 *  object to the type declared in the template method.
 *
 */
void svkImageData::CastDataArrays( int dataTypeVtk, vtkDataSetAttributes* fieldData ) {

    vtkDataArray* source;
    vtkDataArray* target;
    string sourceName;
    string scalarName;

    // If the arrays have names, we copy them, otherwise don't
    bool hasName = 0;
    bool scalarHasName = 0;

    // See if scalars have been set so we can set them when were done
    if( fieldData->GetScalars() ) {
        if( fieldData->GetScalars()->GetName() ) {
            scalarHasName = 1;
            scalarName = fieldData->GetScalars()->GetName();
        }
    } 

    for( int i = 0; i < fieldData->GetNumberOfArrays(); i++ ) {
        /*
         *  We need to get the first array, this is because
         *  the api for adding data will only let us add
         *  to the end of the array list, this means
         *  that as we add onto the end, we pull from the top. 
         */
        source = fieldData->GetArray(0);

        // save the old name of the array
        if( fieldData->GetArray(0)->GetName() ) { 
            hasName = 1;
            sourceName = fieldData->GetArray(0)->GetName();
        }

        // In the case where the name is "NULL" this fails, so we rename it to trash
        fieldData->GetArray(0)->SetName("trash");

        // Create a new array to copy the cast data into
        target = vtkDataArray::CreateDataArray( dataTypeVtk);

        if( hasName ) { 
            target->SetName( sourceName.c_str() );
        } 

        // Lets copy the data via deep copy
        target->DeepCopy( source ); 

        // And remove the old array
        fieldData->RemoveArray("trash");

        // Lets add the new array
        fieldData->AddArray( target );

        target->Delete();
    }

    // Reset the appropriate scalars
    if( scalarHasName ) { 
        fieldData->SetActiveScalars( scalarName.c_str() );
    }
    this->SetScalarType( dataTypeVtk );
}


/*!
 *  Copy and cast using vtkImageData::CopyAndCastFrom( vtkImageData, int[6] ) 
 *  Also copies the dcos.
 */
void svkImageData::CopyAndCastFrom( vtkImageData* inData, int extent[6] )
{
    this->vtkImageData::CopyAndCastFrom( inData, extent );
    this->CopyDcos( inData );
} 
 

/*!
 *  Copy and cast using svkImageData::CopyAndCastFrom( vtkImageData, int[6] ) 
*/
void svkImageData::CopyAndCastFrom( vtkImageData* inData, int x0, int x1, int y0, int y1, int z0, int z1 )
{
    int extent[6] = {x0, x1, y0, y1, z0, z1};
    this->CopyAndCastFrom( inData, extent );
}


/*!
 *  Copies the structure: origin, extent, spacing, dcos... no data.
 */
void svkImageData::CopyStructure( vtkDataSet* ds )
{
    Superclass::CopyStructure( ds );
    this->CopyDcos( ds );
}


/*!
 * Copies the dcos of a src vtkDataObject. The object must be an
 * svkImageData or this will do nothing. It takes a vtkDataObject
 * and checks for its type, this is only for convenience.
 */
void svkImageData::CopyDcos( vtkDataObject* src )
{
    if( src->IsA("svkImageData" )) {
        double dcos[3][3];
        static_cast<svkImageData*>(src)->GetDcos(dcos);
        this->SetDcos( dcos );
    }
}


/*!
 *  DEPRECATED: USE DEEPCOPY!
 *
 *  This method copies a vtkImageData object by getting its origin, spacing,
 *  extent and copying its scalars from its point data, and all arrays from 
 *  its vtkCellData. This is only used to create ad hoc pipelines for svkImageData. 
 *  THIS METHOD IS FOR TEMOPORARY USE ONLY!!!!
 */
void svkImageData::CopyVtkImage( vtkImageData* sourceImage, double sourceDcos[3][3] )
{
    if( sourceImage != NULL && sourceDcos != NULL ) {
        this->ShallowCopy( sourceImage );
        this->SetDcos( sourceDcos );
    }
}


/*!
 *  Get a point based on the point ID. Since ImageData does not store points, it 
 *  is calculated.
 */
double* svkImageData::GetPoint (vtkIdType ptId)
{ 
    static double x[3];
    int i, loc[3];
    const double *origin = this->Origin;
    const double *spacing = this->Spacing;
    const int* extent = this->Extent;

    vtkIdType dims[3];
    dims[0] = extent[1] - extent[0] + 1;
    dims[1] = extent[3] - extent[2] + 1;
    dims[2] = extent[5] - extent[4] + 1;

    x[0] = x[1] = x[2] = 0.0;
    if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0) {
        vtkErrorMacro("Requesting a point from an empty image.");
        return x;
    }

    switch (this->DataDescription) {
        case VTK_EMPTY:
            return x;
        case VTK_SINGLE_POINT:
            loc[0] = loc[1] = loc[2] = 0;
            break;
        case VTK_X_LINE:
            loc[1] = loc[2] = 0;
            loc[0] = ptId;
            break;
        case VTK_Y_LINE:
            loc[0] = loc[2] = 0;
            loc[1] = ptId;
            break;
        case VTK_Z_LINE:
            loc[0] = loc[1] = 0;
            loc[2] = ptId;
            break;
        case VTK_XY_PLANE:
            loc[2] = 0;
            loc[0] = ptId % dims[0];
            loc[1] = ptId / dims[0];
            break;
        case VTK_YZ_PLANE:
            loc[0] = 0;
            loc[1] = ptId % dims[1];
            loc[2] = ptId / dims[1];
            break;
        case VTK_XZ_PLANE:
            loc[1] = 0;
            loc[0] = ptId % dims[0];
            loc[2] = ptId / dims[0];
            break;
        case VTK_XYZ_GRID:
            loc[0] = ptId % dims[0];
            loc[1] = (ptId / dims[0]) % dims[1];
            loc[2] = ptId / (dims[0]*dims[1]);
        break;
    }

    // Here is our oblique modification:
    for (i=0; i<3; i++) {
        x[i] = origin[i];
        for( int j=0; j<3; j++) {
            x[i] += (loc[j]+extent[j*2]) * spacing[j] * dcos[j][i];
        }
    }

    return x;
}


/*!
 *
 */
vtkCell* svkImageData::GetCell (vtkIdType cellId)
{
    // Source here is pulled from vtkImageData, has been modified, 
    // and should work, but needs to be checked
    vtkCell *cell = NULL;
    int loc[3];
    vtkIdType idx, npts;
    int iMin, iMax, jMin, jMax, kMin, kMax;
    double x[3];
    const double *origin = this->Origin;
    const double *spacing = this->Spacing;
    const int* extent = this->Extent;

    // Use vtkIdType to avoid overflow on large images
    vtkIdType dims[3];
    dims[0] = extent[1] - extent[0] + 1;
    dims[1] = extent[3] - extent[2] + 1;
    dims[2] = extent[5] - extent[4] + 1;

  vtkIdType d01 = dims[0]*dims[1];

  iMin = iMax = jMin = jMax = kMin = kMax = 0;

  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
    vtkErrorMacro("Requesting a cell from an empty image.");
    return NULL;
    }

  switch (this->DataDescription)
    {
    case VTK_EMPTY:
      //cell = this->EmptyCell;
      return NULL;

    case VTK_SINGLE_POINT: // cellId can only be = 0
      cell = this->Vertex;
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (dims[0]-1);
      jMax = jMin + 1;
      cell = this->Pixel;
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (dims[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (dims[1]-1);
      kMax = kMin + 1;
      cell = this->Pixel;
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (dims[0]-1);
      kMax = kMin + 1;
      cell = this->Pixel;
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (dims[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (dims[0] - 1)) % (dims[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((dims[0] - 1) * (dims[1] - 1));
      kMax = kMin + 1;
      cell = this->Voxel;
      break;
    }

    // Extract point coordinates and point ids
    // Ids are relative to extent min.
    npts = 0;
    for (loc[2]=kMin; loc[2]<=kMax; loc[2]++) {
        for (loc[1]=jMin; loc[1]<=jMax; loc[1]++) {
            for (loc[0]=iMin; loc[0]<=iMax; loc[0]++) {
                for( int i = 0; i <3; i++ ) {
                    x[i] = origin[i];
                    for( int j=0; j<3; j++) {
                        x[i] += (loc[j]+extent[j*2]) * spacing[j] * dcos[j][i];
                    }
                }

                idx = loc[0] + loc[1]*dims[0] + loc[2]*d01;
                cell->PointIds->SetId(npts,idx);
                cell->Points->SetPoint(npts++,x);
            }
        }
    }

  return cell;
}


/*!
 *  Get a cell based on the point ID. Since ImageData does not store cells, it 
 *  is calculated.
 */
void svkImageData::GetCell (vtkIdType cellId, vtkGenericCell *cell)
{
  vtkIdType npts, idx;
  int loc[3];
  int iMin, iMax, jMin, jMax, kMin, kMax;
  const double *origin = this->Origin;
  const double *spacing = this->Spacing;
  double x[3];
  const int* extent = this->Extent;

  vtkIdType dims[3];
  dims[0] = extent[1] - extent[0] + 1;
  dims[1] = extent[3] - extent[2] + 1;
  dims[2] = extent[5] - extent[4] + 1;
  vtkIdType d01 = dims[0]*dims[1];

  iMin = iMax = jMin = jMax = kMin = kMax = 0;

  if (dims[0] == 0 || dims[1] == 0 || dims[2] == 0)
    {
    vtkErrorMacro("Requesting a cell from an empty image.");
    cell->SetCellTypeToEmptyCell();
    return;
    }

  switch (this->DataDescription)
    {
    case VTK_EMPTY:
      cell->SetCellTypeToEmptyCell();
      return;

    case VTK_SINGLE_POINT: // cellId can only be = 0
      cell->SetCellTypeToVertex();
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (dims[0]-1);
      jMax = jMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (dims[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (dims[1]-1);
      kMax = kMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (dims[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (dims[0]-1);
      kMax = kMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (dims[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (dims[0] - 1)) % (dims[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((dims[0] - 1) * (dims[1] - 1));
      kMax = kMin + 1;
      cell->SetCellTypeToVoxel();
      break;
    }

  // Extract point coordinates and point ids
    npts = 0;
    for (loc[2]=kMin; loc[2]<=kMax; loc[2]++) {
        for (loc[1]=jMin; loc[1]<=jMax; loc[1]++) {
            for (loc[0]=iMin; loc[0]<=iMax; loc[0]++) {

                // Loop over x-y-z coordinates
                for( int i = 0; i <3; i++ ) {
                    // Start at the origin
                    x[i] = origin[i];
                    for( int j=0; j<3; j++) {
                        // Add the components for dim i
                        x[i] += (loc[j]+extent[j*2]) * spacing[j] * dcos[j][i];
                    }
                }

                idx = loc[0] + loc[1]*dims[0] + loc[2]*d01;
                cell->PointIds->SetId(npts,idx);
                cell->Points->SetPoint(npts++,x);
            }
        }
    }

}


/*!
 *  NOT YET IMPLEMENTED-- empty method
 */
void svkImageData::GetCellBounds (vtkIdType cellId, double bounds[6])
{
    vtkGenericCell* cell = vtkGenericCell::New();
    this->GetCell(cellId, cell);
    cell->GetBounds( bounds );
    cell->Delete();
}


/*!
 *  NOT YET IMPLEMENTED-- returns 0 
 */
vtkIdType svkImageData::FindPoint (double x, double y, double z)
{
    return 0;
}


/*!
 *  NOT YET IMPLEMENTED-- returns 0 
 */
vtkIdType svkImageData::FindPoint (double x[3])
{
    return 0;
}


/*!
 *  NOT YET IMPLEMENTED-- returns 0 
 */
vtkIdType svkImageData::FindCell (double x[3], vtkCell *cell, vtkIdType cellId, double tol2, int &subId, double pcoords[3], double *weights)
{
    return 0;
}


/*!
 *  NOT YET IMPLEMENTED-- returns 0 
 */
vtkIdType svkImageData::FindCell (double x[3], vtkCell *cell, vtkGenericCell *gencell, vtkIdType cellId, double tol2, int &subId, double pcoords[3], double *weights)
{
    return 0;
}


/*!
 *  NOT YET IMPLEMENTED-- returns NULL 
 */
vtkCell* svkImageData::FindAndGetCell (double x[3], vtkCell *cell, vtkIdType cellId, double tol2, int &subId, double pcoords[3], double *weights)
{
    return NULL;
}


/*!
 *  NOT YET IMPLEMENTED-- returns 0 
 */
int svkImageData::GetCellType (vtkIdType cellId)
{
    return 0;
}


/*!
 *  NOT YET IMPLEMENTED-- empty method 
 */
void svkImageData::GetCellPoints (vtkIdType cellId, vtkIdList *ptIds)
{

}


/*!
 *  NOT YET IMPLEMENTED-- empty method 
 */
void svkImageData::GetPointCells (vtkIdType ptId, vtkIdList *cellIds)
{

}

/*!
 *  Computes the oblique bounds. 
 */
void svkImageData::ComputeBounds ()
{
    // This method was modified from the origin in vtkImageData
    const double *origin = this->Origin;
    const double *spacing = this->Spacing;
    const int* extent = this->Extent;

    if ( extent[0] > extent[1] ||
        extent[2] > extent[3] ||
        extent[4] > extent[5] ) {
        vtkMath::UninitializeBounds(this->Bounds);
        return;
    }
  
    double vertexPosition[3];  
    int vertexIndex[3];


    // Now we need to calculate the [xMin, xMax, yMin, yMax, zMin, zMax] boundry
    this->Bounds[0] = VTK_DOUBLE_MAX;
    this->Bounds[1] = VTK_DOUBLE_MIN;
    this->Bounds[2] = VTK_DOUBLE_MAX;
    this->Bounds[3] = VTK_DOUBLE_MIN;
    this->Bounds[4] = VTK_DOUBLE_MAX;
    this->Bounds[5] = VTK_DOUBLE_MIN;

    for( int i = 0; i < 2; i++ ) {
        vertexIndex[0] = i;
        for( int j = 0; j < 2; j++ ) {
            vertexIndex[1] = j;
            for( int k = 0; k < 2; k++ ) {
                vertexIndex[2] = k;
                for( int x = 0; x < 3; x++ ) {
                    vertexPosition[x] = origin[x]; 
                    for( int u = 0; u<3; u++ ) {
                        vertexPosition[x] += dcos[u][x]*(extent[2*u + vertexIndex[u] ] * spacing[u]); 
                    } 
                    if( vertexPosition[x] < this->Bounds[2*x] ) {
                        this->Bounds[2*x] = vertexPosition[x]; 
                    } 
                    if( vertexPosition[x] > this->Bounds[2*x+1] ) {
                        this->Bounds[2*x+1] = vertexPosition[x]; 
                    }
                }
            }
        } 
    }
}


/*!
 *  NOT YET IMPLEMENTED-- returns 0 
 */
int svkImageData::ComputeStructuredCoordinates (double x[3], int ijk[3], double pcoords[3])
{
    return 0;
}


/*!
 *  NOT YET IMPLEMENTED-- empty method 
 */
void svkImageData::GetVoxelGradient (int i, int j, int k, vtkDataArray *s, vtkDataArray *g)
{

}


/*!
 *  NOT YET IMPLEMENTED-- empty method 
 */
void svkImageData::GetPointGradient (int i, int j, int k, vtkDataArray *s, double g[3])
{

}


/*!
 *  NOT YET IMPLEMENTED-- empty method 
 */
void svkImageData::SetAxisUpdateExtent (int axis, int min, int max)
{

}


/*!
 *  NOT YET IMPLEMENTED-- empty method 
 */
void svkImageData::GetAxisUpdateExtent (int axis, int &min, int &max)
{

}


/*!
 *  Pure setter, copies the directional cosine matrix.
 */
void svkImageData::SetDcos( double dcos[][3] )
{
    memcpy( this->dcos, dcos, sizeof(double) * 9 );
    this->Modified();
}


/*!
 *  Pure getter, returns the directional cosine matrix.
 */
void svkImageData::GetDcos( double dcos [][3] )
{
    memcpy( dcos, this->dcos, sizeof(double) * 9 );
}



/*!
 *  Pure setter method: this->x = x.
 *
 *  \param dcmHeader the header you wish to set for this svkImageData object
 */
void svkImageData::SetDcmHeader(svkDcmHeader* dcmHeader)
{
    this->dcmHeader = dcmHeader;
    this->Modified();
}


/*!
 *  Pure getter method: returns header object.
 *
 *  \return the header for this svkImageData object
 */
svkDcmHeader* svkImageData::GetDcmHeader()
{
    return this->dcmHeader;
}


/*!
 * Pure setter method: this->x = x.
 *
 *  \param provenance the provenance object you wish to set for this svkImageData object
 */
void svkImageData::SetProvenance(svkProvenance* provenance)
{
    this->provenance = provenance;
    this->Modified();
}


/*!
 *  Pure getter method: returns provenance object.
 *
 *  \return provenance the provenance for this svkImageData object
 */
svkProvenance* svkImageData::GetProvenance()
{
    return this->provenance;
}


/*!
 *  Compute the plotID from the x, y, z index
 *
 *  \param indexX the x index of the voxel who's id you want
 *  \param indexY the y index of the voxel who's id you want
 *  \param indexY the z index of the voxel who's id you want
 *
 *  \return the id of the plot at (indexX, indexY, indexZ)
 */
int svkImageData::GetIDFromIndex(int indexX, int indexY, int indexZ)
{
    int numVoxels[3];

    this->GetNumberOfVoxels(numVoxels);

    int numPtsX = numVoxels[0];
    int numPtsY = numVoxels[1];
    int numPtsZ = numVoxels[2];

    return (numPtsX * numPtsY) * indexZ +
                      numPtsX  * indexY +
                                 indexX;
}


/*!
 *  Compute the x, y indices from the voxelID.
 *
 *  \param voxelID the id of the voxel you wish to know the indecies of
 *
 *  \param index pointer to the integer indices
 *
 */
void svkImageData::GetIndexFromID(int voxelID, int* index)
{
    this->GetIndexFromID(voxelID, &index[0], &index[1], &index[2]);
}


/*!
 *  Compute the x, y, z indices from the voxelID.
 *
 *  \param voxelID the id of the voxel you wish to know the indecies of
 *
 *  \param indexX pointer to the integer you wish to set to be the x index of the given ID
 *  \param indexY pointer to the integer you wish to set to be the y index of the given ID
 *  \param indexZ pointer to the integer you wish to set to be the z index of the given ID
 *
 */
void svkImageData::GetIndexFromID(int voxelID, int* indexX, int* indexY, int* indexZ)
{
    int numVoxels[3];

    this->GetNumberOfVoxels(numVoxels);

    int numPtsX = numVoxels[0];
    int numPtsY = numVoxels[1];
    int numPtsZ = numVoxels[2];

    *indexZ = static_cast <int> ( voxelID/(numPtsX * numPtsY) );
    *indexX = static_cast <int> ( voxelID%numPtsX );
    *indexY = static_cast <int> ((voxelID - (*indexZ * numPtsX * numPtsY))/numPtsX);

    vtkDebugMacro( << this->GetClassName() << ":: GetIndexFromID: "
        << voxelID << " -> "  << *indexX << " " << *indexY << " " << *indexZ
    );

}


/*!
 *  Compute the row, column, slice index for a given LPS coordinate
 *
 *  \param L
 *  \param P
 *  \param S
 *
 *  \return the x,y,z index of the voxel at that LPS position. If the position
 *          is outside the dataset, the closests voxel will be selected.
 */
void svkImageData::GetIndexFromPosition(double posLPS[3], int* index)
{

    double origin[3];
    this->GetDcmHeader()->GetOrigin(origin);

    double pixelSpacing[3];
    this->GetDcmHeader()->GetPixelSpacing(pixelSpacing);

    // Origin is center of tlc voxel, so to calculate the actual oblique distance
    // to the target position it's necessary to back off by 1/2 voxel:
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            origin[i] -= (pixelSpacing[j]/2) * dcos[j][i];
        }
    }

    double rowNormal[3];
    this->GetDataBasis(rowNormal, svkImageData::ROW );
    double columnNormal[3];
    this->GetDataBasis(columnNormal, svkImageData::COLUMN );
    double sliceNormal[3];
    this->GetDataBasis(sliceNormal, svkImageData::SLICE );

    index[0] = (int)floor((vtkMath::Dot(posLPS, rowNormal)  - vtkMath::Dot(origin, rowNormal))/pixelSpacing[0]);
    index[1] = (int)floor((vtkMath::Dot(posLPS, columnNormal) - vtkMath::Dot(origin, columnNormal))/pixelSpacing[1]);
    index[2] = (int)floor((vtkMath::Dot(posLPS, sliceNormal) - vtkMath::Dot(origin, sliceNormal) )/pixelSpacing[2]);
    for (int i = 0; i < 3; i++) { 
        if( index[i] >= this->GetDimensions()[i] ) {
            index[i] = this->GetDimensions()[i]-1;
        } else if ( index[i] < 0 ) {
            index[i] = 0;
        }
    }
}


/*! 
 *  Returns an vtkActorCollection that represents a topological feature
 *  of the data set acquisition.  For example, the svkMRSImageData
 *  sub-class has 3 topological constructs (grid, selection box, sat bands)
 *  that represent acquisition properties.  These are represented as
 *  3 separate vtkActorCollections at actorIndex 0, 1 and 2 to represent
 *  the acquisition grid, volume selection box, and sat bands.
 *  Topologies are managed by index, and are generated by the svkImageTopologyGenerator
 *  object, which should be instantiated in any subclass's constructor. Also no copy of
 *  the topologies are kept internally so subclasses should delete them, but do not need
 *  to register with them when they acquire them.
 *
 *  \param actorIndex the index of the topology you wish to get
 *
 *  \return a vtkActorCollection containing all of the individual actors that represent the topology
 */ 
vtkActorCollection* svkImageData::GetTopoActorCollection(int actorIndex)
{
    vtkActorCollection* topology = NULL;
    if( topoGenerator != NULL ) {
        topology = topoGenerator->GetTopoActorCollection( this, actorIndex );
    }
    return topology;
}


//! Getter of the data range variable
void svkImageData::GetDataRange( double range[2], int component )
{
    if ( this->WasModified() ) {
        this->UpdateSvkParams();
        this->UpdateRange( component );
    }

    range[0] = this->range[component][0];
    range[1] = this->range[component][1];
}


//! Setter of the data range variable
void svkImageData::SetDataRange( double range[2], int component )
{

    this->range[component][0] = range[0];
    this->range[component][1] = range[1];
    this->Modified();

}


/*!
 *  Compute LPS coordinate for a given x,y,z index
 *
 *  \param x
 *  \param y
 *  \param z
 *
 *  \return the L,P,S coordinate of the center of the voxel at
 *  that xyz index.
 */
void svkImageData::GetPositionFromIndex(int* index, double* posLPS)
{

    double origin[3];
    this->GetDcmHeader()->GetOrigin(origin);
    double pixelSpacing[3]; 
    this->GetDcmHeader()->GetPixelSpacing(pixelSpacing);
 
    double dcos[3][3];
    this->GetDcos(dcos);

    for (int i = 0; i < 3; i++) {
        posLPS[i] = origin[i];  
        for (int j = 0; j < 3; j++) { 
            posLPS[i] += (pixelSpacing[i] * index[j]) * dcos[j][i];
        }
    }
}


/*!
 *  This method will get the origin of a given slice, for a given orientation. Default uses the data's 
 *  orientation.
 */
void svkImageData::GetSliceOrigin(int slice, double* sliceOrigin, svkDcmHeader::Orientation sliceOrientation )
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    int index = this->GetOrientationIndex( sliceOrientation );
    double normal[3];
    this->GetSliceNormal( normal, sliceOrientation );
    sliceOrigin[0] = this->GetOrigin()[0] + normal[0]*this->GetSpacing()[index]*slice;
    sliceOrigin[1] = this->GetOrigin()[1] + normal[1]*this->GetSpacing()[index]*slice;
    sliceOrigin[2] = this->GetOrigin()[2] + normal[2]*this->GetSpacing()[index]*slice;

}


/*!
 *  This method will get the center of a given slice, for a given orientation. Default uses the data's 
 *  orientation.
 */
void svkImageData::GetSliceCenter(int slice, double* sliceCenter, svkDcmHeader::Orientation sliceOrientation )
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    double origin[3] = {0,0,0};
    this->GetDcmHeader()->GetOrigin(origin);
    double pixelSpacing[3] = {0,0,0};
    this->GetDcmHeader()->GetPixelSpacing(pixelSpacing);
    float index[3] = {0,0,0};

    int numVoxels[3] = {0,0,0};
    numVoxels[0] = this->GetDcmHeader()->GetIntValue("Columns");
    numVoxels[1] = this->GetDcmHeader()->GetIntValue("Rows");
    numVoxels[2] = this->GetDcmHeader()->GetNumberOfSlices(); 

    double dcos[3][3];
    this->GetDcos(dcos);
    for (int i = 0; i < 3; i++) {
        index[i] = (numVoxels[i]-1)/2.0;
    }
    int orientationIndex = this->GetOrientationIndex( sliceOrientation );
    index[orientationIndex] = slice;

    for (int i = 0; i < 3; i++) {
        sliceCenter[i] = origin[i];  
        for (int j = 0; j < 3; j++) { 
            sliceCenter[i] += (pixelSpacing[i] * index[j]) * dcos[j][i];
        }
    }

}


/*!
 * Gets the normal vector to a given orientation.
 */
void svkImageData::GetSliceNormal(double* normal, svkDcmHeader::Orientation sliceOrientation )
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;

    switch ( this->GetDcmHeader()->GetOrientationType() ) {
        case svkDcmHeader::AXIAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    normal[0] = dcos[2][0];  
                    normal[1] = dcos[2][1];  
                    normal[2] = dcos[2][2];  
                    break;
                case svkDcmHeader::CORONAL:
                    normal[0] = dcos[1][0];  
                    normal[1] = dcos[1][1];  
                    normal[2] = dcos[1][2];  
                    break;
                case svkDcmHeader::SAGITTAL:
                    normal[0] = dcos[0][0];  
                    normal[1] = dcos[0][1];  
                    normal[2] = dcos[0][2];  
                    break;
            }
            break;
        case svkDcmHeader::CORONAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    normal[0] = dcos[1][0];  
                    normal[1] = dcos[1][1];  
                    normal[2] = dcos[1][2];  
                    break;
                case svkDcmHeader::CORONAL:
                    normal[0] = dcos[2][0];  
                    normal[1] = dcos[2][1];  
                    normal[2] = dcos[2][2];  
                    break;
                case svkDcmHeader::SAGITTAL:
                    normal[0] = dcos[0][0];  
                    normal[1] = dcos[0][1];  
                    normal[2] = dcos[0][2];  
                    break;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    normal[0] = dcos[1][0];  
                    normal[1] = dcos[1][1];  
                    normal[2] = dcos[1][2];  
                    break;
                case svkDcmHeader::CORONAL:
                    normal[0] = dcos[0][0];  
                    normal[1] = dcos[0][1];  
                    normal[2] = dcos[0][2];  
                    break;
                case svkDcmHeader::SAGITTAL:
                    normal[0] = dcos[2][0];  
                    normal[1] = dcos[2][1];  
                    normal[2] = dcos[2][2];  
                    break;
            }
            break;
    } 

}


/*!
 *  Calculates the closests slice to a given LPS coordinate for a given orientation.
 */
int svkImageData::GetClosestSlice(double* posLPS, svkDcmHeader::Orientation sliceOrientation )
{

    double* origin  = new double[3]; 
    this->GetDcmHeader()->GetOrigin(origin, 0); 
    double* spacing = this->GetSpacing();

    int slice = this->FindMatchingSlice( posLPS, sliceOrientation, origin, spacing ); 
    
    delete[] origin; 
    
    return slice; 
}


/*!
 *  This projects a posLPS onto a normal vector through the real world origin.  This is a common reference vector for 
 *  both the MRI and MRS data.  The projection is normalized by the relevant data slice spacing to get the nearest 
 *  slice.
 */
int svkImageData::FindMatchingSlice( double* posLPS, svkDcmHeader::Orientation sliceOrientation, double* origin, double* spacing ) 
{

    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;

    double normal[3];
    this->GetSliceNormal( normal, sliceOrientation );

    //  Project posLPS onto the normal vector through the 
    //  origin of world coordinate space (this is the same 
    //  reference for both data sets (e.g MRI and MRS). 
    double imageCenter = vtkMath::Dot( posLPS, normal ); 

    //  Which index (0,1,2) represents the 
    //  coordinate specified by sliceOrientation
    int index = this->GetOrientationIndex( sliceOrientation );

    //  Origin here is the center of the toplc slice (DICOM origin). 
    //  This is fractional and needs to be rounded to get the integer 
    //  slice value. 
    double exactSlice = ( imageCenter - vtkMath::Dot( origin, normal ) )/(spacing[index] );

    //  floor 0.5 is a nint implementation to get the nearest int slice.  
    return (int) floor( exactSlice + 0.5 );
}


/*!
 *  Determins the number of slices for a given orientation.
 */
int svkImageData::GetNumberOfSlices( svkDcmHeader::Orientation sliceOrientation)
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    int numSlices;
    switch ( this->GetDcmHeader()->GetOrientationType()) {
        case svkDcmHeader::AXIAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    numSlices = this->GetDimensions()[2];
                    break;
                case svkDcmHeader::CORONAL:
                    numSlices = this->GetDimensions()[1];
                    break;
                case svkDcmHeader::SAGITTAL:
                    numSlices = this->GetDimensions()[0];
                    break;
            }
            break;
        case svkDcmHeader::CORONAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    numSlices = this->GetDimensions()[1];
                    break;
                case svkDcmHeader::CORONAL:
                    numSlices = this->GetDimensions()[2];
                    break;
                case svkDcmHeader::SAGITTAL:
                    numSlices = this->GetDimensions()[0];
                    break;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    numSlices = this->GetDimensions()[1];
                    break;
                case svkDcmHeader::CORONAL:
                    numSlices = this->GetDimensions()[0];
                    break;
                case svkDcmHeader::SAGITTAL:
                    numSlices = this->GetDimensions()[2];
                    break;
            }
            break;
    }
    return numSlices;
}


/*!
 *  Gets the first slice in a given orientation.
 */
int svkImageData::GetFirstSlice( svkDcmHeader::Orientation sliceOrientation)
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    int firstSlice;
    switch ( this->GetDcmHeader()->GetOrientationType()) {
        case svkDcmHeader::AXIAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    firstSlice = this->GetExtent()[4];
                    break;
                case svkDcmHeader::CORONAL:
                    firstSlice = this->GetExtent()[2];
                    break;
                case svkDcmHeader::SAGITTAL:
                    firstSlice = this->GetExtent()[0];
                    break;
            }
            break;
        case svkDcmHeader::CORONAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    firstSlice = this->GetExtent()[2];
                    break;
                case svkDcmHeader::CORONAL:
                    firstSlice = this->GetExtent()[4];
                    break;
                case svkDcmHeader::SAGITTAL:
                    firstSlice = this->GetExtent()[0];
                    break;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    firstSlice = this->GetExtent()[2];
                    break;
                case svkDcmHeader::CORONAL:
                    firstSlice = this->GetExtent()[0];
                    break;
                case svkDcmHeader::SAGITTAL:
                    firstSlice = this->GetExtent()[4];
                    break;
            }
            break;
    }
    return firstSlice;
}


/*!
 *  Gets the last slice in a given orientation.
 */
int svkImageData::GetLastSlice( svkDcmHeader::Orientation sliceOrientation)
{
    sliceOrientation = (sliceOrientation == svkDcmHeader::UNKNOWN_ORIENTATION ) ? 
                                this->GetDcmHeader()->GetOrientationType() : sliceOrientation;
    int lastSlice;
    switch ( this->GetDcmHeader()->GetOrientationType()) {
        case svkDcmHeader::AXIAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    lastSlice = this->GetExtent()[5];
                    break;
                case svkDcmHeader::CORONAL:
                    lastSlice = this->GetExtent()[3];
                    break;
                case svkDcmHeader::SAGITTAL:
                    lastSlice = this->GetExtent()[1];
                    break;
            }
            break;
        case svkDcmHeader::CORONAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    lastSlice = this->GetExtent()[3];
                    break;
                case svkDcmHeader::CORONAL:
                    lastSlice = this->GetExtent()[5];
                    break;
                case svkDcmHeader::SAGITTAL:
                    lastSlice = this->GetExtent()[1];
                    break;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            switch ( sliceOrientation ) {
                case svkDcmHeader::AXIAL:
                    lastSlice = this->GetExtent()[3];
                    break;
                case svkDcmHeader::CORONAL:
                    lastSlice = this->GetExtent()[1];
                    break;
                case svkDcmHeader::SAGITTAL:
                    lastSlice = this->GetExtent()[5];
                    break;
            }
            break;
    }
    return lastSlice;
}


/*!
 *  Gets the data basis. Some calculations are easier conceptually if you use
 *  the different vectors of the dcos. The basis are ROW, COLUMN, SLICE,
 *  LR, PA, SI.
 */
void svkImageData::GetDataBasis( double basisVector[3], DataBasis basis )
{
    switch( basis ) {
        case ROW:
            basisVector[0] = dcos[0][0];
            basisVector[1] = dcos[0][1];
            basisVector[2] = dcos[0][2];
            break;
        case COLUMN:
            basisVector[0] = dcos[1][0];
            basisVector[1] = dcos[1][1];
            basisVector[2] = dcos[1][2];
            break;
        case SLICE:
            basisVector[0] = dcos[2][0];
            basisVector[1] = dcos[2][1];
            basisVector[2] = dcos[2][2];
            break;
        case LR:
            basisVector[0] = dcos[0][0];
            basisVector[1] = dcos[1][0];
            basisVector[2] = dcos[2][0];
            break;
        case PA:
            basisVector[0] = dcos[0][1];
            basisVector[1] = dcos[1][1];
            basisVector[2] = dcos[2][1];
            break;
        case SI:
            basisVector[0] = dcos[0][2];
            basisVector[1] = dcos[1][2];
            basisVector[2] = dcos[2][2];
            break;

    }
}


/*!
 *  The orientation index is the index of a dataset for a given orientation.
 *  It represents whether a given orientation progresses in the row, column, or slice
 *  dimension.
 *
 *  \param orientation the orientation whose index you wish to get
 *  \return the index, 0 is rows, 1 is columns, 2 is slices.
 */
int svkImageData::GetOrientationIndex( svkDcmHeader::Orientation orientation )
{
    int index;
    switch( this->GetDcmHeader()->GetOrientationType() ) {
        case svkDcmHeader::AXIAL:
            switch( orientation ) {
                case svkDcmHeader::AXIAL:
                    index = 2;
                    break;
                case svkDcmHeader::CORONAL:
                    index = 1;
                    break;
                case svkDcmHeader::SAGITTAL:
                    index = 0;
                    break;
            }
            break;
        case svkDcmHeader::CORONAL:
            switch( orientation ) {
                case svkDcmHeader::AXIAL:
                    index = 1;
                    break;
                case svkDcmHeader::CORONAL:
                    index = 2;
                    break;
                case svkDcmHeader::SAGITTAL:
                    index = 0;
                    break;
            }
            break;
        case svkDcmHeader::SAGITTAL:
            switch( orientation ) {
                case svkDcmHeader::AXIAL:
                    index = 1;
                    break;
                case svkDcmHeader::CORONAL:
                    index = 0;
                    break;
                case svkDcmHeader::SAGITTAL:
                    index = 2;
                    break;
            }
            break;
    }
    return index;
    
}


/*!
 *  Compute center of the image data volume 
 *
 *
 *  \return the L,P,S coordinate of the center of the image 
 */
void svkImageData::GetImageCenter( double* posLPS)
{

    double origin[3] = {0,0,0};
    this->GetDcmHeader()->GetOrigin(origin);
    double pixelSpacing[3] = {0,0,0}; 
    this->GetDcmHeader()->GetPixelSpacing(pixelSpacing);
    float index[3] = {0,0,0};

    int numVoxels[3] = {0,0,0};
    numVoxels[0] = this->GetDcmHeader()->GetIntValue("Columns");
    numVoxels[1] = this->GetDcmHeader()->GetIntValue("Rows");
    numVoxels[2] = this->GetDcmHeader()->GetNumberOfSlices(); 

    double dcos[3][3];
    this->GetDcos(dcos);
    for (int i = 0; i < 3; i++) {
        index[i] = (numVoxels[i]-1)/2.0;
    }
    for (int i = 0; i < 3; i++) {
        posLPS[i] = origin[i];  
        for (int j = 0; j < 3; j++) { 
            posLPS[i] += (pixelSpacing[i] * index[j]) * dcos[j][i];
        }
    }
}


/*! 
 *  Updates svk parameters.
 */     
void svkImageData::UpdateSvkParams()
{
    this->lastUpdateTime = this->GetMTime();
}


/*! 
 *
 */     
void svkImageData::UpdateRange( int component )
{
}


/*! 
 *
 */     
bool svkImageData::WasModified()
{
    if (this->GetMTime() > lastUpdateTime) {
        return 1;
    } else {
       return 0;
    }   
}

