/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: appUDP
 * File: appUDP.c
 * Author: goran
 * Created: Jan 25, 2013
 ********************************************************************
 * Implementation of program appUDP
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif

/*
###############################################################################
Utilities
###############################################################################
*/

/**
 Simple logging
 Log goes to an array of PV strings > see _LOCAL STRING log[][] variable below
 Log output is visible via Automation Studio watch window
 Example of a log statement used within the code is: 
 DBG(lgr, "This is log example. %d ", value1);
 To complety disable logging from this source file, change next line to: #if 0
IMPORTANT:
==========
DBGINIT() statement MUST BE PLACED as a first statement within the task _INIT function!
Example:
void _INIT myTaskINIT(void)
{
	//TODO: Add code here
	//init logger object
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);
}
*/
#if 1
#include <../../bglib/bgPVLogger.c>
#define LOG_ENTRY_COUNT 20	//max number of log entries
#define LOG_ENTRY_LENGTH  32 //max length of individual log message. Longer msg are truncated
_LOCAL STRING log[LOG_ENTRY_COUNT][LOG_ENTRY_LENGTH]; //log output visible from Automation Studio!
static bgPVLogger_typ lgr;	//PV logger object
#else
#define DBG(loggerInstance, format, args...) ;
#define DBGINIT(loggerInstance, logoutput, logEntryCount, logEntrySize)  ;
#endif


/*
###############################################################################
UDP Server
###############################################################################
*/


static int jsonGetMsgId(UDINT pRxBuffer, UDINT lenRxData)
{
	char* pPatternBeg = ("\"id\":");
	char* pPatternEnd = ("}");
	char txtValueBuffer[32];
	int patternBegLen = strlen(pPatternBeg);
	
	char* pValueTxtBeg;
	char* pValueTxtEnd;
	char* pDst;
	char* pSrc;
	
	
	int msgid = -1;
	
	char* pText = pRxBuffer;
	if(lenRxData >= (patternBegLen + 8) ){
		pText = pRxBuffer + lenRxData - (patternBegLen + 8);	//id is at the end of the json message
	}

	
	
	pValueTxtBeg = strstr(pText, pPatternBeg);	
	if(!pValueTxtBeg)
		return msgid;
	
	pValueTxtEnd = strstr(pValueTxtBeg, pPatternEnd);	
	if(!pValueTxtEnd)
		return msgid;

	for(pSrc = (pValueTxtBeg + patternBegLen + 1), pDst=txtValueBuffer; pSrc < pValueTxtEnd; pSrc++, pDst++){
		*pDst = *pSrc;
	}
	*pDst=0;	
	
	sscanf2(txtValueBuffer, "%d", &msgid);
	return msgid;
}


static char* jsonGetResultBeg(UDINT pSrc, UDINT lenRxData)
{
	char* pPatternBeg = ("\"result\": [[");
	int patternBegLen = strlen(pPatternBeg);
	char* pValueTxtBeg;
	
	pValueTxtBeg = strstr(pSrc, pPatternBeg);	
	if(pValueTxtBeg){
		pValueTxtBeg += patternBegLen;
	}
	return (pValueTxtBeg);
}

static BOOL jsonGetIsResultEnd(char* pSrc, char* pPatternBeg, int id)
{
	char* pValueTxtBeg;
	char txtBuff[32];
	char numBuff[32];
	char patternBuff[128];
	
	//Example for the patternBuff: "\"id\": 30"
	strcpy(patternBuff, pPatternBeg); itoa(id, numBuff); strcat(patternBuff, numBuff);
	
	memcpy(txtBuff, pSrc, 20);
	txtBuff[20] = 0;
	
	pValueTxtBeg = strstr(txtBuff, patternBuff);	
	if(pValueTxtBeg){
		return 1;
	}
	return 0;
}

