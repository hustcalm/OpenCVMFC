
// OpenCVMFCDoc.cpp : implementation of the COpenCVMFCDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "OpenCVMFC.h"
#endif

#include "OpenCVMFCDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// COpenCVMFCDoc

IMPLEMENT_DYNCREATE(COpenCVMFCDoc, CDocument)

BEGIN_MESSAGE_MAP(COpenCVMFCDoc, CDocument)
END_MESSAGE_MAP()


// COpenCVMFCDoc construction/destruction

COpenCVMFCDoc::COpenCVMFCDoc()
{
	// TODO: add one-time construction code here
	pImg = NULL;
	m_Display = -1;
}

COpenCVMFCDoc::~COpenCVMFCDoc()
{
	// Release Image
	if (pImg)
		cvReleaseImage(&pImg);
}

BOOL COpenCVMFCDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// COpenCVMFCDoc serialization

void COpenCVMFCDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void COpenCVMFCDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void COpenCVMFCDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void COpenCVMFCDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// COpenCVMFCDoc diagnostics

#ifdef _DEBUG
void COpenCVMFCDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void COpenCVMFCDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// COpenCVMFCDoc commands

BOOL COpenCVMFCDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// TODO: Add your specialized creation code here
	BOOL loadStatus;

	Load(&pImg, lpszPathName);
	if (pImg != NULL) 
		loadStatus = true;
	else 
		loadStatus = false;
	return(loadStatus);
}

//---------------------------------------------------------
//---------------------------------------------------------
//  ImageIO

BOOL COpenCVMFCDoc::Load(IplImage** pp, LPCTSTR csFileName)
{
	IplImage* pImg = NULL;

	pImg = cvLoadImage(csFileName,-1);      //  Read Image File (DSCV)
	if (!pImg) 
		return(false);
	cvFlip(pImg);                           //  Flip to convert to DIB Format

	if (*pp) {
		cvReleaseImage(pp);
	}

	(*pp) = pImg;
	m_Display = 0;
	return(true);
}

BOOL COpenCVMFCDoc::Save(LPCTSTR csFileName,IplImage* pImg)
{
	int   saveStatus;

	cvFlip(pImg);                           //  Restore to OpenCV Image Format
	saveStatus = cvSaveImage(csFileName,pImg);        //  Save Image To Disk
	return(saveStatus);
}
