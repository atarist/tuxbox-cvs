/*
 * setup_language.cpp
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
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
 * $Id: setup_language.cpp,v 1.10.2.7 2003/05/24 14:12:09 ghostrider Exp $
 */

#include <setup_language.h>

#include <lib/base/i18n.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>

eZapLanguageSetup::eZapLanguageSetup(): eWindow(0)
{
	setText(_("Language Selector"));
	move(ePoint(150, 136));
	cresize(eSize(390, 160));

	eLabel *l=new eLabel(this);
	l->setText(_("Language:"));
  l->setFlags(eLabel::flagVCenter);
	l->move(ePoint(20, 20));
	l->resize(eSize(140, 35));

	language=new eListBox<eListBoxEntryText>(this, l);
	language->loadDeco();
	language->setFlags( eListBoxBase::flagNoUpDownMovement );
	language->move(ePoint(140, 20));
	language->resize(eSize(150, 35));
	language->setHelpText(_("select your language (left, right)"));
	
	FILE *f=fopen("/share/locale/locales", "rt");

	char *temp;
	if ( eConfig::getInstance()->getKey("/elitedvb/language", temp) )
		temp=0;

	eListBoxEntryText *cur=0;

	if (!f)
	{
		new eListBoxEntryText(language, "English", (void*) new eString("C"));
		new eListBoxEntryText(language, "Deutsch", (void*) new eString("de_DE"));
	} else
	{
		char line[256];
		while (fgets(line, 256, f))
		{
			line[strlen(line)-1]=0;
			char *id=line, *d;
			if ((d=strchr(line, ' ')))
			{
				*d++=0;
				eListBoxEntryText *l=new eListBoxEntryText(language, d, (void*) new eString(id));
				if (temp && !strcmp(id, temp))
					cur=l;
			}
		}
		fclose(f);
	}
	free(temp);
	
	language->setCurrent(cur);

	ok=new eButton(this);
	ok->setText(_("save"));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->move(ePoint(20, 80));
	ok->resize(eSize(170, 40));
	ok->setHelpText(_("save changes and return"));
	ok->loadDeco();
	CONNECT(ok->selected, eZapLanguageSetup::okPressed);

	abort=new eButton(this);
	abort->setText(_("abort"));
	abort->move(ePoint(210, 80));
	abort->resize(eSize(170, 40));
	abort->setHelpText(_("ignore changes and return"));
	abort->loadDeco();
	CONNECT(abort->selected, eWidget::reject );

	statusbar = new eStatusBar(this);
	statusbar->move( ePoint(0, clientrect.height()-30) );
	statusbar->resize( eSize( clientrect.width(), 30) );
	statusbar->loadDeco();
}

void eZapLanguageSetup::okPressed()
{
	eConfig::getInstance()->setKey("/elitedvb/language", ((eString*) language->getCurrent()->getKey())->c_str() );
	eConfig::getInstance()->flush();
	close(1);
}

int eZapLanguageSetup::eventHandler( const eWidgetEvent & e )
{
	switch (e.type)
	{
		case eWidgetEvent::execDone:
		{
			char *tmp;
			if ( !eConfig::getInstance()->getKey("/elitedvb/language", tmp ) )
			{
				setlocale(LC_ALL, tmp );
				free(tmp);
			}
			else
				setlocale(LC_ALL, "C" );
			break;
		}
		default:
			return eWindow::eventHandler( e );
	}
	return 1;
}

eZapLanguageSetup::~eZapLanguageSetup()
{
	while ( language->getCurrent()->getKey() )
	{
		delete (eString*)language->getCurrent()->getKey();
		language->getCurrent()->getKey()=0;
		language->goNext();
	}
}

