#!/usr/bin/python
# @file viewer_manifest.py
# @author Ryan Williams
# @brief Description of all installer viewer files, and methods for packaging
#        them into installers for all supported platforms.
#
# $LicenseInfo:firstyear=2006&license=viewergpl$
# 
# Copyright (c) 2006-2009, Linden Research, Inc.
# 
# Second Life Viewer Source Code
# The source code in this file ("Source Code") is provided by Linden Lab
# to you under the terms of the GNU General Public License, version 2.0
# ("GPL"), unless you have obtained a separate licensing agreement
# ("Other License"), formally executed by you and Linden Lab.  Terms of
# the GPL can be found in doc/GPL-license.txt in this distribution, or
# online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
# 
# There are special exceptions to the terms and conditions of the GPL as
# it is applied to this Source Code. View the full text of the exception
# in the file doc/FLOSS-exception.txt in this software distribution, or
# online at
# http://secondlifegrid.net/programs/open_source/licensing/flossexception
# 
# By copying, modifying or distributing this software, you acknowledge
# that you have read and understood your obligations described above,
# and agree to abide by those obligations.
# 
# ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
# WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
# COMPLETENESS OR PERFORMANCE.
# $/LicenseInfo$

# DO NOT RUN THIS FILE DIRECTLY
# Instead, run develop.py with "configure -DPACKAGE:BOOL=ON" e.g.:
#   develop.py -G vc80 configure -DPACKAGE:BOOL=ON
# to generate the "package" project in Visual Studio 2005
# Note: as of Imprudence 1.3, this defaults to on for Windows

import sys
import os.path
import re
import tarfile
viewer_dir = os.path.dirname(__file__)
# add llmanifest library to our path so we don't have to muck with PYTHONPATH
sys.path.append(os.path.join(viewer_dir, '../lib/python/indra/util'))
from llmanifest import LLManifest, main, proper_windows_path, path_ancestors

class ViewerManifest(LLManifest):
    def construct(self):
        super(ViewerManifest, self).construct()
        self.exclude("*.svn*")
        self.path(src="../../scripts/messages/message_template.msg", dst="app_settings/message_template.msg")
        self.path(src="../../etc/message.xml", dst="app_settings/message.xml")

        if self.prefix(src="app_settings"):
            self.exclude("logcontrol.xml")
            self.exclude("logcontrol-dev.xml")
            self.path("*.pem")
            self.path("*.ini")
            self.path("*.xml")
            self.path("*.db2")

            # include the entire shaders directory recursively
            self.path("shaders")
            # ... and the entire windlight directory
            self.path("windlight")
            # ...and the entire dictionaries directory
            self.path("dictionaries")
                  
            self.end_prefix("app_settings")

        if self.prefix(src="character"):
            self.path("*.llm")
            self.path("*.xml")
            self.path("*.tga")
            self.end_prefix("character")

        # Include our fonts
        if self.prefix(src="fonts"):
            self.path("GPL.txt")
            self.path("Liberation-License.txt")
            self.path("LiberationSans-Bold.ttf")
            self.path("LiberationSans-Regular.ttf")

            self.path("Delicious-License.txt")
            self.path("Delicious-BoldItalic.otf")
            self.path("Delicious-Bold.otf")
            self.path("Delicious-Italic.otf")
            self.path("Delicious-Roman.otf")

            self.path("Droid-License.txt")
            self.path("DroidSans-Bold.ttf")
            self.path("DroidSans.ttf")

            self.path("DejaVu-License.txt")
            self.path("DejaVuSansCondensed-BoldOblique.ttf")
            self.path("DejaVuSansCondensed-Bold.ttf")
            self.path("DejaVuSansCondensed-Oblique.ttf")
            self.path("DejaVuSansCondensed.ttf")

            self.path("Vera-License.txt")
            self.path("VeraMono.ttf")
            self.end_prefix("fonts")

        # skins
        if self.prefix(src="skins"):
                self.path("paths.xml")
                # include the entire textures directory recursively
                if self.prefix(src="*/textures"):
                        self.path("*.tga")
                        self.path("*.j2c")
                        self.path("*.jpg")
                        self.path("*.png")
                        self.path("textures.xml")
                        self.end_prefix("*/textures")
                self.path("*/xui/*/*.xml")
                self.path("*/*.xml")
                
                # Local HTML files (e.g. loading screen)
                if self.prefix(src="*/html"):
                        self.path("*.png")
                        self.path("*/*/*.html")
                        self.path("*/*/*.gif")
                        self.path("*/*/*.png")
                        self.end_prefix("*/html")
                self.end_prefix("skins")
        
        # Files in the newview/ directory
        self.path("gpu_table.txt")


    # Gather up the README file, etc.
    def gather_documents(self):
        # From the top level directory (imprudence)
        if self.prefix("../../..", dst=""):
            self.path("README.txt")
            self.path("MANIFESTO.txt")
            self.path("CONTRIBUTE.txt")
            self.path("ChangeLog.txt", required=False)
            self.end_prefix("../../..")

        # From the linden directory
        if self.prefix("../..", dst="doc"):
            self.path("LICENSE-source.txt")
            self.path("LICENSE-logos.txt", "LICENSE-artwork.txt")
            self.end_prefix("../..")

        # From the linden/doc directory
        if self.prefix("../../doc", dst="doc"):
            self.path("contributions.txt")
            self.path("GPL-license.txt", "GPL.txt")
            self.path("FLOSS-exception.txt")
            self.end_prefix("../../doc")


    def login_channel(self):
        """Channel reported for login and upgrade purposes ONLY;
        used for A/B testing"""
        # NOTE: Do not return the normal channel if login_channel
        # is not specified, as some code may branch depending on
        # whether or not this is present
        return self.args.get('login_channel')

    def standalone(self):
        return self.args['standalone'] == "ON"
    def debug(self):
        return self.args['buildtype'] == "DEBUG"
    def grid(self):
        return self.args['grid']
    def channel(self):
        return self.args['channel']
    def channel_unique(self):
        return self.channel().replace("Imprudence", "").strip()
    def channel_oneword(self):
        return "".join(self.channel_unique().split())
    def channel_lowerword(self):
        return self.channel_oneword().lower()

    def flags_list(self):
        """ Convenience function that returns the command-line flags
        for the grid"""

        # Set command line flags relating to the target grid
        grid_flags = ''
        if not self.default_grid():
            grid_flags = "--grid %(grid)s "\
                         "--helperuri http://preview-%(grid)s.secondlife.com/helpers/" %\
                           {'grid':self.grid()}

        # set command line flags for channel
        channel_flags = ''
        if self.login_channel() and self.login_channel() != self.channel():
            # Report a special channel during login, but use default
            channel_flags = '--channel "%s"' % (self.login_channel())
        elif not self.default_channel():
            channel_flags = '--channel "%s"' % self.channel()

        # Deal with settings 
        setting_flags = ''
        if not self.default_channel() or not self.default_grid():
            if self.default_grid():
                setting_flags = '--settings settings_%s.xml'\
                                % self.channel_lowerword()
            else:
                setting_flags = '--settings settings_%s_%s.xml'\
                                % (self.grid(), self.channel_lowerword())
                                                
        return " ".join((channel_flags, grid_flags, setting_flags)).strip()


