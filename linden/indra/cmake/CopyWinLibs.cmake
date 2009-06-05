# -*- cmake -*-

# The copy_win_libs folder contains file lists and a script used to 
# copy dlls, exes and such needed to run the SecondLife from within 
# VisualStudio. 

include(CMakeCopyIfDifferent)

set(gst_plugin_release_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release/lib/gstreamer-plugins")
set(gst_plugin_release_files
	libgstaacparse.dll
	libgstadder.dll
	libgstaiffparse.dll
	libgstalaw.dll
	libgstalpha.dll
	libgstasfdemux.dll
	libgstaudioconvert.dll
	libgstaudiofx.dll
	libgstaudiorate.dll
	libgstaudioresample.dll
	libgstauparse.dll
	libgstautoconvert.dll
	libgstautodetect.dll
	libgstavi.dll
	libgstcoreelements.dll
	libgstcutter.dll
	libgstdecodebin.dll
	libgstdecodebin2.dll
	libgstdshowdecwrapper.dll
	libgstdshowsrcwrapper.dll
	libgstdshowvideosink.dll
	libgstffmpeg.dll
	libgstffmpegcolorspace.dll
	libgstflv.dll
	libgstgdp.dll
	libgstgio.dll
	libgsth264parse.dll
	libgsticydemux.dll
	libgstid3demux.dll
	libgstinterleave.dll
	libgstjpeg.dll
	libgstmpeg4videoparse.dll
	libgstmpegaudioparse.dll
	libgstmpegdemux.dll
	libgstmpegstream.dll
	libgstmpegtsmux.dll
	libgstmpegvideoparse.dll
	libgstmulaw.dll
	libgstmultifile.dll
	libgstneon.dll
	libgstplaybin.dll
	libgstpng.dll
	libgstqtdemux.dll
	libgstqtmux.dll
	libgstqueue2.dll
	libgstreal.dll
	libgstrealmedia.dll
	libgstrtpmanager.dll
	libgstrtpmux.dll
	libgstrtp_good.dll
	libgstrtsp_good.dll
	libgstselector.dll
	libgstsmpte.dll
	libgsttheora.dll
	libgsttypefindfunctions.dll
	libgstudp.dll
	libgstvideobox.dll
	libgstvideocrop.dll
	libgstvideofilterbalance.dll
	libgstvideofiltergamma.dll
	libgstvideomixer.dll
	libgstvideorate.dll
	libgstvideoscale.dll
	libgstvideosignal.dll
	libgstvolume.dll
	libgstwasapi.dll
	libgstwaveform.dll
	libgstwavparse.dll
	libgstwininet.dll
	libgstwinks.dll
	libgstx264.dll
    )

copy_if_different(
    ${gst_plugin_release_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Release/lib/gstreamer-plugins"
    out_targets 
    ${gst_plugin_release_files}
    )
set(all_targets ${all_targets} ${out_targets})

set(gst_plugin_debug_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug/lib/gstreamer-plugins")
set(gst_plugin_debug_files
	libgstaacparse.dll
	libgstadder.dll
	libgstaiffparse.dll
	libgstalaw.dll
	libgstalpha.dll
	libgstasfdemux.dll
	libgstaudioconvert.dll
	libgstaudiofx.dll
	libgstaudiorate.dll
	libgstaudioresample.dll
	libgstauparse.dll
	libgstautoconvert.dll
	libgstautodetect.dll
	libgstavi.dll
	libgstcoreelements.dll
	libgstcutter.dll
	libgstdecodebin.dll
	libgstdecodebin2.dll
	libgstdshowdecwrapper.dll
	libgstdshowsrcwrapper.dll
	libgstdshowvideosink.dll
	libgstffmpeg.dll
	libgstffmpegcolorspace.dll
	libgstflv.dll
	libgstgdp.dll
	libgstgio.dll
	libgsth264parse.dll
	libgsticydemux.dll
	libgstid3demux.dll
	libgstinterleave.dll
	libgstjpeg.dll
	libgstmpeg4videoparse.dll
	libgstmpegaudioparse.dll
	libgstmpegdemux.dll
	libgstmpegstream.dll
	libgstmpegtsmux.dll
	libgstmpegvideoparse.dll
	libgstmulaw.dll
	libgstmultifile.dll
	libgstneon.dll
	libgstplaybin.dll
	libgstpng.dll
	libgstqtdemux.dll
	libgstqtmux.dll
	libgstqueue2.dll
	libgstreal.dll
	libgstrealmedia.dll
	libgstrtpmanager.dll
	libgstrtpmux.dll
	libgstrtp_good.dll
	libgstrtsp_good.dll
	libgstselector.dll
	libgstsmpte.dll
	libgsttheora.dll
	libgsttypefindfunctions.dll
	libgstudp.dll
	libgstvideobox.dll
	libgstvideocrop.dll
	libgstvideofilterbalance.dll
	libgstvideofiltergamma.dll
	libgstvideomixer.dll
	libgstvideorate.dll
	libgstvideoscale.dll
	libgstvideosignal.dll
	libgstvolume.dll
	libgstwasapi.dll
	libgstwaveform.dll
	libgstwavparse.dll
	libgstwininet.dll
	libgstwinks.dll
	libgstx264.dll
    )

copy_if_different(
    ${gst_plugin_debug_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Debug/lib/gstreamer-plugins"
    out_targets 
    ${gst_plugin_debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

set(vivox_src_dir "${CMAKE_SOURCE_DIR}/newview/vivox-runtime/i686-win32")
set(vivox_files
    tntk.dll
    libeay32.dll
    ssleay32.dll
    srtp.dll
    ortp.dll
    wrap_oal.dll
    )

set(debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug")
set(debug_files
    alut.dll
    freebl3.dll
    gksvggdiplus.dll
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
	libneon-27.dll
	libogg-0.dll
	liboil-0.3-0.dll
	libopenjpeg-2.dll
	libschroedinger-1.0-0.dll
	libspeex-1.dll
	libtheora-0.dll
	libvorbis-0.dll
	libvorbisenc-2.dll
	libxml2-2.dll
    ogg.dll
    vorbis.dll
    xvidcore.dll
    zlib1.dll
    libpng12-0.dll
    )

copy_if_different(
    ${debug_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    out_targets 
    ${debug_files}
    )
set(all_targets ${all_targets} ${out_targets})

copy_if_different(
    ${vivox_src_dir} 
    "${CMAKE_CURRENT_BINARY_DIR}/Debug"
    out_targets 
    ${vivox_files}
    )
set(all_targets ${all_targets} ${out_targets})

set(release_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/release")
set(release_files
    alut.dll
    freebl3.dll
    gksvggdiplus.dll
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
	libneon-27.dll
	libogg-0.dll
	liboil-0.3-0.dll
	libopenjpeg-2.dll
	libschroedinger-1.0-0.dll
	libspeex-1.dll
	libtheora-0.dll
	libvorbis-0.dll
	libvorbisenc-2.dll
	libxml2-2.dll
    ogg.dll
    vorbis.dll
    xvidcore.dll
    zlib1.dll
    libpng12-0.dll
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
# *TODO - Adapt this to support VC9
if (MSVC80)
    FIND_PATH(debug_msvc8_redist_path msvcr80d.dll
        PATHS
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VC;ProductDir]/redist/Debug_NonRedist/x86/Microsoft.VC80.DebugCRT
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
    endif (EXISTS ${debug_msvc8_redist_path})

    FIND_PATH(release_msvc8_redist_path msvcr80.dll
        PATHS
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\8.0\\Setup\\VC;ProductDir]/redist/x86/Microsoft.VC80.CRT
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

    endif (EXISTS ${release_msvc8_redist_path})
endif (MSVC80)

add_custom_target(copy_win_libs ALL DEPENDS ${all_targets})