static BOOL jsonGetIsResultError(char* pSrc, char* pPatternBeg, int lenRxData)
{
	char* pValueTxtBeg;
	char txtBuff[256];
	int cplen = (lenRxData > 64) ? 64 : lenRxData;//inspect only first 64 bytes or less > see format of the error reply
	
	memcpy(txtBuff, pSrc, cplen);
	txtBuff[cplen] = 0;
	
	pValueTxtBeg = strstr(txtBuff, pPatternBeg);	
	if(pValueTxtBeg){
		return 1;
	}
	return 0;
}

static BOOL jsonGetIsResultEmpty(char* pRxBuffer)
{
	char* pPatternBeg = ("\"result\": [[]]");
	int patternBegLen = strlen(pPatternBeg);
	char* pValueTxtBeg;
	
	pValueTxtBeg = strstr(pRxBuffer, pPatternBeg);	
	if(pValueTxtBeg){
		return 1;
	}
	return 0;
}


static void jsonMsgHandleErrorMySQL(char* pRxBuffer)
{
	char *p1;
	char *p2;
	
	DBG(lgr, "MySQLErr");
	p1 = strstr(pRxBuffer, "[[[");
	if(!p1)return;
	p2 = strstr(pRxBuffer, "]]]");
	if(!p2)return;
	*p2 = 0;
	//for now, just log error msg
	DBG(lgr, "%s", (p1+3));
	
	//Generate an alarm
	//gYPAlarms.alarmActive[APP_ALARM_DBERROR]= gConfig.active.diag.disableDBAlarm ? 0 : 1; 
}

static void jsonMsgHandleErrorDbProxy(char* pRxBuffer)
{
	char *p1;
	char *p2;
	int errorCode;
		
	DBG(lgr, "DBProxyErr");
	p1 = strstr(pRxBuffer, "code\":");
	if(!p1)return;
	p1 += strlen("code\":");
	p2 = strstr(p1, ",");
	if(!p2)return;
	*p2 = 0;
	sscanf2(p1, "%d", &errorCode);

	
	p1 = strstr(p2+1, "message\":");
	if(!p1)return;
	p1 += strlen("message\":");
	p2 = strstr(p1, "}");
	if(!p2)return;
	*p2 = 0;
	
	//for now, just log error msg
	DBG(lgr, "DBProxyErr: code=%d, msg=%s", errorCode, p1);
	
	//Generate an alarm
	//gYPAlarms.alarmActive[APP_ALARM_DBPROXYERROR]= gConfig.active.diag.disableDBProxAlarm ? 0 : 1;
}

	//Clear DB access alarms
static void jsonMsgHandleNoErrors()
{
	//gYPAlarms.alarmActive[APP_ALARM_DBPROXYTIMEOUT]= 0;//timeout to dbproxy > db proxy is down, can't talk to it
	//gYPAlarms.alarmActive[APP_ALARM_DBPROXYERROR]= 0;//error from dbproxy > db may not be connected
	//gYPAlarms.alarmActive[APP_ALARM_DBERROR]= 0;//error from mysql
}


static char* jsonGetRecordStart(char* pSrc)
{
	char* pPatternBeg = "[";
	int patternBegLen = strlen(pPatternBeg);
	char* pValueTxtBeg;
	
	pValueTxtBeg = strstr(pSrc, pPatternBeg);	
	
	if(pValueTxtBeg){
		pValueTxtBeg += patternBegLen;
	}
	
	return pValueTxtBeg;	
}

static char* jsonGetRecordEnd(char* pSrc)
{
	char* pPatternBeg = "]";
	int patternBegLen = strlen(pPatternBeg);
	char* pValueTxtBeg;
	
	pValueTxtBeg = strstr(pSrc, pPatternBeg);	
	
	if(pValueTxtBeg){
		pValueTxtBeg += patternBegLen;
	}
	
	return pValueTxtBeg;	
}


