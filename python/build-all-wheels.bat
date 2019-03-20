:: Based on https://gist.github.com/spacecowboy/23fcd4d40cfd1c1cd88a#file-build-all-wheels-bat, added Python 3.6
:: Credit to @spacecowboy http://cowboyprogrammer.org/building-python-wheels-for-windows/

:: Required: Visual Studio 10.0 Express, Visual Studio 9 Express,
::           Windows 7 SDK (.NET 4), Windows 7 SDK (.NET 3.5), Anaconda/Miniconda

:::::::::::::::::::::::::::::::::::
:: User configurable stuff start ::
:::::::::::::::::::::::::::::::::::
echo off
:: These depends on where you installed Anaconda.
:: Python2 or Python3 doesn't matter but you need both 32bit and 64bit versions.
set ANACONDA_BASE=C:\ProgramData\Anaconda2
set ANACONDA_BASE64=C:\ProgramData\Anaconda2-64

:: Location of your package, can be a directory or a git repo.

:: A directory
set PKG_REPO=C:\Local\Sam.Moore\uWebSockets-bindings\python

set WHEEL_DIR=%PKG_REPO%\dist

:: A git repo
:: set PKG_REPO=git+https://github.com/uNetworking/bindings.git

:::::::::::::::::::::::::::::::::
:: User configurable stuff end ::
:::::::::::::::::::::::::::::::::

:: Save original path so we can restore it
set BASEPATH=%PATH%

:: Set up your conda environments like this, do it for both 32bit and 64bit.
:: navigate to the Anaconda\Scripts / Anaconda-64\Scripts directory to avoid setting any Paths
:: conda create -n py3.6 python=3.6 numpy pip mingw
:: conda create -n py3.4 python=3.4 numpy pip mingw
:: conda create -n py3.3 python=3.3 numpy pip mingw
:: conda create -n py2.7 python=2.7 numpy pip mingw

:: These depend on what you named your environments above
set ENV27=envs\py2.7
set ENV33=envs\py3.3
set ENV34=envs\py3.4
set ENV36=envs\py3.6
set ENV37=envs\py3.7

:: cd %ANACONDA_BASE%
:: Use -p not -n to avoid paths messing with each other
:: Scripts\conda.exe create -p envs\py3.7 python=3.7 numpy pip mingw wheel ssl

:: Tell Python to select correct SDK version
set DISTUTILS_USE_SDK=1
set MSSdk=1

set ANACONDA_BASE=C:\ProgramData\Anaconda3
set ANACONDA_BASE64=C:\ProgramData\Anaconda3-64

set PATH=%BASEPATH%
:: Python3 requires Visual Studio 2010 compiler present (Express version is fine)
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.Cmd" /Release /x86
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
set CPATH=%PATH%
:: Python3.6 32bit
set PATH=%ANACONDA_BASE%\%ENV36%;%ANACONDA_BASE%\%ENV36%\Scripts;%CPATH%
echo "Python 3.6 32bit"
call :Build

:: 3.7
set PATH=%ANACONDA_BASE%\%ENV37%;%ANACONDA_BASE%\%ENV37%\Scripts;%CPATH%
echo "Python 3.7 32bit"
call :Build


:: Set 64-bit environment

call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.Cmd" /Release /x64
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
:: 64-bit cl.exe is here
set CPATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\x86_amd64;%CPATH%
:: Python3.6 64bit
set PATH=%ANACONDA_BASE64%\%ENV36%;%ANACONDA_BASE64%\%ENV36%\Scripts;%CPATH%;%PATH%
echo "Python 3.6 64bit"
call :Build


:: 3.7
set PATH=%ANACONDA_BASE64%\%ENV37%;%ANACONDA_BASE64%\%ENV37%\Scripts;%CPATH%
echo "Python 3.7 64bit"
call :Build



goto:END

:Build
set PATH=%PATH%;C:\Program Files (x86)\Windows Kits\8.1\bin\x86
color 07
python --version
pip --version
:: Display architecture for sanity purposes
python -c "import ctypes; print(ctypes.sizeof(ctypes.c_voidp)*8)"
color 07
:: And sys.executable
python -c "import sys; print(sys.executable)"
color 07
pip install wheel -q
color 07
pip wheel --no-deps %PKG_REPO% --wheel-dir %WHEEL_DIR%
color 07
python %PKG_REPO%\setup.py fix-wheel
color 07
EXIT /B 0

:END
:: pip messes with terminal colours
color 07
