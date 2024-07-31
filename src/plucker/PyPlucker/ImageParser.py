#!/usr/bin/env python
#  -*- mode: python; indent-tabs-mode: nil; -*-

"""
ImageParser.py   $Id: ImageParser.py,v 1.60 2004/03/23 21:17:38 chrish Exp $

This defines various classes to parse an image to a PluckerImageDocument.

It will try to identify the best available solution to do so and
define that as a default_parser function.


Some parts Copyright 1999 by Ondrej Palkovsky <ondrap@penguin.cz> and
others Copyright 1999 by Holger Duerer <holly@starship.python.net>

Distributable under the GNU General Public License Version 2 or newer.
"""

import os, sys, string, tempfile, re, operator
try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO
from PyPlucker import PluckerDocs, DEFAULT_IMAGE_PARSER_SETTING
from PyPlucker.UtilFns import message, error


binary_flag = ""
if sys.platform == 'os2':
    binary_flag = 'b'

# This is the maximum size for a simple stored image.
SimpleImageMaxSize = (60 * 1024)


# Match pattern for information in a PNM header file
pnmheader_pattern = re.compile(r"(P[1,4]\n([0-9]+)\s([0-9]+)\n)|(P[2,3,5,6]\n([0-9]+)\s([0-9]+)\n([0-9]+)\n)", re.DOTALL)
geometry_pattern = re.compile(r"([0-9]+)x([0-9]+)\+([0-9]+)\+([0-9]+)")


# Match pattern for information in a MIFF header file
miffrows_pattern = re.compile("\srows=(?P<rows>[0-9]+)\s", re.MULTILINE)
miffcols_pattern = re.compile("\scolumns=(?P<columns>[0-9]+)\s", re.MULTILINE)


#Match pattern for convert version
version_pattern = re.compile("\s([0-9]+).([0-9]+).([0-9]+)[+\s]")


# simple test for checking to see whether a string contains an int
int_pattern = re.compile(r'(^[0-9]+$)|(^0x[0-9a-fA-F]+$)')
def is_int (s):
    return int_pattern.match(s)


#####################################################################
##
## Some standard exceptions used by ImageParser classes.
## 

class UnimplementedMethod(AttributeError):
    def __init__(self, value):
        AttributeError.__init__(self, value)

class ImageSize(ValueError):
    def __init__(self, value):
        ValueError.__init__(self, value)

#####################################################################
##
## A general class to handle image parsing.  Actual functionality
## is provided by a subclass which manipulates images via some
## image-processing toolkit.
## 

class ImageParser:
    """Provides functions needed to properly convert an image to the Plucker image format.
    Actual functionality is provided by subclasses which manipulate images via some
    image-processing toolkit."""

    # an ordered list of known bit depths
    _known_depths = [1, 2, 4, 8, 16]

    def __init__(self, url, type, bits, config, attributes):
        # sys.stderr.write("attributes are " + str(attributes) + "\n")
        self._url = str(url)
        self._type = type
        self._bits = bits
        self._config = config
        self._attribs = attributes
        self._verbose = config.get_int ('verbosity', 1)
        self._auto_scale = config.get_bool('auto_scale_images', 0) or config.get_bool('try_reduce_dimension', 0) or config.get_bool('try_reduce_bpp', 0)

    def size(self):

        "Returns width and height of original image, as integers, in pixels"

        raise UnimplementedMethod("method 'size' not implemented in class " + str(self.__class__))

    def convert(self, width, height, depth, section=None, prescale=None):

        """Takes width, height, and depth, and returns the bits of a Plucker
        format image having that width, height, and depth.  If section is
        defined, it should be a tuple containing 4 elements:
        (upper-left x, upper-left y, width, height), and means that only
        that section of the original image will be converted.
        If prescale is defined, it should be a tuple containing 2 elements:
        (width, height) and means that image should be scaled to this size
        before cropping. Used for building multi-image documents."""

        raise UnimplementedMethod("method 'convert' not implemented in class " + str(self.__class__))


    def calculate_desired_size(self):

        """Returns a tuple of (DESIRED_SIZE, LIMITS, and SCALING_FACTOR),
        where DESIRED_SIZE is a 4-ple of WIDTH (pixels), HEIGHT (pixels),
        BPP, and SECTION, where section is a 4-ple of (ULX, ULY, WIDTH, HEIGHT),
        indicating a subarea of the whole original image that should be
        converted.  SECTION may be None if the whole images is to be taken.
        LIMITS is a 2-ple containing the values for maxwidth and maxheight
        actually used, and SCALING_FACTOR is a number (float or int)
        the original image size was scaled by.
        It should normally never be necessary to override this method."""

        geometry = self._attribs.get('section')
        section = None
        if geometry:
            m = geometry_pattern.match(geometry)
            if m:
                section = (int(m.group(3)), int(m.group(4)), int(m.group(1)), int(m.group(2)))
        bpp = int(self._attribs.get('bpp') or self._config.get_int('bpp', 1))
        maxwidth = int(self._attribs.get('maxwidth') or self._config.get_int ('maxwidth', 0))
        maxheight = int(self._attribs.get('maxheight') or self._config.get_int ('maxheight', 0))
        width = 0
        height = 0

        # First, look for an explicit width and/or height
        w = self._attribs.get('width')
        h = self._attribs.get('height')
        if w and w[-1] != '%':
            width = int(w)
            scaling_factor = 0
        if h and h[-1] != '%':
            height = int(h)
            scaling_factor = 0

        # Next, check for a geometry cut, and use that if present
        if not width and section and section[2]:
            width = section[2]
        if not height and section and section[3]:
            height = section[3]

        # finally, use the current size of the image
        size = self.size()
        if not width:
            width = size[0]
        if not height:
            height = size[1]

        # now scale to fit in maxwidth/maxheight
        scaling_factor = 1
        if (maxwidth and width>maxwidth) or (maxheight and height>maxheight):
            if (not maxheight or (maxwidth and float(width)/maxwidth > float(height)/maxheight)):
                scaling_factor = float(maxwidth)/float(width) 
            else:
                scaling_factor = float(maxheight)/float(height) 
        
        width = int(width * scaling_factor)
        height = int(height * scaling_factor)
        size = (width, height, bpp, section)
        message(2, "desired_size of image is %s with maxwidth=%d, maxheight=%d" % (size, maxwidth, maxheight))
        return (size, (maxwidth, maxheight), scaling_factor)
        

    def _related_images (self, scaled):

        """For in-line images, we sometimes provide an alternate version of the image
        which uses the "alt_maxwidth" and "alt_maxheight" parameters, and which is linked
        to the the smaller version.  There may be other reasons to add alternate versions
        of an image, too.  This method generates a list of (URL, ATTRIBUTES) pairs, each
        describing a desired alternate version of the current image, and returns them."""

        new_sizes = []

        if self._attribs.get('_plucker_from_image') and scaled:
            # this is an in-line image.  Let's create the right attributes for a larger
            # version.
            if self._config.get_string('alt_maxwidth') or self._config.get_string('alt_maxheight'):
                # We can turn off alt images by maxing either one or the other to a string of "-1".
                if self._config.get_string('alt_maxwidth') != "-1" and self._config.get_string('alt_maxheight') != "-1":
                    new_attributes = self._attribs.copy()
                    del new_attributes['_plucker_from_image']
                    new_attributes['_plucker_alternate_image'] = 1
                    if self._config.get_string('alt_maxwidth', None):
                        new_attributes['maxwidth'] = self._config.get_string('alt_maxwidth', None)
                    if self._config.get_string('alt_maxheight', None):
                        new_attributes['maxheight'] = self._config.get_string('alt_maxheight', None)
                    new_sizes.append((self._url, new_attributes))

        return new_sizes


    def get_plucker_doc(self):

        """Returns the PluckerDocs.PluckerImageDocument associated with this image, converting
        the original image along the way, if necessary.  It should normally never be necessary
        to override this method."""

        (width, height, depth, section), limits, scaling_factor = self.calculate_desired_size()
        message(2, "Converting image %s with %s" % (self._url, str(self.__class__)))
        newbits = self.convert(width, height, depth, section)

        if len(newbits) > SimpleImageMaxSize and self._auto_scale:

            if self._config.get_bool('try_reduce_bpp', 0) and depth in self._known_depths:
                # try to reduce the depth while keeping the size
                i = self._known_depths.index(depth)
                while (i > 0) and (len(newbits) > SimpleImageMaxSize):
                    i = i - 1
                    olddepth = depth
                    depth = self._known_depths[i]
                    message(2, "Plucker version of image at %dx%dx%d was"
                            " %.0f%% too large, trying depth of %d...\n",
                            width, height, olddepth,
                            (float(len(newbits)-SimpleImageMaxSize)/
                             float(SimpleImageMaxSize)) * 100.0, depth)
                    newbits = self.convert(width, height, depth, section)

            elif (self._config.get_bool('try_reduce_dimension', 0) or
                  self._config.get_bool('auto_scale_images')):
                # try to reduce the image size to fit in a Plucker record
                import math
                target_size = SimpleImageMaxSize
                while len(newbits) > SimpleImageMaxSize:
                    old_target_size = target_size
                    target_size = 0.95 * target_size
                    scaling_factor = math.sqrt(float(target_size)/float(len(newbits)))
                    message(2, "Plucker version of image at %dx%d was "
                            "%.0f%% too large, trying %dx%d...\n",
                            width, height,
                            (float(len(newbits)-old_target_size)/float(old_target_size)) * 100.0,
                            int(scaling_factor * width),
                            int(scaling_factor * height))
                    width = int(scaling_factor * width)
                    height = int(scaling_factor * height)
                    newbits = self.convert(width, height, depth, section)
            else:
                message(2, "You aren't using a try_reduce_depth=1 nor a "
                        "try_reduce_dimension=1. Will proceed to multiimage..")

        if len(newbits) == 0:
            # Oops, nothing fetched?!?
            raise ImageSize("Converted image size for %s is zero bytes. Nothing fetched?" % self._url)

        elif len(newbits) > SimpleImageMaxSize:
            # image bits too large for a _SINGLE_ Plucker image record
            return self.write_multiimage(width, height, depth)

        newurl = "%s?width=%d&height=%d&depth=%d" % (self._url, width, height, depth)
        if section:
            newurl = newurl + "&section=%dx%d+%d+%d" % section
        doc = PluckerDocs.PluckerImageDocument (newurl, self._config)
