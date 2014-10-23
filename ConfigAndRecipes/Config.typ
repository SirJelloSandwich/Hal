(********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Package: ConfigAndRecipes
 * File: Config.typ
 * Author: goran_2
 * Created: August 06, 2011
 ********************************************************************
 * Data types of package ConfigAndRecipes
 ********************************************************************)

TYPE
	cfgConfig_enums : 
		(
		ENUM_APPCFG_INFO_TEXT_LEN := 127,
		ENUM_APPCFG_PATHNAME_MAXLEN := 64,
		ENUM_APPCFG_PWD_MAXLEN := 15,
		ENUM_APPCFG_DEFECT_MAXINDEX := 11,
		ENUM_APPCFG_DEFECT_TXTMAXLEN := 31,
		ENUM_APPCFG_LOOKUP_MAXIDX := 10,
		MAGIC_NUMBER := 16#55555555 (*Magic word (010101...) sequence (to detect battery backed up memory alterations)*)
		);
END_TYPE

(*==================================================================*)
(*Application specific config related data structures. Change as needed!*)
(*==================================================================*)

TYPE
	cfgMachineSystem_typ : 	STRUCT 
		infoText : STRING[127]; (*Text stored in the config file to describe the file.*)
		author : STRING[31];
		dateLastSaved : STRING[31];
	END_STRUCT;
	cfgMachineHmi_typ : 	STRUCT 
		loginTimeout : REAL; (*Login timeout timer setting*)
		dialogBoxTimeout : REAL; (*Dialog box (i.e. save data? yes/no) timeout for waiting on input*)
		splashScreenTimeout : REAL; (*Duration of the initial screen (i.e. manufacturer logo) *)
		passwordSupervisor : STRING[15];
		passwordAdmin : STRING[15];
	END_STRUCT;
	cfgMachineTrainier_typ : 	STRUCT 
		lineDepth : REAL;
		casingBottom : REAL;
		freewheelStart : UDINT;
		totalBottom : REAL;
	END_STRUCT;
	cfgMachineDiag_typ : 	STRUCT 
		suppressAlarmIOModule : UDINT;
		diagMode : UDINT;
	END_STRUCT;
END_TYPE

(*==================================================================*)
(*Application config structure. Do not change the type name! Change content as needed.*)
(*==================================================================*)

TYPE
	cfgMachineConfig_typ : 	STRUCT  (*Machine configuration includes settings for timers and settings for servo drives.*)
		system : cfgMachineSystem_typ;
		hmi : cfgMachineHmi_typ;
		trainer : cfgMachineTrainier_typ;
		diag : cfgMachineDiag_typ;
	END_STRUCT;
END_TYPE

(*############################################################################################################################################################*)
(*############################################################################################################################################################*)
(*############################################################################################################################################################*)
(*==================================================================*)
(*Config related structures that shall not be changed.*)
(*==================================================================*)

TYPE
	New_Datatype : 	STRUCT 
	END_STRUCT;
	cfgConfigPrivate_typ : 	STRUCT  (*Machine configuration includes settings for timers and settings for servo drives.*)
		isValid : UDINT; (*Set to 1 after starup when config data is restored, ready and available*)
		isPowerupDone : UDINT; (*Set to 1 after power cycle is complete*)
		cfgFilename : STRING[ENUM_APPCFG_PATHNAME_MAXLEN];
		cfgFilenameFactory : STRING[ENUM_APPCFG_PATHNAME_MAXLEN];
		isChanged : UDINT; (*Set to 1 when cfg changes (i.e. editing or similar)*)
		isChangeAccepted : UDINT; (*Set to 1 when user confirms that change shall be accepted*)
		isChangeRejected : UDINT; (*Set to 1 when user confirms that change shall be rejected*)
		hmiBtnSaveCfgVisible : UDINT; (*Set to 0 when hmi button to allow cfg saveing shall be visible *)
		hmiBtnSaveCfgPressed : UDINT; (*Set to 1 by the hmi button to trigger saveing of cfg*)
		reqConfigWr : UDINT;
		reqConfigRd : UDINT; (*Set to 1 to request config file rd*)
		reqOk : UDINT; (*Set to 1 if request completed ok (clear before issueing a request)*)
		reqError : UDINT; (*Set to 1 if request completed w. error (clear before issuing a request)*)
		writecounterok : UINT;
		writecountererr : UINT;
		readcounterok : UINT;
		readcountererr : UINT;
	END_STRUCT;
	cfgConfig_typ : 	STRUCT  (*Machine configuration includes settings for timers and settings for servo drives.*)
		active : cfgMachineConfig_typ;
		cfgDefault : cfgMachineConfig_typ;
		prv : cfgConfigPrivate_typ; (*Private data to manage config file storage*)
	END_STRUCT;
END_TYPE
