/*
 *  Copyright © 2009-2017 The Regents of the University of California.
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

/*
 * THIS CLASS NEEDS THE VTK HEADER SINCE SOURCE WAS DIRECTLY TAKEN FROM vtkImage/OpenGLImageActor
 */

#include <svkOpenGLOrientedImageActor.h>
#include </usr/include/vtk/vtkGraphicsFactory.h>
#include </usr/include/vtk/vtkImageData.h>
#include </usr/include/vtk/vtkMath.h>
#include </usr/include/vtk/vtkRenderer.h>
#include </usr/include/vtk/vtkTransform.h>


using namespace svk;


//vtkCxxRevisionMacro(svkOpenGLOrientedImageActor, "$Revision$");
vtkStandardNewMacro(svkOpenGLOrientedImageActor);


/*!
 * Constructor.
 */
svkOpenGLOrientedImageActor::svkOpenGLOrientedImageActor()
{
}


/*!
 * Destructor.
 */
svkOpenGLOrientedImageActor::~svkOpenGLOrientedImageActor()
{
}

//! Get the bounds for this Volume as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *svkOpenGLOrientedImageActor::GetDisplayBounds()
{
    if( !this->Input ) {
        return this->DisplayBounds;
    }
    this->Input->UpdateInformation();
    double *spacing = this->Input->GetSpacing();
    double *origin = this->Input->GetOrigin();

    // if the display extent has not been set, then compute one
    int *wExtent = this->Input->GetWholeExtent();
    if (this->DisplayExtent[0] == -1) {
        this->ComputedDisplayExtent[0] = wExtent[0];
        this->ComputedDisplayExtent[1] = wExtent[1];
        this->ComputedDisplayExtent[2] = wExtent[2];
        this->ComputedDisplayExtent[3] = wExtent[3];
        this->ComputedDisplayExtent[4] = wExtent[4];
        this->ComputedDisplayExtent[5] = wExtent[4];
    }
    double corner[3];     
    int maxMin[3];
    double dcos[3][3];
    static_cast<svkImageData*>(this->Input)->GetDcos( dcos );
    /*
    cout<<" dcos is : "<<endl;
    cout<<"| "<<dcos[0][0]<<" "<<dcos[0][1]<<" "<<dcos[0][2]<<" |"<<endl;
    cout<<"| "<<dcos[1][0]<<" "<<dcos[1][1]<<" "<<dcos[1][2]<<" |"<<endl;
    cout<<"| "<<dcos[2][0]<<" "<<dcos[2][1]<<" "<<dcos[2][2]<<" |"<<endl;
    */
    this->DisplayBounds[0] = this->DisplayBounds[2] = this->DisplayBounds[4] = VTK_DOUBLE_MAX;
    this->DisplayBounds[1] = this->DisplayBounds[3] = this->DisplayBounds[5] = -VTK_DOUBLE_MAX;
    for( int i = 0; i < 2; i++ ) {
        maxMin[0] = i;
        for( int j = 0; j < 2; j++ ) {
            maxMin[1] = j;
            for( int k = 0; k < 2; k++ ) {
                maxMin[2] = k;
                for( int x = 0; x < 3; x++ ) {
                    corner[x] = origin[x];
                    for( int u = 0; u<3; u++ ) {
                        corner[x] += dcos[u][x]*(ComputedDisplayExtent[2*u + maxMin[u] ] * spacing[u]);
                    }
                    if( corner[x] < this->DisplayBounds[2*x] ) {
                        this->DisplayBounds[2*x] = corner[x];
                    } else if( corner[x] > this->DisplayBounds[2*x+1] ) {
                        this->DisplayBounds[2*x+1] = corner[x];
                    }
                }
            }
        }
    }
    /*
    cout<<" Display Bounds:"<<endl;
    cout<<"| "<<DisplayBounds[0]<<" "<<DisplayBounds[2]<<" "<<DisplayBounds[4]<<" |"<<endl;
    cout<<"| "<<DisplayBounds[1]<<" "<<DisplayBounds[3]<<" "<<DisplayBounds[5]<<" |"<<endl;
    */

  
    return this->DisplayBounds;
}

