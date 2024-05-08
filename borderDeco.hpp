#pragma once

#define WLR_USE_UNSTABLE

#include <hyprland/src/render/decorations/IHyprWindowDecoration.hpp>


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

  private:
    SWindowDecorationExtents m_seExtents;

    PHLWINDOWREF             m_pWindow;

    CBox                     m_bLastRelativeBox;

    Vector2D                 m_vLastWindowPos;
    Vector2D                 m_vLastWindowSize;

    double                   m_fLastThickness = 0;
};
