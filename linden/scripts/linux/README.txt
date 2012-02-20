How to build a linux version.

Edit config-SL-source to suit, the one that comes with the viewer
is the one used to build the linux release.

Run the scripts in order -

0-patch-SL-source
1-get-libraries-from-SL
2-trim-libraries-from-SL
3-compile-SL-source

The patch script should do nothing unless you have setup patches, so you
can skip it.  The get-libraries script needs a working network
connection, it fetches pre built libraries.  It also sets up the build. 
Trim-libraries removes those pre built libraries again if you configured
things to not use them.  The compile script does the actual work. 
Usually when making changes, only the compile script needs to be run
again.

4-package-viewer can be run at a later stage to actually build a tarball
of the viewer, ready for users to install.

These scripts are based on cmake-SL v1.31 (c)2008-2009 Henri Beauchamp.
Released under GPL license v2: http://www.gnu.org/licenses/gpl.txt
Modifications made by David Seikel.
