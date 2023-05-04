#include <PalmOS.h>
#include <VFSMgr.h>

#include "wav.h"
#include "bytes.h"
#include "debug.h"

#define WAV_HEADER_SIZE 44

#define RIFF_id 0x52494646
#define WAVE_id 0x57415645
#define fmt_id  0x666d7420
#define data_id 0x64617461

Boolean WavBufferHeader(UInt8 *header, UInt32 *rate, SndSampleType *type, SndStreamWidth *width) {
  UInt32 chunkID, chunkSize, format;
  UInt32 subChunk1ID, subChunk1Size, sampleRate, byteRate;
  UInt32 subChunk2ID, subChunk2Size;
  UInt16 audioFormat, numChannels, blockAlign, bitsPerSample;
  Boolean r = false;
  int i = 0;

  if (header && rate && type && width) {
    i += get4b(&chunkID, header, i);
    i += get4l(&chunkSize, header, i);
    i += get4b(&format, header, i);
    i += get4b(&subChunk1ID, header, i);
    i += get4l(&subChunk1Size, header, i);
    i += get2l(&audioFormat, header, i);
    i += get2l(&numChannels, header, i);
    i += get4l(&sampleRate, header, i);
    i += get4l(&byteRate, header, i);
    i += get2l(&blockAlign, header, i);
    i += get2l(&bitsPerSample, header, i);
    i += get4b(&subChunk2ID, header, i);
    i += get4l(&subChunk2Size, header, i);

    r = ((chunkID == RIFF_id) &&
         (chunkSize >= (36 + subChunk2Size)) &&
         (format == WAVE_id) &&
         (subChunk1ID == fmt_id) &&
         (subChunk1Size == 16) &&
         (audioFormat == 1) &&
         (numChannels == 1 || numChannels == 2) &&
         (byteRate == (sampleRate * numChannels * bitsPerSample / 8)) &&
         (blockAlign == (numChannels * bitsPerSample / 8)) &&
         (bitsPerSample == 8 || bitsPerSample == 16) &&
         (subChunk2ID == data_id));

    if (r) {
      *type = (bitsPerSample == 8) ? sndInt8 : sndInt16;
      *width = (numChannels == 1) ? sndMono : sndStereo;
      *rate = sampleRate;
    }
  }

  return r;
}

Boolean WavFileHeader(FileRef fileRef, UInt32 *rate, SndSampleType *type, SndStreamWidth *width) {
  UInt8 header[WAV_HEADER_SIZE];
  UInt32 numBytesRead;
  Boolean r = false;

debug(1, "XXX", "WavFileHeader begin");
  if (VFSFileRead(fileRef, WAV_HEADER_SIZE, header, &numBytesRead) == errNone && numBytesRead == WAV_HEADER_SIZE) {
debug(1, "XXX", "WavFileHeader read ok");
    r = WavBufferHeader(header, rate, type, width);
debug(1, "XXX", "WavFileHeader header r=%d", r);
  }
debug(1, "XXX", "WavFileHeader end r=%d", r);

  return r;
}
