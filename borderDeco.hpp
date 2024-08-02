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

    virtual void                       onPositioningReply(const SDecorationPositioningReply& reply);

    virtual void                       draw(CMonitor*, float a);

    virtual eDecorationType            getDecorationType();

    virtual void                       updateWindow(PHLWINDOW);

    virtual void                       damageEntire();

    virtual uint64_t                   getDecorationFlags();

    virtual eDecorationLayer           getDecorationLayer();

    virtual std::string                getDisplayName();

    void                               renderBorder(CBox*, const CGradientValueData&, int round, int borderSize, float a = 1.0, int outerRound = -1 /* use round */);

    void                               renderBorderTexture();

  private:
    //SWindowDecorationExtents m_seExtents;
    SBoxExtents              m_seExtents;

    PHLWINDOWREF             m_pWindow;

    CBox                     m_bLastRelativeBox;

    Vector2D                 m_vLastWindowPos;
    Vector2D                 m_vLastWindowSize;

    SP<CTexture>             m_tBorderShape;

    double                   m_fLastThickness = 0;
};
