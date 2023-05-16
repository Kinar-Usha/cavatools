# cavatools

    mkdir RISCV
    cd RISCV
    export RISCV=/path/from/home/to/RISCV
    export PATH=$PATH:$RISCV/bin

    git clone riscv-gnu-toolchain (large size 3gb src)
    git clone riscv-isa-sim
    git clone riscv-pk
    git clone riscv-opcodes

    cd riscv-gnu-toolchain
    mkdir build
    cd build
    ../configure --prefix=$RISCV
    make (add -j3 for faster compilation)
    make install


    cd riscv-isa-sim
    git branch branchname commithash
    git checkout branchname
    mkdir build
    cd build
    ../configure --prefix=$RISCV --enable-histogram
    make 
    make install

    cd riscv-pk
    mkdir build
    cd build
    ../configure --prefix=$RISCV --host=riscv64-unknown-elf
    make 
    make install

    clone cavatools
    set RVTOOLS to RISCV
    mkdir bin and lib in $CAVA directory
    
    
    cd cavatools 
    make install


    
RISC-V instruction set simulator and performance analysis tools
===============================================================

Cavatools simulates a multi-core RISC-V machine.  The simulator runs on X86 Linux host and presents a user-mode multi-threaded Linux interface to the guest program.  Standard RISC-V tool chain (GNU or LLVM) with GLIBC is used to compile the guest program.  All host Linux system calls are available to the guest program.  Multi-threaded guest programs compiled with Pthead and OpenMP are supported.  Cavatools has several parts:
*  **uspike** is a RISC-V instruction set interpreter.  Python scripts extract instruction bit encoding and execution semantics from the official GitHub repository.
*  **caveat** is a performance simulator.  Currently it models a multicore chip with single-issue in-order pipelines and private instruction and data level-1 caches.
*  **erised** is a real-time performance viewer for caveat while the guest program is running.  For each static instruction erised shows the execution frequency, cycles per instruction, instruction and cache misses while the program is running.

###  Prerequisites

Environment variable RVTOOLS should be path to the riscv-tools tree.  The subtrees are:

    riscv-tools/riscv-opcodes       [from https://github.com/riscv/riscv-opcodes commit-hash  21b508e]
    riscv-tools/riscv-isa-sim       [from https://github.com/riscv-software-src/riscv-isa-sim commit-hash 641d7d03e6a8d822b720fcb1a19f261813c00c4a]
    riscv-tools/riscv-pk            [from https://github.com/riscv-software-src/riscv-pk]
    riscv-tools/riscv-gnu-toolchain [from https://github.com/riscv-collab/riscv-gnu-toolchain]

You need to build at least riscv-isa-sim (spike) because uspike links to libraries there.  RVTOOLS defaults to /opt/riscv-tools.

###  Getting the sources

The repository is on GitHub:

    $ git clone https://github.com/phaa-eu/cavatools

###  Installation

Environment variable CAVA is installation path, default to home directory $(HOME).  To build Cavatools:

    $ cd cavatools
    $ make install

The following files are installed:

    uspike, caveat, erised in $(CAVA)/bin/
    libcava.a in $(CAVA)/lib/
    several .h files in $(CAVA)/include/cava

Include $(CAVA)/bin in $(PATH) to run Cavatools programs.

###  Running Cavatools

Programs should be compiled -static using the Linux/glibc riscv-gnu-toolchain:

    $ riscv64-unknown-linux-gnu-gcc -static ... testpgm.c -o testpgm

To execute a RISC-V program without simulation:

    $ uspike testpgm <any number of flags and arguments to testpgm>

To see instruction execution performance in real time run in one window:

    $ caveat testpgm <any number of flags and arguments to testpgm>

and this in another window:

    $ erised testpgm

Shared memory segment /dev/shm/caveat (filename can be changed with option) containing counters is created.  Erised passively reads /dev/shm/caveat and displays performance data in its window, controlled interactively by mouse.