#       doc = PluckerDocs.PluckerImageDocument (self._url, self._config)
        doc.set_data(newbits)
        # check for alternative versions of this image
        # First, figure out if the image has been scaled down at all
        full_width, full_height = self.size()
        attrib_width = self._attribs.has_key('width') and is_int(self._attribs.get('width')) and int(self._attribs.get('width'))
        attrib_height = self._attribs.has_key('height') and is_int(self._attribs.get('height')) and int(self._attribs.get('height'))
        versions = self._related_images(width < (attrib_width or (section and section[2]) or full_width) or
                                        height < (attrib_height or (section and section[3]) or full_height))
        map(lambda x, doc=doc: doc.add_related_image(x[0], x[1]), versions)
        return doc


    def write_multiimage(self, width, height, depth):
        """Write a multi image record!"""

        if depth == 1:
            wide = 800
            high = 600
        elif depth == 2:
            wide = 600
            high = 400
        elif depth == 4:
            wide = 400
            high = 300
        elif depth == 8:
            wide = 300
            high = 200
        else:
            wide = 300
            high = 100

        cols = width / wide
        if width % wide:
            cols = cols + 1

        rows = height / high
        if height % high:
            rows = rows + 1

        multiurl = "%s?width=%d&height=%d&depth=%d" % (self._url, width, height, depth)
        doc = PluckerDocs.PluckerMultiImageDocument (multiurl, self._config)
        doc.set_size(cols, rows)

        count = 0
        Y = 0
        X = 0

        while Y < height:
            while X < width:
                W = min(wide, width - X)
                H = min(high, height - Y)

                piece_url = "%sMulti%d?width=%d&height=%d&depth=%d" % (self._url, count, width, height, depth)
                piece_doc = PluckerDocs.PluckerImageDocument (piece_url, self._config)
                bits = self.convert(W, H, depth, (X, Y, W, H), (width, height))
                piece_doc.set_data(bits)
                id = PluckerDocs.obtain_fresh_id()
                doc.add_piece_image(piece_doc, id)
                count = count + 1
                X = X + wide
            X = 0
            Y = Y + high

        # check for alternative versions of this image
        # First, figure out if the image has been scaled down at all
        full_width, full_height = self.size()
        versions = self._related_images(width < full_width or height < full_height)
        map(lambda x, doc=doc: doc.add_related_image(x[0], x[1]), versions)
        return doc



#####################################################################
##
## This is the Image Magick parser from Chris.  It depends on os.popen
## and the availability of ImageMagick (convert) plus the Tbmp-tools
##
##

class ImageMagickImageParser:
    "Convert an image to the PalmBitmap. Uses 'convert' from ImageMagick"

    def __init__(self, url, type, data, config, attribs, compress=1):
        self._url = url
        self._verbose = config.get_int ('verbosity', 1)
        self._config = config
        self._maxwidth = attribs.get('maxwidth')
        self._maxheight = attribs.get('maxheight')
        self._bpp = attribs.get('bpp')
        self._doc = PluckerDocs.PluckerImageDocument (str(url), config)
        self._scaled = 0

        tmpfile = tempfile.mktemp()

        try:
            try:
                size_data = self.convert_to_pnm(data, tmpfile)
                self._doc.set_data(self.convert_to_Tbmp(tmpfile))
                if self._verbose > 1:
                    sys.stderr.write("input image was " + str(size_data[0]) + ", maxwidth/height used was " + str(size_data[1]) + ", output image size is " + str(size_data[2]) + "\n")
            finally:
                try: os.unlink(tmpfile)
                except: pass
        except RuntimeError:
            # This we pass through...
            raise
        except:
            raise RuntimeError, "Error while converting image (%s)." % self._url


    def get_plucker_doc(self):
        return self._doc


    def scaled(self):
        return self._scaled


    def convert_to_Tbmp(self, tmpfile):

        ppmtoTbmp = self._config.get_string ('ppmtoTbmp_program', 'ppmtoTbmp')
        if self._bpp == 2:
            ppmtoTbmp = ppmtoTbmp + " -2bit"
        elif self._bpp == 4:
            ppmtoTbmp = ppmtoTbmp + " -4bit"

        if self._verbose <= 1:
            ppmtoTbmp = ppmtoTbmp + " -quiet"

        outfile = tempfile.mktemp()

        command = ppmtoTbmp + " " + tmpfile + " > " + outfile

        if self._verbose > 1:
            message(2, "Running: ", command)

        try:
            if os.system (command):
                raise RuntimeError, "Error while executing command '%s'" % command
            f = open (outfile, "rb")
            data = f.read ()
            f.close ()

            if len (data) == 0:
                # Oops, nothing fetched?!?
                raise RuntimeError, "No data from parsing image! (%s)" % self._url
            elif len(data) > SimpleImageMaxSize:
                raise RuntimeError, "Image data too large (%d bytes)!  Scale it down." % len(data)
        finally:
            try: os.unlink(outfile) 
            except: pass
        return data


    def convert_to_pnm(self, data, tmpfile):        

        infile = tempfile.mktemp()

        try:
            #message(0, "Open infile " + infile)
            f = open(infile, "wb")
            f.write(data)
            f.close()

            convert = self._config.get_string ('convert_program', 'convert')
            maxwidth = self._config.get_string ('maxwidth', '150')
            maxheight = self._config.get_string ('maxheight', '250')

            if self._maxheight != None:
                maxheight = self._maxheight
            if self._maxwidth != None:
                maxwidth = self._maxwidth
        
            command = "identify -ping " + infile
            message("Running: " + command)
            pipe = os.popen(command)
            info = pipe.read()
            pipe.close()
            match = re.search(r"\s([0-9]+)x([0-9]+)[+\s]", info)
            if not match:
                raise RuntimeError, "Can't determine image size from output of ImageMagick 'identify' program:  " + info
            else:
                mywidth = int(match.group(1))
                myheight = int(match.group(2))
            if mywidth > int(maxwidth) \
                    or myheight > int(maxheight):
                self._scaled = 1
                if self._verbose > 2: print '...image (natively %dx%d) must be scaled' % (mywidth, myheight)

            size = "\"" + maxwidth + "x" + maxheight + ">\""

            if self._bpp == 1:
                ncolors = "2"
            elif self._bpp == 2:
                ncolors = "4"
            else:
                ncolors = "16"

            if self._bpp == 1:
                convert = convert + " -monochrome"

            command = convert + " -colors " + ncolors + " -dither -geometry " + size + " " + infile +" ppm:" + tmpfile

            if self._verbose > 1:
                print "Running: ", command
            if os.system (command):
                raise RuntimeError, "Error while executing command '%s'" % command

            return ((mywidth, myheight), (int(maxwidth), int(maxheight)), (int(maxwidth), int(maxheight)))

        finally:
            try: os.unlink(infile) 
            except: pass



