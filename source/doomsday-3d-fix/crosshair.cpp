// GPU Selector: the 3D crosshair (laser dot) for Doomsday's own 3D.
//
// A dot sits on what you aim at, at its depth, in both eyes (like a VR laser sight).
// Doomsday draws each eye's view into the same framebuffer, one eye after the other
// (GameWidget::draw, ViewCompositor::renderGameView):
//
//   1. When Doomsday asks for the current eye and it changed, the last eye's view is finished
//      and still in the framebuffer: read the depth in a small box where that eye's dot is, and
//      keep the closest. The dot then floats in front of everything under it (no wall cuts it).
//   2. At that eye's next first projection, its view was just cleared and nothing is drawn yet:
//      clear a small dot shape there in color and depth, a little in front of what it hit.
//      The world drawn after it covers it only where something is closer.
//
// Settings from Doomsday's command line (the DOOM card adds these):
//   -laserdot-circle  -laserdot-cross  -laserdot-off
//   -laserdot-game with -laserdot-ddtype=1-5: Doomsday's own crosshair shape, drawn at depth by us
//     (-laserdot-ddsize=0-1, -laserdot-ddangle=0-1, -laserdot-ddwidth=0.5-5, -laserdot-ddcolor=RRGGBB:
//     its own view-cross-* settings, which the DOOM card reads from Doomsday's config)
//   -laserdot-color=RRGGBB  -laserdot-size=N (50-200 %)  -laserdot-hover=N (0-300 %)
//
// Written for GPU Selector. No Doomsday code is in it.

#include <windows.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include "crosshair.h"

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef float GLfloat;
typedef double GLdouble;
typedef unsigned char GLboolean;

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_SCISSOR_TEST 0x0C11
#define GL_SCISSOR_BOX 0x0C10
#define GL_VIEWPORT 0x0BA2
#define GL_COLOR_CLEAR_VALUE 0x0C22
#define GL_DEPTH_CLEAR_VALUE 0x0B73
#define GL_DEPTH_WRITEMASK 0x0B72
#define GL_DEPTH_COMPONENT 0x1902
#define GL_FLOAT 0x1406
#define GL_NEAREST 0x2600
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#define GL_RENDERBUFFER 0x8D41
#define GL_DEPTH24_STENCIL8 0x88F0
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A
#define GL_PIXEL_PACK_BUFFER 0x88EB
#define GL_PIXEL_PACK_BUFFER_BINDING 0x88ED

// ---- Settings ----

enum DotStyle
{
    StyleDot,
    StyleCircle,
    StyleCross,
    StyleDoomsday,    // Doomsday's own crosshair shape (x_hair.c, the five in g_game.cpp)
    StyleNone
};

static struct
{
    bool read = false;
    DotStyle style = StyleDot;
    float size = 1.f;
    float hover = 1.f;
    float color[3] = { 1.f, 0.15f, 0.1f };
    int ddType = 0;          // Doomsday's own crosshair: its shape (1-5), size, angle, line width, color
    float ddSize = 0.5f;
    float ddAngle = 0.f;
    float ddWidth = 1.f;
    float ddColor[3] = { 1.f, 1.f, 1.f };
} settings;

static bool HasSwitch(int count, wchar_t** args, const wchar_t* name)
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

// "-name=value": the text after "=", or nullptr
static const wchar_t* SwitchValue(int count, wchar_t** args, const wchar_t* name)
{
    const size_t length = wcslen(name);
    for (int i = 1; i < count; ++i)
    {
        if (_wcsnicmp(args[i], name, length) == 0 && args[i][length] == L'=')
        {
            return args[i] + length + 1;
        }
    }
    return nullptr;
}

static float Clamp(float value, float low, float high)
{
    return value < low ? low : value > high ? high : value;
}

