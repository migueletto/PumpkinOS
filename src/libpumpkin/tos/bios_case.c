    case 0: { // void Getmpb(MPB *ptr)
        int valid = 0;
        uint32_t aptr = ARG32;
        MPB *ptr = (MPB *)(memory + aptr);
        valid |= (uint8_t *)ptr >= data->memory && (uint8_t *)ptr < data->memory + data->memorySize;
        if (valid) {
          Getmpb(ptr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Getmpb(0x%08X)", aptr);
      }
      break;

    case 1: { // int16_t Bconstat(int16_t dev)
        int16_t dev = ARG16;
        int16_t res = 0;
        res = Bconstat(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconstat(%d): %d", dev, res);
      }
      break;

    case 2: { // int32_t Bconin(int16_t dev)
        int16_t dev = ARG16;
        int32_t res = 0;
        res = Bconin(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconin(%d): %d", dev, res);
      }
      break;

    case 3: { // void Bconout(int16_t dev, int16_t c)
        int16_t dev = ARG16;
        int16_t c = ARG16;
        Bconout(dev, c);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconout(%d, %d)", dev, c);
      }
      break;

    case 4: { // int32_t Rwabs(int16_t rwflag, void *buff, int16_t cnt, int16_t recnr, int16_t dev, int32_t lrecno)
        int valid = 0;
        int16_t rwflag = ARG16;
        uint32_t abuff = ARG32;
        void *buff = (void *)(memory + abuff);
        valid |= (uint8_t *)buff >= data->memory && (uint8_t *)buff < data->memory + data->memorySize;
        int16_t cnt = ARG16;
        int16_t recnr = ARG16;
        int16_t dev = ARG16;
        int32_t lrecno = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Rwabs(rwflag, buff, cnt, recnr, dev, lrecno);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Rwabs(%d, 0x%08X, %d, %d, %d, %d): %d", rwflag, abuff, cnt, recnr, dev, lrecno, res);
      }
      break;

    case 5: { // int32_t Setexc(int16_t number, void *vec)
        int valid = 0;
        int16_t number = ARG16;
        uint32_t avec = ARG32;
        void *vec = (void *)(memory + avec);
        valid |= (uint8_t *)vec >= data->memory && (uint8_t *)vec < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Setexc(number, vec);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setexc(%d, 0x%08X): %d", number, avec, res);
      }
      break;

    case 6: { // int32_t Tickcal(void)
        int32_t res = 0;
        res = Tickcal();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tickcal(): %d", res);
      }
      break;

    case 7: { // BPB *Getbpb(int16_t dev)
        int16_t dev = ARG16;
        BPB *res = NULL;
        res = Getbpb(dev);
        uint32_t ares = res ? ((uint8_t *)res - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Getbpb(%d): 0x%08X", dev, ares);
      }
      break;

    case 8: { // int32_t Bcostat(int16_t dev)
        int16_t dev = ARG16;
        int32_t res = 0;
        res = Bcostat(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bcostat(%d): %d", dev, res);
      }
      break;

    case 9: { // int32_t Mediach(int16_t dev)
        int16_t dev = ARG16;
        int32_t res = 0;
        res = Mediach(dev);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mediach(%d): %d", dev, res);
      }
      break;

    case 10: { // int32_t Drvmap(void)
        int32_t res = 0;
        res = Drvmap();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Drvmap(): %d", res);
      }
      break;

    case 11: { // int32_t Kbshift(int16_t mode)
        int16_t mode = ARG16;
        int32_t res = 0;
        res = Kbshift(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Kbshift(%d): %d", mode, res);
      }
      break;

