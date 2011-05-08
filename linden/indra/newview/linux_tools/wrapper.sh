#!/bin/bash

## Here are some configuration options for Linux Client Testers.
## These options are for self-assisted troubleshooting during this beta
## testing phase; you should not usually need to touch them.

## If the default configuration of openal-soft isn't working for you.
## There are 3 places where it looks for a configuration file:
## /etc/openal/alsoft.conf $HOME/.alsoftrc and ALSOFT_CONF
## ALSOFT_CONF is the full path including the filename, like: /home/myuser/myconfigfile.txt
## If none of them is set a hardcoded default is taken.
## If you set several: ALSOFT_CONF 'wins' always, and $HOME/.alsoftrc 'wins' over /etc/openal/alsoft.conf
#export ALSOFT_CONF="$(pwd)/alsoft.conf"

## - Avoids using the OpenAL audio driver; disables any inworld sound effects.
##   NOTE: - OpenAL is not used for any streaming audio in Imprudence.
##         - Other export LL_BAD_<driver> have no effect in Imprudence.
#export LL_BAD_OPENAL_DRIVER=x

## If you have custom gstreamer plugins, 
## e.g. you want to use Imprudence-1.4.x with the gstreamer plugins of Imprudence-1.3.1: 
##			Imprudence-1.3.1-Linux-x86/lib/gstreamer-plugins
##	respectively	Imprudence-1.3.1-Linux-x86_64/lib64/gstreamer-plugins
## NOTE: *WAY* better is to install the gstreamer plugins that come with your distros package manager,
##	thats why Imprudence-1.4.x comes without gstreamer plugins.
#export GST_PLUGIN_PATH="'${HOME}/Imprudence-1.3.1-Linux-x86/lib/gstreamer-plugins':'${GST_PLUGIN_PATH}'"

## - Avoids the optional OpenGL extensions which have proven most problematic
##   on some hardware.  Disabling this option may cause BETTER PERFORMANCE but
##   may also cause CRASHES and hangs on some unstable combinations of drivers
##   and hardware.
## NOTE: This is now disabled by default.
#export LL_GL_BASICEXT=x

## - Avoids *all* optional OpenGL extensions.  This is the safest and least-
##   exciting option.  Enable this if you experience stability issues, and
##   report whether it helps in the Linux Client Testers forum.
#export LL_GL_NOEXT=x

## - For advanced troubleshooters, this lets you disable specific GL
##   extensions, each of which is represented by a letter a-o.  If you can
##   narrow down a stability problem on your system to just one or two
##   extensions then please post details of your hardware (and drivers) to
##   the Linux Client Testers forum along with the minimal
##   LL_GL_BLACKLIST which solves your problems.
#export LL_GL_BLACKLIST=abcdefghijklmno

## - Some ATI/Radeon users report random X server crashes when the mouse
##   cursor changes shape.  If you suspect that you are a victim of this
##   driver bug, try enabling this option and report whether it helps:
#export LL_ATI_MOUSE_CURSOR_BUG=x

## - If you experience crashes with streaming video and music, you can
##   disable these by enabling this option:
#export LL_DISABLE_GSTREAMER=x


## Everything below this line is just for advanced troubleshooters.
##-------------------------------------------------------------------
## - For advanced debugging cases, you can run the viewer under the
##   control of another program, such as strace, gdb, or valgrind.  If
##   you're building your own viewer, bear in mind that the executable
##   in the bin directory will be stripped: you should replace it with
##   an unstripped binary before you run.
#export LL_WRAPPER='gdb --args'
#export LL_WRAPPER='valgrind --smc-check=all --error-limit=no --log-file=secondlife.vg --leak-check=full --suppressions=/usr/lib/valgrind/glibc-2.5.supp --suppressions=secondlife-i686.supp'

## - Avoids an often-buggy X feature that doesn't really benefit us anyway.
export SDL_VIDEO_X11_DGAMOUSE=0

