/*
 * setup_harddisk.cpp
 *
 * Copyright (C) 2002 Felix Domke <tmbinc@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: setup_harddisk.cpp,v 1.2.2.13 2003/02/12 22:08:27 ghostrider Exp $
 */

#include <setup_harddisk.h>
#include <lib/gui/emessage.h>
#include <sys/vfs.h> // for statfs
#include <unistd.h>
#include <signal.h>

 #define EXT3

static int getCapacity(int dev)
{
	int c='a'+dev;
	
	FILE *f=fopen(eString().sprintf("/proc/ide/hd%c/capacity", c).c_str(), "r");
	if (!f)
		return -1;
	int capacity=-1;
	fscanf(f, "%d", &capacity);
	fclose(f);
	return capacity;
}

static eString getModel(int dev)
{
	int c='a'+dev;
	char line[1024];

	FILE *f=fopen(eString().sprintf("/proc/ide/hd%c/model", c).c_str(), "r");
	if (!f)
		return "";
	*line=0;
	fgets(line, 1024, f);
	fclose(f);
	if (!*line)
		return "";
	line[strlen(line)-1]=0;
	return line;
}

static int numPartitions(int dev)
{
	FILE *f=fopen("/proc/partitions", "rb");
	if (!f)
		return 0;
	eString path;
	int host=dev/4;
	int bus=!!(dev&2);
	int target=!!(dev&1);
	path.sprintf("ide/host%d/bus%d/target%d/lun0/", host, bus, target);

	int numpart=-1;		// account for "disc"
	
	while (1)
	{
		char line[1024];
		if (!fgets(line, 1024, f))
			break;
		if (line[1] != ' ')
			continue;
		if (!strncmp(line+22, path.c_str(), path.size()))
			numpart++;
	}
	fclose(f);
	return numpart;
}

int freeDiskspace(int dev, eString mp="")
{
/*	FILE *f=fopen("/proc/mounts", "rb");
	if (!f)
		return -1;
	eString path;
	int host=dev/4;
	int bus=!!(dev&2);
	int target=!!(dev&1);
	path.sprintf("/dev/ide/host%d/bus%d/target%d/lun0/", host, bus, target);
	
	while (1)
	{
		char line[1024];
		if (!fgets(line, 1024, f))
			break;
		if (!strncmp(line, path.c_str(), path.size()))
		{
			eString mountpoint=line;
			mountpoint=mountpoint.mid(mountpoint.find(' ')+1);
			mountpoint=mountpoint.left(mountpoint.find(' '));
//			eDebug("mountpoint: %s", mountpoint.c_str());
			if ( mp && mountpoint != mp )
				return -1;*/
			struct statfs s;
			int free;
//			if (statfs(mountpoint.c_str(), &s)<0)
			if (statfs(mp.c_str(), &s)<0)
				free=-1;
			else
				free=s.f_bfree/1000*s.f_bsize/1000;
//			fclose(f);
			return free;
/*		}
	}
	fclose(f);
	return -1;*/
}

eString getPartFS(int dev, eString mp="")
{
	FILE *f=fopen("/proc/mounts", "rb");
	if (!f)
		return "";
	eString path;
	int host=dev/4;
	int bus=!!(dev&2);
	int target=!!(dev&1);
	path.sprintf("/dev/ide/host%d/bus%d/target%d/lun0/", host, bus, target);

	while (1)
	{
		char line[1024];
		if (!fgets(line, 1024, f))
			break;

		if (!strncmp(line, path.c_str(), path.size()))
		{
			eString mountpoint=line;
			mountpoint=mountpoint.mid(mountpoint.find(' ')+1);
			mountpoint=mountpoint.left(mountpoint.find(' '));
//			eDebug("mountpoint: %s", mountpoint.c_str());
			if ( mp && mountpoint != mp )
				continue;

			if (!strncmp(line, path.c_str(), path.size()))
			{
				eString fs=line;
				fs=fs.mid(fs.find(' ')+1);
				fs=fs.mid(fs.find(' ')+1);
				fs=fs.left(fs.find(' '));
				eString mpath=line;
				mpath=mpath.left(mpath.find(' '));
				mpath=mpath.mid(mpath.rfind('/')+1);
				fclose(f);
				return fs+','+mpath;
			}
		}
	}
	fclose(f);
	return "";
}

