/*
	$Id: keybind_setup.h,v 1.6 2011/11/14 21:10:59 rhabarber1848 Exp $

	keybindings setup implementation - Neutrino-GUI

	Copyright (C) 2001 Steffen Hehn 'McClean'
	and some other guys
	Homepage: http://dbox.cyberphoria.org/

	Copyright (C) 2010 T. Graf 'dbt'
	Homepage: http://www.dbox2-tuning.net/

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

#ifndef __keybind_setup__
#define __keybind_setup__

#include <gui/widget/menue.h>
#include <gui/widget/icons.h>

#include <system/setting_helpers.h>

#include <string>

class CKeybindSetup : public CMenuTarget
{
	private:
		enum keynames {
			VIRTUALKEY_TV_RADIO_MODE,
			VIRTUALKEY_PAGE_UP,
			VIRTUALKEY_PAGE_DOWN,
			VIRTUALKEY_CANCEL_ACTION,
			VIRTUALKEY_SORT,
			VIRTUALKEY_SEARCH,
			VIRTUALKEY_ADD_RECORD,
			VIRTUALKEY_ADD_REMIND,
			VIRTUALKEY_RELOAD,
			VIRTUALKEY_CHANNEL_UP,
			VIRTUALKEY_CHANNEL_DOWN,
			VIRTUALKEY_VOLUME_UP,
			VIRTUALKEY_VOLUME_DOWN,
			VIRTUALKEY_BOUQUET_UP,
			VIRTUALKEY_BOUQUET_DOWN,
			VIRTUALKEY_SUBCHANNEL_UP,
			VIRTUALKEY_SUBCHANNEL_DOWN,
			VIRTUALKEY_SUBCHANNEL_TOGGLE,
			VIRTUALKEY_ZAP_HISTORY,
			VIRTUALKEY_LASTCHANNEL,
			VIRTUALKEY_MENU_PAGE_UP,
			VIRTUALKEY_MENU_PAGE_DOWN,
		
			MAX_NUM_KEYNAMES
		};

		CKeySetupNotifier      *keySetupNotifier;

		int width, selected;

		neutrino_locale_t menue_title;
		std::string menue_icon;

		void showSetup();

	public:	
		CKeybindSetup(const neutrino_locale_t title = NONEXISTANT_LOCALE, const char * const IconName = NULL);
		~CKeybindSetup();
		int exec(CMenuTarget* parent, const std::string & actionKey);
};

#endif
