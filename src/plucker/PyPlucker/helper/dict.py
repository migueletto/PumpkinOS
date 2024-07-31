"""
PyPlucker.helper.dict: Helper class(es) for dictionaries.

Copyright 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.

"""

import string, types


def copy_dict (src, dest):
    for k in src.keys():
        dest[k] = src[k]



class DictCompartment:
    """A class that maintains a part of a dictionary (all things whose
    keys start with a certain text) and represents that again as a
    dictionary.

    Useful e.g. for shelves.

    Restriction:  All keys are converted to string.
    """

    def __init__ (self, dict, prefix_name, read_only=0):
        if hasattr (dict, '_get_real_prefix_and_dict'):
            p, d = dict._get_real_prefix_and_dict ()
            self._prefix = p + prefix_name
            self._dict = d
        else:
            self._dict = dict
            self._prefix = prefix_name
        self._official_prefix = prefix_name
        self._read_only = read_only


    def _get_real_prefix_and_dict (self):
        return (self._prefix, self._dict)

    
    def _prefix_cleaner (self, key):
        if type (key) != types.StringType:
            return None
        
        l = len (self._prefix)
        if key[:l] == self._prefix:
            return key[l:]
        else:
            return None

    def _prefix_adder (self, key):
        return self._prefix + str(key)

    
    def keys(self):
        return filter (None, map (self._prefix_cleaner, self._dict.keys ()))
    

    def values(self):
        return map (self.__getitem__, self.keys ())

    def update (self, other_dict):
        for key in other_dict.keys ():
            self[key] = other_dict[key]

    def items(self):
        res = []
        for k in self.keys ():
            res.append ((k, self[k]))
        return res
    

    def __len__(self):
        return len(self.keys ())
        

    def has_key(self, key):
        return self._dict.has_key (self._prefix_adder (key))


    def __getitem__(self, key):
        return self._dict[self._prefix_adder (key)]


    def __setitem__(self, key, value):
        if self._read_only:
            raise RuntimeError, "Trying to set value '%s' in read-only DictCompartment '%s'" % (key, self._prefix)
        self._dict[self._prefix_adder (key)] = value


    def __delitem__(self, key):
        if self._read_only:
            raise RuntimeError, "Trying to delete value '%s' in read-only DictCompartment '%s'" % (key, self._prefix)
        del self._dict[self._prefix_adder (key)]


    def clear (self):
        for k in self.keys ():
            try:
                del self[k]
            except KeyError:
                # these appear misteriously for me...
                pass
            

    def __str__ (self):
        res = []
        for x in self.keys ():
            res.append ("%s: %s" % (repr (x), self[x]))
        return  "{" + string.join (res, ", ") + "}"
    
    def __repr__ (self):
        res = []
        for x in self.keys ():
            res.append ("%s: %s" % (repr (x), self[x]))
        return  ("<DictCompartment '%s': " % self._official_prefix) + string.join (res, ", ") + ">"

    def sync (self):
        if hasattr (self._dict, 'sync'):
            self._dict.sync ()

            
    

class CachingDictCompartment (DictCompartment):
    """Just like a DictCompartment (see comment there) but caches the keys.

      (This assumes, that nobody else is messing in this name space of the dict.
       I.e. two CachingDictCompartments with the same dict and prefix
       do not interact at all.)
    """

    def __init__ (self, dict, prefix_name, read_only=0):
        self._keys = None
        DictCompartment.__init__ (self, dict, prefix_name, read_only)


    def _get_real_prefix_and_dict (self):
        return ("", self)


    def _get_keys (self):
        self._keys = {}
        for key in filter (None, map (self._prefix_cleaner, self._dict.keys ())):
            self._keys[key] = None

    
    def keys(self):
        if self._keys is None:
            self._get_keys ()
        return self._keys.keys ()
        

    def __setitem__(self, key, value):
        if self._read_only:
            raise RuntimeError, "Trying to set value '%s' in read-only DictCompartment '%s'" % (key, self._prefix)
        if self._keys is None:
            self._get_keys ()
        self._dict[self._prefix_adder (key)] = value
        self._keys[key] = None


    def __delitem__(self, key):
        if self._read_only:
            raise RuntimeError, "Trying to delete value '%s' in read-only DictCompartment '%s'" % (key, self._prefix)
        if self._keys is None:
            self._get_keys ()
        del self._dict[self._prefix_adder (key)]
        del self._keys[key]


    def clear (self):
        for k in self.keys ():
            try:
                del self[k]
            except KeyError:
                # these appear misteriously for me...
                pass
    
    def __repr__ (self):
        res = []
        for x in self.keys ():
            res.append ("%s: %s" % (repr (x), self[x]))
        return  ("<CachingDictCompartments '%s': " % self._official_prefix) + string.join (res, ", ") + ">"
    