eHarddiskSetup::eHarddiskSetup()
: eListBoxWindow<eListBoxEntryText>(_("harddisk setup..."), 5, 420, true)
{
	nr=0;
	
	move(ePoint(150, 136));
	
	new eListBoxEntryText(&list, _("back"), (void*)-1);
	
	for (int host=0; host<1; host++)
		for (int bus=0; bus<1; bus++)
			for (int target=0; target<1; target++)
			{
				int num=target+bus*2+host*4;
				
				int c='a'+num;
				
							// check for presence
				char line[1024];
				int ok=1;
				FILE *f=fopen(eString().sprintf("/proc/ide/hd%c/media", c).c_str(), "r");
				if (!f)
					continue;
				if ((!fgets(line, 1024, f)) || strcmp(line, "disk\n"))
					ok=0;
				fclose(f);

				if (ok)
				{
					int capacity=getCapacity(num);
					if (capacity < 0)
						continue;
						
					capacity=capacity/1000*512/1000;

					eString sharddisks;
					sharddisks=getModel(num);
					sharddisks+=" (";
					if (c&1)
						sharddisks+="master";
					else
						sharddisks+="slave";
					if (capacity)
						sharddisks+=eString().sprintf(", %d.%03d GB", capacity/1024, capacity%1024);
					sharddisks+=")";
					
					nr++;
					
					new eListBoxEntryText(&list, sharddisks, (void*)num);
				}
	}
	
	CONNECT(list.selected, eHarddiskSetup::selectedHarddisk);
}

void eHarddiskSetup::selectedHarddisk(eListBoxEntryText *t)
{
	if ((!t) || (((int)t->getKey())==-1))
	{
		close(0);
		return;
	}
	int dev=(int)t->getKey();
	
	eHarddiskMenu menu(dev);
	
	hide();
	menu.show();
	menu.exec();
	menu.hide();
	show();
}

void eHarddiskMenu::check()
{
	hide();
	ePartitionCheck check(dev);
	check.show();
	check.exec();
	check.hide();
	show();
}

