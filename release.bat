call .\precompile.bat
call .\build_release.bat
call  copy .\tree-sitter-exe-lint\exe\tree_sitter_exe.exe .\tree-sitter-exe-lint\exe\tree_sitter_exe_cc.exe
call copy .\Release\tree_sitter_dll.dll .\tree-sitter-exe-lint\exe
call copy .\Release\tree_sitter_exe.exe .\tree-sitter-exe-lint\exe
call .\tree-sitter-exe-lint\exe\tree_sitter_exe.exe < input.txt