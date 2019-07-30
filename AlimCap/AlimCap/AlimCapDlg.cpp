
// AlimCapDlg.cpp : 实现文件
//

#include "AlimCap.h"
#include "AlimCapDlg.h"
#include "afxdialogex.h"
#include"Markup.h"
#include"mymsg.h"
#include"SendMsg.h"
#include"stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAlimCapDlg 对话框




CAlimCapDlg::CAlimCapDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAlimCapDlg::IDD, pParent)
	, mCapFile(_T(""))
	, mPassWord(_T(""))
	, mSessionKey(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pPwdDialog = NULL;
	pSnifferDialog = NULL;
	pDialogDec = NULL;
	pDialogScanKey = NULL;
}

CAlimCapDlg::~CAlimCapDlg()
{
	if (NULL != pPwdDialog)
	{
		delete pPwdDialog;
		pPwdDialog = NULL;
	}
	if (NULL != pSnifferDialog)
	{
		delete pSnifferDialog;
		pSnifferDialog = NULL;
	}
	if (NULL != pDialogDec)
	{
		delete pDialogDec;
		pDialogDec = NULL;
	}
	if (NULL != pDialogScanKey)
	{
		delete pDialogScanKey;
		pDialogScanKey = NULL;
	}
}

void CAlimCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Text(pDX, IDC_MFCEDITBROWSE1, mCapFile);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, mFileEditBox);
	DDX_Text(pDX, IDC_EDIT1, mPassWord);
	//DDX_Text(pDX, IDC_EDIT2, mUserId);
	DDX_Control(pDX, IDC_COMBO2, cbSessions);
	DDX_Control(pDX, IDC_STATIC_RESULT, mResult);
	DDX_Control(pDX, IDC_IPADDRESS1, mFilterSrcIp);
	DDX_Control(pDX, IDC_IPADDRESS2, mFilterDstIp);
	DDX_Control(pDX, IDC_EDIT3, mFilterSrcPort);
	DDX_Control(pDX, IDC_EDIT4, mFilterDstPort);
	DDX_Text(pDX, IDC_EDIT5, mSessionKey);
	DDX_Control(pDX, IDC_COMBO1, mUserList);
}

BEGIN_MESSAGE_MAP(CAlimCapDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_MFCBUTTON2, &CAlimCapDlg::OnBnClickedMfcbutton2)
	ON_WM_COPYDATA()
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO3, OnBnClickedRadio)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CAlimCapDlg::OnSessionSelchange)
	ON_COMMAND(ID_32783, &CAlimCapDlg::OnMenuDec)
	ON_COMMAND(ID_32784, &CAlimCapDlg::OnSetPwd)
	ON_COMMAND(ID_32786, &CAlimCapDlg::OnSniferDialog)
	ON_COMMAND(ID_32787, &CAlimCapDlg::OnSendMsg)
	ON_COMMAND(ID_32794, &CAlimCapDlg::OnScanMemECDHKEY)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO1, &CAlimCapDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_CHECK5, &CAlimCapDlg::OnBnClickedCheck5)
END_MESSAGE_MAP()


// CAlimCapDlg 消息处理程序