static char* jsonGetNextInt(char* pSrc, int* pDstInt)
{
	char* pPatternBeg = pSrc;
	char* pPatternEnd = ",";
	char txtValueBuffer[32];
	char* pValueTxtEnd;
	char* pDst;
	
	pValueTxtEnd = strstr(pSrc, pPatternEnd);	
	if(!pValueTxtEnd)
		return 0L;

	//compensate for the last element within the record which does not have a comma at the end of record
	//rather, it is the end of the record character ']'
	if( *(pValueTxtEnd-1) == ']'){
		pValueTxtEnd -= 1;
	}
	
	for(pSrc, pDst=txtValueBuffer; pSrc < pValueTxtEnd; pSrc++, pDst++){
		*pDst = *pSrc;
	}
	*pDst=0;	
	*pDstInt = 0;
	
	sscanf2(txtValueBuffer, "%d", pDstInt);
	return (pValueTxtEnd+1);
}

static char* jsonGetNextFloat(char* pSrc, REAL* pDstFloat)
{
	char* pPatternBeg = pSrc;
	char* pPatternEnd = ",";
	char txtValueBuffer[32];
	char* pValueTxtEnd;
	char* pDst;
	float floatVal;
	
	pValueTxtEnd = strstr(pSrc, pPatternEnd);	
	if(!pValueTxtEnd)
		return 0L;

	//compensate for the last element within the record which does not have a comma at the end of record
	//rather, it is the end of the record character ']'
	if( *(pValueTxtEnd-1) == ']'){
		pValueTxtEnd -= 1;
	}
	
	for(pSrc, pDst=txtValueBuffer; pSrc < pValueTxtEnd; pSrc++, pDst++){
		*pDst = *pSrc;
	}
	*pDst=0;	
	
	sscanf2(txtValueBuffer, "%f", &floatVal);
	*pDstFloat = floatVal;
	return (pValueTxtEnd+1);
}

static char* jsonGetNextStr(char* pSrcOrig, char* pDst, int dstLen)
{
	char* pPatternBeg = "\"";
	char* pPatternEnd = "\"";
	char* pNullStr = "null";
	char* pValueTxtBeg;
	char* pValueTxtEnd;
	char* pSrc;
	char textBuff[128];
	
	//support null string return
	memcpy(textBuff, pSrcOrig, 8);
	textBuff[8]=0;
	pValueTxtBeg = strstr(textBuff, pPatternBeg);//search for string start mark		
	if(!pValueTxtBeg){
		//string start not found > check if 'null' string present
		pValueTxtBeg = strstr(textBuff, pNullStr);	
		if(pValueTxtBeg){
			pValueTxtBeg = strstr(pSrcOrig, pNullStr);	
			strcpy(pDst, "n.a.");
			return (pValueTxtBeg + strlen(pNullStr) + 2);
		}
		return 0L;
	}
	
	//string start marker found > extract string 
	pValueTxtBeg = strstr(pSrcOrig, pPatternBeg);		
	pValueTxtBeg += strlen(pPatternBeg);
	
	pValueTxtEnd = strstr(pValueTxtBeg, pPatternEnd);	
	if(!pValueTxtEnd)
		return 0L;
	
	for(pSrc=pValueTxtBeg; pSrc < pValueTxtEnd; pSrc++, pDst++){
		*pDst = *pSrc;
	}
	*pDst=0;		

	//if(*(pValueTxtEnd+2) == ']'){
	//	return(pValueTxtEnd+3);//end of the last string column is marked by: ""\],"
	//}
	return (pValueTxtEnd+2); //end of string column is marked by: ""\,"
}
//END JSON PROCESSING UTILITES

