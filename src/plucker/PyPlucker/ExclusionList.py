#!/usr/bin/env python

"""
ExclusionList.py   $Id: ExclusionList.py,v 1.7 2002/01/17 02:16:52 janssen Exp $

A class to handle information about what URLs to exclude from the
plucking process.

Copyright 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.

"""

import re, string, sys
from PyPlucker.UtilFns import error, message


class ExclusionList:

    """A class to maintain information about what URLs to exclude from
    the plucking process.

    This implements an ordered, prioritizes list of regexps that will
    state that an URLs should either be included or excluded.

    Items are loaded into the list from a file.
    Each line in a file is processed separately:
      - all leading and trailing white space is removed
      - empty lines are ignored
      - lines starting with a '#' are ignored (comment)
      - everthing else should be of the format:
        prio:action:regexp
        where:
         - prio is an integer
         - action is either a plus or a minus ('+' or '-')
         - regexp is a valid regular expression
    """

    def __init__ (self, include_by_default=1):
        self._items = {}
        self._default_action = include_by_default


    def load_file (self, filename):
        """Load new items from the file 'filename'."""
        f = open (filename, "r")
        for orig_line in f.readlines():
            if orig_line[-1] == '\n':
                orig_line = orig_line[:-1]
            line = string.strip (orig_line)
            if not line:
                # empty line
                continue
            if line[0] == '#':
                # a comment
                continue

            m = re.match (r"([-+]?\d+):([-+]):(.*)", line)
            if not m:
                error ("ExclusionList: Cannot parse line: %s\n" % orig_line)
            else:
                prio = int (m.group (1))
                action = m.group (2) == '+'
                regexp = m.group (3)
                new_item = (action, regexp)
                if self._items.has_key (prio):
                    self._items[prio].append (new_item)
                else:
                    self._items[prio] = [new_item]
                    
   

    def check (self, url):
        """Check if 'url' is to be included (result=1) or excluded
        (result=0)."""

        prios = self._items.keys ()
        prios.sort ()
        prios.reverse ()

        for prio in prios:
            for action, regexp in self._items[prio]:
                if re.match (regexp, url):
                    return action
        return self._default_action