BOOL CAlimCapDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	AfxOleInit();
	// TODO: 在此添加额外的初始化代码
	if (!GetCACap()->Init("test"))
	{
		AfxMessageBox("初始化失败");
		return TRUE;
	}
	//加载密码hash标
	GetLocalPwdHash();
	//设置UI
	mHexEdit = HexEditControl::ShowHexControl(m_hWnd, 16, 345, 767, 100);
	HexEditControl::SetData(mHexEdit, 0, 0);

	mHexEditText = HexEditControl::ShowHexControl(m_hWnd, 16, 450, 767, 100);
	HexEditControl::SetData(mHexEditText, 0, 0);

	//设置默认选中安卓
	CButton* radio = (CButton*)GetDlgItem(IDC_RADIO2);
	radio->SetCheck(1);
	OnBnClickedRadio(IDC_RADIO2);

	//默认生成分析报告
	radio = (CButton*)GetDlgItem(IDC_CHECK6);
	radio->SetCheck(1);
	
	//设置过滤条件
	((CButton*)GetDlgItem(IDC_CHECK1))->EnableWindow(0);
	((CButton*)GetDlgItem(IDC_CHECK2))->EnableWindow(0);
	((CButton*)GetDlgItem(IDC_CHECK3))->SetCheck(1);
	((CButton*)GetDlgItem(IDC_CHECK4))->SetCheck(1);

	mCapFile = "C:\\Users\\Administrator\\Desktop\\QQ\\1.pcap";
	this->UpdateData(FALSE);

	CRect rect;
	GetClientRect(&rect);     //取客户区大小  
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CAlimCapDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAlimCapDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CAlimCapDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//分析pcap文件
void CAlimCapDlg::OnBnClickedMfcbutton2()
{
	UpdateData(TRUE);
	if (((CButton*)GetDlgItem(IDC_CHECK6))->GetCheck())
	{
		GetCACap()->SetReport(true);
	}else
		GetCACap()->SetReport(false);

	GetFilterInfo();
	ClearControl();
	if (mCapFile.GetLength() == 0 || !PathFileExistsA(mCapFile.GetBuffer()))
	{
		//无效文件
		return;
	}
	pcap_t *fp = NULL;
	if (!(fp = GetCACap()->CapOpen(mCapFile)))
	{
		AfxMessageBox("Cap文件错误!");
		return;
	}
	//GetCACap()->SetFilter(fp, "tcp || udp");
	//解密SessionKey
	if (GetCACap()->mSessions.GetClientType() == ClientEnum::Enum_PC)
	{
		if (GetCACap()->ScanPCCap(fp))
		{
			GetCACap()->AnalysisPCPacket(GetCACap()->mSessions.mCachePackets, GetCACap()->mSessions);
			//加载sessions
			std::list<CSessionElement>::iterator vtor;
			for (vtor = GetCACap()->mSessions.mSessions.begin(); vtor != GetCACap()->mSessions.mSessions.end(); vtor++)
			{
				unsigned int _uuid=vtor->Getuuid();
				CString strSessionId = "";
				strSessionId.Format("%s", vtor->pUserInfo->GetMainId().szUserId);
				this->cbSessions.AddString(strSessionId);
				int index = cbSessions.GetCount()-1;
				cbSessions.SetItemData(index, _uuid);
			}
			this->cbSessions.SetCurSel(0);
			OnSessionSelchange();
		}
	}
	else
	{
		if (GetCACap()->ScanCapMobile(fp))
		{
			GetCACap()->AnalysisMBPacket(GetCACap()->mSessions.mCachePackets, GetCACap()->mSessions);
			//加载sessions
			std::list<CSessionElement>::iterator vtor;
			for (vtor = GetCACap()->mSessions.mSessions.begin(); vtor != GetCACap()->mSessions.mSessions.end(); vtor++)
			{
				unsigned int _uuid = vtor->Getuuid();
				CString strSessionId = "";
				strSessionId.Format("%s", vtor->pUserInfo->GetMainId().szUserId);
				this->cbSessions.AddString(strSessionId);
				int index = cbSessions.GetCount() - 1;
				cbSessions.SetItemData(index, _uuid);
			}
			this->cbSessions.SetCurSel(0);
			OnSessionSelchange();
		}
	}
	GetCACap()->CapClose(fp);
	UpdateData(FALSE);
}

