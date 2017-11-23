/**
*Buzz Chess Engine
*movegen.c
*
*Copyright (C) 2007 Pradu Kannan. All rights reserved.
**/

#include "movegen.h"
#include "bitinstructions.h"

#ifdef abs
	#undef abs
#endif

#define abs(x) ((x)>=0?(x):-(x))

const U64 Pmoves[2][64]=
{
	{
		C64(0x0000000000000100), C64(0x0000000000000200), C64(0x0000000000000400), C64(0x0000000000000800),
		C64(0x0000000000001000), C64(0x0000000000002000), C64(0x0000000000004000), C64(0x0000000000008000),
		C64(0x0000000000010000), C64(0x0000000000020000), C64(0x0000000000040000), C64(0x0000000000080000),
		C64(0x0000000000100000), C64(0x0000000000200000), C64(0x0000000000400000), C64(0x0000000000800000),
		C64(0x0000000001000000), C64(0x0000000002000000), C64(0x0000000004000000), C64(0x0000000008000000),
		C64(0x0000000010000000), C64(0x0000000020000000), C64(0x0000000040000000), C64(0x0000000080000000),
		C64(0x0000000100000000), C64(0x0000000200000000), C64(0x0000000400000000), C64(0x0000000800000000),
		C64(0x0000001000000000), C64(0x0000002000000000), C64(0x0000004000000000), C64(0x0000008000000000),
		C64(0x0000010000000000), C64(0x0000020000000000), C64(0x0000040000000000), C64(0x0000080000000000),
		C64(0x0000100000000000), C64(0x0000200000000000), C64(0x0000400000000000), C64(0x0000800000000000),
		C64(0x0001000000000000), C64(0x0002000000000000), C64(0x0004000000000000), C64(0x0008000000000000),
		C64(0x0010000000000000), C64(0x0020000000000000), C64(0x0040000000000000), C64(0x0080000000000000), 
		C64(0x0100000000000000), C64(0x0200000000000000), C64(0x0400000000000000), C64(0x0800000000000000),
		C64(0x1000000000000000), C64(0x2000000000000000), C64(0x4000000000000000), C64(0x8000000000000000),
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000),
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000)
	},
	{
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000),
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000),
		C64(0x0000000000000001), C64(0x0000000000000002), C64(0x0000000000000004), C64(0x0000000000000008),
		C64(0x0000000000000010), C64(0x0000000000000020), C64(0x0000000000000040), C64(0x0000000000000080),
		C64(0x0000000000000100), C64(0x0000000000000200), C64(0x0000000000000400), C64(0x0000000000000800),
		C64(0x0000000000001000), C64(0x0000000000002000), C64(0x0000000000004000), C64(0x0000000000008000),
		C64(0x0000000000010000), C64(0x0000000000020000), C64(0x0000000000040000), C64(0x0000000000080000),
		C64(0x0000000000100000), C64(0x0000000000200000), C64(0x0000000000400000), C64(0x0000000000800000),
		C64(0x0000000001000000), C64(0x0000000002000000), C64(0x0000000004000000), C64(0x0000000008000000),
		C64(0x0000000010000000), C64(0x0000000020000000), C64(0x0000000040000000), C64(0x0000000080000000),
		C64(0x0000000100000000), C64(0x0000000200000000), C64(0x0000000400000000), C64(0x0000000800000000),
		C64(0x0000001000000000), C64(0x0000002000000000), C64(0x0000004000000000), C64(0x0000008000000000),
		C64(0x0000010000000000), C64(0x0000020000000000), C64(0x0000040000000000), C64(0x0000080000000000),
		C64(0x0000100000000000), C64(0x0000200000000000), C64(0x0000400000000000), C64(0x0000800000000000),
		C64(0x0001000000000000), C64(0x0002000000000000), C64(0x0004000000000000), C64(0x0008000000000000),
		C64(0x0010000000000000), C64(0x0020000000000000), C64(0x0040000000000000), C64(0x0080000000000000)
	}
};

const U64 Pcaps[2][64]=
{
	{
		C64(0x0000000000000200), C64(0x0000000000000500), C64(0x0000000000000A00), C64(0x0000000000001400),
		C64(0x0000000000002800), C64(0x0000000000005000), C64(0x000000000000A000), C64(0x0000000000004000),
		C64(0x0000000000020000), C64(0x0000000000050000), C64(0x00000000000A0000), C64(0x0000000000140000),
		C64(0x0000000000280000), C64(0x0000000000500000), C64(0x0000000000A00000), C64(0x0000000000400000),
		C64(0x0000000002000000), C64(0x0000000005000000), C64(0x000000000A000000), C64(0x0000000014000000),
		C64(0x0000000028000000), C64(0x0000000050000000), C64(0x00000000A0000000), C64(0x0000000040000000),
		C64(0x0000000200000000), C64(0x0000000500000000), C64(0x0000000A00000000), C64(0x0000001400000000),
		C64(0x0000002800000000), C64(0x0000005000000000), C64(0x000000A000000000), C64(0x0000004000000000),
		C64(0x0000020000000000), C64(0x0000050000000000), C64(0x00000A0000000000), C64(0x0000140000000000),
		C64(0x0000280000000000), C64(0x0000500000000000), C64(0x0000A00000000000), C64(0x0000400000000000),
		C64(0x0002000000000000), C64(0x0005000000000000), C64(0x000A000000000000), C64(0x0014000000000000),
		C64(0x0028000000000000), C64(0x0050000000000000), C64(0x00A0000000000000), C64(0x0040000000000000),
		C64(0x0200000000000000), C64(0x0500000000000000), C64(0x0A00000000000000), C64(0x1400000000000000),
		C64(0x2800000000000000), C64(0x5000000000000000), C64(0xA000000000000000), C64(0x4000000000000000),
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000),
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000)
	},
	{
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000),
		C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000), C64(0x0000000000000000),
		C64(0x0000000000000002), C64(0x0000000000000005), C64(0x000000000000000A), C64(0x0000000000000014),
		C64(0x0000000000000028), C64(0x0000000000000050), C64(0x00000000000000A0), C64(0x0000000000000040),
		C64(0x0000000000000200), C64(0x0000000000000500), C64(0x0000000000000A00), C64(0x0000000000001400),
		C64(0x0000000000002800), C64(0x0000000000005000), C64(0x000000000000A000), C64(0x0000000000004000),
		C64(0x0000000000020000), C64(0x0000000000050000), C64(0x00000000000A0000), C64(0x0000000000140000),
		C64(0x0000000000280000), C64(0x0000000000500000), C64(0x0000000000A00000), C64(0x0000000000400000),
		C64(0x0000000002000000), C64(0x0000000005000000), C64(0x000000000A000000), C64(0x0000000014000000),
		C64(0x0000000028000000), C64(0x0000000050000000), C64(0x00000000A0000000), C64(0x0000000040000000),
		C64(0x0000000200000000), C64(0x0000000500000000), C64(0x0000000A00000000), C64(0x0000001400000000),
		C64(0x0000002800000000), C64(0x0000005000000000), C64(0x000000A000000000), C64(0x0000004000000000),
		C64(0x0000020000000000), C64(0x0000050000000000), C64(0x00000A0000000000), C64(0x0000140000000000),
		C64(0x0000280000000000), C64(0x0000500000000000), C64(0x0000A00000000000), C64(0x0000400000000000),
		C64(0x0002000000000000), C64(0x0005000000000000), C64(0x000A000000000000), C64(0x0014000000000000),
		C64(0x0028000000000000), C64(0x0050000000000000), C64(0x00A0000000000000), C64(0x0040000000000000)
	}
};

const U64 Nmoves[64]=
{
	C64(0x0000000000020400), C64(0x0000000000050800), C64(0x00000000000A1100), C64(0x0000000000142200),
	C64(0x0000000000284400), C64(0x0000000000508800), C64(0x0000000000A01000), C64(0x0000000000402000),
	C64(0x0000000002040004), C64(0x0000000005080008), C64(0x000000000A110011), C64(0x0000000014220022),
	C64(0x0000000028440044), C64(0x0000000050880088), C64(0x00000000A0100010), C64(0x0000000040200020),
	C64(0x0000000204000402), C64(0x0000000508000805), C64(0x0000000A1100110A), C64(0x0000001422002214),
	C64(0x0000002844004428), C64(0x0000005088008850), C64(0x000000A0100010A0), C64(0x0000004020002040),
	C64(0x0000020400040200), C64(0x0000050800080500), C64(0x00000A1100110A00), C64(0x0000142200221400),
	C64(0x0000284400442800), C64(0x0000508800885000), C64(0x0000A0100010A000), C64(0x0000402000204000),
	C64(0x0002040004020000), C64(0x0005080008050000), C64(0x000A1100110A0000), C64(0x0014220022140000),
	C64(0x0028440044280000), C64(0x0050880088500000), C64(0x00A0100010A00000), C64(0x0040200020400000),
	C64(0x0204000402000000), C64(0x0508000805000000), C64(0x0A1100110A000000), C64(0x1422002214000000),
	C64(0x2844004428000000), C64(0x5088008850000000), C64(0xA0100010A0000000), C64(0x4020002040000000),
	C64(0x0400040200000000), C64(0x0800080500000000), C64(0x1100110A00000000), C64(0x2200221400000000),
	C64(0x4400442800000000), C64(0x8800885000000000), C64(0x100010A000000000), C64(0x2000204000000000),
	C64(0x0004020000000000), C64(0x0008050000000000), C64(0x00110A0000000000), C64(0x0022140000000000),
	C64(0x0044280000000000), C64(0x0088500000000000), C64(0x0010A00000000000), C64(0x0020400000000000)
};

