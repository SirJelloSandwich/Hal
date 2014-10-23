/********************************************************************
 * COPYRIGHT -- HP
 ********************************************************************
 * Program: j1939test
 * File: j1939test.c
 * Author: goran
 * Created: February 20, 2013
 ********************************************************************
 * Implementation of program j1939test
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif


_GLOBAL UINT sliderTemp_F;


static UINT paiByteSwapUINT(UINT val)
{
	UINT temp;
	UINT byte0 = val & 0xff;
	UINT byte1 = (val >> 8) & 0xff;	
	temp = ((byte0 << 8) & 0xff00) + byte1;
	return temp;
}

static INT paiByteSwapINT(INT val)
{
	INT temp;
	INT byte0 = val & 0xff;
	INT byte1 = (val >> 8) & 0xff;	
	temp = ((byte0 << 8) & 0xff00) + byte1;
	return temp;
}

static DINT paiByteSwapDINT(DINT val)
{
	DINT temp;
	
	DINT byte0 = val & 0xff;
	DINT byte1 = (val >> 8) & 0xff;	
	DINT byte2 = (val >> 16) & 0xff;	
	DINT byte3 = (val >> 24) & 0xff;	

	temp = ((byte0 << 24) & 0xff000000) + ((byte1 << 16) & 0x00ff0000) + ((byte2 << 8) & 0xff00ff00) + ((byte3 << 0) & 0x000000ff);
	return temp;
}

static USINT paiJ1939RealToSINT(REAL value, REAL k, REAL n)
{
	REAL scaledVal = (value - n) / k;
	SINT intVal = (SINT)scaledVal;
	return intVal;
}

static UINT paiJ1939RealToINT(REAL value, REAL k, REAL n)
{
	REAL scaledVal = (value - n) / k;
	UINT intVal = (INT)scaledVal;
	return intVal;
}

static void paiJ1939SimulateReal(REAL* value, REAL min, REAL max, REAL inc)
{
	if( *value <= min){
		*value = min;
	}
	
	if( *value > max){
		*value = min;
	}
	
	*value += inc;
}


//Init CAN bus IF for use in J1939 (MUST BE CALLED DURING TASK _INIT)
static void paiJ1939Init(paiJ1939Driver_typ* p, char* canOpenString)
{
	if(!p)
		return;
	
	p->prvCANlib.canOpen.enable = 1;
	p->prvCANlib.canOpen.cob_anz = paiCAN_MaxIDCount;
	p->prvCANlib.canOpen.device = canOpenString;
	p->prvCANlib.canOpen.info = 0;
	CANopen(&p->prvCANlib.canOpen);
	
	if (p->prvCANlib.canOpen.status != 0){
		return;
	}

	p->prvCANlib.canWrite.us_ident = p->prvCANlib.canOpen.us_ident;
	p->prvCANlib.canRead.us_ident = p->prvCANlib.canOpen.us_ident;
}

//Define a J1939 item to rx or tx
static void paiJ1939PGNDefine(paiJ1939PGN_typ* pPGN, UDINT PGN, UDINT rxOrTx, UDINT txIntervalXms)
{
/*
J1938 CAN ID Mapping

BIT INDEX / USAGE:
28	26 	25 	24	16	8 	0
P 	EDP DP 	PF	PS	SA 

P:
Message priority. Must come first.
EDP:
Extended data page. J1939 devices must set to 0.
DP:
Data page. Used to create a second page of PGNs.
PF:
PDU format:
< 240, PS is destination address. (PDU1 format)
>= 240, PS is group extension. (PDU2 format)
PS:
PDU specific. Either destination address or group extension.
SA:
Source address of controller application (CA).


J1939 PGN Mapping
•If PF < 240,
then PGN = (DP << 9)+(PF<<8),
else PGN = (DP << 9)+(PF<<8)+PS
•Max number of PGNs: (240 + (16 x 256)) x 2) = 8,672
*/

	pPGN->PGN = PGN;
	pPGN->txIntervalXms = txIntervalXms;
	pPGN->tx = rxOrTx;
	pPGN->address = 0;
	
	//TODO: check the J1939 specs and redo the CAN identifier calculation
	//For now, following should work
	pPGN->canIdentifier = 0x18000000 + (PGN << 8);	
	
	//If tx, start the tx timer
	if(rxOrTx){
		pPGN->prv.txTimer.IN=1;
		pPGN->prv.txTimer.PT=txIntervalXms;
		TON(&pPGN->prv.txTimer);
	}
}