class WindowsManifest(ViewerManifest):
    def final_exe(self):
        if self.default_channel():
            if self.default_grid():
                return "imprudence.exe"
            else:
                return "imprudencepreview.exe"
        else:
            return ''.join(self.channel().split()) + '.exe'


    def construct(self):
        super(WindowsManifest, self).construct()
        # the final exe is complicated because we're not sure where it's coming from,
        # nor do we have a fixed name for the executable
        self.path(self.find_existing_file('debug/imprudence-bin.exe', 'release/imprudence-bin.exe', 'relwithdebinfo/imprudence-bin.exe'), dst=self.final_exe())

        self.gather_documents()

        if self.prefix("../..", dst="doc"):
            self.path("LICENSE-libraries.txt")
            self.end_prefix("../..")


        self.path("imprudence.url")

        # Plugin host application
        self.path(os.path.join(os.pardir,
                               'llplugin', 'slplugin', self.args['configuration'], "SLPlugin.exe"),
                  "SLPlugin.exe")


        self.path("featuretable.txt")

        # For use in crash reporting (generates minidumps)
        self.path("dbghelp.dll")

        # For using FMOD for sound... DJS
        #self.path("fmod.dll")
        
        # For spellchecking
        if self.prefix(src=self.args['configuration'], dst=""):
            self.path("libhunspell.dll")
            self.end_prefix()

        # Get llcommon and deps.
        if self.prefix(src=self.args['configuration'], dst=""):
            self.path('libapr-1.dll')
            self.path('libaprutil-1.dll')
            self.path('libapriconv-1.dll')
            self.path('llcommon.dll')
            self.end_prefix()

        # For textures
        if self.prefix(src="../../libraries/i686-win32/lib/release", dst=""):
            self.path("openjpeg.dll")
            self.end_prefix()

        # For sound
        if self.prefix(src="../../libraries/i686-win32/lib/release", dst=""):
            self.path("openal32.dll")
            self.path("alut.dll")
            self.end_prefix()           

        # Media plugins - QuickTime
        if self.prefix(src='../media_plugins/quicktime/%s' % self.args['configuration'], dst="llplugin"):
            self.path("media_plugin_quicktime.dll")
            self.end_prefix()

        # Media plugins - WebKit/Qt
        if self.prefix(src='../media_plugins/webkit/%s' % self.args['configuration'], dst="llplugin"):
            self.path("media_plugin_webkit.dll")
            self.end_prefix()

        # Media plugins - GStreamer
        if self.prefix(src='../media_plugins/gstreamer010/%s' % self.args['configuration'], dst="llplugin"):
            self.path("media_plugin_gstreamer010.dll")
            self.end_prefix()            
 
        # For WebKit/Qt plugin runtimes
        if self.prefix(src="../../libraries/i686-win32/lib/release", dst="llplugin"):
            self.path("libeay32.dll")
            self.path("qtcore4.dll")
            self.path("qtgui4.dll")
            self.path("qtnetwork4.dll")
            self.path("qtopengl4.dll")
            self.path("qtwebkit4.dll")
            self.path("qtxmlpatterns4.dll")
            self.path("ssleay32.dll")
            self.end_prefix()

        # For WebKit/Qt plugin runtimes (image format plugins)
        if self.prefix(src="../../libraries/i686-win32/lib/release/imageformats", dst="llplugin/imageformats"):
            self.path("qgif4.dll")
            self.path("qico4.dll")
            self.path("qjpeg4.dll")
            self.path("qmng4.dll")
            self.path("qsvg4.dll")
            self.path("qtiff4.dll")
            self.end_prefix()

        # These need to be installed as a SxS assembly, currently a 'private' assembly.
        # See http://msdn.microsoft.com/en-us/library/ms235291(VS.80).aspx
        if self.prefix(src=self.args['configuration'], dst=""):
            if self.args['configuration'] == 'Debug':
                self.path("msvcr80d.dll")
                self.path("msvcp80d.dll")
                self.path("Microsoft.VC80.DebugCRT.manifest")
            else:
                self.path("msvcr80.dll")
                self.path("msvcp80.dll")
                self.path("Microsoft.VC80.CRT.manifest")
            self.end_prefix()

        # The config file name needs to match the exe's name.
        self.path(src="%s/imprudence-bin.exe.config" % self.args['configuration'], dst=self.final_exe() + ".config")

        # We need this one too, so that llkdu loads at runtime - DEV-41194
        #self.path(src="%s/imprudence-bin.exe.config" % self.args['configuration'], dst="llkdu.dll.2.config")
        self.path("llkdu.dll.2.config")

        # We need this one too, so that win_crash_logger.exe loads at runtime - DEV-19004
        #self.path(src="%s/imprudence-bin.exe.config" % self.args['configuration'], dst="win_crash_logger.exe.config")

        # same thing for auto-updater.
        #self.path(src="%s/imprudence-bin.exe.config" % self.args['configuration'], dst="updater.exe.config")

        # Vivox runtimes
        if self.prefix(src="vivox-runtime/i686-win32", dst=""):
        #    self.path("alut.dll")
            self.path("wrap_oal.dll")
            self.path("SLVoice.exe")
        #    self.path("SLVoiceAgent.exe")
        #    self.path("libeay32.dll")
        #    self.path("srtp.dll")
        #    self.path("ssleay32.dll")
        #    self.path("tntk.dll")
            self.path("vivoxsdk.dll")
            self.path("ortp.dll")

            self.end_prefix()

        # Gstreamer plugins
        if self.prefix(src="lib/gstreamer-plugins", dst=""):
            self.path("*.dll", dst="lib/gstreamer-plugins/*.dll")
            self.end_prefix()

        # Gstreamer libs
        if (not self.standalone()) and self.prefix(src="../../libraries/i686-win32/lib/release", dst=""):
            self.path("avcodec-gpl-52.dll")
            self.path("avdevice-gpl-52.dll")
            self.path("avfilter-gpl-1.dll")
            self.path("avformat-gpl-52.dll")
            self.path("avutil-gpl-50.dll")
            self.path("iconv.dll")
            self.path("liba52-0.dll")
            self.path("libbz2.dll")
            self.path("libcelt-0.dll")
            self.path("libdca-0.dll")
            self.path("libexpat-1.dll")
            self.path("libfaad-2.dll")
            self.path("libFLAC-8.dll")
            self.path("libgcrypt-11.dll")
            self.path("libgio-2.0-0.dll")
            self.path("libglib-2.0-0.dll")
            self.path("libgmodule-2.0-0.dll")
            self.path("libgnutls-26.dll")
            self.path("libgobject-2.0-0.dll")
            self.path("libgpg-error-0.dll")
            self.path("libgstapp-0.10.dll")
            self.path("libgstaudio-0.10.dll")
            self.path("libgstbase-0.10.dll")
            self.path("libgstcontroller-0.10.dll")
            self.path("libgstdataprotocol-0.10.dll")
            self.path("libgstfft-0.10.dll")
            self.path("libgstinterfaces-0.10.dll")
            self.path("libgstnet-0.10.dll")
            self.path("libgstnetbuffer-0.10.dll")
            self.path("libgstpbutils-0.10.dll")
            self.path("libgstphotography-0.10.dll")
            self.path("libgstreamer-0.10.dll")
            self.path("libgstriff-0.10.dll")
            self.path("libgstrtp-0.10.dll")
            self.path("libgstrtsp-0.10.dll")
            self.path("libgstsdp-0.10.dll")
            self.path("libgstsignalprocessor-0.10.dll")
            self.path("libgsttag-0.10.dll")
            self.path("libgstvideo-0.10.dll")
            self.path("libgthread-2.0-0.dll")
            self.path("libmms-0.dll")
            self.path("libmpeg2-0.dll")
            self.path("libneon-27.dll")
            self.path("libogg-0.dll")
            self.path("liboil-0.3-0.dll")
            self.path("libsoup-2.4-1.dll")
            self.path("libtasn1-3.dll")
            self.path("libtheora-0.dll")
            self.path("libtheoradec-1.dll")
            self.path("libvorbis-0.dll")
            self.path("libvorbisenc-2.dll")
            self.path("libvorbisfile-3.dll")
            self.path("libwavpack-1.dll")
            self.path("libx264-67.dll")
            self.path("libxml2-2.dll")
            self.path("libxml2.dll")
            self.path("SDL.dll")
            self.path("xvidcore.dll")
            self.path("z.dll")
            self.end_prefix()

