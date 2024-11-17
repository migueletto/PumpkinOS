    case 0: { // void Initmouse(int16_t type, MOUSE *par, void *mousevec)
        int valid = 0;
        int16_t type = ARG16;
        uint32_t apar = ARG32;
        MOUSE *par = (MOUSE *)(memory + apar);
        valid |= (uint8_t *)par >= data->memory && (uint8_t *)par < data->memory + data->memorySize;
        uint32_t amousevec = ARG32;
        void *mousevec = (void *)(memory + amousevec);
        valid |= (uint8_t *)mousevec >= data->memory && (uint8_t *)mousevec < data->memory + data->memorySize;
        if (valid) {
          Initmouse(type, par, mousevec);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Initmouse(%d, 0x%08X, 0x%08X)", type, apar, amousevec);
      }
      break;

    case 1: { // void *Ssbrk(int16_t count)
        int16_t count = ARG16;
        void *res = NULL;
        res = Ssbrk(count);
        uint32_t ares = res ? ((uint8_t *)res - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ssbrk(%d): 0x%08X", count, ares);
      }
      break;

    case 2: { // void *Physbase(void)
        uint32_t res = data->physbase;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Physbase(): 0x%08X", res);
      }
      break;

    case 3: { // void *Logbase(void)
        uint32_t res = data->logbase;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Logbase(): 0x%08X", res);
      }
      break;

    case 4: { // int16_t Getrez(void)
        // returns the current resolution of the screen:
        // 0  320x200 (4 planes)
        // 1  640x200 (2 planes)
        // 2  640x400 (one plane)

        int16_t res = data->screen_res;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Getrez(): %d", res);
      }
      break;

    case 5: { // void Setscreen(void *laddr, void *paddr, int16_t rezz)
        // alter the screen resolution and screen memory addresses
        // laddr: address of the logical screen memory
        // paddr: address of the physical screen memory
        // rezz: same as in Getrez
        // a value of -1 means that the corresponding address or resolution will not be altered

        uint32_t laddr = ARG32;
        uint32_t paddr = ARG32;
        int16_t rezz = ARG16;

        if (laddr != 0xFFFFFFFF) {
          data->logbase = laddr;
        }

        if (paddr != 0xFFFFFFFF) {
          data->physbase = paddr;
        }

        if (rezz != -1) {
          switch (rezz) {
            case 0:
            case 1:
              data->screen_res = rezz;
              data->font = tos8x8Font;
              FntSetFont(data->font);
              break;
            case 2:
              data->screen_res = rezz;
              data->font = tos8x16Font;
              FntSetFont(data->font);
              break;
            default:
              debug(DEBUG_ERROR, "TOS", "GEMDOS Setscreen invalid resolution %d", rezz);
              break;
          }
        }

        debug(DEBUG_TRACE, "TOS", "GEMDOS Setscreen(0x%08X, 0x%08X, %d)", laddr, paddr, rezz);
      }
      break;

    case 6: { // void Setpalette(void *pallptr)
        // loads the ST color lookup table with a new palette
        uint32_t pallptr = ARG32;
        if (pallptr) {
          uint16_t color, rgb;
          uint32_t colornum;
          for (colornum = 0; colornum < 16; colornum++) {
            color = m68k_read_memory_16(pallptr);
            rgb = tos_convert_color(color);
            data->tos_color[colornum] = color;
            data->pumpkin_color[colornum] = rgb;
            debug(DEBUG_TRACE, "TOS", "GEMDOS Setpalette %u color 0x%04X rgb565 0x%04X", colornum, color, rgb);
            pallptr += 2;
          }
          data->screen_updated = 1;
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setpalette(0x%08X)", pallptr);
      }
      break;

    case 7: { // int16_t Setcolor(int16_t colornum, int16_t color)
        // returns the value of a colour register or sets this to a new value
        // colornum: number of the colour register (0..15)
        // color: new colour value (-1 = don't alter)
        // returns the old value of the colour register
        // each of the sixteen color registers is bitmapped into a WORD as follows:
        // xxxx xRRR xGGG xBBB
        // xxxx x321 x321 x321

        uint16_t colornum = ARG16;
        uint16_t color = ARG16;
        uint16_t rgb, old = 0;

        if (colornum < 16) {
          old = data->tos_color[colornum];
          if (color != 0xffff) {
            rgb = tos_convert_color(color);
            data->tos_color[colornum] = color;
            data->pumpkin_color[colornum] = rgb;
            data->screen_updated = 1;
            debug(DEBUG_TRACE, "TOS", "GEMDOS Setcolor %u color 0x%04X rgb565 0x%04X", colornum, color, rgb);
          }
        }
        m68k_set_reg(M68K_REG_D0, old);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setcolor(%u, %u): %u", colornum, color, old);
      }
      break;

    case 8: { // int16_t Floprd(void *buf, int32_t filler, int16_t devno, int16_t sectno, int16_t trackno, int16_t sideno, int16_t count)
        int valid = 0;
        uint32_t abuf = ARG32;
        void *buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        int32_t filler = ARG32;
        int16_t devno = ARG16;
        int16_t sectno = ARG16;
        int16_t trackno = ARG16;
        int16_t sideno = ARG16;
        int16_t count = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Floprd(buf, filler, devno, sectno, trackno, sideno, count);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Floprd(0x%08X, %d, %d, %d, %d, %d, %d): %d", abuf, filler, devno, sectno, trackno, sideno, count, res);
      }
      break;

    case 9: { // int16_t Flopwr(void *buf, int32_t filler, int16_t devno, int16_t sectno, int16_t trackno, int16_t sideno, int16_t count)
        int valid = 0;
        uint32_t abuf = ARG32;
        void *buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        int32_t filler = ARG32;
        int16_t devno = ARG16;
        int16_t sectno = ARG16;
        int16_t trackno = ARG16;
        int16_t sideno = ARG16;
        int16_t count = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Flopwr(buf, filler, devno, sectno, trackno, sideno, count);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flopwr(0x%08X, %d, %d, %d, %d, %d, %d): %d", abuf, filler, devno, sectno, trackno, sideno, count, res);
      }
      break;

    case 10: { // int16_t Flopfmt(void)
        int16_t res = 0;
        res = Flopfmt();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flopfmt(): %d", res);
      }
      break;

    case 11: { // void Dbmsg(int16_t rsrvd, int16_t msg_num, int32_t msg_arg)
        int16_t rsrvd = ARG16;
        int16_t msg_num = ARG16;
        int32_t msg_arg = ARG32;
        Dbmsg(rsrvd, msg_num, msg_arg);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dbmsg(%d, %d, %d)", rsrvd, msg_num, msg_arg);
      }
      break;

    case 12: { // void Midiws(int16_t cnt, void *ptr)
        int valid = 0;
        int16_t cnt = ARG16;
        uint32_t aptr = ARG32;
        void *ptr = (void *)(memory + aptr);
        valid |= (uint8_t *)ptr >= data->memory && (uint8_t *)ptr < data->memory + data->memorySize;
        if (valid) {
          Midiws(cnt, ptr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Midiws(%d, 0x%08X)", cnt, aptr);
      }
      break;

    case 13: { // void Mfpint(int16_t number, void *vector)
        int valid = 0;
        int16_t number = ARG16;
        uint32_t avector = ARG32;
        void *vector = (void *)(memory + avector);
        valid |= (uint8_t *)vector >= data->memory && (uint8_t *)vector < data->memory + data->memorySize;
        if (valid) {
          Mfpint(number, vector);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Mfpint(%d, 0x%08X)", number, avector);
      }
      break;

    case 14: { // IOREC *Iorec(int16_t dev)
        int16_t dev = ARG16;
        IOREC *res = NULL;
        res = Iorec(dev);
        uint32_t ares = res ? ((uint8_t *)res - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Iorec(%d): 0x%08X", dev, ares);
      }
      break;

    case 15: { // int32_t Rsconf(int16_t baud, int16_t ctr, int16_t ucr, int16_t rsr, int16_t tsr, int16_t scr)
        int16_t baud = ARG16;
        int16_t ctr = ARG16;
        int16_t ucr = ARG16;
        int16_t rsr = ARG16;
        int16_t tsr = ARG16;
        int16_t scr = ARG16;
        int32_t res = 0;
        res = Rsconf(baud, ctr, ucr, rsr, tsr, scr);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Rsconf(%d, %d, %d, %d, %d, %d): %d", baud, ctr, ucr, rsr, tsr, scr, res);
      }
      break;

    case 16: { // KEYTAB *Keytbl(void *unshift, void *shift, void *capslock)
        int valid = 0;
        uint32_t aunshift = ARG32;
        void *unshift = (void *)(memory + aunshift);
        valid |= (uint8_t *)unshift >= data->memory && (uint8_t *)unshift < data->memory + data->memorySize;
        uint32_t ashift = ARG32;
        void *shift = (void *)(memory + ashift);
        valid |= (uint8_t *)shift >= data->memory && (uint8_t *)shift < data->memory + data->memorySize;
        uint32_t acapslock = ARG32;
        void *capslock = (void *)(memory + acapslock);
        valid |= (uint8_t *)capslock >= data->memory && (uint8_t *)capslock < data->memory + data->memorySize;
        KEYTAB *res = NULL;
        if (valid) {
          res = Keytbl(unshift, shift, capslock);
        }
        uint32_t ares = res ? ((uint8_t *)res - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Keytbl(0x%08X, 0x%08X, 0x%08X): 0x%08X", aunshift, ashift, acapslock, ares);
      }
      break;

    case 17: { // int32_t Random(void)
        int32_t res = 0;
        res = Random();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Random(): %d", res);
      }
      break;

    case 18: { // void Protobt(void *buf, int32_t serialno, int16_t disktype, int16_t execflag)
        int valid = 0;
        uint32_t abuf = ARG32;
        void *buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        int32_t serialno = ARG32;
        int16_t disktype = ARG16;
        int16_t execflag = ARG16;
        if (valid) {
          Protobt(buf, serialno, disktype, execflag);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Protobt(0x%08X, %d, %d, %d)", abuf, serialno, disktype, execflag);
      }
      break;

    case 19: { // int16_t Flopver(void *buf, int32_t filler, int16_t devno, int16_t sectno, int16_t trackno, int16_t sideno, int16_t count)
        int valid = 0;
        uint32_t abuf = ARG32;
        void *buf = (void *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        int32_t filler = ARG32;
        int16_t devno = ARG16;
        int16_t sectno = ARG16;
        int16_t trackno = ARG16;
        int16_t sideno = ARG16;
        int16_t count = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Flopver(buf, filler, devno, sectno, trackno, sideno, count);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Flopver(0x%08X, %d, %d, %d, %d, %d, %d): %d", abuf, filler, devno, sectno, trackno, sideno, count, res);
      }
      break;

    case 20: { // void Scrdmp(void)
        Scrdmp();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Scrdmp()");
      }
      break;

    case 21: { // int16_t Cursconf(int16_t func, int16_t rate)
        int16_t func = ARG16;
        int16_t rate = ARG16;
        int16_t res = 0;
        res = Cursconf(func, rate);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Cursconf(%d, %d): %d", func, rate, res);
      }
      break;

    case 22: { // void Settime(uint32_t time)
        uint32_t time = ARG32;
        Settime(time);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Settime(%d)", time);
      }
      break;

    case 23: { // uint32_t Gettime(void)
        uint32_t res = 0;
        res = Gettime();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Gettime(): %d", res);
      }
      break;

    case 24: { // void Bioskeys(void)
        Bioskeys();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bioskeys()");
      }
      break;

    case 25: { // void Ikbdws(int16_t count, int8_t *ptr)
        int valid = 0;
        int16_t count = ARG16;
        uint32_t aptr = ARG32;
        int8_t *ptr = (int8_t *)(memory + aptr);
        valid |= (uint8_t *)ptr >= data->memory && (uint8_t *)ptr < data->memory + data->memorySize;
        if (valid) {
          Ikbdws(count, ptr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ikbdws(%d, 0x%08X)", count, aptr);
      }
      break;

    case 26: { // void Jdisint(int16_t number)
        int16_t number = ARG16;
        Jdisint(number);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Jdisint(%d)", number);
      }
      break;

    case 27: { // void Jenabint(int16_t number)
        int16_t number = ARG16;
        Jenabint(number);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Jenabint(%d)", number);
      }
      break;

    case 28: { // int8_t Giaccess(int16_t dat, int16_t regno)
        int16_t dat = ARG16;
        int16_t regno = ARG16;
        int8_t res = 0;
        res = Giaccess(dat, regno);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Giaccess(%d, %d): %d", dat, regno, res);
      }
      break;

    case 29: { // void Offgibit(int16_t bitno)
        int16_t bitno = ARG16;
        Offgibit(bitno);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Offgibit(%d)", bitno);
      }
      break;

    case 30: { // void Ongibit(void)
        Ongibit();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Ongibit()");
      }
      break;

    case 31: { // void Xbtimer(int16_t timer, int16_t control, int16_t dat, void *vector)
        int valid = 0;
        int16_t timer = ARG16;
        int16_t control = ARG16;
        int16_t dat = ARG16;
        uint32_t avector = ARG32;
        void *vector = (void *)(memory + avector);
        valid |= (uint8_t *)vector >= data->memory && (uint8_t *)vector < data->memory + data->memorySize;
        if (valid) {
          Xbtimer(timer, control, dat, vector);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Xbtimer(%d, %d, %d, 0x%08X)", timer, control, dat, avector);
      }
      break;

    case 32: { // void *Dosound(int8_t *buf)
        int valid = 0;
        uint32_t abuf = ARG32;
        int8_t *buf = (int8_t *)(memory + abuf);
        valid |= (uint8_t *)buf >= data->memory && (uint8_t *)buf < data->memory + data->memorySize;
        void *res = NULL;
        if (valid) {
          res = Dosound(buf);
        }
        uint32_t ares = res ? ((uint8_t *)res - memory) : 0;
        m68k_set_reg(M68K_REG_D0, ares);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dosound(0x%08X): 0x%08X", abuf, ares);
      }
      break;

    case 33: { // int16_t Setprt(int16_t config)
        int16_t config = ARG16;
        int16_t res = 0;
        res = Setprt(config);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Setprt(%d): %d", config, res);
      }
      break;

    case 34: { // KBDVBASE *Kbdvbase(void)
        // returns a pointer to a KBDVBASE structure contaning a jump table to system vector handlers

        // typedef struct {
        //    void   (*kb_midivec)();   /* MIDI interrupt vector    */
        //    void   (*kb_vkbderr)();   /* Keyboard error vector    */
        //    void   (*kb_vmiderr)();   /* MIDI error vector        */
        //    void   (*kb_statvec)();   /* Keyboard status          */
        //    void   (*kb_mousevec)();  /* Keyboard mouse status    */
        //    void   (*kb_clockvec)();  /* Keyboard clock           */
        //    void   (*kb_joyvec)();    /* Keyboard joystick status */
        //    void   (*kb_midisys)();   /* System Midi vector       */
        //    void   (*kb_kbdsys)();    /* Keyboard vector          */
        //    int8_t drvstat;           /* Keyboard driver status   */
        // } KBDVBASE;
        // The element drvstat contains a non-zero value when the IKBD is in the process of sending a packet

        uint32_t res = data->kbdvbase;
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Kbdvbase(): 0x%08X", res);
      }
      break;

    case 35: { // int16_t Kbrate(int16_t initial, int16_t repeat)
        int16_t initial = ARG16;
        int16_t repeat = ARG16;
        int16_t res = 0;
        res = Kbrate(initial, repeat);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Kbrate(%d, %d): %d", initial, repeat, res);
      }
      break;

    case 36: { // int16_t Prtblk(PBDEF *par)
        int valid = 0;
        uint32_t apar = ARG32;
        PBDEF *par = (PBDEF *)(memory + apar);
        valid |= (uint8_t *)par >= data->memory && (uint8_t *)par < data->memory + data->memorySize;
        int16_t res = 0;
        if (valid) {
          res = Prtblk(par);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Prtblk(0x%08X): %d", apar, res);
      }
      break;

    case 37: { // void Vsync(void)
        Vsync();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Vsync()");
      }
      break;

    case 38: { // int32_t Supexec(void *func)
        uint32_t func = ARG32;
        debug(DEBUG_TRACE, "TOS", "GEMDOS Supexec(0x%08X)", func);
        r = func;
      }
      break;

    case 39: { // void Puntaes(void)
        Puntaes();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Puntaes()");
      }
      break;

    case 41: { // int16_t Floprate(int16_t devno, int16_t newrate)
        int16_t devno = ARG16;
        int16_t newrate = ARG16;
        int16_t res = 0;
        res = Floprate(devno, newrate);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Floprate(%d, %d): %d", devno, newrate, res);
      }
      break;

    case 42: { // int16_t DMAread(int32_t sector, int16_t count, void *buffer, int16_t devno)
        int valid = 0;
        int32_t sector = ARG32;
        int16_t count = ARG16;
        uint32_t abuffer = ARG32;
        void *buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int16_t devno = ARG16;
        int16_t res = 0;
        if (valid) {
          res = DMAread(sector, count, buffer, devno);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS DMAread(%d, %d, 0x%08X, %d): %d", sector, count, abuffer, devno, res);
      }
      break;

    case 43: { // int16_t DMAwrite(int32_t sector, int16_t count, void *buffer, int16_t devno)
        int valid = 0;
        int32_t sector = ARG32;
        int16_t count = ARG16;
        uint32_t abuffer = ARG32;
        void *buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int16_t devno = ARG16;
        int16_t res = 0;
        if (valid) {
          res = DMAwrite(sector, count, buffer, devno);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS DMAwrite(%d, %d, 0x%08X, %d): %d", sector, count, abuffer, devno, res);
      }
      break;

    case 44: { // int32_t Bconmap(int16_t devno)
        int16_t devno = ARG16;
        int32_t res = 0;
        res = Bconmap(devno);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Bconmap(%d): %d", devno, res);
      }
      break;

    case 46: { // int16_t NVMaccess(int16_t op, int16_t start, int16_t count, int8_t *buffer)
        int valid = 0;
        int16_t op = ARG16;
        int16_t start = ARG16;
        int16_t count = ARG16;
        uint32_t abuffer = ARG32;
        int8_t *buffer = (int8_t *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int16_t res = 0;
        if (valid) {
          res = NVMaccess(op, start, count, buffer);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS NVMaccess(%d, %d, %d, 0x%08X): %d", op, start, count, abuffer, res);
      }
      break;

    case 48: { // void Metainit(META_INFO_1 *buffer)
        int valid = 0;
        uint32_t abuffer = ARG32;
        META_INFO_1 *buffer = (META_INFO_1 *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        if (valid) {
          Metainit(buffer);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metainit(0x%08X)", abuffer);
      }
      break;

    case 49: { // int32_t Metaopen(int16_t drive, META_DRVINFO *buffer)
        int valid = 0;
        int16_t drive = ARG16;
        uint32_t abuffer = ARG32;
        META_DRVINFO *buffer = (META_DRVINFO *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Metaopen(drive, buffer);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaopen(%d, 0x%08X): %d", drive, abuffer, res);
      }
      break;

    case 50: { // int32_t Metaclose(int16_t drive)
        int16_t drive = ARG16;
        int32_t res = 0;
        res = Metaclose(drive);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaclose(%d): %d", drive, res);
      }
      break;

    case 51: { // int32_t Metaread(int16_t drive, void *buffer, int32_t blockno, int16_t count)
        int valid = 0;
        int16_t drive = ARG16;
        uint32_t abuffer = ARG32;
        void *buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int32_t blockno = ARG32;
        int16_t count = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Metaread(drive, buffer, blockno, count);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaread(%d, 0x%08X, %d, %d): %d", drive, abuffer, blockno, count, res);
      }
      break;

    case 52: { // int32_t Metawrite(int16_t drive, void *buffer, int32_t blockno, int16_t count)
        int valid = 0;
        int16_t drive = ARG16;
        uint32_t abuffer = ARG32;
        void *buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int32_t blockno = ARG32;
        int16_t count = ARG16;
        int32_t res = 0;
        if (valid) {
          res = Metawrite(drive, buffer, blockno, count);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metawrite(%d, 0x%08X, %d, %d): %d", drive, abuffer, blockno, count, res);
      }
      break;

    case 53: { // int32_t Metaseek(int16_t drive, int32_t blockno)
        int16_t drive = ARG16;
        int32_t blockno = ARG32;
        int32_t res = 0;
        res = Metaseek(drive, blockno);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaseek(%d, %d): %d", drive, blockno, res);
      }
      break;

    case 54: { // int32_t Metastatus(int16_t drive, void *buffer)
        int valid = 0;
        int16_t drive = ARG16;
        uint32_t abuffer = ARG32;
        void *buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Metastatus(drive, buffer);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metastatus(%d, 0x%08X): %d", drive, abuffer, res);
      }
      break;

    case 55: { // int32_t Metaioctl(int16_t drive, int32_t magic, int16_t opcode, void *buffer)
        int valid = 0;
        int16_t drive = ARG16;
        int32_t magic = ARG32;
        int16_t opcode = ARG16;
        uint32_t abuffer = ARG32;
        void *buffer = (void *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Metaioctl(drive, magic, opcode, buffer);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metaioctl(%d, %d, %d, 0x%08X): %d", drive, magic, opcode, abuffer, res);
      }
      break;

    case 59: { // int32_t Metastartaudio(int16_t drive, int16_t flag, uint8_t *bytearray)
        int valid = 0;
        int16_t drive = ARG16;
        int16_t flag = ARG16;
        uint32_t abytearray = ARG32;
        uint8_t *bytearray = (uint8_t *)(memory + abytearray);
        valid |= (uint8_t *)bytearray >= data->memory && (uint8_t *)bytearray < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Metastartaudio(drive, flag, bytearray);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metastartaudio(%d, %d, 0x%08X): %d", drive, flag, abytearray, res);
      }
      break;

    case 60: { // int32_t Metastopaudio(int16_t drive)
        int16_t drive = ARG16;
        int32_t res = 0;
        res = Metastopaudio(drive);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metastopaudio(%d): %d", drive, res);
      }
      break;

    case 61: { // int32_t Metasetsongtime(int16_t drive, int16_t repeat, int32_t starttime, int32_t endtime)
        int16_t drive = ARG16;
        int16_t repeat = ARG16;
        int32_t starttime = ARG32;
        int32_t endtime = ARG32;
        int32_t res = 0;
        res = Metasetsongtime(drive, repeat, starttime, endtime);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metasetsongtime(%d, %d, %d, %d): %d", drive, repeat, starttime, endtime, res);
      }
      break;

    case 62: { // int32_t Metagettoc(int16_t drive, int16_t flag, CD_TOC_ENTRY *buffer)
        int valid = 0;
        int16_t drive = ARG16;
        int16_t flag = ARG16;
        uint32_t abuffer = ARG32;
        CD_TOC_ENTRY *buffer = (CD_TOC_ENTRY *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Metagettoc(drive, flag, buffer);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metagettoc(%d, %d, 0x%08X): %d", drive, flag, abuffer, res);
      }
      break;

    case 63: { // int32_t Metadiscinfo(int16_t drive, CD_DISC_INFO *p)
        int valid = 0;
        int16_t drive = ARG16;
        uint32_t ap = ARG32;
        CD_DISC_INFO *p = (CD_DISC_INFO *)(memory + ap);
        valid |= (uint8_t *)p >= data->memory && (uint8_t *)p < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Metadiscinfo(drive, p);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Metadiscinfo(%d, 0x%08X): %d", drive, ap, res);
      }
      break;

    case 64: { // int16_t Blitmode(int16_t mode)
        int16_t mode = ARG16;
        int16_t res = 0;
        res = Blitmode(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Blitmode(%d): %d", mode, res);
      }
      break;

    case 80: { // int16_t EsetShift(int16_t shftMode)
        int16_t shftMode = ARG16;
        int16_t res = 0;
        res = EsetShift(shftMode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetShift(%d): %d", shftMode, res);
      }
      break;

    case 81: { // int16_t EgetShift(void)
        int16_t res = 0;
        res = EgetShift();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EgetShift(): %d", res);
      }
      break;

    case 82: { // int16_t EsetBank(int16_t bankNum)
        int16_t bankNum = ARG16;
        int16_t res = 0;
        res = EsetBank(bankNum);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetBank(%d): %d", bankNum, res);
      }
      break;

    case 83: { // int16_t EsetColor(int16_t colorNum, int16_t color)
        int16_t colorNum = ARG16;
        int16_t color = ARG16;
        int16_t res = 0;
        res = EsetColor(colorNum, color);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetColor(%d, %d): %d", colorNum, color, res);
      }
      break;

    case 84: { // void EsetPalette(int16_t colorNum, int16_t count, int16_t *palettePtr)
        int valid = 0;
        int16_t colorNum = ARG16;
        int16_t count = ARG16;
        uint32_t apalettePtr = ARG32;
        int16_t *palettePtr = (int16_t *)(memory + apalettePtr);
        valid |= (uint8_t *)palettePtr >= data->memory && (uint8_t *)palettePtr < data->memory + data->memorySize;
        if (valid) {
          EsetPalette(colorNum, count, palettePtr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetPalette(%d, %d, 0x%08X)", colorNum, count, apalettePtr);
      }
      break;

    case 85: { // void EgetPalette(int16_t colorNum, int16_t count, int16_t *palettePtr)
        int valid = 0;
        int16_t colorNum = ARG16;
        int16_t count = ARG16;
        uint32_t apalettePtr = ARG32;
        int16_t *palettePtr = (int16_t *)(memory + apalettePtr);
        valid |= (uint8_t *)palettePtr >= data->memory && (uint8_t *)palettePtr < data->memory + data->memorySize;
        if (valid) {
          EgetPalette(colorNum, count, palettePtr);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS EgetPalette(%d, %d, 0x%08X)", colorNum, count, apalettePtr);
      }
      break;

    case 86: { // int16_t EsetGray(int16_t sw)
        int16_t sw = ARG16;
        int16_t res = 0;
        res = EsetGray(sw);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetGray(%d): %d", sw, res);
      }
      break;

    case 87: { // int16_t EsetSmear(int16_t sw)
        int16_t sw = ARG16;
        int16_t res = 0;
        res = EsetSmear(sw);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS EsetSmear(%d): %d", sw, res);
      }
      break;

    case 88: { // int16_t VsetMode(int16_t mode)
        int16_t mode = ARG16;
        int16_t res = 0;
        res = VsetMode(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetMode(%d): %d", mode, res);
      }
      break;

    case 89: { // int16_t mon_type(void)
        int16_t res = 0;
        res = mon_type();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS mon_type(): %d", res);
      }
      break;

    case 90: { // void VsetSync(int16_t flag)
        int16_t flag = ARG16;
        VsetSync(flag);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetSync(%d)", flag);
      }
      break;

    case 91: { // int32_t VgetSize(int16_t mode)
        int16_t mode = ARG16;
        int32_t res = 0;
        res = VgetSize(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VgetSize(%d): %d", mode, res);
      }
      break;

    case 93: { // void VsetRGB(int16_t index, int16_t count, int32_t *array)
        int valid = 0;
        int16_t index = ARG16;
        int16_t count = ARG16;
        uint32_t aarray = ARG32;
        int32_t *array = (int32_t *)(memory + aarray);
        valid |= (uint8_t *)array >= data->memory && (uint8_t *)array < data->memory + data->memorySize;
        if (valid) {
          VsetRGB(index, count, array);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetRGB(%d, %d, 0x%08X)", index, count, aarray);
      }
      break;

    case 94: { // void VgetRGB(int16_t index, int16_t count, int32_t *array)
        int valid = 0;
        int16_t index = ARG16;
        int16_t count = ARG16;
        uint32_t aarray = ARG32;
        int32_t *array = (int32_t *)(memory + aarray);
        valid |= (uint8_t *)array >= data->memory && (uint8_t *)array < data->memory + data->memorySize;
        if (valid) {
          VgetRGB(index, count, array);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS VgetRGB(%d, %d, 0x%08X)", index, count, aarray);
      }
      break;

    case 95: { // int16_t ValidMode(int16_t mode)
        int16_t mode = ARG16;
        int16_t res = 0;
        res = ValidMode(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS ValidMode(%d): %d", mode, res);
      }
      break;

    case 96: { // void Dsp_DoBlock(int8_t *data_in, int32_t size_in, int8_t *data_out, int32_t size_out)
        int valid = 0;
        uint32_t adata_in = ARG32;
        int8_t *data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        int32_t size_in = ARG32;
        uint32_t adata_out = ARG32;
        int8_t *data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t size_out = ARG32;
        if (valid) {
          Dsp_DoBlock(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_DoBlock(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 97: { // void Dsp_BlkHandShake(int8_t *data_in, int32_t size_in, int8_t *data_out, int32_t size_out)
        int valid = 0;
        uint32_t adata_in = ARG32;
        int8_t *data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        int32_t size_in = ARG32;
        uint32_t adata_out = ARG32;
        int8_t *data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t size_out = ARG32;
        if (valid) {
          Dsp_BlkHandShake(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkHandShake(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 98: { // void Dsp_BlkUnpacked(int32_t *data_in, int32_t size_in, int32_t *data_out, int32_t size_out)
        int valid = 0;
        uint32_t adata_in = ARG32;
        int32_t *data_in = (int32_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        int32_t size_in = ARG32;
        uint32_t adata_out = ARG32;
        int32_t *data_out = (int32_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t size_out = ARG32;
        if (valid) {
          Dsp_BlkUnpacked(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkUnpacked(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 99: { // void Dsp_InStream(int8_t *data_in, int32_t block_size, int32_t num_blocks, int32_t *blocks_done)
        int valid = 0;
        uint32_t adata_in = ARG32;
        int8_t *data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        int32_t block_size = ARG32;
        int32_t num_blocks = ARG32;
        uint32_t ablocks_done = ARG32;
        int32_t *blocks_done = (int32_t *)(memory + ablocks_done);
        valid |= (uint8_t *)blocks_done >= data->memory && (uint8_t *)blocks_done < data->memory + data->memorySize;
        if (valid) {
          Dsp_InStream(data_in, block_size, num_blocks, blocks_done);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_InStream(0x%08X, %d, %d, 0x%08X)", adata_in, block_size, num_blocks, ablocks_done);
      }
      break;

    case 100: { // void Dsp_OutStream(int8_t *data_out, int32_t block_size, int32_t num_blocks, int32_t *blocks_done)
        int valid = 0;
        uint32_t adata_out = ARG32;
        int8_t *data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t block_size = ARG32;
        int32_t num_blocks = ARG32;
        uint32_t ablocks_done = ARG32;
        int32_t *blocks_done = (int32_t *)(memory + ablocks_done);
        valid |= (uint8_t *)blocks_done >= data->memory && (uint8_t *)blocks_done < data->memory + data->memorySize;
        if (valid) {
          Dsp_OutStream(data_out, block_size, num_blocks, blocks_done);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_OutStream(0x%08X, %d, %d, 0x%08X)", adata_out, block_size, num_blocks, ablocks_done);
      }
      break;

    case 101: { // void Dsp_IOStream(int8_t *data_in, int8_t *data_out, int32_t block_insize, int32_t block_outsize, int32_t num_blocks, int32_t *blocks_done)
        int valid = 0;
        uint32_t adata_in = ARG32;
        int8_t *data_in = (int8_t *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        uint32_t adata_out = ARG32;
        int8_t *data_out = (int8_t *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t block_insize = ARG32;
        int32_t block_outsize = ARG32;
        int32_t num_blocks = ARG32;
        uint32_t ablocks_done = ARG32;
        int32_t *blocks_done = (int32_t *)(memory + ablocks_done);
        valid |= (uint8_t *)blocks_done >= data->memory && (uint8_t *)blocks_done < data->memory + data->memorySize;
        if (valid) {
          Dsp_IOStream(data_in, data_out, block_insize, block_outsize, num_blocks, blocks_done);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_IOStream(0x%08X, 0x%08X, %d, %d, %d, 0x%08X)", adata_in, adata_out, block_insize, block_outsize, num_blocks, ablocks_done);
      }
      break;

    case 102: { // void Dsp_RemoveInterrupts(int16_t mask)
        int16_t mask = ARG16;
        Dsp_RemoveInterrupts(mask);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_RemoveInterrupts(%d)", mask);
      }
      break;

    case 103: { // int16_t Dsp_GetWordSize(void)
        int16_t res = 0;
        res = Dsp_GetWordSize();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_GetWordSize(): %d", res);
      }
      break;

    case 104: { // int16_t Dsp_Lock(void)
        int16_t res = 0;
        res = Dsp_Lock();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Lock(): %d", res);
      }
      break;

    case 105: { // void Dsp_Unlock(void)
        Dsp_Unlock();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Unlock()");
      }
      break;

    case 106: { // void Dsp_Available(int32_t *xavailable, int32_t *yavailable)
        int valid = 0;
        uint32_t axavailable = ARG32;
        int32_t *xavailable = (int32_t *)(memory + axavailable);
        valid |= (uint8_t *)xavailable >= data->memory && (uint8_t *)xavailable < data->memory + data->memorySize;
        uint32_t ayavailable = ARG32;
        int32_t *yavailable = (int32_t *)(memory + ayavailable);
        valid |= (uint8_t *)yavailable >= data->memory && (uint8_t *)yavailable < data->memory + data->memorySize;
        if (valid) {
          Dsp_Available(xavailable, yavailable);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Available(0x%08X, 0x%08X)", axavailable, ayavailable);
      }
      break;

    case 107: { // int16_t Dsp_Reserve(int32_t xreserve, int32_t yreserve)
        int32_t xreserve = ARG32;
        int32_t yreserve = ARG32;
        int16_t res = 0;
        res = Dsp_Reserve(xreserve, yreserve);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Reserve(%d, %d): %d", xreserve, yreserve, res);
      }
      break;

    case 108: { // int16_t Dsp_LoadProg(int8_t *file, int16_t ability, int8_t *buffer)
        int valid = 0;
        uint32_t afile = ARG32;
        int8_t *file = (int8_t *)(memory + afile);
        valid |= (uint8_t *)file >= data->memory && (uint8_t *)file < data->memory + data->memorySize;
        int16_t ability = ARG16;
        uint32_t abuffer = ARG32;
        int8_t *buffer = (int8_t *)(memory + abuffer);
        valid |= (uint8_t *)buffer >= data->memory && (uint8_t *)buffer < data->memory + data->memorySize;
        int16_t res = 0;
        if (valid) {
          res = Dsp_LoadProg(file, ability, buffer);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_LoadProg(0x%08X, %d, 0x%08X): %d", afile, ability, abuffer, res);
      }
      break;

    case 109: { // void Dsp_ExecProg(int8_t *codeptr, int32_t codesize, int16_t ability)
        int valid = 0;
        uint32_t acodeptr = ARG32;
        int8_t *codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        int32_t codesize = ARG32;
        int16_t ability = ARG16;
        if (valid) {
          Dsp_ExecProg(codeptr, codesize, ability);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_ExecProg(0x%08X, %d, %d)", acodeptr, codesize, ability);
      }
      break;

    case 110: { // void Dsp_ExecBoot(int8_t *codeptr, int32_t codesize, int16_t ability)
        int valid = 0;
        uint32_t acodeptr = ARG32;
        int8_t *codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        int32_t codesize = ARG32;
        int16_t ability = ARG16;
        if (valid) {
          Dsp_ExecBoot(codeptr, codesize, ability);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_ExecBoot(0x%08X, %d, %d)", acodeptr, codesize, ability);
      }
      break;

    case 111: { // int32_t Dsp_LodToBinary(int8_t *file, int8_t *codeptr)
        int valid = 0;
        uint32_t afile = ARG32;
        int8_t *file = (int8_t *)(memory + afile);
        valid |= (uint8_t *)file >= data->memory && (uint8_t *)file < data->memory + data->memorySize;
        uint32_t acodeptr = ARG32;
        int8_t *codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = Dsp_LodToBinary(file, codeptr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_LodToBinary(0x%08X, 0x%08X): %d", afile, acodeptr, res);
      }
      break;

    case 112: { // void Dsp_TriggerHC(int16_t vector)
        int16_t vector = ARG16;
        Dsp_TriggerHC(vector);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_TriggerHC(%d)", vector);
      }
      break;

    case 113: { // int16_t Dsp_RequestUniqueAbility(void)
        int16_t res = 0;
        res = Dsp_RequestUniqueAbility();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_RequestUniqueAbility(): %d", res);
      }
      break;

    case 114: { // int16_t Dsp_GetProgAbility(void)
        int16_t res = 0;
        res = Dsp_GetProgAbility();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_GetProgAbility(): %d", res);
      }
      break;

    case 115: { // void Dsp_FlushSubroutines(void)
        Dsp_FlushSubroutines();
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_FlushSubroutines()");
      }
      break;

    case 116: { // int16_t Dsp_LoadSubroutine(int8_t *codeptr, int32_t codesize, int16_t ability)
        int valid = 0;
        uint32_t acodeptr = ARG32;
        int8_t *codeptr = (int8_t *)(memory + acodeptr);
        valid |= (uint8_t *)codeptr >= data->memory && (uint8_t *)codeptr < data->memory + data->memorySize;
        int32_t codesize = ARG32;
        int16_t ability = ARG16;
        int16_t res = 0;
        if (valid) {
          res = Dsp_LoadSubroutine(codeptr, codesize, ability);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_LoadSubroutine(0x%08X, %d, %d): %d", acodeptr, codesize, ability, res);
      }
      break;

    case 117: { // int16_t Dsp_InqSubrAbility(int16_t ability)
        int16_t ability = ARG16;
        int16_t res = 0;
        res = Dsp_InqSubrAbility(ability);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_InqSubrAbility(%d): %d", ability, res);
      }
      break;

    case 118: { // int16_t Dsp_RunSubroutine(int16_t handle)
        int16_t handle = ARG16;
        int16_t res = 0;
        res = Dsp_RunSubroutine(handle);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_RunSubroutine(%d): %d", handle, res);
      }
      break;

    case 119: { // int16_t Dsp_Hf0(int16_t flag)
        int16_t flag = ARG16;
        int16_t res = 0;
        res = Dsp_Hf0(flag);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf0(%d): %d", flag, res);
      }
      break;

    case 120: { // int16_t Dsp_Hf1(int16_t flag)
        int16_t flag = ARG16;
        int16_t res = 0;
        res = Dsp_Hf1(flag);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf1(%d): %d", flag, res);
      }
      break;

    case 121: { // int16_t Dsp_Hf2(void)
        int16_t res = 0;
        res = Dsp_Hf2();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf2(): %d", res);
      }
      break;

    case 122: { // int16_t Dsp_Hf3(void)
        int16_t res = 0;
        res = Dsp_Hf3();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_Hf3(): %d", res);
      }
      break;

    case 123: { // void Dsp_BlkWords(void *data_in, int32_t size_in, void *data_out, int32_t size_out)
        int valid = 0;
        uint32_t adata_in = ARG32;
        void *data_in = (void *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        int32_t size_in = ARG32;
        uint32_t adata_out = ARG32;
        void *data_out = (void *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t size_out = ARG32;
        if (valid) {
          Dsp_BlkWords(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkWords(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 124: { // void Dsp_BlkBytes(void *data_in, int32_t size_in, void *data_out, int32_t size_out)
        int valid = 0;
        uint32_t adata_in = ARG32;
        void *data_in = (void *)(memory + adata_in);
        valid |= (uint8_t *)data_in >= data->memory && (uint8_t *)data_in < data->memory + data->memorySize;
        int32_t size_in = ARG32;
        uint32_t adata_out = ARG32;
        void *data_out = (void *)(memory + adata_out);
        valid |= (uint8_t *)data_out >= data->memory && (uint8_t *)data_out < data->memory + data->memorySize;
        int32_t size_out = ARG32;
        if (valid) {
          Dsp_BlkBytes(data_in, size_in, data_out, size_out);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_BlkBytes(0x%08X, %d, 0x%08X, %d)", adata_in, size_in, adata_out, size_out);
      }
      break;

    case 125: { // int8_t Dsp_HStat(void)
        int8_t res = 0;
        res = Dsp_HStat();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_HStat(): %d", res);
      }
      break;

    case 126: { // void Dsp_SetVectors(void *receiver, void *transmitter)
        int valid = 0;
        uint32_t areceiver = ARG32;
        void *receiver = (void *)(memory + areceiver);
        valid |= (uint8_t *)receiver >= data->memory && (uint8_t *)receiver < data->memory + data->memorySize;
        uint32_t atransmitter = ARG32;
        void *transmitter = (void *)(memory + atransmitter);
        valid |= (uint8_t *)transmitter >= data->memory && (uint8_t *)transmitter < data->memory + data->memorySize;
        if (valid) {
          Dsp_SetVectors(receiver, transmitter);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_SetVectors(0x%08X, 0x%08X)", areceiver, atransmitter);
      }
      break;

    case 127: { // void Dsp_MultBlocks(int32_t numsend, int32_t numreceive, DSPBLOCK *sendblocks, DSPBLOCK *receiveblocks)
        int valid = 0;
        int32_t numsend = ARG32;
        int32_t numreceive = ARG32;
        uint32_t asendblocks = ARG32;
        DSPBLOCK *sendblocks = (DSPBLOCK *)(memory + asendblocks);
        valid |= (uint8_t *)sendblocks >= data->memory && (uint8_t *)sendblocks < data->memory + data->memorySize;
        uint32_t areceiveblocks = ARG32;
        DSPBLOCK *receiveblocks = (DSPBLOCK *)(memory + areceiveblocks);
        valid |= (uint8_t *)receiveblocks >= data->memory && (uint8_t *)receiveblocks < data->memory + data->memorySize;
        if (valid) {
          Dsp_MultBlocks(numsend, numreceive, sendblocks, receiveblocks);
        }
        debug(DEBUG_TRACE, "TOS", "GEMDOS Dsp_MultBlocks(%d, %d, 0x%08X, 0x%08X)", numsend, numreceive, asendblocks, areceiveblocks);
      }
      break;

    case 128: { // int32_t locksnd(void)
        int32_t res = 0;
        res = locksnd();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS locksnd(): %d", res);
      }
      break;

    case 129: { // int32_t unlocksnd(void)
        int32_t res = 0;
        res = unlocksnd();
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS unlocksnd(): %d", res);
      }
      break;

    case 130: { // int32_t soundcmd(int16_t mode, int16_t dat)
        int16_t mode = ARG16;
        int16_t dat = ARG16;
        int32_t res = 0;
        res = soundcmd(mode, dat);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS soundcmd(%d, %d): %d", mode, dat, res);
      }
      break;

    case 131: { // int32_t setbuffer(int16_t reg, void *begaddr, void *endaddr)
        int valid = 0;
        int16_t reg = ARG16;
        uint32_t abegaddr = ARG32;
        void *begaddr = (void *)(memory + abegaddr);
        valid |= (uint8_t *)begaddr >= data->memory && (uint8_t *)begaddr < data->memory + data->memorySize;
        uint32_t aendaddr = ARG32;
        void *endaddr = (void *)(memory + aendaddr);
        valid |= (uint8_t *)endaddr >= data->memory && (uint8_t *)endaddr < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = setbuffer(reg, begaddr, endaddr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setbuffer(%d, 0x%08X, 0x%08X): %d", reg, abegaddr, aendaddr, res);
      }
      break;

    case 132: { // int32_t setmode(int16_t mode)
        int16_t mode = ARG16;
        int32_t res = 0;
        res = setmode(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setmode(%d): %d", mode, res);
      }
      break;

    case 133: { // int32_t settracks(int16_t playtracks, int16_t rectracks)
        int16_t playtracks = ARG16;
        int16_t rectracks = ARG16;
        int32_t res = 0;
        res = settracks(playtracks, rectracks);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS settracks(%d, %d): %d", playtracks, rectracks, res);
      }
      break;

    case 134: { // int32_t setmontracks(int16_t montrack)
        int16_t montrack = ARG16;
        int32_t res = 0;
        res = setmontracks(montrack);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setmontracks(%d): %d", montrack, res);
      }
      break;

    case 135: { // int32_t setinterrupt(int16_t src_inter, int16_t cause)
        int16_t src_inter = ARG16;
        int16_t cause = ARG16;
        int32_t res = 0;
        res = setinterrupt(src_inter, cause);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS setinterrupt(%d, %d): %d", src_inter, cause, res);
      }
      break;

    case 136: { // int32_t buffoper(int16_t mode)
        int16_t mode = ARG16;
        int32_t res = 0;
        res = buffoper(mode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS buffoper(%d): %d", mode, res);
      }
      break;

    case 137: { // int32_t dsptristate(int16_t dspxmit, int16_t dsprec)
        int16_t dspxmit = ARG16;
        int16_t dsprec = ARG16;
        int32_t res = 0;
        res = dsptristate(dspxmit, dsprec);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS dsptristate(%d, %d): %d", dspxmit, dsprec, res);
      }
      break;

    case 138: { // int32_t gpio(int16_t mode, int16_t dat)
        int16_t mode = ARG16;
        int16_t dat = ARG16;
        int32_t res = 0;
        res = gpio(mode, dat);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS gpio(%d, %d): %d", mode, dat, res);
      }
      break;

    case 139: { // int32_t devconnect(int16_t src, int16_t dst, int16_t srcclk, int16_t prescale, int16_t protocol)
        int16_t src = ARG16;
        int16_t dst = ARG16;
        int16_t srcclk = ARG16;
        int16_t prescale = ARG16;
        int16_t protocol = ARG16;
        int32_t res = 0;
        res = devconnect(src, dst, srcclk, prescale, protocol);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS devconnect(%d, %d, %d, %d, %d): %d", src, dst, srcclk, prescale, protocol, res);
      }
      break;

    case 140: { // int16_t sndstatus(int16_t reset)
        int16_t reset = ARG16;
        int16_t res = 0;
        res = sndstatus(reset);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS sndstatus(%d): %d", reset, res);
      }
      break;

    case 141: { // int32_t buffptr(int32_t *ptr)
        int valid = 0;
        uint32_t aptr = ARG32;
        int32_t *ptr = (int32_t *)(memory + aptr);
        valid |= (uint8_t *)ptr >= data->memory && (uint8_t *)ptr < data->memory + data->memorySize;
        int32_t res = 0;
        if (valid) {
          res = buffptr(ptr);
        }
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS buffptr(0x%08X): %d", aptr, res);
      }
      break;

    case 150: { // void VsetMask(int32_t ormask, int32_t andmask, int16_t overlay)
        int32_t ormask = ARG32;
        int32_t andmask = ARG32;
        int16_t overlay = ARG16;
        VsetMask(ormask, andmask, overlay);
        debug(DEBUG_TRACE, "TOS", "GEMDOS VsetMask(%d, %d, %d)", ormask, andmask, overlay);
      }
      break;

    case 160: { // int32_t CacheCtrl(int16_t OpCode, int16_t Param)
        int16_t OpCode = ARG16;
        int16_t Param = ARG16;
        int32_t res = 0;
        res = CacheCtrl(OpCode, Param);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS CacheCtrl(%d, %d): %d", OpCode, Param, res);
      }
      break;

    case 161: { // int32_t WdgCtrl(int16_t OpCode)
        int16_t OpCode = ARG16;
        int32_t res = 0;
        res = WdgCtrl(OpCode);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS WdgCtrl(%d): %d", OpCode, res);
      }
      break;

    case 162: { // int32_t Xbios(int16_t command, int16_t device, int32_t param)
        int16_t command = ARG16;
        int16_t device = ARG16;
        int32_t param = ARG32;
        int32_t res = 0;
        res = Xbios(command, device, param);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS Xbios(%d, %d, %d): %d", command, device, param, res);
      }
      break;

    case 165: { // int32_t WavePlay(int16_t flags, int32_t rate, int32_t sptr, int32_t slen)
        int16_t flags = ARG16;
        int32_t rate = ARG32;
        int32_t sptr = ARG32;
        int32_t slen = ARG32;
        int32_t res = 0;
        res = WavePlay(flags, rate, sptr, slen);
        m68k_set_reg(M68K_REG_D0, res);
        debug(DEBUG_TRACE, "TOS", "GEMDOS WavePlay(%d, %d, %d, %d): %d", flags, rate, sptr, slen, res);
      }
      break;

