#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <time.h>
#include "putty.h"
#include "misc.h"

static void get_unitab(int codepage, wchar_t * unitab, int ftype);

/* Character conversion arrays; they are usually taken from windows,
 * the xterm one has the four scanlines that have no unicode 2.0
 * equivalents mapped to their unicode 3.0 locations.
 */
static char **uni_tbl;

static WCHAR unitab_xterm_std[32] = {
    0x2666, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0, 0x00b1,
    0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0x23ba,
    0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534, 0x252c,
    0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7, 0x0020
};

/*
 * If the codepage is non-zero it's a window codepage, zero means use a
 * local codepage. The name is always converted to the first of any
 * duplicate definitions.
 */

static wchar_t iso_8859_10[] = {
    0x00A0, 0x0104, 0x0112, 0x0122, 0x012A, 0x0128, 0x0136, 0x00A7,
    0x013B, 0x0110, 0x0160, 0x0166, 0x017D, 0x00AD, 0x016A, 0x014A,
    0x00B0, 0x0105, 0x0113, 0x0123, 0x012B, 0x0129, 0x0137, 0x00B7,
    0x013C, 0x0111, 0x0161, 0x0167, 0x017E, 0x2014, 0x016B, 0x014B,
    0x0100, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x012E,
    0x010C, 0x00C9, 0x0118, 0x00CB, 0x0116, 0x00CD, 0x00CE, 0x00CF,
    0x00D0, 0x0145, 0x014C, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0168,
    0x00D8, 0x0172, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF,
    0x0101, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x012F,
    0x010D, 0x00E9, 0x0119, 0x00EB, 0x0117, 0x00ED, 0x00EE, 0x00EF,
    0x00F0, 0x0146, 0x014D, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0169,
    0x00F8, 0x0173, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0138
};

static wchar_t iso_8859_11[] = {
    0x00A0, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07,
    0x0E08, 0x0E09, 0x0E0A, 0x0E0B, 0x0E0C, 0x0E0D, 0x0E0E, 0x0E0F,
    0x0E10, 0x0E11, 0x0E12, 0x0E13, 0x0E14, 0x0E15, 0x0E16, 0x0E17,
    0x0E18, 0x0E19, 0x0E1A, 0x0E1B, 0x0E1C, 0x0E1D, 0x0E1E, 0x0E1F,
    0x0E20, 0x0E21, 0x0E22, 0x0E23, 0x0E24, 0x0E25, 0x0E26, 0x0E27,
    0x0E28, 0x0E29, 0x0E2A, 0x0E2B, 0x0E2C, 0x0E2D, 0x0E2E, 0x0E2F,
    0x0E30, 0x0E31, 0x0E32, 0x0E33, 0x0E34, 0x0E35, 0x0E36, 0x0E37,
    0x0E38, 0x0E39, 0x0E3A, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD, 0x0E3F,
    0x0E40, 0x0E41, 0x0E42, 0x0E43, 0x0E44, 0x0E45, 0x0E46, 0x0E47,
    0x0E48, 0x0E49, 0x0E4A, 0x0E4B, 0x0E4C, 0x0E4D, 0x0E4E, 0x0E4F,
    0x0E50, 0x0E51, 0x0E52, 0x0E53, 0x0E54, 0x0E55, 0x0E56, 0x0E57,
    0x0E58, 0x0E59, 0x0E5A, 0x0E5B, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD
};

static wchar_t iso_8859_13[] = {
    0x00A0, 0x201D, 0x00A2, 0x00A3, 0x00A4, 0x201E, 0x00A6, 0x00A7,
    0x00D8, 0x00A9, 0x0156, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00C6,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x201C, 0x00B5, 0x00B6, 0x00B7,
    0x00F8, 0x00B9, 0x0157, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00E6,
    0x0104, 0x012E, 0x0100, 0x0106, 0x00C4, 0x00C5, 0x0118, 0x0112,
    0x010C, 0x00C9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012A, 0x013B,
    0x0160, 0x0143, 0x0145, 0x00D3, 0x014C, 0x00D5, 0x00D6, 0x00D7,
    0x0172, 0x0141, 0x015A, 0x016A, 0x00DC, 0x017B, 0x017D, 0x00DF,
    0x0105, 0x012F, 0x0101, 0x0107, 0x00E4, 0x00E5, 0x0119, 0x0113,
    0x010D, 0x00E9, 0x017A, 0x0117, 0x0123, 0x0137, 0x012B, 0x013C,
    0x0161, 0x0144, 0x0146, 0x00F3, 0x014D, 0x00F5, 0x00F6, 0x00F7,
    0x0173, 0x0142, 0x015B, 0x016B, 0x00FC, 0x017C, 0x017E, 0x2019
};

static wchar_t iso_8859_14[] = {
    0x00A0, 0x1E02, 0x1E03, 0x00A3, 0x010A, 0x010B, 0x1E0A, 0x00A7,
    0x1E80, 0x00A9, 0x1E82, 0x1E0B, 0x1EF2, 0x00AD, 0x00AE, 0x0178,
    0x1E1E, 0x1E1F, 0x0120, 0x0121, 0x1E40, 0x1E41, 0x00B6, 0x1E56,
    0x1E81, 0x1E57, 0x1E83, 0x1E60, 0x1EF3, 0x1E84, 0x1E85, 0x1E61,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x0174, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x1E6A,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x0176, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x0175, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x1E6B,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x0177, 0x00FF
};

static wchar_t iso_8859_15[] = {
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20ac, 0x00a5, 0x0160, 0x00a7,
    0x0161, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x017d, 0x00b5, 0x00b6, 0x00b7,
    0x017e, 0x00b9, 0x00ba, 0x00bb, 0x0152, 0x0153, 0x0178, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};

