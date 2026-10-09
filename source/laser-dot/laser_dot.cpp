// GPU Selector Laser Dot for UZDoom / GZDoom (native vr_mode 3D)
//
// A stand-in dinput8.dll. Everything DirectInput is passed to Windows' own dinput8.dll.
// UZDoom asks Windows for its OpenGL functions by name (GetProcAddress, gl_load.c). We point
// the program's GetProcAddress entry at our own, so a few OpenGL functions come to us first:
//
//   1. glClear / glViewport     remember the 3D scene's framebuffer and its view rectangle
//   2. glInvalidateFramebuffer  read the depth under the aim point before it is thrown away
//   3. glBlitFramebuffer        when an eye's finished picture is copied out (BlitToEyeTexture),
//                               read the depth at the aim point and draw the dot for that eye
//
// The dot sits at the depth of what it points at (like a VR laser sight), a little in front of it.
// No engine code is changed or shipped. Command line (the card adds these):
//   +vr_ipd +vr_screendist +vr_hunits_per_meter +vr_swap_eyes +vr_mode +fov   (UZDoom's own)
//   -laserdot-circle   a mini circle instead of the dot
//   -laserdot-cross    a small cross instead of the dot
//   -laserdot-game     no dot: the game's own crosshair is moved to the depth instead
//   -laserdot-hide     no dot, and the game's own crosshair is hidden (in 3D)
// With the dot, circle or cross, the game's own crosshair is hidden in 3D too. The DLL does that
// itself (it skips that one draw), so the port's own crosshair settings are never changed.
//   -laserdot-off      no dot
//   -laserdot-color=RRGGBB  its color (hex, default FF2619 red)
//   -laserdot-size=N   dot size in percent (50-200, 100 = normal)
//   -laserdot-hover=N  how far it floats in front, in percent (0-300, 100 = normal)

#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef float GLfloat;
typedef unsigned char GLboolean;

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_SCISSOR_TEST 0x0C11
#define GL_BLEND 0x0BE2
#define GL_DEPTH_TEST 0x0B71
#define GL_COLOR_WRITEMASK 0x0C23
#define GL_VIEWPORT 0x0BA2
#define GL_TRIANGLES 0x0004
#define GL_UNSIGNED_INT 0x1405
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_STREAM_DRAW 0x88E0
#define GL_SCISSOR_BOX 0x0C10
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_DEPTH_COMPONENT 0x1902
#define GL_DEPTH 0x1801
#define GL_FLOAT 0x1406
#define GL_NEAREST 0x2600
#define GL_NONE 0
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER 0x8D41
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED

// ---- OpenGL functions (the real ones) ----

typedef PROC(WINAPI* WglGetProcAddressFn)(LPCSTR);
typedef HDC(WINAPI* WglGetCurrentDCFn)();
typedef void(WINAPI* ClearFn)(GLbitfield);
typedef void(WINAPI* ViewportFn)(GLint, GLint, GLsizei, GLsizei);
typedef void(WINAPI* GetIntegervFn)(GLenum, GLint*);
typedef void(WINAPI* GetFloatvFn)(GLenum, GLfloat*);
typedef void(WINAPI* GetBooleanvFn)(GLenum, GLboolean*);
typedef void(WINAPI* DepthMaskFn)(GLboolean);
typedef void(WINAPI* BufferDataFn)(GLenum, ptrdiff_t, const void*, GLenum);
typedef void(WINAPI* DrawElementsFn)(GLenum, GLsizei, GLenum, const void*);
typedef GLboolean(WINAPI* IsEnabledFn)(GLenum);
typedef void(WINAPI* EnableFn)(GLenum);
typedef void(WINAPI* ScissorFn)(GLint, GLint, GLsizei, GLsizei);
typedef void(WINAPI* ClearColorFn)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void(WINAPI* ReadPixelsFn)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
typedef GLenum(WINAPI* GetErrorFn)();
typedef void(WINAPI* BlitFn)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
typedef void(WINAPI* InvalidateFn)(GLenum, GLsizei, const GLenum*);
typedef void(WINAPI* BindFramebufferFn)(GLenum, GLuint);
typedef void(WINAPI* GenFn)(GLsizei, GLuint*);
typedef void(WINAPI* BindRenderbufferFn)(GLenum, GLuint);
typedef void(WINAPI* RenderbufferStorageFn)(GLenum, GLenum, GLsizei, GLsizei);
typedef void(WINAPI* FramebufferRenderbufferFn)(GLenum, GLenum, GLenum, GLuint);
typedef void(WINAPI* GetAttachmentFn)(GLenum, GLenum, GLenum, GLint*);
typedef void(WINAPI* BindBufferFn)(GLenum, GLuint);

static WglGetProcAddressFn realWglGetProcAddress;
static ClearFn realClear;
static ViewportFn realViewport;
static DepthMaskFn realDepthMask;
static BufferDataFn realBufferData;
static DrawElementsFn realDrawElements;
static BlitFn realBlit;
static InvalidateFn realInvalidate;

static struct
{
    bool loaded;
    WglGetCurrentDCFn getCurrentDC;
    GetIntegervFn getIntegerv;
    GetFloatvFn getFloatv;
    GetBooleanvFn getBooleanv;
    IsEnabledFn isEnabled;
    EnableFn enable;
    EnableFn disable;
    ScissorFn scissor;
    ClearColorFn clearColor;
    ReadPixelsFn readPixels;
    GetErrorFn getError;
    BindFramebufferFn bindFramebuffer;
    GenFn genFramebuffers;
    GenFn genRenderbuffers;
    BindRenderbufferFn bindRenderbuffer;
    RenderbufferStorageFn renderbufferStorage;
    FramebufferRenderbufferFn framebufferRenderbuffer;
    GetAttachmentFn getAttachment;
    BindBufferFn bindBuffer;
} gl;

// ---- Settings ----

enum DotStyle
{
    StyleDot,
    StyleCircle,
    StyleCross,
    StyleGame,     // the game's own crosshair, moved to the depth
    StyleHide,     // no crosshair at all in 3D
    StyleOff
};

static struct
{
    float ipd = 0.062f;            // UZDoom's defaults
    float screenDist = 0.80f;
    float unitsPerMeter = 41.0f;
    float fov = 90.0f;
    bool swapEyes = false;
    int vrMode = 0;
    DotStyle style = StyleDot;
    float size = 1.f;     // -laserdot-size=N  (percent, 50-200)
    float hover = 1.f;    // -laserdot-hover=N (percent, 0-300)
    float color[3] = { 1.f, 0.15f, 0.1f };    // -laserdot-color=RRGGBB
} settings;

// ---- Log (first lines only, in %TEMP%, for testing) ----

static int logLines = 0;

static void Log(const char* format, ...)
{
    if (logLines >= 200)
    {
        return;
    }
    ++logLines;
    wchar_t path[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, path))
    {
        return;
    }
    wcscat_s(path, L"uzdoom_laserdot.log");
    FILE* file = nullptr;
    if (_wfopen_s(&file, path, logLines == 1 ? L"w" : L"a") != 0 || !file)
    {
        return;
    }
    va_list args;
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fputc('\n', file);
    fclose(file);
}

