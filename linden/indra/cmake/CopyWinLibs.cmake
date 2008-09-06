# -*- cmake -*-

# The copy_win_libs folder contains file lists and a script used to 
# copy dlls, exes and such needed to run the SecondLife from within 
# VisualStudio. 

include(CMakeCopyIfDifferent)

set(vivox_src_dir "${CMAKE_SOURCE_DIR}/newview/vivox-runtime/i686-win32")
set(vivox_files
    tntk.dll
    libeay32.dll
    SLVoice.exe
    ssleay32.dll
    SLVoiceAgent.exe
    srtp.dll
    alut.dll
    vivoxsdk.dll
    ortp.dll
    wrap_oal.dll
    )

set(debug_src_dir "${CMAKE_SOURCE_DIR}/../libraries/i686-win32/lib/debug")
set(debug_files
    freebl3.dll
    gksvggdiplus.dll
    js3250.dll
    nspr4.dll
    nss3.dll
    nssckbi.dll
    plc4.dll
    plds4.dll
    smime3.dll
    softokn3.dll
    ssl3.dll
    xpcom.dll
    xul.dll
    openjpegd.dll
    windbgdlg.exe
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
    freebl3.dll
    gksvggdiplus.dll
    js3250.dll
    nspr4.dll
    nss3.dll
    nssckbi.dll
    plc4.dll
    plds4.dll
    smime3.dll
    softokn3.dll
    ssl3.dll
    xpcom.dll
    xul.dll
    openjpeg.dll
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

set(internal_llkdu_path "${CMAKE_SOURCE_DIR}/llkdu")
if(EXISTS ${internal_llkdu_path})
    set(internal_llkdu_src "${CMAKE_BINARY_DIR}/llkdu/${CMAKE_CFG_INTDIR}/llkdu.dll")
    set(llkdu_dst "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/llkdu.dll")
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${llkdu_dst}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${internal_llkdu_src} ${llkdu_dst}
        DEPENDS ${internal_llkdu_src}
        COMMENT "Copying llkdu.dll ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}"
        )
    set(all_targets ${all_targets} ${llkdu_dst})
else(EXISTS ${internal_llkdu_path})
    set(debug_llkdu_src "${debug_src_dir}/llkdu.dll")
    set(debug_llkdu_dst "${CMAKE_CURRENT_BINARY_DIR}/Debug/llkdu.dll")
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${debug_llkdu_dst}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${debug_llkdu_src} ${debug_llkdu_dst}
        DEPENDS ${debug_llkdu_src}
        COMMENT "Copying llkdu.dll ${CMAKE_CURRENT_BINARY_DIR}/Debug"
        )
    set(all_targets ${all_targets} ${debug_llkdu_dst})

    set(release_llkdu_src "${release_src_dir}/llkdu.dll")
    set(release_llkdu_dst "${CMAKE_CURRENT_BINARY_DIR}/Release/llkdu.dll")
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${release_llkdu_dst}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${release_llkdu_src} ${release_llkdu_dst}
        DEPENDS ${release_llkdu_src}
        COMMENT "Copying llkdu.dll ${CMAKE_CURRENT_BINARY_DIR}/Release"
        )
    set(all_targets ${all_targets} ${release_llkdu_dst})

    set(relwithdebinfo_llkdu_dst "${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/llkdu.dll")
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${relwithdebinfo_llkdu_dst}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${release_llkdu_src} ${relwithdebinfo_llkdu_dst}
        DEPENDS ${release_llkdu_src}
        COMMENT "Copying llkdu.dll ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo"
        )
    set(all_targets ${all_targets} ${relwithdebinfo_llkdu_dst})
   
endif (EXISTS ${internal_llkdu_path})

add_custom_target(copy_win_libs ALL DEPENDS ${all_targets})

if(EXISTS ${internal_llkdu_path})
    add_dependencies(copy_win_libs llkdu)
endif(EXISTS ${internal_llkdu_path})
