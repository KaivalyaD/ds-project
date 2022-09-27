cls

del *.exe
del *.obj
del *.res

cl.exe /c /EHsc OGL.cpp LS/ls.cpp

rc.exe OGL.rc

link.exe *.obj *.res user32.lib gdi32.lib winmm.lib /OUT:OGL.exe /SUBSYSTEM:WINDOWS /MACHINE:x64
