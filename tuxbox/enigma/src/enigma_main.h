#ifndef __enigma_main_h
#define __enigma_main_h

#include <sselect.h>
#include <enigma_lcd.h>
#include <lib/dvb/si.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/multipage.h>
#include <lib/gui/emessage.h>
#include <lib/gui/numberactions.h>
#include <lib/gui/textinput.h>
#include <lib/base/message.h>
#include <lib/dvb/service.h>

class eLabel;
class eProgress;
class EIT;
class SDT;
class SDTEntry;
class PMT;
class PMTEntry;
class eNumber;
class eCheckbox;
class eButton;
class gPainter;
class NVODReferenceEntry;
class LinkageDescriptor;
class eServiceSelector;
class eRecordingStatus;
class ePlaylistEntry;
class eHelpWindow;
class eEPGWindow;

class eZapMessage
{
	int type;
	eString caption, body;
	int timeout;
public:
	eZapMessage(int type, const eString &caption, const eString &body, int timeout=0)
		: type(type), caption(caption), body(body), timeout(timeout)
	{
	}

	eZapMessage::eZapMessage()
	{
	}

	eZapMessage::eZapMessage(int type)
		: type(type)
	{
	}

	const eString & getCaption() const { return caption; }
	const eString & getBody() const { return body; }
	void setTimeout(int _timeout)
	{
		timeout=_timeout;
	}
	int getTimeout() const
	{
		return timeout;
	}
	bool isSameType(const eZapMessage &msg) const
	{
		return type == msg.type;
	}
	bool isEmpty() const
	{
		return ! (caption.length() || body.length());
	}
};

class eZapSeekIndices
{
private:
	eString filename;
	std::map<int,int> index;
	int changed;
public:
	void load(const eString &filename);
	void save();
	void add(int real, int time);
	void remove(int real);
	/**
	 * \brief retrieves next index.
	 * \param dir 0 for nearest, <0 for prev or >0 for next. returns -1 if nothing found.
	 */
	int getNext(int, int dir); 
	int getTime(int real);
		// you don't need that.
	std::map<int,int> &getIndices();
};

class NVODStream: public eListBoxEntryTextStream
{
	friend class eListBox<NVODStream>;
	friend class eNVODSelector;
	int begTime;
	const eString &redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
	void EITready(int error);
	int validate();
	void selfDestroy();
	Signal0<void> ready;
	bool operator<( const eListBoxEntry& e ) const
	{
		return begTime < ((NVODStream&)e).begTime;
	}
public:
	int getBegTime() const { return begTime; }
	NVODStream(eListBox<NVODStream> *listbox, eDVBNamespace dvb_namespace, const NVODReferenceEntry *ref, int type);
	eServiceReferenceDVB service;
	EIT eit;
};

class eNVODSelector: public eListBoxWindow<NVODStream>
{
	int count;
	void selected(NVODStream *);
	Signal0<void> clearEntrys;
	void readyCallBack();
public:
	eNVODSelector();
	void clear();
	void add(eDVBNamespace dvb_namespace, NVODReferenceEntry *ref);
};

class AudioStream: public eListBoxEntryText
{
	friend class eListBox<AudioStream>;
	friend class eAudioSelector;
	int isAC3, isDTS;
	int component_tag;
	void EITready(int error);
	void parseEIT(EIT* eit);
public:
	AudioStream(eListBox<AudioStream> *listbox, PMTEntry *stream);
	PMTEntry *stream;
	bool operator < ( const AudioStream& e) const	{	return 0;	}
};

class eAudioSelector: public eListBoxWindow<AudioStream>
{
	void selected(AudioStream *);
public:
	eAudioSelector();
	void clear();
	void add(PMTEntry *);
};

class VideoStream: public eListBoxEntryText
{
	friend class eListBox<VideoStream>;
	friend class eVideoSelector;
	int component_tag;
	void EITready(int error);
	void parseEIT(EIT* eit);
public:
	VideoStream(eListBox<VideoStream> *listbox, PMTEntry *stream);
	PMTEntry *stream;
	bool operator < ( const VideoStream& e) const	{	return 0;	}
};

