@echo off

REM ============================================================
REM restore_login_page
REM ============================================================
REM
REM Purpose:
REM
REM     Undo the work of set_login_page.
REM 

set ORIGINAL=skins\xui\en-us\panel_login.xml
set TEMPFILE=.\panel_login.xml

if not exist %TEMPFILE% goto end

echo Restoring login page in %ORIGINAL% 

copy /Y %TEMPFILE% %ORIGINAL% 
del %TEMPFILE%

:END
