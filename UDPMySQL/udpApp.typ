(********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * File: Global.typ
 * Author: goran
 * Created: January 23, 2013
 ********************************************************************
 * Global data types of project Wireline
 ********************************************************************)

TYPE
	fifoMangerConst_typ : 
		(
		CONST_JSONMSG_MAXID := 200,
		CONST_FIFO_MAX_SQL_MSG_SIZE := 3000,
		CONST_FIFO_MAX_SQL_ENTRIES := 10,
		CONST_FIFO_MAX_SQL_ENTRYID := 9,
		CONST_FIFO_MAX_ENTRIES := 40,
		CONST_FIFO_MAX_ENTRYID := 39
		);
	appDataRecordSQLQuery_typ : 	STRUCT 
		queryId : UDINT;
		text : STRING[CONST_FIFO_MAX_SQL_MSG_SIZE];
	END_STRUCT;
END_TYPE
