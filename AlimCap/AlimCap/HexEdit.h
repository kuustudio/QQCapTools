//////////////////////////////////////////////////////////////////////////////
// This source code and all associated files and resources are copyrighted by
// the author(s). This source code and all associated files and resources may
// be used as long as they are used according to the terms and conditions set
// forth in The Code Project Open License (CPOL), which may be viewed at
// http://www.blackbeltcoder.com/Legal/Licenses/CPOL.
//
// Copyright (c) 2010 Jonathan Wood
//

#pragma once
#include<afxole.h>
#include<string>

// Internal structure
typedef struct _COLUMN_METRICS {
	int left;
	int right;
} COLUMN_METRICS, *LPCOLUMN_METRICS;

// Selection modes
#define HEX_SEL_CLEAR 0
#define HEX_SEL_IGNORE 1
#define HEX_SEL_EXTEND 2

// Update hints
#define HEX_HINT_ALL 0
#define HEX_HINT_LINE 1
#define HEX_HINT_LINES 2
#define HEX_HINT_RANGE 3

//CopyPaste Type
#define CT_Binary		0
#define CT_BinaryString	1
#define CT_HEXString	2
#define CT_View			3


//////////////////////////////////////////////////////////////////////////////
// CHexEdit

class CHexEdit : public CWnd
{
public:
	CHexEdit(void);
	virtual ~CHexEdit(void);

	// Public interface members
	int GetDataLength() { return m_nLength; }
	int SetData(int nLength, BYTE* pBuffer);
	int GetData(int nMaxLen, BYTE* pBuffer);
	int GetColumn() { return m_bHexCol; }
	void SetColumn(int nColumn) { m_bHexCol = (nColumn != 0); }
	int GetReadOnly() { return m_bReadOnly; }
	void SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; }
	ULONG GetSelection();
	void SetSelection(int nSelStart = 0, int nSelEnd = 0);
	int InsertData(int nCount);
	int DeleteData(int nCount);
	int GetPosition() { return m_nPosition; }
	int SetPosition(int nPos);
	BOOL GetWideView() { return (BOOL)(m_nBytesPerRow == 0x10); }
	void SetWideView(BOOL bWideView);
	BOOL GetInsertMode() { return m_bInsertMode; }
	void SetInsertMode(BOOL bMode, BOOL bToggle = TRUE);
	void LimitLength(int nLimit);
	void ShowAllAscii(BOOL bShowAllAscii);

protected:

	// Internal implementation members
	static BOOL RegisterWndClass();

	void SetDisplayMetrics();
	void ClearAll();
	int GetClientRows();
	int GetClientColumns();
	int Insert(int nCount);
	int Delete(int nCount);

	int OffsetFromPoint(CPoint point);
	void GetCaretXY(POINT& point);
	void SetCaret(POINT& point);
	void SetCaret();
	void SetCurrPos(int nPosition, int nSelAction = HEX_SEL_CLEAR, int bInByte = 0);
	void SetVerScroll(int nTopPosition);
	void SetHorScroll(int nTopPosition);

	BOOL HasSelection() { return m_nSelStart != m_nSelEnd; }
	int GetSelLength() { return m_nSelEnd - m_nSelStart; }
	void DeleteSelection();

	BOOL CanPaste();
	COleDataSource* CreateDataSource(unsigned int type);
	BOOL DoPasteData(COleDataObject* pDataObject,unsigned int type);
	unsigned int GetPasteData(unsigned char *buffer,unsigned int len);
	unsigned int SetPasteData(unsigned char *dstbuffer, unsigned char *srcBuffer,unsigned int len);
	void HexToStr(BYTE *pbSrc, int nLen, BYTE*dstbuffer);
	void StrToHex(BYTE *pbSrc, int nLen, BYTE*dstbuffer);
	unsigned int SetPasteView(unsigned char *dstbuffer, unsigned char *srcBuffer, unsigned int len);

	void GetOffsetString(TCHAR* sBuffer, int nOffset);
	void GetHexString(TCHAR* sBuffer, BYTE* lpBuffer, int nCount);
	void GetChrString(TCHAR* sBuffer, BYTE* lpBuffer, int nCount);

	CSize GetClientSize();
	void Update(int nHint = HEX_HINT_ALL, int nStart = 0, int nEnd = 0);
	void UpdateScrollbar();

protected:

	// Data members
	static const TCHAR m_szWndClassName[];
	static UINT m_cfFormat;
	static BOOL m_bIsRegistered;

	BYTE* m_pBuffer;
	
	int m_nLength;
	int m_nHorMaxLength;
	int m_nHorMinLength;
	int m_nPosition;
	int m_nTopPosition;
	int m_nRightPosition;
	BOOL m_bInByte;
	BOOL m_bCaretIsVisible;
	BOOL m_bTrailingCaret;
	int m_nSelStart, m_nSelEnd;
	int m_nSelAnchor;
	int m_nTimerID;

	CSize m_CharSize;
	COLUMN_METRICS m_ColMetrics[3];

	BOOL m_bInsertMode;
	BOOL m_bInsertToggle;
	BOOL m_bHexCol;
	BOOL m_bReadOnly;
	int m_nBytesPerRow;
	int m_nLimit;
	BOOL m_bShowAllChars;

protected:

	// Generated message map functions
	afx_msg void OnPaint();
	afx_msg UINT OnGetDlgCode();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnEditCut();
	afx_msg void OnEditDelete();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditSelectAll();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnCopyBinary();
	afx_msg void OnCopyString();
	afx_msg void OnCopyHexstring();
	afx_msg void OnPasteBinary();
	afx_msg void OnPasteString();

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnPasteHexstring();
	afx_msg void OnCopyView();
};
