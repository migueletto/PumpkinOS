void SatInit(Boolean highDensity, Int16 sx, Int16 sy, Int16 xsky, Int16 ysky, Int16 rsky, Int16 tsky, WinHandle whBuf);
void SatColor(Int16 whiteColor, Int16 blackColor, Int16 usedColor, Int16 validColor, Int16 invalidColor);
void SatDrawChannels(int16_t n, ChannelSummary *chs);
void SatDrawSky(int16_t n, VisibleSatellite *sat, ChannelSummary *chs);
void SatResetChannels(ChannelSummary *chs, VisibleSatellite *sat);