#        # pull in the crash logger and updater from other projects
#        self.path(src=self.find_existing_file( # tag:"crash-logger" here as a cue to the exporter
#                "../win_crash_logger/debug/windows-crash-logger.exe",
#                "../win_crash_logger/release/windows-crash-logger.exe",
#                "../win_crash_logger/relwithdebinfo/windows-crash-logger.exe"),
#                  dst="win_crash_logger.exe")
        self.path(src=self.find_existing_file(
                "../win_updater/debug/windows-updater.exe",
                "../win_updater/release/windows-updater.exe",
                "../win_updater/relwithdebinfo/windows-updater.exe"),
                  dst="updater.exe")

        # For google-perftools tcmalloc allocator.
        #if self.prefix(src="../../libraries/i686-win32/lib/release", dst=""):
        #        self.path("libtcmalloc_minimal.dll")
        #        self.end_prefix()


    def nsi_file_commands(self, install=True):
        def wpath(path):
            if path.endswith('/') or path.endswith(os.path.sep):
                path = path[:-1]
            path = path.replace('/', '\\')
            return path

        result = ""
        dest_files = [pair[1] for pair in self.file_list if pair[0] and os.path.isfile(pair[1])]
        # sort deepest hierarchy first
        dest_files.sort(lambda a,b: cmp(a.count(os.path.sep),b.count(os.path.sep)) or cmp(a,b))
        dest_files.reverse()
        out_path = None
        for pkg_file in dest_files:
            rel_file = os.path.normpath(pkg_file.replace(self.get_dst_prefix()+os.path.sep,''))
            installed_dir = wpath(os.path.join('$INSTDIR', os.path.dirname(rel_file)))
            pkg_file = wpath(os.path.normpath(pkg_file))
            if installed_dir != out_path:
                if install:
                    out_path = installed_dir
                    result += 'SetOutPath ' + out_path + '\n'
            if install:
                result += 'File ' + pkg_file + '\n'
            else:
                result += 'Delete ' + wpath(os.path.join('$INSTDIR', rel_file)) + '\n'
        # at the end of a delete, just rmdir all the directories
        if not install:
            deleted_file_dirs = [os.path.dirname(pair[1].replace(self.get_dst_prefix()+os.path.sep,'')) for pair in self.file_list]
            # find all ancestors so that we don't skip any dirs that happened to have no non-dir children
            deleted_dirs = []
            for d in deleted_file_dirs:
                deleted_dirs.extend(path_ancestors(d))
            # sort deepest hierarchy first
            deleted_dirs.sort(lambda a,b: cmp(a.count(os.path.sep),b.count(os.path.sep)) or cmp(a,b))
            deleted_dirs.reverse()
            prev = None
            for d in deleted_dirs:
                if d != prev:   # skip duplicates
                    result += 'RMDir ' + wpath(os.path.join('$INSTDIR', os.path.normpath(d))) + '\n'
                prev = d

        return result

    def package_finish(self):
        # a standard map of strings for replacing in the templates
        substitution_strings = {
            'version' : '.'.join(self.args['version']).replace(' ', '_'),
            'version_short' : '.'.join(self.args['version'][:-1]).replace(' ', '_'),
            'version_dashes' : '-'.join(self.args['version']).replace(' ', '_'),
            'final_exe' : self.final_exe(),
            'grid':self.args['grid'],
            'grid_caps':self.args['grid'].upper(),
            # escape quotes becase NSIS doesn't handle them well
            'flags':self.flags_list().replace('"', '$\\"'),
            'channel':self.channel(),
            'channel_oneword':self.channel_oneword(),
            'channel_unique':self.channel_unique(),
            }

        version_vars = """
        !define INSTEXE  "%(final_exe)s"
        !define VERSION "%(version_short)s"
        !define VERSION_LONG "%(version)s"
        !define VERSION_DASHES "%(version_dashes)s"
        """ % substitution_strings
        if self.default_channel():
            if self.default_grid():
                # release viewer
                installer_file = "Imprudence_%(version_dashes)s_Setup.exe"
                grid_vars_template = """
                OutFile "%(installer_file)s"
                !define INSTFLAGS "%(flags)s"
                !define INSTNAME   "Imprudence"
                !define SHORTCUT   "Imprudence"
                !define URLNAME   "imprudence"
                Caption "Imprudence ${VERSION}"
                """
            else:
                # beta grid viewer
                installer_file = "Imprudence_%(version_dashes)s_(%(grid_caps)s)_Setup.exe"
                grid_vars_template = """
                OutFile "%(installer_file)s"
                !define INSTFLAGS "%(flags)s"
                !define INSTNAME   "Imprudence%(grid_caps)s"
                !define SHORTCUT   "Imprudence (%(grid_caps)s)"
                !define URLNAME   "imprudence%(grid)s"
                !define UNINSTALL_SETTINGS 1
                Caption "Imprudence %(grid)s ${VERSION}"
                """
        else:
            # some other channel on some grid
            installer_file = "Imprudence_%(version_dashes)s_%(channel_oneword)s_Setup.exe"
            grid_vars_template = """
            OutFile "%(installer_file)s"
            !define INSTFLAGS "%(flags)s"
            !define INSTNAME   "Imprudence%(channel_oneword)s"
            !define SHORTCUT   "%(channel)s"
            !define URLNAME   "imprudence"
            !define UNINSTALL_SETTINGS 1
            Caption "%(channel)s ${VERSION}"
            """
        if 'installer_name' in self.args:
            installer_file = self.args['installer_name']
        else:
            installer_file = installer_file % substitution_strings
        substitution_strings['installer_file'] = installer_file

        tempfile = "imprudence_setup_tmp.nsi"
        # the following replaces strings in the nsi template
        # it also does python-style % substitution
        self.replace_in("installers/windows/installer_template.nsi", tempfile, {
                "%%VERSION%%":version_vars,
                "%%SOURCE%%":self.get_src_prefix(),
                "%%GRID_VARS%%":grid_vars_template % substitution_strings,
                "%%INSTALL_FILES%%":self.nsi_file_commands(True),
                "%%DELETE_FILES%%":self.nsi_file_commands(False)})

        # We use the Unicode version of NSIS, available from
        # http://www.scratchpaper.com/
        try:
          import _winreg as reg
          NSIS_path = reg.QueryValue(reg.HKEY_LOCAL_MACHINE, r"SOFTWARE\NSIS\Unicode") + '\\makensis.exe'
          self.run_command('"' + proper_windows_path(NSIS_path) + '" ' + self.dst_path_of(tempfile))
        except:
          try:
            NSIS_path = os.environ['ProgramFiles'] + '\\NSIS\\Unicode\\makensis.exe'
            self.run_command('"' + proper_windows_path(NSIS_path) + '" ' + self.dst_path_of(tempfile))
          except:
            NSIS_path = os.environ['ProgramFiles(X86)'] + '\\NSIS\\Unicode\\makensis.exe'
        self.run_command('"' + proper_windows_path(NSIS_path) + '" ' + self.dst_path_of(tempfile))
        # self.remove(self.dst_path_of(tempfile))
        # If we're on a build machine, sign the code using our Authenticode certificate. JC
        sign_py = 'C:\\buildscripts\\code-signing\\sign.py'
        if os.path.exists(sign_py):
            self.run_command(sign_py + ' ' + self.dst_path_of(installer_file))
        else:
            print "Skipping code signing,", sign_py, "does not exist"
        self.created_path(self.dst_path_of(installer_file))
        self.package_file = installer_file


