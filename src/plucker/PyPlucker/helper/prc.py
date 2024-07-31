#
#  $Id: prc.py,v 1.13 2002/10/10 21:58:18 janssen Exp $
#  Original RCSId: prc.py,v 1.6 1999/07/16 05:32:01 rob Exp 
#
#  Copyright 1998-1999 Rob Tillotson <robt@debian.org>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License, version 2,
#  as published by the Free Software Foundation.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with this program; if not, write the Free Software Foundation,
#  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
#
# Changes by Holger D_rer:  (compared to the one from Pyrite-0.7.7)
#   - removed print statements
#   - added my own RCS-Id (I left a copy of the original)
#
"""PRC/PDB file I/O in pure Python.

  This module serves two purposes: one, it allows access to Palm OS(tm)
  database files on the desktop in pure Python without requiring
  pilot-link (hence, it may be useful for import/export utilities),
  and two, it caches the contents of the file in memory so it can
  be freely modified using an identical API to databases over a
  DLP connection.
"""

__version__ = 'Id: prc.py,v 1.6 1999/07/16 05:32:01 rob Exp '

__copyright__ = 'Copyright 1998-1999 Rob Tillotson <robt@debian.org>'


#
# DBInfo structure:
#
#   int more
#   unsigned int flags
#   unsigned int miscflags
#   unsigned long type
#   unsigned long creator
#   unsigned int version
#   unsigned long modnum
#   time_t createDate, modifydate, backupdate
#   unsigned int index
#   char name[34]
#
#
# DB Header:
#   32 name
#   2  flags
#   2  version
#   4  creation time
#   4  modification time
#   4  backup time
#   4  modification number
#   4  appinfo offset
#   4  sortinfo offset
#   4  type
#   4  creator
#   4  unique id seed (garbage?)
#   4  next record list id (normally 0)
#   2  num of records for this header
#   (maybe 2 more bytes)
#
# Resource entry header: (if low bit of attr = 1)
#   4  type
#   2  id
#   4  offset
#
# record entry header: (if low bit of attr = 0)
#   4  offset
#   1  attributes
#   3  unique id
#
# then 2 bytes of 0
#
# then appinfo then sortinfo
#

import sys, os, stat, struct

PI_HDR_SIZE = 78
PI_RESOURCE_ENT_SIZE = 10
PI_RECORD_ENT_SIZE = 8
PI_MAX_DOC_NAME_SIZE = 31

PILOT_TIME_DELTA = 2082844800L

flagResource = 0x0001
flagReadOnly = 0x0002
flagAppInfoDirty = 0x0004
flagBackup = 0x0008
flagOpen = 0x8000
# 2.x
flagNewer = 0x0010
flagReset = 0x0020
#
flagExcludeFromSync = 0x0080

flagCopyPrevention = 0x0040;
flagLaunchableData = 0x0200;

attrDeleted = 0x80
attrDirty = 0x40
attrBusy = 0x20
attrSecret = 0x10
attrArchived = 0x08

default_info = {
    'name': '',
    'type': 'DATA',
    'creator': '    ',
    'createDate': 0,
    'modifyDate': 0,
    'backupDate': 0,
    'modnum': 0,
    'version': 0,
    'flagReset': 0,
    'flagResource': 0,
    'flagNewer': 0,
    'flagExcludeFromSync': 0,
    'flagAppInfoDirty': 0,
    'flagReadOnly': 0,
    'flagBackup': 0,
    'flagOpen': 0,
    'flagCopyPrevention': 0,
    'flagLaunchableData': 0,
    'more': 0,
    'index': 0
    }

def null_terminated(s):
    for x in range(0, len(s)):
        if s[x] == '\000': return s[:x]
    return s

def trim_null(s):
    return string.split(s, '\0')[0]

def pad_null(s, l):    
    if len(s) > l - 1:
        s = s[:l-1]
        s = s + '\0'
    if len(s) < l: s = s + '\0' * (l - len(s))
    return s

