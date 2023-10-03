#ifndef _M6809PRIV_H
#define _M6809PRIV_H

#define M6809_IRQ_LINE	0	// IRQ line number
#define M6809_FIRQ_LINE 1	// FIRQ line number
#define M6809_NMI_LINE	2	// NMI line number

#define M6809_RDMEM(Addr)	m6809_getb(m6809,Addr)
#define M6809_WRMEM(Addr,Value)	m6809_putb(m6809,Addr,Value)
#define M6809_RDOP(Addr)	M6809_RDMEM(Addr)
#define M6809_RDOP_ARG(Addr)	M6809_RDOP(Addr)

#define ILLEGAL0  0
#define ILLEGAL1  1
#define ILLEGAL2  2

#define LSB_FIRST 1

typedef union {
#ifdef LSB_FIRST
  struct { uint8_t l,h,h2,h3; } b;
  struct { uint16_t l,h; } w;
#else
  struct { uint8_t h3,h2,h,l; } b;
  struct { uint16_t h,l; } w;
#endif
  uint32_t d;
} PAIR;

enum { CLEAR_LINE=0, ASSERT_LINE};

#define CC_C    0x01	// Carry
#define CC_V    0x02	// Overflow
#define CC_Z    0x04	// Zero
#define CC_N    0x08	// Negative
#define CC_II   0x10	// Inhibit IRQ
#define CC_H    0x20	// Half (auxiliary) carry
#define CC_IF   0x40	// Inhibit FIRQ
#define CC_E    0x80	// entire state pushed

#define pPPC    m6809->ppc
#define pPC 	m6809->pc
#define pU	m6809->u
#define pS	m6809->s
#define pX	m6809->x
#define pY	m6809->y
#define pD	m6809->d

#define	PPC	m6809->ppc.w.l
#define PC  	m6809->pc.w.l
#define PCD 	m6809->pc.d
#define U	m6809->u.w.l
#define UD	m6809->u.d
#define S	m6809->s.w.l
#define SD	m6809->s.d
#define X	m6809->x.w.l
#define XD	m6809->x.d
#define Y	m6809->y.w.l
#define YD	m6809->y.d
#define D   	m6809->d.w.l
#define A   	m6809->d.b.h
#define B	m6809->d.b.l
#define DP	m6809->dp.b.h
#define DPD 	m6809->dp.d
#define CC  	m6809->cc

#define mEA	m6809->ea
#define EAB	m6809->ea.b.l
#define EA	m6809->ea.w.l
#define EAD	m6809->ea.d

#define m6809_ICount m6809->count

#define M6809_CWAI	8	// set when CWAI is waiting for an interrupt
#define M6809_SYNC	16	// set when SYNC is waiting for an interrupt
#define M6809_LDS	32	// set when LDS occured at least once

#define RM(Addr)        M6809_RDMEM(Addr)
#define WM(Addr,Value)  M6809_WRMEM(Addr,Value)
#define ROP(Addr)       M6809_RDOP(Addr)
#define ROP_ARG(Addr)   M6809_RDOP_ARG(Addr)

#define IMMBYTE(b)      b = ROP_ARG(PC); PC++
#define IMMWORD(w)      w.d = (ROP_ARG(PC)<<8) | ROP_ARG((PC+1)&0xffff); PC+=2

#define PUSHBYTE(b)     --S; WM(SD,b)
#define PUSHWORD(w)     --S; WM(SD,w.b.l); --S; WM(SD,w.b.h)
#define PULLBYTE(b)     b = RM(SD); S++
#define PULLWORD(w)     w = RM(SD)<<8; S++; w |= RM(SD); S++

#define PSHUBYTE(b)     --U; WM(UD,b);
#define PSHUWORD(w)     --U; WM(UD,w.b.l); --U; WM(UD,w.b.h)
#define PULUBYTE(b)     b = RM(UD); U++
#define PULUWORD(w)     w = RM(UD)<<8; U++; w |= RM(UD); U++

#define CLR_HNZVC       CC&=~(CC_H|CC_N|CC_Z|CC_V|CC_C)
#define CLR_NZV         CC&=~(CC_N|CC_Z|CC_V)
#define CLR_NZ          CC&=~(CC_N|CC_Z)
#define CLR_HNZC        CC&=~(CC_H|CC_N|CC_Z|CC_C)
#define CLR_NZVC        CC&=~(CC_N|CC_Z|CC_V|CC_C)
#define CLR_Z           CC&=~(CC_Z)
#define CLR_NZC         CC&=~(CC_N|CC_Z|CC_C)
#define CLR_ZC          CC&=~(CC_Z|CC_C)

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)        if(!a)SEZ
#define SET_Z8(a)       SET_Z((uint8_t)a)
#define SET_Z16(a)      SET_Z((uint16_t)a)
#define SET_N8(a)       CC|=((a&0x80)>>4)
#define SET_N16(a)      CC|=((a&0x8000)>>12)
#define SET_H(a,b,r)    CC|=(((a^b^r)&0x10)<<1)
#define SET_C8(a)       CC|=((a&0x100)>>8)
#define SET_C16(a)      CC|=((a&0x10000)>>16)
#define SET_V8(a,b,r)   CC|=(((a^b^r^(r>>1))&0x80)>>6)
#define SET_V16(a,b,r)  CC|=(((a^b^r^(r>>1))&0x8000)>>14)

