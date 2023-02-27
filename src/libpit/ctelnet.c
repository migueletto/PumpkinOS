#include "sys.h"
#include "pit_io.h"
#include "filter.h"
#include "ctelnet.h"
#include "debug.h"
#include "xalloc.h"

#define MAX_BUF   256
#define MAX_VALUE 64

// WILL: sender suggests it would like to use an available option
// WONT: sender informs the receiver it will not use that option
// DO  : sender instructs the receiver to use an available option
// DONT: sender instructs the receiver not to use that option

// Sender:WILL  Recipient:DO   -> sender wants to enable an option, and the receiver agrees to that
// Sender:WILL  Recipient:DONT -> sender wants to enable an option, and the receiver does not agree
// Sender:DO    Recipient:WILL -> sender wants the receiver to enable an option, and the receiver agrees
// Sender:DO    Recipient:WONT -> sender wants the receiver to enable an option, and the receiver does not agree
// Sender:WONT  Recipient:DONT -> sender wants not to use and option, and the receiver confirms to that
// Sender:DONT  Recipient:WONT -> sender wants the receiver not to use and option, and the receiver confirms to that

#define TELNET_xEOF     236
#define TELNET_SUSP     237
#define TELNET_ABORT    238
#define TELNET_EOR      239
#define TELNET_SE       240
#define TELNET_NOP      241
#define TELNET_DM       242
#define TELNET_BRK      243
#define TELNET_IP       244
#define TELNET_AO       245
#define TELNET_AYT      246
#define TELNET_EC       247
#define TELNET_EL       248
#define TELNET_GA       249
#define TELNET_SB       250
#define TELNET_WILL     251
#define TELNET_WONT     252
#define TELNET_DO       253
#define TELNET_DONT     254
#define TELNET_IAC      255

#define TELNET_SEND     1
#define TELNET_IS       0

#define TELOPT_BINARY   0   // RFC 856
#define TELOPT_ECHO     1   // RFC 857
#define TELOPT_RECONN   2   // reconnection
#define TELOPT_SGA      3   // suppress go ahead RFC 858
#define TELOPT_AMSN     4   // approx message size negotiation
#define TELOPT_STATUS   5   // RFC 859
#define TELOPT_TM       6   // timing mark RFC 860
#define TELOPT_RCTE     7   // remote controlled transmssion and echoing RFC 726
#define TELOPT_OLW      8   // output line width
#define TELOPT_OPS      9   // output page size
#define TELOPT_OCRD     10  // output carriage-return disposition RFC 652
#define TELOPT_OHTS     11  // output horizontal tab stops RFC 653
#define TELOPT_OHTD     12  // output horizontal tab disposition RFC 654
#define TELOPT_OFFD     13  // output formfeed disposition RFC 655
#define TELOPT_OVTS     14  // output vertical tab stops RFC 656
#define TELOPT_OVTD     15  // output vertical tab disposition RFC 657
#define TELOPT_OLFD     16  // output linefeed disposition RFC 658
#define TELOPT_EXASCII  17  // extended ASCII RFC 698
#define TELOPT_LOGOUT   18  // logout RFC 727
#define TELOPT_BMACRO   19  // byte macro RFC 735
#define TELOPT_DET      20  // data entry terminal RFC 1043
#define TELOPT_SUPDUP   21  // RFC 736
#define TELOPT_SUPDUPO  22  // RFC 749
#define TELOPT_SENDLOC  23  // send location RFC 779
#define TELOPT_TTYPE    24  // terminal type RFC 930
#define TELOPT_EOR      25  // end of record RFC 885
#define TELOPT_TACACS   26  // TACACS user id RFC 927
#define TELOPT_OM       27  // output marking RFC 933
#define TELOPT_TLN      28  // terminal location number RFC 946
#define TELOPT_TN3270   29  // TN3270 regime RFC 1041
#define TELOPT_X3PAD    30  // X.3 pad RFC 1053
#define TELOPT_NAWS     31  // negotiate abour window size RFC 1073
#define TELOPT_TSPEED   32  // terminal speed RFC 1079
#define TELOPT_RFLOWC   33  // remote flow control RFC 1372
#define TELOPT_LINEMODE 34  // linemode RFC 1184
#define TELOPT_XDISPLOC 35  // X display location RFC 1096
#define TELOPT_ENV      36  // environment RFC 1408
#define TELOPT_AUTHOPT  37  // authentication option RFC 2941
#define TELOPT_ENCOPT   38  // encryption option RFC 2946
#define TELOPT_NEWENV   39  // new environment RFC 1572
#define TELOPT_TN3270E  40  // TN3270E RFC 2355
#define TELOPT_XAUTH    41  // XAUTH
#define TELOPT_CHARSET  42  // charset RFC 2066
#define TELOPT_RSP      43  // remote serial port
#define TELOPT_CPCO     44  // COM port control option RFC 2217
#define TELOPT_SLE      45  // suppress local echo
#define TELOPT_STARTTLS 46  // start TLS
#define TELOPT_KERMIT   47  // KERMIT
#define TELOPT_SENDURL  48  // send URL
#define TELOPT_FWDX     49  // forward X

