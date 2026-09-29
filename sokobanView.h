// sokobanView.h : interface of the CsokobanView class
//

#pragma once

class CsokobanView : public CView
{
protected: // create from serialization only
	CsokobanView() noexcept;
	DECLARE_DYNCREATE(CsokobanView)

	// Attributes
public:
	CsokobanDoc* GetDocument() const;

	// Operations
public:
	int m_workerX;
	int m_workerY;
	int workeoriginX;
	int workeoriginY;

	char** content; //存儲遊戲內容，包括工人和箱子的位置
	int bitmapWidth;
	int bitmapHeight;
	int newBoxX;
	int newBoxY;

	static const int numRows = 32;
	static const int numCols = 32;
	char mapData[numRows][numCols]; //當前地圖資料
	char originalMapData[numRows][numCols]; //原始地圖資料
	int x_right = 0;


	int m_steps;
	CString m_strSteps;
	int m_currentLevel;
	int m_boxesAtDestination;
	int destinationCount;

	bool moveWorker;
	bool m_bAllBoxesOnDestination;
	bool m_levelCompleted;//判斷是否完成關卡

	// Overrides
public:

	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	// Implementation
public:
	virtual ~CsokobanView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CBitmap m_bmpWall;
	CBitmap m_bmpBox;
	CBitmap m_bmpDestination;
	CBitmap m_bmpWorker;
	CBitmap m_bmpArrival;
	CBitmap m_bmpBlank;
	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void readMap(int n);
	void DrawMap(CDC* pDC);
	bool AllBoxesOnDestination();
	void nextlevel();
	void currentlevel();
	void prelevel();
	void head_next_level();
};

#ifndef _DEBUG  // debug version in sokobanView.cpp
inline CsokobanDoc* CsokobanView::GetDocument() const
{
	return reinterpret_cast<CsokobanDoc*>(m_pDocument);
}
#endif

