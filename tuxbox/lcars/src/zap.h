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

#ifndef ZAP_H
#define ZAP_H

#include "settings.h"
#include "osd.h"
#include "tuner.h"
#include "cam.h"

class zap
{
	int vid, aud, frontend, video, audio;
	int old_frequ;
	settings setting;
	osd osdd;
	tuner tune;
	cam ca;
	int old_TS;
public:
	zap(settings &set, osd &o, tuner &t, cam &c);
	~zap();

	void zap_allstop();
	void zap_to(int VPID, int APID, int ECM, int SID, int ONID, int TS, int PID1 = -1, int PID2 = -1);
	void zap_audio(int VPID, int APID, int ECM, int SID, int ONID);
	void close_dev();
	void dmx_start();
	void dmx_stop();
};
#endif // ZAP_H
