@setlocal
@echo off
@rem Generate doc.inl
gcc -Wall -o doc.exe -DMK_DOC doc.c
doc >nul
@rem convert grammar into C code
bison --output=zmac.c zmac.y
@rem compile the rest
gcc -Wall -s -o zmac.exe -DMSDOS zmac.c mio.c doc.c zi80dis.cpp
@endlocal