#####################################################################
##
## This is a new version of the ImageMagick parser that relies on the
## built-in Palm image format support added in IM 5.4.4.
##

class NewImageMagickImageParser(ImageParser):
    "Convert an image to the PalmBitmap. Uses ImageMagic 5.4.4 or later."    
    "Takes advantage of changes in ImageMagick 5.5.8 if available."

    def __init__(self, url, type, data, config, attribs, compress=1):
        ImageParser.__init__(self, url, type, data, config, attribs)
        self._size = None
        self._miffdata = None
        self._tmpfile = tempfile.mktemp()
        self._convert_version = 0
        try:
            self._convert_to_miff()
        except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError("Error while opening image " + self._url + " with ImageMagick")


    def _convert_to_miff(self):

        command = self._config.get_string ('imagemagick_convert_command', 'convert')

        # so convert to MIFF bits by running the appropriate command
        if command == None:
            # already in MIFF format, skip conversion
            self._miffdata = self._bits
        else:
            try:
                version_command = command + " -version"
                pipe = os.popen(version_command, "r"+binary_flag)
                version_string = pipe.read()
                pipe.close()
                m = version_pattern.search(version_string)
                if m:
                    self._convert_version = m.group(1) + m.group(2) + m.group(3)

                f = open(self._tmpfile, 'w'+binary_flag)
                f.write(self._bits)
                f.close()

                # [0] means use only the first image (animated gif, etc)
                command = command + " %s[0] miff:-" % self._tmpfile
                if self._verbose > 1:
                    message(2, "Running:  " + command)
                else:
                    command = "( " + command + " ) 2>/dev/null"
                pipe = os.popen(command, "r"+binary_flag)
                self._miffdata = pipe.read()
                status = pipe.close()
                if status:
                    raise RuntimeError("call to '" + command + "' returned status " + str(status))
            finally:
                if os.path.exists(self._tmpfile): os.unlink(self._tmpfile)

        # now read the width and height from the MIFF data
        miffrows = miffrows_pattern.search(self._miffdata)
        miffcols = miffcols_pattern.search(self._miffdata)
        if not miffrows or not miffcols:
            header_end = string.find (self._miffdata, ":\x1a")
            if header_end >= 0:
                header = self._miffdata[:header_end]
            else:
                header = str(self._miffdata[:min(len(self._miffdata),30)])
            raise RuntimeError("Invalid MIFF header found in converted MIFF data:  %s" % header)
        self._size = (int(miffcols.group('columns')), int(miffrows.group('rows')))


    def size (self):
        return self._size


    def convert (self, width, height, bpp, section, prescale = None):

        convert_command = self._config.get_string ('imagemagick_convert_command', 'convert')

        if prescale:
            convert_command = convert_command + " -resize %dx%d\\! " % (prescale)

        if section:
            convert_command = convert_command + (" -crop %dx%d+%d+%d " % (section[2], section[3], section[0], section[1]))
            size = (section[2], section[3])
        else:
            size = self._size

        if width != size[0] or size[1] != height:
            message(2, "Scaling original %dx%d image by %f,%f to %dx%dx%d" % (size[0], size[1], float(width)/float(size[0]), float(height)/float(size[1]), width, height, bpp))
            convert_command = convert_command + " -resize %dx%d\\! " % (width, height)

        # -depth (100 + X) forces bit depth = X (ImageMagick uses least bits)
        if bpp == 1:
            convert_command = convert_command + " -monochrome -dither "
        elif bpp == 2:
            convert_command = convert_command + " -colorspace gray -colors 4 "
            if self._convert_version >= 558:
                convert_command = convert_command + " -depth 102 "
        elif bpp == 4:
            convert_command = convert_command + " -colorspace gray -colors 16 "
            if self._convert_version >= 558:
                convert_command = convert_command + " -depth 104 "
        elif bpp == 8:
            convert_command = convert_command + " -colors 256 "
            if self._convert_version >= 558:
                convert_command = convert_command + " -depth 108 "
            else:
                convert_command = convert_command + " +comment -comment colormap "
        elif bpp == 16:         # direct color
            convert_command = convert_command + " -colors 65536 "
            if self._convert_version >= 558:
                convert_command = convert_command + " -depth 116 " 
        else:
            raise RuntimeError("ImageMagick2 image parser can't handle bpp value of %d" % bpp)

        if self._verbose > 1:
            convert_command = convert_command + " -verbose"

        convert_command = convert_command + " - palm:" + self._tmpfile
        if self._verbose > 1:
            message(2, "Running:  " + convert_command)
        else:
            convert_command = "( " + convert_command + " ) 2>/dev/null"
        try:
            pipe = os.popen(convert_command, 'w'+binary_flag)
            pipe.write(self._miffdata)
            status = pipe.close()
            if status:
                raise RuntimeError("call to '" + convert_command + "' returned status " + str(status))
            f = open(self._tmpfile, 'r'+binary_flag)
            newbits = f.read()
            f.close()
            return newbits
        finally:
            if os.path.exists(self._tmpfile): os.unlink(self._tmpfile)


## This is the standard parser from Ondrej.  It depends on os.popen
## and the availability of the pbmtools plus the Tbmp-tools
##
##


class NetPBMImageParser:
    "Convert an image to the PalmBitmap. Uses netpbm."    

    def __init__(self, url, type, data, config, attribs, compress=1):
        self._type = type
        self._url = url
        self.scale = 0
        self.width = 1
        self.height = 1
        self._verbose = config.get_int ('verbosity', 1)
        self._config = config
        self._maxwidth = attribs.get('maxwidth')
        self._maxheight = attribs.get('maxheight')
        self._bpp = attribs.get('bpp')
        self._doc = PluckerDocs.PluckerImageDocument(str(url), config)
        self._scaled = 0

        tmpfile = tempfile.mktemp()

        try:
            self.convert_to_pnm(data, tmpfile)
            self.fetch_info(tmpfile)
            size_data = self.define_scale()
            self._doc.set_data(self.convert_to_Tbmp(tmpfile))        
        except:
            try: os.unlink(tmpfile)
            except: pass
            raise RuntimeError, "Error while converting image."
        try: os.unlink(tmpfile)
        except: pass


    def get_plucker_doc(self):
        return self._doc


    def scaled(self):
        return self._scaled


    def convert_to_Tbmp(self, tmpfile):
        ppmquant = self._config.get_string ('ppmquant_program', 'ppmquant')
        ppmtoTbmp = self._config.get_string ('ppmtoTbmp_program', 'ppmtoTbmp')
        if self._bpp == 1:
            ppmquant = ppmquant + " -fs 2 "
        elif self._bpp == 2:
            ppmquant = ppmquant + " -fs 4 "
            ppmtoTbmp = ppmtoTbmp + " -2bit"
        else:
            ppmquant = ppmquant + " -fs 16 "
            ppmtoTbmp = ppmtoTbmp + " -4bit"

        if self._verbose <= 1:
            ppmtoTbmp = ppmtoTbmp + " -quiet "
            ppmquant = ppmquant + " -quiet "

        if not self.scale:
            command = ppmquant  + tmpfile + "|" + ppmtoTbmp
        else:
            command = self.scale + tmpfile + "|" + ppmquant + "|" + ppmtoTbmp

        if self._verbose > 1:
            print "Running: ", command
        else:
            command = "( " + command + " ) 2>/dev/null"
        pipe = os.popen(command, "r"+binary_flag)
        data = pipe.read()
        pipe.close()
        if len (data) == 0:
            # Oops, nothing fetched?!?
            raise RuntimeError, "No data from parsing image! (%s)" % self._url
        elif len(data) > SimpleImageMaxSize:
            raise RuntimeError, "Image data too large (%d bytes)!  Scale it down." % len(data)
        return data


    def define_scale(self):
        maxwidth = self._config.get_int ('maxwidth', 150)
        maxheight = self._config.get_int ('maxheight', 250)
        pnmscale = self._config.get_string ('pnmscale_program', 'pnmscale')

        if self._maxwidth != None:
            maxwidth = int(self._maxwidth)
        if self._maxheight != None:
            maxheight = int(self._maxheight)

        if (self.width>maxwidth or self.height>maxheight):
            self._scaled = 1
            if (float(self.width)/maxwidth > float(self.height)/maxheight):
                self.scale = pnmscale + (" -xsize %d " % maxwidth)
                size = (self.width * float(maxwidth)/float(self.width), self.height * float(maxwidth)/float(self.width))
            else:
                self.scale = pnmscale + (" -ysize %d " % maxheight)
                size = (self.width * float(maxheight)/float(self.height), self.height * float(maxheight)/float(self.height))
        else:
            size = (self.width, self.height)
        return ((self.width, self.height), (maxwidth, maxheight), size)


    def fetch_info(self, tmpfile):
        pnmfile = self._config.get_string ('pnmfile_program', 'pnmfile')
        pipe = os.popen(pnmfile + " " + tmpfile, "r")
        info = pipe.read()
        pipe.close()
        info = string.split(info) #info is now an array
        self.width = int(info[3])
        self.height = int(info[5])        


    def convert_to_pnm(self, data, tmpfile):        
        giftopnm = self._config.get_string ('giftopnm_program', 'giftopnm')
        djpeg = self._config.get_string ('djpeg_program', 'djpeg')
        pngtopnm = self._config.get_string ('pngtopnm_program', 'pngtopnm')
        if (self._type=='image/gif'):
            command = giftopnm + ' >' + tmpfile
        elif (self._type=='image/jpeg'):
            command = djpeg + ' -pnm ' + ' >' + tmpfile
        elif (self._type=='image/png'):
            command = pngtopnm + ' >' + tmpfile
        if self._verbose > 1:
            print "Running: ", command
        pipe = os.popen(command, "w"+binary_flag)
        pipe.write(data)
        pipe.close()



