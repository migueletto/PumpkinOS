#define zipRsc 'ZipF'

int pumpkin_unzip_memory(UInt8 *p, UInt32 size, char *dir);
int pumpkin_unzip_resource(UInt32 type, UInt16 id, char *dir);
int pumpkin_unzip_filename(char *filename, char *dir);
int pumpkin_unzip_file(FileRef f, char *dir);
