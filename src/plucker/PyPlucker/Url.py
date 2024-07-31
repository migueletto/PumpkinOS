#!/usr/bin/env python

"""
Url.py   $Id: Url.py,v 1.20 2003/12/17 02:28:47 jimj Exp $

Utility class to encapsulate information about an URL and the useful
operations thereon.


Copyright 1999, 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.

"""

import urlparse, urllib, string, sys, os

urlparse.uses_relative.append ('plucker')
#urlparse.uses_netloc.append ('plucker')
#urlparse.uses_params.append ('plucker')
#urlparse.uses_query.append ('plucker')
urlparse.uses_fragment.append ('plucker')



######################################################################
# Replacement for the urlparse lib, because this is buggy on Windows #
######################################################################
def windows_file_url_parse (url):
    prot='file'
    fragment=''
    i = string.rfind(url, '#')
    if i >= 0:
        fragment = url[i+1:]
        url = url[:i]
    path=url
    if string.lower(path[0:7]) == 'file://':
        path=path[7:]
    if string.lower(path[0:5]) == 'file:':
        path=path[5:]
    if ((string.upper(path[0:1]) >= 'A') and (string.upper(path[0:1]) <= 'Z')) and (path[1:2] == ':'):
            path = string.upper(path[0:1]) + path[1:]
    host=''
    params=''
    query=''
    return prot, host, path, params, query, fragment



######################################################################
# Replacement for the urlparse lib, because this is buggy on Windows #
######################################################################
def windows_file_urljoin(base, url):
    def add_fragment(path, frag):
        if frag != '':
            res = path + '#' + frag
        else:
            res = path
        return res

    i = string.find(url, ':')
    # a new http:// file:// not based to source is _not_ used
    if (i < 3) or (i > 10):
        (prot, host, path, params, query, fragment) = windows_file_url_parse (url)
        if path != '':
            ######################################
            # FIX ME!!!!                         #
            # path like .\test\..\images\        #
            # are not work yet!                  #
            ######################################
            # .\file.ext == file.ext
            if (path[0:2] == '.\\') or (path[0:2] == './'):
                path = path[2:]
                url = os.path.join (os.path.dirname(str (base)), add_fragment(path, fragment))
                return url
            # one dir up
            if (path[0:3] == '..\\') or (path[0:3] == '../'):
                path = path[3:]
                url = os.path.join (os.path.dirname(os.path.dirname(str (base))), add_fragment(path, fragment))
                return url
            # two dir up
            if (path[0:4] == '...\\') or (path[0:4] == '.../'):
                path = path[4:]
                url = os.path.join (os.path.dirname(os.path.dirname(os.path.dirname(str (base)))), add_fragment(path, fragment))
                return url
            # Root dir
            if (path[0:1] == '\\') or (path[0:1] == '/'):
                path = path[1:]
                str_base = str (base)
                url = os.path.join ('file:' + str_base[5] + ':' , add_fragment(path, fragment))
                return url
            # normale case
            else:
                url = os.path.join (os.path.dirname(str (base)), add_fragment(path, fragment))
                return url
        else:
            url = base + '#' + fragment
            return url
    else:
        return url

    return url



######################################################################
# Replacement for the urlparse lib, because this is buggy on Windows #
# And its behavior changed in Python 2.2.2 CRH
######################################################################
def plucker_file_urlunparse(protocol, host, path, params, query, fragment):
    text = ''
    if protocol != '':
        text = text + protocol + ':' + path
    if fragment != '':
        text = text + '#' + fragment
    return text



