#include "enigma_plugins.h"

#include "config.h"
#include <plugin.h>
#include <dbox/avia_gt_vbi.h>

#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#include <apps/enigma/enigma.h>
#include <core/base/eerror.h>
#include <core/gdi/lcd.h>
#include <core/gdi/font.h>
#include <core/driver/rc.h>
#include <core/driver/streamwd.h>
#include <core/dvb/edvb.h>
#include <core/dvb/decoder.h>
#include <core/gui/emessage.h>
#include <core/gui/eskin.h>

eString getInfo(const char *file, const char *info)
{
	FILE *f=fopen(file, "rt");
	if (!f)
		return 0;

	eString result;

	char buffer[128];

	while (fgets(buffer, 127, f))
	{
		if (strlen(buffer))
			buffer[strlen(buffer)-1]=0;

		if (strstr(buffer, info))
		{
  		result=eString(buffer).mid(strlen(info)+1, strlen(buffer)-strlen(info+1));
			break;
		}
	}	
	fclose(f);
	return result;
}

PluginParam *first=0, *tmp=0;

void MakeParam(char* id, int val)
{
	PluginParam* p = new PluginParam;

	if (tmp)
		tmp->next = p;

	p->id = new char[strlen(id)+1];
	strcpy(p->id, id);
	char buf[10];
	sprintf(buf, "%i", val);
	p->val = new char[strlen(buf)+1];
	strcpy(p->val, buf);

	if (!first)
		first = p;

	p->next=0;
	tmp = p;		
}

ePlugin::ePlugin(eListBox<ePlugin> *parent, const char *cfgfile, const char* descr)
	:eListBoxEntryText((eListBox<eListBoxEntryText>*)parent)
{
	if (!cfgfile)
	{
		text="[back]";
		return;
	}

	eDebug(cfgfile);
	text=getInfo(cfgfile, "name");

	if (text.isNull())
		text="(" + eString(cfgfile) + " is invalid)";
		
	eString desc=getInfo(cfgfile, "desc");

	if (desc)
	{
		text+=" - "+desc;
	}

	depend=getInfo(cfgfile, "depend");

	eString atype=getInfo(cfgfile, "type"),
					apluginVersion=getInfo(cfgfile, "pluginversion"),
					aneedfb=getInfo(cfgfile, "needfb"),
					aneedrc=getInfo(cfgfile, "needrc"),
					aneedlcd=getInfo(cfgfile, "needlcd"),
					aneedvtxtpid=getInfo(cfgfile, "needvtxtpid"),
					aneedoffsets=getInfo(cfgfile, "needoffsets"),
					apigon=getInfo(cfgfile, "pigon");

	needfb=(aneedfb.isNull()?false:atoi(aneedfb.c_str()));
	needlcd=(aneedlcd.isNull()?false:atoi(aneedlcd.c_str()));
	needrc=(aneedrc.isNull()?false:atoi(aneedrc.c_str()));
	needvtxtpid=(aneedvtxtpid.isNull()?false:atoi(aneedvtxtpid.c_str()));
	needoffsets=(aneedoffsets.isNull()?false:atoi(aneedoffsets.c_str()));
	version=(apluginVersion.isNull()?0:atoi(apluginVersion.c_str()));
	showpig=(apigon.isNull()?false:atoi(apigon.c_str()));

	sopath=eString(cfgfile).left(strlen(cfgfile)-4)+".so";	// uarg

	pluginname=eString(cfgfile).mid(eString(cfgfile).rfind('/')+1);

	pluginname=pluginname.left(pluginname.length()-4);
}

eZapPlugins::eZapPlugins(eWidget* lcdTitle, eWidget* lcdElement)
	:eListBoxWindow<ePlugin>(_("Plugins"), 10, 400/*, true*/)
{
	move(ePoint(150, 50));
	setLCD(lcdTitle, lcdElement);
	new ePlugin(&list, 0);
	CONNECT(list.selected, eZapPlugins::selected);
}

int eZapPlugins::exec()
{
	const eString PluginPath = PLUGINDIR "/";
	struct dirent **namelist;

	int n = scandir(PLUGINDIR "/", &namelist, 0, alphasort);

	if (n < 0)
	{
		eDebug("Error Read Plugin Directory");
		eMessageBox msg("Error Read Plugin Directory", "Error");
		msg.show();
		msg.exec();
		msg.hide();
		return -1;
	}

	for(int count=0;count<n;count++)
	{
		eString	FileName = namelist[count]->d_name;

		if ( FileName.find(".cfg") != eString::npos )
			new ePlugin(&list, (PluginPath+FileName).c_str());		

		free(namelist[count]);
  }

	free(namelist);

	show();
	int res=eListBoxWindow<ePlugin>::exec();
	hide();
	return res;
}