#if 0
static void tableViewPersonsInit(hmiTablePersons_typ* p, hmiTableItemDataPerson_typ* pItemsAll, UDINT itemCount, UDINT viewIndexMax)
{
	int ii;
	
	if(!p || !pItemsAll || !itemCount || !viewIndexMax)
		return;

	
	p->pItemsAll = (UDINT)pItemsAll;

	p->viewIndexMax = viewIndexMax;
	if(p->viewIndexMax > hmiTablePersonMAXVIEWINDEX){
		p->viewIndexMax = hmiTablePersonMAXVIEWINDEX;
	}
	
	//tableViewShowGrid(p);
	
	//reset selection/activate indexes and datapoints wired to hotspots
	p->selectedItem = -1;
	p->selectedViewItem = -1;
	p->selectViewItemDataPoint = -1;
	p->activateViewItemDataPoint = -1;
	
	//simulate some data
	p->itemCountUnfiltered = 0;
	p->itemCount = 0;
	for(ii=0; ii<itemCount;ii++){
		if(pItemsAll[ii].usedEntry){
			p->itemCountUnfiltered++;
			p->itemCount++;
		}
	}	
}

static void tableViewProductsInit(hmiTableProducts_typ* p, hmiTableItemDataProduct_typ* pItemsAll, UDINT itemCount, UDINT viewIndexMax)
{
}


static int jsonMsgPostFilterIsWorkerAssinged(int dbRecId, int shift)
{
}
/*
Processing following SQL query reply:
SELECT id,lastname,firstname,employeeid,active,shift FROM workers
*/
static void jsonMsgParseSQLQueryReplyWorkers(char* pRxBuffer, UDINT lenRxData, hmiTablePersons_typ* p, hmiTableItemDataPerson_typ* pItemsAll, UDINT itemCount, UDINT viewIndexMax, char* pDBTableName, int jsonSQLQueryid)
{
	char* p1 = pRxBuffer;
	char* p2;
	
	int myId;
	char myText[128];
	char myText1[128];
	char myText2[128];
	int active;
	int shift;	
	int ii;
	
	//make all entries unused
	for(ii=0;ii<itemCount;ii++){
		(pItemsAll + ii)->usedEntry = 0;
	}	
	
	if(jsonGetIsResultEmpty(pRxBuffer)){
		DBG(lgr, "received 0 DB records");				
		tableViewPersonsInit(p, pItemsAll, itemCount, viewIndexMax);//shorter version of the person dialog
		return;
	}
	
	p1 = jsonGetResultBeg(pRxBuffer, lenRxData);
	ii=0;
	while(1){
		if(!p1)break;
		p1 = jsonGetRecordStart(p1);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &myId);
		if(!p1)break;
		p1 = jsonGetNextStr(p1, myText, 128);
		if(!p1)break;
		p1 = jsonGetNextStr(p1, myText1, 128);	
		if(!p1)break;
		p1 = jsonGetNextStr(p1, myText2, 128);	
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &active);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &shift);
				
		if(p->assignedFilter){
			if(p->showBothShiftsForAssg){
				//both shifts are allowed for worker assigment
				//query will return active workers from both shifts
				//filter out assigned worker that are assigned to the shift curretnly beeing edited
				if( (jsonMsgPostFilterIsWorkerAssinged(myId, 1) && (gHmi.status.tabKeysRadioDataPoint==1))|| 
					(jsonMsgPostFilterIsWorkerAssinged(myId, 2) && (gHmi.status.tabKeysRadioDataPoint==2))){
					//DBG(lgr, "ASSGND_SKIP1: %d, %s, %s, %s, %d, %d", myId, myText, myText1, myText2, active, shift);	
					goto jsonMsgParserSkip1;							
				} 			
			}else{
				//only single shift is allowed for worker assigment
				if(jsonMsgPostFilterIsWorkerAssinged(myId, shift)){
					//DBG(lgr, "ASSGND_SKIP2: %d, %s, %s, %s, %d, %d", myId, myText, myText1, myText2, active, shift);	
					goto jsonMsgParserSkip1;							
				} 
			}			
		}
		
			
		(pItemsAll + ii)->usedEntry = 1;
		(pItemsAll + ii)->dbRecID = myId;
		(pItemsAll + ii)->active = active;
		(pItemsAll + ii)->index = ii;
		(pItemsAll + ii)->indexRaw = ii;
		(pItemsAll + ii)->shift = shift;
		(pItemsAll + ii)->employeeId = atoi(myText2);
		strcpy((pItemsAll + ii)->firstName, myText1);
		strcpy((pItemsAll + ii)->lastName, myText);
		strcpy((pItemsAll + ii)->employeeIdTxt, myText2);
		
		ii++;

