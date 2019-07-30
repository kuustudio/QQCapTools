//////////////////////////////////////////////////////////////////////////////
// This source code and all associated files and resources are copyrighted by
// the author(s). This source code and all associated files and resources may
// be used as long as they are used according to the terms and conditions set
// forth in The Code Project Open License (CPOL), which may be viewed at
// http://www.blackbeltcoder.com/Legal/Licenses/CPOL.
//
// Copyright (c) 2010 Jonathan Wood
//

#include "resource.h"
#include "HexEdit.h"

#define COL_OFFSET 0
#define COL_HEX 1
#define COL_ASCII 2


//////////////////////////////////////////////////////////////////////////////
// CHexEdit

// Control class name
const _TCHAR CHexEdit::m_szWndClassName[] = _T("SoftCircuitsHexEdit");

// Clipboard format ID
UINT CHexEdit::m_cfFormat = RegisterClipboardFormat(_T("SoftCircuitsBinary"));

// Flag to indicate class is registered
// As a static variable, this causes RegisterWndClass() to be called
// automatically during application start up
BOOL CHexEdit::m_bIsRegistered = CHexEdit::RegisterWndClass();

// Out of memory message
static const TCHAR szNoMem[] = _T("Out of memory : Insufficient memory to resize edit buffer.");

// Hex character lookup table
static const TCHAR szHexDigits[] = _T("0123456789ABCDEF");


/////////////////////////////////////////////////////////////////////////////
// CHexEdit

BEGIN_MESSAGE_MAP(CHexEdit, CWnd)
	ON_WM_PAINT()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CHexEdit::OnEditSelectAll)
	ON_WM_HSCROLL()
	ON_COMMAND(ID_COPY_BINARY, &CHexEdit::OnCopyBinary)
	ON_COMMAND(ID_COPY_STRING, &CHexEdit::OnCopyString)
	ON_COMMAND(ID_PASTE_BINARY, &CHexEdit::OnPasteBinary)
	ON_COMMAND(ID_PASTE_STRING, &CHexEdit::OnPasteString)
	ON_COMMAND(ID_COPY_HEXSTRING, &CHexEdit::OnCopyHexstring)
	ON_COMMAND(ID_PASTE_HEXSTRING, &CHexEdit::OnPasteHexstring)
	ON_COMMAND(ID_COPY_VIEW, &CHexEdit::OnCopyView)
END_MESSAGE_MAP()


CHexEdit::CHexEdit()
{
	// Initialize
	m_pBuffer = NULL;
	ClearAll();

	m_CharSize = CSize(0,0);
	memset(m_ColMetrics, '\0', sizeof(m_ColMetrics));

	m_bInsertMode = TRUE;
	m_bInsertToggle = TRUE;	// Insert key toggles insert mode
	m_bHexCol = TRUE;
	m_bReadOnly = FALSE;
	m_nBytesPerRow = 0x08;
	m_nLimit = 0xFFFF;	// Default max bytes

	m_nTimerID = 0;

	m_bShowAllChars = FALSE;

	SetDisplayMetrics();
}

CHexEdit::~CHexEdit()
{
	ClearAll();
}


/////////////////////////////////////////////////////////////////////////////
// Public methods

// Sets the current data
// Return value: New length of data (which is less than nLength if error)
int CHexEdit::SetData(int nLength, BYTE* pBuffer)
{
	ASSERT(nLength >= 0);

	// Clear existing data
	ClearAll();

	// Insert as much of requested bytes as we can
	int nCount = Insert(nLength);

	// Copy data to local buffer
	if (nCount != 0)
		memcpy(m_pBuffer, pBuffer, nCount);

	// Repaint entire window
	Update();

	return m_nLength;
}

// Returns the current data
// Return value: Number of bytes copied
int CHexEdit::GetData(int nMaxLen, BYTE* pBuffer)
{
	int i = min(nMaxLen, m_nLength);
	memcpy((BYTE*)pBuffer, m_pBuffer, i);
	return i;
}

// Returns the current selection range
// Return Value: Selection start in low word, end in high word
ULONG CHexEdit::GetSelection()
{
	return HasSelection() ? MAKELRESULT(m_nSelStart, m_nSelEnd) : 0;
}

// Inserts the specified number of bytes at the current position.
// Inserted bytes are not initialized and their content is
// undefined. The return value is the number of bytes inserted
// which may be less than requested if the current length limit
// was reached.
// Return Value: Number of bytes inserted.
int CHexEdit::InsertData(int nCount)
{
	ASSERT(nCount >= 0);
	int nLength = Insert(nCount);
	if (nLength > 0)
		Update(HEX_HINT_LINES, m_nPosition);
	return nLength;
}

// Deletes the specified number of bytes at the current position.
// The return value is the number of bytes deleteted which may be
// less than requested if the end of the data was reached.
// Return Value: Number of bytes deleted.
int CHexEdit::DeleteData(int nCount)
{
	ASSERT(nCount >= 0);
	int nLength = Delete(nCount);
	if (nLength > 0)
		Update(HEX_HINT_LINES, m_nPosition);
	return nLength;
}

// Sets the caret 0-based position. Returns the new position
// which may differ from the requested position if it was out
// of range. Any selection is cleared.
// Return Value: New position
int CHexEdit::SetPosition(int nCount)
{
	ASSERT(nCount >= 0);
	SetCurrPos(nCount);
	return m_nPosition;
}

// Determine if the control displays 16 bytes per row (TRUE) or
// 8 bytes per row (FALSE)
void CHexEdit::SetWideView(BOOL bWideView)
{
	int nBytesPerRow = (bWideView) ? 0x10 : 0x08;
	if (nBytesPerRow != m_nBytesPerRow)
	{
		// Set new bytes per row
		m_nBytesPerRow = nBytesPerRow;
		// Update screen
		SetDisplayMetrics();
		// Force top position to even line boundary
		SetVerScroll(m_nTopPosition);
		Update();
		SetCurrPos(m_nPosition, HEX_SEL_EXTEND);
		UpdateScrollbar();
	}
}

// Returns TRUE if control displays 16 bytes per row, or FALSE
// if control displays 8 bytes per row
void CHexEdit::SetInsertMode(BOOL bMode, BOOL bToggle)
{
	m_bInsertMode = (BOOL)bMode;
	m_bInsertToggle = (BOOL)bToggle;
}

// Limits the amount of data that can be entered into the control
void CHexEdit::LimitLength(int nLimit)
{
	// Maximum length is limited to 65,535 bytes
	ASSERT(nLimit >= 0 && nLimit <= 0xFFFF);
	m_nLimit = nLimit;
}

// Specifies if non-printable characters are shown in the ASCII
// column.
void CHexEdit::ShowAllAscii(BOOL bShowAllAscii)
{
	m_bShowAllChars = bShowAllAscii;
	Update();
}


/////////////////////////////////////////////////////////////////////////////
// CHexEdit support routines

// Registers our Windows class
BOOL CHexEdit::RegisterWndClass()
{
	WNDCLASS wndclass;

	if (m_bIsRegistered)
		return TRUE;

	// Register new window control class
	wndclass.style = 0;
	wndclass.lpfnWndProc =::DefWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	wndclass.hIcon = NULL;
	wndclass.hCursor = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);
	wndclass.hbrBackground = NULL;

	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = m_szWndClassName;

	return AfxRegisterClass(&wndclass);
}

