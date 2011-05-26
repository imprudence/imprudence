#!/usr/bin/env python
#
# @file viewer_info.py
# @author Jacek Antonelli
# @brief Scans and prints the viewer name and/or version.
# 
# Copyright (c) 2010, Jacek Antonelli
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# 
# 
# Usage:
# 
#   viewer_info.py --name      # viewer name (e.g. "Kokua")
#   viewer_info.py --version   # viewer version (e.g. "1.0.0-RC1"
#   viewer_info.py --combined  # name + version (e.g. "Kokua-1.0.0-RC1")
# 
# You can pass multiple flags to print each thing on a separate line.
# E.g. `viewer_info.py --name --version' will print 2 lines, e.g.:
# 
#   Kokua
#   1.0.0-RC1
# 

import errno, os, re, string, sys


class ViewerInfo:

    def __init__(self, filepath=None):
        f = open(filepath or self.default_file())
        data = f.read()
        f.close()

        self.name  = re.search('NAME\s*=\s*"([^"]*)"',  data).group(1)
        self.major = re.search('MAJOR\s*=\s*(\d+)',     data).group(1)
        self.minor = re.search('MINOR\s*=\s*(\d+)',     data).group(1)
        self.patch = re.search('PATCH\s*=\s*(\d+)',     data).group(1)
        self.extra = re.search('EXTRA\s*=\s*"([^"]*)"', data).group(1)
        self.bundle_id = re.search('BUNDLE_ID\s*=\s*"([^"]*)"', data).group(1)

        self.version = "%s.%s.%s"%(self.major, self.minor, self.patch)
        if len(self.extra) > 0:
            # Replace spaces and some puncuation with '-' in extra
            extra = re.sub('[- \t:;,!+/\\"\'`]+', '-', self.extra)
            # Strip any leading or trailing "-"s
            extra = string.strip(extra, '-')
            self.version += "-" + extra

        self.combined = self.name + "-" + self.version

    @classmethod
    def default_file(klass):
        scripts_dir = sys.path[0] # directory containing this script
        viewerinfo = os.path.join('indra', 'newview', 'viewerinfo.cpp')
        filepath = os.path.join(scripts_dir, '..', viewerinfo)
        return os.path.abspath(filepath)


if __name__ == '__main__':

    try:
        info = ViewerInfo()
    except IOError, err:
        if err.errno == errno.ENOENT:
            print >> sys.stderr, 'File not found:', ViewerInfo.default_file()
            sys.exit(1)
        else:
            raise

    args = sys.argv[1:]

    if not args:
        print "Usage: %s [--name] [--version] [--combined]"%(sys.argv[0])
    for arg in args:
        if '--name' == arg:
            print info.name
        elif '--version' == arg:
            print info.version
        elif '--combined' == arg:
            print info.combined
        elif '--bundle-id' == arg:
            print info.bundle_id
