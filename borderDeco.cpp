#include "borderDeco.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <pango/pangocairo.h>

#include "globals.hpp"

CFancyBorder::CFancyBorder(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow), m_pWindow(pWindow) {
    m_vLastWindowPos  = pWindow->m_vRealPosition.value();
    m_vLastWindowSize = pWindow->m_vRealSize.value();

    //m_tBorderShape = makeShared<CTexture>();
}

CFancyBorder::~CFancyBorder() {
    damageEntire();
}

SDecorationPositioningInfo CFancyBorder::getPositioningInfo() {
    static auto* const                        PBORDERS = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-borders:add_borders")->getDataStaticPtr();

    static std::vector<Hyprlang::INT* const*> PSIZES;
    for (size_t i = 0; i < 9; ++i) {
        PSIZES.push_back((Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-borders:border_size_" + std::to_string(i + 1))->getDataStaticPtr());
    }

    SDecorationPositioningInfo info;
    info.policy   = DECORATION_POSITION_STICKY;
    info.reserved = true;
    info.priority = 9990;
    info.edges    = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;

    if (m_fLastThickness == 0) {
        double size = 0;

        for (int i = 0; i < **PBORDERS; ++i) {
            size += **PSIZES[i];
        }

        info.desiredExtents = {{size, size}, {size, size}};
        m_fLastThickness    = size;
    } else {
        info.desiredExtents = {{m_fLastThickness, m_fLastThickness}, {m_fLastThickness, m_fLastThickness}};
    }

    return info;
}

void CFancyBorder::onPositioningReply(const SDecorationPositioningReply& reply) {
    m_bAssignedGeometry = reply.assignedGeometry;
}

uint64_t CFancyBorder::getDecorationFlags() {
    return DECORATION_PART_OF_MAIN_WINDOW;
}

eDecorationLayer CFancyBorder::getDecorationLayer() {
    return DECORATION_LAYER_OVER;
}

    
std::string CFancyBorder::getDisplayName() {
    return "FancyBorders";
}

void CFancyBorder::draw(PHLMONITOR pMonitor, float const& a) {
    if (!validMapped(m_pWindow))
        return;

	  const auto PWINDOW = m_pWindow.lock();

    if(!PWINDOW->m_sWindowData.decorate.valueOrDefault())
        return;

    static std::vector<Hyprlang::INT* const*> PCOLORS;
    static std::vector<Hyprlang::INT* const*> PSIZES;
    for (size_t i = 0; i < 9; ++i) {
        PCOLORS.push_back((Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-borders:col.border_" + std::to_string(i + 1))->getDataStaticPtr());
        PSIZES.push_back((Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-borders:border_size_" + std::to_string(i + 1))->getDataStaticPtr());
    }
    static auto* const PBORDERS      = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-borders:add_borders")->getDataStaticPtr();
    static auto* const PBORDERSIZE   = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "general:border_size")->getDataStaticPtr();

    if (**PBORDERS < 1)
        return;

    if (m_bAssignedGeometry.width < m_seExtents.topLeft.x + 1 || m_bAssignedGeometry.height < m_seExtents.topLeft.y + 1)
        return;

    const auto PWORKSPACE      = PWINDOW->m_pWorkspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !PWINDOW->m_bPinned ? PWORKSPACE->m_vRenderOffset.value() : Vector2D();

    auto       rounding      = PWINDOW->rounding() == 0 ? 0 : (PWINDOW->rounding() + **PBORDERSIZE) * pMonitor->scale;

    CBox       fullBox = m_bAssignedGeometry;
    fullBox.translate(g_pDecorationPositioner->getEdgeDefinedPoint(DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP, m_pWindow.lock()));

    fullBox.translate(PWINDOW->m_vFloatingOffset - pMonitor->vecPosition + WORKSPACEOFFSET);

    if (fullBox.width < 1 || fullBox.height < 1)
        return;

    double fullThickness = 0;

    for (int i = 0; i < **PBORDERS; ++i) {
        const int THISBORDERSIZE = **(PSIZES[i]) == -1 ? **PBORDERSIZE : (**PSIZES[i]);
        fullThickness += THISBORDERSIZE;
    }

    fullBox.expand(-fullThickness).scale(pMonitor->scale).round();

    for (int i = 0; i < **PBORDERS; ++i) {
        const int PREVBORDERSIZESCALED = i == 0 ? 0 : (**PSIZES[i - 1] == -1 ? **PBORDERSIZE : **(PSIZES[i - 1])) * pMonitor->scale;
        const int THISBORDERSIZE       = **(PSIZES[i]) == -1 ? **PBORDERSIZE : (**PSIZES[i]);

        if (i != 0) {
            rounding += rounding == 0 ? 0 : PREVBORDERSIZESCALED;
            fullBox.x -= PREVBORDERSIZESCALED;
            fullBox.y -= PREVBORDERSIZESCALED;
            fullBox.width += PREVBORDERSIZESCALED * 2;
            fullBox.height += PREVBORDERSIZESCALED * 2;
        }

        if (fullBox.width < 1 || fullBox.height < 1)
            break;

        scissor((CBox*)nullptr);

        m_bBlend = glIsEnabled(GL_BLEND);
        m_bEndFrame = false;
        m_RenderData = g_pHyprOpenGL->m_RenderData;
        m_pCurrentWindow = g_pHyprOpenGL->m_pCurrentWindow;

        renderBorder(
            &fullBox, 
            CHyprColor{(uint64_t) **PCOLORS[i]}, //grad
            rounding, 
            THISBORDERSIZE,
            a, //alpha
            -1);
    }

    m_seExtents = {{fullThickness, fullThickness}, {fullThickness, fullThickness}};

    // create image texture, just a red square currently
    //if( m_tBorderShape->m_iTexID == 0 )
    //    renderBorderTexture();

    // draw image texture to window
    //if( m_tBorderShape->m_iTexID != 0 )
    //    g_pHyprOpenGL->renderTexture(m_tBorderShape, &square_test, a);

	  m_bLastRelativeBox = CBox{0, 0, m_vLastWindowSize.x, m_vLastWindowSize.y}.addExtents(m_seExtents);

    if (fullThickness != m_fLastThickness) {
        m_fLastThickness = fullThickness;
        g_pDecorationPositioner->repositionDeco(this);
    }
}

void CFancyBorder::renderBorderTexture() {
    const auto CAIROSURFACE = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 100, 100);
    const auto CAIRO        = cairo_create(CAIROSURFACE);

    // clear the pixmap
    cairo_save(CAIRO);
    cairo_set_operator(CAIRO, CAIRO_OPERATOR_CLEAR);
    cairo_paint(CAIRO);
    cairo_restore(CAIRO);

    // draw stuff
    cairo_set_source_rgba(CAIRO, 1.0, 0.0, 0.0, 1.0);
    cairo_rectangle (CAIRO, 0.0, 0.0, 25.0, 100.0);
    cairo_fill (CAIRO);
    cairo_set_source_rgba(CAIRO, 0.0, 1.0, 0.0, 1.0);
    cairo_rectangle (CAIRO, 25.0, 0.0, 25.0, 100.0);
    cairo_fill (CAIRO);
    cairo_set_source_rgba(CAIRO, 0.0, 0.0, 1.0, 1.0);
    cairo_rectangle (CAIRO, 50.0, 0.0, 25.0, 100.0);
    cairo_fill (CAIRO);
    cairo_set_source_rgba(CAIRO, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle (CAIRO, 75.0, 0.0, 25.0, 100.0);
    cairo_fill (CAIRO);
    //cairo_surface_flush(CAIROSURFACE);

    // cairo to texture
    const auto DATA = cairo_image_surface_get_data(CAIROSURFACE);
    m_tBorderShape->allocate();
    glBindTexture(GL_TEXTURE_2D, m_tBorderShape->m_iTexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#ifndef GLES2
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_BLUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, DATA);

    // delete cairo
    cairo_destroy(CAIRO);
    cairo_surface_destroy(CAIROSURFACE);
}

eDecorationType CFancyBorder::getDecorationType() {
    return DECORATION_CUSTOM;
}

void CFancyBorder::updateWindow(PHLWINDOW pWindow) {
    m_vLastWindowPos  = pWindow->m_vRealPosition.value();
    m_vLastWindowSize = pWindow->m_vRealSize.value();

    damageEntire();
}

void CFancyBorder::damageEntire() {
    CBox dm = m_bLastRelativeBox.copy().translate(m_vLastWindowPos).expand(2);
    g_pHyprRenderer->damageBox(&dm);
}
