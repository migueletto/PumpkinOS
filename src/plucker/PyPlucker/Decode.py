#!/usr/bin/env python
"""
Decode.py $Id: Decode.py,v 1.4 2002/05/18 10:28:24 nordstrom Exp $

   Simple depacker for pdb files

This module dumps a PDB back into the 'cache' directory.
"""

import sys
import helper.prc
import getopt
import os
import shutil

def dump_pdb(pdbfile,cachedir):
    pdb = helper.prc.File(pdbfile,read=1,write=0)
    for i in range(pdb.getRecords()):
        raw,tmp,id,attr,category = pdb.getRecord(i)
        print "Writing ID: %d" % id
        cache_file = open(os.path.join(cachedir,"%d" % id),"w")
        cache_file.write(raw)
        cache_file.close()
    pdb.close()

def main(argv):
    def usage():
        print "Usage: %s [-h] [-v] [-c <cachedir>] dbfile" % sys.argv[0]
        print "WARNING: THE CACHE DIRECTORY GETS ERASED !!!"
        print "You have been warned."
    try:
        optlist,args = getopt.getopt(argv[1:],"hvc:",['help','version'])
    except getopt.error,msg:
        print msg
        usage()
        sys.exit(1)

    cachedir = os.path.join(os.path.expanduser("~"),'.plucker','cache')

    for arg,value in optlist:
        if arg=='-h' or arg=='--help':
            usage()
            sys.exit(0)
        elif arg=='-v' or arg=='--version':
            print "$Revision: 1.4 $"
            sys.exit(0)
        elif arg=='-c':
            cachedir=value
    if len(args)!=1:
        print "Error on command line"
        usage()
        sys.exit(1)

    pdbfile = args[0]
    if not os.path.exists(pdbfile):
        print "Database %s doesn't exist." % pdbfile
        sys.exit(1)

    if os.path.exists(cachedir):
        print "Removing %s." % cachedir
        shutil.rmtree(cachedir)
        
    os.mkdir(cachedir)
    dump_pdb(pdbfile,cachedir)
    

if __name__=='__main__':
    main(sys.argv)