#define SET_FLAGS8I(t)          {CC|=m6809->flags8i[(t)&0xff];}
#define SET_FLAGS8D(t)          {CC|=m6809->flags8d[(t)&0xff];}

#define SET_NZ8(a)              {SET_N8(a);SET_Z(a);}
#define SET_NZ16(a)             {SET_N16(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)       {SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)      {SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

/* for treating an unsigned byte as a signed word */
#define SIGNED(b) ((uint16_t)(b&0x80?b|0xff00:b))

/* macros for addressing modes (postbytes have their own code) */
#define DIRECT  EAD = DPD; IMMBYTE(EAB)
#define IMM8    EAD = PCD; PC++
#define IMM16   EAD = PCD; PC+=2
#define EXTENDED IMMWORD(mEA)

/* macros to set status flags */
#define SEC CC|=CC_C
#define CLC CC&=~CC_C
#define SEZ CC|=CC_Z
#define CLZ CC&=~CC_Z
#define SEN CC|=CC_N
#define CLN CC&=~CC_N
#define SEV CC|=CC_V
#define CLV CC&=~CC_V
#define SEH CC|=CC_H
#define CLH CC&=~CC_H

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}

#define NXORV  ((CC&CC_N)^((CC&CC_V)<<2))

#define RM16(Addr)      ((RM(Addr) << 8) | RM(((Addr)+1)&0xffff))
#define WM16(Addr,p)    (WM((Addr),p.b.h),WM(((Addr)+1)&0xffff,p.b.l))

#define CHECK_IRQ_LINES 												\
	if( m6809->irq_state[M6809_IRQ_LINE] != CLEAR_LINE ||				\
		m6809->irq_state[M6809_FIRQ_LINE] != CLEAR_LINE )				\
		m6809->int_state &= ~M6809_SYNC; /* clear SYNC flag */			\
	if( m6809->irq_state[M6809_FIRQ_LINE]!=CLEAR_LINE && !(CC & CC_IF) ) \
	{																	\
		/* fast IRQ */													\
		/* HJB 990225: state already saved by CWAI? */					\
		if( m6809->int_state & M6809_CWAI )								\
		{																\
			m6809->int_state &= ~M6809_CWAI;  /* clear CWAI */			\
			m6809->extracycles += 7;		 /* subtract +7 cycles */	\
        }                                                               \
		else															\
		{																\
			CC &= ~CC_E;				/* save 'short' state */        \
			PUSHWORD(pPC);												\
			PUSHBYTE(CC);												\
			m6809->extracycles += 10;	/* subtract +10 cycles */		\
		}																\
		CC |= CC_IF | CC_II;			/* inhibit FIRQ and IRQ */		\
		PC=RM16(0xfff6);												\
		m6809->irq_state[M6809_FIRQ_LINE] = CLEAR_LINE; \
	}																	\
	else																\
	if( m6809->irq_state[M6809_IRQ_LINE]!=CLEAR_LINE && !(CC & CC_II) )	\
	{																	\
		/* standard IRQ */												\
		/* HJB 990225: state already saved by CWAI? */					\
		if( m6809->int_state & M6809_CWAI )								\
		{																\
			m6809->int_state &= ~M6809_CWAI;  /* clear CWAI flag */		\
			m6809->extracycles += 7;		 /* subtract +7 cycles */	\
		}																\
		else															\
		{																\
			CC |= CC_E; 				/* save entire state */ 		\
			PUSHWORD(pPC);												\
			PUSHWORD(pU);												\
			PUSHWORD(pY);												\
			PUSHWORD(pX);												\
			PUSHBYTE(DP);												\
			PUSHBYTE(B);												\
			PUSHBYTE(A);												\
			PUSHBYTE(CC);												\
			m6809->extracycles += 19;	 /* subtract +19 cycles */		\
		}																\
		CC |= CC_II;					/* inhibit IRQ */				\
		PC=RM16(0xfff8);												\
		m6809->irq_state[M6809_IRQ_LINE] = CLEAR_LINE; \
	}

/* macros for branch instructions */
#define BRANCH(f) { 					\
	uint8_t t;							\
	IMMBYTE(t); 						\
	if( f ) 							\
	{									\
		PC += SIGNED(t);				\
	}									\
}

#define LBRANCH(f) {                    \
	PAIR t; 							\
	IMMWORD(t); 						\
	if( f ) 							\
	{									\
		m6809_ICount -= 1;				\
		PC += t.w.l;					\
	}									\
}

#endif
