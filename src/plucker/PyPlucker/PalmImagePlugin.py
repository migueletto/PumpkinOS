#
# The Python Imaging Library.
# $Id: PalmImagePlugin.py,v 1.6 2004/03/16 14:45:25 prussar Exp $
#
# Palm pixmap image handling
#
#


__version__ = "1.0"

import Image, ImageFile
import StringIO

_Palm8BitColormapValues = (
    ( 255, 255, 255 ), ( 255, 204, 255 ), ( 255, 153, 255 ), ( 255, 102, 255 ),
    ( 255,  51, 255 ), ( 255,   0, 255 ), ( 255, 255, 204 ), ( 255, 204, 204 ), 
    ( 255, 153, 204 ), ( 255, 102, 204 ), ( 255,  51, 204 ), ( 255,   0, 204 ),
    ( 255, 255, 153 ), ( 255, 204, 153 ), ( 255, 153, 153 ), ( 255, 102, 153 ), 
    ( 255,  51, 153 ), ( 255,   0, 153 ), ( 204, 255, 255 ), ( 204, 204, 255 ),
    ( 204, 153, 255 ), ( 204, 102, 255 ), ( 204,  51, 255 ), ( 204,   0, 255 ),
    ( 204, 255, 204 ), ( 204, 204, 204 ), ( 204, 153, 204 ), ( 204, 102, 204 ),
    ( 204,  51, 204 ), ( 204,   0, 204 ), ( 204, 255, 153 ), ( 204, 204, 153 ),
    ( 204, 153, 153 ), ( 204, 102, 153 ), ( 204,  51, 153 ), ( 204,   0, 153 ),
    ( 153, 255, 255 ), ( 153, 204, 255 ), ( 153, 153, 255 ), ( 153, 102, 255 ),
    ( 153,  51, 255 ), ( 153,   0, 255 ), ( 153, 255, 204 ), ( 153, 204, 204 ),
    ( 153, 153, 204 ), ( 153, 102, 204 ), ( 153,  51, 204 ), ( 153,   0, 204 ),
    ( 153, 255, 153 ), ( 153, 204, 153 ), ( 153, 153, 153 ), ( 153, 102, 153 ),
    ( 153,  51, 153 ), ( 153,   0, 153 ), ( 102, 255, 255 ), ( 102, 204, 255 ),
    ( 102, 153, 255 ), ( 102, 102, 255 ), ( 102,  51, 255 ), ( 102,   0, 255 ),
    ( 102, 255, 204 ), ( 102, 204, 204 ), ( 102, 153, 204 ), ( 102, 102, 204 ),
    ( 102,  51, 204 ), ( 102,   0, 204 ), ( 102, 255, 153 ), ( 102, 204, 153 ),
    ( 102, 153, 153 ), ( 102, 102, 153 ), ( 102,  51, 153 ), ( 102,   0, 153 ),
    (  51, 255, 255 ), (  51, 204, 255 ), (  51, 153, 255 ), (  51, 102, 255 ),
    (  51,  51, 255 ), (  51,   0, 255 ), (  51, 255, 204 ), (  51, 204, 204 ),
    (  51, 153, 204 ), (  51, 102, 204 ), (  51,  51, 204 ), (  51,   0, 204 ),
    (  51, 255, 153 ), (  51, 204, 153 ), (  51, 153, 153 ), (  51, 102, 153 ),
    (  51,  51, 153 ), (  51,   0, 153 ), (   0, 255, 255 ), (   0, 204, 255 ),
    (   0, 153, 255 ), (   0, 102, 255 ), (   0,  51, 255 ), (   0,   0, 255 ),
    (   0, 255, 204 ), (   0, 204, 204 ), (   0, 153, 204 ), (   0, 102, 204 ),
    (   0,  51, 204 ), (   0,   0, 204 ), (   0, 255, 153 ), (   0, 204, 153 ),
    (   0, 153, 153 ), (   0, 102, 153 ), (   0,  51, 153 ), (   0,   0, 153 ),
    ( 255, 255, 102 ), ( 255, 204, 102 ), ( 255, 153, 102 ), ( 255, 102, 102 ),
    ( 255,  51, 102 ), ( 255,   0, 102 ), ( 255, 255,  51 ), ( 255, 204,  51 ),
    ( 255, 153,  51 ), ( 255, 102,  51 ), ( 255,  51,  51 ), ( 255,   0,  51 ),
    ( 255, 255,   0 ), ( 255, 204,   0 ), ( 255, 153,   0 ), ( 255, 102,   0 ),
    ( 255,  51,   0 ), ( 255,   0,   0 ), ( 204, 255, 102 ), ( 204, 204, 102 ),
    ( 204, 153, 102 ), ( 204, 102, 102 ), ( 204,  51, 102 ), ( 204,   0, 102 ),
    ( 204, 255,  51 ), ( 204, 204,  51 ), ( 204, 153,  51 ), ( 204, 102,  51 ),
    ( 204,  51,  51 ), ( 204,   0,  51 ), ( 204, 255,   0 ), ( 204, 204,   0 ),
    ( 204, 153,   0 ), ( 204, 102,   0 ), ( 204,  51,   0 ), ( 204,   0,   0 ),
    ( 153, 255, 102 ), ( 153, 204, 102 ), ( 153, 153, 102 ), ( 153, 102, 102 ),
    ( 153,  51, 102 ), ( 153,   0, 102 ), ( 153, 255,  51 ), ( 153, 204,  51 ),
    ( 153, 153,  51 ), ( 153, 102,  51 ), ( 153,  51,  51 ), ( 153,   0,  51 ),
    ( 153, 255,   0 ), ( 153, 204,   0 ), ( 153, 153,   0 ), ( 153, 102,   0 ),
    ( 153,  51,   0 ), ( 153,   0,   0 ), ( 102, 255, 102 ), ( 102, 204, 102 ),
    ( 102, 153, 102 ), ( 102, 102, 102 ), ( 102,  51, 102 ), ( 102,   0, 102 ),
    ( 102, 255,  51 ), ( 102, 204,  51 ), ( 102, 153,  51 ), ( 102, 102,  51 ),
    ( 102,  51,  51 ), ( 102,   0,  51 ), ( 102, 255,   0 ), ( 102, 204,   0 ),
    ( 102, 153,   0 ), ( 102, 102,   0 ), ( 102,  51,   0 ), ( 102,   0,   0 ),
    (  51, 255, 102 ), (  51, 204, 102 ), (  51, 153, 102 ), (  51, 102, 102 ),
    (  51,  51, 102 ), (  51,   0, 102 ), (  51, 255,  51 ), (  51, 204,  51 ),
    (  51, 153,  51 ), (  51, 102,  51 ), (  51,  51,  51 ), (  51,   0,  51 ),
    (  51, 255,   0 ), (  51, 204,   0 ), (  51, 153,   0 ), (  51, 102,   0 ),
    (  51,  51,   0 ), (  51,   0,   0 ), (   0, 255, 102 ), (   0, 204, 102 ),
    (   0, 153, 102 ), (   0, 102, 102 ), (   0,  51, 102 ), (   0,   0, 102 ),
    (   0, 255,  51 ), (   0, 204,  51 ), (   0, 153,  51 ), (   0, 102,  51 ),
    (   0,  51,  51 ), (   0,   0,  51 ), (   0, 255,   0 ), (   0, 204,   0 ),
    (   0, 153,   0 ), (   0, 102,   0 ), (   0,  51,   0 ), (  17,  17,  17 ),
    (  34,  34,  34 ), (  68,  68,  68 ), (  85,  85,  85 ), ( 119, 119, 119 ),
    ( 136, 136, 136 ), ( 170, 170, 170 ), ( 187, 187, 187 ), ( 221, 221, 221 ),
    ( 238, 238, 238 ), ( 192, 192, 192 ), ( 128,   0,   0 ), ( 128,   0, 128 ),
    (   0, 128,   0 ), (   0, 128, 128 ), (   0,   0,   0 ), (   0,   0,   0 ),
    (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ),
    (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ),
    (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ),
    (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ),
    (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ),
    (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ), (   0,   0,   0 ))
    
