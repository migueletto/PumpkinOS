#!/usr/bin/env python

"""
AliasList.py   $Id: AliasList.py,v 1.6 2002/05/18 10:28:24 nordstrom Exp $

Maintain information about what URLs are an alias for another.


Copyright 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""

from PyPlucker.Url import URL

class AliasList:

    def __init__ (self, aDict=None):
        """Initialize an empty AliasList.  (Unless the passed
        dictionary already contains state from a previous AliasList
        instance.)"""
        if aDict is None:
            self._dict = {}
        else:
            self._dict = aDict


    def as_dict(self):
        return self._dict.copy()


    def add (self, old_url, new_url):
        old_url = URL (old_url).as_string (with_fragment=0)
        new_url = URL (new_url).as_string (with_fragment=0)

        if old_url != new_url:
            self._dict[old_url] = new_url


    def get (self, url):
        url = URL (url).as_string (with_fragment=0)
        while 1:
            if not self._dict.has_key (url):
                return url
            url = self._dict[url]

            
    def __repr__ (self):
        import string
        res = []
        for i in self._dict.keys ():
            res.append ("'%s' -> '%s'" % (i, self.get (i)))
        return "<AliasList: " + string.join (res, ", ") + ">"
