/* Automation Studio generated header file */
/* Do not edit ! */

#ifndef _CAN_LIB_
#define _CAN_LIB_
#ifdef __cplusplus
extern "C" 
{
#endif

#include <bur/plctypes.h>

#include <runtime.h>

#ifndef _IEC_CONST
#define _IEC_CONST _WEAK const
#endif

/* Constants */
#ifdef _REPLACE_CONST
 #define canERR_NO_DATA 8877U
 #define canERR_NO_OBJ_FREE 8876U
 #define canERR_DATEN_INKONSISTENT 8875U
 #define canERR_MAX_RETRY_CNT 8874U
 #define canERR_MAXRETRY_DATENINKONSIST 8873U
 #define canERR_MSG_LOST 8872U
 #define canERR_STATUS 8871U
 #define canERR_NORMAL_NOT_POSIBLE 8870U
 #define canERR_NO_STATE_BON 8869U
 #define canERR_NO_STATE_INIT 8868U
 #define canERR_NO_STATE_BOFF 8867U
 #define canERR_NO_WR_MODE 8866U
 #define canERR_NO_RD_MODE 8865U
 #define canERR_WRONG_CAN_ID 8864U
 #define canERR_RXIE_ALLOWED 8863U
 #define canERR_RXIE_NOT_ALLOWED 8862U
 #define canERR_TXIE_ALLOWED 8861U
 #define canERR_TXIE_NOT_ALLOWED 8860U
 #define canERR_WRONG_MSG_LEN 8859U
 #define canERR_WRONG_CAN_MODE 8858U
 #define canERR_READ_ONLY_OBJ 8857U
 #define canERR_OBJ_ACTIVE 8856U
 #define canERR_CPU_UPDATE 8855U
 #define canERR_HW_RESET_ACTIVE 8854U
 #define canERR_NO_BUFFER_CFG 8853U
 #define canERR_NO_BUSOFF_STATE 8852U
 #define canERR_NO_INIT_STATE 8851U
 #define canERR_WRONG_OBJNR 8850U
 #define canERR_CAN_CRC 8846U
 #define canERR_CAN_BIT0 8845U
 #define canERR_CAN_BIT1 8844U
 #define canERR_CAN_ACK 8843U
 #define canERR_CAN_FORM 8842U
 #define canERR_CAN_STUFF 8841U
 #define canERR_FBCAN_RXBUF 8840U
 #define canERR_FBCAN_HANDLE 8839U
 #define canERR_FBCAN_NODENR 8838U
 #define canERR_FBCAN_COB 8837U
 #define canERR_CAN_BITMODE 8834U
 #define canERR_CAN_WROBJ 8833U
 #define canERR_CMS_MULTIPLEX 8832U
 #define canERR_CAN_EXTENDED 8831U
 #define canERR_CAN_FBHANDLE 8830U
 #define canERR_CAN_ABORTRECEIV 8829U
 #define canERR_CAN_ABORTED 8828U
 #define canERR_CAN_IRQHANDLE 8827U
 #define canERR_CAN_NILPTR 8826U
 #define canERR_CAN_NO_NODE 8825U
 #define canERR_CAN_WRMODE 8824U
 #define canERR_CAN_WRSTRUCT 8823U
 #define canERR_WRCMSOBJ 8822U
 #define canERR_CAN_WRUSRTYP 8821U
 #define canERR_CAN_NOFUNC 8820U
 #define canERR_CAN_HTLQUIT 8819U
 #define canERR_CAN_HTLXINF 8818U
 #define canERR_CAN_NOCONTR 8817U
 #define canERR_CAN_COBANZ 8816U
 #define canERR_CAN_BDRATE 8815U
 #define canERR_CAN_WRLNG 8814U
 #define canERR_CAN_NOTINST 8813U
 #define canERR_CAN_NOHIGH 8812U
 #define canERR_CAN_BUSOFF 8811U
 #define canERR_CAN_QFULL 8810U
 #define canERR_CAN_BUSY 8809U
 #define canERR_CAN_WRHDLE 8808U
 #define canERR_CAN_WRDEF 8807U
 #define canERR_CAN_COBDEF 8806U
 #define canERR_CAN_WRUSERID 8805U
 #define canERR_CAN_WRCANID 8804U
 #define canERR_CAN_COBUSED 8803U
 #define canERR_CAN_WRIDENT 8802U
 #define canERR_CAN_NOMEM 8801U
 #define canERR_CAN_NOENTRY 8800U
#else
 _IEC_CONST unsigned short canERR_NO_DATA = 8877U;
 _IEC_CONST unsigned short canERR_NO_OBJ_FREE = 8876U;
 _IEC_CONST unsigned short canERR_DATEN_INKONSISTENT = 8875U;
 _IEC_CONST unsigned short canERR_MAX_RETRY_CNT = 8874U;
 _IEC_CONST unsigned short canERR_MAXRETRY_DATENINKONSIST = 8873U;
 _IEC_CONST unsigned short canERR_MSG_LOST = 8872U;
 _IEC_CONST unsigned short canERR_STATUS = 8871U;
 _IEC_CONST unsigned short canERR_NORMAL_NOT_POSIBLE = 8870U;
 _IEC_CONST unsigned short canERR_NO_STATE_BON = 8869U;
 _IEC_CONST unsigned short canERR_NO_STATE_INIT = 8868U;
 _IEC_CONST unsigned short canERR_NO_STATE_BOFF = 8867U;
 _IEC_CONST unsigned short canERR_NO_WR_MODE = 8866U;
 _IEC_CONST unsigned short canERR_NO_RD_MODE = 8865U;
 _IEC_CONST unsigned short canERR_WRONG_CAN_ID = 8864U;
 _IEC_CONST unsigned short canERR_RXIE_ALLOWED = 8863U;
 _IEC_CONST unsigned short canERR_RXIE_NOT_ALLOWED = 8862U;
 _IEC_CONST unsigned short canERR_TXIE_ALLOWED = 8861U;
 _IEC_CONST unsigned short canERR_TXIE_NOT_ALLOWED = 8860U;
 _IEC_CONST unsigned short canERR_WRONG_MSG_LEN = 8859U;
 _IEC_CONST unsigned short canERR_WRONG_CAN_MODE = 8858U;
 _IEC_CONST unsigned short canERR_READ_ONLY_OBJ = 8857U;
 _IEC_CONST unsigned short canERR_OBJ_ACTIVE = 8856U;
 _IEC_CONST unsigned short canERR_CPU_UPDATE = 8855U;
 _IEC_CONST unsigned short canERR_HW_RESET_ACTIVE = 8854U;
 _IEC_CONST unsigned short canERR_NO_BUFFER_CFG = 8853U;
 _IEC_CONST unsigned short canERR_NO_BUSOFF_STATE = 8852U;
 _IEC_CONST unsigned short canERR_NO_INIT_STATE = 8851U;
 _IEC_CONST unsigned short canERR_WRONG_OBJNR = 8850U;
 _IEC_CONST unsigned short canERR_CAN_CRC = 8846U;
 _IEC_CONST unsigned short canERR_CAN_BIT0 = 8845U;
 _IEC_CONST unsigned short canERR_CAN_BIT1 = 8844U;
 _IEC_CONST unsigned short canERR_CAN_ACK = 8843U;
 _IEC_CONST unsigned short canERR_CAN_FORM = 8842U;
 _IEC_CONST unsigned short canERR_CAN_STUFF = 8841U;
 _IEC_CONST unsigned short canERR_FBCAN_RXBUF = 8840U;
 _IEC_CONST unsigned short canERR_FBCAN_HANDLE = 8839U;
 _IEC_CONST unsigned short canERR_FBCAN_NODENR = 8838U;
 _IEC_CONST unsigned short canERR_FBCAN_COB = 8837U;
 _IEC_CONST unsigned short canERR_CAN_BITMODE = 8834U;
 _IEC_CONST unsigned short canERR_CAN_WROBJ = 8833U;
 _IEC_CONST unsigned short canERR_CMS_MULTIPLEX = 8832U;
 _IEC_CONST unsigned short canERR_CAN_EXTENDED = 8831U;
 _IEC_CONST unsigned short canERR_CAN_FBHANDLE = 8830U;
 _IEC_CONST unsigned short canERR_CAN_ABORTRECEIV = 8829U;
 _IEC_CONST unsigned short canERR_CAN_ABORTED = 8828U;
 _IEC_CONST unsigned short canERR_CAN_IRQHANDLE = 8827U;
 _IEC_CONST unsigned short canERR_CAN_NILPTR = 8826U;
 _IEC_CONST unsigned short canERR_CAN_NO_NODE = 8825U;
 _IEC_CONST unsigned short canERR_CAN_WRMODE = 8824U;
 _IEC_CONST unsigned short canERR_CAN_WRSTRUCT = 8823U;
 _IEC_CONST unsigned short canERR_WRCMSOBJ = 8822U;
 _IEC_CONST unsigned short canERR_CAN_WRUSRTYP = 8821U;
 _IEC_CONST unsigned short canERR_CAN_NOFUNC = 8820U;
 _IEC_CONST unsigned short canERR_CAN_HTLQUIT = 8819U;
 _IEC_CONST unsigned short canERR_CAN_HTLXINF = 8818U;
 _IEC_CONST unsigned short canERR_CAN_NOCONTR = 8817U;
 _IEC_CONST unsigned short canERR_CAN_COBANZ = 8816U;
 _IEC_CONST unsigned short canERR_CAN_BDRATE = 8815U;
 _IEC_CONST unsigned short canERR_CAN_WRLNG = 8814U;
 _IEC_CONST unsigned short canERR_CAN_NOTINST = 8813U;
 _IEC_CONST unsigned short canERR_CAN_NOHIGH = 8812U;
 _IEC_CONST unsigned short canERR_CAN_BUSOFF = 8811U;
 _IEC_CONST unsigned short canERR_CAN_QFULL = 8810U;
 _IEC_CONST unsigned short canERR_CAN_BUSY = 8809U;
 _IEC_CONST unsigned short canERR_CAN_WRHDLE = 8808U;
 _IEC_CONST unsigned short canERR_CAN_WRDEF = 8807U;
 _IEC_CONST unsigned short canERR_CAN_COBDEF = 8806U;
 _IEC_CONST unsigned short canERR_CAN_WRUSERID = 8805U;
 _IEC_CONST unsigned short canERR_CAN_WRCANID = 8804U;
 _IEC_CONST unsigned short canERR_CAN_COBUSED = 8803U;
 _IEC_CONST unsigned short canERR_CAN_WRIDENT = 8802U;
 _IEC_CONST unsigned short canERR_CAN_NOMEM = 8801U;
 _IEC_CONST unsigned short canERR_CAN_NOENTRY = 8800U;
#endif




/* Datatypes and datatypes of function blocks */
typedef struct INFO_typ
{	unsigned long received;
	unsigned long transmit;
	unsigned long error;
	unsigned long busoff;
	unsigned long warning;
	unsigned long other;
	unsigned long lost;
	unsigned long stufferror;
	unsigned long formerror;
	unsigned long ackerror;
	unsigned long bit1error;
	unsigned long bit0error;
	unsigned long crcerror;
	unsigned long lec0;
	unsigned long rxirq;
	unsigned long txirq;
} INFO_typ;

typedef struct DevParam
{	unsigned char baudrate;
	unsigned short cob_nr;
	unsigned long pError;
	unsigned long pDevice;
	unsigned short info;
} DevParam;

typedef struct CANinfo
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long info_adr;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANinfo_typ;

typedef struct CANMulQueue
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long pCAN_id;
	unsigned char ID_nr;
	unsigned short size;
	/* VAR_OUTPUT (analog) */
	unsigned long q_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANMulQueue_typ;

typedef struct CANxMulQueue
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long pCAN_id;
	unsigned long ID_nr;
	unsigned short size;
	/* VAR_OUTPUT (analog) */
	unsigned long q_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANxMulQueue_typ;

typedef struct CANMulOpen
{
	/* VAR_INPUT (analog) */
	unsigned long pDevParam;
	unsigned char dev_nr;
	/* VAR_OUTPUT (analog) */
	unsigned long us_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANMulOpen_typ;

typedef struct CANMulrd
{
	/* VAR_INPUT (analog) */
	unsigned long q_ident;
	unsigned long data_adr;
	/* VAR_OUTPUT (analog) */
	unsigned short size;
	unsigned long ID;
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANMulrd_typ;

typedef struct CANTime
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned long pSystemTick;
	unsigned long pSystemStamp;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long old_id;
	unsigned long cob_handle;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANTime_typ;

typedef struct CANquwr
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned long data_adr;
	unsigned char data_lng;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANquwr_typ;

typedef struct SetNdNr
{
	/* VAR_INPUT (analog) */
	unsigned long device;
	unsigned char nodenr;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} SetNdNr_typ;

typedef struct GetNdNr
{
	/* VAR_INPUT (analog) */
	unsigned long dev_adr;
	/* VAR_OUTPUT (analog) */
	unsigned char nodenr;
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} GetNdNr_typ;

typedef struct CMSmain
{
	/* VAR_INPUT (analog) */
	unsigned long cms_ident;
	unsigned long cms_mode;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CMSmain_typ;

typedef struct CMSinit
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned char user_type;
	unsigned short queue_cnt;
	unsigned long modul_name;
	/* VAR_OUTPUT (analog) */
	unsigned short cms_entry;
	unsigned long cms_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CMSinit_typ;

typedef struct CMSdlreq
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long client_id;
	unsigned long server_id;
	unsigned long mult;
	unsigned long data_adr;
	unsigned long data_len;
	/* VAR_OUTPUT (analog) */
	unsigned short add_code;
	unsigned char err_class;
	unsigned char err_code;
	unsigned long dl_len;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	plcbit initiate;
	/* VAR (digital) */
	plcbit old_enable;
} CMSdlreq_typ;

typedef struct CMSdlcon
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long client_id;
	unsigned long server_id;
	unsigned long data_adr;
	unsigned long data_len;
	/* VAR_OUTPUT (analog) */
	unsigned short add_code;
	unsigned char err_class;
	unsigned char err_code;
	unsigned long mult;
	unsigned long dl_len;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	plcbit initiate;
	/* VAR (digital) */
	plcbit old_enable;
} CMSdlcon_typ;

typedef struct CANxnode
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	/* VAR_OUTPUT (analog) */
	unsigned char node_nr;
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANxnode_typ;

typedef struct CANwrite
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned long data_adr;
	unsigned char data_lng;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long old_id;
	unsigned long cob_handle;
	unsigned long init_cnt;
	unsigned char wr_order;
	unsigned char align_byte;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANwrite_typ;

typedef struct CANupd
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned long data_adr;
	unsigned char data_lng;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long old_id;
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANupd_typ;

typedef struct CANngp
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned char nd_state;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long old_id;
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANngp_typ;

typedef struct CANrwtab
{
	/* VAR_INPUT (analog) */
	unsigned long tab_ident;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANrwtab_typ;

typedef struct CANrtr
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned long data_adr;
	/* VAR_OUTPUT (analog) */
	unsigned char data_lng;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	plcbit request;
	/* VAR (digital) */
	plcbit old_enable;
} CANrtr_typ;

typedef struct CANread
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned long data_adr;
	/* VAR_OUTPUT (analog) */
	unsigned char data_lng;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANread_typ;

typedef struct CANrd
{
	/* VAR_INPUT (analog) */
	unsigned long q_ident;
	unsigned long data_adr;
	/* VAR_OUTPUT (analog) */
	unsigned short size;
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANrd_typ;

typedef struct CANqueue
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned short size;
	/* VAR_OUTPUT (analog) */
	unsigned long q_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long cob_handle;
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANqueue_typ;

typedef struct CANopen
{
	/* VAR_INPUT (analog) */
	unsigned char baud_rate;
	unsigned short cob_anz;
	unsigned long error_adr;
	unsigned long device;
	unsigned short info;
	/* VAR_OUTPUT (analog) */
	unsigned long us_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANopen_typ;

typedef struct CANnode
{
	/* VAR_OUTPUT (analog) */
	unsigned char node_nr;
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANnode_typ;

typedef struct CANgetid
{
	/* VAR_INPUT (analog) */
	unsigned long data_adr;
	/* VAR_OUTPUT (analog) */
	unsigned char data_lng;
	unsigned long can_id;
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANgetid_typ;

typedef struct CANexcep
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long can_id;
	unsigned char exc_nr;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	unsigned long cob_handle;
	unsigned long ex_p;
	unsigned long ex_maxtask;
	unsigned long ss_irqvw_p;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANexcep_typ;

typedef struct CANdftab
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long table_adr;
	unsigned short tab_num;
	/* VAR_OUTPUT (analog) */
	unsigned long tab_ident;
	unsigned short status;
	/* VAR (analog) */
	unsigned long init_cnt;
	/* VAR_INPUT (digital) */
	plcbit enable;
	/* VAR (digital) */
	plcbit old_enable;
} CANdftab_typ;

typedef struct CANbtreg
{
	/* VAR_INPUT (analog) */
	unsigned long us_ident;
	unsigned long treg0;
	unsigned long treg1;
	/* VAR_OUTPUT (analog) */
	unsigned short status;
	/* VAR_INPUT (digital) */
	plcbit enable;
} CANbtreg_typ;



/* Prototyping of functions and function blocks */
void CANinfo(struct CANinfo* inst);
void CANMulQueue(struct CANMulQueue* inst);
void CANxMulQueue(struct CANxMulQueue* inst);
void CANMulOpen(struct CANMulOpen* inst);
void CANMulrd(struct CANMulrd* inst);
void CANTime(struct CANTime* inst);
void CANquwr(struct CANquwr* inst);
void SetNdNr(struct SetNdNr* inst);
void GetNdNr(struct GetNdNr* inst);
void CMSmain(struct CMSmain* inst);
void CMSinit(struct CMSinit* inst);
void CMSdlreq(struct CMSdlreq* inst);
void CMSdlcon(struct CMSdlcon* inst);
void CANxnode(struct CANxnode* inst);
void CANwrite(struct CANwrite* inst);
void CANupd(struct CANupd* inst);
void CANngp(struct CANngp* inst);
void CANrwtab(struct CANrwtab* inst);
void CANrtr(struct CANrtr* inst);
void CANread(struct CANread* inst);
void CANrd(struct CANrd* inst);
void CANqueue(struct CANqueue* inst);
void CANopen(struct CANopen* inst);
void CANnode(struct CANnode* inst);
void CANgetid(struct CANgetid* inst);
void CANexcep(struct CANexcep* inst);
void CANdftab(struct CANdftab* inst);
void CANbtreg(struct CANbtreg* inst);


#ifdef __cplusplus
};
#endif
#endif /* _CAN_LIB_ */

                                                           
