#include <parentallock.h>

#include <lib/base/i18n.h>
#include <lib/gui/emessage.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>

ParentalLockWindow::ParentalLockWindow( const char* windowText, int curNum )
:eWindow(0)
{
	resize( eSize( 300, 150 ) );
	move( ePoint( 200, 200 ) );
	setText(windowText);

	lPin = new eLabel(this);
	lPin->move( ePoint( 10, 10 ) );
	lPin->resize( eSize( width()-20, 30 ) );
	lPin->setText(_("please enter pin:"));
	lPin->loadDeco();

	nPin=new eNumber(this, 4, 0, 9, 1, 0, 0, lPin, 1);
	nPin->move( ePoint( 10, 50 ) );
	nPin->resize( eSize( 100, 30 ) );
	nPin->loadDeco();
	nPin->setNumber(curNum);
	nPin->setFlags( eNumber::flagHideInput );
	CONNECT( nPin->selected, ParentalLockWindow::numEntered );
}

void ParentalLockWindow::numEntered(int *i)
{
	close( nPin->getNumber() );
}

eParentalSetup::eParentalSetup():
	eWindow(0)
{
	setText(_("Parental setup"));
	cmove(ePoint(170, 186));
	cresize(eSize(390, 230));

	loadSettings();

	parentallock=new eCheckbox(this, sparentallock, 1);
	parentallock->setText(_("Parental lock"));
	parentallock->move(ePoint(10, 20));
	parentallock->resize(eSize(200, 30));
	parentallock->setHelpText(_("enable/disable parental lock"));
	CONNECT(parentallock->checked, eParentalSetup::plockChecked );

	changeParentalPin = new eButton(this);
	changeParentalPin->setText(_("change PIN"));
	changeParentalPin->move( ePoint( 220, 15 ) );
	changeParentalPin->resize( eSize(160, 40) );
	changeParentalPin->setHelpText(_("change Parental PIN (ok)"));
	changeParentalPin->loadDeco();
	CONNECT(changeParentalPin->selected_id, eParentalSetup::changePin );
	if ( !sparentallock )
		changeParentalPin->hide();
  
	setuplock=new eCheckbox(this, ssetuplock, 1);
	setuplock->setText(_("Setup lock"));
	setuplock->move(ePoint(10, 70));
	setuplock->resize(eSize(200, 30));
	setuplock->setHelpText(_("enable/disable setup lock"));
	CONNECT(setuplock->checked, eParentalSetup::slockChecked );

	changeSetupPin = new eButton(this);
	changeSetupPin->setText(_("change PIN"));
	changeSetupPin->move( ePoint( 220, 65 ) );
	changeSetupPin->resize( eSize(160, 40) );
	changeSetupPin->setHelpText(_("change Setup PIN (ok)"));
	changeSetupPin->loadDeco();
	CONNECT(changeSetupPin->selected_id, eParentalSetup::changePin );
	if ( !ssetuplock )
		changeSetupPin->hide();

	ok=new eButton(this);
	ok->setText(_("save"));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->move(ePoint(10, 140));
	ok->resize(eSize(170, 40));
	ok->setHelpText(_("save changes and return"));
	ok->loadDeco();

	CONNECT(ok->selected, eParentalSetup::okPressed);

	abort=new eButton(this);
	abort->loadDeco();
	abort->setText(_("abort"));
	abort->move(ePoint(210, 140));
	abort->resize(eSize(170, 40));
	abort->setHelpText(_("ignore changes and return"));

	CONNECT(abort->selected, eParentalSetup::abortPressed);

	statusbar=new eStatusBar(this);
	statusbar->move( ePoint(0, clientrect.height()-30 ) );
	statusbar->resize( eSize( clientrect.width(), 30) );
	statusbar->loadDeco();

	setHelpID(2);
}

void eParentalSetup::plockChecked(int i)
{
	if ( i && !changeParentalPin->isVisible() )
		changeParentalPin->show();
	else
	{
		if ( checkPin( parentalpin, _("parental") ) )
		{
			parentalpin=0;
			changeParentalPin->hide();
		}
		else
			parentallock->setCheck(1);
	}
}

void eParentalSetup::slockChecked(int i)
{
	if ( i && !changeSetupPin->isVisible() )
		changeSetupPin->show();
	else
	{
		if ( checkPin( setuppin, _("setup") ) )
		{
			setuppin=0;
			changeSetupPin->hide();
		}
		else
			setuplock->setCheck(1);
	}
}

void eParentalSetup::changePin(eButton *p)
{
	const char *text = ( p == changeParentalPin ) ? _("parental") : _("setup");

	int oldpin = (p == changeParentalPin) ? parentalpin : setuppin;

	int ret = 0;

	if ( oldpin )  // let enter the oldpin.. and validate
	{
		if ( !checkPin( oldpin, text ) )
			return;
	}

	int newPin=0;
	do
	{
		{
			ParentalLockWindow w(eString().sprintf(_("New %s PIN"),text).c_str(), 0 );
			w.show();
			newPin = w.exec();
			w.hide();
			if ( newPin == -1 ) // cancel pressed
				return;
		}
		ParentalLockWindow w(eString().sprintf(_("Reenter %s PIN"),text).c_str(), 0 );
		w.show();
		ret = w.exec();
		w.hide();
		if ( ret == -1 ) // cancel pressed
			return;
		else if ( ret != newPin )
		{
			eMessageBox mb(_("The PINs are not equal!\n\nDo you want to retry?"),
					_("PIN validation failed"),
					eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
					eMessageBox::btYes );
			mb.show();
			int ret = mb.exec();
			mb.hide();
			if ( ret == eMessageBox::btNo || ret == -1 )
				return;
		}
	}
	while ( newPin != ret );

	if ( p == changeParentalPin )
		parentalpin = newPin;
	else
		setuppin = newPin;

	eMessageBox mb(_("PIN change completed"),
		_("PIN changed"),
		eMessageBox::btOK|eMessageBox::iconInfo,
		eMessageBox::btOK );
	mb.show();
	mb.exec();
	mb.hide();
}


void eParentalSetup::loadSettings()
{
	parentalpin = eConfig::getInstance()->getParentalPin();

	sparentallock = parentalpin != 0;

	if (eConfig::getInstance()->getKey("/elitedvb/pins/setuplock", setuppin))
		setuppin=0;

	ssetuplock = setuppin != 0;
}

void eParentalSetup::saveSettings()
{
	eConfig::getInstance()->setKey("/elitedvb/pins/setuplock", setuppin);
	eConfig::getInstance()->setParentalPin(parentalpin);
	eConfig::getInstance()->flush();
}

eParentalSetup::~eParentalSetup()
{
}

void eParentalSetup::okPressed()
{
	saveSettings();
	close(1);
}

void eParentalSetup::abortPressed()
{
	close(0);
}

bool checkPin( int pin, const char * text )
{
	if ( !pin )
		return true;
	ParentalLockWindow w(eString().sprintf(_("current %s PIN"),text).c_str(), 0 );
	int ret=0;
	do
	{
		w.show();
		ret = w.exec();
		w.hide();
		if ( ret == -1 )  // cancel pressed
			return false;
		else if ( ret != pin )
		{
			eMessageBox mb(_("The entered PIN is incorrect.\nDo you want to retry?"),
					_("PIN validation failed"),
					eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
					eMessageBox::btYes );
			mb.show();
			ret = mb.exec();
			mb.hide();
			if ( ret == eMessageBox::btNo || ret == -1 )
				return false;
		}
	}
	while ( ret != pin );
	return true;
}
