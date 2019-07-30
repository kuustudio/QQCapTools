// DialogPwd.cpp : 实现文件
//

#include "AlimCap.h"
#include "DialogPwd.h"
#include "afxdialogex.h"
#include "AnalysisCap.h"
#include "mymsg.h"
#include"Markup.h"
#include"AlimCapDlg.h"

#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))

// CDialogPwd 对话框

IMPLEMENT_DYNAMIC(CDialogPwd, CDialogEx)

CDialogPwd::CDialogPwd(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogPwd::IDD, pParent)
	, mQQ(_T(""))
	, mPwd(_T(""))
	, mShareKey(_T(""))
	, mSessionKey(_T(""))
	, mAddShareKey(_T(""))
{

}

CDialogPwd::~CDialogPwd()
{
}

void CDialogPwd::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, mQQ);
	DDX_Text(pDX, IDC_EDIT8, mPwd);
	DDX_Control(pDX, IDC_LIST1, mPwdList);
	DDX_Control(pDX, IDC_LIST2, mCurPwdList);
	DDX_Text(pDX, IDC_EDIT3, mShareKey);
	DDX_Text(pDX, IDC_EDIT2, mSessionKey);
	DDX_Text(pDX, IDC_EDIT5, mAddShareKey);
	DDX_Control(pDX, IDC_LIST3, mCurSessionKeys);
	DDX_Control(pDX, IDC_COMBO2, mClientType);
}


BEGIN_MESSAGE_MAP(CDialogPwd, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDialogPwd::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDialogPwd::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDialogPwd::OnBnClickedButton3)
	ON_WM_SHOWWINDOW()
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


// CDialogPwd 消息处理程序

BOOL CDialogPwd::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CenterWindow();

	char _column1[][MAX_HEADLENGTH] = { "序号", "端类型","版本","ShareKey","备注" };
	mPwdList.SetMainType(0x10,2);
	mPwdList.SetHeaders(_column1, sizeof(_column1) / sizeof(*_column1));
	mPwdList.SetColumnWidth(0,50);
	mPwdList.SetColumnWidth(1,80);
	mPwdList.EnsureVisible(mPwdList.GetItemCount() - 1, FALSE);

	char _column2[][MAX_HEADLENGTH] = { "设为当前", "QQ","端类型", "密码串", "ShareKey", "SessionKey" };
	mCurPwdList.SetMainType(0x11, 2);
	mCurPwdList.SetHeaders(_column2, sizeof(_column2) / sizeof(*_column2));
	mCurPwdList.SetColumnWidth(0, 60);
	mCurPwdList.SetColumnWidth(1, 80);
	mCurPwdList.EnsureVisible(mPwdList.GetItemCount() - 1, FALSE);
	mCurPwdList.SetExtendedStyle(mCurPwdList.GetExtendedStyle() | LVS_EX_CHECKBOXES);

	char _column3[][MAX_HEADLENGTH] = { "序号", "QQ", "SessionKey" };
	mCurSessionKeys.SetMainType(0x12, 2);
	mCurSessionKeys.SetHeaders(_column3, sizeof(_column3) / sizeof(*_column3));
	mCurSessionKeys.SetColumnWidth(0, 50);
	mCurSessionKeys.SetColumnWidth(1, 80);
	mCurSessionKeys.EnsureVisible(mPwdList.GetItemCount() - 1, FALSE);

	mClientType.AddString("PC");
	mClientType.AddString("安卓");
	mClientType.AddString("IOS");
	mClientType.AddString("未知");

	InitListData();
	return TRUE;
}