class URL:
    """Encapsulate some useful things from urllib and urlparse"""

    def __init__ (self, url, base = None):
        if isinstance (url, URL) and base is None:
            # Simple copy constructor: make it more efficient
            self._protocol = url._protocol
            self._host = url._host
            self._path = url._path
            self._params = url._params
            self._query = url._query
            self._fragment = url._fragment
        else:            
            url = str (url)
            # Sometimes an URL is interrupted by a line break.
            # Fetching works better if the linebreak is removed.
            url = string.replace(url, "\r", "")
            url = string.replace(url, "\n", "")
            if base is not None:
                if sys.platform == 'win32' and string.lower(str (base)[0:5]) == 'file:':
                    url = windows_file_urljoin (str (base), url)
                else:
                    url = urlparse.urljoin (str (base), url)
            # according to RFC 2396, this 'unquote' is inappropriate
            # according to the HTML 4.01 spec, this 'unquote' is unnecessary
            # url = urllib.unquote (url)
            if sys.platform == 'win32' and string.lower(url[0:5]) == 'file:':
                (prot, host, path, params, query, fragment) = windows_file_url_parse (url)
            else:
                (prot, host, path, params, query, fragment) = urlparse.urlparse (url)
            host = string.lower (host)
            self._protocol = prot
            self._host = host
            self._path = path
            self._params = params
            self._query = query
            self._fragment = fragment

    def as_string (self, with_fragment):
        if with_fragment:
            fragment = self._fragment
        else:
            fragment = ""
        if self._protocol == 'plucker' or self._protocol == 'file':
            text = plucker_file_urlunparse (self._protocol,
                                            self._host,
                                            self._path,
                                            self._params,
                                            self._query,
                                            fragment)
        else:
            text = urlparse.urlunparse ((self._protocol,
                                         self._host,
                                         self._path,
                                         self._params,
                                         self._query,
                                         fragment))
        return text

     
    def __str__ (self):
        return self.as_string (with_fragment=1)
    
    def __repr__ (self):
        return "URL (%s)" % repr (self.as_string (with_fragment=1))

    def get_protocol (self):
        return self._protocol
            
    def get_host (self):
        return self._host
            
    def get_path (self):
        return self._path

    def get_fragment (self):
        return self._fragment

    def get_full_path (self, with_fragment):
        if with_fragment:
            fragment = self._fragment
        else:
            fragment = ""
        if sys.platform == 'win32' and self._protocol == 'file':
            text = plucker_file_urlunparse ("",
                                            "",
                                            self._path,
                                            self._params,
                                            self._query,
                                            fragment)
        else:
            text = urlparse.urlunparse (("",
                                         "",
                                         self._path,
                                         self._params,
                                         self._query,
                                         fragment))
        return text

    def remove_fragment (self):
        self._fragment = ""



def CleanURL (url, base=None):
    """Remove leading and trailing white space and generally clean up
    this URL"""
    if isinstance (url, URL):
        # This branch is currently never taken, we get always called
        # with a string as 'url'
        if base is not None:
            # FIXME!!  Does this make sense at all?  URLs should always be
            # absoulte, so giving a base is moot...
            result = Url (url, base).as_string (with_fragment=1)
        else:
            result = url.as_string (with_fragment=1)
    else:
        url = string.strip (str (url))
        url = URL (url, base)

        result = url.as_string (with_fragment=1)
    return result




