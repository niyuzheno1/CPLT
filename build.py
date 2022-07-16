import subprocess
import os
def run_batch_script(script_name):
    return subprocess.check_output(script_name, shell=True)

def byte_to_string(byte_array):
    return byte_array.decode('utf-8')

# main process 
def main():
    precompile = run_batch_script('.\\precompile.bat')
    print(byte_to_string(precompile))
    build = run_batch_script('.\\build_debug.bat')
    print(byte_to_string(build))
    os.system('move .\\tree_sitter_exe\\src\\main.cpp .\\tree_sitter_exe\\src\\main.bak')
    return 0

if __name__ == "__main__":
    main()