class DarwinManifest(ViewerManifest):
    def construct(self):
        # copy over the build result (this is a no-op if run within the xcode script)
        self.path(self.args['configuration'] + "/Imprudence.app", dst="")

        if self.prefix(src="", dst="Contents"):  # everything goes in Contents
            
            self.path("Info-Imprudence.plist", dst="Info.plist")

            # copy additional libs in <bundle>/Contents/MacOS/
            if (not self.standalone()) and self.prefix(src="../../libraries/universal-darwin/lib_release", dst="MacOS/"):

                self.path("libndofdev.dylib")
                self.path("libhunspell-1.2.dylib")
                
                self.path("libopenal.1.dylib")
                self.path("libalut.0.dylib")

                # self.path("libglib-2.0.dylib")
                # self.path("libgmodule-2.0.dylib")
                # self.path("libgobject-2.0.dylib")
                # self.path("libgthread-2.0.dylib")
                
                # self.path("libgstreamer-0.10.dylib")
                # self.path("libgstapp-0.10.dylib")
                # self.path("libgstaudio-0.10.dylib")
                # self.path("libgstbase-0.10.dylib")
                # self.path("libgstcdda-0.10.dylib")
                # self.path("libgstcontroller-0.10.dylib")
                # self.path("libgstdataprotocol-0.10.dylib")
                # self.path("libgstfft-0.10.dylib")
                # self.path("libgstinterfaces-0.10.dylib")
                # self.path("libgstnet-0.10.dylib")
                # self.path("libgstnetbuffer-0.10.dylib")
                # self.path("libgstpbutils-0.10.dylib")
                # self.path("libgstriff-0.10.dylib")
                # self.path("libgstrtp-0.10.dylib")
                # self.path("libgstrtsp-0.10.dylib")
                # self.path("libgstsdp-0.10.dylib")
                # self.path("libgsttag-0.10.dylib")
                # self.path("libgstvideo-0.10.dylib")

                # self.path("libxml2.2.dylib")
                # self.path("libfaad.2.dylib")
                # self.path("libFLAC.8.dylib")
                # self.path("libintl.3.dylib")
                self.path("libjpeg.62.dylib")
                self.path("libpng12.0.dylib")
                # self.path("libneon.27.dylib")
                self.path("libogg.0.dylib")
                # self.path("liboil-0.3.0.dylib")
                self.path("libopenjpeg.1.4.dylib")
                # self.path("libtheora.0.dylib")
                self.path("libvorbis.0.dylib")
                self.path("libvorbisenc.2.dylib")
                self.path("libvorbisfile.3.dylib")

                self.end_prefix("../../libraries/universal-darwin/lib_release")

            # most everything goes in the Resources directory
            if self.prefix(src="", dst="Resources"):
                super(DarwinManifest, self).construct()

                if self.prefix("cursors_mac"):
                    self.path("*.tif")
                    self.end_prefix("cursors_mac")

                # From the linden directory
                if self.prefix("../..", dst="doc"):
                    self.path("LICENSE-libraries.txt")
                    self.end_prefix("../..")

                self.gather_documents()

                self.path("featuretable_mac.txt")
                self.path("SecondLife.nib")

                self.path("viewer.icns")
                
                # Translations
                self.path("English.lproj")
                self.path("German.lproj")
                self.path("Japanese.lproj")
                self.path("Korean.lproj")
                self.path("da.lproj")
                self.path("es.lproj")
                self.path("fr.lproj")
                self.path("hu.lproj")
                self.path("it.lproj")
                self.path("nl.lproj")
                self.path("pl.lproj")
                self.path("pt.lproj")
                self.path("ru.lproj")
                self.path("tr.lproj")
                self.path("uk.lproj")
                self.path("zh-Hans.lproj")


                # if (not self.standalone()) and self.prefix(src="../../libraries/universal-darwin/lib_release/gstreamer-plugins", dst="lib/gstreamer-plugins"):
                #     self.path("libgstaacparse.so")
                #     self.path("libgstadder.so")
                #     self.path("libgstaiffparse.so")
                #     self.path("libgstamrparse.so")
                #     self.path("libgstapp.so")
                #     self.path("libgstaudioconvert.so")
                #     self.path("libgstaudiorate.so")
                #     self.path("libgstaudioresample.so")
                #     self.path("libgstautodetect.so")
                #     self.path("libgstavi.so")
                #     self.path("libgstcoreelements.so")
                #     self.path("libgstcoreindexers.so")
                #     self.path("libgstdebug.so")
                #     self.path("libgstdecodebin.so")
                #     self.path("libgstdecodebin2.so")
                #     self.path("libgstdeinterlace2.so")
                #     self.path("libgstequalizer.so")
                #     self.path("libgstfaad.so")
                #     self.path("libgstffmpeg.so")
                #     self.path("libgstffmpegcolorspace.so")
                #     self.path("libgstffmpegscale.so")
                #     self.path("libgstfilter.so")
                #     self.path("libgstflac.so")
                #     self.path("libgstflv.so")
                #     self.path("libgstgdp.so")
                #     self.path("libgsth264parse.so")
                #     self.path("libgsticydemux.so")
                #     self.path("libgstid3demux.so")
                #     self.path("libgstinterleave.so")
                #     self.path("libgstjpeg.so")
                #     self.path("libgstlevel.so")
                #     self.path("libgstmetadata.so")
                #     self.path("libgstmpeg4videoparse.so")
                #     self.path("libgstmpegdemux.so")
                #     self.path("libgstmpegvideoparse.so")
                #     self.path("libgstmultifile.so")
                #     self.path("libgstmultipart.so")
                #     self.path("libgstneonhttpsrc.so")
                #     self.path("libgstogg.so")
                #     self.path("libgstosxaudio.so")
                #     self.path("libgstosxvideosink.so")
                #     self.path("libgstplaybin.so")
                #     self.path("libgstpng.so")
                #     self.path("libgstpostproc.so")
                #     self.path("libgstqtdemux.so")
                #     #self.path("libgstqtwrapper.so")
                #     self.path("libgstqueue2.so")
                #     self.path("libgstreal.so")
                #     self.path("libgstrtp.so")
                #     self.path("libgstrtpmanager.so")
                #     self.path("libgstrtsp.so")
                #     self.path("libgstsdpelem.so")
                #     self.path("libgstselector.so")
                #     self.path("libgststereo.so")
                #     self.path("libgsttcp.so")
                #     self.path("libgsttheora.so")
                #     self.path("libgsttypefindfunctions.so")
                #     self.path("libgstudp.so")
                #     self.path("libgstvideobalance.so")
                #     self.path("libgstvideobox.so")
                #     self.path("libgstvideocrop.so")
                #     self.path("libgstvideoflip.so")
                #     self.path("libgstvideomixer.so")
                #     self.path("libgstvideorate.so")
                #     self.path("libgstvideoscale.so")
                #     self.path("libgstvideosignal.so")
                #     self.path("libgstvolume.so")
                #     self.path("libgstvorbis.so")
                #     self.path("libgstwavparse.so")
                    
                #     self.end_prefix("../../libraries/universal-darwin/lib_release/gstreamer-plugins")


                # SLVoice and vivox lols
                self.path("vivox-runtime/universal-darwin/libalut.dylib", "libalut.dylib")
                self.path("vivox-runtime/universal-darwin/libopenal.dylib", "libopenal.dylib")
                self.path("vivox-runtime/universal-darwin/libortp.dylib", "libortp.dylib")
                self.path("vivox-runtime/universal-darwin/libvivoxsdk.dylib", "libvivoxsdk.dylib")
                self.path("vivox-runtime/universal-darwin/SLVoice", "SLVoice")
                #self.path("vivox-runtime/universal-darwin/SLVoiceAgent.app", "SLVoiceAgent.app")

                libdir = "../../libraries/universal-darwin/lib_release"

                self.path(os.path.join(os.pardir,
                                       "llcommon",
                                       self.args['configuration'],
                                       "libllcommon.dylib"),
                          dst="libllcommon.dylib")


                for libfile in ("libapr-1.0.3.7.dylib",
                                "libaprutil-1.0.3.8.dylib",
                                "libexpat.0.5.0.dylib"):
                    self.path(os.path.join(libdir, libfile), libfile)

                # our apps
                # self.path("../mac_crash_logger/" + self.args['configuration'] + "/mac-crash-logger.app", "mac-crash-logger.app")
                # self.path("../mac_updater/" + self.args['configuration'] + "/mac-updater.app", "mac-updater.app")

                # plugin launcher
                self.path("../llplugin/slplugin/" + self.args['configuration'] + "/SLPlugin.app", "SLPlugin.app")

                # symlinks for SLPlugin.app dependencies
                slplugin_res_path = self.dst_path_of("SLPlugin.app/Contents/Resources")
                for libfile in ("libllcommon.dylib",
                                "libapr-1.0.3.7.dylib",
                                "libaprutil-1.0.3.8.dylib",
                                "libexpat.0.5.0.dylib",
                                ):
                    target_lib = os.path.join('../../..', libfile)
                    self.run_command("ln -sf %(target)r %(link)r" % 
                                     {'target': target_lib,
                                      'link' : os.path.join(slplugin_res_path, libfile)}
                                     )

                # plugins
                if self.prefix(src="", dst="llplugin"):
                    self.path("../media_plugins/quicktime/" + self.args['configuration'] + "/media_plugin_quicktime.dylib", "media_plugin_quicktime.dylib")
                    self.path("../media_plugins/webkit/" + self.args['configuration'] + "/media_plugin_webkit.dylib", "media_plugin_webkit.dylib")
                    self.path("../../libraries/universal-darwin/lib_release/libllqtwebkit.dylib", "libllqtwebkit.dylib")

                    self.end_prefix("llplugin")

                # Per platform MIME config on the cheap.  See SNOW-307 / DEV-41388
                self.path("skins/default/xui/en-us/mime_types_mac.xml", "skins/default/xui/en-us/mime_types.xml")
 
                # command line arguments for connecting to the proper grid
                self.put_in_file(self.flags_list(), 'arguments.txt')

                self.end_prefix("Resources")

            self.end_prefix("Contents")

        # NOTE: the -S argument to strip causes it to keep enough info for
        # annotated backtraces (i.e. function names in the crash log).  'strip' with no
        # arguments yields a slightly smaller binary but makes crash logs mostly useless.
        # This may be desirable for the final release.  Or not.
        if ("package" in self.args['actions'] or 
            "unpacked" in self.args['actions']):
            self.run_command('strip -S "%(viewer_binary)s"' %
                             { 'viewer_binary' : self.dst_path_of('Contents/MacOS/Second Life')})


    def package_finish(self):
        channel_standin = 'Imprudence'  # hah, our default channel is not usable on its own
        if not self.default_channel():
            channel_standin = self.channel()

        imagename="Imprudence_" + '_'.join(self.args['version'])

        # MBW -- If the mounted volume name changes, it breaks the .DS_Store's background image and icon positioning.
        #  If we really need differently named volumes, we'll need to create multiple DS_Store file images, or use some other trick.

        volname="Imprudence Installer"  # DO NOT CHANGE without understanding comment above

        if self.default_channel():
            if not self.default_grid():
                # beta case
                imagename = imagename + '_' + self.args['grid'].upper()
        else:
            # first look, etc
            imagename = imagename + '_' + self.channel_oneword().upper()

        sparsename = imagename + ".sparseimage"
        finalname = imagename + ".dmg"
        # make sure we don't have stale files laying about
        self.remove(sparsename, finalname)

        self.run_command('hdiutil create "%(sparse)s" -volname "%(vol)s" -fs HFS+ -type SPARSE -megabytes 400 -layout SPUD' % {
                'sparse':sparsename,
                'vol':volname})

        # mount the image and get the name of the mount point and device node
        hdi_output = self.run_command('hdiutil attach -private "' + sparsename + '"')
        devfile = re.search("/dev/disk([0-9]+)[^s]", hdi_output).group(0).strip()
        volpath = re.search('HFS\s+(.+)', hdi_output).group(1).strip()

        # Copy everything in to the mounted .dmg

        if self.default_channel() and not self.default_grid():
            app_name = "Imprudence " + self.args['grid']
        else:
            app_name = channel_standin.strip()

        # Hack:
        # Because there is no easy way to coerce the Finder into positioning
        # the app bundle in the same place with different app names, we are
        # adding multiple .DS_Store files to svn. There is one for release,
        # one for release candidate and one for first look. Any other channels
        # will use the release .DS_Store, and will look broken.
        # - Ambroff 2008-08-20
        dmg_template = os.path.join(
            'installers', 
            'darwin',
            '%s-dmg' % "".join(self.channel_unique().split()).lower())

        if not os.path.exists (self.src_path_of(dmg_template)):
            dmg_template = os.path.join ('installers', 'darwin', 'release-dmg')

        for s,d in {self.get_dst_prefix():app_name + ".app",
                    os.path.join(dmg_template, "_VolumeIcon.icns"): ".VolumeIcon.icns",
                    os.path.join(dmg_template, "background.jpg"): "background.jpg",
                    os.path.join(dmg_template, "_DS_Store"): ".DS_Store"}.items():
            print "Copying to dmg", s, d
            self.copy_action(self.src_path_of(s), os.path.join(volpath, d))

        # Hide the background image, DS_Store file, and volume icon file (set their "visible" bit)
        self.run_command('SetFile -a V "' + os.path.join(volpath, ".VolumeIcon.icns") + '"')
        self.run_command('SetFile -a V "' + os.path.join(volpath, "background.jpg") + '"')
        self.run_command('SetFile -a V "' + os.path.join(volpath, ".DS_Store") + '"')

        # Create the alias file (which is a resource file) from the .r
        self.run_command('rez "' + self.src_path_of("installers/darwin/release-dmg/Applications-alias.r") + '" -o "' + os.path.join(volpath, "Applications") + '"')

        # Set the alias file's alias and custom icon bits
        self.run_command('SetFile -a AC "' + os.path.join(volpath, "Applications") + '"')

        # Set the disk image root's custom icon bit
        self.run_command('SetFile -a C "' + volpath + '"')

        # Unmount the image
        self.run_command('hdiutil detach -force "' + devfile + '"')

        print "Converting temp disk image to final disk image"
        self.run_command('hdiutil convert "%(sparse)s" -format UDZO -imagekey zlib-level=9 -o "%(final)s"' % {'sparse':sparsename, 'final':finalname})
        # get rid of the temp file
        self.package_file = finalname
        self.remove(sparsename)