//Cyclic J1939 related CAN tx
static BOOL paiJ1939CyclicTx(paiJ1939Driver_typ* p, paiJ1939PGN_typ* pPGN, UDINT pData, UDINT dataLen)
{
	//execute the timer
	TON(&pPGN->prv.txTimer);
	
	//disable CANwrite lib call
	p->prvCANlib.canWrite.enable = 0;
	
	//check if timer timeout
	if(pPGN->prv.txTimer.Q){	
		//restart timer	
		pPGN->prv.txTimer.IN=0;
		TON(&pPGN->prv.txTimer);
		pPGN->prv.txTimer.IN=1;
		pPGN->prv.txTimer.PT=pPGN->txIntervalXms;
		TON(&pPGN->prv.txTimer);
		
		//check data size and copy data to tx buffer
		if(dataLen > paiCAN_msgByteSize)
			dataLen = paiCAN_msgByteSize;
			
		memcpy(p->prvCANRxTxData.canTxData, pData, dataLen);
			
		//execute CANwrite library call
		p->prvCANlib.canWrite.enable = 1;
		p->prvCANlib.canWrite.data_adr = p->prvCANRxTxData.canTxData;
		p->prvCANlib.canWrite.data_lng = dataLen;
		p->prvCANlib.canWrite.can_id = pPGN->canIdentifier;			
	}
	
	CANwrite(&p->prvCANlib.canWrite);  /* execute  CANwrite lib call*/
	
	pPGN->prv.txStatus = p->prvCANlib.canWrite.status;
	
	if(p->prvCANlib.canWrite.status){
		pPGN->prv.txCountError++;
	}else{
		if(p->prvCANlib.canWrite.enable){
			pPGN->prv.txCount++;			
			pPGN->prv.txEvent = 1;		
		}
	}
}

//Cyclic J1939 related CAN rx
static BOOL paiJ1939CyclicRx(paiJ1939Driver_typ* p, paiJ1939PGN_typ* pPGN, UDINT pData, UDINT dataLen)
{
	p->prvCANlib.canRead.enable = 1;
	p->prvCANlib.canRead.data_adr = p->prvCANRxTxData.canRxData;
	p->prvCANlib.canRead.can_id = pPGN->canIdentifier;			
	CANread(&p->prvCANlib.canRead);  /* execute  CANread lib call*/

	pPGN->prv.rxStatus = p->prvCANlib.canRead.status;
	
	if (p->prvCANlib.canRead.status == 8877)          
		return;

	if(p->prvCANlib.canRead.status){
		pPGN->prv.rxCountError++;
	}else{
		pPGN->prv.rxCount++;	
		pPGN->prv.rxEvent = 1;		
		memcpy(pData, p->prvCANRxTxData.canRxData, dataLen);
	}
}

//Cyclic J1939 related function
static BOOL paiJ1939Cyclic(paiJ1939Driver_typ* p, paiJ1939PGN_typ* pPGN, UDINT pData, UDINT dataLen)
{
	//sanity check
	if(!p || !pPGN || !pData)
		return;
	
	//can open check 	
	if(!p->prvCANlib.canOpen.us_ident)	
		return;

	//PGN tx active?
	if(pPGN->tx){
		paiJ1939CyclicTx(p, pPGN, pData, dataLen);
	}else{
		paiJ1939CyclicRx(p, pPGN, pData, dataLen);	
	}		
}