void CHexEdit::SetDisplayMetrics()
{
	// Calculate font and column measurements
	CClientDC dc(NULL);
	TEXTMETRIC tm;

	// Select fo
	CFont* pOldFont = (CFont*)dc.SelectStockObject(ANSI_FIXED_FONT);

	// Calculate character and column metrics
	dc.GetTextMetrics(&tm);
	m_CharSize.cx = tm.tmAveCharWidth;
	m_CharSize.cy = tm.tmHeight + tm.tmExternalLeading;
	m_ColMetrics[COL_OFFSET].left = m_CharSize.cx / 2;
	m_ColMetrics[COL_OFFSET].right = m_ColMetrics[COL_OFFSET].left + (m_CharSize.cx * 4);
	m_ColMetrics[COL_HEX].left = m_ColMetrics[COL_OFFSET].right + ((m_CharSize.cx * 3) / 2);
	m_ColMetrics[COL_HEX].right = m_ColMetrics[COL_HEX].left + (m_CharSize.cx * ((m_nBytesPerRow * 3) - 1));
	m_ColMetrics[COL_ASCII].left = m_ColMetrics[COL_HEX].right + ((m_CharSize.cx * 3) / 2);
	m_ColMetrics[COL_ASCII].right = m_ColMetrics[COL_ASCII].left + (m_CharSize.cx * m_nBytesPerRow);
	dc.SelectObject(pOldFont);
}

void CHexEdit::ClearAll()
{
	// Free buffer memory
	if (m_pBuffer != NULL)
		free(m_pBuffer);

	m_pBuffer = NULL;
	m_nLength = 0;

	m_nPosition = m_nTopPosition = 0;
	m_nRightPosition = 0;
	m_bInByte = 0;
	m_bTrailingCaret = FALSE;
	m_nSelAnchor = 0;

	m_nSelStart = m_nSelEnd = 0;
}

// Inserts the specified number of bytes at the current position
int CHexEdit::Insert(int nCount)
{
	ASSERT(nCount >= 0);

	// Test for length limit
	if ((m_nLength + nCount) > m_nLimit)
		nCount = max(0, (m_nLimit - m_nLength));

	if (nCount > 0)
	{
		BYTE* pTemp = (BYTE*)realloc(m_pBuffer, m_nLength + nCount);
		if (pTemp == NULL)
		{
			AfxMessageBox(szNoMem);
			ClearAll();
			// Invalidate entire window
			Update();
			nCount = 0;
		}
		else
		{
			m_pBuffer = pTemp;
			// Move bytes up to make room for new data
			memmove(m_pBuffer + m_nPosition + nCount, m_pBuffer + m_nPosition, m_nLength - m_nPosition);
			m_nLength += nCount;
		}
	}
	return nCount;
}

// Deletes the specified number of bytes from the current position
int CHexEdit::Delete(int nCount)
{
	ASSERT(nCount >= 0);

	if (nCount > 0)
	{
		nCount = min(nCount, m_nLength - m_nPosition);
		// Move bytes down to fill deleted ones
		m_nLength -= nCount;
		memmove(m_pBuffer + m_nPosition, m_pBuffer + m_nPosition + nCount, m_nLength - m_nPosition);
		m_pBuffer = (BYTE*)realloc(m_pBuffer, m_nLength);
		// NOTE: m_pBuffer could be NULL if m_nLength == 0
		ASSERT( m_pBuffer != NULL || m_nLength == 0);
	}
	return nCount;
}

// Determines the buffer offset position from the given point
int CHexEdit::OffsetFromPoint(CPoint point)
{
	m_bTrailingCaret = FALSE;

	// Determine hex or ASCII column
	if (point.x <= (m_ColMetrics[COL_ASCII].left - ((m_CharSize.cx * 3) / 4)))
	{
		m_bHexCol = TRUE;
		point.x += m_CharSize.cx;
		point.x = (point.x - m_ColMetrics[COL_HEX].left) / (m_CharSize.cx * 3);
	}
	else
	{
		m_bHexCol = FALSE;
		point.x += (m_CharSize.cx >> 1);
		point.x = (point.x - m_ColMetrics[COL_ASCII].left) / m_CharSize.cx;
	}
	// Keep column position within range
	if (point.x < 0)
		point.x = 0;
	else if (point.x >= m_nBytesPerRow)
	{
		m_bTrailingCaret = TRUE;
		point.x = m_nBytesPerRow;
	}
	// Determine row position
	point.y = m_nTopPosition + ((point.y / m_CharSize.cy) * m_nBytesPerRow);
	// Keep row position in range
	if (point.y < 0)
		point.y = 0;
	else if (point.y > (m_nLength & ~(m_nBytesPerRow - 1)))
		point.y = (m_nLength & ~(m_nBytesPerRow - 1));
	ASSERT((point.y + point.x) >= 0);
	return min(point.y + point.x, m_nLength);
}

// Calculates the caret position from the current buffer offset
void CHexEdit::GetCaretXY(POINT& point)
{
	int nHexFudge = 0;
	int nWinOffset = (m_nPosition - m_nTopPosition);

	if (nWinOffset >= 0)
	{
		point.x = (nWinOffset % m_nBytesPerRow);
		point.y = (nWinOffset / m_nBytesPerRow);
		if (m_bTrailingCaret)
		{
			if (point.x == 0 && m_nPosition != 0 && !m_bInByte)
			{
				point.x += m_nBytesPerRow;
				point.y--;
				nHexFudge = m_CharSize.cx;
			}
			else m_bTrailingCaret = FALSE;
		}
	}
	else
	{
		// Position off-screen
		point.x = 0;
		point.y = -1;
	}
	point.x *= m_CharSize.cx;
	point.y *= m_CharSize.cy;
	if (m_bHexCol)
	{
		point.x *= 3;
		point.x += m_ColMetrics[COL_HEX].left - nHexFudge;
		if (m_bInByte) point.x += m_CharSize.cx;
	}
	else
	{
		point.x += m_ColMetrics[COL_ASCII].left;
	}
}

// Positions the caret
void CHexEdit::SetCaret(POINT& point)
{
	CRect rect;
	GetClientRect(&rect);

	if (rect.PtInRect(point))
	{
		SetCaretPos(point);
		if (!m_bCaretIsVisible)
		{
			ShowCaret();
			m_bCaretIsVisible = TRUE;
		}
	}
	else
	{
		if (m_bCaretIsVisible)
		{
			HideCaret();
			m_bCaretIsVisible = FALSE;
		}
	}
}

// Sets the selection, or highlight, range
void CHexEdit::SetSelection(int nStart, int nEnd)
{
	// Static structure we return the address of
	int nUpdateStart, nUpdateEnd;

	// Normalize selection range
	if (nStart > nEnd)
	{
		int nTemp = nStart;
		nStart = nEnd;
		nEnd = nTemp;
	}

	// Lucidity check
	ASSERT(nStart >= 0 && nStart <= m_nLength);
	ASSERT(nEnd >= 0 && nEnd <= m_nLength);

	// Just return if selection hasn't changed
	if (m_nSelStart == nStart && m_nSelEnd == nEnd)
		return;

	if (nStart == nEnd)
	{
		// Update old selection
		nUpdateStart = m_nSelStart;
		nUpdateEnd = m_nSelEnd;
		// Clear selection
		m_nSelStart = m_nSelEnd = 0;
	}
	else
	{
		if (!HasSelection())
		{
			// No old selection, just update new one
			nUpdateStart = nStart;
			nUpdateEnd = nEnd;
		}
		else
		{
			// Determine minimum update range (for fast update)
			nUpdateStart = (nStart != m_nSelStart) ? min(nStart, m_nSelStart)
				: min(nEnd, m_nSelEnd);
			nUpdateEnd = (nEnd != m_nSelEnd) ? max(nEnd, m_nSelEnd)
				: max(nStart, m_nSelStart);
		}
		m_nSelStart = nStart;
		m_nSelEnd = nEnd;
	}
	// Update window
	if (nUpdateStart != nUpdateEnd)
		Update(HEX_HINT_RANGE, nUpdateStart, nUpdateEnd);
}