//! Get the bounds for this Prop3D as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *svkOpenGLOrientedImageActor::GetBounds()
{
    int i,n;
    double *bounds, bbox[24], *fptr;
  
    bounds = this->GetDisplayBounds();
    // Check for the special case when the data bounds are unknown
    if (!bounds) {
        return bounds;
    }

      // fill out vertices of a bounding box
    bbox[ 0] = bounds[1]; bbox[ 1] = bounds[3]; bbox[ 2] = bounds[5];
    bbox[ 3] = bounds[1]; bbox[ 4] = bounds[2]; bbox[ 5] = bounds[5];
    bbox[ 6] = bounds[0]; bbox[ 7] = bounds[2]; bbox[ 8] = bounds[5];
    bbox[ 9] = bounds[0]; bbox[10] = bounds[3]; bbox[11] = bounds[5];
    bbox[12] = bounds[1]; bbox[13] = bounds[3]; bbox[14] = bounds[4];
    bbox[15] = bounds[1]; bbox[16] = bounds[2]; bbox[17] = bounds[4];
    bbox[18] = bounds[0]; bbox[19] = bounds[2]; bbox[20] = bounds[4];
    bbox[21] = bounds[0]; bbox[22] = bounds[3]; bbox[23] = bounds[4];
  
    // now calc the new bounds
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = VTK_DOUBLE_MAX;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_DOUBLE_MAX;
    for (i = 0; i < 8; i++) {
        for (n = 0; n < 3; n++) {
            if (bbox[i*3+n] < this->Bounds[n*2]) {
                this->Bounds[n*2] = bbox[i*3+n];
            }
            if (bbox[i*3+n] > this->Bounds[n*2+1]) {
                this->Bounds[n*2+1] = bbox[i*3+n];
            }
        }
    }
    return this->Bounds;
}


/*!
 * This Method has been copied from vtk for two reasons:
 * 1. Because it makes a call the MakeDataSuitable which is not virtual
 *    and we had to override it.
 * 2. There semes to be a bug in the vtk implementation. When the
 *    actor is moved from one render window to another its
 *    RenderWindow member variable is not updated UNLESS 
 *    the Texture size changes. To work around this we hare setting the
 *    texture size to 0 whene the render window changes. This causes
 *    the algorithm to regenerate the texture. 
 */
void svkOpenGLOrientedImageActor::Load(vtkRenderer *ren)
{
  GLenum format = GL_LUMINANCE;

  // need to reload the texture
  if (this->GetMTime() > this->LoadTime.GetMTime() ||
      this->Input->GetMTime() > this->LoadTime.GetMTime() ||
      ren->GetRenderWindow() != this->RenderWindow ||
      static_cast<vtkOpenGLRenderWindow*>(ren->GetRenderWindow())->GetContextCreationTime() >
      this->LoadTime)
    {

    /*********************** SVK MODIFICATION START **********************/
    // THIS IS ADDED. The actor does not appropriately track its renderwindow.
    // If we release the graphics resources and set the TextureSize to 0
    // It will correctly regenerate the texture of the image.
    if( ren->GetRenderWindow() != this->RenderWindow ) {
       this->ReleaseGraphicsResources( this->RenderWindow );
       this->TextureSize[0] = 0;
       this->TextureSize[1] = 0;

       }
    /*********************** SVK MODIFICATION END **********************/

    int xsize, ysize;
    int release, reuseTexture;
    unsigned char *data = this->MakeDataSuitable(xsize,ysize,
                                                 release, reuseTexture);
    int bytesPerPixel = this->Input->GetNumberOfScalarComponents();
    GLuint tempIndex=0;

    if (reuseTexture)
      {
#ifdef GL_VERSION_1_1
      glBindTexture(GL_TEXTURE_2D, this->Index);
#endif
      }
    else
      {
      // free any old display lists
      this->ReleaseGraphicsResources(ren->GetRenderWindow());
      this->RenderWindow = ren->GetRenderWindow();

      // define a display list for this texture
      // get a unique display list id
#ifdef GL_VERSION_1_1
      glGenTextures(1, &tempIndex);
      this->Index = static_cast<long>(tempIndex);
      glBindTexture(GL_TEXTURE_2D, this->Index);
#else
      this->Index = glGenLists(1);
      glDeleteLists (static_cast<GLuint>(this->Index),
                     static_cast<GLsizei>(0));
      glNewList (static_cast<GLuint>(this->Index), GL_COMPILE);
#endif

      static_cast<vtkOpenGLRenderWindow *>(ren->GetRenderWindow())
        ->RegisterTextureResource( this->Index );
      }
    
    if (this->Interpolate)
      {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       GL_LINEAR);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       GL_LINEAR );
      }
    else
      {
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      }
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP );

    int internalFormat = bytesPerPixel;
    switch (bytesPerPixel)
      {
      case 1: format = GL_LUMINANCE; break;
      case 2: format = GL_LUMINANCE_ALPHA; break;
      case 3: format = GL_RGB; break;
      case 4: format = GL_RGBA; break;
      }
    // if we are using OpenGL 1.1, you can force 32 or16 bit textures
