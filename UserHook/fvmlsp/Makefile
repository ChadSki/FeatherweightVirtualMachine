!include <win32.mak>

!IFDEF WIN2K
WIN_VER = -DWIN_2K
!ELSEIFDEF WINXP
WIN_VER = -DWIN_XP
!endif

!IFNDEF NODEBUG
.cpp.obj:
	$(cc) $(cdebug) $(cflags) $(cvarsdll) -DSTRICT -YX\
        /D "DEBUG_TRACING"\
        /D "TRACING" /Od /D "_DEBUG" $(WIN_VER) $<
!else
.cpp.obj:
	$(cc) $(cdebug) $(cflags) $(cvarsdll) -DSTRICT -YX -DNODEBUG $(WIN_VER) $<
!endif

CPP_FILES= fvmLsp.cpp

OBJS= $(CPP_FILES:.cpp=.obj)

all: fvmLsp.dll fvmInLsp.exe

fvmLsp.dll: $(OBJS)
	$(link) $(linkdebug) $(dlllflags) \
    -map \
	-export:WSPStartup \
	-out:$*.dll $(OBJS) $(guilibsdll) ws2_32.lib

fvmInLsp.obj: fvmInLsp.cpp
	$(cc) $(cdebug) $(cflags) $(cvars) fvmInLsp.cpp

fvmInLsp.exe: fvmLsp.obj fvmInLsp.obj
	$(link) $(linkdebug) $(conlflags) -out:$*.exe fvmLsp.obj fvmInLsp.obj\
        rpcrt4.lib /NODEFAULTLIB:LIBCD.lib $(conlibs) ws2_32.lib sporder.lib User32.lib

clean:
    -del *.obj
	-del *.dll
	-del *.exe
