#!/usr/bin/env python3

import os
import sys

def replace_includes(path):
    for filename in os.listdir(path):
        if filename.endswith(".h"):
         files = os.path.join(path,filename)
         with open(files, 'r') as f:
          print(files)  
          content = f.read()

    # Replace the old include path with the new one
         updated_content = content.replace(
         '#include </usr/include/vtk/',
         '#include </mnt/nfs/rad/apps/netopt/versions/vtk/VTK-9.3.0/include/vtk-9.3/'
        )
    # Write the updated content back to the same file
         with open(files, 'w') as f:
          f.write(updated_content)
         print(f"Updated includes in {files}")
if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <filename>")
        sys.exit(1)

    directory = sys.argv[1]
    replace_includes(directory)