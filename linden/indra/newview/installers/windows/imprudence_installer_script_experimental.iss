; Imprudence inno setup installer script by McCabe Maxsted
; This script only works with VS2005, currently

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)

; These will change
AppId={{1B3E68BC-13EB-4277-9439-CB5FF9259460}
AppName=Imprudence Viewer Experimental
AppVerName=Imprudence Viewer 1.4.0.3 exp 1 windows test release
DefaultDirName={pf}\ImprudenceExperimental
DefaultGroupName=Imprudence Viewer Experimental
VersionInfoProductName=Imprudence Viewer Experimental
OutputBaseFilename=Imprudence-1.4.0.3-exp-1-windows-test
VersionInfoVersion=1.4.0.3
VersionInfoTextVersion=1.4.0.3
VersionInfoProductVersion=1.4.0.3
AppVersion=1.4.0.3
VersionInfoCopyright=2011

; These won't change
VersionInfoCompany=Imprudence
AppPublisher=The Imprudence Project
AppPublisherURL=http://kokuaviewer.org
AppSupportURL=http://kokuaviewer.org
AllowNoIcons=true
InfoAfterFile=..\..\..\..\..\README.txt
OutputDir=C:\imprudence_installers
SetupIconFile=..\windows\imp_icon.ico
Compression=lzma2/ultra64
InternalCompressLevel=ultra64
SolidCompression=true
PrivilegesRequired=poweruser
AllowRootDirectory=true
WizardImageFile=..\windows\imprudence_installer_icon_left.bmp
WizardSmallImageFile=..\windows\imprudence_installer_icon_right.bmp
SetupLogging=true
RestartIfNeededByRun=false
AlwaysRestart=false

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
Name: slurlassociate; Description: Associate Imprudence with SLURLs; GroupDescription: Associations:; Languages: ; Flags: checkedonce
; TODO: use scripting for something like this on uninstall:
; Name: uninstallsettings; Description: Remove user settings; Flags: checkablealone; Languages: ; GroupDescription: Uninstall:

[Files]
Source: ..\..\..\build-vc80\newview\release\package\imprudence.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\character\*; DestDir: {app}\character; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\fonts\*; DestDir: {app}\fonts; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\app_settings\*; DestDir: {app}\app_settings; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\skins\*; DestDir: {app}\skins; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\doc\*; DestDir: {app}\doc; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\llplugin\*; DestDir: {app}\llplugin; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\alut.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\ChangeLog.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\dbghelp.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\featuretable.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\gpu_table.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\imprudence.url; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libapr-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libapriconv-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libaprutil-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libhunspell.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\llcommon.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\llkdu.dll.2.config; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\openal32.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\OpenJPEG.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\SLPlugin.exe; DestDir: {app}; Flags: ignoreversion

; Gstreamer-specific files below
Source: ..\..\..\build-vc80\newview\release\package\lib\*; DestDir: {app}\lib; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\..\..\build-vc80\newview\release\package\avcodec-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\avdevice-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\avfilter-gpl-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\avformat-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\avutil-gpl-50.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\iconv.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\liba52-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libbz2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libcelt-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libdca-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libexpat-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libfaad-2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libFLAC-8.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgcrypt-11.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgio-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libglib-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgmodule-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgnutls-26.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgobject-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgpg-error-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstapp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstaudio-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstbase-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstcontroller-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstdataprotocol-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstfft-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstinterfaces-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstnet-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstnetbuffer-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstpbutils-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstphotography-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstreamer-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstriff-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstrtp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstrtsp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstsdp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstsignalprocessor-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgsttag-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgstvideo-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libgthread-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libmms-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libmpeg2-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libneon-27.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libogg-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\liboil-0.3-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libsoup-2.4-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libtasn1-3.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libtheora-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libtheoradec-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libvorbis-0.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libvorbisenc-2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libvorbisfile-3.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libwavpack-1.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libx264-67.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libxml2-2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\libxml2.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\SDL.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\xvidcore.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\z.dll; DestDir: {app}; Flags: ignoreversion

; Voice files
Source: ..\..\..\build-vc80\newview\release\package\ortp.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\SLVoice.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\vivoxsdk.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\..\..\build-vc80\newview\release\package\wrap_oal.dll; DestDir: {app}; Flags: ignoreversion

; VC++ 2005 SP1 x86, VC++ 2008 SP1 x86, and VC++ 2010 SP1 x86 redist
Source: ..\windows\vcredist_x86_VS2005_SP1_MFC_SEC.exe; DestDir: {app}\redist; DestName: vcredist_x86_VS2005_SP1_MFC_SEC.exe
;Source: ..\windows\vcredist_x86_VS2008_SP1_ATL_SEC.exe; DestDir: {app}\redist; DestName: vcredist_x86_VS2008_SP1_ATL_SEC.exe
Source: ..\windows\vcredist_x86_VS2010_SP1.exe; DestDir: {app}\redist; DestName: vcredist_x86_VS2010_SP1.exe

; Old files we don't use anymore:
; Source: ..\..\..\build-vc80\newview\release\package\dronesettings.xml; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\package\volume_settings.xml; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\package\srtp.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\package\ssleay32.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\package\tntk.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\package\libeay32.dll; DestDir: {app}; Flags: ignoreversion
; Source: ..\..\..\build-vc80\newview\release\package\lsl_guide.html; DestDir: {app}; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files
;Source: ..\..\..\build-vc80\newview\release\package\msvcr71.dll; DestDir: {app}; Flags: ignoreversion; MinVersion: 0,6.01; Tasks: ; Languages:

[Registry]
Root: HKCR; Subkey: secondlife; ValueType: string; Flags: uninsdeletekey deletekey; Tasks: slurlassociate; ValueName: (default); ValueData: URL:Second Life
Root: HKCR; Subkey: secondlife; ValueType: string; Flags: uninsdeletekey deletekey; Tasks: slurlassociate; ValueName: URL Protocol
Root: HKCR; Subkey: secondlife\DefaultIcon; Flags: uninsdeletekey deletekey; ValueType: string; Tasks: slurlassociate; ValueData: {app}\imprudence.exe
Root: HKCR; Subkey: secondlife\shell\open\command; ValueType: expandsz; Flags: uninsdeletekey deletekey; Tasks: slurlassociate; ValueData: "{app}\imprudence.exe --settings settings_imprudence_experimental.xml -url ""%1"""; Languages: 
; Root: HKCU; Subkey: Environment; ValueType: string; ValueName: GST_PLUGIN_PATH; Flags: deletevalue uninsdeletevalue; ValueData: {app}\lib
; Root: HKCU; Subkey: Environment; ValueType: expandsz; ValueName: PATH; ValueData: {app}

[Icons]
Name: {group}\{cm:UninstallProgram,Imprudence Experimental}; Filename: {uninstallexe}
Name: {commondesktop}\Imprudence Experimental; Filename: {app}\imprudence.exe; Tasks: desktopicon; WorkingDir: {app}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Imprudence Experimental; Filename: {app}\imprudence.exe; Tasks: quicklaunchicon; WorkingDir: {app}
Name: {group}\Imprudence Experimental; Filename: {app}\imprudence.exe; WorkingDir: {app}; Comment: imprudence; IconIndex: 0

[Run]
Filename: {app}\imprudence.exe; WorkingDir: {app}; Flags: nowait postinstall
Filename: {app}\imprudence.url; WorkingDir: {app}; Flags: nowait postinstall shellexec; Description: See what makes Imprudence different

; Install redistributables. 
;
;     !!!!BEWARE!!!! 
;
; Command line parameters and filenames WILL change with each version. Blame Microsoft.

; Always use /q for VS2005 rather than something quieter such as Parameters: "/q:a c:""msiexec /i vcredist.msi /qn"" ". The redist will fail sometimes if you do otherwise.
Filename: {app}\redist\vcredist_x86_VS2005_SP1_MFC_SEC.exe; Parameters: "/q"; Check: Needs2005Redist; Flags: runhidden
;Filename: {app}\redist\vcredist_x86_VS2008_SP1_ATL_SEC.exe; Parameters: "/q"; Check: Needs2008Redist; Flags: runhidden
Filename: {app}\redist\vcredist_x86_VS2010_SP1.exe; Parameters: "/q /norestart"; Check: Needs2010Redist; Flags: runhidden

[UninstallDelete]
Name: {userappdata}\Imprudence\user_settings\password.dat; Type: files; Languages: 
Name: {userappdata}\Imprudence\user_settings\settings.xml; Type: files; Languages: 
Name: {userappdata}\Imprudence\user_settings\settings_imprudence_experimental.xml; Type: files; Languages: 
; 1.2 and lower cache location:
Name: {userappdata}\Imprudence\cache; Type: filesandordirs
; 1.3 and higher cache location:
Name: {localappdata}\Imprudence\cache; Type: filesandordirs
Name: {userappdata}\Imprudence\logs; Type: filesandordirs
Name: {userappdata}\Imprudence\browser_profile; Type: filesandordirs
Name: C:\Users\{username}\.gstreamer-0.10; Type: filesandordirs
Name: C:\Documents and Settings\{username}\.gstreamer-0.10; Type: filesandordirs

[InstallDelete]
; Name: {app}\*.dll; Type: files; Tasks: ; Languages:
Name: {app}\lib\gstreamer-plugins\*; Type: filesandordirs; Tasks: ; Languages: 
; Name: {app}\skins\default\xui\*; Type: filesandordirs; Tasks: ; Languages:
; Old xui skin files can cause bugs, always kill them
Name: {app}\skins\silver\xui\en-us\*; Type: filesandordirs; Tasks: ; Languages:
Name: {app}\app_settings\mozilla; Type: filesandordirs; Tasks: ; Languages:
Name: {app}\app_settings\mozilla_debug; Type: filesandordirs; Tasks: ; Languages:
Name: {app}\app_settings\viewerversion.xml; Type: filesandordirs; Tasks: ; Languages:
Name: C:\Documents and Settings\{username}\.gstreamer-0.10\*; Type: filesandordirs
Name: C:\Users\{username}\.gstreamer-0.10\*; Type: filesandordirs
; Breaks the browser if installing on top of 1.1:
Name: {app}\gksvggdiplus.dll; Type: files; Tasks: ; Languages: 

; Pre-plugin files:
Name: {app}\charset.dll; Type: files; Tasks: ; Languages: 
Name: {app}\freebl3.dll; Type: files; Tasks: ; Languages: 
Name: {app}\glew32.dll; Type: files; Tasks: ; Languages: 
Name: {app}\iconv.dll; Type: files; Tasks: ; Languages: 
Name: {app}\intl.dll; Type: files; Tasks: ; Languages: 
Name: {app}\js3250.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libcairo-2.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libfaad-2.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgcrypt-11.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgio-2.0-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libglib-2.0-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgmodule-2.0-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgnutls-26.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgobject-2.0-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgpg-error-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstapp.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstaudio.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstaudio-0.10.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstbase-0.10.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstcdda.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstcontroller-0.10.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstdataprotocol-0.10.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstdshow.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstfft.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstinterfaces.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstnet-0.10.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstnetbuffer.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstpbutils.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstreamer-0.10.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstriff.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstrtp.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstrtsp.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstsdp.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgsttag.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgstvideo.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libgthread-2.0-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libjpeg.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libmp3lame-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libneon-27.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libogg-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\liboil-0.3-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libopenjpeg-2.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libpng12-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libschroedinger-1.0-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libspeex-1.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libtheora-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libvorbis-0.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libvorbisenc-2.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libxml2-2.dll; Type: files; Tasks: ; Languages: 
Name: {app}\libxml2.dll; Type: files; Tasks: ; Languages: 
Name: {app}\nspr4.dll; Type: files; Tasks: ; Languages: 
Name: {app}\nss3.dll; Type: files; Tasks: ; Languages: 
Name: {app}\nssckbi.dll; Type: files; Tasks: ; Languages: 
Name: {app}\plc4.dll; Type: files; Tasks: ; Languages: 
Name: {app}\plds4.dll; Type: files; Tasks: ; Languages: 
Name: {app}\RELEASE_NOTES.txt; Type: files; Tasks: ; Languages: 
Name: {app}\smime3.dll; Type: files; Tasks: ; Languages: 
Name: {app}\softokn3.dll; Type: files; Tasks: ; Languages: 
Name: {app}\ssl3.dll; Type: files; Tasks: ; Languages: 
Name: {app}\xpcom.dll; Type: files; Tasks: ; Languages: 
Name: {app}\xul.dll; Type: files; Tasks: ; Languages: 
Name: {app}\xvidcore.dll; Type: files; Tasks: ; Languages: 
Name: {app}\zlib1.dll; Type: files; Tasks: ; Languages: 

; We don't distribute the CRT like this anymore; kill old files
Name: {app}\SLPlugin.exe.config; Type: files; Tasks: ; Languages:
Name: {app}\Microsoft.VC80.CRT.manifest; Type: files; Tasks: ; Languages:
Name: {app}\msvcp80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr71.dll; Type: files; Tasks: ; Languages:
Name: {app}\imprudence.exe.config; Type: files; Tasks: ; Languages:


[Code]
// [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86] 
//   Installed = 1 (REG_DWORD)
function IsVS2010RedistInstalled(): Boolean;
var
  V: Cardinal;
  Success: Boolean;
begin
  if IsWin64 then begin
    Success := RegQueryDWordValue(HKLM64, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86', 'Installed', V);
  end else begin
    Success := RegQueryDWordValue(HKLM, 'SOFTWARE\Microsoft\VisualStudio\10.0\VC\VCRedist\x86', 'Installed', V);
  end

  if Success = TRUE then begin
    if V = 1 then begin
      Result := TRUE;
    end else begin
      Result := FALSE;
    end
  end else begin
    Result := FALSE;
  end
end;

function Needs2010Redist(): Boolean;
begin
  Result := (IsVS2010RedistInstalled = FALSE);
  if (Result = TRUE) then begin
    Log('User system needs VS 2010 SP1 x86 Redistributable, installing.');
  end else begin
    Log('User already has VS 2010 SP1 x86 Redistributable installed, skipping.');
  end
end;

// VS2008 and 2005 x86 redists. Always look for the latest version we know about. I wish there were a better way to check for these
const
  VS2005_X86 =              '{A49F249F-0C91-497F-86DF-B2585E8E76B7}';    // http://www.microsoft.com/downloads/details.aspx?familyid=32BC1BEE-A3F9-4C13-9C99-220B62A191EE
  VS2005_SP1_X86 =          '{7299052B-02A4-4627-81F2-1818DA5D550D}';    // 8.0.50727.762: http://www.microsoft.com/downloads/details.aspx?FamilyID=200B2FD9-AE1A-4A14-984D-389C36F85647
  VS2005_SP1_X86_ATL_SEC =  '{837B34E3-7C30-493C-8F6A-2B0F04E2912C}';    // 8.0.50727.4053: http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=14431
  VS2005_SP1_X86_MFC_SEC =  '{710f4c1c-cc18-4c49-8cbf-51240c89a1a2}';    // 8.0.50727.6195: http://www.microsoft.com/download/en/details.aspx?displaylang=en&id=26347     
  
  VS2008_X86 =              '{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}';    // http://www.microsoft.com/downloads/details.aspx?FamilyID=9b2da534-3e03-4391-8a4d-074b9f2bc1bf
  VS2008_SP1_X86 =          '{9A25302D-30C0-39D9-BD6F-21E6EC160475}';    // 9.0.30729.17: http://www.microsoft.com/downloads/details.aspx?familyid=A5C84275-3B97-4AB7-A40D-3802B2AF5FC2
  VS2008_SP1_X86_ATL_SEC =  '{1F1C2DFC-2D24-3E06-BCB8-725134ADF989}';    // 9.0.30729.4148: http://www.microsoft.com/downloads/details.aspx?familyid=2051A0C1-C9B5-4B0A-A8F5-770A549FD78C
  // These updates currently don't have redist links:
  // 9.0.30729.5026:
  // 9.0.30729.5570: 
  // 9.0.30729.6161: http://support.microsoft.com/kb/2538243

  INSTALLSTATE_INVALIDARG	= -2;	  // An invalid parameter was passed to the function
  INSTALLSTATE_UNKNOWN	  = -1;   // The product is not advertised or installed
  INSTALLSTATE_ADVERTISED	= 1;	  // The product is advertised but not installed
  INSTALLSTATE_ABSENT	    = 2;	  // The product is installed for a different user
  INSTALLSTATE_DEFAULT	  = 5;    // The product is installed for the current user

function MsiQueryProductState(ProductCode: String): Integer;
  external 'MsiQueryProductStateA@msi.dll stdcall';

function IsProductCodeInstalled(ProductUUID: String): Boolean;
begin
  Result := (MsiQueryProductState(ProductUUID) = INSTALLSTATE_DEFAULT);
end;

function Needs2005Redist(): Boolean;
begin
  Result:= (IsProductCodeInstalled(VS2005_SP1_X86_MFC_SEC) = FALSE);
  if (Result = TRUE) then begin
    Log('User system needs VS 2005 SP1 x86 Redistributable, installing.');
  end else begin
    Log('User already has VS 2005 SP1 x86 Redistributable installed, skipping.');
  end
end;

function Needs2008Redist(): Boolean;
begin
  Result := (IsProductCodeInstalled(VS2008_SP1_X86_ATL_SEC) = FALSE);
  if (Result = TRUE) then begin
    Log('User system needs VS 2008 SP1 x86 Redistributable, installing.');
  end else begin
    Log('User already has VS 2008 SP1 x86 Redistributable installed, skipping.');
  end
end;
