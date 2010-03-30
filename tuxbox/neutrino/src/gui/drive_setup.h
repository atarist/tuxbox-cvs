/*
	$Id: drive_setup.h,v 1.22 2010/03/29 19:48:15 dbt Exp $

	Neutrino-GUI  -   DBoxII-Project

	hdd setup implementation, fdisk frontend for Neutrino gui

	based upon ideas for the neutrino ide_setup by Innuendo and riker

	Copyright (C) 2009 Thilo Graf (dbt)
	http://www.dbox2-tuning.de

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

	Special thx for hardware support by stingray www.dbox2.com and gurgel www.hallenberg.com !

*/

#ifndef __drive_setup__
#define __drive_setup__

#include <configfile.h>

#include <gui/widget/menue.h>
#include <gui/widget/dirchooser.h>

#include <driver/framebuffer.h>
#include <system/settings.h>

#include <string>
#include <vector>

// maximal count of usable devices
#define MAXCOUNT_DRIVE 3
// possible count of partitions per device
#define MAXCOUNT_PARTS 4
// maximal count of possible supported mmc driver modules
#define MAXCOUNT_MMC_MODULES 3

// drive settings
struct SDriveSettings
{
	std::string 	drive_partition_mountpoint[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
	std::string 	drive_advanced_modul_command_load_options;
	std::string 	drive_modul_dir;
	std::string 	drive_mmc_modul_parameter[MAXCOUNT_MMC_MODULES];

	int 	drive_use_fstab;
	int	drive_use_fstab_auto_fs;

	int 	drive_activate_ide;
	int 	drive_write_cache[MAXCOUNT_DRIVE];
	int 	drive_partition_activ[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
	char	drive_partition_fstype[MAXCOUNT_DRIVE][MAXCOUNT_PARTS][8];
	char 	drive_spindown[MAXCOUNT_DRIVE][3];
	char 	drive_partition_size[MAXCOUNT_DRIVE][MAXCOUNT_PARTS][8];
	char 	drive_mmc_module_name[10];
	char	drive_swap_size[3];

#ifdef ENABLE_NFSSERVER
	std::string 	drive_partition_nfs_host_ip[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
	int 	drive_partition_nfs[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
#endif /*ENABLE_NFSSERVER*/

#ifdef ENABLE_SAMBASERVER
	std::string 	drive_partition_samba_share_name[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
	std::string 	drive_partition_samba_share_comment[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
	int 	drive_partition_samba[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
 	int 	drive_partition_samba_ro[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
 	int 	drive_partition_samba_public[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];
#endif /*ENABLE_SAMBASERVER*/

};


enum ON_OFF_NUM	
{
	OFF,
	ON
};
// switch on/off Option
#define OPTIONS_ON_OFF_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_ON_OFF_OPTIONS[OPTIONS_ON_OFF_OPTION_COUNT] =
{
	{ OFF, LOCALE_OPTIONS_OFF  },
	{ ON, LOCALE_OPTIONS_ON }
};


enum YES_NO_NUM	
{
	NO,
	YES
};
// switch enable/disable partition
#define OPTIONS_YES_NO_OPTION_COUNT 2
const CMenuOptionChooser::keyval OPTIONS_YES_NO_OPTIONS[OPTIONS_YES_NO_OPTION_COUNT] =
{
	{ NO, LOCALE_DRIVE_SETUP_PARTITION_ACTIVATE_NO  },
	{ YES, LOCALE_DRIVE_SETUP_PARTITION_ACTIVATE_YES }
};


// modes count for enum IDE_DRIVERMODES collection
enum IDE_DRIVERMODES
{
	IDE_OFF,
	IDE_ACTIVE,
	IDE_ACTIVE_IRQ6
};
// switch activate/deactivate ide interface
#define OPTIONS_IDE_ACTIVATE_OPTION_COUNT 3
const CMenuOptionChooser::keyval OPTIONS_IDE_ACTIVATE_OPTIONS[OPTIONS_IDE_ACTIVATE_OPTION_COUNT] =
{
	{ IDE_OFF, LOCALE_OPTIONS_OFF  },
	{ IDE_ACTIVE, LOCALE_DRIVE_SETUP_IDE_ACTIVATE_ON },
	{ IDE_ACTIVE_IRQ6, LOCALE_DRIVE_SETUP_IDE_ACTIVATE_IRQ6 }
};


class CDriveSetup : public CMenuTarget
{
	private:
		
		typedef enum 	
		{
			ADD,
			DELETE,
			DELETE_CLEAN
		} action_int_t;

		// set nums for commands collection, used in v_init_ide_L_cmds, this is also the order of commands in the init file
		// commands count for enum INIT_COMMANDS collection
		#define INIT_COMMANDS_COUNT 6
		typedef enum 	
		{
			LOAD_IDE_CORE,
			LOAD_DBOXIDE,
			LOAD_IDE_DETECT,
			LOAD_IDE_DISK,
			SET_MASTER_HDPARM_OPTIONS,
			SET_SLAVE_HDPARM_OPTIONS
		} INIT_COMMANDS;

		typedef enum 	
		{
			DEVICE,
			MOUNTPOINT,
			FS,
			OPTIONS
		} MTAB_INFO_NUM;
		
		#define SWAP_INFO_NUM_COUNT 5
		typedef enum 	
		{
			FILENAME,
			TYPE,
			SIZE,
			USED,
			PRIORITY
		} SWAP_INFO_NUM;

		typedef enum 	
		{
			START_CYL,
			END_CYL,
			SIZE_BLOCKS,
			ID,
			SIZE_CYL,
			COUNT_CYL,
			PART_SIZE
		} PARTINFO_TYPE_NUM;

		typedef enum   //column numbers	
		{
			FDISK_INFO_START_CYL	= 1,
			FDISK_INFO_END_CYL	= 2,
			FDISK_INFO_SIZE_BLOCKS	= 3,
			FDISK_INFO_ID		= 4
		} fdisk_info_uint_t;

		#define INIT_FILE_TYPE_NUM_COUNT 2
		typedef enum 	
		{
			INIT_FILE_MODULES,
			INIT_FILE_MOUNTS
		} INIT_FILE_TYPE_NUM;

		CFrameBuffer 	*frameBuffer;
		CConfigFile	configfile;
		SDriveSettings	d_settings;

		int x, y, width, height, hheight, mheight;
		int pb_x, pb_y, pb_w, pb_h;
		int msg_timeout; 	// timeout for messages
	

		const char* msg_icon; 	// icon for all hdd setup windows
		char part_num_actionkey[MAXCOUNT_PARTS][17];
		std::string make_part_actionkey[MAXCOUNT_PARTS]; //action key strings for make_partition_$
		std::string mount_partition[MAXCOUNT_PARTS]; //action key strings for mount_partition_$
		std::string unmount_partition[MAXCOUNT_PARTS]; //action key strings for unmount_partition_$
		std::string delete_partition[MAXCOUNT_PARTS]; //action key strings for delete_partition_$
		std::string check_partition[MAXCOUNT_PARTS]; //action key strings for check_partition_$
		std::string format_partition[MAXCOUNT_PARTS]; //action key strings for format_partition_$
		std::string sel_device_num_actionkey[MAXCOUNT_DRIVE]; //"sel_device_0 ... sel_device_n""
		std::string s_init_mmc_cmd; // system load command for mmc modul
		std::string mmc_modules[MAXCOUNT_MMC_MODULES]; //all supported mmc modules
		std::string moduldir[4]; //possible dirs of modules
		std::string k_name; //kernel name
		std::string partitions[MAXCOUNT_DRIVE][MAXCOUNT_PARTS];

		//error messages
		#define ERROR_DESCRIPTIONS_NUM_COUNT 26 
		typedef enum 	
		{
			ERR_CHKFS,
			ERR_FORMAT_PARTITION,
			ERR_HDPARM,
			ERR_INIT_FSDRIVERS,
			ERR_INIT_IDEDRIVERS,
			ERR_INIT_MMCDRIVER,
			ERR_INIT_MODUL,
			ERR_MK_PARTITION,
			ERR_MK_EXPORTS,
			ERR_MK_FS,
			ERR_MK_FSTAB,
			ERR_MK_MOUNTS,
			ERR_MK_SMBCONF,
			ERR_MK_SMBINITFILE,
			ERR_MOUNT_ALL,
			ERR_MOUNT_PARTITION,
			ERR_MOUNT_DEVICE,
			ERR_SAVE_DRIVE_SETUP,
			ERR_UNLOAD_FSDRIVERS,
			ERR_UNLOAD_IDEDRIVERS,
			ERR_UNLOAD_MMC_DRIVERS,
			ERR_UNLOAD_MODUL,
			ERR_UNMOUNT_ALL,
			ERR_UNMOUNT_PARTITION,
			ERR_UNMOUNT_DEVICE,
			ERR_WRITE_SETTINGS
		} errnum_uint_t;
		std::string err[ERROR_DESCRIPTIONS_NUM_COUNT];
		
		int current_device; 	//MASTER || SLAVE || MMCARD, current edit device
		int hdd_count; 		// count of hdd drives
 		int part_count[MAXCOUNT_DRIVE /*MASTER || SLAVE || MMCARD*/]; //count of partitions at device
		int next_part_number;// number of next free partition that can be added from device 1...4

		unsigned long long start_cylinder;
		unsigned long long end_cylinder;
		unsigned long long part_size;

		void setStartCylinder();

		bool device_isActive[MAXCOUNT_DRIVE /*MASTER || SLAVE || MMCARD*/];
		bool have_mmc_modul[MAXCOUNT_MMC_MODULES];	//collection of mmc modules state true=available false=not available 

		std::vector<std::string> v_mmc_modules;		//collection of available mmc modules
		std::vector<std::string> v_mmc_modules_opts;	//collection of available mmc module options, must be synchronized with v_mmc_modules !!!
		std::vector<std::string> v_model_name;		//collection of names of models
		std::vector<std::string> v_fs_modules;		//collection of available fs modules
		std::vector<std::string> v_init_ide_L_cmds; 	//collection of ide load commands
		std::vector<std::string> v_init_fs_L_cmds; 	//collection of fs load commands
		std::vector<std::string> v_init_fs_U_cmds; 	//collection of fs unload commands
		std::vector<std::string> v_device_temp;  	//collection of temperature of devices
		std::vector<std::string> v_hdparm_cmds;		//collection of available hdparm commands

		unsigned long long device_size[MAXCOUNT_DRIVE];		// contains sizes of all devices
		unsigned long long device_cylcount[MAXCOUNT_DRIVE]; 	// contains count of devices for all devices
		unsigned long long device_cyl_size[MAXCOUNT_DRIVE]; 	// contains bytes of one cylinder for all devices in bytes
		unsigned long long device_heads_count[MAXCOUNT_DRIVE];	 // contains count of heads
		unsigned long long device_sectors_count[MAXCOUNT_DRIVE]; // contains count of sectors

		const char *getFsTypeStr(long &fs_type_const);

		bool foundHdd(const std::string& mountpoint);
		bool foundMmc();
		bool loadFdiskPartTable(const int& device_num /*MASTER || SLAVE || MMCARD*/, bool use_extra = false /*using extra funtionality table*/);
		bool isActivePartition(const std::string& partname);
		bool isModulLoaded(const std::string& modulname);
		bool isMountedPartition(const std::string& partname);
		bool isSwapPartition(const std::string& partname);
		bool initFsDrivers(bool do_unload_first = true);
		bool loadHddParams(const bool do_reset = false);
		bool initIdeDrivers(const bool irq6 = false);
		bool initModulDeps(const std::string& modulname);
		bool initModul(const std::string& modul_name, bool do_unload_first = true, const std::string& options = "");
		bool mountPartition(const int& device_num /*MASTER || SLAVE || MMCARD*/, const int& part_number,  const std::string& fs_name, const std::string& mountpoint, const bool force_mount = true);
		bool mountDevice(const int& device_num, const bool force_mount = true);
		bool mountAll();
		bool unmountPartition(const int& device_num /*MASTER || SLAVE || MMCARD*/, const int& part_number);
		bool unmountDevice(const int& device_num);
		bool unmountAll();
		bool saveHddSetup();
		bool unloadFsDrivers();
		bool unloadMmcDrivers();
		bool initMmcDriver();
		bool unloadIdeDrivers();
		bool unloadModul(const std::string& modulname);
		bool writeInitFile(const bool clear = false);
		bool mkMounts();
		bool mkFstab(bool write_defaults_only = false);
	#ifdef ENABLE_NFSSERVER
		bool mkExports();
	#endif
	#ifdef ENABLE_SAMBASERVER
		bool mkSmbConf();
		bool mkSambaInitFile();
		std::string getInitSmbFilePath();
	#endif
		bool haveSwap();
		bool isMmcActive();
		bool isMmcEnabled();
		bool isIdeInterfaceActive();
		bool linkInitFiles();
		bool haveActiveParts(const int& device_num);
		bool haveMounts(const int& device_num, bool without_swaps = false);
		
		bool mkPartition(const int& device_num /*MASTER || SLAVE || MMCARD*/, const action_int_t& action, const int& part_number, const unsigned long long& start_cyl = 0, const unsigned long long& size = 0);
		bool mkFs(const int& device_num /*MASTER || SLAVE || MMCARD*/, const int& part_number,  const std::string& fs_name);
		bool chkFs(const int& device_num /*MASTER || SLAVE || MMCARD*/, const int& part_number,  const std::string& fs_name);
		bool formatPartition(const int& device_num, const int& part_number);
		void showStatus(const int& progress_val, const std::string& msg, const int& max = 5); // helper

		unsigned int getFirstUnusedPart(const int& device_num);
		unsigned long long getPartData(const int& device_num /*MASTER || SLAVE || MMCARD*/, const int& part_number, const int& info_t_num /*START||END*/, bool refresh_table = true);
		unsigned long long getPartSize(const int& device_num /*MASTER || SLAVE || MMCARD*/, const int& part_number = -1);
	
		void hide();
		void Init();

		void calPartCount();
		void loadHddCount();
		void loadHddModels();
		void loadFsModulList();
		void loadMmcModulList();
		void loadPartitions();
		void loadFdiskData();
		void loadDriveTemps();
		void loadModulDirs();
						
		void showHddSetupMain();
		void showHddSetupSub();
		void showHelp();

		bool writeDriveSettings();
		void loadDriveSettings();

		unsigned long long getFreeDiskspace(const char *mountpoint);
		unsigned long long getUnpartedDeviceSize(const int& device_num /*MASTER || SLAVE || MMCARD*/);
		unsigned long long getFileEntryLong(const char* filename, const std::string& filter_entry, const int& column_num);

		unsigned long long free_size[MAXCOUNT_DRIVE][MAXCOUNT_PARTS]; // contains free unused size of disc in MB
		unsigned long long calcCyl(const int& device_num /*MASTER || SLAVE || MMCARD*/, const unsigned long long& bytes);

		long long getDeviceInfo(const char *mountpoint/*/hdd...*/, const int& device_info /*KB_BLOCKS,
											KB_AVAILABLE,
											PERCENT_USED,
											PERCENT_FREE,
											FILESYSTEM...*/);
// 		examples: 	getFsTypeStr(getDeviceInfo("/hdd", FILESYSTEM));
// 				getDeviceInfo("/hdd", PERCENT_USED);

		
		std::string getMountInfo(const std::string& partname /*HDA1...HDB4*/, const int& mtab_info_num /*MTAB_INFO_NUM*/);
		std::string getSwapInfo(const std::string& partname /*HDA1...HDB4*/, const int& swap_info_num  /*SWAP_INFO_NUM*/);
		std::string getFileEntryString(const char* filename, const std::string& filter_entry, const int& column_num);
		std::string convertByteString(const unsigned long long& byte_size);
		std::string getUsedMmcModulName();
		std::string getFilePath(const char* main_file, const char* default_file);
		std::string getInitIdeFilePath();
		std::string getInitMountFilePath();
		std::string getFstabFilePath();
	#ifdef ENABLE_NFSSERVER
		std::string getExportsFilePath();
	#endif
		std::string getDefaultSysMounts();
		std::string getDefaultFstabEntries();
		std::string getTimeStamp();
		std::string getInitFileHeader(std::string & filename);
		std::string getInitFileMountEntries();
		std::string getInitFileModulEntries(bool with_unload_entries = false);
		std::string getInitModulLoadStr(const std::string& modul_name);
		std::string getPartEntryString(std::string& partname);
		std::string getMountPoint(const int& device_num, const int& part_number);

		//helper
		std::string iToString(int int_val);

		int exec(CMenuTarget* parent, const std::string & actionKey);

		//settings	
		char mmc_parm[27];
		char mountpoint_opt[31];
		char spindown_opt[17];
		char partsize_opt[25];
		char fstype_opt[27];
		char write_cache_opt[20];
		char partition_activ_opt[26];
	
	#ifdef ENABLE_NFSSERVER
		char partition_nfs_opt[24];
		char partition_nfs_host_ip_opt[32];
	#endif /*ENABLE_NFSSERVER*/

	#ifdef ENABLE_SAMBASERVER
		char partition_samba_opt[26];
		char partition_samba_opt_ro[29];
		char partition_samba_opt_public[35];
		char partition_samba_share_name[35];
		char partition_samba_share_comment[40];
	#endif /*ENABLE_SAMBASERVER*/

	public:
		enum DRIVE_NUM	
		{
			MASTER,
			SLAVE,
			MMCARD
		};
		
		enum MMC_NUM	
		{
			MMC,
			MMC2,
			MMCCOMBO
		};

		#define DEVICE_INFO_COUNT 5
		enum DEVICE_INFO	
		{
			KB_BLOCKS,
			KB_USED,
			KB_AVAILABLE,
			PERCENT_USED,
			PERCENT_FREE,
			FILESYSTEM,
			FREE_HOURS
		};
		
		enum PARTSIZE_TYPE_NUM	
		{
			KB,
			MB
		};
		
		CDriveSetup();
		~CDriveSetup();

		std::string getHddTemp(const int& device_num /*MASTER || SLAVE || MMCARD*/); //hdd temperature
		std::string getModelName(const std::string& mountpoint);
		std::string getDriveSetupVersion();

};

class CDriveSetupFsNotifier : public CChangeObserver
{
	private:

	#ifdef ENABLE_NFSSERVER
		CMenuForwarder* toDisable[3];
	#else
		CMenuForwarder* toDisable[2];
	#endif
	public:
		CDriveSetupFsNotifier( 	
					#ifdef ENABLE_NFSSERVER
						CMenuForwarder*, 
						CMenuForwarder*, 
						CMenuForwarder*);
					#else
						
						CMenuForwarder*, 
						CMenuForwarder*);
					#endif
		bool changeNotify(const neutrino_locale_t, void * Data);
};

#ifdef ENABLE_NFSSERVER
class CDriveSetupNFSHostNotifier : public CChangeObserver
{
	private:
		CMenuForwarder* toDisable;
	public:
		CDriveSetupNFSHostNotifier( CMenuForwarder*);
		bool changeNotify(const neutrino_locale_t, void * Data);
};
#endif /*ENABLE_NFSSERVER*/

#ifdef ENABLE_SAMBASERVER
class CDriveSetupSambaNotifier : public CChangeObserver
{
	private:
		CMenuForwarder* toDisablefw[2];
		CMenuOptionChooser* toDisableoj[2];
	public:
		CDriveSetupSambaNotifier(CMenuForwarder*, CMenuForwarder*, CMenuOptionChooser*, CMenuOptionChooser*);
		bool changeNotify(const neutrino_locale_t, void * Data);
};
#endif /*ENABLE_SAMBASERVER*/

class CDriveSetupFstabNotifier : public CChangeObserver
{
	private:
		CMenuOptionChooser* toDisable;
	public:
		CDriveSetupFstabNotifier( CMenuOptionChooser* );
		bool changeNotify(const neutrino_locale_t, void * Data);
};

class CDriveSetupMmcNotifier : public CChangeObserver
{
	private:
		CMenuForwarder* toModifi;
	public:
		CDriveSetupMmcNotifier( CMenuForwarder* f1);
		bool changeNotify(const neutrino_locale_t, void * Data);
};

#endif