// Sets the caret position scrolling and updating the window as needed
void CHexEdit::SetCurrPos(int nPosition, int nSelAction, int bInByte)
{
	POINT point;

	int nOldPosition = m_nPosition;

	m_nPosition = nPosition;

	// Keep position within valid range
	if (m_nPosition < 0)
	{
		m_nPosition = 0;
		m_bTrailingCaret = FALSE;
	}
	else if (m_nPosition > m_nLength)
	{
		m_nPosition = m_nLength;
		m_bTrailingCaret = FALSE;
	}

	m_bInByte = bInByte;

	// Scroll if needed
	if (m_bTrailingCaret)
	{
		if ((m_nPosition - 1) < m_nTopPosition)
			SetVerScroll(m_nPosition - 1);
		else if (m_nPosition > (m_nTopPosition + (GetClientRows() * m_nBytesPerRow)))
			SetVerScroll(m_nPosition - (GetClientRows() * m_nBytesPerRow));
	}
	else
	{
		if (m_nPosition < m_nTopPosition)
			SetVerScroll(m_nPosition);
		else if (m_nPosition >= (m_nTopPosition + (GetClientRows() * m_nBytesPerRow)))
			SetVerScroll(m_nPosition - ((GetClientRows() - 1) * m_nBytesPerRow));
	}

	// Update or clear selection if requested
	if (nSelAction == HEX_SEL_EXTEND)
	{
		// Extend selection
		if (!HasSelection())
			m_nSelAnchor = nOldPosition;
		SetSelection(m_nSelAnchor, m_nPosition);
	}
	else if (nSelAction == HEX_SEL_CLEAR && HasSelection())
	{
		// Clear selection
		SetSelection();
	}

	GetCaretXY(point);
	SetCaret(point);
}

// Scrolls to the specified scroll position
void CHexEdit::SetVerScroll(int nTopPosition)
{
	// Keep position within valid range
	if (nTopPosition < 0)
		nTopPosition = 0;
	else if (nTopPosition > m_nLength)
		nTopPosition = m_nLength;

	// Keep top row position on even line boundary
	nTopPosition &= ~(m_nBytesPerRow - 1);

	// Has scroll position changed?
	if (nTopPosition != m_nTopPosition)
	{
		// Calculate rows to scroll (negative scrolls up)
		int cyScroll = ((m_nTopPosition - nTopPosition) / m_nBytesPerRow);
		// Convert to pixels
		cyScroll *= m_CharSize.cy;
		// Update top row position
		m_nTopPosition = nTopPosition;

		CRect Rect;
		GetClientRect(&Rect);

		// Update window for new scroll position
		if (abs(cyScroll) < Rect.Height())
			ScrollWindow(0, cyScroll, NULL, NULL);
		else
			Update();
	}
}

void CHexEdit::SetHorScroll(int nRightPosition)
{
	// Keep position within valid range
	if (nRightPosition < 0)
		nRightPosition = 0;
	else if (nRightPosition > m_nHorMaxLength - m_nHorMinLength+1)
		nRightPosition = m_nHorMaxLength - m_nHorMinLength+1;

	// Keep top row position on even line boundary
	//nRightPosition &= ~(m_nBytesPerRow - 1);

	// Has scroll position changed?
	if (nRightPosition != m_nRightPosition)
	{
		// Calculate rows to scroll (negative scrolls up)
		int cxScroll = m_nRightPosition-nRightPosition;
		// Convert to pixels
		cxScroll *= m_CharSize.cx;
		// Update top row position
		m_nRightPosition = nRightPosition;
		CRect Rect;
		GetClientRect(&Rect);

		// Update window for new scroll position
		if (abs(cxScroll) < Rect.Width())
			ScrollWindow(cxScroll, 0, NULL, NULL);
		else
			Update();
	}
}

// Returns the size of the client window
CSize CHexEdit::GetClientSize()
{
	CRect Rect;
	CSize Size;
	GetClientRect(&Rect);
	Size.cx = Rect.Width();
	Size.cy = Rect.Height();
	return Size;
}

//
void CHexEdit::Update(int nHint, int nStart, int nEnd)
{
	CRect Rect;
	CSize Size;

	switch (nHint)
	{
		case HEX_HINT_ALL:
			// Update entire window
			Invalidate();
			break;

		case HEX_HINT_LINE:
			// Update line containing nOffset
			Size = GetClientSize();
			nStart &= ~(m_nBytesPerRow - 1);
			Rect.top = (((nStart - m_nTopPosition) / m_nBytesPerRow) * m_CharSize.cy);
			if (Rect.top >= 0 && Rect.top < Size.cy)
			{
				if (Rect.top < 0)
					Rect.top = 0;
				// Fill out rest of update rectangle
				Rect.bottom = (Rect.top + m_CharSize.cy);
				Rect.left = 0;
				Rect.right = Size.cx;
				// Invalidate rectangle
				InvalidateRect(&Rect);
			}
			break;

		case HEX_HINT_LINES:
			// Update line containing nOffset and all lines that follow
			Size = GetClientSize();
			nStart &= ~(m_nBytesPerRow - 1);
			Rect.top = (((nStart - m_nTopPosition) / m_nBytesPerRow) * m_CharSize.cy);
			if (Rect.top < Size.cy)
			{
				if (Rect.top < 0)
					Rect.top = 0;
				// Fill out rest of update rectangle
				Rect.bottom = Size.cy;
				Rect.left = 0;
				Rect.right = Size.cx;
				// Invalidate rectangle
				InvalidateRect(&Rect);
			}
			break;

		case HEX_HINT_RANGE:
			// Update lines containing range
			Size = GetClientSize();
			nStart &= ~(m_nBytesPerRow - 1);
			nEnd = (nEnd + (m_nBytesPerRow - 1)) & ~(m_nBytesPerRow - 1);
			ASSERT(nStart <= nEnd);
			Rect.top = (((nStart - m_nTopPosition) / m_nBytesPerRow) * m_CharSize.cy);
			Rect.bottom = (((nEnd - m_nTopPosition) / m_nBytesPerRow) * m_CharSize.cy);
			if (Rect.top < Size.cy && Rect.bottom >= 0)
			{
				if (Rect.top < 0)
					Rect.top = 0;
				if (Rect.bottom > Size.cy)
					Rect.bottom = Size.cy;
				// Fill out rest of update rectangle
				Rect.left = 0;
				Rect.right = Size.cx;
				// Invalidate rectangle
				InvalidateRect(&Rect);
			}
			break;

		default:
			ASSERT(FALSE);
	}
}

// Returns the number of text rows
int CHexEdit::GetClientRows()
{
	CRect rc;
	GetClientRect(&rc);
	return (rc.Height() / m_CharSize.cy);
}

// Returns the number of text columns
int CHexEdit::GetClientColumns()
{
	CRect rc;
	GetClientRect(&rc);
	return (rc.Width() / m_CharSize.cx);
}

// Positions the caret to the current position
void CHexEdit::SetCaret()
{
	POINT point;
	GetCaretXY(point);
	SetCaret(point);
}


/////////////////////////////////////////////////////////////////////////////
// CHexEdit message handlers

