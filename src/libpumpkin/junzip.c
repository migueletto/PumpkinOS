// JUnzip library by Joonas Pihlajamaa. See junzip.h for license and details.

#include "sys.h"
#include "junzip.h"
#include "debug.h"

unsigned char jzBuffer[JZ_BUFFER_SIZE]; // limits maximum zip descriptor size

// Read ZIP file end record. Will move within file.
int jzReadEndRecord(JZFile *zip, JZEndRecord *endRecord) {
    int32_t fileSize, readBytes, i;
    JZEndRecord *er;

    if(zip->seek(zip, 0, SYS_SEEK_END)) {
        debug(DEBUG_ERROR, "unzip", "couldn't go to end of zip file");
        return Z_ERRNO;
    }

    if((fileSize = zip->tell(zip)) <= sizeof(JZEndRecord)) {
        debug(DEBUG_ERROR, "unzip", "too small file to be a zip");
        return Z_ERRNO;
    }

    readBytes = (fileSize < sizeof(jzBuffer)) ? fileSize : sizeof(jzBuffer);

    if(zip->seek(zip, fileSize - readBytes, SYS_SEEK_SET)) {
        debug(DEBUG_ERROR, "unzip", "cannot seek in zip file");
        return Z_ERRNO;
    }

    if(zip->read(zip, jzBuffer, readBytes) < readBytes) {
        debug(DEBUG_ERROR, "unzip", "couldn't read end of zip file");
        return Z_ERRNO;
    }

    // Naively assume signature can only be found in one place...
    for(i = readBytes - sizeof(JZEndRecord); i >= 0; i--) {
        er = (JZEndRecord *)(jzBuffer + i);
        if(er->signature == 0x06054B50)
            break;
    }

    if(i < 0) {
        debug(DEBUG_ERROR, "unzip", "end record signature not found in zip");
        return Z_ERRNO;
    }

    sys_memcpy(endRecord, er, sizeof(JZEndRecord));

    if(endRecord->diskNumber || endRecord->centralDirectoryDiskNumber ||
            endRecord->numEntries != endRecord->numEntriesThisDisk) {
        debug(DEBUG_ERROR, "unzip", "multifile zips not supported");
        return Z_ERRNO;
    }

    return Z_OK;
}

// Read ZIP file global directory. Will move within file.
int jzReadCentralDirectory(JZFile *zip, JZEndRecord *endRecord,
        JZRecordCallback callback, void *user_data) {
    JZGlobalFileHeader fileHeader;
    JZFileHeader header;
    int i;

    if(zip->seek(zip, endRecord->centralDirectoryOffset, SYS_SEEK_SET)) {
        debug(DEBUG_ERROR, "unzip", "cannot seek in zip file");
        return Z_ERRNO;
    }

    for(i=0; i<endRecord->numEntries; i++) {
        if(zip->read(zip, &fileHeader, sizeof(JZGlobalFileHeader)) <
                sizeof(JZGlobalFileHeader)) {
            debug(DEBUG_ERROR, "unzip", "couldn't read file header %d", i);
            return Z_ERRNO;
        }

        if(fileHeader.signature != 0x02014B50) {
            debug(DEBUG_ERROR, "unzip", "invalid file header signature %d", i);
            return Z_ERRNO;
        }

        if(fileHeader.fileNameLength + 1 >= JZ_BUFFER_SIZE) {
            debug(DEBUG_ERROR, "unzip", "too long file name %d", i);
            return Z_ERRNO;
        }

        if(zip->read(zip, jzBuffer, fileHeader.fileNameLength) <
                fileHeader.fileNameLength) {
            debug(DEBUG_ERROR, "unzip", "couldn't read filename %d", i);
            return Z_ERRNO;
        }

        jzBuffer[fileHeader.fileNameLength] = '\0'; // NULL terminate

        if(zip->seek(zip, fileHeader.extraFieldLength, SYS_SEEK_CUR) ||
                zip->seek(zip, fileHeader.fileCommentLength, SYS_SEEK_CUR)) {
            debug(DEBUG_ERROR, "unzip", "couldn't skip extra field or file comment %d", i);
            return Z_ERRNO;
        }

        // Construct JZFileHeader from global file header
        sys_memcpy(&header, &fileHeader.compressionMethod, sizeof(header));
        header.offset = fileHeader.relativeOffsetOflocalHeader;

        if(!callback(zip, i, &header, (char *)jzBuffer, user_data))
            break; // end if callback returns zero
    }

    return Z_OK;
}

// Read local ZIP file header. Silent on errors so optimistic reading possible.
int jzReadLocalFileHeaderRaw(JZFile *zip, JZLocalFileHeader *header,
        char *filename, int len) {

    if(zip->read(zip, header, sizeof(JZLocalFileHeader)) <
            sizeof(JZLocalFileHeader))
        return Z_ERRNO;

    if(header->signature != 0x04034B50)
        return Z_ERRNO;

    if(len) { // read filename
        if(header->fileNameLength >= len)
            return Z_ERRNO; // filename cannot fit

        if(zip->read(zip, filename, header->fileNameLength) <
                header->fileNameLength)
            return Z_ERRNO; // read fail

        filename[header->fileNameLength] = '\0'; // NULL terminate
    } else { // skip filename
        if(zip->seek(zip, header->fileNameLength, SYS_SEEK_CUR))
            return Z_ERRNO;
    }

    if(header->extraFieldLength) {
        if(zip->seek(zip, header->extraFieldLength, SYS_SEEK_CUR))
            return Z_ERRNO;
    }

    // For now, silently ignore bit flags and hope ZLIB can uncompress
    // if(header->generalPurposeBitFlag)
    //     return Z_ERRNO; // Flags not supported

    if(header->compressionMethod == 0 &&
            (header->compressedSize != header->uncompressedSize))
        return Z_ERRNO; // Method is "store" but sizes indicate otherwise, abort

    return Z_OK;
}

int jzReadLocalFileHeader(JZFile *zip, JZFileHeader *header,
        char *filename, int len) {
    JZLocalFileHeader localHeader;

    if(jzReadLocalFileHeaderRaw(zip, &localHeader, filename, len) != Z_OK)
        return Z_ERRNO;

    sys_memcpy(header, &localHeader.compressionMethod, sizeof(JZFileHeader));
    header->offset = 0; // not used in local context

    return Z_OK;
}

// Read data from file stream, described by header, to preallocated buffer
int jzReadData(JZFile *zip, JZFileHeader *header, void *buffer) {
    if(header->compressionMethod == 0) { // Store - just read it
        if(zip->read(zip, buffer, header->uncompressedSize) <
                header->uncompressedSize || zip->error(zip))
            return Z_ERRNO;
    } else if(header->compressionMethod == 8) { // Deflate - using puff()
        uint32_t destlen = header->uncompressedSize,
                      sourcelen = header->compressedSize;
        uint8_t *comp = (uint8_t *)sys_malloc(sourcelen);
        if(comp == NULL) return Z_ERRNO; // couldn't allocate
        sys_size_t read = zip->read(zip, comp, sourcelen);
        if(read != sourcelen) return Z_ERRNO; // TODO: more robust read loop
        int ret = puff((uint8_t *)buffer, &destlen, comp, &sourcelen);
        sys_free(comp);
        if(ret) return Z_ERRNO; // something went wrong
    } else {
        return Z_ERRNO;
    }

    return Z_OK;
}
