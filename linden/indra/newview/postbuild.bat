@echo off

if %1==debug goto debug
if %1==release goto release
if %1==releasenoopt goto releasenoopt
if %1==releasefordownload goto releasefordownload

echo ** unknown configuration **
goto end

:debug
echo copying debug files
if exist .\debug\freebl3.dll goto end
copy ..\..\libraries\i686-win32\lib_debug\freebl3.dll			.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\gksvggdiplus.dll		.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\js3250.dll			.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\nspr4.dll				.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\nss3.dll				.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\nssckbi.dll			.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\plc4.dll				.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\plds4.dll				.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\smime3.dll			.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\softokn3.dll			.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\ssl3.dll				.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\xpcom.dll				.\debug\ /y
copy ..\..\libraries\i686-win32\lib_debug\xul.dll				.\debug\ /y
rem --- this is required for mozilla debug builds and displays the aborty/retry/ignore dialog on an assert - crashes without it ---
copy ..\..\libraries\i686-win32\lib_debug\windbgdlg.exe			.\debug\ /y

@IF NOT EXIST ..\llkdu\Debug\llkdu.dll (
	@IF EXIST ..\..\libraries\i686-win32\lib_debug\llkdu.dll (
		SET KDU_DLL=..\..\libraries\i686-win32\lib_debug\llkdu.dll
	) ELSE (
		SET KDU_DLL=..\..\libraries\i686-win32\lib_release\llkdu.dll
	)
) ELSE (
	SET KDU_DLL=..\llkdu\Debug\llkdu.dll
)
copy %KDU_DLL%  .\debug\ /y
goto end

:release
echo copying release files
if exist .\Release\freebl3.dll goto end
copy ..\..\libraries\i686-win32\lib_release\freebl3.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\gksvggdiplus.dll	.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\js3250.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\nspr4.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\nss3.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\nssckbi.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\plc4.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\plds4.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\smime3.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\softokn3.dll		.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\ssl3.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\xpcom.dll			.\Release\ /y
copy ..\..\libraries\i686-win32\lib_release\xul.dll				.\Release\ /y

@IF NOT EXIST ..\llkdu\Release\llkdu.dll (
	copy ..\..\libraries\i686-win32\lib_release\llkdu.dll 		.\Release\ /y
) ELSE (
	copy ..\llkdu\Release\llkdu.dll .\Release\ /y
)
goto end

:releasenoopt
echo copying releasenoopt files
if exist .\ReleaseNoOpt\freebl3.dll goto end
copy ..\..\libraries\i686-win32\lib_release\freebl3.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\gksvggdiplus.dll	.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\js3250.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\nspr4.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\nss3.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\nssckbi.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\plc4.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\plds4.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\smime3.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\softokn3.dll		.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\ssl3.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\xpcom.dll			.\ReleaseNoOpt\ /y
copy ..\..\libraries\i686-win32\lib_release\xul.dll				.\ReleaseNoOpt\ /y

@IF NOT EXIST ..\llkdu\ReleaseNoOpt\llkdu.dll (
	copy ..\..\libraries\i686-win32\lib_release\llkdu.dll		.\ReleaseNoOpt\ /y
) ELSE (
  	copy ..\llkdu\ReleaseNoOpt\llkdu.dll .\ReleaseNoOpt\ /y
)
goto end

:releasefordownload
echo copying releasefordownload files
if exist .\ReleaseForDownload\freebl3.dll goto end
copy ..\..\libraries\i686-win32\lib_release\freebl3.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\gksvggdiplus.dll	.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\js3250.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\nspr4.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\nss3.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\nssckbi.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\plc4.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\plds4.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\smime3.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\softokn3.dll		.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\ssl3.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\xpcom.dll			.\ReleaseForDownload\ /y
copy ..\..\libraries\i686-win32\lib_release\xul.dll				.\ReleaseForDownload\ /y
@IF NOT EXIST ..\llkdu\Release\llkdu.dll (
	copy ..\..\libraries\i686-win32\lib_release\llkdu.dll		.\ReleaseForDownload\ /y
) ELSE (
	copy ..\llkdu\Release\llkdu.dll .\ReleaseForDownload\ /y
)

goto end

:end
