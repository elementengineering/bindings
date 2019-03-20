import os
import shutil
import sys
import platform
import ctypes
arch = ctypes.sizeof(ctypes.c_voidp)*8


#from distutils.core import setup, Extension
from setuptools import setup, Extension

upstream_dir = os.path.join(os.path.dirname(__file__),"uWebSockets", "src")
if not os.path.exists(upstream_dir):
    shutil.copytree(os.path.join(os.path.dirname(__file__), "..", "uWebSockets"), os.path.join(os.path.dirname(__file__), "uWebSockets"))

upstream_src = [os.path.join(upstream_dir, f) for f in os.listdir(upstream_dir) if f.split(".")[-1] in ("cpp", "cxx", "c")]
upstream_headers = [os.path.join(upstream_dir, f) for f in os.listdir(upstream_dir) if f.split(".")[-1] in ("hpp", "hxx", "h")]

extra_objects = []
platform_link_args = []
platform_include_dirs = []
platform_cflags = []
platform_libraries = ["ssl", "crypto", "z", "uv"]
platform_defines = []
platform_package_data = {"uWebSockets":[]}

if platform.system().lower() == "linux":
    platform_defines = [("UWS_THREADSAFE", 1)] # FIXME: Does not work on Windoze
else:
    platform_defines = []
    


if platform.system().lower() == "windows":
    platform_libraries = []
    platform_include_dirs += [os.path.join(os.path.dirname(__file__),"windows")]
    platform_include_dirs += [os.path.join("C:\\","Program Files (x86)", "libuv", "include")]
    
    platform_include_dirs += [os.path.join("C:\\", "zlib")]
    
    platform_libraries += ["legacy_stdio_definitions", "legacy_stdio_wide_specifiers"]
    platform_libraries += ["crypt32", "User32", "GDI32", "Advapi32"]
    platform_link_args += ["/LIBPATH:"+os.path.join(os.path.dirname(sys.executable),"..","..","..","libs")]
    platform_libraries += ["python%d%d" % (sys.version_info[0], sys.version_info[1])]
    platform_libraries += ["python%d" % (sys.version_info[0],)]
    
    
    platform_libraries += ["zlibstat"]
    
    platform_package_data["uWebSockets"] += ["libuv.dll"]
    platform_libraries += ["libuv"]
    
    
    platform_link_args += ["/NODEFAULTLIB:libc"]
    if arch == 32:
        platform_include_dirs += [os.path.join("C:\\","OpenSSL-Win32","include")]
        platform_link_args += ["/LIBPATH:"+os.path.join("C:\\","OpenSSL-Win32","lib", "VC", "static")]
        platform_link_args += ["/LIBPATH:"+os.path.join("C:\\","zlib", "static32")]
        platform_link_args += ["/LIBPATH:"+os.path.join("C:\\","Program Files (x86)", "libuv")]
        platform_libraries += ["libssl32MT", "libcrypto32MT"]
        _external_dlls = [os.path.join("C:\\", "Program Files (x86)","libuv", "libuv.dll")]
    
    elif arch == 64:
        platform_include_dirs += [os.path.join("C:\\","OpenSSL-Win64","include")]
        platform_link_args += ["/LIBPATH:"+os.path.join("C:\\","OpenSSL-Win64","lib", "VC", "static")]
        platform_link_args += ["/LIBPATH:"+os.path.join("C:\\","zlib", "static_x64")]
        platform_link_args += ["/LIBPATH:"+os.path.join("C:\\","Program Files", "libuv")]
        platform_libraries += ["libssl64MT", "libcrypto64MT"]
        _external_dlls = [os.path.join("C:\\", "Program Files","libuv", "libuv.dll")]

    
    else:
        raise Exception("Unknown architecture %s" % repr(arch))
    
    platform_defines = [("MSVC",1), ("_WIN32", 1), ("ZLIB_WINAPI",1)]
    if sys.version_info[0] == 2:
        platform_defines += [("_NO_THREADS",1)]
    else:
        """
            We have to manually set these includes
        """
        platform_include_dirs += [os.path.join("C:\\","Program Files (x86)","Microsoft Visual Studio 14.0", "VC", "INCLUDE")]
        platform_include_dirs += [os.path.join("C:\\","Program Files (x86)","Microsoft Visual Studio 14.0", "VC", "ATLMFC", "INCLUDE")]
        platform_include_dirs += [os.path.join("C:\\","Program Files (x86)","Windows Kits", "10", "INCLUDE", "10.0.10240.0", "ucrt")]
    platform_cflags += ["--std=c++0x"]

