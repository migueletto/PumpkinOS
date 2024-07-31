#!/usr/bin/env python
#  -*- mode: python; indent-tabs-mode: nil; -*-

"""
Writer.py   $Id: Writer.py,v 1.39 2004/04/08 00:57:34 chrish Exp $

Write a collection of documents into various formats.

Currently implemented: the traditional Plucker cache format.


Copyright 1999,2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""


import os, struct, string, time, helper.PQAAppInfo, sys, urllib
import PyPlucker
from PyPlucker import Url, PluckerDocs
from PyPlucker.helper import prc, dict
from PyPlucker.helper.CharsetMapping import charset_mibenum_to_name
from PyPlucker.UtilFns import message, error
from PyPlucker.Url import CompareURL


class Mapper:

     """This class handles all the mappings from URLs and PluckerDocs.PluckerDocument instances
     to record-IDs.  It contains a method "get_docs" which returns all the PluckerDocument
     instances it knows about; it contains a method "build_links" which returns a sequence of
     all the URLs in record-ID order, with zero-length URLs for unused record-IDs.  It
     contains a method "print_mapping" which sends a display of the mapping to stderr.
     Finally, it contains a method "get_or_add", which takes either a URL or a PluckerDocument
     instance, and returns its record-ID."""

     def __init__ (self, collection, alias_list):

        # maintains a mapping of URLs to PluckerDocs.PluckerDocument instances.
        # Keys are either a string URL, in which case the value is just a single instance,
        # or a (url, fragment-id) pair, in which case the value is a (doc-instance, paragraph-number) pair.

        self._url_to_doc_mapping = {}
        temp_list = []
        for (key, doc) in collection.items():
            url = string.split(key, '\0')[0]
            self._url_to_doc_mapping[url] = doc
            # record internal URL name, as well
            if isinstance(doc, PluckerDocs.PluckerDocument):
                self._url_to_doc_mapping[doc.get_url()] = doc
                # has sub-docs?  If so, get them and record them
                subdocs = doc.get_documents()
                if len(subdocs) > 1:
                    # first subdoc is always the main doc, so we skip that
                    for subdoc in subdocs[1:]:
                        self._url_to_doc_mapping[subdoc.get_url()] = subdoc
            # check for internal fragment names in the page
            name_mapping = isinstance(doc, PluckerDocs.PluckerTextDocument) and doc.get_name_map()
            if name_mapping:
                for (name, (internalurl, paragraph_number)) in name_mapping.items():
                    internalurl = alias_list.get(internalurl, internalurl)
                    temp_list.append((key, url, name, internalurl, paragraph_number,))
        for (key, url, name, internalurl, paragraph_number) in temp_list:
            doc = self._url_to_doc_mapping.get(internalurl)
            if not doc:
                sys.stderr.write("***** Can't find doc for URL " + str((key, url, internalurl, paragraph_number,)) + '\n')
            else:
                self._url_to_doc_mapping[(url, name)] = (doc, paragraph_number)

        # a list of URL->URL mappings
        self._alias_list = alias_list or {}

        # Maps a PluckerDocs.PluckerDocument instance to a record ID.
        # Keys are either a doc, or a tuple containing (doc, para#).
        self._doc_to_id_mapping = {}

        # Maps straight URLs without documents to a record ID.
        # Used mainly for external links.
        self._url_to_id_mapping = {}

        # first record ID issued.  Records 1-10 are reserved.
        self._current_id = 11

        # make sure record number 2 goes to the 'home' document (why?)
        url = self._alias_list.get('plucker:/home.html')
        if url:
            while self._alias_list.has_key(url):
                url = self._alias_list.get(url)
            doc = self._url_to_doc_mapping.get(url)
            if doc:
                self._doc_to_id_mapping[doc] = 2
            else:
                self._url_to_id_mapping[url] = 2
        else:
            doc = self._url_to_doc_mapping.get('plucker:/home.html')
            if doc:
                self._doc_to_id_mapping[doc] = 2
            else:
                self._url_to_id_mapping[url] = 2
        if doc:
            # note that the first part is already done
            parts = doc.get_documents()[1:]
            for subdoc in parts:
                self._get_id_for_doc(doc)

        # finally, make sure each doc has an ID assigned
        sorted_list=collection.items()
        sorted_list.sort(lambda x, y: CompareURL(x[0],y[0]))

        for (url, doc) in sorted_list:
            parts = doc.get_documents()
            for subdoc in parts:
                self._get_id_for_doc(subdoc)
        del sorted_list

     def _get_id_for_doc(self, idoc, add=1):
         if type(idoc) == type(()):
             doc = idoc[0]
         else:
             doc = idoc
         id = self._doc_to_id_mapping.get(doc)
         if not id:
             id = self._url_to_id_mapping.get(doc.get_url())
             if id:
                 self._doc_to_id_mapping[doc] = id
         if not id:
             if not add:
                 return None
             if isinstance(doc, PluckerDocs.PluckerIndexDocument):
                 # there's only one, and it always has record # 1
                 id = 1
             elif isinstance(doc, PluckerDocs.PluckerBookmarkDocument):
                 id = 6
             elif isinstance(doc, PluckerDocs.PluckerLinkIndexDocument):
                 id = 3
             elif isinstance(doc, PluckerDocs.PluckerCategoryDocument):
                 id = 4
             elif isinstance(doc, PluckerDocs.PluckerMetadataDocument):
                 id = 5
             else:
                 id = self._current_id
                 self._current_id = self._current_id + 1
             self._doc_to_id_mapping[doc] = id
             url_mapping = self._url_to_doc_mapping.get(doc.get_url())
             if (url_mapping != doc):
                 if (url_mapping != None):
                     message("URL %s for doc %s points to doc %s\n" %
                             (doc.get_url(), str(doc), str(url_mapping)))
                 self._url_to_doc_mapping[doc.get_url()] = doc           
             # message("new document " + str(doc) + " => " + str(id) + "\n")
         if type(idoc) == type(()):
             return (id, idoc[1])
         else:
             return id


     def _get_id_for_url (self, url, add=1):
         doc = self._url_to_doc_mapping.get(url)
         id = doc and self._get_id_for_doc(doc, add)
         id = id or self._url_to_id_mapping.get(url)
         if not id:
             # possibly valid main part, but invalid tag.  Return ID for main part in that case.
             if type(url) == type(()):
                 id = self._get_id_for_url(url[0], 0)
             if not id and add:
                 # OK, no ID, but we should assign one
                 id = self._current_id
                 self._current_id = self._current_id + 1
                 self._url_to_id_mapping[url] = id
                 # message("** Gave ID %s to url %s\n" % (id, url))
         return id


     def get_or_add (self, url_or_doc):
        # For a standard URL, returns the numeric record ID.
        # For a URL which has a fragment-id:
        #   If the fragment is a paragraph of a text page, a pair
        #   (record-id, paragraph-id) is returned.
        #   Otherwise, just the record id is returned.
        # If arg is PluckerDocument, returns the id assigned for that document.
        # If arg is integer, treats it as a registered-document id.  Get-only.
        if type(url_or_doc) == type(''):
            import urllib
            url, tag = urllib.splittag(url_or_doc)
            finalurl = self._alias_list.get(url, url)
            if tag:
                id = self._get_id_for_url((finalurl, tag))
            else:
                id = self._get_id_for_url(finalurl)
            return id
        elif isinstance(url_or_doc, PluckerDocs.PluckerDocument):
            url = url_or_doc.get_url()
            if not self._url_to_doc_mapping.has_key(url):
                self._url_to_doc_mapping[url] = url_or_doc
            if not self._doc_to_id_mapping.has_key(url_or_doc) and self._url_to_id_mapping.has_key(url):
                self._doc_to_id_mapping[url_or_doc] = self._url_to_id_mapping[url]
            if not self._doc_to_id_mapping.has_key(url_or_doc):
                message(2, "New document %s added", url_or_doc)
            return self._get_id_for_doc(url_or_doc)
        else:
            raise ValueError("not a URL or an instance of " + str(PluckerDocs.PluckerDocument))


     def build_links (self):
         # build and return a list of the URL strings for all IDs used
         key_dict = self._url_to_doc_mapping.copy()
         key_dict.update(self._url_to_id_mapping)
         # build a list of all URLs and associated IDs
         for key in key_dict.keys():
             if type(key) == type('') and len(key) > 7 and key[:7] == 'mailto:':
                 del key_dict[key]
                 continue
             if type(key) == type(()):
                 # either resolved tag, in which case value is tuple,
                 # or unresolved tag, in which case value is integer
                 value = key_dict[key]
                 del key_dict[key]
                 if type(value) == type(()):
                     # truncate key to just plain record
                     key = key[0]
                     value = value[0]
                 else:
                     key = key[0] + '#' + key[1]
                 key_dict[key] = value
             if isinstance(key_dict[key], PluckerDocs.PluckerDocument):
                 key_dict[key] = self._get_id_for_doc(key_dict[key])
         # invert the dictionary
         for item in key_dict.items():
             del key_dict[item[0]]
             key_dict[item[1]] = item[0]
         # build up the list of URLs
         urls = []
         for i in range(self._current_id):
             urls.append(key_dict.get(i) or '')
         urls[1] = ''        # no URL needed for index record
         return urls


     def get_docs(self):
         # return a list of all the PluckerDocuments known to the mapper
         return self._doc_to_id_mapping.keys()


     def print_mapping(self):
         # print a list of all the URL's and associated IDs
         message(0, '*********\n')
         message(0, 'PluckerDoc record ids:')
         for (doc, id) in self._doc_to_id_mapping.items():
             #sys.stderr.write(str(doc) + '  ' + str(id) + '\n')
             if type(doc) == type(()):
                 url = doc[0].get_url()
                 message(0, '%70s => %3d (%s)\n' % (url, id, str(doc[1])))
             else:
                 url = doc.get_url()
                 message(0, '%70s => %3d\n' % (url, id))
         if len(self._url_to_id_mapping) > 0:
             message(0, 'Non-included URL record ids:')
             for (url, id) in self._url_to_id_mapping.items():
                 message(0, '%70s => %3d\n' % (url, id))
         message(0, '*********\n')



class Writer:
    """Abstract base class from which to derive the various writers
    for documents"""

    def __init__ (self, collection, config, urlmapper=None):
        self._collection = collection
        self._config = config
        self._mapper = urlmapper


    def save_data (self, data, url, id, verbose):
        """This needs to be implemented in the derived class to
        actually output the 'data' (human readably denoted as
        'url') as something with id 'id'."""
        raise NotImplementedError("PyPlucker.Writer.Writer.save_doc()")


    def _write_doc (self, out_dict, pluckerdoc, url, id, verbose):

        def _print_convert_msg (url, id, verbose, config):
            if verbose > 1:
                line_length = config.get_int('status_line_length', 60)
                urltext = str (url)
                if len (urltext) > line_length:
                    urltext = urltext[:line_length - 20] + "....." + urltext[-15:]
                message("Converted %4d:  %s" % (id, urltext))

        if id != self._mapper.get_or_add(pluckerdoc):
            raise ValueError("bad id %d instead of %d" % (id, self._mapper.get_or_add(pluckerdoc)))
        if pluckerdoc.is_text_document ():
            dumps = pluckerdoc.dump_record_with_splits (self._mapper)
            # sys.stderr.write("dumps is %s\n" % str(map(lambda p: (p[0], p[1]), dumps)))
            if dumps[0][1] != id:
                message("****** bad id %d instead of %d" % (dumps[0][1], id,))
            for dump in dumps:
                (the_url, the_id, dump) = dump
                if the_id == 0:
                    the_id = id # original
                out_dict [the_id] = (dump, the_url, the_id, verbose)
                _print_convert_msg(the_url, the_id, verbose, self._config)
            return
        else:
            dump = pluckerdoc.dump_record (id)
            out_dict [id] = (dump, url, id, verbose)
            _print_convert_msg(url, id, verbose, self._config)

    
    def write (self, verbose, alias_list=None):
        """Write out the collection.  Returns the mapping that was
        used to generate the ids."""

        def _print_convert_msg (url, verbose, config):
            if verbose > 0:
                line_length = config.get_int('status_line_length', 60)
                urltext = str (url)
                if len (urltext) > line_length:
                    urltext = urltext[:line_length - 20] + "....." + urltext[-15:]
                message("Converting %s..." % urltext)

        self._mapper = Mapper(self._collection, alias_list.as_dict())

        # figure default charset
        mibenum = self._config.get_int('default_charset', 0) or None
        charsets = {}

        if verbose > 2:
            self._mapper.print_mapping()

        out_dict = {}
        bookmarks = {}
        for pluckerdoc in self._mapper.get_docs():
            id = self._mapper.get_or_add(pluckerdoc)
            _print_convert_msg(pluckerdoc.get_url(), verbose, self._config)
            if pluckerdoc.is_multiimage_document ():
                pluckerdoc.resolve_ids (self._mapper)
            if pluckerdoc.is_table_document ():
                pluckerdoc.resolve_ids (self._mapper)
            if pluckerdoc.is_text_document ():
                pluckerdoc.resolve_ids (self._mapper)
                doc_mibenum = pluckerdoc.get_charset()
                if verbose > 2:
                    charset_name = charset_mibenum_to_name(doc_mibenum)
                    message(2, pluckerdoc.get_url() + ' has charset ' + str(doc_mibenum) + ((charset_name and " (" + charset_name + ")") or "") + "\n")
                if charsets.has_key(doc_mibenum):
                    charsets[doc_mibenum].append(id)
                else:
                    charsets[doc_mibenum] = [id]

                # Add doc.bookmarks to bookmark list
                if self._config and self._config.get_bool('bookmark_pages', 0):
                    key = pluckerdoc.get_url()
                    pid = self._mapper.get_or_add(key)
                    key = string.split(key, ":")
                    key = key[-1]
                    key = string.split(key, "/")
                    key = key[-1]
                    key = string.split(key, "?")
                    key = key[0]
                    if not len(key):
                        key = 'Home Page'
                    if not bookmarks.has_key(key):
                        bookmarks[key] = (pid, 0)

                if self._config and self._config.get_bool('bookmarks', 0):
                    tmp_book = pluckerdoc.get_bookmark_ids()
                    for key in tmp_book.keys():
                        if not bookmarks.has_key(key):
                            bookmarks[key] = tmp_book[key]

            self._write_doc (out_dict, pluckerdoc, pluckerdoc.get_url(), id, verbose)

        ## Do some error checking
        if not out_dict.has_key (2):
            raise RuntimeError("The collection process failed to generate a 'home' document")
        
        ## set up the metadata mapping, if any
        metadata = {}
        # set the default to the charset which has the 'most' pages
        items = charsets.items()
        if len(items) > 0:        # have to allow for image-only document
            items.sort(lambda x, y: ((len(x[1]) < len(y[1]) and 1) or ((len(x[1]) > len(y[1])) and -1) or 0))
            mibenum = items[0][0]
            odd_charsets = []
            if len(items) > 1:
                for item in items[1:]:
                    for id in item[1]:
                        odd_charsets.append((id, item[0] or 0,))
        else:
            mibenum = None
            odd_charsets = []
        if mibenum != None:
            metadata['CharSet'] = mibenum
            if verbose > 1:
                charset_name = charset_mibenum_to_name(mibenum)
                message('Default charset is MIBenum ' + str(mibenum) + ((charset_name and " (" + charset_name + ")") or ""))
        else:
            message('No default charset')
        if len(odd_charsets) > 0:
            metadata['ExceptionalCharSets'] = odd_charsets
            message("ExceptionalCharSets is " + str(odd_charsets) + "\n")
        intended_owner = self._config.get_string('owner_id_build')
        if intended_owner:
            metadata['OwnerID'] = intended_owner
            message(2, "OwnerID is '%s'", intended_owner)
        author = self._config.get_string('author_md')
        if author:
            metadata['Author'] = author
            message(2, "Author is '%s'", author)
        title = self._config.get_string('title_md')
        if title:
            metadata['Title'] = title
            message(2, "Title is '%s'", title)

        ## write the index record
        tmp_url = "plucker:/~special~/index"
        type = PluckerDocs.PluckerIndexDocument (tmp_url, self._config, metadata, bookmarks)
        self._write_doc (out_dict, type, tmp_url, 1, verbose)

        ## write the bookmark record (if any)
        if len(bookmarks):
            tmp_url = "plucker:/~special~/bookmarks"
            bookdoc = PluckerDocs.PluckerBookmarkDocument(tmp_url, bookmarks)
            self._write_doc (out_dict, bookdoc, tmp_url, 6, verbose)

        ## write the URL information, if desired
        if not self._config.get_bool ('no_urlinfo', 0):
            links = self._mapper.build_links()
            # for i in range(len(links)):
            #   message(0, "%3d: '%s'", i, links[i])
            linksdocs = []
            for i in range(1, len(links), 200):
                tmp_url = "plucker:/~special~/links" + str(i)
                linksdoc = PluckerDocs.PluckerLinksDocument(tmp_url, links, i)
                self._mapper.get_or_add(linksdoc)
                linksdocs.append(linksdoc)
            # now make links index
            tmp_url = "plucker:/~special~/pluckerlinks"
            indexdoc = PluckerDocs.PluckerLinkIndexDocument(tmp_url, linksdocs, self._mapper)
            self._mapper.get_or_add(indexdoc)
            # OK, write the links index document
            self._write_doc (out_dict, indexdoc, tmp_url, 3, verbose)
            # and write the various links documents
            for doc in linksdocs:
                self._write_doc (out_dict, doc, doc.get_url(), self._mapper.get_or_add(doc), verbose)

        ## write the category information, if present
        if self._config.get_string ('category') is not None:
            tmp_url = "plucker:/~special~/category"
            type = PluckerDocs.PluckerCategoryDocument (tmp_url, self._config)
            self._write_doc (out_dict, type, tmp_url, 4, verbose)

        ## write the metadata record, if any
        if metadata:
            tmp_url = "plucker:/~special~/metadata"
            type = PluckerDocs.PluckerMetadataDocument (tmp_url, metadata)
            self._write_doc (out_dict, type, tmp_url, 5, verbose)

        ## now write everything else
        the_ids = out_dict.keys ()
        the_ids.sort ()  # they are numeric, so sort does the right thing
        for id in the_ids:
            dump, the_url, the_id, verbose = out_dict[id]
            self.save_data (dump, the_url, the_id, verbose)
            if verbose:
                line_length = self._config.get_int('status_line_length', 60)
                urltext = str (the_url)
                if len (urltext) > line_length:
                    urltext = urltext[:line_length - 20] + "....." + urltext[-15:]
                message("Wrote %d <= %s" % (the_id, urltext))

        return self._mapper



class CacheWriter (Writer):
    """A Writer that writes the traditional format of a separate files
    in a cache directory"""

    def __init__ (self, collection, config, cachedir):
        Writer.__init__ (self, collection, config)
        self._cachedir = cachedir


    def write (self, verbose, alias_list):
        cachedir = os.path.expandvars (self._cachedir)
        cachedir = os.path.expanduser (cachedir)
        if not os.path.exists (cachedir):
            error("%s does not exists!" % cachedir)
            return
        if not os.path.isdir (cachedir):
            error("%s is not a directory" % cachedir)
            return

        # clear the cache directory
        for name in os.listdir (cachedir):
            fname = os.path.join (cachedir, name)
            if os.path.isfile (fname):
                os.unlink (fname)

        # Now call the super class to do the actual work
        return Writer.write (self, verbose, alias_list=alias_list)
        

    def save_data (self, data, url, id, verbose):
        filename = os.path.join (self._cachedir, "%d" % id)
        file = open (filename, "wb")
        file.write (data)
        file.close ()


class PDBWriter (Writer):
    """A Writer that writes the items into a ready-to-synch PDB
    file."""

    def __init__ (self, collection, config, name, version, filename):
        Writer.__init__ (self, collection, config)
        self._filename = filename
        self._dbname = name
        self._dbversion = version
        self._pdb_file = None
        self._flag_copy_prevention = config.get_bool ('copyprevention_bit')
        self._flag_launchableData = config.get_bool ('launchable_bit')
        self._flag_backup = config.get_bool ('backup_bit')
        self._icon = config.get_bool ('icon') or config.get_bool('launchable_bit')
        self._big_icon = config.get_string ('big_icon','')
        self._small_icon = config.get_string ('small_icon','')
        self._config = config
        self._creator_id = config.get_string('creator_id', 'Plkr')


    def write (self, verbose, alias_list, mapping=None):
        if os.path.exists (self._filename):
            os.unlink (self._filename)
        if self._filename == '<stdout>':
            if sys.platform == "win32":
                import msvcrt
                msvcrt.setmode(sys.stdout.fileno(), os.O_BINARY)
            self._pdb_file = prc.File (sys.stdout, read=0, write=1)
        else:
            self._pdb_file = prc.File (self._filename, read=0, write=1)
        info = self._pdb_file.getDBInfo ()
        info['name'] = self._dbname
        info['version'] = self._dbversion
        info['creator'] = self._creator_id
        #info['creator'] = 'Plkr'
        info['type'] = 'Data'
        info['createDate'] = int (time.time())
        info['modifyDate'] = info['createDate']
        info['backupDate'] = -2082844800L
        info['flagCopyPrevention'] = self._flag_copy_prevention
        info['flagLaunchableData'] = self._flag_launchableData
        info['flagBackup'] = self._flag_backup
        if self._icon:
            self._pdb_file.setAppBlock( \
                helper.PQAAppInfo.pqa_app_info_block(self._config, \
                                                     self._dbname, \
                                                     self._dbversion, \
                                                     self._big_icon, \
                                                     self._small_icon))
        self._pdb_file.setDBInfo (info)

        # Now call the super class to do the actual work
        result = Writer.write (self, verbose, alias_list=alias_list)

        self._pdb_file.close ()
        return result
        

    def save_data (self, data, url, id, verbose):
        assert self._pdb_file is not None, "write_doc called with unintialized pdb file"

        self._pdb_file.setRecord (attr=0, id=id, cat=0, data=data)




class DictWriter (Writer):
    """A Writer that writes each record into a passed dictionary with
    the record number as the key"""

    def __init__ (self, collection, config, dict):
        Writer.__init__ (self, collection, config)
        self._dict = dict


    def save_data (self, data, url, id, verbose):
        self._dict[id] = data
