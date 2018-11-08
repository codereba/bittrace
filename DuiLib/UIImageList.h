#ifndef __UI_IMAGE_LIST_H__
#define __UI_IMAGE_LIST_H__

#ifdef _DEBUG_MEM_LEAKS
#ifdef new
#undef new
#endif //new 
#endif //_DEBUG_MEM_LEAKS

#include <vector>

#ifdef _DEBUG_MEM_LEAKS

#ifdef DBG_NORMAL_BLOCK
#define new DBG_NORMAL_BLOCK
#endif //DBG_NORMAL_BLOCK

#endif //_DEBUG_MEM_LEAKS

//#pragma pack(push)
//#pragma pack(8)

namespace DuiLib
{
	struct ImageNodeData
	{
		int level_;
		bool folder_;
		bool child_visible_;
		bool has_child_;
		CStdString text_;
		CStdString value;
		CListContainerElementUI* list_element_;
	};

	double CalculateDelay(double state);

	class ImageNode
	{
	public:
		ImageNode();
		explicit ImageNode(ImageNodeData t);
		ImageNode(ImageNodeData t, ImageNode* parent);
		~ImageNode();
		ImageNodeData& data();
		int num_children() const;
		ImageNode* child(int i);
		ImageNode* parent();
		bool folder() const;
		bool has_children() const;
		void add_child(ImageNode* child);
		void remove_child(ImageNode* child);
		ImageNode* get_last_child();

	private:
		void set_parent(ImageNode* parent);

	private:
		typedef std::vector <ImageNode*>	Children;

		Children	children_;
		ImageNode*		parent_;

		ImageNodeData    data_;
	};

	typedef enum _ITEM_ICON_TYPE 
	{
		ICON_FILE_NAME, 
		ICON_HANDLE
	} ITEM_ICON_TYPE, *PITEM_ICON_TYPE; 

	typedef struct _image_list_item_info
	{
		bool folder;
		bool empty;
		ITEM_ICON_TYPE icon_type;  
		CStdString id; 

		//union
		CStdString icon; 
		HANDLE icon_handle; 

		//CStdString nick_name;
		CStdString description;
	} image_list_item_info, *pimage_list_item_info; 


	class CImageListUI : public CListUI
	{
	public:
		enum {SCROLL_TIMERID = 10};

		CImageListUI(); 
		CImageListUI( CPaintManagerUI* paint_manager );

		~CImageListUI();

		//bool AddItem(CControlUI* pControl);

		//bool AddItemAt(CControlUI* pControl, int iIndex);

		//bool RemoveSubItem(CControlUI* pControl, ULONG );

		//bool RemoveItemAt(int iIndex);

		void RemoveSubAllItem( ULONG ClassId );

		void DoEvent(TEventUI& event);

		ImageNode* GetRoot();

		ImageNode* AddNode( const image_list_item_info &item, 
			ImageNode* parent = NULL);
		ImageNode* AddNodeEx( bool is_folder, 
			bool empty, 
			ITEM_ICON_TYPE icon_type, 
			LPCWSTR id, 
			LPCWSTR icon, 
			HANDLE icon_handle, 
			LPCWSTR description, 
			ImageNode* parent ); 

		void RemoveNode(ImageNode* node);

		void SetChildVisible(ImageNode* node, bool visible);

		bool CanExpand(ImageNode* node) const;

		//bool SelectListItem(int iIndex); 
		
		LRESULT set_paint_manage( CPaintManagerUI *paint_manager )
		{
			paint_manager_ = paint_manager; 
		}

	private:
		ImageNode*	root_node_;
		LONG	delay_deltaY_;
		DWORD	delay_number_;
		DWORD	delay_left_;
		CRect	text_padding_;
		int level_text_start_pos_;
		CStdString level_expand_image_;
		CStdString level_collapse_image_;
		CPaintManagerUI *paint_manager_;

		CDialogBuilder m_dlgBuilder;
	};

} // DuiLib

//#pragma pack(pop)

#endif // __UI_IMAGE_LIST_H__