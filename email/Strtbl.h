/*
 * ERC
 *
 * Strtbl.h (EN)
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_STR_TBL_H
#define _INC_STR_TBL_H

// General
#ifdef _WIN32_WCE
#define STR_DEFAULT_FONT			TEXT("")
#else
#define STR_DEFAULT_FONT			TEXT("")
#endif
#define STR_DEFAULT_FONTCHARSET		DEFAULT_CHARSET

#define STR_DEFAULT_BURA			TEXT("")
#define STR_DEFAULT_OIDA			TEXT("")

#define STR_DEFAULT_HEAD_CHARSET	TEXT("")
#define STR_DEFAULT_HEAD_ENCODE		0		// 0-7bit 1-8bit 2-base64 3-quoted-printable
#define STR_DEFAULT_BODY_CHARSET	TEXT("")
#define STR_DEFAULT_BODY_ENCODE		0		// 0-7bit 1-8bit 2-base64 3-quoted-printable

#define STR_DEFAULT_DATEFORMAT		TEXT("MM/dd/yyyy")
#define STR_DEFAULT_TIMEFORMAT		TEXT("HH:mm")

#define STR_FILE_FILTER				TEXT("All Files (*.*)\0*.*\0\0")
#define STR_TEXT_FILTER				TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0")
#define STR_WAVE_FILTER				TEXT("Sound Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0\0")

// Error
#define STR_ERR_MEMALLOC			TEXT("Memory Allocation error")
#define STR_ERR_INIT				TEXT("Initialization error")
#define STR_ERR_OPEN				TEXT("Open File error")
#define STR_ERR_SAVEEND				TEXT("Save File error \nContinue?")
#define STR_ERR_SAVE				TEXT("Save File error")
#define STR_ERR_ADD					TEXT("Address not added")
#define STR_ERR_VIEW				TEXT("Display failed")
#define STR_ERR_SELECTMAILBOX		TEXT("No account specified")
#define STR_ERR_SELECTMAILADDR		TEXT("No address selected")
#define STR_ERR_SETMAILADDR			TEXT("No mail address set")
#define STR_ERR_INPUTMAILADDR		TEXT("No mail address entered")
#define STR_ERR_CREATECOPY			TEXT("Copy failed")
#define STR_ERR_SAVECOPY			TEXT("Copy to Savebox failed")
#define STR_ERR_NOITEM1				TEXT("Item 1 not set")
#define STR_ERR_INPUTFINDSTRING		TEXT("No 'Find' string entered")
#define STR_ERR_NOMAIL				TEXT("No Mail in the list")
#define STR_ERR_SENDLOCK			TEXT("Sending barred! Transmission in progress")

// Socket error
#define STR_ERR_SOCK_SELECT			TEXT("Selection error")
#define STR_ERR_SOCK_DISCONNECT		TEXT("Server disconnected")
#define STR_ERR_SOCK_CONNECT		TEXT("Connection failed")
#define STR_ERR_SOCK_SENDRECV		TEXT("Send/Recv error")
#define STR_ERR_SOCK_EVENT			TEXT("Event setting error")
#define STR_ERR_SOCK_NOSERVER		TEXT("Server name not set")
#define STR_ERR_SOCK_GETIPADDR		TEXT("Unable to find email server")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("Socket creation error")
#define STR_ERR_SOCK_TIMEOUT		TEXT("Connection timed-out")
#define STR_ERR_SOCK_SEND			TEXT("Send error")
#define STR_ERR_SOCK_RESPONSE		TEXT("Response not analysed\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("Item information not acquired")
#define STR_ERR_SOCK_MAILSYNC 		TEXT("Mail numbers not synchronised\n\n")\
									TEXT("New and existing numbers must agree")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Message-Id not acquired")
#define STR_ERR_SOCK_NOUSERID		TEXT("No username set")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("No password set")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("Username or password error\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("Account was not accepted\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("Server does not accept APOP")
#define STR_ERR_SOCK_STAT			TEXT("STAT failed\n\n")
#define STR_ERR_SOCK_TOP			TEXT("TOP failed\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR failed\n\n")
#define STR_ERR_SOCK_DELE			TEXT("Deletion failed\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("Attached file was not found")
#define STR_ERR_SOCK_BADFROM		TEXT("Address format error")
#define STR_ERR_SOCK_HELO 			TEXT("HELO failed\n")\
									TEXT("Please check address format\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("Login to SMTP server failed\n\n")
#define STR_ERR_SOCK_RSET			TEXT("RSET failed\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("MAIL FROM failed\n")\
									TEXT("Please check address format\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("Destination not set")
#define STR_ERR_SOCK_RCPTTO			TEXT("RCPT TO failed\n")\
									TEXT("Please check destination address\n\n")
#define STR_ERR_SOCK_DATA			TEXT("DATA failed\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Mail not sent\n\n")
#define STR_ERR_SOCK_SSL_INIT		TEXT("Initialization failed of SSL\n%s")
#define STR_ERR_SOCK_SSL_VERIFY		TEXT("Verify failed of SSL\n%s")
#define STR_ERR_SOCK_NOSSL			TEXT("Initialization failure of 'npopssl.dll'")
	
// Ras error
#define STR_ERR_RAS_NOSET			TEXT("No dial-up setting available")
#define STR_ERR_RAS_CONNECT			TEXT("Dial-up connection failed")
#define STR_ERR_RAS_DISCONNECT		TEXT("Dial-up was cut or line dropped")

// Question
#define STR_Q_DELETE				TEXT("Delete it?")
#define STR_Q_DELSERVERMAIL			TEXT("Warning!  This will delete mail from the server")
#define STR_Q_DELLISTMAIL			TEXT("Delete %d mail from the list?%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(Is not deleted from the server)")
#define STR_Q_DELMAILBOX			TEXT("Delete account?")
#define STR_Q_DELATTACH				TEXT("Delete attached files?")
#define STR_Q_COPY					TEXT("Copy %d mail to the Savebox?")
#define STR_Q_DEPENDENCE			TEXT("There is a character depending on the model.  Proceed?")
#define STR_Q_UNLINKATTACH			TEXT("Release the link to the attached file?")
#define STR_Q_ADDADDRESS			TEXT("Add %d mail addresses to the address book?")
#define STR_Q_NEXTFIND				TEXT("Search completed!\nRedo from the start?")
#define STR_Q_EDITCANSEL			TEXT("Cancel the edit?")
#define STR_Q_SENDMAIL				TEXT("Send it?")
#define STR_Q_ATTACH				TEXT("Open this file?")

// Message
#define STR_MSG_NOMARK				TEXT("There is no marked mail")
#define STR_MSG_NOBODY				TEXT("Message body not downloaded. Cannot be opened\n\n")\
									TEXT("Mark it and update server to download body")
#define STR_MSG_NONEWMAIL			TEXT("No new mail!")
#define STR_MSG_NOFIND				TEXT("\"%s\" not found")

// Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [Accounts with Unread mail: %d]")
#define STR_TITLE_MAILEDIT			TEXT("Mail Edit")
#define STR_TITLE_MAILVIEW			TEXT("Mail View")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [No.%d]")

// Message title
#define STR_TITLE_EXEC				TEXT("Update marked items")
#define STR_TITLE_ALLEXEC			TEXT("Update all accounts")
#define STR_TITLE_SEND				TEXT("Send now")
#define STR_TITLE_OPEN				TEXT("Open")
#define STR_TITLE_SAVE				TEXT("Save")
#define STR_TITLE_COPY				TEXT("Copy")
#define STR_TITLE_DELETE			TEXT("Delete")
#define STR_TITLE_ERROR				TEXT("Error")
#define STR_TITLE_SETMAILBOX		TEXT("Account Settings")
#define STR_TITLE_OPTION			TEXT("Options")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("Startup")
#define STR_TITLE_SHOWPASSWORD		TEXT("Show")
#endif
#define STR_TITLE_FIND				TEXT("Find")
#define STR_TITLE_ALLFIND			TEXT("Look up \"%s\"")
#define STR_TITLE_ATTACH_MSG		TEXT("Open")

// Window status
#define STR_STATUS_VIEWINFO			TEXT("View %d")
#define STR_STATUS_MAILBOXINFO		TEXT("View %d/ Server %d")
#define STR_STATUS_MAILINFO			TEXT("New %d, Unread %d")

// Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("Finding Host...")
#define STR_STATUS_CONNECT			TEXT("Connecting...")
#define STR_STATUS_SSL				TEXT("SSL connect...")
#define STR_STATUS_RECV				TEXT("Receiving...")
#define STR_STATUS_SENDBODY			TEXT("Sending body...")
#define STR_STATUS_SOCKINFO			TEXT("%d byte %s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("Recv")
#define STR_STATUS_SOCKINFO_SEND	TEXT("Send")
#define STR_STATUS_SEND_USER		TEXT("Send username")
#define STR_STATUS_SEND_PASS		TEXT("Send password")

// Ras status
#define STR_STATUS_RAS_START		TEXT("Starting Dial-up")
#define STR_STATUS_RAS_PORTOPEN		TEXT("Opening port...")
#define STR_STATUS_RAS_DEVICE		TEXT("Connecting device...")
#define STR_STATUS_RAS_AUTH			TEXT("Verifying user...")
#define STR_STATUS_RAS_CONNECT		TEXT("Dial-up connected")
#define STR_STATUS_RAS_DISCONNECT	TEXT("Dial-up disconnected")

// Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s bytes")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s KB")

// Mail list
#define STR_SAVEBOX_NAME			TEXT("[Savebox]")
#define STR_SENDBOX_NAME			TEXT("[Outbox]")
#define STR_MAILBOX_NONAME			TEXT("Untitled")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("Subject")
#define STR_LIST_LVHEAD_FROM		TEXT("From")
#define STR_LIST_LVHEAD_TO			TEXT("To")
#define STR_LIST_LVHEAD_DATE		TEXT("Date")
#define STR_LIST_LVHEAD_SIZE		TEXT("Size")
#define STR_LIST_NOSUBJECT			TEXT("(no subject)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("&Property...")
#define STR_LIST_MENU_REPLY			TEXT("&Reply...")
#define STR_LIST_MENU_SENDMARK		TEXT("&Mark for send\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("Create cop&y\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("&Mark for download\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("Copy to &Savebox\tCtrl+C")

// Mail view
#define STR_VIEW_HEAD_FROM			TEXT("From: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\nSubject: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\nDate: ")

#define STR_VIEW_MENU_ATTACH		TEXT("&Show attach")
#define STR_VIEW_MENU_SOURCE		TEXT("&View source")
#define STR_VIEW_MENU_DELATTACH		TEXT("&Delete attach")

// Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("Account: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\nTo: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\nSubject: ")

// SSL
#define STR_SSL_AUTO				TEXT("Auto")
#define STR_SSL_TLS10				TEXT("TLS 1.0")
#define STR_SSL_SSL30				TEXT("SSL 3.0")
#define STR_SSL_SSL20				TEXT("SSL 2.0")
#define STR_SSL_STARTTLS			TEXT("STARTTLS")

// Filter
#define STR_FILTER_USE				TEXT("Use")
#define STR_FILTER_NOUSE			TEXT("Unused")
#define STR_FILTER_STATUS			TEXT("Status")
#define STR_FILTER_ACTION			TEXT("Action")
#define STR_FILTER_ITEM1			TEXT("Item 1")
#define STR_FILTER_CONTENT1			TEXT("Content 1")
#define STR_FILTER_ITEM2			TEXT("Item 2")
#define STR_FILTER_CONTENT2			TEXT("Content 2")

#define STR_FILTER_UNRECV			TEXT("Not download")
#define STR_FILTER_RECV				TEXT("Download")
#define STR_FILTER_DOWNLOADMARK		TEXT("Mark for download")
#define STR_FILTER_DELETEMARK		TEXT("Mark for delete")
#define STR_FILTER_READICON			TEXT("Mark as read")
#define STR_FILTER_SAVE				TEXT("Copy to Savebox")

// Cc list
#define STR_CCLIST_TYPE				TEXT("Type")
#define STR_CCLIST_MAILADDRESS		TEXT("Mail address")

// Set send
#define STR_SETSEND_BTN_CC			TEXT("Cc, Bcc")
#define STR_SETSEND_BTN_ATTACH		TEXT("Attach")
#define STR_SETSEND_BTN_ETC			TEXT("Other")

// Mail Prop
#define STR_MAILPROP_HEADER			TEXT("Header")
#define STR_MAILPROP_MAILADDRESS	TEXT("Mail address")

// Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Mail address")
#define STR_ADDRESSLIST_COMMENT		TEXT("Comment")

// WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("Check for new mail")
#define STR_CMDBAR_ALLCHECK			TEXT("Check all accounts")
#define STR_CMDBAR_EXEC				TEXT("Update marked items")
#define STR_CMDBAR_ALLEXEC			TEXT("Update all accounts")
#define STR_CMDBAR_STOP				TEXT("Stop")
#define STR_CMDBAR_NEWMAIL			TEXT("New message")
#define STR_CMDBAR_RAS_CONNECT		TEXT("Dial-up connect")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("Dial-up disconnect")

#define STR_CMDBAR_PREVMAIL			TEXT("Prev mail")
#define STR_CMDBAR_NEXTMAIL			TEXT("Next mail")
#define STR_CMDBAR_NEXTNOREAD		TEXT("Next unread mail")
#define STR_CMDBAR_REMESSEGE		TEXT("Reply")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("Reply to all")
#define STR_CMDBAR_DOWNMARK			TEXT("Mark for download")
#define STR_CMDBAR_DELMARK			TEXT("Mark for delete")

#define STR_CMDBAR_SEND				TEXT("Send now")
#define STR_CMDBAR_SENDBOX			TEXT("Save to Outbox")
#define STR_CMDBAR_SENDINFO			TEXT("Property")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("&Property")
#define STR_LIST_PPCMENU_REPLY		TEXT("&Reply")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("&Mark for send")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("Create cop&y")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("&Mark for download")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("Copy to &Savebox")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("&Show attach")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("&View source")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("&Delete attach")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP-AUTH")
#define STR_TITLE_SETSSL			TEXT("Set SSL")
#define STR_TITLE_FILTER			TEXT("Set filter")
#define STR_TITLE_INITMAILBOX		TEXT("Initialize")
#define STR_TITLE_CCBCC				TEXT("Cc, Bcc")
#define STR_TITLE_ATTACH			TEXT("Attach files")
#define STR_TITLE_ETCHEADER			TEXT("Other headers")
#define STR_TITLE_SENDINFO			TEXT("Property")
#define STR_TITLE_ADDRESSINFO		TEXT("Address information")
#define STR_TITLE_EDITADDRESS		TEXT("Address")
#define STR_TITLE_ADDRESSLIST		TEXT("Address book")
#define STR_TITLE_FIND				TEXT("Find")
#define STR_TITLE_ENCODE			TEXT("Encoding")

#define STR_SF_TITLE				TEXT("Select file")
#define STR_SF_LV_NAME				TEXT("Name")
#define STR_SF_LV_TYPE				TEXT("Type")
#define STR_SF_LV_SIZE				TEXT("Size")
#define STR_SF_LV_DATE				TEXT("Date")
#define STR_SF_Q_OVERWRITE			TEXT("Replace existing file?")
#endif	//_WIN32_WCE_PPC

#ifdef _WIN32_WCE_LAGENDA
#define STR_MENU_FILE				TEXT("File")
#define STR_MENU_MAIL				TEXT("Mail")
#define STR_MENU_EDIT				TEXT("Edit")
#endif	//_WIN32_WCE_LAGENDA
#endif	//_WIN32_WCE

#endif	//_INC_STR_TBL_H
/* End of source */