void CHexEdit::OnPaint() 
{
	TCHAR szBuffer[50];
	int cyRow, nCount, nPosition;

	CPaintDC dc(this); // device context for painting

	// Select font
	CFont* pOldFont = (CFont*)dc.SelectStockObject(ANSI_FIXED_FONT);

	RECT rect;
	dc.GetClipBox(&rect);

	COLORREF rgbNormal = GetSysColor(COLOR_WINDOW);
	COLORREF rgbHighlight = GetSysColor(COLOR_HIGHLIGHT);
	COLORREF rgbNormalText = GetSysColor(COLOR_WINDOWTEXT);
	COLORREF rgbHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);

	dc.SetBkMode(OPAQUE);
	
	nCount = rect.top / m_CharSize.cy;
	cyRow = nCount * m_CharSize.cy;
	nPosition = m_nTopPosition + (nCount * m_nBytesPerRow);

	// Use separate draw loop if we are showing selection
	if (HasSelection())
	{
		ASSERT(m_nSelStart < m_nSelEnd);
		ASSERT(m_nSelEnd <= m_nLength);

		static int nBoundaries[4];
		int nBoundary;

		// Set up boundaries to indicate where selection starts/ends
		nBoundaries[0] = m_nSelStart;
		nBoundaries[1] = m_nSelEnd;
		nBoundaries[2] = m_nLength;
		nBoundaries[3] = m_nLength + m_nBytesPerRow;

		// Find current boundary
		for (nBoundary = 0; nPosition >= nBoundaries[nBoundary]; nBoundary++)
			;

		// Print rows of data
		while (cyRow < rect.bottom && nPosition <= m_nLength)
		{
			static TCHAR szHexStr[160], szAsciiStr[160];

			// Use normal color to display offset
			dc.SetBkColor(rgbNormal);
			dc.SetTextColor(rgbNormalText);

			// Print line offset
			GetOffsetString(szBuffer, nPosition);
			dc.ExtTextOut(m_ColMetrics[COL_OFFSET].left, cyRow, 0, NULL, szBuffer, 4, NULL);

			// Get line length
			nCount = min(m_nBytesPerRow, m_nLength - nPosition);

			// Get hexadecimal and ASCII strings
			GetHexString(szHexStr, m_pBuffer + nPosition, nCount);
			if (nCount > (m_nBytesPerRow / 2)) szHexStr[((m_nBytesPerRow / 2) * 3) - 1] = ' ';
			GetChrString(szAsciiStr, m_pBuffer + nPosition, nCount);

			// Print data for this row
			int i, nChunk, nLeading = 0, nSubTrailing = 1;

			for (i = 0; i < m_nBytesPerRow; i += nChunk)
			{
				// Check if we reached a new boundary?
				while ((nPosition + i) >= nBoundaries[nBoundary])
					nBoundary++;

				// Set text color for this piece of data
				if (nBoundary == 1)
				{
					dc.SetTextColor(rgbHighlightText);
					dc.SetBkColor(rgbHighlight);
				}
				else
				{
					dc.SetTextColor(rgbNormalText);
					dc.SetBkColor(rgbNormal);
					// If we are folling selection, paint space
					// before this byte in hex column
					if (i > 0)
						nLeading = 1;
				}

				// Get number of bytes remaining on this line
				nChunk = m_nBytesPerRow - i;
				// Check for end of boundary within this line
				if ((nPosition + i + nChunk) > nBoundaries[nBoundary])
				{
					nChunk = nBoundaries[nBoundary] - (nPosition + i);
					// If this section is followed by selection on same
					// line, go ahead and paint trailing space
					if (nBoundary == 1)
						nSubTrailing = 1;
					else
						nSubTrailing = 0;
				}
				else nSubTrailing = 1;

				ASSERT(nChunk > 0);

				// Print in hex format
				int cxLeft = m_ColMetrics[COL_HEX].left + (((i * 3) - nLeading) * m_CharSize.cx);
				TCHAR *pStrHex = szHexStr + ((i * 3) - nLeading);
				int nLength = (nChunk * 3) + nLeading - nSubTrailing;
				dc.ExtTextOut(cxLeft, cyRow, 0, NULL, pStrHex, nLength, NULL);
				// Print in ASCII format
				cxLeft = m_ColMetrics[COL_ASCII].left + (i * m_CharSize.cx);
				dc.ExtTextOut(cxLeft, cyRow, 0, NULL, szAsciiStr + i, nChunk, NULL);
			}
			// Bump y position
			cyRow += m_CharSize.cy;
			// Advance position to next line
			nPosition += m_nBytesPerRow;
		}
	}
	else
	{
		dc.SetTextColor(rgbNormalText);
		dc.SetBkColor(rgbNormal);

		while (cyRow < rect.bottom && nPosition <= m_nLength) {

			// Print line offset
			GetOffsetString(szBuffer, nPosition);
			dc.ExtTextOut(m_ColMetrics[COL_OFFSET].left, cyRow, 0, NULL, szBuffer, 4, NULL);

			// Get line length
			nCount = min(m_nBytesPerRow, m_nLength - nPosition);

			// Print in hex format
			GetHexString(szBuffer, m_pBuffer + nPosition, nCount);
			if (nCount > (m_nBytesPerRow / 2)) szBuffer[((m_nBytesPerRow / 2) * 3) - 1] = ' ';
			dc.ExtTextOut(m_ColMetrics[COL_HEX].left, cyRow, 0, NULL, szBuffer, (m_nBytesPerRow * 3) - 1, NULL);

			// Print in ASCII format
			GetChrString(szBuffer, m_pBuffer + nPosition, nCount);
			dc.ExtTextOut(m_ColMetrics[COL_ASCII].left, cyRow, 0, NULL, szBuffer, m_nBytesPerRow, NULL);

			// Bump row position
			cyRow += m_CharSize.cy;

			nPosition += m_nBytesPerRow;
		}
	}

	CBrush Brush(GetSysColor(COLOR_WINDOW));

	// Erase any remaining rows
	if (cyRow < rect.bottom)
	{
		rect.top = cyRow;
		dc.FillRect(&rect, &Brush);
	}

	// Fill areas between columns
	rect.top = 0; rect.bottom = cyRow;
	rect.left = m_ColMetrics[COL_ASCII].right;
	dc.FillRect(&rect, &Brush);
	rect.left = m_ColMetrics[COL_HEX].right; rect.right = m_ColMetrics[COL_ASCII].left;
	dc.FillRect(&rect, &Brush);
	rect.left = m_ColMetrics[COL_OFFSET].right; rect.right = m_ColMetrics[COL_HEX].left;
	dc.FillRect(&rect, &Brush);
	rect.left = 0; rect.right = m_ColMetrics[COL_OFFSET].left;
	dc.FillRect(&rect, &Brush);

	// Clean up
	dc.SelectObject(pOldFont);

	// If we have focus
	ShowCaret();

	// Update scrollbar to show position
	UpdateScrollbar();
}

// Updates the scrollbar to reflect the current position
void CHexEdit::UpdateScrollbar()
{
	SCROLLINFO si1;

	si1.cbSize = sizeof(SCROLLINFO);
	si1.fMask = SIF_ALL;

	// Vertical scrollbar
	si1.nMin = 0;
	si1.nMax = (m_nLength / m_nBytesPerRow);
	si1.nPage = GetClientRows();
	si1.nPos = (m_nTopPosition / m_nBytesPerRow);
	SetScrollInfo(SB_VERT, &si1, TRUE);


	// Horizontal scrollbar
	HDC hdc;
	TEXTMETRIC tm;
	hdc = ::GetDC(m_hWnd);
	// Extract font dimensions from the text metrics. 
	GetTextMetrics(hdc, &tm);
	int xChar = tm.tmAveCharWidth;
	int xClientMax = 4*xChar+47*xChar+16*xChar+3*xChar;
	m_nHorMaxLength = xClientMax / xChar;
	m_nHorMinLength = GetClientColumns();

	SCROLLINFO si2;
	si2.cbSize = sizeof(SCROLLINFO);
	si2.fMask = SIF_ALL;
	si2.nMin = 0;
	si2.nMax = xClientMax / xChar;
	si2.nPage = GetClientColumns();
	si2.nPos = m_nRightPosition;
	SetScrollInfo(SB_HORZ, &si2, TRUE);

}

