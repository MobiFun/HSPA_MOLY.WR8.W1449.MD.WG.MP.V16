/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 * Filename:
 * ---------
 * Custom_at_command.c
 *
 * Description:
 * ------------
 *   This file is intends for Customer to implement and parse its own defined AT commands
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 * removed!
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 * removed!
 * removed!
 * removed!
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
*****************************************************************************/
//#include "kal_release.h"
//#include "kal_non_specific_general_types.h"
#include "stdio.h"
#include "string.h"
//#include "stack_config.h"	

#include "kal_general_types.h"
#include "kal_trace.h"
#if (defined(__COMPOSITE_WEBCAM__))
#include "cal_api.h"
#include "cal_comm_def.h"
#endif
#ifdef __TWOMICNR_SUPPORT__
#include "two_mic_NR_custom_if.h" 
#endif
#include "ps_public_enum.h"
#include "hal_ul1_struct.h"
#include "smsal_l4c_enum.h"

#ifdef __PPE_INTERFACE_SUPPORT__
#include "rmmi_rspfmttr.h"
#include "rmmi_utility.h"
#endif 

extern kal_bool gas_command_hdlr( kal_uint8* cmd_name, kal_uint8*full_cmd_string);

extern void kal_monitor_buffer(kal_uint32 index);
extern kal_bool twomicnr_command_hdlr(char *full_cmd_string);

/*****************************************************************************
* CUSTOM_SYMBOL:
*	customer can modify this symbol.
*     Every command begin with this symbol will be recognized as a customer-defined command
*	and passed to custom_command_hdlr().
*     For example: if CUSTOM_SYMBOL is set to '^', 
*				then AT^AAAA, AT^BBBB will be customer-defined command.
*   	NOTE:  '+'  and '/' and ' \ 'is not allowed since they might be standard command format
*****************************************************************************/
#define CUSTOM_SYMBOL  '^'	 // '+'  and '/' and ' \ 'is NOT allowed   

/*****************************************************************************
* MAX_UART_LEN = 128 bytes.
*	when you write a lot of data to UART at the same time, 
*	UART's buffer might be full and cannot handle.
*     so the safer way is to seperate the long data, each time only write data length <= 128
*****************************************************************************/
#define MAX_UART_LEN	128  

kal_uint8 custom_get_atcmd_symbol(void);
#ifdef __PPE_INTERFACE_SUPPORT__
kal_bool custom_command_hdlr(char *full_cmd_string, kal_uint8 src_id, kal_bool check_custom_cmd);
#else
kal_bool custom_command_hdlr(char *full_cmd_string);
#endif

/*****************************************************************************
* FUNCTION
*  	rmmi_write_to_uart()
* DESCRIPTION
*   	This function extern L4C RMMI function,
*     is to write data to UART
* PARAMETERS
*   	kal_uint8 * buffer 	- the data to be written
*	kal_uint16 length 	- the length of data to be written
*     kal_bool stuff 		- KAL_TRUE : <CR><LF> will be added at the beginning and end. (eg.<CR><LF>OK<CR><LF>)
*					   KAL_FALSE: no <CR><LF> is added
* RETURNS
*	none
*****************************************************************************/
extern void rmmi_write_to_uart(kal_uint8 * buffer, kal_uint16 length, kal_bool stuff); 

#ifdef __GATI_ENABLE__
extern kal_bool gati_command_hdlr(kal_uint8* cmd_string, kal_uint8* full_string);

module_type gati_mod_table[] = 
{
    MOD_PHB,
    MOD_L4C,
    MOD_L4C,
    MOD_L4C, 
    MOD_NIL         /* Need to be the last one */
};

kal_char *gati_cmd_table[] = 
{
    "CPBBA",
    "CPBBR",
    "CPBBS",
    "CPBB"
};

#endif