// "RRGGBB" into three 0-1 values (left alone when it isn't one)
static void ReadHexColor(const wchar_t* text, float color[3])
{
    if (!text || wcslen(text) != 6)
    {
        return;
    }
    wchar_t* end = nullptr;
    const unsigned long rgb = wcstoul(text, &end, 16);
    if (end && *end == 0)
    {
        color[0] = ((rgb >> 16) & 255) / 255.f;
        color[1] = ((rgb >> 8) & 255) / 255.f;
        color[2] = (rgb & 255) / 255.f;
    }
}

static void ReadSettings()
{
    settings.read = true;
    int count = 0;
    wchar_t** args = CommandLineToArgvW(GetCommandLineW(), &count);
    if (!args)
    {
        return;
    }
    if (HasSwitch(count, args, L"-laserdot-circle"))
    {
        settings.style = StyleCircle;
    }
    if (HasSwitch(count, args, L"-laserdot-cross"))
    {
        settings.style = StyleCross;
    }
    if (HasSwitch(count, args, L"-laserdot-off") || HasSwitch(count, args, L"-laserdot-game"))
    {
        settings.style = StyleNone;
    }
    if (const wchar_t* type = SwitchValue(count, args, L"-laserdot-ddtype"))
    {
        settings.ddType = _wtoi(type);
        if (HasSwitch(count, args, L"-laserdot-game") && settings.ddType >= 1 && settings.ddType <= 5)
        {
            settings.style = StyleDoomsday;
        }
    }
    if (const wchar_t* size = SwitchValue(count, args, L"-laserdot-ddsize"))
    {
        settings.ddSize = Clamp(static_cast<float>(_wtof(size)), 0.f, 1.f);
    }
    if (const wchar_t* angle = SwitchValue(count, args, L"-laserdot-ddangle"))
    {
        settings.ddAngle = Clamp(static_cast<float>(_wtof(angle)), 0.f, 1.f);
    }
    if (const wchar_t* width = SwitchValue(count, args, L"-laserdot-ddwidth"))
    {
        settings.ddWidth = Clamp(static_cast<float>(_wtof(width)), 0.5f, 5.f);
    }
    ReadHexColor(SwitchValue(count, args, L"-laserdot-ddcolor"), settings.ddColor);
    if (const wchar_t* size = SwitchValue(count, args, L"-laserdot-size"))
    {
        settings.size = Clamp(static_cast<float>(_wtof(size)), 50.f, 200.f) / 100.f;
    }
    if (const wchar_t* hover = SwitchValue(count, args, L"-laserdot-hover"))
    {
        settings.hover = Clamp(static_cast<float>(_wtof(hover)), 0.f, 300.f) / 100.f;
    }
    ReadHexColor(SwitchValue(count, args, L"-laserdot-color"), settings.color);
    LocalFree(args);
}

// ---- Log (first lines only, in %TEMP%, for testing) ----

static int logLines = 0;

