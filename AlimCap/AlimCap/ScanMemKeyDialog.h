#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "tlhelp32.h"


// CScanMemKeyDialog 对话框

class CScanMemKeyDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CScanMemKeyDialog)

public:
	CScanMemKeyDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CScanMemKeyDialog();

// 对话框数据
	enum { IDD = IDD_SCANMEM_ECDHKEY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnLbnDblclkList1();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	CComboBox mProcess;
	CListBox mList;
	CToolTipCtrl m_Mytip;
public:
	CAnalysisCap *GetCACap(){ return &(((CAlimCapApp*)AfxGetApp())->CACap); };
	std::vector<PROCESSENTRY32> processlist;
	int TestProcessList();
	static unsigned int __stdcall ThreadFunc1(void*pArguments);

};