const U64 Kmoves[64]=
{
	C64(0x0000000000000302), C64(0x0000000000000705), C64(0x0000000000000E0A), C64(0x0000000000001C14),
	C64(0x0000000000003828), C64(0x0000000000007050), C64(0x000000000000E0A0), C64(0x000000000000C040),
	C64(0x0000000000030203), C64(0x0000000000070507), C64(0x00000000000E0A0E), C64(0x00000000001C141C),
	C64(0x0000000000382838), C64(0x0000000000705070), C64(0x0000000000E0A0E0), C64(0x0000000000C040C0),
	C64(0x0000000003020300), C64(0x0000000007050700), C64(0x000000000E0A0E00), C64(0x000000001C141C00),
	C64(0x0000000038283800), C64(0x0000000070507000), C64(0x00000000E0A0E000), C64(0x00000000C040C000),
	C64(0x0000000302030000), C64(0x0000000705070000), C64(0x0000000E0A0E0000), C64(0x0000001C141C0000),
	C64(0x0000003828380000), C64(0x0000007050700000), C64(0x000000E0A0E00000), C64(0x000000C040C00000),
	C64(0x0000030203000000), C64(0x0000070507000000), C64(0x00000E0A0E000000), C64(0x00001C141C000000),
	C64(0x0000382838000000), C64(0x0000705070000000), C64(0x0000E0A0E0000000), C64(0x0000C040C0000000),
	C64(0x0003020300000000), C64(0x0007050700000000), C64(0x000E0A0E00000000), C64(0x001C141C00000000),
	C64(0x0038283800000000), C64(0x0070507000000000), C64(0x00E0A0E000000000), C64(0x00C040C000000000),
	C64(0x0302030000000000), C64(0x0705070000000000), C64(0x0E0A0E0000000000), C64(0x1C141C0000000000),
	C64(0x3828380000000000), C64(0x7050700000000000), C64(0xE0A0E00000000000), C64(0xC040C00000000000),
	C64(0x0203000000000000), C64(0x0507000000000000), C64(0x0A0E000000000000), C64(0x141C000000000000),
	C64(0x2838000000000000), C64(0x5070000000000000), C64(0xA0E0000000000000), C64(0x40C0000000000000)
};

const U64 BmovesNoOcc[64]=
{
	C64(0x8040201008040200),C64(0x0080402010080500),C64(0x0000804020110A00),C64(0x0000008041221400),
	C64(0x0000000182442800),C64(0x0000010204885000),C64(0x000102040810A000),C64(0x0102040810204000),
	C64(0x4020100804020002),C64(0x8040201008050005),C64(0x00804020110A000A),C64(0x0000804122140014),
	C64(0x0000018244280028),C64(0x0001020488500050),C64(0x0102040810A000A0),C64(0x0204081020400040),
	C64(0x2010080402000204),C64(0x4020100805000508),C64(0x804020110A000A11),C64(0x0080412214001422),
	C64(0x0001824428002844),C64(0x0102048850005088),C64(0x02040810A000A010),C64(0x0408102040004020),
	C64(0x1008040200020408),C64(0x2010080500050810),C64(0x4020110A000A1120),C64(0x8041221400142241),
	C64(0x0182442800284482),C64(0x0204885000508804),C64(0x040810A000A01008),C64(0x0810204000402010),
	C64(0x0804020002040810),C64(0x1008050005081020),C64(0x20110A000A112040),C64(0x4122140014224180),
	C64(0x8244280028448201),C64(0x0488500050880402),C64(0x0810A000A0100804),C64(0x1020400040201008),
	C64(0x0402000204081020),C64(0x0805000508102040),C64(0x110A000A11204080),C64(0x2214001422418000),
	C64(0x4428002844820100),C64(0x8850005088040201),C64(0x10A000A010080402),C64(0x2040004020100804),
	C64(0x0200020408102040),C64(0x0500050810204080),C64(0x0A000A1120408000),C64(0x1400142241800000),
	C64(0x2800284482010000),C64(0x5000508804020100),C64(0xA000A01008040201),C64(0x4000402010080402),
	C64(0x0002040810204080),C64(0x0005081020408000),C64(0x000A112040800000),C64(0x0014224180000000),
	C64(0x0028448201000000),C64(0x0050880402010000),C64(0x00A0100804020100),C64(0x0040201008040201)
};

const U64 RmovesNoOcc[64]=
{
	C64(0x01010101010101FE),C64(0x02020202020202FD),C64(0x04040404040404FB),C64(0x08080808080808F7),
	C64(0x10101010101010EF),C64(0x20202020202020DF),C64(0x40404040404040BF),C64(0x808080808080807F),
	C64(0x010101010101FE01),C64(0x020202020202FD02),C64(0x040404040404FB04),C64(0x080808080808F708),
	C64(0x101010101010EF10),C64(0x202020202020DF20),C64(0x404040404040BF40),C64(0x8080808080807F80),
	C64(0x0101010101FE0101),C64(0x0202020202FD0202),C64(0x0404040404FB0404),C64(0x0808080808F70808),
	C64(0x1010101010EF1010),C64(0x2020202020DF2020),C64(0x4040404040BF4040),C64(0x80808080807F8080),
	C64(0x01010101FE010101),C64(0x02020202FD020202),C64(0x04040404FB040404),C64(0x08080808F7080808),
	C64(0x10101010EF101010),C64(0x20202020DF202020),C64(0x40404040BF404040),C64(0x808080807F808080),
	C64(0x010101FE01010101),C64(0x020202FD02020202),C64(0x040404FB04040404),C64(0x080808F708080808),
	C64(0x101010EF10101010),C64(0x202020DF20202020),C64(0x404040BF40404040),C64(0x8080807F80808080),
	C64(0x0101FE0101010101),C64(0x0202FD0202020202),C64(0x0404FB0404040404),C64(0x0808F70808080808),
	C64(0x1010EF1010101010),C64(0x2020DF2020202020),C64(0x4040BF4040404040),C64(0x80807F8080808080),
	C64(0x01FE010101010101),C64(0x02FD020202020202),C64(0x04FB040404040404),C64(0x08F7080808080808),
	C64(0x10EF101010101010),C64(0x20DF202020202020),C64(0x40BF404040404040),C64(0x807F808080808080),
	C64(0xFE01010101010101),C64(0xFD02020202020202),C64(0xFB04040404040404),C64(0xF708080808080808),
	C64(0xEF10101010101010),C64(0xDF20202020202020),C64(0xBF40404040404040),C64(0x7F80808080808080)
};

U64 inBetween[64][64];
U64 lineOf[64][64];

