/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: hmilogic
 * File: hmiAlert.h
 * Author: goran
 * Created: September 05, 2013
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif
void hmiAlertHide(hmiAlert_typ* p);
BOOL hmiAlertIsActive(hmiAlert_typ* p);
void hmiAlertShow(hmiAlert_typ* p, char* alertText, UDINT alertDuration);
void hmiAlertCyclic(hmiAlert_typ* p);



