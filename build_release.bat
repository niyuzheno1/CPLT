@echo off
msbuild cplt.sln /p:Configuration=RELEASE /p:Platform=x64
move .\tree_sitter_exe\src\main.cpp .\tree_sitter_exe\src\main.bak