#define COMMAND_LINE_SIZE    (180)
#define NULL_TERMINATOR_LENGTH (1)

 typedef struct 
{
  short  position;
  short  length;
  char   character[COMMAND_LINE_SIZE + NULL_TERMINATOR_LENGTH];
#ifdef __PPE_INTERFACE_SUPPORT__
  kal_uint8 src_id;
#endif
} custom_cmdLine;

typedef enum
{
    CUSTOM_WRONG_MODE,
    CUSTOM_SET_OR_EXECUTE_MODE,
    CUSTOM_READ_MODE,
    CUSTOM_TEST_MODE,
    CUSTOM_ACTIVE_MODE
} custom_cmd_mode_enum;

typedef enum
{
    CUSTOM_RSP_ERROR = -1,
    CUSTOM_RSP_OK = 0,
    CUSTOM_RSP_BYPASS,
#ifdef __PPE_INTERFACE_SUPPORT__
    CUSTOM_RSP_PPE
#endif
} custom_rsp_type_enum;

typedef struct
{
	char *commandString;
	custom_rsp_type_enum (*commandFunc)(custom_cmdLine *commandBuffer_p);
} custom_atcmd;

custom_cmd_mode_enum custom_find_cmd_mode(custom_cmdLine *cmd_line);

custom_rsp_type_enum custom_test_func(custom_cmdLine *commandBuffer_p)
{
    custom_cmd_mode_enum result;
    custom_rsp_type_enum ret_value  = CUSTOM_RSP_ERROR;

    result = custom_find_cmd_mode(commandBuffer_p);		
    switch (result)
    {
        case CUSTOM_READ_MODE:
            ret_value = CUSTOM_RSP_OK;
            break;
        case CUSTOM_ACTIVE_MODE:
            ret_value = CUSTOM_RSP_OK;
            break;
        case CUSTOM_SET_OR_EXECUTE_MODE:
            ret_value = CUSTOM_RSP_OK;
            break;
        case CUSTOM_TEST_MODE:
            ret_value = CUSTOM_RSP_OK;
            break;    
        default:
            ret_value = CUSTOM_RSP_ERROR;
            break;
	}
    return ret_value;
}

extern custom_rsp_type_enum custom_ppe_cmgs_func(custom_cmdLine *commandBuffer_p);
extern custom_rsp_type_enum custom_ppe_atd_func(custom_cmdLine *commandBuffer_p);
extern custom_rsp_type_enum custom_ppe_cgact_func(custom_cmdLine *commandBuffer_p);


/********************************
* The following table is used for the customer
* They can defined their own command and 
* corresponding handler function here
* Note that the last line {NULL, NULL} should not be removed
* since it is the terminator indicator
*********************************/
const custom_atcmd custom_cmd_table[ ] =
{    
    {"AT%CUSTOM",custom_test_func},
#ifdef __PPE_INTERFACE_SUPPORT__
    {"AT+CMGS",custom_ppe_cmgs_func},
    {"AT+EMGS",custom_ppe_cmgs_func},
    {"ATD",custom_ppe_atd_func},
    {"AT+CGACT",custom_ppe_cgact_func},
#endif        
    {NULL, NULL}  // this lind should not be removed, it will be treat as 
};

/*****************************************************************************
* FUNCTION
*     custom_find_cmd_mode()
* DESCRIPTION
*     This function is used to parsing the command mode for the customer
*     The input position of the cmd_line should stay at the character right after the command name
* PARAMETERS
*     cmd_line      the custom_cmdLine which contains the command string and its current parsing position
* RETURNS
*     custom_cmd_mode_enum
*****************************************************************************/
custom_cmd_mode_enum custom_find_cmd_mode(custom_cmdLine *cmd_line)
{
    custom_cmd_mode_enum result;

    if (cmd_line->position < cmd_line->length - 1)
    {
        switch (cmd_line->character[cmd_line->position])
        {
            case '?':  /* AT+...? */
            {
                cmd_line->position++;
                result = CUSTOM_READ_MODE;
                break;
            }
            case '=':  /* AT+...= */
            {
                cmd_line->position++;
                if ((cmd_line->position < cmd_line->length - 1 ) &&
                    (cmd_line->character[cmd_line->position] == '?'))
                {
                    cmd_line->position++;
                    result = CUSTOM_TEST_MODE;
                }
                else
                {
                    result = CUSTOM_SET_OR_EXECUTE_MODE;
                }
                break;
            }
            default:  /* AT+... */
            {
                result = CUSTOM_ACTIVE_MODE;
                break;
            }
        }
    }
    else
    {
        result = CUSTOM_ACTIVE_MODE;
    }
    return (result);
}