class LinuxManifest(ViewerManifest):
    def construct(self):
        super(LinuxManifest, self).construct()

        self.path("res/imprudence_icon.png","imprudence_icon.png")
        if self.prefix("linux_tools", dst=""):
            self.path("alsoft.conf")
            #self.path("client-readme.txt","README-linux.txt")
            self.path("client-readme-voice.txt","README-linux-voice.txt")
            #self.path("client-readme-joystick.txt","README-linux-joystick.txt")
            self.path("wrapper.sh","imprudence")
            self.path("handle_secondlifeprotocol.sh")
            self.path("register_secondlifeprotocol.sh")
            self.end_prefix("linux_tools")

        self.gather_documents()

        # From the linden directory
        if self.prefix("../..", dst="doc"):
            self.path("LICENSE-libraries.txt")
            self.end_prefix("../..")

        # Create an appropriate gridargs.dat for this package, denoting required grid.
        self.put_in_file(self.flags_list(), 'gridargs.dat')
        self.path("linux_tools/launch_url.sh","launch_url.sh")
        self.path("../llplugin/slplugin/SLPlugin", "bin/SLPlugin")
        if self.prefix("res-sdl"):
            self.path("*")
            # recurse
            self.end_prefix("res-sdl")

        # plugins
        if self.prefix(src="", dst="bin/llplugin"):
            self.path("../media_plugins/webkit/libmedia_plugin_webkit.so", "libmedia_plugin_webkit.so")
            self.path("../media_plugins/gstreamer010/libmedia_plugin_gstreamer010.so", "libmedia_plugin_gstreamer.so")
            self.end_prefix("bin/llplugin")

        # Per platform MIME config on the cheap.  See SNOW-307 / DEV-41388
        self.path("skins/default/xui/en-us/mime_types_linux.xml", "skins/default/xui/en-us/mime_types.xml")

        self.path("featuretable_linux.txt")


    def package_finish(self):
        if 'installer_name' in self.args:
            installer_name = self.args['installer_name']
        else:
            installer_name_components = ['Imprudence_', self.args.get('arch')]
            installer_name_components.extend(self.args['version'])
            installer_name = "_".join(installer_name_components)
            if self.default_channel():
                if not self.default_grid():
                    installer_name += '_' + self.args['grid'].upper()
            else:
                installer_name += '_' + self.channel_oneword().upper()

        # Fix access permissions
        self.run_command("""
                find %(dst)s -type d | xargs --no-run-if-empty chmod 755;
                find %(dst)s -type f -perm 0700 | xargs --no-run-if-empty chmod 0755;
                find %(dst)s -type f -perm 0500 | xargs --no-run-if-empty chmod 0555;
                find %(dst)s -type f -perm 0600 | xargs --no-run-if-empty chmod 0644;
                find %(dst)s -type f -perm 0400 | xargs --no-run-if-empty chmod 0444;
                true""" %  {'dst':self.get_dst_prefix() })

        self.package_file = installer_name + '.tar.bz2'

        # Disabled for now. It's a waste of time to package every compile.

        # if("package" in self.args['actions'] or
        #    "unpacked" in self.args['actions']):
        #
        #     # temporarily move directory tree so that it has the right
        #     # name in the tarfile
        #     self.run_command("mv %(dst)s %(inst)s" % {
        #         'dst': self.get_dst_prefix(),
        #         'inst': self.build_path_of(installer_name)})
        #     try:
        #         # --numeric-owner hides the username of the builder for
        #         # security etc.
        #         self.run_command('tar -C %(dir)s --numeric-owner -cjf '
        #                          '%(inst_path)s.tar.bz2 %(inst_name)s' % {
        #             'dir': self.get_build_prefix(),
        #             'inst_name': installer_name,
        #             'inst_path':self.build_path_of(installer_name)})
        #     finally:
        #         self.run_command("mv %(inst)s %(dst)s" % {
        #             'dst': self.get_dst_prefix(),
        #             'inst': self.build_path_of(installer_name)})

