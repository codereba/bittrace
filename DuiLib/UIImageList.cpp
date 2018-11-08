/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <math.h>
#include "UIImageList.h"

#if _MSC_VER <= 1310
#define _stprintf_s _sntprintf
#define _vstprintf_s _vsntprintf
#endif

namespace DuiLib
{
	void ImageNode::set_parent( ImageNode* parent )
	{
		parent_ = parent;
	}

	ImageNode::ImageNode()
		: parent_( NULL )
	{}

	ImageNode::ImageNode( ImageNodeData t )
		: data_( t )
		, parent_( NULL )
	{}

	ImageNode::ImageNode( ImageNodeData t, ImageNode* parent )
		: data_ ( t )
		, parent_ ( parent )
	{}

	ImageNode::~ImageNode() 
	{
		for( int i = 0; i < num_children(); ++i )
			delete children_[ i ]; 
	}

	ImageNodeData& ImageNode::data()
	{
		return data_;
	}

	int ImageNode::num_children() const
	{
		return static_cast< int >( children_.size() );
	}

	ImageNode* ImageNode::child(int i)
	{
		return children_[i];
	}

	ImageNode* ImageNode::parent()
	{
		return ( parent_ );
	}

	bool ImageNode::has_children() const
	{
		return num_children() > 0;
	}

	bool ImageNode::folder() const
	{
		return data_.folder_;
	}

	void ImageNode::add_child( ImageNode* child )
	{
		child->set_parent( this ); 
		children_.push_back( child ); 
	}

	void ImageNode::remove_child( ImageNode* child )
	{
		Children::iterator iter = children_.begin();
		for( ; iter < children_.end(); ++iter )
		{
			if( *iter == child ) 
			{
				children_.erase( iter );
				return;
			}
		}
	}

	ImageNode* ImageNode::get_last_child()
	{
		if( has_children() )
		{
			return child( num_children() - 1 )->get_last_child();
		}
		return this;
	}

	const int list_item_normal_height = 95;
	const int list_item_selected_height = 95;

	CImageListUI::CImageListUI() 
	{
		CImageListUI( NULL ); 
	}

	CImageListUI::CImageListUI( CPaintManagerUI *paint_manager )
		: root_node_(NULL)
		, delay_deltaY_(0)
		, delay_number_(0)
		, delay_left_(0)
		, level_expand_image_(_T("<i tree_btn_expanded.png>"))
		, level_collapse_image_(_T("<i tree_btn_collapsed.png>"))
		, level_text_start_pos_(10)
		, text_padding_(10, 0, 0, 0)
		, paint_manager_( paint_manager )
	{
		SetItemShowHtml( true );

		root_node_ = new ImageNode;
		root_node_->data().level_ = -1;
		root_node_->data().child_visible_ = true;
		root_node_->data().has_child_ = true;
		root_node_->data().list_element_ = NULL;
	}

	CImageListUI::~CImageListUI()
	{
		if( NULL != root_node_ )
		{
			delete root_node_;
		}

		root_node_ = NULL;
	}

	void CImageListUI::RemoveSubAllItem( ULONG ClassId )
	{
		CListUI::RemoveSubAllItem( ClassId );
		delete root_node_;

		if( IS_UI_LIST_ELEMENT_CLASS_ID( ClassId ) )
		{
			root_node_ = new ImageNode;
			root_node_->data().level_ = -1;
			root_node_->data().child_visible_ = true;
			root_node_->data().has_child_ = true;
			root_node_->data().list_element_ = NULL;
		}
	}

