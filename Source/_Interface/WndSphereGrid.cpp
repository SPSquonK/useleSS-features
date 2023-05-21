#include "StdAfx.h"
#include "WndSphereGrid.h"
#include "AppDefine.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include <format>
#include "Clipboard.h"

const char * FindDstString(int nDstParam);

///////////////////////////////////////////////////////////////////////////////
// Grid Layout

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
  [[nodiscard]] std::string ToJson() const;

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

std::string GridLayout::ToJson() const {
  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> writer(s);
  writer.StartObject();

  writer.Key("nodes");
  writer.StartArray();
  for (const Node & node : nodes) {
    if (!node.content) continue;

    writer.StartObject();

    writer.Key("x"); writer.Int(node.point.x);
    writer.Key("y"); writer.Int(node.point.y);

    writer.Key("stat");
    const CString * statName = CScript::m_defines.Lookup(node.content.value(), "DST_");
    if (!statName) return "Invalid layout";
    writer.String(statName->GetString());

    writer.EndObject();
  }
  writer.EndArray();

  writer.Key("links");
  writer.StartArray();
  for (const Link & link : links) {
    writer.StartObject();

    if (link.from.x != link.to.x) {
      writer.Key("x1"); writer.Int(link.from.x);
      writer.Key("x2"); writer.Int(link.to.x);
      writer.Key("y"); writer.Int(link.from.y);
    } else {
      writer.Key("x"); writer.Int(link.from.x);
      writer.Key("y1"); writer.Int(link.from.y);
      writer.Key("y2"); writer.Int(link.to.y);
    }

    writer.EndObject();
  }
  writer.EndArray();

  return std::string(s.GetString());
}

///////////////////////////////////////////////////////////////////////////////
// Grid display

// Using Pimpl idiom for prototyping

class CWndGridImpl : public CWndSphereGrid::CWndGrid {

  void OnInitialUpdate() override;
  void OnDraw(C2DRender * p2DRender) override;

  void OnRButtonUp(UINT nFlags, CPoint point) override;
  void OnRButtonDown(UINT nFlags, CPoint point) override;
  void OnMouseMove(UINT nFlags, CPoint point) override;
  void OnMButtonUp(UINT nFlags, CPoint point) override;

  // BOOL OnChildNotify(UINT message, UINT nID, LRESULT * pLResult) override;


private:
  GridLayout m_layout;

  [[nodiscard]] CPoint ConvertPoint(CPoint point) const;

  CPoint m_center;
  std::optional<CPoint> m_mouseMoveStart;
  std::optional<CPoint> m_mouseMoveCurrent;


};

CPoint CWndGridImpl::ConvertPoint(CPoint point) const {
  CPoint delta = m_center;
  if (m_mouseMoveStart && m_mouseMoveCurrent) {
    delta += m_mouseMoveCurrent.value() - m_mouseMoveStart.value();
  }

  point.x = point.x * 64 + delta.x;
  point.y = point.y * 64 + delta.y;
  return point;
}

void CWndGridImpl::OnDraw(C2DRender * p2DRender) {
  p2DRender->RenderFillRect(CRect(CPoint(-5, -5), CSize(700, 700)), 0xFF00EECC);
  
  std::string s = std::format("Center = {} {}", m_center.x, m_center.y);
  p2DRender->TextOut(5, 5, s.c_str(), 0xFF0000FF);

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

  m_center = GetClientRect().CenterPoint();

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


#pragma region GridMoving

void CWndGridImpl::OnRButtonUp(UINT nFlags, CPoint point) {
  if (m_mouseMoveStart) {
    const CPoint delta = point - m_mouseMoveStart.value();
    m_center += delta;
    m_mouseMoveStart = std::nullopt;
    m_mouseMoveCurrent = std::nullopt;
    ReleaseCapture();
  }
}

void CWndGridImpl::OnRButtonDown(UINT nFlags, CPoint point) {
  m_mouseMoveStart = point;
  m_mouseMoveCurrent = point;
  SetCapture();
}

void CWndGridImpl::OnMouseMove(UINT nFlags, CPoint point) {
  if (m_mouseMoveStart) m_mouseMoveCurrent = point;
}

#pragma endregion


void CWndGridImpl::OnMButtonUp(UINT, CPoint) {
  g_WndMng.PutString("Hey catch this!");

  std::string json = m_layout.ToJson();

  g_WndMng.PutString(json.c_str());
  g_WndMng.PutString("Now you just have to run some OCR program on the text!");

  CClipboard::SetText(json.c_str());
}


///////////////////////////////////////////////////////////////////////////////
// Main window

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


