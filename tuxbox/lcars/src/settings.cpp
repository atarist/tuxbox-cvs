/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <dbox/info.h>
#include <ost/ca.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ost/dmx.h>
#include <ost/video.h>
#include <ost/frontend.h>
#include <ost/audio.h>
#include <ost/sec.h>
#include <ost/sec.h>
#include <ost/ca.h>
#include <memory.h>

#include "settings.h"
#include "help.h"
#include "cam.h"

settings::settings(cam &c): ca(c)
{
	FILE *fp;
	char buffer[100];
	int type = -1;
	isGTX = false;

	printf("----------------> SETTINGS <--------------------\n");
	fp = fopen("/proc/bus/dbox", "r");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "fe=%d", &type);
		sscanf(buffer, "mID=%d", &box);



		int gtx = 0;
		sscanf(buffer, "gtxID=%x\n", &gtx);
		if (gtx != 0)
		{
			if ((unsigned int)gtx != 0xffffffff)
			{
				isGTX = true;
			}
			else
			{
				isGTX = false;
			}
		}
	
	}	
	fclose(fp);

	if (box == 3)
		printf ("Sagem-Box\n");
	else if (box == 1)
		printf("Nokia-Box\n");
	else if (box == 2)
		printf("Philips-Box\n");
	else
		printf("Unknown Box\n");
	
	isCable = (type == DBOX_FE_CABLE);

	CAID = ca.getCAID();
	printf("Set-CAID: %x\n", CAID);
	
	oldTS = -1;
	usediseqc = true;
	timeoffset = 60;
	
}

void settings::initme()
{

	
}

int settings::getEMMpid(int TS = -1)
{
	if (EMM < 2 || oldTS != TS || TS == -1)
	{
		printf("Getting EMM\n");
		EMM = find_emmpid(CAID);
		oldTS = TS;
	}
	return EMM;
}

int settings::find_emmpid(int ca_system_id) {
	char buffer[1000];
	int fd, r = 1000, count;
	struct dmxSctFilterParams flt;

	fd=open("/dev/ost/demux0", O_RDWR);
	if (fd<0)
	{
		perror("/dev/ost/demux0");
		return -fd;
	}

	memset(&flt.filter.filter, 0, DMX_FILTER_SIZE);
	memset(&flt.filter.mask, 0, DMX_FILTER_SIZE);

	flt.pid=1;
	flt.filter.filter[0]=1;
	flt.filter.mask[0]  =0xFF;
	flt.timeout=10000;
	flt.flags=DMX_ONESHOT;

	if (ioctl(fd, DMX_SET_FILTER, &flt)<0)
	{
		perror("DMX_SET_FILTER");
		return 1;
	}

	ioctl(fd, DMX_START, 0);
	if ((r=read(fd, buffer, r))<=0)
	{
		perror("read");
		return 1;
	}

	close(fd);

	if (r<=0) return 0;

	r=((buffer[1]&0x0F)<<8)|buffer[2];

	count=8;
	while(count<r-1)
	{
    	if ((((buffer[count+2]<<8)|buffer[count+3]) == ca_system_id) && (buffer[count+2] == ((0x18|0x27)&0xD7)))
		return (((buffer[count+4]<<8)|buffer[count+5])&0x1FFF);
		count+=buffer[count+1]+2;
	}
	return 0;
}

bool settings::boxIsCable()
{
	return isCable;
}

bool settings::boxIsSat()
{
	return !isCable;
}

int settings::getCAID()
{
	return CAID;
}

int settings::getTransparentColor()
{
	if (isGTX)
		return 0xFC0F;
	else
		return 0;
}

void settings::setIP(char n1, char n2, char n3, char n4)
{
	int fd = open("/var/lcars/lcars.conf", O_WRONLY|O_TRUNC|O_CREAT, 0666);
	if (fd < 0)
	{
		perror("lcars.conf");
		return;
	}

	write(fd, &n1, 1);
	write(fd, &n2, 1);
	write(fd, &n3, 1);
	write(fd, &n4, 1);

	close(fd);

}

char settings::getIP(char number)
{
	char ip;
	int fd = open("/var/lcars/lcars.conf", O_RDONLY);

	if (fd < 0)
		return 0;

	for (int i = 0; i <= number; i++)
		read(fd, &ip, 1);

	close(fd);
	return ip;
}