//手动添加密码
void CDialogPwd::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);

	u_int64 qq = atoll(mQQ.GetBuffer(0));
	CString strType="";
	mClientType.GetLBText(mClientType.GetCurSel(), strType);
	ClientEnum cType=Enum_Unkonw;
	if (strType == "PC")
		cType = Enum_PC;
	else if (strType == "安卓")
		cType = Enum_Android;
	else if (strType = "IOS")
		cType = Enum_IOS;
	std::string strHEXPwdHash = "";
	if (mPwd != "")
	{
		ByteBuffer data;
		data.append(mPwd.GetBuffer(0), mPwd.GetLength());
		ByteBuffer md5;
		CAnalysisCap::GetMd5(data, md5);
		md5.append(htonll(qq));
		data.clear();
		CAnalysisCap::GetMd5(md5, data);
		std::string strSTR = CAnalysisCap::HexToStr((BYTE*)data.contents(), data.size());
		strHEXPwdHash.append((char*)data.contents(), data.size());
	}

	std::string strHEXShareKey = CAnalysisCap::StrToHex((BYTE*)mShareKey.GetBuffer(), mShareKey.GetLength());
	std::string strHEXSessionKey = CAnalysisCap::StrToHex((BYTE*)mSessionKey.GetBuffer(), mSessionKey.GetLength());
	
	
	if (strHEXPwdHash == "")
	{
		AfxMessageBox("密码不能为空");
		return;
	}
	IdKeyUnion mKeyUnion;
	mKeyUnion.id = qq;
	mKeyUnion.cType = cType;
	mKeyUnion.isValid = false;
	mKeyUnion.PwdHash = strHEXPwdHash;
	mKeyUnion.ShareKey = strHEXShareKey;
	mKeyUnion.SessionKey = strHEXSessionKey;
	GetCACap()->mUserKeys.AddItem(mKeyUnion);

	int number = this->mCurPwdList.GetItemCount();
	char buf[0x10] = { 0 };
	//ultoa(number + 1, buf, 10);
	mCurPwdList.InsertItem(number, "");
	mCurPwdList.SetItemData(number, mKeyUnion.Getuuid());

	mCurPwdList.SetCheck(number, false);

	ultoa(mKeyUnion.id, buf, 10);
	mCurPwdList.SetItemText(number, 1, buf);
	switch (mKeyUnion.cType)
	{
	case Enum_PC:
	{
					mCurPwdList.SetItemText(number, 2, "PC");
	}break;
	case Enum_Android:
	{
						 mCurPwdList.SetItemText(number, 2, "安卓");
	}break;
	case Enum_IOS:
	{
					 mCurPwdList.SetItemText(number, 2, "IOS");
	}break;
	default:
		mCurPwdList.SetItemText(number, 2, "未知");
		break;
	}
	std::string strSTR = CAnalysisCap::HexToStr((BYTE*)mKeyUnion.PwdHash.c_str(), mKeyUnion.PwdHash.size());
	mCurPwdList.SetItemText(number, 3, strSTR.c_str());
	strSTR = CAnalysisCap::HexToStr((BYTE*)mKeyUnion.ShareKey.c_str(), mKeyUnion.ShareKey.size());
	mCurPwdList.SetItemText(number, 4, strSTR.c_str());
	strSTR = CAnalysisCap::HexToStr((BYTE*)mKeyUnion.SessionKey.c_str(), mKeyUnion.SessionKey.size());
	mCurPwdList.SetItemText(number, 5, strSTR.c_str());

}

//添加sharekey
void CDialogPwd::OnBnClickedButton2()
{
	int number = this->mCurPwdList.GetItemCount();
	char buf[0x10] = { 0 };
	ultoa(number, buf, 10);
	mPwdList.InsertItem(number, buf);
	mPwdList.SetItemText(number-1, 1, "安卓");
	mPwdList.SetItemText(number-1, 2, "7.1.8");
	mPwdList.SetItemText(number-1, 3, "345668");
}

//保存到本地文件
void CDialogPwd::OnBnClickedButton3()
{
	//保存到文件，然后主程序可以加载
	HWND msgHandle = GetParent()->GetSafeHwnd();
	COPYDATASTRUCT cpd;
	cpd.dwData = WM_SAVE_PWD;                                                                                                         //标志发送字符串
	::SendMessage(msgHandle, WM_COPYDATA, 0, (LPARAM)&cpd);
}

