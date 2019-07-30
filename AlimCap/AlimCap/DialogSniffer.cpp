// DialogSniffer.cpp : 实现文件
//


#include "AlimCap.h"
#include "DialogSniffer.h"
#include "afxdialogex.h"
#include"../Packetyzer/Packetyzer.h"
#include"mymsg.h"
#include"DialogMsg.h"
#include"SendMsg.h"


// CDialogSniffer 对话框

IMPLEMENT_DYNAMIC(CDialogSniffer, CDialogEx)

CDialogSniffer::CDialogSniffer(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogSniffer::IDD, pParent)
{

}

CDialogSniffer::~CDialogSniffer()
{
}

void CDialogSniffer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, devCBbox);
	DDX_Control(pDX, IDC_LIST1, mList);
	DDX_Control(pDX, IDC_EDIT1, cTrlKey);
	DDX_Control(pDX, IDC_CUSTOM1, mHexEdit);
	DDX_Control(pDX, IDC_CUSTOM2, mHexEditTxt);
	DDX_Control(pDX, IDC_EDIT2, mQQctrl);
}


BEGIN_MESSAGE_MAP(CDialogSniffer, CDialogEx)
	ON_BN_CLICKED(IDC_MFCBUTTON3, &CDialogSniffer::OnBnClickedMfcbutton3)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO2, OnBnClickedRadio)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


// CDialogSniffer 消息处理程序


BOOL CDialogSniffer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CenterWindow();
	LoadAllDevs();

	char _column[][MAX_HEADLENGTH] = { "序号","id", "cmdid", "收发行为" };
	this->mList.SetHeaders(_column, sizeof(_column) / sizeof(*_column));
	this->mList.SetColumnWidth(0, 50);
	this->mList.SetColumnWidth(1, 100);
	this->mList.EnsureVisible(mList.GetItemCount() - 1, FALSE);

	CButton* radio = (CButton*)GetDlgItem(IDC_RADIO2);
	radio->SetCheck(1);
	OnBnClickedRadio(IDC_RADIO2);

	mHexEdit.SetWideView(true);
	mHexEdit.ShowAllAscii(true);

	mHexEditTxt.SetWideView(true);
	mHexEditTxt.ShowAllAscii(true);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CDialogSniffer::OnBnClickedMfcbutton3()
{
	// TODO:  在此添加控件通知处理程序代码
	CString strText = "";
	devCBbox.GetWindowTextA(strText);
	if (strText.GetLength() == 0) return;
	strText = "";
	CButton* button = (CButton*)GetDlgItem(IDC_MFCBUTTON3);
	button->GetWindowTextA(strText);
	if (strText == "Start")
	{
		//先清空控件
		mList.Clear();
		mHexEdit.DeleteData(mHexEdit.GetDataLength());
		mHexEditTxt.DeleteData(mHexEditTxt.GetDataLength());
		//开始嗅听
		button->SetWindowTextA("Stop");
		devCBbox.EnableWindow(false);

		int num = devCBbox.GetCurSel();
		strText = GetCACap()->mNetCardList[num].name.c_str();
		HWND handle = this->GetSafeHwnd();

		CString strQQ = "";
		this->mQQctrl.GetWindowTextA(strQQ);
		GetCACap()->mSniffer.filterId = strtoul(strQQ, NULL, 10);
		if (!GetCACap()->StartOpenSniffer(strText,this->GetSafeHwnd()))
		{
			AfxMessageBox("监听错误");
			return;
		}
	}
	else
	{
		//停止嗅听
		devCBbox.GetWindowTextA(strText);
		GetCACap()->StopOpenSniffer();
		
		button->SetWindowTextA("Start");
		devCBbox.EnableWindow(true);
	}
}

bool CDialogSniffer::LoadAllDevs()
{
	GetCACap()->LoadAllDevs(GetCACap()->mNetCardList);
	NetCardInfos::iterator itor = GetCACap()->mNetCardList.begin();
	for (; itor != GetCACap()->mNetCardList.end(); itor++)
	{
		devCBbox.AddString(itor->description.c_str());
	}
	devCBbox.SetCurSel(0);
	GetCACap()->Freealldevs();
	return true;
}

