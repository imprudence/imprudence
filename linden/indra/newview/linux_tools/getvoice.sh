#!/bin/bash

SCRIPTSRC=`readlink -f "$0" || echo "$0"`
RUN_PATH=`dirname "${SCRIPTSRC}" || echo .`

#if mozilla-runtime-linux-x86_64 is present we are using 64bit Imprudence on 64bit Linux
if [ -d ${RUN_PATH}/app_settings/mozilla-runtime-linux-x86_64/ ]; then
	LIB_INSTALLDIR="lib32/" # It's 32bit voice on 64bit Linux and 64bit viewer. Not using lib/ for avoiding ambiguity.
else
	LIB_INSTALLDIR="lib/" # It's 32bit voice on 32 or 64bit Linux and 32bit viewer.
fi

mkdir -p $LIB_INSTALLDIR
wget http://s3.amazonaws.com/viewer-source-downloads/install_pkgs/vivox-2.1.3010.6270-linux-20090309.tar.bz2
tar -C ./bin --strip-components 4 -xjf vivox-*.tar.bz2 --wildcards '*SLVoice'
tar -C ./$LIB_INSTALLDIR --strip-components 4 -xjf vivox-*.tar.bz2 --wildcards '*.so*'
rm vivox-*.tar.bz2

#now we have Vivox' OpenAL, but we want Imprudence (32bit for voice) OpenAL which is way better:
wget http://imprudenceviewer.org/download/libs/openal-linux32-20100426.tar.bz2
tar -C ./$LIB_INSTALLDIR --strip-components 3 -xjf openal-*.tar.bz2 --wildcards '*openal.so*'
rm openal-*.tar.bz2

# For 64bit viewer on 64bit Linux we also need a 32bit libidn.so.11 and libuuid.so.1
if [ -d ${RUN_PATH}/lib32/ ]; then
	wget http://imprudenceviewer.org/download/libs/libidn20100312.tar.bz2
	tar -C $LIB_INSTALLDIR --strip-components 1 -xjf  libidn*.tar.bz2 --wildcards '*.so*'
	rm libidn*.tar.bz2

	wget http://s3.amazonaws.com/viewer-source-downloads/install_pkgs/libuuid-linux-20090417.tar.bz2
	tar -C ./$LIB_INSTALLDIR --strip-components 3 -xjf libuuid-*.tar.bz2 --wildcards '*.so*'
	rm libuuid-*.tar.bz2
fi
 
