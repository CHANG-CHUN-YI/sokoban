// sokobanView.cpp : implementation of the CsokobanView class
//

#include <cstdio> 
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "sokoban.h"
#endif

#include "sokobanDoc.h"
#include "sokobanView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include <map>

// CsokobanView

IMPLEMENT_DYNCREATE(CsokobanView, CView)

BEGIN_MESSAGE_MAP(CsokobanView, CView)
    // Standard printing commands
    ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CsokobanView construction/destruction

CsokobanView::CsokobanView() noexcept
{
    // TODO: add construction code here
    m_bmpWall.LoadBitmap(wall);
    m_bmpBox.LoadBitmap(box);
    m_bmpDestination.LoadBitmap(distination);
    m_bmpWorker.LoadBitmap(worker);
    m_bmpArrival.LoadBitmap(arrival);
    m_bmpBlank.LoadBitmap(blank);

    m_workerX = -1; //初始工人位置是W的位置
    m_workerY = -1;
    m_steps = 0;
    m_currentLevel = 0;
    m_boxesAtDestination = 0;
    destinationCount = 0;
    readMap(0);
    //尋找W的位置
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            if (mapData[row][col] == 'W') {
                m_workerX = col;
                m_workerY = row;
            }
            if (m_workerX != -1 && m_workerY != -1) {
                break; //找到工人位置後可以提前結束迴圈
            }
        }
        if (m_workerX != -1 && m_workerY != -1) {
            break; //沒有找到W的位置，加上錯誤處理
        }
    }
}

CsokobanView::~CsokobanView()
{
}

BOOL CsokobanView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Modify the Window class or styles here by modifying
    return CView::PreCreateWindow(cs);
}

// CsokobanView drawing

void CsokobanView::OnDraw(CDC* pDC)
{
    CsokobanDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc)
        return;

    DrawMap(pDC);
    CString strSteps;
    CString strLevel;
    CString strDestination;
    CString strDestinations;
    strSteps.Format(L"Steps:%d", m_steps);
    strLevel.Format(L"Level:%d", m_currentLevel);
    strDestination.Format(L"Arrival:%d", m_boxesAtDestination);
    strDestinations.Format(L"Destination: %d", destinationCount);
    pDC->TextOut(900, 50, strLevel);
    pDC->TextOut(900, 90, strDestination);
    pDC->TextOut(900, 110, strSteps);
    pDC->TextOut(900, 70, strDestinations);
}

// CsokobanView printing

BOOL CsokobanView::OnPreparePrinting(CPrintInfo* pInfo)
{
    // default preparation
    return DoPreparePrinting(pInfo);
}

void CsokobanView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add extra initialization before printing
}

void CsokobanView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
    // TODO: add cleanup after printing
}

// CsokobanView diagnostics

#ifdef _DEBUG
void CsokobanView::AssertValid() const
{
    CView::AssertValid();
}

void CsokobanView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CsokobanDoc* CsokobanView::GetDocument() const // non-debug version is inline
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CsokobanDoc)));
    return (CsokobanDoc*)m_pDocument;
}
#endif //_DEBUG

