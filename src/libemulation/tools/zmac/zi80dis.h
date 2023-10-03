//
// zi80dis.h - extract disassembly and timing information for Z-80 and 8080 instructions.
//

// The Zi80dis class is little more than a bag of values filled in when Disassemble()
// is called.  For most programs "mem" points to the start of the Z-80's memory and the
// instruction to disassemble is at mem + pc.  Passing "true" for the 3rd argument means
// "mem" points to the start of the instruction.  In this case the caller needs to
// guarantee or at least check that sufficient data is available.
//
// Call Format() to output the disassembled instruction in a reasonably generic way.
// Advanced disassemblers will want to replace the %s arguments in m_format with
// symbolic names as directed by the associated m_arg[] and m_argType[] values.
//

#if defined(__cplusplus)

class Zi80dis
{
public:
	Zi80dis();

	// Don't re-order these or you'l break zmac.
	enum Processor {
		proc8080,
		procZ80,
		procZ180
	};

	void SetProcessor(Processor proc);	// default is procZ80
	void SetAssemblerable(bool assemblerable); // default true, slightly alters Format() output
	void SetDollarHex(bool dollarhex); // default false, prints hex constants with $ notation
	void SetUndocumented(bool undoc); // default false, does not disassemble undocumented instructions

	void Disassemble(const unsigned char *mem, int pc, bool memPointsToInstruction = false);
	void Format(char *outputBuffer);

	bool IsUndefined();

	enum ArgType {
		argByte,		// byte constant
		argWord,		// word constant
		argAddress,		// memory address
		argRelative,	// relative jump (but value is a memory address not an offset)
		argCall,		// call address
		argRst,			// restart address
		argIndex,		// IX or IY index offset
		argPort			// I/O port
	};

	// Data resulting from call to Disassemble()

	int	m_length;			// length of instruction in bytes
	int m_maxT;				// maximum number of T states to execute on z80
	int m_minT;				// minimum number of T states to execute on z80
	int m_max8080T;			// maximum number of T states to execute on 8080
	int m_min8080T;			// minimum number of T states to execute on 8080
	int m_max180T;			// maximum number of T states to execute on z180
	int m_min180T;			// minimum number of T states to execute on z180
	int m_ocf;				// number of opcode fetches (M1 states)
	char m_format[16];		// format string for disassembly
	int m_numArg;			// number of arguments
	int m_arg[2];			// decoded arguments
	ArgType m_argType[2];	// type of argument
	bool m_neverNext;		// processor never proceeds to the following instruction
	int m_stableMask;		// bit set for each byte of instruction invariant to relocation
	int m_attrMask;			// bit set indicating read/write/byte/word/in/out/jp/jr/jp(rr) (see AttrBit)

	enum AttrBit {
		attrRead = 1,
		attrWrite = 2,
		attrByte = 4,
		attrWord = 8,
		attrIn = 16,
		attrOut = 32,
		attrJump = 64,
		attrBranch = 128,
		attrCall = 256,
		attrRst = 512,
		attrJumpReg = 1024
	};

	// Options.

	Processor m_processor;
	bool m_assemblerable;
	bool m_dollarhex;
	bool m_undoc;
};

#else

// For the one holdout still using C (zmac).

int zi_tstates(const unsigned char *inst, int proc, int *low, int *high, int *ocf, char *disasm);

#endif /* defined(__cplusplus) */
