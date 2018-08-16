Dockerfile based on PyPi manylinux https://github.com/pypa/manylinux

manylinux1 is the PEP for portable linux python wheels.

*Unfortunately* it is currently based on a Centos/5 build environment.
It is likely that sooner or later, manylinux2 will come out, but until then, a number of hacks are required.

This took a while to work out.

Here's a brief description of issues with work arounds:

1. `openssl-devel` provides ancient openssl which results in undefined symbols
 * We download a newer openssl source, BUT: this requires Perl 5.10.0 to configure...
 * We download Perl 5.12 source and compile it first, then we can use that perl to...
 * Compile openssl from source, so we can link dynamically. This gives us the symbols and `auditwheel` will work out the rest.
   * (Apparently PyPi does this too on various `manylinux` wheels but they keep it a secret)
 
2. `libuv-devel` does not exist
 * We download and compile from source (1.1.0) , this surprisingly Just Works

3. `g++` does not support `--std=c++11` 
 * Save some time and fix 4. first (using the existing Centos/5 toolchain), then come back to this.
 * Install `devtools-2` which gives us a newer g++ and gcc in `/opt`
 * Fool the devtools compilers by replacing their `ld` executable (!)
   * The devtools compilers pass an option (`--build-info`) which the system default doesn't understand, so
     we actually replace their ld with a wrapper script (!) which seds out (!!) the invalid argument (!!!)
     then calls the system default ld (!!!!)

4. Centos/5 glibc doesn't contain `eventfd` but nothing newer is supported by `auditwheel`
 * We are restricted to GLIB 2.5 or earlier
 * But nobody says we can't statically link with a newer glibc...
  * After reviewing commit history (and trying some versions), glibc 2.10 seems the safest
  * Compile glibc 2.10 with the original (system default) gcc

5. Various Linker Hacks
 * At this point we have three libc's on the system so understandably things get confusing
 * (setup.py)[../../setup.py] uses environment variable `MANYLINUX` to set a bunch of hacks.
 * (The linker wrapper script)[./ld-patch.sh] does more hacks
  1. The devtools-2 compiler tries to use its own crtbegin/crtend/libc/libgcc so replace those with the system defaults
     If this is not done, the library loads properly, but `auditwheel` detects external references to `GLIBC_PRIVATE`
  2. Statically link to the devtools-2 version of libstd++
  4. We need PIC glibc2.10 to statically link; fortunately this is an intermediate step so is left over in the build directory
  5. PIC glibc2.10 leaves out a symbol which is in soinit.so, this is left in the build directory so pull it in
  6. glibc2.10 causes multiple definitions from crti.o, so use the `-z muldefs` flag to discard repeated definitions (!!!!)
  7. Due to our hack in 1. (or possibly 6 breaking something in crtend/crtn) we now get a segmentation fault on library _fini
     So, in `Bindings.cpp` we have a `MANYLINUX` define (set from (setup.py)[../../setup.py]) and we override the default `_fini` with it.
     * This `__wrap_fini` does absolutely nothing and no cleanup.
       * This is probably bad and is probably going to cause kernel memory leaks or some bizarre bugs
  8. Statically link libssl, lz, luv, etcetera.
     Debatedly we shouldn't do this but in the spirit of `manylinux` "will work on any linux" we want it to run on linux without installing a bunch of development libraries.
     Especially since I just pulled the newest available libssl due to security concerns with older versions
  
  
 

