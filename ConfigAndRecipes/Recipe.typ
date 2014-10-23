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
(*==================================================================*)
(*Application recipe enums. Change as needed.*)
(*==================================================================*)

TYPE
	cfgRecipe_enums : 
		(
		APP_RECIPE_NAMELEN := 127, (*Length of the recipe name/description string*)
		APP_RECIPE_SHORTSTR_LEN := 15, (*Length of the recipe name/description string*)
		APP_RECIPE_MEDIUMSTR_LEN := 31, (*Length of the recipe name/description string*)
		APP_RECIPE_IDLEN := 5, (*Length of the recipe ID (as a string)*)
		APP_RECIPE_MAXID := 30 (*Max value for a recipe ID#. Recipe ID-s are in range 1..MAX_ID*)
		);
END_TYPE

(*==================================================================*)
(*Application specific recipe related data structures. Change as needed!*)
(*==================================================================*)

TYPE
	cfgRecipeGeneric_typ : 	STRUCT 
		value1 : REAL; (*Example PV1*)
		value2 : REAL; (*Example PV2*)
	END_STRUCT;
END_TYPE

(*==================================================================*)
(*Application recipe structure. Do not change the type name! Change content as needed.*)
(*==================================================================*)

TYPE
	cfgMachineRecipe_typ : 	STRUCT  (*Recipe structure*)
		name : STRING[126]; (*Recipe name*)
		author : STRING[31];
		dateLastSaved : STRING[31];
		settings : cfgRecipeGeneric_typ; (*Generic recipe settings.*)
	END_STRUCT;
END_TYPE

(*############################################################################################################################################################*)
(*############################################################################################################################################################*)
(*############################################################################################################################################################*)
(*==================================================================*)
(*Recipe related structures that shall not be changed.*)
(*==================================================================*)

TYPE
	cfgRecipePrivate_typ : 	STRUCT  (*Data to manage recipes - DO NOT CHANGE*)
		isValid : BOOL; (*Set to 1 after starup when recipe data is restored, ready and available*)
		isChanged : BOOL; (*Set to 1 when recipe changes (i.e. editing or similar)*)
		isChangeAccepted : BOOL; (*Set to 1 when user confirms that change shall be accepted*)
		isChangeRejected : BOOL; (*Set to 1 when user confirms that change shall be rejected*)
		reqWr : BOOL; (*Request for active recipe write*)
		reqWrRec : BOOL; (*Request for active recipe write under the name RexNN where NN is ID of the currently active rec.*)
		reqRd : BOOL; (*Request for active recipe read*)
		reqList : BOOL; (*Request to generate/update recipe list*)
		reqSaveAs : BOOL; (*Request to saveas a recipe*)
		reqDel : BOOL; (*Request to delete a recipe*)
		reqSetCurrent : BOOL; (*Request to set a new current recipe*)
		reqIdx : UINT; (*Index of the recipe int the above request*)
		reqIdx1 : UINT; (*Second index of the recipe in the above request*)
		currentRecId : UINT;
		requestRestoreRecId : BOOL; (*Flag to trigger special case of restoring an recId after REM mem was deleted (eg. battery failed)*)
		counter1 : UINT;
		counter2 : UINT;
		hmiBtnSaveRecVisible : UDINT; (*Set to 0 when hmi button to allow cfg saveing shall be visible *)
		hmiBtnSaveRecPressed : UDINT; (*Set to 1 by the hmi button to trigger saveing of cfg*)
		listRecUsed : ARRAY[0..APP_RECIPE_MAXID]OF BOOL;
		listRecIds : ARRAY[0..APP_RECIPE_MAXID]OF STRING[APP_RECIPE_IDLEN];
		listRecNames : ARRAY[0..APP_RECIPE_MAXID]OF STRING[APP_RECIPE_NAMELEN]; (*Note: this is not a file name of the recipe > it is a field within the recipe structure that is called "name"*)
	END_STRUCT;
	cfgRecipe_typ : 	STRUCT  (*Recipe strucuture. DO NOT CHANGE.*)
		active : cfgMachineRecipe_typ; (*Active recipe*)
		recDefault : cfgMachineRecipe_typ; (*Default recipe (predefined, hardcoded values that are good starting point)*)
		prv : cfgRecipePrivate_typ; (*Private variables to manage recipes*)
	END_STRUCT;
END_TYPE
