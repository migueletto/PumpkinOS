#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "pit_io.h"
#include "filter.h"
#include "telnet.h"
#include "sys.h"
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
#define TELOPT_SGA      3   // suppress go ahead RFC 858
#define TELOPT_STATUS   5   // RFC 859
#define TELOPT_TM       6   // timing mark RFC 860
#define TELOPT_TTYPE    24  // terminal type RFC 930
#define TELOPT_EOR      25  // end of record RFC 885
#define TELOPT_NAWS     31  // window size RFC 1073
#define TELOPT_TSPEED   32  // terminal speed RFC 1079
#define TELOPT_RFLOWC   33  // remote flow control RFC 1372
#define TELOPT_LINEMODE 34  // RFC 1184
#define TELOPT_ENV      36  // environment RFC 1408
#define TELOPT_NEWENV   39  // new environment RFC 1572

typedef struct {
  int telnet_s;
  int echo;
  uint8_t telnetSB;
  int telnetBinary;
  int ttype_sent;
  int rows, cols;
  char term[MAX_VALUE];
  uint8_t value[MAX_VALUE];
  char inbuf[MAX_BUF];
  int iptr, optr, len;
  int ivalue;
} telnet_t;

static void telnet_put(conn_filter_t *filter, uint8_t *buf, int n) {
  telnet_t *telnet = (telnet_t *)filter->data;
  uint8_t c, outbuf[64];
  int i;

  for (i = 0; i < n; i++) {
    c = buf[i];
    debug(DEBUG_TRACE, "TELNET", "client sent %d", c);
    switch (telnet->telnet_s) {
      case 1:
        if (c == TELNET_IAC) {
          telnet->telnet_s = 2;
          break;
        }
pass:
        switch (c) {
          case '\0':
            break;
          default:
            if (telnet->len < MAX_BUF) {
              telnet->len++;
              telnet->inbuf[telnet->iptr++] = c;
              if (telnet->iptr == MAX_BUF) {
                telnet->iptr = 0;
              }
            }
        }
        break;

      case 2:
        switch (c) {
          case TELNET_IAC:
            telnet->telnet_s = 1;
            goto pass;
          case TELNET_NOP:
          case TELNET_DM:
          case TELNET_BRK:
          case TELNET_IP:
          case TELNET_AO:
          case TELNET_AYT:
          case TELNET_GA:
          case TELNET_EC:
          case TELNET_EL:
            telnet->telnet_s = 1;
            debug(DEBUG_INFO, "TELNET", "< TELNET %d", c);
            break;
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
          default:
            telnet->telnet_s = 1;
            break;
        }
        break;

      case 7:	// DO
        switch (c) {
          case TELOPT_EOR:
            debug(DEBUG_INFO, "TELNET", "< DO EOR");
            outbuf[1] = TELNET_WONT;
            break;
          case TELOPT_BINARY:
            debug(DEBUG_INFO, "TELNET", "< DO BINARY");
            //telnet->telnetBinary = 1;
            //outbuf[1] = TELNET_WILL;
            outbuf[1] = TELNET_WONT;
            break;
          case TELOPT_ECHO:
            debug(DEBUG_INFO, "TELNET", "< DO ECHO");
            telnet->echo = 1;
            telnet->telnet_s = 1;
            continue;
            break;
          case TELOPT_TTYPE:
            debug(DEBUG_INFO, "TELNET", "< DO TTYPE");
            outbuf[1] = TELNET_WILL;
            break;
          case TELOPT_SGA:
            debug(DEBUG_INFO, "TELNET", "< DO SGA");
            outbuf[1] = TELNET_WILL;
            break;
          default:
            debug(DEBUG_INFO, "TELNET", "< DO %d", c);
            outbuf[1] = TELNET_WONT;
            break;
        }
        outbuf[0] = TELNET_IAC;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        telnet->telnet_s = 1;
        if (outbuf[1] == TELNET_WILL) {
          debug(DEBUG_INFO, "TELNET", "> WILL %d", c);
        } else {
          debug(DEBUG_INFO, "TELNET", "> WONT %d", c);
        }
        break;

      case 5:	// WILL
        switch (c) {
          case TELOPT_EOR:
            debug(DEBUG_INFO, "TELNET", "< WILL EOR");
            outbuf[1] = TELNET_DONT;
            break;
          case TELOPT_ECHO:
            debug(DEBUG_INFO, "TELNET", "< WILL ECHO");
            outbuf[1] = TELNET_DO;
            break;
          case TELOPT_BINARY:
            debug(DEBUG_INFO, "TELNET", "< WILL BINARY");
            //outbuf[1] = TELNET_DO;
            outbuf[1] = TELNET_DONT;
            break;
          case TELOPT_TTYPE:  // sender is willing to send terminal type information in a subsequent sub-negotiation
            debug(DEBUG_INFO, "TELNET", "< WILL TTYPE");
            if (telnet->ttype_sent) {
              outbuf[1] = TELNET_DONT;
            } else {
              outbuf[1] = TELNET_DO;
              telnet->ttype_sent = 1;
            }
            break;
          case TELOPT_NAWS:
            debug(DEBUG_INFO, "TELNET", "< WILL NAWS");
            outbuf[1] = TELNET_DO;
            break;
          case TELOPT_LINEMODE:
            debug(DEBUG_INFO, "TELNET", "< WILL LINEMODE");
            outbuf[1] = TELNET_DONT;
            break;
          default:
            debug(DEBUG_INFO, "TELNET", "< WILL %d", c);
            outbuf[1] = TELNET_DONT;
            break;
        }

        outbuf[0] = TELNET_IAC;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        telnet->telnet_s = 1;
        if (outbuf[1] == TELNET_DO) {
          debug(DEBUG_INFO, "TELNET", "> DO %d", c);
        } else {
          debug(DEBUG_INFO, "TELNET", "> DONT %d", c);
        }

        if (c == TELOPT_TTYPE) {
          outbuf[0] = TELNET_IAC;
          outbuf[1] = TELNET_SB;
          outbuf[2] = TELOPT_TTYPE;
          outbuf[3] = TELNET_SEND;
          outbuf[4] = TELNET_IAC;
          outbuf[5] = TELNET_SE;
          filter->next->write(filter->next, outbuf, 6);
          debug(DEBUG_INFO, "TELNET", "> SB TTYPE SEND");
        }
        break;

      case 3:	// DONT
        switch (c) {
          case TELOPT_ECHO:
            debug(DEBUG_INFO, "TELNET", "< DONT ECHO");
            debug(DEBUG_INFO, "TELNET", "echo off");
            telnet->echo = 0;
            break;
          default:
            debug(DEBUG_INFO, "TELNET", "< DONT %d", c);
            break;
        }
        telnet->telnet_s = 1;
        break;

      case 6:	// WONT
        switch (c) {
          case TELOPT_BINARY:
            debug(DEBUG_INFO, "TELNET", "< WONT BINARY");
            telnet->telnetBinary = 0;
            break;
          case TELOPT_ECHO:
            telnet->telnet_s = 1;
            continue;
            break;
          case TELOPT_EOR:
            debug(DEBUG_INFO, "TELNET", "< WONT EOR");
            break;
          default:
            debug(DEBUG_INFO, "TELNET", "< WONT %d", c);
            break;
        }
        outbuf[0] = TELNET_IAC;
        outbuf[1] = TELNET_DONT;
        outbuf[2] = c;
        filter->next->write(filter->next, outbuf, 3);
        telnet->telnet_s = 1;
        debug(DEBUG_INFO, "TELNET", "> DONT %d", c);
        break;

      case 4:	// SB
        switch (c) {
          case TELOPT_TTYPE:
            telnet->telnet_s = 8;
            break;
          case TELOPT_NAWS:
            telnet->ivalue = 0;
            telnet->telnet_s = 9;
            break;
          default:
            telnet->ivalue = 0;
            telnet->telnet_s = 9;
            break;
        }
        telnet->telnetSB = c;
        break;

      case 8:	// SB option IS
        switch (c) {
          case TELNET_IS:
            debug(DEBUG_INFO, "TELNET", "< SB %d IS", telnet->telnetSB);
            telnet->ivalue = 0;
            telnet->telnet_s = 9;
            break;
          case TELNET_SEND:
            debug(DEBUG_INFO, "TELNET", "< SB %d SEND", telnet->telnetSB);
            if (telnet->telnetSB == TELOPT_TTYPE) {
            }
            telnet->telnet_s = 1;
            break;
          default:
            debug(DEBUG_INFO, "TELNET", "< SB %d ??? %d", telnet->telnetSB, c);
            telnet->telnet_s = 1;
            break;
        }
        break;

      case 9:	// SB option IS value
        if (c == TELNET_IAC) {
          telnet->telnet_s = 10;
        } else {
          if (telnet->ivalue < MAX_VALUE-1) {
            telnet->value[telnet->ivalue++] = c;
          }
        }
        break;

      case 10:	// SB option IS value IAC
        switch (c) {
          case TELNET_IAC:
            if (telnet->ivalue < MAX_VALUE-1) {
              telnet->value[telnet->ivalue++] = c;
            }
            telnet->telnet_s = 9;
            break;
          case TELNET_SE:
            switch (telnet->telnetSB) {
              case TELOPT_TTYPE:
                telnet->value[telnet->ivalue] = 0;
                strncpy(telnet->term, (char *)telnet->value, MAX_VALUE-1);
                debug(DEBUG_INFO, "TELNET", "client terminal is \"%s\"", telnet->term);
                break;
              case TELOPT_NAWS:
                //telnet->cols = telnet->value[0] * 256 + telnet->value[1];
                //telnet->rows = telnet->value[2] * 256 + telnet->value[3];
                telnet->cols = telnet->value[0];
                telnet->rows = telnet->value[1];
                debug(DEBUG_INFO, "TELNET", "client window is %dx%d", telnet->cols, telnet->rows);
                break;
              default:
                debug(DEBUG_INFO, "TELNET", "client option %d is %d bytes", telnet->telnetSB, telnet->ivalue);
                break;
            }
            telnet->telnet_s = 1;
            break;
          default:
            debug(DEBUG_ERROR, "TELNET", "unexpected sequence SB %d IS value IAC %d", telnet->telnetSB, c);
            telnet->telnet_s = 1;
            break;
        }
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

/*
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > WILL ECHO
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < DO SGA
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > WILL 3
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL TTYPE
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DO 24
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > SB TTYPE SEND
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL NAWS
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DO 31
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL 32
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DONT 32
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL 33
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DONT 33
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL LINEMODE
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DONT 34
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL 39
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DONT 39
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < DO 5
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > WONT 5
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < WILL 35
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: > DONT 35
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: < DO ECHO
2019-03-14 22:47:31.974 I 03588 TELNETD  TELNET: echo on
2019-03-14 22:47:32.016 I 03588 TELNETD  TELNET: < SB 24 IS
2019-03-14 22:47:32.016 I 03588 TELNETD  TELNET: client terminal is "xterm-256color"
2019-03-14 22:47:32.016 I 03588 TELNETD  TELNET: client window is 190x51

WILL Indicates the desire to begin
     performing, or confirmation that
     you are now performing, the
     indicated option.

WONT Indicates the refusal to perform,
     or continue performing, the
     indicated option.

DO   Indicates the request that the
     other party perform, or
     confirmation that you are expecting
     the other party to perform, the
     indicated option.

DONT Indicates the demand that the
     other party stop performing,
     or confirmation that you are no
     longer expecting the other party
     to perform, the indicated option.
*/

static void telnet_opt(conn_filter_t *next, uint8_t verb, uint8_t opt) {
  uint8_t outbuf[4];

  outbuf[0] = TELNET_IAC;
  outbuf[1] = verb;
  outbuf[2] = opt;
  next->write(next, outbuf, 3);
  debug(DEBUG_INFO, "TELNET", "> %u %u", verb, opt);
}

void telnet_echo(conn_filter_t *filter, int echo) {
  if (echo) {
    telnet_opt(filter->next, TELNET_WILL, TELOPT_ECHO);
  } else {
    telnet_opt(filter->next, TELNET_WONT, TELOPT_ECHO);
  }
}

void telnet_linemode(conn_filter_t *filter, int linemode) {
  if (linemode) {
    telnet_opt(filter->next, TELNET_DO,   TELOPT_LINEMODE);
    telnet_opt(filter->next, TELNET_WONT, TELOPT_SGA);
  } else {
    telnet_opt(filter->next, TELNET_DONT, TELOPT_LINEMODE);
    telnet_opt(filter->next, TELNET_WILL, TELOPT_SGA);
  }
}

void telnet_naws(conn_filter_t *filter) {
  telnet_opt(filter->next, TELNET_DO, TELOPT_NAWS);
}

int telnet_term(conn_filter_t *filter, char *term, int n, int *cols, int *rows) {
  telnet_t *telnet = (telnet_t *)filter->data;

  *cols = telnet->cols;
  *rows = telnet->rows;
  strncpy(term, telnet->term, n);

  return 0;
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
    if (r < 0) return -1;
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
    if (r == 0) return 0;
    if (r < 0) return -1;
    if (*b) telnet_put(filter, b, 1);
  }

  return 0;
}

static int telnet_filter_write(conn_filter_t *filter, uint8_t *buf, int n) {
  uint8_t iac;
  int i, k;

  iac = TELNET_IAC;

  for (i = 0, k = 0; i < n; i++) {
    if (buf[i] == TELNET_IAC) {
      filter->next->write(filter->next, &buf[k], i-k+1);
      filter->next->write(filter->next, &iac, 1);
      k = i+1;
    }
  }

  if (i > k) {
    filter->next->write(filter->next, &buf[k], i-k);
  }

  return n;
}

static telnet_t *telnet_init(void) {
  telnet_t *telnet;

  if ((telnet = xcalloc(1, sizeof(telnet_t))) != NULL) {
    telnet->telnet_s = 1;
    telnet->telnetSB = 0;
    telnet->telnetBinary = 0;
  }

  return telnet;
}

conn_filter_t *telnet_filter(conn_filter_t *next) {
  conn_filter_t *filter;

  if ((filter = xcalloc(1, sizeof(conn_filter_t))) != NULL) {
    filter->data = telnet_init();
    filter->peek = telnet_filter_peek;
    filter->read = telnet_filter_read;
    filter->write = telnet_filter_write;
    filter->next = next;
  }

  return filter;
}

void telnet_close(conn_filter_t *filter) {
  telnet_t *telnet;

  if (filter) {
    telnet = (telnet_t *)filter->data;
    if (telnet) xfree(telnet);
    xfree(filter);
  }
}

#if 0
servidor telnet do supsnet:

#define TELNET_ECHO     1
#define TELNET_CHARMODE 3
#define TELNET_LINEMODE 34

    /* Envia opcao Telnet: nao suporta modo de linha */
    sendpacket[0] = TELNET_IAC;
    sendpacket[1] = TELNET_DONT;
    sendpacket[2] = TELNET_LINEMODE;
    send(nsock, sendpacket, 3, 0);
    debug(DEBUG_TELNET, "Local %s %u",
      telnet_cmd[TELNET_DONT - TELNET_BASE], TELNET_LINEMODE);

    /* Envia opcao Telnet: suporta modo de caracter */
    sendpacket[0] = TELNET_IAC;
    sendpacket[1] = TELNET_WILL;
    sendpacket[2] = TELNET_CHARMODE;
    send(nsock, sendpacket, 3, 0);
    debug(DEBUG_TELNET, "Local %s %u",
      telnet_cmd[TELNET_WILL - TELNET_BASE], TELNET_CHARMODE);

    /* Envia opcao Telnet: suporta echo de caracter */
    sendpacket[0] = TELNET_IAC;
    sendpacket[1] = TELNET_WILL;
    sendpacket[2] = TELNET_ECHO;
    send(nsock, sendpacket, 3, 0);
    debug(DEBUG_TELNET, "Local %s %u",
      telnet_cmd[TELNET_WILL - TELNET_BASE], TELNET_ECHO);


          case TELNET_WILL:
            /* Rejeita opcoes Telnet propostas pelo cliente */
            sendpacket[0] = TELNET_IAC;
            sendpacket[1] = TELNET_WONT;
            sendpacket[2] = c;
            send(nsock, sendpacket, 3, 0);
            debug(DEBUG_TELNET, "Local %s %u",
              telnet_cmd[TELNET_WONT - TELNET_BASE], c);
            break;
          case TELNET_DO:
            /* Rejeita opcoes Telnet diferentes de ECHO e CHARMOE */
            if (c == TELNET_ECHO || c == TELNET_CHARMODE)
              break;
            sendpacket[0] = TELNET_IAC;
            sendpacket[1] = TELNET_DONT;
            sendpacket[2] = c;
            send(nsock, sendpacket, 3, 0);
            debug(DEBUG_TELNET, "Local %s %u",
              telnet_cmd[TELNET_DONT - TELNET_BASE], c);
            break;

#endif