static void j1939Init()
{

	//IF1 on X20CS2770 (in this test used to transmit J1939 PGNs)
	//paiJ1939Init(&paiJ1939Drv1, "IF6.ST1.IF1 /BD=250000 /EX=1");
	paiJ1939Init(&paiJ1939Drv1, "SL1.IF1 /BD=250000 /EX=1");   //Run for 29bit CAN ident
//	paiJ1939Init(&paiJ1939Drv1, "SL1.IF1 /BD=250000 /EX=0");   //Run for 11bit CAN ident

	//IF2 on X20CS2770 (in this test used to receive J1939 PGNs)
	//paiJ1939Init(&paiJ1939Drv2, "IF6.ST1.IF2 /BD=250000 /EX=1");
	
	//PGNs to tx on IF1
	//paiJ1939PGNDefine(&J1939EngineTempTx.pgn, 0xFEEE, 1, 1000);
	paiJ1939PGNDefine(&J1939EngineTempTx.pgn, paiJ1939PGN_EngineTemp, 1, 800);
	
	paiJ1939PGNDefine(&J1939EEC1Tx.pgn, paiJ1939PGN_EEC1, 1, 500);
		
	paiJ1939PGNDefine(&J1939FluidPressureTx.pgn, paiJ1939PGN_FluidPressure, 1, 1000);

	paiJ1939PGNDefine(&J1939ElectricPowerTx.pgn, paiJ1939PGN_ElectricalPower, 1, 1200);
	

//	else
//	{
	paiJ1939PGNDefine(&J1939WirelineDepthRx.pgn, paiJ1939PGN_WirelineDepth, 0, 1000); 
	J1939WirelineDepthRx.pgn.canIdentifier = paiJ1939PGN_WirelineDepth;
//	}
		
		
	
	//PGNs to rx
	paiJ1939PGNDefine(&J1939EngineTempRx.pgn, paiJ1939PGN_EngineTemp, 0, 1000);


	paiJ1939PGNDefine(&J1939Rex181.pgn, paiJ1939PGN_RexrothHMI181, 0, 1000);
	J1939Rex181.pgn.canIdentifier = paiJ1939PGN_RexrothHMI181;	
	
	paiJ1939PGNDefine(&J1939Rex281.pgn, paiJ1939PGN_RexrothHMI281, 0, 1000);
	J1939Rex281.pgn.canIdentifier = paiJ1939PGN_RexrothHMI281;
	
	paiJ1939PGNDefine(&J1939Rex381.pgn, paiJ1939PGN_RexrothHMI381, 0, 1000);
	J1939Rex381.pgn.canIdentifier = paiJ1939PGN_RexrothHMI381;
	
	paiJ1939PGNDefine(&J1939Rex481.pgn, paiJ1939PGN_RexrothHMI481, 0, 1000);
	J1939Rex481.pgn.canIdentifier = paiJ1939PGN_RexrothHMI481;
	
	paiJ1939PGNDefine(&J1939Rex581.pgn, paiJ1939PGN_RexrothHMI581, 0, 1000);
	J1939Rex581.pgn.canIdentifier = paiJ1939PGN_RexrothHMI581;
	
	//Deafults 
	J1939EngineTempTx.engineOilTemp = 200.0; //degC
	J1939ElectricPowerTx.batteryVoltage_V = 13.5; //VDC
	J1939FluidPressureTx.engineOilPressure_psi = 100;  //kPa
	J1939EEC1Tx.rpm = 1500; //RPM
	J1939EngineTempTx.data.engineTemp = 140; //deg C  140 = 212 F
	
}

