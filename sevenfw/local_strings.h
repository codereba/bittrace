/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LOCAL_STRINGS_H__
#define __LOCAL_STRINGS_H__

#include "string_common.h"

extern ULONG lang_id; 

typedef enum _LANG_ID
{
	LANG_ID_CN, 
	LANG_ID_EN, 
	MAX_LANG_ID
} LANG_ID, *PLANG_ID; 

#define is_valid_lang_id( id ) ( id >= LANG_ID_CN && id <= LANG_ID_EN )

INLINE LPCWSTR get_lang_desc( LANG_ID id )
{
	LPCWSTR desc = L"UNKNOWN_LANG"; 

	switch( id )
	{
	case LANG_ID_CN:
		desc = L"����"; 
		break; 
	case LANG_ID_EN: 
		desc = L"English"; 
		break; 
	default:
		break; 
	}

	return desc; 
}

typedef enum _ALL_LANG_STRINGS_ID
{
	//Main button and title
	TEXT_MAIN_BUTTON_TRACE, 
	TEXT_MAIN_BUTTON_TRIGGER, 
	TEXT_MAIN_BUTTON_TOOLS, 
	TEXT_MAIN_BUTTON_HELP, 
	TEXT_LOGO_7LAYER_FW, 
	TEXT_MAIN_BITSAFE_TITLE, 
	TEXT_MAIN_SYS_MENU_HIDE, 
	TEXT_MAIN_BITSAFE_STARTED_BALLOON_TIP, 

	//Security logo description
	TEXT_SECURITY_LOGO_OPENED, 
	TEXT_SECURITY_LOGO_CLOSED, 
	TEXT_SECURITY_OPEN, 

	//Help panel
	TEXT_HELP_PANEL_HELP, 
	TEXT_HELP_PANEL_FORUM, 
	TEXT_HELP_PANEL_ABOUT, 
	TEXT_HELP_PANEL_MANUAL_UPDATE, 
	TEXT_HELP_PANEL_MANUAL_SUGGESTION, 
	TEXT_HELP_PANEL_ERC, 
	TEXT_HELP_PANEL_RELEARN_RULE, 
	TEXT_HELP_PANEL_OPTION, 
	//TEXT_LANG_BTN_TEXT, 
	TEXT_SUGGESTION_TITLE, 
	TEXT_HELP_TITLE, 
	TEXT_HELP_PANEL_UNINSTALL, 

	//Defense panel
	TEXT_DEFENSE_PANEL_NETWORK_PANEL, 
	TEXT_DEFENSE_PANEL_ALL_PROCESS, 
	TEXT_DEFENSE_PANEL_DEFENSE_POWER, 
	TEXT_DEFENSE_PANEL_DEFENSE_SETTING, 
	TEXT_DEFENSE_PANEL_TRUST_FILE, 
	TEXT_DEFENSE_PANEL_DISTRUST_FILE, 
	TEXT_DEFENSE_PANEL_DEFENSE_LOG, 

	//Firewall panel
	TEXT_FIREWALL_PANEL_LOG, 
	TEXT_FIREWALL_PANEL_TRUST_FILE, 
	TEXT_FIREWALL_PANEL_DISTRUST_FILE, 
	TEXT_FIREWALL_PANEL_SETTING, 
	TEXT_FIREWALL_PANEL_ICMP_CTRL,  
	TEXT_FIREWALL_PANEL_ALL_CONNECTIONS, 
	TEXT_FIREWALL_PANEL_ANTI_ARP, 
	TEXT_FIREWALL_PANEL_USER_ACCOUNT, 
	TEXT_FIREWALL_PANEL_WORK_MODE, 

	//Summary panel
	TEXT_SUMMARY_PANEL_FIREWALL_TITLE, 
	TEXT_SUMMARY_PANEL_FIREWALL_BLOCK_COUNT_LEFT, 
	TEXT_SUMMARY_PANEL_FIREWALL_BLOCK_COUNT_RIGHT, 
	TEXT_SUMMARY_PANEL_ALL_UPLOADED_LEFT, 
	TEXT_SUMMARY_PANEL_ALL_UPLOADED_RIGHT, 
	TEXT_SUMMARY_PANEL_ALL_DOWNLOADED_LEFT, 
	TEXT_SUMMARY_PANEL_ALL_DOWNLOADED_RIGHT, 
	TEXT_SUMMARY_PANEL_DEFENSE_TITLE, 
	TEXT_SUMMARY_PANEL_DEFENSE_BLOCK_COUNT_LEFT, 
	TEXT_SUMMARY_PANEL_DEFENSE_BLOCK_COUNT_RIGHT, 
	TEXT_SUMMARY_PANEL_SYS_MONITOR_TIP, 
	
	//bitsafe slogan
	TEXT_SLOGAN_NOT_TIP_AGAIN, 
	TEXT_SLOGAN_SECURITY_SOFT_COMPATIBLE, 	
	TEXT_BITSAFE_SLOGAN, 
	TEXT_BITSAFE_UPDATING_TIP, 

	TEXT_COMMON_NOT_SUPPORT_64BIT_OS, 
	TEXT_COMMON_MUST_RESTART, 
	TEXT_COMMON_DONT_HAVE_PRIVILEGE, 

	TEXT_UI_CUSTOM_TITLE, 

	//Common button text
	TEXT_COMMON_BUTTON_OK, 
	TEXT_COMMON_BUTTON_CANCEL, 

	TEXT_COMMON_BUTTON_YES, 
	TEXT_COMMON_BUTTON_NO, 

	TEXT_COMMON_BUTTON_CONFIRM, 
	TEXT_COMMON_BUTTON_CANCEL2, 

	TEXT_COMMON_BUTTON_OPEN, 
	TEXT_COMMON_BUTTON_CLOSE, 

	//User login text
	TEXT_USER_LOGIN_TITLE, 
	TEXT_USER_LOGIN_LOGIN_BUTTON, 
	TEXT_USER_LOGIN_PASSWORD, 
	TEXT_USER_LOGIN_LOGIN_TOOLTIP, 
	TEXT_USER_LOGIN_CHANGE_PASSWORD_BUTTON, 
	TEXT_USER_LOGIN_CALCEL_BUTTON_TOOLTIP, 
	TEXT_USER_LOGIN_INCORRECT_PASSWORK_TIP, 
	TEXT_USER_LOGIN_SUCCESSFUL_TIP, 

	//Change password dialog
	TEXT_CHANGE_PASSWORD_NEW_NOT_SAME_ERROR, 
	TEXT_CHANGE_PASSWORD_OLD_INCORRECT_ERROR, 
	TEXT_CHANGE_PASSWORD_NEW_LENGTH_ERROR, 
	TEXT_CHANGE_PASSWORD_SUCCESSFUL_TIP, 
	TEXT_CHANGE_PASSWORD_TITLE, 
	TEXT_CHANGE_PASSWORD_OLD_LABEL, 
	TEXT_CHANGE_PASSWORD_OLD_TOOLTIP, 
	TEXT_CHANGE_PASSWORD_NEW_LABEL, 
	TEXT_CHANGE_PASSWORD_NEW_TOOLTIP, 
	TEXT_CHANGE_PASSWORD_NEW_AGAIN_LABEL, 
	TEXT_CHANGE_PASSWORD_NEW_AGAIN_TOOLTIP, 
	TEXT_CHANGE_PASSWORD_ORIGINAL_TIP, 
	//Change password
	TEXT_CHANGE_PASSWORD_CONF_DATA_CURRUPT_TIP, 
	TEXT_CHANGE_PASSWORD_HAVE_NOT_SETTING_PWD_TIP, 
	TEXT_CHANGE_PASSWORD_USER_MANAGE_TIP, 
	TEXT_CHANGE_PASSWORD_SET_PWD_TITLE, 

	//Account management dialog
	TEXT_ACCOUNT_MANAGEMENT_TITLE, 
	TEXT_ACCOUNT_MANAGEMENT_OLD_PWD, 
	TEXT_ACCOUNT_MANAGEMENT_NEW_PWD, 
	TEXT_ACCOUNT_MANAGEMENT_NEW_PWD_AGAIN, 

	TEXT_ACCOUNT_MANAGEMENT_NOTICE, 

	//Anti arp dialog
	TEXT_ANTI_ARP_OPEN, 
	TEXT_ANTI_ARP_CLOSE, 

	TEXT_ANTI_ARP_NOTICE, 
	TEXT_ANTI_ARP_HOST_MAC_LIST, 
	TEXT_ANTI_ARP_NEED_REBOOT_TIP, 

	//Background work
	TEXT_BK_WORK_BITSAFE_NEED_INSTALL_TIP, 

	//Work mode dialog
	TEXT_WORK_MODE_TITLE, 
	TEXT_WORK_MODE_CHANGE_TOOLTIP, 
	TEXT_WORK_MODE_LABLE, 
	TEXT_WORK_MODE_FREE, 
	TEXT_WORK_MODE_ACL_CTRL, 
	TEXT_WORK_MODE_BLOCK_ALL,

	//Change work mode
	TEXT_CHANGE_WORK_MODE_FAILED_TIP, 
	TEXT_CHANGE_WORK_MODE_SUCCESSFULLY_TIP, 

	//Response action type
	TEXT_RESPONSE_BLOCK, 
	TEXT_RESPONSE_ALLOW, 
	TEXT_RESPONSE_LEARN, 

	//System action response
	TEXT_ACTION_RESPONSE_TRUST_APP, 
	TEXT_ACTION_RESPONSE_RECORD_ACTION_TYPE, 
	TEXT_ACTION_RESPONSE_RECORD_ACTION, 

	//System action type
	TEXT_SYS_ACTION_LOAD_DRIVER_DESC, 
	TEXT_SYS_ACTION_INSTALL_COM_DESC, 
	TEXT_SYS_ACTION_MODIFY_REGISTRY_DESC, 
	TEXT_SYS_ACTION_MODIFY_FILE_DESC, 
	TEXT_SYS_ACTION_DELETE_FILE_DESC, 
	TEXT_SYS_ACTION_LOCATE_TO_URL_DESC, 
	TEXT_SYS_ACTION_INSTALL_HOOK_DESC, 
	TEXT_SYS_ACTION_CREATE_PROCESS_DESC, 
	TEXT_SYS_ACTION_ACCESS_REMOTE_MEMORY_DESC, 
	TEXT_SYS_ACTION_OTHER_PROCTECTED_ACTION, 
	TEXT_SYS_ACTION_CREATE_CONNECTION_DESC, 
	TEXT_SYS_ACTION_SEND_DATA_DESC, 
	TEXT_SYS_ACTION_RECEIVE_DATA_DESC, 

	//System action dialog
	TEXT_SYS_ACTION_ALLOW_TIP, 
	TEXT_SYS_ACTION_LOAD_DRIVER, 
	TEXT_SYS_ACTION_INSTALL_SYS_HOOK, 
	TEXT_SYS_ACTION_CREATE_PROCESS, 
	TEXT_SYS_ACTION_ACCESS_REMOTE_MEMORY, 
	TEXT_SYS_ACTION_INSTALL_COM, 
	TEXT_SYS_ACTION_MODIFY_REGISTRY, 
	TEXT_SYS_ACTION_MODIFY_FILE, 
	TEXT_SYS_ACTION_DELETE_FILE, 
	TEXT_SYS_ACTION_COMMUCATE_TO_OTHER_PROC, 
	TEXT_SYS_ACTION_CREATE_TCP_SOCKET, 
	TEXT_SYS_ACTION_CREATE_UDP_SOCKET, 
	TEXT_SYS_ACTION_SEND_DATA, 
	TEXT_SYS_ACTION_RECEIVE_DATA, 
	TEXT_SYS_ACTION_PROCTECTED_ACTION, 

	//Network action log dialog
	TEXT_FIREWALL_LOG_PROC_ID, 
	TEXT_FIREWALL_LOG_PROC_NAME, 
	TEXT_FIREWALL_LOG_ACTION, 
	TEXT_FIREWALL_LOG_PROTOCOL, 
	TEXT_FIREWALL_LOG_DEST_IP, 
	TEXT_FIREWALL_LOG_DEST_PORT, 
	TEXT_FIREWALL_LOG_SOURCE_IP, 
	TEXT_FIREWALL_LOG_SOURCE_PORT, 
	TEXT_FIREWALL_LOG_RESPONSE, 
	TEXT_FIREWALL_LOG_DESCRIPTION, 
	TEXT_FIREWALL_LOG_TIME, 

	//add trust application
	TEXT_TRUST_APP_ADD, 
	TEXT_TRUST_APP_MODULE_FILE_NAME, 
	TEXT_TRUST_APP_BROWSER_FILE, 
	TEXT_TRUST_APP_ADD_ERROR_TIP, 
	TEXT_TRUST_APP_ADD_SUCCESSFULLY_TIP, 

	//add block application
	TEXT_DISTRUST_APP_ADD, 

	//url rule tab
	TEXT_URL_RULE_APP_NAME, 
	TEXT_URL_RULE_URL, 
	TEXT_URL_RULE_ACTION, 
	TEXT_URL_RULE_DESCRIPTION, 

	TEXT_RULE_CTRL_BUTTON_ADD, 
	TEXT_RULE_CTRL_BUTTON_EDIT, 
	TEXT_RULE_CTRL_BUTTON_DELETE, 

	//ip rule tab
	TEXT_IP_RULE_SOURCE_IP_BEGIN, 
	TEXT_IP_RULE_SOURCE_IP_END, 
	TEXT_IP_RULE_SOURCE_PORT_BEGIN, 
	TEXT_IP_RULE_SOURCE_PORT_END, 

	TEXT_IP_RULE_DEST_IP_BEGIN, 
	TEXT_IP_RULE_DEST_IP_END, 
	TEXT_IP_RULE_DEST_PORT_BEGIN, 
	TEXT_IP_RULE_DEST_PORT_END, 

	TEXT_IP_RULE_PROTO,  

	//name edit dialog
	TEXT_NAME_EDIT_OPERA, 
	TEXT_NAME_EDIT_DESC, 
	TEXT_NAME_EDIT_APP, 
	TEXT_NAME_EDIT_URL_RULE_TITLE, 
	TEXT_NAME_EDIT_URL_NAME, 
	TEXT_NAME_EDIT_FILE_RULE_TITLE, 
	TEXT_NAME_EDIT_FILE_NAME, 
	TEXT_NAME_EDIT_REG_RULE_TITLE, 
	TEXT_NAME_EDIT_REG_NAME, 

	//IP edit dialog
	TEXT_IP_EDIT_OPERA, 
	TEXT_IP_EDIT_APP, 
	TEXT_IP_EDIT_DESC, 
	TEXT_IP_EDIT_PROTOCOL, 
	TEXT_IP_EDIT_DEST_PORT, 
	TEXT_IP_EDIT_SOURCE_PORT, 
	TEXT_IP_EDIT_DEST_IP, 
	TEXT_IP_EDIT_SOURCE_IP, 
	TEXT_IP_EDIT_PROT_TCP, 
	TEXT_IP_EDIT_PROT_UDP, 
	TEXT_IP_EDIT_PROT_ALL, 
	TEXT_IP_EDIT_PROT_SEL_TIP, 

	//acl edit dialog
	TEXT_ACL_EDIT_DESC_LENGTH_ERROR, 
	TEXT_ACL_EDIT_APP_PATH_LENGTH_ERROR, 
	TEXT_ACL_EDIT_BEGIN_SOURCE_IP_FMT_ERROR, 
	TEXT_ACL_EDIT_END_SOURCE_IP_FMT_ERROR, 
	TEXT_ACL_EDIT_BEGIN_DESTINATION_IP_FMT_ERROR, 
	TEXT_ACL_EDIT_END_DESTINATION_IP_FMT_ERROR, 
	TEXT_ACL_EDIT_BEGIN_SOURCE_PORT_FMT_ERROR, 
	TEXT_ACL_EDIT_END_SOURCE_PORT_FMT_ERROR, 
	TEXT_ACL_EDIT_BEGIN_DESTINATION_PORT_FMT_ERROR, 
	TEXT_ACL_EDIT_END_DESTINATION_PORT_FMT_ERROR, 
	TEXT_ACL_EDIT_INPUT_ERROR, 
	TEXT_ACL_EDIT_SOURCE_IP_AND_DEST_IP_ALL_IS_NULL_ERROR, 
	TEXT_ACL_EDIT_SOURCE_PORT_AND_DEST_PORT_ALL_IS_NULL_ERROR, 
	TEXT_ACL_EDIT_ADD_SOCKET_RULE_SUCCESSFULLY, 

	//acl management
	TEXT_ACL_MANAGEMENT_ADD_SUCCESSULLY_TIP, 
	TEXT_ACL_MANAGEMENT_EDIT_SUCCESSULLY_TIP, 

	//icon menu 
	TEXT_ICON_MENU_OPEN, 
	TEXT_ICON_MENU_LOGIN, 
	TEXT_ICON_MENU_WORK_MODE, 
	TEXT_ICON_MENU_EXIT, 

	//icmp ping dialog
	TEXT_ICMP_TITLE, 
	TEXT_ICMP_RULE_NOTICE, 
	TEXT_ICMP_RULE_BLOCK_PING, 
	TEXT_ICMP_RULE_ALLOW_PING, 
	TEXT_ICMP_RULE_ALLOW_PING_FAILED_TIP, 
	TEXT_ICMP_RULE_ALLOW_PING_SUCCESSFULLY, 
	TEXT_ICMP_RULE_BLOCK_PING_FAILED_TIP, 
	TEXT_ICMP_RULE_BLOCK_PING_SUCCESSFULLY, 

	//All network connection dialog
	TEXT_NETWORK_CONN_PROC_ID, 
	TEXT_NETWORK_CONN_MODULE_FILE, 
	TEXT_NETWORK_CONN_ALL_UPLOAD, 
	TEXT_NETWORK_CONN_ALL_DWONLOAD, 
	TEXT_NETWORK_CONN_BLOCK_STATUS, 
	TEXT_NETWORK_CONN_UPLOAD_SPEED, 
	TEXT_NETWORK_CONN_UPLOAD_SPEED_LIMIT, 
	TEXT_NETWORK_CONN_DOWNLOAD_SPEED, 
	TEXT_NETWORK_CONN_BLOCK_UP, 
	TEXT_NETWORK_CONN_BLOCK_DOWN, 
	TEXT_NETWORK_CONN_ALLOW_UP, 
	TEXT_NETWORK_CONN_ALLOW_DOWN, 
	TEXT_NETWORK_CONN_UPLOAD_SPEED_CTRL, 
	TEXT_NETWORK_CONN_SELECT_TARGET_TIP, 
	//TEXT_NETWORK_CONN_TCP_PROT, 
	//TEXT_NETWORK_CONN_UDP_PROT, 
	//TEXT_NETWORK_CONN_ALL_PROT, 

	//All strings in defense panel
	//Defense log dialog
	TEXT_DEFENSE_LOG_APP_NAME, 
	TEXT_DEFENSE_LOG_ACTION, 
	TEXT_DEFENSE_LOG_TARGET, 
	TEXT_DEFENSE_LOG_RESPONSE, 
	TEXT_DEFENSE_LOG_TIME, 

	//All running process panel
	TEXT_RUNNING_PROC_PROC_ID, 
	TEXT_RUNNING_PROC_PROC_NAME, 
	TEXT_RUNNING_PROC_DESCRIPTION, 

	TEXT_RUNNING_PROC_ADD_TRUST, 
	TEXT_RUNNING_PROC_ADD_BLOCK, 

	//File system rule panel
	TEXT_FILE_RULE_APP_NAME, 
	TEXT_FILE_RULE_FILE_PATH, 
	TEXT_FILE_RULE_ACTION, 
	TEXT_FILE_RULE_DESCRIPTION, 

	//Registry rule panel
	//__t( "Application name" ), 
	TEXT_REG_RULE_REG_PATH, 
	//__t( "Action" ), 
	//__t( "Description" ), 


	//rule configuration dialog 
	TEXT_CONFIG_DIALOG_NO_SELECT_TIP, 
	TEXT_CONFIG_DIALOG_DONT_DELETE_SUPER_TIP, 
	TEXT_CONFIG_DIALOG_DELETE_SUCCESS_TIP, 

	//Get defense power
	TEXT_GET_DEFENSE_POWER_NOTICE, 
	TEXT_GET_DEFENSE_POWER_FROM_OTHER, 
	TEXT_GET_DEFENSE_POWER_ACQUIRE, 
	TEXT_GET_DEFENSE_POWER_RELEASE, 

	//Network control panel management
	TEXT_NETWORK_CTRL_PANEL_TITLE, 
	TEXT_NETWORK_CTRL_PANEL_CLOSE, 
	TEXT_NETWORK_CTRL_PANEL_OPEN, 
	TEXT_NETWORK_CTRL_PANEL_NOTICE, 
	TEXT_NETWORK_CTRL_PANEL_OPEN_FAILED, 
	TEXT_NETWORK_CTRL_PANEL_OPEN_SUCCESSFULLY, 
	TEXT_NETWORK_CTRL_PANEL_CLOSE_FAILED, 
	TEXT_NETWORK_CTRL_PANEL_CLOSE_SUCCESSFULLY, 

	//Relearn message
	TEXT_RELEARN_TITLE, 
	TEXT_RELEARN_TIP_MSG, 

	//Erc dialog
	TEXT_ERC_TITLE, 
	TEXT_ERC_OPERATION, 
	TEXT_ERC_OPERATION_BLOCK, 
	TEXT_ERC_HOLD_TIME, 
	TEXT_ERC_TIME_HOUR, 
	TEXT_ERC_TIME_MINUTE, 
	TEXT_ERC_SEND_CMD_BUTTON, 
	TEXT_ERC_EMAIL_ACCOUNT, 
	TEXT_ERC_OPEN_LOCAL_ERC_SERVICE, 
	TEXT_ERC_CLOSE_LOCAL_ERC_SERVICE, 
	TEXT_ERC_ACCOUNT_SETTING_NOTICE, 
	TEXT_ERC_REMOTE_COMMAND_COMBO_TIP, 
	TEXT_ERC_REMOTE_COMMAND_COMBO_TEXT, 

	//Email account setting dialog
	TEXT_ERC_ACCOUNT_SETTING_DISPLAY_NAME, 
	TEXT_ERC_ACCOUNT_SETTING_DISPLAY_NAME_TIP, 
	TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT, 
	TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_TIP, 
	TEXT_ERC_ACCOUNT_SETTING_PASSWORD, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_IP, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_IP_TIP, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_PORT, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_PORT_TIP, 
	TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_IP, 
	TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_PORT, 
	TEXT_ERC_ACCOUNT_SETTING_TEST, 
	TEXT_ERC_ACCOUNT_SETTING_CONFIRM, 
	TEXT_ERC_ACCOUNT_SETTING_NOTICE_TEXT, 
	TEXT_ERC_ACCOUNT_SETTING_DISPLAY_NAME_LENGTH_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_LENGTH_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_FORMAT_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_EMAIL_ACCOUNT_MUST_SET, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_LENGTH_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_SERVER_MUST_SET, 
	TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_LENGTH_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_POP3_SERVER_MUST_SET, 
	TEXT_ERC_ACCOUNT_SETTING_PASSWORD_LENGTH_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_NO_NEED_PASSWORD_TIP, 
	TEXT_ERC_ACCOUNT_SETTING_SMTP_PORT_FMT_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_POP3_PORT_FMT_ERROR, 
	TEXT_ERC_ACCOUNT_SETTING_TEST_EMAIL_ACCOUNT_SUCCESSFULLY, 
	TEXT_ERC_ACCOUNT_SETTING_TEST_EMAIL_ACCOUNT_FAILED, 
	TEXT_ERC_ACCOUNT_SETTING_RECEIVE_ALLOW_COMMAND, 
	TEXT_ERC_ACCOUNT_SETTING_RECEIVE_BLOCK_COMMAND, 
	TEXT_ERC_ACCOUNT_SETTING_RECEIVE_ACL_COMMAND, 
	TEXT_ERC_ACCOUNT_SETTING_COMMAND_HOLD_TIME, 
	TEXT_ERC_ACCOUNT_SETTING_SEND_COMMAND_SUCCESSFULLY, 
	TEXT_ERC_ACCOUNT_SETTING_SEND_COMMAND_FAILED, 
	TEXT_ERC_ACCOUNT_SETTING_SEND_COMMAND_NOT_IN_ORDER, 
	TEXT_ERC_ACCOUNT_SETTING_LOAD_ERC_CONFIG_FAILED, 
	TEXT_ERC_ACCOUNT_SETTING_SUCCESSFULLY, 
	TEXT_ERC_ACCOUNT_SETTING_TITLE, 
	TEXT_ERC_ACCOUNT_SETTING_TIME_MUST_BE_SET, 
	TEXT_ERC_ACCOUNT_SETTING_SETTING_NOW, 

	//ERC notice dialog
	TEXT_ERC_NOTICE_CONTENT, 

	//About dialog
	TEXT_ABOUT_SOFTWARE_NAME, 
	TEXT_ABOUT_BALANCESFOT_COPYRIGHT, 
	TEXT_ABOUT_ALL_RIGHT_RESERVED, 
	TEXT_ABOUT_LICENSE, 

	//Language dialog
	TEXT_LANG_TITLE, 
	TEXT_LANG_TIP_LABEL, 
	TEXT_LANG_SELECT_TOOLTIP, 
	TEXT_LANG_SET_FAILED, 
	TEXT_LANG_SET_SUCCESSFULLY, 
	TEXT_LANG_LOAD_LANG_CONFIG_ERROR, 
	TEXT_LANG_LANG_FILE_CURRPTED, 
	TEXT_LANG_HAVE_NOT_SET_LANG_NOW, 
	//TEXT_LANG_TITLE, 
	//TEXT_LANG_SET_LANG_SUCCESSFULLY, 
	//TEXT_LANG_SET_LANG_FAILED, 

	//update dialog
	TEXT_UPDATE_TITLE, 
	TEXT_UPDATE_FILE_DOWNLOAD_TIP_FMT, 
	TEXT_UPDATE_UPDATE_FAILED, 
	TEXT_UPDATE_UPDATE_SUCCESSFUL, 
	TEXT_UPDATE_CONFIGURATION_FILE_CORRUPTED, 
	TEXT_UPDATE_GET_UPDATE_INFO_FAILED, 
	TEXT_UPDATE_UPDATE_SERVER_MAINTAINING, 
	TEXT_UPDATE_UPDATE_SERVER_CONFIG_ERROR, 
	TEXT_UPDATE_ALREADY_NEWEST_VERSION, 
	TEXT_UPDATE_OLD_VERSION_HAVE_NOT_UNINSTALL, 
	TEXT_UPDATE_UPDATING_FILE_NO_TIP, 
	TEXT_UPDATE_ALREADY_NEWEST_VERSION_TIP, 
	TEXT_UPDATE_NEWER_VERSION_CHECKED, 
	TEXT_UPDATE_ALL_FILE_COUNT_TIP, 

	//suggestion dialog 
	TEXT_SUGGESTION_TIP_TITLE, 
	TEXT_SUGGESTION_TIP, 
	TEXT_SUGGESTION_SEND, 
	TEXT_SUGGESTION_SEND_SUCCESSFULLY, 

	//System action tip dialog
	TEXT_SYS_ACTION_TIP_SPECIOUS_ACTION, 

	//Uninstallation
	TEXT_UNINSTALL_NEED_RESTART, 
	TEXT_UNISNTALL_FAILED,  

	//Installation
	TEXT_INSTALL_NEED_RESTART, 
	TEXT_INSTALL_MUST_RESTART, 
	TEXT_INSTALL_COMPATIBILITY_ERROR, 
	TEXT_INSTALL_RETRY_EXEC_APP, 
	TEXT_INSTALL_REINSTALL, 

	//Help me dialog
	TEXT_HELP_TIP, 
	TEXT_HELP_NOTICE, 

	//Skin customizing dialog
	TEXT_SKIN_CUSTOM_TRANPARENT, 
	TEXT_SKIN_THEME_CHANGE, 

	//Run-time log dialog
	TEXT_RUN_TIME_LOG_TITLE, 
	TEXT_RUN_TIME_LOG_COMMON_MSG, 
	TEXT_RUN_TIME_LOG_FILE_MSG, 
	TEXT_RUN_TIME_LOG_REG_MSG, 
	TEXT_RUN_TIME_LOG_NETWORK_MSG, 

	//bitsafe setting
	TEXT_BITSAFE_SETTING_AUTORUN, 

	//option dialog
	TEXT_OPTION_DIALOG_TITLE, 
	TEXT_OPTION_DIALOG_UI_TAB, 
	TEXT_OPTION_DIALOG_SECURITY_TAB, 

	//security option dialog
	TEXT_SECURITY_TAB_URL_FIREWALL, 
	TEXT_SECURITY_TAB_ARP_FIREWALL, 
	TEXT_SECURITY_TAB_BASE_FIREWALL, 
	TEXT_SECURITY_TAB_TRFFIC_FIREWALL, 
	TEXT_SECURITY_TAB_DEFENSE_FIREWALL, 
	TEXT_SECURITY_TAB_FILE_SHIELD, 
	TEXT_SECURITY_TAB_REG_SHIELD, 

	TEXT_SECURITY_TAB_URL_FIREWALL_DESC, 
	TEXT_SECURITY_TAB_ARP_FIREWALL_DESC, 
	TEXT_SECURITY_TAB_BASE_FIREWALL_DESC, 
	TEXT_SECURITY_TAB_TRFFIC_FIREWALL_DESC, 
	TEXT_SECURITY_TAB_DEFENSE_FIREWALL_DESC, 
	TEXT_SECURITY_TAB_FILE_SHIELD_DESC, 
	TEXT_SECURITY_TAB_REG_SHIELD_DESC, 

	TEXT_SECURITY_TAB_OPENED_BTN, 
	TEXT_SECURITY_TAB_CLOSED_BTN, 

	//action information box
	TEXT_ACTION_INFO_DATA, 

	MAX_TEXT_ID
} ALL_LANG_STRINGS_ID, *PALL_LANG_STRINGS_ID;