static void Log(const char* format, ...)
{
    if (logLines >= 40)
    {
        return;
    }
    ++logLines;
    wchar_t path[MAX_PATH];
    if (!GetTempPathW(MAX_PATH, path))
    {
        return;
    }
    wcscat_s(path, L"doomsday_laserdot.log");
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

// ---- OpenGL (Doomsday's context is current while it draws) ----

static struct
{
    bool loaded;
    void(WINAPI* getIntegerv)(GLenum, GLint*);
    void(WINAPI* getFloatv)(GLenum, GLfloat*);
    void(WINAPI* getBooleanv)(GLenum, GLboolean*);
    GLboolean(WINAPI* isEnabled)(GLenum);
    void(WINAPI* enable)(GLenum);
    void(WINAPI* disable)(GLenum);
    void(WINAPI* scissor)(GLint, GLint, GLsizei, GLsizei);
    void(WINAPI* clearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
    void(WINAPI* clearDepth)(GLdouble);
    void(WINAPI* clear)(GLbitfield);
    void(WINAPI* depthMask)(GLboolean);
    void(WINAPI* readPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, void*);
    GLenum(WINAPI* getError)();
    void(WINAPI* bindFramebuffer)(GLenum, GLuint);
    void(WINAPI* genFramebuffers)(GLsizei, GLuint*);
    void(WINAPI* genRenderbuffers)(GLsizei, GLuint*);
    void(WINAPI* bindRenderbuffer)(GLenum, GLuint);
    void(WINAPI* renderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei);
    void(WINAPI* framebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint);
    void(WINAPI* blitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
    void(WINAPI* bindBuffer)(GLenum, GLuint);
    HDC(WINAPI* getCurrentDC)();
} gl;

template <typename T>
static void Load(T& target, HMODULE module, PROC(WINAPI* wglGet)(LPCSTR), const char* name)
{
    PROC function = wglGet ? wglGet(name) : nullptr;
    if (!function)
    {
        function = GetProcAddress(module, name);
    }
    target = reinterpret_cast<T>(function);
}

static bool LoadGl()
{
    if (gl.loaded)
    {
        return gl.blitFramebuffer != nullptr;
    }
    gl.loaded = true;
    HMODULE module = GetModuleHandleW(L"opengl32.dll");
    if (!module)
    {
        return false;
    }
    auto wglGet = reinterpret_cast<PROC(WINAPI*)(LPCSTR)>(GetProcAddress(module, "wglGetProcAddress"));
    Load(gl.getIntegerv, module, wglGet, "glGetIntegerv");
    Load(gl.getFloatv, module, wglGet, "glGetFloatv");
    Load(gl.getBooleanv, module, wglGet, "glGetBooleanv");
    Load(gl.isEnabled, module, wglGet, "glIsEnabled");
    Load(gl.enable, module, wglGet, "glEnable");
    Load(gl.disable, module, wglGet, "glDisable");
    Load(gl.scissor, module, wglGet, "glScissor");
    Load(gl.clearColor, module, wglGet, "glClearColor");
    Load(gl.clearDepth, module, wglGet, "glClearDepth");
    Load(gl.clear, module, wglGet, "glClear");
    Load(gl.depthMask, module, wglGet, "glDepthMask");
    Load(gl.readPixels, module, wglGet, "glReadPixels");
    Load(gl.getError, module, wglGet, "glGetError");
    Load(gl.bindFramebuffer, module, wglGet, "glBindFramebuffer");
    Load(gl.genFramebuffers, module, wglGet, "glGenFramebuffers");
    Load(gl.genRenderbuffers, module, wglGet, "glGenRenderbuffers");
    Load(gl.bindRenderbuffer, module, wglGet, "glBindRenderbuffer");
    Load(gl.renderbufferStorage, module, wglGet, "glRenderbufferStorage");
    Load(gl.framebufferRenderbuffer, module, wglGet, "glFramebufferRenderbuffer");
    Load(gl.blitFramebuffer, module, wglGet, "glBlitFramebuffer");
    Load(gl.bindBuffer, module, wglGet, "glBindBuffer");
    Load(gl.getCurrentDC, module, nullptr, "wglGetCurrentDC");
    const bool ok = gl.getIntegerv && gl.getFloatv && gl.getBooleanv && gl.isEnabled && gl.enable && gl.disable &&
                    gl.scissor && gl.clearColor && gl.clearDepth && gl.clear && gl.depthMask && gl.readPixels &&
                    gl.getError && gl.bindFramebuffer && gl.genFramebuffers && gl.genRenderbuffers &&
                    gl.bindRenderbuffer && gl.renderbufferStorage && gl.framebufferRenderbuffer && gl.blitFramebuffer &&
                    gl.bindBuffer;
    Log("OpenGL functions: %s", ok ? "yes" : "NO");
    if (!ok)
    {
        gl.blitFramebuffer = nullptr;
    }
    return ok;
}

static GLint Integer(GLenum name)
{
    GLint value = 0;
    gl.getIntegerv(name, &value);
    return value;
}

// ---- Each eye ----

struct Rect
{
    GLint x, y, w, h;
};

struct Eye
{
    GLuint framebuffer;
    Rect view;
    int mode;
    float focal, shift, screenUnits, nearClip, farClip;
    bool viewKnown;
    bool distanceKnown;
    float distance;
    bool placed;          // where the dot was last drawn in this eye, how far it reaches, its depth
    GLint dotX, dotY, reach;
    float dotDepth;
};

static Eye eyes[3];
static int lastEye = 0;
static bool pending[3];

// ---- Reading the depth under the aim point ----

static GLuint probeFramebuffer = 0;
static const GLint kProbeSize = 320;    // the largest box read at once (a big crosshair at 4K fits)

static bool MakeProbe()
{
    if (probeFramebuffer)
    {
        return true;
    }
    GLuint renderbuffer = 0;
    gl.genRenderbuffers(1, &renderbuffer);
    gl.bindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    gl.renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, kProbeSize, kProbeSize);
    gl.bindRenderbuffer(GL_RENDERBUFFER, 0);
    gl.genFramebuffers(1, &probeFramebuffer);
    const GLint oldDraw = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, probeFramebuffer);
    gl.framebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(oldDraw));
    return probeFramebuffer != 0;
}

