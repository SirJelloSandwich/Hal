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
	cfgRemanentData_typ : 	STRUCT  (*Configuration and recipe related data stored in remanent (battery backed) memory*)
		magicPrefix : UDINT; (*Set to 0x55555555 (MAGIC_NUMBER) to detect when/if battery backed memory changes*)
		dataIsValid : BOOL; (*Flag set at startup to indicate if content of the remanent memory is valid or not*)
		currentRecId : UINT; (*Index of the current recipe*)
		activeCfg : cfgMachineConfig_typ; (*Current / active machine configuration*)
		activeRec : cfgMachineRecipe_typ; (*Current / active machine recipe*)
		magicPosfix : UDINT; (*Set to 0x55555555 (MAGIC_NUMBER) to detect when/if battery backed memory changes*)
	END_STRUCT;
END_TYPE
