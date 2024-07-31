#!/usr/bin/env python

"""
ConversionParser.py   $Id: ConversionParser.py,v 1.4 2003/02/19 14:00:46 chrish Exp $

Copyright 2003 Bill Nalen <bill.nalen@towers.com>

Distributable under the GNU General Public License Version 2 or newer.

Provides methods to wrap external convertors to return PluckerTextDocuments
"""

import os, sys, string, tempfile
from PyPlucker import TextParser
from UtilFns import message, error

def WordParser (url, data, headers, config, attributes):
    """Convert a Word document to HTML and returns a PluckerTextDocument"""

    # retrieve config information
    worddoc_converter = config.get_string('worddoc_converter')
    if worddoc_converter is None:
        message(0, "Could not find Word conversion command")
        return None

    check = os.path.basename (worddoc_converter)
    (check, ext) = os.path.splitext (check)
    check = string.lower (check)

    if check == 'wvware':
        # need to save data to a local file
        tempbase = tempfile.mktemp()
        tempdoc = os.path.join(tempfile.tempdir, tempbase + ".doc")
        try:
            file = open (tempdoc, "wb")
            file.write (data)
            file.close ()
        except IOError, text:
            message(0, "Error saving temporary file %s" % tempdoc)
            return None

        # then convert it > local.html
        temphtml = os.path.join(tempfile.tempdir, tempbase + ".html")
        command = worddoc_converter
        command = command + " -d " + tempfile.tempdir + " -b " + os.path.join(tempfile.tempdir, tempbase)
        command = command + " " + tempdoc + " > " + temphtml
        try:
            if os.system (command):
                message(0, "Error running Word converter %s" % command)
                return None
        except:
            message(0, "Exception running word converter %s" % command)
            return None

        # then load the local.html file to data2
        try:
            file = open (temphtml, "rb")
            data2 = file.read ()
            file.close ()
        except IOError, text:
            message(0, "Error reading temporary file %s" % temphtml)
            return None

        # then create a structuredhtmlparser from data2
        parser = TextParser.StructuredHTMLParser (url, data2, headers, config, attributes)

        return parser.get_plucker_doc ()

    else:
        return None

