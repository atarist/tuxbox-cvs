#include "../include/debug.h"
#include "../global.h"

CBouquetList::CBouquetList( const std::string &Name )
{
	name = Name;
	selected = 0;
	width = 500;
	height = 440;
	theight= g_Fonts->menu_title->getHeight();
	fheight= g_Fonts->channellist->getHeight();
	listmaxshow = (height-theight-0)/fheight;
	height = theight+0+listmaxshow*fheight; // recalc height
	x=(((g_settings.screen_EndX- g_settings.screen_StartX)-width) / 2) + g_settings.screen_StartX;
	y=(((g_settings.screen_EndY- g_settings.screen_StartY)-height) / 2) + g_settings.screen_StartY;
	liststart = 0;
	tuned=0xfffffff;
}

CBouquetList::~CBouquetList()
{
	for(unsigned int count=0;count<Bouquets.size();count++)
	{
		delete Bouquets[count];
	}
	Bouquets.clear();
}

CBouquet* CBouquetList::addBouquet(const std::string& name, int BouquetKey )
{
	if ( BouquetKey==-1 )
		BouquetKey= Bouquets.size();

	CBouquet* tmp = new CBouquet( BouquetKey, name );
	Bouquets.insert(Bouquets.end(), tmp);
	return(tmp);
}

void CBouquetList::setName(const std::string& Name)
{
	name = Name;
}

const std::string& CBouquetList::getActiveBouquetName()
{
	return Bouquets[selected]->name;
}

int CBouquetList::getActiveBouquetNumber()
{
	return selected;
}

int CBouquetList::showChannelList( int nBouquet = -1 )
{
	if (nBouquet == -1)
		nBouquet = selected;

	int nNewChannel = Bouquets[nBouquet]->channelList->show();
	if (nNewChannel > -1)
	{
		selected = nBouquet;
		orgChannelList->zapTo(Bouquets[selected]->channelList->getKey(nNewChannel)-1);

		nNewChannel= -2; // exit!
	}

	return nNewChannel;
}

void CBouquetList::adjustToChannel( int nChannelNr)
{
	for (uint i=0; i<Bouquets.size(); i++)
	{
		int nChannelPos = Bouquets[i]->channelList->hasChannel(nChannelNr);
		if (nChannelPos > -1)
		{
			selected = i;
			Bouquets[i]->channelList->setSelected(nChannelPos);
			return;
		}
	}
}


int CBouquetList::activateBouquet( int id, bool bShowChannelList = false )
{
	int res = menu_return::RETURN_REPAINT;

	selected = id;
	if (bShowChannelList)
	{
		int nNewChannel = Bouquets[selected]->channelList->show();

		if (nNewChannel > -1)
		{
			orgChannelList->zapTo(Bouquets[selected]->channelList->getKey(nNewChannel)-1);
		}
		else if ( nNewChannel = -2 )
		{
			// -2 bedeutet EXIT_ALL
			res = menu_return::RETURN_EXIT_ALL;
		}
	}

	return res;
}

int CBouquetList::exec( bool bShowChannelList)
{
    int res= show();

	if ( res > -1)
	{
		return activateBouquet( selected, bShowChannelList );
	}
	else if ( res = -1)
	{
		// -1 bedeutet nur REPAINT
		return menu_return::RETURN_REPAINT;
	}
	else
	{
		// -2 bedeutet EXIT_ALL
		return menu_return::RETURN_EXIT_ALL;
	}

	return res;
}

