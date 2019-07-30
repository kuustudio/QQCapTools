#pragma once
#include "MyListCtrl.h"
#include<map>
#include<afxdialogex.h>
#include "afxwin.h"
#include "afxcmn.h"
#include"AnalysisCap.h"


// CDialogPwd 对话框

class CDialogPwd : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogPwd)

public:
	CDialogPwd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogPwd();

// 对话框数据
	enum { IDD = IDD_DIALOG_PWD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
public:
	CString mQQ;
	CString mPwd;
	MyListCtrl mPwdList;
	MyListCtrl mCurPwdList;
	CString mShareKey;
	CString mSessionKey;
	CString mAddShareKey;
	MyListCtrl mCurSessionKeys;
	CComboBox mClientType;
public:
	void InitListData();
	void SetRefreshList();
	CAnalysisCap* GetCACap();
};
