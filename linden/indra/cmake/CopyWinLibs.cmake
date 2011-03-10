# -*- cmake -*-

# The copy_win_libs folder contains file lists and a script used to 
# copy dlls, exes and such needed to run Imprudence from within 
# VisualStudio. 

include(CMakeCopyIfDifferent)

# Copying vivox's alut.dll breaks inworld audio, never use it
set(vivox_src_dir "${CMAKE_SOURCE_DIR}/newview/vivox-runtime/i686-win32")
set(vivox_files
    SLVoice.exe
    #alut.dll
    vivoxsdk.dll
    ortp.dll
    wrap_oal.dll
    )
copy_if_different(
    ${vivox_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    out_targets
    ${vivox_files}
    )
set(all_targets ${all_targets} ${out_targets})

set(debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug")
set(debug_files
    alut.dll
    openal32.dll
    openjpegd.dll
    libhunspell.dll
    libapr-1.dll
    libaprutil-1.dll
    libapriconv-1.dll
	
	# gstreamer streaming files below
	avcodec-gpl-52.dll
	avdevice-gpl-52.dll
	avfilter-gpl-1.dll
	avformat-gpl-52.dll
	avutil-gpl-50.dll
	iconv.dll
	liba52-0.dll
	libbz2.dll
	libcelt-0.dll
	libdca-0.dll
	libexpat-1.dll
	libfaad-2.dll
	libFLAC-8.dll
	libgcrypt-11.dll
	libgio-2.0-0.dll
	libglib-2.0-0.dll
	libgmodule-2.0-0.dll
	libgnutls-26.dll
	libgobject-2.0-0.dll
	libgpg-error-0.dll
	libgstapp-0.10.dll
	libgstaudio-0.10.dll
	libgstbase-0.10.dll
	libgstcontroller-0.10.dll
	libgstdataprotocol-0.10.dll
	libgstfft-0.10.dll
	libgstinterfaces-0.10.dll
	libgstnet-0.10.dll
	libgstnetbuffer-0.10.dll
	libgstpbutils-0.10.dll
	libgstphotography-0.10.dll
	libgstreamer-0.10.dll
	libgstriff-0.10.dll
	libgstrtp-0.10.dll
	libgstrtsp-0.10.dll
	libgstsdp-0.10.dll
	libgstsignalprocessor-0.10.dll
	libgsttag-0.10.dll
	libgstvideo-0.10.dll
	libgthread-2.0-0.dll
	libmms-0.dll
	libmpeg2-0.dll
	libneon-27.dll
	libogg-0.dll
	liboil-0.3-0.dll
	libsoup-2.4-1.dll
	libtasn1-3.dll
	libtheora-0.dll
	libtheoradec-1.dll
	libvorbis-0.dll
	libvorbisenc-2.dll
	libvorbisfile-3.dll
	libwavpack-1.dll
	libx264-67.dll
	libxml2-2.dll
	libxml2.dll
	SDL.dll
	xvidcore.dll
	z.dll
    )

copy_if_different(
    ${debug_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    out_targets 
    ${debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

# Debug config runtime files required for the plugin test mule
set(plugintest_debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug")
set(plugintest_debug_files
    libeay32.dll
    qtcored4.dll
    qtguid4.dll
    qtnetworkd4.dll
    qtopengld4.dll
    qtwebkitd4.dll
	qtxmlpatternsd4.dll
    ssleay32.dll
    )
copy_if_different(
    ${plugintest_debug_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/../test_apps/llplugintest/Debug"
    out_targets
    ${plugintest_debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

# Debug config runtime files required for the plugin test mule (Qt image format plugins)
set(plugintest_debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug/imageformats")
set(plugintest_debug_files
    qgifd4.dll
    qicod4.dll
    qjpegd4.dll
    qmngd4.dll
    qsvgd4.dll
    qtiffd4.dll
    )
copy_if_different(
    ${plugintest_debug_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/../test_apps/llplugintest/Debug/imageformats"
    out_targets
    ${plugintest_debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${plugintest_debug_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/llplugin/imageformats"
    out_targets
    ${plugintest_debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

# Release & ReleaseDebInfo config runtime files required for the plugin test mule
set(plugintest_release_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release")
set(plugintest_release_files
    libeay32.dll
    qtcore4.dll
    qtgui4.dll
    qtnetwork4.dll
    qtopengl4.dll
    qtwebkit4.dll
	qtxmlpatterns4.dll
    ssleay32.dll
    )
copy_if_different(
    ${plugintest_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/../test_apps/llplugintest/Release"
    out_targets
    ${plugintest_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${plugintest_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/../test_apps/llplugintest/RelWithDebInfo"
    out_targets
    ${plugintest_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

# Release & ReleaseDebInfo config runtime files required for the plugin test mule (Qt image format plugins)
set(plugintest_release_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release/imageformats")
set(plugintest_release_files
    qgif4.dll
    qico4.dll
    qjpeg4.dll
    qmng4.dll
    qsvg4.dll
    qtiff4.dll
    )
copy_if_different(
    ${plugintest_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/../test_apps/llplugintest/Release/imageformats"
    out_targets
    ${plugintest_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${plugintest_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/../test_apps/llplugintest/RelWithDebInfo/imageformats"
    out_targets
    ${plugintest_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${plugintest_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/Release/llplugin/imageformats"
    out_targets
    ${plugintest_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${plugintest_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/llplugin/imageformats"
    out_targets
    ${plugintest_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

# Debug config runtime files required for the plugins
set(plugins_debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug")
set(plugins_debug_files
    libeay32.dll
    qtcored4.dll
    qtguid4.dll
    qtnetworkd4.dll
    qtopengld4.dll
    qtwebkitd4.dll
	qtxmlpatternsd4.dll
    ssleay32.dll
    )
copy_if_different(
    ${plugins_debug_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/Debug/llplugin"
    out_targets
    ${plugins_debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

# Release & ReleaseDebInfo config runtime files required for the plugins
set(plugins_release_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release")
set(plugins_release_files
    libeay32.dll
    qtcore4.dll
    qtgui4.dll
    qtnetwork4.dll
    qtopengl4.dll
    qtwebkit4.dll
	qtxmlpatterns4.dll
    ssleay32.dll
    )
copy_if_different(
    ${plugins_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/Release/llplugin"
    out_targets
    ${plugins_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${plugins_release_src_dir}
    "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/llplugin"
    out_targets
    ${plugins_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

set(release_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release")
set(release_files
    alut.dll
    openal32.dll
    openjpeg.dll
    libhunspell.dll
    libapr-1.dll
    libaprutil-1.dll
    libapriconv-1.dll
	
	# gstreamer streaming files below
	avcodec-gpl-52.dll
	avdevice-gpl-52.dll
	avfilter-gpl-1.dll
	avformat-gpl-52.dll
	avutil-gpl-50.dll
	iconv.dll
	liba52-0.dll
	libbz2.dll
	libcelt-0.dll
	libdca-0.dll
	libexpat-1.dll
	libfaad-2.dll
	libFLAC-8.dll
	libgcrypt-11.dll
	libgio-2.0-0.dll
	libglib-2.0-0.dll
	libgmodule-2.0-0.dll
	libgnutls-26.dll
	libgobject-2.0-0.dll
	libgpg-error-0.dll
	libgstapp-0.10.dll
	libgstaudio-0.10.dll
	libgstbase-0.10.dll
	libgstcontroller-0.10.dll
	libgstdataprotocol-0.10.dll
	libgstfft-0.10.dll
	libgstinterfaces-0.10.dll
	libgstnet-0.10.dll
	libgstnetbuffer-0.10.dll
	libgstpbutils-0.10.dll
	libgstphotography-0.10.dll
	libgstreamer-0.10.dll
	libgstriff-0.10.dll
	libgstrtp-0.10.dll
	libgstrtsp-0.10.dll
	libgstsdp-0.10.dll
	libgstsignalprocessor-0.10.dll
	libgsttag-0.10.dll
	libgstvideo-0.10.dll
	libgthread-2.0-0.dll
	libmms-0.dll
	libmpeg2-0.dll
	libneon-27.dll
	libogg-0.dll
	liboil-0.3-0.dll
	libsoup-2.4-1.dll
	libtasn1-3.dll
	libtheora-0.dll
	libtheoradec-1.dll
	libvorbis-0.dll
	libvorbisenc-2.dll
	libvorbisfile-3.dll
	libwavpack-1.dll
	libx264-67.dll
	libxml2-2.dll
	libxml2.dll
	SDL.dll
	xvidcore.dll
	z.dll
    )
    
copy_if_different(
    ${release_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Release"
    out_targets 
    ${release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${vivox_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Release"
    out_targets 
    ${vivox_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${release_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo"
    out_targets 
    ${release_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${vivox_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo"
    out_targets 
    ${vivox_files}
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
