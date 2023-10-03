void optable(z80_t *z, int op) {
  int temp, sum, cbits, acu;
  uint8_t b;

  switch (op) {
    case 0x00:      /* NOP */
      break;
    case 0x01:      /* LD bc[selr],nnnn */
      bc[selr] = getWord(PC);
      PC += 2;
      break;
    case 0x02:      /* LD (bc[selr]),A */
      putByte(bc[selr], hreg(af[sela]));
      break;
    case 0x03:      /* INC bc[selr] */
      ++bc[selr];
      break;
    case 0x04:      /* INC B */
      bc[selr] += 0x100;
      temp = hreg(bc[selr]);
      af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
      (((temp & 0xff) == 0 ? 1 : 0) << 6) |
      (((temp & 0xf) == 0 ? 1 : 0) << 4) |
      ((temp == 0x80 ? 1 : 0) << 2);
      break;
    case 0x05:      /* DEC B */
      bc[selr] -= 0x100;
      temp = hreg(bc[selr]);
      af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
      (((temp & 0xff) == 0 ? 1 : 0) << 6) |
      (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
      ((temp == 0x7f ? 1 : 0) << 2) | 2;
      break;
    case 0x06:      /* LD B,nn */
      bc[selr] = sethreg(bc[selr], getByte(PC)); ++PC;
      break;
    case 0x07:      /* RLCA */
      af[sela] = ((af[sela] >> 7) & 0x0128) | ((af[sela] << 1) & ~0x1ff) |
      (af[sela] & 0xc4) | ((af[sela] >> 15) & 1);
      break;
    case 0x08:      /* EX af[sela],af[sela]' */
      sela = 1 - sela;
      break;
    case 0x09:      /* ADD hl[selr],bc[selr] */
      hl[selr] &= 0xffff;
      bc[selr] &= 0xffff;
      sum = hl[selr] + bc[selr];
      cbits = (hl[selr] ^ bc[selr] ^ sum) >> 8;
      hl[selr] = sum;
      af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
      (cbits & 0x10) | ((cbits >> 8) & 1);
      break;
      case 0x0A:      /* LD A,(bc[selr]) */
        af[sela] = sethreg(af[sela], getByte(bc[selr]));
        break;
      case 0x0B:      /* DEC bc[selr] */
        --bc[selr];
        break;
      case 0x0C:      /* INC C */
        temp = lreg(bc[selr])+1;
        bc[selr] = setlreg(bc[selr], temp);
        af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
        (((temp & 0xff) == 0 ? 1 : 0) << 6) |
        (((temp & 0xf) == 0 ? 1 : 0) << 4) |
        ((temp == 0x80 ? 1 : 0) << 2);
        break;
      case 0x0D:      /* DEC C */
        temp = lreg(bc[selr])-1;
        bc[selr] = setlreg(bc[selr], temp);
        af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
        (((temp & 0xff) == 0 ? 1 : 0) << 6) |
        (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
        ((temp == 0x7f ? 1 : 0) << 2) | 2;
        break;
      case 0x0E:      /* LD C,nn */
        bc[selr] = setlreg(bc[selr], getByte(PC)); ++PC;
        break;
      case 0x0F:      /* RRCA */
        temp = hreg(af[sela]);
        sum = temp >> 1;
        af[sela] = ((temp & 1) << 15) | (sum << 8) |
        (sum & 0x28) | (af[sela] & 0xc4) | (temp & 1);
        break;
        case 0x10:      /* DJNZ dd */
          PC += (((bc[selr] -= 0x100) & 0xff00) != 0) ? (char)getByte(PC) + 1 : 1;
          break;
        case 0x11:      /* LD de[selr],nnnn */
          de[selr] = getWord(PC);
          PC += 2;
          //System.out.printf("DE=%04X\n", de[selr]);
          break;
        case 0x12:      /* LD (de[selr]),A */
          putByte(de[selr], hreg(af[sela]));
          break;
        case 0x13:      /* INC de[selr] */
          ++de[selr];
          break;
        case 0x14:      /* INC D */
          de[selr] += 0x100;
          temp = hreg(de[selr]);
          af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
          (((temp & 0xff) == 0 ? 1 : 0) << 6) |
          (((temp & 0xf) == 0 ? 1 : 0) << 4) |
          ((temp == 0x80 ? 1 : 0) << 2);
          break;
        case 0x15:      /* DEC D */
          de[selr] -= 0x100;
          temp = hreg(de[selr]);
          af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
          (((temp & 0xff) == 0 ? 1 : 0) << 6) |
          (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
          ((temp == 0x7f ? 1 : 0) << 2) | 2;
          break;
        case 0x16:      /* LD D,nn */
          de[selr] = sethreg(de[selr], getByte(PC)); ++PC;
          break;
        case 0x17:      /* RLA */
          af[sela] = ((af[sela] << 8) & 0x0100) | ((af[sela] >> 7) & 0x28) | ((af[sela] << 1) & ~0x01ff) |
          (af[sela] & 0xc4) | ((af[sela] >> 15) & 1);
          break;
        case 0x18:      /* JR dd */
          PC += (char)getByte(PC) + 1;
          break;
        case 0x19:      /* ADD hl[selr],de[selr] */
          hl[selr] &= 0xffff;
          de[selr] &= 0xffff;
          sum = hl[selr] + de[selr];
          cbits = (hl[selr] ^ de[selr] ^ sum) >> 8;
          hl[selr] = sum;
          af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
          (cbits & 0x10) | ((cbits >> 8) & 1);
          break;
          case 0x1A:      /* LD A,(de[selr]) */
            af[sela] = sethreg(af[sela], getByte(de[selr]));
            break;
          case 0x1B:      /* DEC de[selr] */
            --de[selr];
            break;
          case 0x1C:      /* INC E */
            temp = lreg(de[selr])+1;
            de[selr] = setlreg(de[selr], temp);
            af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
            (((temp & 0xff) == 0 ? 1 : 0) << 6) |
            (((temp & 0xf) == 0 ? 1 : 0) << 4) |
            ((temp == 0x80 ? 1 : 0) << 2);
            break;
          case 0x1D:      /* DEC E */
            temp = lreg(de[selr])-1;
            de[selr] = setlreg(de[selr], temp);
            af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
            (((temp & 0xff) == 0 ? 1 : 0) << 6) |
            (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
            ((temp == 0x7f ? 1 : 0) << 2) | 2;
            break;
          case 0x1E:      /* LD E,nn */
            de[selr] = setlreg(de[selr], getByte(PC)); ++PC;
            break;
          case 0x1F:      /* RRA */
            temp = hreg(af[sela]);
            sum = temp >> 1;
            af[sela] = ((af[sela] & 1) << 15) | (sum << 8) |
            (sum & 0x28) | (af[sela] & 0xc4) | (temp & 1);
            break;
            case 0x20:      /* JR NZ,dd */
              PC += (TSTFLAG(FLAG_Z) == 0) ? (char)getByte(PC) + 1 : 1;
              break;
            case 0x21:      /* LD hl[selr],nnnn */
              hl[selr] = getWord(PC);
              PC += 2;
              break;
            case 0x22:      /* LD (nnnn),hl[selr] */
              temp = getWord(PC);
              putWord(temp, hl[selr]);
              PC += 2;
              break;
            case 0x23:      /* INC hl[selr] */
              ++hl[selr];
              break;
            case 0x24:      /* INC H */
              hl[selr] += 0x100;
              temp = hreg(hl[selr]);
              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
              (((temp & 0xff) == 0 ? 1 : 0) << 6) |
              (((temp & 0xf) == 0 ? 1 : 0) << 4) |
              ((temp == 0x80 ? 1 : 0) << 2);
              break;
            case 0x25:      /* DEC H */
              hl[selr] -= 0x100;
              temp = hreg(hl[selr]);
              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
              (((temp & 0xff) == 0 ? 1 : 0) << 6) |
              (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
              ((temp == 0x7f ? 1 : 0) << 2) | 2;
              break;
            case 0x26:      /* LD H,nn */
              hl[selr] = sethreg(hl[selr], getByte(PC)); ++PC;
              break;
            case 0x27:      /* DAA */
              acu = hreg(af[sela]);
              temp = ldig(acu);
              cbits = TSTFLAG(FLAG_C);
              if (TSTFLAG(FLAG_N) != 0) {  /* last operation was a subtract */
                int hd = cbits != 0 || acu > 0x99;
                if (TSTFLAG(FLAG_H) != 0 || (temp > 9)) { /* adjust low digit */
                  if (temp > 5)
                    SETFLAG(z, FLAG_H, 0);
                  acu -= 6;
                  acu &= 0xff;
                }
                if (hd) {   /* adjust high digit */
                  acu -= 0x160;
                }
              }
              else {      /* last operation was an add */
                if (TSTFLAG(FLAG_H) != 0 || (temp > 9)) { /* adjust low digit */
                  SETFLAG(z, FLAG_H, (temp > 9) ? 1 : 0);
                  acu += 6;
                }
                if (cbits != 0 || ((acu & 0x1f0) > 0x90)) /* adjust high digit */
                  acu += 0x60;
              }
              cbits |= (acu >> 8) & 1;
              acu &= 0xff;
              af[sela] = (acu << 8) | (acu & 0xa8) | ((acu == 0 ? 1 : 0) << 6) |
              (af[sela] & 0x12) | parity(acu) | cbits;
              break;
            case 0x28:      /* JR Z,dd */
              PC += (TSTFLAG(FLAG_Z) != 0) ? (char)getByte(PC) + 1 : 1;
              break;
            case 0x29:      /* ADD hl[selr],hl[selr] */
              hl[selr] &= 0xffff;
              sum = hl[selr] + hl[selr];
              cbits = (hl[selr] ^ hl[selr] ^ sum) >> 8;
              hl[selr] = sum;
              af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
              (cbits & 0x10) | ((cbits >> 8) & 1);
              break;
              case 0x2A:      /* LD hl[selr],(nnnn) */
                temp = getWord(PC);
                hl[selr] = getWord(temp);
                PC += 2;
                break;
              case 0x2B:      /* DEC hl[selr] */
                --hl[selr];
                break;
              case 0x2C:      /* INC L */
                temp = lreg(hl[selr])+1;
                hl[selr] = setlreg(hl[selr], temp);
                af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                (((temp & 0xf) == 0 ? 1 : 0) << 4) |
                ((temp == 0x80 ? 1 : 0) << 2);
                break;
              case 0x2D:      /* DEC L */
                temp = lreg(hl[selr])-1;
                hl[selr] = setlreg(hl[selr], temp);
                af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
                ((temp == 0x7f ? 1 : 0) << 2) | 2;
                break;
              case 0x2E:      /* LD L,nn */
                hl[selr] = setlreg(hl[selr], getByte(PC)); ++PC;
                break;
              case 0x2F:      /* CPL */
                af[sela] = (~af[sela] & ~0xff) | (af[sela] & 0xc5) | ((~af[sela] >> 8) & 0x28) | 0x12;
                break;
              case 0x30:      /* JR NC,dd */
                PC += (TSTFLAG(FLAG_C) == 0) ? (char)getByte(PC) + 1 : 1;
                break;
              case 0x31:      /* LD SP,nnnn */
                SP = getWord(PC);
                PC += 2;
                break;
              case 0x32:      /* LD (nnnn),A */
                temp = getWord(PC);
                putByte(temp, hreg(af[sela]));
                PC += 2;
                break;
              case 0x33:      /* INC SP */
                ++SP;
                break;
              case 0x34:      /* INC (hl[selr]) */
                temp = getByte(hl[selr])+1;
                putByte(hl[selr], temp);
                af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                (((temp & 0xf) == 0 ? 1 : 0) << 4) |
                ((temp == 0x80 ? 1 : 0) << 2);
                break;
              case 0x35:      /* DEC (hl[selr]) */
                temp = getByte(hl[selr])-1;
                putByte(hl[selr], temp);
                af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
                ((temp == 0x7f ? 1 : 0) << 2) | 2;
                break;
              case 0x36:      /* LD (hl[selr]),nn */
                putByte(hl[selr], getByte(PC)); ++PC;
                break;
              case 0x37:      /* SCF */
                af[sela] = (af[sela]&~0x3b)|((af[sela]>>8)&0x28)|1;
                break;
              case 0x38:      /* JR C,dd */
                PC += (TSTFLAG(FLAG_C) != 0) ? (char)getByte(PC) + 1 : 1;
                break;
              case 0x39:      /* ADD hl[selr],SP */
                hl[selr] &= 0xffff;
                SP &= 0xffff;
                sum = hl[selr] + SP;
                cbits = (hl[selr] ^ SP ^ sum) >> 8;
                hl[selr] = sum;
                af[sela] = (af[sela] & ~0x3b) | ((sum >> 8) & 0x28) |
                (cbits & 0x10) | ((cbits >> 8) & 1);
                break;
                case 0x3A:      /* LD A,(nnnn) */
                  temp = getWord(PC);
                  af[sela] = sethreg(af[sela], getByte(temp));
                  PC += 2;
                  break;
                case 0x3B:      /* DEC SP */
                  --SP;
                  break;
                case 0x3C:      /* INC A */
                  af[sela] += 0x100;
                  temp = hreg(af[sela]);
                  af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                  (((temp & 0xf) == 0 ? 1 : 0) << 4) |
                  ((temp == 0x80 ? 1 : 0) << 2);
                  break;
                case 0x3D:      /* DEC A */
                  af[sela] -= 0x100;
                  temp = hreg(af[sela]);
                  af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                  (((temp & 0xf) == 0xf ? 1 : 0) << 4) |
                  ((temp == 0x7f ? 1 : 0) << 2) | 2;
                  break;
                case 0x3E:      /* LD A,nn */
                  af[sela] = sethreg(af[sela], getByte(PC)); ++PC;
                  break;
                case 0x3F:      /* CCF */
                  //af[sela] = (af[sela]&~0x3b)|((af[sela]>>8)&0x28)|((af[sela]&1)<<4)|(~af[sela]&1);

                  // 0xC4

                  // #define FLAG_S   0x80
                  // #define FLAG_Z   0x40
                  // #define FLAG_Y   0x20
                  // #define FLAG_H   0x10
                  // #define FLAG_X   0x08
                  // #define FLAG_P   0x04
                  // #define FLAG_N   0x02
                  // #define FLAG_C   0x01

                  // S is not affected.
                  // Z is not affected.
                  // H, previous carry is copied.
                  // P/V is not affected.
                  // N is reset.
                  // C is set if CY was 0 before operation; otherwise, it is reset.

                  af[sela] = (af[sela] & 0xEC) | (af[sela] ^ 0x0001) | ((af[sela] & 0x01) << 4);
                  break;
                case 0x40:      /* LD B,B */
                  /* nop */
                  break;
                case 0x41:      /* LD B,C */
                  bc[selr] = (bc[selr] & 255) | ((bc[selr] & 255) << 8);
                  break;
                case 0x42:      /* LD B,D */
                  bc[selr] = (bc[selr] & 255) | (de[selr] & ~255);
                  break;
                case 0x43:      /* LD B,E */
                  bc[selr] = (bc[selr] & 255) | ((de[selr] & 255) << 8);
                  break;
                case 0x44:      /* LD B,H */
                  bc[selr] = (bc[selr] & 255) | (hl[selr] & ~255);
                  break;
                case 0x45:      /* LD B,L */
                  bc[selr] = (bc[selr] & 255) | ((hl[selr] & 255) << 8);
                  break;
                case 0x46:      /* LD B,(hl[selr]) */
                  bc[selr] = sethreg(bc[selr], getByte(hl[selr]));
                  break;
                case 0x47:      /* LD B,A */
                  bc[selr] = (bc[selr] & 255) | (af[sela] & ~255);
                  break;
                case 0x48:      /* LD C,B */
                  bc[selr] = (bc[selr] & ~255) | ((bc[selr] >> 8) & 255);
                  break;
                case 0x49:      /* LD C,C */
                  /* nop */
                  break;
                case 0x4A:      /* LD C,D */
                  bc[selr] = (bc[selr] & ~255) | ((de[selr] >> 8) & 255);
                  break;
                case 0x4B:      /* LD C,E */
                  bc[selr] = (bc[selr] & ~255) | (de[selr] & 255);
                  break;
                case 0x4C:      /* LD C,H */
                  bc[selr] = (bc[selr] & ~255) | ((hl[selr] >> 8) & 255);
                  break;
                case 0x4D:      /* LD C,L */
                  bc[selr] = (bc[selr] & ~255) | (hl[selr] & 255);
                  break;
                case 0x4E:      /* LD C,(hl[selr]) */
                  bc[selr] = setlreg(bc[selr], getByte(hl[selr]));
                  break;
                case 0x4F:      /* LD C,A */
                  bc[selr] = (bc[selr] & ~255) | ((af[sela] >> 8) & 255);
                  break;
                case 0x50:      /* LD D,B */
                  de[selr] = (de[selr] & 255) | (bc[selr] & ~255);
                  break;
                case 0x51:      /* LD D,C */
                  de[selr] = (de[selr] & 255) | ((bc[selr] & 255) << 8);
                  break;
                case 0x52:      /* LD D,D */
                  /* nop */
                  break;
                case 0x53:      /* LD D,E */
                  de[selr] = (de[selr] & 255) | ((de[selr] & 255) << 8);
                  break;
                case 0x54:      /* LD D,H */
                  de[selr] = (de[selr] & 255) | (hl[selr] & ~255);
                  break;
                case 0x55:      /* LD D,L */
                  de[selr] = (de[selr] & 255) | ((hl[selr] & 255) << 8);
                  break;
                case 0x56:      /* LD D,(hl[selr]) */
                  de[selr] = sethreg(de[selr], getByte(hl[selr]));
                  break;
                case 0x57:      /* LD D,A */
                  de[selr] = (de[selr] & 255) | (af[sela] & ~255);
                  break;
                case 0x58:      /* LD E,B */
                  de[selr] = (de[selr] & ~255) | ((bc[selr] >> 8) & 255);
                  break;
                case 0x59:      /* LD E,C */
                  de[selr] = (de[selr] & ~255) | (bc[selr] & 255);
                  break;
                case 0x5A:      /* LD E,D */
                  de[selr] = (de[selr] & ~255) | ((de[selr] >> 8) & 255);
                  break;
                case 0x5B:      /* LD E,E */
                  /* nop */
                  break;
                case 0x5C:      /* LD E,H */
                  de[selr] = (de[selr] & ~255) | ((hl[selr] >> 8) & 255);
                  break;
                case 0x5D:      /* LD E,L */
                  de[selr] = (de[selr] & ~255) | (hl[selr] & 255);
                  break;
                case 0x5E:      /* LD E,(hl[selr]) */
                  de[selr] = setlreg(de[selr], getByte(hl[selr]));
                  break;
                case 0x5F:      /* LD E,A */
                  de[selr] = (de[selr] & ~255) | ((af[sela] >> 8) & 255);
                  break;
                case 0x60:      /* LD H,B */
                  hl[selr] = (hl[selr] & 255) | (bc[selr] & ~255);
                  break;
                case 0x61:      /* LD H,C */
                  hl[selr] = (hl[selr] & 255) | ((bc[selr] & 255) << 8);
                  break;
                case 0x62:      /* LD H,D */
                  hl[selr] = (hl[selr] & 255) | (de[selr] & ~255);
                  break;
                case 0x63:      /* LD H,E */
                  hl[selr] = (hl[selr] & 255) | ((de[selr] & 255) << 8);
                  break;
                case 0x64:      /* LD H,H */
                  /* nop */
                  break;
                case 0x65:      /* LD H,L */
                  hl[selr] = (hl[selr] & 255) | ((hl[selr] & 255) << 8);
                  break;
                case 0x66:      /* LD H,(hl[selr]) */
                  hl[selr] = sethreg(hl[selr], getByte(hl[selr]));
                  break;
                case 0x67:      /* LD H,A */
                  hl[selr] = (hl[selr] & 255) | (af[sela] & ~255);
                  break;
                case 0x68:      /* LD L,B */
                  hl[selr] = (hl[selr] & ~255) | ((bc[selr] >> 8) & 255);
                  break;
                case 0x69:      /* LD L,C */
                  hl[selr] = (hl[selr] & ~255) | (bc[selr] & 255);
                  break;
                case 0x6A:      /* LD L,D */
                  hl[selr] = (hl[selr] & ~255) | ((de[selr] >> 8) & 255);
                  break;
                case 0x6B:      /* LD L,E */
                  hl[selr] = (hl[selr] & ~255) | (de[selr] & 255);
                  break;
                case 0x6C:      /* LD L,H */
                  hl[selr] = (hl[selr] & ~255) | ((hl[selr] >> 8) & 255);
                  break;
                case 0x6D:      /* LD L,L */
                  /* nop */
                  break;
                case 0x6E:      /* LD L,(hl[selr]) */
                  hl[selr] = setlreg(hl[selr], getByte(hl[selr]));
                  break;
                case 0x6F:      /* LD L,A */
                  hl[selr] = (hl[selr] & ~255) | ((af[sela] >> 8) & 255);
                  break;
                case 0x70:      /* LD (hl[selr]),B */
                  putByte(hl[selr], hreg(bc[selr]));
                  break;
                case 0x71:      /* LD (hl[selr]),C */
                  putByte(hl[selr], lreg(bc[selr]));
                  break;
                case 0x72:      /* LD (hl[selr]),D */
                  putByte(hl[selr], hreg(de[selr]));
                  break;
                case 0x73:      /* LD (hl[selr]),E */
                  putByte(hl[selr], lreg(de[selr]));
                  break;
                case 0x74:      /* LD (hl[selr]),H */
                  putByte(hl[selr], hreg(hl[selr]));
                  break;
                case 0x75:      /* LD (hl[selr]),L */
                  putByte(hl[selr], lreg(hl[selr]));
                  break;
                case 0x76:      /* HALT */
                  z80_halt(z, 1);
                  //iCount = 0;
                  break;
                case 0x77:      /* LD (hl[selr]),A */
                  putByte(hl[selr], hreg(af[sela]));
                  break;
                case 0x78:      /* LD A,B */
                  af[sela] = (af[sela] & 255) | (bc[selr] & ~255);
                  break;
                case 0x79:      /* LD A,C */
                  af[sela] = (af[sela] & 255) | ((bc[selr] & 255) << 8);
                  break;
                case 0x7A:      /* LD A,D */
                  af[sela] = (af[sela] & 255) | (de[selr] & ~255);
                  break;
                case 0x7B:      /* LD A,E */
                  af[sela] = (af[sela] & 255) | ((de[selr] & 255) << 8);
                  break;
                case 0x7C:      /* LD A,H */
                  af[sela] = (af[sela] & 255) | (hl[selr] & ~255);
                  break;
                case 0x7D:      /* LD A,L */
                  af[sela] = (af[sela] & 255) | ((hl[selr] & 255) << 8);
                  break;
                case 0x7E:      /* LD A,(hl[selr]) */
                  af[sela] = sethreg(af[sela], getByte(hl[selr]));
                  break;
                case 0x7F:      /* LD A,A */
                  /* nop */
                  break;
                case 0x80:      /* ADD A,B */
                  temp = hreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x81:      /* ADD A,C */
                  temp = lreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x82:      /* ADD A,D */
                  temp = hreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x83:      /* ADD A,E */
                  temp = lreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x84:      /* ADD A,H */
                  temp = hreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x85:      /* ADD A,L */
                  temp = lreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x86:      /* ADD A,(hl[selr]) */
                  temp = getByte(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x87:      /* ADD A,A */
                  temp = hreg(af[sela]);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x88:      /* ADC A,B */
                  temp = hreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x89:      /* ADC A,C */
                  temp = lreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x8A:      /* ADC A,D */
                  temp = hreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x8B:      /* ADC A,E */
                  temp = lreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x8C:      /* ADC A,H */
                  temp = hreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x8D:      /* ADC A,L */
                  temp = lreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x8E:      /* ADC A,(hl[selr]) */
                  temp = getByte(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x8F:      /* ADC A,A */
                  temp = hreg(af[sela]);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  break;
                case 0x90:      /* SUB B */
                  temp = hreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x91:      /* SUB C */
                  temp = lreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x92:      /* SUB D */
                  temp = hreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x93:      /* SUB E */
                  temp = lreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x94:      /* SUB H */
                  temp = hreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x95:      /* SUB L */
                  temp = lreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x96:      /* SUB (hl[selr]) */
                  temp = getByte(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x97:      /* SUB A */
                  temp = hreg(af[sela]);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x98:      /* SBC A,B */
                  temp = hreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x99:      /* SBC A,C */
                  temp = lreg(bc[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x9A:      /* SBC A,D */
                  temp = hreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x9B:      /* SBC A,E */
                  temp = lreg(de[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x9C:      /* SBC A,H */
                  temp = hreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x9D:      /* SBC A,L */
                  temp = lreg(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x9E:      /* SBC A,(hl[selr]) */
                  temp = getByte(hl[selr]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0x9F:      /* SBC A,A */
                  temp = hreg(af[sela]);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  break;
                case 0xA0:      /* AND B */
                  //int x11 = af[sela] >> 8;
                  //int x21 = bc[selr] >> 8;
                  sum = ((af[sela] & (bc[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) |
                  ((sum == 0 ? 1 : 0) << 6) | 0x10 | parity(sum);
                  //System.out.printf("AND B %02X %02X HL=%04X DE=%04X z=%d\n", x11, x21, hl[selr], de[selr], TSTFLAG(FLAG_Z));
                  break;
                case 0xA1:      /* AND C */
                  sum = ((af[sela] >> 8) & bc[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                  ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xA2:      /* AND D */
                  sum = ((af[sela] & (de[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) |
                  ((sum == 0 ? 1 : 0) << 6) | 0x10 | parity(sum);
                  break;
                case 0xA3:      /* AND E */
                  sum = ((af[sela] >> 8) & de[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                  ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xA4:      /* AND H */
                  sum = ((af[sela] & (hl[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) |
                  ((sum == 0 ? 1 : 0) << 6) | 0x10 | parity(sum);
                  break;
                case 0xA5:      /* AND L */
                  sum = ((af[sela] >> 8) & hl[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                  ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xA6:      /* AND (hl[selr]) */
                  sum = ((af[sela] >> 8) & getByte(hl[selr])) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                  ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xA7:      /* AND A */
                  sum = ((af[sela] & (af[sela])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) |
                  ((sum == 0 ? 1 : 0) << 6) | 0x10 | parity(sum);
                  break;
                case 0xA8:      /* XOR B */
                  sum = ((af[sela] ^ (bc[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xA9:      /* XOR C */
                  sum = ((af[sela] >> 8) ^ bc[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xAA:      /* XOR D */
                  sum = ((af[sela] ^ (de[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xAB:      /* XOR E */
                  sum = ((af[sela] >> 8) ^ de[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xAC:      /* XOR H */
                  sum = ((af[sela] ^ (hl[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xAD:      /* XOR L */
                  sum = ((af[sela] >> 8) ^ hl[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xAE:      /* XOR (hl[selr]) */
                  sum = ((af[sela] >> 8) ^ getByte(hl[selr])) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xaf:      /* XOR A */
                  sum = ((af[sela] ^ (af[sela])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB0:      /* OR B */
                  sum = ((af[sela] | (bc[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB1:      /* OR C */
                  sum = ((af[sela] >> 8) | bc[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB2:      /* OR D */
                  sum = ((af[sela] | (de[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB3:      /* OR E */
                  sum = ((af[sela] >> 8) | de[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB4:      /* OR H */
                  sum = ((af[sela] | (hl[selr])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB5:      /* OR L */
                  sum = ((af[sela] >> 8) | hl[selr]) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB6:      /* OR (hl[selr]) */
                  sum = ((af[sela] >> 8) | getByte(hl[selr])) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB7:      /* OR A */
                  sum = ((af[sela] | (af[sela])) >> 8) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  break;
                case 0xB8:      /* CP B */
                  temp = hreg(bc[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xB9:      /* CP C */
                  temp = lreg(bc[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xBA:      /* CP D */
                  temp = hreg(de[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xBB:      /* CP E */
                  temp = lreg(de[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xbc:      /* CP H */
                  temp = hreg(hl[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xBD:      /* CP L */
                  temp = lreg(hl[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xBE:      /* CP (hl[selr]) */
                  temp = getByte(hl[selr]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xBF:      /* CP A */
                  temp = hreg(af[sela]);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  break;
                case 0xC0:      /* RET NZ */
                  if (TSTFLAG(FLAG_Z) == 0) PC = POP(z);
                  break;
                case 0xC1:      /* POP bc[selr] */
                  bc[selr] = POP(z);
                  break;
                case 0xC2:      /* JP NZ,nnnn */
                  JPC(z, TSTFLAG(FLAG_Z) == 0);
                  break;
                case 0xC3:      /* JP nnnn */
                  JPC(z, 1);
                  break;
                case 0xC4:      /* CALL NZ,nnnn */
                  CALLC(z, TSTFLAG(FLAG_Z) == 0);
                  break;
                case 0xC5:      /* PUSH bc[selr] */
                  PUSH(z, bc[selr]);
                  break;
                case 0xC6:      /* ADD A,nn */
                  temp = getByte(PC);
                  acu = hreg(af[sela]);
                  sum = acu + temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  ++PC;
                  break;
                case 0xC7:      /* RST 0 */
                  PUSH(z, PC); PC = 0;
                  break;
                case 0xC8:      /* RET Z */
                  if (TSTFLAG(FLAG_Z) != 0) PC = POP(z);
                  break;
                case 0xC9:      /* RET */
                  PC = POP(z);
                  break;
                case 0xCA:      /* JP Z,nnnn */
                  JPC(z, TSTFLAG(FLAG_Z) != 0);
                  break;
                case 0xCB:      /* CB prefix */
                  cbPrefix(z, hl[selr]);
                  break;
                case 0xCC:      /* CALL Z,nnnn */
                  CALLC(z, TSTFLAG(FLAG_Z) != 0);
                  break;
                case 0xCD:      /* CALL nnnn */
                  CALLC(z, 1);
                  break;
                case 0xCE:      /* ADC A,nn */
                  temp = getByte(PC);
                  acu = hreg(af[sela]);
                  sum = acu + temp + TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                  ((cbits >> 8) & 1);
                  ++PC;
                  break;
                case 0xCF:      /* RST 8 */
                  PUSH(z, PC); PC = 8;
                  break;
                case 0xD0:      /* RET NC */
                  if (TSTFLAG(FLAG_C) == 0) PC = POP(z);
                  break;
                case 0xD1:      /* POP de[selr] */
                  de[selr] = POP(z);
                  break;
                case 0xD2:      /* JP NC,nnnn */
                  JPC(z, TSTFLAG(FLAG_C) == 0);
                  break;
                case 0xD3:      /* OUT (nn),A */
                  output(z, getByte(PC), hreg(af[sela])); ++PC;
                  break;
                case 0xD4:      /* CALL NC,nnnn */
                  CALLC(z, TSTFLAG(FLAG_C) == 0);
                  break;
                case 0xD5:      /* PUSH de[selr] */
                  PUSH(z, de[selr]);
                  break;
                case 0xD6:      /* SUB nn */
                  temp = getByte(PC);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  ++PC;
                  break;
                case 0xD7:      /* RST 10H */
                  PUSH(z, PC); PC = 0x10;
                  break;
                case 0xD8:      /* RET C */
                  if (TSTFLAG(FLAG_C) != 0) PC = POP(z);
                  break;
                case 0xD9:      /* EXX */
                  selr = 1 - selr;
                  break;
                case 0xDA:      /* JP C,nnnn */
                  JPC(z, TSTFLAG(FLAG_C) != 0);
                  break;
                case 0xDB:      /* IN A,(nn) */           
                  af[sela] = sethreg(af[sela], input(z, (af[sela] & 0xFF00) | getByte(PC))); ++PC;
                  break;
                case 0xDC:      /* CALL C,nnnn */
                  CALLC(z, TSTFLAG(FLAG_C) != 0);
                  break;
                case 0xDD:      /* DD prefix */
                  IX = dfdPrefix(z, IX);
                  break;
                case 0xde:      /* SBC A,nn */
                  temp = getByte(PC);
                  acu = hreg(af[sela]);
                  sum = acu - temp - TSTFLAG(FLAG_C);
                  cbits = acu ^ temp ^ sum;
                  af[sela] = ((sum & 0xff) << 8) | (sum & 0xa8) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (cbits & 0x10) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  ((cbits >> 8) & 1);
                  ++PC;
                  break;
                case 0xDF:      /* RST 18H */
                  PUSH(z, PC); PC = 0x18;
                  break;
                case 0xE0:      /* RET PO */
                  if (TSTFLAG(FLAG_P) == 0) PC = POP(z);
                  break;
                case 0xE1:      /* POP hl[selr] */
                  hl[selr] = POP(z);
                  break;
                case 0xE2:      /* JP PO,nnnn */
                  JPC(z, TSTFLAG(FLAG_P) == 0);
                  break;
                case 0xE3:      /* EX (SP),hl[selr] */
                  temp = hl[selr]; hl[selr] = POP(z); PUSH(z, temp);
                  break;
                case 0xE4:      /* CALL PO,nnnn */
                  CALLC(z, TSTFLAG(FLAG_P) == 0);
                  break;
                case 0xE5:      /* PUSH hl[selr] */
                  PUSH(z, hl[selr]);
                  break;
                case 0xE6:      /* AND nn */
                  sum = ((af[sela] >> 8) & getByte(PC)) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | 0x10 |
                  ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  ++PC;
                  break;
                case 0xE7:      /* RST 20H */
                  PUSH(z, PC); PC = 0x20;
                  break;
                case 0xE8:      /* RET PE */
                  if (TSTFLAG(FLAG_P) != 0) PC = POP(z);
                  break;
                case 0xE9:      /* JP (hl[selr]) */
                  PC = hl[selr];
                  break;
                case 0xEA:      /* JP PE,nnnn */
                  JPC(z, TSTFLAG(FLAG_P) != 0);
                  break;
                case 0xEB:      /* EX de[selr],hl[selr] */
                  temp = hl[selr];
                  hl[selr] = de[selr];
                  de[selr] = temp;
                  break;
                case 0xEC:      /* CALL PE,nnnn */
                  CALLC(z, TSTFLAG(FLAG_P) != 0);
                  break;
                case 0xED:      /* ED prefix */
                  PC++;
                  op = getByte(PC-1);
                  opCycles += cycles_ed_opcode[op];
                  
                  switch (op) {
                    case 0x40:      /* IN B,(C) */
                      //temp = Input(lreg(bc[selr]));
                      temp = input(z, bc[selr]);
                      bc[selr] = sethreg(bc[selr], temp);
                      af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                      (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                      parity(temp);
                      break;
                    case 0x41:      /* OUT (C),B */
                      output(z, bc[selr], hreg(bc[selr]));
                      break;
                    case 0x42:      /* SBC hl[selr],bc[selr] */
                      hl[selr] &= 0xffff;
                      bc[selr] &= 0xffff;
                      sum = hl[selr] - bc[selr] - TSTFLAG(FLAG_C);
                      cbits = (hl[selr] ^ bc[selr] ^ sum) >> 8;
                      hl[selr] = sum;
                      af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                      (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                      (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                      (cbits & 0x10) | 2 | ((cbits >> 8) & 1);
                      break;
                      case 0x43:      /* LD (nnnn),bc[selr] */
                        temp = getWord(PC);
                        putWord(temp, bc[selr]);
                        PC += 2;
                        break;
                      case 0x44:      /* NEG */
                        temp = hreg(af[sela]);
                        af[sela] = (-(af[sela] & 0xff00) & 0xff00);
                        af[sela] |= ((af[sela] >> 8) & 0xa8) | (((af[sela] & 0xff00) == 0 ? 1 : 0) << 6) |
                        (((temp & 0x0f) != 0 ? 1 : 0) << 4) | ((temp == 0x80 ? 1 : 0) << 2) |
                        2 | (temp != 0 ? 1 : 0);
                        break;
                      case 0x45:      /* RETN */
                        PC = POP(z);
                        //IFF1 |= IFF2;
                        
                        if (IFF1 == 0 && IFF2 != 0) {
                          IFF1 = 1;
                          if (irqState != 0) {
                            checkIrqLine(z);
                          }
                        } else if (IFF2 != 0) {
                          IFF1 = IFF2;
                        }
                        break;
                      case 0x46:      /* IM 0 */
                        IM = 0;
                        break;
                      case 0x47:      /* LD I,A */
                        IR = (IR & 0xff) | (af[sela] & ~0xff);
                        break;
                      case 0x48:      /* IN C,(C) */
                        //temp = Input(lreg(bc[selr]));
                        temp = input(z, bc[selr]);
                        bc[selr] = setlreg(bc[selr], temp);
                        af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                        (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                        parity(temp);
                        break;
                      case 0x49:      /* OUT (C),C */
                        output(z, bc[selr], lreg(bc[selr]));
                        break;
                      case 0x4A:      /* ADC hl[selr],bc[selr] */
                        hl[selr] &= 0xffff;
                        bc[selr] &= 0xffff;
                        sum = hl[selr] + bc[selr] + TSTFLAG(FLAG_C);
                        cbits = (hl[selr] ^ bc[selr] ^ sum) >> 8;
                        hl[selr] = sum;
                        af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                        (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                        (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                        (cbits & 0x10) | ((cbits >> 8) & 1);
                        break;
                        case 0x4B:      /* LD bc[selr],(nnnn) */
                          temp = getWord(PC);
                          bc[selr] = getWord(temp);
                          PC += 2;
                          break;
                        case 0x4D:      /* RETI */
                          PC = POP(z);
                          IFF1 |= IFF2;
                          break;
                        case 0x4F:      /* LD R,A */
                          IR = (IR & ~0xff) | ((af[sela] >> 8) & 0xff);
                          break;
                        case 0x50:      /* IN D,(C) */
                          temp = input(z, bc[selr]);
                          de[selr] = sethreg(de[selr], temp);
                          af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                          (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                          parity(temp);
                          break;
                        case 0x51:      /* OUT (C),D */
                          output(z, bc[selr], hreg(de[selr]));
                          break;
                        case 0x52:      /* SBC hl[selr],de[selr] */
                          hl[selr] &= 0xffff;
                          de[selr] &= 0xffff;
                          sum = hl[selr] - de[selr] - TSTFLAG(FLAG_C);
                          cbits = (hl[selr] ^ de[selr] ^ sum) >> 8;
                          hl[selr] = sum;
                          af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                          (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                          (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                          (cbits & 0x10) | 2 | ((cbits >> 8) & 1);
                          break;
                          case 0x53:      /* LD (nnnn),de[selr] */
                            temp = getWord(PC);
                            putWord(temp, de[selr]);
                            PC += 2;
                            break;
                          case 0x56:      /* IM 1 */
                            IM = 1;
                            break;
                          case 0x57:      /* LD A,I */
                            af[sela] = (af[sela] & 0x29) | (IR & ~255) | ((IR >> 8) & 0x80) | (((IR & ~255) == 0 ? 1 : 0) << 6) | (IFF2 << 2);
                            break;
                          case 0x58:      /* IN E,(C) */
                            //temp = Input(lreg(bc[selr]));
                            temp = input(z, bc[selr]);
                            de[selr] = setlreg(de[selr], temp);
                            af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                            (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                            parity(temp);
                            break;
                          case 0x59:      /* OUT (C),E */
                            output(z, bc[selr], lreg(de[selr]));
                            break;
                          case 0x5A:      /* ADC hl[selr],de[selr] */
                            hl[selr] &= 0xffff;
                            de[selr] &= 0xffff;
                            sum = hl[selr] + de[selr] + TSTFLAG(FLAG_C);
                            cbits = (hl[selr] ^ de[selr] ^ sum) >> 8;
                            hl[selr] = sum;
                            af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                            (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                            (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                            (cbits & 0x10) | ((cbits >> 8) & 1);
                            break;
                            case 0x5B:      /* LD de[selr],(nnnn) */
                              temp = getWord(PC);
                              de[selr] = getWord(temp);
                              PC += 2;
                              break;
                            case 0x5E:      /* IM 2 */
                              IM = 2;
                              break;
                            case 0x5F:      /* LD A,R */
                              af[sela] = (af[sela] & 0x29) | ((IR & 255) << 8) | (IR & 0x80) | (((IR & 255) == 0 ? 1 : 0) << 6) | (IFF2 << 2);
                              break;
                            case 0x60:      /* IN H,(C) */
                              //temp = Input(lreg(bc[selr]));
                              temp = input(z, bc[selr]);
                              hl[selr] = sethreg(hl[selr], temp);
                              af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                              (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                              parity(temp);
                              break;
                            case 0x61:      /* OUT (C),H */
                              output(z, bc[selr], hreg(hl[selr]));
                              break;
                            case 0x62:      /* SBC hl[selr],hl[selr] */
                              hl[selr] &= 0xffff;
                              sum = hl[selr] - hl[selr] - TSTFLAG(FLAG_C);
                              cbits = (hl[selr] ^ hl[selr] ^ sum) >> 8;
                              hl[selr] = sum;
                              af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                              (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                              (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                              (cbits & 0x10) | 2 | ((cbits >> 8) & 1);
                              break;
                              case 0x63:      /* LD (nnnn),hl[selr] */
                                temp = getWord(PC);
                                putWord(temp, hl[selr]);
                                PC += 2;
                                break;
                              case 0x67:      /* RRD */
                                temp = getByte(hl[selr]);
                                acu = hreg(af[sela]);
                                putByte(hl[selr], hdig(temp) | (ldig(acu) << 4));
                                acu = (acu & 0xf0) | ldig(temp);
                                af[sela] = (acu << 8) | (acu & 0xa8) | (((acu & 0xff) == 0 ? 1 : 0) << 6) |
                                parity(acu) | (af[sela] & 1);
                                break;
                              case 0x68:      /* IN L,(C) */
                                //temp = Input(lreg(bc[selr]));
                                temp = input(z, bc[selr]);
                                hl[selr] = setlreg(hl[selr], temp);
                                af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                                (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                                parity(temp);
                                break;
                              case 0x69:      /* OUT (C),L */
                                output(z, bc[selr], lreg(hl[selr]));
                                break;
                              case 0x6A:      /* ADC hl[selr],hl[selr] */
                                hl[selr] &= 0xffff;
                                sum = hl[selr] + hl[selr] + TSTFLAG(FLAG_C);
                                cbits = (hl[selr] ^ hl[selr] ^ sum) >> 8;
                                hl[selr] = sum;
                                af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                                (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                                (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                                (cbits & 0x10) | ((cbits >> 8) & 1);
                                break;
                                case 0x6B:      /* LD hl[selr],(nnnn) */
                                  temp = getWord(PC);
                                  hl[selr] = getWord(temp);
                                  PC += 2;
                                  break;
                                case 0x6F:      /* RLD */
                                  temp = getByte(hl[selr]);
                                  acu = hreg(af[sela]);
                                  putByte(hl[selr], (ldig(temp) << 4) | ldig(acu));
                                  acu = (acu & 0xf0) | hdig(temp);
                                  af[sela] = (acu << 8) | (acu & 0xa8) | (((acu & 0xff) == 0 ? 1 : 0) << 6) |
                                  parity(acu) | (af[sela] & 1);
                                  break;
                                case 0x70:      /* IN (C) */
                                  //temp = Input(lreg(bc[selr]));
                                  temp = input(z, bc[selr]);
                                  temp = setlreg(temp, temp);
                                  af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                                  (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                                  parity(temp);
                                  break;
                                case 0x71:      /* OUT (C),0 */
                                  output(z, bc[selr], 0);
                                  break;
                                case 0x72:      /* SBC hl[selr],SP */
                                  hl[selr] &= 0xffff;
                                  SP &= 0xffff;
                                  sum = hl[selr] - SP - TSTFLAG(FLAG_C);
                                  cbits = (hl[selr] ^ SP ^ sum) >> 8;
                                  hl[selr] = sum;
                                  af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                                  (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                                  (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                                  (cbits & 0x10) | 2 | ((cbits >> 8) & 1);
                                  break;
                                  case 0x73:      /* LD (nnnn),SP */
                                    temp = getWord(PC);
                                    putWord(temp, SP);
                                    PC += 2;
                                    break;
                                  case 0x78:      /* IN A,(C) */
                                    //temp = Input(lreg(bc[selr]));
                                    temp = input(z, bc[selr]);
                                    af[sela] = sethreg(af[sela], temp);
                                    af[sela] = (af[sela] & ~0xfe) | (temp & 0xa8) |
                                    (((temp & 0xff) == 0 ? 1 : 0) << 6) |
                                    parity(temp);
                                    break;
                                  case 0x79:      /* OUT (C),A */
                                    output(z, bc[selr], hreg(af[sela]));
                                    break;
                                  case 0x7A:      /* ADC hl[selr],SP */
                                    hl[selr] &= 0xffff;
                                    SP &= 0xffff;
                                    sum = hl[selr] + SP + TSTFLAG(FLAG_C);
                                    cbits = (hl[selr] ^ SP ^ sum) >> 8;
                                    hl[selr] = sum;
                                    af[sela] = (af[sela] & ~0xff) | ((sum >> 8) & 0xa8) |
                                    (((sum & 0xffff) == 0 ? 1 : 0) << 6) |
                                    (((cbits >> 6) ^ (cbits >> 5)) & 4) |
                                    (cbits & 0x10) | ((cbits >> 8) & 1);
                                    break;
                                    case 0x7B:      /* LD SP,(nnnn) */
                                      temp = getWord(PC);
                                      SP = getWord(temp);
                                      PC += 2;
                                      break;
                                    case 0xA0:      /* LDI */
                                      acu = getByte(hl[selr]); ++hl[selr];
                                      putByte(de[selr], acu); ++de[selr];
                                      acu += hreg(af[sela]);
                                      bc[selr]--;
                                      af[sela] = (af[sela] & ~0x3e) | (acu & 8) | ((acu & 2) << 4) |
                                      (((bc[selr] & 0xffff) != 0 ? 1 : 0) << 2);
                                      break;
                                    case 0xA1:      /* CPI */
                                      acu = hreg(af[sela]);
                                      temp = getByte(hl[selr]); ++hl[selr];
                                      sum = acu - temp;
                                      cbits = acu ^ temp ^ sum;
                                      bc[selr]--;
                                      af[sela] = (af[sela] & ~0xfe) | (sum & 0x80) | (((sum & 0xff) != 0 ? 0 : 1) << 6) |
                                      (((sum - ((cbits&16)>>4))&2) << 4) | (cbits & 16) |
                                      ((sum - ((cbits >> 4) & 1)) & 8) |
                                      ((bc[selr] & 0xffff) != 0 ? 1 : 0) << 2 | 2;
                                      if ((sum & 15) == 8 && (cbits & 16) != 0)
                                        af[sela] &= ~8;
                                      break;
                                    case 0xA2:      /* INI */
                                      //putByte(hl[selr], Input(lreg(bc[selr]))); ++hl[selr];

                                      //putByte(hl[selr], Input(bc[selr])); ++hl[selr];
                                      //SETFLAG(z, FLAG_N, 1);
                                      //SETFLAG(z, P, (--bc[selr] & 0xffff) != 0);

                                      //bc[selr] = sethreg(bc[selr], hreg(bc[selr]) - 1);
                                      //putByte(hl[selr], input(z, bc[selr])); ++hl[selr];
                                      //SETFLAG(z, FLAG_N, 1);
                                      //SETFLAG(z, FLAG_Z, hreg(bc[selr]) == 0 ? 1 : 0);
                                      
                                      putByte(hl[selr], input(z, bc[selr])); ++hl[selr];
                                      SETFLAG(z, FLAG_N, 1);
                                      
                                      bc[selr] = sethreg(bc[selr], hreg(bc[selr]) - 1);
                                      SETFLAG(z, FLAG_P, (hreg(bc[selr]) & 0xff) != 0 ? 1 : 0);
                                      
                                      //SETFLAG(z, FLAG_P, (--bc[selr] & 0xffff) != 0 ? 1 : 0);
                                      break;
                                    case 0xA3:      /* OUTI */
                                      output(z, bc[selr], getByte(hl[selr])); ++hl[selr];
                                      SETFLAG(z, FLAG_N, 1);
                                      bc[selr] = sethreg(bc[selr], hreg(bc[selr]) - 1);
                                      SETFLAG(z, FLAG_Z, hreg(bc[selr]) == 0 ? 1 : 0);
                                      break;
                                    case 0xA8:      /* LDD */
                                      acu = getByte(hl[selr]); --hl[selr];
                                      putByte(de[selr], acu); --de[selr];
                                      acu += hreg(af[sela]);
                                      af[sela] = (af[sela] & ~0x3e) | (acu & 8) | ((acu & 2) << 4) |
                                      (((--bc[selr] & 0xffff) != 0 ? 1 : 0) << 2);
                                      break;
                                    case 0xA9:      /* CPD */
                                      acu = hreg(af[sela]);
                                      temp = getByte(hl[selr]); --hl[selr];
                                      sum = acu - temp;
                                      cbits = acu ^ temp ^ sum;
                                      af[sela] = (af[sela] & ~0xfe) | (sum & 0x80) | (((sum & 0xff) != 0 ? 0 : 1) << 6) |
                                      (((sum - ((cbits&16)>>4))&2) << 4) | (cbits & 16) |
                                      ((sum - ((cbits >> 4) & 1)) & 8) |
                                      ((--bc[selr] & 0xffff) != 0 ? 1 : 0) << 2 | 2;
                                      if ((sum & 15) == 8 && (cbits & 16) != 0)
                                        af[sela] &= ~8;
                                      break;
                                    case 0xAA:      /* IND */
                                      //putByte(hl[selr], Input(lreg(bc[selr]))); --hl[selr];

                                      //putByte(hl[selr], Input(bc[selr])); --hl[selr];
                                      //SETFLAG(z, FLAG_N, 1);
                                      //bc[selr] = sethreg(bc[selr], lreg(bc[selr]) - 1);
                                      //SETFLAG(z, Z, lreg(bc[selr]) == 0);

                                      //bc[selr] = sethreg(bc[selr], hreg(bc[selr]) - 1);
                                      //putByte(hl[selr], input(z, bc[selr])); --hl[selr];
                                      //SETFLAG(z, FLAG_N, 1);
                                      //SETFLAG(z, FLAG_Z, hreg(bc[selr]) == 0 ? 1 : 0);
                                      
                                      putByte(hl[selr], input(z, bc[selr])); --hl[selr];
                                      SETFLAG(z, FLAG_N, 1);
                                      bc[selr] = sethreg(bc[selr], lreg(bc[selr]) - 1);
                                      SETFLAG(z, FLAG_Z, lreg(bc[selr]) == 0 ? 1 : 0);

                                      break;
                                    case 0xAB:      /* OUTD */
                                      output(z, bc[selr], getByte(hl[selr])); --hl[selr];
                                      SETFLAG(z, FLAG_N, 1);
                                      bc[selr] = sethreg(bc[selr], lreg(bc[selr]) - 1);
                                      SETFLAG(z, FLAG_Z, lreg(bc[selr]) == 0 ? 1 : 0);
                                      break;
                                    case 0xB0:      /* LDIR */
                                      acu = hreg(af[sela]);
                                      bc[selr] &= 0xffff;
                                      do {
                                        acu = getByte(hl[selr]); ++hl[selr];
                                        putByte(de[selr], acu); ++de[selr];
                                        bc[selr]--;
                                      } while (bc[selr] != 0);
                                      acu += hreg(af[sela]);
                                      af[sela] = (af[sela] & ~0x3e) | (acu & 8) | ((acu & 2) << 4);
                                      break;
                                    case 0xB1:      /* CPIR */
                                      acu = hreg(af[sela]);
                                      bc[selr] &= 0xffff;
                                      do {
                                        temp = getByte(hl[selr]); ++hl[selr];
                                        op = --bc[selr] != 0 ? 1 : 0;
                                        sum = acu - temp;
                                      } while (op != 0 && sum != 0);
                                      cbits = acu ^ temp ^ sum;
                                      af[sela] = (af[sela] & ~0xfe) | (sum & 0x80) | (((sum & 0xff) != 0 ? 0 : 1) << 6) |
                                      (((sum - ((cbits&16)>>4))&2) << 4) |
                                      (cbits & 16) | ((sum - ((cbits >> 4) & 1)) & 8) |
                                      op << 2 | 2;
                                      if ((sum & 15) == 8 && (cbits & 16) != 0)
                                        af[sela] &= ~8;
                                      break;
                                    case 0xB2:      /* INIR */
/*
                                      temp = hreg(bc[selr]);
                                      do {
                                        //putByte(hl[selr], Input(lreg(bc[selr]))); ++hl[selr];
                                        putByte(hl[selr], input(z, bc[selr])); ++hl[selr];
                                      } while (--temp != 0);
                                      bc[selr] = sethreg(bc[selr], 0);
                                      SETFLAG(z, FLAG_N, 1);
                                      SETFLAG(z, FLAG_Z, 1);
*/
                                      b = hreg(bc[selr]) >> 8;
                                      do {
                                        putByte(hl[selr], input(z, bc[selr])); ++hl[selr];
                                        b--;
                                      } while (b);
                                      bc[selr] = sethreg(bc[selr], 0);
                                      SETFLAG(z, FLAG_N, 1);
                                      SETFLAG(z, FLAG_Z, 1);
                                      break;
                                    case 0xB3:      /* OTIR */
                                      temp = hreg(bc[selr]);
                                      do {
                                        output(z, bc[selr], getByte(hl[selr])); ++hl[selr];
                                      } while (--temp != 0);
                                      bc[selr] = sethreg(bc[selr], 0);
                                      SETFLAG(z, FLAG_N, 1);
                                      SETFLAG(z, FLAG_Z, 1);
                                      break;
                                    case 0xB8:      /* LDDR */
                                      bc[selr] &= 0xffff;
                                      do {
                                        acu = getByte(hl[selr]); --hl[selr];
                                        putByte(de[selr], acu); --de[selr];
                                      } while (--bc[selr] != 0);
                                      acu += hreg(af[sela]);
                                      af[sela] = (af[sela] & ~0x3e) | (acu & 8) | ((acu & 2) << 4);
                                      break;
                                    case 0xB9:      /* CPDR */
                                      acu = hreg(af[sela]);
                                      bc[selr] &= 0xffff;
                                      do {
                                        temp = getByte(hl[selr]); --hl[selr];
                                        op = --bc[selr] != 0 ? 1 : 0;
                                        sum = acu - temp;
                                      } while (op != 0 && sum != 0);
                                      cbits = acu ^ temp ^ sum;
                                      af[sela] = (af[sela] & ~0xfe) | (sum & 0x80) | (((sum & 0xff) != 0 ? 0 : 1) << 6) |
                                      (((sum - ((cbits&16)>>4))&2) << 4) |
                                      (cbits & 16) | ((sum - ((cbits >> 4) & 1)) & 8) |
                                      op << 2 | 2;
                                      if ((sum & 15) == 8 && (cbits & 16) != 0)
                                        af[sela] &= ~8;
                                      break;
                                    case 0xBA:      /* INDR */
                                      temp = hreg(bc[selr]);
                                      do {
                                        //putByte(hl[selr], Input(lreg(bc[selr]))); --hl[selr];
                                        putByte(hl[selr], input(z, bc[selr])); --hl[selr];
                                      } while (--temp != 0);
                                      bc[selr] = sethreg(bc[selr], 0);
                                      SETFLAG(z, FLAG_N, 1);
                                      SETFLAG(z, FLAG_Z, 1);
                                      break;
                                    case 0xBB:      /* OTDR */
                                      temp = hreg(bc[selr]);
                                      do {
                                        output(z, bc[selr], getByte(hl[selr])); --hl[selr];
                                      } while (--temp != 0);
                                      bc[selr] = sethreg(bc[selr], 0);
                                      SETFLAG(z, FLAG_N, 1);
                                      SETFLAG(z, FLAG_Z, 1);
                                      break;
                                    default: if (0x40 <= op && op <= 0x7f) PC--;    /* ignore ED */
                  }
                  break;
                case 0xEE:      /* XOR nn */
                  sum = ((af[sela] >> 8) ^ getByte(PC)) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  ++PC;
                  break;
                case 0xEF:      /* RST 28H */
                  PUSH(z, PC); PC = 0x28;
                  break;
                case 0xF0:      /* RET P */
                  if (TSTFLAG(FLAG_S) == 0) PC = POP(z);
                  break;
                case 0xF1:      /* POP af[sela] */
                  af[sela] = POP(z);
                  break;
                case 0xF2:      /* JP P,nnnn */
                  JPC(z, TSTFLAG(FLAG_S) == 0);
                  break;
                case 0xF3:      /* DI */
                  IFF1 = IFF2 = 0;
                  break;
                case 0xF4:      /* CALL P,nnnn */
                  CALLC(z, TSTFLAG(FLAG_S) == 0);
                  break;
                case 0xF5:      /* PUSH af[sela] */
                  PUSH(z, af[sela]);
                  break;
                case 0xF6:      /* OR nn */
                  sum = ((af[sela] >> 8) | getByte(PC)) & 0xff;
                  af[sela] = (sum << 8) | (sum & 0xa8) | ((sum == 0 ? 1 : 0) << 6) | parity(sum);
                  ++PC;
                  break;
                case 0xF7:      /* RST 30H */
                  PUSH(z, PC); PC = 0x30;
                  break;
                case 0xF8:      /* RET M */
                  if (TSTFLAG(FLAG_S) != 0) PC = POP(z);
                  break;
                case 0xF9:      /* LD SP,hl[selr] */
                  SP = hl[selr];
                  break;
                case 0xFA:      /* JP M,nnnn */
                  JPC(z, TSTFLAG(FLAG_S) != 0);
                  break;
                case 0xFB:      /* EI */
                  IFF1 = IFF2 = 1;
                  break;
                case 0xFC:      /* CALL M,nnnn */
                  CALLC(z, TSTFLAG(FLAG_S) != 0);
                  break;
                case 0xFD:      /* FD prefix */
                  IY = dfdPrefix(z, IY);
                  break;
                case 0xFE:      /* CP nn */
                  temp = getByte(PC);
                  af[sela] = (af[sela] & ~0x28) | (temp & 0x28);
                  acu = hreg(af[sela]);
                  sum = acu - temp;
                  cbits = acu ^ temp ^ sum;
                  af[sela] = (af[sela] & ~0xff) | (sum & 0x80) |
                  (((sum & 0xff) == 0 ? 1 : 0) << 6) | (temp & 0x28) |
                  (((cbits >> 6) ^ (cbits >> 5)) & 4) | 2 |
                  (cbits & 0x10) | ((cbits >> 8) & 1);
                  //System.out.printf("acu=%02X, temp=%02X, sum=%02X, cond=%d, Z=%d\n", acu, temp, sum, ((sum & 0xff) == 0 ? 1 : 0), TSTFLAG(FLAG_Z));
                  ++PC;
                  break;
                case 0xFF:      /* RST 38H */
                  PUSH(z, PC); PC = 0x38;
  }
}