# so build a prototype image to be used for palette resampling
def build_prototype_image():
    image = Image.new("L", (1,len(_Palm8BitColormapValues),))
    image.putdata(range(len(_Palm8BitColormapValues)))
    palettedata = ()
    for i in range(len(_Palm8BitColormapValues)):
        palettedata = palettedata + _Palm8BitColormapValues[i]
    for i in range(256 - len(_Palm8BitColormapValues)):
        palettedata = palettedata + (0, 0, 0)
    image.putpalette(palettedata)
    return image

Palm8BitColormapImage = build_prototype_image()

# OK, we now have in Palm8BitColormapImage, a "P"-mode image with the right palette
#
# --------------------------------------------------------------------

_FLAGS = {
    "is-compressed":   0x8000,
    "custom-colormap": 0x4000,
    "has-transparent": 0x2000,
    "indirect":               0x1000,
    "forScreen":       0x0800,
    "directColor":     0x0400,
    "fourByteField":   0x0200
    }

_COMPRESSION_TYPES = {
    "none":        0xFF,
    "rle":        0x01,
    "scanline":        0x00,
    }

def o16b(i):
    return chr(i>>8&255) + chr(i&255)

def reduce_rgb(pixel):
    # maps RGB pixel to Palm 16-bit direct color (5:6:5)
    return (((pixel[0] >> 3) << 11) | ((pixel[1] >> 2) << 5) | (pixel[2] >> 3))

