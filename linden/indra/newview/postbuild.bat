@echo off

if %1==debug goto debug
if %1==release goto release
if %1==releasenoopt goto releasenoopt
if %1==releasefordownload goto releasefordownload

echo ** unknown configuration **
goto end

:debug
echo copying debug files
xcopy ..\..\libraries\i686-win32\lib_debug\freebl3.dll			.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\gksvggdiplus.dll		.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\js3250.dll			.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\nspr4.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\nss3.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\nssckbi.dll			.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\plc4.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\plds4.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\smime3.dll			.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\softokn3.dll			.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\ssl3.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\xpcom.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\xul.dll				.\debug\ /y
xcopy ..\..\libraries\i686-win32\lib_debug\openjpeg.dll		.\debug\ /y
rem --- this is required for mozilla debug builds and displays the aborty/retry/ignore dialog on an assert - crashes without it ---
xcopy ..\..\libraries\i686-win32\lib_debug\windbgdlg.exe			.\debug\ /y

rem --- runtime pieces for the bHear stuff.
xcopy .\vivox-runtime\i686-win32\tntk.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\libeay32.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\SLVoice.exe	.\ /y
xcopy .\vivox-runtime\i686-win32\ssleay32.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\SLVoiceAgent.exe	.\ /y
xcopy .\vivox-runtime\i686-win32\srtp.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\alut.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\vivoxsdk.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\ortp.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\wrap_oal.dll	.\ /y

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
xcopy ..\..\libraries\i686-win32\lib_release\freebl3.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\gksvggdiplus.dll	.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\js3250.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nspr4.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nss3.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nssckbi.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\plc4.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\plds4.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\smime3.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\softokn3.dll		.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\ssl3.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\xpcom.dll			.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\xul.dll				.\Release\ /y
xcopy ..\..\libraries\i686-win32\lib_release\openjpeg.dll		.\Release\ /y

rem --- runtime pieces for the bHear stuff.
xcopy .\vivox-runtime\i686-win32\tntk.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\libeay32.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\SLVoice.exe	.\ /y
xcopy .\vivox-runtime\i686-win32\ssleay32.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\SLVoiceAgent.exe	.\ /y
xcopy .\vivox-runtime\i686-win32\srtp.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\alut.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\vivoxsdk.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\ortp.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\wrap_oal.dll	.\ /y

@IF NOT EXIST ..\llkdu\Release\llkdu.dll (
	xcopy ..\..\libraries\i686-win32\lib_release\llkdu.dll 		.\Release\ /y
) ELSE (
	xcopy ..\llkdu\Release\llkdu.dll .\Release\ /y
)
goto end

:releasenoopt
echo copying releasenoopt files
xcopy ..\..\libraries\i686-win32\lib_release\freebl3.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\gksvggdiplus.dll	.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\js3250.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nspr4.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nss3.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nssckbi.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\plc4.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\plds4.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\smime3.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\softokn3.dll		.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\ssl3.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\xpcom.dll			.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\xul.dll				.\ReleaseNoOpt\ /y
xcopy ..\..\libraries\i686-win32\lib_release\openjpeg.dll		.\ReleaseNoOpt\ /y

rem --- runtime pieces for the bHear stuff.
xcopy .\vivox-runtime\i686-win32\tntk.dll	. /y
xcopy .\vivox-runtime\i686-win32\libeay32.dll	. /y
xcopy .\vivox-runtime\i686-win32\SLVoice.exe	. /y
xcopy .\vivox-runtime\i686-win32\ssleay32.dll	. /y
xcopy .\vivox-runtime\i686-win32\SLVoiceAgent.exe	. /y
xcopy .\vivox-runtime\i686-win32\srtp.dll	. /y
xcopy .\vivox-runtime\i686-win32\alut.dll	. /y
xcopy .\vivox-runtime\i686-win32\vivoxsdk.dll	. /y
xcopy .\vivox-runtime\i686-win32\ortp.dll	. /y
xcopy .\vivox-runtime\i686-win32\wrap_oal.dll	. /y

@IF NOT EXIST ..\llkdu\ReleaseNoOpt\llkdu.dll (
	xcopy ..\..\libraries\i686-win32\lib_release\llkdu.dll		.\ReleaseNoOpt\ /y
) ELSE (
  	xcopy ..\llkdu\ReleaseNoOpt\llkdu.dll .\ReleaseNoOpt\ /y
)
goto end

:releasefordownload
echo copying releasefordownload files
xcopy ..\..\libraries\i686-win32\lib_release\freebl3.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\gksvggdiplus.dll	.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\js3250.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nspr4.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nss3.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\nssckbi.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\plc4.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\plds4.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\smime3.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\softokn3.dll		.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\ssl3.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\xpcom.dll			.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\xul.dll				.\ReleaseForDownload\ /y
xcopy ..\..\libraries\i686-win32\lib_release\openjpeg.dll		.\ReleaseForDownload\ /y
rem --- runtime pieces for the bHear stuff.
xcopy .\vivox-runtime\i686-win32\tntk.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\libeay32.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\SLVoice.exe	.\ /y
xcopy .\vivox-runtime\i686-win32\ssleay32.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\SLVoiceAgent.exe	.\ /y
xcopy .\vivox-runtime\i686-win32\srtp.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\alut.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\vivoxsdk.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\ortp.dll	.\ /y
xcopy .\vivox-runtime\i686-win32\wrap_oal.dll	.\ /y

@IF NOT EXIST ..\llkdu\Release\llkdu.dll (
	xcopy ..\..\libraries\i686-win32\lib_release\llkdu.dll		.\ReleaseForDownload\ /y
) ELSE (
	xcopy ..\llkdu\Release\llkdu.dll .\ReleaseForDownload\ /y
)

goto end
:BuildFailed
echo POSTBUILD FAILED
exit 1
:end
echo POSTBUILD SUCCESSFUL

:end