BOOL CAlimCapDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// 进程间同步数据
	switch (pCopyDataStruct->dwData)
	{
	case 0:
		break;
	case WM_SEL_LIST:
		{
			//更新edit
			unsigned int uuid = cbSessions.GetItemData(cbSessions.GetCurSel());

			const CSessionElement *pCurSession = GetCACap()->mSessions.FindSession(uuid);
			if (NULL == pCurSession)
				break;
			unsigned int *num = (unsigned int *)pCopyDataStruct->lpData;
			if (*num <= pCurSession->mPackets.mPackets.size())
			{
				std::list<CSyncPacketElement>::const_iterator vtor = pCurSession->mPackets.mPackets.begin();
				std::advance(vtor, *num);

				//特殊数据包，如0825，0836，0828需要特殊处理
				UserKey mSessionKey0;

				if (((CButton*)GetDlgItem(IDC_CHECK5))->GetCheck())
				{
					UpdateData(TRUE);
					std::string strHex = GetCACap()->StrToHex((BYTE*)mSessionKey.GetBuffer(0), mSessionKey.GetLength());
					memcpy(mSessionKey0.SessionKey, strHex.c_str(), sizeof(mSessionKey0.SessionKey));
				}
				else
				{
					if (pCurSession->pUserInfo->KeysUinion.find(vtor->id) != pCurSession->pUserInfo->KeysUinion.end())
					{
						mSessionKey0 = pCurSession->pUserInfo->KeysUinion.find(vtor->id)->second;
					}
					std::string strSTR = GetCACap()->HexToStr(mSessionKey0.SessionKey, sizeof(mSessionKey0.SessionKey));
					mSessionKey.SetString(strSTR.c_str(), strSTR.length());
					UpdateData(FALSE);
				}

				HexEditControl::SetData(mHexEdit, (BYTE*)vtor->_payload.contents(), vtor->_payload.size());

				if (pCurSession->pUserInfo->mClientType == Enum_Android || pCurSession->pUserInfo->mClientType == Enum_IOS)
				{
					if (vtor->ciphertype == 1)
					{
						unsigned char *dst = NULL;
						unsigned int bytes = 0;
						GetCACap()->unEncrypt((unsigned char*)vtor->_payload.contents() + vtor->ciphertextoffset, vtor->ciphertextlen, mSessionKey0.SessionKey, &dst, &bytes);
						HexEditControl::SetData(mHexEditText, dst, bytes);
						free(dst);
						dst = NULL;
					}
					else if (vtor->ciphertype == 2)
					{
						unsigned char *dst = NULL;
						unsigned int bytes = 0;
						GetCACap()->unEncrypt((unsigned char*)vtor->_payload.contents() + vtor->ciphertextoffset, vtor->ciphertextlen, pCurSession->pUserInfo->Key0, &dst, &bytes);
						HexEditControl::SetData(mHexEditText, dst, bytes);
						free(dst);
						dst = NULL;
					}
					else if (vtor->ciphertype == 0)
					{
						HexEditControl::SetData(mHexEditText, (BYTE*)vtor->_payload.contents(), vtor->_payload.size());
					}
					else
					{
						AfxMessageBox("new ciphertype");
					}
				}
				else if (pCurSession->pUserInfo->mClientType == Enum_PC)
				{
					unsigned char *dst = NULL;
					unsigned int bytes = 0;
					GetCACap()->unEncrypt((unsigned char*)vtor->_payload.contents() + vtor->ciphertextoffset, vtor->ciphertextlen, mSessionKey0.SessionKey, &dst, &bytes);
					HexEditControl::SetData(mHexEditText, dst, bytes);
					free(dst);
					dst = NULL;
				}
			}
		}break;
		case WM_SAVE_PWD:
		{
			if (SaveLocalPwdHash())
			{
				AfxMessageBox("保存成功!");
			}
			else
			{
				AfxMessageBox("保存失败!");
			}
		}break;
		case WM_SNIFFER_PARENT:
		{
			CSendMsg* pSend = (CSendMsg*)pCopyDataStruct->lpData;
			pCopyDataStruct->dwData = WM_MAIN_SEND;
			if (pSend->pMsgDialog == NULL) break;

			unsigned int uuid = cbSessions.GetItemData(cbSessions.GetCurSel());
			const CSessionElement *pCurSession = GetCACap()->mSessions.FindSession(uuid);
			if (NULL == pCurSession)
				break;
			pSend->mNetInfo = pCurSession->mPackets.mNetInfo;
			pSend->mMacInfo = pCurSession->mPackets.mMacInfo;

			CDialogMsg *diamsg = static_cast<CDialogMsg*>(pSend->pMsgDialog);
			diamsg->SetKey(mSessionKey);
			CString strQQ;
			mUserList.GetLBText(mUserList.GetCurSel(), strQQ);
			diamsg->SetQQ(strQQ);
			diamsg->UpdateData(FALSE);
			diamsg->SendMessage(WM_COPYDATA, 0, (LPARAM)pCopyDataStruct);
		}break;
		case WM_DECRYPTSAVE_PARENT:
		{
			unsigned int uuid = cbSessions.GetItemData(cbSessions.GetCurSel());
			const CSessionElement *pCurSession = GetCACap()->mSessions.FindSession(uuid);
			if (NULL == pCurSession)
				break;
			std::list<CSyncPacketElement>::const_iterator vtor = pCurSession->mPackets.mPackets.begin();
			for (; vtor != pCurSession->mPackets.mPackets.end(); vtor++)
			{
				unsigned char *dst = NULL;
				unsigned int bytes = 0;
				UserKey mKey;
				if (pCurSession->pUserInfo->KeysUinion.find(vtor->id) != pCurSession->pUserInfo->KeysUinion.end())
					mKey = pCurSession->pUserInfo->KeysUinion.find(vtor->id)->second;

				GetCACap()->unEncrypt((unsigned char*)(vtor->_payload.contents() + vtor->ciphertextoffset), vtor->ciphertextlen, mKey.SessionKey, &dst, &bytes);
				free(dst);
				dst = NULL;
			}
		}break;
	}
	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}

