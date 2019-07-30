// MyListCtrl.cpp : 实现文件
//

#include"resource.h"
#include "MyListCtrl.h"
#include"mymsg.h"
#include"AnalysisCap.h"
#include"AlimCap.h"
#include"DialogMsg.h"
#include"SendMsg.h"


// MyListCtrl

IMPLEMENT_DYNAMIC(MyListCtrl, CMFCListCtrl)

MyListCtrl::MyListCtrl()
{
	pMsgDialog = NULL;
	nMenu = 0;
	mMainType = 0;
}

MyListCtrl::~MyListCtrl()
{
	if (NULL != pMsgDialog)
	{
		delete pMsgDialog;
		pMsgDialog = NULL;
	}
}


BEGIN_MESSAGE_MAP(MyListCtrl, CMFCListCtrl)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &MyListCtrl::OnLvnColumnclick)
	ON_NOTIFY_REFLECT(NM_CLICK, &MyListCtrl::OnNMClick)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &MyListCtrl::OnLvnItemchanged)
	ON_NOTIFY_REFLECT(NM_RCLICK, &MyListCtrl::OnNMRClick)
	ON_COMMAND(ID_32789, &MyListCtrl::OnSendMsg)
	ON_COMMAND(ID_32790, &MyListCtrl::OnDecryptToSave)
	ON_COMMAND(ID_32795, &MyListCtrl::OnDelete)
END_MESSAGE_MAP()

// MyListCtrl 消息处理程序

void MyListCtrl::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	LONG   lStyle; 
	lStyle=GetWindowLong(this->m_hWnd,GWL_STYLE);//获取当前窗口类型 
	lStyle   &=   ~LVS_TYPEMASK;   //清除显示方式位 
	lStyle|=(LVS_REPORT);   //设置显示方式为报表方式 
	SetWindowLong(this->m_hWnd,GWL_STYLE,lStyle);//设置窗口类型

	SetExtendedStyle(GetExtendedStyle()|LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

	CMFCListCtrl::PreSubclassWindow();
}

void MyListCtrl::SetHeaders(TCHAR Array[][MAX_HEADLENGTH],int number)
{
	this->DeleteAllItems();
	CRect rect;
	this->GetWindowRect(&rect);
	for(int i=0;i<number;i++)
	{
		LVCOLUMN co;
		co.mask=LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;
		co.fmt=LVCFMT_CENTER;
		co.cx=rect.Width()/number;
		co.pszText=Array[i];
		co.cchTextMax=sizeof(co.pszText);
		co.iSubItem=i;
		co.iOrder=i;
		this->InsertColumn(i,&co);
	}
}

int CALLBACK MyListCtrl::ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) 
{
	DATA* pV=static_cast<DATA*>((PVOID)lParamSort); 
     
	int col=pV->m_nSortedCol;//点击的列项传递给col,用来判断点击了第几列  
    //取项的字符串  

	int nItem1,nItem2;
	LVFINDINFO FindInfo;
	FindInfo.flags=LVFI_PARAM;
	FindInfo.lParam=lParam1;
	nItem1=pV->Ctrl->FindItem(&FindInfo,-1);
	FindInfo.lParam=lParam2;
	nItem2=pV->Ctrl->FindItem(&FindInfo,-1);

	if((nItem1==-1)||(nItem2==-1))
	{
	   return 0;
	}
    CString strItem1, strItem2;
	strItem1=pV->Ctrl->GetItemText(lParam1,col);  
    strItem2=pV->Ctrl->GetItemText(lParam2,col);//获得要比较的字符
    int iCompRes=0;
    switch(pV->m_nSortedCol) 
    { 
		case(1):
		case(2):
			{
				if(strItem1.CompareNoCase(strItem2)>0)
					iCompRes=1;
				else if(strItem1.CompareNoCase(strItem2)==0)
					iCompRes=0;
				else
					iCompRes=-1;
			}break;
		default: 
			{
				//默认排序
				if(StrToInt(strItem1)>StrToInt(strItem2))
					iCompRes=1;
				else if(StrToInt(strItem1)==StrToInt(strItem2))
					iCompRes=0;
				else
					iCompRes=-1;
			}
			break; 
    } 
    ////根据当前的排序方式进行调整 
    if(pV->m_fAsc) 
        return iCompRes; 
    else 
        return iCompRes*-1; 
}

void MyListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	//暂不排序
	//DATA da={0};
	//da.Ctrl=this;
	////设置排序方式
 //   if( pNMLV->iSubItem == da.m_nSortedCol ) 
 //       da.m_fAsc = !da.m_fAsc; 
 //   else 
 //   { 
 //       da.m_fAsc = TRUE; 
 //       da.m_nSortedCol = pNMLV->iSubItem; 
 //   } 

	//int    i=this->GetItemCount();                //   这两句函数是关键哦。。。。
 //   while(i--) this->SetItemData(i,i);   
 //   //调用排序函数 
 //   this->SortItems( MyListCtrl::ListCompare,(DWORD_PTR)&da);
	*pResult = 0;
}

void MyListCtrl::Clear()
{
	this->DeleteAllItems();
	//删除控件列头
	/*int cols=this->GetHeaderCtrl().GetItemCount();
	for(int i=cols-1;i>=0;i--)
	{
		this->DeleteColumn(i);
	}
	*/
}

void MyListCtrl::SetStyle(DWORD style)
{
	LONG   lStyle; 
	lStyle=GetWindowLong(this->m_hWnd,GWL_STYLE);//获取当前窗口类型  
	lStyle|=(style);
	SetWindowLong(this->m_hWnd,GWL_STYLE,lStyle);//设置窗口类型
}


void MyListCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	//添加代码

	//首先得到点击的位置 
	POSITION pos = GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		//没有选中或者当前没有数据
		return;
	}
	//得到行号，通过POSITION转化 
	int nId = (int)GetNextSelectedItem(pos);
	//得到列中的内容（0表示第一列，同理1,2,3...表示第二，三，四...列） 
	CString str1 = GetItemText(nId, 0);
	CString str2 = GetItemText(nId, 1);

	HWND msgHandle = GetParent()->GetSafeHwnd();
	COPYDATASTRUCT cpd;
	unsigned int data[2] = { 0 };//行数,uuid
	data[0] = nId;
	data[1] = GetItemData(nId);
	cpd.dwData = WM_SEL_LIST; //选中的列                                                                                                         //标志发送字符串
	cpd.cbData = sizeof(data);
	cpd.lpData = data;
	::SendMessage(msgHandle, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cpd);
	*pResult = 0;
}


void MyListCtrl::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	//checkbox状态
	if ((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(1)) /* old state : unchecked */
		&& (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(2)) /* new state : checked */
		)
	{
		TRACE("Item %d is checked\n", pNMLV->iItem);

		HWND msgHandle = GetParent()->GetSafeHwnd();
		COPYDATASTRUCT cpd;
		unsigned int data[3] = { 0 }; //行数,uuid,checked/unchecked
		data[0]= pNMLV->iItem;
		data[1] = GetItemData(pNMLV->iItem);
		data[2] = 1;
		cpd.dwData = WM_SEL_CHECK; //选中的列                                                                                                         //标志发送字符串
		cpd.cbData = sizeof(data);
		cpd.lpData = data;
		::SendMessage(msgHandle, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cpd);
	}
	else if ((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(2)) /* old state : checked */
		&& (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(1)) /* new state : unchecked */
		)
	{
		TRACE("Item %d is unchecked\n", pNMLV->iItem);
		HWND msgHandle = GetParent()->GetSafeHwnd();
		COPYDATASTRUCT cpd;
		unsigned int data[3] = { 0 };//行数,uuid,checked/unchecked
		data[0] = pNMLV->iItem;
		data[1] = GetItemData(pNMLV->iItem);
		data[2] = 0;
		cpd.dwData = WM_SEL_CHECK; //选中的列                                                                                                         //标志发送字符串
		cpd.cbData = sizeof(data);
		cpd.lpData = data;
		::SendMessage(msgHandle, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cpd);
	}
	else
	{
		TRACE("Item %d does't change the check-status\n", pNMLV->iItem);
	}
	*pResult = 0;
}

void MyListCtrl::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && nMenu!=0)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		//添加线程操作
		VERIFY(menu.LoadMenu(IDR_MENU_LIST));			//这里是我们在1中定义的MENU的文件名称
		CMenu* popup = menu.GetSubMenu(nMenu);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	*pResult = 0;
}

void MyListCtrl::OnSendMsg()
{
	// TODO:  在此添加命令处理程序代码
	if (NULL == pMsgDialog)
	{
		pMsgDialog = new CDialogMsg();
		pMsgDialog->Create(IDD_DIALOG_MSG, this);
	}
	pMsgDialog->ShowWindow(SW_SHOW);

	POSITION m_pstion = GetFirstSelectedItemPosition();
	int m_nIndex = GetNextSelectedItem(m_pstion);
	HWND msgHandle = this->GetParent()->GetSafeHwnd();
	
	CSendMsg mSend;
	mSend.pMsgDialog = pMsgDialog;
	mSend.nIndex = m_nIndex;

	COPYDATASTRUCT cpd;
	unsigned int num = m_nIndex;
	cpd.dwData = WM_SNIFFER_PARENT; //选中的列                                                                                                         //标志发送字符串
	cpd.cbData = sizeof(mSend);
	cpd.lpData = &mSend;
	::SendMessage(msgHandle, WM_COPYDATA, 0, (LPARAM)&cpd);
}

//解密并保存,暂时解密所有的数据包
void MyListCtrl::OnDecryptToSave()
{
	// TODO:  在此添加命令处理程序代码

	POSITION m_pstion = GetFirstSelectedItemPosition();
	int m_nIndex = GetNextSelectedItem(m_pstion);
	HWND msgHandle = this->GetParent()->GetSafeHwnd();

	COPYDATASTRUCT cpd;
	unsigned int num = m_nIndex;
	cpd.dwData = WM_DECRYPTSAVE_PARENT; //选中的列    
	cpd.cbData = sizeof(m_nIndex);
	cpd.lpData = &m_nIndex;
	::SendMessage(msgHandle, WM_COPYDATA, 0, (LPARAM)&cpd);
}


void MyListCtrl::OnDelete()
{
	// TODO:  在此添加命令处理程序代码

	POSITION m_pstion = GetFirstSelectedItemPosition();
	int m_nIndex = GetNextSelectedItem(m_pstion);
	HWND msgHandle = this->GetParent()->GetSafeHwnd();

	COPYDATASTRUCT cpd;
	unsigned int data[2] = {0}; //行数,uuid
	data[0] = m_nIndex;
	data[1] = GetItemData(m_nIndex);
	cpd.dwData = WM_DELETE_ITEM; //选中的列                                                                                                         //标志发送字符串
	cpd.cbData = sizeof(data);
	cpd.lpData = data;
	::SendMessage(msgHandle, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cpd);
}