#define is_valid_string_id( id ) ( id >= 0 && id < MAX_TEXT_ID )

extern pcchar_t* all_strings_sel; 

#include "lang_en.h"
#include "lang_cn.h"

#define DEBUG_LANG_STRING

FORCEINLINE pcchar_t _get_string_by_id( ALL_LANG_STRINGS_ID id, LPCTSTR def_string )
{
	pcchar_t text_ret = NULL; 

	if( all_strings_sel == NULL )
	{
		text_ret = def_string; 
		goto _return; 
	}

#ifdef DEBUG_LANG_STRING
	ASSERT( MAX_TEXT_ID + 1 == all_strings_en_count ); 
	ASSERT( 0 == wcscmp( all_strings_en[ MAX_TEXT_ID ], LAST_EN_STRING ) ); 
#endif //DEBUG_LANG_STRING

	if( FALSE == is_valid_string_id( id ) )
	{
		text_ret = def_string; 
		goto _return; 
	}

#define CN_LANG_TEXT_END_INDEX TEXT_BITSAFE_UPDATING_TIP

#ifdef DEBUG_LANG_STRING
	ASSERT( ( CN_LANG_TEXT_END_INDEX + 1 )== all_strings_cn_count ); 
	ASSERT( 0 == wcscmp( all_strings_cn[ CN_LANG_TEXT_END_INDEX ], LAST_CN_STRING ) ); 
#endif //DEBUG_LANG_STRING

	 
	if( all_strings_sel == all_strings_cn )
	{
		if( id > CN_LANG_TEXT_END_INDEX )
		{
			text_ret = def_string; 
			goto _return; 			
		}
	}

	text_ret = all_strings_sel[ id ]; 

_return:

	return text_ret; 
}

FORCEINLINE pcchar_t get_string_by_id( ALL_LANG_STRINGS_ID id )
{

//_return:
	return _get_string_by_id( id, NULL ); 
}


LRESULT set_cur_lang( LANG_ID id ); 

//INLINE pcchar_t get_lang_string( char_t *txt )
//{
//	return ( char_t* )( ( PBYTE )txt + ( lang_id * ALL_STRINGS_SIZE ) ); 
//}

LANG_ID get_cur_lang_id(); 

#endif //__LOCAL_STRINGS_H__