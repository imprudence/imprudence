@rem Invoke the script which preps then runs the installer.
@rem This batch file is customized per grid.

set LONGPAGEURL=http://secondlife.com/app/login/beta/
set ARGS=/DGANGA

"secondlife setup build.bat" %LONGPAGEURL% %ARGS%
