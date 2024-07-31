#!/usr/bin/env python
#  -*- mode: python; indent-tabs-mode: nil; -*- coding: iso-8859-1 -*-

"""

TextParser.py   $Id: TextParser.py,v 1.66 2004/04/13 03:37:16 jimj Exp $


Copyright 1999,2000 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""

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

import string
import re
try:
    # if the user has the new xml package installed this might be faster
    # as that package includes one that uses C code to parse
    from xml.parsers import sgmllib
except ImportError:
    import sgmllib
    # Fix for the missing comma in raw attributes
    import re, string
    
    #
    #  Originally, plucker overrode this to catch the "@" of 
    #  mailto links.  This bugfix is already in newer pythons.
    #  If you have a new enough version (the sgmllib distributed
    #  with Python 2.3.3), then we should use the (newer) python
    #  version.  This checks whether or not attrfind knows that 
    #  @ can appear in unquoted attribute values. If so, use the
    #  python version.
    #
    temp=sgmllib.attrfind.match('href=mailto:user@host').groups()[2]
    if temp is None or '@' not in temp:
        sgmllib.attrfind = re.compile(
            r'\s*([a-zA-Z_][-:.a-zA-Z_0-9]*)(\s*=\s*'
            r'(\'[^\']*\'|"[^"]*"|[-a-zA-Z0-9./,:;+*%?!&$\(\)_#=~\'"@]*))?')


import sys
import struct
import urllib
import htmlentitydefs
from PyPlucker import PluckerDocs
from PyPlucker import Url
from PyPlucker import DEFAULT_LOCALE_CHARSET_ENCODING
from PyPlucker.helper.CharsetMapping import charset_name_to_mibenum
from PyPlucker import UtilFns

message = UtilFns.message

# the following constant states how big (approximately) one single
# paragraphs should maximally be
Max_Paragraph_Size = 1000
# how much more to allow in order not to break an anchor
Max_Paragraph_Size_Anchor_Stretch = 150


## The following are used in the parser to clean up things.
_RE_WHITESPACE = re.compile ("[\n\f \t]+")
_RE_NONSPACEWHITESPACE = re.compile ("[\n\f\t\t]+")
_CLEANUP_TRANSTABLE = string.maketrans ("\f", "\n")

##
## There are several colorsets in use on the web.
## 
## Plucker has traditionally used the HTML4 color names
## (Colornames_Strict). 
## 
## CSS2.1 <URL: http://www.w3.org/TR/CSS21/syndata.html#color-units> 
## adds orange ("FFA500") and several "colors" that depend on 
## current system settings.  It isn't clear what to do with 
## system colors, except maybe defaulting to black.
##
## SVG adds several other colors (from the X windows color 
## set, plus some grays).
##
## CSS3 <URL: http://www.w3.org/TR/2003/CR-css3-color-20030514 > 
## color names represent the future of standards, but splits
## color names into several different profiles.  Full support 
## is more complicated than Plucker currently supports.
##
## Some web sites use Crayola colors.
##
Colornames_Strict = {
        'black':   "000000",
        'silver':  "C0C0C0",
        'gray':    "808080",
        'white':   "FFFFFF",
        'maroon':  "800000",
        'red':     "FF0000",
        'purple':  "800080",
        'fuchsia': "FF00FF",
        'green':   "008000",
        'lime':    "00FF00",
        'olive':   "808000",
        'yellow':  "FFFF00",
        'navy':    "000080",
        'blue':    "0000FF",
        'teal':    "008080",
        'aqua':    "00FFFF",
        }

Colornames_SVG = {
        'aliceblue':            'F0F8FF',
        'antiquewhite':         'FAEBD7',
        'aqua':                 '00FFFF',
        'aquamarine':           '7FFFD4',
        'azure':                'F0FFFF',
        'beige':                'F5F5DC',
        'bisque':               'FFE4C4',
        'black':                '000000',
        'blanchedalmond':       'FFEBCD',
        'blue':                 '0000FF',
        'blueviolet':           '8A2BE2',
        'brown':                'A52A2A',
        'burlywood':            'DEB887',
        'cadetblue':            '5F9EA0',
        'chartreuse':           '7FFF00',
        'chocolate':            'D2691E',
        'coral':                'FF7F50',
        'cornflowerblue':       '6495ED',
        'cornsilk':             'FFF8DC',
        'crimson':              'DC143C',
        'cyan':                 '00FFFF',
        'darkblue':             '00008B',
        'darkcyan':             '008B8B',
        'darkgoldenrod':        'B8860B',
        'darkgray':             'A9A9A9',
        'darkgreen':            '006400',
        'darkgrey':             'A9A9A9',
        'darkkhaki':            'BDB76B',
        'darkmagenta':          '8B008B',
        'darkolivegreen':       '556B2F',
        'darkorange':           'FF8C00',
        'darkorchid':           '9932CC',
        'darkred':              '8B0000',
        'darksalmon':           'E9967A',
        'darkseagreen':         '8FBC8F',
        'darkslateblue':        '483D8B',
        'darkslategray':        '2F4F4F',
        'darkslategrey':        '2F4F4F',
        'darkturquoise':        '00CED1',
        'darkviolet':           '9400D3',
        'deeppink':             'FF1493',
        'deepskyblue':          '00BFFF',
        'dimgray':              '696969',
        'dimgrey':              '696969',
        'dodgerblue':           '1E90FF',
        'firebrick':            'B22222',
        'floralwhite':          'FFFAF0',
        'forestgreen':          '228B22',
        'fuchsia':              'FF00FF',
        'gainsboro':            'DCDCDC',
        'ghostwhite':           'F8F8FF',
        'gold':                 'FFD700',
        'goldenrod':            'DAA520',
        'gray':                 '808080',
        'green':                '008000',
        'greenyellow':          'ADFF2F',
        'grey':                 '808080',
        'honeydew':             'F0FFF0',
        'hotpink':              'FF69B4',
        'indianred':            'CD5C5C',
        'indigo':               '4B0082',
        'ivory':                'FFFFF0',
        'khaki':                'F0E68C',
        'lavender':             'E6E6FA',
        'lavenderblush':        'FFF0F5',
        'lawngreen':            '7CFC00',
        'lemonchiffon':         'FFFACD',
        'lightblue':            'ADD8E6',
        'lightcoral':           'F08080',
        'lightcyan':            'E0FFFF',
        'lightgoldenrodyellow': 'FAFAD2',
        'lightgray':            'D3D3D3',
        'lightgreen':           '90EE90',
        'lightgrey':            'D3D3D3',
        'lightpink':            'FFB6C1',
        'lightsalmon':          'FFA07A',
        'lightseagreen':        '20B2AA',
        'lightskyblue':         '87CEFA',
        'lightslategray':       '778899',
        'lightslategrey':       '778899',
        'lightsteelblue':       'B0C4DE',
        'lightyellow':          'FFFFE0',
        'lime':                 '00FF00',
        'limegreen':            '32CD32',
        'linen':                'FAF0E6',
        'magenta':              'FF00FF',
        'maroon':               '800000',
        'mediumaquamarine':     '66CDAA',
        'mediumblue':           '0000CD',
        'mediumorchid':         'BA55D3',
        'mediumpurple':         '9370DB',
        'mediumseagreen':       '3CB371',
        'mediumslateblue':      '7B68EE',
        'mediumspringgreen':    '00FA9A',
        'mediumturquoise':      '48D1CC',
        'mediumvioletred':      'C71585',
        'midnightblue':         '191970',
        'mintcream':            'F5FFFA',
        'mistyrose':            'FFE4E1',
        'moccasin':             'FFE4B5',
        'navajowhite':          'FFDEAD',
        'navy':                 '000080',
        'oldlace':              'FDF5E6',
        'olive':                '808000',
        'olivedrab':            '6B8E23',
        'orange':               'FFA500',
        'orangered':            'FF4500',
        'orchid':               'DA70D6',
        'palegoldenrod':        'EEE8AA',
        'palegreen':            '98FB98',
        'paleturquoise':        'AFEEEE',
        'palevioletred':        'DB7093',
        'papayawhip':           'FFEFD5',
        'peachpuff':            'FFDAB9',
        'peru':                 'CD853F',
        'pink':                 'FFC0CB',
        'plum':                 'DDA0DD',
        'powderblue':           'B0E0E6',
        'purple':               '800080',
        'red':                  'FF0000',
        'rosybrown':            'BC8F8F',
        'royalblue':            '4169E1',
        'saddlebrown':          '8B4513',
        'salmon':               'FA8072',
        'sandybrown':           'F4A460',
        'seagreen':             '2E8B57',
        'seashell':             'FFF5EE',
        'sienna':               'A0522D',
        'silver':               'C0C0C0',
        'skyblue':              '87CEEB',
        'slateblue':            '6A5ACD',
        'slategray':            '708090',
        'slategrey':            '708090',
        'snow':                 'FFFAFA',
        'springgreen':          '00FF7F',
        'steelblue':            '4682B4',
        'tan':                  'D2B48C',
        'teal':                 '008080',
        'thistle':              'D8BFD8',
        'tomato':               'FF6347',
        'turquoise':            '40E0D0',
        'violet':               'EE82EE',
        'wheat':                'F5DEB3',
        'white':                'FFFFFF',
        'whitesmoke':           'F5F5F5',
        'yellow':               'FFFF00',
        'yellowgreen':          '9ACD32',
}

Colornames = Colornames_SVG

def _parse_color (value):
    """Get the RGB value.  
    
    Try text colorname (e.g. 'Silver'), 
    then try to lower-case that (e.g., 'silver')
    then try an RGB value (e.g. #C0C0C0),
    then try an RGB value missing the # (e.g. C0C0C0)
    then try RGB (with and without the #) with only one char/color.
    then default to black ("#000000").
    """
    
    try:
        return Colornames[value]
    except KeyError:
        pass    
    
    # This is redundant, if the page is coded entirely properly.
    try:
        return Colornames[value.lower()]
    except KeyError:
        pass    
    
    if value[0] == '#':
        value = value[1:]
    
    try:
        val = '%06x' % string.atoi(value, 16)
        return val.upper()
    except ValueError:
        pass
    
    try:
        val = '%03x' % string.atoi(value, 16)
        val = val.upper()
        val = ((val[0]) * 2) + ((val[1]) * 2) + ((val[2]) * 2)
        return val
    except ValueError:
        pass
    
    message (1, "Giving up on color %s, using black.", value)
    return "000000"


def _list_to_dict (alist):
    """Convert [(attr1,val), (attr2,val) ...] to {attr1:val, attr2:val}
    
    The sgml parser returns attributes as a list of key-value pairs. 
    This function puts them in a dictionary, for more easier use."""
    
    assert type(alist) == type([])
    result = {}
    for (key,val) in alist:
        # string.lower is done by Python's own parser already
        # but the XML package's version does not.
        result[string.lower (key)] = cleanup_attribute (val)
    return result



_entitycharref = re.compile('^(.*)&([#a-zA-Z][-.a-zA-Z0-9]*);(.*)$')
_html_char_ref_pattern = re.compile('^&#([0-9]+);$')

# These junk "alt" attribute values are not worth showing.
junk_alt_attributes = ("img", "[img]", "spacer", "")

def cleanup_attribute (text):
    m = _entitycharref.search (text)
    if not m:
        return text
    pre, content, post = m.groups (0)
    if content[0] == "#":
        content = content[1:]
        try:
            n = int(content)
            if 0 <= n <= 255:
                content = chr (n)
            else:
                #self._add_unicode_char(val, "&#%d;" % val)
                # Not in a "self", so can't add the unicode properly.
                content="&#%d" % n
        except ValueError:
            #content = "?"
            # might as well pass it through.  no worse than a "?"
            content = "&#" + content
    else:
        if htmlentitydefs.entitydefs.has_key (content):
            content = htmlentitydefs.entitydefs[content]
        else:
            # content = "?"
            # usually unescaped &amp -- AT&T should return AT&T.
            content = "&" + content
    return cleanup_attribute (pre) + content + post
    


def _clean_newlines (text):
    """Try to clean up newlines in source code to be UNIXy.
    We assume that CP/M derived OSes use \r\n as line terminator and
    MacOS uses only \r.  Both version are converted to use only \n.
    XXX We should probably use python's universal newlines now."""
    
    text = string.replace (text, "\r\n", "\n")
    text = string.replace (text, "\r", "\n")
    return text




class AttributeStack:
    """A data structure to maintain information about the current
    text attributes.

    The raw value for the plucker DB is the font to set (as stated by
    Michael Nordstrom on plucker-dev or found in os.c):
    | OS2
    | ---
    | 0: stdFont,   1: boldFont,      2: boldFont,      3: boldFont, 
    | 4: boldFont,  5: stdFont,       6: stdFont,       7: stdFont
    | 8: stdFont    9: stdFont
    | 
    | OS3
    | ---
    | 0: stdFont,   1: largeBoldFont, 2: largeBoldFont, 3: largeFont, 
    | 4: largeFont, 5: boldFont,      6: boldFont,      7: boldFont
    | 8: fixedWidthFont               9: stdFont
    The awk parser sets:
     <B>: 7
     <Hn>: n (for n= 1, 2, 3, 4, 5, 6)
    """
    def __init__ (self):
        self._tags = {
            "" : 0,
            "b": 7,
            "th": 7,        # we also make table heads configurable
            "h1": 1,
            "h2": 2,
            "h3": 3,
            "h4": 4,
            "h5": 5,
            "h6": 6,
            "tt" : 8,
            "pre": 8,
            "small": 9,
            "sub" : 10,
            "sup" : 11,
            }
        self._stack = [""]
        self._alignment = [0]
        self._left_margin = 0
        self._right_margin = 0
        self._italics_depth = 0
        self._underline_depth = 0
        self._strike_depth = 0
        self._forecolor = ["000000"]              # Default color for text.
        self._tableborder_forecolor = ["default"] # Default color for table borders
        
    def indent (self, change_left=0, change_right=0):
        self._left_margin = min (60, max (0, self._left_margin + change_left))
        self._right_margin = min (120, max (0, self._right_margin + change_right))

        return (self._left_margin, self._right_margin)


    def change_italics (self, increment):
        """Add 'increment' to the italics counter and return the new value"""
        self._italics_depth = self._italics_depth + increment
        return self._italics_depth


    def change_underline (self, increment):
        """Add 'increment' to the underline counter and return the new value"""
        self._underline_depth = self._underline_depth + increment
        return self._underline_depth


    def change_strike (self, increment):
        """Add 'increment' to the strike counter and return the new value"""
        self._strike_depth = self._strike_depth + increment
        return self._strike_depth

        
    def push_alignment (self, newvalue):
        """Push a new alignment value.  Return true if value has changed"""
        assert 0 <= newvalue <= 3, \
               "Alignment value must be >=0 and <=3 but is %d" % newvalue
        self._alignment.append (newvalue)
        return self._alignment[-1] != self._alignment[-2]

        
    def pop_alignment (self, value):
        """Pop some alignment value.  Return true if value has changed"""
        assert value == None or self._alignment[-1] == value, \
               "Trying to pop alignment %s but is %s" % (value, self._alignment[-1])
        res = self._alignment[-1] != self._alignment[-2]
        del self._alignment[-1]
        return res
        
        
    def get_alignment (self):
        """Return the current alignment."""
        assert self._alignment != [], "Alignment stack must not be empty"
        return self._alignment[-1]
        
        
    def push_forecolor (self, value):
        """Push a new forecolor value.  Return true if value has changed"""
        self._forecolor.append (value)
        return self._forecolor[-1] != self._forecolor[-2]

        
    def pop_forecolor (self, value):
        """Pop some forecolor value.  Return true if value has changed"""
        assert self._forecolor[-1] == value, \
               "Trying to pop forecolor %s but is %s" % (value, self._forecolor[-1])
        foreres = self._forecolor[-1] != self._forecolor[-2]
        del self._forecolor[-1]
        return foreres

        
    def get_forecolor (self):
        """Return the current forecolor."""
        assert self._forecolor != [], "Forecolor stack must not be empty"
        return self._forecolor[-1]     

        
    def push (self, tag):
        """Push the style for 'tag' onto the stack.  

        Return True if the style was actually changed by this."""
        
        tag = string.lower (tag)
        assert self._tags.has_key (tag), "Unknown style code %s" % tag
        self._stack.append (tag)
        assert len(self._stack) >= 2
        return self._tags[self._stack[-1]] != self._tags[self._stack[-2]]

    def pop (self, tag):
        """Pop a style from the stack and verify that it is 'tag'.  Return
        true if the style now changed (i.e. new top of stack is different from
        the popped item)."""
        tag = string.lower (tag)
        assert self._tags.has_key (tag), "Unknown style code %s" % tag
        assert len(self._stack) >= 2, "Trying to pop from empty stack"
        res = self._tags[self._stack[-1]] != self._tags[self._stack[-2]]
        top = self._stack[-1]
        assert top == tag, "Expected TOS not found"
        self._stack = self._stack[:-1]
        assert self._stack != [], "Stack must not be empty"
        return res

    def get_style (self):
        """Return the numeric style code as used by Plucker for
        the current state."""
        assert self._stack != [], "Stack must not be empty"
        return self._tags[self._stack[-1]]




class TextDocBuilder:
    """Encapsulate the knowledge of when to change styles, add paragraphs, etc."""

    def __init__ (self, url, config, **keyword_args):
        self._doc = PluckerDocs.PluckerTextDocument (url)
        self._config = config
        self._attributes = AttributeStack ()
        self._paragraph = PluckerDocs.PluckerTextParagraph ()
        self._is_new_paragraph = 1
        self._is_new_line = 1
        self._approximate_size = 0
        self._anchor_dict = None
        self._max_para_size = ((keyword_args.has_key("max_paragraph_size") 
                               and keyword_args["max_paragraph_size"] > 0 
                               and keyword_args["max_paragraph_size"]) 
                               or Max_Paragraph_Size)
        self._max_para_size_stretch = ((keyword_args.has_key("max_paragraph_size_anchor_stretch") 
                                       and keyword_args["max_paragraph_size_anchor_stretch"] > 0 
                                       and keyword_args["max_paragraph_size_anchor_stretch"]) 
                                       or Max_Paragraph_Size_Anchor_Stretch)
        # If document has no <body> tag, then will draw in device's default text color 
        # (which may not be black if they have hacked it with Kroma or similar utility) until
        # first color change, or new paragraph, then will go to black. This makes sure document
        # starts off in black.
        self._color_paragraphs = config.get_bool("color_paragraphs")
        if (self._color_paragraphs):
            self._paragraph.add_set_forecolor (self._attributes.get_forecolor ())


    def _within_anchor (self):
        return not (self._anchor_dict is None)

    
    def set_charset(self, charset):
        self._doc.set_charset(charset_name_to_mibenum(charset))

    def set_id_tag(self, tag):
        self._doc.register_doc(tag)

    def get_doc (self):
        """Finish up and get the PluckerTextDocument that we built"""
        self.close ()
        return self._doc


    def close (self):
        """Finish off"""
        if not self._is_new_paragraph:
            self._doc.add_paragraph (self._paragraph)
            self._paragraph = PluckerDocs.PluckerTextParagraph ()
            self._is_new_paragraph = 1
        if not self._doc.get_charset():
            # see if we can supply a default charset
            url = self._doc.get_url()
            if self._config:
                userspec = self._config.get_int('default_charset', 0)
            else:
                userspec = None
            locale_default = charset_name_to_mibenum(DEFAULT_LOCALE_CHARSET_ENCODING)
            # the userspec will take precedence
            if userspec:
                self._doc.set_charset(userspec)
            # OK, so we have no idea.  Use the HTTP default of ISO-8859-1 (4) for
            # http: URLs, and the environment default (if any) for others
            elif (string.lower(url[:5]) == 'http:' or string.lower(url[:6]) == 'https:'):
                self._doc.set_charset(4)
            elif locale_default:
                self._doc.set_charset(locale_default)

    def add_name (self, name):
        """Give name to the current paragraph"""
        self._paragraph.add_name (name)


    def indent (self, change_left, change_right):
        (l, r) = self._attributes.indent (change_left, change_right)
        self._paragraph.add_set_margin (l, r)


    def start_italics (self):
        """Change to italics if not already so"""
        newval = self._attributes.change_italics (1)
        if newval > 1:
            # was already italics on, so nothing needs be done
            pass
        else:
            self._paragraph.add_italics_start ()
            

    def end_italics (self):
        """Change to italics off if this is the last end_italics to come"""
        newval = self._attributes.change_italics (-1)
        if newval >= 1:
            # italics is still on (cascaded calls)
            pass
        else:
            self._paragraph.add_italics_end ()
            
            
    def start_underline (self):
        """Change to underlining text if not already so"""
        newval = self._attributes.change_underline (1)
        if newval > 1:
            # was already in underline mode, so nothing needs be done
            pass
        else:
            self._paragraph.add_underline_start ()
            

    def end_underline (self):
        """Change to underline off if this is the last end_underline to come"""
        newval = self._attributes.change_underline (-1)
        if newval >= 1:
            # underline is still on (cascaded calls)
            pass
        else:
            self._paragraph.add_underline_end ()


    def start_strike (self):
        """Change to strikethrough if not already so"""
        newval = self._attributes.change_strike (1)
        if newval > 1:
            # was already strikethrough on, so nothing needs be done
            pass
        else:
            self._paragraph.add_strike_start ()
            

    def end_strike (self):
        """Change to strikethrough off if this is the last end_strike to come"""
        newval = self._attributes.change_strike (-1)
        if newval >= 1:
            # strikethrough is still on (cascaded calls)
            pass
        else:
            self._paragraph.add_strike_end ()
    
           
    def set_style (self, style):
        """Set current style to 'tag', where tag is "b", "h1", "h2", ..."""
        if self._attributes.push (style):
            # style has changed
            self._add_style_change ()


    def unset_style (self, style):
        """Un-set a style change by a previous 'set_style'.
        Make sure it previously set 'style'."""
        if self._attributes.pop (style):
            # style has changed
            self._add_style_change ()


    def _add_style_change (self):
        """Add info about a new style to take effect."""
        self._paragraph.add_style_change (self._attributes.get_style ())


    def get_alignment (self):
        """Get current alignment (values  0, 1, or 2)"""
        return self._attributes.get_alignment ()


    def set_alignment (self, value):
        """Set current alignment to 'value', where value = 0, 1, 2"""
        if self._attributes.push_alignment (value):
            # alignment has changed
            self._add_alignment_change ()
   
   
    def unset_alignment (self, value):
        """Un-set an alignment change by a previous 'set_alignment'.
        Make sure it previously set 'value' (unless 'value' is None)."""
        if self._attributes.pop_alignment (value):
            # style has changed
            self._add_alignment_change ()    


    def get_forecolor (self):
        """Get current forecolor value. value should be an rgb"""
        return self._attributes.get_forecolor ()


    def set_forecolor (self, value):
        """Set current forecolor to 'value' """
        rgb = _parse_color (value)

        if not rgb:
            return

        # Don't want any white text on PDA's white form since would be invisible,
        # so if white, just darken it a bit to silver.
        if string.atoi(rgb, 16) == 0xFFFFFF:
            rgb = "C0C0C0"

        if self._attributes.push_forecolor (rgb):
            # forecolor has changed
            self._add_forecolor_change () 
            
    def unset_forecolor (self, value):
        """Un-set an alignment change by a previous 'set_forecolor'.
        Make sure it previously set 'value' (unless 'value' is None)."""
        if self._attributes.pop_forecolor (value):
            # forecolor has changed
            self._add_forecolor_change ()            
            
    def _add_alignment_change (self):
        """Add info about a new alignment to take effect."""
        self._paragraph.add_set_alignment (self._attributes.get_alignment ())

    def _add_forecolor_change (self):
        """Add info about a new forecolor to take effect."""
        self._paragraph.add_set_forecolor (self._attributes.get_forecolor ())        
        

    def _ship_paragraph (self):
        """Finish the current paragraph and start a fresh one"""

        # finish off the old paragraph
        the_anchor_dict = None
        if self._within_anchor ():
            the_anchor_dict = {}
            the_anchor_dict.update (self._anchor_dict)
            self.add_document_link_end ()
        if self._attributes.change_italics (0):
            self._paragraph.add_italics_end ()
        if self._attributes.change_underline (0):
            self._paragraph.add_underline_end ()
        if self._attributes.change_strike (0):
            self._paragraph.add_strike_end ()
     
            
        # now start new paragraph
        self._doc.add_paragraph (self._paragraph)
        self._paragraph = PluckerDocs.PluckerTextParagraph ()
        self._is_new_paragraph = 1
        self._is_new_line = 1
        self._approximate_size = 0

        if self._attributes.get_style ():
            # we are in non-default style
            self._add_style_change ()

        if self._attributes.get_alignment ():
            # we are in non-default alignment
            self._add_alignment_change ()
        
        if self._attributes.get_forecolor () or self._color_paragraphs:
            # we are in non-default forecolor
            self._paragraph.add_set_forecolor (self._attributes.get_forecolor ())    

        (l, r) = self._attributes.indent ()
        message(4, "-- New paragraph:  margins %d, %d", l, r)
        if l != 0 or r != 0:
            self._paragraph.add_set_margin (l, r)

        if self._attributes.change_italics (0):
            self._paragraph.add_italics_start ()

        if self._attributes.change_underline (0):
            self._paragraph.add_underline_start ()

        if self._attributes.change_strike (0):
            self._paragraph.add_strike_start ()
            
        # re-start the link if there was one
        if the_anchor_dict is not None:
            self.add_document_link_start (the_anchor_dict)
            

    def add_vspace (self, n_units=2, additional=0):
        """Make the representation to have a new line.
        Add a new paragraph, unless this one is already new and has no extra spacing"""

        n_units = min (n_units, 7)

        if n_units==0 and not self._is_new_paragraph:
            if not self._is_new_line:
                # special case: use <NL> code
                self._paragraph.add_newline ()
                self._is_new_line = 1
                return
            if not additional:
                # is already newline and we don't want additional vspace
                return
            else:
                # we are on a new line, so we need to add 4 units to get an additional new line
                n_units = 4


        if self._is_new_paragraph:
            if n_units == 0 and additional:
                n_units = 4
            old_spacing = self._paragraph.get_extra_spacing ()
            if not additional and (old_spacing >= n_units):
                 # already enough space
                 return
            if additional:
                new_spacing = old_spacing + n_units
            else:
                new_spacing = max (old_spacing, n_units)
            if new_spacing <= 7:
                self._paragraph.set_extra_spacing (new_spacing)
            else:
                while new_spacing > 7:
                    self._paragraph.set_extra_spacing (7)
                    new_spacing = new_spacing - 7
                    self.add_text ("  ")
                    self._ship_paragraph ()
                self._paragraph.set_extra_spacing (new_spacing)
        else:
            self._ship_paragraph ()
            self._paragraph.set_extra_spacing (n_units)


    def _find_text_split (self, line, size):
        """Split line so that the first part is approx. size bytes long.
        
        Return (first_part, rest)."""
        # XXX Why do we care?  Could we use TextWrapper?
        
        first = line[:size]
        rest = line[size:]

        # We try to split at a space:
        if " " in rest:
            f = string.split(rest, None, 1)
            if len (f) > 0:
                # Shouldn't this always be the case?  Mike reports that it can happen...
                first = first + f[0]
                if len (f) > 1:
                    rest = f[1]
                else:
                    rest = ""
            else:
                # Strange... how does this happen?
                first = first + rest
                rest = ""
        else:
            # No decent split found: just don't split it...
            first = first + rest
            rest = ""

        return (first, rest)
        

    def add_text (self, text):
        """Add some text, maybe even many lines."""
        lines = string.split (text, "\n")
        for i in range (len (lines)):
            line = lines[i]
            while 1:
                new_size = self._approximate_size + len (line)
                if self._within_anchor ():
                    max_size = self._max_para_size+self._max_para_size_stretch
                else:
                    max_size = self._max_para_size
                if new_size < max_size:
                    break
                rest_size = self._max_para_size - self._approximate_size
                if rest_size < 0:
                    rest_size = 0
                (first, rest) = self._find_text_split (line, rest_size)
                self._paragraph.add_text (first)
                self._approximate_size = self._approximate_size + len (first)
                self._is_new_paragraph = 0
                self._is_new_line = 0
                line = rest
                self._ship_paragraph ()
                if not line:
                    break
            
            if line:
                self._paragraph.add_text (line)
                self._approximate_size = self._approximate_size + len (line)
                self._is_new_paragraph = 0
                self._is_new_line = 0

            if i != len (lines)-1:
                # add the newline that was left out
                self.add_vspace (n_units=0, additional=1)

    def add_unicode_char (self, char_code, text_alternative):
        """Add a Unicode character, along with a non-Unicode text alternative."""
        self._paragraph.add_unicode_char (char_code, text_alternative)
        self._is_new_line = 0
        self._is_new_paragraph = 0
        self._approximate_size = self._approximate_size + 7 + len(text_alternative)

    def add_image (self, attributes):
        """Add an image reference"""
        self._is_new_paragraph = 0
        self._is_new_line = 0
        self._paragraph.add_image_reference (attributes)
        # print 'image attributes are ' + str(attributes)

    def add_table (self, dict_of_items):
        """Add a table"""
        self._is_new_paragraph = 0
        self._is_new_line = 0
        self._paragraph.add_table (dict_of_items)


    def add_document_link_start (self, dict_of_items):
        """Add an achor start"""
        if not self._within_anchor ():
            self._is_new_paragraph = 0
            self._is_new_line = 0
            self._paragraph.add_anchor_start (dict_of_items)
            self._anchor_dict = dict_of_items


    def add_document_link_end (self):
        """Add an achor end"""
        if self._within_anchor ():
            self._is_new_paragraph = 0
            self._is_new_line = 0
            self._paragraph.add_anchor_end ()
            self._anchor_dict = None
            

    def add_hr (self, height=0, width=0, perc_width=0):
        """Add a hr"""
        self._is_new_paragraph = 0
        self._is_new_line = 0
        self._paragraph.add_hr (height, width, perc_width)




class PlainTextParser:
    """Parsing a simple Text"""

    def __init__ (self, url, text, headers, config, attribs):
        text = _clean_newlines (text)
        # This we use to build the document
        self._doc = TextDocBuilder (url, config)
        if headers.has_key("charset"):
            self._doc.set_charset (headers["charset"])
        elif attribs.has_key("charset"):
            self._doc.set_charset (attribs["charset"])
        self._url = url
        self._text = text
        # In these two lists we store tuples of (url, attributes) for encountered anchors
        # and image references.  Currently we don't even search for these...
        self._anchors = []
        self._images = []

        self._doc.add_text (text)

        self._doc.close ()


    def get_plucker_doc (self):
        """Get the PluckerTextDocument.  Useful after a close()"""
        return self._doc.get_doc ()


    def get_anchors (self):
        """Return the list of found anchors"""
        return self._anchors


    def get_images (self):
        """Return the list of found images"""
        return self._images


    def has_unknown (self):
        """Check if during parsing we found unknown things"""
        return 0

    def print_unknown (self, prefix):
        """Print a summary of the unknown things found during parsing"""
        pass

    
    def get_unknown (self):
        """Get a list unknown things found during parsing."""
        return {}


# the following lists are derived from the HTML 4.01 spec.  Don't change them!
#  Actually, LI is not in the spec, but the spec defines it to act very much
#  as a block-level element, so we put it in our list for the moment.

HTML_BLOCK_ELEMENTS = ('head', 'body', 'li', 'dl', 'div', 'center', 'dir',
                        'menu', 'noscript', 'blockquote', 'form',
                        'hr', 'table', 'fieldset', 'address',
                        'noframes', 'isindex', 'h1', 'h2', 'h3', 'h4',
                        'h5', 'h6', 'ul', 'ol', 'pre')

HTML_OPTIONAL_END_ELEMENTS = ('body', 'colgroup', 'dd', 'dt', 'head', 'html',
                              'li', 'option', 'p', 'tbody', 'tfoot', 'thead')

HTML_TABLE_ELEMENTS = ('td', 'th', 'tr')

HTML_FORBIDDEN_END_ELEMENTS = ('area', 'base', 'basefont', 'br', 'col', 'frame',
                               'hr', 'img', 'input', 'isindex', 'link', 'meta',
                               'param')

HTML_NO_ID_ELEMENTS = ('base', 'head', 'html', 'meta', 'script', 'style', 'title')

class StructuredHTMLParser (sgmllib.SGMLParser):
    """Parsing correct HTML, and digesting it into a PluckerTextDocument."""

    def __init__ (self, url, text, headers = {}, config = None, attribs = {}):
        sgmllib.SGMLParser.__init__ (self)

        # Convert all <tag/> to <tag /> for XHTML compatability
        text = string.replace (text, "/>", " />")

        text = _clean_newlines (text)
        # This we use to build the document
        self._doc = TextDocBuilder (url, config, max_paragraph_size=3000)
        self._url = url
        self._base = None        # use this if defined for relative URLs
        self._config = config
        self._attribs = attribs
        # initialize verbosity...
        self._verbosity_stack = []
        # We use different indicator for diffent depths of <menu>, <ul> etc.
        self._ul_list_depth = 0
        self._ol_list_depth = 0
        # Remember the original text
        self._text = text
        # Figure out paragraph indentation style
        self._indent_paragraphs = config and config.get_bool('indent_paragraphs', 0)
        # Flag whether things found should be output.
        # This gets temporarily set to false during parse of headers.
        # It is now also used to hide non-content "tags", such as
        # style and script.  It is not perfect though; for example,
        # javascript:document.write("<div>") turns it back on, because
        # it only recognizes the div, not the javascript.
        self._visible = 1
        self._charset = headers.has_key('charset') and charset_name_to_mibenum(headers['charset'])
        if self._charset:
            self._doc.set_charset(headers['charset'])
        # Since some users are really stupid and use HTML wrong, we need a
        # stack of these values
        self._visibility_stack = []
        # In <pre> context, whitespace should not be massaged.  Since somebody
        # may use this recursivly, we make a stack out of it
        self._clean_whitespace = [1]
        # temporary hack: remeber if first <td> in <tr>
        self._first_td = 1
        # A simple stack of some tags, we are in.
        # This is mainly to detect if a <p> is on 'toplevel' or somewhere in
        # a table, list, etc.  There are two stacks because HTML has two categories
        # of element, block and inline.
        self._elements = []
        self._blocks = []
        self._inlines = []
        # Special list for table elements
        self._intables = []
        # A flag to tell if we are at the beginning of the most specific
        # containing element -- we clear this when we add text
        self._element_beginning = 0
        # In these two lists we store tupels of (url, attributes) for encountered anchors
        # and image references
        self._anchors = []
        self._images = []
        # Here we collect unknown things.  You can use this to detect if there are
        # tags, etc. in your documents that *should be handled by this parser but aren't.
        self._unknown = {}
        self._unhandled_tags = {

            # Add tags to this dictionary that you
            # know exist but do not want to handle.
            # as unknown items.

            'html' : 1,        
            'span' : 1,
            'code' : 1,
            'kbd' : 1,
            'big' : 1,
            'ilayer' : 1,
            'iframe' : 1,
            'layer' : 1,
            'marquee' : 1,
            'nobr' : 1,
            'blink': 1,        # you must be kidding!
            'form' : 1,
            'area' : 1,
            # 'option' : 1,    # now "handled" by ignoring it. Useless until we process forms.
            'input' : 1,
            'map' : 1,
            'noscript' : 1,
            'select' : 1,
                               

            # the following are non-standard, but people to use them
            'ahref': 1,  # it is amazing how many people mistype <a href=...> as this
            'pagebreak': 1, # are we printing, or what?
            'pagbreak': 1,
            
            # the following I found in various documents
            # no idea what they mean or should do...
            'basefont': 1, #Deprecated.  Default (and original for CSS inheritance) font
            'droplink': 1,
            }

        # initialize "_anchor_forecolor" in case we come across HTML with a <BODY> tag
        self._anchor_forecolor = config and config.get_string("anchor_color")

        # Table Stuff
        self.atable = None
        self.table_stack = []
        self.table_open_element_stack = []
        self.in_cell = 0
        self.table_count = 0
        self.last_table_style = 0
        self.last_table_forecolor = "000000"
        self.last_table_italics = 0
        self.last_table_underline = 0
        self.last_table_strike = 0

        # all set up, now process...
        self.feed (text)
        self.close ()

    def close (self):
        """Finish parsing."""
        sgmllib.SGMLParser.close (self)
        # we can only check the charset specified in the attribs after parsing
        # the document for <META> tags.  Seems kind of backward, but that's the
        # HTML spec.
        if not self._charset and self._attribs.has_key('charset'):
            self._set_charset(self._attribs['charset'])
        self._doc.close ()

    def get_plucker_doc (self):
        """Get the PluckerTextDocument.  Useful after a close()"""
        return self._doc.get_doc ()


    def get_anchors (self):
        """Return the list of found anchors"""
        return self._anchors


    def get_images (self):
        """Return the list of found images"""
        return self._images


    def has_unknown (self):
        """Check if during parsing we found unknown things"""
        return len (self._unknown.keys ())

    def print_unknown (self, prefix):
        """Print a summary of the unknown things found during parsing"""
        items = self._unknown.keys ()
        items.sort ()
        for item in items:
            print "%s%s" % (prefix, item)

    def get_unknown (self):
        """Get a list unknown things found during parsing."""
        items = self._unknown.keys ()
        items.sort ()
        return items

    def handle_starttag(self, tag, method, attrs):
        is_block = tag in HTML_BLOCK_ELEMENTS
        is_optional = tag in HTML_OPTIONAL_END_ELEMENTS
        can_have_id = tag not in HTML_NO_ID_ELEMENTS
        is_table = tag in HTML_TABLE_ELEMENTS
        # for things like <HR>, we need to process both start and end in one fell swoop
        is_oneswoop = not hasattr(self, "start_" + tag)

        message(4, "START %s (%s)%s, blocks is %s, inlines is %s, stack is %s", tag, is_block and "block" or "inline",
                is_oneswoop and "*" or "",  self._blocks, self._inlines, self.stack)

        # SGMLParser pushes the tag onto "stack" before calling us, *if* the element is
        # "oneswoop", but that's not the configuration we need, so temporarily remove it
        # while closing other elements
        if not is_oneswoop:
            del self.stack[-1]

        # Check for "id" attribute in element
        if can_have_id:
            id = _list_to_dict(attrs).get('id')
            if id:
                self._doc.add_name (id)

        if is_block:
            # end all in-line spans that are open

            while self._inlines:
                oldlen = len(self._inlines)
                t = self._inlines[0]
                self.finish_endtag(t)
                assert len(self._inlines) < oldlen, "Failed to reduce stack of inline elements %s by calling end_%s" % (self._inlines, t)
            self._blocks.insert(0, tag)

            if tag == 'table':
                self.in_cell = 0;

        elif is_table:
            if tag in self._intables:
                t = None
                while t != tag:
                    t = self._intables[0]
                    self.finish_endtag(t)

            self._intables.insert(0, tag)

        else:
            if is_optional and tag in self._inlines:
                t = None
                while t != tag:
                    t = self._inlines[0]
                    self.finish_endtag(t)

            self._inlines.insert(0, tag)

        # restore "stack" with tag
        if not is_oneswoop:
            self.stack.append(tag)

        sgmllib.SGMLParser.handle_starttag(self, tag, method, attrs)

        if is_oneswoop or (tag in HTML_FORBIDDEN_END_ELEMENTS):
            self.handle_endtag(tag, None)

    def handle_endtag(self, tag, method):
        message(4, "END   %s, blocks is %s, inlines is %s, stack is %s", tag,
                self._blocks, self._inlines, self.stack)

        if tag in self._inlines:

            while self._inlines[0] != tag:
                t = self._inlines[0]
                self.finish_endtag(t)
                del self._inlines[0]
            if method: sgmllib.SGMLParser.handle_endtag(self, tag, method)
            del self._inlines[0]

        elif tag in self._intables:

            while self._intables[0] != tag:
                t = self._intables[0]
                self.finish_endtag(t)
                del self._intables[0]
            if method: sgmllib.SGMLParser.handle_endtag(self, tag, method)
            del self._intables[0]


        elif tag in self._blocks:

            while self._inlines:
                t = self._inlines[0]
                self.finish_endtag(t)
                del self._inlines[0]
            while self._blocks[0] != tag:
                t = self._blocks[0]
                self.finish_endtag(t)
                del self._blocks[0]
            if method: sgmllib.SGMLParser.handle_endtag(self, tag, method)
            del self._blocks[0]

        else:
            message("Odd end tag </%s> found with no pending start tag", tag)

    ##
    ##  Private functions follow
    ##

    def _add_text (self, text):
        """Add some text.  This may contain newlines, however use
        _add_vspace() to do that explicitly if you want to."""
        if self._visible:
            if self.atable is not None and self.in_cell:
                self.atable.add_cell_text (text)
            else:
                self._doc.add_text (text)
                self._element_beginning = 0


    def _add_unicode_char (self, char_code, text_alternative):
        """Add a Unicode character, along with a non-Unicode alternative text string."""
        if self._visible:
            if self.atable is not None and self.in_cell:
                self.atable.add_cell_text (text_alternative)
            else:
                self._doc.add_unicode_char (char_code, text_alternative)
                self._element_beginning = 0


    def _add_vspace (self, n):
        """Add a newline to the document"""
        if self._visible:
            if self.atable is not None and self.in_cell:
                self.atable.add_cell_text ('\000\070')
            else:
                self._doc.add_vspace (n_units=n)
                self._element_beginning = 0


    def _push_verbosity (self, level):
        self._verbosity_stack.append(UtilFns.CurrentVerbosityLevel)
        UtilFns.CurrentVerbosityLevel = level
        message(2, "Changed verbosity level to %d", UtilFns.CurrentVerbosityLevel)

    def _pop_verbosity (self):
        if len(self._verbosity_stack) > 0:
            newlevel = self._verbosity_stack[-1]
            message(2, "Changing verbosity level to %d", newlevel)
            UtilFns.CurrentVerbosityLevel = newlevel
            del self._verbosity_stack[-1]


    def _push_element (self, element, data=None):
        """Push element onto our stack"""
        self._elements.append ((element, data))
        self._element_beginning = 1

    def _pop_element (self, element):
        """Pop an element from our stack and make sure it is element"""
        assert self._elements != [], "Trying to pop %s from empty element stack" % element
        assert self._elements[-1][0] == element, "Trying to pop %s but found %s (stack: %s)" % \
               (element, self._elements[-1][0], repr (self._elements))
        del self._elements [-1]
        self._element_beginning = 0


    def _get_top_element (self):
        """Return the top element to check things..."""
        if self._elements:
            return self._elements[-1][0]
        else:
            # can only happen with bad HTML...
            return ""

    def _get_element_info (self, element, default=None):
        """Return the information added with the last 'element'..."""
        for idx in range (len (self._elements)-1, -1, -1):
            if self._elements[idx][0] == element:
                return self._elements[idx][1]
        return default


    def _is_toplevel (self):
        """Check if we are on top-level, i.e. not within a list, a table etc."""
        return self._elements == []

    def _needs_newpara (self):
        """Check if we should output something for a new paragraph"""
        return self._elements == [] or self._elements[-1] == "table" or (not self._element_beginning)

    def _push_visibility (self, visiblilty):
        """Push 'visiblilty' onto the private visibility stack"""
        self._visibility_stack.append (self._visible)
        self._visible = visiblilty

    def _pop_visibility (self):
        """Pop the previous value from the visibility stack"""
        if self._visibility_stack:
            self._visible = self._visibility_stack [-1]
            del self._visibility_stack [-1]
        else:
            # This should not happen unless we encounter badly structured HTML...
            self._visible = 1

    def _set_charset (self, charset):
        if charset_name_to_mibenum(charset):
            self._charset = charset
            self._doc.set_charset(charset)

    ################################################################################
    ######## HTML specifics
    ######## 
    ######## these are actually private, but the sgmlparser from which we
    ######## inherit expects these public names
    ################################################################################

    def start_head (self, attributes):
        # <head> is found, this is all header infomation that we
        # just ignore...
        self._push_visibility (0)

    def end_head (self):
        # </head> is found, from now on everything is data for us to
        # process
        self._pop_visibility ()

    def start_title (self, attributes):
        # <title> is found, this is all header infomation that we
        # just ignore...
        self._push_visibility (0)

    def end_title (self):
        # </title> is found, from now on everything is data for us to
        # process
        self._pop_visibility ()


    def handle_comment(self, data):
        # print 'comment: ' + str(data)
        pass


    def do_meta (self, data):
        # if the charset is not already assigned (from the HTTP headers, presumably)
        # and it's available here, then use it
        if not self._charset and string.lower(data[0][0]) == 'http-equiv' and string.lower(data[0][1]) == 'content-type':
            from PyPlucker.Retriever import parse_http_header_value
            ctype, parameters = parse_http_header_value(data[1][1])
            for parameter in parameters:
                if parameter[0] == 'charset':
                    self._set_charset(parameter[1])
        

    def handle_charref (self, name):
        try:
            n = int(name)
        except ValueError:
            self.unknown_entityref(name)
            return
        if not 0 <= n <= 255:
            self.unknown_charref(name)
            return
        self.handle_data(chr(n))


    def handle_special (self, name):
        # Needed for fast xml-SGMLParser
        pass


    def handle_proc (self, foo, bar):
        # Needed for fast xml-SGMLParser
        pass

    
    def handle_cdata (self, data):
        # Needed for fast xml-SGMLParser
        #print "\tCDATA: %s" % repr(data)
        pass

    
    def handle_data (self, data):
        if self._clean_whitespace[-1]:
            data = _RE_WHITESPACE.sub(" ", data, 0)   # data = re.sub("[\n\r\f \t]+", " ", data)
        else:
            # we should retain the whitespace layout.
            # However Plucker cannot handle tabs, form-feeds, so we need to
            # do something useful with that
            data = string.translate (data, _CLEANUP_TRANSTABLE)
            data = string.replace (data, "\t", "  ")


        #stripped_data = string.strip(data)
        if data:
            # not just blank or empty text (e.g. from comments), so we
            # add it...
            if self.atable is not None and self.in_cell:
                new_style = self._doc._attributes.get_style()
                if new_style != self.last_table_style:
                    style_str = struct.pack("<BBB", 0, 17, new_style)
                    self.atable.add_cell_text(style_str)
                    self.last_table_style = new_style
                new_forecolor = self._doc.get_forecolor()
                if new_forecolor != self.last_table_forecolor:
                    rgb = new_forecolor
                    r = string.atoi (rgb[0:2], 16)
                    g = string.atoi (rgb[2:4], 16)
                    b = string.atoi (rgb[4:6], 16)
                    style_str = struct.pack ("<BBBBB", 0, 0x53, r, g, b)
                    self.atable.add_cell_text(style_str)
                    self.last_table_forecolor = new_forecolor
                new_italics = self._doc._attributes.change_italics (0)
                if new_italics != self.last_table_italics:
                    if new_italics:
                        style_str = struct.pack ("<BB", 0, 0x40)
                    else:
                        style_str = struct.pack ("<BB", 0, 0x48)
                    self.atable.add_cell_text(style_str)
                    self.last_table_italics = new_italics
                new_underline = self._doc._attributes.change_underline (0)
                if new_underline != self.last_table_underline:
                    if new_underline:
                        style_str = struct.pack ("<BB", 0, 0x60)
                    else:
                        style_str = struct.pack ("<BB", 0, 0x68)
                    self.atable.add_cell_text(style_str)
                    self.last_table_underline = new_underline
                new_strike = self._doc._attributes.change_strike (0)
                if new_strike != self.last_table_strike:
                    if new_strike:
                        style_str = struct.pack ("<BB", 0, 0x70)
                    else:
                        style_str = struct.pack ("<BB", 0, 0x78)
                    self.atable.add_cell_text(style_str)
                    self.last_table_strike = new_strike

            self._add_text (data)


    def start_body (self, attributes):
        attributes = _list_to_dict (attributes)

        # Place the rgb value of <body>'s 'link' into our global variable for coloring anchors
        if attributes.has_key ('link'):
            rgb = string.lower (attributes['link'])
            self._anchor_forecolor = rgb

        # Append rgb value of <body>'s 'text' into our regular forecolor stack.
        # Note that bak in TextDocBuilders's init function, the document was 
        # initialized with a forecolor set to TextParser.py's default value for
        # color (which is black). That addresses the case of there being no
        # <body> tag in the document. If <body text=""> does exist, just place
        # it as a second color function after the first.
        if attributes.has_key ('text'):
            rgb = string.lower (attributes['text'])
            self._doc.set_forecolor (rgb)                        
        # This signals that start seeing the page's text from <body> to </body>.
        self._push_visibility (1) # should be 1 anyway, but make sure        
        

    def end_body (self):
        # End of body. Nothing should follow, but we explicitly ignore
        # it anyway
        self._pop_visibility ()

    
    def do_img (self, attributes):
        if self._visible:
            attributes = _list_to_dict (attributes)
            # PDA device can render a black and white image in color, which is nice
            # since can then get a single-colored icon and a b&w icon out of the same icon
            # and the b&w version won't have a blank with only a [8bpp], which would get 
            # if used a color icon and looked at it in lower color depth. This is called
            # by setting a 'color' attribute inside the <img> tag you want to color. If not
            # specified, use b&w only.
            rgb = "#000000"
            
            if attributes.has_key ('alt'):
                # Clean up cruft
                attributes['alt'] = _RE_NONSPACEWHITESPACE.sub (' ', attributes['alt'], 0)
                if attributes['alt'].lower() in junk_alt_attributes:
                    del attributes['alt']
            if attributes.has_key ('src'):
                # Set the forecolor, replacing our '#000000' with a color attribute if specified
                if attributes.has_key ('color'):
                    rgb = string.lower (attributes['color'])
                    self._doc.set_forecolor (rgb)
                else:
                    rgb = None
                
                attributes['src'] = Url.CleanURL (attributes['src'], self._base or self._url)
                attributes['_plucker_id_tag_inlineimage'] = PluckerDocs.obtain_fresh_id()
                attributes['_plucker_id_tag_outoflineimage'] = PluckerDocs.obtain_fresh_id()
                if self.atable is not None and self.in_cell:
                    self.atable.add_cell_image(image = attributes['_plucker_id_tag_inlineimage'], attr = attributes)
                    self._doc._paragraph._image_refs.append((attributes['src'], attributes))
                else:
                    self._doc.add_image (attributes)
                # CRH do this in either case, so the images get spidered
                self._images.append ((attributes['src'], attributes))

                # Unset the forecolor after img is over
                if rgb:
                    forecolor = self._doc.get_forecolor ()
                    self._doc.unset_forecolor (forecolor) 
                
                self._element_beginning = 0
            else:
                print "<img> without src!  (%s)" % repr (attributes)


    def start_a (self, attributes):
        attributes = _list_to_dict (attributes)
        if attributes.has_key ("href"):
            if self._visible:
                if attributes.has_key ('href'):
                    attributes['href'] = Url.CleanURL (attributes['href'], self._base or self._url)
                    attributes['_plucker_id_tag'] = PluckerDocs.obtain_fresh_id()
                    attributes['_plucker_from_url'] = self._url
                    the_url = Url.URL (attributes['href'])
                    #if the_url.get_protocol () == 'mailto':
                    #    # Mailtos are not yet handled...
                    #    attributes['href'] = "plucker:/mailto.html"
                    
                    # Color the anchor according to anchor_forecolor
                    # (set with 'link' attribute of body tag). Can override
                    # with a <font color=""> inside the anchor
                    if self._anchor_forecolor:
                        self._doc.set_forecolor(self._anchor_forecolor)
                    
                    self._anchors.append ((attributes['href'], attributes))
                    self._doc.add_document_link_start (attributes)
                    self._push_element ("a")

                    if self.atable is not None and self.in_cell:
                        self.atable.add_link(attr = attributes)

        if attributes.has_key ("name"):
            self._doc.add_name (attributes['name'])


    def end_a (self):
        # only set end anchor if this was a remote reference
        # (as compared to just a named anchor for later referencing)
        if self._get_top_element () == "a":
            if self._visible:
                self._pop_element ("a")
                self._doc.add_document_link_end ()
                if self.atable is not None and self.in_cell:
                    self.atable.end_link()
                if self._anchor_forecolor:
                    # Stop coloring the anchor
                    forecolor = self._doc.get_forecolor ()
                    self._doc.unset_forecolor (forecolor) 

    def do_base(self, attributes):
        # set self._base even if not in visible section
        attributes = _list_to_dict (attributes)
        if attributes.has_key ('href'):
            # clean up cruft
            self._base = Url.CleanURL (attributes['href'], self._url)

    #
    # The "contents" of a style or script tag is not really intended
    # for human consumption.  Until we use the information, the best
    # we can do is make it invisible, so that at least people don't
    # have to read it.  Note that this is not foolproof - visibility
    # seems to turn back on at the "div" of 
    #    javascript:document.write("<div>")
    #
    def start_script (self, attributes):
        self._push_visibility (0)

    def end_script (self):
        self._pop_visibility ()


    def start_javascript (self, attributes):
        self._push_visibility (0)

    def end_javascript (self):
        self._pop_visibility ()

    # Plucker does not yet handle forms, but we need to display
    # them because so many sites wrap the full content within a
    # form.  Even a <select> may contain information about what
    # sort of information will be required.  Listing all the 
    # possible options just interrupts the reader.  (Gee, this
    # site can name all 50 states!)
    def start_option (self, attributes):
        self._push_visibility (0)

    def end_option (self):
        self._pop_visibility ()

    def start_style (self, attributes):
        self._push_visibility (0)

    def end_style (self):
        self._pop_visibility ()



    def start_b (self, attributes):
        self._doc.set_style ("b")


    def end_b (self):
        self._doc.unset_style ("b")


    def start_small (self, attributes):
        self._doc.set_style ("small")


    def end_small (self):
        self._doc.unset_style ("small")


    def start_sub (self, attributes):
        self._doc.set_style ("sub")


    def end_sub (self):
        self._doc.unset_style ("sub")


    def start_sup (self, attributes):
        self._doc.set_style ("sup")


    def end_sup (self):
        self._doc.unset_style ("sup")


    def start_strong (self, attributes):
        self.start_b (attributes)


    def end_strong (self):
        self.end_b ()


    def start_i (self, attributes):
        self._doc.start_italics ()


    def end_i (self):
        self._doc.end_italics ()


    def start_u (self, attributes):
        self._doc.start_underline ()


    def end_u (self):
        self._doc.end_underline ()


    def start_s (self, attributes):
        self._doc.start_strike ()


    def end_s (self):
        self._doc.end_strike ()


    def start_strike (self, attributes):
        self._doc.start_strike ()


    def end_strike (self):
        self._doc.end_strike ()
        
        
    def start_font (self, attributes):
        attributes = _list_to_dict (attributes)
        if attributes.has_key ('color'): 
            rgb = string.lower (attributes['color'])
            self._doc.set_forecolor (rgb)
        # Following keeps the forecolor stack intact if it is a <font>
        # without a 'color' attribute, such as <font size="+1">
        else:    
            self._doc.set_forecolor (self._doc.get_forecolor ())
            
            
    def end_font (self):
        forecolor = self._doc.get_forecolor ()
        self._doc.unset_forecolor (forecolor) 


    def start_address (self, attributes):
        self._doc.start_italics ()


    def end_address (self):
        self._doc.end_italics ()


    def start_em (self, attributes):
        self._doc.start_italics ()


    def end_em (self):
        self._doc.end_italics ()


    def start_h1 (self, attributes):
        self._add_vspace (3)
        self._doc.set_style ("h1")
        self._doc.set_alignment (2)


    def end_h1 (self):
        self._doc.unset_style ("h1")
        self._add_vspace (2)
        self._doc.unset_alignment (2)


    def start_h2 (self, attributes):
        self._add_vspace (3)
        self._doc.set_style ("h2")


    def end_h2 (self):
        self._doc.unset_style ("h2")
        self._add_vspace (2)


    def start_h3 (self, attributes):
        self._add_vspace (2)
        self._doc.set_style ("h3")


    def end_h3 (self):
        self._doc.unset_style ("h3")
        self._add_vspace (1)


    def start_h4 (self, attributes):
        self._add_vspace (2)
        self._doc.set_style ("h4")


    def end_h4 (self):
        self._doc.unset_style ("h4")
        self._add_vspace (1)


    def start_h5 (self, attributes):
        self._add_vspace (2)
        self._doc.set_style ("h5")


    def end_h5 (self):
        self._doc.unset_style ("h5")
        self._add_vspace (1)


    def start_h6 (self, attributes):
        self._add_vspace (2)
        self._doc.set_style ("h6")


    def end_h6 (self):
        self._doc.unset_style ("h6")
        self._add_vspace (1)


    def start_p (self, attributes):
        attribs = _list_to_dict (attributes)
        alignment = self._doc.get_alignment ()
        if attribs.has_key ('align'):
            align = string.lower (attribs['align'])
            if align == 'left':
                alignment = 0
            elif align == 'center':
                alignment = 2
            elif align == 'right':
                alignment = 1
            elif align == 'justify':
                alignment = 3
            else:
                print "Unknown alignment '%s' requested in <p>" % align
        if self.atable is not None and self.in_cell:
            self.atable.set_align (alignment)
        else:
            self._doc.add_vspace (0)
        self._doc.set_alignment (alignment)
        if self.atable is None:
            self.do_p (attributes)

    def end_p (self):
        self._doc.add_vspace (0)
        self._doc.unset_alignment (value=None)

    def do_p (self, attributes):
        if self._needs_newpara ():
            if self._indent_paragraphs:
                self._add_text('\xa0\xa0\xa0\xa0\xa0\xa0')
            else:
                self._add_vspace (2)


    def start_div (self, attributes):
        attributes = _list_to_dict (attributes)
        alignment = self._doc.get_alignment ()
        if attributes.has_key ('align'):
            align = string.lower (attributes['align'])
            if align == 'left':
                alignment = 0
            elif align == 'center':
                alignment = 2
            elif align == 'right':
                alignment = 1
            elif align == 'justify':
                alignment = 3
            else:
                print "Unknown alignment '%s' requested in <div>" % align
        if self.atable is not None and self.in_cell:
            self.atable.set_align (alignment)
        else:
            self._doc.add_vspace (0)
        self._doc.set_alignment (alignment)
                

    def end_div (self):
        if self.atable is None:
            self._doc.add_vspace (0)
        self._doc.unset_alignment (value=None)


    def start_center (self, attributes):
        if self.atable is not None and self.in_cell:
            self.atable.set_align (2)
        else:
            self._doc.add_vspace (0)
        self._doc.set_alignment (2)


    def end_center (self):
        if self.atable is None:
            self._doc.add_vspace (0)
        self._doc.unset_alignment (2)

    
    def do_br (self, attributes):
        if self.atable is not None and self.in_cell:
            self.atable.add_cell_text ('\000\070')
        else:
            self._add_vspace (0)


    def do_hr (self, attributes):
        height = 2 # hr thickness size 0=viewer.prc's default, which is 2.
        width = 0 #0=viewer's default behavior: full screen width.
        perc_width = 0 #0=viewer's default behavior: screen width.
        needs_align_clean = 0 # whether need to call an unset_alignment after drawing.
        attributes = _list_to_dict (attributes)

        if attributes.has_key ('size'):
             height = int (attributes['size'])
        if attributes.has_key ('color'):
            rgb = string.lower (attributes['color'])
        else:
            rgb = "#808080" # Default to gray for hr if color not specified
        if attributes.has_key ('width'):
             w = attributes['width']
             if w != '100%': # pointless to handle/store 100%s.
                  if w[-1] == '%': # rightmost character is a '%' sign
                       perc_width = int (w[:-1]) # shave off trailing '%'. 
                  else: #if no % must be a pixel width
                       width = int (w) 
                       if width > 150: # If > 150 pixels, cut to 150 pixels, so all screens work.
                            width = 150
                  if attributes.has_key ('align'): # hr aligns should override a current div
                       align = string.lower (attributes['align'])
                       if align == 'left':
                            alignment = 0
                       elif align == 'center':
                            alignment = 2
                       elif align == 'right':
                            alignment = 1
                       else:
                            alignment = 2 # webauthor error: unsupported hr align.
                       self._doc.set_alignment (alignment)
                       needs_align_clean = 1 # call unset_alignment after drawing hr.
                  else: # no alignment in specified in hr
                       alignment = 2 # hr should be drawn centered 
                       self._doc.set_alignment (alignment) 
                       needs_align_clean = 1 # call unset_alignment after drawing hr.
        self._add_vspace (4) # some spacing above hr
        self._doc.set_forecolor (rgb)
        self._doc.add_hr (height=height, width=width, perc_width=perc_width) #add the hr directive. 
        forecolor = self._doc.get_forecolor ()
        self._doc.unset_forecolor (forecolor) 
        self._add_vspace (4) # some spacing below hr
        if needs_align_clean == 1: # if changed alignment, unset it.
             self._doc.unset_alignment (alignment)   


    def start_ul (self, attributes):
        self._ul_list_depth = self._ul_list_depth + 1
        self._push_element ("list")


    def end_ul (self):
        if self._get_top_element () == "li":
            self._end_li ()
        self._ul_list_depth = self._ul_list_depth - 1
        assert(self._ul_list_depth >= 0)
        self._pop_element ("list")


    def start_ol (self, attributes):
        self._ol_list_depth = self._ol_list_depth + 1
        self._push_element ("list", [1])


    def end_ol (self):
        if self._get_top_element () == "li":
            self._end_li ()
        self._ol_list_depth = self._ol_list_depth - 1
        assert(self._ol_list_depth >= 0)
        self._pop_element ("list")


    def start_menu (self, attributes):
        self.start_ul (attributes)


    def end_menu (self):
        self.end_ul ()


    def do_li (self, attributes):
        if self._get_top_element () == "li":
            self._end_li ()
        self._add_vspace (0)
        data = self._get_element_info ("list")
        if data:
            number = data[0]
            data[0] = number + 1
            indent = 12
            if self._ol_list_depth % 3 == 1:
                text = "%d) " % number
            elif self._ol_list_depth % 3 == 2:
                if number < 26:
                    text = string.uppercase[number-1] + ") "
                else:
                    text = "?) "
            elif self._ol_list_depth % 3 == 0:
                if number < 26:
                    text = string.lowercase[number-1] + ") "
                else:
                    text = "?) "
            table_margin = self._ol_list_depth
        else:
            if self._ul_list_depth == 1:
                text = ((0x2022, "o"), " ")
                indent = 7
            elif self._ul_list_depth == 2:
                text = chr(0xbb) + " "
                indent = 6
            elif self._ul_list_depth == 3:
                text = "+ "
                indent = 8
            else:
                text = "> "
                indent = 4  # Mike: is this correct?
            table_margin = self._ul_list_depth

        self._doc.indent (indent, 0)

        self._doc.set_style ("")  # make sure we render the 'bullet' marker in normal style
        if self.atable is not None and self.in_cell:
            self._add_text('\xa0\xa0' * table_margin)
            style_str = struct.pack ("<BBBBB", 0, 0x53, 0, 0, 0) # black
            self.atable.add_cell_text(style_str)

        if type(text) == type(""):
            self._add_text (text)
        elif type(text) == type(()):
            for element in text:
                if type(element) == type(""):
                    self._add_text(element)
                elif type(element) == type(()) and len(element) == 2:
                    self._add_unicode_char(element[0], element[1])
        self._doc.unset_style ("")

        if self.atable is not None and self.in_cell:
            new_forecolor = self._doc.get_forecolor()
            rgb = new_forecolor
            r = string.atoi (rgb[0:2], 16)
            g = string.atoi (rgb[2:4], 16)
            b = string.atoi (rgb[4:6], 16)
            style_str = struct.pack ("<BBBBB", 0, 0x53, r, g, b)
            self.atable.add_cell_text(style_str)

        self._push_element ("li", indent)


    def _end_li (self):
        if self._get_top_element () == "li":
            indent = self._get_element_info ("li")
            self._pop_element ("li")
            self._doc.indent (-indent, 0)
            self._add_vspace (1)


    def start_li (self, attributes):
        self.do_li (attributes)


    def end_li (self):
        self._end_li ()


    def start_dl (self, attributes):
        self._push_element ("dl")


    def end_dl (self):
        self._pop_element ("dl")


    def start_dt (self, attributes):
        self._add_vspace (1)


    def end_dt (self):
        pass


    def start_dd (self, attributes):
        self._add_vspace (1)


    def end_dd (self):
        pass


    def start_table (self, attributes):
        border = 0
        attr = _list_to_dict (attributes)
        if attr.has_key ('border') and attr['border']!="0":
            border = 1
        # Robert: Colored nested tables aren't supported, but could be added.
        if attr.has_key ('bordercolor'):
            rgb = string.lower (attr['bordercolor'])
            self._tableborder_forecolor = rgb
        else:
            self._tableborder_forecolor = "default"   
        self._push_element ("table", border)
        tables = self._config and self._config.get_bool('tables', 0)
        if self._attribs.has_key('tables') or tables:
            turl = Url.CleanURL (self._url) + "Table%d" % self.table_count
            attributes = _list_to_dict (attributes)
            attributes['href'] = turl
            attributes['_plucker_id_tag'] = PluckerDocs.obtain_fresh_id()
            attributes['_plucker_from_url'] = turl
            if self.atable is not None:
                self.atable.add_table(table = turl, attr = attributes)
                self.table_stack.append(self.atable)
                self.table_open_element_stack.append(self._intables)
                self._intables = []
            bpp = self._config.get_int('bpp', 1)
            border_color = 0
            if attr.has_key ('bordercolor'):
                rgb = _parse_color(attr['bordercolor'])
                if rgb is not None:
                    border_color = string.atoi(rgb, 16)
            link_color = 0
            if self._anchor_forecolor:
                rgb = _parse_color(self._anchor_forecolor)
                if rgb is not None:
                    link_color = string.atoi(rgb, 16)
            self.atable = PluckerDocs.PluckerTableDocument(turl, border, attributes, bpp, border_color, link_color)
            if not len(self.table_stack):
                self._doc.add_table (attributes)
            self.table_count = self.table_count + 1
            self.last_table_style = 0
            self.last_table_forecolor = "000000"


    def end_table (self):
        if self.atable is not None:
            self._doc._doc.add_table (self.atable)
#           self.atable.dump ()
            self.atable = None
            if len(self.table_stack):
                self.atable = self.table_stack[-1]
                del self.table_stack[-1]
                self._intables = self.table_open_element_stack[-1]
                del self.table_open_element_stack[-1]
                self.in_cell = 1
        else:
            if self._get_element_info ('table'):
                self._add_vspace (1)
                if self._tableborder_forecolor == "default":
                    self._doc.set_forecolor ("#999999")
                else:
                    self._doc.set_forecolor (self._tableborder_forecolor)
                self._doc.add_hr (height=2)
                self._doc.unset_forecolor (self._doc.get_forecolor ()) 
                self._add_vspace (1)
            else:
                self._add_vspace (1)
        self._pop_element ("table")
        # Reset global for table colors
        self._tableborder_forecolor = "default"


    def start_tr (self, attributes):
        if self.atable is not None:
            self.atable.add_row ()
        else:
            if self._get_element_info ('table'):
                self._add_vspace (1)
                if self._tableborder_forecolor == "default":
                    self._doc.set_forecolor ("#999999")
                else:
                    self._doc.set_forecolor (self._tableborder_forecolor)
                self._doc.add_hr (height=2)
                self._doc.unset_forecolor (self._doc.get_forecolor ()) 
                self._add_vspace (1)
            else:
                self._add_vspace (1)
            self._first_td = 1



    def end_tr (self):
        if self.atable is None:
            self._add_vspace (0)


    def start_th (self, attributes):
        self._doc.set_style ("th")
        self.start_td (attributes)


    def end_th (self):
        self.end_td ()
        self._doc.unset_style ("th")


    def start_td (self, attributes):
        if self.atable is None:
            if self._first_td:
                self._add_vspace (0)
                self._first_td = 0
            else:
                if self._get_element_info ('table'):
                    self._add_vspace (1)
                    if self._tableborder_forecolor == "default":
                        self._doc.set_forecolor ("#C0C0C0")
                    else:
                        self._doc.set_forecolor (self._tableborder_forecolor)
                    self._doc.add_hr (height=1)
                    self._doc.unset_forecolor (self._doc.get_forecolor ()) 
                    self._add_vspace (0)
                else:
                    self._add_vspace (1)
        attributes = _list_to_dict (attributes)
        alignment = 0
        if attributes.has_key ('align'):
            align = string.lower (attributes['align'])
            if align == 'left':
                alignment = 0
            elif align == 'center':
                alignment = 2
            elif align == 'right':
                alignment = 1
            elif align == 'justify':
                alignment = 3
            elif align == 'top' or align == 'bottom' or align == 'middle':
                # vertical alignment not handled...
                pass
            else:
                print "Unknown alignment '%s' requested in <td>/<th>" % align
        self._doc.set_alignment (alignment)
        if self.atable is not None:
            if self._doc._attributes._stack[-1] == 'th':
                alignment = 2
            self.in_cell = 1
            self.atable.add_cell(alignment, attributes)


    def end_td (self):
        self._doc.unset_alignment (value=None)
        if self.atable is not None:
            self.in_cell = 0


    def start_pre (self, attr):
        self._doc.set_style ("pre")
        self._clean_whitespace.append (0)


    def end_pre (self):
        del self._clean_whitespace[-1]
        self._doc.unset_style ("pre")


    def start_tt (self, attr):
        self._doc.set_style ("pre")


    def end_tt (self):
        self._doc.unset_style ("pre")


    def start_blockquote (self, attr):
        self._add_vspace (2)
        self._doc.indent (12, 12)


    def end_blockquote (self):
        self._doc.indent (-12, -12)
        self._add_vspace (2)


    def start_q (self, attr):
        self._add_vspace (2)
        self._doc.indent (5, 5)
        self._add_text ("  ``")
        self._doc.indent (7, 7)
 

    def end_q (self):
        # We stagger the removal of the indentation, so that
        # the closing quotes can be right of the quoted block
        # but not left of them
        self._doc.indent (0, -12)
        self._add_text ("´´")
        self._doc.indent (-12, 0)
        self._add_vspace (2)


    def start_cite (self, attr):
        self.start_i (attr)
        #self.start_blockquote (attr)


    def end_cite (self):
        self.end_i ()
        #self.end_blockquote ()


    def do_link (self, attr):
        if self._config and self._config.get_bool('bookmarks', 0):
            attrib = _list_to_dict(attr)
            if attrib.has_key('rel') and attrib['rel'] == 'bookmark':
                if attrib.has_key('title') and attrib.has_key('href'):
                    href = Url.CleanURL (attrib['href'], self._base or self._url) 
                    self._doc._doc.add_bookmark (attrib['title'], href)


    ## These get called on unhandled things.
    ## We collect info about them in self._unknown and print that out
    ## on demand via print_unknown()

    def unknown_starttag (self, tag, attributes):
        #print "\tUNKNOWN <%s>" % tag

        if tag == "plucker":

            attributes = _list_to_dict(attributes)
            if attributes.has_key("verbosity"):
                level = string.atoi(attributes["verbosity"])
                if level:
                    self._push_verbosity(level)
                else:
                    self._push_verbosity(UtilFns.CurrentVerbosityLevel)
            else:
                self._pop_verbosity()

        else:
            if self._visible:
                if not self._unhandled_tags.has_key (tag):
                    self._unknown["<%s>"%tag] = 1

    def unknown_endtag (self, tag):
        #print "\tUNKNOWN </%s>" % tag
        if self._visible:
            if not self._unhandled_tags.has_key (tag):
                self._unknown["</%s>"%tag] = 1

    def unknown_charref (self, ref):
        if self._visible:
            val = int(ref)
            if val == 8211:
                self._add_unicode_char (val, "-")
            elif val == 8212:
                self._add_unicode_char (val, "--")
            elif val == 8216:
                self._add_unicode_char (val, "`")
            elif val == 8217:
                self._add_unicode_char (val, "´")
            elif val == 8220:
                self._add_unicode_char (val, "\"")
            elif val == 8230:
                        self._add_unicode_char (val, "...")
            elif val == 8221:
                self._add_unicode_char (val, "\"")
            elif val == 8226:
                # what's this?  Unbreakable space?
                self._add_unicode_char (val, " ")
            elif val == 8482:
                self._add_unicode_char (val, "(tm)")
            else:
                self._unknown["charref-%s" % ref] = 1
                self._add_unicode_char (val, "&#%d;" % val)

    def unknown_entityref (self, ref):
        if self._visible:
            if htmlentitydefs.entitydefs.has_key (ref):
                s = htmlentitydefs.entitydefs[ref]
                if len(s) == 1:
                    val = ord(s)
                    if (val >= 0xa0 and val < 0x100) or (val >= 0x00 and val < 0xFF):
                        self.handle_data (s)
                    else:
                        self._add_unicode_char(val, "&#%d;" % val)
                else:
                    m = _html_char_ref_pattern.match(s)
                    if m:
                        self.unknown_charref(m.group(1))
            else:
                self._unknown["entityref-%s"%ref] = 1
                #self.handle_data('?')
                self.handle_data("&"+ref)



## If called as a script, we just parse the given files
## and dump their representations:
if __name__ == "__main__":
    import getopt
    def usage(reason=None):
        if reason is not None:
            print reason
        print "Usage: %s  {-c|-p} <filename> [  <filename> ....]" % sys.argv[0]
        print "     -p  parses the html file(s) and shows the internal representation"
        print "     -c  checks the html file(s) for unknown/unhandled things"
        sys.exit (1)

    def id_resolver (any_dict, as_image):
        return 0

    # the option dictionary will be used to hold flags if that option is set
    # it gets initialized to all false
    option = {}
    for letter in string.lowercase + string.uppercase:
        option[letter] = None
    (optlist, args) = getopt.getopt(sys.argv[1:], "cp")
    for (k,v) in optlist:
        # k is of the form e.g. '-s', so we pick the second char
        if v == "":
            v = 1
        option[k[1]] = v

    if args:
        for filename in args:
            print "\nParsing %s..." % filename
            file = open(filename, "r")
            text = string.join (file.readlines (), "")
            file.close()
            converter = StructuredHTMLParser ('file:/'+filename, text)
            if option['p']:
                print "All parsed.  Contents is:"
                pluckerdoc = converter.get_plucker_doc ()
                pluckerdoc.resolve_ids (id_resolver)
                #pluckerdoc.pretty_print () -- has no pretty printing :-(
                binformat = pluckerdoc.dump_record (1)
                new_pluckerdoc = PluckerDocs.PluckerTextDocument ('file:/'+filename)
                new_pluckerdoc.undump_record (binformat, verbose=1)
            print "Found anchors and images:" 
            for (url, attr) in converter.get_anchors ():
                print "  anchor '%s'" % url
            for (url, attr) in converter.get_images ():
                print "  image  '%s'" % url
            if option['c']:
                if converter.has_unknown ():
                    print "\nUnhandled things in the document:"
                    converter.print_unknown ("  ")
        print "done"
    else:
        usage("Please specify some arguments")
        