class eVideoSelector: public eListBoxWindow<VideoStream>
{
	void selected(VideoStream *);
public:
	eVideoSelector();
	void clear();
	void add(PMTEntry *);
};

class SubService: public eListBoxEntryText
{
	friend class eListBox<SubService>;
	friend class eSubServiceSelector;
public:
	SubService(eListBox<SubService> *listbox, eDVBNamespace dvb_namespace, const LinkageDescriptor *descr);
	eServiceReferenceDVB service;
};

class eSubServiceSelector
: public eListBoxWindow<SubService>
{
	int quickzap;
	void selected(SubService *);
	eButton *bAddToUserBouquet, *bToggleQuickZap;
	virtual void willShow();
	void addPressed();
	void quickZapPressed();
public:
	Signal2<void, eServiceReference*, int> addToUserBouquet;
	bool quickzapmode();
	void prev();
	void next();
	eSubServiceSelector(bool showbuttons=true);
	eServiceReferenceDVB *getSelected() { return list.getCount()?&list.getCurrent()->service:0; }
	void clear();
	void selectCurrent();
	void disableQuickZap();
	void add(eDVBNamespace dvb_namespace, const LinkageDescriptor *ref);
};

class eServiceNumberWidget: public eWindow
{
	eNumber *number;
	int chnum;
	eTimer *timer;
private:
	void selected(int*);
	void timeout();
	void numberChanged();
public:
	eServiceNumberWidget(int initial);
	~eServiceNumberWidget();
};

#define ENIGMA_NVOD		1	
#define ENIGMA_AUDIO	2
#define ENIGMA_SUBSERVICES 4
#define ENIGMA_VIDEO	8

class eEventDisplay;
class eServiceEvent;

class ePlaylist;

class TextEditWindow: public eWindow
{
	eTextInputField *input;
	eLabel *descr, *image;
	eButton *save, *cancel;
public:
	TextEditWindow( const char *InputFieldDescr, const char* useableChar=0 );
	const eString& getEditText() { return input->getText(); }
	void setEditText( const eString& str ) { input->setText( str ); }
	void setMaxChars( int maxChars ) { input->setMaxChars( maxChars ); }
};

class UserBouquetSelector: public eListBoxWindow<eListBoxEntryText>
{
	std::list<ePlaylistEntry> &SourceList;
	void selected( eListBoxEntryText * );
public:
	UserBouquetSelector( std::list<ePlaylistEntry> &list );
	eServiceReference curSel;
};

class eZapMain: public eWidget
{
	friend class eEPGSelector;
public:
	enum { modeTV, modeRadio, modeFile, modeEnd };
	enum { stateSleeping=2, stateInTimerMode=4, stateRecording=8, recDVR=16, recVCR=32, recNgrab=64 };
	enum { messageGoSleep=2, messageShutdown, messageNoRecordSpaceLeft };

private:
	eLabel 	*ChannelNumber, *ChannelName, *Clock,
		*EINow, *EINext, *EINowDuration, *EINextDuration,
		*EINowTime, *EINextTime, *Description, *fileinfos,
		*ButtonRedEn, *ButtonRedDis, 
		*ButtonGreenEn, *ButtonGreenDis, 
		*ButtonYellowEn, *ButtonYellowDis,
		*ButtonBlueEn, *ButtonBlueDis,
		*DolbyOn, *DolbyOff, *CryptOn, *CryptOff, *WideOn, *WideOff, *recstatus,
		mute, volume,
		*DVRSpaceLeft;

	eWidget *dvrInfoBar, *dvbInfoBar, *fileInfoBar;
	int dvrfunctions;

	// eRecordingStatus *recstatus;

	eProgress *Progress, *VolumeBar;
	eMessageBox *pMsg, *pRotorMsg;

	eLock messagelock;
	std::list<eZapMessage> messages;
	eFixedMessagePump<int> message_notifier;

	eTimer timeout, clocktimer, messagetimeout,
					progresstimer, volumeTimer, recStatusBlink,
					doubleklickTimer, delayedStandbyTimer;

	Connection doubleklickTimerConnection;

	int cur_start, cur_duration, cur_event_id;
	eString cur_event_text;
	
