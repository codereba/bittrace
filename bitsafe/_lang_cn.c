#include "common_func.h"
#include "lang_cn.h"
#include "local_strings.h"

pcchar_t all_strings_cn[] = {
	//Main button and title
	__t( "统计信息" ), 
		__t( "网络管理" ), 
		__t( "系统管理" ), 
		__t( "关于" ), 
		__t( "比特安全" ), 
		__t( "比特安全" ), 
		__t( "隐藏(&H)"), 
		__t( "比特安全已经启动" ), 

		//Security logo descrition
		__t( "多功能保护已经开启,系统在受保护状态" ), 
		__t( "多功能保护还没有开启,系统在不安全状态" ), 
		__t( "打开多层保护"), 

		//Help panel
		__t( "帮助" ), 
		__t( "比特安全论坛" ), 
		__t( "关于比特安全" ), 
		__t( "更新比特安全 (请注意网络保护设置)" ), 
		__t( "提出建议" ), 
		__t( "通过邮件进行远程控制" ), 
		__t( "重新学习规则" ), 
		__t( "选项" ), 
		//__t( "语言" ), 
		__t( "提出建议" ), 
		__t( "轻点一下,支持我们" ), 
		__t( "卸载" ), 

		//Defense panel
		__t( "关闭/开启网络控制面板" ), 
		__t( "所有运行的程序" ), 
		__t( "接管系统管理功能(适用于360)" ), 
		__t( "主动防御安全策略" ), 
		__t( "被信任的文件" ), 
		__t( "未知的文件" ), 
		__t( "主动防御日志" ), 

		//Firewall panel
		__t( "防火墙日志" ), 
		__t( "添加被信任的程序" ), 
		__t( "添加被限制的程序" ), 
		__t( "防火墙设置" ), 
		__t( "阻止/允许ICMP PING" ), 
		__t( "所有网络连接" ), 
		__t( "ARP防火墙" ), 
		__t( "家长管理" ), 
		__t( "改变工作模式" ), 

		//Summary panel
		__t( "防火墙" ), 
		__t( "防火墙共阻止" ), 
		__t( "次网络动作" ), 
		__t( "共上传" ), 
		__t( "字节网络流量" ), 
		__t( "共下载 " ), 
		__t( "字节网络流量" ), 

		__t( "主动防御" ), 
		__t( "主动防御共阻止" ), 
		__t( "次系统动作" ), 
		__t( "检视系统当前行为" ), 

		//slogan
		__t( "下次不再提示?" ), 
		__t( "当比特安全与其它安全软件同时使用时，一段时间后，系统运行可能会变慢，或出现其它系统错误。考虑到其它安全软件的竞争和功能特性等多方面因素，比特安全下一步将会移交部分保护功能，由其它安全软件完全处理." ), 
		__t( "有1BIT未知的数据由本程序运行?马上通知我.有1BIT未知的数据由其它程序运行?我来帮你查看.请通过email:codereba@gmail.com或论坛上报." ), 
		__t( "如果使用2.0.0.288之前版本,进行升级后,请先不要点击安装提示的确认按钮进行安装,而是先进行卸载后,然后重启WINDOWS,再运行,不然可能由于版本问题,导致系统出错。造成的不便请谅解." ), 
		
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
		//__t( "\nContact us by email:" ) BITSAFE_EMAIL __t( " or http://www.codereba.com/bbs\n" ), 

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
		//__t( "Get updating information failed, please check network,if that's correct, you download manually from:\"http://www.codereba.com/bitsafev2.zip\" wait server have maintained." ), 
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
