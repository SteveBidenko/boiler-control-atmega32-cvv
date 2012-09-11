#ifndef _BITS_DEFINED_
#define _BITS_DEFINED_
// Установки
#define BIT(x) (1 << (x))							//
#define SETBITS(x,y) ((x) |= (y))					// Установить несколько бит
#define CLEARBITS(x,y) ((x) &= (~(y)))				// Снять несколько бит
#define SETBIT(x,y) SETBITS((x), (BIT((y))))		// Установить 1 бит
#define CLEARBIT(x,y) CLEARBITS((x), (BIT((y))))	// Снять 1 бит
// Проверки
#define BITSET(x,y) ((x) & (BIT(y)))				// Установлен ли бит
#define BITCLEAR(x,y) !BITSET((x), (y))				// Снят ли бит
#define BITSSET(x,y) (((x) & (y)) == (y))			// Установлены ли несколько бит
#define BITSCLEAR(x,y) (((x) & (y)) == 0)			// Сняты ли несколько бит
#define BITVAL(x,y) (((x)>>(y)) & 1)
#endif