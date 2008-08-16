@echo off

rem -- Check current message template against the master

"../../scripts/template_verifier.py" --mode="development"  --cache_master

if errorlevel 1 goto BuildFailed
goto end

:BuildFailed
echo PREBUILD FAILED
exit 1

:end
echo PREBUILD SUCCESSFUL
