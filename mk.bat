cd z80
wcl386 /c /ox /ot /fp3 /4r /s z80.cpp
wcl386 /c /ox /ot /fp3 /4r /s z80_ops.cpp
cd ..
wcl386 /c /ox /ot /fp3 /4r /s estex.cpp
wcl386 /c /ox /ot /fp3 /4r /s unigraf.cpp /dFONT8X8
wcl386 /c /ox /ot /fp3 /4r /s targa.cpp
wcl386 /ox /ot /fp3 /4r /s sprint.cpp z80\z80.obj z80\z80_ops.obj estex.obj unigraf.obj targa.obj
COPY SPRINT.EXE SPRINTGW.EXE
DEL SPRINT.EXE
