
// OpenCVMFCDoc.h : interface of the COpenCVMFCDoc class
//


#pragma once


class COpenCVMFCDoc : public CDocument
{
protected: // create from serialization only
	COpenCVMFCDoc();
	DECLARE_DYNCREATE(COpenCVMFCDoc)

	BOOL Load(IplImage** pImg, LPCTSTR pszFilename);
	BOOL Save(LPCTSTR pszFilename, IplImage* pImg);

// Attributes
public:
	IplImage*	pImg;
	int			m_Display;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~COpenCVMFCDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