jsonMsgParserSkip1:
			
		if(ii > itemCount){
			DBG(lgr, "rx: table=%s, rec#=%d MAX!", pDBTableName, ii);				
			break;
		}
		
		if((jsonSQLQueryid == CONST_JSONMSG_WORKERS) && jsonGetIsResultEnd(p1, "\"id\": ", CONST_JSONMSG_WORKERS)){
			DBG(lgr, "rx: table=%s, rec#=%d", pDBTableName, ii);				
			break;
		}else
		if((jsonSQLQueryid == CONST_JSONMSG_INSPECTORS) && jsonGetIsResultEnd(p1, "\"id\": ", CONST_JSONMSG_INSPECTORS)){
			DBG(lgr, "rx: table=%s, rec#=%d", pDBTableName, ii);				
			break;
		}
		
		
	}	
	
	tableViewPersonsInit(p, pItemsAll, itemCount, viewIndexMax);//shorter version of the person dialog
}

/*
Processing following SQL query reply:
*/
static void jsonMsgParseSQLQueryReplyProducts(char* pRxBuffer, UDINT lenRxData, hmiTableProducts_typ* p, hmiTableItemDataProduct_typ* pItemsAll, UDINT itemCount, UDINT viewIndexMax, char* pDBTableName, int jsonSQLQueryid)
{
}


static int jsonDashbmainGetEntryFromWorkerId(int workerId, int shift)
{
}

static void jsonUpdateTimeAccTotalTimes(char* productionDate, UDINT workerId, UDINT productId, UDINT shiftId, UDINT totalAccumTime)
{

}

static jsonDashbmainAllEmptyRecords()
{
}

static void jsonMsgParseSQLQueryReplyDashbmain(char* pRxBuffer, UDINT lenRxData, char* pDBTableName, int jsonSQLQueryid)
{

	
}

static void jsonMsgParseSQLQueryReplyDashbdefects(char* pRxBuffer, UDINT lenRxData, char* pDBTableName, int jsonSQLQueryid)
{
	char* p1 = pRxBuffer;
	char* p2;
	hmiDashboardMainItem_typ* pDashbItem;
	hmiStationAssigmentItem_typ* pStAssgm;
	hmiDashboardMainItem_typ totalsShift;
	char productiondateRx[128];
	int shiftRx;
	int productidRx;
	int workeridRx;
	int defects[YP_MAX_QCDEFECT_IDX + 1];	
	int ii;
	
	for(ii=0;ii<=YP_MAX_QCDEFECT_IDX;ii++){
		totalsShift.defects[ii]=0;
		defects[ii]=0;
	}
	
		
	if(jsonGetIsResultEmpty(pRxBuffer)){
		DBG(lgr, "Dashbmain: received 0 DB records");
		//jsonDashbdefectsAllEmptyRecords();		
		return;
	}
	
	
	p1 = jsonGetResultBeg(pRxBuffer, lenRxData);
	ii=0;
	while(1){
		if(!p1)break;
		p1 = jsonGetRecordStart(p1);
		if(!p1)break;
		p1 = jsonGetNextStr(p1, productiondateRx, 128);	
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &shiftRx);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &productidRx);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &workeridRx);
		if(!p1)break;
		
		p1 = jsonGetNextInt(p1, &defects[0]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[1]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[2]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[3]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[4]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[5]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[6]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[7]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[8]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[9]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[10]);
		if(!p1)break;
		p1 = jsonGetNextInt(p1, &defects[11]);
		if(!p1)break;

		//Dashboard defects summary callculation: shift totals
		for(ii=0;ii<=YP_MAX_QCDEFECT_IDX;ii++){
			 totalsShift.defects[ii]+=defects[ii];
		}
		
		ii = jsonDashbmainGetEntryFromWorkerId(workeridRx, shiftRx);
		if(ii < 0){
			goto jsonReplyDashbmainExit;
		}
	
		pDashbItem = &gDashboardData.stationItem[ii];
		for(ii=0;ii<=YP_MAX_QCDEFECT_IDX;ii++){
			pDashbItem->defects[ii] = defects[ii];
		}
		
