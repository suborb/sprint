del *.obj
del sprint.exe
cd z80
call ..\c_sdl -c -ox z80.cpp
call ..\c_sdl -c -ox z80_ops.cpp
cd ..
call c_sdl -c -ox estex.cpp
call c_sdl -c -ox unigraf.cpp -DFONT8X8 -DSDL -DUNIFOPEN
call c_sdl -c -ox targa.cpp
call c_sdl sprint.cpp -DWINDOWS z80\z80.obj z80\z80_ops.obj estex.obj unigraf.obj targa.obj
