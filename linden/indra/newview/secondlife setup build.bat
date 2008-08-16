@REM Invoke the installer-generation process after
@REM creating and modifying files necessary for pointing
@REM the viewer at different grids (production preview, etc)
@REM
@REM Usage:
@REM
@REM     "secondlife setup build.bat" login-page-url installer-args
@REM
@REM Examples:
@REM
@REM     "secondlife setup build.bat" http://secondlife.com/app/login/
@REM     "secondlife setup build.bat" http://secondlife.com/app/login/beta/ /DADITI 
@REM 

@IF EXIST ReleaseForDownload\Secondlife.exe GOTO RELEASE_EXE
@IF EXIST Secondlife.exe GOTO LOCAL_EXE
@IF EXIST ReleaseNoOpt\newview_noopt.exe GOTO RELEASE_NOOPT_EXE

@echo Could not find Secondlife.exe!
@pause
exit 0

:RELEASE_NOOPT_EXE
@echo Cound not find SecondLife.exe.
@echo Using newview_noopt.exe - DEVELOPMENT BUILD!
@pause
SET EXEFILE=ReleaseNoOpt\newview_noopt.exe
SET KDUDLL=ReleaseNoOpt\llkdu.dll
GOTO CONTINUE

:RELEASE_EXE
SET EXEFILE=ReleaseForDownload\Secondlife.exe
SET KDUDLL=ReleaseForDownload\llkdu.dll
GOTO CONTINUE

:LOCAL_EXE
SET EXEFILE=Secondlife.exe
SET KDUDLL=..\..\libraries\i686-win32\lib_release\llkdu.dll
:CONTINUE

@rem Set login page to reflect beta grid status
call "set_login_page.bat" %1

@rem Extract version information.
branding\ResHacker -extract %EXEFILE%, branding\version.rc, versioninfo, 1,

@rem Process version information.
cd branding
grep FILEVERSION version.rc | gawk "{ print $2; }" > version.txt
gawk -F , "{ print \"!define VERSION_MAJOR \" $1; }" version.txt > version.include
gawk -F , "{ print \"!define VERSION_MINOR \" $2; }" version.txt >> version.include
gawk -F , "{ print \"!define VERSION_PATCH \" $3; }" version.txt >> version.include
gawk -F , "{ print \"!define VERSION_BUILD \" $4; }" version.txt >> version.include
echo !define EXE_LOCATION %EXEFILE% >> version.include
echo !define KDU_DLL %KDUDLL% >> version.include

cd ..
@rem Build installer
"C:\Program Files\NSIS\makensis" %2 %3 %4 %5 %6 %7 %8 %9 "secondlife setup.nsi"

@rem Clean up
del branding\version.include
del branding\version.rc
del branding\version.txt
del branding\ResHacker.log
del branding\ResHacker.ini

@rem Restore login page changes to avoid accidental commits
call "restore_login_page.bat"



@rem pause

:END