//加载本地缓存
void CDialogPwd::InitListData()
{
	mPwdList.Clear();

	CMarkup xml;
	bool bLoad = false;

	char chpath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, (LPSTR)chpath, sizeof(chpath));
	PathRemoveFileSpec(chpath);
	StrCat(chpath, "\\PwdHashs.xml");


	bLoad = xml.Load(chpath);
	if (!bLoad)
	{
		//printf("没有找到XML文件!\n");
		return;
	}
	xml.ResetMainPos();//把xml对象指向的位置初始化，使其指向文件开始
	xml.FindElem();//查找任意元素，此处查找到的第一个元素即为根元素
	xml.IntoElem();//进入Root

	xml.FindElem("ECDH");
	xml.IntoElem();
	int number = 0;
	while (xml.FindElem("ShareKeys"))
	{
		xml.IntoElem();

		char buf[0x10] = { 0 };
		ultoa(number + 1, buf, 10);
		mPwdList.InsertItem(number, buf);

		CString strValue;//元素内容
		xml.FindChildElem("type");
		strValue = xml.GetData();
		if (strValue=="0")
			mPwdList.SetItemText(number, 1, "PC");
		else if (strValue == "1")
			mPwdList.SetItemText(number, 1, "安卓");
		else if (strValue=="2")
			mPwdList.SetItemText(number, 1, "IOS");
		else
			mPwdList.SetItemText(number, 1, "未知");

		xml.FindElem("ver");
		strValue = xml.GetData();
		mPwdList.SetItemText(number, 2, strValue);

		xml.FindElem("key");
		strValue = xml.GetData();
		mPwdList.SetItemText(number, 3, strValue);

		number++;
		xml.OutOfElem();
	}

	//加载本地缓存密码
	mCurPwdList.Clear();
	std::list<IdKeyUnion>::iterator itr = GetCACap()->mUserKeys._IdKeyUnions.begin();
	for (; itr != GetCACap()->mUserKeys._IdKeyUnions.end(); itr++)
	{
		int number = this->mCurPwdList.GetItemCount();
		char buf[0x10] = { 0 };
		//ultoa(number + 1, buf, 10);
		mCurPwdList.InsertItem(number, "");
		mCurPwdList.SetItemData(number, itr->Getuuid());

		if (itr->isValid)
			mCurPwdList.SetCheck(number, true);
		else
			mCurPwdList.SetCheck(number, false);

		ultoa(itr->id, buf, 10);
		mCurPwdList.SetItemText(number, 1, buf);
		switch (itr->cType)
		{
		case Enum_PC:
		{
						mCurPwdList.SetItemText(number, 2, "PC");
		}break;
		case Enum_Android:
		{
							 mCurPwdList.SetItemText(number, 2, "安卓");
		}break;
		case Enum_IOS:
		{
						 mCurPwdList.SetItemText(number, 2, "IOS");
		}break;
		default:
			mCurPwdList.SetItemText(number, 2, "未知");
			break;
		}
		std::string strSTR = CAnalysisCap::HexToStr((BYTE*)itr->PwdHash.c_str(), itr->PwdHash.size());
		mCurPwdList.SetItemText(number, 3, strSTR.c_str());
		strSTR = CAnalysisCap::HexToStr((BYTE*)itr->ShareKey.c_str(), itr->ShareKey.size());
		mCurPwdList.SetItemText(number, 4, strSTR.c_str());
		strSTR = CAnalysisCap::HexToStr((BYTE*)itr->SessionKey.c_str(), itr->SessionKey.size());
		mCurPwdList.SetItemText(number, 5, strSTR.c_str());
	}


}

//加载当前的会话密钥
void CDialogPwd::SetRefreshList()
{
	mCurSessionKeys.Clear();
	std::list<CSessionElement>::const_iterator itor = GetCACap()->mSessions.mSessions.begin();
	for (int i = 0; itor != GetCACap()->mSessions.mSessions.end(); itor++, i++)
	{
		std::map<unsigned int, UserKey>::const_iterator itor_map = itor->pUserInfo->KeysUinion.begin();
		for (; itor_map != itor->pUserInfo->KeysUinion.end(); itor_map++)
		{
			int number = mCurSessionKeys.GetItemCount();
			char buf[0x10] = { 0 };
			ultoa(number + 1, buf, 10);
			mCurSessionKeys.InsertItem(number, buf);
			ultoa(itor_map->first, buf, 10);
			mCurSessionKeys.SetItemText(number, 1, buf);
			std::string strSTR = GetCACap()->HexToStr((BYTE*)itor_map->second.SessionKey, sizeof(itor_map->second.SessionKey));
			mCurSessionKeys.SetItemText(number, 2, strSTR.c_str());
		}
	}
}