jsonReplyDashbmainExit:		
			if((jsonSQLQueryid == jsonSQLQueryid) && jsonGetIsResultEnd(p1, "\"id\": ", jsonSQLQueryid)){
				DBG(lgr, "rx: table=%s", pDBTableName);				
				break;
			}		
	}
		
	//Dashboard defects summary callculation: shift totals
	for(ii=0;ii<=YP_MAX_QCDEFECT_IDX;ii++){
		gDashboardData.productionTotalsShift.defects[ii] = totalsShift.defects[ii];
	}
	
}
#endif

//UDP Server Receive function
static void udpServerCallback(UDINT pRxBuffer, UDINT lenRxData)
{
	int msgId;
	char* p1;
	
	//DBG(lgr, "lenRxData=%d", lenRxData);	
		
	msgId = jsonGetMsgId(pRxBuffer, lenRxData);
	//DBG(lgr, "msgId=%d", msgId);	
	
	if( jsonGetIsResultError(pRxBuffer, "MySQLError:", lenRxData) ){	
		jsonMsgHandleErrorMySQL(pRxBuffer);
		return;
	}	
	if( jsonGetIsResultError(pRxBuffer, "\"error\": {\"code\":", lenRxData) ){	
		jsonMsgHandleErrorDbProxy(pRxBuffer);
		return;
	}	
	
	//Msg wo. errors received > clear DB related YP alarms
	jsonMsgHandleNoErrors();
	
	if(msgId >= CONST_JSONMSG_MAXID){
		//TODO: process reply to "INSERT INTO measurement_data..." sql
		//DBG(lgr, "msgId=%d WTRPLY TODO", msgId);	
		return;
	}
	
	switch(msgId){
		case 10:
			DBG(lgr, "msgId=%d TODO", msgId);	
			break;
		case 20:
			DBG(lgr, "msgId=%d TODO", msgId);	
			break;
#if 0			
		case CONST_JSONMSG_WORKERS:
			jsonMsgParseSQLQueryReplyWorkers(pRxBuffer, lenRxData, &gHmi.tableViewPersons, &gWorkersData[0], hmiTablePersonsMAXINDEXWORKER+1, hmiTablePersonMAXVIEWINDEX, "workers", CONST_JSONMSG_WORKERS);
			break;
		case CONST_JSONMSG_INSPECTORS:
			jsonMsgParseSQLQueryReplyWorkers(pRxBuffer, lenRxData, &gHmi.tableViewPersons, &gInspectorsData[0], hmiTablePersonsMAXINDEXINSPECTOR+1, hmiTablePersonMAXVIEWINDEX, "inspectors", CONST_JSONMSG_INSPECTORS);
		break;
		case CONST_JSONMSG_PRODUCTS:
			jsonMsgParseSQLQueryReplyProducts(pRxBuffer, lenRxData, &gHmi.tableViewProducts, &gProductsData[0], hmiTableProductsMAXINDEXPRODUCT+1, hmiTableProductsMAXVIEWINDEX, "products", CONST_JSONMSG_PRODUCTS);
			break;
		case CONST_JSONMSG_DASHBMAIN:
			jsonMsgParseSQLQueryReplyDashbmain(pRxBuffer, lenRxData, "workers_totals", CONST_JSONMSG_DASHBMAIN);
			break;
		case CONST_JSONMSG_DASHBDEFECTS:
			jsonMsgParseSQLQueryReplyDashbdefects(pRxBuffer, lenRxData, "workers_totals", CONST_JSONMSG_DASHBDEFECTS);
			break;
		case CONST_JSONMSG_WRKTMUPD:
		case CONST_JSONMSG_STATIONDATA:
			//nothing to do: 
			break;
#endif			
		default:
			DBG(lgr, "msgId=%d UNHANDLED", msgId);	
			break;
	}
}