//Fills of Bishop, Rook, and Knight moves for moves around the king
const U64 KNFill[64]=
{
        C64(0x00000000070D0B0E), C64(0x000000000F1F171D), C64(0x000000001F3F2E3B), C64(0x000000003E7F5D77),
        C64(0x000000007CFEBAEE), C64(0x00000000F8FC74DC), C64(0x00000000F0F8E8B8), C64(0x00000000E0B0D070),
        C64(0x000000070D0F0E0F), C64(0x0000000F1F1F1D1F), C64(0x0000001F3F3F3B3F), C64(0x0000003E7F7F777F),
        C64(0x0000007CFEFEEEFE), C64(0x000000F8FCFCDCFC), C64(0x000000F0F8F8B8F8), C64(0x000000E0B0F070F0),
        C64(0x0000070D0F0E0F0D), C64(0x00000F1F1F1D1F1F), C64(0x00001F3F3F3B3F3F), C64(0x00003E7F7F777F7F),
        C64(0x00007CFEFEEEFEFE), C64(0x0000F8FCFCDCFCFC), C64(0x0000F0F8F8B8F8F8), C64(0x0000E0B0F070F0B0),
        C64(0x00070D0F0E0F0D07), C64(0x000F1F1F1D1F1F0F), C64(0x001F3F3F3B3F3F1F), C64(0x003E7F7F777F7F3E),
        C64(0x007CFEFEEEFEFE7C), C64(0x00F8FCFCDCFCFCF8), C64(0x00F0F8F8B8F8F8F0), C64(0x00E0B0F070F0B0E0),
        C64(0x070D0F0E0F0D0700), C64(0x0F1F1F1D1F1F0F00), C64(0x1F3F3F3B3F3F1F00), C64(0x3E7F7F777F7F3E00),
        C64(0x7CFEFEEEFEFE7C00), C64(0xF8FCFCDCFCFCF800), C64(0xF0F8F8B8F8F8F000), C64(0xE0B0F070F0B0E000),
        C64(0x0D0F0E0F0D070000), C64(0x1F1F1D1F1F0F0000), C64(0x3F3F3B3F3F1F0000), C64(0x7F7F777F7F3E0000),
        C64(0xFEFEEEFEFE7C0000), C64(0xFCFCDCFCFCF80000), C64(0xF8F8B8F8F8F00000), C64(0xB0F070F0B0E00000),
        C64(0x0F0E0F0D07000000), C64(0x1F1D1F1F0F000000), C64(0x3F3B3F3F1F000000), C64(0x7F777F7F3E000000),
        C64(0xFEEEFEFE7C000000), C64(0xFCDCFCFCF8000000), C64(0xF8B8F8F8F0000000), C64(0xF070F0B0E0000000),
        C64(0x0E0B0D0700000000), C64(0x1D171F0F00000000), C64(0x3B2E3F1F00000000), C64(0x775D7F3E00000000),
        C64(0xEEBAFE7C00000000), C64(0xDC74FCF800000000), C64(0xB8E8F8F000000000), C64(0x70D0B0E000000000)
};
const U64 KBFill[64]=
{
        C64(0xC0E070381C0F0707), C64(0xC0E0F0783D1F0F0F), C64(0x80C0E0F17B3F1F1F), C64(0x0080C1E3F77F3E3E),
        C64(0x000183C7EFFE7C7C), C64(0x0103078FDEFCF8F8), C64(0x03070F1EBCF8F0F0), C64(0x03070E1C38F0E0E0),
        C64(0xE0F0783C1F0F070F), C64(0xE0F0F87D3F1F0F1F), C64(0xC0E0F1FB7F3F1F3F), C64(0x80C1E3F7FF7F3E7F),
        C64(0x0183C7EFFFFE7CFE), C64(0x03078FDFFEFCF8FC), C64(0x070F1FBEFCF8F0F8), C64(0x070F1E3CF8F0E0F0),
        C64(0xF0783C1F0F070F1F), C64(0xF0F87D3F1F0F1F3F), C64(0xE0F1FB7F3F1F3F7F), C64(0xC1E3F7FF7F3E7FFF),
        C64(0x83C7EFFFFE7CFEFF), C64(0x078FDFFEFCF8FCFE), C64(0x0F1FBEFCF8F0F8FC), C64(0x0F1E3CF8F0E0F0F8),
        C64(0x783C1F0F070F1F3C), C64(0xF87D3F1F0F1F3F7D), C64(0xF1FB7F3F1F3F7FFB), C64(0xE3F7FF7F3E7FFFF7),
        C64(0xC7EFFFFE7CFEFFEF), C64(0x8FDFFEFCF8FCFEDF), C64(0x1FBEFCF8F0F8FCBE), C64(0x1E3CF8F0E0F0F83C),
        C64(0x3C1F0F070F1F3C78), C64(0x7D3F1F0F1F3F7DF8), C64(0xFB7F3F1F3F7FFBF1), C64(0xF7FF7F3E7FFFF7E3),
        C64(0xEFFFFE7CFEFFEFC7), C64(0xDFFEFCF8FCFEDF8F), C64(0xBEFCF8F0F8FCBE1F), C64(0x3CF8F0E0F0F83C1E),
        C64(0x1F0F070F1F3C78F0), C64(0x3F1F0F1F3F7DF8F0), C64(0x7F3F1F3F7FFBF1E0), C64(0xFF7F3E7FFFF7E3C1),
        C64(0xFFFE7CFEFFEFC783), C64(0xFEFCF8FCFEDF8F07), C64(0xFCF8F0F8FCBE1F0F), C64(0xF8F0E0F0F83C1E0F),
        C64(0x0F070F1F3C78F0E0), C64(0x1F0F1F3F7DF8F0E0), C64(0x3F1F3F7FFBF1E0C0), C64(0x7F3E7FFFF7E3C180),
        C64(0xFE7CFEFFEFC78301), C64(0xFCF8FCFEDF8F0703), C64(0xF8F0F8FCBE1F0F07), C64(0xF0E0F0F83C1E0F07),
        C64(0x07070F1C3870E0C0), C64(0x0F0F1F3D78F0E0C0), C64(0x1F1F3F7BF1E0C080), C64(0x3E3E7FF7E3C18000),
        C64(0x7C7CFEEFC7830100), C64(0xF8F8FCDE8F070301), C64(0xF0F0F8BC1E0F0703), C64(0xE0E0F0381C0E0703)
};
const U64 KRFill[64]=
{
        C64(0x030303030303FFFF), C64(0x070707070707FFFF), C64(0x0E0E0E0E0E0EFFFF), C64(0x1C1C1C1C1C1CFFFF),
        C64(0x383838383838FFFF), C64(0x707070707070FFFF), C64(0xE0E0E0E0E0E0FFFF), C64(0xC0C0C0C0C0C0FFFF),
        C64(0x0303030303FFFFFF), C64(0x0707070707FFFFFF), C64(0x0E0E0E0E0EFFFFFF), C64(0x1C1C1C1C1CFFFFFF),
        C64(0x3838383838FFFFFF), C64(0x7070707070FFFFFF), C64(0xE0E0E0E0E0FFFFFF), C64(0xC0C0C0C0C0FFFFFF),
        C64(0x03030303FFFFFF03), C64(0x07070707FFFFFF07), C64(0x0E0E0E0EFFFFFF0E), C64(0x1C1C1C1CFFFFFF1C),
        C64(0x38383838FFFFFF38), C64(0x70707070FFFFFF70), C64(0xE0E0E0E0FFFFFFE0), C64(0xC0C0C0C0FFFFFFC0),
        C64(0x030303FFFFFF0303), C64(0x070707FFFFFF0707), C64(0x0E0E0EFFFFFF0E0E), C64(0x1C1C1CFFFFFF1C1C),
        C64(0x383838FFFFFF3838), C64(0x707070FFFFFF7070), C64(0xE0E0E0FFFFFFE0E0), C64(0xC0C0C0FFFFFFC0C0),
        C64(0x0303FFFFFF030303), C64(0x0707FFFFFF070707), C64(0x0E0EFFFFFF0E0E0E), C64(0x1C1CFFFFFF1C1C1C),
        C64(0x3838FFFFFF383838), C64(0x7070FFFFFF707070), C64(0xE0E0FFFFFFE0E0E0), C64(0xC0C0FFFFFFC0C0C0),
        C64(0x03FFFFFF03030303), C64(0x07FFFFFF07070707), C64(0x0EFFFFFF0E0E0E0E), C64(0x1CFFFFFF1C1C1C1C),
        C64(0x38FFFFFF38383838), C64(0x70FFFFFF70707070), C64(0xE0FFFFFFE0E0E0E0), C64(0xC0FFFFFFC0C0C0C0),
        C64(0xFFFFFF0303030303), C64(0xFFFFFF0707070707), C64(0xFFFFFF0E0E0E0E0E), C64(0xFFFFFF1C1C1C1C1C),
        C64(0xFFFFFF3838383838), C64(0xFFFFFF7070707070), C64(0xFFFFFFE0E0E0E0E0), C64(0xFFFFFFC0C0C0C0C0),
        C64(0xFFFF030303030303), C64(0xFFFF070707070707), C64(0xFFFF0E0E0E0E0E0E), C64(0xFFFF1C1C1C1C1C1C),
        C64(0xFFFF383838383838), C64(0xFFFF707070707070), C64(0xFFFFE0E0E0E0E0E0), C64(0xFFFFC0C0C0C0C0C0)
};
const U64 KQFill[64]=
{
        C64(0xC3E3733B1F0FFFFF), C64(0xC7E7F77F3F1FFFFF), C64(0x8ECEEEFF7F3FFFFF), C64(0x1C9CDDFFFF7FFFFF),
        C64(0x3839BBFFFFFEFFFF), C64(0x717377FFFEFCFFFF), C64(0xE3E7EFFEFCF8FFFF), C64(0xC3C7CEDCF8F0FFFF),
        C64(0xE3F37B3F1FFFFFFF), C64(0xE7F7FF7F3FFFFFFF), C64(0xCEEEFFFF7FFFFFFF), C64(0x9CDDFFFFFFFFFFFF),
        C64(0x39BBFFFFFFFFFFFF), C64(0x7377FFFFFEFFFFFF), C64(0xE7EFFFFEFCFFFFFF), C64(0xC7CFDEFCF8FFFFFF),
        C64(0xF37B3F1FFFFFFF1F), C64(0xF7FF7F3FFFFFFF3F), C64(0xEEFFFF7FFFFFFF7F), C64(0xDDFFFFFFFFFFFFFF),
        C64(0xBBFFFFFFFFFFFFFF), C64(0x77FFFFFEFFFFFFFE), C64(0xEFFFFEFCFFFFFFFC), C64(0xCFDEFCF8FFFFFFF8),
        C64(0x7B3F1FFFFFFF1F3F), C64(0xFF7F3FFFFFFF3F7F), C64(0xFFFF7FFFFFFF7FFF), C64(0xFFFFFFFFFFFFFFFF),
        C64(0xFFFFFFFFFFFFFFFF), C64(0xFFFFFEFFFFFFFEFF), C64(0xFFFEFCFFFFFFFCFE), C64(0xDEFCF8FFFFFFF8FC),
        C64(0x3F1FFFFFFF1F3F7B), C64(0x7F3FFFFFFF3F7FFF), C64(0xFF7FFFFFFF7FFFFF), C64(0xFFFFFFFFFFFFFFFF),
        C64(0xFFFFFFFFFFFFFFFF), C64(0xFFFEFFFFFFFEFFFF), C64(0xFEFCFFFFFFFCFEFF), C64(0xFCF8FFFFFFF8FCDE),
        C64(0x1FFFFFFF1F3F7BF3), C64(0x3FFFFFFF3F7FFFF7), C64(0x7FFFFFFF7FFFFFEE), C64(0xFFFFFFFFFFFFFFDD),
        C64(0xFFFFFFFFFFFFFFBB), C64(0xFEFFFFFFFEFFFF77), C64(0xFCFFFFFFFCFEFFEF), C64(0xF8FFFFFFF8FCDECF),
        C64(0xFFFFFF1F3F7BF3E3), C64(0xFFFFFF3F7FFFF7E7), C64(0xFFFFFF7FFFFFEECE), C64(0xFFFFFFFFFFFFDD9C),
        C64(0xFFFFFFFFFFFFBB39), C64(0xFFFFFFFEFFFF7773), C64(0xFFFFFFFCFEFFEFE7), C64(0xFFFFFFF8FCDECFC7),
        C64(0xFFFF0F1F3B73E3C3), C64(0xFFFF1F3F7FF7E7C7), C64(0xFFFF3F7FFFEECE8E), C64(0xFFFF7FFFFFDD9C1C),
        C64(0xFFFFFEFFFFBB3938), C64(0xFFFFFCFEFF777371), C64(0xFFFFF8FCFEEFE7E3), C64(0xFFFFF0F8DCCEC7C3)
};

