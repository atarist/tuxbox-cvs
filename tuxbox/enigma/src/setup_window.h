#ifndef __setup_window_h
#define __setup_window_h

#include <lib/gui/listbox.h>

class eSetupWindow:public eListBoxWindow<eListBoxEntryMenu>
{
	void sel_num(int n);
protected:
	int eventHandler(const eWidgetEvent &event);
public:
 	static Signal1<void,eSetupWindow*> setupHook;
	eSetupWindow( const char *titlemm, int entries, int width );
};

#endif // __setup_window_h
