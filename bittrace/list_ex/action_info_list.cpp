#include "stdafx.h"
#include "action_info_list.hpp"

using namespace DuiLib; 

namespace DuiLib
{

const int kGroupListItemNormalHeight = 32;
const int kGroupListItemSelectedHeight = 48;

CGroupsUI::CGroupsUI(CPaintManagerUI& paint_manager)
	: root_node_(NULL)
	, delay_deltaY_(0)
	, delay_number_(0)
	, delay_left_(0)
	, level_expand_image_(_T("<i list_icon_b.png>"))
	, level_collapse_image_(_T("<i list_icon_a.png>"))
	, level_text_start_pos_(10)
	, text_padding_(10, 0, 0, 0)
	, paint_manager_(paint_manager)
{
	SetItemShowHtml(true);

	root_node_ = new ListNode;
	root_node_->data().level = -1;
	root_node_->data().child_visible = true;
	root_node_->data().has_child = true;
	root_node_->data().list_element = NULL;
}

CGroupsUI::~CGroupsUI()
{
	if (root_node_)
		delete root_node_;

	root_node_ = NULL;
}

bool CGroupsUI::Add(CControlUI* pControl)
{
	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) == 0)
		return false;

	return CListUI::Add(pControl);
}

bool CGroupsUI::AddAt(CControlUI* pControl, int iIndex)
{
	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) == 0)
		return false;

	return CListUI::AddAt(pControl, iIndex);
}

bool CGroupsUI::Remove(CControlUI* pControl)
{
	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) == 0)
		return false;

	return CListUI::Remove(pControl);
}

bool CGroupsUI::RemoveAt(int iIndex)
{
	CControlUI* pControl = GetItemAt(iIndex);
	if (!pControl)
		return false;

	if (_tcsicmp(pControl->GetClass(), _T("ListContainerElementUI")) == 0)
		return false;

	return CListUI::RemoveAt(iIndex);
}
//
//struct NodeData
//{
//	int level_;
//	bool folder_;
//	bool child_visible;
//	bool has_child_;
//	tString text_;
//	tString value;
//	CListContainerElementUI* list_element_;
//};

void CGroupsUI::RemoveAll()
{
	CListUI::RemoveAll();
	delete root_node_;

	root_node_ = new ListNode;
	root_node_->data().level = -1;
	root_node_->data().child_visible = true;
	root_node_->data().has_child = true;
	root_node_->data().list_element = NULL;
}

void CGroupsUI::DoEvent(TEventUI& event) 
{
	if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND)
	{
		if (m_pParent != NULL)
			m_pParent->DoEvent(event);
		else
			CVerticalLayoutUI::DoEvent(event);
		return;
	}

	if (event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID)
	{
		if (delay_left_ > 0)
		{
			--delay_left_;
			SIZE sz = GetScrollPos();
			LONG lDeltaY =  (LONG)(CalculateDelay((double)delay_left_ / delay_number_) * delay_deltaY_);
			if ((lDeltaY > 0 && sz.cy != 0)  || (lDeltaY < 0 && sz.cy != GetScrollRange().cy ))
			{
				sz.cy -= lDeltaY;
				SetScrollPos(sz);
				return;
			}
		}
		delay_deltaY_ = 0;
		delay_number_ = 0;
		delay_left_ = 0;
		m_pManager->KillTimer(this, SCROLL_TIMERID);
		return;
	}
	if (event.Type == UIEVENT_SCROLLWHEEL)
	{
		LONG lDeltaY = 0;
		if (delay_number_ > 0)
			lDeltaY =  (LONG)(CalculateDelay((double)delay_left_ / delay_number_) * delay_deltaY_);
		switch (LOWORD(event.wParam))
		{
		case SB_LINEUP:
			if (delay_deltaY_ >= 0)
				delay_deltaY_ = lDeltaY + 8;
			else
				delay_deltaY_ = lDeltaY + 12;
			break;
		case SB_LINEDOWN:
			if (delay_deltaY_ <= 0)
				delay_deltaY_ = lDeltaY - 8;
			else
				delay_deltaY_ = lDeltaY - 12;
			break;
		}
		if
			(delay_deltaY_ > 100) delay_deltaY_ = 100;
		else if
			(delay_deltaY_ < -100) delay_deltaY_ = -100;

		delay_number_ = (DWORD)sqrt((double)abs(delay_deltaY_)) * 5;
		delay_left_ = delay_number_;
		m_pManager->SetTimer(this, SCROLL_TIMERID, 50U);
		return;
	}

	CListUI::DoEvent(event);
}

