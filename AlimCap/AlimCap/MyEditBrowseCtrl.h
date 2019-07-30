#pragma once
#include<afxeditbrowsectrl.h>
#include<afx.h>
#include<afxdlgs.h>


// CMyEditBrowseCtrl

class CMyEditBrowseCtrl : public CMFCEditBrowseCtrl
{
	DECLARE_DYNAMIC(CMyEditBrowseCtrl)

public:
	CMyEditBrowseCtrl();
	virtual ~CMyEditBrowseCtrl();
protected:
	DECLARE_MESSAGE_MAP()

public:  
    CString m_strFileType;//自定义开文件类型
private:  
    virtual void OnBrowse();//重写CMFCEditBrowseCtrl的打开函数


public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};


