#ifndef __UITREE_H__
#define __UITREE_H__

#include <vector>

namespace DuiLib
{
	struct NodeData
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

	class Node
	{
	public:
		Node();
		explicit Node(NodeData t);
		Node(NodeData t, Node* parent);
		~Node();
		NodeData& data();
		int num_children() const;
		Node* child(int i);
		Node* parent();
		bool folder() const;
		bool has_children() const;
		void add_child(Node* child);
		void remove_child(Node* child);
		Node* get_last_child();

	private:
		void set_parent(Node* parent);

	private:
		typedef std::vector <Node*>	Children;

		Children	children_;
		Node*		parent_;

		NodeData    data_;
	};

	typedef struct _tree_list_item_info
	{
		bool folder;
		bool empty;
		CStdString id;
		CStdString icon;
		//CStdString nick_name;
		CStdString description;
	} tree_list_item_info, *ptree_list_item_info; 


	class CTreeUI : public CListUI
	{
	public:
		enum {SCROLL_TIMERID = 10};

		CTreeUI(); 
		CTreeUI( CPaintManagerUI* paint_manager );

		~CTreeUI();

		bool AddItem(CControlUI* pControl);

		bool AddItemAt(CControlUI* pControl, int iIndex);

		bool RemoveItem(CControlUI* pControl);

		bool RemoveItemAt(int iIndex);

		void RemoveAllItem();

		void DoEvent(TEventUI& event);

		Node* GetRoot();

		Node* AddNode(const tree_list_item_info& item, Node* parent = NULL);

		void RemoveNode(Node* node);

		void SetChildVisible(Node* node, bool visible);

		bool CanExpand(Node* node) const;

		bool SelectListItem(int iIndex); 
		
		LRESULT set_paint_manage( CPaintManagerUI *paint_manager )
		{
			paint_manager_ = paint_manager; 
		}

	private:
		Node*	root_node_;
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

#endif // __UITREE_H__