static wchar_t iso_8859_16[] = {
    0x00A0, 0x0104, 0x0105, 0x0141, 0x20AC, 0x201E, 0x0160, 0x00A7,
    0x0161, 0x00A9, 0x0218, 0x00AB, 0x0179, 0x00AD, 0x017A, 0x017B,
    0x00B0, 0x00B1, 0x010C, 0x0142, 0x017D, 0x201D, 0x00B6, 0x00B7,
    0x017E, 0x010D, 0x0219, 0x00BB, 0x0152, 0x0153, 0x0178, 0x017C,
    0x00C0, 0x00C1, 0x00C2, 0x0102, 0x00C4, 0x0106, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0x0110, 0x0143, 0x00D2, 0x00D3, 0x00D4, 0x0150, 0x00D6, 0x015A,
    0x0170, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0118, 0x021A, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x0103, 0x00E4, 0x0107, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0x0111, 0x0144, 0x00F2, 0x00F3, 0x00F4, 0x0151, 0x00F6, 0x015B,
    0x0171, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x0119, 0x021B, 0x00FF
};

static wchar_t roman8[] = {
    0x00A0, 0x00C0, 0x00C2, 0x00C8, 0x00CA, 0x00CB, 0x00CE, 0x00CF,
    0x00B4, 0x02CB, 0x02C6, 0x00A8, 0x02DC, 0x00D9, 0x00DB, 0x20A4,
    0x00AF, 0x00DD, 0x00FD, 0x00B0, 0x00C7, 0x00E7, 0x00D1, 0x00F1,
    0x00A1, 0x00BF, 0x00A4, 0x00A3, 0x00A5, 0x00A7, 0x0192, 0x00A2,
    0x00E2, 0x00EA, 0x00F4, 0x00FB, 0x00E1, 0x00E9, 0x00F3, 0x00FA,
    0x00E0, 0x00E8, 0x00F2, 0x00F9, 0x00E4, 0x00EB, 0x00F6, 0x00FC,
    0x00C5, 0x00EE, 0x00D8, 0x00C6, 0x00E5, 0x00ED, 0x00F8, 0x00E6,
    0x00C4, 0x00EC, 0x00D6, 0x00DC, 0x00C9, 0x00EF, 0x00DF, 0x00D4,
    0x00C1, 0x00C3, 0x00E3, 0x00D0, 0x00F0, 0x00CD, 0x00CC, 0x00D3,
    0x00D2, 0x00D5, 0x00F5, 0x0160, 0x0161, 0x00DA, 0x0178, 0x00FF,
    0x00DE, 0x00FE, 0x00B7, 0x00B5, 0x00B6, 0x00BE, 0x2014, 0x00BC,
    0x00BD, 0x00AA, 0x00BA, 0x00AB, 0x25A0, 0x00BB, 0x00B1, 0xFFFD
};

static wchar_t koi8_u[] = {
    0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524,
    0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,
    0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2022, 0x221A, 0x2248,
    0x2264, 0x2265, 0x00A0, 0x2321, 0x00B0, 0x00B2, 0x00B7, 0x00F7,
    0x2550, 0x2551, 0x2552, 0x0451, 0x0454, 0x2554, 0x0456, 0x0457,
    0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x0491, 0x255D, 0x255E,
    0x255F, 0x2560, 0x2561, 0x0401, 0x0404, 0x2563, 0x0406, 0x0407,
    0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x0490, 0x256C, 0x00A9,
    0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
    0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
    0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
    0x044C, 0x044B, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x044A,
    0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
    0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
    0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
    0x042C, 0x042B, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042A
};

static wchar_t vscii[] = {
    0x0000, 0x0001, 0x1EB2, 0x0003, 0x0004, 0x1EB4, 0x1EAA, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x1EF6, 0x0015, 0x0016, 0x0017,
    0x0018, 0x1EF8, 0x001a, 0x001b, 0x001c, 0x001d, 0x1EF4, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007f,
    0x1EA0, 0x1EAE, 0x1EB0, 0x1EB6, 0x1EA4, 0x1EA6, 0x1EA8, 0x1EAC,
    0x1EBC, 0x1EB8, 0x1EBE, 0x1EC0, 0x1EC2, 0x1EC4, 0x1EC6, 0x1ED0,
    0x1ED2, 0x1ED4, 0x1ED6, 0x1ED8, 0x1EE2, 0x1EDA, 0x1EDC, 0x1EDE,
    0x1ECA, 0x1ECE, 0x1ECC, 0x1EC8, 0x1EE6, 0x0168, 0x1EE4, 0x1EF2,
    0x00D5, 0x1EAF, 0x1EB1, 0x1EB7, 0x1EA5, 0x1EA7, 0x1EA8, 0x1EAD,
    0x1EBD, 0x1EB9, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC5, 0x1EC7, 0x1ED1,
    0x1ED3, 0x1ED5, 0x1ED7, 0x1EE0, 0x01A0, 0x1ED9, 0x1EDD, 0x1EDF,
    0x1ECB, 0x1EF0, 0x1EE8, 0x1EEA, 0x1EEC, 0x01A1, 0x1EDB, 0x01AF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x1EA2, 0x0102, 0x1EB3, 0x1EB5,
    0x00C8, 0x00C9, 0x00CA, 0x1EBA, 0x00CC, 0x00CD, 0x0128, 0x1EF3,
    0x0110, 0x1EE9, 0x00D2, 0x00D3, 0x00D4, 0x1EA1, 0x1EF7, 0x1EEB,
    0x1EED, 0x00D9, 0x00DA, 0x1EF9, 0x1EF5, 0x00DD, 0x1EE1, 0x01B0,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x1EA3, 0x0103, 0x1EEF, 0x1EAB,
    0x00E8, 0x00E9, 0x00EA, 0x1EBB, 0x00EC, 0x00ED, 0x0129, 0x1EC9,
    0x0111, 0x1EF1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x1ECF, 0x1ECD,
    0x1EE5, 0x00F9, 0x00FA, 0x0169, 0x1EE7, 0x00FD, 0x1EE3, 0x1EEE
};

static wchar_t dec_mcs[] = {
    0x00A0, 0x00A1, 0x00A2, 0x00A3, 0xFFFD, 0x00A5, 0xFFFD, 0x00A7,
    0x00A4, 0x00A9, 0x00AA, 0x00AB, 0xFFFD, 0xFFFD, 0xFFFD, 0xFFFD,
    0x00B0, 0x00B1, 0x00B2, 0x00B3, 0xFFFD, 0x00B5, 0x00B6, 0x00B7,
    0xFFFD, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0xFFFD, 0x00BF,
    0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7,
    0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF,
    0xFFFD, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x0152,
    0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x0178, 0xFFFD, 0x00DF,
    0x00E0, 0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7,
    0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF,
    0xFFFD, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x0153,
    0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FF, 0xFFFD, 0xFFFD
};