// Creates a 4 digit, hexidecimal representation of nOffset
void CHexEdit::GetOffsetString(TCHAR* sBuffer, int nOffset)
{
	*sBuffer++ = szHexDigits[(nOffset >> (4 * 3)) & 0xf];
	*sBuffer++ = szHexDigits[(nOffset >> (4 * 2)) & 0xf];
	*sBuffer++ = szHexDigits[(nOffset >> (4 * 1)) & 0xf];
	*sBuffer++ = szHexDigits[nOffset & 0xf];
}

// Creates a string with two hex characters followed by a space for each byte
// in lpBuffer. (Spaces are appended if nCount < bytes per row.)
void CHexEdit::GetHexString(TCHAR* sBuffer, BYTE* lpBuffer, int nCount)
{
	for (int i = 0; i < nCount; i++)
	{
		int j = (TCHAR)*lpBuffer++;
		*sBuffer++ = szHexDigits[(j >> 4) & 0xf];
		*sBuffer++ = szHexDigits[j & 0xf];
		*sBuffer++ = ' ';
	}
	memset(sBuffer, ' ', (m_nBytesPerRow - nCount) * 3);
}

// Creates a string with an ASCII character for each byte in lpBuffer.
// (Spaces are appended if nCount < bytes per row.)
void CHexEdit::GetChrString(TCHAR* sBuffer, BYTE* lpBuffer, int nCount)
{
	if (m_bShowAllChars)
	{
		for (int i = 0; i < nCount; i++)
		{
			TCHAR c = (TCHAR)*lpBuffer++;
			if (c >= 32 && c <= 126)
				*sBuffer++ = (TCHAR)*lpBuffer++;
			else
				*sBuffer++ = '.';
		}	
		memset(sBuffer, ' ', (m_nBytesPerRow - nCount) * 3);
	}
	else
	{
		for (int i = 0; i < nCount; i++)
		{
			TCHAR c = (TCHAR)*lpBuffer++;
			if (c < ' ' || c > '~')
				c = '.';
			*sBuffer++ = c;
		}
		memset(sBuffer, ' ', (m_nBytesPerRow - nCount) * 3);
	}
}


////////////////////////////////////////////////////////////////////////
//

// Deletes the current selection
void CHexEdit::DeleteSelection()
{
	if (HasSelection())
	{
		SetCurrPos(m_nSelStart, HEX_SEL_IGNORE);
		Delete(GetSelLength());
		SetSelection();
		Update(HEX_HINT_LINES, m_nPosition);
	}
}

// Handle getting focus
void CHexEdit::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	// Destroy current caret in case there is already one
	HideCaret();
	DestroyCaret();

	// We should have valid metrics by now
	ASSERT(m_CharSize.cx != 0 && m_CharSize.cy != 0);

	// Create new caret
	CreateSolidCaret(::GetSystemMetrics(SM_CXBORDER), m_CharSize.cy);
	m_bCaretIsVisible = FALSE;
	SetCaret();
}
	
// Handle losing focus
void CHexEdit::OnKillFocus(CWnd* pNewWnd)
{
	CWnd::OnKillFocus(pNewWnd);
	
	HideCaret();
	DestroyCaret();
}

// NOTE: Our control wants to process tab controls and arrow keys
UINT CHexEdit::OnGetDlgCode()
{
	return DLGC_WANTCHARS | DLGC_WANTARROWS | DLGC_WANTTAB;
}

// Process keystroke characters
void CHexEdit::OnChar(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if (GetKeyState(VK_CONTROL) < 0)
	{
		// Check for clipboard commands
		switch (nChar)
		{
			case 0x18:	// Ctrl+X - Cut
				OnEditCut();
				break;
			case 0x03:	// Ctrl+C - Copy
				//OnEditCopy();
				break;
			case 0x16:	// Ctrl+V - Paste
				//OnEditPaste();
				break;
			case 0x01:
				OnEditSelectAll();	// Ctrl+A - Select All
				break;
		}
	}
	else if (!m_bReadOnly)
	{
		if (m_bHexCol)
		{
			if (isxdigit(nChar))
			{
				BYTE nByteVal;
				int bInByte, nAdvance;
				// If selection, delete selected data
				DeleteSelection();
				if (m_bInByte)
				{
					ASSERT(m_nPosition < m_nLength);
					nByteVal = m_pBuffer[m_nPosition];
					nByteVal &= 0xF0;
					nByteVal |= isdigit(nChar) ? (nChar & 0x0F) : ((nChar & 0x0F) + 9);
					bInByte = 0;
					nAdvance = 1;
				}
				else
				{
					nByteVal = (BYTE)(isdigit(nChar) ? (nChar & 0x0F) : ((nChar & 0x0F) + 9));
					nByteVal <<= 4;
					if (m_bInsertMode || m_nPosition >= m_nLength)
					{
						if (Insert(1) == 0)
							return;
					}
					else nByteVal |= (m_pBuffer[m_nPosition] & 0x0F);
					bInByte = 1;
					nAdvance = 0;
				}
				m_pBuffer[m_nPosition] = nByteVal;
				Update(HEX_HINT_LINES, m_nPosition);
				SetCurrPos(m_nPosition + nAdvance, HEX_SEL_CLEAR, bInByte);
			}
		}
		else
		{
			if (nChar >= ' ' && nChar <= '~')
			{
				// If selection, delete selected data
				DeleteSelection();
				if (m_bInsertMode || m_nPosition >= m_nLength)
				{
					if (Insert(1) == 0)
						return;
				}
				m_pBuffer[m_nPosition] = (BYTE)nChar;
				Update(HEX_HINT_LINES, m_nPosition);
				// Will clear selection
				SetCurrPos(m_nPosition + 1);
			}
		}
	}
}