// ---- Command line ----

static float ArgFloat(int count, wchar_t** args, const wchar_t* name, float fallback)
{
    for (int i = 1; i + 1 < count; ++i)
    {
        if (_wcsicmp(args[i], name) == 0)
        {
            return static_cast<float>(_wtof(args[i + 1]));
        }
    }
    return fallback;
}

static bool ArgSwitch(int count, wchar_t** args, const wchar_t* name)
{
    for (int i = 1; i < count; ++i)
    {
        if (_wcsicmp(args[i], name) == 0)
        {
            return true;
        }
    }
    return false;
}

// "-name=N" as a number, else the fallback
static float ArgValue(int count, wchar_t** args, const wchar_t* name, float fallback)
{
    const size_t length = wcslen(name);
    for (int i = 1; i < count; ++i)
    {
        if (_wcsnicmp(args[i], name, length) == 0 && args[i][length] == L'=')
        {
            return static_cast<float>(_wtof(args[i] + length + 1));
        }
    }
    return fallback;
}

static float Clamp(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}

// "-laserdot-color=RRGGBB"
static void ReadColor(int count, wchar_t** args)
{
    const wchar_t* name = L"-laserdot-color=";
    const size_t length = wcslen(name);
    for (int i = 1; i < count; ++i)
    {
        if (_wcsnicmp(args[i], name, length) != 0 || wcslen(args[i] + length) != 6)
        {
            continue;
        }
        wchar_t* end = nullptr;
        const unsigned long rgb = wcstoul(args[i] + length, &end, 16);
        if (end && *end == 0)
        {
            settings.color[0] = ((rgb >> 16) & 255) / 255.f;
            settings.color[1] = ((rgb >> 8) & 255) / 255.f;
            settings.color[2] = (rgb & 255) / 255.f;
        }
    }
}

static HMODULE selfModule = nullptr;

// A game started by its own launcher (like Dismantled) loses the 3D switches on the way. GPU Selector
// then also writes them to gpuselector_3d.txt next to this DLL: read when the command line has none.
static std::wstring SettingsText()
{
    std::wstring line = GetCommandLineW();
    if (line.find(L"+vr_mode") != std::wstring::npos || !selfModule)
    {
        return line;
    }
    wchar_t path[MAX_PATH] = L"";
    if (!GetModuleFileNameW(selfModule, path, MAX_PATH))
    {
        return line;
    }
    std::wstring file = path;
    file = file.substr(0, file.find_last_of(L"\\/") + 1) + L"gpuselector_3d.txt";
    FILE* in = nullptr;
    if (_wfopen_s(&in, file.c_str(), L"rb") != 0 || !in)
    {
        return line;
    }
    char text[2048] = "";
    const size_t got = fread(text, 1, sizeof(text) - 1, in);
    fclose(in);
    text[got] = 0;
    wchar_t wide[2048] = L"";
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, 2048);
    Log("3D switches from gpuselector_3d.txt (the game's launcher dropped them)");
    return L"game.exe " + std::wstring(wide);
}

static void ReadSettings()
{
    int count = 0;
    const std::wstring line = SettingsText();
    wchar_t** args = CommandLineToArgvW(line.c_str(), &count);
    if (!args)
    {
        return;
    }
    settings.ipd = ArgFloat(count, args, L"+vr_ipd", settings.ipd);
    settings.screenDist = ArgFloat(count, args, L"+vr_screendist", settings.screenDist);
    settings.unitsPerMeter = ArgFloat(count, args, L"+vr_hunits_per_meter", settings.unitsPerMeter);
    settings.fov = ArgFloat(count, args, L"+fov", settings.fov);
    settings.swapEyes = ArgFloat(count, args, L"+vr_swap_eyes", 0.f) != 0.f;
    settings.vrMode = static_cast<int>(ArgFloat(count, args, L"+vr_mode", 0.f));
    if (ArgSwitch(count, args, L"-laserdot-circle"))
    {
        settings.style = StyleCircle;
    }
    if (ArgSwitch(count, args, L"-laserdot-cross"))
    {
        settings.style = StyleCross;
    }
    if (ArgSwitch(count, args, L"-laserdot-hide"))
    {
        settings.style = StyleHide;
    }
    if (ArgSwitch(count, args, L"-laserdot-game"))
    {
        settings.style = StyleGame;
    }
    if (ArgSwitch(count, args, L"-laserdot-off"))
    {
        settings.style = StyleOff;
    }
    ReadColor(count, args);
    settings.size = Clamp(ArgValue(count, args, L"-laserdot-size", 100.f), 50.f, 200.f) / 100.f;
    settings.hover = Clamp(ArgValue(count, args, L"-laserdot-hover", 100.f), 0.f, 300.f) / 100.f;
    LocalFree(args);
    if (settings.screenDist < 0.01f)
    {
        settings.screenDist = 0.01f;
    }
    Log("settings: ipd %.4f screendist %.4f units %.1f fov %.1f swap %d vr_mode %d style %d size %.2f hover %.2f",
        settings.ipd, settings.screenDist, settings.unitsPerMeter, settings.fov, settings.swapEyes ? 1 : 0,
        settings.vrMode, static_cast<int>(settings.style), settings.size, settings.hover);
}

// ---- Loading the helper OpenGL functions (needs a current context) ----

template <typename T>
static void Load(T& target, HMODULE module, const char* name)
{
    PROC function = realWglGetProcAddress ? realWglGetProcAddress(name) : nullptr;
    if (!function && module)
    {
        function = GetProcAddress(module, name);
    }
    target = reinterpret_cast<T>(function);
}

static bool LoadHelpers()
{
    if (gl.loaded)
    {
        return gl.getIntegerv != nullptr;
    }
    gl.loaded = true;
    HMODULE module = GetModuleHandleW(L"opengl32.dll");
    Load(gl.getCurrentDC, module, "wglGetCurrentDC");
    Load(gl.getIntegerv, module, "glGetIntegerv");
    Load(gl.getFloatv, module, "glGetFloatv");
    Load(gl.getBooleanv, module, "glGetBooleanv");
    Load(gl.isEnabled, module, "glIsEnabled");
    Load(gl.enable, module, "glEnable");
    Load(gl.disable, module, "glDisable");
    Load(gl.scissor, module, "glScissor");
    Load(gl.clearColor, module, "glClearColor");
    Load(gl.readPixels, module, "glReadPixels");
    Load(gl.getError, module, "glGetError");
    Load(gl.bindFramebuffer, module, "glBindFramebuffer");
    Load(gl.genFramebuffers, module, "glGenFramebuffers");
    Load(gl.genRenderbuffers, module, "glGenRenderbuffers");
    Load(gl.bindRenderbuffer, module, "glBindRenderbuffer");
    Load(gl.renderbufferStorage, module, "glRenderbufferStorage");
    Load(gl.framebufferRenderbuffer, module, "glFramebufferRenderbuffer");
    Load(gl.getAttachment, module, "glGetFramebufferAttachmentParameteriv");
    Load(gl.bindBuffer, module, "glBindBuffer");
    const bool ok = gl.getIntegerv && gl.getFloatv && gl.getBooleanv && gl.isEnabled && gl.enable && gl.disable && gl.scissor &&
                    gl.clearColor && gl.readPixels && gl.getError && gl.bindFramebuffer && gl.genFramebuffers &&
                    gl.genRenderbuffers && gl.bindRenderbuffer && gl.renderbufferStorage &&
                    gl.framebufferRenderbuffer && gl.getAttachment && gl.bindBuffer && realClear;
    Log("helpers loaded: %s", ok ? "yes" : "NO");
    if (!ok)
    {
        gl.getIntegerv = nullptr;
    }
    return ok;
}