void initMoveGen()
{
	initmagicmoves();

	//Initializing inBetween
	{
		int i;
		int j;
		for(i=0;i<64;i++)
			for(j=0;j<64;j++)
			{
				if(i==j)
					inBetween[i][j]=U64EMPTY;
				else if(ROW(i)==ROW(j))
				{
					U64 tempBB=i>j?toBit(i):toBit(j);
					while(!((tempBB&toBit(i)) && tempBB&toBit(j)))
						tempBB|=tempBB>>1;
					tempBB^=toBit(i)|toBit(j);
					inBetween[i][j]=tempBB;
				}
				else if(COL(i)==COL(j))
				{
					U64 tempBB=i>j?toBit(i):toBit(j);
					while(!((tempBB&toBit(i)) && tempBB&toBit(j)))
						tempBB|=tempBB>>8;
					tempBB^=toBit(i)|toBit(j);
					inBetween[i][j]=tempBB;
				}
				else if(abs(COL(i)-COL(j))==abs(ROW(i)-ROW(j)))
				{
					U64 tempBB=i>j?toBit(i):toBit(j);
					int dir;
					if(((COL(i)-COL(j))<0 && (ROW(i)-ROW(j))<0) || ((COL(i)-COL(j))>0 && (ROW(i)-ROW(j))>0))
						dir=9;
					else
						dir=7;
					while(!((tempBB&toBit(i)) && tempBB&toBit(j)))
						tempBB|=tempBB>>dir;
					tempBB^=toBit(i)|toBit(j);
					inBetween[i][j]=tempBB;
				}
				else
					inBetween[i][j]=U64EMPTY;
			}
	}

	//Initializing lineOf
	{
		int i;
		int j;
		for(i=0;i<64;i++)
			for(j=0;j<64;j++)
			{
				if(i==j)
					lineOf[i][j]=U64EMPTY;
				else if(ROW(i)==ROW(j) || COL(i)==COL(j))
				{
					lineOf[i][j]=(RmovesNoOcc(i)&RmovesNoOcc(j))|toBit(i)|toBit(j);
				}
				else if(abs(COL(i)-COL(j))==abs(ROW(i)-ROW(j)))
				{
					lineOf[i][j]=(BmovesNoOcc(i)&BmovesNoOcc(j))|toBit(i)|toBit(j);
				}
				else
					lineOf[i][j]=U64EMPTY;
			}
	}
}

