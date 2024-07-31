#!/usr/bin/env python

"""
ConfigFiles.py   $Id: ConfigFiles.py,v 1.9 2002/05/18 10:28:24 nordstrom Exp $

Defines easy access to the various .ini config files for PyPlucker.

Copyright 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""

import os, sys, types, string, ConfigParser
import PyPlucker



SYS_CONFIG_FILE = os.path.join (PyPlucker.lib_dir, PyPlucker._SYS_CONFIGFILE_NAME)
if os.environ.has_key ('HOME'):
    USER_CONFIG_FILE = os.path.join (os.environ["HOME"], PyPlucker._USER_CONFIGFILE_NAME)
else:
    # should not happen, but what's it like on those lesser operating systems?
    USER_CONFIG_FILE = SYS_CONFIG_FILE


class _ConfigGetter:
    """A helper class to maintain one section in one config file"""

    def __init__ (self, config, section):
        self._config = config
        self._section = section


    def get_string (self, option):
        try:
            return self._config.get (self._section, option, raw=1)
        except:
            return None


    def get_int (self, option):
        return int (self.get_string (option))
    




class Configuration:
    """A Class to maintain information about all possivble user
    settable options from various .ini config files."""

    def __init__ (self, pluckerhome, pluckerdir, extra_sections=[], error_logger=None):
        """Load .ini files from all possible places and present one
        unified view"""

        self._configs = []
        if sys.platform == 'win32':
            self._sections = ['WINDOWS']
        elif sys.platform == 'os2':
            self._sections = ['POSIX', 'OS2']
        else:
            self._sections = ['POSIX']
        self._sections = self._sections + extra_sections
        self._dict = {}

        self.maybe_load_config (SYS_CONFIG_FILE, error_logger)
        self.maybe_load_config (USER_CONFIG_FILE, error_logger)
        if pluckerhome:
            self.maybe_load_config (os.path.join (pluckerhome, PyPlucker._USER_CONFIGFILE_NAME),
                                    error_logger)
        if pluckerdir:
            self.maybe_load_config (os.path.join (pluckerdir, PyPlucker._USER_CONFIGFILE_NAME),
                                    error_logger)

        if pluckerhome:
            self.set ('PLUCKERHOME', pluckerhome)
        if pluckerdir:
            self.set ('pluckerdir', pluckerdir)

            

    def maybe_load_config (self, filename, error_logger):
        """Load all sections from config file 'filename'"""
        if os.path.exists (filename):
            # reverse the list, so that appends become prependes
            self._configs.reverse ()
            
            try:
                c = ConfigParser.ConfigParser ()
                c.read (filename)

                for section in self._sections:
                    if c.has_section (section):
                        self._configs.append (_ConfigGetter (c, section))
            except ConfigParser.Error, text:
                if error_logger is not None:
                    error_logger ("Error parsing config file '%s': %s" % (filename, text))
                pass

            # reverse again...
            self._configs.reverse ()

    
    def _get_string (self, option):
        if self._dict.has_key (option):
            return self._dict[option]
        aList = map (lambda x, o=option: x.get_string (o), self._configs)
        result = reduce (lambda a, b: a or b, aList, None)
        return result

    ## these should probably be re-written so that they always either raise
    ## an exception, or return a value of the specified type.  In other words,
    ## re-written so that they never return None. -- wcj

    def get_string (self, option, default=None):
        if not (default is None):
            assert (type(default) == types.StringType)
        result = self._get_string(option)
        if result is None:
            return default
        else:
            return result

    
    def get_int (self, option, default=None):
        if not (default is None):
            assert (type(default) == types.IntType or type(default) == types.LongType)
        if self._dict.has_key (option):
            return int (self._dict[option])
        result = self._get_string (option)
        if result is None:
            return int (default)
        else:
            return int (result)
        

    def get_bool (self, option, default=None):
        res = self._get_string (option)
        if res is None:
            if default is None:
                return None
            else:
                res = default
        if type (res) == types.StringType:
            res = string.lower (res)
        if res == 1 or res == "1" or res == "y" or res == "yes" or res == "true" or res == "on":
            return 1
        if res == 0 or res == "0" or res == "n" or res == "no" or res == "false" or res == "off":
            return 0
        else:
            raise RuntimeError("Illegal non-boolean value '%s' found for option '%s'" % (repr (res), option))
        

    def set (self, option, value):
        self._dict[option] = value
        