class OFile:
    def __init__(self, name=None, read=1, write=0, info={}):
        self.filename = name
        self.records = []
        self.ids = []
        self.resource_types = []
        self.appinfo = ''
        self.sortinfo = ''
        self.info = {}
        self.info.update(default_info)
        self.info.update(info)
        self.writeback = write
        self.next = 0
        self.isopen = 1
        self.dirty = 0
        
        if read:
            f = open(name, 'rb')
            self.slurp(f)
            

    def close(self):
        if self.writeback and self.dirty:
            f = open(self.filename, 'wb')
            self.barf(f)
            f.close ()
            self.dirty = 0
        self.isopen = 0

    def __del__(self):
        self.close()
            
    def slurp(self, f):
        """Load the cache from the specified file.
        """
        # a simple slurp...
        data = f.read()

        if len(data) < PI_HDR_SIZE: raise IOError, 'file too short'
        (name, flags, ver, ctime, mtime, btime, mnum, appinfo, sortinfo,
         type, creator, uid, nextrec, numrec) \
         = struct.unpack('>32shhLLLlll4s4sllh', data[:PI_HDR_SIZE])
        
        if nextrec or appinfo < 0 or sortinfo < 0 or numrec < 0:
            raise IOError, 'invalid database header'

        if ctime: ctime = ctime - PILOT_TIME_DELTA,
        if mtime: mtime = btime - PILOT_TIME_DELTA,
        if btime: btime = mtime - PILOT_TIME_DELTA,
        
        self.info = {
            'name': null_terminated(name),
            'type': type,
            'creator': creator,
            'createDate': ctime,
            'modifyDate': mtime,
            'backupDate': btime,
            'modnum': mnum,
            'version': ver,
            'flagReset': flags & flagReset,
            'flagResource': flags & flagResource,
            'flagNewer': flags & flagNewer,
            'flagExcludeFromSync': flags & flagExcludeFromSync,
            'flagAppInfoDirty': flags & flagAppInfoDirty,
            'flagReadOnly': flags & flagReadOnly,
            'flagBackup': flags & flagBackup,
            'flagOpen': flags & flagOpen,
            'flagCopyPrevention': flags & flagCopyPrevention,
            'flagLaunchableData': flags & flagLaunchableData,
            'more': 0,
            'index': 0
         }

        rsrc = flags & flagResource
        if rsrc: s = PI_RESOURCE_ENT_SIZE
        else: s = PI_RECORD_ENT_SIZE

        entries = []

        pos = PI_HDR_SIZE
        for x in range(0,numrec):
            hstr = data[pos:pos+s]
            pos = pos + s
            if not hstr or len(hstr) < s:
                raise IOError, 'bad database header'

            if rsrc:
                (typ, id, offset) = struct.unpack('>4shl', hstr)
                entries.append((offset, typ, id))
            else:
                (offset, auid) = struct.unpack('>ll', hstr)
                attr = (auid & 0xff000000) >> 24
                uid = auid & 0x00ffffff
                entries.append((offset, attr, uid))
                
        offset = len(data)
        entries.reverse()
        for of, q, id in entries:
            size = offset - of
            if size < 0: raise IOError, 'bad pdb/prc record entry (size < 0)'
            d = data[of:offset]
            offset = of
            if len(d) != size: raise IOError, 'failed to read record'
            if rsrc:
                self.records.append( [id, q, d] )
                self.resource_types.append( (q, id) ) # tuple, so it is immutable/hashable
            else:
                self.records.append( [id, q & 0xf0, q & 0x0f, d] )
            self.ids.append(id)
        self.records.reverse()
        self.ids.reverse()
        self.resource_types.reverse()
        
        if sortinfo:
            sortinfo_size = offset - sortinfo
            offset = sortinfo
        else:
            sortinfo_size = 0

        if appinfo:
            appinfo_size = offset - appinfo
            offset = appinfo
        else:
            appinfo_size = 0

        if appinfo_size < 0 or sortinfo_size < 0:
            raise IOError, 'bad database header (appinfo or sortinfo size < 0)'

        if appinfo_size:
            self.appinfo = data[appinfo:appinfo+appinfo_size]
            if len(self.appinfo) != appinfo_size:
                raise IOError, 'failed to read appinfo block'

        if sortinfo_size:
            self.sortinfo = data[sortinfo:sortinfo+sortinfo_size]
            if len(self.sortinfo) != sortinfo_size:
                raise IOError, 'failed to read sortinfo block'

    def barf(self, f):
        """Dump the cache to a file.
        """

        # first, we need to precalculate the offsets.
        if self.info.get('flagResource'):
            entries_len = 10 * len(self.records)
        else: entries_len = 8 * len(self.records)

        off = PI_HDR_SIZE + entries_len
        if self.appinfo:
            appinfo_offset = off
            off = off + len(self.appinfo)
        else:
            appinfo_offset = 0
        if self.sortinfo:
            sortinfo_offset = off
            off = off + len(self.sortinfo)
        else:
            sortinfo_offset = 0
        
        rec_offsets = []
        for x in self.records:
            rec_offsets.append(off)
            off = off + len(x[-1])

        
        info = self.info
        creattime = info.get('createDate',0L)
        if creattime: creattime = creattime + PILOT_TIME_DELTA
        modtime = info.get('modifyDate',0L)
        if modtime: modtime = modtime + PILOT_TIME_DELTA
        backtime = info.get('backupDate',0L)
        if backtime: backtime = backtime + PILOT_TIME_DELTA
        hdr = struct.pack('>32shhLLLlll4s4sllh',
                          pad_null(info.get('name',''), 32),
                          0, #flags
                          info.get('version',0),
                          creattime,
                          modtime,
                          backtime,
                          info.get('modnum',0),
                          appinfo_offset, # appinfo
                          sortinfo_offset, # sortinfo
                          info.get('type','    '),
                          info.get('creator','    '),
                          0, # uid???
                          0, # nextrec???
                          len(self.records))

        f.write(hdr)

        entries = []
        record_data = []
        rsrc = self.info.get('flagResource')
        for x, off in map(None, self.records, rec_offsets):
            if rsrc:
                id, type, data = tuple(x)
                record_data.append(data)
                entries.append(struct.pack('>4shl', type, id, off))
            else:
                id, attr, cat, data = tuple(x)
                record_data.append(data)
                a = ((attr | cat) << 24) | id
                entries.append(struct.pack('>ll', off, a))

        for x in entries: f.write(x)
        f.write(self.appinfo)
        f.write(self.sortinfo)
        for x in record_data: f.write(x)
                
        
    def load(self):
        file_length = os.stat(self.filename)[6]
        hstr = self.f.read(PI_HDR_SIZE)
        if not hstr or len(hstr) < PI_HDR_SIZE:
            raise IOError, 'prc/pdb header too short'

        (name, flags, ver, ctime, mtime, btime, mnum, appinfo, sortinfo,
         type, creator, uid, nextrec, numrec) \
         = struct.unpack('>32shhLLLlll4s4sllh', hstr)

        if nextrec or appinfo < 0 or sortinfo < 0 or numrec < 0:
            raise IOError, 'invalid prc/pdb header'

        self.info = {
            'name': null_terminated(name),
            'type': type,
            'creator': creator,
            'createDate': ctime - PILOT_TIME_DELTA,
            'modifyDate': mtime - PILOT_TIME_DELTA,
            'backupDate': btime - PILOT_TIME_DELTA,
            'modnum': mnum,
            'version': ver,
            'flagReset': flags & flagReset,
            'flagResource': flags & flagResource,
            'flagNewer': flags & flagNewer,
            'flagExcludeFromSync': flags & flagExcludeFromSync,
            'flagAppInfoDirty': flags & flagAppInfoDirty,
            'flagReadOnly': flags & flagReadOnly,
            'flagBackup': flags & flagBackup,
            'flagOpen': flags & flagOpen,
            'flagCopyPrevention': flags & flagCopyPrevention,
            'flagLaunchableData': flags & flagLaunchableData,
            'more': 0,
            'index': 0
         }

        # read entries
        rsrc = flags & flagResource
        if rsrc: s = PI_RESOURCE_ENT_SIZE
        else: s = PI_RECORD_ENT_SIZE

        entries = []

        for x in range(0,numrec):
            hstr = self.f.read(s)
            if not hstr or len(hstr) < s:
                raise IOError, 'bad prc/pdb header'

            if rsrc:
                (typ, id, offset) = struct.unpack('>lhl', hstr)
                entries.append((offset, typ, id))
            else:
                (offset, attr, uid0, uid1, uid2) = struct.unpack('>lbbbb', hstr)
                uid = uid0 << 16 | uid1 << 8 | uid2
                entries.append((offset, attr, uid))

        # read records, from the end of the file back
        # (since we have to go backwards calculating offsets anyway)
        offset = file_length
        entries.reverse()
        for of, q, id in entries:
            size = offset - of
            if size < 0: raise IOError, 'bad pdb/prc record entry (size < 0)'
            offset = of
            self.f.seek(of)
            d = self.f.read(size)
            if len(d) != size: raise IOError, 'failed to read record'
            self.records.append( [id, q, d] )

        self.records.reverse()

        if sortinfo:
            sortinfo_size = offset - sortinfo
            offset = sortinfo
        else:
            sortinfo_size = 0

        if appinfo:
            appinfo_size = offset - appinfo
            offset = appinfo
        else:
            appinfo_size = 0

        if appinfo_size < 0 or sortinfo_size < 0:
            raise IOError, 'bad pdb/prc header (appinfo or sortinfo size < 0)'

        if appinfo_size:
            self.f.seek(appinfo)
            self.appinfo = self.f.read(appinfo_size)
            if len(self.appinfo) != appinfo_size:
                raise IOError, 'failed to read appinfo block'

        if sortinfo_size:
            self.f.seek(sortinfo)
            self.sortinfo = self.f.read(sortinfo_size)
            if len(self.sortinfo) != sortinfo_size:
                raise IOError, 'failed to read sortinfo block'

            
    # pi-file API
    def getRecords(self):
        return len(self.records)

    def getAppBlock(self):
        if not self.appinfo: return None
        else: return self.appinfo

    def setAppBlock(self, blk):
        self.dirty = 1
        self.appinfo = blk

    def getSortBlock(self):
        if not self.sortinfo: return None
        else: return self.sortinfo

    def setSortBlock(self, blk):
        self.dirty = 1
        self.sortinfo = blk

    def checkID(self, id):
        return id in self.ids

    def getRecord(self, i):
        try:
            id, attr, cat, d = self.records[i]
        except:
            return None
        return (d, i, id, attr, cat)

    def getRecordByID(self, id):
        try:
            i = self.ids.index(id)
        except:
            return None
        return self.getRecord(i)
    
    def getResource(self, i):
        try:
            id, typ, d = self.records[i]
        except:
            return None
        return (d, typ, id)

    def getDBInfo(self):
        return self.info

    def setDBInfo(self, info):
        self.dirty = 1
        self.info = {}
        self.info.update(info)

    # DLP database api
    def setRecord(self, attr, id, cat, data):
        # yes, it is indeed true that this always puts the replacement
        # record at the END of the database.  That's what the OS does...
        if not id:
            id = max(self.ids) + 1
            
        if id in self.ids:
            self.deleteRecord(id)
        self.ids.append(id)
        self.records.append( [id, attr, cat, data] )
        self.dirty = 1
        return id
    
    def setResource(self, typ, id, data):
        if (typ, id) in self.resource_types:
            self.deleteResource(typ, id)
        self.ids.append(id)
        self.resource_types.append((typ, id))
        self.records.append( [id, typ, data] )
        self.dirty = 1
        return id

    def getNextRecord(self, cat):
        while self.next < len(self.records):
            (id, attr, c, data) = self.records[self.next]
            i = self.next
            self.next = self.next + 1
            if c == cat:
                return (data, i, id, attr, c)
        return ''

    def getNextModRecord(self, cat=-1):
        while self.next < len(self.records):
            (id, attr, c, data) = self.records[self.next]
            i = self.next
            self.next = self.next + 1
            if (attr & attrModified) and (cat < 0 or c == cat):
                return (data, i, id, attr, c)
        return ''

    def getResourceByID(self, type, id):
        try:
            i = self.resource_types.index((type, id))
        except:
            return None
        return self.getResource(i)

    def deleteRecord(self, id):
        try:
            i = self.ids.index(id)
        except:
            return
        del self.ids[i]
        del self.records[i]
        self.dirty = 1

    def deleteRecords(self):
        self.records = []
        self.ids = []
        self.dirty = 1
        
    def deleteResource(self, type, id):
        try:
            i = self.resource_types.index((type, id))
        except:
            return
        del self.ids[i]
        del self.resource_types[i]
        del self.records[i]
        self.dirty = 1

    def deleteResources(self):
        self.records = []
        self.resource_types = []
        self.ids = []
        self.dirty = 1

    def getRecordIDs(self, sort=0):
        i = self.ids[:]
        if sort: i.sort()
        return i

    def moveCategory(self, frm, to):
        for r in self.records:
            if r[2] == frm:
                r[2] = to
        self.dirty = 1

    def deleteCategory(self, cat):
        raise RuntimeError, 'unimplemented'

    def purge(self):
        self.records = filter(lambda x: not (x[1] & attrDeleted), self.records)
        self.dirty = 1

    def resetNext(self):
        self.next = 0

    def resetFlags(self):
        # reset dirty flags
        if not self.info.get('flagResource', 0):
            for x in self.records:
                x[1] = x[1] & ~attrDirty
        # reset sync time ... not possible?
        self.dirty = 1
        

