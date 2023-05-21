#include "StdAfx.h"
#include "WndSphereGrid.h"
#include "AppDefine.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"

// Using Pimpl idiom for prototyping

const char * FindDstString(int nDstParam);

struct Node {
  CPoint point;
  std::optional<int> content;
};

struct Link {
  CPoint from;
  CPoint to;
};

struct GridLayout {
  std::vector<Node> nodes;
  std::vector<Link> links;

  static GridLayout FromJson(const rapidjson::GenericObject<false, rapidjson::Value>::ValueType & document);


};

GridLayout GridLayout::FromJson(const rapidjson::GenericObject<false, rapidjson::Value>::ValueType & fullLayout) {
  GridLayout result;

	try {
    const auto & jsonNodes = fullLayout.FindMember("nodes")->value.GetArray();

    result.nodes.emplace_back(Node{ CPoint(0, 0), std::nullopt });

    for (const rapidjson::Value & jsonNode : jsonNodes) {
      Node node;
      node.point.x = jsonNode.FindMember("x")->value.GetInt();
      node.point.y = jsonNode.FindMember("y")->value.GetInt();
      node.content = CScript::GetDefineNumOpt(jsonNode.FindMember("stat")->value.GetString()).value();

      result.nodes.emplace_back(node);
    }

    const auto & jsonLinks = fullLayout.FindMember("links")->value.GetArray();
    for (const rapidjson::Value & jsonLink : jsonLinks) {

      if (jsonLink.HasMember("x")) {
        const int x = jsonLink.FindMember("x")->value.GetInt();
        const int y1 = jsonLink.FindMember("y1")->value.GetInt();
        const int y2 = jsonLink.FindMember("y2")->value.GetInt();

        result.links.emplace_back(Link{ CPoint(x, y1), CPoint(x, y2) });
      } else {
        const int y = jsonLink.FindMember("y")->value.GetInt();
        const int x1 = jsonLink.FindMember("x1")->value.GetInt();
        const int x2 = jsonLink.FindMember("x2")->value.GetInt();
        
        result.links.emplace_back(Link{ CPoint(x1, y), CPoint(x2, y) });
      }
    }
	} catch (std::exception e) {
    CString str;
    str.Format(__FUNCTION__"(): %s", e.what());
    g_WndMng.PutString(str.GetString(), nullptr, 0xFFFF0000);
	}

  return result;
}

class CWndGridImpl : public CWndSphereGrid::CWndGrid {

  void OnInitialUpdate() override;
  void OnDraw(C2DRender * p2DRender) override;

private:
  GridLayout m_layout;

  CPoint ConvertPoint(CPoint point);


};

CPoint CWndGridImpl::ConvertPoint(CPoint point) {
  point.x = (point.x + 4) * 64;
  point.y = (point.y + 1) * 64;
  return point;
}

void CWndGridImpl::OnDraw(C2DRender * p2DRender) {
  p2DRender->RenderFillRect(CRect(CPoint(-5, -5), CSize(700, 700)), 0xFF00EECC);
  

  for (const Link & link : m_layout.links) {
    CPoint from = ConvertPoint(link.from);
    CPoint to = ConvertPoint(link.to);

    if (from.x > to.x) std::swap(from.x, to.x);
    if (from.y > to.y) std::swap(from.y, to.y);
    
    from -= CPoint(1, 1);
    to   += CPoint(1, 1);

    p2DRender->RenderFillRect(CRect(from, to), 0xFF000000);
  }

  for (const Node & node : m_layout.nodes) {
    CPoint where = ConvertPoint(node.point);

    const char * text;
    if (node.content) {
      text = FindDstString(node.content.value());
    } else {
      text = "Start";
    }

    const SIZE textSize = m_pFont->GetTextExtent(text);

    const SIZE nodeSize = node.content ? SIZE(32, 32) : (CSize(2, 2) + textSize);
    CRect rect = CRect(where, nodeSize);
    rect.OffsetRect(-rect.Width() / 2, -rect.Height() / 2);
    p2DRender->RenderFillRect(rect, 0xFFFFFFFF);
    p2DRender->RenderRect(rect, 0xFF000000);

    where.x -= textSize.cx / 2;
    where.y -= textSize.cy / 2;

    p2DRender->TextOut(where.x, where.y, text, 0xFF000000);
  }


}

void CWndGridImpl::OnInitialUpdate() {
  CWndNeuz::OnInitialUpdate();

  CScanner scanner;
  if (scanner.Load("SphereGrid.json")) {
    rapidjson::Document document;
    document.Parse(scanner.m_pProg);

    const auto & jsonLayout = document.GetObject()["Rising Tides"];
    m_layout = GridLayout::FromJson(jsonLayout);

  } else {
    g_WndMng.PutString("Failed to load");
  }

  


  

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