	void CImageListUI::DoEvent( TEventUI& event ) 
	{
		if( !IsMouseEnabled() 
			&& event.Type > UIEVENT__MOUSEBEGIN 
			&& event.Type < UIEVENT__MOUSEEND )
		{
			if( m_pParent != NULL )
			{
				m_pParent->DoEvent( event ); 
			}
			else
			{
				CVerticalLayoutUI::DoEvent( event ); 
			}

			return; 
		}

		if( event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID )
		{
			if( delay_left_ > 0 )
			{
				--delay_left_; 

				SIZE sz = GetScrollPos();
				
				LONG lDeltaY = ( LONG )( CalculateDelay( ( double )delay_left_ / delay_number_ ) * delay_deltaY_ ); 

				if( ( lDeltaY > 0 && sz.cy != 0 ) 
					|| ( lDeltaY < 0 && sz.cy != GetScrollRange().cy ) )
				{
					sz.cy -= lDeltaY;
					SetScrollPos( sz ); 
					return; 
				}
			}

			delay_deltaY_ = 0; 
			delay_number_ = 0; 
			delay_left_ = 0; 
			m_pManager->KillTimer( this, SCROLL_TIMERID ); 
			return;
		}

		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			LONG lDeltaY = 0; 

			if( delay_number_ > 0 )
				lDeltaY = ( LONG )( CalculateDelay( ( double )delay_left_ / delay_number_ ) * delay_deltaY_ ); 

			switch( LOWORD( event.wParam ) )
			{
			case SB_LINEUP:
				if( delay_deltaY_ >= 0 )
				{
					delay_deltaY_ = lDeltaY + 8;
				}
				else
				{
					delay_deltaY_ = lDeltaY + 12;
				}
				break;
			case SB_LINEDOWN:
				if( delay_deltaY_ <= 0 )
				{
					delay_deltaY_ = lDeltaY - 8;
				}
				else
				{
					delay_deltaY_ = lDeltaY - 12;
				}
				break;
			}

			if( delay_deltaY_ > 100 ) 
			{
				delay_deltaY_ = 100;
			}
			else if( delay_deltaY_ < -100 ) 
			{
				delay_deltaY_ = -100;
			}

			delay_number_ = ( DWORD )sqrt( ( double )abs( delay_deltaY_ ) ) * 5; 

			delay_left_ = delay_number_; 

			m_pManager->SetTimer( this, SCROLL_TIMERID, 50U ); 
			return;
		}
		
