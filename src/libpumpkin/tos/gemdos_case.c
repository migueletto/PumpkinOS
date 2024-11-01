    case 0: { // void Pterm0(void)
        Pterm0();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pterm0()");
      }
      break;

    case 1: { // int32_t Cconin(void)
        int32_t res = 0;
        res = Cconin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconin(): %d", res);
      }
      break;

    case 2: { // int32_t Cconout(int16_t c)
        int16_t c = ARG16;
        int32_t res = 0;
        res = Cconout(c);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconout(%d): %d", c, res);
      }
      break;

    case 3: { // int32_t Cauxin(void)
        int32_t res = 0;
        res = Cauxin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxin(): %d", res);
      }
      break;

    case 4: { // int32_t Cauxout(int16_t c)
        int16_t c = ARG16;
        int32_t res = 0;
        res = Cauxout(c);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxout(%d): %d", c, res);
      }
      break;

    case 5: { // int32_t Cprnout(int16_t c)
        int16_t c = ARG16;
        int32_t res = 0;
        res = Cprnout(c);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cprnout(%d): %d", c, res);
      }
      break;

    case 6: { // int32_t Crawio(int16_t w)
        int16_t w = ARG16;
        int32_t res = 0;
        res = Crawio(w);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Crawio(%d): %d", w, res);
      }
      break;

    case 7: { // int32_t Crawcin(void)
        int32_t res = 0;
        res = Crawcin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Crawcin(): %d", res);
      }
      break;

    case 8: { // int32_t Cnecin(void)
        int32_t res = 0;
        res = Cnecin();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cnecin(): %d", res);
      }
      break;

    case 9: { // int32_t Cconws(uint8_t *buf)
        int valid = 0;
        uint32_t abuf = ARG32;
        uint8_t *buf = (uint8_t *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Cconws(buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconws(0x%08X): %d", abuf, res);
      }
      break;

    case 10: { // int32_t Cconrs(void)
        int32_t res = 0;
        res = Cconrs();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconrs(): %d", res);
      }
      break;

    case 11: { // int32_t Cconis(void)
        int32_t res = 0;
        res = Cconis();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconis(): %d", res);
      }
      break;

    case 14: { // int32_t Dsetdrv(int16_t drv)
        int16_t drv = ARG16;
        int32_t res = 0;
        res = Dsetdrv(drv);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsetdrv(%d): %d", drv, res);
      }
      break;

    case 16: { // int16_t Cconos(void)
        int16_t res = 0;
        res = Cconos();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cconos(): %d", res);
      }
      break;

    case 17: { // int16_t Cprnos(void)
        int16_t res = 0;
        res = Cprnos();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cprnos(): %d", res);
      }
      break;

    case 18: { // int16_t Cauxis(void)
        int16_t res = 0;
        res = Cauxis();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxis(): %d", res);
      }
      break;

    case 19: { // int16_t Cauxos(void)
        int16_t res = 0;
        res = Cauxos();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cauxos(): %d", res);
      }
      break;

    case 20: { // int32_t Maddalt(void *start, int32_t size)
        int valid = 0;
        uint32_t astart = ARG32;
        void *start = (void *)(ram + astart);
        valid |= (uint8_t *)start >= data->block && (uint8_t *)start < data->block + data->blockSize;
        int32_t size = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Maddalt(start, size);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Maddalt(0x%08X, %d): %d", astart, size, res);
      }
      break;

    case 21: { // int32_t Srealloc(int32_t len)
        int32_t len = ARG32;
        int32_t res = 0;
        res = Srealloc(len);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Srealloc(%d): %d", len, res);
      }
      break;

    case 22: { // int32_t Slbopen(char *name, char *path, int32_t min_ver, SHARED_LIB *sl, SLB_EXEC *fn)
        int valid = 0;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int32_t min_ver = ARG32;
        uint32_t asl = ARG32;
        SHARED_LIB *sl = (SHARED_LIB *)(ram + asl);
        valid |= (uint8_t *)sl >= data->block && (uint8_t *)sl < data->block + data->blockSize;
        uint32_t afn = ARG32;
        SLB_EXEC *fn = (SLB_EXEC *)(ram + afn);
        valid |= (uint8_t *)fn >= data->block && (uint8_t *)fn < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Slbopen(name, path, min_ver, sl, fn);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Slbopen(0x%08X, 0x%08X, %d, 0x%08X, 0x%08X): %d", aname, apath, min_ver, asl, afn, res);
      }
      break;

    case 23: { // int32_t Slbclose(SHARED_LIB *sl)
        int valid = 0;
        uint32_t asl = ARG32;
        SHARED_LIB *sl = (SHARED_LIB *)(ram + asl);
        valid |= (uint8_t *)sl >= data->block && (uint8_t *)sl < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Slbclose(sl);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Slbclose(0x%08X): %d", asl, res);
      }
      break;

    case 25: { // int16_t Dgetdrv(void)
        int16_t res = 0;
        res = Dgetdrv();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dgetdrv(): %d", res);
      }
      break;

    case 26: { // void Fsetdta(DTA *buf)
        int valid = 0;
        uint32_t abuf = ARG32;
        DTA *buf = (DTA *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        if (valid) {
          Fsetdta(buf);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsetdta(0x%08X)", abuf);
      }
      break;

    case 32: { // int32_t Super(void *stack)
        int valid = 0;
        uint32_t astack = ARG32;
        void *stack = (void *)(ram + astack);
        valid |= (uint8_t *)stack >= data->block && (uint8_t *)stack < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Super(stack);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Super(0x%08X): %d", astack, res);
      }
      break;

    case 42: { // uint16_t Tgetdate(void)
        uint16_t res = 0;
        res = Tgetdate();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tgetdate(): %d", res);
      }
      break;

    case 43: { // int16_t Tsetdate(uint16_t date)
        uint16_t date = ARG16;
        int16_t res = 0;
        res = Tsetdate(date);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsetdate(%d): %d", date, res);
      }
      break;

    case 44: { // uint32_t Tgettime(void)
        uint32_t res = 0;
        res = Tgettime();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tgettime(): %d", res);
      }
      break;

    case 45: { // int16_t Tsettime(uint16_t time)
        uint16_t time = ARG16;
        int16_t res = 0;
        res = Tsettime(time);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsettime(%d): %d", time, res);
      }
      break;

    case 47: { // DTA *Fgetdta(void)
        DTA *res = NULL;
        res = Fgetdta();
        uint32_t ares = res ? ((uint8_t *)res - ram) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetdta(): 0x%08X", ares);
      }
      break;

    case 48: { // uint16_t Sversion(void)
        uint16_t res = 0;
        res = Sversion();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sversion(): %d", res);
      }
      break;

    case 49: { // void Ptermres(int32_t keepcnt, int16_t retcode)
        int32_t keepcnt = ARG32;
        int16_t retcode = ARG16;
        Ptermres(keepcnt, retcode);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ptermres(%d, %d)", keepcnt, retcode);
      }
      break;

    case 51: { // int32_t Sconfig(int16_t mode, int32_t flags)
        int16_t mode = ARG16;
        int32_t flags = ARG32;
        int32_t res = 0;
        res = Sconfig(mode, flags);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sconfig(%d, %d): %d", mode, flags, res);
      }
      break;

    case 54: { // int16_t Dfree(DISKINFO *buf, int16_t driveno)
        int valid = 0;
        uint32_t abuf = ARG32;
        DISKINFO *buf = (DISKINFO *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int16_t driveno = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Dfree(buf, driveno);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dfree(0x%08X, %d): %d", abuf, driveno, res);
      }
      break;

    case 57: { // int32_t Dcreate(char *path)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Dcreate(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dcreate(0x%08X): %d", apath, res);
      }
      break;

    case 58: { // int32_t Ddelete(char *path)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Ddelete(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ddelete(0x%08X): %d", apath, res);
      }
      break;

    case 59: { // int16_t Dsetpath(char *path)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int16_t res = 0;
        if (valid) {
          res = Dsetpath(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsetpath(0x%08X): %d", apath, res);
      }
      break;

    case 60: { // int16_t Fcreate(char *fname, int16_t attr)
        int valid = 0;
        uint32_t afname = ARG32;
        char *fname = (char *)(ram + afname);
        valid |= (uint8_t *)fname >= data->block && (uint8_t *)fname < data->block + data->blockSize;
        int16_t attr = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Fcreate(fname, attr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fcreate(0x%08X, %d): %d", afname, attr, res);
      }
      break;

    case 61: { // int32_t Fopen(char *fname, int16_t mode)
        int valid = 0;
        uint32_t afname = ARG32;
        char *fname = (char *)(ram + afname);
        valid |= (uint8_t *)fname >= data->block && (uint8_t *)fname < data->block + data->blockSize;
        int16_t mode = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Fopen(fname, mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fopen(0x%08X, %d): %d", afname, mode, res);
      }
      break;

    case 62: { // int16_t Fclose(int16_t handle)
        int16_t handle = ARG16;
        int16_t res = 0;
        res = Fclose(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fclose(%d): %d", handle, res);
      }
      break;

    case 63: { // int32_t Fread(int16_t handle, int32_t count, void *buf)
        int valid = 0;
        int16_t handle = ARG16;
        int32_t count = ARG32;
        uint32_t abuf = ARG32;
        void *buf = (void *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fread(handle, count, buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fread(%d, %d, 0x%08X): %d", handle, count, abuf, res);
      }
      break;

    case 64: { // int32_t Fwrite(int16_t handle, int32_t count, void *buf)
        int valid = 0;
        int16_t handle = ARG16;
        int32_t count = ARG32;
        uint32_t abuf = ARG32;
        void *buf = (void *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fwrite(handle, count, buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fwrite(%d, %d, 0x%08X): %d", handle, count, abuf, res);
      }
      break;

    case 65: { // int16_t Fdelete(char *fname)
        int valid = 0;
        uint32_t afname = ARG32;
        char *fname = (char *)(ram + afname);
        valid |= (uint8_t *)fname >= data->block && (uint8_t *)fname < data->block + data->blockSize;
        int16_t res = 0;
        if (valid) {
          res = Fdelete(fname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdelete(0x%08X): %d", afname, res);
      }
      break;

    case 66: { // int32_t Fseek(int32_t offset, int16_t handle, int16_t seekmode)
        int32_t offset = ARG32;
        int16_t handle = ARG16;
        int16_t seekmode = ARG16;
        int32_t res = 0;
        res = Fseek(offset, handle, seekmode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fseek(%d, %d, %d): %d", offset, handle, seekmode, res);
      }
      break;

    case 67: { // int16_t Fattrib(char *filename, int16_t wflag, int16_t attrib)
        int valid = 0;
        uint32_t afilename = ARG32;
        char *filename = (char *)(ram + afilename);
        valid |= (uint8_t *)filename >= data->block && (uint8_t *)filename < data->block + data->blockSize;
        int16_t wflag = ARG16;
        int16_t attrib = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Fattrib(filename, wflag, attrib);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fattrib(0x%08X, %d, %d): %d", afilename, wflag, attrib, res);
      }
      break;

    case 68: { // void *Mxalloc(int32_t amount, int16_t mode)
        int32_t amount = ARG32;
        int16_t mode = ARG16;
        void *res = NULL;
        res = Mxalloc(amount, mode);
        uint32_t ares = res ? ((uint8_t *)res - ram) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mxalloc(%d, %d): 0x%08X", amount, mode, ares);
      }
      break;

    case 69: { // int16_t Fdup(int16_t handle)
        int16_t handle = ARG16;
        int16_t res = 0;
        res = Fdup(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdup(%d): %d", handle, res);
      }
      break;

    case 70: { // int16_t Fforce(int16_t stdh, int16_t nonstdh)
        int16_t stdh = ARG16;
        int16_t nonstdh = ARG16;
        int16_t res = 0;
        res = Fforce(stdh, nonstdh);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fforce(%d, %d): %d", stdh, nonstdh, res);
      }
      break;

    case 71: { // int16_t Dgetpath(char *path, int16_t driveno)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int16_t driveno = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Dgetpath(path, driveno);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dgetpath(0x%08X, %d): %d", apath, driveno, res);
      }
      break;

    case 72: { // void *Malloc(int32_t number)
        int32_t number = ARG32;
        void *res = NULL;
        res = Malloc(number);
        uint32_t ares = res ? ((uint8_t *)res - ram) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Malloc(%d): 0x%08X", number, ares);
      }
      break;

    case 73: { // int32_t Mfree(void *block)
        int valid = 0;
        uint32_t ablock = ARG32;
        void *block = (void *)(ram + ablock);
        valid |= (uint8_t *)block >= data->block && (uint8_t *)block < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Mfree(block);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mfree(0x%08X): %d", ablock, res);
      }
      break;

    case 74: { // int32_t Mshrink(void *block, int32_t newsiz)
        int valid = 0;
        uint32_t ablock = ARG32;
        void *block = (void *)(ram + ablock);
        valid |= (uint8_t *)block >= data->block && (uint8_t *)block < data->block + data->blockSize;
        int32_t newsiz = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Mshrink(block, newsiz);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mshrink(0x%08X, %d): %d", ablock, newsiz, res);
      }
      break;

    case 76: { // void Pterm(uint16_t retcode)
        uint16_t retcode = ARG16;
        Pterm(retcode);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pterm(%d)", retcode);
      }
      break;

    case 78: { // int32_t Fsfirst(char *filename, int16_t attr)
        int valid = 0;
        uint32_t afilename = ARG32;
        char *filename = (char *)(ram + afilename);
        valid |= (uint8_t *)filename >= data->block && (uint8_t *)filename < data->block + data->blockSize;
        int16_t attr = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Fsfirst(filename, attr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsfirst(0x%08X, %d): %d", afilename, attr, res);
      }
      break;

    case 79: { // int16_t Fsnext(void)
        int16_t res = 0;
        res = Fsnext();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsnext(): %d", res);
      }
      break;

    case 86: { // int32_t Frename(char *oldname, char *newname)
        int valid = 0;
        uint32_t aoldname = ARG32;
        char *oldname = (char *)(ram + aoldname);
        valid |= (uint8_t *)oldname >= data->block && (uint8_t *)oldname < data->block + data->blockSize;
        uint32_t anewname = ARG32;
        char *newname = (char *)(ram + anewname);
        valid |= (uint8_t *)newname >= data->block && (uint8_t *)newname < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Frename(oldname, newname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frename(0x%08X, 0x%08X): %d", aoldname, anewname, res);
      }
      break;

    case 87: { // void Fdatime(DOSTIME *timeptr, int16_t handle, int16_t wflag)
        int valid = 0;
        uint32_t atimeptr = ARG32;
        DOSTIME *timeptr = (DOSTIME *)(ram + atimeptr);
        valid |= (uint8_t *)timeptr >= data->block && (uint8_t *)timeptr < data->block + data->blockSize;
        int16_t handle = ARG16;
        int16_t wflag = ARG16;
        if (valid) {
          Fdatime(timeptr, handle, wflag);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdatime(0x%08X, %d, %d)", atimeptr, handle, wflag);
      }
      break;

    case 92: { // int32_t Flock(int16_t handle, int16_t mode, int32_t start, int32_t length)
        int16_t handle = ARG16;
        int16_t mode = ARG16;
        int32_t start = ARG32;
        int32_t length = ARG32;
        int32_t res = 0;
        res = Flock(handle, mode, start, length);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flock(%d, %d, %d, %d): %d", handle, mode, start, length, res);
      }
      break;

    case 96: { // int32_t Nversion(void)
        int32_t res = 0;
        res = Nversion();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Nversion(): %d", res);
      }
      break;

    case 98: { // int32_t Frlock(int16_t handle, int32_t start, int32_t length)
        int16_t handle = ARG16;
        int32_t start = ARG32;
        int32_t length = ARG32;
        int32_t res = 0;
        res = Frlock(handle, start, length);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frlock(%d, %d, %d): %d", handle, start, length, res);
      }
      break;

    case 99: { // int32_t Frunlock(int16_t handle, int32_t start)
        int16_t handle = ARG16;
        int32_t start = ARG32;
        int32_t res = 0;
        res = Frunlock(handle, start);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frunlock(%d, %d): %d", handle, start, res);
      }
      break;

    case 100: { // int32_t Flock2(int16_t handle, int32_t length)
        int16_t handle = ARG16;
        int32_t length = ARG32;
        int32_t res = 0;
        res = Flock2(handle, length);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flock2(%d, %d): %d", handle, length, res);
      }
      break;

    case 101: { // int32_t Funlock(int16_t handle)
        int16_t handle = ARG16;
        int32_t res = 0;
        res = Funlock(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Funlock(%d): %d", handle, res);
      }
      break;

    case 102: { // int32_t Fflush(int16_t handle)
        int16_t handle = ARG16;
        int32_t res = 0;
        res = Fflush(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fflush(%d): %d", handle, res);
      }
      break;

    case 255: { // void Syield(void)
        Syield();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Syield()");
      }
      break;

    case 256: { // int32_t Fpipe(int16_t *usrh)
        int valid = 0;
        uint32_t ausrh = ARG32;
        int16_t *usrh = (int16_t *)(ram + ausrh);
        valid |= (uint8_t *)usrh >= data->block && (uint8_t *)usrh < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fpipe(usrh);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fpipe(0x%08X): %d", ausrh, res);
      }
      break;

    case 257: { // int32_t Ffchown(int16_t fd, int16_t uid, int16_t gid)
        int16_t fd = ARG16;
        int16_t uid = ARG16;
        int16_t gid = ARG16;
        int32_t res = 0;
        res = Ffchown(fd, uid, gid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffchown(%d, %d, %d): %d", fd, uid, gid, res);
      }
      break;

    case 258: { // int32_t Ffchmod(int16_t fd, int16_t mode)
        int16_t fd = ARG16;
        int16_t mode = ARG16;
        int32_t res = 0;
        res = Ffchmod(fd, mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffchmod(%d, %d): %d", fd, mode, res);
      }
      break;

    case 259: { // int16_t Fsync(int16_t handle)
        int16_t handle = ARG16;
        int16_t res = 0;
        res = Fsync(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsync(%d): %d", handle, res);
      }
      break;

    case 261: { // int32_t Finstat(int16_t fh)
        int16_t fh = ARG16;
        int32_t res = 0;
        res = Finstat(fh);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Finstat(%d): %d", fh, res);
      }
      break;

    case 262: { // int32_t Foutstat(int16_t fh)
        int16_t fh = ARG16;
        int32_t res = 0;
        res = Foutstat(fh);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Foutstat(%d): %d", fh, res);
      }
      break;

    case 263: { // int32_t Fgetchar(int16_t fh, int16_t mode)
        int16_t fh = ARG16;
        int16_t mode = ARG16;
        int32_t res = 0;
        res = Fgetchar(fh, mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetchar(%d, %d): %d", fh, mode, res);
      }
      break;

    case 264: { // int32_t Fputchar(int16_t fh, int32_t ch, int16_t mode)
        int16_t fh = ARG16;
        int32_t ch = ARG32;
        int16_t mode = ARG16;
        int32_t res = 0;
        res = Fputchar(fh, ch, mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fputchar(%d, %d, %d): %d", fh, ch, mode, res);
      }
      break;

    case 265: { // int32_t Pwait(void)
        int32_t res = 0;
        res = Pwait();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pwait(): %d", res);
      }
      break;

    case 266: { // int16_t Pnice(int16_t delta)
        int16_t delta = ARG16;
        int16_t res = 0;
        res = Pnice(delta);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pnice(%d): %d", delta, res);
      }
      break;

    case 267: { // int16_t Pgetpid(void)
        int16_t res = 0;
        res = Pgetpid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetpid(): %d", res);
      }
      break;

    case 268: { // int16_t Pgetppid(void)
        int16_t res = 0;
        res = Pgetppid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetppid(): %d", res);
      }
      break;

    case 269: { // int16_t Pgetpgrp(void)
        int16_t res = 0;
        res = Pgetpgrp();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetpgrp(): %d", res);
      }
      break;

    case 270: { // int16_t Psetpgrp(int16_t pid, int16_t newgrp)
        int16_t pid = ARG16;
        int16_t newgrp = ARG16;
        int16_t res = 0;
        res = Psetpgrp(pid, newgrp);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetpgrp(%d, %d): %d", pid, newgrp, res);
      }
      break;

    case 271: { // int16_t Pgetuid(void)
        int16_t res = 0;
        res = Pgetuid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetuid(): %d", res);
      }
      break;

    case 272: { // int16_t Psetuid(int16_t id)
        int16_t id = ARG16;
        int16_t res = 0;
        res = Psetuid(id);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetuid(%d): %d", id, res);
      }
      break;

    case 273: { // int32_t Pkill(int16_t pid, int16_t sig)
        int16_t pid = ARG16;
        int16_t sig = ARG16;
        int32_t res = 0;
        res = Pkill(pid, sig);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pkill(%d, %d): %d", pid, sig, res);
      }
      break;

    case 274: { // int32_t Psignal(int16_t sig, int32_t handler)
        int16_t sig = ARG16;
        int32_t handler = ARG32;
        int32_t res = 0;
        res = Psignal(sig, handler);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psignal(%d, %d): %d", sig, handler, res);
      }
      break;

    case 275: { // int16_t Pvfork(void)
        int16_t res = 0;
        res = Pvfork();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pvfork(): %d", res);
      }
      break;

    case 276: { // int16_t Pgetgid(void)
        int16_t res = 0;
        res = Pgetgid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetgid(): %d", res);
      }
      break;

    case 277: { // int16_t Psetgid(int16_t id)
        int16_t id = ARG16;
        int16_t res = 0;
        res = Psetgid(id);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetgid(%d): %d", id, res);
      }
      break;

    case 278: { // int32_t Psigblock(int32_t mask)
        int32_t mask = ARG32;
        int32_t res = 0;
        res = Psigblock(mask);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigblock(%d): %d", mask, res);
      }
      break;

    case 279: { // int32_t Psigsetmask(int32_t mask)
        int32_t mask = ARG32;
        int32_t res = 0;
        res = Psigsetmask(mask);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigsetmask(%d): %d", mask, res);
      }
      break;

    case 280: { // int32_t Pusrval(int32_t val)
        int32_t val = ARG32;
        int32_t res = 0;
        res = Pusrval(val);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pusrval(%d): %d", val, res);
      }
      break;

    case 281: { // int16_t Pdomain(int16_t dom)
        int16_t dom = ARG16;
        int16_t res = 0;
        res = Pdomain(dom);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pdomain(%d): %d", dom, res);
      }
      break;

    case 282: { // void Psigreturn(void)
        Psigreturn();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigreturn()");
      }
      break;

    case 283: { // int16_t Pfork(void)
        int16_t res = 0;
        res = Pfork();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pfork(): %d", res);
      }
      break;

    case 284: { // int32_t Pwait3(int16_t flag, int32_t *rusage)
        int valid = 0;
        int16_t flag = ARG16;
        uint32_t arusage = ARG32;
        int32_t *rusage = (int32_t *)(ram + arusage);
        valid |= (uint8_t *)rusage >= data->block && (uint8_t *)rusage < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Pwait3(flag, rusage);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pwait3(%d, 0x%08X): %d", flag, arusage, res);
      }
      break;

    case 285: { // int32_t Fselect(uint16_t timeout, int32_t *rfds, int32_t *wfds)
        int valid = 0;
        uint16_t timeout = ARG16;
        uint32_t arfds = ARG32;
        int32_t *rfds = (int32_t *)(ram + arfds);
        valid |= (uint8_t *)rfds >= data->block && (uint8_t *)rfds < data->block + data->blockSize;
        uint32_t awfds = ARG32;
        int32_t *wfds = (int32_t *)(ram + awfds);
        valid |= (uint8_t *)wfds >= data->block && (uint8_t *)wfds < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fselect(timeout, rfds, wfds);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fselect(%d, 0x%08X, 0x%08X): %d", timeout, arfds, awfds, res);
      }
      break;

    case 286: { // int32_t Prusage(int32_t *r)
        int valid = 0;
        uint32_t ar = ARG32;
        int32_t *r = (int32_t *)(ram + ar);
        valid |= (uint8_t *)r >= data->block && (uint8_t *)r < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Prusage(r);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Prusage(0x%08X): %d", ar, res);
      }
      break;

    case 287: { // int32_t Psetlimit(int16_t lim, int32_t value)
        int16_t lim = ARG16;
        int32_t value = ARG32;
        int32_t res = 0;
        res = Psetlimit(lim, value);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetlimit(%d, %d): %d", lim, value, res);
      }
      break;

    case 288: { // int32_t Talarm(int32_t time)
        int32_t time = ARG32;
        int32_t res = 0;
        res = Talarm(time);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Talarm(%d): %d", time, res);
      }
      break;

    case 289: { // void Pause(void)
        Pause();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pause()");
      }
      break;

    case 290: { // int32_t Sysconf(int16_t n)
        int16_t n = ARG16;
        int32_t res = 0;
        res = Sysconf(n);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sysconf(%d): %d", n, res);
      }
      break;

    case 291: { // int32_t Psigpending(void)
        int32_t res = 0;
        res = Psigpending();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigpending(): %d", res);
      }
      break;

    case 292: { // int32_t Dpathconf(uint8_t *name, int16_t mode)
        int valid = 0;
        uint32_t aname = ARG32;
        uint8_t *name = (uint8_t *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int16_t mode = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Dpathconf(name, mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dpathconf(0x%08X, %d): %d", aname, mode, res);
      }
      break;

    case 293: { // int32_t Pmsg(int16_t mode, int32_t mbox, void *msg)
        int valid = 0;
        int16_t mode = ARG16;
        int32_t mbox = ARG32;
        uint32_t amsg = ARG32;
        void *msg = (void *)(ram + amsg);
        valid |= (uint8_t *)msg >= data->block && (uint8_t *)msg < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Pmsg(mode, mbox, msg);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pmsg(%d, %d, 0x%08X): %d", mode, mbox, amsg, res);
      }
      break;

    case 294: { // int32_t Fmidipipe(int16_t pid, int16_t in, int16_t out)
        int16_t pid = ARG16;
        int16_t in = ARG16;
        int16_t out = ARG16;
        int32_t res = 0;
        res = Fmidipipe(pid, in, out);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fmidipipe(%d, %d, %d): %d", pid, in, out, res);
      }
      break;

    case 295: { // int32_t Prenice(int16_t pid, int16_t delta)
        int16_t pid = ARG16;
        int16_t delta = ARG16;
        int32_t res = 0;
        res = Prenice(pid, delta);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Prenice(%d, %d): %d", pid, delta, res);
      }
      break;

    case 296: { // int32_t Dopendir(char *name, int16_t flag)
        int valid = 0;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int16_t flag = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Dopendir(name, flag);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dopendir(0x%08X, %d): %d", aname, flag, res);
      }
      break;

    case 297: { // int32_t Dreaddir(int16_t len, int32_t dirhandle, char *buf)
        int valid = 0;
        int16_t len = ARG16;
        int32_t dirhandle = ARG32;
        uint32_t abuf = ARG32;
        char *buf = (char *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Dreaddir(len, dirhandle, buf);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dreaddir(%d, %d, 0x%08X): %d", len, dirhandle, abuf, res);
      }
      break;

    case 298: { // int32_t Drewinddir(int32_t handle)
        int32_t handle = ARG32;
        int32_t res = 0;
        res = Drewinddir(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Drewinddir(%d): %d", handle, res);
      }
      break;

    case 299: { // int32_t Dclosedir(int32_t dirhandle)
        int32_t dirhandle = ARG32;
        int32_t res = 0;
        res = Dclosedir(dirhandle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dclosedir(%d): %d", dirhandle, res);
      }
      break;

    case 300: { // int32_t Fxattr(int16_t flag, char *name, XATTR *xattr)
        int valid = 0;
        int16_t flag = ARG16;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t axattr = ARG32;
        XATTR *xattr = (XATTR *)(ram + axattr);
        valid |= (uint8_t *)xattr >= data->block && (uint8_t *)xattr < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fxattr(flag, name, xattr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fxattr(%d, 0x%08X, 0x%08X): %d", flag, aname, axattr, res);
      }
      break;

    case 301: { // int32_t Flink(char *oldname, char *newname)
        int valid = 0;
        uint32_t aoldname = ARG32;
        char *oldname = (char *)(ram + aoldname);
        valid |= (uint8_t *)oldname >= data->block && (uint8_t *)oldname < data->block + data->blockSize;
        uint32_t anewname = ARG32;
        char *newname = (char *)(ram + anewname);
        valid |= (uint8_t *)newname >= data->block && (uint8_t *)newname < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Flink(oldname, newname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flink(0x%08X, 0x%08X): %d", aoldname, anewname, res);
      }
      break;

    case 302: { // int32_t Fsymlink(char *oldname, char *newname)
        int valid = 0;
        uint32_t aoldname = ARG32;
        char *oldname = (char *)(ram + aoldname);
        valid |= (uint8_t *)oldname >= data->block && (uint8_t *)oldname < data->block + data->blockSize;
        uint32_t anewname = ARG32;
        char *newname = (char *)(ram + anewname);
        valid |= (uint8_t *)newname >= data->block && (uint8_t *)newname < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fsymlink(oldname, newname);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsymlink(0x%08X, 0x%08X): %d", aoldname, anewname, res);
      }
      break;

    case 303: { // int32_t Freadlink(int16_t bufsiz, char *buf, char *name)
        int valid = 0;
        int16_t bufsiz = ARG16;
        uint32_t abuf = ARG32;
        char *buf = (char *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Freadlink(bufsiz, buf, name);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Freadlink(%d, 0x%08X, 0x%08X): %d", bufsiz, abuf, aname, res);
      }
      break;

    case 304: { // int32_t Dcntl(int16_t cmd, char *name, int32_t arg)
        int valid = 0;
        int16_t cmd = ARG16;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int32_t arg = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Dcntl(cmd, name, arg);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dcntl(%d, 0x%08X, %d): %d", cmd, aname, arg, res);
      }
      break;

    case 305: { // int32_t Fchown(char *name, int16_t uid, int16_t gid)
        int valid = 0;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int16_t uid = ARG16;
        int16_t gid = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Fchown(name, uid, gid);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchown(0x%08X, %d, %d): %d", aname, uid, gid, res);
      }
      break;

    case 306: { // int32_t Fchmod(char *name, int16_t mode)
        int valid = 0;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int16_t mode = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Fchmod(name, mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchmod(0x%08X, %d): %d", aname, mode, res);
      }
      break;

    case 307: { // int16_t Pumask(int16_t mode)
        int16_t mode = ARG16;
        int16_t res = 0;
        res = Pumask(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pumask(%d): %d", mode, res);
      }
      break;

    case 308: { // int32_t Psemaphore(int16_t mode, int32_t id, int32_t timeout)
        int16_t mode = ARG16;
        int32_t id = ARG32;
        int32_t timeout = ARG32;
        int32_t res = 0;
        res = Psemaphore(mode, id, timeout);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psemaphore(%d, %d, %d): %d", mode, id, timeout, res);
      }
      break;

    case 309: { // int32_t Dlock(int16_t mode, int16_t drv)
        int16_t mode = ARG16;
        int16_t drv = ARG16;
        int32_t res = 0;
        res = Dlock(mode, drv);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dlock(%d, %d): %d", mode, drv, res);
      }
      break;

    case 310: { // void Psigpause(int32_t mask)
        int32_t mask = ARG32;
        Psigpause(mask);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigpause(%d)", mask);
      }
      break;

    case 311: { // int32_t Psigaction(int16_t sig, struct sigaction *act, struct sigaction *oact)
        int valid = 0;
        int16_t sig = ARG16;
        uint32_t aact = ARG32;
        struct sigaction *act = (struct sigaction *)(ram + aact);
        valid |= (uint8_t *)act >= data->block && (uint8_t *)act < data->block + data->blockSize;
        uint32_t aoact = ARG32;
        struct sigaction *oact = (struct sigaction *)(ram + aoact);
        valid |= (uint8_t *)oact >= data->block && (uint8_t *)oact < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Psigaction(sig, act, oact);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigaction(%d, 0x%08X, 0x%08X): %d", sig, aact, aoact, res);
      }
      break;

    case 312: { // int32_t Pgeteuid(void)
        int32_t res = 0;
        res = Pgeteuid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgeteuid(): %d", res);
      }
      break;

    case 313: { // int32_t Pgetegid(void)
        int32_t res = 0;
        res = Pgetegid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetegid(): %d", res);
      }
      break;

    case 314: { // int32_t Pwaitpid(int16_t pid, int16_t flag, int32_t *rusage)
        int valid = 0;
        int16_t pid = ARG16;
        int16_t flag = ARG16;
        uint32_t arusage = ARG32;
        int32_t *rusage = (int32_t *)(ram + arusage);
        valid |= (uint8_t *)rusage >= data->block && (uint8_t *)rusage < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Pwaitpid(pid, flag, rusage);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pwaitpid(%d, %d, 0x%08X): %d", pid, flag, arusage, res);
      }
      break;

    case 315: { // int32_t Dgetcwd(char *path, int16_t drv, int16_t size)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int16_t drv = ARG16;
        int16_t size = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Dgetcwd(path, drv, size);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dgetcwd(0x%08X, %d, %d): %d", apath, drv, size, res);
      }
      break;

    case 316: { // void Salert(char *msg)
        int valid = 0;
        uint32_t amsg = ARG32;
        char *msg = (char *)(ram + amsg);
        valid |= (uint8_t *)msg >= data->block && (uint8_t *)msg < data->block + data->blockSize;
        if (valid) {
          Salert(msg);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Salert(0x%08X)", amsg);
      }
      break;

    case 317: { // int32_t Tmalarm(int32_t time)
        int32_t time = ARG32;
        int32_t res = 0;
        res = Tmalarm(time);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tmalarm(%d): %d", time, res);
      }
      break;

    case 318: { // int32_t Psigintr(int16_t vec, int16_t sig)
        int16_t vec = ARG16;
        int16_t sig = ARG16;
        int32_t res = 0;
        res = Psigintr(vec, sig);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psigintr(%d, %d): %d", vec, sig, res);
      }
      break;

    case 319: { // int32_t Suptime(int32_t *uptime, int32_t *loadaverage)
        int valid = 0;
        uint32_t auptime = ARG32;
        int32_t *uptime = (int32_t *)(ram + auptime);
        valid |= (uint8_t *)uptime >= data->block && (uint8_t *)uptime < data->block + data->blockSize;
        uint32_t aloadaverage = ARG32;
        int32_t *loadaverage = (int32_t *)(ram + aloadaverage);
        valid |= (uint8_t *)loadaverage >= data->block && (uint8_t *)loadaverage < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Suptime(uptime, loadaverage);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Suptime(0x%08X, 0x%08X): %d", auptime, aloadaverage, res);
      }
      break;

    case 320: { // int16_t Ptrace(int16_t request, int16_t pid, void *addr, int32_t data)
        int valid = 0;
        int16_t request = ARG16;
        int16_t pid = ARG16;
        uint32_t aaddr = ARG32;
        void *addr = (void *)(ram + aaddr);
        valid |= (uint8_t *)addr >= data->block && (uint8_t *)addr < data->block + data->blockSize;
        int32_t data = ARG32;
        int16_t res = 0;
        if (valid) {
          res = Ptrace(request, pid, addr, data);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ptrace(%d, %d, 0x%08X, %d): %d", request, pid, aaddr, data, res);
      }
      break;

    case 321: { // int32_t Mvalidate(int16_t pid, void *start, int32_t size, int32_t *flags)
        int valid = 0;
        int16_t pid = ARG16;
        uint32_t astart = ARG32;
        void *start = (void *)(ram + astart);
        valid |= (uint8_t *)start >= data->block && (uint8_t *)start < data->block + data->blockSize;
        int32_t size = ARG32;
        uint32_t aflags = ARG32;
        int32_t *flags = (int32_t *)(ram + aflags);
        valid |= (uint8_t *)flags >= data->block && (uint8_t *)flags < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Mvalidate(pid, start, size, flags);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mvalidate(%d, 0x%08X, %d, 0x%08X): %d", pid, astart, size, aflags, res);
      }
      break;

    case 322: { // int32_t Dxreaddir(int16_t ln, int32_t dirh, char *buf, XATTR *xattr, int32_t *xr)
        int valid = 0;
        int16_t ln = ARG16;
        int32_t dirh = ARG32;
        uint32_t abuf = ARG32;
        char *buf = (char *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        uint32_t axattr = ARG32;
        XATTR *xattr = (XATTR *)(ram + axattr);
        valid |= (uint8_t *)xattr >= data->block && (uint8_t *)xattr < data->block + data->blockSize;
        uint32_t axr = ARG32;
        int32_t *xr = (int32_t *)(ram + axr);
        valid |= (uint8_t *)xr >= data->block && (uint8_t *)xr < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Dxreaddir(ln, dirh, buf, xattr, xr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dxreaddir(%d, %d, 0x%08X, 0x%08X, 0x%08X): %d", ln, dirh, abuf, axattr, axr, res);
      }
      break;

    case 323: { // int32_t Pseteuid(int16_t euid)
        int16_t euid = ARG16;
        int32_t res = 0;
        res = Pseteuid(euid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pseteuid(%d): %d", euid, res);
      }
      break;

    case 324: { // int32_t Psetegid(int16_t egid)
        int16_t egid = ARG16;
        int32_t res = 0;
        res = Psetegid(egid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetegid(%d): %d", egid, res);
      }
      break;

    case 325: { // int16_t Pgetauid(void)
        int16_t res = 0;
        res = Pgetauid();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetauid(): %d", res);
      }
      break;

    case 326: { // int16_t Psetauid(int16_t id)
        int16_t id = ARG16;
        int16_t res = 0;
        res = Psetauid(id);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetauid(%d): %d", id, res);
      }
      break;

    case 327: { // int32_t Pgetgroups(int16_t len, int16_t *gidset)
        int valid = 0;
        int16_t len = ARG16;
        uint32_t agidset = ARG32;
        int16_t *gidset = (int16_t *)(ram + agidset);
        valid |= (uint8_t *)gidset >= data->block && (uint8_t *)gidset < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Pgetgroups(len, gidset);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetgroups(%d, 0x%08X): %d", len, agidset, res);
      }
      break;

    case 328: { // int32_t Psetgroups(int16_t len, int16_t *gidset)
        int valid = 0;
        int16_t len = ARG16;
        uint32_t agidset = ARG32;
        int16_t *gidset = (int16_t *)(ram + agidset);
        valid |= (uint8_t *)gidset >= data->block && (uint8_t *)gidset < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Psetgroups(len, gidset);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetgroups(%d, 0x%08X): %d", len, agidset, res);
      }
      break;

    case 329: { // int32_t Tsetitimer(int16_t which, int32_t *interval, int32_t *value, int32_t *ointerval, int32_t *ovalue)
        int valid = 0;
        int16_t which = ARG16;
        uint32_t ainterval = ARG32;
        int32_t *interval = (int32_t *)(ram + ainterval);
        valid |= (uint8_t *)interval >= data->block && (uint8_t *)interval < data->block + data->blockSize;
        uint32_t avalue = ARG32;
        int32_t *value = (int32_t *)(ram + avalue);
        valid |= (uint8_t *)value >= data->block && (uint8_t *)value < data->block + data->blockSize;
        uint32_t aointerval = ARG32;
        int32_t *ointerval = (int32_t *)(ram + aointerval);
        valid |= (uint8_t *)ointerval >= data->block && (uint8_t *)ointerval < data->block + data->blockSize;
        uint32_t aovalue = ARG32;
        int32_t *ovalue = (int32_t *)(ram + aovalue);
        valid |= (uint8_t *)ovalue >= data->block && (uint8_t *)ovalue < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Tsetitimer(which, interval, value, ointerval, ovalue);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsetitimer(%d, 0x%08X, 0x%08X, 0x%08X, 0x%08X): %d", which, ainterval, avalue, aointerval, aovalue, res);
      }
      break;

    case 330: { // int32_t Dchroot(char *path)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Dchroot(path);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dchroot(0x%08X): %d", apath, res);
      }
      break;

    case 331: { // int32_t Fstat64(int16_t flag, char *name, STAT *stat)
        int valid = 0;
        int16_t flag = ARG16;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t astat = ARG32;
        STAT *stat = (STAT *)(ram + astat);
        valid |= (uint8_t *)stat >= data->block && (uint8_t *)stat < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fstat64(flag, name, stat);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fstat64(%d, 0x%08X, 0x%08X): %d", flag, aname, astat, res);
      }
      break;

    case 332: { // int32_t Fseek64(int32_t hioffset, uint32_t lowoffset, int16_t handle, int16_t seekmode, int64_t *newpos)
        int valid = 0;
        int32_t hioffset = ARG32;
        uint32_t lowoffset = ARG32;
        int16_t handle = ARG16;
        int16_t seekmode = ARG16;
        uint32_t anewpos = ARG32;
        int64_t *newpos = (int64_t *)(ram + anewpos);
        valid |= (uint8_t *)newpos >= data->block && (uint8_t *)newpos < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fseek64(hioffset, lowoffset, handle, seekmode, newpos);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fseek64(%d, %d, %d, %d, 0x%08X): %d", hioffset, lowoffset, handle, seekmode, anewpos, res);
      }
      break;

    case 333: { // int32_t Dsetkey(int32_t hidev, int32_t lowdev, char *key, int16_t cipher)
        int valid = 0;
        int32_t hidev = ARG32;
        int32_t lowdev = ARG32;
        uint32_t akey = ARG32;
        char *key = (char *)(ram + akey);
        valid |= (uint8_t *)key >= data->block && (uint8_t *)key < data->block + data->blockSize;
        int16_t cipher = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Dsetkey(hidev, lowdev, key, cipher);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsetkey(%d, %d, 0x%08X, %d): %d", hidev, lowdev, akey, cipher, res);
      }
      break;

    case 334: { // int32_t Psetreuid(int16_t ruid, int16_t euid)
        int16_t ruid = ARG16;
        int16_t euid = ARG16;
        int32_t res = 0;
        res = Psetreuid(ruid, euid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetreuid(%d, %d): %d", ruid, euid, res);
      }
      break;

    case 335: { // int32_t Psetregid(int16_t rgid, int16_t egid)
        int16_t rgid = ARG16;
        int16_t egid = ARG16;
        int32_t res = 0;
        res = Psetregid(rgid, egid);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetregid(%d, %d): %d", rgid, egid, res);
      }
      break;

    case 336: { // void Sync(void)
        Sync();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Sync()");
      }
      break;

    case 337: { // int32_t Shutdown(int32_t mode)
        int32_t mode = ARG32;
        int32_t res = 0;
        res = Shutdown(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Shutdown(%d): %d", mode, res);
      }
      break;

    case 338: { // int32_t Dreadlabel(char *path, char *label, int16_t length)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        uint32_t alabel = ARG32;
        char *label = (char *)(ram + alabel);
        valid |= (uint8_t *)label >= data->block && (uint8_t *)label < data->block + data->blockSize;
        int16_t length = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Dreadlabel(path, label, length);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dreadlabel(0x%08X, 0x%08X, %d): %d", apath, alabel, length, res);
      }
      break;

    case 339: { // int32_t Dwritelabel(char *path, char *label)
        int valid = 0;
        uint32_t apath = ARG32;
        char *path = (char *)(ram + apath);
        valid |= (uint8_t *)path >= data->block && (uint8_t *)path < data->block + data->blockSize;
        uint32_t alabel = ARG32;
        char *label = (char *)(ram + alabel);
        valid |= (uint8_t *)label >= data->block && (uint8_t *)label < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Dwritelabel(path, label);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dwritelabel(0x%08X, 0x%08X): %d", apath, alabel, res);
      }
      break;

    case 340: { // int32_t Ssystem(int16_t mode, int32_t arg1, int32_t arg2)
        int16_t mode = ARG16;
        int32_t arg1 = ARG32;
        int32_t arg2 = ARG32;
        int32_t res = 0;
        res = Ssystem(mode, arg1, arg2);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ssystem(%d, %d, %d): %d", mode, arg1, arg2, res);
      }
      break;

    case 341: { // int32_t Tgettimeofday(struct timeval *tv, timezone *tzp)
        int valid = 0;
        uint32_t atv = ARG32;
        struct timeval *tv = (struct timeval *)(ram + atv);
        valid |= (uint8_t *)tv >= data->block && (uint8_t *)tv < data->block + data->blockSize;
        uint32_t atzp = ARG32;
        timezone *tzp = (timezone *)(ram + atzp);
        valid |= (uint8_t *)tzp >= data->block && (uint8_t *)tzp < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Tgettimeofday(tv, tzp);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tgettimeofday(0x%08X, 0x%08X): %d", atv, atzp, res);
      }
      break;

    case 342: { // int32_t Tsettimeofday(struct timeval *tv, timezone *tzp)
        int valid = 0;
        uint32_t atv = ARG32;
        struct timeval *tv = (struct timeval *)(ram + atv);
        valid |= (uint8_t *)tv >= data->block && (uint8_t *)tv < data->block + data->blockSize;
        uint32_t atzp = ARG32;
        timezone *tzp = (timezone *)(ram + atzp);
        valid |= (uint8_t *)tzp >= data->block && (uint8_t *)tzp < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Tsettimeofday(tv, tzp);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tsettimeofday(0x%08X, 0x%08X): %d", atv, atzp, res);
      }
      break;

    case 343: { // int Tadjtime(struct timeval *delta, struct timeval *olddelta)
        int valid = 0;
        uint32_t adelta = ARG32;
        struct timeval *delta = (struct timeval *)(ram + adelta);
        valid |= (uint8_t *)delta >= data->block && (uint8_t *)delta < data->block + data->blockSize;
        uint32_t aolddelta = ARG32;
        struct timeval *olddelta = (struct timeval *)(ram + aolddelta);
        valid |= (uint8_t *)olddelta >= data->block && (uint8_t *)olddelta < data->block + data->blockSize;
        int res = 0;
        if (valid) {
          res = Tadjtime(delta, olddelta);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Tadjtime(0x%08X, 0x%08X): %d", adelta, aolddelta, res);
      }
      break;

    case 344: { // int32_t Pgetpriority(int16_t which, int16_t who)
        int16_t which = ARG16;
        int16_t who = ARG16;
        int32_t res = 0;
        res = Pgetpriority(which, who);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Pgetpriority(%d, %d): %d", which, who, res);
      }
      break;

    case 345: { // int32_t Psetpriority(int16_t which, int16_t who, int16_t pri)
        int16_t which = ARG16;
        int16_t who = ARG16;
        int16_t pri = ARG16;
        int32_t res = 0;
        res = Psetpriority(which, who, pri);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psetpriority(%d, %d, %d): %d", which, who, pri, res);
      }
      break;

    case 346: { // int32_t Fpoll(POLLFD *fds, uint32_t nfds, uint32_t timeout)
        int valid = 0;
        uint32_t afds = ARG32;
        POLLFD *fds = (POLLFD *)(ram + afds);
        valid |= (uint8_t *)fds >= data->block && (uint8_t *)fds < data->block + data->blockSize;
        uint32_t nfds = ARG32;
        uint32_t timeout = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fpoll(fds, nfds, timeout);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fpoll(0x%08X, %d, %d): %d", afds, nfds, timeout, res);
      }
      break;

    case 347: { // int32_t Fwritev(int16_t handle, struct iovec *iov, int32_t niov)
        int valid = 0;
        int16_t handle = ARG16;
        uint32_t aiov = ARG32;
        struct iovec *iov = (struct iovec *)(ram + aiov);
        valid |= (uint8_t *)iov >= data->block && (uint8_t *)iov < data->block + data->blockSize;
        int32_t niov = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fwritev(handle, iov, niov);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fwritev(%d, 0x%08X, %d): %d", handle, aiov, niov, res);
      }
      break;

    case 348: { // int32_t Freadv(int16_t handle, struct iovec *iov, int32_t niov)
        int valid = 0;
        int16_t handle = ARG16;
        uint32_t aiov = ARG32;
        struct iovec *iov = (struct iovec *)(ram + aiov);
        valid |= (uint8_t *)iov >= data->block && (uint8_t *)iov < data->block + data->blockSize;
        int32_t niov = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Freadv(handle, iov, niov);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Freadv(%d, 0x%08X, %d): %d", handle, aiov, niov, res);
      }
      break;

    case 349: { // int32_t Ffstat64(int16_t fd, STAT *stat)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t astat = ARG32;
        STAT *stat = (STAT *)(ram + astat);
        valid |= (uint8_t *)stat >= data->block && (uint8_t *)stat < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Ffstat64(fd, stat);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffstat64(%d, 0x%08X): %d", fd, astat, res);
      }
      break;

    case 350: { // int32_t Psysctl(int32_t *name, uint32_t namelen, void *old, uint32_t *oldlenp, void *new, uint32_t newlen)
        int valid = 0;
        uint32_t aname = ARG32;
        int32_t *name = (int32_t *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t namelen = ARG32;
        uint32_t aold = ARG32;
        void *old = (void *)(ram + aold);
        valid |= (uint8_t *)old >= data->block && (uint8_t *)old < data->block + data->blockSize;
        uint32_t aoldlenp = ARG32;
        uint32_t *oldlenp = (uint32_t *)(ram + aoldlenp);
        valid |= (uint8_t *)oldlenp >= data->block && (uint8_t *)oldlenp < data->block + data->blockSize;
        uint32_t anew = ARG32;
        void *new = (void *)(ram + anew);
        valid |= (uint8_t *)new >= data->block && (uint8_t *)new < data->block + data->blockSize;
        uint32_t newlen = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Psysctl(name, namelen, old, oldlenp, new, newlen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Psysctl(0x%08X, %d, 0x%08X, 0x%08X, 0x%08X, %d): %d", aname, namelen, aold, aoldlenp, anew, newlen, res);
      }
      break;

    case 352: { // int32_t Fsocket(int32_t domain, int32_t type, int32_t protocol)
        int32_t domain = ARG32;
        int32_t type = ARG32;
        int32_t protocol = ARG32;
        int32_t res = 0;
        res = Fsocket(domain, type, protocol);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsocket(%d, %d, %d): %d", domain, type, protocol, res);
      }
      break;

    case 353: { // int32_t Fsocketpair(int32_t domain, int32_t type, int32_t protocol, int16_t *fds)
        int valid = 0;
        int32_t domain = ARG32;
        int32_t type = ARG32;
        int32_t protocol = ARG32;
        uint32_t afds = ARG32;
        int16_t *fds = (int16_t *)(ram + afds);
        valid |= (uint8_t *)fds >= data->block && (uint8_t *)fds < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fsocketpair(domain, type, protocol, fds);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsocketpair(%d, %d, %d, 0x%08X): %d", domain, type, protocol, afds, res);
      }
      break;

    case 354: { // int32_t Faccept(int16_t fd, struct sockaddr *name, uint32_t *anamelen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t aname = ARG32;
        struct sockaddr *name = (struct sockaddr *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t aanamelen = ARG32;
        uint32_t *anamelen = (uint32_t *)(ram + aanamelen);
        valid |= (uint8_t *)anamelen >= data->block && (uint8_t *)anamelen < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Faccept(fd, name, anamelen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Faccept(%d, 0x%08X, 0x%08X): %d", fd, aname, aanamelen, res);
      }
      break;

    case 355: { // int32_t Fconnect(int16_t fd, struct sockaddr *name, uint32_t anamelen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t aname = ARG32;
        struct sockaddr *name = (struct sockaddr *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t anamelen = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fconnect(fd, name, anamelen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fconnect(%d, 0x%08X, %d): %d", fd, aname, anamelen, res);
      }
      break;

    case 356: { // int32_t Fbind(int16_t fd, struct sockaddr *name, uint32_t anamelen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t aname = ARG32;
        struct sockaddr *name = (struct sockaddr *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        uint32_t anamelen = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fbind(fd, name, anamelen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fbind(%d, 0x%08X, %d): %d", fd, aname, anamelen, res);
      }
      break;

    case 357: { // int32_t Flisten(int16_t fd, int32_t backlog)
        int16_t fd = ARG16;
        int32_t backlog = ARG32;
        int32_t res = 0;
        res = Flisten(fd, backlog);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flisten(%d, %d): %d", fd, backlog, res);
      }
      break;

    case 358: { // int32_t Frecvmsg(int16_t fd, struct msghdr *msg, int32_t flags)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t amsg = ARG32;
        struct msghdr *msg = (struct msghdr *)(ram + amsg);
        valid |= (uint8_t *)msg >= data->block && (uint8_t *)msg < data->block + data->blockSize;
        int32_t flags = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Frecvmsg(fd, msg, flags);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frecvmsg(%d, 0x%08X, %d): %d", fd, amsg, flags, res);
      }
      break;

    case 359: { // int32_t Fsendmsg(int16_t fd, struct msghdr *msg, int32_t flags)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t amsg = ARG32;
        struct msghdr *msg = (struct msghdr *)(ram + amsg);
        valid |= (uint8_t *)msg >= data->block && (uint8_t *)msg < data->block + data->blockSize;
        int32_t flags = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fsendmsg(fd, msg, flags);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsendmsg(%d, 0x%08X, %d): %d", fd, amsg, flags, res);
      }
      break;

    case 360: { // int32_t Frecvfrom(int16_t fd, void *buf, int32_t buflen, int32_t flags, struct sockaddr *to, uint32_t *addrlen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t abuf = ARG32;
        void *buf = (void *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int32_t buflen = ARG32;
        int32_t flags = ARG32;
        uint32_t ato = ARG32;
        struct sockaddr *to = (struct sockaddr *)(ram + ato);
        valid |= (uint8_t *)to >= data->block && (uint8_t *)to < data->block + data->blockSize;
        uint32_t aaddrlen = ARG32;
        uint32_t *addrlen = (uint32_t *)(ram + aaddrlen);
        valid |= (uint8_t *)addrlen >= data->block && (uint8_t *)addrlen < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Frecvfrom(fd, buf, buflen, flags, to, addrlen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Frecvfrom(%d, 0x%08X, %d, %d, 0x%08X, 0x%08X): %d", fd, abuf, buflen, flags, ato, aaddrlen, res);
      }
      break;

    case 361: { // int32_t Fsendto(int16_t fd, void *buf, int32_t buflen, int32_t flags, struct sockaddr *to, uint32_t addrlen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t abuf = ARG32;
        void *buf = (void *)(ram + abuf);
        valid |= (uint8_t *)buf >= data->block && (uint8_t *)buf < data->block + data->blockSize;
        int32_t buflen = ARG32;
        int32_t flags = ARG32;
        uint32_t ato = ARG32;
        struct sockaddr *to = (struct sockaddr *)(ram + ato);
        valid |= (uint8_t *)to >= data->block && (uint8_t *)to < data->block + data->blockSize;
        uint32_t addrlen = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fsendto(fd, buf, buflen, flags, to, addrlen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsendto(%d, 0x%08X, %d, %d, 0x%08X, %d): %d", fd, abuf, buflen, flags, ato, addrlen, res);
      }
      break;

    case 362: { // int32_t Fsetsockopt(int16_t fd, int32_t level, int32_t name, void *val, uint32_t valsize)
        int valid = 0;
        int16_t fd = ARG16;
        int32_t level = ARG32;
        int32_t name = ARG32;
        uint32_t aval = ARG32;
        void *val = (void *)(ram + aval);
        valid |= (uint8_t *)val >= data->block && (uint8_t *)val < data->block + data->blockSize;
        uint32_t valsize = ARG32;
        int32_t res = 0;
        if (valid) {
          res = Fsetsockopt(fd, level, name, val, valsize);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fsetsockopt(%d, %d, %d, 0x%08X, %d): %d", fd, level, name, aval, valsize, res);
      }
      break;

    case 363: { // int32_t Fgetsockopt(int16_t fd, int32_t level, int32_t name, void *val, uint32_t *valsize)
        int valid = 0;
        int16_t fd = ARG16;
        int32_t level = ARG32;
        int32_t name = ARG32;
        uint32_t aval = ARG32;
        void *val = (void *)(ram + aval);
        valid |= (uint8_t *)val >= data->block && (uint8_t *)val < data->block + data->blockSize;
        uint32_t avalsize = ARG32;
        uint32_t *valsize = (uint32_t *)(ram + avalsize);
        valid |= (uint8_t *)valsize >= data->block && (uint8_t *)valsize < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fgetsockopt(fd, level, name, val, valsize);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetsockopt(%d, %d, %d, 0x%08X, 0x%08X): %d", fd, level, name, aval, avalsize, res);
      }
      break;

    case 364: { // int32_t Fgetpeername(int16_t fd, struct sockaddr *asa, uint32_t *alen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t aasa = ARG32;
        struct sockaddr *asa = (struct sockaddr *)(ram + aasa);
        valid |= (uint8_t *)asa >= data->block && (uint8_t *)asa < data->block + data->blockSize;
        uint32_t aalen = ARG32;
        uint32_t *alen = (uint32_t *)(ram + aalen);
        valid |= (uint8_t *)alen >= data->block && (uint8_t *)alen < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fgetpeername(fd, asa, alen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetpeername(%d, 0x%08X, 0x%08X): %d", fd, aasa, aalen, res);
      }
      break;

    case 365: { // int32_t Fgetsockname(int16_t fd, struct sockaddr *asa, uint32_t *alen)
        int valid = 0;
        int16_t fd = ARG16;
        uint32_t aasa = ARG32;
        struct sockaddr *asa = (struct sockaddr *)(ram + aasa);
        valid |= (uint8_t *)asa >= data->block && (uint8_t *)asa < data->block + data->blockSize;
        uint32_t aalen = ARG32;
        uint32_t *alen = (uint32_t *)(ram + aalen);
        valid |= (uint8_t *)alen >= data->block && (uint8_t *)alen < data->block + data->blockSize;
        int32_t res = 0;
        if (valid) {
          res = Fgetsockname(fd, asa, alen);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fgetsockname(%d, 0x%08X, 0x%08X): %d", fd, aasa, aalen, res);
      }
      break;

    case 366: { // int32_t Fshutdown(int16_t fd, int32_t how)
        int16_t fd = ARG16;
        int32_t how = ARG32;
        int32_t res = 0;
        res = Fshutdown(fd, how);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fshutdown(%d, %d): %d", fd, how, res);
      }
      break;

    case 381: { // int32_t Maccess(void *start, int32_t size, int16_t mode)
        int valid = 0;
        uint32_t astart = ARG32;
        void *start = (void *)(ram + astart);
        valid |= (uint8_t *)start >= data->block && (uint8_t *)start < data->block + data->blockSize;
        int32_t size = ARG32;
        int16_t mode = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Maccess(start, size, mode);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Maccess(0x%08X, %d, %d): %d", astart, size, mode, res);
      }
      break;

    case 384: { // int32_t Fchown16(char *name, int16_t uid, int16_t gid, int16_t flag)
        int valid = 0;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int16_t uid = ARG16;
        int16_t gid = ARG16;
        int16_t flag = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Fchown16(name, uid, gid, flag);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchown16(0x%08X, %d, %d, %d): %d", aname, uid, gid, flag, res);
      }
      break;

    case 385: { // int32_t Fchdir(int16_t handle)
        int16_t handle = ARG16;
        int32_t res = 0;
        res = Fchdir(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fchdir(%d): %d", handle, res);
      }
      break;

    case 386: { // int32_t Ffdopendir(int16_t fd)
        int16_t fd = ARG16;
        int32_t res = 0;
        res = Ffdopendir(fd);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ffdopendir(%d): %d", fd, res);
      }
      break;

    case 387: { // int16_t Fdirfd(int32_t handle)
        int32_t handle = ARG32;
        int16_t res = 0;
        res = Fdirfd(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Fdirfd(%d): %d", handle, res);
      }
      break;

    case 1296: { // int32_t Dxopendir(char *name, int16_t flag)
        int valid = 0;
        uint32_t aname = ARG32;
        char *name = (char *)(ram + aname);
        valid |= (uint8_t *)name >= data->block && (uint8_t *)name < data->block + data->blockSize;
        int16_t flag = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Dxopendir(name, flag);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dxopendir(0x%08X, %d): %d", aname, flag, res);
      }
      break;

