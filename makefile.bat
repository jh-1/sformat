
call wcc sformat.c -za -ml -wx -s -zls -ecc 2>&1 | ^
find /v "DOS/4GW Protected Mode Run-time" | ^
find /v "Copyright (c) Rational Systems" | ^
find /v "Open Watcom C16 Optimizing Compiler" | ^
find /v "Portions Copyright (c)" | ^
find /v "Source code is available under" | ^
find /v "See http://www.openwatcom.org/" | ^
find /v "Code size:"
ren sformat.OBJ sformat.obj