#define TELOPT_LAST     TELOPT_FWDX
#define UNKNOWN         "UNKNOWN"

static const char *telnet_option[TELOPT_LAST+1] = {
  "BINARY",
  "ECHO",
  "RECONN",
  "SGA",
  "AMSN",
  "STATUS",
  "TM",
  "RCTE",
  "OLW",
  "OPS",
  "OCRD",
  "OHTS",
  "OHTD",
  "OFFD",
  "OVTS",
  "OVTD",
  "OLFD",
  "EXASCII",
  "LOGOUT",
  "BMACRO",
  "DET",
  "SUPDUP",
  "SUPDUPO",
  "SENDLOC",
  "TTYPE",
  "EOR",
  "TACACS",
  "OM",
  "TLN",
  "TN3270",
  "X3PAD",
  "NAWS",
  "TSPEED",
  "RFLOWC",
  "LINEMODE",
  "XDISPLOC",
  "ENV",
  "AUTHOPT",
  "ENCOPT",
  "NEWENV",
  "TN3270E",
  "XAUTH",
  "CHARSET",
  "RSP",
  "CPCO",
  "SLE",
  "STARTTLS",
  "KERMIT",
  "SENDURL",
  "FWDX"
};

typedef struct {
  int telnet_s;
  int echo;
  int binary;
  int rows, cols;
  char term[MAX_BUF];
  char inbuf[MAX_BUF];
  int iptr, optr, len;
  uint8_t sb;
} telnet_t;