/*
###############################################################################
SQL Fifo
###############################################################################
*/
UDINT pushSQLQuery(UDINT queryId, unsigned char* queryText, int textlen)
{
	gSQLToFIFO.queryId = queryId;
	strcpy(gSQLToFIFO.text, queryText);//TODO: limit strcpy
	return bgFIFOPush(&fifoMgrSQL, (UDINT)&gSQLToFIFO, sizeof(appDataRecordSQLQuery_typ));
}

/*
INSERT INTO time_data(new_seconds_count, workerid, productid, productiondate, shift, time_stamp) VALUES(5343, 41, 6, '2013-12-1', 1, '2013-12-1 11:38:47') ON DUPLICATE KEY UPDATE new_seconds_count = 5343;
*/
static void testSQLHali()
{

	char gUdpClient_txbuf[512];
	char values_txt[256];
	
	//production_date, shift, product_id
	strcpy(&gUdpClient_txbuf[0], 
		"{\"paipvilite\": \"1.0\", \"method\": \"executesqlquery\", \"params\": {\"sqlquery\":\"INSERT INTO students(firstname) VALUES (");
	snprintf2(values_txt, 128, "'%s'", gTestString);
	strcat(&gUdpClient_txbuf[0], values_txt);
	
	strcat(&gUdpClient_txbuf[0], ")\"}, \"id\": ");
	strcat(&gUdpClient_txbuf[0], "10");
	strcat(&gUdpClient_txbuf[0], "}");
		
	
	pushSQLQuery(0, gUdpClient_txbuf, strlen(gUdpClient_txbuf));	
}


/*
###############################################################################
UDP Client/Server Stuff
###############################################################################
*/
/*
###############################################################################
UDP callback functions
###############################################################################
*/
//UDP Server Receive function
static void udpServerCallbackAfterReceive(UDINT pRxBuffer, UDINT lenRxData)
{
	udpServerCallback(pRxBuffer, lenRxData);
}


//UDP Client Send Callback function
static void udpClientCallback(UDINT pTxBuffer, UDINT lenTxBuffer, UDINT* pTxDataLen)
{
	(*pTxDataLen) = strlen(&gUdpClientMySQL.txbuf[0]);
}

//UDP Client Send Callback function
static void udpClientCallbackBeforeSend(UDINT pTxBuffer, UDINT lenTxBuffer, UDINT* pTxDataLen)
{
	udpClientCallback(pTxBuffer, lenTxBuffer, pTxDataLen);
}


static void	udpTimeOutHandler()
{
	//application logic - an example of handling a UDP receive timeout event 
	static BOOL oldRxTimeout;
	if( gUdpServerMySQL.serverIf.oRxIsTimeout && !oldRxTimeout){
		DBG(lgr, "udp server rx timeout");	
	}
	if( !gUdpServerMySQL.serverIf.oRxIsTimeout && oldRxTimeout){
		DBG(lgr, "udp server rx ok");	
	}	
	oldRxTimeout = gUdpServerMySQL.serverIf.oRxIsTimeout;
}