if os.environ.get("MANYLINUX",None) is not None:
    platform_libraries = []
    # To satisfy manylinux1 tag, we need to jump through a lot of hoops; see docker/manylinux
    platform_link_args = [
        # Don't link with standard library, the script in docker/manylinux/ld-patch.sh will add the correct flags for that.
        "-nostdlib",
        "-Wl,--add-needed",
        "-Wl,--demangle",
        "-Wl,-Bstatic",
        "-lz",
        "-luv",
        "-L/opt/openssl/lib/",
        "-lssl",
        "-lcrypto",
        
        
        "/glibc-glibc-2.10/build/elf/soinit.os",
        "-Wl,-z,muldefs",
        "-Wl,--gc-sections",
        "-Wl,-fini=__wrap_fini",
        "-Wl,--wrap=atexit",
        "-Wl,-z,initfirst",
        "/glibc-glibc-2.10/build/libc_pic.a",
        "-Wl,--no-gc-sections",
        
        "-Wl,--default-symver",
        "-Wl,--version-script=/vagrant/manylinux.map",
        "-Wl,-Map=/vagrant/uWebSockets.map",
        "-Wl,--cref",
        
        "-Wl,-Bdynamic",

        "/opt/rh/devtoolset-2/root/usr/lib/gcc/x86_64-CentOS-linux/4.8.2/libgcc_eh.a",
        "/opt/rh/devtoolset-2/root/usr/lib/gcc/x86_64-CentOS-linux/4.8.2/libstdc++.a",        

    ]
    platform_include_dirs = ["/opt/glibc2.10/include", "/opt/openssl/include/"]
    platform_cflags = ["-fPIC", "-g", "-Og"]
    platform_defines += [("MANYLINUX",1)] # ("DEBUG_PYTHON_BINDINGS", 1)]


uWebSockets = Extension("uWebSockets", 
    sources=["Bindings.cpp"] + upstream_src,
    include_dirs=[upstream_dir] + platform_include_dirs,
    libraries=platform_libraries,
    #define_macros=[("UWS_THREADSAFE", 1), ("PYTHON_FLAVOUR",sys.version_info[0])], ## FIXME: UWS_THREADSAFE unsupported on Windows and OSX
    define_macros=[("PYTHON_FLAVOUR", sys.version_info[0]), ("__STDC_LIMIT_MACROS",1)] + platform_defines,
    extra_compile_args=["-std=c++11"]+platform_cflags, # FIXME: Won't work on older compilers
    extra_link_args=platform_link_args,
    extra_objects = extra_objects
)

version="0.14.8a1"

"""
Windoze Hack - Add dlls
"""
if platform.system().lower() == "windows" and "fix-wheel" in sys.argv:
    whl = os.path.join(os.path.dirname(__file__), "dist", "uWebSockets-%s-cp%d%d-cp%d%dm-win%s.whl" % (version, sys.version_info[0],sys.version_info[1],sys.version_info[0],sys.version_info[1],"32" if arch==32 else "_amd64"))
    print("wheel in %s" % repr(whl))
    assert(os.path.exists(whl))
    import zipfile
    with zipfile.ZipFile(whl,"a") as z:
        for dll in _external_dlls:
            print("Add %s" % dll)
            z.write(dll, os.path.basename(dll))
        z.printdir()
        z.testzip()
        z.close()
    sys.exit(0)

setup(name="uWebSockets",
    version=version,
    description="Python Bindings for the uWebSockets library",
    url="https://github.com/elementengineering/uWebSockets-bindings/",
    author="Sam Moore",
    author_email="smoore@elementengineering.com.au",
    license="MIT",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Topic :: Software Development",
        "License :: OSI Approved :: MIT License",

        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7"
    ], 
    keywords="websockets development",
    ext_modules =[uWebSockets],
    test_suite='nose.collector',
    tests_require=['nose'],
    include_package_data=True,
    package_data=platform_package_data
)