static GLint Integer(GLenum name)
{
    GLint value = 0;
    gl.getIntegerv(name, &value);
    return value;
}

static bool HasDepth(GLenum target)
{
    GLint type = GL_NONE;
    gl.getAttachment(target, GL_DEPTH_ATTACHMENT, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
    return type != GL_NONE;
}

static void DrainErrors()
{
    for (int i = 0; i < 8 && gl.getError() != 0; ++i)
    {
    }
}

// ---- The 3D scene of the current eye ----

struct Rect
{
    GLint x, y, w, h;
};

struct SceneTarget
{
    GLuint framebuffer;
    Rect view;
    bool viewKnown;
    bool depthKnown;
    float depth;
};

static SceneTarget scenes[8];
static int sceneCount = 0;
static Rect lastFullSize = { 0, 0, 0, 0 };
// UZDoom's current eye: 0 at the start of a frame, and every NextEye (an eye copied out, then
// the next one copied back in) moves on. The 2D layer is drawn into each eye too, in this order.
static int currentEye = 0;

// The last eye copy was "the next eye begins" (BlitFromEyeTexture). A new 3D scene that does not
// follow one is the first eye of a frame: UZDoom's FirstEye set its eye back to 0 without any copy
// (it does that when a frame had no 3D scene, like in some menus).
static bool lastCopyStartedEye = false;

// A scene was drawn (depth cleared) since the last copy to an eye: for older GZDoom (OlderEyeCopy)
static bool sceneSinceCopy = false;

// The last sideways shift of each eye (for the game's own crosshair, drawn in the 2D layer)
static float eyeOffset[2] = { 0.f, 0.f };

static void ForgetScenes()
{
    sceneCount = 0;
}

static SceneTarget* FindScene(GLuint framebuffer)
{
    for (int i = sceneCount - 1; i >= 0; --i)
    {
        if (scenes[i].framebuffer == framebuffer)
        {
            return &scenes[i];
        }
    }
    return nullptr;
}

static void RememberSceneClear(GLuint framebuffer)
{
    SceneTarget* scene = FindScene(framebuffer);
    if (!scene)
    {
        if (sceneCount == 8)
        {
            memmove(scenes, scenes + 1, sizeof(SceneTarget) * 7);
            --sceneCount;
        }
        scene = &scenes[sceneCount++];
    }
    scene->framebuffer = framebuffer;
    scene->viewKnown = false;
    scene->depthKnown = false;
}

static Rect ViewOf(const SceneTarget& scene)
{
    return scene.viewKnown ? scene.view : lastFullSize;
}

// Where the game's crosshair was last drawn, as a height in the eye's picture (pixels from the
// bottom). Mods like Precise Crosshair move it up or down to where shots really land, so the depth
// is read there and our dot goes there. aimSeen counts down: 0 = not seen lately (use the middle).
static float aimY = 0.f;
static int aimSeen = 0;

static void RememberAimHeight(float cy, float height)
{
    GLint viewport[4];
    gl.getIntegerv(GL_VIEWPORT, viewport);
    if (height > 0.f && viewport[3] > 0)
    {
        aimY = viewport[1] + viewport[3] * (1.f - cy / height);
        aimSeen = 120;    // eye copies: about a second
    }
}

// The aim height inside the 3D view: where the crosshair was drawn, else the middle
static float AimY(const Rect& view)
{
    if (aimSeen > 0 && aimY > view.y && aimY < view.y + view.h)
    {
        return aimY;
    }
    return view.y + view.h * 0.5f;
}

// ---- Reading the depth under the aim point ----

static GLuint probeFramebuffer = 0;

static bool MakeProbe()
{
    if (probeFramebuffer)
    {
        return true;
    }
    GLuint renderbuffer = 0;
    gl.genRenderbuffers(1, &renderbuffer);
    gl.bindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    gl.renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1, 1);
    gl.bindRenderbuffer(GL_RENDERBUFFER, 0);
    gl.genFramebuffers(1, &probeFramebuffer);
    const GLint oldDraw = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, probeFramebuffer);
    gl.framebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(oldDraw));
    return probeFramebuffer != 0;
}

// Copies one depth sample (works for multisampled scenes too) and reads it back
static bool ReadCenterDepth(SceneTarget& scene)
{
    const Rect view = ViewOf(scene);
    if (view.w <= 0 || view.h <= 0 || !MakeProbe())
    {
        return false;
    }
    const GLint cx = view.x + view.w / 2;
    const GLint cy = static_cast<GLint>(AimY(view));
    const GLint oldRead = Integer(GL_READ_FRAMEBUFFER_BINDING);
    const GLint oldDraw = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
    const GLint oldPack = Integer(GL_PIXEL_PACK_BUFFER_BINDING);
    const bool scissorOn = gl.isEnabled(GL_SCISSOR_TEST) != 0;

    DrainErrors();
    gl.disable(GL_SCISSOR_TEST);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, scene.framebuffer);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, probeFramebuffer);
    realBlit(cx, cy, cx + 1, cy + 1, 0, 0, 1, 1, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    float depth = 1.f;
    bool ok = gl.getError() == 0;
    if (ok)
    {
        gl.bindFramebuffer(GL_READ_FRAMEBUFFER, probeFramebuffer);
        gl.readPixels(0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    }
    else
    {
        // Not the same depth format: read the scene straight (only works without multisampling)
        DrainErrors();
        gl.readPixels(cx, cy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
    }
    ok = gl.getError() == 0;

    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(oldRead));
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(oldDraw));
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(oldPack));
    if (scissorOn)
    {
        gl.enable(GL_SCISSOR_TEST);
    }
    if (ok)
    {
        scene.depth = depth;
        scene.depthKnown = true;
    }
    return ok;
}

// ---- Where the dot goes for one eye ----

// Map units from the depth buffer (UZDoom: near 5, far 65536, normal OpenGL depth)
static float DistanceFromDepth(float depth)
{
    const float n = 5.f;
    const float f = 65536.f;
    if (depth >= 0.999999f)
    {
        return 1.0e9f;    // sky: as far as it gets
    }
    const float ndc = depth * 2.f - 1.f;
    return 2.f * n * f / ((f + n) - ndc * (f - n));
}