	eNVODSelector nvodsel;
	eSubServiceSelector subservicesel;
	eAudioSelector audiosel;
	eVideoSelector videosel;
	eEventDisplay *actual_eventDisplay;
	eServiceReferenceDVB refservice;
	
	ePlaylist *playlist; // history / current playlist entries
	eServiceReference playlistref;

#ifndef DISABLE_FILE
	ePlaylist *recordings;
	eServiceReference recordingsref;
#endif 

	ePlaylist *userTVBouquets;
	eServiceReference userTVBouquetsRef;

	ePlaylist *userRadioBouquets;
	eServiceReference userRadioBouquetsRef;

	ePlaylist *userFileBouquets;
	eServiceReference userFileBouquetsRef;

	ePlaylist *currentSelectedUserBouquet; // in addToFavourite Mode...
	eServiceReference currentSelectedUserBouquetRef;

	eString eplPath; // where we store Favlists? user changeable...

	ePlaylist *addUserBouquet( ePlaylist*, const eString&, const eString&, eServiceReference &, bool );
	
  int wasSleeping;
	int playlistmode; // curlist is a list controlled by the user (not just a history).
	int entered_playlistmode;

	int flags;
	int isVT;
	int isEPG;
	int showOSDOnEITUpdate;
	int serviceFlags;
	int isSeekable() const { return serviceFlags & eServiceHandler::flagIsSeekable; }
#ifndef DISABLE_LCD
	eZapLCD lcdmain;
#endif
	void eraseBackground(gPainter *, const eRect &where);
	void setEIT(EIT *);
	void handleNVODService(SDTEntry *sdtentry);

	// actions
	void showServiceSelector(int dir, eServiceReference root=eServiceReference(), eServiceReference path=eServiceReference(), int newmode=-1 );
	void nextService(int add=0);
	void prevService();
	void playlistNextService();
	void volumeUp();
	void volumeDown();
	void hideVolumeSlider();
	void toggleMute();
	void showMainMenu();

	timeval standbyTime;

	void delayedStandby();
	void standbyPress();
	void standbyRepeat();
	void standbyRelease();
	void showSubserviceMenu();
	void showAudioMenu();
	void runVTXT();
	void runPluginExt();
	void showEPGList();
	void showEPG();
	void showEPG_Streaminfo();	
	void showInfobar();
	void hideInfobar();
	void showHelp( ePtrList<eAction>*, int );

#ifndef DISABLE_FILE
	void play();
	void stop();
	void pause();
	void record();
	enum { skipForward, skipReverse };
	int skipcounter;
	int skipping;
	void startSkip(int dir);
	void repeatSkip(int dir);
	void stopSkip(int dir);
#endif

	void showServiceMenu(eServiceSelector*);

	void addService(const eServiceReference &service);

	void doPlaylistAdd(const eServiceReference &service);
	void addServiceToUserBouquet(eServiceReference *s, int dontask=0);
	void addServiceToCurUserBouquet(const eServiceReference &ref);
	void removeServiceFromUserBouquet( eServiceSelector *s );

	enum { listAll, listSatellites, listProvider, listBouquets };
	void showList(int list);

	void showServiceInfobar(int show);

	static eZapMain *instance;

	eServicePath modeLast[modeEnd][2];

	int mode,             // current mode .. TV, Radio, File
			state;
	void onRotorStart( int newPos );
	void onRotorStop();
	void onRotorTimeout();
protected:
	int eventHandler(const eWidgetEvent &event);
private:
	bool CheckService(const eServiceReference &ref );
	void handleServiceEvent(const eServiceEvent &event);
	void startService(const eServiceReference &, int);
	void gotEIT();
	void gotSDT();
	void gotPMT();
	void timeOut();
	void leaveService();
	void clockUpdate();
	void updateVolume(int mute_state, int vol);
	void prepareDVRHelp();
	void prepareNonDVRHelp();
	void set16_9Logo(int aspect);
	void setSmartcardLogo(bool b);
	void setAC3Logo(bool b);
	void setVTButton(bool b);
	void setEPGButton(bool b);
	void updateProgress();
	void updateServiceNum( const eServiceReference& );
	void getPlaylistPosition();
	void setPlaylistPosition();
	bool handleState(int justask=0);

#ifndef DISABLE_FILE
	void blinkRecord();
	void toggleIndexmark();
	eZapSeekIndices indices;
	ePtrList<eLabel> indexmarks;
	void redrawIndexmarks();
#endif // DISABLE_FILE
public:
	void toggleScart( int state );
	void postMessage(const eZapMessage &message, int clear=0);
	void gotMessage(const int &);
	void startMessages();
	void stopMessages();
	void pauseMessages();
	void nextMessage();
	static eZapMain *getInstance() { return instance; }