#ifdef GL_VERSION_1_1
    switch (bytesPerPixel)
      {
      case 1: internalFormat = GL_LUMINANCE8; break;
      case 2: internalFormat = GL_LUMINANCE8_ALPHA8; break;
      case 3: internalFormat = GL_RGB8; break;
      case 4: internalFormat = GL_RGBA8; break;
      }
#endif

    if (reuseTexture)
      {
#ifdef GL_VERSION_1_1
      glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei( GL_UNPACK_ROW_LENGTH, 0);
      glTexSubImage2D(GL_TEXTURE_2D, 0,
                      0, 0, xsize, ysize, format, 
                      GL_UNSIGNED_BYTE,
                      static_cast<const GLvoid *>(data));
#endif
      }
    else
      {
      glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,
                   xsize, ysize, 0, format, 
                   GL_UNSIGNED_BYTE, static_cast<const GLvoid *>(data));
      this->TextureSize[0] = xsize;
      this->TextureSize[1] = ysize;
      }

#ifndef GL_VERSION_1_1
    glEndList ();
#endif
    // modify the load time to the current time
    this->LoadTime.Modified();
    if (release)
      {
      delete [] data;
      }
    }
  
  // execute the display list that uses creates the texture
#ifdef GL_VERSION_1_1
  glBindTexture(GL_TEXTURE_2D, this->Index);
#else
  glCallList(static_cast<GLuint>(this->Index));
#endif
  
  // don't accept fragments if they have zero opacity. this will stop the
  // zbuffer from be blocked by totally transparent texture fragments.
  glAlphaFunc (GL_GREATER, static_cast<GLclampf>(0));
  glEnable (GL_ALPHA_TEST);

  // now bind it 
  glEnable(GL_TEXTURE_2D);
  
  GLint uUseTexture=-1;
  GLint uTexture=-1;
  
  vtkOpenGLRenderer *oRenderer=static_cast<vtkOpenGLRenderer *>(ren);
  if(oRenderer->GetDepthPeelingHigherLayer())
    {
    cerr<<"UNSUPPORTED OPERATION: svkOpenGLOrientedImage does no support depth peeling in this way!"<<endl;
    exit(1);
 /* 
    uUseTexture=oRenderer->GetUseTextureUniformVariable();
    uTexture=oRenderer->GetTextureUniformVariable();
    vtkgl::Uniform1i(uUseTexture,1);
    vtkgl::Uniform1i(uTexture,0); // active texture 0
*/
    }
  
  // draw the quad
  if ( vtkMapper::GetResolveCoincidentTopology() )
    {
    if ( vtkMapper::GetResolveCoincidentTopology() == 
         VTK_RESOLVE_SHIFT_ZBUFFER )
      {
      }
    else
      {
#ifdef GL_VERSION_1_1
      double f, u;
      glEnable(GL_POLYGON_OFFSET_FILL);
      vtkMapper::GetResolveCoincidentTopologyPolygonOffsetParameters(f,u);
      glPolygonOffset(f,u);
#endif      
      }
    }
  glDisable(GL_COLOR_MATERIAL);
  glDisable (GL_CULL_FACE);
  glDisable( GL_LIGHTING );
  glColor4f( 1.0, 1.0, 1.0, this->Opacity );
  glBegin( GL_QUADS );
  for (int i = 0; i < 4; i++ )
    {
    glTexCoord2dv( this->TCoords + i*2 );
    glVertex3dv(this->Coords + i*3);
    }  
  glEnd();
  // Turn lighting back on
  glEnable( GL_LIGHTING );
}

