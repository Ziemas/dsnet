#ifndef MDEBUG_H_
#define MDEBUG_H_

/* Borrowed from CCC */

enum SymbolType {
    ST_NIL = 0,
    ST_GLOBAL = 1,
    ST_STATIC = 2,
    ST_PARAM = 3,
    ST_LOCAL = 4,
    ST_LABEL = 5,
    ST_PROC = 6,
    ST_BLOCK = 7,
    ST_END = 8,
    ST_MEMBER = 9,
    ST_TYPEDEF = 10,
    ST_FILE_SYMBOL = 11,
    ST_STATICPROC = 14,
    ST_CONSTANT = 15
};

enum SymbolClass {
    SC_NIL = 0,
    SC_TEXT = 1,
    SC_DATA = 2,
    SC_BSS = 3,
    SC_REGISTER = 4,
    SC_ABS = 5,
    SC_UNDEFINED = 6,
    SC_LOCAL = 7,
    SC_BITS = 8,
    SC_DBX = 9,
    SC_REG_IMAGE = 10,
    SC_INFO = 11,
    SC_USER_STRUCT = 12,
    SC_SDATA = 13,
    SC_SBSS = 14,
    SC_RDATA = 15,
    SC_VAR = 16,
    SC_COMMON = 17,
    SC_SCOMMON = 18,
    SC_VAR_REGISTER = 19,
    SC_VARIANT = 20,
    SC_SUNDEFINED = 21,
    SC_INIT = 22,
    SC_BASED_VAR = 23,
    SC_XDATA = 24,
    SC_PDATA = 25,
    SC_FINI = 26,
    SC_NONGP = 27
};

// See stab.def from gcc for documentation on what all these are.
enum StabsCode {
    SC_STAB = 0x00,
    SC_N_GSYM = 0x20,
    SC_N_FNAME = 0x22,
    SC_N_FUN = 0x24,
    SC_N_STSYM = 0x26,
    SC_N_LCSYM = 0x28,
    SC_N_MAIN = 0x2a,
    SC_N_ROSYM = 0x2c,
    SC_N_PC = 0x30,
    SC_N_NSYMS = 0x32,
    SC_N_NOMAP = 0x34,
    SC_N_OBJ = 0x38,
    SC_N_OPT = 0x3c,
    SC_N_RSYM = 0x40,
    SC_N_M2C = 0x42,
    SC_N_SLINE = 0x44,
    SC_N_DSLINE = 0x46,
    SC_N_BSLINE = 0x48,
    SC_N_EFD = 0x4a,
    SC_N_FLINE = 0x4c,
    SC_N_EHDECL = 0x50,
    SC_N_CATCH = 0x54,
    SC_N_SSYM = 0x60,
    SC_N_ENDM = 0x62,
    SC_N_SO = 0x64,
    SC_N_ALIAS = 0x6c,
    SC_N_LSYM = 0x80,
    SC_N_BINCL = 0x82,
    SC_N_SOL = 0x84,
    SC_N_PSYM = 0xa0,
    SC_N_EINCL = 0xa2,
    SC_N_ENTRY = 0xa4,
    SC_N_LBRAC = 0xc0,
    SC_N_EXCL = 0xc2,
    SC_N_SCOPE = 0xc4,
    SC_N_RBRAC = 0xe0,
    SC_N_BCOMM = 0xe2,
    SC_N_ECOMM = 0xe4,
    SC_N_ECOML = 0xe8,
    SC_N_WITH = 0xea,
    SC_N_NBTEXT = 0xf0,
    SC_N_NBDATA = 0xf2,
    SC_N_NBBSS = 0xf4,
    SC_N_NBSTS = 0xf6,
    SC_N_NBLCS = 0xf8,
    SC_N_LENG = 0xfe
};

#endif // MDEBUG_H_
