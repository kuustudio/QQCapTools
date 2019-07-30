// DialogDec.cpp : 实现文件
//

#include"stdafx.h"
#include "AlimCap.h"
#include "DialogDec.h"
#include "afxdialogex.h"
#include "AnalysisCap.h"

// CDialogDec 对话框

IMPLEMENT_DYNAMIC(CDialogDec, CDialogEx)

CDialogDec::CDialogDec(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogDec::IDD, pParent)
{

}

CDialogDec::~CDialogDec()
{
}

void CDialogDec::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, cTrlKey);
}


BEGIN_MESSAGE_MAP(CDialogDec, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO2, OnBnClickedRadio)
	ON_BN_CLICKED(IDC_BUTTON1, &CDialogDec::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDialogDec 消息处理程序


BOOL CDialogDec::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	CenterWindow();

	mHexEdit1 = HexEditControl::ShowHexControl(m_hWnd, 10, 60, 600, 150);
	HexEditControl::SetData(mHexEdit1, 0, 0);

	mHexEdit2 = HexEditControl::ShowHexControl(m_hWnd, 10, 220, 600, 150);
	HexEditControl::SetData(mHexEdit2, 0, 0);

	CButton* radio = (CButton*)GetDlgItem(IDC_RADIO2);
	radio->SetCheck(1);
	OnBnClickedRadio(IDC_RADIO2);

	cTrlKey.SetWindowTextA(mKey);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

void CDialogDec::OnBnClickedRadio(UINT idCtl)
{
	//清除表头
	if (idCtl == IDC_RADIO1)
	{
		//
		cTrlKey.EnableWindow(TRUE);
	}
	if (idCtl == IDC_RADIO2)
	{
		//
		cTrlKey.EnableWindow(FALSE);
	}

}

//解密
void CDialogDec::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	CString strKey = "";
	cTrlKey.GetWindowTextA(strKey);
	if (strKey.GetLength() != 0)
	{
		unsigned int len = HexEditControl::GetDataLength(mHexEdit1);
		char *buffer = (char*)malloc(len);
		memset(buffer, 0, len);
		HexEditControl::GetData(mHexEdit1, (BYTE*)buffer,len);

		//取出SessionKey
		std::string sKey = CAnalysisCap::StrToHex((BYTE*)strKey.GetBuffer(), strKey.GetLength());

		unsigned char key[KEYLEN] = { 0 };
		memcpy(key, sKey.c_str(), KEYLEN);

		unsigned char *dst = NULL;
		unsigned int bytes = 0;
		CAnalysisCap::unEncrypt((unsigned char*)buffer, len, key, &dst, &bytes);
		HexEditControl::SetData(mHexEdit2,dst, bytes);
		free(dst);
		dst = NULL;
	}
}

void CDialogDec::SetKey(CString key)
{
	mKey = key;
}
