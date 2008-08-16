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


@rem use manifest to build installer
@"viewer_manifest.py" %1 %2 %3 %4 %5 %6 %7 %8

@rem pause

:END
