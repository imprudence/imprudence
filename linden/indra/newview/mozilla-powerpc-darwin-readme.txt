NOTE: these directions have been obsoleted.  There are now shell scripts that will check out, build, and package the necessary pieces
of the mozilla code in lindelib/mozilla-1.8.0/mac-build.  I'm leaving this file here for historical interest.
-------------
Written by Monroe on June 17, 2005.

Here's how to rebuild the Mozilla components for the Mac build.

Check out the mozilla source from cvs

cd to the mozilla directory (the same one that contains client.mk)

cp .mozconfig.opt.shared.small .mozconfig

Add the following line to the .mozconfig file you just created:

ac_add_options --with-macos-sdk=/Developer/SDKs/MacOSX10.2.8.sdk

make -f client.mk build

wait a while.

The build products you need to extract are in objdir-opt-shared-small/dist/bin and objdir-opt-shared-small/dist/lib.
 
Copy the following to linden/libraries/firefox-1.0.4/<arch>/lib_release and linden/libraries/firefox-1.0.4/<arch>/lib_release:

objdir-opt-shared-small/dist/lib/libembed_base_s.a
objdir-opt-shared-small/dist/lib/libxpcomglue_s.a
objdir-opt-shared-small/dist/bin/libxpcom.dylib
objdir-opt-shared-small/dist/bin/libplds4.dylib
objdir-opt-shared-small/dist/bin/libplc4.dylib
objdir-opt-shared-small/dist/bin/libnspr4.dylib
objdir-opt-shared-small/dist/bin/libgkgfx.dylib

This first part should be repeated with .mozconfig.debug.shared.small to generate the libraries in the two matching lib_debug directories.  The debug version of the bin directory is prohibitively large, so we're just using the release version of that part.

Much of the contents of objdir-opt-shared-small/dist/bin also needs to go into a tar file that will be used when generating the application bundle.  

The bin directory will be populated with symlinks.  If you just tar it up as-is, you'll get a tar file full of symlinks, which is not useful.

Use 'cp -RL source dest' to make a copy of the bin directory with all symlinks expanded.  This will be more useful.

Remove things that aren't needed.  This includes at least:

asdecode
firefox
firefox-bin
firefox-config
LICENSE
nsinstall
mangle
regxpcom
regchrome
README.txt
run-mozilla.sh
xpcshell
xpt_dump
shlibsign
xpt_link
xpidl
xpicleanup

There may be other pieces that aren't needed as well.  I expect this will be refined moving forward.

Because of the way the tar file will be expanded (directly inside the application bundle, in Contents/MacOS), it's important to create it so that it won't expand at a subdirectory of the current path.  The way to to this is to cd to the dist/bin directory and do something like this:

tar -zcvf ../mozilla-powerpc-darwin.tgz .

This will create a tar file containing everything in the current directory, and will place the file one level up (so it doesn't interfere with its own creation).  This file should replace the file with the above name checked into cvs in linden/indra/newview/.  One of the lines in the shell script phase of the build extracts it appropriately into the application bundle.

If any of this is unclear, please contact Monroe and I'll try to clarify and update this file.