void eHarddiskMenu::s_format()
{
	hide();
	do
	{
		{
			eMessageBox msg(
				 _("Are you SURE that you want to format this disk?\n"),
				 _("formatting harddisk..."),
				 eMessageBox::btYes|eMessageBox::btCancel, eMessageBox::btCancel);
			msg.show();
			int res=msg.exec();
			msg.hide();
			if (res != eMessageBox::btYes)
				break;
		}
		if (numpart)
		{
			eMessageBox msg(
				 _("There's data on this harddisk.\n"
				 "You will lose that data. Proceed?"),
				 _("formatting harddisk..."),
				 eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo);
			msg.show();
			int res=msg.exec();
			msg.hide();
			if (res != eMessageBox::btYes)
				break;
		}
		int host=dev/4;
		int bus=!!(dev&2);
		int target=!!(dev&1);

// kill samba server... (exporting /hdd)
		system("killall -9 smbd");

		system(
				eString().sprintf(
				"/bin/umount /dev/ide/host%d/bus%d/target%d/lun0/part*", host, bus, target).c_str());

		eMessageBox msg(
			_("please wait while initializing harddisk.\nThis might take some minutes.\n"),
			_("formatting harddisk..."), 0);
		msg.show();

		FILE *f=popen(
				eString().sprintf(
				"/sbin/sfdisk -f /dev/ide/host%d/bus%d/target%d/lun0/disc", host, bus, target).c_str(), "w");
		if (!f)
		{
			eMessageBox msg(
				_("sorry, couldn't find sfdisk utility to partition harddisk."),
				_("formatting harddisk..."),
				 eMessageBox::btOK|eMessageBox::iconError);
			msg.show();
			msg.exec();
			msg.hide();
			break;
		}
		fprintf(f, "0,\n;\n;\n;\ny\n");
		fclose(f);

		
		if ( (system("sync") >> 8)
			||
			( system(
#ifdef EXT3
				eString().sprintf(
				"/sbin/mkfs.ext3 /dev/ide/host%d/bus%d/target%d/lun0/part1", host, bus, target).c_str())>>8 )
				||
				(system("sync") >> 8)
				||
				(system(
				eString().sprintf(
				"/bin/mount -t ext3 /dev/ide/host%d/bus%d/target%d/lun0/part1 /hdd", host, bus, target).c_str())>>8 ) ||
#else // REISERFS
				eString().sprintf(
				"/sbin/mkreiserfs -f -f /dev/ide/host%d/bus%d/target%d/lun0/part1", host, bus, target).c_str())>>8 )
				||
				(system("sync") >> 8)
				||
				(system(
				eString().sprintf(
				"/bin/mount -t reiserfs /dev/ide/host%d/bus%d/target%d/lun0/part1 /hdd", host, bus, target).c_str())>>8 ) ||
#endif
				(system("mkdir /hdd/movie")>>8 )
				)
		{
			eMessageBox msg(
				_("creating filesystem failed."),
				_("formatting harddisk..."),
				 eMessageBox::btOK|eMessageBox::iconError);
			msg.show();
			msg.exec();
			msg.hide();
			break;
		}
		msg.hide();
		{
			eMessageBox msg(
				_("successfully formatted your disk!"),
				_("formatting harddisk..."),
				 eMessageBox::btOK|eMessageBox::iconInfo);
			msg.show();
			msg.exec();
			msg.hide();
		}
		readStatus();
	} while (0);
	// restart samba...
	system("/bin/smbd -D");
	show();
}

void eHarddiskMenu::readStatus()
{
	if (!(dev & 1))
		bus->setText("master");
	else
		bus->setText("slave");

	eString mod=getModel(dev);
	setText(mod);
	model->setText(mod);
	int cap=getCapacity(dev)/1000*512/1000;
	
	if (cap != -1)
		capacity->setText(eString().sprintf("%d.%03d GB", cap/1000, cap%1000));
		
	numpart=numPartitions(dev);
	int fds;
	
	if (numpart == -1)
		status->setText(_("(error reading information)"));
	else if (!numpart)
		status->setText(_("uninitialized - format it to use!"));
	else if ((fds=freeDiskspace(dev)) != -1)
		status->setText(eString().sprintf(_("in use, %d.%03d GB (~%d minutes) free"), fds/1000, fds%1000, fds/33 ));
	else
		status->setText(_("initialized, but unknown filesystem"));
}

eHarddiskMenu::eHarddiskMenu(int dev): dev(dev)
{
	status=new eLabel(this); status->setName("status");
	model=new eLabel(this); model->setName("model");
	capacity=new eLabel(this); capacity->setName("capacity");
	bus=new eLabel(this); bus->setName("bus");
	
	close=new eButton(this); close->setName("close");
	format=new eButton(this); format->setName("format");
	bcheck=new eButton(this); bcheck->setName("check");

	if (eSkin::getActive()->build(this, "eHarddiskMenu"))
		eFatal("skin load of \"eHarddiskMenu\" failed");

	readStatus();

	CONNECT(close->selected, eWidget::accept);
	CONNECT(format->selected, eHarddiskMenu::s_format);
	CONNECT(bcheck->selected, eHarddiskMenu::check);
}

ePartitionCheck::ePartitionCheck( int dev )
:dev(dev), fsck(0)
{
	lState = new eLabel(this);
	lState->setName("state");
	bCancel = new eButton(this);
	bCancel->setName("cancel");
	bClose = new eButton(this);
	bClose->setName("close");
	CONNECT( bCancel->selected, ePartitionCheck::onCancel );
	CONNECT( bClose->selected, ePartitionCheck::accept );
	if (eSkin::getActive()->build(this, "ePartitionCheck"))
		eFatal("skin load of \"ePartitionCheck\" failed");
	bClose->hide();
}