/*****************************************************************************
* FUNCTION
*  	custom_command_hdlr()
* DESCRIPTION
*   	This function should parse the custom AT command and do correspondent action.
*     Customer should maintain and modify the code.
* PARAMETERS
*   	kal_uint8 * cmd_string
* RETURNS
*    KAL_TRUE : the command is handled by the customer
*    KAL_FALSE: the command is not handled by the customer
*****************************************************************************/
#ifdef __PPE_INTERFACE_SUPPORT__
kal_bool custom_command_hdlr(char *full_cmd_string, kal_uint8 src_id, kal_bool check_custom_cmd)
#else
kal_bool custom_command_hdlr(char *full_cmd_string)
#endif
{
    char buffer[MAX_UART_LEN+1]; //MAUI_02377056
    char *cmd_name, *cmdString;
    kal_uint8 index = 0; 
    kal_uint16 length;
    kal_uint16 i;
    custom_cmdLine command_line;
#ifdef __PPE_INTERFACE_SUPPORT__
    if (check_custom_cmd == KAL_FALSE)
    {
        return KAL_FALSE;
    }
#endif
    cmd_name = buffer;

    length = strlen(full_cmd_string);
    length = length > MAX_UART_LEN ? MAX_UART_LEN : length;    
    while ((full_cmd_string[index] != '=' ) &&  //might be TEST command or EXE command
        (full_cmd_string[index] != '?' ) && // might be READ command
        (full_cmd_string[index] != 13 ) && //carriage return
        index < length)  
    {
        cmd_name[index] = full_cmd_string[index] ;
        index ++;
    }
#ifdef __PPE_INTERFACE_SUPPORT__
    if (strncmp(cmd_name,"ATD",3)==0)//mtk04499 for PPE ATD/ATDE
    {
        cmd_name[3] = '\0';
    }
    else
#endif
    {
        cmd_name[index] = '\0' ;    
    }
    
    for (i = 0 ; custom_cmd_table[i].commandString != NULL; i++ )
    {
        cmdString = custom_cmd_table[i].commandString;
        if (strcmp(cmd_name, cmdString) == 0 )
        {
            custom_rsp_type_enum rsp;

            strncpy(command_line.character, full_cmd_string, COMMAND_LINE_SIZE + NULL_TERMINATOR_LENGTH);
            command_line.character[COMMAND_LINE_SIZE] = '\0';
            command_line.length = strlen(command_line.character);
            command_line.position = index;
		#ifdef __PPE_INTERFACE_SUPPORT__
            command_line.src_id = src_id;
		#endif
            rsp = custom_cmd_table[i].commandFunc(&command_line);

            if (rsp == CUSTOM_RSP_OK) 
            {
                sprintf(buffer, "OK");
                rmmi_write_to_uart((kal_uint8*)buffer, strlen(buffer), KAL_TRUE);
            }
            else if (rsp == CUSTOM_RSP_ERROR)
            {
                sprintf(buffer, "ERROR");
                rmmi_write_to_uart((kal_uint8*)buffer, strlen(buffer), KAL_TRUE);
            }
            else if (rsp == CUSTOM_RSP_BYPASS)
            {
                return KAL_FALSE;
            }

            return KAL_TRUE;
        }
    }    

    if (index <= 2)
        return KAL_FALSE;
	
    /* just a very basic example : customer can implement their own */
    cmd_name += 3;
    if (strcmp(cmd_name, "EXAMPLE") == 0)
    {		
        /* BEGIN: do the following parsing and correspondent action */
        /*  												    */
        /*  												    */
        /*  												    */
        /* END: do the following parsing and correspondent action    */

        /* generate final result code: "OK" or "ERROR" */
        if(KAL_TRUE)    // if operation is success
        {
                sprintf(buffer, "OK");
                rmmi_write_to_uart((kal_uint8*)buffer, strlen(buffer), KAL_TRUE);
        }
        else if (KAL_FALSE) // if operation is fail
        {
            sprintf(buffer, "ERROR");
            rmmi_write_to_uart((kal_uint8*)buffer, strlen(buffer), KAL_TRUE);
        }
        return KAL_TRUE;
    }
    #if !defined(__MMI_FMI__) || defined(WISDOM_MMI)
    else if (strcmp(cmd_name, "buffer") == 0)
    {		
        kal_sys_trace("Buffer Monitor enabled!");
        kal_monitor_buffer(0xffffffff);		
        sprintf(buffer, "OK");
        rmmi_write_to_uart((kal_uint8*)buffer, strlen(buffer), KAL_TRUE);

        return KAL_TRUE;
    }	
    #endif	
    #ifdef __TWOMICNR_SUPPORT__ 
    else if (twomicnr_command_hdlr(full_cmd_string) == KAL_TRUE)
    {
	return KAL_TRUE;
    }
    #endif
    #if (defined(__COMPOSITE_WEBCAM__)) 
    else if (strcmp(cmd_name, "SENSOR") == 0)
    {
        CAL_FEATURE_CTRL_STRUCT CalInPara;
        CalInPara.FeatureId = CAL_FEATURE_CAMERA_TUNING_SENSOR;
        CalInPara.FeatureSetValue = (kal_uint32)(full_cmd_string);
        CalCtrl(CAL_FEATURE_CTRL, (void*)&CalInPara, NULL, 0, NULL);
        return KAL_TRUE;
    }
    #endif
    else
    { 
#ifdef __GAS_SUPPORT__
        if( gas_command_hdlr( (kal_uint8*)cmd_name, (kal_uint8*)full_cmd_string)==KAL_TRUE )
        {
            // it is a gas command and we will send the command to the appropriate module
            return KAL_TRUE;
        }
           
#endif
    #ifdef __GATI_ENABLE__ 
        /* Judge if the command string is a GATI command*/
        if( gati_command_hdlr( (kal_uint8*)cmd_name, (kal_uint8*)full_cmd_string)==KAL_TRUE )
        {
            // it is a gati command and we will send the command to the appropriate module
            return KAL_TRUE;
        }		 	
    #endif
    }	
		 
    /* unrecognized command */
    /* we should return KAL_FALSE to let ATCI judge if the it can handle the command */
    return KAL_FALSE;
	
}