#
# ***********************************************************
# New AVL-based prc code
# ***********************************************************

# Record object to be put in tree...
class PRecord:
    def __init__(self, attr=0, id=0, category=0, raw=''):
        self.raw = raw
        self.id = id
        self.attr = attr
        self.category = category

    # comparison and hashing are done by ID;
    # thus, the id value *may not be changed* once
    # the object is created.
    def __cmp__(self, obj):
        if type(obj) == type(0):
            return cmp(self.id, obj)
        else:
            return cmp(self.id, obj.id)

    def __hash__(self):
        return self.id

class PResource:
    def __init__(self, typ='    ', id=0, raw=''):
        self.raw = raw
        self.id = id
        self.type = typ

    def __cmp__(self, obj):
        if type(obj) == type(()):
            return cmp( (self.type, self.id), obj)
        else:
            return cmp( (self.type, self.id), (obj.type, obj.id) )

    def __hash__(self):
        return hash((self.type, self.id))
    

class PCache:
    def __init__(self):
        self.data = []
        self.appblock = ''
        self.sortblock = ''
        self.dirty = 0
        self.next = 0
        self.info = {}
        self.info.update(default_info)

    # pi-file API
    def getRecords(self): return len(self.data)
    def getAppBlock(self): return self.appblock and self.appblock or None
    def setAppBlock(self, raw):
        self.dirty = 1
        self.appblock = raw
    def getSortBlock(self): return self.sortblock and self.sortblock or None
    def setSortBlock(self, raw):
        self.dirty = 1
        self.sortblock = raw
    def checkID(self, id): return id in self.data
    def getRecord(self, i):
        try: r = self.data[i]
        except: return None
        return r.raw, i, r.id, r.attr, r.category
    def getRecordByID(self, id):
        try:
            i = self.data.index(id)
            r = self.data[i]
        except: return None
        return r.raw, i, r.id, r.attr, r.category
    def getResource(self, i):
        try: r = self.data[i]
        except: return None
        return r.raw, r.type, r.id
    def getDBInfo(self): return self.info
    def setDBInfo(self, info):
        self.dirty = 1
        self.info = {}
        self.info.update(info)
        
    def setRecord(self, attr, id, cat, data):
        if not id:
            if not len(self.data): id = 1
            else:
                xid = self.data[0].id + 1
                while xid in self.data: xid = xid + 1
                id = xid
                
        r = PRecord(attr, id, cat, data)
        if id in self.data:
            self.data.remove(id)
        self.data.append(r)
        self.dirty = 1
        return id
    
    def setResource(self, typ, id, data):
        if (typ, id) in self.data:
            self.data.remove((typ,id))
        r = PResource(typ, id, data)
        self.data.append(r)
        self.dirty = 1
        return id
            
    def getNextRecord(self, cat):
        while self.next < len(self.data):
            r = self.data[self.next]
            i = self.next
            self.next = self.next + 1
            if r.category == cat:
                return r.raw, i, r.id, r.attr, r.category
        return ''

    def getNextModRecord(self, cat=-1):
        while self.next < len(self.data):
            r = self.data[self.next]
            i = self.next
            self.next = self.next + 1
            if (r.attr & attrModified) and (cat < 0 or r.category == cat):
                return r.raw, i, r.id, r.attr, r.category

    def getResourceByID(self, type, id):
        try: r = self.data[self.data.index((type,id))]
        except: return None
        return r.raw, r.type, r.id

    def deleteRecord(self, id):
        if not id in self.data: return None
        self.data.remove(id)
        self.dirty = 1

    def deleteRecords(self):
        self.data = []
        self.dirty = 1

    def deleteResource(self, type, id):
        if not (type,id) in self.data: return None
        self.data.remove((type,id))
        self.dirty = 1

    def deleteResources(self):
        self.data = []
        self.dirty = 1

    def getRecordIDs(self, sort=0):
        m = map(lambda x: x.id, self.data)
        if sort: m.sort()
        return m

    def moveCategory(self, frm, to):
        for r in self.data:
            if r.category == frm:
                r.category = to
        self.dirty = 1

    def deleteCategory(self, cat):
        raise RuntimeError, 'unimplemented'

    def purge(self):
        ndata = []
        # change to filter later
        for r in self.data:
            if (r.attr & attrDeleted):
                continue
            ndata.append(r)
        self.data = ndata
        self.dirty = 1

    def resetNext(self):
        self.next = 0

    def resetFlags(self):
        # special behavior for resources
        if not self.info.get('flagResource',0):
            # use map()
            for r in self.data:
                r.attr = r.attr & ~attrDirty
            self.dirty = 1

