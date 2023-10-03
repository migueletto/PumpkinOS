static UInt8 GetByte(Hardware *hardware, UInt16 a) {
  UInt8 b;

  switch (hardware->memmode) {
    case 1:	// apple2p
      if (a < 0x8000)
        return hardware->m0[a];
      if (a < 0xC000)
        return hardware->m1[a & 0x7FFF];
      if (a >= 0xC100)
        return hardware->m1[a & 0x7FFF];
      b = IO_readb(hardware, a & 0xFF);
      return b;

    case 2:	// vic20
      if (a < 0x8000)
        return hardware->m0[a];
      if (a < 0x9000)
        return hardware->m1[a & 0x7FFF];
      if (a < 0x9400)
        return IO_readb(hardware, a);
      return hardware->m1[a & 0x7FFF];

    case 3:	// orica
      if ((a & 0xFF00) == 0x0300)
        return IO_readb(hardware, a);
      if (a < 0x8000)
        return hardware->m0[a];
      if (a < 0xC000)
        return hardware->m1[a & 0x7FFF];
      return hardware->m1[a & 0x7FFF];

    case 4:	// atom
      if (a < 0x8000)
        return hardware->m0[a];
      if (a >= 0xB000 && a < 0xBC00)
        return IO_readb(hardware, a);
      return hardware->m1[a & 0x7FFF];

    case 5:	// nes
      if (a >= 0x8000)	// CART
        return hardware->p[a >> 12][a & 0x0FFF];
      if (a >= 0x6000)	// SRAM
        return hardware->m0[a];
      if (a < 0x2000)
        return hardware->m0[a & 0x07FF];
      if (a < 0x4000)
        return IO_readb(hardware, 0x2000 + (a & 0x0007));
      if (a <= 0x4016)
        return IO_readb(hardware, a);
  }

  return 0;
}

static void SetByte(Hardware *hardware, UInt16 a, UInt8 b) {
  switch (hardware->memmode) {
    case 1:	// apple2p
      if (a >= hardware->gp_begin && a < hardware->gp_end) {
        hardware->m0[a] = b;
        hardware->dirty = 1;
      } else if (a < 0x8000)
        hardware->m0[a] = b;
      else if (a < 0xC000)
        hardware->m1[a & 0x7FFF] = b;
      else if (a < 0xC100)
        IO_writeb(hardware, a & 0xFF, b);
      break;

    case 2:	// vic20
      if (a < 0x8000)
        hardware->m0[a] = b;
      else if (a >= 0x9000 && a < 0x9400)
        IO_writeb(hardware, a, b);
      else if (a >= 0x9400 && a < 0x9800)
        hardware->m1[a & 0x7FFF] = b;
      break;

    case 3:	// orica
      if ((a & 0xFF00) == 0x0300)
        IO_writeb(hardware, a, b);
      else if (a < 0x8000)
        hardware->m0[a] = b;
      else if (a < 0xC000)
        hardware->m1[a & 0x7FFF] = b;
      break;

    case 4:	// atom
      if (a < 0x400 || (a >= 0x2800 && a < 0x3C00))
        hardware->m0[a] = b;
      else if (a >= 0x8000 && a < 0x9800) {
        hardware->m1[a & 0x7FFF] = b;
        if (a < hardware->gp_end)
          hardware->dirty = 1;
      } else if (a >= 0xB000 && a < 0xBC00)
        IO_writeb(hardware, a, b);
      break;

    case 5:	// nes
      if (a < 0x2000)
        hardware->m0[a & 0x07FF] = b;
      else if (a < 0x4000)
        IO_writeb(hardware, 0x2000 + (a & 0x0007), b);
      else if (a <= 0x4016)
        IO_writeb(hardware, a, b);
      else if (a >= 0x6000 && a < 0x8000 && hardware->banksw[0]) // SRAM
        hardware->m0[a] = b;
      else if (a >= 0x8000 && hardware->banksw[1]) // usa memory mapper
        IO_writeb(hardware, a, b);
  }
}