static void telnet_put(conn_filter_t *filter, uint8_t *buf, int n) {
  telnet_t *telnet = (telnet_t *)filter->data;
  uint8_t c, outbuf[256];
  char aux[32];
  int i, j, k;

  for (i = 0; i < n; i++) {
    c = buf[i];
    debug(DEBUG_TRACE, "TELNET", "< 0x%02X", c);
    switch (telnet->telnet_s) {
      case 1:
        if (c == TELNET_IAC) {
          telnet->telnet_s = 2;
          break;
        }
pass:
        if (telnet->len < MAX_BUF) {
          telnet->len++;
          telnet->inbuf[telnet->iptr++] = c;
          if (telnet->iptr == MAX_BUF) {
            telnet->iptr = 0;
          }
        }
        break;

      case 2:
        switch (c) {
          case TELNET_IAC:
            telnet->telnet_s = 1;
            goto pass;
          case TELNET_WILL:
            telnet->telnet_s = 5;
            break;
          case TELNET_WONT:
            telnet->telnet_s = 6;
            break;
          case TELNET_DO:
            telnet->telnet_s = 7;
            break;
          case TELNET_DONT:
            telnet->telnet_s = 3;
            break;
          case TELNET_SB:
            telnet->telnet_s = 4;
            break;
          case TELNET_SE:
            telnet->telnet_s = 1;
            break;
          case TELNET_EOR:
            debug(DEBUG_INFO, "TELNET", "< EOR");
            telnet->telnet_s = 1;
            break;
          case TELNET_GA:
            debug(DEBUG_INFO, "TELNET", "< GA");
            telnet->telnet_s = 1;
            break;
          default:
            telnet->telnet_s = 1;
            debug(DEBUG_INFO, "TELNET", "< TELNET %d", c);
            break;
        }
        break;

      case 7:	// DO
        debug(DEBUG_INFO, "TELNET", "< DO %s (%d)", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        switch (c) {
          case TELOPT_EOR:
            outbuf[1] = TELNET_WONT;
            break;
          case TELOPT_BINARY:
            telnet->binary = 1;
            outbuf[1] = TELNET_WILL;
            break;
          case TELOPT_ECHO:
            telnet->echo = 1;  // server is asking us to echo
            outbuf[1] = TELNET_WILL;
            break;
          case TELOPT_TTYPE:
            outbuf[1] = TELNET_WILL;
            break;
          case TELOPT_NAWS:
            outbuf[1] = TELNET_WILL;
            break;
          case TELOPT_TSPEED:
            outbuf[1] = TELNET_WILL;
            break;
          default:
            outbuf[1] = TELNET_WONT;
            break;
        }
        outbuf[0] = TELNET_IAC;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        debug(DEBUG_INFO, "TELNET", "> %s %s (%d)", outbuf[1] == TELNET_WILL ? "WILL" : "WONT", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        telnet->telnet_s = 1;

        switch (c) {
          case TELOPT_NAWS:
            j = 0;
            outbuf[j++] = TELNET_IAC;
            outbuf[j++] = TELNET_SB;
            outbuf[j++] = TELOPT_NAWS;
            outbuf[j++] = 0;
            outbuf[j++] = telnet->cols;
            outbuf[j++] = 0;
            outbuf[j++] = telnet->rows;
            outbuf[j++] = TELNET_IAC;
            outbuf[j++] = TELNET_SE;
            filter->next->write(filter->next, outbuf, j);
            debug(DEBUG_INFO, "TELNET", "> SB NAWS %d %d", telnet->cols, telnet->rows);
            break;
        }
        break;

      case 5:	// WILL
        debug(DEBUG_INFO, "TELNET", "< WILL %s (%d)", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        switch (c) {
          case TELOPT_ECHO:
            outbuf[1] = TELNET_DO;
            telnet->echo = 0; // server will echo, so we don't have to
            break;
          case TELOPT_SGA:
            outbuf[1] = TELNET_DO;
            break;
          case TELOPT_BINARY:
            outbuf[1] = TELNET_DO;
            telnet->binary = 1;
            break;
          default:
            outbuf[1] = TELNET_DONT;
            break;
        }

        outbuf[0] = TELNET_IAC;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        debug(DEBUG_INFO, "TELNET", "> %s %s (%d)", outbuf[1] == TELNET_DO ? "DO" : "DONT", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        telnet->telnet_s = 1;
        break;

      case 3:	// DONT
        debug(DEBUG_INFO, "TELNET", "< DONT %s (%d)", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        switch (c) {
          case TELOPT_ECHO:
            telnet->echo = 0; // server does not want us to echo
            break;
        }
        outbuf[0] = TELNET_IAC;
        outbuf[1] = TELNET_WONT;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        debug(DEBUG_INFO, "TELNET", "> WONT %s (%d)", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        telnet->telnet_s = 1;
        telnet->telnet_s = 1;
        break;

      case 6:	// WONT
        debug(DEBUG_INFO, "TELNET", "< WONT %s (%d)", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        switch (c) {
          case TELOPT_BINARY:
            telnet->binary = 0;
            break;
          case TELOPT_ECHO:
            telnet->echo = 1; // server will not echo, so we do
            break;
        }
        outbuf[0] = TELNET_IAC;
        outbuf[1] = TELNET_DONT;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        debug(DEBUG_INFO, "TELNET", "> DONT %s (%d)", c < TELOPT_LAST ? telnet_option[c] : UNKNOWN, c);
        telnet->telnet_s = 1;
        break;

      case 4:	// IAC SB type
        telnet->sb = c;
        telnet->telnet_s = 8;
        break;

      case 8:	// IAC SB type SEND
        if (c == TELNET_SEND) {
          telnet->telnet_s = 9;
        } else {
          telnet->telnet_s = 1;
        }
        break;

      case 9:	// IAC SB type SEND IAC
        if (c == TELNET_IAC) {
          telnet->telnet_s = 10;
        } else {
          telnet->telnet_s = 1;
        }
        break;

      case 10:	// IAC SB type SEND IAC SE
        switch (telnet->sb) {
          case TELOPT_TTYPE:
            debug(DEBUG_INFO, "TELNET", "< SB TTYPE SEND");
            j = 0;
            outbuf[j++] = TELNET_IAC;
            outbuf[j++] = TELNET_SB;
            outbuf[j++] = TELOPT_TTYPE;
            outbuf[j++] = TELNET_IS;
            for (k = 0; telnet->term[k]; k++) outbuf[j++] = telnet->term[k];
            outbuf[j++] = TELNET_IAC;
            outbuf[j++] = TELNET_SE;
            filter->next->write(filter->next, outbuf, j);
            debug(DEBUG_INFO, "TELNET", "> SB TTYPE IS \"%s\"", telnet->term);
            break;
          case TELOPT_TSPEED:
            debug(DEBUG_INFO, "TELNET", "< SB TSPEED SEND");
            snprintf(aux, sizeof(aux)-1, "%d,%d", 19200, 19200);
            j = 0;
            outbuf[j++] = TELNET_IAC;
            outbuf[j++] = TELNET_SB;
            outbuf[j++] = TELOPT_TSPEED;
            outbuf[j++] = TELNET_IS;
            for (k = 0; aux[k]; k++) outbuf[j++] = aux[k];
            outbuf[j++] = TELNET_IAC;
            outbuf[j++] = TELNET_SE;
            filter->next->write(filter->next, outbuf, j);
            debug(DEBUG_INFO, "TELNET", "> SB TSPEED IS \"%s\"", aux);
            break;
          default:
            debug(DEBUG_INFO, "TELNET", "< SB %d SEND", telnet->sb);
            break;
        }
        telnet->telnet_s = 1;
        break;
    }
  }
}

static int telnet_get(telnet_t *telnet, uint8_t *buf, int n) {
  int i;

  for (i = 0; i < n && i < telnet->len; i++) {
    buf[i] = telnet->inbuf[telnet->optr++];
    if (telnet->optr == MAX_BUF) {
      telnet->optr = 0;
    }
  }
  telnet->len -= i;

  return i;
}

static int telnet_filter_peek(conn_filter_t *filter, uint32_t us) {
  telnet_t *telnet = (telnet_t *)filter->data;
  uint8_t b;
  int r;

  for (;;) {
    if (telnet->len) {
      return 1;
    }
    r = filter->next->peek(filter->next, us);
    if (r == 0) return 0;
    if (r == -1) return -1;

    r = filter->next->read(filter->next, &b);
    if (r <= 0) return -1;
    if (b) telnet_put(filter, &b, 1);
  }

  return 0;
}

static int telnet_filter_read(conn_filter_t *filter, uint8_t *b) {
  telnet_t *telnet = (telnet_t *)filter->data;
  int r;

  for (;;) {
    if (telnet->len) {
      return telnet_get(telnet, b, 1);
    }
    r = filter->next->read(filter->next, b);
    if (r <= 0) return -1;
    if (*b) telnet_put(filter, b, 1);
  }

  return 0;
}

static int telnet_filter_write(conn_filter_t *filter, uint8_t *buf, int n) {
  telnet_t *telnet = (telnet_t *)filter->data;
  uint8_t iac;
  int i, k;

  iac = TELNET_IAC;

  for (i = 0, k = 0; i < n; i++) {
    if (buf[i] == iac) {
      if (filter->next->write(filter->next, &buf[k], i-k+1) == -1) return -1;
      if (filter->next->write(filter->next, &iac, 1) == -1) return -1;
      k = i+1;
    }
  }

  if (i > k) {
    if (filter->next->write(filter->next, &buf[k], i-k) == -1) return -1;
  }

  if (telnet->echo) {
    telnet_put(filter, buf, n);
  }

  return n;
}

static void telnet_opt(conn_filter_t *filter, uint8_t verb, uint8_t opt) {
  uint8_t outbuf[4];

  outbuf[0] = TELNET_IAC;
  outbuf[1] = verb;
  outbuf[2] = opt;
  filter->next->write(filter->next, outbuf, 3);
}

static telnet_t *telnet_init(char *term, int cols, int rows) {
  telnet_t *telnet;

  if ((telnet = xcalloc(1, sizeof(telnet_t))) != NULL) {
    telnet->telnet_s = 1;
    telnet->echo = 1;
    telnet->binary = 0;

    telnet->cols = cols;
    telnet->rows = rows;
    sys_strncpy(telnet->term, term, MAX_BUF-1);
  }

  return telnet;
}

conn_filter_t *telnet_client_filter(conn_filter_t *next, char *term, int cols, int rows) {
  conn_filter_t *filter;

  if ((filter = xcalloc(1, sizeof(conn_filter_t))) != NULL) {
    filter->peek = telnet_filter_peek;
    filter->read = telnet_filter_read;
    filter->write = telnet_filter_write;
    filter->next = next;
    filter->data = telnet_init(term, cols, rows);

    debug(DEBUG_INFO, "TELNET", "> WILL %s (%d)", telnet_option[TELOPT_TTYPE], TELOPT_TTYPE);
    telnet_opt(filter, TELNET_WILL, TELOPT_TTYPE);
    debug(DEBUG_INFO, "TELNET", "> WILL %s (%d)", telnet_option[TELOPT_NAWS], TELOPT_NAWS);
    telnet_opt(filter, TELNET_WILL, TELOPT_NAWS);
  }

  return filter;
}

void telnet_client_close(conn_filter_t *filter) {
  telnet_t *telnet;

  if (filter) {
    telnet = (telnet_t *)filter->data;
    if (telnet) xfree(telnet);
    xfree(filter);
  }
}

char telnet_client_state(conn_filter_t *filter) {
  telnet_t *telnet;
  char c = '?';

  if (filter) {
    telnet = (telnet_t *)filter->data;
    c = '0' + telnet->telnet_s;
  }

  return c;
}