#
# --------------------------------------------------------------------

def _save(im, fp, filename, check=0):

    if im.mode == 'RGB':

        # we assume that this is to be converted to the Palm 16-bit directcolor
        # format, which has 5 bits of red, 6 bits of green, and 5 bits of blue

        pixels = im.getdata()

        monochrome = 1
        for p in pixels:
            if p != (255,255,255) and p != (0,0,0):
                 monochrome = 0
                 break

        if monochrome:
            im = im.convert('1')
            rawmode = '1;I'
            bpp = 1
            version = 1
        else:
            newpixels = map(reduce_rgb, pixels)
            im = Image.new('I', im.size, 1)
            im.load()
            index = 0
            for y in range(im.size[1]):
                for x in range(im.size[0]):
                    im.putpixel((x,y), newpixels[index])
                    index = index + 1
            rawmode = 'I;16B'
            bpp = 16
            version = 2

    elif im.mode == "P":
        # we assume this is a color Palm image with the standard colormap,
        # unless the "info" dict has a "custom-colormap" field
        # FIXME: Fails if 0 and 255 are remapped in a custom way.


        monochrome = 1
        if im.info.has_key("custom-colormap"):
            monochrome = 0
        else:
            for p in im.getdata():
                c = _Palm8BitColormapValues[ p ];
                if c != (255,255,255) and c != (0,0,0):
                     monochrome = 0
                     break
        if monochrome:
            im = im.convert('1')
            rawmode = '1;I'
            bpp = 1
            version = 1
        else:
            rawmode = "P"
            bpp = 8
            version = 1

    elif im.mode == "L" and im.encoderinfo.has_key("bpp") and im.encoderinfo["bpp"] in (1, 2, 4):
        # this is 8-bit grayscale, so we shift it to get the high-order bits, and invert it because
        # Palm does greyscale from white (0) to black (1)
        monochrome = 1
        for p in im.getdata():
            if p != 0 and p != 255:
                 monochrome = 0
                 break
        if monochrome:
            im = im.convert('1')
            rawmode = '1;I'
            bpp = 1
            version = 1
        else:
            bpp = im.encoderinfo["bpp"]
            im = im.point(lambda x, shift=8-bpp, maxval=(1 << bpp)-1: maxval - (x >> shift))
            # we ignore the palette here
            im.mode = "P"
            rawmode = "P;" + str(bpp)
            version = 1

    elif im.mode == "L" and im.info.has_key("bpp") and im.info["bpp"] in (1, 2, 4):
        # here we assume that even though the inherent mode is 8-bit grayscale, only
        # the lower bpp bits are significant.  We invert them to match the Palm.
        maxval = (1 << bpp)-1
        pixels = im.getdata()
        monochrome = 1
        for p in pixels:
            if (p&maxval) != 0 and (p&maxval) != maxval:
                 monochrome = 0
                 break
        if monochrome:
            im = Image.new('1', im.size, 1)
            im.load()
            index = 0
            for y in range(im.size[1]):
                for x in range(im.size[0]):
                    if ( pixels[index] == maxval ):
                        value = 1
                    else:
                        value = 0
                    im.putpixel((x,y), value)
                    index = index + 1
            rawmode = '1;I'
            bpp = 1
            version = 1
        else:
            bpp = im.info["bpp"]
            im = im.point(lambda x: maxval - (x & maxval))
            # we ignore the palette here
            im.mode = "P"
            rawmode = "P;" + str(bpp)
            version = 1

    elif im.mode == "1":

        # monochrome -- write it inverted, as is the Palm standard
        rawmode = "1;I"
        bpp = 1
        version = 0

    else:

        raise IOError("cannot write mode %s as Palm" % im.mode)

    if check:
        return check

    #
    # make sure image data is available
    im.load()

    # write header

    cols = im.size[0]
    rows = im.size[1]

    rowbytes = ((cols + (16/bpp - 1)) / (16 / bpp)) * 2;
    transparent_index = 0
    compression_type = _COMPRESSION_TYPES["none"]

    flags = 0;
    if im.mode == "P" and im.info.has_key("custom-colormap"):
        flags = flags | _FLAGS["custom-colormap"]
        colormapsize = 4 * 256 + 2;
        colormapmode = im.palette.mode
        colormap = im.getdata().getpalette()
    else:
        colormapsize = 0

    if rawmode == 'I;16B':
        flags = flags | _FLAGS["directColor"]

    if im.info.has_key("offset"):
        offset = (rowbytes * rows + 16 + 3 + colormapsize) / 4;
    else:
        offset = 0

    fp.write(o16b(cols) + o16b(rows) + o16b(rowbytes) + o16b(flags))
    fp.write(chr(bpp))
    fp.write(chr(version))
    fp.write(o16b(offset))
    fp.write(chr(transparent_index))
    fp.write(chr(compression_type))
    fp.write(o16b(0))        # reserved by Palm

    # now write colormap if necessary

    if colormapsize > 0:
        fp.write(o16b(256))
        for i in range(256):
            fp.write(chr(i))
            if colormapmode == 'RGB':
                fp.write(chr(colormap[3 * i]) + chr(colormap[3 * i + 1]) + chr(colormap[3 * i + 2]))
            elif colormapmode == 'RGBA':
                fp.write(chr(colormap[4 * i]) + chr(colormap[4 * i + 1]) + chr(colormap[4 * i + 2]))

    # if directColor, write out bpp info
    if rawmode == 'I;16B':
        fp.write(chr(5) + chr(6) + chr(5) + chr(0))
        # now write out the transparent color
        fp.write(chr(0) + chr(0) + chr(0) + chr(0))

    # now convert data to raw form
    ImageFile._save(im, fp, [("raw", (0,0)+im.size, 0, (rawmode, rowbytes, 1))])

    fp.flush()


#
# --------------------------------------------------------------------

Image.register_save("Palm", _save)

Image.register_extension("Palm", ".palm")

Image.register_mime("Palm", "image/palm")
