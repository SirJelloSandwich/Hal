/********************************************************************
 * COPYRIGHT -- belgor
 ********************************************************************
 * Program: hmilogic
 * File: hmiDialog.h
 * Author: goran
 * Created: September 05, 2013
 *******************************************************************/

#include <bur/plctypes.h>
#ifdef _DEFAULT_INCLUDES
#include <AsDefault.h>
#endif
BOOL hmiDialogIsActive(hmiDialog_typ* p);
void hmiDialogHide(hmiDialog_typ* p);
void hmiDialogShow(hmiDialog_typ* p, char* msg, UINT timeout_ms, UINT resultForOk, UINT resultForCancel, UINT timeoutConfig);
UINT hmiDialogCyclic(hmiDialog_typ* p);