static void DrainErrors()
{
    for (int i = 0; i < 8 && gl.getError() != 0; ++i)
    {
    }
}

// The closest depth in a box around (cx, cy) in the eye's view (works with multisampling too).
// The dot's own depth (`skip`, or -1 for none) is not counted. False when nothing else is there.
static bool ReadNearestDepth(const Eye& eye, GLint cx, GLint cy, GLint half, float skip, float& nearest)
{
    if (eye.view.w <= 0 || eye.view.h <= 0 || !MakeProbe())
    {
        return false;
    }
    half = half < 0 ? 0 : half > kProbeSize / 2 - 1 ? kProbeSize / 2 - 1 : half;
    GLint x0 = cx - half, y0 = cy - half, x1 = cx + half + 1, y1 = cy + half + 1;
    x0 = x0 < eye.view.x ? eye.view.x : x0;
    y0 = y0 < eye.view.y ? eye.view.y : y0;
    x1 = x1 > eye.view.x + eye.view.w ? eye.view.x + eye.view.w : x1;
    y1 = y1 > eye.view.y + eye.view.h ? eye.view.y + eye.view.h : y1;
    if (x1 <= x0 || y1 <= y0)
    {
        return false;
    }
    const GLsizei w = x1 - x0, h = y1 - y0;
    const GLint oldRead = Integer(GL_READ_FRAMEBUFFER_BINDING);
    const GLint oldDraw = Integer(GL_DRAW_FRAMEBUFFER_BINDING);
    const GLint oldPack = Integer(GL_PIXEL_PACK_BUFFER_BINDING);
    const bool scissorOn = gl.isEnabled(GL_SCISSOR_TEST) != 0;
    static float depths[kProbeSize * kProbeSize];

    DrainErrors();
    gl.disable(GL_SCISSOR_TEST);
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, eye.framebuffer);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, probeFramebuffer);
    gl.blitFramebuffer(x0, y0, x1, y1, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    if (gl.getError() == 0)
    {
        gl.bindFramebuffer(GL_READ_FRAMEBUFFER, probeFramebuffer);
        gl.readPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, depths);
    }
    else
    {
        DrainErrors();    // another depth format: read it straight (without multisampling only)
        gl.readPixels(x0, y0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, depths);
    }
    bool ok = gl.getError() == 0;
    bool found = false;
    nearest = 1.f;
    const float step = 1.5f / 16777215.f;    // one step of a 24-bit depth buffer
    for (GLsizei i = 0; ok && i < w * h; ++i)
    {
        if (std::fabs(depths[i] - skip) > step)
        {
            nearest = depths[i] < nearest ? depths[i] : nearest;
            found = true;
        }
    }
    ok = ok && found;

    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(oldRead));
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(oldDraw));
    gl.bindBuffer(GL_PIXEL_PACK_BUFFER, static_cast<GLuint>(oldPack));
    if (scissorOn)
    {
        gl.enable(GL_SCISSOR_TEST);
    }
    return ok;
}

