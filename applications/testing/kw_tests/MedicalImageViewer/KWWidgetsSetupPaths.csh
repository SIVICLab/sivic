setenv PATH "/netopt/lib/vtk-5.2:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets:/netopt/lib/KWWidgets:/usr/bin:/usr/bin:$PATH"
if ( $?LD_LIBRARY_PATH ) then
  setenv LD_LIBRARY_PATH "/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets:$LD_LIBRARY_PATH"
else
  setenv LD_LIBRARY_PATH "/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets"
endif
if ( $?TCLLIBPATH ) then
  setenv TCLLIBPATH ' {/netopt/lib/vtk-5.2} {/netopt/lib/KWWidgets/tcl} '$TCLLIBPATH
else
  setenv TCLLIBPATH ' {/netopt/lib/vtk-5.2} {/netopt/lib/KWWidgets/tcl}'
endif
if ( $?PYTHONPATH ) then
  setenv PYTHONPATH "/netopt/lib/vtk-5.2/../python2.4/site-packages:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets/../python2.4/site-packages:/netopt/lib/KWWidgets:$PYTHONPATH"
else
  setenv PYTHONPATH "/netopt/lib/vtk-5.2/../python2.4/site-packages:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets/../python2.4/site-packages:/netopt/lib/KWWidgets"
endif
if ( "0" == "1" ) then
  setenv ITCL_LIBRARY ""
  setenv ITK_LIBRARY ""
endif

