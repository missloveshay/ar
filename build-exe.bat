@echo off

setlocal
if (%1)==(-clean) goto :cleanup
set CC=gcc

%CC% -fno-builtin -O2 -c ar.c
%CC% -fno-builtin -O2 -c arlib.c
%CC% -o ar ar.o arlib.o
rm *.o
endlocal