/*
###############################################################################
sql Transport FIFO 
- implements a simple priority 
- measurment data sql has higher priority then other sqls
###############################################################################
*/
static void sqlUDPTransferCyclic(bgFIFO_typ* pSQLQuery)
{
	int ii;
	appDataRecordSQLQuery_typ pullDataSQL;
	
	//sanity check
	if(!pSQLQuery)
		return;	
	
	//keep checking if new record shall be pulled from FIFO for transfer over UDP
	if(	(!bgFIFOGetCount(pSQLQuery) ) || //any records in FIFO?
		gUdpClientMySQL.client.send.enable //client is sending?
	){	
		return;		
	}
	
	if(bgFIFOGetCount(pSQLQuery)){
		//pull next entry from FIFO
		bgFIFOPull(pSQLQuery, (UDINT)&pullDataSQL, (UDINT)(sizeof(appDataRecordSQLQuery_typ)));
		
		strcpy(&gUdpClientMySQL.txbuf[0], pullDataSQL.text);
		gUdpClientMySQL.clientIf.iSend = 1;	
	}
}
/*
###############################################################################
Task execution entry points: Init & Cyclic Functions
###############################################################################
*/
void _INIT udpAppINIT(void)
{
	/* TODO: Add code here */
	//init logger object
	DBGINIT(lgr, log, LOG_ENTRY_COUNT, LOG_ENTRY_LENGTH);

	//Define application specific UDP client tx callback function 
	gUdpClientMySQL.clientIf.iTxCallback = (UDINT)udpClientCallbackBeforeSend;

	//Define application specific UDP server rx callback function 
	gUdpServerMySQL.serverIf.iRxCallback = (UDINT)udpServerCallbackAfterReceive;
	gUdpServerMySQL.serverIf.oRxDataLenMax = (UDINT)sizeof(ctrlInterfaceCmd_typ);
	
	//Adjust UDP client/server settings according to the application needs (i.e. use HMI if needed)
	//For other defaults see:
	//paiUDPService.var (i.e. defined UDP related const)
	//udpserv.c/udpservINIT() and udpclient.c/udpclientINIT()
	//gUdpSystemCfgMySQL.udpClientMode = 0;//echo mode
	//gUdpSystemCfgMySQL.udpClientUpdateRate = 10;//[Hz]
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.100.31");
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.100.171");
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.100.255");
	//strcpy(gUdpSystemCfg.udpRemoteHostIpAddress.text, "192.168.2.100");
	//gUdpClient.clientIf.iTaskCycleTimeXms = 10;//udp client runs under 10ms cycle time
	gUdpServerMySQL.serverIf.iRxTimeoutXms = 4000;//udp server rx timeout time
	
	
	bgFIFOInit(&fifoMgrSQL, sizeof(appDataRecordSQLQuery_typ), CONST_FIFO_MAX_SQL_ENTRIES, (UDINT)&fifoMemSQL[0]);
	
}

void _CYCLIC udpAppCYCLIC(void)
{
	/* TODO: Add code here */
		
	//application logic - on send complete, copy status into last statusSent
	if(gUdpClientMySQL.clientIf.oSendDone){
		gUdpClientMySQL.clientIf.oSendDone = 0;		
		//save last transmitted data
	}


	//application logic - en example of handling a UDP rx event	
	if(gUdpServerMySQL.serverIf.oRxReady){
		gUdpServerMySQL.serverIf.oRxReady = 0;
		
		//use server rx event to trigger udp send to host
		//gUdpClientMySQL.clientIf.iSendEcho = 1;
		//strcpy(gUdpClientMySQL.clientIf.iSendEchoRmtIpAddr, gUdpServerMySQL.serverIf.oRxRmtIP);
		
	}
	
	if(testSQLHaliGo){
		testSQLHaliGo=0;
		testSQLHali();
	}
	
	//Process SQL Msg Fifo
	sqlUDPTransferCyclic(&fifoMgrSQL);
	
	//handle a UDP rx timeout
	udpTimeOutHandler();
}
//EOF

