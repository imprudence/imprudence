@echo off

REM ============================================================
REM set_login_page
REM ============================================================
REM
REM Purpose:
REM
REM     Modify the login page (panel_login.xml) to point to a 
REM     specific URL  for a viewer build, e.g. a viewer pointing 
REM     at a beta grid.
REM 
REM Usage:
REM
REM     set_login_page.bat url
REM
REM Example:
REM
REM     set_login_page.bat http://secondlife.com/app/login/beta/
REM 
REM Notes:
REM 
REM     * Depends on branding\gawk.exe 
REM     * gawk.exe needs to be a Win32 executable (not Win16)
REM     * It's pretty dumb, and replaces the regular expression:
REM             http://secondlife.com/app/login/.*

set ORIGINAL=skins\xui\en-us\panel_login.xml
set TEMPFILE=.\panel_login.xml
set LOGINPAGEURL=%1

IF defined LOGINPAGEURL GOTO do_replace
goto end

:do_replace

echo Setting login page in %ORIGINAL% to "%LOGINPAGEURL%"

copy %ORIGINAL% %TEMPFILE%
branding\gawk.exe --assign=uri=%LOGINPAGEURL% "{ gsub( /http:\/\/secondlife\.com\/app\/login\/.*/, uri ); print; }" %TEMPFILE% > %ORIGINAL%

REM Leave tempfile in place so it can be restored
REM del %TEMPFILE%

:END