static float WindowAspect(const Rect& view)
{
    if (gl.getCurrentDC)
    {
        RECT client;
        HWND window = WindowFromDC(gl.getCurrentDC());
        if (window && GetClientRect(window, &client) && client.bottom > 0)
        {
            return static_cast<float>(client.right) / static_cast<float>(client.bottom);
        }
    }
    return view.h > 0 ? static_cast<float>(view.w) / static_cast<float>(view.h) : 16.f / 9.f;
}

// Sideways pixel shift of a point straight ahead at this distance, for this eye.
// Same math as UZDoom's VREyeInfo::GetProjection and GetViewShift (hw_vrmodes.cpp).
// -1 for the eye that sits left of center, +1 for the right one
static float EyeSide(int eye)
{
    const float side = eye == 0 ? -1.f : 1.f;
    return settings.swapEyes ? -side : side;
}

static float EyeShiftPixels(int eye, float distance, const Rect& view)
{
    const float shift = EyeSide(eye) * 0.5f * settings.ipd;
    const float screenUnits = settings.unitsPerMeter * settings.screenDist;
    const float ratio = WindowAspect(view);
    const float fovRatio = ratio >= 1.3f ? 1.333333f : ratio;
    const float tanHalf = std::tan(settings.fov * 3.14159265f / 360.f) / fovRatio;
    const float eyeScale = settings.vrMode == 3 ? 0.5f : 1.f;    // Side by Side Full: half-width eyes
    const float ndc = (shift / settings.screenDist) * (1.f - screenUnits / distance) / (tanHalf * ratio * eyeScale);
    return ndc * static_cast<float>(view.w) * 0.5f;
}

// ---- Drawing the dot (scissored clears: no shaders, no state left behind) ----

static void FillSpan(GLint x0, GLint x1, GLint y)
{
    if (x1 < x0)
    {
        return;
    }
    gl.scissor(x0, y, x1 - x0 + 1, 1);
    realClear(GL_COLOR_BUFFER_BIT);
}

struct Radii
{
    float x, y;
};

// How far a row of an ellipse reaches: pixel centers inside it. -1 = the row is outside.
static int HalfWidth(const Radii& r, int dy)
{
    const float ry = r.y + 0.5f;
    const float t = 1.f - static_cast<float>(dy * dy) / (ry * ry);
    if (t < 0.f || r.x <= 0.f)
    {
        return -1;
    }
    return static_cast<int>(std::floor((r.x + 0.5f) * std::sqrt(t)));
}

// A filled ellipse (inner 0) or a ring between two ellipses
static void FillRing(GLint cx, GLint cy, const Radii& outer, const Radii& inner)
{
    const int rows = static_cast<int>(std::ceil(outer.y));
    for (int dy = -rows; dy <= rows; ++dy)
    {
        const int halfOuter = HalfWidth(outer, dy);
        if (halfOuter < 0)
        {
            continue;
        }
        const int halfInner = HalfWidth(inner, dy);
        if (halfInner >= 0)
        {
            FillSpan(cx - halfOuter, cx - halfInner - 1, cy + dy);
            FillSpan(cx + halfInner + 1, cx + halfOuter, cy + dy);
        }
        else
        {
            FillSpan(cx - halfOuter, cx + halfOuter, cy + dy);
        }
    }
}

// Squeezed 3D modes are stretched back by the display, so the dot is drawn squeezed the same way
// to come out round: Side by Side squished (4) and column interleaved (13) are half width,
// Top and Bottom (11) and row interleaved (12) are half height.
static Radii Squeeze()
{
    const float x = settings.vrMode == 4 || settings.vrMode == 13 ? 0.5f : 1.f;
    const float y = settings.vrMode == 11 || settings.vrMode == 12 ? 0.5f : 1.f;
    return { x, y };
}

static Radii Scaled(float radius, const Radii& squeeze, float extra)
{
    return { radius * squeeze.x + extra, radius * squeeze.y + extra };
}

// A plus sign: arms reach `arm` from the center, `half` is half their thickness
static void FillCross(GLint cx, GLint cy, const Radii& arm, const Radii& half)
{
    const GLint ax = static_cast<GLint>(std::floor(arm.x + 0.5f));
    const GLint ay = static_cast<GLint>(std::floor(arm.y + 0.5f));
    const GLint hx = static_cast<GLint>(std::floor(half.x));
    const GLint hy = static_cast<GLint>(std::floor(half.y));
    gl.scissor(cx - ax, cy - hy, 2 * ax + 1, 2 * hy + 1);
    realClear(GL_COLOR_BUFFER_BIT);
    gl.scissor(cx - hx, cy - ay, 2 * hx + 1, 2 * ay + 1);
    realClear(GL_COLOR_BUFFER_BIT);
}

// The user's color. Glasses modes (green/magenta 1, red/cyan 2, amber/blue 9) get it as a grey
// of the same brightness: each eye only sees part of the colors, and a red dot would turn black
// in the other eye.
static void SetDotColor()
{
    const float* c = settings.color;
    const bool glasses = settings.vrMode == 1 || settings.vrMode == 2 || settings.vrMode == 9;
    if (glasses)
    {
        const float grey = c[0] > c[1] ? (c[0] > c[2] ? c[0] : c[2]) : (c[1] > c[2] ? c[1] : c[2]);
        const float bright = grey < 0.5f ? 0.5f : grey;
        gl.clearColor(bright, bright, bright, 1.f);
    }
    else
    {
        gl.clearColor(c[0], c[1], c[2], 1.f);
    }
}

static void DrawDot(GLuint framebuffer, const Rect& view, float x, float y)
{
    // Size from the full screen height (about 2.7 pixels at 1440p)
    const Radii squeeze = Squeeze();
    const float fullHeight = view.h > 0 ? view.h / squeeze.y : 1080.f;
    const float base = fullHeight / 540.f * settings.size;
    const float radius = base > 1.f ? base : 1.f;
    const GLint cx = static_cast<GLint>(std::floor(x + 0.5f));
    const GLint cy = static_cast<GLint>(std::floor(y + 0.5f));

    const GLint oldDraw = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
    const bool scissorOn = gl.isEnabled(GL_SCISSOR_TEST) != 0;
    GLint oldBox[4];
    gl.getIntegerv(GL_SCISSOR_BOX, oldBox);
    GLfloat oldColor[4];
    gl.getFloatv(GL_COLOR_CLEAR_VALUE, oldColor);

    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
    gl.enable(GL_SCISSOR_TEST);
    if (settings.style == StyleCircle)
    {
        const float outer = radius * 2.f;
        const float inner = outer - (radius * 0.5f > 1.f ? radius * 0.5f : 1.f);
        gl.clearColor(0.f, 0.f, 0.f, 1.f);
        FillRing(cx, cy, Scaled(outer, squeeze, 1.f), Scaled(inner, squeeze, -1.f));
        SetDotColor();
        FillRing(cx, cy, Scaled(outer, squeeze, 0.f), Scaled(inner, squeeze, 0.f));
    }
    else if (settings.style == StyleCross)
    {
        const float arm = radius * 3.f;
        const float half = radius * 0.25f;    // half the arm's thickness: 1 pixel at 1440p, more on big screens
        gl.clearColor(0.f, 0.f, 0.f, 1.f);
        FillCross(cx, cy, Scaled(arm, squeeze, 1.f), Scaled(half, squeeze, 1.f));
        SetDotColor();
        FillCross(cx, cy, Scaled(arm, squeeze, 0.f), Scaled(half, squeeze, 0.f));
    }
    else
    {
        const Radii none = { 0.f, 0.f };
        gl.clearColor(0.f, 0.f, 0.f, 1.f);
        FillRing(cx, cy, Scaled(radius, squeeze, 1.f), none);
        SetDotColor();
        FillRing(cx, cy, Scaled(radius, squeeze, 0.f), none);
    }

    gl.clearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
    gl.scissor(oldBox[0], oldBox[1], oldBox[2], oldBox[3]);
    if (!scissorOn)
    {
        gl.disable(GL_SCISSOR_TEST);
    }
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(oldDraw));
}

