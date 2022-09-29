//------------------------------------------------------------------------------------------------
//
//  Project         :  Entw. IP Media Network (Lacenet)
//  Project-Nr.     :  EP19-0004
//  Product         :  SLGU, ...
//  File            :  func.c
//  Copyright       :  Salzbrenner media GmbH
//  Authors         :  $Author: wrein $
//  Date            :  10.07.2022
//  SVN-Date        :  $Date:: 2021-05-31 00:00:00 #$
//  SVN-Revision    :  $Revision:: 000              $
//  Standard-Tabs   :  no tabs only spaces
//
//  Description:
//  - Includes common functions
//
//------------------------------------------------------------------------------------------------
#include <stdlib.h>
#ifndef _GLOBALS_H_
#include "globals.h"
#endif //_GLOBAL_H_S

//------------------------------------------------------------------------------------------------
//  FUNCTIONS (implementation)
//------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------
// random_number()
//
//------------------------------------------------------------------------------------------------

int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;
    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    }
    else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    //srand(time(NULL));
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

//------------------------------------------------------------------------------------------------
// ptTXgenerate()
//
///\brief					Complete printout of the transfer and telegram buffer
//
///\param[in]				pu8DataOut	- Output TX buffer
///\param[in]				pu8DataIn	- Input RX buffer
///\param[in]				u8Length	- Length of buffer
///\param[out]
//
///\retval
//------------------------------------------------------------------------------------------------
struct t_SpiDebug* ptTXgenerate(struct t_BufTXgen* ptTelGen)
{
    static struct t_SpiDebug* ptSpiTel;				// A structure for status feedback and debugging
    int z = 0;

    *(ptTelGen->pu8DataOut) = ptTelGen->u8Header;
    *(ptTelGen->pu8DataOut + 1) = 0;        //HEADER
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 - 1) = ptTelGen->u8Header;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 - 2) = ptTelGen->u8PortID;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 - 3) = 1;

    if ((*(ptTelGen->pui0)) >= (ptTelGen->u8LenPT1 - 2))
    {
        (*(ptTelGen->pui0)) = 3;
        (*(ptTelGen->puj0)) = 0;
        //memset(bufTX0, 0, 9);
        *(ptTelGen->pu8DataOut) = ptTelGen->u8Header;
        for (z = 1; z < (ptTelGen->u8LenPT1 - 3); z++)
        {
            *(ptTelGen->pu8DataOut + z) = 0;
        }
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 - 1) = ptTelGen->u8Header;
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 - 2) = ptTelGen->u8PortID;
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 - 3) = 1;
    }
    *(ptTelGen->pu8DataOut + (*(ptTelGen->pui0))) = *(ptTelGen->pu8DataOut + (*(ptTelGen->pui0))) + 1;
    (*(ptTelGen->puj0))++;
    if ((*(ptTelGen->puj0)) >= 160)
    {
        (*(ptTelGen->pui0))++;
        (*(ptTelGen->puj0)) = 0;
    }
    // Port 0 Tel 2
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1) = ptTelGen->u8Header;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1 + 1) = 0;   //HEADER
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 - 1) = ptTelGen->u8Header;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 - 2) = ptTelGen->u8PortID;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 - 3) = 2;

    if ((*(ptTelGen->pui1)) >= (ptTelGen->u8LenPT2 - 2))
    {
        (*(ptTelGen->pui1)) = ptTelGen->u8LenPT1 + 3;
        (*(ptTelGen->puj1)) = 0;
        //memset(bufTX0, 0, 9);
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT1) = ptTelGen->u8Header;
        for (z = (ptTelGen->u8LenPT1 + 1); z < (ptTelGen->u8LenPT2 - 3); z++)
        {
            *(ptTelGen->pu8DataOut + z) = 0;
        }
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 - 1) = ptTelGen->u8Header;
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 - 2) = ptTelGen->u8PortID;
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 - 3) = 2;
    }
    *(ptTelGen->pu8DataOut + (*(ptTelGen->pui1))) = *(ptTelGen->pu8DataOut + (*(ptTelGen->pui1))) + 1;
    (*(ptTelGen->puj1))++;
    if ((*(ptTelGen->puj1)) >= 160)
    {
        (*(ptTelGen->pui1))++;
        (*(ptTelGen->puj1)) = 0;
    }

    // Port 0 Tel 3
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2) = ptTelGen->u8Header;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2 + 1) = 0;   //HEADER
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT3 - 1) = ptTelGen->u8Header;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT3 - 2) = ptTelGen->u8PortID;
    *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT3 - 3) = 3;

    if ((*(ptTelGen->pui2)) >= (ptTelGen->u8LenPT3 - 2))
    {
        (*(ptTelGen->pui2)) = ptTelGen->u8LenPT2 + 3;
        (*(ptTelGen->puj2)) = 0;
        //memset(bufTX0, 0, 9);
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT2) = ptTelGen->u8Header;
        for (z = (ptTelGen->u8LenPT2 + 1); z < (ptTelGen->u8LenPT3 - 3); z++)
        {
            *(ptTelGen->pu8DataOut + z) = 0;
        }
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT3 - 1) = ptTelGen->u8Header;
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT3 - 2) = ptTelGen->u8PortID;
        *(ptTelGen->pu8DataOut + ptTelGen->u8LenPT3 - 3) = 3;
    }
    *(ptTelGen->pu8DataOut + (*(ptTelGen->pui2))) = *(ptTelGen->pu8DataOut + (*(ptTelGen->pui2))) + 1;
    (*(ptTelGen->puj2))++;
    if ((*(ptTelGen->puj2)) >= 160)
    {
        (*(ptTelGen->pui2))++;
        (*(ptTelGen->puj2)) = 0;
    }

    return ptSpiTel;
}
