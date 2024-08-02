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
    static auto* const                        PBORDERS = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-border:add_borders")->getDataStaticPtr();

    static std::vector<Hyprlang::INT* const*> PSIZES;
    for (size_t i = 0; i < 9; ++i) {
        PSIZES.push_back((Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-border:border_size_" + std::to_string(i + 1))->getDataStaticPtr());
    }

    SDecorationPositioningInfo info;
    info.policy   = DECORATION_POSITION_ABSOLUTE;
    info.reserved = true;
    info.priority = 9990;
    info.edges    = DECORATION_EDGE_BOTTOM | DECORATION_EDGE_LEFT | DECORATION_EDGE_RIGHT | DECORATION_EDGE_TOP;

    if (m_fLastThickness == 0) {
        double size = 0;

        for (size_t i = 0; i < **PBORDERS; ++i) {
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
    ; // ignored
}

uint64_t CFancyBorder::getDecorationFlags() {
    return 0;
}

eDecorationLayer CFancyBorder::getDecorationLayer() {
    return DECORATION_LAYER_OVER;
}

    
std::string CFancyBorder::getDisplayName() {
    return "FancyBorders";
}

void CFancyBorder::draw(CMonitor* pMonitor, float a) {
    if (!validMapped(m_pWindow))
        return;

	  const auto PWINDOW = m_pWindow.lock();

    if(!PWINDOW->m_sWindowData.decorate.valueOrDefault())
        return;

    static std::vector<Hyprlang::INT* const*> PCOLORS;
    static std::vector<Hyprlang::INT* const*> PSIZES;
    for (size_t i = 0; i < 9; ++i) {
        PCOLORS.push_back((Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-border:col.border_" + std::to_string(i + 1))->getDataStaticPtr());
        PSIZES.push_back((Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-border:border_size_" + std::to_string(i + 1))->getDataStaticPtr());
    }
    static auto* const PBORDERS      = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-border:add_borders")->getDataStaticPtr();
            //grad,
    static auto* const PNATURALROUND = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:fancy-border:natural_rounding")->getDataStaticPtr();
    static auto* const PROUNDING     = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "decoration:rounding")->getDataStaticPtr();
    static auto* const PBORDERSIZE   = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "general:border_size")->getDataStaticPtr();

    if (**PBORDERS < 1)
        return;

    const auto PWORKSPACE      = PWINDOW->m_pWorkspace;
    const auto WORKSPACEOFFSET = PWORKSPACE && !PWINDOW->m_bPinned ? PWORKSPACE->m_vRenderOffset.value() : Vector2D();

    auto       rounding      = PWINDOW->rounding() == 0 ? 0 : PWINDOW->rounding() * pMonitor->scale + **PBORDERSIZE;
    const auto ORIGINALROUND = rounding == 0 ? 0 : PWINDOW->rounding() * pMonitor->scale + **PBORDERSIZE;
    CBox       fullBox       = {m_vLastWindowPos.x, m_vLastWindowPos.y, m_vLastWindowSize.x, m_vLastWindowSize.y};

    fullBox.translate(PWINDOW->m_vFloatingOffset - pMonitor->vecPosition + WORKSPACEOFFSET).scale(pMonitor->scale);

    // create box fullbox.xy, 100x100 in pixels
    //CBox square_test = {fullBox.x, fullBox.y, 100.0, 100.0};

    double fullThickness = 0;

    fullBox.x -= **PBORDERSIZE * pMonitor->scale;
    fullBox.y -= **PBORDERSIZE * pMonitor->scale;
    fullBox.width += **PBORDERSIZE * 2 * pMonitor->scale;
    fullBox.height += **PBORDERSIZE * 2 * pMonitor->scale;


    for (size_t i = 0; i < **PBORDERS; ++i) {
        const int PREVBORDERSIZESCALED = i == 0 ? 0 : (**PSIZES[i - 1] == -1 ? **PBORDERSIZE : **(PSIZES[i - 1])) * pMonitor->scale;
        const int THISBORDERSIZE       = **(PSIZES[i]) == -1 ? **PBORDERSIZE : (**PSIZES[i]);

        if (i != 0) {
            rounding += rounding == 0 ? 0 : PREVBORDERSIZESCALED / pMonitor->scale;
            fullBox.x -= PREVBORDERSIZESCALED;
            fullBox.y -= PREVBORDERSIZESCALED;
            fullBox.width += PREVBORDERSIZESCALED * 2;
            fullBox.height += PREVBORDERSIZESCALED * 2;
        }

        if (fullBox.width < 1 || fullBox.height < 1)
            break;

        g_pHyprOpenGL->scissor((CBox*)nullptr);

        renderBorder(
            &fullBox, 
            CColor{(uint64_t) * *PCOLORS[i]}, //grad
            **PNATURALROUND ? ORIGINALROUND : rounding, 
            THISBORDERSIZE,
            a, //alpha
            **PNATURALROUND ? ORIGINALROUND : -1);

        fullThickness += THISBORDERSIZE;
    }

    m_seExtents = {{fullThickness, fullThickness}, {fullThickness, fullThickness}};

    // create image texture, just a red square currently
    //if( m_tBorderShape->m_iTexID == 0 )
    //    renderBorderTexture();

    // draw image texture to window
    //if( m_tBorderShape->m_iTexID != 0 )
    //    g_pHyprOpenGL->renderTexture(m_tBorderShape, &square_test, a);

	  m_bLastRelativeBox = CBox{0, 0, m_vLastWindowSize.x, m_vLastWindowSize.y}.expand(**PBORDERSIZE).addExtents(m_seExtents);
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

// copy of hyprland src/Hyprland/src/render/OpenGl.cpp:renderBorder
// scissor(), blend(), m_RenderData and m_pCurrentWindow obtained from g_pHyprOpenGL
// shBORDER1 shader program replaced with local g_pGlobalState->borderShader
// added bool m_bEndFrame = false; // this normally does not get set to true with this call
// replaced m_bBlend with glIsEnabled(GL_BLEND); to get state from opengl directly
void CFancyBorder::renderBorder(CBox* box, const CGradientValueData& grad, int round, int borderSize, float a, int outerRound) {
    RASSERT((box->width > 0 && box->height > 0), "Tried to render rect with width/height < 0!");
    RASSERT(g_pHyprOpenGL->m_RenderData.pMonitor, "Tried to render rect without begin()!");

    TRACY_GPU_ZONE("RenderBorder");

    bool m_bEndFrame = false; // normally false

    if (g_pHyprOpenGL->m_RenderData.damage.empty() || (g_pHyprOpenGL->m_pCurrentWindow.lock() && g_pHyprOpenGL->m_pCurrentWindow->m_sWindowData.noBorder.valueOrDefault()))
        return;

    CBox newBox = *box;
    g_pHyprOpenGL->m_RenderData.renderModif.applyToBox(newBox);

    box = &newBox;

    if (borderSize < 1)
        return;

    int scaledBorderSize = std::round(borderSize * g_pHyprOpenGL->m_RenderData.pMonitor->scale);
    scaledBorderSize     = std::round(scaledBorderSize * g_pHyprOpenGL->m_RenderData.renderModif.combinedScale());

    // adjust box
    box->x -= scaledBorderSize;
    box->y -= scaledBorderSize;
    box->width += 2 * scaledBorderSize;
    box->height += 2 * scaledBorderSize;

    round += round == 0 ? 0 : scaledBorderSize;

    float matrix[9];
    projectBox(matrix, newBox, wlTransformToHyprutils(invertTransform(!m_bEndFrame ? WL_OUTPUT_TRANSFORM_NORMAL : g_pHyprOpenGL->m_RenderData.pMonitor->transform)), newBox.rot,
               g_pHyprOpenGL->m_RenderData.monitorProjection.data());

    float glMatrix[9];
    matrixMultiply(glMatrix, g_pHyprOpenGL->m_RenderData.projection, matrix);

    //const auto BLEND = m_bBlend;
    //blend(true);
    const auto BLEND = glIsEnabled(GL_BLEND);
    g_pHyprOpenGL->blend(true);


    glUseProgram(g_pGlobalState->borderShader.program);

#ifndef GLES2
    glUniformMatrix3fv(g_pGlobalState->borderShader.proj, 1, GL_TRUE, glMatrix);
#else
    matrixTranspose(glMatrix, glMatrix);
    glUniformMatrix3fv(g_pGlobalState->borderShader.proj, 1, GL_FALSE, glMatrix);
#endif

    static_assert(sizeof(CColor) == 4 * sizeof(float)); // otherwise the line below this will fail

    glUniform4fv(g_pGlobalState->borderShader.gradient, grad.m_vColors.size(), (float*)grad.m_vColors.data());
    glUniform1i(g_pGlobalState->borderShader.gradientLength, grad.m_vColors.size());
    glUniform1f(g_pGlobalState->borderShader.angle, (int)(grad.m_fAngle / (PI / 180.0)) % 360 * (PI / 180.0));
    glUniform1f(g_pGlobalState->borderShader.alpha, a);

    CBox transformedBox = *box;
    transformedBox.transform(wlTransformToHyprutils(invertTransform(g_pHyprOpenGL->m_RenderData.pMonitor->transform)), g_pHyprOpenGL->m_RenderData.pMonitor->vecTransformedSize.x,
                             g_pHyprOpenGL->m_RenderData.pMonitor->vecTransformedSize.y);

    const auto TOPLEFT  = Vector2D(transformedBox.x, transformedBox.y);
    const auto FULLSIZE = Vector2D(transformedBox.width, transformedBox.height);

    glUniform2f(g_pGlobalState->borderShader.topLeft, (float)TOPLEFT.x, (float)TOPLEFT.y);
    glUniform2f(g_pGlobalState->borderShader.fullSize, (float)FULLSIZE.x, (float)FULLSIZE.y);
    glUniform2f(g_pGlobalState->borderShader.fullSizeUntransformed, (float)box->width, (float)box->height);
    glUniform1f(g_pGlobalState->borderShader.radius, round);
    glUniform1f(g_pGlobalState->borderShader.radiusOuter, outerRound == -1 ? round : outerRound);
    glUniform1f(g_pGlobalState->borderShader.thick, scaledBorderSize);

    glVertexAttribPointer(g_pGlobalState->borderShader.posAttrib, 2, GL_FLOAT, GL_FALSE, 0, fullVerts);
    glVertexAttribPointer(g_pGlobalState->borderShader.texAttrib, 2, GL_FLOAT, GL_FALSE, 0, fullVerts);

    glEnableVertexAttribArray(g_pGlobalState->borderShader.posAttrib);
    glEnableVertexAttribArray(g_pGlobalState->borderShader.texAttrib);

    if (g_pHyprOpenGL->m_RenderData.clipBox.width != 0 && g_pHyprOpenGL->m_RenderData.clipBox.height != 0) {
        CRegion damageClip{g_pHyprOpenGL->m_RenderData.clipBox.x, g_pHyprOpenGL->m_RenderData.clipBox.y, g_pHyprOpenGL->m_RenderData.clipBox.width, g_pHyprOpenGL->m_RenderData.clipBox.height};
        damageClip.intersect(g_pHyprOpenGL->m_RenderData.damage);

        if (!damageClip.empty()) {
            for (auto& RECT : damageClip.getRects()) {
                g_pHyprOpenGL->scissor(&RECT);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    } else {
        for (auto& RECT : g_pHyprOpenGL->m_RenderData.damage.getRects()) {
            g_pHyprOpenGL->scissor(&RECT);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    glDisableVertexAttribArray(g_pGlobalState->borderShader.posAttrib);
    glDisableVertexAttribArray(g_pGlobalState->borderShader.texAttrib);

    g_pHyprOpenGL->blend(BLEND);
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
