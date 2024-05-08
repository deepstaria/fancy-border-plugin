#include "borderDeco.hpp"

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>

#include "globals.hpp"

CFancyBorder::CFancyBorder(PHLWINDOW pWindow) : IHyprWindowDecoration(pWindow), m_pWindow(pWindow) {
    m_vLastWindowPos  = pWindow->m_vRealPosition.value();
    m_vLastWindowSize = pWindow->m_vRealSize.value();
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
    if (!PWINDOW->m_sSpecialRenderData.decorate)
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

	m_bLastRelativeBox = CBox{0, 0, m_vLastWindowSize.x, m_vLastWindowSize.y}.expand(**PBORDERSIZE).addExtents(m_seExtents);
    if (fullThickness != m_fLastThickness) {
        m_fLastThickness = fullThickness;
        g_pDecorationPositioner->repositionDeco(this);
    }
}


void CFancyBorder::renderBorder(CBox* box, const CGradientValueData& grad, int round, int borderSize, float a, int outerRound) {
    RASSERT((box->width > 0 && box->height > 0), "Tried to render rect with width/height < 0!");
    RASSERT(g_pHyprOpenGL->m_RenderData.pMonitor, "Tried to render rect without begin()!");

    TRACY_GPU_ZONE("RenderBorder");

    bool m_bEndFrame = false; // normally false


    if (g_pHyprOpenGL->m_RenderData.damage.empty() || (g_pHyprOpenGL->m_pCurrentWindow.lock() && g_pHyprOpenGL->m_pCurrentWindow->m_sAdditionalConfigData.forceNoBorder))
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
    wlr_matrix_project_box(matrix, box->pWlr(), wlr_output_transform_invert(!m_bEndFrame ? WL_OUTPUT_TRANSFORM_NORMAL : g_pHyprOpenGL->m_RenderData.pMonitor->transform), newBox.rot,
                           g_pHyprOpenGL->m_RenderData.pMonitor->projMatrix.data()); // TODO: write own, don't use WLR here

    float glMatrix[9]; 
    wlr_matrix_multiply(glMatrix, g_pHyprOpenGL->m_RenderData.projection, matrix);
    
    const auto BLEND = glIsEnabled(GL_BLEND); //m_bBlend;
    g_pHyprOpenGL->blend(true);

    glUseProgram(g_pGlobalState->borderShader.program);

#ifndef GLES2
    glUniformMatrix3fv(g_pGlobalState->borderShader.proj, 1, GL_TRUE, glMatrix);
#else
    wlr_matrix_transpose(glMatrix, glMatrix);
    glUniformMatrix3fv(g_pGlobalState->borderShader.proj, 1, GL_FALSE, glMatrix);
#endif

    static_assert(sizeof(CColor) == 4 * sizeof(float)); // otherwise the line below this will fail

    glUniform4fv(g_pGlobalState->borderShader.gradient, grad.m_vColors.size(), (float*)grad.m_vColors.data());
    glUniform1i(g_pGlobalState->borderShader.gradientLength, grad.m_vColors.size());
    glUniform1f(g_pGlobalState->borderShader.angle, (int)(grad.m_fAngle / (PI / 180.0)) % 360 * (PI / 180.0));
    glUniform1f(g_pGlobalState->borderShader.alpha, a);

    CBox transformedBox = *box;
    transformedBox.transform(wlr_output_transform_invert(g_pHyprOpenGL->m_RenderData.pMonitor->transform), g_pHyprOpenGL->m_RenderData.pMonitor->vecTransformedSize.x,
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
