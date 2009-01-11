"""\
@file lllog.py
@brief Logging for event processing

$LicenseInfo:firstyear=2008&license=mit$

Copyright (c) 2008-2009, Linden Research, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
$/LicenseInfo$
"""

from indra.base.llsd import format_notation

try:
    import syslog
except ImportError:
    # Windows
    import sys

    class syslog(object):
        _logfp = sys.stderr

        def syslog(msg):
            _logfp.write(msg)
            if not msg.endswith('\n'):
                _logfp.write('\n')

        syslog = staticmethod(syslog)

class Logger(object):
    def __init__(self, name='indra'):
        self._sequence = 0
        try:
            syslog.openlog(name, syslog.LOG_CONS | syslog.LOG_PID,
                           syslog.LOG_LOCAL0)
        except AttributeError:
            # No syslog module on Windows
            pass

    def next(self):
        self._sequence += 1
        return self._sequence

    def log(self, msg, llsd):
        payload = 'INFO: log: LLLOGMESSAGE (%d) %s %s' % (self.next(), msg,
                                               format_notation(llsd))
        syslog.syslog(payload)

_logger = None

def log(msg, llsd):
    global _logger
    if _logger is None:
        _logger = Logger()
    _logger.log(msg, llsd)
