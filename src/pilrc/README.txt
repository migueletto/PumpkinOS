----------------------------------------------------------------------------
 PilRC version 3.2.x                                             07 Feb 2007
----------------------------------------------------------------------------

  INSTALLATION:
  =============

  PilRC is an application  that takes a resource script file and generates
  one or more binary resource files  that are to be  used when  developing
  for  the  Palm Computing Platform.  PilRCUI gives you a  preview of your
  resource file.

  Copyright (C) 1997-1999 Wes Cherry
  Copyright (C) 2000-2004 Aaron Ardiri
  Copyright (C) 2004-2010 Alexander Pruss

  This program is free software;  you can redistribute it and/or modify it 
  under the terms of the GNU  General  Public  License as published by the 
  Free Software Foundation;  either version  2 of the License, or (at your 
  option) any later version.

  This  program  is  distributed in  the hope that it will be useful,  but 
  WITHOUT   ANY   WARRANTY;   without   even   the  implied   warranty  of 
  MERCHANTABILITY  or  FITNESS  FOR A  PARTICULAR  PURPOSE.  See  the  GNU 
  General Public License for more details.

  You should  have received a copy of the GNU General Public License along 
  with this program;  if not, write to the Free Software Foundation, Inc., 
  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  You should obtain the latest version of PilRC before compiling it. It is
  available from the following locations:

    http://sourceforge.net/projects/pilrc
    http://pilrc.sourceforge.net/

  BUILDING
  ========

  The  source code and  binaries are  available  free of  charge  and  are 
  Copyrighted 1997-2004 by Wes Cherry and Aaron Ardiri. We'd encourage you
  to send modifications (bug-fixes and/or improvements) of PilRC to  Aaron 
  Ardiri so that they can be integrated into the next release(s). 

  > generic "compile"

  A generic  Makefile has  been included that compiles using a very simple
  structure  that can be edited  to suit your  own needs.  It has not been 
  configured against your system and may require some modification.  PilRC
  can be compiled by typing the following at your command prompt:

     make -f Makefile.generic

  PilRCUI can be compiled  by typing the following at your  command prompt
  when using a system with "gtk" support (unix systems mainly):

     make -f Makefile.gui

  NOTE: PilRCUI has not been updated, and may not build correctly - unless
        there is a fix for this, you should ignore it and use another tool
        such as 'pilrcedit' to do resource editing.
 
  > systems (mainly Unix) detectable via configure

  If  you are using a system on which autoconf-style configure scripts are
  usable, PilRC can be built and installed by typing the following at your
  command prompt:

     unix/configure
     make
     make install

  Use "unix/configure --help" to see what configure options are available;
  the PilRC-specific options are --enable-pilrcui, which causes PilRCUI to
  be built  (which requires GTK development headers and libraries,  and is
  disabled by default),   and  --enable-build-warnings,   which turns on a
  number of GCC warning options during the build.

  Not  all  of the files in the "unix" subdirectory are PilRC source files
  which can usefully be edited.  They can be divided into three categories:

     Genuine PilRC source files:
        acinclude.m4, configure.ac, Makefile.am, pilrc.build, pilrc.spec

     Standard utility scripts copied from gnulib:
        depcomp, install-sh, missing, mkinstalldirs

     Generated files (regenerated with "aclocal && automake && autoconf"):
        aclocal.m4, configure, Makefile.in

  > solaris

  PilRC often produces "core dumps" under solaris when  processing bitmap 
  resources (reading BMP files). This is due to the byte alignment of the 
  BMP headers and the optimization performed by the solaris gcc compiler.

  Remove the "-O2" option  from the CCFLAGS  variable in the Makefile and
  the execution of PilRC should have no problems :))

  > win32 (Microsoft Visual Studio)

  If you have Microsoft Visual Studio installed, PilRC can be built using
  the "pilrc.dsp" project and/or the "pilrc.mak" file in the "win32_msvc"
  directory.

  > win32 (CodeWarrior for Windows)
  
  If you have CodeWarrior for Windows V8, you can build the pilrc.exe
  executable using the "pilrc.mcp" project file in the "win32_cw" directory.
  
  Just open the project in the CW IDE, and use the Project/Make command.

  > OS/2 Warp

  PilRC has been built on OS/2 that has E. Matthes' EMX package and DMAKE 
  by Dennis Vadura installed. Please read the additional information 
  provided in "makefile.os2". Compilation is done as follows:

     dmake -f makefile.os2

  Note that these files are no longer supported, and have not changed.

  Thankyou for downloading and using PilRC! 

  // az
  aaron@ardiri.com

----------------------------------------------------------------------------
