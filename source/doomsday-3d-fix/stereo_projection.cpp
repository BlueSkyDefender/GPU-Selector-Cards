// GPU Selector: 3D convergence for Doomsday 2.3.2 (build 3869).
//
// Doomsday's own projection keeps both eyes looking straight ahead (its convergence code is
// switched off), so the whole 3D picture comes out of the screen. This small DLL takes the
// place of deng_appfw.dll: every function goes on to Doomsday's original DLL (renamed
// deng_appfw_doomsday.dll, not changed), except two:
//   1. the projection (below)
//   2. the current eye, so the 3D crosshair knows when each eye is finished (crosshair.cpp)
//
// Our projection: a normal perspective view for each eye, moved sideways by the eye shift,
// plus one more term so a point straight ahead at the screen distance lands in the same
// place in both eyes (the zero parallax plane, the ZPD).
//   screen distance (map units) = rend-vr-hud-distance (meters) x map units per meter
//
// Written for GPU Selector. Only Doomsday's public function names are used: their source is not.

#include <windows.h>
#include <cmath>
#include "forwarders.h"
#include "crosshair.h"

namespace de
{
    // The same memory layout as Doomsday's types: two floats, and 16 floats in column order
    template <typename T>
    class Vector2
    {
    public:
        T x;
        T y;
    };

    template <typename T>
    class Matrix4
    {
    public:
        Matrix4()
        {
            for (T& value : values)
            {
                value = T(0);
            }
        }

        T& at(int row, int column)
        {
            return values[column * 4 + row];
        }

        T values[16];
    };

    class VRConfig
    {
    public:
        enum StereoMode
        {
            Mono
        };

        enum Eye
        {
            NeitherEye,
            LeftEye,
            RightEye
        };

        // In Doomsday's original DLL
        __declspec(dllimport) StereoMode mode() const;
        __declspec(dllimport) float eyeShift() const;
        __declspec(dllimport) float screenDistance() const;
        __declspec(dllimport) float mapUnitsPerMeter() const;
        __declspec(dllimport) bool frustumShift() const;
        __declspec(dllimport) float viewAspect(Vector2<float> const& viewPortSize) const;

        // Ours
        Matrix4<float> projectionMatrix(float fovDegrees, Vector2<float> const& viewPortSize,
                                        float nearClip, float farClip) const;
        Eye currentEye() const;

    private:
        Eye originalCurrentEye() const;
    };

    // The original's currentEye has the same name as ours, so it is found by name
    VRConfig::Eye VRConfig::originalCurrentEye() const
    {
        typedef int (*CurrentEyeFn)(const VRConfig*);
        static CurrentEyeFn original = reinterpret_cast<CurrentEyeFn>(
            GetProcAddress(GetModuleHandleW(L"deng_appfw_doomsday.dll"), "?currentEye@VRConfig@de@@QEBA?AW4Eye@12@XZ"));
        return original ? static_cast<Eye>(original(this)) : NeitherEye;
    }

    VRConfig::Eye VRConfig::currentEye() const
    {
        const Eye eye = originalCurrentEye();
        CrosshairEye(static_cast<int>(eye));
        return eye;
    }

    Matrix4<float> VRConfig::projectionMatrix(float fovDegrees, Vector2<float> const& viewPortSize,
                                              float nearClip, float farClip) const
    {
        const float pi = 3.14159265358979f;
        const float focal = 1.f / std::tan(0.5f * fovDegrees * pi / 180.f);   // fovDegrees is horizontal
        const float aspect = viewAspect(viewPortSize);
        const float shift = eyeShift();

        Matrix4<float> m;
        m.at(0, 0) = focal;
        m.at(1, 1) = focal * aspect;
        m.at(2, 2) = (farClip + nearClip) / (nearClip - farClip);
        m.at(2, 3) = 2.f * farClip * nearClip / (nearClip - farClip);
        m.at(3, 2) = -1.f;

        // The eye moves sideways by "shift" map units
        m.at(0, 3) = -focal * shift;

        // Convergence: the zero parallax plane at the screen distance
        const float screenUnits = screenDistance() * mapUnitsPerMeter();
        const bool converge = frustumShift() && screenUnits > 0.f && shift != 0.f;
        if (converge)
        {
            m.at(0, 2) = -focal * shift / screenUnits;
        }

        // The 3D crosshair: drawn at each eye's first projection (its view was just cleared)
        CrosshairProjection(static_cast<int>(originalCurrentEye()), static_cast<int>(mode()), focal, shift,
                            converge ? screenUnits : 0.f, nearClip, farClip);
        return m;
    }
}
