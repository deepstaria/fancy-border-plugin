#define WLR_USE_UNSTABLE

#include <unistd.h>

//#include <any>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/config/ConfigManager.hpp>

#include "borderDeco.hpp"
#include "globals.hpp"
#include "shaders/Border.hpp"
//#include "shaders/Shadow.hpp"
#include "shaders/Textures.hpp"


// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

void onNewWindow(void* self, std::any data) {
    // data is guaranteed
    auto* const PWINDOW = std::any_cast<CWindow*>(data);

    HyprlandAPI::addWindowDecoration(PHANDLE, PWINDOW, std::make_unique<CFancyBorder>(PWINDOW));
}

GLuint CompileShader(const GLuint& type, std::string src) {
    auto shader = glCreateShader(type);

    auto shaderSource = src.c_str();

    glShaderSource(shader, 1, (const GLchar**)&shaderSource, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

    if (ok == GL_FALSE)
        throw std::runtime_error("compileShader() failed!");

    return shader;
}

GLuint CreateProgram(const std::string& vert, const std::string& frag) {
    auto vertCompiled = CompileShader(GL_VERTEX_SHADER, vert);

    if (!vertCompiled)
        throw std::runtime_error("Compiling vshader failed.");

    auto fragCompiled = CompileShader(GL_FRAGMENT_SHADER, frag);

    if (!fragCompiled)
        throw std::runtime_error("Compiling fshader failed.");

    auto prog = glCreateProgram();
    glAttachShader(prog, vertCompiled);
    glAttachShader(prog, fragCompiled);
    glLinkProgram(prog);

    glDetachShader(prog, vertCompiled);
    glDetachShader(prog, fragCompiled);
    glDeleteShader(vertCompiled);
    glDeleteShader(fragCompiled);

    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);

    if (ok == GL_FALSE)
        throw std::runtime_error("createProgram() failed! GL_LINK_STATUS not OK!");

    return prog;
}

int onTick(void* data) {
    EMIT_HOOK_EVENT("trailTick", nullptr);

    const int TIMEOUT = g_pHyprRenderer->m_pMostHzMonitor ? 1000.0 / g_pHyprRenderer->m_pMostHzMonitor->refreshRate : 16;
    wl_event_source_timer_update(g_pGlobalState->tick, TIMEOUT);

    return 0;
}

void initGlobal() {
    RASSERT(eglMakeCurrent(wlr_egl_get_display(g_pCompositor->m_sWLREGL), EGL_NO_SURFACE, EGL_NO_SURFACE, wlr_egl_get_context(g_pCompositor->m_sWLREGL)),
            "Couldn't set current EGL!");

    GLuint prog                              = CreateProgram(QUADVERTSRC, FRAGBORDER2);
    g_pGlobalState->borderShader1.program     = prog;
    g_pGlobalState->borderShader1.proj        = glGetUniformLocation(prog, "proj");
    g_pGlobalState->borderShader1.thick       = glGetUniformLocation(prog, "thick");
    g_pGlobalState->borderShader1.posAttrib   = glGetAttribLocation(prog, "pos");
    g_pGlobalState->borderShader1.texAttrib   = glGetAttribLocation(prog, "texcoord");
    g_pGlobalState->borderShader1.topLeft     = glGetUniformLocation(prog, "topLeft");
    g_pGlobalState->borderShader1.bottomRight = glGetUniformLocation(prog, "bottomRight");
    g_pGlobalState->borderShader1.fullSize    = glGetUniformLocation(prog, "fullSize");
    g_pGlobalState->borderShader1.fullSizeUntransformed = glGetUniformLocation(prog, "fullSizeUntransformed");
    g_pGlobalState->borderShader1.radius      = glGetUniformLocation(prog, "radius");
    g_pGlobalState->borderShader1.radiusOuter = glGetUniformLocation(prog, "radiusOuter");
    g_pGlobalState->borderShader1.gradient    = glGetUniformLocation(prog, "gradient");
    g_pGlobalState->borderShader1.gradientLength = glGetUniformLocation(prog, "gradientLength");
    g_pGlobalState->borderShader1.angle       = glGetUniformLocation(prog, "angle");
    g_pGlobalState->borderShader1.alpha       = glGetUniformLocation(prog, "alpha");

    g_pGlobalState->borderShader0.program     = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.program;
    g_pGlobalState->borderShader0.proj        = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.proj;
    g_pGlobalState->borderShader0.thick       = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.thick;
    g_pGlobalState->borderShader0.posAttrib   = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.posAttrib;
    g_pGlobalState->borderShader0.texAttrib   = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.texAttrib;
    g_pGlobalState->borderShader0.topLeft     = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.topLeft;
    g_pGlobalState->borderShader0.bottomRight = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.bottomRight;
    g_pGlobalState->borderShader0.fullSize    = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.fullSize;
    g_pGlobalState->borderShader0.fullSizeUntransformed = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.fullSizeUntransformed;
    g_pGlobalState->borderShader0.radius      = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.radius;
    g_pGlobalState->borderShader0.radiusOuter = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.radiusOuter;
    g_pGlobalState->borderShader0.gradient    = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.gradient;
    g_pGlobalState->borderShader0.gradientLength = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.gradientLength;
    g_pGlobalState->borderShader0.angle       = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.angle;
    g_pGlobalState->borderShader0.alpha       = g_pHyprOpenGL->m_RenderData.pCurrentMonData->m_shBORDER1.alpha;

    RASSERT(eglMakeCurrent(wlr_egl_get_display(g_pCompositor->m_sWLREGL), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT), "Couldn't unset current EGL!");

    g_pGlobalState->tick = wl_event_loop_add_timer(g_pCompositor->m_sWLEventLoop, &onTick, nullptr);
    wl_event_source_timer_update(g_pGlobalState->tick, 1);
}


APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[fancy-border] Failure in initialization: Version mismatch (headers ver is not equal to running hyprland ver)",
                                     CColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[bpp] Version mismatch");
    }

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:fancy-border:add_borders", Hyprlang::INT{1});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:fancy-border:natural_rounding", Hyprlang::INT{1});

    for (size_t i = 0; i < 9; ++i) {
        HyprlandAPI::addConfigValue(PHANDLE, "plugin:fancy-border:col.border_" + std::to_string(i + 1), Hyprlang::INT{configStringToInt("rgba(000000ee)")});
        HyprlandAPI::addConfigValue(PHANDLE, "plugin:fancy-border:border_size_" + std::to_string(i + 1), Hyprlang::INT{-1});
    }

    HyprlandAPI::reloadConfig();

    HyprlandAPI::registerCallbackDynamic(PHANDLE, "openWindow", [&](void* self, SCallbackInfo& info, std::any data) { onNewWindow(self, data); });

    g_pGlobalState = std::make_unique<SGlobalState>();
    initGlobal();

    // add deco to existing windows
    for (auto& w : g_pCompositor->m_vWindows) {
        if (w->isHidden() || !w->m_bIsMapped)
            continue;

        HyprlandAPI::addWindowDecoration(PHANDLE, w.get(), std::make_unique<CFancyBorder>(w.get()));
    }

    HyprlandAPI::addNotification(PHANDLE, "[fancy-border] Initialized successfully!", CColor{0.2, 1.0, 0.2, 1.0}, 5000);

    return {"fancy-border", "A plugin to add more borders to windows.", "Ren", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    wl_event_source_remove(g_pGlobalState->tick);
    CFancyBorder::unhijackShader();
}