void CAlimCapDlg::ClearControl()
{
	GetCACap()->ReleaseSession();
	//devCBbox.ResetContent();
	cbSessions.ResetContent();
	mSessionKey="";
	m_List.DeleteAllItems();
	HexEditControl::SetData(mHexEdit,0, 0);
	HexEditControl::SetData(mHexEditText,0,0);
	UpdateData(FALSE);
}


bool CAlimCapDlg::GetLocalPwdHash()
{
	CMarkup xml;
	bool bLoad = false;

	char chpath[MAX_PATH] = {0};
	GetModuleFileName(NULL, (LPSTR)chpath, sizeof(chpath));
	PathRemoveFileSpec(chpath);
	StrCat(chpath, "\\PwdHashs.xml");


	bLoad = xml.Load(chpath);
	if (!bLoad)
	{
		//printf("没有找到XML文件!\n");
		return false;
	}
	xml.ResetMainPos();//把xml对象指向的位置初始化，使其指向文件开始
	xml.FindElem();//查找任意元素，此处查找到的第一个元素即为根元素
	xml.IntoElem();//进入Root

	xml.FindElem("users");
	xml.IntoElem();
	while (xml.FindElem("user"))//不能使用if，因为要遍历所有的Student元素
	{
		std::string strId=xml.GetAttrib("id");
		std::string strtype = xml.GetAttrib("type");

		xml.FindChildElem("pwdhash");
		xml.IntoElem();
		unsigned int id = strtoll(strId.c_str(),0,10);
		std::string strpwd = xml.GetData();
		strpwd = GetCACap()->StrToHex((BYTE*)strpwd.c_str(), strpwd.size());

		xml.FindElem("ShareKey");
		std::string strsharekey = xml.GetData();
		strsharekey = GetCACap()->StrToHex((BYTE*)strsharekey.c_str(), strsharekey.size());

		xml.FindElem("SessionKey");
		std::string strsessionkey = xml.GetData();
		strsessionkey = GetCACap()->StrToHex((BYTE*)strsessionkey.c_str(), strsessionkey.size());

		IdKeyUnion mKeyUnion;
		mKeyUnion.id = id;
		mKeyUnion.isValid = false;
		if (strtype=="0")
			mKeyUnion.cType = ClientEnum::Enum_PC;
		else if (strtype == "1")
			mKeyUnion.cType = ClientEnum::Enum_Android;
		else if (strtype == "2")
			mKeyUnion.cType = ClientEnum::Enum_IOS;
		else
			mKeyUnion.cType = ClientEnum::Enum_Unkonw;

		mKeyUnion.PwdHash = strpwd;
		mKeyUnion.ShareKey = strsharekey;
		mKeyUnion.SessionKey = strsessionkey;
		GetCACap()->mUserKeys.AddItem(mKeyUnion);
		//返回上一层
		xml.OutOfElem();
	}
	return true;
}

bool CAlimCapDlg::SaveLocalPwdHash()
{
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
		return false;
	}
	//删除所有节点
	xml.ResetPos();//把xml对象指向的位置初始化，使其指向文件开始
	xml.FindElem();//查找任意元素，此处查找到的第一个元素即为根元素
	xml.IntoElem();//进入Root
	xml.FindElem("users");
	xml.IntoElem();
	while (xml.FindElem("user"))//不能使用if，因为要遍历所有的Student元素
	{
		xml.RemoveElem();
	}


	//插入当前节点列表
	xml.ResetPos();//把xml对象指向的位置初始化，使其指向文件开始
	xml.FindElem();//查找任意元素，此处查找到的第一个元素即为根元素
	xml.IntoElem();//进入Root
	xml.FindElem("users");

	std::list<IdKeyUnion>::iterator itor = GetCACap()->mUserKeys._IdKeyUnions.begin();
	for (; itor != GetCACap()->mUserKeys._IdKeyUnions.end(); itor++)
	{
		xml.AddChildElem("user");
		xml.IntoElem();
		char buf[0x10] = { 0 };
		ultoa(itor->id, buf, 10);
		xml.SetAttrib("id", buf);
		ultoa(itor->cType, buf, 10);
		xml.SetAttrib("type", buf);
		std::string strSTR = GetCACap()->HexToStr((BYTE*)itor->PwdHash.c_str(), itor->PwdHash.size());
		xml.AddChildElem("pwdhash", strSTR.c_str());
		strSTR = GetCACap()->HexToStr((BYTE*)itor->ShareKey.c_str(), itor->ShareKey.size());
		xml.AddChildElem("ShareKey", strSTR.c_str());
		strSTR = GetCACap()->HexToStr((BYTE*)itor->SessionKey.c_str(), itor->SessionKey.size());
		xml.AddChildElem("SessionKey", strSTR.c_str());
		xml.OutOfElem();
	}
	xml.Save(chpath);
	return true;
}