// In a level, UZDoom grabs the mouse and hides the pointer. In menus, the console, the title
// screen, intermissions, pause and when switched away, it lets the pointer go. So: dot only
// while the pointer is hidden.
static bool PlayingNow()
{
    CURSORINFO cursor = {};
    cursor.cbSize = sizeof(cursor);
    if (!GetCursorInfo(&cursor))
    {
        return true;
    }
    return (cursor.flags & CURSOR_SHOWING) == 0;
}

static int eyesDrawn = 0;

// An eye's finished picture is about to be copied out of `framebuffer`
static void FinishEye(GLuint framebuffer, int eye)
{
    if (!PlayingNow())
    {
        return;
    }
    SceneTarget* scene = nullptr;
    for (int i = sceneCount - 1; i >= 0; --i)
    {
        if (scenes[i].framebuffer != framebuffer)
        {
            scene = &scenes[i];
            break;
        }
    }
    if (!scene || (!scene->depthKnown && !ReadCenterDepth(*scene)))
    {
        static bool noSceneLogged = false;
        if (!noSceneLogged)
        {
            noSceneLogged = true;
            Log("eye %d: no scene depth (title screen or menu)", eye);
        }
        return;
    }
    const Rect view = ViewOf(*scene);
    const float distance = DistanceFromDepth(scene->depth);
    // Hover: the dot comes a few pixels forward in each eye (closer = toward the other eye),
    // so it floats just in front of what it points at at any distance
    // The game's own crosshair is bigger than our dot: it floats 1.5 times as far by default
    const float hoverPixels = (view.h / 1440.f > 0.75f ? view.h / 1440.f : 0.75f) * settings.hover *
                              (settings.style == StyleGame ? 1.5f : 1.f);
    // The float fades with distance: far away a pixel is a big step in depth, so a fixed float would
    // stop the crosshair short of far walls. Full float up to 256 map units, then less and less.
    const float fade = distance > 256.f ? 256.f / distance : 1.f;
    const float offset = EyeShiftPixels(eye, distance > 6.f ? distance : 6.f, view) - EyeSide(eye) * hoverPixels * fade;
    if (eyesDrawn < 6 || eyesDrawn % 900 == 0)
    {
        Log("eye %d: fb %u view %d,%d %dx%d aim y %.1f (seen %d) depth %.6f distance %.1f shift %.2f px",
            eye, scene->framebuffer, view.x, view.y, view.w, view.h, AimY(view), aimSeen, scene->depth, distance, offset);
    }
    ++eyesDrawn;
    eyeOffset[eye & 1] = offset;
    if (settings.style == StyleDot || settings.style == StyleCircle || settings.style == StyleCross)
    {
        DrawDot(framebuffer, view, view.x + view.w * 0.5f + offset, AimY(view));
    }
}

// ---- The game's own crosshair ----
// UZDoom's 2D layer (hw_draw2d.cpp) uploads its vertices and indices with glBufferData (stream)
// and draws each command with glDrawElements (GL_UNSIGNED_INT). A vertex is x, y, z, u, v, color
// (24 bytes). The crosshair is one small square (6 indices) in the middle of the screen. We keep a
// copy of the last 2D buffers and move that one draw sideways for each eye with the viewport.

struct BufferCopy
{
    GLuint buffer;
    size_t size;
    unsigned char* data;
};

static BufferCopy bufferCopies[4];

static BufferCopy* CopyOf(GLuint buffer)
{
    for (BufferCopy& copy : bufferCopies)
    {
        if (copy.buffer == buffer && copy.data)
        {
            return &copy;
        }
    }
    return nullptr;
}

static void KeepCopy(GLuint buffer, size_t size, const void* data)
{
    BufferCopy* copy = CopyOf(buffer);
    if (!copy)
    {
        static int next = 0;
        copy = &bufferCopies[next];
        next = (next + 1) % 4;
    }
    unsigned char* bytes = static_cast<unsigned char*>(realloc(copy->data, size));
    if (!bytes)
    {
        return;
    }
    memcpy(bytes, data, size);
    copy->buffer = buffer;
    copy->size = size;
    copy->data = bytes;
}

// The screen's size in 2D coordinates: the size of the picture the port draws (an eye copy's size),
// which is not the window's size when the port scales its picture (like LZDoom at a lower resolution)
static bool ScreenSize(float& width, float& height)
{
    if (lastFullSize.w > 0 && lastFullSize.h > 0)
    {
        width = static_cast<float>(lastFullSize.w);
        height = static_cast<float>(lastFullSize.h);
        return true;
    }
    RECT client;
    HWND window = gl.getCurrentDC ? WindowFromDC(gl.getCurrentDC()) : nullptr;
    if (!window || !GetClientRect(window, &client) || client.right <= 0 || client.bottom <= 0)
    {
        return false;
    }
    width = static_cast<float>(client.right);
    height = static_cast<float>(client.bottom);
    return true;
}