// Process non-character keystrokes
void CHexEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int nExtSelect = (GetKeyState(VK_SHIFT) < 0) ? HEX_SEL_EXTEND : HEX_SEL_CLEAR;

	switch (nChar)
	{
		case VK_DOWN:
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Scroll line down
				SendMessage(WM_VSCROLL, SB_LINEDOWN, 0);
			}
			else
			{
				// Move caret line down
				SetCurrPos(m_nPosition + m_nBytesPerRow, nExtSelect);
			}
			break;

		case VK_UP:
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Scroll line up
				SendMessage(WM_VSCROLL, SB_LINEUP, 0);
			}
			else
			{
				// Move caret line up
				SetCurrPos(m_nPosition - m_nBytesPerRow, nExtSelect);
			}
			break;

		case VK_RIGHT:
			// Byte right
			SetCurrPos(m_nPosition + 1, nExtSelect);
			break;

		case VK_LEFT:
			// Byte left
			SetCurrPos(m_nPosition - 1, nExtSelect);
			break;

		case VK_NEXT:
			// Page Down
			SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0);
			SetCurrPos(m_nPosition + (GetClientRows() * m_nBytesPerRow), nExtSelect);
			break;

		case VK_PRIOR:
			// Page Up
			SendMessage(WM_VSCROLL, SB_PAGEUP, 0);
			SetCurrPos(m_nPosition - (GetClientRows() * m_nBytesPerRow), nExtSelect);
			break;

		case VK_HOME:
			m_bInByte = 0;
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// Start of buffer
				SetCurrPos(0, nExtSelect);
			}
			else
			{
				// Start of line
				int nNewPos;
				if (m_bTrailingCaret)
				{
					nNewPos = (m_nPosition - 1) & ~(m_nBytesPerRow - 1);
					m_bTrailingCaret = FALSE;
				}
				else nNewPos = m_nPosition & ~(m_nBytesPerRow - 1);
				SetCurrPos(nNewPos, nExtSelect);
			}
			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL) < 0)
			{
				// End of buffer
				m_bTrailingCaret = FALSE;
				SetCurrPos(m_nLength, nExtSelect);
			}
			else
			{
				// End of line
				int nNewPos;
				if (!m_bTrailingCaret)
				{
					nNewPos = (m_nPosition | (m_nBytesPerRow - 1)) + 1;
					m_bTrailingCaret = TRUE;
				}
				else
				{
					ASSERT((m_nPosition & (m_nBytesPerRow - 1)) == 0);
					nNewPos = m_nPosition;
				}
				SetCurrPos(nNewPos, nExtSelect);
			}
			break;

		case VK_TAB:
			// Ctrl+Tab sets focus to next/previous control
			if (GetKeyState(VK_CONTROL) < 0)
			{
				GetParent()->PostMessage(WM_NEXTDLGCTL, GetKeyState(VK_SHIFT) < 0, 0);
			}
			else
			{
				// Toggle between hex and ascii edit
				m_bHexCol = !m_bHexCol;
				SetCurrPos(m_nPosition, HEX_SEL_IGNORE);
			}
			break;

		case VK_DELETE:
			// Delete at cursor (or selection)
			if (!m_bReadOnly)
			{
				if (HasSelection())
				{
					DeleteSelection();
				}
				else if (m_nPosition < m_nLength)
				{
					VERIFY(Delete(1));
					SetCurrPos(m_nPosition);
					Update(HEX_HINT_LINES, m_nPosition);
				}
			}
			break;

		case VK_BACK:
			// Delete left of cursor (or selection)
			if (!m_bReadOnly) {
				if (HasSelection())
				{
					DeleteSelection();
				}
				else if (m_nPosition > 0 || m_bInByte)
				{
					if (m_bInByte)
						SetCurrPos(m_nPosition);
					else
						SetCurrPos(m_nPosition - 1);
					VERIFY(Delete(1));
					Update(HEX_HINT_LINES, m_nPosition);
				}
			}
			break;

		case VK_INSERT:
			if (m_bInsertToggle)
				m_bInsertMode = !m_bInsertMode;
			break;

		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

// Handle vertical scrollbar message
void CHexEdit::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* /*pScrollBar*/)
{
	switch (nSBCode)
	{
		case SB_LINEDOWN:
			if ((m_nTopPosition + m_nBytesPerRow) < m_nLength)
				SetVerScroll(m_nTopPosition + m_nBytesPerRow);
			break;

		case SB_LINEUP:
			if (m_nTopPosition > 0)
				SetVerScroll(m_nTopPosition - m_nBytesPerRow);
			break;

		case SB_PAGEDOWN:
			if ((m_nTopPosition + m_nBytesPerRow) < m_nLength)
				SetVerScroll(m_nTopPosition + (GetClientRows() * m_nBytesPerRow));
			break;

		case SB_PAGEUP:
			if (m_nTopPosition > 0)
				SetVerScroll(m_nTopPosition - (GetClientRows() * m_nBytesPerRow));
			break;

		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			{
				int i = nPos * m_nBytesPerRow;
				if (m_nTopPosition != i)
				{
					SetVerScroll(i);
					// This is needed to prevent scrollbars
					// from flickering at their old position
					UpdateScrollbar();
				}
			}
			break;
	}
}

// Handle Horizontal scrollbar message
void CHexEdit::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO scrollinfo;
	GetScrollInfo(SB_HORZ, &scrollinfo, SIF_ALL);
	switch (nSBCode)
	{
	case SB_LEFT:
		SetHorScroll(0);
		break;
	case SB_RIGHT:
		SetHorScroll(m_nHorMaxLength);
		break;
	case SB_LINELEFT:
		SetHorScroll(m_nRightPosition - 1);
		break;
	case SB_LINERIGHT:
		SetHorScroll(m_nRightPosition + 1);
		break;
	case SB_PAGELEFT:
		SetHorScroll(m_nRightPosition - 1*5);
		break;
	case SB_PAGERIGHT:
		SetHorScroll(m_nRightPosition + 1*5);
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		{
			int i = nPos;
			if (m_nRightPosition != i)
			{
				SetHorScroll(i);
				UpdateScrollbar();
			}
		}
			break;
	case SB_ENDSCROLL:
		break;
	}
	//Invalidate(true);
}

/////////////////////////////////////////////////////////////////////////////
// Mouse handlers

// Handle mouse button down
void CHexEdit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Windows won't set focus to our control automatically
	if (GetFocus() != this)
		SetFocus();
	// Get file offset indicated by cursor position
	int nOffset = OffsetFromPoint(point);
	// Extend selection if shift key pressed
	SetCurrPos(nOffset, (nFlags & MK_SHIFT) ? HEX_SEL_EXTEND : HEX_SEL_CLEAR);
	// Begin selection drag with mouse
	SetCapture();
}

// Handle mouse move
void CHexEdit::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	// If we have captured mouse input, we must be dragging selection
	if (GetCapture() == this)
	{
		// Process mouse move only if we are not auto-scrolling
		if (m_nTimerID == 0)
		{
			int nPosition = OffsetFromPoint(point);
			// Extend selection if position has changed
			if (nPosition != m_nPosition)
			{
				SetCurrPos(nPosition, HEX_SEL_EXTEND);
				// If mouse leaves window, start timer for auto-scroll
				CRect rect;
				GetClientRect(&rect);
				if (!rect.PtInRect(point))
					m_nTimerID = SetTimer(1, 150, NULL);
			}
		}
	}
}

// Timer handler used for auto-scrolling
void CHexEdit::OnTimer(UINT nIDEvent)
{
	// If we are in auto-scroll mode
	if (GetCapture() == this)
	{
		if (nIDEvent == (UINT)m_nTimerID)
		{
			// Get client rectangle
			CRect rect;
			GetClientRect(&rect);
			// Get mouse position in client coordinates
			CPoint point;
			VERIFY(::GetCursorPos(&point));
			ScreenToClient(&point);
			if (!rect.PtInRect(point))
			{
				int nPosition = OffsetFromPoint(point);
				// Extend selection if position has changed
				if (nPosition != m_nPosition)
					SetCurrPos(nPosition, HEX_SEL_EXTEND);
			}
			else
			{
				// If mouse re-entered window, stop auto-scroll
				m_nTimerID = 0;
			}
		}
	}
	else m_nTimerID = 0;

	// Kill timer if requested
	if (m_nTimerID == 0)
		KillTimer(nIDEvent);
}

// Handle mouse button release
void CHexEdit::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/) 
{
	// Terminate drag selection operation
	if (GetCapture() == this)
		ReleaseCapture();
}

// Run context menu
void CHexEdit::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// Windows won't set focus to our control automatically
	if (GetFocus() != this)
		SetFocus();
	// Create popup menu
	CMenu PopupMenu;
	if (PopupMenu.LoadMenu(IDR_HEXCTRLMENU))
	{
		CMenu* pPopupMenu = PopupMenu.GetSubMenu(0);
		// Enable/disable menu commands
		pPopupMenu->EnableMenuItem(ID_EDIT_CUT,
			HasSelection() && !m_bReadOnly ? MF_ENABLED : MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_EDIT_COPY,
			HasSelection() ? MF_ENABLED : MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_EDIT_PASTE,
			CanPaste() && !m_bReadOnly ? MF_ENABLED : MF_GRAYED);
		pPopupMenu->EnableMenuItem(ID_EDIT_DELETE,
			HasSelection() && !m_bReadOnly ? MF_ENABLED : MF_GRAYED);
		// Check for pop-up menu from keyboard
		if (point.x == -1 && point.y == -1)
		{
			point = CPoint(5,5);
			ClientToScreen(&point);
		}
		pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, point.x, point.y, pWnd);
	}
}


