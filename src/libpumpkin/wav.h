#define WAV_HEADER_SIZE 44

Boolean WavBufferHeader(UInt8 *header, UInt32 *rate, SndSampleType *type, SndStreamWidth *width);
Boolean WavFileHeader(FileRef fileRef, UInt32 *rate, SndSampleType *type, SndStreamWidth *width);