#####################################################################
##
## This is an updated version of the standard parser from Ondrej.  It depends on os.popen
## and the availability of the pbmtools plus the updated Tbmp-tools that can handle color.
##

class NewNetPBMImageParser(ImageParser):
    "Convert an image to the PalmBitmap. Uses netpbm."    

    def __init__(self, url, type, data, config, attribs, compress=1):
        ImageParser.__init__(self, url, type, data, config, attribs)
        self._size = None
        self._pnmdata = None
        self._tmpfile = tempfile.mktemp()
        try:
            self._convert_to_pnm()
        except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError("Error while opening image " + self._url + " with netpbm")


    def _convert_to_pnm(self):

        giftopnm = self._config.get_string ('giftopnm_program', 'giftopnm')
        djpeg = self._config.get_string ('djpeg_program', 'djpeg')
        pngtopnm = self._config.get_string ('pngtopnm_program', 'pngtopnm')
        palmtopnm = self._config.get_string ('palmtopnm_program', 'palmtopnm')
        if (self._type=='image/gif'):
            command = giftopnm
        elif (self._type=='image/jpeg'):
            command = djpeg + ' -pnm'
        elif (self._type=='image/png'):
            command = pngtopnm
        elif (self._type=='image/palm'):
            command = palmtopnm
        elif (self._type=='image/pbm') or (self._type == 'image/x-portable-pixmap') or (self._type == 'image/x-portable-anymap'):
            command = None
        else:
            raise ValueError("unsupported image type " + self._type + " encountered")

        # so convert to PNM bits by running the appropriate command
        if command == None:
            # already in PNM format, skip conversion
            self._pnmdata = self._bits
        else:
            try:
                command = command + " > " + self._tmpfile
                if self._verbose > 1:
                    message(2, "Running:  " + command)
                else:
                    command = "( " + command + " ) 2>/dev/null"
                pipe = os.popen(command, "w"+binary_flag)
                pipe.write(self._bits)
                status = pipe.close()
                if status:
                    raise RuntimeError("call to '" + command + "' returned status " + str(status))
                f = open(self._tmpfile, 'r'+binary_flag)
                self._pnmdata = f.read()
                f.close()
            finally:
                if os.path.exists(self._tmpfile): os.unlink(self._tmpfile)

        # now read the width and height from the PNM data
        m = pnmheader_pattern.match(self._pnmdata)
        if not m:
            raise RuntimeError("Invalid PNM header found in converted PNM data:  %s" % str(self._pnmdata[:min(len(self._pnmdata),15)]))
        if m.group(1):
            # monochrome, so no depth element
            self._size = (int(m.group(2)), int(m.group(3)))
        else:
            # greyscale or color, so use second group
            self._size = (int(m.group(5)), int(m.group(6)))


    def size (self):
        return self._size


    def convert (self, width, height, bpp, section, prescale=None):

        pnmscale = self._config.get_string ('pnmscale_program', 'pnmscale')
        pnmcut = self._config.get_string ('pnmcut_program', 'pnmcut')
        ppmquant = self._config.get_string ('ppmquant_program', 'ppmquant')
        ppmtoTbmp = self._config.get_string ('ppmtoTbmp_program', 'pnmtopalm')
        ppmtopgm = self._config.get_string ('ppmtopgm_program', 'ppmtopgm')
        pgmtopbm = self._config.get_string ('pgmtopbm_program', 'pgmtopbm')
        palm1gray = self._config.get_string ('palm1bit_graymap_file', 'palmgray1.map')
        palm2gray = self._config.get_string ('palm2bit_graymap_file', 'palmgray2.map')
        palm4gray = self._config.get_string ('palm4bit_graymap_file', 'palmgray4.map')
        #palm8color = self._config.get_string ('palm8bit_stdcolormap_file', 'palmcolor8.map')
        palm8color = self._config.get_string ('palm8bit_stdcolormap_file', '/usr/share/netpbm/palmcolor8.map')

        if prescale:
            prescale_cmd = pnmscale + " -width %d -height %d | " % (prescale)
        else:
            prescale_cmd = None

        if section:
            pnmcut_cmd = pnmcut + (" %d %d %d %d |" % section)
            size = (section[2], section[3])
        else:
            pnmcut_cmd = ""
            size = self._size

        if width != size[0] or size[1] != height:
            message(2, "Scaling original %dx%d image by %f,%f to %dx%dx%d" % (size[0], size[1], float(width)/float(size[0]), float(height)/float(size[1]), width, height, bpp))

        if (size[0] != width or size[1] != height):
            scale_cmd = pnmscale + " -width %d -height %d " % (width, height)
        else:
            scale_cmd = None

        if bpp == 1:
            ppmquant = ppmtopgm
            ppmquant2 = "|" + pgmtopbm + " -fs "
            ppmtoTbmp = ppmtoTbmp + " -depth 1"
        elif bpp == 2:
            ppmquant2 = " -map " + palm2gray + "|" + ppmtopgm
            ppmtoTbmp = ppmtoTbmp + " -depth 2"
        elif bpp == 4:
            ppmquant2 = " -map " + palm4gray + "|" + ppmtopgm
            ppmtoTbmp = ppmtoTbmp + " -depth 4"
        elif bpp == 8:
            ppmquant2 = " -map " + palm8color + " "
            ppmtoTbmp = ppmtoTbmp + " -depth 8"
        elif bpp == 16:        # direct color
            ppmquant = "cat"
            ppmquant2 = ""
            ppmtoTbmp = ppmtoTbmp + " -depth 16"
        else:
            raise RuntimeError("Can't handle bpp value of %d" % bpp)

        if self._verbose > 1:
            ppmtoTbmp = ppmtoTbmp + " -verbose "
        else:
            ppmtoTbmp = ppmtoTbmp + " -quiet "
            ppmquant = ppmquant + " -quiet "

        if not scale_cmd:
            command = ppmquant + ppmquant2 + "|" + ppmtoTbmp
        else:
            command = scale_cmd + "|" + ppmquant + ppmquant2 + "|" + ppmtoTbmp
        if pnmcut_cmd:
            command = pnmcut_cmd + command
        if prescale_cmd:
            command = prescale_cmd + command

        command = command + " > " + self._tmpfile
        if self._verbose > 1:
            message(2, "Running:  " + command)
        else:
            command = "( " + command + " ) 2>/dev/null"
        try:
            pipe = os.popen(command, 'w'+binary_flag)
            pipe.write(self._pnmdata)
            status = pipe.close()
            if status:
                raise RuntimeError("call to '" + command + "' returned status " + str(status))
            f = open(self._tmpfile, 'r'+binary_flag)
            newbits = f.read()
            f.close()
            return newbits
        finally:
            if os.path.exists(self._tmpfile): os.unlink(self._tmpfile)