// Our projection (stereo_projection.cpp), with Doomsday's z flip: depth <-> distance in map units
static float DistanceFromDepth(const Eye& eye, float depth)
{
    if (depth >= 0.99999f)
    {
        return 1.0e9f;    // sky
    }
    const float n = eye.nearClip, f = eye.farClip;
    const float m22 = (f + n) / (n - f);
    const float m23 = 2.f * f * n / (n - f);
    return m23 / ((depth * 2.f - 1.f) + m22);
}

static float DepthFromDistance(const Eye& eye, float distance)
{
    const float n = eye.nearClip, f = eye.farClip;
    const float m22 = (f + n) / (n - f);
    const float m23 = 2.f * f * n / (n - f);
    return Clamp(0.5f * (-m22 + m23 / distance) + 0.5f, 0.f, 1.f);
}

// ---- Drawing the dot (scissored clears, color and depth) ----

struct Radii
{
    float x, y;
};

static void FillSpan(GLint x0, GLint x1, GLint y)
{
    if (x1 < x0)
    {
        return;
    }
    gl.scissor(x0, y, x1 - x0 + 1, 1);
    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

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

static void FillCross(GLint cx, GLint cy, const Radii& arm, const Radii& half)
{
    const GLint ax = static_cast<GLint>(std::floor(arm.x + 0.5f));
    const GLint ay = static_cast<GLint>(std::floor(arm.y + 0.5f));
    const GLint hx = static_cast<GLint>(std::floor(half.x));
    const GLint hy = static_cast<GLint>(std::floor(half.y));
    gl.scissor(cx - ax, cy - hy, 2 * ax + 1, 2 * hy + 1);
    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl.scissor(cx - hx, cy - ay, 2 * hx + 1, 2 * ay + 1);
    gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Doomsday's modes: Side by Side (6) is half width, Top and Bottom (5) half height
static Radii Squeeze(int mode)
{
    return { mode == 6 ? 0.5f : 1.f, mode == 5 ? 0.5f : 1.f };
}

static Radii Scaled(float radius, const Radii& squeeze, float extra)
{
    return { radius * squeeze.x + extra, radius * squeeze.y + extra };
}

// The user's color; glasses modes (green/magenta 1, red/cyan 2) get a grey of the same brightness
static void SetDotColor(int mode)
{
    const float* c = settings.color;
    if (mode == 1 || mode == 2)
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

// ---- Doomsday's own crosshair shapes (g_game.cpp, R = 1, y down) ----

struct Point
{
    float x, y;
};

// Each shape: lines through points; a 0,0,0,0 pair ends a line (`gap`)
static const float kGap = 99.f;
static const Point kCross[] = { { -1, 0 }, { -0.4f, 0 }, { kGap, 0 }, { 0, -1 }, { 0, -0.4f }, { kGap, 0 },
                                { 1, 0 }, { 0.4f, 0 }, { kGap, 0 }, { 0, 1 }, { 0, 0.4f } };
static const Point kTwinAngles[] = { { -1, -10.f / 14 }, { -(1 - 10.f / 14), 0 }, { -1, 10.f / 14 }, { kGap, 0 },
                                     { 1, -10.f / 14 }, { 1 - 10.f / 14, 0 }, { 1, 10.f / 14 } };
static const Point kSquare[] = { { -1, -1 }, { -1, 1 }, { 1, 1 }, { 1, -1 }, { -1, -1 } };
static const Point kSquareCorners[] = { { -1, -0.5f }, { -1, -1 }, { -0.5f, -1 }, { kGap, 0 },
                                        { 0.5f, -1 }, { 1, -1 }, { 1, -0.5f }, { kGap, 0 },
                                        { -1, 0.5f }, { -1, 1 }, { -0.5f, 1 }, { kGap, 0 },
                                        { 0.5f, 1 }, { 1, 1 }, { 1, 0.5f } };
static const Point kAngle[] = { { -1, -1 }, { 0, 0 }, { 1, -1 } };

// One thick line: each pixel row gets one span (a thick line is convex)
static void FillSegment(Point a, Point b, float half)
{
    const float dx = b.x - a.x, dy = b.y - a.y;
    const float length2 = dx * dx + dy * dy;
    const auto inside = [&](float px, float py)
    {
        float t = length2 > 0.f ? ((px - a.x) * dx + (py - a.y) * dy) / length2 : 0.f;
        t = t < 0.f ? 0.f : t > 1.f ? 1.f : t;
        const float ex = a.x + t * dx - px, ey = a.y + t * dy - py;
        return ex * ex + ey * ey <= half * half;
    };
    const int y0 = static_cast<int>(std::floor((a.y < b.y ? a.y : b.y) - half));
    const int y1 = static_cast<int>(std::ceil((a.y > b.y ? a.y : b.y) + half));
    const int x0 = static_cast<int>(std::floor((a.x < b.x ? a.x : b.x) - half));
    const int x1 = static_cast<int>(std::ceil((a.x > b.x ? a.x : b.x) + half));
    for (int y = y0; y <= y1; ++y)
    {
        int first = x1 + 1, last = x0 - 1;
        for (int x = x0; x <= x1; ++x)
        {
            if (inside(x + 0.5f, y + 0.5f))
            {
                first = x < first ? x : first;
                last = x;
            }
        }
        FillSpan(first, last, y);
    }
}

// Doomsday's shape `type`, centered at (cx, cy): half its size in pixels, turned by `angle` (0-1)
static void FillDoomsdayShape(GLint cx, GLint cy, int type, float size, float angle, float half, const Radii& squeeze)
{
    const Point* points = type == 1 ? kCross : type == 2 ? kTwinAngles : type == 3 ? kSquare : type == 4 ? kSquareCorners : kAngle;
    const int count = type == 1 ? 11 : type == 2 ? 7 : type == 3 ? 5 : type == 4 ? 15 : 3;
    const float turn = angle * 6.2831853f;
    const float c = std::cos(turn), s = std::sin(turn);
    const auto place = [&](Point p)
    {
        const float rx = p.x * c - p.y * s, ry = p.x * s + p.y * c;
        return Point{ cx + 0.5f + rx * size * squeeze.x, cy + 0.5f - ry * size * squeeze.y };    // y up in OpenGL
    };
    for (int i = 0; i + 1 < count; ++i)
    {
        if (points[i].x != kGap && points[i + 1].x != kGap)
        {
            FillSegment(place(points[i]), place(points[i + 1]), half);
        }
    }
}

// Doomsday's size for its crosshair (x_hair.c): .125 + size x .125 x screen height x 80/200
static float DoomsdayHalfSize(float fullHeight)
{
    return 0.125f + settings.ddSize * 0.125f * fullHeight * (80.f / 200.f);
}

static void DrawShapes(GLint cx, GLint cy, float radius, float fullHeight, const Radii& squeeze, int mode)
{
    if (settings.style == StyleDoomsday)
    {
        const float* c = settings.ddColor;
        const float saved[3] = { settings.color[0], settings.color[1], settings.color[2] };
        settings.color[0] = c[0];
        settings.color[1] = c[1];
        settings.color[2] = c[2];
        SetDotColor(mode);
        settings.color[0] = saved[0];
        settings.color[1] = saved[1];
        settings.color[2] = saved[2];
        FillDoomsdayShape(cx, cy, settings.ddType, DoomsdayHalfSize(fullHeight), settings.ddAngle,
                          settings.ddWidth * 0.5f > 0.5f ? settings.ddWidth * 0.5f : 0.5f, squeeze);
        return;
    }
    if (settings.style == StyleCircle)
    {
        const float outer = radius * 2.f;
        const float inner = outer - (radius * 0.5f > 1.f ? radius * 0.5f : 1.f);
        gl.clearColor(0.f, 0.f, 0.f, 1.f);
        FillRing(cx, cy, Scaled(outer, squeeze, 1.f), Scaled(inner, squeeze, -1.f));
        SetDotColor(mode);
        FillRing(cx, cy, Scaled(outer, squeeze, 0.f), Scaled(inner, squeeze, 0.f));
    }
    else if (settings.style == StyleCross)
    {
        const float arm = radius * 3.f;
        const float half = radius * 0.25f;    // half the arm's thickness: 1 pixel at 1440p, more on big screens
        gl.clearColor(0.f, 0.f, 0.f, 1.f);
        FillCross(cx, cy, Scaled(arm, squeeze, 1.f), Scaled(half, squeeze, 1.f));
        SetDotColor(mode);
        FillCross(cx, cy, Scaled(arm, squeeze, 0.f), Scaled(half, squeeze, 0.f));
    }
    else
    {
        const Radii none = { 0.f, 0.f };
        gl.clearColor(0.f, 0.f, 0.f, 1.f);
        FillRing(cx, cy, Scaled(radius, squeeze, 1.f), none);
        SetDotColor(mode);
        FillRing(cx, cy, Scaled(radius, squeeze, 0.f), none);
    }
}

// In the game, Doomsday hides the mouse pointer; in menus and other windows it shows
static bool PlayingNow()
{
    CURSORINFO cursor = {};
    cursor.cbSize = sizeof(cursor);
    return !GetCursorInfo(&cursor) || (cursor.flags & CURSOR_SHOWING) == 0;
}

static int dotsLogged = 0;

static void DrawDot(Eye& eye)
{
    const Rect& view = eye.view;
    const Radii squeeze = Squeeze(eye.mode);
    const float fullHeight = view.h / squeeze.y;

    // Where a point straight ahead at this distance lands in this eye (our projection's math)
    const float z = eye.distance > eye.nearClip * 1.5f ? eye.distance : eye.nearClip * 1.5f;
    const float ndc = eye.screenUnits > 0.f ? eye.focal * eye.shift * (1.f / eye.screenUnits - 1.f / z)
                                            : -eye.focal * eye.shift / z;
    const float side = eye.shift < 0.f ? -1.f : 1.f;
    // Doomsday's own crosshair is bigger than our dot: it floats 1.5 times as far by default
    const float hover = (fullHeight / 1440.f > 0.75f ? fullHeight / 1440.f : 0.75f) * settings.hover *
                        (settings.style == StyleDoomsday ? 1.5f : 1.f);
    // The float fades with distance (full up to 256 map units): far away a pixel is a big step in depth
    const float fade = z > 256.f ? 256.f / z : 1.f;
    const float x = view.x + view.w * 0.5f + ndc * view.w * 0.5f - side * hover * fade;
    const float y = view.y + view.h * 0.5f;
    const float base = fullHeight / 540.f * settings.size;
    const float radius = base > 1.f ? base : 1.f;
    const float depth = DepthFromDistance(eye, z * 0.97f > eye.nearClip ? z * 0.97f : eye.nearClip);
    if (dotsLogged < 6)
    {
        ++dotsLogged;
        Log("dot: fb %u view %d,%d %dx%d distance %.1f shift %.2f px depth %.6f", eye.framebuffer, view.x, view.y,
            view.w, view.h, eye.distance, ndc * view.w * 0.5f - side * hover, depth);
    }

    const bool scissorOn = gl.isEnabled(GL_SCISSOR_TEST) != 0;
    GLint oldBox[4];
    gl.getIntegerv(GL_SCISSOR_BOX, oldBox);
    GLfloat oldColor[4];
    gl.getFloatv(GL_COLOR_CLEAR_VALUE, oldColor);
    GLfloat oldDepth = 1.f;
    gl.getFloatv(GL_DEPTH_CLEAR_VALUE, &oldDepth);
    GLboolean depthWrites = 1;
    gl.getBooleanv(GL_DEPTH_WRITEMASK, &depthWrites);

    gl.enable(GL_SCISSOR_TEST);
    gl.depthMask(1);
    gl.clearDepth(depth);
    // Remember where it went and how far it reaches (the cross reaches farthest)
    eye.placed = true;
    eye.dotX = static_cast<GLint>(std::floor(x + 0.5f));
    eye.dotY = static_cast<GLint>(std::floor(y + 0.5f));
    const float reach = (settings.style == StyleDoomsday ? DoomsdayHalfSize(fullHeight) * 1.42f + settings.ddWidth :
                         settings.style == StyleCross ? radius * 3.f : settings.style == StyleCircle ? radius * 2.f : radius) + 2.f;
    eye.reach = static_cast<GLint>(std::ceil(reach));
    eye.dotDepth = depth;
    DrawShapes(eye.dotX, eye.dotY, radius, fullHeight, squeeze, eye.mode);

    gl.clearDepth(oldDepth);
    gl.depthMask(depthWrites);
    gl.clearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
    gl.scissor(oldBox[0], oldBox[1], oldBox[2], oldBox[3]);
    if (!scissorOn)
    {
        gl.disable(GL_SCISSOR_TEST);
    }
}

// ---- Called by stereo_projection.cpp ----

void CrosshairEye(int eye)
{
    if (eye == lastEye)
    {
        return;
    }
    if (!settings.read)
    {
        ReadSettings();
        Log("settings: style %d size %.2f hover %.2f", static_cast<int>(settings.style), settings.size, settings.hover);
    }
    // The last eye is finished: its view is still in the framebuffer
    if ((lastEye == 1 || lastEye == 2) && settings.style != StyleNone && eyes[lastEye].viewKnown && LoadGl())
    {
        Eye& done = eyes[lastEye];
        float depth = 1.f;
        const GLint cx = done.placed ? done.dotX : done.view.x + done.view.w / 2;
        const GLint cy = done.placed ? done.dotY : done.view.y + done.view.h / 2;
        // A little past the dot's edge, so the world around it is seen too
        if (ReadNearestDepth(done, cx, cy, done.placed ? done.reach + 3 : 1, done.placed ? done.dotDepth : -1.f, depth))
        {
            done.distanceKnown = true;
            done.distance = DistanceFromDepth(done, depth);
        }
    }
    lastEye = eye;
    if (eye == 1 || eye == 2)
    {
        pending[eye] = true;
    }
}

void CrosshairProjection(int eye, int mode, float focal, float shift, float screenUnits, float nearClip, float farClip)
{
    if ((eye != 1 && eye != 2) || !pending[eye])
    {
        return;
    }
    pending[eye] = false;
    if (!settings.read)
    {
        ReadSettings();
    }
    if (settings.style == StyleNone || !LoadGl())
    {
        return;
    }
    // Draw with last frame's depth for this eye, then remember this frame's view
    Eye& now = eyes[eye];
    GLint viewport[4];
    gl.getIntegerv(GL_VIEWPORT, viewport);
    const GLuint framebuffer = static_cast<GLuint>(Integer(GL_DRAW_FRAMEBUFFER_BINDING));
    if (now.distanceKnown && now.viewKnown && framebuffer == now.framebuffer && PlayingNow())
    {
        DrawDot(now);
    }
    now.framebuffer = framebuffer;
    now.view = { viewport[0], viewport[1], viewport[2], viewport[3] };
    now.mode = mode;
    now.focal = focal;
    now.shift = shift;
    now.screenUnits = screenUnits;
    now.nearClip = nearClip;
    now.farClip = farClip;
    now.viewKnown = framebuffer != 0;
}
