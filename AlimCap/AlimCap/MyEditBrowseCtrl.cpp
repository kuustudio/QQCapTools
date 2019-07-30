// MyEditBrowseCtrl.cpp : 实现文件
//


#include "AlimCap.h"
#include "MyEditBrowseCtrl.h"


// CMyEditBrowseCtrl

IMPLEMENT_DYNAMIC(CMyEditBrowseCtrl, CMFCEditBrowseCtrl)

CMyEditBrowseCtrl::CMyEditBrowseCtrl()
{
	 m_strFileType = _T("Cap Files (*.pcap)|*.pcap||"); 
}

CMyEditBrowseCtrl::~CMyEditBrowseCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyEditBrowseCtrl, CMFCEditBrowseCtrl)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CMyEditBrowseCtrl 消息处理程序
void CMyEditBrowseCtrl::OnBrowse()  
{  
    CString TempPathName;  
  
    CFileDialog dlg(TRUE,NULL,NULL,NULL,m_strFileType,NULL,0,TRUE);  
    (dlg.m_ofn).lpstrTitle=_T("打开文件");  
  
    if(dlg.DoModal()==IDOK)  
    {  
        TempPathName=dlg.GetPathName();  
        SetWindowText(TempPathName);  
    }  
    else  
        return;  
}

void CMyEditBrowseCtrl::OnDropFiles(HDROP hDropInfo)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (hDropInfo)
	{
		int nDrag; //拖拽文件的数量
		nDrag = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
		if (nDrag == 1)
		{
			// 被拖拽的文件的文件名
			TCHAR Path[MAX_PATH + 1] = { 0 };
			int len;
			// 得到被拖拽的文件名
			len = DragQueryFile(hDropInfo, 0, Path, MAX_PATH);
			// 把文件名显示出来
			SetWindowText(Path);
		}
		else
		{
			MessageBox(_T("只能拖拽一个PE文件！！"));
		}
	}
	CMFCEditBrowseCtrl::OnDropFiles(hDropInfo);
}
