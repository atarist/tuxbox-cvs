/*
	Neutrino-GUI  -   DBoxII-Project

	NFS Mount/Umount GUI by Zwen
	
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __neutrino_nfs_gui__
#define __neutrino_nfs_gui__

#include <pthread.h>
#include "gui/widget/menue.h"


class CNFSMountGui : public CMenuTarget
{
 protected:
	
	enum FS_Support
		{
			FS_UNSUPPORTED   = 0,
			FS_READY         = 1,
			FS_NEEDS_MODULES = 2
		};
	
 public:

	enum FSType
		{
			NFS  = 0,
			CIFS = 1
		};
	
 private:
	static FS_Support fsSupported(const FSType fs, const bool load_modules = false);

	int menu();
	int menuEntry(int nr);

	char       m_entry[4][200];
	FS_Support m_nfs_sup;
	FS_Support m_cifs_sup;

 public:
	CNFSMountGui();
	int exec(CMenuTarget* parent, std::string actionKey);
	static void mount(const char * const ip, const char * const dir, const char* local_dir, const FSType fstype, const char * const username, const char * const password, const bool showerror = false);
	static void automount();
};

class CNFSUmountGui : public CMenuTarget
{
	private:

		int menu();

	public:
		CNFSUmountGui(){};
		~CNFSUmountGui(){};
		int  exec(CMenuTarget* parent, std::string actionKey);
		static void umount(std::string dir="");
};

class CNFSSmallMenu : public CMenuTarget
{
	private:

   public:
		CNFSSmallMenu(){};
		~CNFSSmallMenu(){};
		int exec( CMenuTarget* parent, std::string actionKey );
};

bool in_proc_filesystems(const char * const fsname);
bool insert_modules(const CNFSMountGui::FSType fstype);
bool remove_modules(const CNFSMountGui::FSType fstype);

#endif


