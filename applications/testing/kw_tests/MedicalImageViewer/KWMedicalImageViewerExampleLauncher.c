/*=========================================================================

  Module:    $RCSfile: KWWidgetsSetupPathsLauncher.c.in,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
#include <io.h>
#include <windows.h>
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

#if defined(PATH_MAX)
#define MAX_PATH_len PATH_MAX
#elif defined(MAXPATHLEN)
#define MAX_PATH_len MAXPATHLEN
#else
#define MAX_PATH_len 16384
#endif

#define MAX_QUOTED_ARGV 50

int main(int argc, char** argv)
{
  char exe_path[MAX_PATH_len];
  char real_exe_path[MAX_PATH_len];

  char path_env[MAX_PATH_len];
  const char *old_path_env;

  char ld_path_env[MAX_PATH_len];
  const char *old_ld_path_env;

  char tcllibpath_env[MAX_PATH_len];
  const char *old_tcllibpath_env;

  char pythonpath_env[MAX_PATH_len];
  const char *old_pythonpath_env;

#if 0
  char itcl_library_env[MAX_PATH_len];
  char itk_library_env[MAX_PATH_len];
#endif

  char message[MAX_PATH_len];
  char **arg;

  char current_exe[MAX_PATH_len];
  char *slash_pos;

#if defined(_WIN32)
  size_t exe_path_len;
#if !defined(__CYGWIN__)
  char quoted_argv[MAX_QUOTED_ARGV][MAX_PATH_len];
  int nb_quoted_argv;
#endif
#endif

  /* save the current directory */
  char saved_working_directory[MAX_PATH_len+1];
#if defined(_WIN32)
  _getcwd(saved_working_directory,MAX_PATH_len);
#else
  getcwd(saved_working_directory,MAX_PATH_len);
#endif
 
  /* PATH */

  strcpy(path_env, 
         "PATH=/netopt/lib/vtk-5.2:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets:/netopt/lib/KWWidgets:/usr/bin:/usr/bin");
  old_path_env = getenv("PATH");
  if(old_path_env)
    {
    strcat(path_env, ":");
    strcat(path_env, old_path_env);
    }
  putenv(path_env);

  /* LD_LIBRARY_PATH and such */

  strcpy(ld_path_env, 
         "LD_LIBRARY_PATH=/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets");
  old_ld_path_env = getenv("LD_LIBRARY_PATH");
  if(old_ld_path_env)
    {
    strcat(ld_path_env, ":");
    strcat(ld_path_env, old_ld_path_env);
    }
  putenv(ld_path_env);

  /* TCLLIBPATH */

  strcpy(tcllibpath_env, 
         "TCLLIBPATH= {/netopt/lib/vtk-5.2} {/netopt/lib/KWWidgets/tcl}");
  old_tcllibpath_env = getenv("TCLLIBPATH");
  if(old_tcllibpath_env)
    {
    strcat(tcllibpath_env, " ");
    strcat(tcllibpath_env, old_tcllibpath_env);
    }
  putenv(tcllibpath_env);

  /* PYTHONPATH */

  strcpy(pythonpath_env, 
         "PYTHONPATH=/netopt/lib/vtk-5.2/../python2.4/site-packages:/netopt/lib/vtk-5.2:/netopt/lib/KWWidgets/../python2.4/site-packages:/netopt/lib/KWWidgets/../python2.4/site-packages:/netopt/lib/KWWidgets");
  old_pythonpath_env = getenv("PYTHONPATH");
  if(old_pythonpath_env)
    {
    strcat(pythonpath_env, ":");
    strcat(pythonpath_env, old_pythonpath_env);
    }
  putenv(pythonpath_env);

#if 0

  /* ITCL_LIBRARY */

  strcpy(itcl_library_env, 
         "ITCL_LIBRARY=");
  putenv(itcl_library_env);

  /* ITK_LIBRARY */

  strcpy(itk_library_env, 
         "ITK_LIBRARY=");
  putenv(itk_library_env);

#endif

  /* Save the native form of the current executable path so that we can set
     our working dir to its location later on. */

#if defined(_WIN32) && !defined(__CYGWIN__)
  GetFullPathName(argv[0], MAX_PATH_len, current_exe, 0);
#else
  realpath(argv[0], current_exe);
#endif

  /* Replace this process with the real executable. If no executable
     was specified for EXECUTABLE, use the first argument that was passed. */

  strcpy(exe_path, 
         "KWMedicalImageViewerExample");
  if(strlen(exe_path) == 0 && argv)
    {
    argv++;
    if (argv[0])
      {
      strcpy(exe_path, argv[0]);
      }
    }

  /* Append .exe extension if it was not found */

#if defined(_WIN32) 
  exe_path_len = strlen(exe_path);
  if (exe_path_len < 4 || (strcmp(exe_path + exe_path_len - 4, ".exe") && 
                           strcmp(exe_path + exe_path_len - 4, ".EXE") &&
                           strcmp(exe_path + exe_path_len - 4, ".com") &&
                           strcmp(exe_path + exe_path_len - 4, ".COM")))
    {
    strcat(exe_path, ".exe");
    }
#endif

  /* Change the current working dir to the dir of the original executable 
     so that the executable to launch can be found if is relative. */

#if defined(_WIN32) && !defined(__CYGWIN__)
  slash_pos = strrchr(current_exe, '\\');
#else
  slash_pos = strrchr(current_exe, '/');
#endif
  if (slash_pos)
    {
    *slash_pos = '\0';
#if defined(_WIN32) && (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__MINGW32__)) 
    _chdir(current_exe);
#else
    chdir(current_exe);
#endif
    }
  
  /* Retrieve the native path to the executable to launch; this will use the
     current working dir if exe_path is relative. */

#if defined(_WIN32) && !defined(__CYGWIN__)
  GetFullPathName(exe_path, MAX_PATH_len, real_exe_path, 0);
#else
  realpath(exe_path, real_exe_path);
#endif

  argv[0] = real_exe_path;

  /* Quote unquoted args, otherwise args seem to get mixed up */

#if defined(_WIN32) && !defined(__CYGWIN__)
  for(argc = 0, nb_quoted_argv = 0; 
      nb_quoted_argv < MAX_QUOTED_ARGV && argv[argc]; ++argc)
    {
    if (argv[argc][0] != '"' && strstr(argv[argc], " ") != NULL)
      {
      strcpy(quoted_argv[nb_quoted_argv], "\"");
      strcat(quoted_argv[nb_quoted_argv], argv[argc]);
      strcat(quoted_argv[nb_quoted_argv], "\"");
      argv[argc] = quoted_argv[nb_quoted_argv];
      nb_quoted_argv++;
      }
    }
#endif

#if defined(_WIN32) && !defined(__CYGWIN__)
  _chdir(saved_working_directory);
#else
  chdir(saved_working_directory);
#endif

#if defined(_MSC_VER)
  /* _execv can't be used as it does not property return the exit value
     of the process to execute. */
  return _spawnv(_P_WAIT, real_exe_path, argv);
#else
  execv(real_exe_path, argv);
#endif

  /* Report why execution failed.  */
  
#if defined(_WIN32) && !defined(__CYGWIN__)
#else
  strcpy(message, strerror(errno));
#endif

  fprintf(stderr, "Error running");
  for(arg = argv; *arg; ++arg)
    {
    fprintf(stderr, " \"%s\"", *arg);
    }
  fprintf(stderr, ": %s\n", message);

  (void)argc;

  return 1;
}