void CDialogSniffer::OnBnClickedRadio(UINT idCtl)
{
	//清除表头
	if (idCtl == IDC_RADIO1)
	{
		//
		cTrlKey.EnableWindow(TRUE);
		mQQctrl.EnableWindow(TRUE);
	}
	if (idCtl == IDC_RADIO2)
	{
		//
		cTrlKey.EnableWindow(FALSE);
		mQQctrl.EnableWindow(FALSE);
	}

}

BOOL CDialogSniffer::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	switch (pCopyDataStruct->dwData)
	{
		case WM_SNIFFER_ADD:
		{
			CSyncPacketElement *pCurrent = &GetCACap()->mSniffer.mSnifferPackets.mPackets.back();
			//添加list
			int number = this->mList.GetItemCount();
			char buf[0x10] = { 0 };
			ultoa(number + 1, buf, 10);
			mList.InsertItem(number, buf);
			_ultoa(pCurrent->id, buf, 10);
			mList.SetItemText(number, 1, buf);
			sprintf(buf, "%04X", pCurrent->cmdid);
			mList.SetItemText(number, 2, buf);
			if (pCurrent->client2server)
			{
				mList.SetItemText(number, 3, "send");
			}else
				mList.SetItemText(number, 3, "recv");
			mList.SendMessage(WM_VSCROLL, SB_BOTTOM, NULL);
		}break;
		case WM_SEL_LIST:
		{
			int cur = *(int*)pCopyDataStruct->lpData;
			std::list<CSyncPacketElement>::iterator vtor;
			int j = 0;
			for (vtor = GetCACap()->mSniffer.mSnifferPackets.mPackets.begin(); vtor != GetCACap()->mSniffer.mSnifferPackets.mPackets.end(); vtor++, j++)
			{
				if (j == cur)
				{
					//找到当前项
					mHexEdit.SetData(vtor->_payload.size(), (BYTE*)vtor->_payload.contents());
					CString Key = "";
					cTrlKey.GetWindowText(Key);
					if (Key.GetLength() == 0) break;
					unsigned char *dst = NULL;
					unsigned int bytes = 0;
					std::string strHex = GetCACap()->StrToHex((BYTE*)Key.GetBuffer(0), Key.GetLength());
					GetCACap()->unEncrypt((unsigned char*)vtor->_payload.contents() + vtor->ciphertextoffset, vtor->ciphertextlen, (unsigned char*)strHex.c_str(), &dst, &bytes);
					mHexEditTxt.SetData(bytes, dst);
					free(dst);
					dst = NULL;
					break;
				}
			}
		}break;
		case WM_SNIFFER_PARENT:
		{
			CSendMsg* pSend = (CSendMsg*)pCopyDataStruct->lpData;
			pCopyDataStruct->dwData = WM_SNIFFER_SEND;
			if (pSend->pMsgDialog == NULL) break;
			CDialogMsg *diamsg = static_cast<CDialogMsg*>(pSend->pMsgDialog);
			pSend ->mNetInfo= GetCACap()->mSniffer.mSnifferPackets.mNetInfo;
			pSend->mMacInfo = GetCACap()->mSniffer.mSnifferPackets.mMacInfo;
			CString strKey = "";
			cTrlKey.GetWindowTextA(strKey);
			diamsg->SetKey(strKey);
			mQQctrl.GetWindowTextA(strKey);
			diamsg->SetQQ(strKey);
			diamsg->UpdateData(FALSE);
			diamsg->SendMessage(WM_COPYDATA, 0, (LPARAM)pCopyDataStruct);
		}break;
	}
	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}

void CDialogSniffer::SetKey(CString key)
{
	cTrlKey.SetWindowTextA(key);
}

void CDialogSniffer::SetQQ(CString qq)
{
	mQQctrl.SetWindowTextA(qq);
}

