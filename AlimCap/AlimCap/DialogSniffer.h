#pragma once
#include "afxwin.h"
#include "MyListCtrl.h"
#include "HexEdit.h"
#include<afxdialogex.h>
#include"AnalysisCap.h"


// CDialogSniffer 对话框

class CDialogSniffer : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogSniffer)

public:
	CDialogSniffer(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogSniffer();

// 对话框数据
	enum { IDD = IDD_DIALOG1_SNIFFER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedMfcbutton3();
	afx_msg void OnBnClickedRadio(UINT idCtl);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
public:
	CComboBox devCBbox;
	MyListCtrl mList;
	CEdit mQQctrl;
	CEdit cTrlKey;
	CHexEdit mHexEdit;
	CHexEdit mHexEditTxt;
public:
	bool LoadAllDevs();
	CAnalysisCap* GetCACap();
	void SetKey(CString);
	void SetQQ(CString);
};
