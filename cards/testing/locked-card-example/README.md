# Locked Card Example

Shows how a card maker can lock a card, so only people with a key can install it.

## Try it

1. In GPU Selector: **Mods** tab, **+ Add Mod**, **Load from File**, pick `locked_card_example.json`.
2. Press **Download**. GPU Selector asks for a username and password.
3. Type **shrimp** and **shrimp**. This is only an example, so the key is printed here.
4. The prize, **Shrimp Vision**, goes to GPU Selector's ReShade shaders. Turn it on in ReShade's menu in any game.

## How the lock works

1. `ShrimpVision.7z` is locked with 7-Zip (AES-256, file names hidden too). Its password is `shrimp:shrimp` (username:password).
2. The card only says `"password": "login"`. It never holds the key.
3. GPU Selector asks for the key, uses it once to unpack, then forgets it. It is never saved, sent anywhere, or logged.
4. How to lock your own card: the card guide, **Locked Cards**.

## Limits

1. Everyone with the key can unpack the file, and keys can be shared.
2. This is a pure example. It is not in Cards on the House.

## License

BSD Zero Clause, like the rest of the example cards. The shader's source: [source/shrimp-vision](../../../source/shrimp-vision/).
