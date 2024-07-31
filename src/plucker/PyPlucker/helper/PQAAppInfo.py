#!/usr/bin/env python

import PyPlucker
from PyPlucker import ImageParser, Retriever
import os, struct, operator

def_big_icon = '\000\040\000\040\000\004\000\000\001\001\000\000\000\000\000\000' + \
               '\000\000\000\000\000\000\000\000\001\340\170\000\001\020\374\000' + \
               '\001\011\376\000\001\145\377\000\001\005\377\000\001\167\377\000' + \
               '\001\007\377\000\001\167\376\000\001\007\374\000\001\377\340\000' + \
               '\000\377\300\000\001\377\300\000\001\217\340\000\001\215\360\000' + \
               '\001\214\370\000\001\374\070\000\000\370\030\000\000\000\000\000' + \
               '\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000' + \
               '\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000' + \
               '\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000'

def_small_icon = '\000\017\000\011\000\002\000\000\001\001\000\000\000\000\000\000' + \
                 '\360\030\211\074\245\276\205\376\264\374\205\360\266\160\206\130' + \
                 '\375\210'



def swap(value):
    return struct.pack('>H', value)



def is_valid_tbmp(data, default):
    if len(data) < 16:
        return 0
    if (data[0:8] != default[0:8]) or \
       (data[10:6] != default[10:6]) or \
       (data[9] > chr(1)):    # Something is invalid
        return 0
    else:    # All OK
        return 1



def get_str(data):
    def word_boundet(data):
        if len(data) and operator.mod(len(data), 2):
            data = data + chr(0)
        return data
    data = word_boundet(data)
    return swap(len(data)/2) + data;



def get_icon(config, default_icon, req_width, req_height, filename):
    if len(filename) > 0:
        if os.path.isfile(filename):
            f = open(filename, 'rb')
            data = f.read()
            f.close()
            if is_valid_tbmp(data,default_icon):
                icon = data
            else:
                # Not a Tbmp, so convert it to a Tbmp with the image parser.
                parser = ImageParser.get_default_parser(config)
                # Make some image attribs for calling the image parser.
                # Maxheight and Maxwidth remain strings (get converted to int in the image parser),
                # but bpp needs to be int. bpp is always 1 on these appinfo block images.
                attribs = {}
                attribs['maxheight'] = req_height
                attribs['maxwidth'] = req_width
                attribs['bpp'] = 1
                # Do parsing. The final zero parameter is to not compress the images.
                parsed = parser(filename, Retriever.GuessType(filename), data, config, attribs, 0)
                data = parsed.get_plucker_doc()
                data = data._data
                # Check it again
                if is_valid_tbmp(data,default_icon):
                    icon = data
                else:
                    # Something is wrong with the icon. Give up
                    print "Something is wrong with %s, using default" % filename
                    icon = default_icon
        else:
            print "File %s not found, using default" % filename
            icon = default_icon
    else:
        icon = default_icon
    return icon



def pqa_app_info_block(config, name, version, big_icon_file='', small_icon_file=''):
    return 'lnch' + \
           swap(3) + \
           swap(0) + \
           get_str("%s.0\000" % version) + \
           get_str("%s\000" % name) + \
           get_str(get_icon(config, def_big_icon, "32", "32", big_icon_file)) + \
           get_str(get_icon(config, def_small_icon, "15", "9", small_icon_file))