struct cp_list_item {
    char *name;
    int codepage;
    int cp_size;
    wchar_t *cp_table;
};

static struct cp_list_item cp_list[] = {
    {"ISO-8859-1:1987", 28591},
    {"ISO-8859-2:1987", 28592},
    {"ISO-8859-3:1988", 28593},
    {"ISO-8859-4:1988", 28594},
    {"ISO-8859-5:1988", 28595},
    {"ISO-8859-6:1987", 28596},
    {"ISO-8859-7:1987", 28597},
    {"ISO-8859-8:1988", 28598},
    {"ISO-8859-9:1989", 28599},
    {"ISO-8859-10:1993", 0, 96, iso_8859_10},
    {"ISO-8859-11:1997", 0, 96, iso_8859_11},
    {"ISO-8859-13:1998", 0, 96, iso_8859_13},
    {"ISO-8859-14:1998", 0, 96, iso_8859_14},
    {"ISO-8859-15:1998", 0, 96, iso_8859_15},
    {"ISO-8859-16:2001", 0, 96, iso_8859_16},

    {"UTF-8", CP_UTF8},

    {"KOI8-U", 0, 128, koi8_u},
    {"KOI8-R", 20866},
    {"HP-ROMAN8", 0, 96, roman8},
    {"VSCII", 0, 256, vscii},
    {"DEC-MCS", 0, 96, dec_mcs},

    {"Win1250 (Central European)", 1250},
    {"Win1251 (Cyrillic)", 1251},
    {"Win1252 (Western)", 1252},
    {"Win1253 (Greek)", 1253},
    {"Win1254 (Turkish)", 1254},
    {"Win1255 (Hebrew)", 1255},
    {"Win1256 (Arabic)", 1256},
    {"Win1257 (Baltic)", 1257},
    {"Win1258 (Vietnamese)", 1258},

    {"Win1258 (Vietnamese)", 1258},

    {"CP437", 437},
    {"CP819", 28591},
    {"CP878", 20866},

    {"Use font encoding", -1},

    {0, 0}
};

static void link_font(WCHAR * line_tbl, WCHAR * font_tbl, WCHAR attr);

void init_ucs_tables(void)
{
    int i, j;
    int used_dtf = 0;
    char tbuf[256];

    for (i = 0; i < 256; i++)
	tbuf[i] = i;

    /* Decide on the Line and Font codepages */
    line_codepage = decode_codepage(cfg.line_codepage);

    if (font_codepage <= 0) { 
	font_codepage=0; 
	dbcs_screenfont=0; 
    }

    if (cfg.vtmode == VT_OEMONLY) {
	font_codepage = 437;
	dbcs_screenfont = 0;
	if (line_codepage <= 0)
	    line_codepage = GetACP();
    } else if (line_codepage <= 0)
	line_codepage = font_codepage;

    /* Collect screen font ucs table */
    if (dbcs_screenfont || font_codepage == 0) {
	get_unitab(font_codepage, unitab_font, 2);
	for (i = 128; i < 256; i++)
	    unitab_font[i] = (WCHAR) (ATTR_ACP + i);
    } else {
	get_unitab(font_codepage, unitab_font, 1);

	/* CP437 fonts are often broken ... */
	if (font_codepage == 437)
	    unitab_font[0] = unitab_font[255] = 0xFFFF;
    }
    if (cfg.vtmode == VT_XWINDOWS)
	memcpy(unitab_font + 1, unitab_xterm_std,
	       sizeof(unitab_xterm_std));

    /* Collect OEMCP ucs table */
    get_unitab(CP_OEMCP, unitab_oemcp, 1);

    /* Collect CP437 ucs table for SCO acs */
    if (cfg.vtmode == VT_OEMANSI || cfg.vtmode == VT_XWINDOWS)
	memcpy(unitab_scoacs, unitab_oemcp, sizeof(unitab_scoacs));
    else
	get_unitab(437, unitab_scoacs, 1);

    /* Collect line set ucs table */
    if (line_codepage == font_codepage &&
	(dbcs_screenfont || cfg.vtmode == VT_POORMAN || font_codepage==0)) {

	/* For DBCS and POOR fonts force direct to font */
	used_dtf = 1;
	for (i = 0; i < 32; i++)
	    unitab_line[i] = (WCHAR) i;
	for (i = 32; i < 256; i++)
	    unitab_line[i] = (WCHAR) (ATTR_ACP + i);
	unitab_line[127] = (WCHAR) 127;
    } else {
	get_unitab(line_codepage, unitab_line, 0);
    }

#if 0
    debug(
	  ("Line cp%d, Font cp%d%s\n", line_codepage, font_codepage,
	   dbcs_screenfont ? " DBCS" : ""));

    for (i = 0; i < 256; i += 16) {
	for (j = 0; j < 16; j++) {
	    debug(("%04x%s", unitab_line[i + j], j == 15 ? "" : ","));
	}
	debug(("\n"));
    }
#endif

    /* VT100 graphics - NB: Broken for non-ascii CP's */
    memcpy(unitab_xterm, unitab_line, sizeof(unitab_xterm));
    memcpy(unitab_xterm + '`', unitab_xterm_std, sizeof(unitab_xterm_std));
    unitab_xterm['_'] = ' ';

    /* Generate UCS ->line page table. */
    if (uni_tbl) {
	for (i = 0; i < 256; i++)
	    if (uni_tbl[i])
		sfree(uni_tbl[i]);
	sfree(uni_tbl);
	uni_tbl = 0;
    }
    if (!used_dtf) {
	for (i = 0; i < 256; i++) {
	    if (DIRECT_CHAR(unitab_line[i]))
		continue;
	    if (DIRECT_FONT(unitab_line[i]))
		continue;
	    if (!uni_tbl) {
		uni_tbl = smalloc(256 * sizeof(char *));
		memset(uni_tbl, 0, 256 * sizeof(char *));
	    }
	    j = ((unitab_line[i] >> 8) & 0xFF);
	    if (!uni_tbl[j]) {
		uni_tbl[j] = smalloc(256 * sizeof(char));
		memset(uni_tbl[j], 0, 256 * sizeof(char));
	    }
	    uni_tbl[j][unitab_line[i] & 0xFF] = i;
	}
    }

    /* Find the line control characters. */
    for (i = 0; i < 256; i++)
	if (unitab_line[i] < ' '
	    || (unitab_line[i] >= 0x7F && unitab_line[i] < 0xA0))
	    unitab_ctrl[i] = i;
	else
	    unitab_ctrl[i] = 0xFF;

    /* Generate line->screen direct conversion links. */
    if (cfg.vtmode == VT_OEMANSI || cfg.vtmode == VT_XWINDOWS)
	link_font(unitab_scoacs, unitab_oemcp, ATTR_OEMCP);

    link_font(unitab_line, unitab_font, ATTR_ACP);
    link_font(unitab_scoacs, unitab_font, ATTR_ACP);
    link_font(unitab_xterm, unitab_font, ATTR_ACP);

    if (cfg.vtmode == VT_OEMANSI || cfg.vtmode == VT_XWINDOWS) {
	link_font(unitab_line, unitab_oemcp, ATTR_OEMCP);
	link_font(unitab_xterm, unitab_oemcp, ATTR_OEMCP);
    }

    if (dbcs_screenfont && font_codepage != line_codepage) {
	/* F***ing Microsoft fonts, Japanese and Korean codepage fonts
	 * have a currency symbol at 0x5C but their unicode value is 
	 * still given as U+005C not the correct U+00A5. */
	unitab_line['\\'] = ATTR_OEMCP + '\\';
    }

    /* Last chance, if !unicode then try poorman links. */
    if (cfg.vtmode != VT_UNICODE) {
	static char poorman_scoacs[] = 
	    "CueaaaaceeeiiiAAE**ooouuyOUc$YPsaiounNao?++**!<>###||||++||++++++--|-+||++--|-+----++++++++##||#aBTPEsyt******EN=+><++-=... n2* ";
	static char poorman_latin1[] =
	    " !cL.Y|S\"Ca<--R~o+23'u|.,1o>///?AAAAAAACEEEEIIIIDNOOOOOxOUUUUYPBaaaaaaaceeeeiiiionooooo/ouuuuypy";
	static char poorman_vt100[] = "*#****o~**+++++-----++++|****L.";

	for (i = 160; i < 256; i++)
	    if (!DIRECT_FONT(unitab_line[i]) &&
		unitab_line[i] >= 160 && unitab_line[i] < 256)
		unitab_line[i] = (WCHAR) (ATTR_ACP
					  + poorman_latin1[unitab_line[i] -
							   160]);
	for (i = 96; i < 127; i++)
	    if (!DIRECT_FONT(unitab_xterm[i]))
		unitab_xterm[i] =
		    (WCHAR) (ATTR_ACP + poorman_vt100[i - 96]);
	for(i=128;i<256;i++) 
	    if (!DIRECT_FONT(unitab_scoacs[i]))
		unitab_scoacs[i] = 
		    (WCHAR) (ATTR_ACP + poorman_scoacs[i - 128]);
    }
}