// CsokobanView message handlers
void CsokobanView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_levelCompleted = false;
    int dx = 0, dy = 0;
    switch (nChar)
    {
    case 'W': case VK_UP:    dy = -1; break;
    case 'S': case VK_DOWN:  dy = 1;  break;
    case 'A': case VK_LEFT:  dx = -1; break;
    case 'D': case VK_RIGHT: dx = 1;  break;
    case 'E':
        if (m_currentLevel > 0) {
            prelevel();
        }
        return;
    case 'Z':
        if (m_currentLevel < 150) {
            head_next_level();
        }
        return;
    case 'Q': currentlevel(); return;
    }

    if (dx != 0 || dy != 0)
    {
        m_steps++;
        int newX = m_workerX + dx;
        int newY = m_workerY + dy;

        //確認新位置在邊界內且不是牆
        if (newX >= 0 && newX < numCols && newY >= 0 && newY < numRows &&
            mapData[newY][newX] != 'H') //H是牆壁
        {
            bool moveWorker = true;
            char currentPos = mapData[newY][newX]; //當前位置的內容

            //碰到箱子
            if (mapData[newY][newX] == 'B')
            {
                int newBoxX = newX + dx;
                int newBoxY = newY + dy;

                //確認新箱子位置在邊界內且不是牆或另一個箱子
                if (newBoxX >= 0 && newBoxX < numCols && newBoxY >= 0 && newBoxY < numRows &&
                    mapData[newBoxY][newBoxX] != 'H' && mapData[newBoxY][newBoxX] != 'B' && mapData[newBoxY][newBoxX] != 'C')
                {
                    mapData[newBoxY][newBoxX] = 'B';
                    mapData[newY][newX] = (originalMapData[newY][newX] == 'D') ? 'D' : ' ';
                }
                else
                {
                    moveWorker = false; //無法移動箱子，也無法移動工人
                }

            }

            //移動工人位置
            if (moveWorker) {
                //清除前一個工人的位置
                mapData[m_workerY][m_workerX] = (originalMapData[m_workerY][m_workerX] == 'D') ? 'D' : ' ';
                if (originalMapData[m_workerY][m_workerX] == 'C') {
                    mapData[m_workerY][m_workerX] = 'D';
                }
                //移動工人並處理特殊情況
                if (currentPos == 'C') {
                    int nextX = newX + dx;
                    int nextY = newY + dy;
                    if (nextX >= 0 && nextX < numCols && nextY >= 0 && nextY < numRows && mapData[nextY][nextX] == ' ') {
                        mapData[nextY][nextX] = 'B';
                    }
                    else if (nextX >= 0 && nextX < numCols && nextY >= 0 && nextY < numRows && mapData[nextY][nextX] == 'D') {
                        mapData[nextY][nextX] = 'C';
                        if (mapData[nextY][nextX] = 'C') {
                            int nextXX = nextX + dx;
                            int nextYY = nextY + dy;
                            if (nextXX >= 0 && nextXX < numCols && nextYY >= 0 && nextYY < numRows &&
                                mapData[nextYY][nextXX] != 'H' && mapData[nextYY][nextXX] != 'B' && mapData[nextYY][nextXX] != 'C')
                            {
                                mapData[nextY][nextX] = 'B';
                                //mapData[nextYY][nextXX] = 'B';
                                //mapData[nextY][nextX] = (originalMapData[nextY][nextX] == 'D') ? 'D' : ' ';
                            }
                            else
                            {
                                moveWorker = false; //無法移動箱子，也無法移動工人
                            }

                        }

                    }
                    else
                    {
                        moveWorker = false;
                    }
                }
                else
                {
                    moveWorker = false;
                }

                //更新工人的位置
                mapData[newY][newX] = 'W';
                m_workerX = newX;
                m_workerY = newY;
            }

        }
        GetDocument()->UpdateAllViews(NULL); //更新所有視圖

        if (AllBoxesOnDestination())
        {
            //如果所有箱子都在目標位置，則顯示 "Pass!" 訊息視窗
            MessageBox(L"Pass!\nPress SPACE to the next level", L"確定", MB_OK);
            nextlevel();

        }
        CView::OnKeyDown(nChar, nRepCnt, nFlags);
    }

}





void CsokobanView::nextlevel() {
    m_currentLevel++;
    readMap(m_currentLevel);

    //重新初始化關卡相關變數
    m_steps = 0;
    m_levelCompleted = false;
    m_boxesAtDestination = 0;

    //尋找W的位置
    m_workerX = -1;
    m_workerY = -1;
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            if (mapData[row][col] == 'W') {
                m_workerX = col;
                m_workerY = row;
                break; //找到工人位置後可以提前結束迴圈
            }
        }
        if (m_workerX != -1 && m_workerY != -1) {
            break; //確認找到工人位置
        }
    }

    GetDocument()->UpdateAllViews(NULL); //更新所有視圖
    Invalidate(); //強制重繪視圖
}

void CsokobanView::head_next_level() {
    m_currentLevel++;
    readMap(m_currentLevel);

    //重新初始化關卡相關變數
    m_steps = 0;
    m_levelCompleted = false;
    m_boxesAtDestination = 0;

    //尋找W的位置
    m_workerX = -1;
    m_workerY = -1;
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            if (mapData[row][col] == 'W') {
                m_workerX = col;
                m_workerY = row;
                break; //找到工人位置後可以提前結束迴圈
            }
        }
        if (m_workerX != -1 && m_workerY != -1) {
            break; //確認找到工人位置
        }
    }

    GetDocument()->UpdateAllViews(NULL); //更新所有視圖
    Invalidate(); //強制重繪視圖
}