ListNode* CGroupsUI::GetRoot()
{
	return root_node_;
}

const TCHAR* const kLogoButtonControlName = _T("logo");
const TCHAR* const kLogoContainerControlName = _T("logo_container");
const TCHAR* const kNickNameControlName = _T("nickname");
const TCHAR* const kDescriptionControlName = _T("description");
const TCHAR* const kOperatorPannelControlName = _T("operation");

static bool OnLogoButtonEvent(void* event) {
    if( ((TEventUI*)event)->Type == UIEVENT_BUTTONDOWN ) {
        CControlUI* pButton = ((TEventUI*)event)->pSender;
        if( pButton != NULL ) {
            CListContainerElementUI* pListElement = (CListContainerElementUI*)(pButton->GetTag());
            if( pListElement != NULL ) pListElement->DoEvent(*(TEventUI*)event);
        }
    }
    return true;
}

ListNode* CGroupsUI::AddNode(const GroupsListItemInfo& item, ListNode* parent)
{
	if (!parent)
		parent = root_node_;

	TCHAR szBuf[MAX_PATH] = {0};

    CListContainerElementUI* pListElement = NULL;
    if( !m_dlgBuilder.GetMarkup()->IsValid() ) {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create(_T("group_list_item.xml"), (UINT)0, NULL, &paint_manager_));
    }
    else {
        pListElement = static_cast<CListContainerElementUI*>(m_dlgBuilder.Create((UINT)0, &paint_manager_));
    }
    if (pListElement == NULL)
        return NULL;

	ListNode* node = new ListNode;

	node->data().level = parent->data().level + 1;
	if (item.folder)
		node->data().has_child = !item.empty;
	else
		node->data().has_child = false;

	node->data().is_folder = item.folder;

	node->data().child_visible = (node->data().level == 0);

	node->data().text = item.nick_name;
	node->data().list_element = pListElement;

	if (!parent->data().child_visible)
		pListElement->SetVisible(false);

	if (parent != root_node_ && !parent->data().list_element->IsVisible())
		pListElement->SetVisible(false);

	CRect rcPadding = text_padding_;
	for (int i = 0; i < node->data().level; ++i)
	{
		rcPadding.left += level_text_start_pos_;		
	}
	pListElement->SetPadding(rcPadding);

	CButtonUI* log_button = static_cast<CButtonUI*>(paint_manager_.FindSubControlByName(pListElement, kLogoButtonControlName));
	if (log_button != NULL)
	{
		if (!item.folder && !item.logo.empty())
		{
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("%s"), item.logo.c_str());
#else
			_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.logo.c_str());
#endif
			log_button->SetNormalImage(szBuf);
		}
		else
		{
			CContainerUI* logo_container = static_cast<CContainerUI*>(paint_manager_.FindSubControlByName(pListElement, kLogoContainerControlName));
			if (logo_container != NULL)
				logo_container->SetVisible(false);
		}
        log_button->SetTag((UINT_PTR)pListElement);
        log_button->OnEvent += MakeDelegate(&OnLogoButtonEvent);
	}

	tString html_text;
	if (node->data().has_child)
	{
		if (node->data().child_visible)
			html_text += level_expand_image_;
		else
			html_text += level_collapse_image_;

#if defined(UNDER_WINCE)
		_stprintf(szBuf, _T("<x %d>"), level_text_start_pos_);
#else
		_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), level_text_start_pos_);