static bool IsCrosshairDraw(GLsizei count, const void* offset)
{
    BufferCopy* indices = CopyOf(static_cast<GLuint>(Integer(GL_ELEMENT_ARRAY_BUFFER_BINDING)));
    BufferCopy* vertices = CopyOf(static_cast<GLuint>(Integer(GL_ARRAY_BUFFER_BINDING)));
    const size_t first = reinterpret_cast<size_t>(offset);
    if (!indices || !vertices || first % 4 != 0 || first + count * 4 > indices->size)
    {
        return false;
    }
    float left = 1e9f, right = -1e9f, top = 1e9f, bottom = -1e9f;
    for (GLsizei i = 0; i < count; ++i)
    {
        unsigned int index;
        memcpy(&index, indices->data + first + i * 4, 4);
        if ((static_cast<size_t>(index) + 1) * 24 > vertices->size)
        {
            return false;
        }
        float xy[2];
        memcpy(xy, vertices->data + static_cast<size_t>(index) * 24, 8);
        left = xy[0] < left ? xy[0] : left;
        right = xy[0] > right ? xy[0] : right;
        top = xy[1] < top ? xy[1] : top;
        bottom = xy[1] > bottom ? xy[1] : bottom;
    }
    float width = 0.f, height = 0.f;
    if (!ScreenSize(width, height))
    {
        return false;
    }
    const float quadWidth = right - left;
    const float quadHeight = bottom - top;
    const float size = quadWidth > quadHeight ? quadWidth : quadHeight;
    const float cx = (left + right) * 0.5f;
    const float cy = (top + bottom) * 0.5f;
    // Crosshairs are square. Up to 95% of the screen height: mods like Precise Crosshair grow it
    // a lot up close. Full-screen flashes (wider than tall) and centered HUD text do not count.
    const bool square = quadWidth >= quadHeight * 0.8f && quadHeight >= quadWidth * 0.8f;
    // 1. A square crosshair, centered sideways, anywhere in the middle half of the screen: mods like
    //    Precise Crosshair move it up or down and grow it (up to 4x up close). Its image's own offset
    //    can be a pixel or two off, and growing scales that too.
    const float slack = size * 0.12f > 2.f ? size * 0.12f : 2.f;
    const bool moved = square && size >= 2.f && size <= height * 0.95f && std::fabs(cx - width * 0.5f) <= slack &&
                       std::fabs(cy - height * 0.5f) <= height * 0.25f;
    // 2. A small crosshair of any shape on the exact middle of the screen: games like Hedon have
    //    wide, tall or line-shaped ones, some hung from their top edge. HUD text never sits there.
    const float midX = width * 0.5f;
    const float midY = height * 0.5f;
    const bool onMiddle = size >= 1.f && size <= height * 0.15f &&
                          left <= midX + 2.f && right >= midX - 2.f && top <= midY + 2.f && bottom >= midY - 2.f;
    const bool found = moved || onMiddle;
    if (moved)
    {
        RememberAimHeight(cy, height);
    }
    // For testing: small quads near the middle, found or not (crosshair mods move it)
    static int candidatesLogged = 0;
    static int candidateCalls = 0;
    ++candidateCalls;
    if (size >= 2.f && size <= height * 0.95f && std::fabs(cx - width * 0.5f) <= 12.f &&
        (candidatesLogged < 40 || (found && candidateCalls % 600 == 0)))
    {
        ++candidatesLogged;
        Log("quad: center %.1f,%.1f size %.1fx%.1f screen %.0fx%.0f found %d aimY %.1f eye %d shift %.1f", cx, cy, quadWidth, quadHeight, width, height,
            found ? 1 : 0, aimY, currentEye, eyeOffset[currentEye & 1]);
    }
    return found;
}

static bool crosshairLogged = false;

static void WINAPI MyBufferData(GLenum target, ptrdiff_t size, const void* data, GLenum usage)
{
    realBufferData(target, size, data, usage);
    if (settings.style != StyleOff && settings.vrMode != 0 && data && usage == GL_STREAM_DRAW && size > 0 && size <= (1 << 21) &&
        (target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER) && LoadHelpers())
    {
        const GLenum binding = target == GL_ARRAY_BUFFER ? GL_ARRAY_BUFFER_BINDING : GL_ELEMENT_ARRAY_BUFFER_BINDING;
        KeepCopy(static_cast<GLuint>(Integer(binding)), static_cast<size_t>(size), data);
    }
}

static void WINAPI MyDrawElements(GLenum mode, GLsizei count, GLenum type, const void* offset)
{
    if (settings.style != StyleOff && mode == GL_TRIANGLES && count == 6 && type == GL_UNSIGNED_INT &&
        settings.vrMode != 0 && LoadHelpers() && !gl.isEnabled(GL_DEPTH_TEST) && IsCrosshairDraw(count, offset))
    {
        // Our own dot, circle or cross (or none): the game's crosshair is not drawn in 3D
        if (settings.style != StyleGame)
        {
            if (!crosshairLogged)
            {
                crosshairLogged = true;
                Log("game crosshair found: hidden (style %d)", static_cast<int>(settings.style));
            }
            return;
        }
        GLint viewport[4];
        gl.getIntegerv(GL_VIEWPORT, viewport);
        const GLint shift = static_cast<GLint>(std::floor(eyeOffset[currentEye & 1] + 0.5f));
        if (!crosshairLogged)
        {
            crosshairLogged = true;
            Log("game crosshair found: eye %d shift %d px", currentEye, shift);
        }
        realViewport(viewport[0] + shift, viewport[1], viewport[2], viewport[3]);
        realDrawElements(mode, count, type, offset);
        realViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        return;
    }
    realDrawElements(mode, count, type, offset);
}

// ---- The OpenGL functions UZDoom gets from us ----

static void WINAPI MyClear(GLbitfield mask)
{
    if ((mask & GL_DEPTH_BUFFER_BIT) && settings.style != StyleOff && LoadHelpers())
    {
        const GLint framebuffer = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
        static int clearsLogged = 0;
        if (clearsLogged < 6)
        {
            ++clearsLogged;
            Log("depth clear: mask %x framebuffer %d", mask, framebuffer);
        }
        if (framebuffer != 0)
        {
            sceneSinceCopy = true;
            RememberSceneClear(static_cast<GLuint>(framebuffer));
            if (!lastCopyStartedEye)
            {
                currentEye = 0;
            }
        }
    }
    realClear(mask);
}

static void WINAPI MyViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
    realViewport(x, y, w, h);
    if (sceneCount > 0 && gl.getIntegerv)
    {
        SceneTarget& last = scenes[sceneCount - 1];
        if (!last.viewKnown && Integer(GL_DRAW_FRAMEBUFFER_BINDING) == static_cast<GLint>(last.framebuffer))
        {
            last.view = { x, y, w, h };
            last.viewKnown = true;
        }
    }
}

// UZDoom draws its sorted sprite pass (RenderTranslucent, hw_drawinfo.cpp) with depth writes off,
// so sprites and monsters are missing from the depth buffer. In 3D we keep depth writes on for
// that pass only: the scene framebuffer, blending on, depth test on, colors drawn. It is drawn
// back to front, so nothing it covers is drawn after it: the picture stays the same.
static bool IsSpritePass()
{
    if (settings.vrMode == 0 || sceneCount == 0 || !gl.isEnabled(GL_BLEND) || !gl.isEnabled(GL_DEPTH_TEST))
    {
        return false;
    }
    GLboolean colors[4] = { 0, 0, 0, 0 };
    gl.getBooleanv(GL_COLOR_WRITEMASK, colors);
    if (!colors[0] && !colors[1] && !colors[2])
    {
        return false;
    }
    return Integer(GL_DRAW_FRAMEBUFFER_BINDING) == static_cast<GLint>(scenes[sceneCount - 1].framebuffer);
}

static bool spriteDepthLogged = false;

static void WINAPI MyDepthMask(GLboolean on)
{
    if (!on && settings.style != StyleOff && LoadHelpers() && IsSpritePass())
    {
        if (!spriteDepthLogged)
        {
            spriteDepthLogged = true;
            Log("sprite pass: depth writes kept on");
        }
        realDepthMask(1);
        return;
    }
    realDepthMask(on);
}