	enum {
		psAdd=1,      // just add, to not change current
		psRemove=2,   // remove before add
		psActivate=4, // use existing entry if available
		psDontAdd=8,  // just play
		psSeekPos=16, //
		psSetMode=32
	};

	void playService(const eServiceReference &service, int flags);
	void playlistPrevService();

#ifndef DISABLE_FILE
	int recordDVR(int onoff, int user, const char* event_name=0 ); // starts recording
	const eServiceReference& getRecordingsref() { return recordingsref; }
#endif

	void setMode(int mode, int user=0); // user made change?
	int getMode() { return mode; }

	void rotateRoot();
	void toggleTimerMode();
	void toggleEditMode(eServiceSelector *);
	void toggleMoveMode(eServiceSelector *);
	void handleStandby();

	void setNewServiceSelectorRoot( eServiceReference root=eServiceReference(), eServiceReference path=eServiceReference() );
	void setServiceSelectorPath(eServicePath path);
	void getServiceSelectorPath(eServicePath &path);

	void moveService(const eServiceReference &path, const eServiceReference &ref, const eServiceReference &after);
	void showMultiEPG();
	
	void loadPlaylist( bool create = false );
	void savePlaylist( bool destory = false );

#ifndef DISABLE_FILE
	void loadRecordings( bool create = false );
	void saveRecordings( bool destory = false );
#endif

	void loadUserBouquets( bool destroy=true );  // this recreate always all user bouquets...

	void destroyUserBouquets( bool save=false ); 

	void saveUserBouquets();  // only save
	
	void reloadPaths(int reset=0);

	eZapMain();
	~eZapMain();
};

class eServiceContextMenu: public eListBoxWindow<eListBoxEntryText>
{
	eServiceReference ref;
	void entrySelected(eListBoxEntryText *s);
public:
	eServiceContextMenu(const eServiceReference &ref, const eServiceReference &path, eWidget *LCDTitle=0, eWidget *LCDElement=0);
};

class eSleepTimerContextMenu: public eListBoxWindow<eListBoxEntryText>
{
	eServiceReference ref;
	void entrySelected(eListBoxEntryText *s);
public:
	eSleepTimerContextMenu(eWidget *LCDTitle=0, eWidget *LCDElement=0);
};

class eShutdownStandbySelWindow: public eWindow
{
protected:
	eCheckbox *Standby, *Shutdown;
	eButton *cancel;
	void StandbyChanged(int);
	void ShutdownChanged(int);
	void fieldSelected(int *){focusNext(eWidget::focusDirNext);}
	virtual void setPressed()=0;
	eButton *set;
	eNumber *num;
public:
	int getCheckboxState();
	eShutdownStandbySelWindow( eWidget *parent, int len, int min, int max, int maxdigits, int *init, int isactive=0, eWidget* descr=0, int grabfocus=1, const char* deco="eNumber" );
};

class eSleepTimer: public eShutdownStandbySelWindow
{
	void setPressed();
public:
	eSleepTimer();
};

#ifndef DISABLE_FILE

class eRecordContextMenu: public eListBoxWindow<eListBoxEntryText>
{
	eServiceReference ref;
	void entrySelected(eListBoxEntryText *s);
public:
	eRecordContextMenu(eWidget *LCDTitle=0, eWidget *LCDElement=0);
};

class eRecTimeInput: public eShutdownStandbySelWindow
{
	void setPressed();
public:
	eRecTimeInput();
};

class eTimerInput: public eShutdownStandbySelWindow
{
	void setPressed();
public:
	eTimerInput();
};

#endif //DISABLE_FILE

#endif /* __enigma_main_h */