int CBouquetList::show()
{
	int res = -1;
	if(Bouquets.size()==0)
	{
		return res;
	}

	maxpos= 1;
	int i= Bouquets.size();
	while ((i= i/10)!=0)
		maxpos++;

	paintHead();
	paint();

	int oldselected = selected;
	int firstselected = selected+ 1;
	int zapOnExit = false;
	bool loop=true;

	int chn= 0;
	int pos= maxpos;
	while (loop)
	{

		uint msg; uint data;
		g_RCInput->getMsg( &msg, &data, g_settings.timing_chanlist );

		if ( ( msg == CRCInput::RC_timeout ) ||
			 ( msg == g_settings.key_channelList_cancel ) )
		{
			selected = oldselected;
			loop=false;
		}
		else if ( msg == g_settings.key_channelList_pageup )
		{
			selected+=listmaxshow;
			if (selected>Bouquets.size()-1)
				selected=0;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == g_settings.key_channelList_pagedown )
		{
			if ((int(selected)-int(listmaxshow))<0)
				selected=Bouquets.size()-1;
			else
				selected -= listmaxshow;
			liststart = (selected/listmaxshow)*listmaxshow;
			paint();
		}
		else if ( msg == CRCInput::RC_up )
		{
			int prevselected=selected;
			if(selected==0)
			{
				selected = Bouquets.size()-1;
			}
			else
				selected--;
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
			{
				paint();
			}
			else
			{
				paintItem(selected - liststart);
			}
		}
		else if ( msg == CRCInput::RC_down )
		{
			int prevselected=selected;
			selected = (selected+1)%Bouquets.size();
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
			{
				paint();
			}
			else
			{
				paintItem(selected - liststart);
			}
		}
		else if ( msg == CRCInput::RC_ok )
		{
			zapOnExit = true;
			loop=false;
		}
		else if ( ( msg >= 0 ) && ( msg <= 9 ) )
		{
			//numeric
			if ( pos==maxpos )
			{
				if ( msg == 0)
				{
					chn = firstselected;
					pos = maxpos- 1;
				}
				else
				{
					chn = msg;
					pos = 0;
				}
			}
			else
			{
				chn = chn* 10 + msg;
			}

			if (chn> Bouquets.size())
			{
				chn = firstselected;
				pos = maxpos- 1;
			}

			pos++;

            int prevselected=selected;
			selected = (chn- 1)%Bouquets.size();
			paintItem(prevselected - liststart);
			unsigned int oldliststart = liststart;
			liststart = (selected/listmaxshow)*listmaxshow;
			if(oldliststart!=liststart)
			{
				paint();
			}
			else
			{
				paintItem(selected - liststart);
			}

		}
		else if( ( msg == CRCInput::RC_red ) ||
				 ( msg == CRCInput::RC_green ) ||
				 ( msg == CRCInput::RC_yellow ) ||
				 ( msg == CRCInput::RC_blue ) )
		{
			selected = oldselected;
			g_RCInput->pushbackMsg( msg, data );
			loop=false;
		}
		else
		{
			if ( neutrino->handleMsg( msg, data ) == messages_return::cancel_all )
			{
				loop = false;
				res = -2;
			}
		};
	}
	hide();
	if(zapOnExit)
	{
		return (selected);
	}
	else
	{
		return (res);
	}
}

void CBouquetList::hide()
{
	g_FrameBuffer->paintBackgroundBoxRel(x,y, width,height);
}


void CBouquetList::paintItem(int pos)
{
	int ypos = y+ theight+0 + pos*fheight;
	int color = COL_MENUCONTENT;
	if (liststart+pos==selected)
	{
		color = COL_MENUCONTENTSELECTED;
	}

	g_FrameBuffer->paintBoxRel(x,ypos, width- 15, fheight, color);
	if(liststart+pos<Bouquets.size())
	{
		CBouquet* bouq = Bouquets[liststart+pos];
		//number - zum direkten hinh�pfen
		char tmp[10];
		sprintf((char*) tmp, "%d", liststart+pos+ 1);

		int numpos = x+5+numwidth- g_Fonts->channellist_number->getRenderWidth(tmp);
		g_Fonts->channellist_number->RenderString(numpos,ypos+fheight, numwidth+5, tmp, color, fheight);

		g_Fonts->channellist->RenderString(x+ 5+ numwidth+ 10, ypos+ fheight, width- numwidth- 20- 15, bouq->name.c_str(), color);
	}
}

void CBouquetList::paintHead()
{
	g_FrameBuffer->paintBoxRel(x,y, width,theight+0, COL_MENUHEAD);
	g_Fonts->menu_title->RenderString(x+10,y+theight+0, width, "Bouquets" /*g_Locale->getText(name).c_str()*/, COL_MENUHEAD);
}

void CBouquetList::paint()
{
	liststart = (selected/listmaxshow)*listmaxshow;
	int lastnum =  liststart + listmaxshow;

	if(lastnum<10)
		numwidth = g_Fonts->channellist_number->getRenderWidth("0");
	else if(lastnum<100)
		numwidth = g_Fonts->channellist_number->getRenderWidth("00");
	else if(lastnum<1000)
		numwidth = g_Fonts->channellist_number->getRenderWidth("000");
	else if(lastnum<10000)
		numwidth = g_Fonts->channellist_number->getRenderWidth("0000");
	else // if(lastnum<100000)
		numwidth = g_Fonts->channellist_number->getRenderWidth("00000");

	for(unsigned int count=0;count<listmaxshow;count++)
	{
		paintItem(count);
	}

	int ypos = y+ theight;
	int sb = fheight* listmaxshow;
	g_FrameBuffer->paintBoxRel(x+ width- 15,ypos, 15, sb,  COL_MENUCONTENT+ 1);

	int sbc= ((Bouquets.size()- 1)/ listmaxshow)+ 1;
	float sbh= (sb- 4)/ sbc;
	int sbs= (selected/listmaxshow);

	g_FrameBuffer->paintBoxRel(x+ width- 13, ypos+ 2+ int(sbs* sbh) , 11, int(sbh),  COL_MENUCONTENT+ 3);

}

