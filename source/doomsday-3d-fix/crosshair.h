// GPU Selector: the 3D crosshair (laser dot) for Doomsday's own 3D. See crosshair.cpp.
#pragma once

// Doomsday's current eye: 0 neither (no 3D), 1 left, 2 right. Called each time Doomsday asks.
void CrosshairEye(int eye);

// The first projection of each eye: the eye's view is cleared and nothing is drawn yet.
//   mode: Doomsday's 3D mode, focal: 1 / tan(half the horizontal FOV), shift: the eye's sideways
//   move (map units), screenUnits: the zero parallax distance (0 = none), near / far: clip planes
void CrosshairProjection(int eye, int mode, float focal, float shift, float screenUnits, float nearClip, float farClip);