void CDialogPwd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);
	SetRefreshList();
}

BOOL CDialogPwd::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	MyListCtrl *p = static_cast<MyListCtrl *>(pWnd);
	switch (pCopyDataStruct->dwData)
	{
		case WM_SEL_LIST:
		{
			unsigned int *data =(unsigned int*)pCopyDataStruct->lpData;
			if (p != NULL)
			{
				if (p->GetMainType() == 0x10)
				{
					CString sharekey = mPwdList.GetItemText(data[0], 3);
					mAddShareKey = sharekey;
					
				}
				else if (p->GetMainType() == 0x11)
				{
					CString strQQ = mCurPwdList.GetItemText(data[0], 1);
					mQQ = strQQ;
					CString strType = mCurPwdList.GetItemText(data[0], 2);
					if (strType == "PC")
						mClientType.SetCurSel(0);
					else if (strType == "安卓")
						mClientType.SetCurSel(1);
					else if (strType == "IOS")
						mClientType.SetCurSel(2);
					else
						mClientType.SetCurSel(3);
					CString strPwd = mCurPwdList.GetItemText(data[0], 3);
					//mPwd = strPwd;
					CString strSharekey = mCurPwdList.GetItemText(data[0], 4);
					mShareKey = strSharekey;
					CString strSessionKey = mCurPwdList.GetItemText(data[0], 5);
					mSessionKey = strSessionKey;
				}
				else if (p->GetMainType() == 0x12)
				{
				}
			}
			UpdateData(FALSE);
		}break;
		case WM_SEL_CHECK:
		{
			unsigned int *data = (unsigned int*)pCopyDataStruct->lpData;
			if (p != NULL)
			{
				if (p->GetMainType() == 0x10)
				{

				}
				else if (p->GetMainType() == 0x11)
				{
					CString strQQ = mCurPwdList.GetItemText(data[0], 1);
					unsigned int qq = StrToInt(strQQ);
					CString strType = mCurPwdList.GetItemText(data[0], 2);
					ClientEnum cType = ClientEnum::Enum_Unkonw;
					if (strType == "PC")
						cType = ClientEnum::Enum_PC;
					else if (strType == "安卓")
						cType = ClientEnum::Enum_Android;
					else if (strType == "IOS")
						cType = ClientEnum::Enum_IOS;
					if (data[2] == 0)
					{
						//unchecked
						if (GetCACap()->mUserKeys.isFind(qq, cType))
						{
							//找到
							GetCACap()->mUserKeys.GetItem(qq, cType)->isValid = false;
						}
					}
					else if (data[2] == 1)
					{
						//checked
						if (GetCACap()->mUserKeys.isFindValid(qq, cType))
						{
							//找到
							AfxMessageBox("不能重复设置");
							mCurPwdList.SetCheck(data[0], false);
						}
						else
						{
							//未找到
							GetCACap()->mUserKeys.GetItem(qq, cType)->isValid = true;

						}
					}
				}
				else if (p->GetMainType() == 0x12)
				{
				}
			}
		}break;
		case WM_DELETE_ITEM:
		{
			unsigned int *data = (unsigned int*)pCopyDataStruct->lpData;
			if (p != NULL)
			{
				p->DeleteItem(data[0]);
				if (GetCACap()->mUserKeys.isFindByuuid(data[1]))
				{
					GetCACap()->mUserKeys.DeleteItem(GetCACap()->mUserKeys.GetItemByuuid(data[1]));
				}

			}
				
		}break;
	}
	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}

CAnalysisCap* CDialogPwd::GetCACap()
{
	CAlimCapDlg *pMainDlg = static_cast<CAlimCapDlg*>(AfxGetMainWnd());
	return &pMainDlg->CACap;
}
