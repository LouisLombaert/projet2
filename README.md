# LINFO1252 – Systèmes informatiques

## Project Structure

- **lib_tar.h:** Header of lib_tar, untouched
- **lib_tar.c:** Files with all the functions to manage a tar archive
- **Makefile:** Build and run automation script
- **tests.c:** file with all the tests
- **target_file.txt:** file to be linked (symbolic_link.txt)
- **file1.txt:** file to be read

## Building and Running

### Prerequisites

- GCC (GNU Compiler Collection)

### Build
For all the following commands, run them in the project root directory where the 'Makefile' file is.

To build the files run:
```bash
make 
```

To build the archive to test all the tests, run. Careful, you must not have a file named **symbolic_file.txt**:
```bash
make arch
```

To run the tests with the built archive, run:
```bash
./tests arch.tar
```

Finally, to clear the files, run:
```bash
make cls
```