static void link_font(WCHAR * line_tbl, WCHAR * font_tbl, WCHAR attr)
{
    int font_index, line_index, i;
    for (line_index = 0; line_index < 256; line_index++) {
	if (DIRECT_FONT(line_tbl[line_index]))
	    continue;
	for(i = 0; i < 256; i++) {
	    font_index = ((32 + i) & 0xFF);
	    if (line_tbl[line_index] == font_tbl[font_index]) {
		line_tbl[line_index] = (WCHAR) (attr + font_index);
		break;
	    }
	}
    }
}

void lpage_send(int codepage, char *buf, int len, int interactive)
{
    static wchar_t *widebuffer = 0;
    static int widesize = 0;
    int wclen;

    if (codepage < 0) {
	ldisc_send(buf, len, interactive);
	return;
    }

    if (len > widesize) {
	sfree(widebuffer);
	widebuffer = smalloc(len * 2 * sizeof(wchar_t));
	widesize = len * 2;
    }

    wclen =
	MultiByteToWideChar(codepage, 0, buf, len, widebuffer, widesize);
    luni_send(widebuffer, wclen, interactive);
}

void luni_send(wchar_t * widebuf, int len, int interactive)
{
    static char *linebuffer = 0;
    static int linesize = 0;
    int ratio = (in_utf)?3:1;
    int i;
    char *p;

    if (len * ratio > linesize) {
	sfree(linebuffer);
	linebuffer = smalloc(len * ratio * 2 * sizeof(wchar_t));
	linesize = len * ratio * 2;
    }

    if (in_utf) {
	/* UTF is a simple algorithm */
	for (p = linebuffer, i = 0; i < len; i++) {
	    wchar_t ch = widebuf[i];
	    /* Windows wchar_t is UTF-16 */
	    if ((ch&0xF800) == 0xD800) ch = '.';

	    if (ch < 0x80) {
		*p++ = (char) (ch);
	    } else if (ch < 0x800) {
		*p++ = (0xC0 | (ch >> 6));
		*p++ = (0x80 | (ch & 0x3F));
	    } else {
		*p++ = (0xE0 | (ch >> 12));
		*p++ = (0x80 | ((ch >> 6) & 0x3F));
		*p++ = (0x80 | (ch & 0x3F));
	    }
	}
    } else if (!uni_tbl) {
	int rv;
	rv = WideCharToMultiByte(line_codepage, 0, widebuf, len,
				 linebuffer, linesize, NULL, NULL);
	if (rv >= 0)
	    p = linebuffer + rv;
	else
	    p = linebuffer;
    } else {
	/* Others are a lookup in an array */
	for (p = linebuffer, i = 0; i < len; i++) {
	    wchar_t ch = widebuf[i];
	    int by;
	    char *p1;
	    if (uni_tbl && (p1 = uni_tbl[(ch >> 8) & 0xFF])
		&& (by = p1[ch & 0xFF]))
		*p++ = by;
	    else if (ch < 0x80)
		*p++ = (char) ch;
#if 1
	    else
		*p++ = '.';
#endif
	}
    }
    if (p > linebuffer)
	ldisc_send(linebuffer, p - linebuffer, interactive);
}

