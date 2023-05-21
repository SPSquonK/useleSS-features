#pragma once

#include <memory>

class CWndSphereGrid final : public CWndNeuz {
public:
  class CWndGrid : public CWndNeuz {

  };

  BOOL Initialize(CWndBase * pWndParent = NULL, DWORD nType = MB_OK) override;
  BOOL OnChildNotify(UINT message, UINT nID, LRESULT * pLResult) override;
  void OnInitialUpdate() override;


private:
  std::unique_ptr<CWndGrid> m_wndGrid;


};


