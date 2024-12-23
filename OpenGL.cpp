#include "borderDeco.hpp"
#include "globals.hpp"

// wrapper for scissor
void CFancyBorder::scissor(const CBox* pBox, bool transform ) {
  g_pHyprOpenGL->scissor(pBox, transform);
}

// wrapper for scissor
void CFancyBorder::scissor(const pixman_box32* pBox, bool transform ) {
  g_pHyprOpenGL->scissor(pBox, transform);
}

// wrapper for blend()
void CFancyBorder::blend(bool enabled) {
  g_pHyprOpenGL->blend(enabled);
}

// copy of hyprland src/Hyprland/src/render/OpenGl.cpp:renderBorder
// :%s/g_pGlobalState->borderShader/g_pGlobalState->borderShader/g
void CFancyBorder::renderBorder(CBox* box, const CGradientValueData& grad, int round, int borderSize, float a, int outerRound) {
    RASSERT((box->width > 0 && box->height > 0), "Tried to render rect with width/height < 0!");
    RASSERT(m_RenderData.pMonitor, "Tried to render rect without begin()!");

    TRACY_GPU_ZONE("RenderBorder");

    if (m_RenderData.damage.empty() || (m_pCurrentWindow.lock() && m_pCurrentWindow->m_sWindowData.noBorder.valueOrDefault()))
        return;

    CBox newBox = *box;
    m_RenderData.renderModif.applyToBox(newBox);

    box = &newBox;

    if (borderSize < 1)
        return;

    int scaledBorderSize = std::round(borderSize * m_RenderData.pMonitor->scale);
    scaledBorderSize     = std::round(scaledBorderSize * m_RenderData.renderModif.combinedScale());

    // adjust box
    box->x -= scaledBorderSize;
    box->y -= scaledBorderSize;
    box->width += 2 * scaledBorderSize;
    box->height += 2 * scaledBorderSize;

    round += round == 0 ? 0 : scaledBorderSize;

    Mat3x3 matrix = m_RenderData.monitorProjection.projectBox(
        newBox, wlTransformToHyprutils(invertTransform(!m_bEndFrame ? WL_OUTPUT_TRANSFORM_NORMAL : m_RenderData.pMonitor->transform)), newBox.rot);
    Mat3x3     glMatrix = m_RenderData.projection.copy().multiply(matrix);

    const auto BLEND = m_bBlend;
    blend(true); 

    glUseProgram(g_pGlobalState->borderShader.program);

#ifndef GLES2
    glUniformMatrix3fv(g_pGlobalState->borderShader.proj, 1, GL_TRUE, glMatrix.getMatrix().data());
#else
    glMatrix.transpose();
    glUniformMatrix3fv(g_pGlobalState->borderShader.proj, 1, GL_FALSE, glMatrix.getMatrix().data());
#endif

    glUniform4fv(g_pGlobalState->borderShader.gradient, grad.m_vColorsOkLabA.size() / 4, (float*)grad.m_vColorsOkLabA.data());
    glUniform1i(g_pGlobalState->borderShader.gradientLength, grad.m_vColorsOkLabA.size() / 4);
    glUniform1f(g_pGlobalState->borderShader.angle, (int)(grad.m_fAngle / (PI / 180.0)) % 360 * (PI / 180.0));
    glUniform1f(g_pGlobalState->borderShader.alpha, a);
    glUniform1f(g_pGlobalState->borderShader.gradient2Length, 0);

    CBox transformedBox = *box;
    transformedBox.transform(wlTransformToHyprutils(invertTransform(m_RenderData.pMonitor->transform)), m_RenderData.pMonitor->vecTransformedSize.x,
                             m_RenderData.pMonitor->vecTransformedSize.y);

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

    if (m_RenderData.clipBox.width != 0 && m_RenderData.clipBox.height != 0) {
        CRegion damageClip{m_RenderData.clipBox.x, m_RenderData.clipBox.y, m_RenderData.clipBox.width, m_RenderData.clipBox.height};
        damageClip.intersect(m_RenderData.damage);

        if (!damageClip.empty()) {
            for (auto& RECT : damageClip.getRects()) {
                scissor(&RECT);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            }
        }
    } else {
        for (auto& RECT : m_RenderData.damage.getRects()) {
            scissor(&RECT);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    glDisableVertexAttribArray(g_pGlobalState->borderShader.posAttrib);
    glDisableVertexAttribArray(g_pGlobalState->borderShader.texAttrib);

    blend(BLEND);
}