wchar_t xlat_uskbd2cyrllic(int ch)
{
    static wchar_t cyrtab[] = {
            0,      1,       2,      3,      4,      5,      6,      7,
            8,      9,      10,     11,     12,     13,     14,     15,
            16,     17,     18,     19,     20,     21,     22,     23,
            24,     25,     26,     27,     28,     29,     30,     31,
            32,     33, 0x042d,     35,     36,     37,     38, 0x044d,
            40,     41,     42, 0x0406, 0x0431, 0x0454, 0x044e, 0x002e,
            48,     49,     50,     51,     52,     53,     54,     55,
            56,     57, 0x0416, 0x0436, 0x0411, 0x0456, 0x042e, 0x002c,
            64, 0x0424, 0x0418, 0x0421, 0x0412, 0x0423, 0x0410, 0x041f,
        0x0420, 0x0428, 0x041e, 0x041b, 0x0414, 0x042c, 0x0422, 0x0429,
        0x0417, 0x0419, 0x041a, 0x042b, 0x0415, 0x0413, 0x041c, 0x0426,
        0x0427, 0x041d, 0x042f, 0x0445, 0x0457, 0x044a,     94, 0x0404,
            96, 0x0444, 0x0438, 0x0441, 0x0432, 0x0443, 0x0430, 0x043f,
        0x0440, 0x0448, 0x043e, 0x043b, 0x0434, 0x044c, 0x0442, 0x0449,
        0x0437, 0x0439, 0x043a, 0x044b, 0x0435, 0x0433, 0x043c, 0x0446,
        0x0447, 0x043d, 0x044f, 0x0425, 0x0407, 0x042a,    126,    127
       };
    return cyrtab[ch&0x7F];
}

