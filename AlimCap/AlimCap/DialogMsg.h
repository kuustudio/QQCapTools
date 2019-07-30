#pragma once
#include "MyListCtrl.h"
#include "afxwin.h"
#include <afxdialogex.h>

// CDialogMsg 对话框

class CDialogMsg : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogMsg)

public:
	CDialogMsg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogMsg();

// 对话框数据
	enum { IDD = IDD_DIALOG_MSG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
public:
	CString mQQ;
	CString mText;
	MyListCtrl mList;
	DWORD mSrcIP;
	CString mSrcMac;
	CString mSrcPort;
	DWORD mDstIP;
	CString mDstMac;
	CString mDstPort;
	CString mKey;
public:
	unsigned int iSel;
public:
	CAnalysisCap *GetCACap(){ return &(((CAlimCapApp*)AfxGetApp())->CACap); };
	void SetKey(CString str){ mKey = str; };
	void SetQQ(CString str){ mQQ = str; };
	void SetSrcInfo(DWORD ip, CString port, CString mac){ mSrcIP = ip; mSrcMac = mac; mSrcPort = port; }
	void SetDstInfo(DWORD ip, CString port, CString mac){ mDstIP = ip; mDstMac = mac; mDstPort = port; }
	void SetSel(unsigned int sel){ iSel = sel; };
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnBnClickedButton2();
};