/*****************************************************************************
* FUNCTION
*  	custom_get_atcmd_symbol()
* DESCRIPTION
*   	This function returns special symbol for customer-defined AT command, 
*   	so AT parser can recognize its a custom command and forward the string to custom_command_handler().
*
*     CUSTOMER DOES NOT need to modify this function.
* PARAMETERS
*   	none
* RETURNS
*     kal_uint8 
*****************************************************************************/
kal_uint8 custom_get_atcmd_symbol(void)
{
   return (CUSTOM_SYMBOL);
}

/*****************************************************************************
* USER DEFINE CONSTANT
*  	RMMI_CUSTOM_DCD_VALUE_IN_DATA_STATE
* DESCRIPTION
* 	1. This value defines the control DCD value for data mode (CSD/GPRS dialup)
*   2. CUSTOMER CAN modify this value accroding to its TE requirement
*****************************************************************************/
#define RMMI_CUSTOM_DCD_VALUE_IN_DATA_STATE (1)

/*****************************************************************************
* USER DEFINE CONSTANT
*  	RMMI_CUSTOM_DCD_VALUE_IN_CMD_STATE
* DESCRIPTION
* 	1. This value defines the control DCD value for AT command mode (Normal/Escaped AT)
*   2. CUSTOMER CAN modify this value accroding to its TE requirement
*****************************************************************************/
#define RMMI_CUSTOM_DCD_VALUE_IN_CMD_STATE (0)

