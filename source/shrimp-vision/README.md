# Shrimp Vision

A little shrimp floats over your game. Motion in the picture pushes it around. The prize inside the [Locked Card Example](../../cards/testing/locked-card-example/).

## How it works

1. The picture is shrunk to 64 x 36 brightness values (fast).
2. Simple motion vectors from that: the change since the last frame and the DDX / DDY gradients.
3. The shrimp's place and speed live in a 1 x 1 texture. Motion around it pushes it, and it swims a little by itself.
4. The shrimp is drawn with circles and lines. No image files.

## Settings

| Setting | What it does |
|---|---|
| Shrimp Size | How big the shrimp is |
| Push Strength | How hard motion pushes it |
| Shrimp Vision Tint | A pink tint over the picture (0 is off) |

## Lock it again

The card's `ShrimpVision.7z` is made with:

`7z a -t7z -mhe=on -p"shrimp:shrimp" ShrimpVision.7z ShrimpVision.fx`

## License

BSD Zero Clause, like the example cards. Given "as is", with no warranty.
