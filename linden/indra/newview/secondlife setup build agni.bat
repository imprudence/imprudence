@rem Invoke the script which preps then runs the installer.
@rem This batch file is customized per grid.

set LOGINPAGEURL=http://secondlife.com/app/login/
set ARGS=

@rem To build optional "update" installer (doesn't have static VFS):
@rem 
@rem set ARGS=/DUPDATE

@"secondlife setup build.bat" %LOGINPAGEURL% %ARGS%