static void j1939Cyclic(void)
{
	REAL realSliderTemp_F = sliderTemp_F * 1.0;
	REAL realSliderTemp_C = (realSliderTemp_F - 32.0) * 5.0 / 9.0;
	USINT USINTsliderTemp_C = (USINT)(realSliderTemp_C + 40.0) ;

	//Slider (VNC) value coresponds to the temo
//	J1939EngineTempTx.data.engineTemp = USINTsliderTemp_C;
	
	//Call tx on IF1 cyclicly
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939EngineTempTx.pgn, &J1939EngineTempTx.data, sizeof(J1939EngineTempTx.data));
	//When tx event > incremenet temp (i.e. simulate change)
	if(J1939EngineTempTx.pgn.prv.txEvent){
		J1939EngineTempTx.pgn.prv.txEvent = 0;
		sliderTemp_F += 1;
		if(sliderTemp_F >= 410)
			sliderTemp_F = 32;	
			
	
		J1939EngineTempTx.data.engineOilTemp = paiJ1939RealToINT( J1939EngineTempTx.engineOilTemp , 0.03125, -273.0);
	}

		
	//RPM
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939EEC1Tx.pgn, &J1939EEC1Tx.data, sizeof(J1939EEC1Tx.data));
	//When tx event > incremenet RPM (i.e. simulate change)
	if(J1939EEC1Tx.pgn.prv.txEvent){
		J1939EEC1Tx.pgn.prv.txEvent = 0;

//		paiJ1939SimulateReal(&J1939EEC1Tx.rpm, 1000.0, 1500.0, 10.0);
		
		J1939EEC1Tx.data.engineSpeedRPM = paiByteSwapUINT ( paiJ1939RealToINT(J1939EEC1Tx.rpm, 0.125, 0.0) );		
	}
	 
	//Pressure
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939FluidPressureTx.pgn, &J1939FluidPressureTx.data, sizeof(J1939FluidPressureTx.data));
	if(J1939FluidPressureTx.pgn.prv.txEvent){
		J1939FluidPressureTx.pgn.prv.txEvent = 0;

//		paiJ1939SimulateReal(&J1939FluidPressureTx.engineOilPressure_psi, 100.0, 200.0, 1.0);
		
		J1939FluidPressureTx.data.engineOilPressure = paiJ1939RealToSINT(J1939FluidPressureTx.engineOilPressure_psi, 4.0, 0.0);
	}

	//Battery voltage
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939ElectricPowerTx.pgn, &J1939ElectricPowerTx.data, sizeof(J1939ElectricPowerTx.data));
	if(J1939ElectricPowerTx.pgn.prv.txEvent){
		J1939ElectricPowerTx.pgn.prv.txEvent = 0;

//		paiJ1939SimulateReal(&J1939ElectricPowerTx.batteryVoltage_V , 11.0, 13.0, 0.1);
		
		J1939ElectricPowerTx.data.electricalVoltage = paiJ1939RealToINT(J1939ElectricPowerTx.batteryVoltage_V , 0.05, 0.0);
	}
		
	
	//Write data to the HMI panel if the Benchmark 46 is active RAB 4/3/14
	
	if (IOSstatus46.bIOSenabled46)
	{
		paiJ1939PGNDefine(&J1939WirelineDepthTx.pgn, paiJ1939PGN_WirelineDepth, 1, 10);
		J1939WirelineDepthTx.pgn.canIdentifier = paiJ1939PGN_WirelineDepth;
		paiJ1939Cyclic(&paiJ1939Drv1, &J1939WirelineDepthTx.pgn, &J1939WirelineDepthTx.data, sizeof(J1939WirelineDepthTx.data)); 
	//	firstpassCAN = 1;
	}

	if(J1939WirelineDepthTx.pgn.prv.txEvent & IOSstatus46.bIOSenabled46)
	{
		J1939WirelineDepthTx.pgn.prv.txEvent = 0;
			
		J1939WirelineDepthTx.data.depth = (UDINT)(paiByteSwapDINT(gUdpCmdMsg.fCableDepth * 10));
		J1939WirelineDepthTx.data.lineSpeed = (UINT)(paiByteSwapINT(gUdpCmdMsg.fLineSpeed * 10));
		J1939WirelineDepthTx.data.tension = (UINT)(paiByteSwapINT(gUdpCmdMsg.fLineTension));
	}
		
	//Keep rx on IF1
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939WirelineDepthRx.pgn, &J1939WirelineDepthRx.data, sizeof(J1939WirelineDepthRx.data)); 
	if(J1939WirelineDepthRx.pgn.prv.rxEvent){
		J1939WirelineDepthRx.pgn.prv.rxEvent = 0;
		
		J1939WirelineDepthRx.depth = ((REAL)paiByteSwapDINT(J1939WirelineDepthRx.data.depth))/10.0;
		
		J1939WirelineDepthRx.tension = (REAL)paiByteSwapINT(J1939WirelineDepthRx.data.tension);
		J1939WirelineDepthRx.lineSpeed = ((REAL)paiByteSwapINT(J1939WirelineDepthRx.data.lineSpeed)) / 10;
		
	}
	


	paiJ1939Cyclic(&paiJ1939Drv1, &J1939Rex181.pgn, &J1939Rex181.data, sizeof(J1939Rex181.data)); 
	if(J1939Rex181.pgn.prv.rxEvent){
		J1939Rex181.pgn.prv.rxEvent = 0;
		
		J1939Rex181.bottomHoleDepth = (REAL)(J1939Rex181.data.bottomHoleDepth);
		
		J1939Rex181.endOfCasingDepth = (REAL)(J1939Rex181.data.endOfCasingDepth);
		
	}
	
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939Rex281.pgn, &J1939Rex281.data, sizeof(J1939Rex281.data)); 
	if(J1939Rex281.pgn.prv.rxEvent)
	{
		J1939Rex281.pgn.prv.rxEvent = 0;
		
		J1939Rex281.slowSpeed = (REAL)(J1939Rex281.data.slowSpeed);  //set scaling once data is received...RAB
		
		J1939Rex281.lastChanceDepth = (REAL)(J1939Rex281.data.lastChanceDepth);  //set scaling once data is received...RAB
	}
	
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939Rex381.pgn, &J1939Rex381.data, sizeof(J1939Rex381.data)); 
	if(J1939Rex381.pgn.prv.rxEvent)
	{
		J1939Rex381.pgn.prv.rxEvent = 0;
		
		J1939Rex381.maxOutofHoleDepth = (REAL)(J1939Rex381.data.maxOutofHolePosition);  //set scaling once data is received...RAB
	}
	
	
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939Rex481.pgn, &J1939Rex481.data, sizeof(J1939Rex481.data)); 
	if(J1939Rex481.pgn.prv.rxEvent)
	{
		J1939Rex481.pgn.prv.rxEvent = 0;
		
		J1939Rex481.cruiseCtrlSpd = (REAL)(J1939Rex481.data.cruiseCtrlSpd);  //set scaling once data is received...RAB

		J1939Rex481.fishingSpeedCtrl = (REAL)(J1939Rex481.data.fishingSpeedCtrl);  //set scaling once data is received...RAB
		
	}
	
	
	paiJ1939Cyclic(&paiJ1939Drv1, &J1939Rex581.pgn, &J1939Rex581.data, sizeof(J1939Rex581.data)); 
	if(J1939Rex581.pgn.prv.rxEvent)
	{
		J1939Rex581.pgn.prv.rxEvent = 0;
		
		J1939Rex581.upperShevePosition = (REAL)(J1939Rex581.data.upperShevePosition);  //set scaling once data is received...RAB
			
		J1939Rex581.kellyBushingPostion = (REAL)(J1939Rex581.data.kellyBushingPosition);  //set scaling once data is received...RAB
	}
	
	
	
	J1939EngineTempTx.engineOilTemp = gUdpCmdMsg.fEngOilTemp; //degC
	J1939ElectricPowerTx.batteryVoltage_V = gUdpCmdMsg.fEngECM; //VDC
	J1939FluidPressureTx.engineOilPressure_psi = gUdpCmdMsg.fEngOilPress;  //kPa
	J1939EEC1Tx.rpm = gUdpCmdMsg.fEngSpd; //RPM
	J1939EngineTempTx.data.engineTemp = gUdpCmdMsg.fEngCoolTemp + 40.0; //deg C  140 = 212 F
	
	testCANrtr.enable = testEnable;
	testCANrtr.us_ident = paiJ1939Drv1.prvCANlib.canOpen.us_ident;
	testCANrtr.request = testRequest;
	testCANrtr.data_adr = (UDINT)&testBuf;
	testCANrtr.can_id = testId;
	CANrtr(&testCANrtr);
	
	if(testRequest && testCANrtr.status)
		testRequest = 0;
}
/*
###############################################################################
Main entry: Init & Cyclic Functions
###############################################################################
*/
void _INIT j1939testINIT(void)
{
	/* TODO: Add code here */
	 j1939Init();
}

void _CYCLIC j1939testCYCLIC(void)
{
	 j1939Cyclic();
}
//EOF