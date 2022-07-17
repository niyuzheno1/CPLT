call .\precompile.bat
call .\build_debug.bat
call move .\tree_sitter_exe\src\main.cpp .\tree_sitter_exe\src\main.bak
call .\run.bat
call copy .\Debug\tree_sitter_exe.exe .\Debug\tree_sitter_exe_cd.exe