//切换端设备类型
void CAlimCapDlg::OnBnClickedRadio(UINT idCtl)
{
	//清除表头
	CHeaderCtrl *pHeaderCtrl = &m_List.GetHeaderCtrl();
	if (pHeaderCtrl != NULL)
	{
		int  nColumnCount = pHeaderCtrl->GetItemCount();
		for (int i = 0; i<nColumnCount; i++)
		{
			m_List.DeleteColumn(0);
		}
	}

	if (idCtl == IDC_RADIO1)
	{
		//pc
		GetCACap()->mSessions.SetClientType(Enum_PC);
		
		char _column[][MAX_HEADLENGTH] = { "序号", "id","cmdid", "收发行为", "包序号" };
		this->m_List.SetHeaders(_column, sizeof(_column) / sizeof(*_column));
		this->m_List.SetColumnWidth(0, 50);
		this->m_List.SetColumnWidth(1, 100);
		this->m_List.EnsureVisible(m_List.GetItemCount() - 1, FALSE);
		mFilterDstPort.SetWindowText(_T("8000"));
	}
	if (idCtl == IDC_RADIO2)
	{
		//安卓
		GetCACap()->mSessions.SetClientType(Enum_Android);

		char _column[][MAX_HEADLENGTH] = { "序号","id","SSOVersion[1]" ,"serviceCmd", "收发行为", "包序号" };
		this->m_List.SetHeaders(_column, sizeof(_column) / sizeof(*_column));
		this->m_List.SetColumnWidth(0, 50);
		this->m_List.SetColumnWidth(1, 100);
		this->m_List.EnsureVisible(m_List.GetItemCount() - 1, FALSE);
		mFilterDstPort.SetWindowText(_T("8080"));
	}
	if (idCtl == IDC_RADIO3)
	{
		//苹果
		GetCACap()->mSessions.SetClientType(Enum_IOS);

		char _column[][MAX_HEADLENGTH] = { "序号", "id","SSOVersion[1]" ,"serviceCmd", "收发行为", "包序号" };
		this->m_List.SetHeaders(_column, sizeof(_column) / sizeof(*_column));
		this->m_List.SetColumnWidth(0, 50);
		this->m_List.SetColumnWidth(1, 100);
		this->m_List.EnsureVisible(m_List.GetItemCount() - 1, FALSE);
		mFilterDstPort.SetWindowText(_T("8080"));
	}
		
}

