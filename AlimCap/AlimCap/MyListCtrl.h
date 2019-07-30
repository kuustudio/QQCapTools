#pragma once
#include<afxlistctrl.h>
#include<afx.h>

// MyListCtrl
#define MAX_HEADLENGTH 0x80

class CDialogMsg;

class MyListCtrl : public CMFCListCtrl
{
	DECLARE_DYNAMIC(MyListCtrl)

public:
	MyListCtrl();
	virtual ~MyListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
public:
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSendMsg();
	afx_msg void OnDecryptToSave();
	afx_msg void OnDelete();
public:
	CDialogMsg *pMsgDialog;
private:
	unsigned int nMenu;
	unsigned int mMainType;
public:
	void SetMainType(unsigned int _type, unsigned int _submenu){ mMainType = _type; nMenu = _submenu; };
	unsigned int GetMainType(){ return mMainType; }
	void SetStyle(DWORD);
	void SetHeaders(TCHAR[][MAX_HEADLENGTH], int);
	void Clear();
	static int CALLBACK ListCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
};

struct DATA
{
	BOOL m_fAsc;//是否顺序排序 
    int m_nSortedCol;//当前排序的列
	MyListCtrl *Ctrl;//当前的控件
};

