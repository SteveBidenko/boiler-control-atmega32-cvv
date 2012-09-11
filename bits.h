#ifndef _BITS_DEFINED_
#define _BITS_DEFINED_
// ���������
#define BIT(x) (1 << (x))							//
#define SETBITS(x,y) ((x) |= (y))					// ���������� ��������� ���
#define CLEARBITS(x,y) ((x) &= (~(y)))				// ����� ��������� ���
#define SETBIT(x,y) SETBITS((x), (BIT((y))))		// ���������� 1 ���
#define CLEARBIT(x,y) CLEARBITS((x), (BIT((y))))	// ����� 1 ���
// ��������
#define BITSET(x,y) ((x) & (BIT(y)))				// ���������� �� ���
#define BITCLEAR(x,y) !BITSET((x), (y))				// ���� �� ���
#define BITSSET(x,y) (((x) & (y)) == (y))			// ����������� �� ��������� ���
#define BITSCLEAR(x,y) (((x) & (y)) == 0)			// ����� �� ��������� ���
#define BITVAL(x,y) (((x)>>(y)) & 1)
#endif