//session改变
void CAlimCapDlg::OnSessionSelchange()
{
	// TODO:  在此添加控件通知处理程序代码
	this->m_List.DeleteAllItems();
	HexEditControl::SetData(mHexEdit, 0, 0);
	HexEditControl::SetData(mHexEditText, 0, 0);

	unsigned int uuid =  cbSessions.GetItemData(cbSessions.GetCurSel());
	const CSessionElement *pCurSession = GetCACap()->mSessions.FindSession(uuid);
	if (NULL == pCurSession)
		return;

	if (!GetCACap()->mUserKeys.isFindValid(pCurSession->pUserInfo->GetMainId().id, pCurSession->pUserInfo->GetClientType()))
	{
		mResult.SetWindowText(_T("无法找到密码组!"));
		return;
	}

	std::string strHEX = GetCACap()->mUserKeys.GetItemValid(pCurSession->pUserInfo->GetMainId().id, pCurSession->pUserInfo->GetClientType())->PwdHash;
	std::string strSTR = CAnalysisCap::HexToStr((BYTE*)strHEX.c_str(), strHEX.size());
	this->mPassWord.SetString(strSTR.c_str(), strSTR.size());

	mUserList.ResetContent();
	std::list<UserId> mUsers = pCurSession->pUserInfo->GetId();
	std::list<UserId>::iterator itor_users = mUsers.begin();
	for (; itor_users != mUsers.end(); itor_users++)
	{
		mUserList.AddString((char*)itor_users->szUserId);
	}

	if (!pCurSession->pUserInfo->isCanDecrypt)
	{
		mResult.SetWindowText(_T("SessionKey无法拿到!"));
	}
	else
	{
		mResult.SetWindowText(_T("SessionKey解密正常!"));
	}

	std::list<CSyncPacketElement>::const_iterator vtor;
	for (vtor = pCurSession->mPackets.mPackets.begin(); vtor != pCurSession->mPackets.mPackets.end(); vtor++)
	{
		int number = this->m_List.GetItemCount();

		if (GetCACap()->mSessions.GetClientType() == Enum_PC)
		{
			char buf[0x10] = { 0 };
			ultoa(number + 1, buf, 10);
			m_List.InsertItem(number, buf);

			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%u", vtor->id);
			m_List.SetItemText(number, 1, buf);

			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%04X", vtor->cmdid);
			m_List.SetItemText(number, 2, buf);

			if (vtor->client2server)
			{
				m_List.SetItemText(number, 3, "send");
			}
			else
				m_List.SetItemText(number, 3, "recv");

			memset(buf, 0, sizeof(buf));
			ultoa(vtor->inPacketsNo, buf, 10);
			m_List.SetItemText(number, 4, buf);

		}
		else
		{
			char buf[0x10] = { 0 };
			ultoa(number + 1, buf, 10);
			m_List.InsertItem(number, buf);

			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%u", vtor->id);
			m_List.SetItemText(number, 1, buf);

			memset(buf, 0, sizeof(buf));
			sprintf(buf, "%u", vtor->ciphertype);
			m_List.SetItemText(number, 2, buf);

			m_List.SetItemText(number, 3, vtor->serviceCmd.c_str());

			if (vtor->client2server)
			{
				m_List.SetItemText(number, 4, "send");
			}
			else
				m_List.SetItemText(number, 4, "recv");

			memset(buf, 0, sizeof(buf));
			ultoa(vtor->inPacketsNo, buf, 10);
			m_List.SetItemText(number, 5, buf);
		}
	}

	UpdateData(FALSE);
}

//解密
void CAlimCapDlg::OnMenuDec()
{
	// TODO:  在此添加命令处理程序代码
	if (NULL == pDialogDec)
	{
		pDialogDec = new CDialogDec();
		pDialogDec->SetKey(mSessionKey);
		pDialogDec->Create(IDD_DIALOG_DEC, this);
	}
	pDialogDec->ShowWindow(SW_SHOW);
}

//设置密码串
void CAlimCapDlg::OnSetPwd()
{
	// TODO:  在此添加命令处理程序代码
	if (NULL==pPwdDialog)
	{
		pPwdDialog = new CDialogPwd();
		pPwdDialog->Create(IDD_DIALOG_PWD,this);
	}
	pPwdDialog->ShowWindow(SW_SHOW);
}

//sniffer
void CAlimCapDlg::OnSniferDialog()
{
	// TODO:  在此添加命令处理程序代码
	if (NULL == pSnifferDialog)
	{
		pSnifferDialog = new CDialogSniffer();
		pSnifferDialog->Create(IDD_DIALOG1_SNIFFER, this);
	}
	pSnifferDialog->SetKey(mSessionKey);
	CString strQQ;
	mUserList.GetLBText(mUserList.GetCurSel(), strQQ);
	pSnifferDialog->SetQQ(strQQ);
	pSnifferDialog->ShowWindow(SW_SHOW);
}

//直接发送消息
void CAlimCapDlg::OnSendMsg()
{
	// TODO:  在此添加命令处理程序代码
	//if (NULL == diamsg)
	//{
	//	diamsg = new CDialogMsg();
	//	diamsg->Create(IDD_DIALOG_MSG, this);
	//}
	//diamsg->SetKey(mSessionKey);
	//diamsg->SetQQ(mUserId);
	//diamsg->ShowWindow(SW_SHOW);
}

void CAlimCapDlg::OnScanMemECDHKEY()
{
	// TODO:  在此添加命令处理程序代码
	if (NULL == pDialogScanKey)
	{
		pDialogScanKey = new CScanMemKeyDialog();
		pDialogScanKey->Create(IDD_SCANMEM_ECDHKEY, this);
	}
	pDialogScanKey->ShowWindow(SW_SHOW);
}


