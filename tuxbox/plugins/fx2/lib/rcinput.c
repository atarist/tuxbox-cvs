/*
** initial coding by fx2
*/

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <rcinput.h>
#include <draw.h>

static	int				fd = -1;
static	int				kbfd = -1;
		unsigned short	realcode=0xee;
		unsigned short	actcode=0xee;
		int				doexit=0;
		int				debug=1;

#define Debug	if(debug)printf

#ifdef i386
#define RC_IOCTL_BCODES 0
#else
#include <dbox/fp.h>
#endif

static	struct termios	tios;

void	KbInitialize( void )
{
	struct termios	ntios;

#ifndef __i386__
	return;
#endif

	kbfd = 0;

	if ( tcgetattr(kbfd,&tios) == -1 )
	{
		kbfd=-1;
		return;
	}
	memset(&ntios,0,sizeof(ntios));
	if ( tcsetattr(kbfd,TCSANOW,&ntios) == -1 )
	{
		kbfd=-1;
		return;
	}

	return;
}

static	unsigned short kb_translate( unsigned char c )
{
	switch(c)
	{
	case 0x41 :
		return RC_UP;
	case 0x42 :
		return RC_DOWN;
	case 0x43 :
		return RC_RIGHT;
	case 0x44 :
		return RC_LEFT;
	}
	return 0;
}

void		KbGetActCode( void )
{
	unsigned char	buf[256];
	int				x=0;
	int				left;
	unsigned short	code = 0;
	unsigned char	*p = buf;

	realcode=0xee;

	if ( kbfd != -1 )
		x = read(kbfd,buf,256);
	if ( x>0 )
	{
		for(p=buf, left=x; left; left--,p++)
		{
			switch(*p)
			{
			case 0x1b :
				if ( left >= 3 )
				{
					p+=2;
					code = kb_translate(*p);
					if ( code )
						actcode = code;
					left-=2;
				}
				else
					left=1;
				break;
			case 0x03 :
				doexit=3;
				break;
			case 0x0d :
				actcode=RC_OK;
				break;
#if 0
			case 0x1c :
				FBPrintScreen();
				break;
#endif
			case '?' :
				actcode=RC_HELP;
				break;
			case 'b' :
				actcode=RC_BLUE;
				break;
			case 'r' :
				actcode=RC_RED;
				break;
			case 'g' :
				actcode=RC_GREEN;
				break;
			case 'y' :
				actcode=RC_YELLOW;
				break;
			case '0' :
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
				actcode=*p-48;
				break;
			case 'q' :
				actcode=RC_SPKR;
				FBPause();
				break;
			}
		}
		realcode=actcode;
	}
}

void	KbClose( void )
{
	if ( kbfd != -1 )
		tcsetattr(kbfd,TCSANOW,&tios);
}

static	int		fd_is_ext = 0;

int	RcInitialize( int extfd )
{
	char	buf[32];

	KbInitialize();
	if ( extfd == -1 )
	{
		fd_is_ext = 0;
		fd = open( "/dev/dbox/rc0", O_RDONLY );
		if ( fd == -1 )
		{
			return kbfd;
		}
		fcntl(fd, F_SETFL, O_NONBLOCK );
		ioctl(fd, RC_IOCTL_BCODES, 1);
	}
	else
	{
		fd_is_ext = 1;
		fd = extfd;
		fcntl(fd, F_SETFL, O_NONBLOCK );
	}

/* clear rc-buffer */
	read( fd, buf, 32 );

	return 0;
}

static	unsigned short translate( unsigned short code )
{
	if ((code&0xFF00)==0x5C00)
	{
		switch (code&0xFF)
		{
		case 0x0C: return RC_STANDBY;
		case 0x20: return RC_HOME;
		case 0x27: return RC_SETUP;
		case 0x00: return RC_0;
		case 0x01: return RC_1;
		case 0x02: return RC_2;
		case 0x03: return RC_3;
		case 0x04: return RC_4;
		case 0x05: return RC_5;
		case 0x06: return RC_6;
		case 0x07: return RC_7;
		case 0x08: return RC_8;
		case 0x09: return RC_9;
		case 0x3B: return RC_BLUE;
		case 0x52: return RC_YELLOW;
		case 0x55: return RC_GREEN;
		case 0x2D: return RC_RED;
		case 0x54: return RC_PAGE_UP;
		case 0x53: return RC_PAGE_DOWN;
		case 0x0E: return RC_UP;
 		case 0x0F: return RC_DOWN;
		case 0x2F: return RC_LEFT;
 		case 0x2E: return RC_RIGHT;
		case 0x30: return RC_OK;
 		case 0x16: return RC_PLUS;
 		case 0x17: return RC_MINUS;
 		case 0x28: return RC_SPKR;
 		case 0x82: return RC_HELP;
		default:
			//perror("unknown old rc code");
			return 0xee;
		}
	} else if (!(code&0x00))
		return code&0x3F;
	return 0xee;
}

void		RcGetActCode( void )
{
	char			buf[32];
	int				x=0;
	unsigned short	code = 0;
static  unsigned short cw=0;

	if ( fd != -1 )
		x = read( fd, buf, 32 );
	if ( x < 2 )
	{
		KbGetActCode();
		if ( realcode == 0xee )
		{
			if ( cw == 1 )
				cw=0;
		}
		return;
	}

	Debug("%d bytes from FB received ...\n",x);

	x-=2;

	memcpy(&code,buf+x,2);

	code = translate(code);

	realcode=code;

	if ( code == 0xee )
		return;

	Debug("code=%04x\n",code);

	if ( cw == 2 )
	{
		actcode=code;
		return;
	}

	switch(code)
	{
#if 0
	case RC_HELP:
		if ( !cw )
			FBPrintScreen();
		cw=1;
		break;
#endif
	case RC_SPKR:
		if ( !cw )
		{
			cw=2;
			FBPause();
			cw=0;
		}
		break;
	case RC_HOME:
		doexit=3;
		break;
#if 0
	case RC_UP:
	case RC_DOWN:
	case RC_RIGHT:
	case RC_LEFT:
	case RC_OK:
#endif
	default :
		cw=0;
		actcode=code;
		break;
	}

	return;
}

void	RcClose( void )
{
	KbClose();
	if ( fd == -1 )
		return;
	if ( !fd_is_ext )
		close(fd);
}
