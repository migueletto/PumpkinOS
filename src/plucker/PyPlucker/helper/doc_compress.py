#
#  Original RCS-Id from Rob Tillotson <robt@debian.org>:
#      Id: doc_compress.py,v 1.2 1999/08/09 21:59:13 rob Exp
#
#  Copyright 1999 Rob Tillotson <robt@debian.org>
#  All Rights Reserved
#
#  Permission to use, copy, modify, and distribute this software and
#  its documentation for any purpose and without fee or royalty is
#  hereby granted, provided that the above copyright notice appear in
#  all copies and that both the copyright notice and this permission
#  notice appear in supporting documentation or portions thereof,
#  including modifications, that you you make.
#
#  THE AUTHOR ROB TILLOTSON DISCLAIMS ALL WARRANTIES WITH REGARD TO
#  THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
#  AND FITNESS.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
#  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
#  RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
#  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
#  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE!
#
# One slight modification by Holger D_rer <holly@starship.python.net> -- see comment below
#
"""Doc compression in pure Python.
"""

__version__ = '$Id: doc_compress.py,v 1.7 2002/05/18 10:28:24 nordstrom Exp $'

__copyright__ = 'Copyright 1999 Rob Tillotson <robt@debian.org>'


import string

COUNT_BITS = 3
DISP_BITS = 11

def compress(s):
    # optimizations
    # this cut off about 0.1 sec/call
    _find = string.find

    out = []
    space = 0
    
    # first phase: sliding window
    sstart = 0
    i = 0
    imax = len(s)
    while 1:
        if i >= imax: break
        if (i - sstart) > 2047: sstart = i - 2047
        
        #ts = s[i:i+10] # substring to search for
        #while len(ts) >= 3:
        #    f = _find(s, ts, sstart, i)
        #    if f >= 0: break
        #    ts = ts[:-1]

        # see that code above?  it's the obvious way, but it's slow.
        # what it does is basically do string.find on the data ahead
        # of the current position, on an ever-shrinking buffer.  With
        # that code, this function took around 1.6 seconds per call
        # (on a Cyrix M2-266 [207 MHz/83 MHz bus/1MB cache]).
        #
        # The below is a bit different: basically, it takes advantage
        # of the fact that if we don't have a length 3 substring, we
        # can't possibly have a length 4 substring, and so on.  Thus,
        # it first looks for a length 3 substring (we don't care about
        # anything shorter).  If it finds one, it then attempts to
        # find a length 4 substring between that position and the
        # current location, and so on.
        #
        # I suspect the primary benefit of this is to avoid 6 out of 7
        # calls to string.find every time we aren't looking at a
        # compressible string.  At any rate, changing this code cuts
        # the time to about 0.38 seconds per call.
        e = 3
        ns = sstart
        ts = s[i:i+e]
        while (imax - i) >= e and e <= 10:
            f = _find(s, ts, ns, i)
            if f < 0: break
            e = e + 1
            ts = s[i:i+e]
            ns = f
        e = e - 1

        #if len(ts) >= 3: #we have a match, f is the location and len(ts) is the length
        #    l = len(ts)
        #    ns = f
        if e >= 3:
            dist = i - ns
            byte = (dist << 3) | (e-3)
            if space:
                out.append(32)
                space = 0
            out.append(0x80 | (byte >> 8))
            out.append(byte & 0xff)
            i = i + e
            
        else:
            c = ord(s[i])
            i = i + 1
            if space:
                if c >= 0x40 and c <= 0x7f: out.append(c | 0x80)
                else:
                    out.append(32)
                    if c < 0x80 and (c == 0 or c > 8):
                        out.append(c)
                    else:
                        out.append(1)
                        out.append(c)
                space = 0
            else:
                if c == 32: space = 1
                else:
                    if c < 0x80 and (c == 0 or c > 8):
                        out.append(c)
                    else:
                        out.append(1)
                        out.append(c)
        
    if space: out.append(32)
    # second phase: look for repetitions of '1 <x>' and combine up to 8 of them.
    # interestingly enough, in regular text this hardly makes a difference.
    return string.join(map(chr, out),'')
    

def uncompress(s):
    s = map(ord, s)
    x = 0
    o = []
    try:
        while 1:
            c = s[x]
            x = x + 1
            if c > 0 and c < 9:  # just copy that many bytes
                for y in range(0, c):
                    o.append(s[x])
                    x = x + 1
            elif c < 128: # a regular ascii character
                o.append(c)
            # The following line changed by Holger D_rer:  (used to be c > 0xc0)
            elif c >= 0xc0: # a regular ascii character with a space before it
                o.append(32)
                o.append(c & 0x7f)
            else: # a compressed sequence
                c = c << 8
                c = c | s[x]
                x = x + 1
                m = (c & 0x3fff) >> COUNT_BITS
                n = (c & ((1 << COUNT_BITS)-1)) + 3
                for y in range(0, n):
                    o.append(o[len(o)-m])
    except IndexError:
        pass
    return string.join(map(chr, o), '')