void eZapPlugins::execPluginByName(const char* name)
{
	eString PluginPath = PLUGINDIR "/";
	PluginPath+=name;
	ePlugin p(0, PluginPath.c_str());
	execPlugin(&p);
}

void eZapPlugins::execPlugin(ePlugin* plugin)
{
	void *libhandle[20];
	int argc=0;
	eString argv[20];

	if (plugin->depend)
	{
 		char	depstring[129];
		char	*p;
		char	*np;

		strcpy(depstring, plugin->depend.c_str());

		p=depstring;

		while(p)
		{
			np=strchr(p,',');
			if ( np )
				*np=0;
			argv[ argc++ ] = (*p == '/') ?
				eString(p) : eString(PLUGINDIR "/" + eString(p));
			p=np?np+1:0;
		}
	}

	argv[argc++]=plugin->sopath;

	int i;
	eDebug("pluginname is %s", plugin->pluginname.c_str());

	if (plugin->needfb)
		MakeParam(P_ID_FBUFFER, fbClass::getInstance()->lock());

	if (plugin->needrc)
		MakeParam(P_ID_RCINPUT, eRCInput::getInstance()->lock());

	if (plugin->needlcd)
		MakeParam(P_ID_LCD,	eDBoxLCD::getInstance()->lock() );

	if (plugin->needoffsets)
	{
		MakeParam(P_ID_OFF_X, 0);
		MakeParam(P_ID_OFF_Y, 0);
		MakeParam(P_ID_END_X, 720);
		MakeParam(P_ID_END_Y, 576);
	}

 	if (plugin->needvtxtpid)
 	{
		// versuche, den gtx/enx_vbi zu stoppen	
		eDebug("try to stop gtx/enx_vbi");
		if(Decoder::parms.tpid==-1)
		{
			MakeParam(P_ID_VTXTPID, 0);
		}
		else
		{
			MakeParam(P_ID_VTXTPID, Decoder::parms.tpid);
		}
		int fd = open("/dev/dbox/vbi0", O_RDWR);
		if (fd > 0)
		{
			eDebug("stop gtx/enx_vbi");
			ioctl(fd, AVIA_VBI_STOP_VTXT, 0);
			::close(fd);
		}
	}

	for(PluginParam *par = first; par; par=par->next )
	{
		printf ("id: %s - val: %s\n", par->id, par->val);
		printf("%p\n", par->next);
	}

	for (i=0; i<argc; i++)
	{
		eDebug("loading %s" , argv[i].c_str());
		libhandle[i]=dlopen(argv[i].c_str(), RTLD_NOW|RTLD_GLOBAL);
		if (!libhandle[i])
		{
			const char *de=dlerror();
			eDebug(de);
			eMessageBox msg(de, "plugin loading failed");
			msg.show();
			msg.exec();
			msg.hide();
			break;
		}
	}
	
	if (i==argc)
	{
		eDebug("would exec plugin %s", plugin->sopath.c_str());

		PluginExec execPlugin = (PluginExec) dlsym(libhandle[i-1], "plugin_exec");
		if (!execPlugin)
		{
			eMessageBox msg("The symbol " + plugin->pluginname + "_exec" + " was not found. sorry.", "plugin executing failed");
			msg.show();
			msg.exec();
			msg.hide();
		}
		else
		{		
			eDebug("exec Plugin now...");
			execPlugin(first);
			dlclose(libhandle[i-1]);
			eDebug("exec done...");
		}

		while (i--)
			dlclose(libhandle[i]);
	}

	do  // Parameter Liste freigegeben
	{
		tmp = first->next;
		delete first;
		first = tmp;
	}
	while (first);

	if (plugin->needfb)
		fbClass::getInstance()->unlock();
	
	if (plugin->needrc)
		eRCInput::getInstance()->unlock();

	if (plugin->needlcd)
		eDBoxLCD::getInstance()->unlock();

 	if (plugin->needvtxtpid)
 	{
		// versuche, den gtx/enx_vbi wieder zu starten
		eDebug("try to restart gtx/enx_vbi");
 		int fd = open("/dev/dbox/vbi0", O_RDWR);
		if (fd > 0)
		{
			ioctl(fd, AVIA_VBI_START_VTXT, Decoder::parms.tpid);
			::close(fd);
		}
	}

}

void eZapPlugins::selected(ePlugin *plugin)
{
	if (!plugin || !plugin->pluginname )
	{
		close(0);
		return;
	}
	execPlugin(plugin);

	hide();
	show();
}