int ePartitionCheck::eventHandler( const eWidgetEvent &e )
{
	switch(e.type)
	{
		case eWidgetEvent::execBegin:
		{
			eString fs = getPartFS(dev,"/hdd"),
							part = fs.mid( fs.find(",")+1 );

			fs = fs.left( fs.find(",") );

			eDebug("part = %s, fs = %s", part.c_str(), fs.c_str() );

			int host=dev/4;
			int bus=!!(dev&2);
			int target=!!(dev&1);

			// kill samba server... (exporting /hdd)
			system("killall -9 smbd");

			if ( system("/bin/umount /hdd") >> 8)
			{
				eMessageBox msg(
				_("could not unmount the filesystem... "),
				_("check filesystem..."),
				 eMessageBox::btOK|eMessageBox::iconError);
				close(-1);
			}
			if ( fs == "ext3" )
			{
				fsck = new eConsoleAppContainer( eString().sprintf("/sbin/fsck.ext3 -f /dev/ide/host%d/bus%d/target%d/lun0/%s", host, bus, target, part.c_str()) );

				if ( !fsck->running() )
				{
					eMessageBox msg(
						_("sorry, couldn't find fsck.ext3 utility to check the ext3 filesystem."),
						_("check filesystem..."),
						eMessageBox::btOK|eMessageBox::iconError);
					msg.show();
					msg.exec();
					msg.hide();
					close(-1);
				}
				else
				{
					eDebug("fsck.ext3 opened");
					CONNECT( fsck->dataAvail, ePartitionCheck::getData );
					CONNECT( fsck->appClosed, ePartitionCheck::fsckClosed );
				}
			}
			else if ( fs == "reiserfs" )
			{
				fsck = new eConsoleAppContainer( eString().sprintf("/sbin/reiserfsck --fix-fixable /dev/ide/host%d/bus%d/target%d/lun0/%s", host, bus, target, part.c_str()) );

				if ( !fsck->running() )
				{
					eMessageBox msg(
						_("sorry, couldn't find reiserfsck utility to check the reiserfs filesystem."),
						_("check filesystem..."),
						eMessageBox::btOK|eMessageBox::iconError);
					msg.show();
					msg.exec();
					msg.hide();
					close(-1);
				}
				else
				{
					eDebug("reiserfsck opened");
					CONNECT( fsck->dataAvail, ePartitionCheck::getData );
					CONNECT( fsck->appClosed, ePartitionCheck::fsckClosed );
					fsck->write("Yes\n");
				}
			}
			else
			{
				eMessageBox msg(
					_("not supported filesystem for check."),
					_("check filesystem..."),
					eMessageBox::btOK|eMessageBox::iconError);
				msg.show();
				msg.exec();
				msg.hide();
				close(-1);
			}
		}
		break;

		case eWidgetEvent::execDone:
			if (fsck)
				delete fsck;
		break;

		default:
			return eWindow::eventHandler( e );
	}
	return 1;	
}

void ePartitionCheck::onCancel()
{
	if (fsck)
		fsck->kill();
}

void ePartitionCheck::fsckClosed(int state)
{
	int host=dev/4;
	int bus=!!(dev&2);
	int target=!!(dev&1);

	if ( system( eString().sprintf("/bin/mount /dev/ide/host%d/bus%d/target%d/lun0/part1 /hdd", host, bus, target).c_str() ) >> 8 )
		eDebug("mount hdd after check failed");

	system("/bin/smbd -D");
	eDebug("smbd restarted");

	if (fsck)
	{
		delete fsck;
		fsck=0;
	}

	bClose->show();
}

void ePartitionCheck::getData( eString str )
{
	lState->setText(str);
	
	if ( str.find("<y>") != eString::npos )
		fsck->write("y");
	else if ( str.find("[N/Yes]") != eString::npos )
		fsck->write("Yes");
}
