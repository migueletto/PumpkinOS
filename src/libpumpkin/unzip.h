#define zipRsc 'ZipF'

int pumpkin_unzip_memory(UInt8 *p, UInt32 size, UInt16 volRefNum, char *dir);
int pumpkin_unzip_resource(UInt32 type, UInt16 id, UInt16 volRefNum, char *dir);
int pumpkin_unzip_filename(UInt16 srcVolRefNum, char *filename, UInt16 dstVolRefNum, char *dir);
int pumpkin_unzip_file(FileRef f, UInt16 volRefNum, char *dir);