		CListUI::DoEvent( event );
	}

	ImageNode* CImageListUI::GetRoot()
	{
		return root_node_;
	}

	const TCHAR* const icon_btn_name = _T( "icon" );
	const TCHAR* const icon_container_name = _T( "icon_container" );
	const TCHAR* const image_item_name = _T( "image_item" ); 
	const TCHAR* const desc_lbl_name = _T( "desc_text" );

	static bool OnLogoButtonEvent( void* event ) 
	{
		if( ( ( TEventUI* )event )->Type == UIEVENT_BUTTONDOWN ) 
		{
			CControlUI* pButton = ( ( TEventUI* )event )->pSender; 

			if( pButton != NULL ) 
			{
				CListContainerElementUI* pListElement = ( CListContainerElementUI* )( pButton->GetTag() ); 
				
				if( pListElement != NULL ) 
				{
					pListElement->DoEvent( *( TEventUI* )event );
				}
			}
		}

		return true;
	}

	ImageNode* CImageListUI::AddNode( const image_list_item_info& item, ImageNode* parent )
	{
		ImageNode* node = NULL; 
		TCHAR szBuf[ MAX_PATH ] = { 0 }; 
		CListContainerElementUI* pListElement = NULL; 
		CStdString html_text; 
		CButtonUI* icon_btn; 
		CLabelUI* desc_lbl; 
		CRect rcPadding; 

		if( parent == NULL )
		{
			parent = root_node_; 
		}

		ASSERT( paint_manager_ != NULL ); 

		if( !m_dlgBuilder.GetMarkup()->IsValid() ) 
		{
			pListElement = static_cast< CListContainerElementUI* >( m_dlgBuilder.Create( _T( "list_image_row.xml" ), ( UINT )0, NULL, paint_manager_ ) ); 
		}
		else 
		{
			pListElement = static_cast< CListContainerElementUI* >( m_dlgBuilder.Create( ( UINT )0, paint_manager_ ) ); 
		}

		if( pListElement == NULL )
		{
			goto _return; 
		}

		node = new ImageNode;

		node->data().level_ = parent->data().level_ + 1; 
		
		if( item.folder )
		{
			node->data().has_child_ = !item.empty;
		}
		else
		{
			node->data().has_child_ = false;
		}

		node->data().folder_ = item.folder;

		node->data().child_visible_ = ( node->data().level_ == 0 );

		node->data().text_ = item.description;
		node->data().value = item.id;
		node->data().list_element_ = pListElement;

		if( parent->data().child_visible_ == false )
		{
			pListElement->SetVisible( false ); 
		}

		if( parent != root_node_ && parent->data().list_element_->IsVisible() == false )
		{
			pListElement->SetVisible( false );
		} 

		rcPadding = text_padding_;
		for( int i = 0; i < node->data().level_; ++i )
		{
			rcPadding.left += level_text_start_pos_; 
		}

		pListElement->SetPadding( rcPadding );

#define TREE_HEADER_BK_CLR 0x002778DD
#define TREE_ITEM_BK_CLR 0x00D3E5FA

		CContainerUI* tree_item = static_cast< CContainerUI* >( paint_manager_->FindSubControlByName( pListElement, image_item_name ) ); 

		ASSERT( tree_item != NULL ); 

		if( item.folder == true )
		{
			tree_item->SetBkColor( TREE_HEADER_BK_CLR ); 
		}
		else
		{
			tree_item->SetBkColor( TREE_ITEM_BK_CLR ); 
		}

		icon_btn = static_cast< CButtonUI* >( paint_manager_->FindSubControlByName( pListElement, icon_btn_name ) ); 

		icon_btn->SetFixedHeight( 95 ); 
		icon_btn->SetFixedWidth( 130 ); 

		if( icon_btn != NULL )
		{
			if( !item.folder && !item.icon.IsEmpty() )
			{
#if defined( UNDER_WINCE )
				_stprintf( szBuf, _T( "%s" ), item.icon.GetData() ); 
#else
				_stprintf_s( szBuf, MAX_PATH - 1, _T( "%s" ), item.icon ); 
#endif
				icon_btn->SetNormalImage( szBuf ); 
			}
			else
			{
				CContainerUI* icon_container = static_cast< CContainerUI* >( paint_manager_->FindSubControlByName( pListElement, icon_container_name ) ); 

				if( icon_container != NULL )
				{
					icon_container->SetVisible( false ); 
				}
			}
			
			icon_btn->SetTag( ( UINT_PTR )pListElement ); 

			icon_btn->OnEvent += MakeDelegate( &OnLogoButtonEvent );
		}

		if( node->data().has_child_ )
		{
			if( node->data().child_visible_ )
			{
				html_text += level_expand_image_;
			}
			else
			{
				html_text += level_collapse_image_;
			}

#if defined(UNDER_WINCE)
			_stprintf( szBuf, _T( "<x %d>" ), level_text_start_pos_ );
#else
			_stprintf_s( szBuf, MAX_PATH - 1, _T( "<x %d>" ), level_text_start_pos_ );
#endif
			html_text += szBuf; 
		}

		if( item.folder )
		{
			html_text += node->data().text_;
		}
		else
		{
#if defined( UNDER_WINCE )
			_stprintf( szBuf, _T( "%s" ), item.description.GetData() ); 
#else
			_stprintf_s( szBuf, MAX_PATH - 1, _T( "%s" ), item.description );
#endif
			html_text += szBuf;
		}

		if( item.folder == true )
		{
			desc_lbl = static_cast< CLabelUI* >( paint_manager_->FindSubControlByName( pListElement, desc_lbl_name ) ); 

			if( desc_lbl != NULL )
			{
				if( item.folder )
				{
					desc_lbl->SetFixedWidth( 0 ); 
				}

				desc_lbl->SetShowHtml( true ); 
				desc_lbl->SetText( html_text.GetData() ); 
			}
			else
			{
				ASSERT( FALSE ); 
			}
		}
		else
		{
			if( !item.description.IsEmpty() )
			{
				desc_lbl = static_cast< CLabelUI* >( paint_manager_->FindSubControlByName( pListElement, desc_lbl_name ) );
				if( desc_lbl != NULL )
				{
#if defined(UNDER_WINCE)
					_stprintf( szBuf, _T( "<x 20><c #808080>%s</c>"), item.description );
#else
					_stprintf_s( szBuf, MAX_PATH - 1, _T( "<x 20><c #808080>%s</c>" ), item.description ); 
#endif
					desc_lbl->SetShowHtml( true );
					desc_lbl->SetText( szBuf );
				}
				else
				{
					ASSERT( FALSE ); 
				}
			}
		}

		pListElement->SetOwner( this ); 
		pListElement->SetFixedHeight( list_item_normal_height ); 
		pListElement->SetTag( ( UINT_PTR )node ); 

		int index = 0;
		if( parent->has_children() )
		{
			ImageNode* prev = parent->get_last_child(); 
			index = prev->data().list_element_->GetIndex() + 1; 
		}
		else 
		{
			if( parent == root_node_ )
			{
				index = 0; 
			}
			else
			{
				index = parent->data().list_element_->GetIndex() + 1; 
			}
		}

		if( !CListUI::AddSubItemAt( pListElement, index, UI_LIST_CONTAINER_ELEMENT_CLASS_ID ) )
		{
			delete pListElement; 
			delete node; 
			node = NULL; 
			pListElement = NULL; 
			goto _return; 
		}

		parent->add_child( node ); 

_return:
		if( node == NULL )
		{
			if( pListElement != NULL )
			{
				delete pListElement; 
			}
		}

		return node;
	}

	ImageNode* CImageListUI::AddNodeEx( bool is_folder, 
		bool empty, 
		ITEM_ICON_TYPE icon_type, 
		LPCWSTR id, 
		LPCWSTR icon, 
		HANDLE icon_handle, 
		LPCWSTR description, 
		ImageNode* parent )
	{
		image_list_item_info item; 

		item.folder = is_folder; 
		item.description = description; 
		item.empty = empty; 
		item.id = id; 
		item.icon = icon; 
		item.icon_handle = icon_handle; 
		item.description = description; 

		return AddNode( item, parent ); 
	}

	void CImageListUI::RemoveNode( ImageNode* node )
	{
		if( !node || node == root_node_ ) 
		{
			return; 
		}

		for( int i = 0; i < node->num_children(); ++i )
		{
			ImageNode* child = node->child( i ); 
			RemoveNode( child ); 
		}

		CListUI::RemoveItem( node->data().list_element_ ); 

		delete node->data().list_element_; 

		node->parent()->remove_child( node ); 
		
		delete node; 
	}

	void CImageListUI::SetChildVisible( ImageNode* node, bool visible ) 
	{
		if( !node 
			|| node == root_node_ ) 
		{
			return; 
		}

		if( node->data().child_visible_ == visible )
		{
			return;
		}

		node->data().child_visible_ = visible; 

		TCHAR szBuf[ MAX_PATH ] = { 0 }; 
		CStdString html_text;
		
		if( node->data().has_child_ )
		{
			if( node->data().child_visible_ )
			{
				html_text += level_expand_image_;
			}
			else
			{
				html_text += level_collapse_image_;
			}

#if defined(UNDER_WINCE)
			_stprintf( szBuf, _T( "<x %d>" ), level_text_start_pos_ ); 
#else
			_stprintf_s( szBuf, MAX_PATH - 1, _T( "<x %d>" ), level_text_start_pos_ ); 
#endif
			html_text += szBuf; 

			html_text += node->data().text_; 

			CLabelUI* desc_lbl = static_cast< CLabelUI* >( paint_manager_->FindSubControlByName( node->data().list_element_, desc_lbl_name ) ); 

			if( desc_lbl != NULL )
			{
				desc_lbl->SetShowHtml( true ); 
				desc_lbl->SetText( html_text.GetData() ); 
			}
		}

		if( !node->data().list_element_->IsVisible() )
		{
			return; 
		}

		if( !node->has_children() )
		{
			return; 
		}

		ImageNode* begin = node->child( 0 );
		ImageNode* end = node->get_last_child(); 

		for( INT32 i = begin->data().list_element_->GetIndex(); 
			i <= end->data().list_element_->GetIndex(); 
			++i )
		{
			CControlUI* control = GetItemAt( i ); 

			if( _tcsicmp( control->GetClass(), _T( "ListContainerElementUI" ) ) == 0 )
			{
				if( visible ) 
				{
					ImageNode* local_parent = ( ( ImageNode* )control->GetTag() )->parent(); 

					if( local_parent->data().child_visible_ 
						&& local_parent->data().list_element_->IsVisible() )
					{
						control->SetVisible( true ); 
					}
				}
				else
				{
					control->SetVisible( false );
				}
			}
		}
	}

	bool CImageListUI::CanExpand( ImageNode* node ) const
	{
		if( !node || node == root_node_ ) 
			return false;

		return node->data().has_child_;
	}
} 

// namespace DuiLib