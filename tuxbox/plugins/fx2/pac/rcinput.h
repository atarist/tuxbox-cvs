#ifndef RC_INPUT_H
#define RC_INPUT_H


#define RC_0			0x00
#define RC_1			0x01
#define RC_2			0x02
#define RC_3			0x03
#define RC_4			0x04
#define RC_5			0x05
#define RC_6			0x06
#define RC_7			0x07
#define RC_8			0x08
#define RC_9			0x09
#define RC_RIGHT		0x0A
#define RC_LEFT			0x0B
#define RC_UP			0x0C
#define RC_DOWN			0x0D
#define RC_OK			0x0E
#define RC_SPKR			0x0F
#define RC_STANDBY		0x10
#define RC_GREEN		0x11
#define RC_YELLOW		0x12
#define RC_RED			0x13
#define RC_BLUE			0x14
#define RC_PLUS			0x15
#define RC_MINUS		0x16
#define RC_HELP			0x17
#define RC_SETUP		0x18
#define RC_HOME			0x1F
#define RC_PAGE_DOWN	0x53
#define RC_PAGE_UP		0x54

extern	void			RcGetActCode( void );
extern	void			RcInitialize( void );
extern	void			RcClose( void );

#endif