/////////////////////////////////////////////////////////////////////////////
// Clipboard routines

// Perform cut operation
void CHexEdit::OnEditCut()
{
	if (HasSelection() && !m_bReadOnly)
	{
		OnCopyBinary();
		OnEditDelete();
	}
}

// Perform delete operation
void CHexEdit::OnEditDelete() 
{
	if (HasSelection() && !m_bReadOnly)
		DeleteSelection();
}

// Selects the entire control
void CHexEdit::OnEditSelectAll()
{
	SetSelection(0, m_nLength);
}

// Indicates if a paste operation can be performed
BOOL CHexEdit::CanPaste()
{
	COleDataObject dataObject;
	// Note: Paste requires CF_TEXT data even if
	// native clipboard data also exists
	return dataObject.AttachClipboard() && dataObject.IsDataAvailable(CF_TEXT);
}

// Creates data source object for copy or drag and drop operation
COleDataSource* CHexEdit::CreateDataSource(unsigned int type)
{
	COleDataSource* pDataSource = NULL;
	HGLOBAL hMem;
	BYTE* lpByte;
	int nPosition, nSelLength;

	ASSERT(HasSelection());

	try {
		switch (type)
		{
			case CT_View:
			{
				pDataSource = new COleDataSource;
				nPosition = m_nSelStart;
				// Copy data as CF_TEXT
				nSelLength = GetSelLength();
				unsigned int nLength = 0;

				if (nSelLength < 16)
				{
					nLength = 16 * 3 + nSelLength;
				}
				else
				{
					if (nSelLength % 16 == 0)
						nLength = nSelLength * 4 + (nSelLength/16-1)*2;
					else
						nLength = (nSelLength / 16) * 4 * 16 + 16 * 3 + nSelLength / 16 * 2 + nSelLength % 16;
				}



				hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nLength + 1);
				if (hMem == NULL)
					AfxThrowMemoryException();
				lpByte = (BYTE*)::GlobalLock(hMem);
				ASSERT(lpByte != NULL);
				nLength=SetPasteView(lpByte, (unsigned char *)(m_pBuffer + nPosition), nSelLength);
				lpByte[nLength] = 0;
				::GlobalUnlock(hMem);
				pDataSource->CacheGlobalData(CF_TEXT, hMem);
			}break;
			case CT_BinaryString:
			{
				pDataSource = new COleDataSource;
				nPosition = m_nSelStart;
				// Copy data as CF_TEXT
				nSelLength = GetSelLength();
				unsigned int nLength = nSelLength * 5;
				hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nLength + 1);
				if (hMem == NULL)
					AfxThrowMemoryException();
				lpByte = (BYTE*)::GlobalLock(hMem);
				ASSERT(lpByte != NULL);
				SetPasteData(lpByte, (unsigned char *)(m_pBuffer + nPosition), nSelLength);
				lpByte[nLength] = 0;
				::GlobalUnlock(hMem);
				pDataSource->CacheGlobalData(CF_TEXT, hMem);

			}break;
			case CT_HEXString:
			{
				pDataSource = new COleDataSource;
				nPosition = m_nSelStart;
				// Copy data as CF_TEXT
				nSelLength = GetSelLength();
				unsigned int nLength = nSelLength * 2;
				hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nLength + 1);
				if (hMem == NULL)
					AfxThrowMemoryException();
				lpByte = (BYTE*)::GlobalLock(hMem);
				ASSERT(lpByte != NULL);
				HexToStr(m_pBuffer + nPosition, nSelLength, lpByte);
				lpByte[nLength] = 0;
				::GlobalUnlock(hMem);
				pDataSource->CacheGlobalData(CF_TEXT, hMem);
			}break;
			case CT_Binary:
			default:
			{
					pDataSource = new COleDataSource;
					nPosition = m_nSelStart;
					// Copy data as CF_TEXT
					nSelLength = GetSelLength();
					hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, nSelLength + 1);
					if (hMem == NULL)
						AfxThrowMemoryException();
					lpByte = (BYTE*)::GlobalLock(hMem);
					ASSERT(lpByte != NULL);
					memcpy(lpByte, m_pBuffer + nPosition, nSelLength);
					lpByte[nSelLength] = 0;
					::GlobalUnlock(hMem);
					pDataSource->CacheGlobalData(CF_TEXT, hMem);

					// Since binary data may contain '\0's, also create native format
					// that contains only the length of the data
					hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(int));
					if (hMem == NULL)
						AfxThrowMemoryException();
					lpByte = (BYTE*)::GlobalLock(hMem);
					ASSERT(lpByte != NULL);
					*((int*)lpByte) = nSelLength;
					::GlobalUnlock(hMem);
					pDataSource->CacheGlobalData((CLIPFORMAT)m_cfFormat, hMem);
			}break;
		}

	}
	catch (CException* e)
	{
		delete pDataSource;
		pDataSource = NULL;
		e->ReportError();
		e->Delete();
	}
	return pDataSource;
}

// Inserts data from paste or drag and drop operation
BOOL CHexEdit::DoPasteData(COleDataObject* pDataObject,unsigned int type)
{
	HGLOBAL hMem = NULL;
	BYTE* lpByte;
	int nLength = 0;
	BOOL bRetVal = FALSE;

	try
	{
		if (type == CT_Binary)
		{
			if (pDataObject->IsDataAvailable((CLIPFORMAT)m_cfFormat))
			{
				// Get length of CF_TEXT data
				hMem = pDataObject->GetGlobalData((CLIPFORMAT)m_cfFormat);
				if (hMem == NULL)
					AfxThrowMemoryException();
				lpByte = (BYTE*)::GlobalLock(hMem);
				ASSERT(lpByte != NULL);
				nLength = *((int*)lpByte);
				ASSERT(nLength != 0);
				// Requires CF_TEXT for data
				ASSERT(pDataObject->IsDataAvailable(CF_TEXT));
				::GlobalUnlock(hMem);
				::GlobalFree(hMem);
			}
		}

		if (pDataObject->IsDataAvailable(CF_TEXT))
		{
			hMem = pDataObject->GetGlobalData(CF_TEXT);
			if (hMem == NULL)
				AfxThrowMemoryException();
			ASSERT(nLength == 0 || GlobalSize(hMem) >= (UINT)nLength);
			lpByte = (BYTE*)::GlobalLock(hMem);
			ASSERT(lpByte != NULL);
			// If no native length indicator, read as null-terminated string
			if (nLength == 0)
				nLength = _tcslen((TCHAR*)lpByte);
			if (nLength > 0)
			{
				switch (type)
				{
					case CT_Binary:
					{
						// Paste data
						nLength = Insert(nLength);
						if (nLength > 0)
						{
							memcpy(m_pBuffer + m_nPosition, lpByte, nLength);
							Update(HEX_HINT_LINES, m_nPosition);
							SetCurrPos(m_nPosition + nLength, HEX_SEL_CLEAR);
						}
					}break;
					case CT_BinaryString:
					{
						// Paste data
						nLength = GetPasteData(lpByte, nLength);
						nLength = Insert(nLength);
						if (nLength > 0)
						{
							memcpy(m_pBuffer + m_nPosition, lpByte, nLength);
							Update(HEX_HINT_LINES, m_nPosition);
							SetCurrPos(m_nPosition + nLength, HEX_SEL_CLEAR);
						}
					}break;
					case CT_HEXString:
					{
						// Paste data
						BYTE *buffer = (BYTE*)malloc(nLength/2 + 1);
						memset(buffer, 0, nLength/2 + 1);
						StrToHex(lpByte, nLength, buffer);
						nLength = nLength / 2;
						memcpy(lpByte, buffer, nLength);
						free(buffer);
						buffer = NULL;
						nLength = Insert(nLength);
						if (nLength > 0)
						{
							memcpy(m_pBuffer + m_nPosition, lpByte, nLength);
							Update(HEX_HINT_LINES, m_nPosition);
							SetCurrPos(m_nPosition + nLength, HEX_SEL_CLEAR);
						}

					}break;
				}

				bRetVal = TRUE;
			}
			// Release memory
			::GlobalUnlock(hMem);
			::GlobalFree(hMem);
		}
	}
	catch(CException* e)
	{
		e->ReportError();
		e->Delete();
	}
	return bRetVal;
}

