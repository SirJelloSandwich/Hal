(********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Package: paiUDPService
 * File: paiUDPService.typ
 * Author: goran
 * Created: October 12, 2011
 ********************************************************************
 * Data types of package paiUDPService
 ********************************************************************)

TYPE
	udpConst_typ : 
		(
		UDP_IP_ADDR_LEN := 32,
		UDP_SERVER_RXBUF_SIZE := 1400,
		UDP_CLIENT_TXBUF_SIZE := 1400
		);
	udpIPAddress_typ : 	STRUCT 
		digit3 : USINT;
		digit2 : USINT;
		digit1 : USINT;
		digit0 : USINT;
		text : STRING[20];
	END_STRUCT;
	udpSystemConfig_typ : 	STRUCT 
		magicNumber : UDINT; (*Magic number to detect permanent memory changes.*)
		udpClientMode : USINT; (*Outgoing (client) UDP mode of operation. 0-Echo (send status upon receiving command, needs only port number, used IP from received command), 1-Continues (needs host IP address and Port number)*)
		udpClientUpdateRate : USINT; (*Update rate (Hz) for the the UDP client when in mode 1;*)
		udpLocalServerPortNumber : UINT; (*UDP port number of the local UDP server.*)
		udpRemoteHostPortNumber : UINT; (*UDP port number of the remote host*)
		udpRemoteHostIpAddress : udpIPAddress_typ; (*IP address of the remote UDP host*)
		updInterfaceType : UDINT; (*0-ISM native type; 1-Custom interface, type 1*)
	END_STRUCT;
	udpStatistics_typ : 	STRUCT  (*UDP server statistics*)
		datagramCountTotal : UDINT; (*total number of received datagrams*)
		datagramPerSec : UDINT; (*number of received datagrams in the past second*)
		datagramPerSecMax : UDINT; (*max number of received datagrams in a second*)
		bytesTotal : UDINT; (*totral number of received bytes*)
		bytesPerSec : UDINT;
		bytesPerSecMax : UDINT;
		errorCountTotal : UDINT; (*total number of errors*)
	END_STRUCT;
	udpClientInterface_typ : 	STRUCT 
		iConnect : BOOL;
		iRmtIPAddr : STRING[UDP_IP_ADDR_LEN];
		iRmtUdpPort : UINT;
		iMode : USINT; (*0-echo mode (client's tx triggered by server's rx); 1-continues mode (rate timer 1 .. 100Hz used to trigger client's send) *)
		iSend : BOOL;
		iSendEcho : BOOL;
		iSendEchoRmtIpAddr : STRING[UDP_IP_ADDR_LEN];
		iUpdateRate : USINT;
		iTaskCycleTimeXms : USINT;
		iTxDataAdr : UDINT;
		iTxDataLen : UDINT;
		iTxCallback : UDINT;
		oError : BOOL;
		oRmtIpAddr : STRING[UDP_IP_ADDR_LEN];
		oRmtUdpPort : UINT;
		oConnected : BOOL;
		oSendDone : BOOL;
	END_STRUCT;
	udpServerInterface_typ : 	STRUCT 
		iEnable : BOOL;
		iUdpServerPort : UINT;
		iRxCallback : UDINT;
		iRxTimeoutXms : UDINT;
		oRxIsTimeout : BOOL;
		oError : BOOL;
		oRxReady : BOOL;
		oRxRmtIP : STRING[UDP_IP_ADDR_LEN];
		oRxRmtPort : UINT;
		oRxDataLen : UDINT;
		oRxDataLenMax : UDINT;
		oRxDataAdr : UDINT;
	END_STRUCT;
	udpFBKs_typ : 	STRUCT 
		open : UdpOpen;
		close : UdpClose;
		connect : UdpConnect;
		disconnect : UdpDisconnect;
		recv : UdpRecv;
		send : UdpSend;
		ioctl : UdpIoctl;
	END_STRUCT;
	udpClient_typ : 	STRUCT 
		clientIf : udpClientInterface_typ;
		client : udpFBKs_typ;
		txStats : udpStatistics_typ;
		sm : bgSSM_typ;
		txbuf : ARRAY[0..UDP_CLIENT_TXBUF_SIZE]OF USINT;
	END_STRUCT;
	udpServer_typ : 	STRUCT 
		serverIf : udpServerInterface_typ;
		server : udpFBKs_typ;
		rxStats : udpStatistics_typ;
		sm : bgSSM_typ;
		rmtIPAddr : STRING[UDP_IP_ADDR_LEN];
		rxlen : UDINT;
		rxbuf : ARRAY[0..UDP_SERVER_RXBUF_SIZE]OF USINT;
	END_STRUCT;
END_TYPE