static void WINAPI MyInvalidateFramebuffer(GLenum target, GLsizei count, const GLenum* attachments)
{
    if (sceneCount > 0 && gl.getIntegerv && attachments)
    {
        bool depth = false;
        for (GLsizei i = 0; i < count; ++i)
        {
            depth = depth || attachments[i] == GL_DEPTH_ATTACHMENT || attachments[i] == GL_DEPTH_STENCIL_ATTACHMENT ||
                    attachments[i] == GL_DEPTH;
        }
        const GLenum binding = target == GL_READ_FRAMEBUFFER ? GL_READ_FRAMEBUFFER_BINDING : GL_DRAW_FRAMEBUFFER_BINDING;
        SceneTarget* scene = depth ? FindScene(static_cast<GLuint>(Integer(binding))) : nullptr;
        if (scene && !scene->depthKnown)
        {
            ReadCenterDepth(*scene);
        }
    }
    realInvalidate(target, count, attachments);
}

// Older GZDoom (2021, like Hedon's engine) copies an eye's finished picture from a "pipeline"
// picture WITHOUT depth (the scene's depth is in its own framebuffer), and back for the next eye:
//   BlitToEyeTexture:   pipeline -> eye picture      BlitFromEyeTexture: eye picture -> pipeline
// The first such copy after a scene was drawn goes to an eye picture: that is how the eye pictures
// are learned. Returns 1 for a copy to an eye, -1 for a copy from an eye, 0 for anything else.
static GLuint eyePictures[2] = { 0, 0 };

static bool IsEyePicture(GLuint framebuffer)
{
    return framebuffer != 0 && (framebuffer == eyePictures[0] || framebuffer == eyePictures[1]);
}

static int OlderEyeCopy(GLuint read, GLuint draw)
{
    if (IsEyePicture(draw) && !IsEyePicture(read))
    {
        return 1;
    }
    if (IsEyePicture(read) && !IsEyePicture(draw))
    {
        return -1;
    }
    if (sceneSinceCopy && settings.vrMode != 0 && FindScene(read) == nullptr && FindScene(draw) == nullptr)
    {
        if (!eyePictures[0])
        {
            eyePictures[0] = draw;
        }
        else if (!eyePictures[1] && draw != eyePictures[0])
        {
            eyePictures[1] = draw;
        }
        Log("older engine: eye picture %u learned (copied from %u)", draw, read);
        return 1;
    }
    return 0;
}

static void WINAPI MyBlitFramebuffer(GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1,
                                     GLint dy1, GLbitfield mask, GLenum filter)
{
    if (mask == GL_COLOR_BUFFER_BIT && settings.style != StyleOff && LoadHelpers())
    {
        const GLint read = Integer(GL_READ_FRAMEBUFFER_BINDING);
        const GLint draw = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
        if (read != 0 && draw != 0)
        {
            const bool readDepth = HasDepth(GL_READ_FRAMEBUFFER);
            const bool drawDepth = HasDepth(GL_DRAW_FRAMEBUFFER);
            static int blitsLogged = 0;
            if (blitsLogged < 8)
            {
                ++blitsLogged;
                Log("color copy: read %d (depth %d) draw %d (depth %d) %dx%d", read, readDepth ? 1 : 0, draw,
                    drawDepth ? 1 : 0, sx1 - sx0, sy1 - sy0);
            }
            if (readDepth && !drawDepth)
            {
                // BlitToEyeTexture: the eye is finished
                lastFullSize = { 0, 0, sx1 - sx0, sy1 - sy0 };
                FinishEye(static_cast<GLuint>(read), currentEye);
                lastCopyStartedEye = false;
                ForgetScenes();
                sceneSinceCopy = false;
                aimSeen = aimSeen > 0 ? aimSeen - 1 : 0;
            }
            else if (!readDepth && drawDepth)
            {
                // BlitFromEyeTexture: the next eye begins
                currentEye = currentEye == 0 ? 1 : 0;
                lastCopyStartedEye = true;
            }
            else if (!readDepth && !drawDepth)
            {
                // Older GZDoom: neither picture has depth (see OlderEyeCopy)
                const int copy = OlderEyeCopy(static_cast<GLuint>(read), static_cast<GLuint>(draw));
                if (copy == 1)
                {
                    // Which eye: the eye picture it goes to (the first one learned is the left eye).
                    // LZDoom 3.x copies each eye out without copying the next one back, so counting
                    // copies back would never reach the right eye.
                    currentEye = static_cast<GLuint>(draw) == eyePictures[1] ? 1 : 0;
                    lastFullSize = { 0, 0, sx1 - sx0, sy1 - sy0 };
                    FinishEye(static_cast<GLuint>(read), currentEye);
                    lastCopyStartedEye = false;
                    ForgetScenes();
                    sceneSinceCopy = false;
                    aimSeen = aimSeen > 0 ? aimSeen - 1 : 0;
                }
                else if (copy == -1)
                {
                    currentEye = currentEye == 0 ? 1 : 0;
                    lastCopyStartedEye = true;
                }
            }
        }
    }
    realBlit(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1, mask, filter);
}

static PROC WINAPI MyWglGetProcAddress(LPCSTR name)
{
    PROC real = realWglGetProcAddress(name);
    if (!real || !name)
    {
        return real;
    }
    if (strcmp(name, "glBlitFramebuffer") == 0)
    {
        realBlit = reinterpret_cast<BlitFn>(real);
        Log("wrapped glBlitFramebuffer");
        return reinterpret_cast<PROC>(MyBlitFramebuffer);
    }
    if (strcmp(name, "glInvalidateFramebuffer") == 0)
    {
        realInvalidate = reinterpret_cast<InvalidateFn>(real);
        return reinterpret_cast<PROC>(MyInvalidateFramebuffer);
    }
    // Most drivers give the OpenGL 1.1 functions only through GetProcAddress, some give them here too
    if (strcmp(name, "glClear") == 0)
    {
        realClear = reinterpret_cast<ClearFn>(real);
        Log("wrapped glClear (wglGetProcAddress)");
        return reinterpret_cast<PROC>(MyClear);
    }
    if (strcmp(name, "glViewport") == 0)
    {
        realViewport = reinterpret_cast<ViewportFn>(real);
        return reinterpret_cast<PROC>(MyViewport);
    }
    if (strcmp(name, "glDepthMask") == 0)
    {
        realDepthMask = reinterpret_cast<DepthMaskFn>(real);
        return reinterpret_cast<PROC>(MyDepthMask);
    }
    if (strcmp(name, "glDrawElements") == 0)
    {
        realDrawElements = reinterpret_cast<DrawElementsFn>(real);
        return reinterpret_cast<PROC>(MyDrawElements);
    }
    if (strcmp(name, "glBufferData") == 0)
    {
        realBufferData = reinterpret_cast<BufferDataFn>(real);
        return reinterpret_cast<PROC>(MyBufferData);
    }
    return real;
}

