// ScanMemKeyDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "AlimCap.h"
#include "ScanMemKeyDialog.h"
#include "afxdialogex.h"
#include"mymsg.h"


// CScanMemKeyDialog 对话框

IMPLEMENT_DYNAMIC(CScanMemKeyDialog, CDialogEx)

CScanMemKeyDialog::CScanMemKeyDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CScanMemKeyDialog::IDD, pParent)
{

}

CScanMemKeyDialog::~CScanMemKeyDialog()
{
}

void CScanMemKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, mProcess);
	DDX_Control(pDX, IDC_LIST1, mList);
}


BEGIN_MESSAGE_MAP(CScanMemKeyDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CScanMemKeyDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CScanMemKeyDialog::OnBnClickedButton2)
	ON_LBN_DBLCLK(IDC_LIST1, &CScanMemKeyDialog::OnLbnDblclkList1)
END_MESSAGE_MAP()


// CScanMemKeyDialog 消息处理程序


BOOL CScanMemKeyDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CenterWindow();
	TestProcessList();

	m_Mytip.Create(this);
	m_Mytip.AddTool(GetDlgItem(IDC_LIST1), "双击复制");
	m_Mytip.SetDelayTime(TTDT_INITIAL, 10);
	//鼠标保持指向，提示显示多久，毫秒
	m_Mytip.SetDelayTime(TTDT_AUTOPOP, 30000);
	//m_Mytip.SetTipTextColor(RGB(0, 0, 255)); //设置提示文本的颜色
	//m_Mytip.SetTipBkColor(RGB(255, 255, 255)); //设置提示框的背景颜色
	m_Mytip.Activate(TRUE); //设置是否启用提示

	((CButton*)this->GetDlgItem(IDC_RADIO1))->SetCheck(true);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}

//扫描
void CScanMemKeyDialog::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码

	int cur = this->mProcess.GetCurSel();
	DWORD pid = processlist[cur].th32ProcessID;
	DWORD ctrl = (DWORD)&mList;
	unsigned int *arr = new unsigned int[3];
	arr[0] = pid;
	arr[1] = ctrl;
	if (((CButton*)this->GetDlgItem(IDC_RADIO1))->GetCheck())
	{
		arr[2] = 0;
	}
	else if (((CButton*)this->GetDlgItem(IDC_RADIO2))->GetCheck())
	{
		arr[2] = 1;
	}
	else if (((CButton*)this->GetDlgItem(IDC_RADIO3))->GetCheck())
	{
		arr[2] = 2;
	}else
		arr[2] = 0;

	_beginthreadex(NULL, 0, ThreadFunc1, arr, 0, NULL);

}


unsigned int __stdcall CScanMemKeyDialog::ThreadFunc1(void*pArguments)
{
	unsigned int *p = (unsigned int*)pArguments;
	DWORD th32ProcessID = *(p);
	CListBox *ctrl = (CListBox*)(*(p + 1));
	if (th32ProcessID)
	{
		HANDLE hProcessHandle;
		hProcessHandle = ::OpenProcess(PROCESS_VM_READ, FALSE,th32ProcessID);
		if (hProcessHandle == INVALID_HANDLE_VALUE)
		{
			AfxMessageBox("无法打开进程");
			return -1;
		}
		if (*(p + 2) == 0)
		{
			char *buffer = (char*)malloc(0x10000);
			memset(buffer, 0, 0x10000);
			char src[0xF] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00 };
			std::list<std::string> findkey;
			for (DWORD64 memoryAddr = 0x10000; memoryAddr<0x80000000; memoryAddr += 0x10000)
			{
				DWORD read = 0;
				ReadProcessMemory(hProcessHandle, (LPVOID)memoryAddr, buffer, 0x10000, &read);
				if (read > 0)
				{
					for (int i = 0; i < 0x10000 - 0xF; i++)
					{
						if (memcmp(buffer + i, src, sizeof(src)) == 0)
						{
							i += 0xF;
							if (buffer[i + 1] == 0 && buffer[i + 3] == 0) continue;
							ctrl->AddString(CAnalysisCap::HexToStr((BYTE*)buffer + i, 0x18).c_str());
							ctrl->PostMessageA(WM_VSCROLL, SB_LINEDOWN, 0);
						}
					}
				}
				else
				{
					continue;
				}

			}
		}
		else if (*(p + 2) == 1)
		{
			char *buffer = (char*)malloc(0x10000);
			memset(buffer, 0, 0x10000);
			char src[0xF] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00 };
			std::list<std::string> findkey;
			for (DWORD64 memoryAddr = 0x10000; memoryAddr<0x80000000; memoryAddr += 0x10000)
			{
				DWORD read = 0;
				ReadProcessMemory(hProcessHandle, (LPVOID)memoryAddr, buffer, 0x10000, &read);
				if (read > 0)
				{
					for (int i = 0; i < 0x10000 - 0xF; i++)
					{
						if (memcmp(buffer + i, src, sizeof(src)) == 0)
						{
							i += 0xF;
							if (buffer[i + 1] == 0 && buffer[i + 3] == 0) continue;
							ctrl->AddString(CAnalysisCap::HexToStr((BYTE*)buffer + i, 0x19).c_str());
							ctrl->PostMessageA(WM_VSCROLL, SB_LINEDOWN, 0);
						}
					}
				}
				else
				{
					continue;
				}

			}
		}
		else if (*(p + 2) == 2)
		{
		}
		ctrl->AddString("完成...");
		ctrl->PostMessageA(WM_VSCROLL, SB_LINEDOWN, 0);
		CloseHandle(hProcessHandle);
		AfxMessageBox("扫描完毕");
	}
	delete[]p;
	return 0;
}

int CScanMemKeyDialog::TestProcessList()
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	int count = 0;

	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("创建Toolhelp32Snapshot失败\n");
		return -1;
	}

	BOOL bMore = ::Process32First(hProcessSnap, &pe32);

	while (bMore)
	{
		count++;
		if (strcmp(pe32.szExeFile, "QQ.exe") == 0 || strcmp(pe32.szExeFile, "TIM.exe") == 0)
		{
			processlist.push_back(pe32);
			mProcess.AddString(pe32.szExeFile);
		}
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	this->mProcess.SetCurSel(0);
	::CloseHandle(hProcessSnap);

	return 0;
}

void CScanMemKeyDialog::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	//int cur = this->mList.GetCurSel();
	//CString strText = "";
	//mList.GetText(cur, strText);
	//std::string strHex = GetCACap()->StrToHex((BYTE*)strText.GetBuffer(0), strText.GetLength());
	//GetCACap()->mEcdhKeys.push_back(strHex);
	//AfxMessageBox("添加成功");
}


void CScanMemKeyDialog::OnLbnDblclkList1()
{
	// TODO:  在此添加控件通知处理程序代码
	int cur = this->mList.GetCurSel();
	CString strText = "";
	mList.GetText(cur, strText);

	BOOL bret = OpenClipboard();
	EmptyClipboard();
	int len = strText.GetLength();
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, len + 1);
	char *p = (char*)GlobalLock(hGlobal);
	memset(p, 0, len + 1);
	memcpy(p, strText.GetBuffer(), len);
	bret = GlobalUnlock(hGlobal);
	HANDLE hResult = SetClipboardData(CF_TEXT, hGlobal);
	CloseClipboard();
}


BOOL CScanMemKeyDialog::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	if (pMsg->message == WM_MOUSEMOVE && pMsg->hwnd == GetDlgItem(IDC_LIST1)->GetSafeHwnd())
		m_Mytip.RelayEvent(pMsg);
	return CDialogEx::PreTranslateMessage(pMsg);
}
