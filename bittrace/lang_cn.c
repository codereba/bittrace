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

#include "common_func.h"
#include "lang_cn.h"
#include "local_strings.h"

pcchar_t all_strings_cn[] = {
	//Main button and title
	__t( "����" ), 
	__t( "����"), 
		__t( "����" ), 
		__t( "����" ), 
		__t( "���ذ�ȫ" ), 
		__t( "���ذ�ȫ" ), 
		__t( "����(&H)"), 
		__t( "���ذ�ȫ�Ѿ�����" ), 

		//Security logo descrition
		__t( "�๦�ܱ����Ѿ�����,ϵͳ���ܱ���״̬" ), 
		__t( "�๦�ܱ�����û�п���,ϵͳ�ڲ���ȫ״̬" ), 
		__t( "�򿪶�㱣��"), 

		//Help panel
		__t( "����" ), 
		__t( "���ذ�ȫ��̳" ), 
		__t( "���ڱ��ذ�ȫ" ), 
		__t( "���±��ذ�ȫ (��ע�����籣������)" ), 
		__t( "�������" ), 
		__t( "ͨ���ʼ�����Զ�̿���" ), 
		__t( "����ѧϰ����" ), 
		__t( "ѡ��" ), 
		//__t( "����" ), 
		__t( "�������" ), 
		__t( "���һ��,֧������" ), 
		__t( "ж��" ), 

		//Defense panel
		__t( "�ر�/��������������" ), 
		__t( "�������еĳ���" ), 
		__t( "�ӹ�ϵͳ��������(������360)" ), 
		__t( "����������ȫ����" ), 
		__t( "�����ε��ļ�" ), 
		__t( "δ֪���ļ�" ), 
		__t( "����������־" ), 

		//Firewall panel
		__t( "����ǽ��־" ), 
		__t( "���ӱ����εĳ���" ), 
		__t( "���ӱ����Ƶĳ���" ), 
		__t( "����ǽ����" ), 
		__t( "��ֹ/����ICMP PING" ), 
		__t( "������������" ), 
		__t( "ARP����ǽ" ), 
		__t( "�ҳ�����" ), 
		__t( "�ı乤��ģʽ" ), 

		//Summary panel
		__t( "����ǽ" ), 
		__t( "����ǽ����ֹ" ), 
		__t( "�����綯��" ), 
		__t( "���ϴ�" ), 
		__t( "�ֽ���������" ), 
		__t( "������ " ), 
		__t( "�ֽ���������" ), 

		__t( "��������" ), 
		__t( "������������ֹ" ), 
		__t( "��ϵͳ����" ), 
		__t( "����ϵͳ��ǰ��Ϊ" ), 

		//slogan
		__t( "�´β�����ʾ?" ), 
		__t( "�����ذ�ȫ��������ȫ����ͬʱʹ��ʱ��һ��ʱ���ϵͳ���п��ܻ���������������ϵͳ���󡣿��ǵ�������ȫ�����ľ����͹������Եȶ෽�����أ����ذ�ȫ��һ�������ƽ����ֱ������ܣ���������ȫ������ȫ����." ), 
		__t( "��1BITδ֪�������ɱ���������?����֪ͨ��.��1BITδ֪��������������������?��������鿴.��ͨ��email:codereba@gmail.com����̳�ϱ�." ), 
		__t( "���ʹ��2.0.0.288֮ǰ�汾,����������,���Ȳ�Ҫ�����װ��ʾ��ȷ�ϰ�ť���а�װ,�����Ƚ���ж�غ�,Ȼ������WINDOWS,������,��Ȼ�������ڰ汾����,����ϵͳ��������ɵĲ������½�." ), 
		
		//__t( "Current user don't have the privilege to do this action, please login first." ), 

		////Common button text
		//__t( "OK" ), 
		//__t( "Cancel" ), 

		//__t( "Yes" ), 
		//__t( "No" ), 

		//__t( "Confirm" ), 
		//__t( "Cancel" ), 

		////User login text
		//__t( "Login" ), 
		//__t( "Login" ), 
		//__t( "Password" ), 
		//__t( "Clike me to login" ), 
		//__t( "Change password" ), 
		//__t( "Cliek me to cancel" ), 
		//__t( "Passwork is incorrect"), 
		//__t( "Login successfully, you are the administrator now" ), 

		////Change password dialog
		//__t( "Two new password input is not same."), 
		//__t( "Old password is incorrect."), 
		//__t( "Password length need greater than 8." ), 
		//__t( "Change paswword successfully"), 
		//__t( "Password management"), 
		//__t( "Old password:"), 
		//__t( "Input old password here" ), 
		//__t( "New password:"), 
		//__t( "Input new password here" ), 
		//__t( "Again:" ), 
		//__t( "Input new password again" ), 
		//__t( "Note: Original password is null,just don't input anything." ), 

		////Account management dialog
		//__t( "Account management" ), 
		//__t( "Old password" ), 
		//__t( "New password" ), 
		//__t( "Again" ), 

		//__t( "Notice:Original password is null,just don't input anything" ), 

		////Anti arp dialog
		//__t( "Open anti-arp" ), 
		//__t( "Close anti-arp" ), 

		//__t( "Notice:Arti-arp need system performence, don't use it, if don't need." ), 
		//_T( "Responsed mac adresses by arp for host %d.%d.%d.%d: \n" ), 

		////Work mode dialog
		//__t( "Work mode setting" ), 
		//__t( "Click me select the mode you want" ), 
		//__t( "Work mode" ), 
		//__t( "Free mode" ), 
		//__t( "Control mode" ), 
		//__t( "Block all mode" ), 

		////Response action type
		//__t( "Block" ), 
		//__t( "Allow" ), 
		//__t( "Learn" ), 

		////Network action log dialog
		//__t( "Process id" ), 
		//__t( "Process name" ), 
		//__t( "Action" ), 
		//__t( "Protocol" ), 
		//__t( "Dest (Ip or Url)" ), 
		//__t( "Dest port" ), 
		//__t( "Source ip" ), 
		//__t( "Source port" ), 
		//__t( "Response" ), 
		//__t( "Description" ), 
		//__t( "Time" ), 

		////add trust application
		//__t( "Add trust application" ), 
		//__t( "Application executable file name:" ), 
		//__t( "Browser" ), 

		////add block application
		//__t( "Add block application" ), 

		////url rule tab
		//__t( "Application name" ), 
		//__t( "Url" ), 
		//__t( "Action" ), 
		//__t( "Description" ), 
		//__t( "Add" ), 
		//__t( "Edit" ), 
		//__t( "Delete" ), 

		////ip rule tab
		//__t( "Source IP begin" ), 
		//__t( "Source IP end" ), 
		//__t( "Source port begin" ), 
		//__t( "Source port end" ), 

		//__t( "Dest IP begin" ), 
		//__t( "Dest IP end" ), 
		//__t( "Dest port begin" ), 
		//__t( "Dest port end" ), 

		//__t( "Protocol" ), 

		////name edit dialog
		//__t( "Operation:" ), 
		//__t( "Description:" ), 
		//__t( "Application: " ), 

		//__t( "Url rule definition" ), 
		//__t( "Url or demain name" ), 
		//__t( "File rule definition" ), 
		//__t( "File path"), 
		//__t( "Registry rule definition" ), 
		//__t( "Registry path" ), 

		////IP rule edit dialog
		//__t( "Operation:" ), 
		//__t( "Application:" ), 
		//__t( "Description:" ), 
		//__t( "Protocol:" ), 
		//__t( "Dest port:" ), 
		//__t( "Source port:" ), 
		//__t( "Dest IP:" ), 
		//__t( "Source IP:" ), 
		//__t( "TCP" ), 
		//__t( "UDP" ), 
		//__t( "All" ), 
		//__t( "Select protocol" ), 

		////icon menu 
		//__t( "Open" ), 
		//__t( "Login" ), 
		//__t( "Work mode" ), 
		//__t( "Exit" ), 

		////icmp ping dialog
		//__t( "ICMP setting" ), 
		//__t( "Notice: Other host can't ping this host when you turn on the switch,and vice verse" ), 
		//__t( "Block Ping" ), 
		//__t( "Allow Ping" ), 

		////All network connection dialog
		//__t( "Process ID" ), 
		//__t( "Executable file path" ), 
		//__t( "All uploaded" ), 
		//__t( "All downloaded" ), 
		//__t( "Block status" ), 
		//__t( "Upload speed" ), 
		//__t( "Upload speed limit" ), 
		//__t( "download speed" ), 
		//__t( "Block" ), 
		//__t( "Allow" ), 
		//__t( "Up speed" ), 

		////All strings in defense panel
		////Defense log dialog
		//__t( "Application" ), 
		//__t( "Action" ), 
		//__t( "Target" ), 
		//__t( "Response" ), 
		//__t( "Time" ), 

		////All running process panel
		//__t( "Process id" ), 
		//__t( "Process name" ), 
		//__t( "Description" ), 

		//__t( "Add as trusted" ), 
		//__t( "Add as distrusted" ), 

		////File system rule panel
		//__t( "Application name" ), 
		//__t( "File/Directory path" ), 
		//__t( "Action" ), 
		//__t( "Description" ), 

		////Registry rule panel
		////__t( "Application name" ), 
		//__t( "Registry key path" ), 
		////__t( "Action" ), 
		////__t( "Description" ), 

		////rule configuration dialog 
		//__t( "Select one rule first" ), 
		//__t( "Default rule is important for security of the system, deleting it will introduce security vulnerability." ), 
		//__t( "Delete this rule successfully" ), 

		////Get defense power
		//__t( "Notice: Recommended to use other security application which have more function for defense.like safe360.This function need reboot" ), 
		//__t( "Get defense power from other application" ), 
		//__t( "Acquire" ), 
		//__t( "Release" ), 

		////Network control panel management
		//__t( "Switch network control panel" ), 
		//__t( "Close panel" ), 
		//__t( "Open panel" ), 
		//__t( "Will close the network control panel for the security of the network configuration, Notice:Because system enviroment,sometime open the control panel need system restart." ), 

		////Relearn message
		//__t( "Relearn acl" ), 
		//__t( "Relearning control successfully" ), 

		////Erc dialog
		//__t( "Email remote control ( ERC )" ), 
		//__t( "Operation" ), 
		//__t( "Block all commuication" ), 
		//__t( "Hold time:" ), 
		//__t( "Hour " ), 
		//__t( "Minute" ), 
		//__t( "Send command" ), 
		//__t( "Email account setting" ), 
		//__t( "Open localhost ERC service" ), 
		//__t( "Close localhost ERC service" ), 

		////Email account setting dialog
		//__t( "Notice:Please read me first" ),
		//__t( "Display name ( Just for display ) " ), 
		//__t( "Email account ( e.g.john@sohu.com ) " ), 
		//__t( "Smtp server name/ip (e.g.smtp.sohu.com )" ), 
		//__t( "Smtp server port ( Default is 25, please comfirm before setting" ), 
		//__t( "Pop3 server name/ip (e.g.pop.sohu.com )" ), 
		//__t( "Pop3 server port ( Default is 110, please comfire before setting" ), 
		//__t( "Test" ), 
		//__t( "Comfirm" ), 
		//__t( "sohu.com smtp server and pop3 server have tested,is recommended,and remote control email will deleting automatic,notice don't use the \"REMOTE COMMAND\" to be the starting text on the email content" ), 

		////ERC notice dialog
		//__t( "Please notice: \n" ) \
		//__t( "1.Remote control implemented by email,client send the command be encapsulated by email, and server receive and execute it.\n" ) \
		//__t( "2.Remote control is depend on the smtp and pop3 server, so the delay time can't exactly sure.general serveral minutes\n" ) \
		//__t( "3.Sometime smtp server or pope3 server can't receive command, please try again.\n" ) \
		//__t( "4.Recover acl mode when command time is end.\n" ) \
		//__t( "5.If you send block all command, all communication will closed, include communication for remote control\n" ) \
		//__t( "6.Notice: Don't let the stmpt server and the pop3 server conflict with the url rule.\n" ) \
		//__t( "7.sohu.com email is tested\n" ) \
		//__t( "8.Remote control email will deleting automatic,notice don't use the \"REMOTE COMMAND\" to be the starting text on the email content, recommend independent email account\n" ) \
		//__t( "9.One remote control command will be stopped by later received command,so don't send too more time( recommend 2 time ).\n" ) \
		//__t( "10.You can check the remote control command email in your account,if it treated wrong,correct it\n" ) \
		//__t( "11.We develope free software, please support us, we will develpe more enhanced functions\n" ) \
		//__t( "\n") \
		//__t( "\nContact us by email:" ) BITSAFE_EMAIL __t( " or http://www.simpai.net/bbs\n" ), 

		////About dialog
		//__t( "codereba BitSafe(Sevenfw) V2.0" ), 
		//__t( "(C) 2009-2012 BalanceSoft" ), 
		//__t( "All right reserved." ), 
		//__t( "This software is protected by china and internaltional copyright and intellectural property laws, if reverse engine,crack, modify or use it for some commercial purpose without permit by balancesoft, will get the maximum punishment by law." ), 

		////Language dialog
		//__t( "Select language" ), 
		//__t( "Language:"), 
		//__t( "Click here to select the language" ), 
		//__t( "Set language failed\n" ), 
		//__t( "language seted\n" ), 

		////update dialog
		//__t( "File downloaded: %d/%d" ), 
		//__t( "Updating failed" ), 
		//__t( "Updating successful" ), 
		//__t( "BitSafe configuration file is cruppted, reinstall bitsafe can fix it." ), 
		//__t( "Get updating information failed, please check network,if that's correct, you download manually from:\"http://www.simpai.net/bitsafev2.zip\" wait server have maintained." ), 
		//__t( "Updating server is maintained, can't be updating now." ), 
		//__t( "Updating server configuration is error, please report it to administrator ( email:" ) BITSAFE_EMAIL __t( " bbs:codereba.com/bbs ), Thank you." )
		//__t( "You are using newest version, don't need to update." ), 
		//__t( "Old version have not uninstall successfully, uninstall it before you reinstall it and run it ( don't select yes when show message box for installation." ), 
		//__t( "Is updating %d file: %s" ), 
		//__t( "Already is newest version %s, don't need to update" ), 
		//__t( "Checked newer version %s (%s), update to it?" ), 
		//__t( "Updating started,have all %d files" )
}; 

#ifdef _DEBUG
ULONG all_strings_cn_count = ARRAY_SIZE( all_strings_cn ); 
#endif //_DEBUG