class File(PCache):
    def __init__(self, name=None, read=1, write=0, info={}):
        PCache.__init__(self)
        self.filename = name
        self.info.update(info)
        self.writeback = write
        self.isopen = 0
        
        if read:
            self.load(name)
            self.isopen = 1

    def close(self):
        if self.writeback and self.dirty:
            self.save(self.filename)
        self.isopen = 0
            
    def load(self, f):
        if type(f) == type(''): f = open(f, 'rb')

        data = f.read()
        self.unpack(data)
        
    def unpack(self, data):
        
        if len(data) < PI_HDR_SIZE: raise IOError, 'file too short'
        (name, flags, ver, ctime, mtime, btime, mnum, appinfo, sortinfo,
         typ, creator, uid, nextrec, numrec) \
         = struct.unpack('>32shhLLLlll4s4sllh', data[:PI_HDR_SIZE])

        if nextrec or appinfo < 0 or sortinfo < 0 or numrec < 0:
            raise IOError, 'invalid database header'

        self.info = {
            'name': null_terminated(name),
            'type': typ,
            'creator': creator,
            'createDate': ctime - PILOT_TIME_DELTA,
            'modifyDate': mtime - PILOT_TIME_DELTA,
            'backupDate': btime - PILOT_TIME_DELTA,
            'modnum': mnum,
            'version': ver,
            'flagReset': flags & flagReset,
            'flagResource': flags & flagResource,
            'flagNewer': flags & flagNewer,
            'flagExcludeFromSync': flags & flagExcludeFromSync,
            'flagAppInfoDirty': flags & flagAppInfoDirty,
            'flagReadOnly': flags & flagReadOnly,
            'flagBackup': flags & flagBackup,
            'flagOpen': flags & flagOpen,
            'flagCopyPrevention': flags & flagCopyPrevention,
            'flagLaunchableData': flags & flagLaunchableData,
            'more': 0,
            'index': 0
         }
        
        rsrc = flags & flagResource
        if rsrc: s = PI_RESOURCE_ENT_SIZE
        else: s = PI_RECORD_ENT_SIZE

        entries = []

        pos = PI_HDR_SIZE
        for x in range(0,numrec):
            hstr = data[pos:pos+s]
            pos = pos + s
            if not hstr or len(hstr) < s:
                raise IOError, 'bad database header'

            if rsrc:
                (typ, id, offset) = struct.unpack('>4shl', hstr)
                entries.append((offset, typ, id))
            else:
                (offset, auid) = struct.unpack('>ll', hstr)
                attr = (auid & 0xff000000) >> 24
                uid = auid & 0x00ffffff
                entries.append((offset, attr, uid))

        offset = len(data)
        entries.reverse()
        for of, q, id in entries:
            size = offset - of
            if size < 0: raise IOError, 'bad pdb/prc record entry (size < 0)'
            d = data[of:offset]
            offset = of
            if len(d) != size: raise IOError, 'failed to read record'
            if rsrc:
                r = PResource(q, id, d)
                self.data.append(r)
            else:
                r = PRecord(q & 0xf0, id, q & 0x0f, d)
                self.data.append(r)
        self.data.reverse()

        if sortinfo:
            sortinfo_size = offset - sortinfo
            offset = sortinfo
        else:
            sortinfo_size = 0

        if appinfo:
            appinfo_size = offset - appinfo
            offset = appinfo
        else:
            appinfo_size = 0

        if appinfo_size < 0 or sortinfo_size < 0:
            raise IOError, 'bad database header (appinfo or sortinfo size < 0)'

        if appinfo_size:
            self.appblock = data[appinfo:appinfo+appinfo_size]
            if len(self.appblock) != appinfo_size:
                raise IOError, 'failed to read appinfo block'

        if sortinfo_size:
            self.sortblock = data[sortinfo:sortinfo+sortinfo_size]
            if len(self.sortblock) != sortinfo_size:
                raise IOError, 'failed to read sortinfo block'

    def save(self, f):
        """Dump the cache to a file.
        """
        if type(f) == type(''): f = open(f, 'wb')
        
        # first, we need to precalculate the offsets.
        if self.info.get('flagResource'):
            entries_len = 10 * len(self.data)
        else: entries_len = 8 * len(self.data)

        off = PI_HDR_SIZE + entries_len + 2
        ##old code:
        # appinfo_offset = off
        # off = off + len(self.appblock)
        # sortinfo_offset = off
        # off = off + len(self.sortblock)
        if self.appblock:
            appinfo_offset = off
            off = off + len(self.appblock)
        else:
            appinfo_offset = 0

        if self.sortblock:
            sortinfo_offset = off
            off = off + len(self.sortblock)
        else:
            sortinfo_offset = 0

       
        rec_offsets = []
        for x in self.data:
            rec_offsets.append(off)
            off = off + len(x.raw)

        info = self.info
        flg = 0
        if info.get('flagResource',0): flg = flg | flagResource
        if info.get('flagReadOnly',0): flg = flg | flagReadOnly
        if info.get('flagAppInfoDirty',0): flg = flg | flagAppInfoDirty
        if info.get('flagBackup',0): flg = flg | flagBackup
        if info.get('flagOpen',0): flg = flg | flagOpen
        if info.get('flagNewer',0): flg = flg | flagNewer
        if info.get('flagReset',0): flg = flg | flagReset
        if info.get('flagCopyPrevention',0): flg = flg | flagCopyPrevention
        if info.get('flagLaunchableData',0): flg = flg | flagLaunchableData
        # excludefromsync doesn't actually get stored?
        name = info.get('name', '')
        if "_jython" in sys.builtin_module_names:
            import string
            # work around bug in Jython implementation of struct.pack
            nameparts = []
            while name:
                partlen = min(len(name),8)
                nameparts.append(struct.pack('>8s', name[:partlen]))
                name = name[partlen:]
            packed_name = string.join(nameparts, '')
            if len(packed_name) < 32:
                packed_name = packed_name + (32-len(packed_name))*'\0'
            elif len(packed_name) > 31:
                packed_name = packed_name[:31] + '\0'
        else:
            nameval = (len(name) > 31 and (name[:31] + '0')) or name
            packed_name = struct.pack('>32s', name)
        hdr = packed_name + struct.pack('>hhLLLlll4s4sllh',
                                        flg,
                                        info.get('version',0),
                                        info.get('createDate',0L)+PILOT_TIME_DELTA,
                                        info.get('modifyDate',0L)+PILOT_TIME_DELTA,
                                        info.get('backupDate',0L)+PILOT_TIME_DELTA,
                                        info.get('modnum',0),
                                        appinfo_offset, # appinfo
                                        sortinfo_offset, # sortinfo
                                        info.get('type','    '),
                                        info.get('creator','    '),
                                        0, # uid???
                                        0, # nextrec???
                                        len(self.data))

        f.write(hdr)

        entries = []
        record_data = []
        rsrc = self.info.get('flagResource')
        for x, off in map(None, self.data, rec_offsets):
            if rsrc:
                record_data.append(x.raw)
                entries.append(struct.pack('>4shl', x.type, x.id, off))
            else:
                record_data.append(x.raw)
                a = ((x.attr | x.category) << 24) | x.id
                entries.append(struct.pack('>ll', off, a))

        for x in entries: f.write(x)
        # Two bytes padding (if the PDB come from the Pilot the padding are also there)
        f.write(chr(0)+chr(0))
        f.write(self.appblock)
        f.write(self.sortblock)
        for x in record_data: f.write(x)
        f.flush()
