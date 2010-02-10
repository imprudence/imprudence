# -*- cmake -*-

# The copy_win_libs folder contains file lists and a script used to 
# copy dlls, exes and such needed to run Imprudence from within 
# VisualStudio. 

include(CMakeCopyIfDifferent)

set(debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug")
set(debug_files
    alut.dll
    freebl3.dll
    js3250.dll
    nspr4.dll
    nss3.dll
    nssckbi.dll
    openal32.dll
    openjpegd.dll
    plc4.dll
    plds4.dll
    smime3.dll
    softokn3.dll
    ssl3.dll
    xpcom.dll
    xul.dll
    windbgdlg.exe
    iconv.dll
    libxml2.dll
	libcairo-2.dll
    libgio-2.0-0.dll
    libglib-2.0-0.dll
    libgmodule-2.0-0.dll
    libgobject-2.0-0.dll
    libgthread-2.0-0.dll
    charset.dll
	intl.dll
	libgcrypt-11.dll
	libgnutls-26.dll
	libgpg-error-0.dll
	libgstapp.dll
	libgstaudio.dll
	libgstbase-0.10.dll
	libgstcdda.dll
	libgstcontroller-0.10.dll
	libgstdataprotocol-0.10.dll
	libgstdshow.dll
	libgstfft.dll
	libgstinterfaces.dll
	libgstnet-0.10.dll
	libgstnetbuffer.dll
	libgstpbutils.dll
	libgstreamer-0.10.dll
	libgstriff.dll
	libgstrtp.dll
	libgstrtsp.dll
	libgstsdp.dll
	libgsttag.dll
	libgstvideo.dll
	libjpeg.dll
	libmp3lame-0.dll
	libneon-27.dll
	libogg-0.dll
	liboil-0.3-0.dll
	libopenjpeg-2.dll
	libpng12-0.dll
	libschroedinger-1.0-0.dll
	libspeex-1.dll
	libtheora-0.dll
	libvorbis-0.dll
	libvorbisenc-2.dll
	libxml2-2.dll
	glew32.dll
    xvidcore.dll
    zlib1.dll
    )

copy_if_different(
    ${debug_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    out_targets 
    ${debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

set(release_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release")
set(release_files
    alut.dll
    freebl3.dll
    js3250.dll
    nspr4.dll
    nss3.dll
    nssckbi.dll
    openal32.dll
    openjpeg.dll
    plc4.dll
    plds4.dll
    smime3.dll
    softokn3.dll
    ssl3.dll
    xpcom.dll
    xul.dll
    iconv.dll
    libxml2.dll
	libcairo-2.dll
    libgio-2.0-0.dll
    libglib-2.0-0.dll
    libgmodule-2.0-0.dll
    libgobject-2.0-0.dll
    libgthread-2.0-0.dll
	charset.dll
	intl.dll
	libgcrypt-11.dll
	libgnutls-26.dll
	libgpg-error-0.dll
	libgstapp.dll
	libgstaudio.dll
	libgstbase-0.10.dll
	libgstcdda.dll
	libgstcontroller-0.10.dll
	libgstdataprotocol-0.10.dll
	libgstdshow.dll
	libgstfft.dll
	libgstinterfaces.dll
	libgstnet-0.10.dll
	libgstnetbuffer.dll
	libgstpbutils.dll
	libgstreamer-0.10.dll
	libgstriff.dll
	libgstrtp.dll
	libgstrtsp.dll
	libgstsdp.dll
	libgsttag.dll
	libgstvideo.dll
	libjpeg.dll
	libmp3lame-0.dll
	libneon-27.dll
	libogg-0.dll
	liboil-0.3-0.dll
	libopenjpeg-2.dll
	libpng12-0.dll
	libschroedinger-1.0-0.dll
	libspeex-1.dll
	libtheora-0.dll
	libvorbis-0.dll
	libvorbisenc-2.dll
	libxml2-2.dll
	glew32.dll
    xvidcore.dll
    zlib1.dll
    )
    
copy_if_different(
    ${release_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Release"
    out_targets 
    ${release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${release_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo"
    out_targets 
    ${release_files}
    )
set(all_targets ${all_targets} ${out_targets})


# Copy MS C runtime dlls, required for packaging.
# We always need the VS 2005 redist.
# *TODO - Adapt this to support VC9
FIND_PATH(debug_msvc8_redist_path msvcr80d.dll
    PATHS
     [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VC;ProductDir]/redist/Debug_NonRedist/x86/Microsoft.VC80.DebugCRT
    NO_DEFAULT_PATH
    NO_DEFAULT_PATH
    )

if(EXISTS ${debug_msvc8_redist_path})
    set(debug_msvc8_files
        msvcr80d.dll
        msvcp80d.dll
        Microsoft.VC80.DebugCRT.manifest
        )

    copy_if_different(
        ${debug_msvc8_redist_path} 
        "${CMAKE_CURRENT_BINARY_DIR}/Debug"
        out_targets 
        ${debug_msvc8_files}
        )
    set(all_targets ${all_targets} ${out_targets})

    set(debug_appconfig_file ${CMAKE_CURRENT_BINARY_DIR}/Debug/${VIEWER_BINARY_NAME}.exe.config)
    add_custom_command(
        OUTPUT ${debug_appconfig_file}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS
          ${CMAKE_CURRENT_SOURCE_DIR}/build_win32_appConfig.py
          ${CMAKE_CURRENT_BINARY_DIR}/Debug/Microsoft.VC80.DebugCRT.manifest
          ${CMAKE_CURRENT_SOURCE_DIR}/ImprudenceDebug.exe.config
          ${debug_appconfig_file}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Debug/Microsoft.VC80.DebugCRT.manifest
        COMMENT "Creating debug app config file"
        )

endif (EXISTS ${debug_msvc8_redist_path})

FIND_PATH(release_msvc8_redist_path msvcr80.dll
    PATHS
     [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VC;ProductDir]/redist/x86/Microsoft.VC80.CRT
    NO_DEFAULT_PATH
    NO_DEFAULT_PATH
    )

if(EXISTS ${release_msvc8_redist_path})
    set(release_msvc8_files
        msvcr80.dll
        msvcp80.dll
        Microsoft.VC80.CRT.manifest
        )

    copy_if_different(
        ${release_msvc8_redist_path} 
        "${CMAKE_CURRENT_BINARY_DIR}/Release"
        out_targets 
        ${release_msvc8_files}
        )
    set(all_targets ${all_targets} ${out_targets})

    copy_if_different(
        ${release_msvc8_redist_path} 
        "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo"
        out_targets 
        ${release_msvc8_files}
        )
    set(all_targets ${all_targets} ${out_targets})

    set(release_appconfig_file ${CMAKE_CURRENT_BINARY_DIR}/Release/${VIEWER_BINARY_NAME}.exe.config)
    add_custom_command(
        OUTPUT ${release_appconfig_file}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS
          ${CMAKE_CURRENT_SOURCE_DIR}/build_win32_appConfig.py
          ${CMAKE_CURRENT_BINARY_DIR}/Release/Microsoft.VC80.CRT.manifest
          ${CMAKE_CURRENT_SOURCE_DIR}/Imprudence.exe.config
          ${release_appconfig_file}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Release/Microsoft.VC80.CRT.manifest
        COMMENT "Creating release app config file"
        )

    set(relwithdebinfo_appconfig_file ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/${VIEWER_BINARY_NAME}.exe.config)
    add_custom_command(
        OUTPUT ${relwithdebinfo_appconfig_file}
        COMMAND ${PYTHON_EXECUTABLE}
        ARGS
          ${CMAKE_CURRENT_SOURCE_DIR}/build_win32_appConfig.py
          ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/Microsoft.VC80.CRT.manifest
          ${CMAKE_CURRENT_SOURCE_DIR}/Imprudence.exe.config
          ${relwithdebinfo_appconfig_file}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/Microsoft.VC80.CRT.manifest
        COMMENT "Creating relwithdebinfo app config file"
        )
      
endif (EXISTS ${release_msvc8_redist_path})

add_custom_target(copy_win_libs ALL
  DEPENDS 
    ${all_targets}
    ${release_appconfig_file} 
    ${relwithdebinfo_appconfig_file} 
    ${debug_appconfig_file}
  )

if(EXISTS ${internal_llkdu_path})
    add_dependencies(copy_win_libs llkdu)
endif(EXISTS ${internal_llkdu_path})