#####################################################################
##
## This is a parser that relies on the Python Imaging Library (PIL)
## to do all the work (except converting to Tbmp format at the end).
## 
##
##
class PythonImagingLibraryParser:

    "Convert an image to the PalmBitmap. Uses Python Imaging Library."

    def __init__(self, url, type, data, config, attribs, compress=1):
        import Image
        self._url = url
        self._verbose = config.get_int ('verbosity', 1)
        self._config = config
        self._maxwidth = attribs.get('maxwidth')
        self._maxheight = attribs.get('maxheight')
        self._bpp = attribs.get('bpp')
        self._doc = PluckerDocs.PluckerImageDocument (str(url), config)
        self._scaled = 0

        tmpfile = tempfile.mktemp()

        if self._bpp == 1:
            colors = 2
        elif self._bpp == 2:
            colors = 4
        else:
            colors = 16

        try:
            if self._verbose > 1:
                print "Converting image %s with PIL" % url
            im = Image.open (StringIO (data))
            self.width, self.height = im.size
            scaling_factor, size_data = self.define_scale()
            if scaling_factor != 0:
                self._scaled = 1
                if self._verbose > 1:
                    print "Scaling image by " + str(scaling_factor)
                im.thumbnail((int(self.width * scaling_factor),
                                   int(self.height * scaling_factor)))
                if self._verbose > 1:
                    print "New size is", im.size
            im = im.convert ("L", colors=colors)
            try:
                apply(im.save, (tmpfile, "PPM"))
                self._doc.set_data (self.convert_to_Tbmp (tmpfile))
            finally:
                try: os.unlink(tmpfile)
                except: pass
        except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError, "Error while converting image."


    def get_plucker_doc(self):
        return self._doc


    def scaled(self):
        return self._scaled


    def define_scale(self):
        scaling_factor = 0
        maxwidth = self._config.get_int ('maxwidth', 150)
        maxheight = self._config.get_int ('maxheight', 250)

        if self._maxwidth != None:
            maxwidth = int(self._maxwidth)
        if self._maxheight != None:
            maxheight = int(self._maxheight)

        if (self.width>maxwidth or self.height>maxheight):
            if (float(self.width)/maxwidth > float(self.height)/maxheight):
                scaling_factor = float(maxwidth)/self.width
            else:
                scaling_factor = float(maxheight)/self.height
            size = (scaling_factor * self.width, scaling_factor * self.height)
        else:
            size = (self.width, self.height)
        return scaling_factor, ((self.width, self.height), (maxwidth, maxheight), size)

    def convert_to_Tbmp(self, tmpfile):
        ppmtoTbmp = self._config.get_string ('ppmtoTbmp_program', 'ppmtoTbmp')
        pnmdepth = self._config.get_string('pnmdepth_program', 'pnmdepth')
        ppmtopgm = self._config.get_string('ppmtopgm_program', 'ppmtopgm')
        pgmtopbm = self._config.get_string('pgmtopbm_program', 'pgmtopbm')
        if self._bpp == 1:
            ppmquant = pgmtopbm + " | " + pgmtopbm + " -fs "
        elif self._bpp == 2:
            ppmquant = pnmdepth + " 3 | " + ppmtopgm
            ppmtoTbmp = ppmtoTbmp + " -2bit"
        elif self._bpp == 4:
            ppmquant = pnmdepth + " 15 | " + ppmtopgm
            ppmtoTbmp = ppmtoTbmp + " -4bit"
        else:
            raise RuntimeError, "Can't handle depth of " + str(self._bpp)

        if self._verbose <= 1:
            ppmtoTbmp = ppmtoTbmp + " -quiet "
            ppmquant = ppmquant + " -quiet "

        command = "cat " + tmpfile + " | " + ppmquant + " |" + ppmtoTbmp

        if self._verbose > 1:
            print "Running: ", command
        else:
            command = "( " + command + " ) 2>/dev/null"
        pipe = os.popen(command, "r"+binary_flag)
        data = pipe.read()
        pipe.close()
        if len (data) == 0:
            # Oops, nothing fetched?!?
            raise RuntimeError, "No data from parsing image! (%s)" % self._url
        elif len(data) > SimpleImageMaxSize:
            raise RuntimeError, "Image data too large (%d bytes)!  Scale it down." % len(data)
        return data


#####################################################################
##
## This is a parser that relies on the Python Imaging Library (PIL)
## to do *all* the work.
##
##
class NewPythonImagingLibraryParser(ImageParser):

    "Convert an image to the PalmBitmap. Uses Python Imaging Library."

    def __init__(self, url, type, data, config, attribs, compress=1):
        # ---Start of OS X additions---
        # On Macintosh OSX >10.2, there is Python2.2 preinstalled, but we distribute PIL 
        # in the Plucker app bundle, since messing with the system annoys end users.
        # So we distribute a /vm/PIL directory at same level as the /PyPlucker directory.
        if sys.platform == 'darwin' or sys.platform == 'mac':        
            # Get the full filename of the file
            file = sys.argv[0]
            # Resolve the link target if it is a link instead of a real file
            while os.path.islink (file): 
                file = os.readlink (file)
            path_to_bundle_resources = os.path.split (os.path.dirname (file))
            path_to_pil_string = path_to_bundle_resources[0] + '/vm/PIL'
            # Check that the last member of list isn't path to PIL, so don't keep adding it.
            if sys.path[-1] != path_to_pil_string:
                sys.path.append (path_to_pil_string)
        # ---End of OS X additions ---
        # Now try to import the Image module from PIL
        import Image
        # Import the Image plugin for Palm images
        import PalmImagePlugin
        ImageParser.__init__(self, url, type, data, config, attribs)
        self._supports_color = 0
        try:
            quantize_defaults = Image.Image.quantize.im_func.func_defaults
            if len(quantize_defaults) > 3 and quantize_defaults[3] == None:
                self._supports_color = 1
        except:
            pass
        try:
            self._image = Image.open (StringIO (data))
        except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError("Error while opening image " + self._url + " with PIL")

    def _convert_to_Tbmp(self, im, pil_mode, bpp):
        import StringIO
        from PalmImagePlugin import Palm8BitColormapImage
        palmdata = StringIO.StringIO()
        if pil_mode == "1" or bpp == 1:
            im.convert("L").convert("1").save(palmdata, "Palm", bpp=bpp)
        elif pil_mode == "L":
            im.convert(pil_mode).save(palmdata, "Palm", bpp=bpp)
        elif pil_mode == "P" and bpp == 8:
            im.convert("RGB").quantize(palette=Palm8BitColormapImage).save(palmdata, "Palm", bpp=8)
        elif pil_mode == "RGB" and bpp == 16:
            im.convert("RGB").save(palmdata, "Palm", bpp=bpp)
        else:
            raise KeyError("Unsupported PIL mode " + pil_mode + " passed to convert.Tbmp")
        data = palmdata.getvalue()
        palmdata.close()
        return data

    def size(self):
        return self._image.size

    def convert(self, width, height, bpp, section, prescale=None):
        try:
            im = self._image
            if prescale:
                im = im.resize(prescale)
            if section:
                im = im.crop((section[0], section[1],
                                       section[0] + section[2], section[1] + section[3]))
            if width != im.size[0] or height != im.size[1]:
                message(2, "Scaling original %dx%d image by %f to %dx%dx%d" % (im.size[0], im.size[1], float(width)/float(im.size[0]), width, height, bpp))
                im = im.resize((width, height))
            if bpp == 1:
                return self._convert_to_Tbmp (im, "1", 1)
            elif bpp in (2, 4):
                return self._convert_to_Tbmp (im, "L", bpp)
            elif bpp == 8 and self._supports_color:
                return self._convert_to_Tbmp (im, "P", bpp)
            elif bpp == 16:
                return self._convert_to_Tbmp (im, "RGB", bpp)
            else:
                message(0, "%d bpp images not supported with PIL imaging yet.  Using 4 bpp grayscale.\n" % (bpp,))
                return self._convert_to_Tbmp (im, "L", 4)
        except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError("Error while converting image " + self._url + " with PIL")


