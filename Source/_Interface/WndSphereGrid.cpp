#include "StdAfx.h"
#include "WndSphereGrid.h"
#include "AppDefine.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include <format>
#include "Clipboard.h"
#include <variant>
#include <array>

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

struct ColorSet {
  DWORD background;
  DWORD border;
  DWORD text;
};

struct Selection {
  std::optional<CPoint> point1; // If set, this is either a link or a node
  std::optional<CPoint> point2; // If set, this is a link and point1 is set

  [[nodiscard]] bool IsOn(const Node & node) const {
    return point1 && !point2 && point1.value() == node.point;
  }
  [[nodiscard]] bool IsOn(const Link & link) const {
    if (!point2) return false;

    return (point1.value() == link.from && point2.value() == link.to)
      || (point2.value() == link.from && point1.value() == link.to);
  }
};

struct DisplayLayout {
  enum class DisplayMode {
    Normal, Hovered, ClickToCreate
  };

  // Computes dividend / divisor, but floors the result. Assumes divisor > 0.
  [[nodiscard]] static int DivideFloor(int dividend, int divisor);

  static ColorSet GetColorSet(const DisplayMode dm) {
    switch (dm) {
      case DisplayMode::Hovered:
        return ColorSet{
          .background = 0xFFFFD0D0,
          .border = 0xFFFF0000,
          .text = 0xFFFF0000
        };
      case DisplayMode::ClickToCreate:
        return ColorSet{
          .background = 0xFFD0D0D0,
          .border = 0xFF808080,
          .text = 0xFF000000
        };
      case DisplayMode::Normal:
      default:
        return ColorSet{
          .background = 0xFFFFFFFF,
          .border = 0xFF000000,
          .text = 0xFF000000
        };
    }

  }

  struct DisplayNode : public Node {
    CRect rect;

    void Render(C2DRender * p2DRender, CPoint offset, DisplayMode displayMode) const;

    static CRect ComputeRect(CD3DFont * pFont, const Node & node);
    static std::pair<const char *, CSize> GetTextContent(CD3DFont * pFont, const Node & node);
  };

  struct DisplayLink : public Link {
    void Render(C2DRender * p2DRender, CPoint offset, DisplayMode displayMode) const;
  };

  CD3DFont * m_pFont;
  std::vector<DisplayNode> nodes;
  std::vector<DisplayLink> links;
  Selection m_hovered;
  Selection m_selected;
  
  void Update(CD3DFont * pFont, const GridLayout & gridLayout);

  void SetHovered(std::optional<CPoint> point);

  void Render(C2DRender * p2DRender, CPoint offset) const;

  void OnClick();

  // Find the thing the point is (supposed to be) on
  static Selection GetSelection(CPoint point);
};

void DisplayLayout::Update(CD3DFont * pFont, const GridLayout & gridLayout) {
  nodes.clear();
  links.clear();
  
  m_pFont = pFont;

  for (const Link & link : gridLayout.links) {
    links.emplace_back(link);
  }

  for (const Node & node : gridLayout.nodes) {
    CRect rect = DisplayNode::ComputeRect(m_pFont, node);
    nodes.emplace_back(DisplayNode{ node, rect });
  }

  m_hovered = Selection{ std::nullopt, std::nullopt };
  m_selected = Selection{ std::nullopt, std::nullopt };
}

void DisplayLayout::SetHovered(std::optional<CPoint> point) {
  m_hovered = point.transform(GetSelection).value_or(Selection{ std::nullopt, std::nullopt });

  /*
  * Only select real nodes
  for (const DisplayNode & node : nodes) {
    if (node.rect.PtInRect(*point)) {
      m_hovered = Selection{ node.point, std::nullopt };
      return;
    }
  }
  */
}

Selection DisplayLayout::GetSelection(const CPoint point) {
  constexpr auto FindPositionOnAxis = [](const int value) -> std::pair<bool, int> {
    if (value < 0) {
      return { value & 1, (value - 1) / 2 };
    } else {
      return { value & 1, value / 2 };
    }
  };

  CPoint topLeftPoint = CPoint(DivideFloor(16 + point.x, 32), DivideFloor(16 + point.y, 32));

  const auto [xOnLink, xInGridPoint] = FindPositionOnAxis(topLeftPoint.x);
  const auto [yOnLink, yInGridPoint] = FindPositionOnAxis(topLeftPoint.y);

  CPoint inGridPoint = CPoint(xInGridPoint, yInGridPoint);

  if (xOnLink && yOnLink) {
    return Selection{ std::nullopt, std::nullopt };
  } else if (!xOnLink && !yOnLink) {
    return Selection{ inGridPoint, std::nullopt };
  } else if (xOnLink) {
    return Selection{ inGridPoint, inGridPoint + CPoint(1, 0) };
  } else /* (yOnLink) */ {
    return Selection{ inGridPoint, inGridPoint + CPoint(0, 1) };
  }
}

