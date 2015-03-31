To build the FVM driver "hooksys.sys", use the WDK's build environment for 
either Windows 2000 or XP, depending on which platform you want to build the 
driver for.

*******************************************************************************
A typical build would look something like..

D:\fvmDrv>build -wg
BUILD: Compile and Link for x86
BUILD: Loading c:\winddk\6000\build.dat...
BUILD: Computing Include file dependencies:
BUILD: Start time: Mon Feb 25 13:44:46 2008
BUILD: Examining d:\fvmdrv directory for files to compile.
BUILD: Saving c:\winddk\6000\build.dat...
BUILD: Compiling and Linking d:\fvmdrv directory
_NT_TARGET_VERSION SET TO WINXP
Compiling - fvm_vm.c
Compiling - fvm_table.c
Compiling - fvm_util.c
Compiling - fvm_file.c
Compiling - fvm_fileopt.c
Compiling - fvm_reg.c
Compiling - fvm_obj.c
Compiling - fvm_daemon.c
Compiling - hooksys.c
Compiling - generating code...
Linking Executable - bin\i386\hooksys.sys
BUILD: Finish time: Mon Feb 25 13:44:49 2008
BUILD: Done

    11 files compiled - 13150 LPS
    1 executable built

*******************************************************************************

P.S. : Note that FVM only supports the latest Windows Driver Kit (WDK) from 
Microsoft and not the older DDK.