//generates legal moves
void genMoves(const board* pos, moveList* list)
{
	U64 pieceboard;
	U64 pinned;
	static const U64 castlingPassover[2][2]=
	{
		{((U64)0x3)<<5,((U64)0x3)<<2},
		{((U64)0x3)<<(56+5),((U64)0x3)<<(56+2)}
	};
	static const U64 castlingEmptySquares[2][2]=
	{
		{((U64)0x3)<<5,((U64)0x7)<<1},
		{((U64)0x3)<<(56+5),((U64)0x7)<<(56+1)}
	};
	U64 piecesCurrentSide;
	list->moveCount=0;
	
	//piecesCurrentSide is used as a temporary here
	if((piecesCurrentSide=inCheck(*pos,pos->side)))
	{
		genEvasions(pos,list,piecesCurrentSide);
		return;
	}

	piecesCurrentSide=pos->PiecesSide[pos->side];
	pinned=possiblePinned(pos,pos->side);

	//Castling moves
	{
		unsigned int temp;
		if((temp=canCastle(*pos,pos->side)) && !inCheck(*pos,pos->side))
		{
			U64 squares;
			//if Kingside available and pieces clear
			if(temp&1 && !(castlingEmptySquares[pos->side][KINGSIDE] & (pos->AllPieces)))
			{
				squares=castlingPassover[pos->side][KINGSIDE];
				//optimize this
				while(squares)
				{
					unsigned int sq;
					GetBitAndClear(squares,sq);
					if(isAttacked(*pos,sq,pos->side)) goto kdone;
				}
				list->moves[list->moveCount].m=KingsideCastle;
				list->moveCount++;
			}
	kdone:
			//if Queenside available and pieces clear
			if(temp&2 && !(castlingEmptySquares[pos->side][QUEENSIDE]&(pos->AllPieces)))
			{
				squares=castlingPassover[pos->side][QUEENSIDE];
				//optimize this
				while(squares)
				{
					unsigned int sq;
					GetBitAndClear(squares,sq);
					if(isAttacked(*pos,sq,pos->side)) goto castlingdone;
				}
				list->moves[list->moveCount].m=QueensideCastle;
				list->moveCount++;
			}
		}
	}
	castlingdone:
	
	//EP moves
	if(pos->EP)
	{
		assert((pos->EP>=16 && pos->EP<24) || (pos->EP>=40 && pos->EP<48));
		pieceboard=Pcaps[pos->xside][pos->EP]&pos->Pieces[P]&piecesCurrentSide;
		while(pieceboard)
		{
			unsigned int from;
			GetBitAndClear(pieceboard,from);
			if((!(pinned&toBit(from) && !(lineOf(from,pos->KingPos[pos->side])&toBit(pos->EP)))) && /*regular pin*/
				/*pinned when removed ep*/
					!(
						/*not pinned when removed ep, rook directions*/
						(Rmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
						||
						/*not pinned when removed the ep, bishop directions*/
						(Bmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
					)
				)
				list->moves[list->moveCount++].m=encodeTo(pos->EP)|encodePiece(P)|encodeEP(pos->side?pos->EP+8:pos->EP-8)|encodeFrom(from)|encodePromotion(E)|encodeCapture(P);
		}
	}

	//Pawn Moves
	pieceboard=pos->Pieces[P]&piecesCurrentSide&~pinned;
	//Double moves (not pinned)
	{
		static const U64 doubleP[2] = {C64(0x000000000000FF00), C64(0x00FF000000000000)}; /*initial rank*/
		U64 pieceboard2=(pieceboard&doubleP[pos->side]);
		if(pieceboard2)
		{
			if(pos->side) //Black
			{
				pieceboard2>>=8;
				pieceboard2&=~pos->AllPieces;
				pieceboard2>>=8;
				pieceboard2&=~pos->AllPieces;
				while(pieceboard2)
				{
					unsigned int to;
					GetBitAndClear(pieceboard2,to);
					list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+16)|encodeTo(to)|encodeCapture(E)|encodePromotion(E);
					list->moveCount++;
				}
			}
			else
			{
				pieceboard2<<=8;
				pieceboard2&=~pos->AllPieces;
				pieceboard2<<=8;
				pieceboard2&=~pos->AllPieces;
				while(pieceboard2)
				{
					unsigned int to;
					GetBitAndClear(pieceboard2,to);
					list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-16)|encodeTo(to)|encodeCapture(E)|encodePromotion(E);
					list->moveCount++;
				}
			}
		}
	}
	pieceboard=pos->Pieces[P]&piecesCurrentSide&pinned;
	//Double moves (pinned)
	{
		static const U64 doubleP[2] = {C64(0x000000000000FF00), C64(0x00FF000000000000)}; /*initial rank*/
		U64 pieceboard2=(pieceboard&doubleP[pos->side]);
		if(pieceboard2)
		{
			if(pos->side) //Black
			{
				pieceboard2>>=8;
				pieceboard2&=~pos->AllPieces;
				pieceboard2>>=8;
				pieceboard2&=~pos->AllPieces;
				while(pieceboard2)
				{
					unsigned int to;
					GetBitAndClear(pieceboard2,to);
					if(lineOf(to+16,pos->KingPos[pos->side])&toBit(to))
						list->moves[list->moveCount++].m=encodePiece(P)|encodeFrom(to+16)|encodeTo(to)|encodeCapture(E)|encodePromotion(E);
				}
			}
			else
			{
				pieceboard2<<=8;
				pieceboard2&=~pos->AllPieces;
				pieceboard2<<=8;
				pieceboard2&=~pos->AllPieces;
				while(pieceboard2)
				{
					unsigned int to;
					GetBitAndClear(pieceboard2,to);
					if(lineOf(to-16,pos->KingPos[pos->side])&toBit(to))
						list->moves[list->moveCount++].m=encodePiece(P)|encodeFrom(to-16)|encodeTo(to)|encodeCapture(E)|encodePromotion(E);
				}
			}
		}
	}

	//Single Moves (not pinned)
	pieceboard=pos->Pieces[P]&piecesCurrentSide&~pinned;
	if(pos->side) //Black
	{
		U64 pieceboard2=(pieceboard>>8)&~pos->AllPieces;
		U64 pieceboardCap1=((pieceboard>>7)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard>>9)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		while(pieceboard2)
		{
			unsigned int to;
			GetBitAndClear(pieceboard2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+8)|encodeTo(to)|encodeCapture(E);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+9 )|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}
	else //White
	{
		U64 pieceboard2=(pieceboard<<8)&~pos->AllPieces;
		U64 pieceboardCap1=((pieceboard<<9)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard<<7)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		while(pieceboard2)
		{
			unsigned int to;
			GetBitAndClear(pieceboard2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-8)|encodeTo(to)|encodeCapture(E);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}

	//Single Moves (pinned)
	pieceboard=pos->Pieces[P]&piecesCurrentSide&pinned;
	if(pos->side) //Black
	{
		U64 pieceboard2=(pieceboard>>8)&~pos->AllPieces;
		U64 pieceboardCap1=((pieceboard>>7)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard>>9)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		while(pieceboard2)
		{
			unsigned int to;
			GetBitAndClear(pieceboard2, to);
			if(!(lineOf(to+8,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+8)|encodeTo(to)|encodeCapture(E);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			if(!(lineOf(to+7,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			if(!(lineOf(to+9,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}
	else //White
	{
		U64 pieceboard2=(pieceboard<<8)&~pos->AllPieces;
		U64 pieceboardCap1=((pieceboard<<9)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard<<7)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		while(pieceboard2)
		{
			unsigned int to;
			GetBitAndClear(pieceboard2, to);
			if(!(lineOf(to-8,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-8)|encodeTo(to)|encodeCapture(E);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			if(!(lineOf(to-9,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			if(!(lineOf(to-7,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}

	//Regular moves (first not pinned, then pinned for each piece)
	pieceboard=pos->Pieces[N]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Nmoves(from)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(N)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[B]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Bmoves(from,pos->AllPieces)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(B)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[B]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Bmoves(from,pos->AllPieces)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(B)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	pieceboard=pos->Pieces[R]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Rmoves(from,/*pos->AllPieces*/0);
		moves=Rmoves(from,pos->AllPieces)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(R)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[R]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Rmoves(from,/*pos->AllPieces*/0);
		moves=Rmoves(from,pos->AllPieces)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(R)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	pieceboard=pos->Pieces[Q]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Qmoves(from,pos->AllPieces)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(Q)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[Q]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Qmoves(from,pos->AllPieces)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(Q)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}

	//only 1 king
	#ifdef KING_MOVEGEN1
	{
		unsigned int from;
		U64 moves;
		move temp;
		from=pos->KingPos[pos->side];
		moves=Kmoves(from)&~(pos->PiecesSide[pos->side]);
		temp=encodeFrom(from)|encodePiece(K)|encodePromotion(E);
		pieceboard=pos->AllPieces^toBit(from); //this will be used as the occupancy

		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(!isAttackedOcc(*pos,to,pos->side,pieceboard))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	#endif
	
	#ifdef KING_MOVEGEN2
	{
		unsigned int from;
		U64 moves;
		U64 andout=pos->PiecesSide[pos->side];
		U64 attackers;
		move temp;
		from=pos->KingPos[pos->side];
		temp=encodeFrom(from)|encodePiece(K)|encodePromotion(E);
		pieceboard=pos->AllPieces^toBit(from); //this will be used as the occupancy
		//King attackers
		andout|=Kmoves(pos->KingPos[pos->xside]);
		//Pawn attackers
		if(pos->xside) //if white to move
		{
			U64 pblack = piecesBLACK(*pos,P);
			andout|=((pblack&C64(0x7F7F7F7F7F7F7F7F))>>7)|((pblack&C64(0xFEFEFEFEFEFEFEFE))>>9);
		}
		else //black to move
		{
			U64 pwhite = piecesWHITE(*pos,P);
			andout|=((pwhite&C64(0x7F7F7F7F7F7F7F7F))<<9)|((pwhite&C64(0xFEFEFEFEFEFEFEFE))<<7);
		}
		//Diagonal attackers
		attackers = KBFill[from]&(pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[pos->xside];
		while(attackers)
		{
			unsigned int sq;
			GetBitAndClear(attackers,sq);
			andout|=Bmoves(sq,pieceboard);
		}
		//Rook attackers
		attackers = KRFill[from]&(pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[pos->xside];
		while(attackers)
		{
			unsigned int sq;
			GetBitAndClear(attackers,sq);
			andout|=Rmoves(sq,pieceboard);
		}
		//Knight attackers
		attackers = KNFill[from]&piecesXSide(*pos,N);
		while(attackers)
		{
			unsigned int sq;
			GetBitAndClear(attackers,sq);
			andout|=Nmoves(sq);
		}
		//Generate king moves
		moves=Kmoves(from)&(~andout);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	#endif
}


//generates legal evasions
void genEvasions(const board* pos, moveList* list, U64 checkers)
{
	U64 pinned;
	U64 attackers;
	unsigned int from;
	unsigned int to;
	U64 moves;
	move tempm;

	list->moveCount=0;

	//generate king moves
	#ifdef KING_MOVEGEN1
	from=pos->KingPos[pos->side];
	moves=Kmoves(from)&~(pos->PiecesSide[pos->side]);
	tempm=encodeFrom(from)|encodePiece(K)|encodePromotion(E);
	attackers=pos->AllPieces^toBit(from); //this will be used as the occupancy
	while(moves)
	{
		GetBitAndClear(moves,to);
		if(!isAttackedOcc(*pos,to,pos->side,attackers))
			list->moves[list->moveCount++].m=tempm|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
	}
	#endif
	#ifdef KING_MOVEGEN2
	{
		U64 pieceboard;
		U64 andout=pos->PiecesSide[pos->side];
		U64 attackers;

		from=pos->KingPos[pos->side];
		tempm=encodeFrom(from)|encodePiece(K)|encodePromotion(E);
		pieceboard=pos->AllPieces^toBit(from); //this will be used as the occupancy
		//King attackers
		andout|=Kmoves(pos->KingPos[pos->xside]);
		//Pawn attackers
		if(pos->xside) //if white to move
		{
			U64 pblack = piecesBLACK(*pos,P);
			andout|=((pblack&C64(0x7F7F7F7F7F7F7F7F))>>7)|((pblack&C64(0xFEFEFEFEFEFEFEFE))>>9);
		}
		else //black to move
		{
			U64 pwhite = piecesWHITE(*pos,P);
			andout|=((pwhite&C64(0x7F7F7F7F7F7F7F7F))<<9)|((pwhite&C64(0xFEFEFEFEFEFEFEFE))<<7);
		}
		//Diagonal attackers
		attackers = KBFill[from]&(pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[pos->xside];
		while(attackers)
		{
			unsigned int sq;
			GetBitAndClear(attackers,sq);
			andout|=Bmoves(sq,pieceboard);
		}
		//Rook attackers
		attackers = KRFill[from]&(pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[pos->xside];
		while(attackers)
		{
			unsigned int sq;
			GetBitAndClear(attackers,sq);
			andout|=Rmoves(sq,pieceboard);
		}
		//Knight attackers
		attackers = KNFill[from]&piecesXSide(*pos,N);
		while(attackers)
		{
			unsigned int sq;
			GetBitAndClear(attackers,sq);
			andout|=Nmoves(sq);
		}
		//Generate king moves
		moves=Kmoves(from)&(~andout);
		while(moves)
		{
			GetBitAndClear(moves,to);
			list->moves[list->moveCount++].m=tempm|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	#endif

	//make sure there is only 1 checker
	if(checkers&(checkers-1)) return;
	
	pinned=possiblePinned(pos, pos->side);

	//generate attacks to the checker
	to=toIndex(checkers);
	
	//pawns
	//enpassant

	//EP moves
	if(pos->EP && (pos->side?((pos->EP+((unsigned int)8))==to):((to+((unsigned int)8))==pos->EP)))
	{
		U64 pieceboard=Pcaps[pos->xside][pos->EP]&piecesSide(*pos,P);
		while(pieceboard)
		{
			unsigned int from;
			GetBitAndClear(pieceboard,from);
			if((!(pinned&toBit(from) && !(lineOf(from,pos->KingPos[pos->side])&toBit(pos->EP)))) && /*regular pin*/
				/*pinned when removed ep*/
					!(
						/*not pinned when removed ep, rook directions*/
						(Rmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
						||
						/*not pinned when removed the ep, bishop directions*/
						(Bmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
					)
				)
				list->moves[list->moveCount++].m=encodeTo(pos->EP)|encodePiece(P)|encodeEP(pos->side?pos->EP+8:pos->EP-8)|encodeFrom(from)|encodePromotion(E)|encodeCapture(P);
		}
	}



	//regular
	attackers=attacksToP(*pos,to,pos->side)&~pinned;
	tempm=encodeTo(to)|encodePiece(P);
	while(attackers)
	{
		GetBitAndClear(attackers,from);
		list->moves[list->moveCount].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
		if(ROW(to)==7 || ROW(to)==0) //Promotion
		{
			move temp=list->moves[list->moveCount].m;
			list->moves[list->moveCount].m|=encodePromotion(Q);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(R);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(B);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(N);
		}
		else
			list->moves[list->moveCount].m|=encodePromotion(E);
		list->moveCount++;
	}
	attackers=attacksToN(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
	tempm=encodeTo(to)|encodePiece(N)|encodePromotion(E);
	while(attackers)
	{
		GetBitAndClear(attackers,from);
		list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
	}
	attackers=attacksToB(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
	tempm=encodeTo(to)|encodePiece(B)|encodePromotion(E);
	while(attackers)
	{
		GetBitAndClear(attackers,from);
		list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
	}
	attackers=attacksToR(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
	tempm=encodeTo(to)|encodePiece(R)|encodePromotion(E);
	while(attackers)
	{
		GetBitAndClear(attackers,from);
		list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
	}
	attackers=attacksToQ(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
	tempm=encodeTo(to)|encodePiece(Q)|encodePromotion(E);
	while(attackers)
	{
		GetBitAndClear(attackers,from);
		list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
	}

	//generate all blocking moves
	{
		U64 pieceboard;
		moves=inBetween(pos->KingPos[pos->side],to);
		//Pawn Moves

		//Double pawn moves
		pieceboard=piecesSide(*pos,P)&~pinned;
		{
			static const U64 doubleP[2] = {C64(0x000000000000FF00), C64(0x00FF000000000000)}; /*initial rank*/
			U64 pieceboard2=(pieceboard&doubleP[pos->side]);
			if(pieceboard2)
			{
				if(pos->side) //Black
				{
					pieceboard2>>=8;
					pieceboard2&=~pos->AllPieces;
					pieceboard2>>=8;
					pieceboard2&=moves;
					while(pieceboard2)
					{
						unsigned int to;
						GetBitAndClear(pieceboard2,to);
						list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+16)|encodeTo(to)|encodeCapture(E)|encodePromotion(E);
						list->moveCount++;
					}
				}
				else
				{
					pieceboard2<<=8;
					pieceboard2&=~pos->AllPieces;
					pieceboard2<<=8;
					pieceboard2&=moves;
					while(pieceboard2)
					{
						unsigned int to;
						GetBitAndClear(pieceboard2,to);
						list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-16)|encodeTo(to)|encodeCapture(E)|encodePromotion(E);
						list->moveCount++;
					}
				}
			}
		}

		//regular pawn moves
		if(pos->side) //Black
		{
			U64 pieceboard2=(pieceboard>>8)&moves;
			while(pieceboard2)
			{
				unsigned int to;
				GetBitAndClear(pieceboard2, to);
				list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+8)|encodeTo(to)|encodeCapture(E);
				if(ROW(to)==0)
				{
					move temp=list->moves[list->moveCount].m;
					list->moves[list->moveCount].m|=encodePromotion(Q);
					list->moveCount++;
					list->moves[list->moveCount].m=temp|encodePromotion(R);
					list->moveCount++;
					list->moves[list->moveCount].m=temp|encodePromotion(B);
					list->moveCount++;
					list->moves[list->moveCount].m=temp|encodePromotion(N);
				}
				else
					list->moves[list->moveCount].m|=encodePromotion(E);
				list->moveCount++;
			}
		}
		else //White
		{
			U64 pieceboard2=(pieceboard<<8)&moves;
			while(pieceboard2)
			{
				unsigned int to;
				GetBitAndClear(pieceboard2, to);
				list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-8)|encodeTo(to)|encodeCapture(E);
				if(ROW(to)==7)
				{
					move temp=list->moves[list->moveCount].m;
					list->moves[list->moveCount].m|=encodePromotion(Q);
					list->moveCount++;
					list->moves[list->moveCount].m=temp|encodePromotion(R);
					list->moveCount++;
					list->moves[list->moveCount].m=temp|encodePromotion(B);
					list->moveCount++;
					list->moves[list->moveCount].m=temp|encodePromotion(N);
				}
				else
					list->moves[list->moveCount].m|=encodePromotion(E);
				list->moveCount++;
			}
		}

		//regular piece moves
		while(moves)
		{
			GetBitAndClear(moves,to);
			attackers=attacksToN(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
			tempm=encodeTo(to)|encodePiece(N)|encodePromotion(E);
			while(attackers)
			{
				GetBitAndClear(attackers,from);
				list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
			}
			attackers=attacksToB(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
			tempm=encodeTo(to)|encodePiece(B)|encodePromotion(E);
			while(attackers)
			{
				GetBitAndClear(attackers,from);
				list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
			}
			attackers=attacksToR(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
			tempm=encodeTo(to)|encodePiece(R)|encodePromotion(E);
			while(attackers)
			{
				GetBitAndClear(attackers,from);
				list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
			}
			attackers=attacksToQ(*pos,to)&pos->PiecesSide[pos->side]&~pinned;
			tempm=encodeTo(to)|encodePiece(Q)|encodePromotion(E);
			while(attackers)
			{
				GetBitAndClear(attackers,from);
				list->moves[list->moveCount++].m=tempm|encodeFrom(from)|encodeCapture(pos->PieceTypes[to]);
			}
		}

	} //generating blocking moves (end)
}


//Generate legal moves for quiescence search only when not in check
//Generates captures and promotions only
void genQMoves(const board* pos, moveList* list)
{
	U64 pieceboard;
	U64 pinned;
	U64 piecesCurrentSide;
	list->moveCount=0;
	
	
	//no check evasions from genQMoves
	/*if(piecesCurrentSide=inCheck(*pos,pos->side))
	{
		genEvasions(pos,list,piecesCurrentSide);
		return;
	}*/
	assert(!inCheck(*pos,pos->side));

	piecesCurrentSide=pos->PiecesSide[pos->side];
	pinned=possiblePinned(pos,pos->side);
	
	//EP moves
	if(pos->EP)
	{
		assert((pos->EP>=16 && pos->EP<24) || (pos->EP>=40 && pos->EP<48));
		pieceboard=Pcaps[pos->xside][pos->EP]&pos->Pieces[P]&piecesCurrentSide;
		while(pieceboard)
		{
			unsigned int from;
			GetBitAndClear(pieceboard,from);
			if((!(pinned&toBit(from) && !(lineOf(from,pos->KingPos[pos->side])&toBit(pos->EP)))) && /*regular pin*/
				/*pinned when removed ep*/
					!(
						/*not pinned when removed ep, rook directions*/
						(Rmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
						||
						/*not pinned when removed the ep, bishop directions*/
						(Bmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
					)
				)
				list->moves[list->moveCount++].m=encodeTo(pos->EP)|encodePiece(P)|encodeEP(pos->side?pos->EP+8:pos->EP-8)|encodeFrom(from)|encodePromotion(E)|encodeCapture(P);
		}
	}

	//Pawn Moves
	//Single Moves (not pinned)
	pieceboard=pos->Pieces[P]&piecesCurrentSide&~pinned;
	if(pos->side) //Black
	{
		U64 pieceboardCap1=((pieceboard>>7)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard>>9)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		U64 promotion = (pieceboard>>8)&0xFF&(~pos->AllPieces);
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				move temp=list->moves[list->moveCount].m;
				#endif
				list->moves[list->moveCount].m|=encodePromotion(Q);
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
				#endif
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+9 )|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				move temp=list->moves[list->moveCount].m;
				#endif
				list->moves[list->moveCount].m|=encodePromotion(Q);
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
				#endif
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(promotion)
		{
			unsigned int to;
			move temp;
			GetBitAndClear(promotion, to);
			temp=encodePiece(P)|encodeFrom(to+8)|encodeTo(to)|encodeCapture(E);
			list->moves[list->moveCount].m=temp|encodePromotion(Q);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(R);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(B);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(N);
			list->moveCount++;
		}
	}
	else //White
	{
		U64 pieceboardCap1=((pieceboard<<9)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard<<7)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		U64 promotion = (pieceboard<<8)&C64(0xFF00000000000000)&(~pos->AllPieces);
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(promotion)
		{
			unsigned int to;
			move temp;
			GetBitAndClear(promotion, to);
			temp=encodePiece(P)|encodeFrom(to-8)|encodeTo(to)|encodeCapture(E);
			list->moves[list->moveCount].m=temp|encodePromotion(Q);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(R);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(B);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(N);
			list->moveCount++;
		}
	}

	//Single Moves (pinned)
	pieceboard=pos->Pieces[P]&piecesCurrentSide&pinned;
	if(pos->side) //Black
	{
		U64 pieceboardCap1=((pieceboard>>7)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard>>9)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			if(!(lineOf(to+7,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			if(!(lineOf(to+9,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}
	else //White
	{
		U64 pieceboardCap1=((pieceboard<<9)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard<<7)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			if(!(lineOf(to-9,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			if(!(lineOf(to-7,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}

	//Regular moves (first not pinned, then pinned for each piece)
	pieceboard=pos->Pieces[N]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Nmoves(from)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(N)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[B]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Bmoves(from,pos->AllPieces)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(B)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[B]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Bmoves(from,pos->AllPieces)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(B)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	pieceboard=pos->Pieces[R]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Rmoves(from,/*pos->AllPieces*/0);
		moves=Rmoves(from,pos->AllPieces)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(R)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[R]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Rmoves(from,/*pos->AllPieces*/0);
		moves=Rmoves(from,pos->AllPieces)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(R)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	pieceboard=pos->Pieces[Q]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Qmoves(from,pos->AllPieces)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(Q)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[Q]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Qmoves(from,pos->AllPieces)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(Q)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}

	//only 1 king
	{
		unsigned int from;
		U64 moves;
		move temp;
		from=pos->KingPos[pos->side];
		moves=Kmoves(from)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(K)|encodePromotion(E);
		pieceboard=pos->AllPieces^toBit(from); //this will be used as the occupancy
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(!isAttackedOcc(*pos,to,pos->side,pieceboard))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
}

//Generate legal moves for quiescence search only when not in check
//Generates captures, promotions and direct checks
void genQCMoves(const board* pos, moveList* list)
{
	U64 pieceboard;
	U64 pinned;
	U64 piecesCurrentSide;
	U64 checkSquares[5];
	list->moveCount=0;
	
	//no check evasions from genQMoves
	/*if(piecesCurrentSide=inCheck(*pos,pos->side))
	{
		genEvasions(pos,list,piecesCurrentSide);
		return;
	}*/

	assert(!inCheck(*pos,pos->side));

	piecesCurrentSide=pos->PiecesSide[pos->side];
	pinned=possiblePinned(pos,pos->side);

	/*Generate checking squares*/
	checkSquares[P] = Pcaps(pos->KingPos[pos->xside],pos->xside)&~pos->PiecesSide[pos->side];
	checkSquares[N] = Nmoves(pos->KingPos[pos->xside])&~pos->PiecesSide[pos->side];
	checkSquares[B] = Bmoves(pos->KingPos[pos->xside],pos->AllPieces)&~pos->PiecesSide[pos->side];
	checkSquares[R] = Rmoves(pos->KingPos[pos->xside],pos->AllPieces)&~pos->PiecesSide[pos->side];
	checkSquares[Q] = checkSquares[B] | checkSquares[R];

	//EP moves
	if(pos->EP)
	{
		assert((pos->EP>=16 && pos->EP<24) || (pos->EP>=40 && pos->EP<48));
		pieceboard=Pcaps[pos->xside][pos->EP]&pos->Pieces[P]&piecesCurrentSide;
		while(pieceboard)
		{
			unsigned int from;
			GetBitAndClear(pieceboard,from);
			if((!(pinned&toBit(from) && !(lineOf(from,pos->KingPos[pos->side])&toBit(pos->EP)))) && /*regular pin*/
				/*pinned when removed ep*/
					!(
						/*not pinned when removed ep, rook directions*/
						(Rmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
						||
						/*not pinned when removed the ep, bishop directions*/
						(Bmoves(pos->KingPos[pos->side],pos->AllPieces^(toBit(pos->side?pos->EP+8:pos->EP-8)|toBit(from)|toBit(pos->EP))))
						&
						((pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[pos->xside])
					)
				)
				list->moves[list->moveCount++].m=encodeTo(pos->EP)|encodePiece(P)|encodeEP(pos->side?pos->EP+8:pos->EP-8)|encodeFrom(from)|encodePromotion(E)|encodeCapture(P);
		}
	}

	//Pawn Moves
	//Single Moves (not pinned)
	pieceboard=pos->Pieces[P]&piecesCurrentSide&~pinned;
	if(pos->side) //Black
	{
		U64 pieceboardCap1=((pieceboard>>7)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard>>9)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		U64 promotion = (pieceboard>>8)&0xFF&(~pos->AllPieces);
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				move temp=list->moves[list->moveCount].m;
				#endif
				list->moves[list->moveCount].m|=encodePromotion(Q);
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
				#endif
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+9 )|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				move temp=list->moves[list->moveCount].m;
				#endif
				list->moves[list->moveCount].m|=encodePromotion(Q);
				#ifndef QSEARCH_QPROMOTIONS_ONLY
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
				#endif
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(promotion)
		{
			unsigned int to;
			move temp;
			GetBitAndClear(promotion, to);
			temp=encodePiece(P)|encodeFrom(to+8)|encodeTo(to)|encodeCapture(E);
			list->moves[list->moveCount].m=temp|encodePromotion(Q);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(R);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(B);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(N);
			list->moveCount++;
		}
	}
	else //White
	{
		U64 pieceboardCap1=((pieceboard<<9)&C64(0xFEFEFEFEFEFEFEFE))&pos->PiecesSide[pos->xside];
		U64 pieceboardCap2=((pieceboard<<7)&C64(0x7F7F7F7F7F7F7F7F))&pos->PiecesSide[pos->xside];
		U64 promotion = (pieceboard<<8)&C64(0xFF00000000000000)&(~pos->AllPieces);
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(promotion)
		{
			unsigned int to;
			move temp;
			GetBitAndClear(promotion, to);
			temp=encodePiece(P)|encodeFrom(to-8)|encodeTo(to)|encodeCapture(E);
			list->moves[list->moveCount].m=temp|encodePromotion(Q);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(R);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(B);
			list->moveCount++;
			list->moves[list->moveCount].m=temp|encodePromotion(N);
			list->moveCount++;
		}
	}

	//Single Moves (pinned)
	pieceboard=pos->Pieces[P]&piecesCurrentSide&pinned;
	if(pos->side) //Black
	{
		U64 pieceboardCap1=((pieceboard>>7)&C64(0xFEFEFEFEFEFEFEFE))&(pos->PiecesSide[pos->xside]|checkSquares[P]);
		U64 pieceboardCap2=((pieceboard>>9)&C64(0x7F7F7F7F7F7F7F7F))&(pos->PiecesSide[pos->xside]|checkSquares[P]);
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			if(!(lineOf(to+7,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			if(!(lineOf(to+9,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to+9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==0)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}
	else //White
	{
		U64 pieceboardCap1=((pieceboard<<9)&C64(0xFEFEFEFEFEFEFEFE))&(pos->PiecesSide[pos->xside]|checkSquares[P]);
		U64 pieceboardCap2=((pieceboard<<7)&C64(0x7F7F7F7F7F7F7F7F))&(pos->PiecesSide[pos->xside]|checkSquares[P]);
		while(pieceboardCap1)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap1, to);
			if(!(lineOf(to-9,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-9)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
		while(pieceboardCap2)
		{
			unsigned int to;
			GetBitAndClear(pieceboardCap2, to);
			if(!(lineOf(to-7,pos->KingPos[pos->side])&toBit(to))) continue;
			list->moves[list->moveCount].m=encodePiece(P)|encodeFrom(to-7)|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			if(ROW(to)==7)
			{
				move temp=list->moves[list->moveCount].m;
				list->moves[list->moveCount].m|=encodePromotion(Q);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(R);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(B);
				list->moveCount++;
				list->moves[list->moveCount].m=temp|encodePromotion(N);
			}
			else
				list->moves[list->moveCount].m|=encodePromotion(E);
			list->moveCount++;
		}
	}

	//Regular moves (first not pinned, then pinned for each piece)
	pieceboard=pos->Pieces[N]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Nmoves(from)&(pos->PiecesSide[pos->xside]|checkSquares[N]);
		temp=encodeFrom(from)|encodePiece(N)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[B]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Bmoves(from,pos->AllPieces)&(pos->PiecesSide[pos->xside]|checkSquares[B]);
		temp=encodeFrom(from)|encodePiece(B)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[B]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Bmoves(from,pos->AllPieces)&(pos->PiecesSide[pos->xside]|checkSquares[B]);
		temp=encodeFrom(from)|encodePiece(B)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	pieceboard=pos->Pieces[R]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Rmoves(from,/*pos->AllPieces*/0);
		moves=Rmoves(from,pos->AllPieces)&(pos->PiecesSide[pos->xside]|checkSquares[R]);
		temp=encodeFrom(from)|encodePiece(R)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[R]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Rmoves(from,/*pos->AllPieces*/0);
		moves=Rmoves(from,pos->AllPieces)&(pos->PiecesSide[pos->xside]|checkSquares[R]);
		temp=encodeFrom(from)|encodePiece(R)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
	pieceboard=pos->Pieces[Q]&piecesCurrentSide&~pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Qmoves(from,pos->AllPieces)&(pos->PiecesSide[pos->xside]|checkSquares[Q]);
		temp=encodeFrom(from)|encodePiece(Q)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			list->moves[list->moveCount].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
			list->moveCount++;
		}
	}
	pieceboard=pos->Pieces[Q]&piecesCurrentSide&pinned;
	while(pieceboard)
	{
		unsigned int from;
		U64 moves;
		move temp;
		GetBitAndClear(pieceboard,from);
		moves=Qmoves(from,pos->AllPieces)&(pos->PiecesSide[pos->xside]|checkSquares[Q]);
		temp=encodeFrom(from)|encodePiece(Q)|encodePromotion(E);
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(lineOf(from,pos->KingPos[pos->side])&toBit(to))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}

	//only 1 king
	{
		unsigned int from;
		U64 moves;
		move temp;
		from=pos->KingPos[pos->side];
		moves=Kmoves(from)&pos->PiecesSide[pos->xside];
		temp=encodeFrom(from)|encodePiece(K)|encodePromotion(E);
		pieceboard=pos->AllPieces^toBit(from); //this will be used as the occupancy
		while(moves)
		{
			unsigned int to;
			GetBitAndClear(moves,to);
			if(!isAttackedOcc(*pos,to,pos->side,pieceboard))
				list->moves[list->moveCount++].m=temp|encodeTo(to)|encodeCapture(pos->PieceTypes[to]);
		}
	}
}

//returns pieces that are possibly pinned, depending on direction of movement, to a side's king
U64 possiblePinned(const board* pos, const bool side)
{
	U64 ret=0;
	U64 possiblePinned;
	U64 pinners;

	//Diagonal pins
	//First get a list of own pieces that can be attacked by a slider from the king position
	possiblePinned=Bmoves(pos->KingPos[side],pos->AllPieces)&pos->PiecesSide[side];
	//xor the occupancy with the above bitboard to get a list of pinners
	pinners=Bmoves(pos->KingPos[side],pos->AllPieces^possiblePinned)
		& ((pos->Pieces[B]|pos->Pieces[Q])&pos->PiecesSide[!side]);
	//now that we have a list of pinners, we'll figure out the bitboard of pinned pieces
	//you won't go through this loop at all if there arn't any pinned pieces
	while(pinners)
	{
		unsigned int pinnerLocation;
		GetBitAndClear(pinners, pinnerLocation);
		ret|=(inBetween(pinnerLocation,pos->KingPos[side])&possiblePinned);
	}

	//Orthogonal pins
	possiblePinned=Rmoves(pos->KingPos[side],pos->AllPieces)&pos->PiecesSide[side];
	pinners=Rmoves(pos->KingPos[side],pos->AllPieces^possiblePinned)
		& ((pos->Pieces[R]|pos->Pieces[Q])&pos->PiecesSide[!side]);
	while(pinners)
	{
		unsigned int pinnerLocation;
		GetBitAndClear(pinners, pinnerLocation);
		ret|=(inBetween(pinnerLocation,pos->KingPos[side])&possiblePinned);
	}
	return ret;
}

bool moveIsLegal(const board* pos, const move m)
{
	moveList ml;
	unsigned int i;
	genMoves(pos,&ml);
	for(i=0;i<ml.moveCount;i++)
		if(ml.moves[i].m==m)
			return true;
	return false;
}

//Hung pieces - returns a bitboard of pieces that are clearly hung
//EG attacked by a lower valued piece or undefended
/*U64 hungPieces(const board* pos, const bool side)
{

}*/


//Fillled attacks - initial place included
U64 Rfill(U64 pieces, U64 occ)
{
	U64 kocc=(~occ)|pieces;
	return fillUpOccluded(pieces,kocc<<8)|fillDownOccluded(pieces,kocc>>8)
		   |fillRight(pieces,occ)|fillLeftOccluded(pieces,kocc>>1);
}

U64 Bfill(U64 pieces, U64 occ)
{
	occ=(~occ)|pieces;
	return fillUpRightOccluded(pieces,occ<<9)|fillDownLeftOccluded(pieces,occ>>9)
		   |fillUpLeftOccluded(pieces,occ<<7)|fillDownRightOccluded(pieces,occ>>7);
}

U64 Qfill(U64 pieces, U64 occ)
{
	U64 kocc=(~occ)|pieces;
	return fillUpOccluded(pieces,kocc<<8)|fillDownOccluded(pieces,kocc>>8)
		   |fillRight(pieces,occ)|fillLeftOccluded(pieces,kocc>>1)
		   |fillUpRightOccluded(pieces,kocc<<9)|fillDownLeftOccluded(pieces,kocc>>9)
		   |fillUpLeftOccluded(pieces,kocc<<7)|fillDownRightOccluded(pieces,kocc>>7);
}

U64 Nfill(U64 pieces)
{
	return
		pieces |
		//left 2 up 1
		( (pieces<< 6) & C64(0x3F3F3F3F3F3F3F3F) ) |
		//right 2 up 1
		( (pieces<<10) & C64(0xFCFCFCFCFCFCFCFC) ) |
		//left 1 up 2
		( (pieces<<15) & C64(0x7F7F7F7F7F7F7F7F) ) |
		//right 1 up 2
		( (pieces<<17) & C64(0xFEFEFEFEFEFEFEFE) ) |
		//left 2 down 1
		( (pieces>> 6) & C64(0xFCFCFCFCFCFCFCFC) ) |
		//right 2 down 1
		( (pieces>>10) & C64(0x3F3F3F3F3F3F3F3F) ) |
		//left 1 down 2
		( (pieces>>15) & C64(0xFEFEFEFEFEFEFEFE) ) |
		//right 1 down 2
		( (pieces>>17) & C64(0x7F7F7F7F7F7F7F7F) ) ;
}

U64 Kfill(U64 pieces)
{
	pieces |= (pieces << 8) | (pieces >> 8);
	return ((pieces<<1)&C64(0xFEFEFEFEFEFEFEFE)) | ((pieces>>1)&C64(0x7F7F7F7F7F7F7F7F));
}