/*****************************************************************************
* FUNCTION
*  	custom_get_dcd_value_for_data_state()
* DESCRIPTION
*   	This function returns the user-defined DCD value for data state
*
*     CUSTOMER DOES NOT need to modify this function.
* PARAMETERS
*   	none
* RETURNS
*     kal_uint8 
*****************************************************************************/
kal_uint8 custom_get_dcd_value_for_data_state(void)
{
    return (kal_uint8) RMMI_CUSTOM_DCD_VALUE_IN_DATA_STATE;
}

/*****************************************************************************
* FUNCTION
*  	custom_get_dcd_value_for_cmd_state()
* DESCRIPTION
*   	This function returns the user-defined DCD value for command state
*       It shall be inverted value of custom_get_dcd_value_for_data_state()
*     CUSTOMER DOES NOT need to modify this function.
* PARAMETERS
*   	none
* RETURNS
*     kal_uint8 
*****************************************************************************/
kal_uint8 custom_get_dcd_value_for_cmd_state(void)
{
    return (kal_uint8)RMMI_CUSTOM_DCD_VALUE_IN_CMD_STATE;
}

#ifdef __MOD_TCM__
/*****************************************************************************
* FUNCTION
*  	custom_get_gprs_dialup_connect_post_string()
* DESCRIPTION
*   This function returns the post string right after CONNECT in gprs dialup.
*
*   The return string length must be less than 64
*
*   If post_string is XXXXXX, 
*   then MS sends "<cr><lf>CONNECT XXXXXX<cr><lf>" when GPRS dialup
*   Ex.1 if post_string=" 230400", 
*        then entire output string is "<cr><lf>CONNECT 230400<cr><lf>"
*   Ex.2 if post_string=" 56000 V42bis", 
*        then entire output string is "<cr><lf>CONNECT 56000 V42bis<cr><lf>"
*    
* PARAMETERS
*   kal_uint8 act, ACcess Technology, 
                   This field value can be referred to TS27.007
                   0,   GSM
                   1,   GSM Compact
                   2,   UTRAN
                   3,   GSM w/EGPRS
                   4,   UTRAN w/HSDPA
                   5,   UTRAN w/HSUPA
                   6,   UTRAN w/HSDPA and HSUPA
* RETURNS
*     kal_uint8* post_string
*****************************************************************************/
const kal_uint8* custom_get_gprs_dialup_connect_post_string(kal_uint8 act)
{
    /* Note: The return string length must be less than 64!!! */
#ifdef __TC01__
    #ifdef __REL8__     // note: __R7_HSDPA_PLUS_SUPPORT__ not defined
/* under construction !*/
    #elif defined(__HSDPA_SUPPORT__)
/* under construction !*/
	#elif defined(__EGPRS_MODE__)
/* under construction !*/
	#else
/* under construction !*/
	#endif
#elif defined(__PS_DIALUP_CONNECT_STRING_BY_ACT__)
    static kal_uint8 temp_string[64];
    const kal_uint8* post_string = temp_string;    
    if ((act == 0) || (act == 1) || (act == 3))
    {
        strcpy(temp_string, " 236800");
    }
    else
    {
    #if defined(__REL8__) && defined(__UMTS_FDD_MODE__)
        strcpy(temp_string, " 42000000");
    #else
        strcpy(temp_string,  " 7200000");
    #endif
    }    
#else
  //Note: user MUST prepare a space before the speed number
    #if defined(__REL8__) && defined(__UMTS_FDD_MODE__)
        static const kal_uint8 post_string[] = " 42000000"; //42Mbps
    #elif defined(__HSDPA_SUPPORT__) && defined(__UMTS_FDD_MODE__)
        static const kal_uint8 post_string[] =  " 7200000"; //7.2Mbps
    #elif defined(__HSDPA_SUPPORT__) && defined(__UMTS_TDD128_MODE__)
        static const kal_uint8 post_string[] =  " 2624000"; //2.6Mbps
    #elif defined(__UMTS_FDD_MODE__) || defined(__UMTS_TDD128_MODE__)
        static const kal_uint8 post_string[] =   " 384000"; //384kbps
    #elif defined(__EGPRS_MODE__)
        static const kal_uint8 post_string[] =   " 473600"; //473.6kbps
    #else
        static const kal_uint8 post_string[] =    " 80000"; //80kbps  
    #endif
#endif
    return (const kal_uint8*) post_string;
} // MAUI_02386357, mtk02285, MAUI_02825172
#endif /* __MOD_TCM__ */

