#!/usr/bin/env python
#  -*- mode: python; indent-tabs-mode: nil; -*-

"""
Spider.py   $Id: Spider.py,v 1.96 2004/04/10 23:06:39 chrish Exp $

Recursively gets documents and thus collects a document set.


Copyright 1999, 2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""

if __name__ == '__main__':
    ## The following section tries to get the PyPlucker directory onto the
    ## system path if called as a script and if it is not yet there:
    try: import PyPlucker
    except ImportError:
        import os, sys
        file = sys.argv[0]
        while os.path.islink (file): file = os.readlink (file)
        sys.path = [os.path.split (os.path.dirname (file))[0]] + sys.path
        try: import PyPlucker
        except ImportError:
            print "Cannot find where module PyPlucker is located!"
            sys.exit (1)

        # and forget the temp names...
        del file, os
    del PyPlucker
    ##
    ## Now PyPlucker things should generally be importable
    ##



import PyPlucker
from PyPlucker import Parser, ConfigFiles, __version__
from PyPlucker.Url import URL
from PyPlucker.AliasList import AliasList
from PyPlucker.UtilFns import message, error, show_exception
import os, string, sys, types, re


VALID_LINK_ATTRIBUTES = (

    # from the A entity

    "name",
    "href",
    "hreflang",
    "type",
    "rel",
    "rev",
    "charset",
    "id",
    "class",
    "lang",
    "dir",
    "title",
    "style",
    "shape",
    "coords",
    "onfocus",
    "onblur",
    "onclick",
    "ondblclick",
    "onmousedown",
    "onmouseup",
    "onmouseover",
    "onmousemove",
    "onmouseout",
    "onkeypress",
    "onkeydown",
    "onkeyup",
    "target",
    "tabindex",
    "accesskey",

    # from the IMG entity

    "src",
    "longdesc",
    "alt",
    "ismap",
    "usemap",
    "align",
    "border",
    "width",
    "height",
    "hspace",
    "vspace",

    # Plucker additions

    "bpp",
    "stayonhost",
    "stay_on_host",
    "stayondomain",
    "stay_on_domain",
    "staybelow",
    "url_pattern",
    "current_depth",
    "noimages",
    "maxdepth",
    "max_depth",
    "new_max_depth",
    "url",
    "max_width",
    "max_height",
    "alt_maxwidth",
    "alt_maxheight",
    "post",
    "post_data",

    # for the Plucker glyph support
    "glyph",
    "page_x",
    "page_y",
    "spacer",
    "vadjust",

    # other plucker tags
    "_plucker_from_image",
    "_plucker_from_url",
    "_plucker_id_tag",
    "_plucker_id_tag_inlineimage",
    "_plucker_id_tag_outoflineimage",
    "_plucker_alternate_image",
    )


LINK_ATTRIBUTES_TO_IGNORE = (
    "src",
    "ismap",
    "usemap",
    "align",
    "border",
    "current_depth",
    "name",
    "href",
    "hreflang",
    "type",
    "rel",
    "rev",
    "charset",
    "id",
    "class",
    "lang",
    "dir",
    "title",
    "style",
    "shape",
    "coords",
    "onfocus",
    "onblur",
    "onclick",
    "ondblclick",
    "onmousedown",
    "onmouseup",
    "onmouseover",
    "onmousemove",
    "onmouseout",
    "onkeypress",
    "onkeydown",
    "onkeyup",
    "target",
    "tabindex",
    "accesskey",
    )

    
class SpiderLink:
    """A class to maintain and encapsulate information about the
    various attributes related to links while spidering (MAXDEPTH,
    STAYONDOMAIN, STAYONHOST, STAYBELOW, NOIMAGES, etc.)

    In some cases we make a distinction, if the link has already been
    taken or not."""

    def __init__ (self, url, dict={}):
        self._url = url
        self._dict = {}
        self._current_depth = None
        self._max_depth = None
        self._new_max_depth = None
        self._stay_on_domain = None
        self._stay_on_host = None
        self._stay_below = None
        self._maxwidth = None
        self._maxheight = None
        self._bpp = 1
        self._url_pattern = None
        old = dict.copy()
        for key in old.keys():
            if not key in VALID_LINK_ATTRIBUTES:
                message(2, "Ignoring invalid link attribute '%s'", key)
                del old[key]
                continue
            if (key[:9] == '_plucker_' or
                key in LINK_ATTRIBUTES_TO_IGNORE):
                del old[key]
        self._update_from_dict (old, after_taken=1)
        self._from_image = 0
        self.set_post (None)

    def __str__ (self):
        res = "<SpiderLink"
        res = res + (" Depth: %s/%s" % (repr(self._current_depth), repr(self._max_depth)))
        res = res + " MAXWIDTH=%s" % self._maxwidth
        res = res + " MAXHEIGHT=%s" % self._maxheight
        res = res + " BPP=%d" % self._bpp
        if self._stay_on_domain:
            res = res + " STAYONDOMAIN"
        if self._stay_on_host:
            res = res + " STAYONHOST"
        if self._stay_below:
            res = res + (" STAYBELOW=\"%s\"" % self._stay_below)
        res = res + " URL_PATTERN='%s'" % self._url_pattern
        res = res + " URL='%s'" % self._url
        res = res + " " + repr (self._dict)
        res = res + ">"
        return res

    def     _get_domain_from_host(self, url_addy):
        # By definition the host has already removed the initial // markers
        # and the subsequent path.  All that is left is in the form of
        # xxx.domain.type.
        url_len = len(url_addy)
        first_dot = string.rfind(url_addy, ".")
        if first_dot < 1:
            return ""
        temp_url = url_addy[0:first_dot]
        # Locate second-from-right period, from front...
        second_dot = string.rfind(temp_url, ".")
        if second_dot > 1:
            # have a second dot, so output this to end...
            second_dot = second_dot + 1
            domain = url_addy[second_dot:url_len]
            return(domain)
        else:
            return(url_addy)
    
    def _update_from_dict (self, dict, after_taken):
        """Update private values from link attributes in dict.
        If 'after_taken' is true, means that _max_depth can be altered
        immediately.  Otherwise the new value is stored in the helper
        variable _new_max_depth"""
        

        # POST processing
        if dict.has_key ('post'):
            self.set_post (dict['post'])

        # MAXWIDTH processing
        if dict.has_key ('maxwidth'):
            self.set_maxwidth (dict['maxwidth'])

        # MAXHEIGHT processing
        if dict.has_key ('maxheight'):
            self.set_maxheight (dict['maxheight'])

        # BPP processing
        if dict.has_key ('bpp'):
            bpp = dict['bpp']
            try:
                bpp = int (bpp)
                self.set_bpp (bpp)
            except:
                pass

        # MAXDEPTH processing
        if dict.has_key ('maxdepth'):
            if after_taken:
                self.set_max_depth (int (dict['maxdepth']))
                self.set_current_depth (1)
            else:
                self._new_max_depth = int (dict['maxdepth'])

        # NOIMAGES processing
        if dict.has_key ('noimages'):
            self.set_bpp (0)

        # STAYONHOST processing
        # This attribute is only evaluated *after* the link has been taken
        if after_taken and dict.has_key ('stayonhost'):
            self.set_stay_on_host (self._url.get_host ())

        # STAYONDOMAIN processing
        # This attribute is only evaluated *after* the link has been taken
        if after_taken and dict.has_key ('stayondomain'):
            self.set_stay_on_domain (self._get_domain_from_host(self._url.get_host()))


        # STAYBELOW processing
        # This attribute is only evaluated *after* the link has been taken
        if after_taken and dict.has_key ('staybelow'):
            if string.lower (dict['staybelow']) == 'staybelow' or dict['staybelow'] == '':
                the_url = self._url.as_string (with_fragment=0)
                dict['staybelow'] = the_url
                #print "Setting STAYBELOW to %s" % the_url
            self.set_stay_below (dict['staybelow'])


        # Finally update it compleytely...
        self._dict.update (dict)


    def as_dict (self):
        newdict = self._dict.copy()
        newdict['url'] = self._url
        newdict['current_depth'] = self._current_depth
        newdict['max_depth'] = self._max_depth
        newdict['new_max_depth'] = self._new_max_depth
        newdict['stay_on_domain'] = self._stay_on_domain
        newdict['stay_on_host'] = self._stay_on_host
        newdict['stay_below'] = self._stay_below
        newdict['maxheight'] = self._maxheight
        newdict['maxwidth'] = self._maxwidth
        newdict['bpp'] = self._bpp
        newdict['_plucker_from_image'] = self._from_image
        newdict['post_data'] = self._post_data
        return newdict

    def make_child_attributes (self, url, dict, inline):
        """Generate a new SpiderLink object that represents the
        link from a document lead to by self.
        'dict' contains the attributes specified for this link"""
        
        new = SpiderLink (url, self._dict)
        if self._current_depth is not None:
            if inline:
                new.set_current_depth (self._current_depth)
            else:
                new.set_current_depth (self._current_depth + 1)
        if self._max_depth is not None:
            new.set_max_depth (self._max_depth)
        if self._stay_on_domain is not None:
            new.set_stay_on_domain (self._stay_on_domain)
        if self._stay_on_host is not None:
            new.set_stay_on_host (self._stay_on_host)
        if self._stay_below is not None:
            new.set_stay_below (self._stay_below)
        if self._maxwidth is not None:
            new.set_maxwidth (self._maxwidth)
        if self._maxheight is not None:
            new.set_maxheight (self._maxheight)

        new.set_bpp (self._bpp)

        new._update_from_dict (dict, after_taken=0)
        new.set_post (None)
        
        return new



    def link_taken (self, dict_of_attributes):
        """Note that this link has been taken.  The _max_depth is
        now changed to the new value (if there was one)."""
        if self._new_max_depth is not None:
            self._max_depth = self._new_max_depth
            self._current_depth = 1
        self._update_from_dict (dict_of_attributes, after_taken=1)


    def get_post (self):
        """Return the data for the post operation or None if none was specified"""
        if self._post_data is not None:
            return str (self._post_data)
        else:
            return None


    def set_post (self, post_data):
        """Set the data for a post operation"""
        self._post_data = post_data


    def get_maxwidth (self):
        return self._maxwidth


    def set_maxwidth (self, value):
        self._maxwidth = value
        

    def get_maxheight (self):
        return self._maxheight


    def set_maxheight (self, value):
        self._maxheight = value
        

    def get_bpp (self):
        return self._bpp


    def set_bpp (self, value):
        self._bpp = value
        

    def set_current_depth (self, n):
        self._current_depth = n


    def set_max_depth (self, n):
        self._max_depth = n

    def set_stay_on_domain (self, host):
        self._stay_on_domain = self._get_domain_from_host(host)
        

    def set_stay_on_host (self, host):
        self._stay_on_host = host


    def set_stay_below (self, urlpart):
        self._stay_below = urlpart


    def check_fetch (self, as_image):
        """Check whether this link should be taken.  If 'as_image' is
        true, this link points to an inline image"""

        # sys.stderr.write("check_fetch of %s...\n" % self._url.as_string(with_fragment=1))

        # mailto: gets always included
        if (self._url.get_protocol() == 'mailto'):
            return 1
        
        if self._max_depth is not None and self._current_depth is not None:
            if self._current_depth > self._max_depth:
                # Depth exceeded
                return 0
        if as_image and self._bpp == 0:
            # No images wanted
            return 0
            
        if self._stay_on_domain is not None:
            if self._stay_on_domain != self._get_domain_from_host(self._url.get_host()):
                # Got to another domain
                return 0

        if self._stay_on_host is not None:
            if self._stay_on_host != self._url.get_host():
                # Got to another host
                return 0

        if self._stay_below is not None:
            target_url = self._url.as_string (with_fragment=0)
            target_part = target_url[:len (self._stay_below)]
            if self._stay_below != target_part:
                return 0

        m = self._dict.get('url_pattern')
        if m and not m.match(self._url.as_string(with_fragment=0)):
            return 0

        # default: fetch
        return 1

    def get_from_image(self):
        return self._from_image

    def set_from_image(self, n):
        self._from_image = n
        self._dict['_plucker_from_image'] = n


class Spider:
    """A class to collect web pages by spidering from the home document."""

    def __init__ (self, retriever,
                  parser, \
                  collection, \
                  exclusion_list, \
                  config,
                  alias_list):
        """Call with a retriever and a parser function.
        'retriever' gets called with a Url.URL and should return
        a (header-dict, data) tupe.  In header-dict, the values 'error
        code' and 'error text' *must* be defined.
        If the data is valid, 'error code' must be zero.

        'parser' gets called with the url and these header-dict and
        the body and should return a PluckerDocument or None (if
        failed).

        Implementation:
        We have one queue and two dictionaries in which we just
        store, whether some URL has been tried before (and collected
        or failed)."""
        
        self._retriever = retriever
        self._parser = parser
        self._config = config

        # _queue contains a list of (URL, attributes, key) pairs to fetch
        self._queue = []

        # _collected contains a mapping of retrieved URLs to PluckerDocs
        if collection is None:
            self._collected = {}
        else:
            self._collected = collection

        # _failed contains a mapping of failed retrieval URLs to failure headers
        self._failed = {}

        self._exclusion_list = exclusion_list
        if alias_list is None:
            self._alias_list = AliasList ()
        else:
            self._alias_list = alias_list
        self._fatal_error = 0

        # whether we're going depth-first or breadth-first
        self._depth_first = config.get_bool('depth_first', 0)

        # Now initialize the first things we *do* want in the
        # collection of documents
        self._home_url = URL(config.get_string ('home_url', 'plucker:/home.html'))
        bpp = config.get_int ('bpp', 1)
        # ROB (Oct 26,2003) Commenting out this line. doing_big_imgs is never used. We check again in ImageParser.py
        # _related_images anyway, together with the rest of image parsing.
        #self._doing_big_imgs = config.get_string('alt_maxwidth', None) or config.get_string('alt_maxheight', None)
        attributes = {'maxdepth': "%d" % config.get_int ('home_maxdepth', 2),
                      'bpp': "%d" % bpp}
        if config.get_bool ('home_stayonhost', 0):
            attributes['stayonhost'] = 1
        if config.get_bool ('home_stayondomain', 0):
            attributes['stayondomain'] = 1
        tmp = config.get_string ('home_staybelow')
        if tmp is not None:
            attributes['staybelow'] = tmp
        tmp = config.get_string ('home_url_pattern')
        if tmp is not None:
            m = re.compile(tmp)
            if m:
                sys.stderr.write("Regexp pattern is '%s'\n" % m.pattern)
                attributes['url_pattern'] = m
        self.add_queue (self._home_url,
                        SpiderLink (self._home_url, attributes),
                        force=1)


    def _create_id_string (self, attributes):
        values = (type(attributes) == types.DictType and attributes.items()) or attributes.as_dict().items()
        valueslist = []
        for value in values:
            if (value[0][:9] == '_plucker_' or
                value[0] in LINK_ATTRIBUTES_TO_IGNORE):
                continue
            valueslist.append((string.lower(value[0]), value[1],))
        valueslist.sort()
        return str(valueslist)

    def _needs_processing (self, key, url, attr):
        if self._collected.has_key (key):
            # we need to register the already-collected document with the internal plucker keys that
            # are being held by the link under consideration, so that those keys are resolved
            self._register_document(attr, self._collected[key])
            return 0
        if self._failed.has_key (url):
            return 0

# We'll err on the side of adding the URL to the queue, since we already
# check later to see whether or not we've already fetched it.
#
#        for (other_url, attr, key) in self._queue:
#            if other_url == url:
#                return 0
#
        return 1

    def add_queue (self, origurl, attr, force=0):
        """Maybe add url to the queue"""
        url = self._alias_list.get (origurl)
        # mailto: gets always included
        key = str(url) + '\0' + self._create_id_string(attr)
        if (url[:7] == 'mailto:'):
            self._queue.append ((url, attr, key))
            return 1
        if not force and self._exclusion_list:
            if not self._exclusion_list.check (url):
                message(2, "Excluding '%s'\n" % url)
                return 0
        if force or self._needs_processing (key, url, attr):
            self._queue.append ((url, attr, key))
            return 1
        return 0

    def done (self):
        """Check whether something rests to be done in the queue"""
        return len (self._queue) == 0


    def process_all (self, verbose=0):
        """Process until all done"""

        # figure out how many we expect to read
        estimate = 0
        statusfile = None
        statusfilename = self._config.get_string('status_file')
        if statusfilename:
                if os.path.exists(statusfilename):
                    statusfile = open(statusfilename, 'r+')
                else:
                    statusfile = open(statusfilename, 'w+')
                if statusfile:
                    line = statusfile.readline()
                    if line:
                        tokens = string.split(line)
                        if tokens:
                            estimate = string.atoi(tokens[0])

        # OK, process everything
        self._filenum = 1
        while not self.done ():
            self.process (verbose, estimate, statusfile)

        message("---- all %d pages retrieved and parsed ----", len(self._collected))

        if statusfile:
            statusfile.seek(0)
            statusfile.write("%d %d %d\n" % (len(self._collected), len(self._queue), estimate))
            statusfile.flush()
            statusfile.close()


    def _register_document (self, attribs, doc):
        # See if the attribute dictionary has any keys that need to be bound
        # to this document, for links of various kinds.  If so, call the
        # register_doc method on the doc with the specified index.
        attribute_dict = (type(attribs) == types.DictType and attribs) or attribs.as_dict()
        #sys.stderr.write(str(attribute_dict) + '\n')
        if attribute_dict.get('_plucker_from_image'):
            doc.register_doc(attribute_dict.get('_plucker_id_tag_inlineimage'))
        elif attribute_dict.get('_plucker_alternate_image'):
            #sys.stderr.write(str(attribute_dict) + '\n')
            doc.register_doc(attribute_dict.get('_plucker_id_tag_outoflineimage'))
        if attribute_dict.get('_plucker_id_tag'):
            #sys.stderr.write(str(attribute_dict) + '\n')
            doc.register_doc(attribute_dict.get('_plucker_id_tag'))



    def process (self, verbose, estimate, statusfile):
        """Process the next thing in the queue"""

        # We use a number of different forms of the 'URL to be fetched'
        # in this routine, as follows:
        #   urltext -- the string form of the URL popped from the queue
        #   url -- the URL object formed from "urltext"
        #   

        #if verbose>1:
        #    # To help debugging, we can write out the current queue
        #    f = open ("spider.status", "w")
        #    f.write ("Queue:\n")
        #    for (key, attr) in self._queue:
        #        if self._collected.has_key (key):
        #            stat = "done"
        #        elif self._failed.has_key (key):
        #            stat = "failed"
        #        else:
        #            stat = ""
        #        f.write ("  %s %s  %s\n" % (repr(key), stat, str(attr)))
        #    f.write ("\nNot Collected:\n")
        #    for key in self._failed.keys ():
        #        attr = self._failed[key]
        #        f.write ("  %s  %s\n" % (repr(key), str(attr)))
        #    f.close ()

        import tempfile

        if self._queue:
            if estimate:
                message ("---- %d expected, %d collected, %d to do ----\n", estimate, len (self._collected), len (self._queue))
            else:
                message ("---- %d collected, %d to do ----\n", len (self._collected), len (self._queue))
            if statusfile:
                statusfile.seek(0)
                statusfile.write("%d %d %d\n" % (len(self._collected), len(self._queue), estimate))
                statusfile.flush()
            # de-queue end of queue
            if self._depth_first:
                (urltext, attributes, key) = self._queue[-1]
                del self._queue[-1]
            else:
                (urltext, attributes, key) = self._queue[0]
                del self._queue[0]
            attribute_dict_string = self._create_id_string(attributes)
            url = URL (urltext)
            if verbose:
                line_length = self._config.get_int('status_line_length', 60)
                urltext = str(url)
                if len (urltext) > line_length:
                    urltext = urltext[:line_length - 20] + "....." + urltext[-15:]
                message("Processing %s...", urltext)

            # see if it's already been collected
            post_data = attributes.get_post ()
            # urltext is the URL without any fragment
            urltext = url.as_string (with_fragment=0)
            # if there are parameters, we add them back on
            if post_data is not None:
                urltext_key = urltext + post_data
            else:
                urltext_key = urltext
            # OK, now that we know the URL used as the mapping key, check it
            key = urltext_key + '\0' + attribute_dict_string
            message(3, "checking " + str(key))
            #sys.stderr.write('key is ' + key + '\n')
            if self._collected.has_key (key):
                # already collected
                message("  Already retrieved and parsed.")
                self._register_document(attributes, self._collected[key])
                return

            # not collected, how about failed?
            if self._failed.has_key (urltext_key):
                # already tried, but failed
                message("  Already tried, but failed.")
                return
                
            # OK, not collected or failed -- so retrieve the contents
            (header, document) = self._retriever (url, alias_list=self._alias_list, \
                                                  post_data=post_data)
            assert header.has_key ('error code'), "Headers from retriever have no error code"
            assert header.has_key ('URL'), "Headers from retriever have no URL"

            # Check for successful fetch
            if header['error code'] != 0:
                # retrieving has failed.
                # self._failed[urltext_key] = header
                self._failed[urltext_key] = None
                if verbose:
                    if header.has_key ('error code'):
                        code = header['error code']
                    else:
                        code = "No error code"
                    if header.has_key ('error text'):
                        text = header['error text']
                    else:
                        text = "No error text"
                    message("  Retrieval failed: %s -- %s." % (code, text))
                failed_url = urltext
                if failed_url == self._alias_list.get (self._home_url):
                    error("Fetching the home document failed.  Aborting all!")
                    self._queue = []
                    self._collected = {}
                    self._fatal_error = 1
            else:
                assert header.has_key ('content-type'), \
                       "Headers from retriever have no content-type (%s)" % repr (header)
                # Fetched OK
                message("  Retrieved ok.")
                # "new_url" is the URL the HTTP server sent back to us.
                new_url = URL (header['URL']).as_string (with_fragment=0)
                # We compare it with the URL we used to do the HTTP operation.
                ####################################################################
                # header['URL'] are file:C:\path\file.ext on python 1.52 and       #
                # C:\path\file.ext om Python 2.0. Also the case of the Drive       #
                # letter may change. So a move detected here. This should fix that.#
                ####################################################################
                if sys.platform == 'win32' and (string.lower(urltext[0:5]) == 'file:'):
                    if new_url[0:5] != 'file:':
                       new_url = 'file:' + new_url
                       new_url = URL (new_url).as_string (with_fragment=0)
                # again, we form the mapping key to see if it's already been processed
                if post_data is not None:
                    new_url_key = new_url + post_data
                else:
                    new_url_key = new_url
                # now check to see if the new URL is different from the original,
                # and add an alias if that's the case
                if urltext != new_url:
                    message("  Moved from '%s' to '%s'." % (urltext, new_url))
                    if self._alias_list.get (urltext) == urltext:
                        # The move was not recognized by the Retriever!
                        # This can be caused by specifying a file: URL
                        # without the 'file:' part
                        self._alias_list.add (urltext, new_url)

                # Check to see if we already have it, or if we're not fetching
                # this URL...
                if urltext != new_url:
                    key = new_url_key + '\0' + attribute_dict_string
                    if self._collected.has_key (key):
                        message("  Already retrieved and parsed.")
                        self._register_document(attributes, self._collected[key])
                        return
                    if not self._exclusion_list.check (new_url):
                        message("  Is excluded.")
                        return

                # Check for a filter to run document through
                if header['content-type'][:5] == "text/":
                    filter = self._config.get_string ('filter')
                    if filter is not None:
                        try:
                            tmpfile = tempfile.mktemp()
                            f = open(tmpfile, "wb")
                            f.write(document)
                            f.close()

                            command = filter + " " + tmpfile
                            pipe = os.popen(command)
                            document = pipe.read()
                            pipe.close()

                        finally:
                            try: os.unlink(tmpfile) 
                            except: pass

                # OK, it's fair game, so we parse it
                try:
                    pluckerdoc = self._parser (new_url,
                                               header,
                                               document,
                                               self._config,
                                               attributes.as_dict())
                except:
                    show_exception(2)
                    pluckerdoc = None

                # Successful parse?
                if pluckerdoc is None:
                    # No.  Signal error.
                    headers = {'error code': -1,
                               'error text': "parsing failed"}
                    # self._failed[new_url_key] = headers
                    self._failed[new_url_key] = None
                    message("  Parsing failed.")
                    return

                # OK, at this point we have a valid pluckerdoc
                self._collected[key]=pluckerdoc
                #sys.stderr.write('logging ' + key + '\n')
                self._register_document(attributes, pluckerdoc)
                tables = pluckerdoc.get_tables()
                for i in range(0, len(tables)):
                    attrs = tables[i].get_attrs()
                    self._register_document(attrs, tables[i])
                    self._collected[attrs['href']] = tables[i]

                if pluckerdoc.is_multiimage_document():
                    pieces = pluckerdoc.get_pieces()
                    for (piece_doc, piece_id) in pieces:
                        piece_doc.register_doc(piece_id)
                        tkey = piece_doc.get_url() + '\0'
                        self._collected[tkey] = piece_doc

                # Now check for some extra processing, depending on the
                # type of page the URL pointed to

                # For text documents, we want to harvest any links in the text

                if pluckerdoc.is_text_document ():
                    (hrefs, imagerefs) = pluckerdoc.get_external_references ()

                    doc_ref_count = 0
                    for (suburltext, dict) in hrefs:
                        suburl = URL (suburltext)
                        suburl.remove_fragment ()
                        if suburl.as_string(with_fragment=0)[:17] != "plucker:/~parts~/":
                            # Subparts are not needed for fetching
                            message(3, "  Looking at suburl %s...", str(suburltext))
                            new_attr = attributes.make_child_attributes (suburl, dict, inline=0)
                            if new_attr.check_fetch (as_image = 0):
                                new_attr.link_taken (dict)
                                if self.add_queue (suburl, new_attr):
                                    doc_ref_count = doc_ref_count + 1

                    img_ref_count = 0
                    for (suburltext, dict) in imagerefs:
                        suburl = URL (suburltext)
                        suburl.remove_fragment ()
                        new_attr = attributes.make_child_attributes (suburl, dict, inline=1)
                        new_attr.set_from_image(1)
                        if new_attr.check_fetch (as_image = 1):
                            new_attr.link_taken (dict)
                            if self.add_queue (suburl, new_attr):
                                img_ref_count = img_ref_count + 1
                            else:
                                message(2, "  Not fetching image %s (already fetched)", new_attr)
                        else:
                            message(2, "  Not fetching image %s", str (suburl))

                    message ("  Parsed ok%s%s." %
                             (((doc_ref_count > 0 or img_ref_count > 0) and "; ") or "",
                              ("%s%s%s" %
                               (((doc_ref_count > 0) and ("added %d document link%s" % (doc_ref_count, (doc_ref_count != 1 and "s") or ""))) or "",
                                ((doc_ref_count > 0) and (img_ref_count > 0) and " and ") or "",
                                ((img_ref_count > 0) and ("%d image%s" % (img_ref_count, (img_ref_count != 1 and "s") or ""))) or ""))))

                    # pluckerdoc.clear_external_references()


                # For image documents, we want to create alternate renditions
                # of the image, if called for

                elif pluckerdoc.is_image_document() or pluckerdoc.is_multiimage_document():

                    alternate_count = 0
                    others = pluckerdoc.get_related_images()
                    for (other_url, other_attributes) in others:
                        message(2, "  Rendering other versions of image %s...\n" % other_url)
                        testkey = other_url + '\0' + self._create_id_string(other_attributes)
                        if self._collected.has_key(testkey):
                            message(3, "    Reusing already-rendered image version %s.\n" % self._collected[testkey].get_url())
                            continue

                        message(3, "    Key is " + str(key) + ".\n")

                        try:
                            newdoc = self._parser(other_url,
                                                  header,
                                                  document,
                                                  self._config,
                                                  other_attributes)
                            self._collected[testkey] = newdoc
                            self._register_document(other_attributes, newdoc)
                            alternate_count = alternate_count + 1

                            if newdoc.is_multiimage_document():
                                pieces = newdoc.get_pieces()
                                for (piece_doc, piece_id) in pieces:
                                    piece_doc.register_doc(piece_id)
                                    tkey = piece_doc.get_url() + '\0'
                                    self._collected[tkey] = piece_doc

                        except:
                            show_exception(2)
                            message(2, "    Parsing of alternate rendition %s failed", testkey)

                    message ("  Parsed ok%s." %
                             (((alternate_count > 0)
                               and ("; added %d alternate rendition%s" % (alternate_count, (alternate_count != 1 and "s") or ""))) or ""))

                else:        # all other document types

                    message ("  Parsed ok.")
                    
    def get_collected (self):
        return self._collected


    def encountered_fatal_error (self):
        return self._fatal_error



def execute_commands (item_name, config):
    verbosity = config.get_int ('verbosity', 1)
    for affix  in [''] + map (lambda n: str (n), range (1,10)):
        command = config.get_string (item_name + affix, "")
        if command:
            message("Executing '%s': %s" % (item_name+affix, command))
            try:
                if os.system (command):
                    raise RuntimeError("execution of command '%s' failed" % command)
            except:
                error("Error during execution of '%s': %s" % (item_name+affix, command))


def main (config, excl_lists=[]):
    import os, sys

    from PyPlucker.Parser import generic_parser
    from PyPlucker.Retriever import SimpleRetriever
    from PyPlucker.Writer import CacheWriter, PDBWriter
    from PyPlucker.ExclusionList import ExclusionList
    import PyPlucker.PluckerDocs

    PyPlucker.PluckerDocs.PluckerTextDocument.seamless_fragments = config.get_bool ('seamless_fragments',1)
    PyPlucker.PluckerDocs.PluckerTextDocument.link_fragments = config.get_bool ('link_fragments',0)

    pluckerdir = config.get_string ('pluckerdir')
    assert pluckerdir is not None
    pluckerhome = config.get_string ('PLUCKERHOME')
    assert pluckerhome is not None

    if not os.path.exists (pluckerhome) or not os.path.isdir (pluckerhome):
        error ("Pluckerhome (%s) does not exist or isn't a directory\n" % pluckerhome)
        sys.exit (1)
    if not os.path.exists (pluckerdir) or not os.path.isdir (pluckerdir):
        error ("Pluckerdir (%s) does not exist or isn't a directory\n" % pluckerdir)
        sys.exit (1)
    
    verbosity = config.get_int ('verbosity', 1)
    message("Pluckerdir is '%s'..." % pluckerdir)

    if verbosity:
        if os.environ.has_key ('HTTP_PROXY') and not (os.environ.has_key ('HTTP_PROXY_USER') and os.environ.has_key ('HTTP_PROXY_PASS')):
            message("Using proxy '%s'..." % os.environ['HTTP_PROXY'])
        if os.environ.has_key ('HTTP_PROXY') and (os.environ.has_key ('HTTP_PROXY_USER') and os.environ.has_key ('HTTP_PROXY_PASS')):
            message("Using proxy '%s' with authentication for user '%s'..." % (os.environ['HTTP_PROXY'],os.environ['HTTP_PROXY_USER']))

    # try to set a socket timeout -- thanks to lemburg for this patch
    socket_timeout = config.get_int('retrieval_timeout', 0)
    if socket_timeout:
        try:
            try:
                import timeoutsocket
            except:
                from PyPlucker.helper import timeoutsocket
            message(2, "Setting retrieval timeout to %d seconds", socket_timeout)
            timeoutsocket.setDefaultSocketTimeout(socket_timeout)
        except:
            message(2, "Couldn't set retrieval timeout to desired %d seconds -- will wait forever if necessary", socket_timeout)
        
    import PyPlucker.PluckerDocs
    PyPlucker.PluckerDocs._DOC_HEADER_SIZE = 8
    PyPlucker.PluckerDocs._PARA_HEADER_SIZE = 4

    alias_list = AliasList ()

    if config.get_bool ('zlib_compression', 0):
        try:
            import zlib
        except ImportError:
            message(0, "Your Python installation does not support ZLib compression.")
            message(0, "We fall back to DOC compression.")
            config.set ('zlib_compression', 'false')
    if config.get_bool ('zlib_compression', 0):
        owner_id = config.get_string('owner_id_build')
        if owner_id:
            PyPlucker.PluckerDocs.UseZLibCompression (owner_id)
        else:
            PyPlucker.PluckerDocs.UseZLibCompression ()
        message(2, "ZLib compression turned on")
    #
    #  Load the exclusion lists..
    #
    exclusion_list = ExclusionList (include_by_default=1)

    filename = os.path.join (pluckerhome, 'exclusionlist.txt')
    if os.path.exists (filename):
        message(2, "Using exclusion list %s", filename)
        exclusion_list.load_file (filename)

    filename = os.path.join (pluckerdir, 'exclusionlist.txt')
    if os.path.exists (filename):
        message(2, "Using exclusion list %s", filename)
        exclusion_list.load_file (filename)

    config_excl_list = config.get_string ('exclusion_lists')
    if config_excl_list is not None:
        config_excl_list = string.split (config_excl_list, os.pathsep)
        for filename in config_excl_list:
            if not os.path.isabs (filename):
                filename = os.path.join (pluckerdir, filename)
            if os.path.exists (filename) and \
               (os.path.isfile (filename) or os.path.islink (filename)):
                message(2, "Adding extra exclusion list %s", filename)
                exclusion_list.load_file (filename)

    for filename in excl_lists:
        if os.path.exists (filename) and \
           (os.path.isfile (filename) or os.path.islink (filename)):
            message(2, "Adding extra exclusion list %s", filename)
            exclusion_list.load_file (filename)
    #
    # finished loading exclusion lists
    #

    home_url = config.get_string ('home_url', 'plucker:/home.html')
    if not URL(home_url).get_protocol():
        home_url = 'file:' + home_url
        config.set('home_url', home_url)
    if home_url != 'plucker:/home.html':
        alias_list.add ('plucker:/home.html', home_url)

    retriever = SimpleRetriever (pluckerdir, pluckerhome, config)
    
    max_depth = config.get_int ('home_maxdepth', 2)

    assert config.get_bool ('use_cache') is not None
    if config.get_bool ('use_cache'):
        cachedir = os.path.join (pluckerdir, config.get_string ('cache_dir_name', 'cache'))
        if not (os.path.exists(cachedir) and os.path.isdir(cachedir)):
            error("cache directory does not exist:  " + cachedir + "\n")
            return 1
    else:
        if not (os.path.exists(pluckerdir) and os.path.isdir(pluckerdir)):
            error("Plucker directory does not exist:  " + cachedir + "\n")
            return 1
        doc_file = config.get_string ('doc_file')
        if not doc_file:
            doc_file = config.get_string ('db_file')
            if doc_file:
                deprecated( "db_file", "doc_file" )
        if (doc_file == '<stdout>'):
            filename = doc_file
        else:
            extension = config.get_string("filename_extension", "pdb")
            filename = os.path.join (pluckerdir, doc_file + "." + extension)
        doc_name = config.get_string ('doc_name')
        if not doc_name:
            doc_name = config.get_string ('db_name')
            if doc_name:
                deprecated( "db_name", "doc_name" )
            elif (doc_file != '<stdout>'):
                # use basename in case only file name is given
                doc_name = os.path.basename (doc_file)
            else:
                # generate name based on home URL
                if len(home_url) > 31:
                    doc_name = "..." + home_url[-28:]
                else:
                    doc_name = home_url

    spider = Spider (retriever.retrieve,
                     generic_parser, \
                     collection=None, \
                     exclusion_list=exclusion_list, \
                     config=config,
                     alias_list=alias_list)
    spider.process_all(verbose=verbosity)

    if spider.encountered_fatal_error ():
        error("Fatal error while processing.  Nothing written.")
        return 1
    
    if verbosity > 2:
        PyPlucker.PluckerDocs.display_registrations()

    message("\nWriting out collected data...")
    collection = spider.get_collected ()

    # at this point, we don't need anything except the collection,
    # so we can release all the memory used by spider
    del spider

    if config.get_bool ('use_cache'):
        writer = CacheWriter (collection, config, cachedir)
        message("Writing to cache dir %s" % cachedir)
    else:
        writer = PDBWriter (collection, config, name=doc_name, version=1, filename=filename)
        message("Writing document '%s' to file %s" % (doc_name, filename))

    mapping = writer.write (verbose=verbosity, alias_list=alias_list)

    if verbosity > 2:
        mapping.print_mapping()
       
    if verbosity > 1:
        items = Parser.unknown_things.keys ()
        if items:
            message(2, "Unknown items encountered:")
            items.sort ()
            for item in items:
                message(2, "  %s: %s" % (item, Parser.unknown_things[item]))

    message("Done!")

    return 0

def deprecated ( oldname, newname ):
    sys.stderr.write("NOTE: %s is a deprecated option. Please use the %s option instead.\n" % ( oldname, newname ))

def realmain (outputstream, argv=None):

    import getopt, os, string, re, tempfile

    if not argv:
        argv = sys.argv

    if os.environ.has_key ('PLUCKERHOME'):
        pluckerhome = os.environ['PLUCKERHOME']
    else:
        pluckerhome = os.path.expanduser ("~/.plucker")

    if os.environ.has_key ('PLUCKERDIR'):
        pluckerdir = os.environ['PLUCKERDIR']
    else:
        pluckerdir = None



    def display_version ():
        message(0, "Plucker version %s\n" % __version__ )
        
    def usage (reason=None, pluckerhome=pluckerhome):
        if reason:
            error(str(reason))
        sys.stderr.write("Usage: %s [OPTIONS] [HOMEURL]\n" % sys.argv[0])
        sys.stderr.write("(Type '%s --help' for more information.)\n" % sys.argv[0])
        sys.exit (1)

    def display_help (pluckerhome=pluckerhome):

        message(0, "Usage: %s [OPTIONS] [HOMEURL]" % sys.argv[0])
        message(0, "  where HOMEURL is a 'file:' or 'http:' URL (which can alternatively")
        message(0, "  be specified with --home-url=<homeurl>) and OPTIONS are:")
        message(0, "    -c, --update-cache:")
        message(0, "                   Write a traditional cache directory in the <plucker dir>")
        message(0, "    -f <name prefix>, --doc-file=<name prefix>")
        message(0, "                   Specify the name of the output file (see also --pluckerdir).")
        message(0, "                   If not specified, the Plucker doc will be written")
        message(0, "                   to stdout, if stdout is not a tty.")
        message(0, "    -h, --help:    Print this help")
        message(0, "    -N <name>, --doc-name <name>")
        message(0, "                   Specify the name of the document (NOT the filename).")
        message(0, "                   Defaults to -f's argument.")
        message(0, "    -q, --quiet:   Be quiet, i.e. set verbosity level 0")
        message(0, "    -v:            Set verbosity level 1 (which is the default)")
        message(0, "    -V <n>, --verbosity=<n>:")
        message(0, "                   Set verbosity level <n>")
        message(0, "                     Verbosity level 0 is silent except for errors")
        message(0, "                     Verbosity level 1 gives progress status (default)")
        message(0, "                     Verbosity level 2 is used for debugging")
        message(0, "    -P<dir>, --pluckerhome=<dir>:")
        message(0, "                   Use <dir> as plucker home instead of the default ")
        message(0, "                   %s (~/.plucker/ or $PLUCKERHOME)", pluckerhome)
        message(0, "    -p<dir>, --pluckerdir=<dir>:")
        message(0, "                   Use <dir> as plucker dir instead of the default ")
        message(0, "                   Defaults to same as plucker home.")
        message(0, "    --bpp=<num>:   Bits per pixel for images; defaults to 1")
        message(0, "                     <num> =  0, 1, 2, 4, 8 or 16 (16 not on Windows)")
        message(0, "    --noimages:    Do not include images (same as --bpp=0)")
        message(0, "    -H <homeurl>, --home-url=<homeurl>:  Use <homeurl> as the root document.")
        message(0, "                     Defaults to plucker:/home.html (i.e. home.html in")
        message(0, "                     the plucker dir)")
        message(0, "    -M <n>, --maxdepth=<n>:")
        message(0, "                   Use MAXDEPTH=<n> on the home document.  Defaults to 2")
        message(0, "    -E <filename>, --exclusion-list <filename>: ")
        message(0, "                   Add <filename> to list of files searched for exclusion lists")
        message(0, "    -s <secname>, --extra-section=<secname>:")
        message(0, "                   Add <secname> to the list of searched section in the config files")
        message(0, "    --zlib-compression, --doc-compression:")
        message(0, "                   Specify which compression method to use. (For expert's use)")
        message(0, "    --compression=<compression-type>:")
        message(0, "                   Use <compression-type> as the compression format")
        message(0, "                   for the database.  Allowable options are 'doc', for")
        message(0, "                   Palm DOC compression, or 'zlib', for zlib compression.")
        message(0, "                   Zlib compression is typically better than DOC compression.")
        message(0, "    --no-urlinfo:  Do not include info about the URLs")
        message(0, "    --category=<category-name1>[;<category-name2>;..;<category-name16>]:")
        message(0, "                   Put <category-name> in the database as the default")
        message(0, "                   viewer category for the database.")
        message(0, "                   It is possible to assign several categories separated by ';'")
        message(0, "    --depth-first:")
        message(0, "                   Do a depth-first pass through the web graph, rather than")
        message(0, "                   the default breadth-first traversal.")
        message(0, "    --stayonhost:  Do not follow external URLs")
        message(0, "    --stayondomain:")
        message(0, "                   Do not follow URLs off of this domain")
        message(0, "    --staybelow=<url-prefix>:")
        message(0, "                   Automatically exclude any URL that doesn't begin with <url-prefix>.")
        message(0, "    --maxheight=<n>:")
        message(0, "                   Set maximum height of images to <n> pixels.")
        message(0, "    --maxwidth=<n>:")
        message(0, "                   Set maximum width of images to <n> pixels.")
        message(0, "    --alt-maxheight=<n>:")
        message(0, "                   Set alternative maximum height of images to <n> pixels.  This value")
        message(0, "                   is used for 'big' versions of inline images that had to be scaled")
        message(0, "                   down in size to obey the MAXWIDTH and MAXHEIGHT parameters.")
        message(0, "                   Special values that can be used are 0 and -1, which do the following:")
        message(0, "                   Set --alt-maxheight=0 and --alt-maxheight=0 to link to original sized")
        message(0, "                   'big' versions.")
        message(0, "                   Set --alt-maxheight=-1 and --alt-maxheight=-1 to not link to any")
        message(0, "                   'big' versions at all.")
        message(0, "    --alt-maxwidth=<n>:")
        message(0, "                   Set alternative maximum width of images to <n> pixels.  This value")
        message(0, "                   is used for 'big' versions of inline images that had to be scaled")
        message(0, "                   down in size to obey the MAXWIDTH and MAXHEIGHT parameters.")
        message(0, "    --launchable, --not-launchable:")
        message(0, "                   Set (or unset) the launchable bit in the output file.")
        message(0, "    --backup, --no-backup:")
        message(0, "                   Set or clear the backup bit in the output file.")
        message(0, "    --beamable, --not-beamable:")
        message(0, "                   Set or clear the beamable bit in the output file.")
        message(0, "    --charset=<name>:")
        message(0, "                   Set the default charset to that specified by <name>.")
        message(0, "    --owner-id=<name>:")
        message(0, "                   Set owner-id of the output document to <name>.")
        message(0, "    --url-pattern=<regexp-pattern>:")
        message(0, "                   Only fetch URLs if they match the regular expression specified")
        message(0, "                   by <regexp-pattern>.")
        message(0, "    --title=<string>:")
        message(0, "                   Set the title of the document to <string>.")
        message(0, "    --author=<string>:")
        message(0, "                   Set the author of the document to <string>.")
        message(0, "    --status-file=<filename>:")
        message(0, "                   Use <filename> as the status file.")
        message(0, "    --referrer=<referrer string>:")
        message(0, "                   Send the specified <referrer string> when fetching remote pages.")
        message(0, "    --user-agent=<user-agent string>:")
        message(0, "                   Send the specified <user-agent string> as the identification of")
        message(0, "                   of the 'browser' when fetching remote pages.  Defaults to")
        message(0, "                   Plucker/Py-%s." % __version__)
        message(0, "    --http-proxy=<string>:")
        message(0, "                   HTTP proxy used on machine. Example is http://proxy.aol.com:8080")
        message(0, "    --http-proxy-user=<string>:")
        message(0, "                   HTTP proxy username")
        message(0, "    --http-proxy-pass=<string>:")
        message(0, "                   HTTP proxy password")
        message(0, "    --version:")
        message(0, "                   Print out the version of Plucker that is being run and then exit.")
        message(0, "    --tables:")
        message(0, "                   Generate real tables instead of vertical lists of cells.")
        message(0, "    --fragments=<fragment type>")
        message(0, "                   Set the way the 30K fragments of a page are joined to")
        message(0, "                   <fragment type>:")
        message(0, "                     seamless: seamless connection for 1.5+ viewer")
        message(0, "                     link:     insert links to previous/next fragment")
        message(0, "                     both:     insert links and support seamless connection")
        message(0, "                   Defaults to seamless.")
        message(0, "    --creator-id=<CrId>")
        message(0, "                   Sets 4 character Palm creator ID of output database. Example is Plkr") 
        message(0, "                   Plucker viewer loads documents with creator ID of Plkr into")     
        message(0, "                     the document library.") 
        message(0, "                   Defaults to Plkr")   
        message(0, "    --filter=<filter name>:")
        message(0, "                   Pass fetched documents through filter prior to parsing.")
        message(0, "    --bookmarks=<bookmark type>:")
        message(0, "                   Set bookmarks added to database to <bookmark type>.")
        message(0, "                   pages:    add a bookmark for each text/* page collected.")
        message(0, "                   links:    add a bookmark for each bookmark type link tag.")
        message(0, "                   both:     add both kinds of bookmarks.")
        message(0, "                   Defaults to None.")   
        message(0, "")
        message(0, "Note that you must specify either -f or specify HOMEURL as an argument,")
        message(0, " or specify -c to update a cache.")
        message(0, "")

    try:
        home_url = None
        verbosity = None
        bpp = None
        max_depth=None    
        seamless_fragments=None
        link_fragments=None
        use_cache = None
        use_file = None
        doc_name = None
        exclusion_lists = []
        extra_sections = []
        zlib_compression = None
        no_url_info = None
        stayondomain = None
        stayonhost = None
        staybelow = None
        category = None
        maxwidth = None
        maxheight = None
        alt_maxwidth = None
        alt_maxheight = None
        launchable = None
        backup = None
        copy_protect = None
        iconfile = None
        default_charset = None
        owner_id = None
        url_pattern = None
        referrer = None
        user_agent = None
        title = None
        author = None
        statusfile = None
        depthfirst = None
        tables = None
        filter = None
        bookmarks = None
        bookmark_pages = None
        http_proxy = None
        http_proxy_user = None
        http_proxy_pass = None
        creator_id = None

        (opts, args) = getopt.getopt(argv[1:], "f:chqvV:p:P:H:E:M:N:s:", \
                                     [  "db-file=", "doc-file=", "help",
                                        "quiet", "pluckerdir=", "pluckerhome=",
                                        "bpp=", "noimages", "exclusion-list=",
                                        "maxdepth=", "db-name=", "doc-name=",
                                        "extra-section=", "verbosity=",
                                        "zlib-compression", "doc-compression",
                                        "no-urlinfo", "stayondomain",
                                        "stayonhost", "staybelow=", "category=",
                                        "maxheight=", "maxwidth=",
                                        "alt-maxheight=", "alt-maxwidth=",
                                        "compression=", "home-url=",
                                        "update-cache", "launchable",
                                        "not-launchable", "backup",
                                        "no-backup", "beamable", "not-beamable",
                                        "icon=", "charset=", "owner-id=",
                                        "url-pattern=", "referrer=",
                                        "user-agent=", "title=", "author=",
                                        "status-file=", "version",
                                        "tables", "depth-first", "http-proxy=",
                                        "http-proxy-user=", "http-proxy-pass=",
                                        "fragments=", "creator-id=",
                                        "bookmarks="])
        if args:
            # usage ("Only options are allowed as arguments.")
            if len(args) > 1:
                if args[1][0] == '-':
                    usage("All options (such as '" + string.join(args[1:]) + "') must be specified before the argument (" + str(args) + ").")
                else:
                    usage("Only one 'root' document should be specified as an argument.")
            root = args[0]
            if (len(root) > 5) and (((string.lower(root[:6]) == 'https:') or string.lower(root[:5]) == 'http:') or (string.lower(root[:5]) == 'file:')):
                home_url = root
            elif os.path.exists(root):
                home_url = 'file:'+root
            else:
                usage("Can't locate " + root)

        for (opt, arg) in opts:
            if opt == "--version":
                display_version()
                sys.exit(0)
            if opt == "-h" or opt == "--help":
                display_help()
                sys.exit(0)
            elif opt == "--fragments" and arg == "both":
                seamless_fragments = 'true'
                link_fragments     = 'true'
            elif opt == "--fragments" and arg == "link":
                seamless_fragments = 'false'
                link_fragments     = 'true'
            elif opt == "--fragments" and arg == "seamless":
                seamless_fragments = 'true'
                link_fragments     = 'false'
            elif opt == "-c" or opt == "--update-cache":
                use_cache = 1
            elif opt == "-f" or opt == "--doc-file":
                use_file = arg
            elif opt == "--db-file":
                deprecated( "db-file", "doc-file" )
                use_file = arg
            elif opt == "--bpp":
                bpp = int (arg)
                if bpp != 0 and bpp != 1 and bpp != 2 and bpp != 4 and bpp != 8 and bpp != 16:
                    usage ("Only 0, 1, 2, 4, 8 or 16 allowed for -bpp")
            elif opt == "--noimages":
                bpp = 0
            elif opt == "-H" or opt == "--home-url":
                if home_url and (home_url <> arg):
                    usage("Two different root URLs specified:  " + home_url + " and " + arg)
                home_url = arg
            elif opt == "-E" or opt == "--exclusion-list":
                exclusion_lists.append (arg)
            elif opt == "-q" or opt == "--quiet":
                verbosity = 0
            elif opt == "-v":
                verbosity = 1
            elif opt == "-V" or opt == "--verbosity":
                verbosity = int (arg)
            elif opt == "-M" or opt == "--maxdepth":
                max_depth = int (arg)
            elif opt == "-N" or opt == "--doc-name":
                doc_name = arg
                if len(doc_name) > 26:
                    message(0, "Only the first 26 characters \"%s\" of the document name will be used.", arg[:26])
                    doc_name = arg[:26]
            elif opt == "--db-name":
                deprecated( "db-name", "doc-name" )
                doc_name = arg
            elif opt == "--pluckerdir" or opt == "-p":
                pluckerdir = arg
            elif opt == "--pluckerhome" or opt == "-P":
                pluckerhome = arg
            elif opt == "-s" or opt == "--extra-section":
                extra_sections.append (arg)
            elif opt == "--zlib-compression":
                zlib_compression = 'true'
            elif opt == "--doc-compression":
                zlib_compression = 'false'
            elif opt == "--compression" and arg == "doc":
                zlib_compression = 'false'
            elif opt == "--compression" and arg == "zlib":
                zlib_compression = 'true'
            elif opt == "--no-urlinfo":
                no_url_info = 'true'
            elif opt == "--stayondomain":
                stayondomain = 'true'
            elif opt == "--stayonhost":
                stayonhost = 'true'
            elif opt == "--staybelow":
                staybelow = arg
            elif opt == "--url-pattern":
                url_pattern = arg
            elif opt == "--category":
                category = arg
            elif opt == "--maxheight":
                maxheight = arg
            elif opt == "--maxwidth":
                maxwidth = arg
            elif opt == "--alt-maxheight":
                alt_maxheight = arg
            elif opt == "--alt-maxwidth":
                alt_maxwidth = arg
            elif opt == "--launchable":
                launchable = 1
            elif opt == "--not-launchable":
                launchable = 0
            elif opt == "--backup":
                backup = 1
            elif opt == "--no-backup":
                backup = 0
            elif opt == "--beamable":
                copy_protect = 0
            elif opt == "--not-beamable":
                copy_protect = 1
            elif opt == "--icon":
                iconfile = arg
            elif opt == "--charset":
                default_charset = arg
            elif opt == "--owner-id":
                owner_id = arg
            elif opt == "--referrer":
                referrer = arg
            elif opt == "--user-agent":
                user_agent = arg
            elif opt == "--title":
                title = arg
            elif opt == "--author":
                author = arg
            elif opt == "--status-file":
                statusfile = arg
            elif opt == "--depth-first":
                depthfirst = 1
            elif opt == "--tables":
                tables = 1
            elif opt == "--http-proxy":
                http_proxy = arg
            elif opt == "--http-proxy-user":
                http_proxy_user = arg
            elif opt == "--http-proxy-pass":
                http_proxy_pass = arg
            elif opt == "--creator-id":
                creator_id = arg
                if len(creator_id) > 4:
                    message(0, "Only the first 4 characters \"%s\" of the creator ID will be used.", arg[:4])
                    creator_id = arg[:4]    
            elif opt == "--filter":
                filter = arg
            elif opt == "--bookmarks":
                if arg == "both":
                    bookmarks        = 'true'
                    bookmark_pages   = 'true'
                elif arg == "links":
                    bookmarks        = 'true'
                    bookmark_pages   = 'false'
                elif arg == "pages":
                    bookmarks        = 'false'
                    bookmark_pages   = 'true'
                else:
                    bookmarks        = 'false'
                    bookmark_pages   = 'false'
            else:
                usage ("Error:  Unknown option '%s'" % opt)
    except getopt.error, text:
        usage (str(text))

        
    config = ConfigFiles.Configuration (pluckerhome,
                                        pluckerdir,
                                        extra_sections=extra_sections,
                                        error_logger=error)

    if '_jython' in sys.builtin_module_names:
        config.set('jython', 1)
        
    if http_proxy is None:
        http_proxy = config.get_string ( 'http_proxy' )
    if http_proxy is not None:
        os.environ['HTTP_PROXY'] = http_proxy
    if http_proxy_user is None:
        http_proxy_user = config.get_string ( 'http_proxy_user' )
    if http_proxy_user is not None:
        os.environ['HTTP_PROXY_USER'] = http_proxy_user
    if http_proxy_pass is None:
        http_proxy_pass = config.get_string ( 'http_proxy_pass' )
    if http_proxy_pass is not None:
        os.environ['HTTP_PROXY_PASS'] = http_proxy_pass

    if pluckerdir is None:
        pluckerdir = config.get_string ('pluckerdir')
        if pluckerdir is None:
            # also not in the config, so we default to plucker home
            pluckerdir = pluckerhome
            config.set ('pluckerdir', pluckerdir)

    if use_file is None and use_cache is None:
        if config.get_string ('db_file') is not None and config.get_bool ('use_cache'):
            usage ("Config files specify both a 'db_file' and a 'use_cache=1'.\nYou must decide by specifiying an argument!")
        if config.get_string ('doc_file') is not None and config.get_bool ('use_cache'):
            usage ("Config files specify both a 'doc_file' and a 'use_cache=1'.\nYou must decide by specifiying an argument!")
        if config.get_string ('db_file') is None and config.get_string ('doc_file') is None:
            if config.get_string ('use_cache') is None:
                if config.get_int('jython', 0):
                    usage("Output to stdout not allowed in Java version.")
                elif sys.stdout.isatty():
                    usage("No output filename specified, and stdout is a terminal!")
                else:
                    use_file = '<stdout>'
                    verbosity = 0
        else:
            config.set ('use_cache', 0)

    if use_file and use_cache:
        usage ("You must not specify both -f and -c!")

    if doc_name and use_cache:
        usage ("Specify -N/--doc-name only with -f!")

    if use_file and outputstream:
        usage ("Don't specify a --doc_file when passing an outputstream!")

    if outputstream:
        usage ("Output streams not yet supported")

    if verbosity is None:
        verbosity = config.get_int('verbosity', 1)
    PyPlucker.UtilFns.set_verbosity(verbosity)
    config.set ('verbosity', verbosity)

    if zlib_compression is None:
        if config.get_string ('compression') == "doc":
            zlib_compression = 'false'
        elif config.get_string ('compression') == "zlib":
            zlib_compression = 'true'
    if owner_id:
        if zlib_compression == 'false':
            message('Specification of an owner-id forces use of zlib compression...')
        zlib_compression = 'true'
        
    mibenum = None
    # if not specified on command line, look in .pluckerrc
    if default_charset is None:
        default_charset = config.get_string("default_charset")
    # if we have one, validate it
    if default_charset is not None:
        from PyPlucker.helper.CharsetMapping import charset_name_to_mibenum, charset_known_names
        import string, re
        mibenum = charset_name_to_mibenum(default_charset)
        if mibenum:
            config.set('default_charset', mibenum)
        else:
            usage ("Error:  Unsupported charset '" + default_charset + "' specified as default charset.\n"
                   "        Charset must be either a decimal MIBenum value, or one of " + str(charset_known_names()))

    # update the config with the user options
    if use_file is not None:
        config.set ('doc_file', use_file)
        config.set ('use_cache', 0)
    if doc_name is not None:
        config.set ('doc_name', doc_name)
    if use_cache is not None:
        config.set ('use_cache', 1)
    if use_file:
        config.set ('use_cache', 0)
    if category is not None:
        import string
        category_count = string.count (category, ";")
        if category_count < 16:
            config.set ('category', category)
        else:
            usage ("Max number of categories is 16!")
        

    if bpp is not None:
        config.set ('bpp', bpp)
    if max_depth is not None:
        config.set ('home_maxdepth', max_depth)
    if home_url is not None:
        config.set ('home_url', home_url)
    if zlib_compression:
        config.set ('zlib_compression', zlib_compression)
    if no_url_info:
        config.set ('no_urlinfo', no_url_info)
    if seamless_fragments:
        config.set ('seamless_fragments', seamless_fragments)
    if link_fragments:
        config.set ('link_fragments', link_fragments)
    if stayonhost:
        config.set ('home_stayonhost', stayonhost)
    if stayondomain:
        if stayonhost:
            message("Warning: --stayonhost and --stayondomain both set.  Ignoring --stayondomain.")
        else:
            config.set ('home_stayondomain', stayondomain)
    if staybelow:
        config.set ('home_staybelow', staybelow)
    if url_pattern:
        config.set ('home_url_pattern', url_pattern)
    if maxheight is not None:
        config.set ('maxheight', maxheight)
    if maxwidth is not None:
        config.set ('maxwidth', maxwidth)
    if alt_maxheight is not None:
        config.set ('alt_maxheight', alt_maxheight)
    if alt_maxwidth is not None:
        config.set ('alt_maxwidth', alt_maxwidth)
    if launchable == 1:
        config.set ('launchable_bit', 1)
        config.set ('icon', 1)
    elif launchable == 0:
        config.set ('launchable_bit', 0)
    if backup == 1:
        config.set ('backup_bit', 1)
    elif backup == 0:
        config.set ('backup_bit', 0)
    if copy_protect == 1:
        config.set ('copyprevention_bit', 1)
    elif copy_protect == 0:
        config.set ('copyprevention_bit', 0)
    if iconfile is not None:
        config.set ('icon', 1)
        config.set ('big_icon', iconfile)
    if owner_id is not None:
        config.set ('owner_id_build', owner_id)
    if referrer is not None:
        config.set ('referrer', referrer)
    if user_agent is not None:
        config.set ('user_agent', user_agent)
    if author is not None:
        config.set ('author_md', author)
    if title is not None:
        config.set ('title_md', title)
    if mibenum is not None:
        config.set ('default_charset', mibenum)
    if statusfile is not None:
        config.set ('status_file', statusfile)
    if depthfirst is not None:
        config.set ('depth_first', 1)
    if tables is not None:
        config.set ('tables', 1)
    if creator_id is None:
        creator_id = config.get_string ( 'creator_id', 'Plkr' )
    if creator_id is not None:
        config.set ('creator_id', creator_id)
    if filter is not None:
        config.set ('filter', filter)
    if bookmarks is not None:
        config.set ('bookmarks', bookmarks)
    if bookmark_pages is not None:
        config.set ('bookmark_pages', bookmark_pages)

    for i in range (len (exclusion_lists)):
        exclusion_lists[i] = os.path.join (pluckerdir, exclusion_lists[i])

    execute_commands ("before_command", config)
    
    retval = main (config, exclusion_lists)

    execute_commands ("after_command", config)

    if tempfile.tempdir is not None and tempfile.template is not None:
        """ Clean up any temp files """
        base = re.compile (tempfile.template)
        for file in os.listdir (tempfile.tempdir):
            if base.match (file):
                os.remove(os.path.join(tempfile.tempdir, file))

    return retval


if __name__ == '__main__':
    sys.exit(realmain(None))
