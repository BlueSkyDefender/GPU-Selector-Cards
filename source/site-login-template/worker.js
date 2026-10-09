// GPU Selector card site: a small login site for your mods, as a Cloudflare Worker (free plan is fine).
// A card with "site": {"url": "https://<your worker>.workers.dev"} logs in here and shows your files.
// See the card guide, "Card Sites". License: BSD Zero Clause. Given "as is", with no warranty.
//
// Settings (Worker, Settings, Variables; mark USERS and SECRET as secrets):
//   SECRET  any long random text. It signs the logins.
//   USERS   {"shrimp": "<sha256 of the password>", "other": "..."}   (sha256 in hex, lower case)
//   FILES   [{"id": "shrimp-vision", "title": "Shrimp Vision", "version": "1.0", "short": "A shrimp for your game.",
//             "file": "ShrimpVision.zip", "size": 1234, "sha256": "<sha256 of the file>", "users": ["shrimp"]}]
//           "users" is who may get the file. Leave it out for everyone who can log in.
// Binding: an R2 bucket named BUCKET that holds the files (by their "file" name).

const HOURS = 4;

export default
{
	async fetch(request, env)
	{
		const url = new URL(request.url);
		if (url.pathname === "/gpuselector/login" && request.method === "POST")
		{
			const { username, password } = await request.json().catch(() => ({}));
			const users = JSON.parse(env.USERS || "{}");
			const known = typeof username === "string" && users[username];
			if (!known || !(await same(await sha256(String(password)), known)))
			{
				return Response.json({ ok: false, message: "The username or password is wrong." });
			}
			const expires = Math.floor(Date.now() / 1000) + HOURS * 3600;
			const token = username + "." + expires + "." + await sign(env.SECRET, username + "." + expires);
			return Response.json({ ok: true, token, message: "Welcome, " + username + "!" });
		}

		const user = await whoIs(request, env);
		if (!user)
		{
			return new Response("Log in again", { status: 401 });
		}
		const files = JSON.parse(env.FILES || "[]").filter(f => !f.users || f.users.includes(user));

		if (url.pathname === "/gpuselector/files" && request.method === "GET")
		{
			return Response.json({ files: files.map(({ users, ...shown }) => shown) });
		}
		if (url.pathname.startsWith("/gpuselector/download/") && request.method === "GET")
		{
			const id = url.pathname.slice("/gpuselector/download/".length);
			const file = files.find(f => f.id === id);
			const object = file ? await env.BUCKET.get(file.file) : null;
			return object ? new Response(object.body) : new Response("Not found", { status: 404 });
		}
		return new Response("Not found", { status: 404 });
	}
};

// The user of a token that is signed by us and not run out yet, or null
async function whoIs(request, env)
{
	const header = request.headers.get("Authorization") || "";
	const [user, expires, signature] = header.replace(/^Bearer /, "").split(".");
	if (!user || !expires || !signature || Number(expires) < Date.now() / 1000)
	{
		return null;
	}
	return (await same(signature, await sign(env.SECRET, user + "." + expires))) ? user : null;
}

async function sha256(text)
{
	const hash = await crypto.subtle.digest("SHA-256", new TextEncoder().encode(text));
	return [...new Uint8Array(hash)].map(b => b.toString(16).padStart(2, "0")).join("");
}

async function sign(secret, text)
{
	const key = await crypto.subtle.importKey("raw", new TextEncoder().encode(secret), { name: "HMAC", hash: "SHA-256" }, false, ["sign"]);
	const mac = await crypto.subtle.sign("HMAC", key, new TextEncoder().encode(text));
	return [...new Uint8Array(mac)].map(b => b.toString(16).padStart(2, "0")).join("");
}

// Compare without giving away how much matched
async function same(a, b)
{
	const [x, y] = [await sha256(String(a)), await sha256(String(b))];
	let diff = 0;
	for (let i = 0; i < x.length; i++)
	{
		diff |= x.charCodeAt(i) ^ y.charCodeAt(i);
	}
	return diff === 0;
}
