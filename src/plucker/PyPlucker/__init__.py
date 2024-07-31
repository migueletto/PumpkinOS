# parser/python/PyPlucker/__init__.py.  Generated from __init__.py.in by configure.
#
# $Id: __init__.py.in,v 1.8 2003/06/18 04:28:01 robertoconnor Exp $
#

__version__ = "1.8"

lib_dir = "/usr/local/etc"

DEFAULT_IMAGE_PARSER_SETTING = "netpbm2"

import sys
# We also use plucker.ini on OSX since the . files are always invisible to the
# normal user. The python built into OSX 10.2 returns 'mac'. Apparently others
# may return 'darwin' so we check for both.
if sys.platform == 'win32' or sys.platform == 'os2' or sys.platform == 'mac' or sys.platform == 'darwin':
    _SYS_CONFIGFILE_NAME = "plucker.ini"
    _USER_CONFIGFILE_NAME = "plucker.ini"
else:
    _SYS_CONFIGFILE_NAME = "pluckerrc"
    _USER_CONFIGFILE_NAME = ".pluckerrc"


# try to figure out default character set, if any, by using the POSIX locale
DEFAULT_LOCALE_CHARSET_ENCODING = None
try:
    import locale
    locale.setlocale(locale.LC_ALL,"")
    DEFAULT_LOCALE_CHARSET_ENCODING = locale.getlocale()[1]
    ###################################################################
    # locale.getlocale()[1] return an Number (for example 1252)       #
    # on Windows and charset_name_to_mibenum think thats the mibenum  #
    # so we need create an full charset name                          #
    ###################################################################
    if sys.platform == 'win32':
        import re
        if re.match('^[0-9]+$', DEFAULT_LOCALE_CHARSET_ENCODING):
            DEFAULT_LOCALE_CHARSET_ENCODING = "windows-%s" % DEFAULT_LOCALE_CHARSET_ENCODING
except:
    pass


# try to find the gettext package...
try:
    from PyPlucker.helper import gettext
except ImportError:
    try:
        from Pyrite import gettext
    except ImportError:
        try:
	    import gettext
        except ImportError:
            # We give up...
            gettext = None
if gettext is None:
    def _(text):
        return text
else:
    gettext.bindtextdomain('pyplucker', "/usr/local/share/locale")
    gettext.textdomain('pyplucker')
    _ = gettext.gettext