int DisplayLayout::DivideFloor(int dividend, int divisor) {
  if (dividend < 0 && dividend % divisor != 0) {
    return dividend / divisor - 1;
  } else {
    return dividend / divisor;
  }
};

void DisplayLayout::OnClick() {
  if (!m_hovered.point1) return;

  if (!m_hovered.point2) {
    const auto itNode = std::find_if(
      nodes.begin(), nodes.end(),
      [&](const DisplayNode & node) { return m_hovered.IsOn(node); }
    );

    static constexpr auto cycle = std::array<int, 4>{ DST_STR, DST_STA, DST_DEX, DST_INT };

    if (itNode != nodes.end()) {
      if (!itNode->content) return; // Can not delete the starting node

      const auto itCycle = std::find(cycle.begin(), cycle.end(), *itNode->content);

      if (itCycle == cycle.end()) return;
      if (itCycle + 1 == cycle.end()) {
        nodes.erase(itNode);
      } else {
        (*itNode->content) = *(itCycle + 1);
      }
    } else {
      const Node newNode = Node{ *m_hovered.point1, std::optional<int>(cycle[0]) };
      CRect rect = DisplayNode::ComputeRect(m_pFont, newNode);
      nodes.emplace_back(DisplayNode{ newNode, rect });
    }
  } else {
    // Link
    const auto it = std::find_if(links.begin(), links.end(),
      [&](const DisplayLink & link) { return m_hovered.IsOn(link); }
      );

    if (it != links.end()) {
      links.erase(it);
    } else {
      links.emplace_back(DisplayLink{ Link { *m_hovered.point1, *m_hovered.point2 } });
    }
  }
}

CRect DisplayLayout::DisplayNode::ComputeRect(CD3DFont * pFont, const Node & node) {
  const auto [text, textSize] = GetTextContent(pFont, node);

  const SIZE nodeSize = node.content ? SIZE(32, 32) : (CSize(2, 2) + textSize);
  CRect rect = CRect(CPoint(node.point.x * 64, node.point.y * 64), nodeSize);
  rect.OffsetRect(-nodeSize.cx / 2, -nodeSize.cy / 2);
  return rect;
}

std::pair<const char *, CSize> DisplayLayout::DisplayNode::GetTextContent(
  CD3DFont * pFont, const Node & node
) {
  const char * text;
  if (node.content) {
    if (node.content.value() == 0) return std::pair("", CSize(0, 0));

    text = FindDstString(node.content.value());
  } else {
    text = "Start";
  }

  const SIZE textSize = pFont->GetTextExtent(text);

  return std::pair(text, textSize);
}

void DisplayLayout::Render(C2DRender * p2DRender, CPoint offset) const {
  bool seenHoveredLink = false;
  for (const DisplayLayout::DisplayLink & link : links) {
    const bool hovered = m_hovered.IsOn(link);
    seenHoveredLink |= hovered;
    link.Render(p2DRender, offset,
      hovered ? DisplayLayout::DisplayMode::Hovered :
      DisplayMode::Normal);
  }

  if (!seenHoveredLink && m_hovered.point2) {
    DisplayLink{ Link { *m_hovered.point1, *m_hovered.point2 } }
    .Render(p2DRender, offset, DisplayLayout::DisplayMode::ClickToCreate);
  }

  bool seenHoveredNode = false;
  for (const DisplayLayout::DisplayNode & node : nodes) {
    const bool hovered = m_hovered.IsOn(node);
    seenHoveredNode |= hovered;
    node.Render(p2DRender, offset, 
      hovered ? DisplayLayout::DisplayMode::Hovered :
      DisplayMode::Normal
    );
  }

  if (!seenHoveredNode && m_hovered.point1 && !m_hovered.point2) {
    const auto node = Node{ *m_hovered.point1, std::optional<int>(0) };
    const auto dNode = DisplayNode{ node, DisplayNode::ComputeRect(m_pFont, node) };
    dNode.Render(p2DRender, offset, DisplayMode::ClickToCreate);
  }
}

