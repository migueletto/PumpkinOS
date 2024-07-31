#!/usr/bin/env python
"""
Links.py   $Id: PluckerLinks.py,v 1.5 2002/05/18 10:28:24 nordstrom Exp $

A class to build a PluckerLinkIndexDocument and PluckerLinkDocuments
from the Database mapping.

Copyright 2000 by Christopher Robin Hawks <chrish@syscon-intl.com>

Distributable under the GNU General Public License Version 2 or newer.

"""

import string, sys
import struct
import types
import PyPlucker
from PyPlucker import PluckerDocs


class Links:
    def __init__ (self, mapping = None):
        if mapping is not None:
            self._mapping = mapping;
        else:
            self._mapping = ""
        self._lists = []
        self._index = ""


    def is_text_document (self):
        return 0


    def return_index (self):
        if self._index is not None:
            return self._index
        else:
            return None


    def return_list (self, idx):
        if self._lists[idx] is not None:
            return self._lists[idx]
        else:
            return None


    def build_links (self):
        """Create the index and an array of URL lists"""
        count = 1
        last = 0
        last_id = self._mapping.get_max_id ()
        this_id = last_id + 1
        rev = {}
        idx = []

        mapping = self._mapping.get_mapping()
        for i in mapping.keys ():
            if i[:7] != "mailto:":
                rev[mapping[i]] = i

        while (count <= last_id):
            data = []
            i = 0
            max = min (last_id - count + 1, 200)
            while (i < max):
                if rev.has_key(i + count):
                    data.append (rev[i + count])
                data.append ("\000")
                i = i + 1
            data = string.join (data, "")

            last = count + i - 1
            self._lists.append (data)
            sys.stderr.write("index entry:  %d, %d\n" % (last, this_id))
            idx.append (struct.pack("<HH", last, this_id)) 
            count = i + count
            this_id = this_id + 1

        self._index = string.join (idx, "")

        sys.stderr.write("PluckerLinks.Links._lists is\n")
        for element in self._lists:
            sys.stderr.write("   %s\n" % element)

        return len (self._lists)