class Linux_i686Manifest(LinuxManifest):
    def construct(self):
        super(Linux_i686Manifest, self).construct()
        if self.debug():
            self.path("imprudence-bin","bin/do-not-directly-run-imprudence-bin")
        else:
            self.path("imprudence-stripped","bin/do-not-directly-run-imprudence-bin")

        self.path("../llcommon/libllcommon.so", "lib/libllcommon.so")

        if (not self.standalone()) and self.prefix("../../libraries/i686-linux/lib_release_client", dst="lib"):
            self.path("libapr-1.so.0")
            self.path("libaprutil-1.so.0")
            self.path("libdb-4.2.so")
            self.path("libcrypto.so.0.9.7")
            self.path("libexpat.so.1")
            self.path("libhunspell-1.2.so.0.0.0", "libhunspell-1.2.so.0")
            self.path("libssl.so.0.9.7")
            self.path("libuuid.so", "libuuid.so.1")
            self.path("libSDL-1.2.so.0")
            self.path("libELFIO.so")
            self.path("libopenjpeg.so.2")
            self.path("libxml2.so.2")
            self.path("libz.so")
            self.path("libz.so.1")

            # OpenAL
            self.path("libopenal.so.1.12.854", "libopenal.so.1")
            self.path("libalut.so.0.1.0", "libalut.so.0")

            # GTK+ and dependencies
            ## Lets just use the system libraries for all of these:
            ##self.path("libatk-1.0.so.0")
            ##self.path("libcairo.so.2")
            ##self.path("libfontconfig.so.1")
            ##self.path("libfreetype.so.6")
           # self.path("libgdk_pixbuf-2.0.so.0") # see linux64 why
            ##self.path("libgdk-x11-2.0.so.0")
            ##self.path("libgtk-x11-2.0.so.0")
           # self.path("libpango-1.0.so.0")	 # dto.
           # self.path("libpangoft2-1.0.so.0")
           # self.path("libpangox-1.0.so.0")
           # self.path("libpangoxft-1.0.so.0")
            ##self.path("libpixman-1.so.0")