#####################################################################
##
## This is a parser for Windows systems.  It relies on a 
## and the availability of the ImageMagic plus the Tbmp tools
##
##
class WindowsImageParser:
    """Do it on Windows.  Ask Dirk Heiser <plucker@dirk-heiser.de> if
    these tools goof up.  I cannot test it."""
    def __del__ (self):
        self.DeleteTempFiles(self._temp_files)



    def __init__ (self, url, type, data, config, attribs, compress=1):
        # The Result
        self._doc = None
        self._scaled = 0

        #Init some variables
        self._config = config
        self._max_tbmp_size = config.get_int ('max_tbmp_size', SimpleImageMaxSize)
        self._guess_tbmp_size = config.get_bool ('guess_tbmp_size', 1)
        self._try_reduce_bpp = config.get_bool ('try_reduce_bpp', 1)
        self._try_reduce_dimension = config.get_bool ('try_reduce_dimension', 1)
        self._bpp = attribs.get('bpp')
        self._type = type
        self._verbose = config.get_int ('verbosity', 1)
        self._imagemagick = "%s %s" % (self.quotestr(config.get_string ('convert_program','convert.exe')),config.get_string ('convert_program_parameter','%input% bmp:%output%'))
        self._bmp2tbmp = "%s %s" % (self.quotestr(config.get_string ('bmp_to_tbmp', 'Bmp2Tbmp.exe')),config.get_string ('bmp_to_tbmp_parameter','-i=%input% -o=%output% -maxwidth=%maxwidth% -maxheight=%maxheight% -compress=%compress% -bpp=%colors%'))
        if compress:
            self._compress = self._config.get_bool('tbmp_compression', 0)
        else:
            self._compress = 0
        maxwidth = attribs.get('maxwidth')
        maxheight = attribs.get('maxheight')
        if maxwidth == None:
            self._maxwidth = config.get_int ('maxwidth', 150)
        else:
            self._maxwidth = int("%s" % maxwidth)
        if maxheight == None:
            self._maxheight = config.get_int ('maxheight', 150)
        else:
            self._maxheight = int("%s" % maxheight)
        # Create Temp Files
        self._temp_files = self.CreateTempFiles(3)
        # Some globals
        self._scale_step = 10          # Start with 100%, then 90% ...
        self._org_width = 0            # There will be the orginal width of the input file later
        self._org_height = 0           # There will be the orginal heifht of the input file later


        # write the data to the in temp file
        f = open (self._temp_files[0], "wb")
        f.write (data)
        f.close ()


        self._org_width, self._org_height = self.convert_to_bmp(self._temp_files[0], self._temp_files[1])


        if self._guess_tbmp_size == 0:
            data = self.convert_to_Tbmp(self._temp_files[1], self._temp_files[2])
            if self._try_reduce_bpp and (len(data) > self._max_tbmp_size):
                while (len(data) > self._max_tbmp_size) and (self._bpp > 1):
                    self._bpp = self._bpp / 2
                    if self._verbose > 1:
                        print "Bitmap to large, try with bpp: %s" % self._bpp
                    data = self.convert_to_Tbmp(self._temp_files[1], self._temp_files[2])
            if self._try_reduce_dimension and (len(data) > self._max_tbmp_size):
                while (len(data) > self._max_tbmp_size) and (self._scale_step > 1):
                    self._scale_step = self._scale_step - 1
                    if self._verbose > 1:
                        print "Bitmap to large, try with scale: %s%%" % (self._scale_step * 10)
                    data = self.convert_to_Tbmp(self._temp_files[1], self._temp_files[2])
        else:
            if self._try_reduce_bpp:
                guessed_size = self.fake_convert_to_Tbmp()
                if self._verbose > 2:
                    print "Guessed TBmp Size: %s Bytes" % guessed_size
                while ( guessed_size > self._max_tbmp_size) and (self._bpp > 1):
                    self._bpp = self._bpp / 2
                    if self._verbose > 1:
                        print "Bitmap to large, try with bpp: %s" % self._bpp
                    guessed_size = self.fake_convert_to_Tbmp()
                    if self._verbose > 2:
                        print "Guessed TBmp Size: %s Bytes" % guessed_size
            if self._try_reduce_dimension and (len(data) > self._max_tbmp_size):
                guessed_size = self.fake_convert_to_Tbmp()
                if self._verbose > 2:
                    print "Guessed TBmp Size: %s Bytes" % guessed_size
                while (guessed_size > self._max_tbmp_size) and (self._scale_step > 1):
                    self._scale_step = self._scale_step - 1
                    if self._verbose > 1:
                        print "Bitmap to large, try with scale: %s%%" % (self._scale_step * 10)
                    guessed_size = self.fake_convert_to_Tbmp()
                    if self._verbose > 2:
                        print "Guessed TBmp Size: %s Bytes" % guessed_size
            data = self.convert_to_Tbmp(self._temp_files[1], self._temp_files[2])


        if len(data) > self._max_tbmp_size:
            raise RuntimeError, "\nImage too large (Size: %s, Maximum: %s)\n" % (len(data), self._max_tbmp_size)

        if self._verbose > 2:
            print "Resulting Tbmp Size: %s" % len(data)

        self._doc = PluckerDocs.PluckerImageDocument (str (url), config)
        self._doc.set_data (data)
        size = (ord(data[0]) * 256 + ord(data[1]), ord(data[2]) * 256 + ord(data[3]))


    def scale(self, width, height):
        if (width > self._maxwidth) or (height > self._maxheight):
            maxwidth = self._maxwidth
            maxheight = self._maxheight
        else:
            maxwidth = width
            maxheight = height

        new_width = (float(self._scale_step) / 10) * maxwidth
        new_height = (float(self._scale_step) / 10) * maxheight
        if (int(new_width) == 0):
            new_width = 1
        if (int(new_height) == 0):
            new_height = 1

        return (int(new_width), int(new_height))



    def scale_down(self, width, height, maxwidth, maxheight):
        if width > maxwidth:
            height = (height * maxwidth) / width
            width = maxwidth;

        if height > self._maxheight:
            width = (width * maxheight) / height
            height = maxheight;

        return width, height



    def calc_tbmp_size(self, width, height):
        if operator.mod(width * (float(self._bpp) / 8), 2):
            size = int(width * (float(self._bpp) / 8)) + 1
        else:
            size = int(width * (float(self._bpp) / 8))
        if operator.mod(size, 2):
            size = size + 1
        size = (size * height) + 16
        return size



    def convert_to_bmp(self, input_filename, output_filename):
        command = self.ReplaceVariables(self._imagemagick, input_filename, output_filename)
        if self._verbose > 1:
            print "Running %s" % command
        if self._verbose < 2:
            command = command + " > nul"
        res = os.system (command)
        if res:
            raise RuntimeError, "\nCommand %s failed with code %d\n" % (command, res)

        f = open (output_filename, "rb")
        data = f.read ()
        f.close ()

        if len(data) < 26:
            raise RuntimeError, "\nInvalid bitmap file\n"

        width = (ord(data[21]) << 24) + (ord(data[20]) << 16) + (ord(data[19]) << 8) + ord(data[18])
        height = (ord(data[25]) << 24) + (ord(data[24]) << 16) + (ord(data[23]) << 8) + ord(data[22])

        if self._verbose > 2:
            print "Original Bitmap Width: %s x %s" % (width, height)

        return width, height



    def fake_convert_to_Tbmp(self):
        (maxwidth, maxheight) = self.scale(self._org_width, self._org_height)
        (width, height) = self.scale_down(self._org_width, self._org_height, maxwidth, maxheight)
        return self.calc_tbmp_size(width, height)



    def convert_to_Tbmp(self, mid_name, out_name):
        (maxwidth, maxheight) = self.scale(self._org_width, self._org_height)
        command = self.ReplaceVariables(self._bmp2tbmp, mid_name, out_name, maxwidth, maxheight)
        if self._verbose < 2:
            command = command + " > nul"
        if self._verbose > 1:
            print "Running %s" % command
        res = os.system (command)
        if res:
            raise RuntimeError, "\nCommand %s failed with code %d\n" % (command, res)

        f = open (out_name, "rb")
        tbmp_data = f.read ()
        f.close ()

        if len(tbmp_data) < 4:
            raise RuntimeError, "\nInvalid tbmp file\n"

        tbmp_width = (ord(tbmp_data[0]) << 8) + ord(tbmp_data[1])
        tbmp_height = (ord(tbmp_data[2]) << 8) + ord(tbmp_data[3])

        if self._verbose > 2:
            print "TBmp Size: %sx%s" % (tbmp_width, tbmp_height)

        if (self._org_width > self._maxwidth) or (self._org_height > self._maxheight):
            self._scaled = 1
            if self._verbose > 1:
                print "Bitmap scaled down from %sx%s to %sx%s" % (self._org_width, self._org_height, tbmp_width, tbmp_height)

        if self._verbose > 2:
            print "TBmp Size: %s Bytes" % len(tbmp_data)

        return tbmp_data



    def quotestr(self, path):
        out = string.strip(path)
        if (string.find(out,' ') != -1) and (string.find(out,'"') == -1):
            out = "\""+out+"\""
        return out



    def DeleteTempFiles(self, temp):
        for x in range(len(temp)):
            try:
                if self._verbose > 2:
                    print "Deleting Tempoary File: %s" % self._temp_files[x]
                os.unlink (self._temp_files[x])
            except:
                if self._verbose > 2:
                    print "   failed\n"
                pass



    def CreateTempFiles(self, count):
        temp_filenames = []
        ok = 1
        for x in range(count):
            try:
                temp_filenames.append(tempfile.mktemp ())
                f = open (temp_filenames[x], "wb")
                f.write ("Tmp")
                f.close ()
                if self._verbose > 2:
                    print "Creating Tempoary File: %s" % temp_filenames[x]
            except:
                ok = 0
                if self._verbose > 2:
                    print "Creating Tempoary File: %s   failed" % temp_filenames[x]
        if not ok:
            raise RuntimeError, "\nFailed to create the Tempoary files\n"
        return temp_filenames



    def ReplaceVariables(self, CommandLine, InputFile, OutputFile, maxwidth=0, maxheight=0):

        if self._compress:
            compress_str = self._config.get_string ('tbmp_compression_type','yes')
        else:
            compress_str = 'no'

        Line = CommandLine
        Line = string.replace (Line , '%compress%', compress_str)
        Line = string.replace (Line , '%colors%', "%s" % self._bpp)
        Line = string.replace (Line , '%maxwidth%', "%s" % maxwidth)
        Line = string.replace (Line , '%maxheight%', "%s" % maxheight)
        Line = string.replace (Line , '%input%', InputFile)
        Line = string.replace (Line , '%output%', OutputFile)
        return Line



    def get_plucker_doc(self):
        return self._doc



    def scaled(self):
        return self._scaled


