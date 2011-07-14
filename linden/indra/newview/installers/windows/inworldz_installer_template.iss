; InWorldz inno setup installer template by McCabe Maxsted

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)

; These will change
AppId={{DC6CCE02-BC61-43B1-B4CA-292C6BCCCB34}
AppName=%%APPNAME%%
AppVerName=%%APPVERNAME%%
DefaultDirName={pf}\InWorldz
DefaultGroupName=InWorldz
VersionInfoProductName=%%APPNAME%%
OutputBaseFilename=%%INSTALLERFILENAME%%
VersionInfoVersion=%%VERSION%%
VersionInfoTextVersion=%%VERSION%%
VersionInfoProductVersion=%%VERSION%%
AppVersion=%%VERSION%%
VersionInfoCopyright=2011

; These won't change
VersionInfoCompany=InWorldz, LLC
AppPublisher=InWorldz, LLC
AppPublisherURL=http://inworldz.com
AppSupportURL=http://inworldz.com
AllowNoIcons=true
InfoAfterFile=..\..\..\..\..\..\README.txt
OutputDir=.
SetupIconFile=..\..\..\..\newview\installers\windows\install_icon.ico
Compression=lzma2/ultra64
InternalCompressLevel=ultra64
SolidCompression=true
PrivilegesRequired=poweruser
AllowRootDirectory=true
WizardImageFile=..\..\..\..\newview\installers\windows\installer_icon_left.bmp
WizardSmallImageFile=..\..\..\..\newview\installers\windows\installer_icon_right.bmp
SetupLogging=true
RestartIfNeededByRun=false
AlwaysRestart=false

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: checkedonce
; TODO: use scripting for something like this on uninstall:
; Name: uninstallsettings; Description: Remove user settings; Flags: checkablealone; Languages: ; GroupDescription: Uninstall:

; NOTE VS2005 is currently the only version supported anywhere in the packaging system, so we can do this here
[Files]
Source: %%PACKAGEFILES%%\inworldz.exe; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\character\*; DestDir: {app}\character; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\fonts\*; DestDir: {app}\fonts; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\app_settings\*; DestDir: {app}\app_settings; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\skins\*; DestDir: {app}\skins; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\llplugin\*; DestDir: {app}\llplugin; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\doc\*; DestDir: {app}\doc; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\alut.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\dbghelp.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libapr-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libapriconv-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libaprutil-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\llcommon.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\featuretable.txt; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\gpu_table.txt; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\kdu_v64R.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\kdu_v64R.dll.config; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\openal32.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\OpenJPEG.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\README.txt; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\SLPlugin.exe; DestDir: {app}; Flags: ignoreversion

; Gstreamer-specific files below
Source: %%PACKAGEFILES%%\lib\*; DestDir: {app}\lib; Flags: ignoreversion recursesubdirs createallsubdirs
Source: %%PACKAGEFILES%%\avcodec-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\avdevice-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\avfilter-gpl-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\avformat-gpl-52.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\avutil-gpl-50.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\iconv.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\liba52-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libbz2.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libcelt-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libdca-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libexpat-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libfaad-2.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libFLAC-8.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgcrypt-11.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgio-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libglib-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgmodule-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgnutls-26.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgobject-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgpg-error-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstapp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstaudio-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstbase-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstcontroller-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstdataprotocol-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstfft-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstinterfaces-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstnet-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstnetbuffer-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstpbutils-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstphotography-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstreamer-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstriff-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstrtp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstrtsp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstsdp-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstsignalprocessor-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgsttag-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgstvideo-0.10.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libgthread-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libmms-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libmpeg2-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libneon-27.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libogg-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\liboil-0.3-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libsoup-2.4-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libtasn1-3.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libtheora-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libtheoradec-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libvorbis-0.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libvorbisenc-2.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libvorbisfile-3.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libwavpack-1.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libx264-67.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libxml2-2.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\libxml2.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\SDL.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\xvidcore.dll; DestDir: {app}; Flags: ignoreversion
Source: %%PACKAGEFILES%%\z.dll; DestDir: {app}; Flags: ignoreversion

; VC++ 2005 SP1 x86, VC++ 2008 SP1 x86, and VC++ 2010 SP1 x86 redist
Source: ..\..\..\..\newview\installers\windows\vcredist_x86_VS2005_SP1_MFC_SEC.exe; DestDir: {app}\redist; DestName: vcredist_x86_VS2005_SP1_MFC_SEC.exe
Source: ..\..\..\..\newview\installers\windows\vcredist_x86_VS2008_SP1_ATL_SEC.exe; DestDir: {app}\redist; DestName: vcredist_x86_VS2008_SP1_ATL_SEC.exe
Source: ..\..\..\..\newview\installers\windows\vcredist_x86_VS2010_SP1.exe; DestDir: {app}\redist; DestName: vcredist_x86_VS2010_SP1.exe

; Old files we don't use anymore:
; Source: %%PACKAGEFILES%%\dronesettings.xml; DestDir: {app}; Flags: ignoreversion
; Source: %%PACKAGEFILES%%\volume_settings.xml; DestDir: {app}; Flags: ignoreversion
; Source: %%PACKAGEFILES%%\srtp.dll; DestDir: {app}; Flags: ignoreversion
; Source: %%PACKAGEFILES%%\ssleay32.dll; DestDir: {app}; Flags: ignoreversion
; Source: %%PACKAGEFILES%%\tntk.dll; DestDir: {app}; Flags: ignoreversion
; Source: %%PACKAGEFILES%%\libeay32.dll; DestDir: {app}; Flags: ignoreversion
; Source: %%PACKAGEFILES%%\lsl_guide.html; DestDir: {app}; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files
;Source: %%PACKAGEFILES%%\msvcr71.dll; DestDir: {app}; Flags: ignoreversion; MinVersion: 0,6.01; Tasks: ; Languages:

[Registry]
Root: HKCR; Subkey: inworldz; ValueType: string; Flags: uninsdeletekey deletekey; ValueName: (default); ValueData: URL:InWorldz
Root: HKCR; Subkey: inworldz; ValueType: string; Flags: uninsdeletekey deletekey; ValueName: URL Protocol
Root: HKCR; Subkey: inworldz\DefaultIcon; Flags: uninsdeletekey deletekey; ValueType: string; ValueData: {app}\inworldz.exe
Root: HKCR; Subkey: inworldz\shell\open\command; ValueType: expandsz; Flags: uninsdeletekey deletekey; ValueData: "{app}\inworldz.exe -url ""%1"""; Languages:
; Root: HKCU; Subkey: Environment; ValueType: string; ValueName: GST_PLUGIN_PATH; Flags: deletevalue uninsdeletevalue; ValueData: {app}\lib
; Root: HKCU; Subkey: Environment; ValueType: expandsz; ValueName: PATH; ValueData: {app}

[Icons]
Name: {group}\{cm:UninstallProgram,InWorldz}; Filename: {uninstallexe}
Name: {commondesktop}\InWorldz; Filename: {app}\inworldz.exe; Tasks: desktopicon; WorkingDir: {app}; IconIndex: 0
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\InWorldz; Filename: {app}\inworldz.exe; Tasks: quicklaunchicon; WorkingDir: {app}
Name: {group}\InWorldz; Filename: {app}\inworldz.exe; WorkingDir: {app}; Comment: inworldz; IconIndex: 0;

[Run]
; Install redistributables. 
;
;     !!!!BEWARE!!!! 
;
; Command line parameters and filenames WILL change with each version. Blame Microsoft.

; Always use /q for VS2005 rather than something quieter such as Parameters: "/q:a c:""msiexec /i vcredist.msi /qn"" ". The redist will fail sometimes if you do otherwise.
Filename: {app}\redist\vcredist_x86_VS2005_SP1_MFC_SEC.exe; Parameters: "/q"; Check: Needs2005Redist; Flags: runhidden
Filename: {app}\redist\vcredist_x86_VS2008_SP1_ATL_SEC.exe; Parameters: "/q"; Check: Needs2008Redist; Flags: runhidden
Filename: {app}\redist\vcredist_x86_VS2010_SP1.exe; Parameters: "/q /norestart"; Check: Needs2010Redist; Flags: runhidden
Filename: {app}\inworldz.exe; WorkingDir: {app}; Flags: nowait postinstall

[UninstallDelete]
Name: {userappdata}\InWorldz\user_settings\password.dat; Type: files; Languages:
Name: {userappdata}\InWorldz\user_settings\settings.xml; Type: files; Languages:
; 1.1 and lower cache location:
Name: {userappdata}\InWorldz\cache; Type: filesandordirs
; 1.2 and higher cache location:
Name: {localappdata}\InWorldz\cache; Type: filesandordirs
Name: {userappdata}\InWorldz\logs; Type: filesandordirs
Name: {userappdata}\InWorldz\browser_profile; Type: filesandordirs
Name: C:\Users\{username}\.gstreamer-0.10; Type: filesandordirs
Name: C:\Documents and Settings\{username}\.gstreamer-0.10; Type: filesandordirs

[InstallDelete]
; Name: {app}\*.dll; Type: files; Tasks: ; Languages:
Name: {app}\licenses.txt; Type: files; Tasks: ; Languages:
; ALWAYS delete the plugins! Beware if you don't
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
; Breaks inworld audio if it's from an old version with a different GUID
Name: {app}\alut.dll; Type: files; Tasks: ; Languages:

; Old plugin files we want to kill:
Name: {app}\charset.dll; Type: files; Tasks: ; Languages:
Name: {app}\freebl3.dll; Type: files; Tasks: ; Languages:
Name: {app}\glew32.dll; Type: files; Tasks: ; Languages:
Name: {app}\iconv.dll; Type: files; Tasks: ; Languages:
Name: {app}\intl.dll; Type: files; Tasks: ; Languages:
Name: {app}\InWorldzViewer.exe; Type: files; Tasks: ; Languages:
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
Name: {app}\smime3.dll; Type: files; Tasks: ; Languages:
Name: {app}\softokn3.dll; Type: files; Tasks: ; Languages:
Name: {app}\ssl3.dll; Type: files; Tasks: ; Languages:
Name: {app}\xpcom.dll; Type: files; Tasks: ; Languages:
Name: {app}\xul.dll; Type: files; Tasks: ; Languages:
Name: {app}\xvidcore.dll; Type: files; Tasks: ; Languages:
Name: {app}\zlib1.dll; Type: files; Tasks: ; Languages:

; We don't distribute the CRT like this anymore; murder death kill
Name: {app}\SLPlugin.exe.config; Type: files; Tasks: ; Languages:
Name: {app}\Microsoft.VC80.CRT.manifest; Type: files; Tasks: ; Languages:
Name: {app}\msvcp80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr80.dll; Type: files; Tasks: ; Languages:
Name: {app}\msvcr71.dll; Type: files; Tasks: ; Languages:
Name: {app}\inworldz.exe.config; Type: files; Tasks: ; Languages:


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