/*!
 *  The key to getting oblique data to work, this sits up the texture mapping.
 *  NOTE: This method is override in its SUPERCLASS. As of now I am not sure
 *  why this is necessary, but polymorphism fails if this namespace is used.
 *  This is likely due to the fact that this is OpenGL specific, and I have
 *  bypassed overriding the factory-- needs to be fixed.
 */
unsigned char* svkOpenGLOrientedImageActor::MakeDataSuitable(int &xsize, int &ysize,
                                                     int &release,
                                                     int &reuseTexture)
{
    int contiguous = 0;
    unsigned short xs,ys;
    int powOfTwo = 0;
    int numComp = this->Input->GetNumberOfScalarComponents();
    int xdim, ydim;

    reuseTexture = 0;
  
    // it must be a power of two and contiguous
    // find the two used dimensions
    // this assumes a 2D image, no lines here folk
    if (this->ComputedDisplayExtent[0] != this->ComputedDisplayExtent[1]) {
        xdim = 0;
        if (this->ComputedDisplayExtent[2] != this->ComputedDisplayExtent[3]) {
            ydim = 1;
        } else {
            ydim = 2;
        }
    } else {
        xdim = 1;
        ydim = 2;
    }
  
    double *spacing = this->Input->GetSpacing();
    double *origin = this->Input->GetOrigin();
  
    // compute the world coordinates

    // Here are the oblique modifications
    // We recalculate the corners of the image, using the dcos
    double dcos[3][3];
    static_cast<svkImageData*>(this->Input)->GetDcos( dcos ); 
    double x[3];
    this->Coords[0]  = origin[0];
    for ( int i = 0; i<3; i++ ) {
        this->Coords[i] = origin[i];
        for ( int j = 0; j<3; j++ ) {
            this->Coords[i]  += this->ComputedDisplayExtent[2*j]*spacing[j]*dcos[j][i];
        }
    }
    for ( int i = 3; i<6; i++ ) {
        this->Coords[i] = origin[i-3];
        for ( int j = 0; j<3; j++ ) {
            if( j == 1 ) {
                this->Coords[i]  += this->ComputedDisplayExtent[2*j+(xdim==1)]*spacing[j]*dcos[j][(i-3)];
            } else if( j == 0 ){
                this->Coords[i]  += this->ComputedDisplayExtent[2*j+1]*spacing[j]*dcos[j][(i-3)];
            } else {
                this->Coords[i]  += this->ComputedDisplayExtent[2*j]*spacing[j]*dcos[j][(i-3)];
            }
        }
    }
    for ( int i = 6; i<9; i++ ) {
        this->Coords[i] = origin[i-6];
        for ( int j = 0; j<3; j++ ) {
            this->Coords[i]  += this->ComputedDisplayExtent[2*j+1]*spacing[j]*dcos[j][(i-6)];
        }
    }
    for ( int i = 9; i<12; i++ ) {
        this->Coords[i] = origin[i-9];
        for ( int j = 0; j<3; j++ ) {
            if( j == 1 ) {
                this->Coords[i]  += this->ComputedDisplayExtent[2*j+(ydim==1)]*spacing[j]*dcos[j][(i-9)];
            } else if( j == 0 ){
                this->Coords[i]  += this->ComputedDisplayExtent[2*j]*spacing[j]*dcos[j][(i-9)];
            } else {
                this->Coords[i]  += this->ComputedDisplayExtent[2*j+1]*spacing[j]*dcos[j][(i-9)];
            }
        }
    }

    // n w contiguous would require that xdim = 0 and ydim = 1
    // OR xextent = 1 pixel and xdim = 1 and ydim = 2 
    // OR xdim = 0 and ydim = 2 and yextent = i pixel. In addition
    // the corresponding x display extents must match the 
    // extent of the data
    int *ext = this->Input->GetExtent();
  
    if ( ( xdim == 0 && ydim == 1 && 
         this->ComputedDisplayExtent[0] == ext[0] && 
         this->ComputedDisplayExtent[1] == ext[1] )
         ||
       ( ext[0] == ext[1] && xdim == 1 && 
         this->ComputedDisplayExtent[2] == ext[2] && 
         this->ComputedDisplayExtent[3] == ext[3] ) 
         ||
       ( ext[2] == ext[3] && xdim == 0 && ydim == 2 &&
         this->ComputedDisplayExtent[0] == ext[0] && 
         this->ComputedDisplayExtent[1] == ext[1] ) ) {

        contiguous = 1;
    }
      
    // if contiguous is it a pow of 2
    if (contiguous) {
        xsize = ext[xdim*2+1] - ext[xdim*2] + 1;
        // xsize and ysize must be a power of 2 in OpenGL
        xs = static_cast<unsigned short>(xsize);
    while (!(xs & 0x01)) {
          xs = xs >> 1;
      }
    if (xs == 1) {
          powOfTwo = 1;
      }
    }
  
    if (contiguous && powOfTwo) {
        // can we make y a power of two also ?
        ysize = (this->ComputedDisplayExtent[ydim*2+1] -
                 this->ComputedDisplayExtent[ydim*2] + 1);
        ys = static_cast<unsigned short>(ysize);
        while (!(ys & 0x01)) {
            ys = ys >> 1;
        }
        // yes it is a power of two already
        if (ys == 1) {
            release = 0;
            this->TCoords[0] = (this->ComputedDisplayExtent[xdim*2] - 
                              ext[xdim*2] + 0.5)/xsize;
            this->TCoords[1] = 0.5/ysize;  
            this->TCoords[2] = (this->ComputedDisplayExtent[xdim*2+1] -
                              ext[xdim*2] + 0.5)/xsize;
            this->TCoords[3] = this->TCoords[1];  
            this->TCoords[4] = this->TCoords[2];
            this->TCoords[5] = 1.0 - 0.5/ysize;  
            this->TCoords[6] = this->TCoords[0];
            this->TCoords[7] = this->TCoords[5];

#ifdef GL_VERSION_1_1
            // if texture size hasn't changed, reuse old texture
            if (xsize == this->TextureSize[0] && ysize == this->TextureSize[1]) {
                reuseTexture = 1;
            }
#endif
            return static_cast<unsigned char *>(
            this->Input->GetScalarPointerForExtent(this->ComputedDisplayExtent));
        }
    }
  
    // if we made it here then we must copy the data and possibly pad 
    // it as well

    // find the target size
    xsize = 1;
    while (xsize < this->ComputedDisplayExtent[xdim*2+1] - this->ComputedDisplayExtent[xdim*2] + 1) {
        xsize *= 2;
    }
    ysize = 1;
    while (ysize < this->ComputedDisplayExtent[ydim*2+1] - this->ComputedDisplayExtent[ydim*2] + 1) {
        ysize *= 2;
    }
  
    // compute the tcoords
    this->TCoords[0] = 0.5/xsize;
    this->TCoords[1] = 0.5/ysize;  
    this->TCoords[2] = (this->ComputedDisplayExtent[xdim*2+1] -
                        this->ComputedDisplayExtent[xdim*2] + 0.5)/xsize;
    this->TCoords[3] = this->TCoords[1];  
    this->TCoords[4] = this->TCoords[2];
    this->TCoords[5] = (this->ComputedDisplayExtent[ydim*2+1] -
                        this->ComputedDisplayExtent[ydim*2] + 0.5)/ysize;  
    this->TCoords[6] = this->TCoords[0];
    this->TCoords[7] = this->TCoords[5];  

#ifdef GL_VERSION_1_1
    // reuse texture if texture size has not changed
    if (xsize == this->TextureSize[0] && ysize == this->TextureSize[1]) {
        reuseTexture = 1;
        xsize = this->ComputedDisplayExtent[xdim*2+1] -
                this->ComputedDisplayExtent[xdim*2] + 1;
        ysize = this->ComputedDisplayExtent[ydim*2+1] -
                this->ComputedDisplayExtent[ydim*2] + 1;
    }
#endif

    // if contiguous and texture size hasn't changed, don't copy or pad
    if (reuseTexture && contiguous) {
        release = 0;
        return static_cast<unsigned char *>(
          this->Input->GetScalarPointerForExtent(this->ComputedDisplayExtent));
    }

    // allocate the memory
    unsigned char *res = new unsigned char [ysize*xsize*numComp];
    release = 1;
  
    // copy the input data to the memory
    vtkIdType inIncX, inIncY, inIncZ;
    int idxZ, idxY, idxR;
    unsigned char *inPtr = static_cast<unsigned char *>(
                    this->Input->GetScalarPointerForExtent(this->ComputedDisplayExtent));
    this->Input->GetContinuousIncrements(this->ComputedDisplayExtent, 
                                             inIncX, inIncY, inIncZ);
    int rowLength = numComp*(this->ComputedDisplayExtent[1] -
                             this->ComputedDisplayExtent[0] +1);
    unsigned char *outPtr = res;
    vtkIdType outIncY, outIncZ;
    if (ydim == 2) {
        if (xdim == 0) {
            outIncZ = numComp * 
            (xsize - (this->ComputedDisplayExtent[1] -
                      this->ComputedDisplayExtent[0] + 1));
        } else {
            outIncZ = numComp * 
            (xsize - (this->ComputedDisplayExtent[3] -
                      this->ComputedDisplayExtent[2] + 1));
        }
        outIncY = 0;
    } else {
        outIncY = numComp * 
            (xsize - (this->ComputedDisplayExtent[1] -
                      this->ComputedDisplayExtent[0] + 1));
        outIncZ = 0;    
    }
  
      
    for (idxZ = this->ComputedDisplayExtent[4];
          idxZ <= this->ComputedDisplayExtent[5]; idxZ++) {

        for (idxY = this->ComputedDisplayExtent[2];
            idxY <= this->ComputedDisplayExtent[3]; idxY++) {

            for (idxR = 0; idxR < rowLength; idxR++) {
                // Pixel operation
                *outPtr = *inPtr;
                outPtr++;
                inPtr++;
            }
            outPtr += outIncY;
            inPtr += inIncY;
        }
        outPtr += outIncZ;
        inPtr += inIncZ;
    }
  
    return res;
}


