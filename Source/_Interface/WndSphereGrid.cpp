#include "StdAfx.h"
#include "WndSphereGrid.h"
#include "AppDefine.h"

// Using Pimpl idiom for prototyping

class CWndGridImpl : public CWndSphereGrid::CWndGrid {

  void OnDraw(C2DRender * p2DRender) override;

};

void CWndGridImpl::OnDraw(C2DRender * p2DRender) {
  p2DRender->RenderFillRect(CRect(CPoint(-5, -5), CSize(700, 700)), 0xFF00EECC);
  p2DRender->TextOut(70, 70, "Hey it's me, Mario", 0xFF000000);
}




BOOL CWndSphereGrid::Initialize(CWndBase * pWndParent, DWORD nType) {
  return CWndNeuz::InitDialog(APP_SPHEREGRID, pWndParent, 0, CPoint(0, 0));
}

BOOL CWndSphereGrid::OnChildNotify(UINT message, UINT nID, LRESULT * pLResult) {


  return CWndNeuz::OnChildNotify(message, nID, pLResult);
}

void CWndSphereGrid::OnInitialUpdate() {
  CWndNeuz::OnInitialUpdate();

  m_wndGrid = std::make_unique<CWndGridImpl>();

  const CRect gridRect = CRect(CPoint(0, 0), CSize(550, 260));

  CWndTabCtrl * const pWndTabCtrl = GetDlgItem<CWndTabCtrl>(WIDC_TABCTRL);
  m_wndGrid->Create(WBS_CHILD | WBS_NODRAWFRAME, gridRect, pWndTabCtrl, 100000);

  pWndTabCtrl->InsertItem(m_wndGrid.get(), "Sphere Grid editing");


  MoveParentCenter();
}