void CsokobanView::prelevel() {
    m_currentLevel--;
    readMap(m_currentLevel);

    //重新初始化關卡相關變數
    m_steps = 0;
    m_levelCompleted = false;
    m_boxesAtDestination = 0;

    //尋找W的位置
    m_workerX = -1;
    m_workerY = -1;
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            if (mapData[row][col] == 'W') {
                m_workerX = col;
                m_workerY = row;
                break; //找到工人位置後可以提前結束迴圈
            }
        }
        if (m_workerX != -1 && m_workerY != -1) {
            break; //確認找到工人位置
        }
    }

    GetDocument()->UpdateAllViews(NULL); //更新所有視圖
    Invalidate(); //強制重繪視圖
}


void CsokobanView::currentlevel() {
    readMap(m_currentLevel);

    //重新初始化關卡相關變數
    m_steps = 0;
    m_levelCompleted = false;
    m_boxesAtDestination = 0;

    //尋找W的位置
    m_workerX = -1;
    m_workerY = -1;
    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            if (mapData[row][col] == 'W') {
                m_workerX = col;
                m_workerY = row;
                break; //找到工人位置後可以提前結束迴圈
            }
        }
        if (m_workerX != -1 && m_workerY != -1) {
            break; //確認找到工人位置
        }
    }

    GetDocument()->UpdateAllViews(NULL); //更新所有視圖
    Invalidate(); //強制重繪視圖
}



void CsokobanView::readMap(int n) {
    memset(mapData, '0', sizeof(mapData)); //將mapData這個陣列或是記憶體區塊的內容全部設定為 '0'
    memset(originalMapData, '0', sizeof(originalMapData)); //將originalMapData也重置
    TCHAR szDirectory[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szDirectory); //取得當前目錄路徑並將路徑寫入 szDirectory
    CString folderPath(szDirectory);
    CString filePath;
    filePath.Format(L"%s\\map%03d.txt", folderPath, n);

    m_currentLevel = n;
    destinationCount = 0; //重置目標點

    int x = 0, y = 0;
    int maxX = 0, maxY = 0; //用於記錄最大行和列
    CFile file;
    if (file.Open(filePath, CFile::modeRead)) {
        char ch;
        while (file.Read(&ch, 1) == 1) { //函數每次讀取一個字元到 ch 中
            if (ch == '\n') {
                y++;
                x = 0;
            }
            else {
                mapData[y][x] = ch;
                originalMapData[y][x] = ch;
                if (ch == 'D') {
                    destinationCount++; //數目標點
                }
                else if (ch == 'C') {
                    destinationCount++;
                }
                x++;
                if (x > x_right) { x_right = x; }
            }
        }
        file.Close();
    }
}

//畫地圖
void CsokobanView::DrawMap(CDC* pDC) {
    const int bitmapWidth = 32; //每個格子大小
    const int bitmapHeight = 32;

    m_boxesAtDestination = 0;

    //文字改成BITMAP的圖
    std::map<char, CBitmap*> bitmapMap;
    bitmapMap[' '] = &m_bmpBlank;
    bitmapMap['H'] = &m_bmpWall;
    bitmapMap['B'] = &m_bmpBox;
    bitmapMap['D'] = &m_bmpDestination;
    bitmapMap['W'] = &m_bmpWorker;
    bitmapMap['C'] = &m_bmpArrival;

    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            char element = mapData[row][col];
            char originalElement = originalMapData[row][col];
            CDC memDC;
            memDC.CreateCompatibleDC(pDC);
            CBitmap* pOldBitmap = nullptr;

            auto it = bitmapMap.find(element);
            if (it != bitmapMap.end()) {
                pOldBitmap = memDC.SelectObject(it->second);
                if (element == 'B' && originalMapData[row][col] == 'D') {
                    pOldBitmap = memDC.SelectObject(&m_bmpArrival);
                    m_boxesAtDestination++;
                }
                else if (element == 'B' && originalMapData[row][col] == 'C') {
                    pOldBitmap = memDC.SelectObject(&m_bmpArrival);
                    m_boxesAtDestination++;
                }
                else if (element == 'C') {
                    m_boxesAtDestination++;
                }           
                pDC->BitBlt(col * bitmapWidth, row * bitmapHeight, bitmapWidth, bitmapHeight, &memDC, 0, 0, SRCCOPY);
                memDC.SelectObject(pOldBitmap);
            }
        }
    }
}

//確認是否所有的箱子都在目標位置上
bool CsokobanView::AllBoxesOnDestination() {
   for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            if (mapData[row][col] == 'B' && originalMapData[row][col] != 'D') {
                if (mapData[row][col] == 'B' && originalMapData[row][col] != 'C') {
                    return false;
                }
            }
        }
    }
    return true;
}



