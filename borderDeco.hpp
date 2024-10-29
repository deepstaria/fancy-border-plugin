#pragma once

#define WLR_USE_UNSTABLE

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>
#include <hyprland/src/render/OpenGL.hpp>


class CGradientValueData;

class CFancyBorder : public IHyprWindowDecoration {
  public:
    CFancyBorder(PHLWINDOW);
    virtual ~CFancyBorder();

    virtual SDecorationPositioningInfo getPositioningInfo();

    virtual void                onPositioningReply(const SDecorationPositioningReply& reply);

    virtual void                draw(PHLMONITOR, float a);

    virtual eDecorationType     getDecorationType();

    virtual void                updateWindow(PHLWINDOW);

    virtual void                damageEntire();

    virtual uint64_t            getDecorationFlags();

    virtual eDecorationLayer    getDecorationLayer();

    virtual std::string         getDisplayName();


  private:
    SBoxExtents                 m_seExtents;

    PHLWINDOWREF                m_pWindow;

    CBox                        m_bLastRelativeBox;
    CBox                        m_bAssignedGeometry;

    Vector2D                    m_vLastWindowPos;
    Vector2D                    m_vLastWindowSize;

    SP<CTexture>                m_tBorderShape;

    double                      m_fLastThickness        = 0;

    void                        renderBorderTexture();

    // from OpenGL.hpp
    void                        renderBorder(CBox*, const CGradientValueData&, int round, int borderSize, float a = 1.0, int outerRound = -1 /* use round */);
    void                        scissor(const CBox*, bool transform = true);
    void                        scissor(const pixman_box32*, bool transform = true);
    void                        blend(bool enabled);
    bool                        m_bEndFrame             = false;
    bool                        m_bBlend                = false;
    SCurrentRenderData          m_RenderData;
    PHLWINDOWREF                m_pCurrentWindow;       // hack to get the current rendered window
};