// NOTE METHOD NOT CHANGED FROM VTK IMPLEMENTATION
// Actual actor render method.
// Recursive to handle larger textures than can be rendered by
// a given video card. Assumes all video cards can render a texture
// of 256x256 so will fail if card reports that it cannot render
// a texture of this size rather than recursing further
void svkOpenGLOrientedImageActor::Render(vtkRenderer *ren)
{
  glPushAttrib( GL_ENABLE_BIT );
  
  // Save the current display extent since we might change it
  int savedDisplayExtent[6];
  this->GetDisplayExtent( savedDisplayExtent );
 
  // What is the power of two texture big enough to fit the display extent?
  // This should be 1 in some direction.
  int i;
  int pow2[3] = {1,1,1};
  int baseSize[3];
  for ( i = 0; i < 3; i++ )
    {
    baseSize[i] = (this->ComputedDisplayExtent[i*2+1] -
                   this->ComputedDisplayExtent[i*2] + 1);
    while( pow2[i] < baseSize[i] )
      {
      pow2[i] *= 2;
      }
    }
  
  // Find the 2d texture in the 3d pow2 structure
  int size[2];
  if ( pow2[0] == 1 )
    {
    size[0] = pow2[1];
    size[1] = pow2[2];
    }
  else if ( pow2[1] == 1 )
    {
    size[0] = pow2[0];
    size[1] = pow2[2];
    }
  else
    {
    size[0] = pow2[0];
    size[1] = pow2[1];
    }
  
  // Check if we can fit this texture in memory
  if ( this->TextureSizeOK(size) )
    {
    // We can fit it - render
    this->InternalRender( ren );
    }
  else
    {
    // If we can't handle a 256x256 or smaller texture,
    // just give up and don't render anything. Something
    // must be horribly wrong...
    if ( size[0] <= 256 && size[1] <= 256 )
      {
      return;
      }
    
    // We can't fit it - subdivide
    int newDisplayExtent[6];
    int idx;

    // Find the biggest side
    if ( baseSize[0] >= baseSize[1] && baseSize[0] >= baseSize[2] )
      {
      idx = 0;
      }
    else if ( baseSize[1] >= baseSize[0] && baseSize[1] >= baseSize[2] )
      {
      idx = 1;
      }
    else 
      {
      idx = 2;
      }
    
    // For the other two sides, just copy in the display extent
    for ( i = 0; i < 3; i++ )
      {
      if ( i != idx )
        {
        newDisplayExtent[i*2] = this->ComputedDisplayExtent[i*2];
        newDisplayExtent[i*2+1] = this->ComputedDisplayExtent[i*2+1];        
        }
      }
    
    // For the biggest side - divide the power of two size in 1/2
    // This is the first half
    int tempDisplayExtent = this->ComputedDisplayExtent[idx*2+1];
    newDisplayExtent[idx*2] = this->ComputedDisplayExtent[idx*2];
    newDisplayExtent[idx*2+1] = (newDisplayExtent[idx*2] +
                                 baseSize[idx]/2 - 1);
    
    // Set it as the display extent and render
    this->SetDisplayExtent( newDisplayExtent );
    this->Render(ren);

    // This is the remaining side (since the display extent is not 
    // necessarily a power of 2, this is likely to be less than half
    newDisplayExtent[idx*2] = (this->ComputedDisplayExtent[idx*2] +
                               baseSize[idx]/2 - 1);
    newDisplayExtent[idx*2+1] = tempDisplayExtent;
    
    // Set it as the display extent and render
    this->SetDisplayExtent( newDisplayExtent );
    this->Render(ren);
    }
  
  // Restore the old display extent
  this->SetDisplayExtent( savedDisplayExtent ); 
  
  glPopAttrib();
}


