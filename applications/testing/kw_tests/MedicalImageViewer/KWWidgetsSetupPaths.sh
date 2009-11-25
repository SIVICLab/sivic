export PATH="/netopt/lib/vtk-5.2:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets:/netopt/lib/KWWidgets:/usr/bin:/usr/bin:$PATH"
export LD_LIBRARY_PATH="/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets:$LD_LIBRARY_PATH"
export TCLLIBPATH=' {/netopt/lib/vtk-5.2} {/netopt/lib/KWWidgets/tcl} '$TCLLIBPATH
export PYTHONPATH="/netopt/lib/vtk-5.2/../python2.4/site-packages:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets/../python2.4/site-packages:/netopt/lib/KWWidgets:$PYTHONPATH"
if [ "0" == "1" ]; then
  export ITCL_LIBRARY=""
  export ITK_LIBRARY=""
fi
