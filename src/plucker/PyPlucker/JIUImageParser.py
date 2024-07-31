"""
JIUImageParser.py   $Id: JIUImageParser.py,v 1.2 2003/02/12 00:06:35 janssen Exp $

This defines an ImageParser class that use JIU Java code to
convert an image.

Copyright 2002 by Bill Janssen <bill@janssen.org>

Distributable under the GNU General Public License Version 2 or newer.
"""

import os, sys, string
from PyPlucker.UtilFns import message, error
from PyPlucker.ImageParser import ImageParser

#####################################################################
##
## This is a parser that relies on the Java Imaging Utilities
## to do *all* the work.  Obviously, this only works in Jython.
##
##
class JIUImageParser(ImageParser):

    "Convert an image to the PalmBitmap. Uses Java Imaging Utility under Jython."

    def __init__(self, url, type, data, config, attribs, compress=1):
	if not ("_jython" in sys.builtin_module_names):
	    raise RuntimeError("Attempt to use Java module in non-Jython application")
	import java, jarray
	import net.sourceforge.jiu
	ImageParser.__init__(self, url, type, data, config, attribs)
	try:
	    if type == 'image/jpeg' or type == 'image/gif':
		jimage = java.awt.Toolkit.getDefaultToolkit().createImage(jarray.array([ord(x) for x in data], 'b'))
		# get the width to force loading
		jimage.getWidth()
		self._image = net.sourceforge.jiu.gui.awt.ImageCreator.convertImageToRGB24Image(jimage)
	    elif type == 'image/pnm' or type == 'image/ppm' or type == 'image/pgm' or type == 'image/pbm':
		codec = net.sourceforge.jiu.codecs.PNMCodec()
		codec.setInputStream(java.io.StringBufferInputStream(data))
		codec.process()
		self._image = codec.getImage()
	    elif type == 'image/png':
		codec = net.sourceforge.jiu.codecs.PNGCodec()
		codec.setInputStream(java.io.StringBufferInputStream(data))
		codec.process()
		self._image = codec.getImage()
	    else:
		raise RuntimeError("Image type " + type + " not supported by the JIU image conversion class")
	except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError("Error while opening image " + self._url + " with JIU")

    def size(self):
	return (self._image.getWidth(), self._image.getHeight())

    def convert(self, width, height, bpp, section):

	import java, jarray
	import net.sourceforge.jiu

        try:
	    if section:
		cropper = net.sourceforge.jiu.geometry.Crop()
		cropper.setInputImage(self._image)
                cropper.setBounds(section[0], section[1], section[0] + section[2], section[1] + section[3])
		cropper.process()
		im = cropper.getOutputImage()
	    else:
		im = self._image

	    # scale if necessary
	    if width != im.getWidth() or height != im.getHeight():
		message(2, "Scaling original %dx%d image by %f/%f to %dx%dx%d" % (im.getWidth(), im.getHeight(), float(width)/float(im.getWidth()), float(height)/float(im.getHeight()), width, height, bpp))
		scaler = net.sourceforge.jiu.geometry.Resample()
		scaler.setInputImage(im)
		scaler.setSize(width, height)
                # bell filter is reasonably fast and reasonably accurate
                # scaler.setFilter(net.sourceforge.jiu.geometry.Resample.FILTER_TYPE_BELL);
                # b-spline is more accurate
                scaler.setFilter(net.sourceforge.jiu.geometry.Resample.FILTER_TYPE_B_SPLINE);
		scaler.process()
		im = scaler.getOutputImage()

	    # convert to proper bit depth
            if bpp == 1:
                reducer = net.sourceforge.jiu.color.reduction.RGBToGrayConversion()
                reducer.setInputImage(im)
                reducer.process()
                ditherer = net.sourceforge.jiu.color.dithering.ErrorDiffusionDithering();
                ditherer.setType(net.sourceforge.jiu.color.dithering.ErrorDiffusionDithering.TYPE_FLOYD_STEINBERG);
                ditherer.setGrayscaleOutputBits(1)
                ditherer.setInputImage(reducer.getOutputImage())
                ditherer.process();
                im = ditherer.getOutputImage();
            elif bpp in (2, 4, 8):
                if bpp == 2:
                    palette = net.sourceforge.jiu.codecs.PalmCodec.createSystem2BitGrayscalePalette()
                    dither = 1
                elif bpp == 4:
                    palette = net.sourceforge.jiu.codecs.PalmCodec.createSystem4BitGrayscalePalette()
                    dither = 1
                elif bpp == 8:
                    palette = net.sourceforge.jiu.codecs.PalmCodec.createSystem8BitPalette()
                    dither = 0
                quantizer = net.sourceforge.jiu.color.quantization.ArbitraryPaletteQuantizer(palette)
                nodither = self._config.get_bool('no_dithering_in_java_image_quantization') or self._attribs.has_key('nodither')
                if nodither or not dither:
                    quantizer.setInputImage(im)
                    quantizer.process()
                    im = quantizer.getOutputImage()
                else:
                    ditherer = net.sourceforge.jiu.color.dithering.ErrorDiffusionDithering();
                    ditherer.setType(net.sourceforge.jiu.color.dithering.ErrorDiffusionDithering.TYPE_FLOYD_STEINBERG);
                    ditherer.setQuantizer(quantizer)
                    ditherer.setTruecolorOutput(0)
                    ditherer.setInputImage(im)
                    ditherer.process();
                    im = ditherer.getOutputImage();
	    elif bpp != 16:
                message(0, "%d bpp images not supported with JIU imaging yet.  Using 16 bit color.\n" % (bpp,))

	    # finally, turn it into a Palm image
	    codec = net.sourceforge.jiu.codecs.PalmCodec()
	    codec.setImage(im)
            codec.setCompression(net.sourceforge.jiu.codecs.PalmCodec.COMPRESSION_RLE);
	    outputStream = java.io.ByteArrayOutputStream()
	    codec.setOutputStream(outputStream);
	    codec.process()
	    codec.close()
            # bytes = outputStream.toByteArray()
            # bits = string.join(map(lambda x: chr(((x < 0) and (0x100 + x)) or x), bytes), "")
            return outputStream.toString(0)

        except:
            if self._verbose > 1:
                import traceback
                traceback.print_exc()
            raise RuntimeError("Error while converting image " + self._url + " with JIU")