// UNMODIFIED FROM VTK
// This is the non-recursive render that will not check the
// size of the image (it has already been determined to
// be fine)
void svkOpenGLOrientedImageActor::InternalRender(vtkRenderer *ren)
{

  // for picking
  glDepthMask (GL_TRUE);

  // build transformation 
  if (!this->IsIdentity)
    {
    double *mat = this->GetMatrix()->Element[0];
    double mat2[16];
    mat2[0] = mat[0];
    mat2[1] = mat[4];
    mat2[2] = mat[8];
    mat2[3] = mat[12];
    mat2[4] = mat[1];
    mat2[5] = mat[5];
    mat2[6] = mat[9];
    mat2[7] = mat[13];
    mat2[8] = mat[2];
    mat2[9] = mat[6];
    mat2[10] = mat[10];
    mat2[11] = mat[14];
    mat2[12] = mat[3];
    mat2[13] = mat[7];
    mat2[14] = mat[11];
    mat2[15] = mat[15];
    
    // insert model transformation 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixd(mat2);
    }
  
  // Render the texture
  this->Load(ren);

  // pop transformation matrix
  if (!this->IsIdentity)
    {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    }
}


void svkOpenGLOrientedImageActor::SetInput(vtkImageData* args)
{
    double dcos[3][3];
    dcos[0][0] = 1;
    dcos[0][1] = 0;
    dcos[0][2] = 0;
    dcos[1][0] = 0;
    dcos[1][1] = 1;
    dcos[1][2] = 0;
    dcos[2][0] = 0;
    dcos[2][1] = 0;
    dcos[2][2] = -1;
    if (this->Input != args && args != NULL) {                                                           
        if (this->Input != NULL) { 
            this->Input->UnRegister(this); 
        }   
        if( strcmp( args->GetClassName(), "vtkImageData") == 0 ) {
            this->Input = svkMriImageData::New(); 
            static_cast<svkImageData*>(this->Input)->CopyVtkImage( args, dcos); 
        } else {
            this->Input = args;
        }
        if (this->Input != NULL) { 
            this->Input->Register(this); 
        }     
        this->Modified();                                           
    }                                                           
}