def CompareURL (rawurl1, rawurl2):
     """Compare URLs in a smart ordering method."""

     def string_compare (x, y):
     # Plain alphabetic sort
         if x<y:
             return -1
         elif y<x:
             return 1
         else:
             return 0

     def get_integer_string (s):
     # Get an integer from a string
         i=0
         while i<len(s) and s[i]=='0':
             i=i+1
         if i==len(s) or not s[i] in string.digits:
             i=i-1
         start=i
         while i<len(s) and s[i] in string.digits:
             i=i+1
         return s[start:i]

     # This is basically an alphabetic sort, with one exception:
     # If the two strings start differing in some numerical
     # expression, then we compare the numerical expressions
     # numerically.  Thus, "defg" < "efgh", and "1"<"2", but
     # "defg10a" > "defg9z".  This tends to work nicely for ordering
     # various ebook downloads which often have urls like:
     # http://myserver.edu/BookTitle/Chapter2.html.
     # Moreover, index.html type files go first, and files
     # in a higher level of the hierarchy come before ones
     # that are lower down in the same hierarchy.
     # If the above rules are not enough to induce an order,
     # alphabetical ordering is used.
     #
     # The following order is observed:
     #    http://xyz.com/aaaa/bbbb/cccc/hello.htm
     #    http://xyz.com/gggg/index.html
     #    http://xyz.com/gggg/beta.html
     #    http://xyz.com/gggg/gamma99a.html
     #    http://xyz.com/gggg/gamma99b.html
     #    http://xyz.com/gggg/gamma100a.html
     #    http://xyz.com/gggg/aaaa/index.html
     #    http://xyz.com/gggg/aaaa/aaaalowerdown.html
     #    http://xyz.com/gggg/z/sec1A.html
     #    http://xyz.com/gggg/z/sec11.html
     #
     # Moreover, URLs using a \ are ordered as if they used a /,
     # unless the \ / difference is the only difference in which
     # case an alpha sort is used.

     # The input urls can have nulls followed by additional info.
     splitup1 = string.split(rawurl1, '\0')
     splitup2 = string.split(rawurl2, '\0')
     splitup1[0] = string.replace(splitup1[0], '\\', '/')
     splitup2[0] = string.replace(splitup2[0], '\\', '/')
     if splitup1[0] == splitup2[0]:
         # The parts before the nulls match, up to \ / interchange.
         if splitup1 == splitup2:
             # The whole urls match up to \ / interchange.
             return string_compare(rawurl1,rawurl2)
         else:
             # Compare the strings using /'s instead of \'s.
             return string_compare(string.join(splitup1),string.join(splitup2))
     name1 = splitup1[0]
     name2 = splitup2[0]
     i=0
     while i<len(name1) and i<len(name2) and name1[i] == name2[i]:
         i=i+1
     if i==len(name2):
         # Shorter of course goes first.  Note it was guaranteed
         # here that name1<>name2, since otherwise it would have
         # returned at the splitup1 == splitup2 condition.
         return 1
     if i==len(name1):
         return -1
     # i points to the first mismatch now
     # Check first whether perhaps one of these is not higher up in the
     # hierarchy.
     slash1 = string.find(name1[i:],'/')
     slash2 = string.find(name2[i:],'/')
     if -1 <> slash1 and -1 == slash2:
         return 1
     elif -1 == slash1 and -1 <> slash2:
         return -1
     # Now check whether the two do not differ in that one is an
     # index.html (or cognate) and the other is not, and then
     # put the index.html first, unless the other has no last
     # element.
     splitup1 = string.split(name1,'/')
     splitup2 = string.split(name2,'/')
     if splitup1[:-1] == splitup2[:-1]:
         splitup1 = string.split(splitup1[-1],':')
         splitup2 = string.split(splitup2[-1],':')
         if splitup1[:-1] == splitup2[:-1]:
             index1 = (splitup1[-1] == "index.html" or
                 splitup1[-1] == "index.htm" or splitup1[-1] == "INDEX.HTM")
             index2 = (splitup2[-1] == "index.html" or
                 splitup2[-1] == "index.htm" or splitup2[-1] == "INDEX.HTM")
             if index1 and not index2:
                 return -1
             elif not index1 and index2:
                 return 1
     # The remaining rule to be applied is to check for whether
     # the difference is not in numerical strings.
     if not name1[i] in string.digits and not name2[i] in string.digits:
         return string_compare(name1,name2)
     # Back up to the beginning of the number so 11 vs. 1A doesn't
     # become 1 vs. A.  Since name1 and name2 match up to i, we
     # need only look at name1 when backing up.
     while i>0 and name1[i-1] in string.digits:
         i=i-1
     if not name1[i] in string.digits or not name2[i] in string.digits:
        # The mismatch isn't just a numerical one, so do an alphabetic
        # compare.
         return string_compare(name1,name2)
     num1=get_integer_string(name1[i:])
     num2=get_integer_string(name2[i:])
     if num1==num2:
         #if the numbers differ in leading zeros,
         #just do an alphabetical comparison
         return string_compare(name1,name2)
     if len(num1)<len(num2):
         return -1
     elif len(num2)<len(num1):
         return 1
     return string_compare(num1,num2)

