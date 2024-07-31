#!/usr/bin/env python

"""
Retriever.py   $Id: Retriever.py,v 1.28 2003/12/17 03:46:01 jimj Exp $

Retrieve data identified by some URL from the appropiate location.


Copyright 1999, 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""

import os, sys
import string
import re
import urllib
import types

## The following section tries to get the PyPlucker directory onto the
## system path if called as a script and if it is not yet there:
try: import PyPlucker
except ImportError:
    file = sys.argv[0]
    while os.path.islink (file): file = os.readlink (file)
    sys.path = [os.path.split (os.path.dirname (file))[0]] + sys.path
    try: import PyPlucker
    except ImportError:
        print "Cannot find where module PyPlucker is located!"
        sys.exit (1)
    
    # and forget the temp names...
    del file
del PyPlucker
##
## Now PyPlucker things should generally be importable
##

try:
    import gzip
    import StringIO
    _have_gzip = 1
except:
    _have_gzip = 0
    
from PyPlucker import Url, __version__
from UtilFns import error, message

def GuessType (name):
    """Given a name, guess the mime type"""
    name = string.lower (name)
    def has_extension (ext, name=name):
        return name[-len(ext):] == ext

    known_map = { '.gif': 'image/gif',
                  '.png': 'image/png',
                  '.jpg': 'image/jpeg',
                  '.jpe': 'image/jpeg',
                  '.jpeg': 'image/jpeg',
                  '.html': 'text/html',
                  '.htm': 'text/html',
                  '.txt': 'text/plain',
                  '.asc': 'text/plain',
                  }
    for ext in known_map.keys ():
        if has_extension (ext):
            return known_map[ext]
    return 'unknown/unknown'


class PluckerFancyOpener (urllib.FancyURLopener):
    """A subclass of urllib.FancyURLopener, so we can remember an
    error code and the error text."""
    
    def __init__(self, alias_list=None, config=None, *args):
        apply(urllib.FancyURLopener.__init__, (self,) + args)
        self._alias_list = alias_list
        self.remove_header ('User-agent')
        user_agent = (config and config.get_string('user_agent', None)) or 'Plucker/Py-%s' % __version__
        self.addheader ('User-agent', user_agent)
        referrer = config and config.get_string('referrer', None)
        if referrer:
            self.addheader('Referer', referrer)
        self.addheader ('Accept', 'image/jpeg, image/gif, image/png, text/html, text/plain, text/xhtml;q=0.8, text/xml;q=0.6, text/*;q=0.4')

        if os.environ.has_key ('HTTP_PROXY') and (os.environ.has_key ('HTTP_PROXY_USER') and os.environ.has_key ('HTTP_PROXY_PASS')):
            import base64
            self.addheader ('Proxy-Authorization', 'Basic %s' % string.strip(base64.encodestring("%s:%s" % (os.environ['HTTP_PROXY_USER'], os.environ['HTTP_PROXY_PASS']))))
        #for header in self.addheaders: message(0, "%s", header)


    def remove_header (self, header):
        """Remove the header information 'header' if on the header list.
           Return if found on list.
        """
        for i in range (len (self.addheaders)):
            if self.addheaders[i][0] == header:
                del self.addheaders[i]
                return 1
        return 0

    # Do not do this - addinfourl raised an exception on
    # None, because the default had already closed the file.
    #
    # Default error handling -- don't raise an exception, but remember the code
    #
    #def http_error_default(self, url, fp, errcode, errmsg, headers):
    #    res = urllib.addinfourl(fp, headers, "http:" + url)
    #    res.retcode = errcode
    #    res.retmessage = errmsg
    #    return res

    # Do not do this - urllib now handles redirection
    #def http_error_302(self, url, fp, errcode, errmsg, headers,
    #           data=None): 
    #    # XXX The server can force infinite recursion here!
    #    if self._alias_list:
    #        if headers.has_key('location'):
    #            newurl = headers['location']
    #        elif headers.has_key('uri'):
    #            newurl = headers['uri']
    #        else:
    #            return
    #        old_url = Url.URL ('http:'+url)
    #        new_url = Url.URL (newurl, old_url)
    #        self._alias_list.add (old_url, new_url)
    #    if headers.has_key('location'):
    #        newurl = headers['location']
    #    elif headers.has_key('uri'):
    #        newurl = headers['uri']
    #    return urllib.FancyURLopener.http_error_302 (self, url, fp, errcode, errmsg, headers, data)
    #
    #http_error_301 = http_error_302
    #http_error_303 = http_error_302


def parse_http_header_value(headerval):
    mval = None
    parameters = []
    parts = string.split (headerval, ";")
    if parts:
        mval = string.lower(parts[0])
    for part0 in parts[1:]:
        part = string.strip(string.lower(part0))
        m = re.match ('([-a-z0-9]+)=(.*)', part)
        if m:
            parameters.append(m.groups())
    return mval, parameters
            

class SimpleRetriever:
    """A very simple retriver.  Not much of error checking, no
    persistent caching.  Just a wrapper around urllib."""

    def __init__ (self, pluckerdir, pluckerhome, configuration=None):
        self._plucker_dir = os.path.expanduser( os.path.expandvars (pluckerdir))
        self._plucker_home = os.path.expanduser( os.path.expandvars (pluckerhome))
        self._cache = {}
        self._configuration = configuration
        # without this, windows and no proxy was very slow
        self._urlopener = PluckerFancyOpener (config=self._configuration)

    def _retrieve_plucker (self, url, alias_list):
        path = url.get_path ()
        if path[0] != '/':
            raise RuntimeError("plucker: URL must give absolute path! (%s)" % path)
        filename1 = os.path.join (self._plucker_dir, path[1:])
        filename2 = os.path.join (self._plucker_home, path[1:])
        if os.path.exists (filename1):
            filename = filename1
        elif os.path.exists (filename2):
            filename = filename2
        else:
            return ({'URL': url,
                     'error code': 404,
                     'error text': "File not found"},
                    None)
        try:
            file = open (filename, "rb")
            contents = file.read ()
            file.close ()
        except IOError, text:
            return ({'URL': url,
                     'error code': 404,
                     'error text': text},
                    None)
        return ({'URL': url,
                 'error code': 0,
                 'error text': "OK",
                 'content-type': GuessType (filename),
                 'content-length': len (contents)},
                contents)

    def _retrieve (self, url, alias_list, post_data):
        """Really retrieve the url."""
        if url.get_protocol () == 'plucker':
            return self._retrieve_plucker (url, alias_list)

        elif url.get_protocol () == 'mailto':
            # Nothing to fetch really...
            return ({'URL': url,
                     'error code': 0,
                     'error text': "OK",
                     'content-type': "mailto/text",
                     'content-length': 0},
                     "")

        else:
            # not a plucker:... URL
            try:
                real_url = str (url)
                webdoc = self._urlopener.open (real_url, post_data)
                if hasattr (webdoc, 'retcode'):
                    headers_dict = {'URL': real_url,
                                    'error code': webdoc.retcode,
                                    'error text': webdoc.retmessage}
                    doc_info = webdoc.info ()
                    if doc_info is not None:
                        # This should always be a dict, but some people found None... :-(
                        headers_dict.update (doc_info.dict)
                    return (headers_dict, None)
                if hasattr (webdoc, 'url'):
                    #######################################################################
                    # Redhat 7.x default Python installation will return                  #
                    # webdoc.url without a protocol at the beginning                      #
                    # (e.g. ://www.xyz.com instead of http://www.xyz.com).                #
                    # This is due to a bug in RH's /usr/lib/python1.5/urllib.py.          #
                    # -joefefifo@yahoo.com                                                #
                    #######################################################################
                      ################################################
                      # On Windows we wan't use                      #
                      # URL(url).get_protocol to get the protokoll   #
                      # urllib.splittype(url) and all other url      #
                      # manipuling funktions are too buggy           #
                      ################################################



                    if sys.platform == 'win32':
                        from PyPlucker.Url import URL
                        webdoc_protocol = URL(webdoc.url).get_protocol
                    else:
                        (webdoc_protocol, webdoc_rest_of_url) = urllib.splittype(webdoc.url)

                    # check to see we have a valid URL; if not, use one we started with
                    if webdoc_protocol:
                        real_url = webdoc.url

                headers_dict = {'URL': real_url}
                doc_info = webdoc.info ()
                message(3, "doc_info is %s", doc_info);
                if doc_info is not None:
                    # This should always be a dict, but some people found None... :-(
                    headers_dict.update (doc_info.dict)
                if not headers_dict.has_key ('content-type'):
                    message (1, "Guessing type for %s" % url.get_path ())
                    headers_dict['content-type'] = GuessType (url.get_path ())
                else:
                    ctype, parameters = parse_http_header_value(headers_dict['content-type'])
                    headers_dict['content-type'] = ctype
                    for parm in parameters:
                        headers_dict[parm[0]] = parm[1]

                message(3, "headers_dict is %s", headers_dict);

                # Now get the contents
                contents = webdoc.read ()

                # Check if encoded contents...
                if headers_dict.has_key ('content-encoding'):
                    encoding = headers_dict['content-encoding']
                    if encoding == 'gzip' and _have_gzip:
                        s = StringIO.StringIO (contents)
                        g = gzip.GzipFile (fileobj=s)
                        c = g.read ()
                        g.close ()
                        contents = c
                    else:
                        return ({'URL': real_url,
                                 'error code': 404,
                                 'error text': "Unhandled content-encoding '%s'" % encoding},
                                None)

            except IOError, text:
                return ({'URL': real_url,
                         'error code': 404,
                         'error text': text},
                        None)
	    except OSError, text:
                return ({'URL': real_url,
                         'error code': 404,
                         'error text': text},
                        None)
            headers_dict['error code'] = 0
            headers_dict['error text'] = "OK"
            return (headers_dict,
                    contents)


    def retrieve (self, url, alias_list, post_data):
        """Fetch some data.
        Return a tuple (headers_dict, data)"""

        if not isinstance (url, Url.URL):
            url = str (url) # convert to string, if not yet so
            url = Url.URL (Url.CleanURL (url))

        data_key = (str (url), post_data)
        if self._cache.has_key (data_key):
            # has been retrieved before, we just return the cached data
            return self._cache[data_key]
        else:
            result = self._retrieve (url, alias_list, post_data)
            self._cache[data_key] = result
            newurl = getattr(result, 'URL', url).as_string(with_fragment=None)
            alias_list.add(url,newurl)
            return result

        


if __name__ == '__main__':
    # called as a script
    import sys
    retriever = SimpleRetriever ("~/.plucker", "~/.plucker")
    for name in sys.argv[1:]:
        print "\n\nFetching %s" % name
        (header, data) = retriever.retrieve (name, None, None)
        items = header.keys ()
        items.sort ()
        print "Headers:"
        for item in items:
            print "  %s:\t%s" % (item, header[item])
        print "Data:"
        text = repr (data)[1:-1]
        if len (text) > 80:
            text = text[:60] + " ... " + text[-10:]
        print "  " + text
            