void CAlimCapDlg::ReSize()
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小
	CRect recta;
	GetClientRect(&recta);     //取客户区大小  
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	CPoint OldTLPoint, TLPoint; //左上角
	CPoint OldBRPoint, BRPoint; //右下角
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件  
	while (hwndChild)
	{
		::GetWindowRect(hwndChild, Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x*fsp[0]);
		TLPoint.y = long(OldTLPoint.y*fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x *fsp[0]);
		BRPoint.y = long(OldBRPoint.y *fsp[1]);
		::MoveWindow(hwndChild, TLPoint.x, TLPoint.y, BRPoint.x - TLPoint.x, BRPoint.y - TLPoint.y, true);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old = Newp;
}

void CAlimCapDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	if (nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		ReSize();
	}
}

void CAlimCapDlg::GetFilterInfo()
{
	CButton* check1 = (CButton*)GetDlgItem(IDC_CHECK1);
	CButton* check2 = (CButton*)GetDlgItem(IDC_CHECK2);
	CButton* check3 = (CButton*)GetDlgItem(IDC_CHECK3);
	CButton* check4 = (CButton*)GetDlgItem(IDC_CHECK4);
	if (check1->GetCheck())
	{
		DWORD ip;
		mFilterSrcIp.GetAddress(ip);
		if (ip != 0){
			GetCACap()->mSessions.mCachePackets.mFilterInfo.srcip = ip;
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcIpCheck = true;
		}else
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcIpCheck = false;
	}
	else
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcIpCheck = false;
	if (check2->GetCheck())
	{
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcPortCheck = true;
		CString strPort = "";
		mFilterSrcPort.GetWindowText(strPort);
		unsigned short port = StrToInt(strPort);
		if (port != 0)
		{
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcPortCheck = true;
			GetCACap()->mSessions.mCachePackets.mFilterInfo.srcport = port;
		}else
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcPortCheck = false;
	}
	else
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isSrcPortCheck = false;
	if (check3->GetCheck())
	{
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstIpCheck = true;
		DWORD ip;
		mFilterDstIp.GetAddress(ip);
		if (ip != 0)
		{
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstIpCheck = true;
			GetCACap()->mSessions.mCachePackets.mFilterInfo.dstip = ip;
		}else
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstIpCheck = false;
	}
	else
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstIpCheck = false;
	if (check4->GetCheck())
	{
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstPortCheck = true;
		CString strPort = "";
		mFilterDstPort.GetWindowText(strPort);
		unsigned short port = StrToInt(strPort);
		if (port != 0)
		{
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstPortCheck = true;
			GetCACap()->mSessions.mCachePackets.mFilterInfo.dstport = port;
		}else
			GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstPortCheck = false;
	}
	else
		GetCACap()->mSessions.mCachePackets.mFilterInfo.isDstPortCheck = false;
}

//切换账号
void CAlimCapDlg::OnCbnSelchangeCombo1()
{
	unsigned int uuid = cbSessions.GetItemData(cbSessions.GetCurSel());
	const CSessionElement *pCurSession = GetCACap()->mSessions.FindSession(uuid);
	if (NULL == pCurSession)
		return;

	CString strId = "";
	mUserList.GetLBText(mUserList.GetCurSel(), strId);
	unsigned int id = StrToInt(strId);

	UserKey mKey;
	if (pCurSession->pUserInfo->KeysUinion.find(id) != pCurSession->pUserInfo->KeysUinion.end())
	{
		mKey = pCurSession->pUserInfo->KeysUinion.find(id)->second;
	}
	std::string strSTR = CAnalysisCap::HexToStr((BYTE*)mKey.SessionKey, sizeof(mKey.SessionKey));
	this->mSessionKey.SetString(strSTR.c_str(), strSTR.size());

	UpdateData(FALSE);
}


void CAlimCapDlg::OnBnClickedCheck5()
{
	// TODO:  在此添加控件通知处理程序代码
	CButton* check5 = (CButton*)GetDlgItem(IDC_CHECK5);
	if (check5->GetCheck())
	{
		CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT5);
		edit->SetReadOnly(FALSE);
	}
	else
	{
		CEdit* edit = (CEdit*)GetDlgItem(IDC_EDIT5);
		edit->SetReadOnly(TRUE);
	}
}