void DisplayLayout::DisplayNode::Render(C2DRender * p2DRender, CPoint offset, DisplayLayout::DisplayMode displayMode) const {
  const ColorSet colors = GetColorSet(displayMode);

  CRect rect = this->rect;
  rect.OffsetRect(offset);

  p2DRender->RenderFillRect(rect, colors.background);
  p2DRender->RenderRect(rect, colors.border);

  if (displayMode == DisplayMode::Hovered) {
    CRect copy = rect;
    copy.InflateRect(1, 1);
    p2DRender->RenderRect(copy, colors.border);
  }

  if (displayMode != DisplayMode::ClickToCreate) {
    const auto [text, textSize] = GetTextContent(p2DRender->m_pFont, *this);
    CPoint where = CPoint(point.x * 64, point.y * 64) + offset - CPoint(textSize.cx / 2, textSize.cy / 2);
    p2DRender->TextOut(where.x, where.y, text, colors.text);
  }
}

void DisplayLayout::DisplayLink::Render(C2DRender * p2DRender, CPoint offset, DisplayLayout::DisplayMode displayMode) const {
  const ColorSet colors = GetColorSet(displayMode);

  CPoint from = CPoint(this->from.x * 64, this->from.y * 64) + offset;
  CPoint to   = CPoint(this->to.x   * 64, this->to.y   * 64) + offset;

  if (from.x > to.x) std::swap(from.x, to.x);
  if (from.y > to.y) std::swap(from.y, to.y);

  from -= CPoint(1, 1);
  to += CPoint(1, 1);

  p2DRender->RenderFillRect(CRect(from, to), colors.border);
}


class CWndGridImpl : public CWndSphereGrid::CWndGrid {
private:

public:
  void OnInitialUpdate() override;
  void OnDraw(C2DRender * p2DRender) override;

  void OnLButtonDown(UINT nFlags, CPoint point) override;
  void OnRButtonUp(UINT nFlags, CPoint point) override;
  void OnRButtonDown(UINT nFlags, CPoint point) override;
  void OnMouseMove(UINT nFlags, CPoint point) override;
  void OnMButtonUp(UINT nFlags, CPoint point) override;


  void OnMouseWndSurface(CPoint point) override;

  // BOOL OnChildNotify(UINT message, UINT nID, LRESULT * pLResult) override;


private:
  GridLayout m_layout;
  DisplayLayout m_displayLayout;

  [[nodiscard]] CPoint ComputeOffset() const;

  CPoint m_center;
  std::optional<CPoint> m_mouseMoveStart;
  std::optional<CPoint> m_mouseMoveCurrent;
};

CPoint CWndGridImpl::ComputeOffset() const {
  CPoint delta = m_center;
  if (m_mouseMoveStart && m_mouseMoveCurrent) {
    delta += m_mouseMoveCurrent.value() - m_mouseMoveStart.value();
  }
  return delta;
}

void CWndGridImpl::OnDraw(C2DRender * p2DRender) {
  p2DRender->RenderFillRect(CRect(CPoint(-5, -5), CSize(700, 700)), 0xFF00EECC);
  
  constexpr auto FormatPoint = [](CPoint point) {
    return std::format("({}, {})", point.x, point.y);
  };

  const auto FormatOptPoint = [&](std::optional<CPoint> point) -> std::string {
    return point.transform(FormatPoint).value_or("(none)");
  };

  std::string s = "Center = " + FormatPoint(m_center)
    + " ; Hovered = " + FormatOptPoint(m_displayLayout.m_hovered.point1)
    + "/" + FormatOptPoint(m_displayLayout.m_hovered.point2);

  p2DRender->TextOut(5, 5, s.c_str(), 0xFF0000FF);

  const CPoint offset = ComputeOffset();

  m_displayLayout.Render(p2DRender, offset);
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

  m_displayLayout.Update(m_pFont, m_layout);

  


  

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

void CWndGridImpl::OnLButtonDown(UINT nFlags, CPoint point) {
  m_displayLayout.OnClick();
}

void CWndGridImpl::OnMButtonUp(UINT, CPoint) {
  g_WndMng.PutString("Hey catch this!");

  std::string json = m_layout.ToJson();

  g_WndMng.PutString(json.c_str());
  g_WndMng.PutString("Now you just have to run some OCR program on the text!");

  CClipboard::SetText(json.c_str());
}

void CWndGridImpl::OnMouseWndSurface(CPoint point) {
  if (m_mouseMoveStart) {
    m_displayLayout.SetHovered(std::nullopt);
  } else {
    m_displayLayout.SetHovered(point - m_center);
  }
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


