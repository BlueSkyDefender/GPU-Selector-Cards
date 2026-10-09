# Card Site Template

A small login site for your mods. A card with a `"site"` logs in to it, shows each user their files, and installs them. You run it on your own free Cloudflare account. GPU Selector hosts nothing.

## Set it up

1. Make a free Cloudflare account. Create a **Worker** and paste in `worker.js`.
2. Create an **R2 bucket**, upload your files (`.zip` or `.7z`), and bind it to the Worker as `BUCKET`.
3. In the Worker's **Settings, Variables**, add:
   - `SECRET`: any long random text (mark it secret).
   - `USERS`: `{"shrimp": "<sha256 of the password>"}` (mark it secret).
   - `FILES`: your file list (see the top of `worker.js`).
4. In your card: `"site": {"url": "https://<your worker>.workers.dev", "name": "My Mods"}`.

## Make a password's sha256

PowerShell: `[BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash([Text.Encoding]::UTF8.GetBytes("shrimp"))).Replace("-","").ToLower()`

## What it does

1. **Log in:** checks the password and gives back a signed token that runs out after 4 hours.
2. **Files:** lists only the files that user may get.
3. **Download:** sends the file from your R2 bucket.

The full rules are in the card guide, **Card Sites**.

## License

BSD Zero Clause. Given "as is", with no warranty.