/*****************************************************************************
* FUNCTION
*  	custom_is_edit_mode_command()
* DESCRIPTION
*   This function is used to let customer judge if the input command is needed to enter 
* editor mode or not. If true, we will output "> " and wait for 
*       ctrl+z character to execute the whole command string
*       ESC character to cancel the command
*    
* PARAMETERS
*   	data    [IN]    input command string header, maximum length : 10
* RETURNS
*     KAL_TRUE or KAL_FALSE
*****************************************************************************/
kal_bool custom_is_edit_mode_command(kal_uint8 *data)
{
    /*if (strncmp(data, "AT%EDIT", 7) == 0)
        return KAL_TRUE;
    else*/
        return KAL_FALSE;
}

/*****************************************************************************
* FUNCTION
*  	custom_max_bearer_capability()
* DESCRIPTION
*       This function comes from request by customer to limit DL 7M and UL 5M
*       However, mac-ehs(R7R8 feature) is not disable. Therefore, according to
*       bearer capability, it may show H+ icon. This patch will limit bearer 
*       capability to show H icon, as customer expectation.
*    
* PARAMETERS
*   	void
* RETURNS
*       l4c_data_bearer_capablility_enum
*****************************************************************************/
l4c_data_bearer_capablility_enum custom_max_bearer_capability(void)
{
#if defined(__HSDPA_SUPPORT__) && defined(__UMTS_FDD_MODE__) && defined(__MTK_TARGET__)
    FDD_HSDPA_CATEGORY_E dpa_cat;
    FDD_HSUPA_CATEGORY_E upa_cat;
    
    if (KAL_TRUE == UL1D_Get_FDD_HSPA_Phy_Category(&dpa_cat, &upa_cat))
    {
        if ((dpa_cat<=FDD_HSDPA_PHY_CAT12)&&(upa_cat<=FDD_HSUPA_PHY_CAT6))
            return L4C_HSDPA_HSUPA_CAPABILITY;  // limit to HSPA
    }
#endif 
    return L4C_DC_HSPAP_CAPABILITY; // limit to HSPA+
}

l4c_data_speed_support_enum custom_max_data_speed_support(void)
{
#if defined(__HSDPA_SUPPORT__) && defined(__UMTS_FDD_MODE__) && defined(__MTK_TARGET__)
    FDD_HSDPA_CATEGORY_E dpa_cat;
    FDD_HSUPA_CATEGORY_E upa_cat;

    if (KAL_TRUE == UL1D_Get_FDD_HSPA_Phy_Category(&dpa_cat, &upa_cat))
    {
        if ((dpa_cat<=FDD_HSDPA_PHY_CAT12)&&(upa_cat<=FDD_HSUPA_PHY_CAT6))
            return L4C_HSDPA_UPA_SUPPORT;  // limit to HSPA
    }
#endif 
    return L4C_DC_HSPAP_SUPPORT; // limit to HSPA+
}