//把粘贴板数据按照固定格式读出
unsigned int CHexEdit::GetPasteData(unsigned char *buffer, unsigned int len)
{
	//先判断格式
	//if (buffer[0] != '{'||buffer[len - 1] != '}') return 0;
	unsigned int retlen = 0;
	char *dst1 = (char*)malloc(len);
	memset(dst1, 0, len);
	int j = 0;
	for (int i = 0; i <= len-4;)
	{
		if (buffer[i] == 0x30 && buffer[i+1] == 0x78)
		{
			dst1[j++] = buffer[i + 2];
			dst1[j++] = buffer[i + 3];
			i += 4;
			continue;
		}
		i++;
	}
	memset(buffer, 0, len);
	retlen = j / 2;
	//在进行转换
	char h1, h2;
	BYTE s1, s2;
	int i;

	for (i = 0; i*2<j; i++)
	{
		h1 = dst1[2 * i];
		h2 = dst1[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		buffer[i] = s1 * 16 + s2;
	}
	free(dst1);
	return retlen;
}

//读出数据，输出成数组格式放到粘贴板
unsigned int CHexEdit::SetPasteData(unsigned char *buffer, unsigned char *srcBuffer, unsigned int len)
{
	unsigned retlen = 0;

	char *dst = (char*)malloc(len*2);
	memset(dst, 0, len*2);
	char ddl, ddh;
	for (int i = 0; i<len; i++)
	{
		ddh = 48 + srcBuffer[i] / 16;
		ddl = 48 + srcBuffer[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		dst[i * 2] = ddh;
		dst[i * 2 + 1] = ddl;
	}

	unsigned int j = 0;
	for (int i = 0; i < len*2;)
	{
		buffer[j++] = '0';
		buffer[j++] = 'x';
		buffer[j++] = dst[i++];
		buffer[j++] = dst[i++];
		buffer[j++] = ',';
	}

	retlen = j;
	return retlen;
}

//读出数据，输出成数组格式放到粘贴板
unsigned int CHexEdit::SetPasteView(unsigned char *buffer, unsigned char *srcBuffer, unsigned int nLen)
{
	unsigned retlen = 0;

	std::string strContent = "";
	//先转换为小写
	unsigned int row = 0;
	unsigned int sub = 0x10;
	if (nLen % 16 == 0)
		row = nLen / 16;
	else
	{
		row = nLen / 16 + 1;
		sub = nLen % 16;
	}
		
	for (int r = 0; r < row; r++)
	{
		char ddl, ddh;
		int i;
		int rowlen = 0x10;
		if (r ==row - 1)
			rowlen = sub;
		for (i = 0; i<rowlen; i++)
		{
			ddh = 48 + srcBuffer[i + r * 0x10] / 16;
			ddl = 48 + srcBuffer[i + r * 0x10] % 16;
			if (ddh > 57) ddh = ddh + 7;
			if (ddl > 57) ddl = ddl + 7;
			strContent.push_back(ddh);
			strContent.push_back(ddl);
			strContent.push_back(' ');
		}
		if (rowlen != 0x10)
		{
			strContent.insert(strContent.size(), (0x10 - rowlen) * 3,' ');
		}
		//添加assic
		for (i = 0; i < rowlen; i++)
		{
			if (srcBuffer[i + r * 0x10]!=0)
				strContent.push_back(srcBuffer[i + r * 0x10]);
			else
				strContent.push_back('.');
			
		}
		if (r != row - 1)
		{
			strContent.push_back(0xd);
			strContent.push_back(0xa);
		}

	}
	retlen = strContent.size();
	memcpy(buffer, strContent.c_str(), strContent.size());
	return retlen;
}


void CHexEdit::OnCopyBinary()
{
	// TODO:  在此添加命令处理程序代码
	if (HasSelection())
	{
		COleDataSource* pDataSource = CreateDataSource(CT_Binary);
		if (pDataSource != NULL)
			pDataSource->SetClipboard();
	}
}

void CHexEdit::OnCopyString()
{
	// TODO:  在此添加命令处理程序代码
	if (HasSelection())
	{
		COleDataSource* pDataSource = CreateDataSource(CT_BinaryString);
		if (pDataSource != NULL)
			pDataSource->SetClipboard();
	}
}

void CHexEdit::OnCopyHexstring()
{
	// TODO:  在此添加命令处理程序代码
	if (HasSelection())
	{
		COleDataSource* pDataSource = CreateDataSource(CT_HEXString);
		if (pDataSource != NULL)
			pDataSource->SetClipboard();
	}
}

void CHexEdit::OnPasteBinary()
{
	// TODO:  在此添加命令处理程序代码
	COleDataObject dataObject;

	if (CanPaste() && !m_bReadOnly)
	{
		// Replace selection if any
		DeleteSelection();
		if (dataObject.AttachClipboard())
			DoPasteData(&dataObject, CT_Binary);
	}
}

void CHexEdit::OnPasteString()
{
	// TODO:  在此添加命令处理程序代码
	COleDataObject dataObject;

	if (CanPaste() && !m_bReadOnly)
	{
		// Replace selection if any
		DeleteSelection();
		if (dataObject.AttachClipboard())
			DoPasteData(&dataObject,CT_BinaryString);
	}
}

void CHexEdit::OnPasteHexstring()
{
	COleDataObject dataObject;

	if (CanPaste() && !m_bReadOnly)
	{
		// Replace selection if any
		DeleteSelection();
		if (dataObject.AttachClipboard())
			DoPasteData(&dataObject, CT_HEXString);
	}
}


void CHexEdit::HexToStr(BYTE *pbSrc, int nLen, BYTE *dstbuffer)
{
	char ddl, ddh;
	int i;
	int j;
	for (i = 0,j=0; i<nLen; i++,j+=2)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		dstbuffer[j] = ddh;
		dstbuffer[j+1] = ddl;
	}
}

void CHexEdit::StrToHex(BYTE *pbSrc, int nLen, BYTE *dstbuffer)
{
	std::string str;
	char h1, h2;
	BYTE s1, s2;
	int i;

	for (i = 0; i<nLen / 2; i++)
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];

		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;

		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;

		dstbuffer[i]=s1 * 16 + s2;
	}
}

void CHexEdit::OnCopyView()
{
	// TODO:  在此添加命令处理程序代码
	if (HasSelection())
	{
		COleDataSource* pDataSource = CreateDataSource(CT_View);
		if (pDataSource != NULL)
			pDataSource->SetClipboard();
	}
}