#KILL IT WITH FIRE
            ## Gstreamer libs
            #self.path("libgstbase-0.10.so.0")
            #self.path("libgstreamer-0.10.so.0")
            #self.path("libgstaudio-0.10.so.0")
            #self.path("libgstbase-0.10.so.0")
            #self.path("libgstcontroller-0.10.so.0")
            #self.path("libgstdataprotocol-0.10.so.0")
            #self.path("libgstinterfaces-0.10.so.0")
            #self.path("libgstnetbuffer-0.10.so.0")
            #self.path("libgstpbutils-0.10.so.0")
            #self.path("libgstriff-0.10.so.0")
            #self.path("libgstrtp-0.10.so.0")
            #self.path("libgstrtsp-0.10.so.0")
            #self.path("libgstsdp-0.10.so.0")
            #self.path("libgsttag-0.10.so.0")
            #self.path("libgstvideo-0.10.so.0")

            ## Gstreamer plugin dependencies
            #self.path("libfaad.so.0")
            #self.path("libogg.so.0")
            #self.path("libtheora.so.0")
            #self.path("libvorbis.so.0")
            #self.path("libvorbisenc.so.2")
            #self.path("liboil-0.3.so.0")

            ## Gstreamer plugins
            #if self.prefix("gstreamer-plugins"):
                #self.path("libgstalsa.so")
                #self.path("libgstasf.so")
                #self.path("libgstaudioconvert.so")
                #self.path("libgstaudioresample.so")
                #self.path("libgstautodetect.so")
                #self.path("libgstavi.so")
                #self.path("libgstcoreelements.so")
                #self.path("libgstcoreindexers.so")
                #self.path("libgstdecodebin2.so")
                #self.path("libgstdecodebin.so")
                #self.path("libgstesd.so")
                #self.path("libgstfaad.so")
                #self.path("libgstffmpeg.so")
                #self.path("libgstgnomevfs.so")
                #self.path("libgsticydemux.so")
                #self.path("libgstid3demux.so")
                #self.path("libgstmpegdemux.so")
                #self.path("libgstmultifile.so")
                #self.path("libgstmultipart.so")
                #self.path("libgstogg.so")
                #self.path("libgstossaudio.so")
                #self.path("libgstplaybin.so")
                #self.path("libgstpulse.so")
                #self.path("libgstqtdemux.so")
                #self.path("libgstqueue2.so")
                #self.path("libgsttcp.so")
                #self.path("libgsttheora.so")
                #self.path("libgsttypefindfunctions.so")
                #self.path("libgstudp.so")
                #self.path("libgstvideoscale.so")
                #self.path("libgstvolume.so")
                #self.path("libgstvorbis.so")
                #self.path("libgstwavparse.so")
                
                #self.end_prefix("gstreamer-plugins")
            
            self.end_prefix("lib")

            # Vivox runtimes and libs
            if self.prefix(src="vivox-runtime/i686-linux", dst="bin"):
                self.path("SLVoice")
                self.end_prefix("bin")

            if self.prefix(src="vivox-runtime/i686-linux", dst="lib"):
                self.path("libalut.so")
                self.path("libortp.so")
                self.path("libvivoxsdk.so")
                self.end_prefix("lib")

