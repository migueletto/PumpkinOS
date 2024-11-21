typedef struct {
/*
  int8_t   d_reserved[21]; // Reserved for GEMDOS
  uint8_t  d_attrib;       // File attributes
  uint16_t d_time;         // Time
  uint16_t d_date;         // Date
  uint32_t d_length;       // File length
  int8_t   d_fname[14];    // Filename
*/
  uint8_t dta[256];
} DTA;

struct g_sigaction {
  int a;
};

struct g_sockaddr {
  int a;
};

struct g_iovec {
  int a;
};

struct g_timeval {
  int a;
};

struct g_msghdr {
  int a;
};

typedef void *SHARED_LIB;
typedef void *SLB_EXEC;
typedef void *DISKINFO;
typedef void *DOSTIME;
typedef void *XATTR;
typedef void *STAT;
typedef void *POLLFD;
typedef void *COOKIE;
typedef void *timezone;
typedef void *MPB;
typedef void *BPB;
typedef void *MOUSE;
typedef void *IOREC;
typedef void *KEYTAB;
typedef void *KBDVBASE;
typedef void *META_DRVINFO;
typedef void *META_INFO_1;
typedef void *CD_TOC_ENTRY;
typedef void *CD_DISC_INFO;
typedef void *DSPBLOCK;
typedef void *PBDEF;