int check_compose(int first, int second)
{

    static struct {
	char first, second;
	wchar_t composed;
    } composetbl[] = {
	{
	0x2b, 0x2b, 0x0023}, {
	0x41, 0x41, 0x0040}, {
	0x28, 0x28, 0x005b}, {
	0x2f, 0x2f, 0x005c}, {
	0x29, 0x29, 0x005d}, {
	0x28, 0x2d, 0x007b}, {
	0x2d, 0x29, 0x007d}, {
	0x2f, 0x5e, 0x007c}, {
	0x21, 0x21, 0x00a1}, {
	0x43, 0x2f, 0x00a2}, {
	0x43, 0x7c, 0x00a2}, {
	0x4c, 0x2d, 0x00a3}, {
	0x4c, 0x3d, 0x20a4}, {
	0x58, 0x4f, 0x00a4}, {
	0x58, 0x30, 0x00a4}, {
	0x59, 0x2d, 0x00a5}, {
	0x59, 0x3d, 0x00a5}, {
	0x7c, 0x7c, 0x00a6}, {
	0x53, 0x4f, 0x00a7}, {
	0x53, 0x21, 0x00a7}, {
	0x53, 0x30, 0x00a7}, {
	0x22, 0x22, 0x00a8}, {
	0x43, 0x4f, 0x00a9}, {
	0x43, 0x30, 0x00a9}, {
	0x41, 0x5f, 0x00aa}, {
	0x3c, 0x3c, 0x00ab}, {
	0x2c, 0x2d, 0x00ac}, {
	0x2d, 0x2d, 0x00ad}, {
	0x52, 0x4f, 0x00ae}, {
	0x2d, 0x5e, 0x00af}, {
	0x30, 0x5e, 0x00b0}, {
	0x2b, 0x2d, 0x00b1}, {
	0x32, 0x5e, 0x00b2}, {
	0x33, 0x5e, 0x00b3}, {
	0x27, 0x27, 0x00b4}, {
	0x2f, 0x55, 0x00b5}, {
	0x50, 0x21, 0x00b6}, {
	0x2e, 0x5e, 0x00b7}, {
	0x2c, 0x2c, 0x00b8}, {
	0x31, 0x5e, 0x00b9}, {
	0x4f, 0x5f, 0x00ba}, {
	0x3e, 0x3e, 0x00bb}, {
	0x31, 0x34, 0x00bc}, {
	0x31, 0x32, 0x00bd}, {
	0x33, 0x34, 0x00be}, {
	0x3f, 0x3f, 0x00bf}, {
	0x60, 0x41, 0x00c0}, {
	0x27, 0x41, 0x00c1}, {
	0x5e, 0x41, 0x00c2}, {
	0x7e, 0x41, 0x00c3}, {
	0x22, 0x41, 0x00c4}, {
	0x2a, 0x41, 0x00c5}, {
	0x41, 0x45, 0x00c6}, {
	0x2c, 0x43, 0x00c7}, {
	0x60, 0x45, 0x00c8}, {
	0x27, 0x45, 0x00c9}, {
	0x5e, 0x45, 0x00ca}, {
	0x22, 0x45, 0x00cb}, {
	0x60, 0x49, 0x00cc}, {
	0x27, 0x49, 0x00cd}, {
	0x5e, 0x49, 0x00ce}, {
	0x22, 0x49, 0x00cf}, {
	0x2d, 0x44, 0x00d0}, {
	0x7e, 0x4e, 0x00d1}, {
	0x60, 0x4f, 0x00d2}, {
	0x27, 0x4f, 0x00d3}, {
	0x5e, 0x4f, 0x00d4}, {
	0x7e, 0x4f, 0x00d5}, {
	0x22, 0x4f, 0x00d6}, {
	0x58, 0x58, 0x00d7}, {
	0x2f, 0x4f, 0x00d8}, {
	0x60, 0x55, 0x00d9}, {
	0x27, 0x55, 0x00da}, {
	0x5e, 0x55, 0x00db}, {
	0x22, 0x55, 0x00dc}, {
	0x27, 0x59, 0x00dd}, {
	0x48, 0x54, 0x00de}, {
	0x73, 0x73, 0x00df}, {
	0x60, 0x61, 0x00e0}, {
	0x27, 0x61, 0x00e1}, {
	0x5e, 0x61, 0x00e2}, {
	0x7e, 0x61, 0x00e3}, {
	0x22, 0x61, 0x00e4}, {
	0x2a, 0x61, 0x00e5}, {
	0x61, 0x65, 0x00e6}, {
	0x2c, 0x63, 0x00e7}, {
	0x60, 0x65, 0x00e8}, {
	0x27, 0x65, 0x00e9}, {
	0x5e, 0x65, 0x00ea}, {
	0x22, 0x65, 0x00eb}, {
	0x60, 0x69, 0x00ec}, {
	0x27, 0x69, 0x00ed}, {
	0x5e, 0x69, 0x00ee}, {
	0x22, 0x69, 0x00ef}, {
	0x2d, 0x64, 0x00f0}, {
	0x7e, 0x6e, 0x00f1}, {
	0x60, 0x6f, 0x00f2}, {
	0x27, 0x6f, 0x00f3}, {
	0x5e, 0x6f, 0x00f4}, {
	0x7e, 0x6f, 0x00f5}, {
	0x22, 0x6f, 0x00f6}, {
	0x3a, 0x2d, 0x00f7}, {
	0x6f, 0x2f, 0x00f8}, {
	0x60, 0x75, 0x00f9}, {
	0x27, 0x75, 0x00fa}, {
	0x5e, 0x75, 0x00fb}, {
	0x22, 0x75, 0x00fc}, {
	0x27, 0x79, 0x00fd}, {
	0x68, 0x74, 0x00fe}, {
	0x22, 0x79, 0x00ff},
	    /* Unicode extras. */
	{
	0x6f, 0x65, 0x0153}, {
	0x4f, 0x45, 0x0152},
	    /* Compose pairs from UCS */
	{
	0x41, 0x2D, 0x0100}, {
	0x61, 0x2D, 0x0101}, {
	0x43, 0x27, 0x0106}, {
	0x63, 0x27, 0x0107}, {
	0x43, 0x5E, 0x0108}, {
	0x63, 0x5E, 0x0109}, {
	0x45, 0x2D, 0x0112}, {
	0x65, 0x2D, 0x0113}, {
	0x47, 0x5E, 0x011C}, {
	0x67, 0x5E, 0x011D}, {
	0x47, 0x2C, 0x0122}, {
	0x67, 0x2C, 0x0123}, {
	0x48, 0x5E, 0x0124}, {
	0x68, 0x5E, 0x0125}, {
	0x49, 0x7E, 0x0128}, {
	0x69, 0x7E, 0x0129}, {
	0x49, 0x2D, 0x012A}, {
	0x69, 0x2D, 0x012B}, {
	0x4A, 0x5E, 0x0134}, {
	0x6A, 0x5E, 0x0135}, {
	0x4B, 0x2C, 0x0136}, {
	0x6B, 0x2C, 0x0137}, {
	0x4C, 0x27, 0x0139}, {
	0x6C, 0x27, 0x013A}, {
	0x4C, 0x2C, 0x013B}, {
	0x6C, 0x2C, 0x013C}, {
	0x4E, 0x27, 0x0143}, {
	0x6E, 0x27, 0x0144}, {
	0x4E, 0x2C, 0x0145}, {
	0x6E, 0x2C, 0x0146}, {
	0x4F, 0x2D, 0x014C}, {
	0x6F, 0x2D, 0x014D}, {
	0x52, 0x27, 0x0154}, {
	0x72, 0x27, 0x0155}, {
	0x52, 0x2C, 0x0156}, {
	0x72, 0x2C, 0x0157}, {
	0x53, 0x27, 0x015A}, {
	0x73, 0x27, 0x015B}, {
	0x53, 0x5E, 0x015C}, {
	0x73, 0x5E, 0x015D}, {
	0x53, 0x2C, 0x015E}, {
	0x73, 0x2C, 0x015F}, {
	0x54, 0x2C, 0x0162}, {
	0x74, 0x2C, 0x0163}, {
	0x55, 0x7E, 0x0168}, {
	0x75, 0x7E, 0x0169}, {
	0x55, 0x2D, 0x016A}, {
	0x75, 0x2D, 0x016B}, {
	0x55, 0x2A, 0x016E}, {
	0x75, 0x2A, 0x016F}, {
	0x57, 0x5E, 0x0174}, {
	0x77, 0x5E, 0x0175}, {
	0x59, 0x5E, 0x0176}, {
	0x79, 0x5E, 0x0177}, {
	0x59, 0x22, 0x0178}, {
	0x5A, 0x27, 0x0179}, {
	0x7A, 0x27, 0x017A}, {
	0x47, 0x27, 0x01F4}, {
	0x67, 0x27, 0x01F5}, {
	0x4E, 0x60, 0x01F8}, {
	0x6E, 0x60, 0x01F9}, {
	0x45, 0x2C, 0x0228}, {
	0x65, 0x2C, 0x0229}, {
	0x59, 0x2D, 0x0232}, {
	0x79, 0x2D, 0x0233}, {
	0x44, 0x2C, 0x1E10}, {
	0x64, 0x2C, 0x1E11}, {
	0x47, 0x2D, 0x1E20}, {
	0x67, 0x2D, 0x1E21}, {
	0x48, 0x22, 0x1E26}, {
	0x68, 0x22, 0x1E27}, {
	0x48, 0x2C, 0x1E28}, {
	0x68, 0x2C, 0x1E29}, {
	0x4B, 0x27, 0x1E30}, {
	0x6B, 0x27, 0x1E31}, {
	0x4D, 0x27, 0x1E3E}, {
	0x6D, 0x27, 0x1E3F}, {
	0x50, 0x27, 0x1E54}, {
	0x70, 0x27, 0x1E55}, {
	0x56, 0x7E, 0x1E7C}, {
	0x76, 0x7E, 0x1E7D}, {
	0x57, 0x60, 0x1E80}, {
	0x77, 0x60, 0x1E81}, {
	0x57, 0x27, 0x1E82}, {
	0x77, 0x27, 0x1E83}, {
	0x57, 0x22, 0x1E84}, {
	0x77, 0x22, 0x1E85}, {
	0x58, 0x22, 0x1E8C}, {
	0x78, 0x22, 0x1E8D}, {
	0x5A, 0x5E, 0x1E90}, {
	0x7A, 0x5E, 0x1E91}, {
	0x74, 0x22, 0x1E97}, {
	0x77, 0x2A, 0x1E98}, {
	0x79, 0x2A, 0x1E99}, {
	0x45, 0x7E, 0x1EBC}, {
	0x65, 0x7E, 0x1EBD}, {
	0x59, 0x60, 0x1EF2}, {
	0x79, 0x60, 0x1EF3}, {
	0x59, 0x7E, 0x1EF8}, {
	0x79, 0x7E, 0x1EF9},
	    /* Compatible/possibles from UCS */
	{
	0x49, 0x4A, 0x0132}, {
	0x69, 0x6A, 0x0133}, {
	0x4C, 0x4A, 0x01C7}, {
	0x4C, 0x6A, 0x01C8}, {
	0x6C, 0x6A, 0x01C9}, {
	0x4E, 0x4A, 0x01CA}, {
	0x4E, 0x6A, 0x01CB}, {
	0x6E, 0x6A, 0x01CC}, {
	0x44, 0x5A, 0x01F1}, {
	0x44, 0x7A, 0x01F2}, {
	0x64, 0x7A, 0x01F3}, {
	0x2E, 0x2E, 0x2025}, {
	0x21, 0x21, 0x203C}, {
	0x3F, 0x21, 0x2048}, {
	0x21, 0x3F, 0x2049}, {
	0x52, 0x73, 0x20A8}, {
	0x4E, 0x6F, 0x2116}, {
	0x53, 0x4D, 0x2120}, {
	0x54, 0x4D, 0x2122}, {
	0x49, 0x49, 0x2161}, {
	0x49, 0x56, 0x2163}, {
	0x56, 0x49, 0x2165}, {
	0x49, 0x58, 0x2168}, {
	0x58, 0x49, 0x216A}, {
	0x69, 0x69, 0x2171}, {
	0x69, 0x76, 0x2173}, {
	0x76, 0x69, 0x2175}, {
	0x69, 0x78, 0x2178}, {
	0x78, 0x69, 0x217A}, {
	0x31, 0x30, 0x2469}, {
	0x31, 0x31, 0x246A}, {
	0x31, 0x32, 0x246B}, {
	0x31, 0x33, 0x246C}, {
	0x31, 0x34, 0x246D}, {
	0x31, 0x35, 0x246E}, {
	0x31, 0x36, 0x246F}, {
	0x31, 0x37, 0x2470}, {
	0x31, 0x38, 0x2471}, {
	0x31, 0x39, 0x2472}, {
	0x32, 0x30, 0x2473}, {
	0x31, 0x2E, 0x2488}, {
	0x32, 0x2E, 0x2489}, {
	0x33, 0x2E, 0x248A}, {
	0x34, 0x2E, 0x248B}, {
	0x35, 0x2E, 0x248C}, {
	0x36, 0x2E, 0x248D}, {
	0x37, 0x2E, 0x248E}, {
	0x38, 0x2E, 0x248F}, {
	0x39, 0x2E, 0x2490}, {
	0x64, 0x61, 0x3372}, {
	0x41, 0x55, 0x3373}, {
	0x6F, 0x56, 0x3375}, {
	0x70, 0x63, 0x3376}, {
	0x70, 0x41, 0x3380}, {
	0x6E, 0x41, 0x3381}, {
	0x6D, 0x41, 0x3383}, {
	0x6B, 0x41, 0x3384}, {
	0x4B, 0x42, 0x3385}, {
	0x4D, 0x42, 0x3386}, {
	0x47, 0x42, 0x3387}, {
	0x70, 0x46, 0x338A}, {
	0x6E, 0x46, 0x338B}, {
	0x6D, 0x67, 0x338E}, {
	0x6B, 0x67, 0x338F}, {
	0x48, 0x7A, 0x3390}, {
	0x66, 0x6D, 0x3399}, {
	0x6E, 0x6D, 0x339A}, {
	0x6D, 0x6D, 0x339C}, {
	0x63, 0x6D, 0x339D}, {
	0x6B, 0x6D, 0x339E}, {
	0x50, 0x61, 0x33A9}, {
	0x70, 0x73, 0x33B0}, {
	0x6E, 0x73, 0x33B1}, {
	0x6D, 0x73, 0x33B3}, {
	0x70, 0x56, 0x33B4}, {
	0x6E, 0x56, 0x33B5}, {
	0x6D, 0x56, 0x33B7}, {
	0x6B, 0x56, 0x33B8}, {
	0x4D, 0x56, 0x33B9}, {
	0x70, 0x57, 0x33BA}, {
	0x6E, 0x57, 0x33BB}, {
	0x6D, 0x57, 0x33BD}, {
	0x6B, 0x57, 0x33BE}, {
	0x4D, 0x57, 0x33BF}, {
	0x42, 0x71, 0x33C3}, {
	0x63, 0x63, 0x33C4}, {
	0x63, 0x64, 0x33C5}, {
	0x64, 0x42, 0x33C8}, {
	0x47, 0x79, 0x33C9}, {
	0x68, 0x61, 0x33CA}, {
	0x48, 0x50, 0x33CB}, {
	0x69, 0x6E, 0x33CC}, {
	0x4B, 0x4B, 0x33CD}, {
	0x4B, 0x4D, 0x33CE}, {
	0x6B, 0x74, 0x33CF}, {
	0x6C, 0x6D, 0x33D0}, {
	0x6C, 0x6E, 0x33D1}, {
	0x6C, 0x78, 0x33D3}, {
	0x6D, 0x62, 0x33D4}, {
	0x50, 0x48, 0x33D7}, {
	0x50, 0x52, 0x33DA}, {
	0x73, 0x72, 0x33DB}, {
	0x53, 0x76, 0x33DC}, {
	0x57, 0x62, 0x33DD}, {
	0x66, 0x66, 0xFB00}, {
	0x66, 0x69, 0xFB01}, {
	0x66, 0x6C, 0xFB02}, {
	0x73, 0x74, 0xFB06}, {
	0, 0, 0}
    }, *c;

    static int recurse = 0;
    int nc = -1;

    for (c = composetbl; c->first; c++) {
	if (c->first == first && c->second == second)
	    return c->composed;
    }

    if (recurse == 0) {
	recurse = 1;
	nc = check_compose(second, first);
	if (nc == -1)
	    nc = check_compose(toupper(first), toupper(second));
	if (nc == -1)
	    nc = check_compose(toupper(second), toupper(first));
	recurse = 0;
    }
    return nc;
}