class Linux_x86_64Manifest(LinuxManifest):
    def construct(self):
        super(Linux_x86_64Manifest, self).construct()
        if self.debug():
            self.path("imprudence-bin","bin/do-not-directly-run-imprudence-bin")
        else:
            self.path("imprudence-stripped","bin/do-not-directly-run-imprudence-bin")
#        self.path("../linux_crash_logger/linux-crash-logger-stripped","linux-crash-logger.bin")

        self.path("../llcommon/libllcommon.so", "lib64/libllcommon.so")

        self.path("linux_tools/launch_url.sh","launch_url.sh")
        if self.prefix("res-sdl"):
            self.path("*")
            # recurse
            self.end_prefix("res-sdl")

        self.path("featuretable_linux.txt")
        #self.path("secondlife-x86_64.supp")

        if (not self.standalone()) and self.prefix("../../libraries/x86_64-linux/lib_release_client", dst="lib64"):
            self.path("libapr-1.so.0")
            self.path("libaprutil-1.so.0")
            self.path("libdb-4.2.so")
            self.path("libcrypto.so.0.9.8")
            self.path("libexpat.so.1")
            self.path("libhunspell-1.2.so.0.0.0", "libhunspell-1.2.so.0")
            self.path("libssl.so.0.9.8")
            self.path("libuuid.so", "libuuid.so.1")
            self.path("libSDL-1.2.so.0")
            self.path("libELFIO.so")
            self.path("libjpeg.so.7")
            self.path("libpng12.so.0")
            self.path("libopenjpeg.so.2")
            self.path("libxml2.so.2")
            #self.path("libz.so.1") #not needed

            # OpenAL
            self.path("libopenal.so.1.12.854", "libopenal.so.1")
            self.path("libalut.so.0.1.0", "libalut.so.0")
            # GTK+ and dependencies
            ## Lets just use the system libraries for all of these:
            ##self.path("libatk-1.0.so.0")
            ##self.path("libcairo.so.2")
            ##self.path("libfontconfig.so.1")
            ##self.path("libfreetype.so.6")
            #self.path("libgdk_pixbuf-2.0.so.0")
            ##self.path("libgdk-x11-2.0.so.0")
            ##self.path("libgtk-x11-2.0.so.0")
#            self.path("libpango-1.0.so.0")		# use systems pango instead
#            self.path("libpangoft2-1.0.so.0")		# Both gdk pixbufs and pango would load systems modules
#            self.path("libpangox-1.0.so.0")		# and crash if not compatible or present. 
#            self.path("libpangoxft-1.0.so.0")		# So we depend system gdk pixbufs and pango anyway.
            ##self.path("libpixman-1.so.0")

#KILL IT WITH FIRE
            ## Gstreamer libs
            #self.path("libgstbase-0.10.so.0")
            #self.path("libgstreamer-0.10.so.0")
            #self.path("libgstaudio-0.10.so.0")
            #self.path("libgstbase-0.10.so.0")
            #self.path("libgstcontroller-0.10.so.0")
            #self.path("libgstdataprotocol-0.10.so.0")
            #self.path("libgstinterfaces-0.10.so.0")
            #self.path("libgstnetbuffer-0.10.so.0")
            #self.path("libgstpbutils-0.10.so.0")
            #self.path("libgstriff-0.10.so.0")
            #self.path("libgstrtp-0.10.so.0")
            #self.path("libgstrtsp-0.10.so.0")
            #self.path("libgstsdp-0.10.so.0")
            #self.path("libgsttag-0.10.so.0")
            #self.path("libgstvideo-0.10.so.0")

            ## Gstreamer plugin dependencies
            #self.path("libfaad.so.0")
            #self.path("libogg.so.0")
            #self.path("libtheora.so.0")
            #self.path("libvorbis.so.0")
            #self.path("libvorbisenc.so.2")
            #self.path("liboil-0.3.so.0")

            ## Gstreamer plugins
            #if self.prefix("gstreamer-plugins"):
                #self.path("libgstalsa.so")
                #self.path("libgstasf.so")
                #self.path("libgstaudioconvert.so")
                #self.path("libgstaudioresample.so")
                #self.path("libgstautodetect.so")
                #self.path("libgstavi.so")
                #self.path("libgstcoreelements.so")
                #self.path("libgstcoreindexers.so")
                #self.path("libgstdecodebin2.so")
                #self.path("libgstdecodebin.so")
                #self.path("libgstesd.so")
                #self.path("libgstfaad.so")
                #self.path("libgstffmpeg.so")
                #self.path("libgstffmpegcolorspace.so")
                #self.path("libgstgnomevfs.so")
                #self.path("libgsticydemux.so")
                #self.path("libgstid3demux.so")
                #self.path("libgstmpegdemux.so")
                #self.path("libgstmultifile.so")
                #self.path("libgstmultipart.so")
                #self.path("libgstogg.so")
                #self.path("libgstossaudio.so")
                #self.path("libgstplaybin.so")
                #self.path("libgstpulse.so")
                #self.path("libgstqtdemux.so")
                #self.path("libgstqueue2.so")
                #self.path("libgsttcp.so")
                #self.path("libgsttheora.so")
                #self.path("libgsttypefindfunctions.so")
                #self.path("libgstudp.so")
                #self.path("libgstvideoscale.so")
                #self.path("libgstvolume.so")
                #self.path("libgstvorbis.so")
                #self.path("libgstwavparse.so")
                
                #self.end_prefix("gstreamer-plugins")
            self.end_prefix("lib64")
        

            # Vivox runtimes and libs
            if self.prefix(src="vivox-runtime/i686-linux", dst="bin"):
                self.path("SLVoice")
                self.end_prefix("bin")

            if self.prefix(src="vivox-runtime/i686-linux", dst="lib32"):
                #self.path("libalut.so")
                self.path("libortp.so")
                self.path("libvivoxsdk.so")
                self.end_prefix("lib32")

        # 32bit libs needed for voice
        if self.prefix("../../libraries/x86_64-linux/lib_release_client/32bit-compat", dst="lib32"):
            self.path("libalut.so")
            self.path("libidn.so.11")
            self.path("libopenal.so.1")
            # self.path("libortp.so")
            self.path("libuuid.so.1")
            self.end_prefix("lib32")

if __name__ == "__main__":
    main()