// ---- GetProcAddress, as UZDoom sees it ----

typedef FARPROC(WINAPI* GetProcAddressFn)(HMODULE, LPCSTR);
static GetProcAddressFn realGetProcAddress;

static FARPROC WINAPI MyGetProcAddress(HMODULE module, LPCSTR name)
{
    FARPROC real = realGetProcAddress(module, name);
    if (!real || !name || reinterpret_cast<ULONG_PTR>(name) <= 0xFFFF || module != GetModuleHandleW(L"opengl32.dll"))
    {
        return real;
    }
    if (strcmp(name, "wglGetProcAddress") == 0)
    {
        realWglGetProcAddress = reinterpret_cast<WglGetProcAddressFn>(real);
        Log("wrapped wglGetProcAddress");
        return reinterpret_cast<FARPROC>(MyWglGetProcAddress);
    }
    if (strcmp(name, "glClear") == 0)
    {
        realClear = reinterpret_cast<ClearFn>(real);
        Log("wrapped glClear (GetProcAddress)");
        return reinterpret_cast<FARPROC>(MyClear);
    }
    if (strcmp(name, "glViewport") == 0)
    {
        realViewport = reinterpret_cast<ViewportFn>(real);
        return reinterpret_cast<FARPROC>(MyViewport);
    }
    if (strcmp(name, "glDepthMask") == 0)
    {
        realDepthMask = reinterpret_cast<DepthMaskFn>(real);
        return reinterpret_cast<FARPROC>(MyDepthMask);
    }
    if (strcmp(name, "glDrawElements") == 0)
    {
        realDrawElements = reinterpret_cast<DrawElementsFn>(real);
        return reinterpret_cast<FARPROC>(MyDrawElements);
    }
    if (strcmp(name, "glBufferData") == 0)
    {
        realBufferData = reinterpret_cast<BufferDataFn>(real);
        return reinterpret_cast<FARPROC>(MyBufferData);
    }
    return real;
}

// Points the program's own GetProcAddress entry (its import table) at ours. Some ports (like
// GZDoom 4.3) also link wglGetProcAddress straight from opengl32.dll: that entry is pointed at ours too.
static bool RedirectGetProcAddress()
{
    BYTE* base = reinterpret_cast<BYTE*>(GetModuleHandleW(nullptr));
    auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
    auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    const IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.VirtualAddress)
    {
        return false;
    }
    bool done = false;
    for (auto imp = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress); imp->Name; ++imp)
    {
        if (!imp->OriginalFirstThunk)
        {
            continue;
        }
        auto names = reinterpret_cast<IMAGE_THUNK_DATA*>(base + imp->OriginalFirstThunk);
        auto slots = reinterpret_cast<IMAGE_THUNK_DATA*>(base + imp->FirstThunk);
        for (; names->u1.AddressOfData; ++names, ++slots)
        {
            if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal))
            {
                continue;
            }
            auto byName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);
            if (strcmp(reinterpret_cast<const char*>(byName->Name), "wglGetProcAddress") == 0)
            {
                if (!realWglGetProcAddress)
                {
                    realWglGetProcAddress = reinterpret_cast<WglGetProcAddressFn>(slots->u1.Function);
                }
                DWORD old = 0;
                if (VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), PAGE_READWRITE, &old))
                {
                    slots->u1.Function = reinterpret_cast<ULONG_PTR>(MyWglGetProcAddress);
                    VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), old, &old);
                    Log("wrapped wglGetProcAddress (linked)");
                }
                continue;
            }
            if (strcmp(reinterpret_cast<const char*>(byName->Name), "GetProcAddress") != 0)
            {
                continue;
            }
            if (!realGetProcAddress)
            {
                realGetProcAddress = reinterpret_cast<GetProcAddressFn>(slots->u1.Function);
            }
            DWORD old = 0;
            if (VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), PAGE_READWRITE, &old))
            {
                slots->u1.Function = reinterpret_cast<ULONG_PTR>(MyGetProcAddress);
                VirtualProtect(&slots->u1.Function, sizeof(slots->u1.Function), old, &old);
                done = true;
            }
        }
    }
    return done;
}

// ---- dinput8.dll: everything goes to Windows' own copy ----

static HMODULE realDinput8 = nullptr;

static FARPROC RealDinput8(const char* name)
{
    if (!realDinput8)
    {
        wchar_t path[MAX_PATH];
        const UINT length = GetSystemDirectoryW(path, MAX_PATH);
        if (!length || length > MAX_PATH - 16)
        {
            return nullptr;
        }
        wcscat_s(path, L"\\dinput8.dll");
        realDinput8 = LoadLibraryW(path);
    }
    return realDinput8 ? GetProcAddress(realDinput8, name) : nullptr;
}

extern "C" HRESULT WINAPI Proxy_DirectInput8Create(HINSTANCE instance, DWORD version, REFIID iid, LPVOID* out,
                                                   LPUNKNOWN outer)
{
    typedef HRESULT(WINAPI * Fn)(HINSTANCE, DWORD, REFIID, LPVOID*, LPUNKNOWN);
    auto real = reinterpret_cast<Fn>(RealDinput8("DirectInput8Create"));
    return real ? real(instance, version, iid, out, outer) : E_FAIL;
}

extern "C" HRESULT WINAPI Proxy_DllCanUnloadNow()
{
    typedef HRESULT(WINAPI * Fn)();
    auto real = reinterpret_cast<Fn>(RealDinput8("DllCanUnloadNow"));
    return real ? real() : S_FALSE;
}

extern "C" HRESULT WINAPI Proxy_DllGetClassObject(REFCLSID clsid, REFIID iid, LPVOID* out)
{
    typedef HRESULT(WINAPI * Fn)(REFCLSID, REFIID, LPVOID*);
    auto real = reinterpret_cast<Fn>(RealDinput8("DllGetClassObject"));
    return real ? real(clsid, iid, out) : E_FAIL;
}

extern "C" HRESULT WINAPI Proxy_DllRegisterServer()
{
    typedef HRESULT(WINAPI * Fn)();
    auto real = reinterpret_cast<Fn>(RealDinput8("DllRegisterServer"));
    return real ? real() : E_FAIL;
}

extern "C" HRESULT WINAPI Proxy_DllUnregisterServer()
{
    typedef HRESULT(WINAPI * Fn)();
    auto real = reinterpret_cast<Fn>(RealDinput8("DllUnregisterServer"));
    return real ? real() : E_FAIL;
}

extern "C" LPCVOID WINAPI Proxy_GetdfDIJoystick()
{
    typedef LPCVOID(WINAPI * Fn)();
    auto real = reinterpret_cast<Fn>(RealDinput8("GetdfDIJoystick"));
    return real ? real() : nullptr;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(instance);
        selfModule = instance;
        ReadSettings();
        if (settings.style != StyleOff)
        {
            Log("GetProcAddress redirected: %s", RedirectGetProcAddress() ? "yes" : "NO");
        }
    }
    return TRUE;
}