## - Works around a problem with misconfigured 64-bit systems not finding GL
export LIBGL_DRIVERS_PATH="${LIBGL_DRIVERS_PATH}":/usr/lib64/dri:/usr/lib32/dri:/usr/lib/dri

## - The 'scim' GTK IM module widely crashes the viewer.  Avoid it.
if [ "$GTK_IM_MODULE" = "scim" ]; then
    export GTK_IM_MODULE=xim
fi

## - Automatically work around the ATI mouse cursor crash bug:
## (this workaround is disabled as most fglrx users do not see the bug)
#if lsmod | grep fglrx &>/dev/null ; then
#	export LL_ATI_MOUSE_CURSOR_BUG=x
#fi


## Nothing worth editing below this line.
##-------------------------------------------------------------------

SCRIPTSRC=`readlink -f "$0" || echo "$0"`
RUN_PATH=`dirname "${SCRIPTSRC}" || echo .`
echo "Running from ${RUN_PATH}"
cd "${RUN_PATH}"

# Re-register the secondlife:// protocol handler every launch, for now.
./register_secondlifeprotocol.sh
## Before we mess with LD_LIBRARY_PATH, save the old one to restore for
##  subprocesses that care.
export SAVED_LD_LIBRARY_PATH="${LD_LIBRARY_PATH}"

if [ -n "$LL_TCMALLOC" ]; then
    tcmalloc_libs='/usr/lib/libtcmalloc.so.0 /usr/lib/libstacktrace.so.0 /lib/libpthread.so.0'
    all=1
    for f in $tcmalloc_libs; do
        if [ ! -f $f ]; then
	    all=0
	fi
    done
    if [ $all != 1 ]; then
        echo 'Cannot use tcmalloc libraries: components missing' 1>&2
    else
	export LD_PRELOAD=$(echo $tcmalloc_libs | tr ' ' :)
	if [ -z "$HEAPCHECK" -a -z "$HEAPPROFILE" ]; then
	    export HEAPCHECK=${HEAPCHECK:-normal}
	fi
    fi
fi

export VIEWER_BINARY='do-not-directly-run-imprudence-bin'
BINARY_TYPE=$(expr match "$(file -b bin/$VIEWER_BINARY)" '\(.*executable\)')
if [ "${BINARY_TYPE}" == "ELF 64-bit LSB executable" ]; then
	export SL_ENV='LD_LIBRARY_PATH="`pwd`"/lib64:"`pwd`"/lib32:"${LD_LIBRARY_PATH}"'
else
	export SL_ENV='LD_LIBRARY_PATH="`pwd`"/lib:"${LD_LIBRARY_PATH}"'
fi

export SL_CMD='$LL_WRAPPER bin/$VIEWER_BINARY'
export SL_OPT="`cat gridargs.dat` $@"

# Run the program
eval ${SL_ENV} ${SL_CMD} ${SL_OPT} || LL_RUN_ERR=runerr

# Handle any resulting errors
if [ -n "$LL_RUN_ERR" ]; then
	LL_RUN_ERR_MSG=""
	if [ "$LL_RUN_ERR" = "runerr" ]; then
		# generic error running the binary
		echo 'unexpected shutdown'
	fi
fi

LOGS_PATH="${HOME}/.imprudence/logs"
if [ -f "${LOGS_PATH}/stack_trace.log" ]; then
	LOG_PACKAGE_NAME="MAIL-THIS-CRASHLOG-PLEASE.$(date +%y%m%d%H%M).tar.bz2"
	cp "${LOGS_PATH}/stack_trace.log" stack_trace.log
	cp "${LOGS_PATH}/Imprudence.log" Imprudence.log
	tar --numeric-owner -cjf  ${LOG_PACKAGE_NAME} \
				stack_trace.log \
				Imprudence.log
	rm stack_trace.log
	rm Imprudence.log
	echo "You find a crash log package to mail to Imprudence here:"
	echo "${RUN_PATH}/${LOG_PACKAGE_NAME}"
	echo "See where to send: http://wiki.kokuaviewer.org/wiki/Imprudence:Debug_Logs#Where_to_Send_Them"
fi