int decode_codepage(char *cp_name)
{
    char *s, *d;
    struct cp_list_item *cpi;
    int codepage = -1;
    CPINFO cpinfo;

    if (!*cp_name) {
	/*
	 * Here we select a plausible default code page based on
	 * the locale the user is in. We wish to select an ISO code
	 * page or appropriate local default _rather_ than go with
	 * the Win125* series, because it's more important to have
	 * CSI and friends enabled by default than the ghastly
	 * Windows extra quote characters, and because it's more
	 * likely the user is connecting to a remote server that
	 * does something Unixy or VMSy and hence standards-
	 * compliant than that they're connecting back to a Windows
	 * box using horrible nonstandard charsets.
	 * 
	 * Accordingly, Robert de Bath suggests a method for
	 * picking a default character set that runs as follows:
	 * first call GetACP to get the system's ANSI code page
	 * identifier, and translate as follows:
	 * 
	 * 1250 -> ISO 8859-2
	 * 1251 -> KOI8-U
	 * 1252 -> ISO 8859-1
	 * 1253 -> ISO 8859-7
	 * 1254 -> ISO 8859-9
	 * 1255 -> ISO 8859-8
	 * 1256 -> ISO 8859-6
	 * 1257 -> ISO 8859-4
	 * 
	 * and for anything else, choose direct-to-font.
	 */
	int cp = GetACP();
	switch (cp) {
	  case 1250: cp_name = "ISO-8859-2"; break;
	  case 1251: cp_name = "KOI8-U"; break;
	  case 1252: cp_name = "ISO-8859-1"; break;
	  case 1253: cp_name = "ISO-8859-7"; break;
	  case 1254: cp_name = "ISO-8859-9"; break;
	  case 1255: cp_name = "ISO-8859-8"; break;
	  case 1256: cp_name = "ISO-8859-6"; break;
	  case 1257: cp_name = "ISO-8859-4"; break;
	    /* default: leave it blank, which will select -1, direct->font */
	}
    }

    if (cp_name && *cp_name)
	for (cpi = cp_list; cpi->name; cpi++) {
	    s = cp_name;
	    d = cpi->name;
	    for (;;) {
		while (*s && !isalnum(*s) && *s != ':')
		    s++;
		while (*d && !isalnum(*d) && *d != ':')
		    d++;
		if (*s == 0) {
		    codepage = cpi->codepage;
		    if (codepage == CP_UTF8)
			goto break_break;
		    if (codepage == 0) {
			codepage = 65536 + (cpi - cp_list);
			goto break_break;
		    }

		    if (GetCPInfo(codepage, &cpinfo) != 0)
			goto break_break;
		}
		if (tolower(*s++) != tolower(*d++))
		    break;
	    }
	}

    if (cp_name && *cp_name) {
	d = cp_name;
	if (tolower(d[0]) == 'c' && tolower(d[1]) == 'p')
	    d += 2;
	if (tolower(d[0]) == 'i' && tolower(d[1]) == 'b'
	    && tolower(d[1]) == 'm')
	    d += 3;
	for (s = d; *s >= '0' && *s <= '9'; s++);
	if (*s == 0 && s != d)
	    codepage = atoi(d);	       /* CP999 or IBM999 */

	if (codepage == CP_ACP)
	    codepage = GetACP();
	if (codepage == CP_OEMCP)
	    codepage = GetOEMCP();
	if (codepage > 65535)
	    codepage = -2;
    }

  break_break:;
    if (codepage != -1) {
	if (codepage != CP_UTF8 && codepage < 65536) {
	    if (GetCPInfo(codepage, &cpinfo) == 0) {
		codepage = -2;
	    } else if (cpinfo.MaxCharSize > 1)
		codepage = -3;
	}
    }
    if (codepage == -1 && *cp_name)
	codepage = -2;
    return codepage;
}

