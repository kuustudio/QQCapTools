
// AlimCapDlg.h : 头文件
//

#pragma once
#include "afxeditbrowsectrl.h"
#include "afxcmn.h"
#include"MyEditBrowseCtrl.h"
#include"AnalysisCap.h"
#include "afxwin.h"
#include"MyListCtrl.h"
#include "HexEdit.h"
#include"DialogDec.h"
#include"DialogPwd.h"
#include"DialogSniffer.h"
#include"DialogMsg.h"
#include"ScanMemKeyDialog.h"


// CAlimCapDlg 对话框
class CAlimCapDlg : public CDialogEx
{
// 构造
public:
	CAlimCapDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CAlimCapDlg();

// 对话框数据
	enum { IDD = IDD_ALIMCAP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CMyEditBrowseCtrl mFileEditBox;
	//cap文件选择框变量
	CString mCapFile;
	// 数据包会话密码
	CString mPassWord;
	// 用户ID列表
	CComboBox mUserList;
	//CString mUserId;
	// SessionKey
	//CComboBox mSessionKeys;
	CString mSessionKey;
	// 数据显示控件
	MyListCtrl m_List;
	CWnd*		mHexEdit;
	CWnd*		mHexEditText;
	CComboBox cbSessions;
	//对话框
	CDialogPwd *pPwdDialog;
	CDialogSniffer *pSnifferDialog;
	CDialogDec *pDialogDec;
	CScanMemKeyDialog *pDialogScanKey;
	POINT old;
	CStatic mResult;//结果显示
	CIPAddressCtrl mFilterSrcIp;
	CIPAddressCtrl mFilterDstIp;
	CEdit mFilterSrcPort;
	CEdit mFilterDstPort;
public:
	afx_msg void OnBnClickedMfcbutton2();
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnBnClickedRadio(UINT idCtl);
	afx_msg void OnSessionSelchange();
	afx_msg void OnMenuDec();
	afx_msg void OnSetPwd();
	afx_msg void OnSniferDialog();
	afx_msg void OnSendMsg();
	afx_msg void OnScanMemECDHKEY();
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	void ReSize();
	void ClearControl();
	bool GetLocalPwdHash();
	bool SaveLocalPwdHash();
	void GetFilterInfo();
	CAnalysisCap* GetCACap(){ return &(((CAlimCapApp*)AfxGetApp())->CACap); };
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedCheck5();
};