#####################################################################
##
## This is a parser for Windows systems.  It relies on a 
## and the availability of the ImageMagic plus the Tbmp tools
##
##
class WindowsPILImageParser:
    """Do it on Windows.  Ask Dirk Heiser <plucker@dirk-heiser.de> if
    these tools goof up.  I cannot test it."""
    def __del__ (self):
        self.DeleteTempFiles(self._temp_files)



    def __init__ (self, url, type, data, config, attribs, compress=1):
        # The Result
        self._doc = None
        self._scaled = 0

        #Init some variables
        self._url = url
        self._config = config
        self._max_tbmp_size = config.get_int ('max_tbmp_size', SimpleImageMaxSize)
        self._guess_tbmp_size = config.get_bool ('guess_tbmp_size', 1)
        self._try_reduce_bpp = config.get_bool ('try_reduce_bpp', 1)
        self._try_reduce_dimension = config.get_bool ('try_reduce_dimension', 1)
        self._bpp = attribs.get('bpp')
        self._type = type
        self._verbose = config.get_int ('verbosity', 1)
        self._imagemagick = "%s %s" % (self.quotestr(config.get_string ('convert_program','convert.exe')),config.get_string ('convert_program_parameter','%input% bmp:%output%'))
        self._bmp2tbmp = "%s %s" % (self.quotestr(config.get_string ('bmp_to_tbmp', 'Bmp2Tbmp.exe')),config.get_string ('bmp_to_tbmp_parameter','-i=%input% -o=%output% -maxwidth=%maxwidth% -maxheight=%maxheight% -compress=%compress% -bpp=%colors%'))
        if compress:
            self._compress = self._config.get_bool('tbmp_compression', 0)
        else:
            self._compress = 0
        maxwidth = attribs.get('maxwidth')
        maxheight = attribs.get('maxheight')
        if maxwidth == None:
            self._maxwidth = config.get_int ('maxwidth', 150)
        else:
            self._maxwidth = int("%s" % maxwidth)
        if maxheight == None:
            self._maxheight = config.get_int ('maxheight', 150)
        else:
            self._maxheight = int("%s" % maxheight)
        # Create Temp Files
        self._temp_files = self.CreateTempFiles(2)
        # Some globals
        self._scale_step = 10          # Start with 100%, then 90% ...
        self._org_width = 0            # There will be the orginal width of the input file later
        self._org_height = 0           # There will be the orginal heifht of the input file later

        try:
            import Image
        except ImportError:
            raise RuntimeError, "\nError: Could not load PIL libary\n"

        if self._verbose > 1:
            print "Loading Image: %s" % (self._url)
        try:
            im = Image.open (StringIO (data))
        except:
            raise RuntimeError, "\nError: Could not read: %s\n" % self._url

        self.CheckPalette(im)
        self._org_width, self._org_height = im.size
        if self._verbose > 2:
            print "Original Bitmap Width: %s x %s" % (self._org_width, self._org_height)

        if self._guess_tbmp_size == 0:
            data = self.convert_to_Tbmp(im, self._temp_files[0], self._temp_files[1])
            if self._try_reduce_bpp and (len(data) > self._max_tbmp_size):
                while (len(data) > self._max_tbmp_size) and (self._bpp > 1):
                    self._bpp = self._bpp / 2
                    if self._verbose > 1:
                        print "Bitmap to large, try with bpp: %s" % self._bpp
                    data = self.convert_to_Tbmp(im, self._temp_files[0], self._temp_files[1])
            if self._try_reduce_dimension and (len(data) > self._max_tbmp_size):
                while (len(data) > self._max_tbmp_size) and (self._scale_step > 1):
                    self._scale_step = self._scale_step - 1
                    if self._verbose > 1:
                        print "Bitmap to large, try with scale: %s%%" % (self._scale_step * 10)
                    data = self.convert_to_Tbmp(im, self._temp_files[0], self._temp_files[1])
        else:
            if self._try_reduce_bpp:
                guessed_size = self.fake_convert_to_Tbmp()
                if self._verbose > 2:
                    print "Guessed TBmp Size: %s Bytes" % guessed_size
                while ( guessed_size > self._max_tbmp_size) and (self._bpp > 1):
                    self._bpp = self._bpp / 2
                    if self._verbose > 1:
                        print "Bitmap to large, try with bpp: %s" % self._bpp
                    guessed_size = self.fake_convert_to_Tbmp()
                    if self._verbose > 2:
                        print "Guessed TBmp Size: %s Bytes" % guessed_size
            if self._try_reduce_dimension:
                guessed_size = self.fake_convert_to_Tbmp()
                if self._verbose > 2:
                    print "Guessed TBmp Size: %s Bytes" % guessed_size
                while (guessed_size > self._max_tbmp_size) and (self._scale_step > 1):
                    self._scale_step = self._scale_step - 1
                    if self._verbose > 1:
                        print "Bitmap to large, try with scale: %s%%" % (self._scale_step * 10)
                    guessed_size = self.fake_convert_to_Tbmp()
                    if self._verbose > 2:
                        print "Guessed TBmp Size: %s Bytes" % guessed_size
            data = self.convert_to_Tbmp(im, self._temp_files[0], self._temp_files[1])


        if len(data) > self._max_tbmp_size:
            raise RuntimeError, "\nImage too large (Size: %s, Maximum: %s)\n" % (len(data), self._max_tbmp_size)

        if self._verbose > 2:
            print "Resulting Tbmp Size: %s" % len(data)

        self._doc = PluckerDocs.PluckerImageDocument (str (url), config)
        self._doc.set_data (data)
        size = (ord(data[0]) * 256 + ord(data[1]), ord(data[2]) * 256 + ord(data[3]))
        size_data = ((self._org_width, self._org_height), (self._maxwidth, self._maxheight), size)
        if self._verbose > 1:
            sys.stderr.write("input image was " + str(size_data[0]) + ", maxwidth/height used was " + str(size_data[1]) + ", output image size is " + str(size_data[2]) + "\n")



    def scale(self, width, height):
        if (width > self._maxwidth) or (height > self._maxheight):
            maxwidth = self._maxwidth
            maxheight = self._maxheight
        else:
            maxwidth = width
            maxheight = height

        new_width = (float(self._scale_step) / 10) * maxwidth
        new_height = (float(self._scale_step) / 10) * maxheight
        if (int(new_width) == 0):
            new_width = 1
        if (int(new_height) == 0):
            new_height = 1

        return (int(new_width), int(new_height))



    def scale_down(self, width, height, maxwidth, maxheight):
        if width > maxwidth:
            height = (height * maxwidth) / width
            width = maxwidth;

        if height > self._maxheight:
            width = (width * maxheight) / height
            height = maxheight;

        return width, height



    def calc_tbmp_size(self, width, height):
        if operator.mod(width * (float(self._bpp) / 8), 2):
            size = int(width * (float(self._bpp) / 8)) + 1
        else:
            size = int(width * (float(self._bpp) / 8))
        if operator.mod(size, 2):
            size = size + 1
        size = (size * height) + 16
        return size



    def CheckPalette(self, im):
        if im.info.has_key('transparency'):
            try:
                if self._verbose > 2:
                    print dir(im)
                    try:
                        print "Palette size %sbytes" % len(im.palette.data)
                        print dir(im.palette)
                        print "Palette Type: %s" % im.palette.rawmode
                    except:
                        pass

                if im.palette.rawmode <> 'RGB':
                    print "Error: Unknown Palette Type"
                    raise RuntimeError

                pal = []
                for i in range(0, len(im.palette.data)):
                    pal.append(ord(im.palette.data[i]))

                transparrency_index = int("%s" % im.info['transparency'])

                if self._verbose > 1:
                    print "Image have transparency. Palette Index: %s RGB: %s:%s:%s)" % (transparrency_index, pal[transparrency_index*3], pal[(transparrency_index*3)+1], pal[(transparrency_index*3)+2])

                if ((pal[transparrency_index*3] <> 255) or (pal[(transparrency_index*3)+1] <> 255) or (pal[(transparrency_index*3)+2] <> 255)):
                    if self._verbose > 1:
                        print "Set transparent color to white"

                    pal[transparrency_index*3] = 255
                    pal[(transparrency_index*3)+1] = 255
                    pal[(transparrency_index*3)+2] = 255

                    im.putpalette(pal)

            except:
                sys.stderr.write("Error working with transparency of: %s" % self._url)
                pass



    def fake_convert_to_Tbmp(self):
        (maxwidth, maxheight) = self.scale(self._org_width, self._org_height)
        (width, height) = self.scale_down(self._org_width, self._org_height, maxwidth, maxheight)
        return self.calc_tbmp_size(width, height)



    def convert_to_Tbmp(self, im, mid_name, out_name):
        (maxwidth, maxheight) = self.scale(self._org_width, self._org_height)

        im.thumbnail( \
               self.scale_down(self._org_width, \
                               self._org_height, \
                                 maxwidth, maxheight \
                              ) \
                    )

        w, h =                self.scale_down(self._org_width, \
                               self._org_height, \
                                 maxwidth, maxheight \
                              ) 

        if self._verbose > 1:
            print "Scale down from %dx%d to %dx%d" % (self._org_width, self._org_height, w, h)

        if self._bpp <= 8:
            im.save(self._temp_files[0], "BMP")
            command = self.ReplaceVariables(self._bmp2tbmp, mid_name, out_name, maxwidth, maxheight)
            if self._verbose < 2:
                command = command + " > nul"
            if self._verbose > 1:
                print "Running %s" % command
            res = os.system (command)
            if res:
                raise RuntimeError, "\nCommand %s failed with code %d\n" % (command, res)

            f = open (out_name, "rb")
            tbmp_data = f.read ()
            f.close ()

            if len(tbmp_data) < 4:
                raise RuntimeError, "\nInvalid tbmp file\n"

            tbmp_width = (ord(tbmp_data[0]) << 8) + ord(tbmp_data[1])
            tbmp_height = (ord(tbmp_data[2]) << 8) + ord(tbmp_data[3])

            if self._verbose > 2:
                print "TBmp Size: %sx%s" % (tbmp_width, tbmp_height)

            if (self._org_width > self._maxwidth) or (self._org_height > self._maxheight):
                self._scaled = 1
                if self._verbose > 1:
                    print "Bitmap scaled down from %sx%s to %sx%s" % (self._org_width, self._org_height, tbmp_width, tbmp_height)

            if self._verbose > 2:
                print "TBmp Size: %s Bytes" % len(tbmp_data)
            return tbmp_data
        else:
            import StringIO
            palmdata = StringIO.StringIO()
            im.convert("RGB").save(palmdata, "Palm", bpp=16)
            tbmp_data = palmdata.getvalue()
            if self._verbose > 2:
                print "TBmp Size: %s Bytes" % len(tbmp_data)
            palmdata.close()
            return tbmp_data



    def quotestr(self, path):
        out = string.strip(path)
        if (string.find(out,' ') != -1) and (string.find(out,'"') == -1):
            out = "\""+out+"\""
        return out



    def DeleteTempFiles(self, temp):
        for x in range(len(temp)):
            try:
                if self._verbose > 2:
                    print "Deleting Tempoary File: %s" % self._temp_files[x]
                os.unlink (self._temp_files[x])
            except:
                if self._verbose > 2:
                    print "   failed\n"
                pass



    def CreateTempFiles(self, count):
        temp_filenames = []
        ok = 1
        for x in range(count):
            try:
                temp_filenames.append(tempfile.mktemp ())
                f = open (temp_filenames[x], "wb")
                f.write ("Tmp")
                f.close ()
                if self._verbose > 2:
                    print "Creating Tempoary File: %s" % temp_filenames[x]
            except:
                ok = 0
                if self._verbose > 2:
                    print "Creating Tempoary File: %s   failed" % temp_filenames[x]
        if not ok:
            raise RuntimeError, "\nFailed to create the Tempoary files\n"
        return temp_filenames



    def ReplaceVariables(self, CommandLine, InputFile, OutputFile, maxwidth=0, maxheight=0):

        if self._compress:
            compress_str = self._config.get_string ('tbmp_compression_type','yes')
        else:
            compress_str = 'no'

        Line = CommandLine
        Line = string.replace (Line , '%compress%', compress_str)
        Line = string.replace (Line , '%colors%', "%s" % self._bpp)
        Line = string.replace (Line , '%maxwidth%', "%s" % maxwidth)
        Line = string.replace (Line , '%maxheight%', "%s" % maxheight)
        Line = string.replace (Line , '%input%', InputFile)
        Line = string.replace (Line , '%output%', OutputFile)
        return Line



    def get_plucker_doc(self):
        return self._doc



    def scaled(self):
        return self._scaled






def map_parser_name(name):
    parser = string.lower (name)
    if parser == "windows":
        return WindowsImageParser
    elif parser == "windowspil":
        return WindowsPILImageParser
    elif parser == "netpbm":
        return NetPBMImageParser
    elif parser == "netpbm2":
        return NewNetPBMImageParser
    elif parser == "imagemagick":
        return ImageMagickImageParser
    elif parser == "imagemagick2":
        return NewImageMagickImageParser
    elif parser == "pil":
        return PythonImagingLibraryParser
    elif parser == "pil2":
        return NewPythonImagingLibraryParser
    else:
        return None


if sys.platform == 'win32':
    DefaultParser = WindowsImageParser
else:
    DefaultParser = map_parser_name(DEFAULT_IMAGE_PARSER_SETTING)


def get_default_parser (config):
    parser = config.get_string ('image_parser')
    return (parser and map_parser_name(parser)) or DefaultParser


if __name__ == '__main__':
    # Called as a script
    print "This file currently does nothing when called as a script"