char *cp_name(int codepage)
{
    struct cp_list_item *cpi, *cpno;
    static char buf[32];

    if (codepage == -1) {
	sprintf(buf, "Use font encoding");
	return buf;
    }

    if (codepage > 0 && codepage < 65536)
	sprintf(buf, "CP%03d", codepage);
    else
	*buf = 0;

    if (codepage >= 65536) {
	cpno = 0;
	for (cpi = cp_list; cpi->name; cpi++)
	    if (cpi == cp_list + (codepage - 65536)) {
		cpno = cpi;
		break;
	    }
	if (cpno)
	    for (cpi = cp_list; cpi->name; cpi++) {
		if (cpno->cp_table == cpi->cp_table)
		    return cpi->name;
	    }
    } else {
	for (cpi = cp_list; cpi->name; cpi++) {
	    if (codepage == cpi->codepage)
		return cpi->name;
	}
    }
    return buf;
}

/*
 * Return the nth code page in the list, for use in the GUI
 * configurer.
 */
char *cp_enumerate(int index)
{
    if (index < 0 || index >= lenof(cp_list))
	return NULL;
    return cp_list[index].name;
}

static void get_unitab(int codepage, wchar_t * unitab, int ftype)
{
    char tbuf[4];
    int i, max = 256, flg = MB_ERR_INVALID_CHARS;

    if (ftype)
	flg |= MB_USEGLYPHCHARS;
    if (ftype == 2)
	max = 128;

    if (codepage == CP_UTF8)
	codepage = 28591;
    else if (codepage == CP_ACP)
	codepage = GetACP();
    else if (codepage == CP_OEMCP)
	codepage = GetOEMCP();

    if (codepage > 0 && codepage < 65536) {
	for (i = 0; i < max; i++) {
	    tbuf[0] = i;

	    if (MultiByteToWideChar(codepage, flg, tbuf, 1, unitab + i, 1)
		!= 1)
		unitab[i] = 0xFFFD;
	}
    } else {
	int j = 256 - cp_list[codepage & 0xFFFF].cp_size;
	for (i = 0; i < max; i++)
	    unitab[i] = i;
	for (i = j; i < max; i++)
	    unitab[i] = cp_list[codepage & 0xFFFF].cp_table[i - j];
    }
}
