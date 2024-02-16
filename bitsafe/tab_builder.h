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

#pragma once

#include "list_ex_common.h"
#include "UIListSubElementEx.h"
#include "list_log.h"
#include "trace_frame.h"
#include "tool_frame.h"

class defense_frame : public CHorizontalLayoutUI
{
public:
	defense_frame()
	{
		CDialogBuilder builder;
		CContainerUI* defense_ui = static_cast<CContainerUI*>(builder.Create(_T("defense_tab.xml"), (UINT)0));
		if( defense_ui ) {
			this->AddItem(defense_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class summary_frame : public CHorizontalLayoutUI
{
public:
	summary_frame()
	{
		CDialogBuilder builder;
		CContainerUI* summary_ui = static_cast<CContainerUI*>(builder.Create(_T("summary_tab.xml"), (UINT)0));
		if( summary_ui ) {
			this->AddItem(summary_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class fw_frame : public CHorizontalLayoutUI
{
public:
	fw_frame() 
	{
		CDialogBuilder builder;
		CContainerUI* fw_ui = static_cast<CContainerUI*>(builder.Create(_T("fw_tab.xml"), (UINT)0));
		if( fw_ui ) {
			this->AddItem(fw_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
}; 

class about_frame : public CHorizontalLayoutUI
{
public:
	about_frame( CPaintManagerUI *pManager )
	{
		CDialogBuilder builder;
		CContainerUI* about_ui = static_cast<CContainerUI*>(builder.Create(_T("about_tab.xml"), (UINT)0, NULL, pManager, NULL ) );
		if( about_ui ) {
			this->AddItem(about_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class url_rule_frame : public CHorizontalLayoutUI
{
public:
	url_rule_frame()
	{
		CDialogBuilder builder;

		CPaintManagerUI m_pm; 
		CContainerUI* about_ui = static_cast<CContainerUI*>(builder.Create(_T("url_rule_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		if( about_ui ) {
			this->AddItem(about_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class ip_rule_frame : public CHorizontalLayoutUI
{
public:
	ip_rule_frame()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* about_ui = static_cast<CContainerUI*>(builder.Create(_T("ip_rule_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		
		if( about_ui ) 
		{
			this->AddItem(about_ui);
		}
		else 
		{
			this->RemoveAllItem();
			return;
		}
	}
};

class app_rule_frame : public CHorizontalLayoutUI
{
public:
	app_rule_frame()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* about_ui = static_cast<CContainerUI*>(builder.Create(_T("app_rule_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		if( about_ui ) {
			this->AddItem(about_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class reg_rule_frame : public CHorizontalLayoutUI
{
public:
	reg_rule_frame()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* about_ui = static_cast<CContainerUI*>(builder.Create(_T("reg_rule_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		if( about_ui ) {
			this->AddItem(about_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class file_rule_frame : public CHorizontalLayoutUI
{
public:
	file_rule_frame()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* about_ui = static_cast<CContainerUI*>(builder.Create(_T("file_rule_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		if( about_ui ) {
			this->AddItem(about_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class protect_option__frame : public CHorizontalLayoutUI
{
public:
	protect_option__frame ()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* option_ui = static_cast<CContainerUI*>(builder.Create(_T("shield_option_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		if( option_ui ) {
			this->AddItem(option_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class analyze_option_frame : public CHorizontalLayoutUI
{
public:
	analyze_option_frame()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* option_ui = static_cast<CContainerUI*>(builder.Create(_T("analyze_help_option_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		
		if( option_ui ) 
		{
			this->AddItem( option_ui ); 
		}
		else 
		{
			this->RemoveAllItem();
		}

	}
};

class ui_option__frame : public CHorizontalLayoutUI
{
public:
	ui_option__frame()
	{
		CDialogBuilder builder;
		CPaintManagerUI m_pm; 
		CContainerUI* option_ui = static_cast<CContainerUI*>(builder.Create(_T("ui_option_tab.xml"), (UINT)0, NULL, &m_pm, NULL ) );
		if( option_ui ) {
			this->AddItem(option_ui);
		}
		else {
			this->RemoveAllItem();
			return;
		}
	}
};

class tabs_builder : public IDialogBuilderCallback
{
public:
	tabs_builder( )
	{ 
	}

public:
	CControlUI* CreateControl(LPCTSTR pstrClass, CPaintManagerUI *pManager ) 
	{
		if( _tcscmp(pstrClass, _T("trace_tab")) == 0 ) 
		{
			return new trace_frame( pManager ); 
		}
		else if( _tcscmp(pstrClass, _T("tool_tab")) == 0 ) 
		{
			return new tool_frame( pManager ); 
		}
		else if( _tcscmp(pstrClass, _T("summary_tab")) == 0 ) 
		{
			return new summary_frame(); 
		}
		else if( _tcscmp(pstrClass, _T("fw_tab")) == 0 ) 
		{
			return new fw_frame(); 
		}
		else if( _tcscmp(pstrClass, _T("defense_tab")) == 0 ) 
		{
			return new defense_frame(); 
		}
		else if( _tcscmp(pstrClass, _T("about_tab")) == 0 ) 
		{
			return new about_frame( pManager ); 
		}
		else if( _tcscmp(pstrClass, _T("url_rule_tab")) == 0 ) 
		{
			return new url_rule_frame( ); 
		}
		else if( _tcscmp(pstrClass, _T("ip_rule_tab")) == 0 ) 
		{
			return new ip_rule_frame( ); 
		}
		else if( _tcscmp(pstrClass, _T("app_rule_tab")) == 0 ) 
		{
			return new app_rule_frame( ); 
		}
		else if( _tcscmp(pstrClass, _T("reg_rule_tab")) == 0 ) 
		{
			return new reg_rule_frame( ); 
		}
		else if( _tcscmp(pstrClass, _T("file_rule_tab")) == 0 ) 
		{
			return new file_rule_frame( ); 
		}
		else if( _tcscmp(pstrClass, _T("protect_option_tab")) == 0 ) 
		{
			return new protect_option__frame( ); 
		}
		else if( _tcscmp(pstrClass, _T("ui_option_tab")) == 0 ) 
		{
			return new ui_option__frame( );  
		}
		else if( _tcscmp( pstrClass, _T( "analyze_help_option_tab" ) ) == 0 )
		{
			return new analyze_option_frame(); 
		}

		return NULL;
	}
};