#ifdef __PPE_INTERFACE_SUPPORT__
custom_rsp_type_enum custom_ppe_cmgs_func(custom_cmdLine *commandBuffer_p)
{
    custom_cmd_mode_enum mode;
    custom_rsp_type_enum ret_value = CUSTOM_RSP_ERROR;
    void *arg_list[2];
    kal_uint8 cmd_str[768];
    kal_uint8 src_id;
    kal_uint16 string_length = 0;
    kal_bool result = KAL_FALSE;

    mode = custom_find_cmd_mode(commandBuffer_p);
    src_id = commandBuffer_p->src_id;

    switch (mode)
    {
        case CUSTOM_SET_OR_EXECUTE_MODE:
            arg_list[0] = (void*)commandBuffer_p->character;
            string_length = rmmi_fast_string_print(cmd_str,
                                                   (kal_uint8*) "%s",
                                                   arg_list,
                                                   1);

            result = atci_ppe_proxy_cmd_ind(PPE_SMS, PPE_MO_SMS, src_id, cmd_str, string_length);
            ret_value = (result == KAL_TRUE)? CUSTOM_RSP_PPE : CUSTOM_RSP_ERROR;
            break;
        default:
            ret_value = CUSTOM_RSP_BYPASS;
            break;
    }
    return ret_value;
}

custom_rsp_type_enum custom_ppe_atd_func(custom_cmdLine *commandBuffer_p)
{
    kal_bool result;
    custom_rsp_type_enum ret_value  = CUSTOM_RSP_ERROR;
    kal_uint8 cmd_str[768];
    kal_uint16 string_length = 0;
    void *arg_list[2];
    kal_uint8 call_id = 0;

    l4ccsm_cc_get_CSM_call_id(&call_id);

    arg_list[0] = (void*)commandBuffer_p->character;
    arg_list[1] = (void*)&call_id;//get call_id and set context to remember this
    string_length = rmmi_fast_string_print(cmd_str, (kal_uint8*) "%s,%d", arg_list, 2);
    result = atci_ppe_proxy_cmd_ind(PPE_CC, PPE_MO_CALL, commandBuffer_p->src_id, cmd_str, string_length);
    ret_value = (result == KAL_TRUE)? CUSTOM_RSP_PPE : CUSTOM_RSP_ERROR;
    return ret_value;
}

custom_rsp_type_enum custom_ppe_cgact_func(custom_cmdLine *commandBuffer_p)
{
    custom_cmd_mode_enum result;
    custom_rsp_type_enum ret_value = CUSTOM_RSP_ERROR;
    void *arg_list[2];
    kal_uint8 cmd_str[512];
    kal_uint8 src_id, i;
    kal_uint16 string_length = 0;
    kal_bool proxy_result;
    kal_uint8 state = 1;

    result = custom_find_cmd_mode(commandBuffer_p);

    /* PPE checking always use PDU mode*/

    switch (result)
    {
        case CUSTOM_SET_OR_EXECUTE_MODE:


            // bypass deactivate PDP (AT+CGACT=0,<cid>)
            for (i=0; i < commandBuffer_p->length; i++)
            {
                if (commandBuffer_p->character[i] == '0')
                {
                    state = 0;
                    break;
                }
            }

            if (state == 1)
            {            
                arg_list[0] = (void*)commandBuffer_p->character;
                string_length = rmmi_fast_string_print(cmd_str,
                                                       (kal_uint8*) "%s",
                                                       arg_list,
                                                       1);


                src_id = commandBuffer_p->src_id;
                proxy_result = atci_ppe_proxy_cmd_ind(PPE_GPRS, PPE_MO_PS_CALL, src_id, cmd_str, string_length);
                
                if (proxy_result == KAL_TRUE)
                {
                    ret_value = CUSTOM_RSP_PPE;
                }
            }
            else
            {
                ret_value = CUSTOM_RSP_BYPASS;
            }
            break;
            

        default:
            ret_value = CUSTOM_RSP_BYPASS;
            break;
    }
    return ret_value;
}


#endif