#endif
		html_text += szBuf;
	}

	if (item.folder)
	{
		html_text += node->data().text;
	}
	else
	{
#if defined(UNDER_WINCE)
		_stprintf(szBuf, _T("%s"), item.nick_name.c_str());
#else
		_stprintf_s(szBuf, MAX_PATH - 1, _T("%s"), item.nick_name.c_str());
#endif
		html_text += szBuf;
	}

	CLabelUI* nick_name = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kNickNameControlName));
	if (nick_name != NULL)
	{
		if (item.folder)
			nick_name->SetFixedWidth(0);

		nick_name->SetShowHtml(true);
		nick_name->SetText(html_text.c_str());
	}

	if (!item.folder && !item.description.empty())
	{
		CLabelUI* description = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(pListElement, kDescriptionControlName));
		if (description != NULL)
		{
#if defined(UNDER_WINCE)
			_stprintf(szBuf, _T("<x 20><c #808080>%s</c>"), item.description.c_str());
#else
			_stprintf_s(szBuf, MAX_PATH - 1, _T("<x 20><c #808080>%s</c>"), item.description.c_str());
#endif
			description->SetShowHtml(true);
			description->SetText(szBuf);
		}
	}

	pListElement->SetFixedHeight(kGroupListItemNormalHeight);
	pListElement->SetTag((UINT_PTR)node);
	int index = 0;
	if (parent->has_children())
	{
		ListNode* prev = parent->get_last_child();
		index = prev->data().list_element->GetIndex() + 1;
	}
	else 
	{
		if (parent == root_node_)
			index = 0;
		else
			index = parent->data().list_element->GetIndex() + 1;
	}
	if (!CListUI::AddAt(pListElement, index))
	{
		delete pListElement;
		delete node;
		node = NULL;
	}

	parent->add_child(node);
	return node;
}

void CGroupsUI::RemoveNode(ListNode* node)
{
	if (!node || node == root_node_) return;

	for (int i = 0; i < node->num_children(); ++i)
	{
		ListNode* child = node->child(i);
		RemoveNode(child);
	}

	CListUI::Remove(node->data().list_element);
	delete node->data().list_element;
	node->parent()->remove_child(node);
	delete node;
}

void CGroupsUI::SetChildVisible(ListNode* node, bool visible)
{
	if (!node || node == root_node_)
		return;

	if (node->data().child_visible == visible)
		return;

	node->data().child_visible = visible;

	TCHAR szBuf[MAX_PATH] = {0};
	tString html_text;
	if (node->data().has_child)
	{
		if (node->data().child_visible)
			html_text += level_expand_image_;
		else
			html_text += level_collapse_image_;

#if defined(UNDER_WINCE)
		_stprintf(szBuf, _T("<x %d>"), level_text_start_pos_);
#else
		_stprintf_s(szBuf, MAX_PATH - 1, _T("<x %d>"), level_text_start_pos_);
#endif
		html_text += szBuf;

		html_text += node->data().text;

		CLabelUI* nick_name = static_cast<CLabelUI*>(paint_manager_.FindSubControlByName(node->data().list_element, kNickNameControlName));
		if (nick_name != NULL)
		{
			nick_name->SetShowHtml(true);
			nick_name->SetText(html_text.c_str());
		}
	}

	if (!node->data().list_element->IsVisible())
		return;

	if (!node->has_children())
		return;

	ListNode* begin = node->child(0);
	ListNode* end = node->get_last_child();
	for (int i = begin->data().list_element->GetIndex(); i <= end->data().list_element->GetIndex(); ++i)
	{
		CControlUI* control = GetItemAt(i);
		if (_tcsicmp(control->GetClass(), _T("ListContainerElementUI")) == 0)
		{
			if (visible) 
			{
				ListNode* local_parent = ((ListNode*)control->GetTag())->parent();
				if (local_parent->data().child_visible && local_parent->data().list_element->IsVisible())
				{
					control->SetVisible(true);
				}
			}
			else
			{
				control->SetVisible(false);
			}
		}
	}
}

bool CGroupsUI::CanExpand(ListNode* node) const
{
	if (!node || node == root_node_) return false;

	return node->data().has_child;
}

} // namespace DuiLib