## Sources for Windows 7

 * Current as of 2018-05-04
 * Maintained by @szmoore
 * From the future? Try [webarchive](https://archive.org/) but it probably won't have any of these.
 * We require C++11 (or at least C++0x) support, which causes issues with Python 2.7, but you might be able to get it to work
 * To compile we require the Anaconda fork of Python, but the compiled wheel will still work with default PyPi provided Python (of the correct version and architecture) on a windows system.
     * We use Anaconda because it supplies stable development libraries for python that actually *work* with visual studio
     * **WARNING** Do *not* update `conda` because it will break. `4.4.10` is the working version.
         * If you see a blank screen press Y and Enter. It appears to be a stdio bug on windows.
     



### Python2.7

 * **This is not tested because I gave up, but it will probably work if you compile with:**
     * `-DNO_THREADS`
 * [Anaconda2 32bit](https://repo.continuum.io/archive/Anaconda2-5.1.0-Windows-x86.exe)
     * `cd C:\ProgramData\Anaconda\Scripts && conda create -n py2.7 python=2.7 numpy pip mingw`
 * [Anaconda2 64bit](https://repo.continuum.io/archive/Anaconda2-5.1.0-Windows-x86_64.exe)
     * `cd C:\ProgramData\Anaconda-64\Scripts && conda create -n py2.7 python=2.7 numpy pip mingw`
 
 * [.NET 3.5](https://www.microsoft.com/en-au/download/details.aspx?id=21)(Good luck)
 * [Windows 7 SDK](https://download.microsoft.com/download/7/A/B/7ABD2203-C472-4036-8BA0-E505528CCCB7/winsdk_web.exe)
 * [Visual C++ 2008](http://download.microsoft.com/download/A/5/4/A54BADB6-9C3F-478D-8657-93B3FC9FE62D/vcsetup.exe)

###Python3.X

 * [Anaconda3 32bit](https://repo.continuum.io/archive/Anaconda3-5.1.0-Windows-x86.exe)
     * `cd C:\ProgramData\Anaconda3\Scripts && conda create -n py3.6 python=3.6 numpy pip mingw`
 * [Anaconda3 64bit](https://repo.continuum.io/archive/Anaconda3-5.1.0-Windows-x86_64.exe)
     * `cd C:\ProgramData\Anaconda3-64\Scripts && conda create -n py3.6 python=3.6 numpy pip mingw`
 * [.NET 4.0](https://download.microsoft.com/download/1/B/E/1BE39E79-7E39-46A3-96FF-047F95396215/dotNetFx40_Full_setup.exe)
     * On Windows 10 you can skip this, as .NET 4.7 is already installed
 * [Visual C++ 2015](https://my.visualstudio.com/downloads?q=visual%20studio%20community%202015)
     * Note you must include C++ language support in the installer setup menu because it's not installed by default
 * If you use `-DNOTHREADS` as with Python2.7 you can probably use 2010, which is what Python3.X is *meant* to be compiled with.
     * [Visual C++ 2010](http://download.microsoft.com/download/1/D/9/1D9A6C0E-FC89-43EE-9658-B9F0E3A76983/vc_web.exe)
 * [Windows 7 SDK](https://download.microsoft.com/download/A/6/A/A6AC035D-DA3F-4F0C-ADA4-37C8E5D34E3D/winsdk_web.exe)
 

#### WINDOZE TEN

 * You need Visual Studio 2015 because 2017 removes command line tools.
 * You can probably use VS 2017 and Windows 10 SDK but since they removed command line tools it's pretty shit to do from a batch script that can be put under source control...

 * To actually install the Windows 7 SDK and get 64 bit compilers to work, you need to edit registry values before installing it due to a bug, this is described [here](   https://stackoverflow.com/questions/32091593/cannot-install-windows-sdk-7-1-on-windows-10) but for completeness:
 
   * regdit the keys `HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\NET Framework Setup\NDP\v4\Client\Version`
     and `HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\NET Framework Setup\NDP\v4\Full\Version`
     to read `4.0.30319` then install. Then edit the keys back to whatever they were before `4.7.02556`

 * You may enouncter a bug with `rc.exe` not existing [see here](https://stackoverflow.com/questions/14372706/visual-studio-cant-build-due-to-rc-exe)
   * Install the Windows 8.1 Kit (not called SDK anymore?)
   * Add `C:\Program Files (x86)\Windows Kits\8.1\bin\x86` to path.


### OpenSSL
 * These now `404`, send me a letter and $200 if you want my saved copies...
    * Install them in `C:\OpenSSL-WinXX` (`32` and `64` respectively).
 * [32 bit](http://slproweb.com/download/Win32OpenSSL-1_1_0g.exe)
 * [64 bit](http://slproweb.com/download/Win64OpenSSL-1_1_0g.exe)
 
### zlib 
 * [Windows version](https://sourceforge.net/projects/gnuwin32/files/zlib/1.2.3/zlib-1.2.3-lib.zip/download?use_mirror=jaist&download=) - Extract all headers **and** the *static* libraries to `C:\zlib`

### libuv
  * [32 bit](https://dist.libuv.org/dist/v1.9.1/libuv-x86-v1.9.1.build10.exe) in `C:\Program Files (x86)\libuv`
  * [64 bit](https://dist.libuv.org/dist/v1.9.1/libuv-x64-v1.9.1.build10.exe) in `C:\Program Files\libuv`
