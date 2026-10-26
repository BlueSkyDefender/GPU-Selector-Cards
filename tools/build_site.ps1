# PowerShell version of build_site.py (with format_cards.py and export_guide.py), for PCs without Python.
# Same steps and the same output:
#   0a. a card's own files (its guide and font) get their SHA256 filled in
#   0.  every card laid out for people to read (format_cards.py)
#   1.  the cards copied into docs\cards (Linux line endings for .json and .txt)
#   2.  the DOOM card package (doom_card_package.zip)
#   3.  Cards on the House (docs\house_cards.json)
#   4.  the site pages from tools\pages and tools\page_template.html
#   5.  the card guide exported from GPU Selector (export_guide.py)
#   powershell -ExecutionPolicy Bypass -File tools\build_site.ps1 [path to mod_guide_handler.cpp]
param([string]$GuideSource = 'G:\General Projects\Game Selector App\GPUSelectorGUI_VS\src\mod_guide_handler.cpp')

$ErrorActionPreference = 'Stop'
$Here = $PSScriptRoot
$Root = Split-Path $Here -Parent
$Site = Join-Path $Root 'docs'
$Utf8 = New-Object System.Text.UTF8Encoding $false

# Reads a text file like Python does: any CR LF or lone CR becomes LF
function ReadText([string]$path)
{
    return [IO.File]::ReadAllText($path, $Utf8).Replace("`r`n", "`n").Replace("`r", "`n")
}

Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;

// A JSON reader and writer that lays cards out exactly like format_cards.py (Python's json module)
public static class CardLayout
{
    class Node
    {
        public char Kind;    // o object, a array, s string, n number, l literal
        public List<KeyValuePair<string, Node>> Fields;
        public List<Node> Items;
        public string Text;
    }

    static string src;
    static int pos;

    static void Skip() { while (pos < src.Length && char.IsWhiteSpace(src[pos])) pos++; }

    static Node Read()
    {
        Skip();
        char c = src[pos];
        if (c == '{')
        {
            var node = new Node { Kind = 'o', Fields = new List<KeyValuePair<string, Node>>() };
            pos++; Skip();
            if (src[pos] == '}') { pos++; return node; }
            while (true)
            {
                Skip();
                string key = ReadString();
                Skip(); if (src[pos] != ':') throw new Exception("':' expected at " + pos); pos++;
                node.Fields.Add(new KeyValuePair<string, Node>(key, Read()));
                Skip();
                if (src[pos] == ',') { pos++; continue; }
                if (src[pos] == '}') { pos++; return node; }
                throw new Exception("',' or '}' expected at " + pos);
            }
        }
        if (c == '[')
        {
            var node = new Node { Kind = 'a', Items = new List<Node>() };
            pos++; Skip();
            if (src[pos] == ']') { pos++; return node; }
            while (true)
            {
                node.Items.Add(Read());
                Skip();
                if (src[pos] == ',') { pos++; continue; }
                if (src[pos] == ']') { pos++; return node; }
                throw new Exception("',' or ']' expected at " + pos);
            }
        }
        if (c == '"') return new Node { Kind = 's', Text = ReadString() };
        int start = pos;
        while (pos < src.Length && "+-0123456789.eE".IndexOf(src[pos]) >= 0) pos++;
        if (pos > start) return new Node { Kind = 'n', Text = Number(src.Substring(start, pos - start)) };
        foreach (var word in new[] { "true", "false", "null" })
        {
            if (string.CompareOrdinal(src, pos, word, 0, word.Length) == 0) { pos += word.Length; return new Node { Kind = 'l', Text = word }; }
        }
        throw new Exception("Unexpected character at " + pos);
    }

    static string ReadString()
    {
        if (src[pos] != '"') throw new Exception("String expected at " + pos);
        pos++;
        var sb = new StringBuilder();
        while (src[pos] != '"')
        {
            char c = src[pos++];
            if (c != '\\') { sb.Append(c); continue; }
            char e = src[pos++];
            switch (e)
            {
                case 'n': sb.Append('\n'); break;
                case 'r': sb.Append('\r'); break;
                case 't': sb.Append('\t'); break;
                case 'b': sb.Append('\b'); break;
                case 'f': sb.Append('\f'); break;
                case 'u': sb.Append((char)Convert.ToInt32(src.Substring(pos, 4), 16)); pos += 4; break;
                default: sb.Append(e); break;
            }
        }
        pos++;
        return sb.ToString();
    }

    // Python reads a number with a '.' or an exponent as a float and writes it back with repr()
    static string Number(string raw)
    {
        if (raw.IndexOfAny(new[] { '.', 'e', 'E' }) < 0)
        {
            return raw == "-0" ? "0" : raw;
        }
        double d = double.Parse(raw, CultureInfo.InvariantCulture);
        string r = d.ToString("R", CultureInfo.InvariantCulture);
        int ePos = r.IndexOf('E');
        if (ePos >= 0)
        {
            string mantissa = r.Substring(0, ePos);
            int exp = int.Parse(r.Substring(ePos + 1), CultureInfo.InvariantCulture);
            return mantissa + "e" + (exp < 0 ? "-" : "+") + Math.Abs(exp).ToString("00");
        }
        // Python writes 1e16 and up, and below 1e-4, with an exponent
        double a = Math.Abs(d);
        if (a != 0 && (a >= 1e16 || a < 1e-4))
        {
            string s = d.ToString("E16", CultureInfo.InvariantCulture);
            double m = double.Parse(s.Substring(0, s.IndexOf('E')), CultureInfo.InvariantCulture);
            int exp = int.Parse(s.Substring(s.IndexOf('E') + 1), CultureInfo.InvariantCulture);
            string ms = m.ToString("R", CultureInfo.InvariantCulture);
            return ms + "e" + (exp < 0 ? "-" : "+") + Math.Abs(exp).ToString("00");
        }
        return r.IndexOf('.') < 0 ? r + ".0" : r;
    }

    // json.dumps(s, ensure_ascii=False)
    static string Quote(string s)
    {
        var sb = new StringBuilder("\"");
        foreach (char c in s)
        {
            switch (c)
            {
                case '"': sb.Append("\\\""); break;
                case '\\': sb.Append("\\\\"); break;
                case '\n': sb.Append("\\n"); break;
                case '\r': sb.Append("\\r"); break;
                case '\t': sb.Append("\\t"); break;
                case '\b': sb.Append("\\b"); break;
                case '\f': sb.Append("\\f"); break;
                default:
                    if (c < 0x20) sb.Append("\\u").Append(((int)c).ToString("x4"));
                    else sb.Append(c);
                    break;
            }
        }
        return sb.Append('"').ToString();
    }

    static bool Plain(Node n) { return n.Kind != 'o' && n.Kind != 'a'; }

    static string Value(Node n) { return n.Kind == 's' ? Quote(n.Text) : n.Text; }

    // Python counts characters, not UTF-16 halves
    static int Length(string s)
    {
        int count = 0;
        foreach (char c in s) if (!char.IsLowSurrogate(c)) count++;
        return count;
    }

    const string Indent = "  ";
    const int ShortList = 100;

    static string Dump(Node n, int depth, string path)
    {
        string pad = Repeat(depth), inner = Repeat(depth + 1);
        if (n.Kind == 'o')
        {
            if (n.Fields.Count == 0) return "{}";
            bool spaced = path == "" || path == "/catalog";
            var sb = new StringBuilder();
            bool? lastPlain = null;
            foreach (var f in n.Fields)
            {
                string line = inner + Quote(f.Key) + ": " + Dump(f.Value, depth + 1, path + "/" + f.Key);
                if (lastPlain != null)
                {
                    sb.Append(spaced && !(lastPlain.Value && Plain(f.Value)) ? ",\n\n" : ",\n");
                }
                sb.Append(line);
                lastPlain = Plain(f.Value);
            }
            return "{\n" + sb + "\n" + pad + "}";
        }
        if (n.Kind == 'a')
        {
            if (n.Items.Count == 0) return "[]";
            bool allPlain = true, allObjects = true;
            foreach (var i in n.Items) { allPlain &= Plain(i); allObjects &= i.Kind == 'o'; }
            if (allPlain)
            {
                var parts = new List<string>();
                foreach (var i in n.Items) parts.Add(Value(i));
                string oneLine = "[" + string.Join(", ", parts) + "]";
                if (Length(oneLine) <= ShortList) return oneLine;
            }
            string gap = allObjects && n.Items.Count > 1 ? ",\n\n" : ",\n";
            var lines = new List<string>();
            foreach (var i in n.Items) lines.Add(inner + Dump(i, depth + 1, path + "/[]"));
            return "[\n" + string.Join(gap, lines) + "\n" + pad + "]";
        }
        return Value(n);
    }

    static string Repeat(int depth)
    {
        var sb = new StringBuilder();
        for (int i = 0; i < depth; i++) sb.Append(Indent);
        return sb.ToString();
    }

    static Node Parse(string text)
    {
        src = text; pos = 0;
        if (src.Length > 0 && src[0] == '\uFEFF') pos = 1;
        Node n = Read();
        Skip();
        if (pos != src.Length) throw new Exception("Extra text after the card at " + pos);
        return n;
    }

    // Returns the card laid out, and checks that laying it out again changes nothing
    public static string Format(string text)
    {
        string pretty = Dump(Parse(text), 0, "") + "\n";
        if (Dump(Parse(pretty), 0, "") + "\n" != pretty) throw new Exception("format: the layout is not stable");
        return pretty;
    }

    // Reads one string value by a path like "ui/guide/sha256" (null when it isn't there)
    public static string Get(string text, string path)
    {
        Node n = Parse(text);
        foreach (string part in path.Split('/'))
        {
            if (n == null || n.Kind != 'o') return null;
            Node next = null;
            foreach (var f in n.Fields) if (f.Key == part) next = f.Value;
            n = next;
        }
        return n != null && n.Kind == 's' ? n.Text : null;
    }
}
'@

function Sha256([string]$path)
{
    return (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLower()
}

# 0a. A card's own files (its guide and font, in the <card>_files folder next to it) get their
#     SHA256 filled in, so editing GUIDE.md never hides the Guide button by a stale hash
foreach ($card in Get-ChildItem (Join-Path $Root 'cards') -Directory | ForEach-Object { Get-ChildItem $_.FullName -Directory } | ForEach-Object { Get-ChildItem $_.FullName -Filter *.json -File })
{
    $filesFolder = Join-Path $card.DirectoryName ($card.BaseName + '_files')
    if (-not (Test-Path $filesFolder))
    {
        continue
    }
    $text = [IO.File]::ReadAllText($card.FullName, $Utf8)
    foreach ($key in 'guide', 'font')
    {
        $file = [CardLayout]::Get($text, "ui/$key/file")
        $sha = [CardLayout]::Get($text, "ui/$key/sha256")
        if (-not $file -or -not $sha)
        {
            continue
        }
        $local = Join-Path $filesFolder $file
        if (Test-Path -LiteralPath $local)
        {
            $new = Sha256 $local
            if ($new -ne $sha)
            {
                $text = $text.Replace('"' + $sha + '"', '"' + $new + '"')
                Write-Output ("updated ui." + $key + " sha256 in " + $card.Name)
            }
        }
    }
    [IO.File]::WriteAllText($card.FullName, $text, $Utf8)
}

# 0. Every card laid out for people to read (spaces only, the content never changes)
$changed = @()
foreach ($card in Get-ChildItem (Join-Path $Root 'cards') -Recurse -Filter *.json -File | Sort-Object FullName)
{
    $text = ReadText $card.FullName
    $pretty = [CardLayout]::Format($text)
    if ($pretty -ne $text)
    {
        [IO.File]::WriteAllText($card.FullName, $pretty, $Utf8)
        $changed += $card.FullName.Substring((Join-Path $Root 'cards').Length + 1)
    }
}
Write-Output ("cards laid out" + $(if ($changed.Count) { ": " + ($changed -join ', ') } else { " (no changes)" }))

# 1. The cards, for the site's Copy Link and Download buttons
$siteCards = Join-Path $Site 'cards'
if (Test-Path $siteCards)
{
    Remove-Item $siteCards -Recurse -Force
}
Copy-Item (Join-Path $Root 'cards') $siteCards -Recurse
Get-ChildItem $siteCards -Recurse -Filter README.md -File | Remove-Item -Force
# Text files with Linux line endings, the same on every PC, so their SHA256 always matches
foreach ($file in Get-ChildItem $siteCards -Recurse -File | Where-Object { $_.Extension -in '.json', '.txt' })
{
    $bytes = [IO.File]::ReadAllBytes($file.FullName)
    $out = New-Object System.Collections.Generic.List[byte]
    for ($i = 0; $i -lt $bytes.Length; $i++)
    {
        if ($bytes[$i] -eq 13 -and $i + 1 -lt $bytes.Length -and $bytes[$i + 1] -eq 10)
        {
            continue
        }
        $out.Add($bytes[$i])
    }
    [IO.File]::WriteAllBytes($file.FullName, $out.ToArray())
}

# 2. The DOOM card package: the card plus its doom_card_files folder
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
$doom = Join-Path $siteCards '4-expert\doom'
$packagePath = Join-Path $doom 'doom_card_package.zip'
$package = [IO.Compression.ZipFile]::Open($packagePath, [IO.Compression.ZipArchiveMode]::Create)
try
{
    [void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile($package, (Join-Path $doom 'doom_card.json'), 'doom_card.json', [IO.Compression.CompressionLevel]::Optimal)
    foreach ($file in Get-ChildItem (Join-Path $doom 'doom_card_files') -File | Sort-Object Name)
    {
        [void][IO.Compression.ZipFileExtensions]::CreateEntryFromFile($package, $file.FullName, 'doom_card_files/' + $file.Name, [IO.Compression.CompressionLevel]::Optimal)
    }
}
finally
{
    $package.Dispose()
}
Write-Output 'docs/cards (with doom_card_package.zip)'

# 3. Cards on the House: the list GPU Selector shows in + Add Mod (made by us, so trusted).
#    Only these cards; the pure examples (Shader Starter Pack, Feature Test) are left out.
#    Add a new card here and it shows up in GPU Selector without an app update.
$house = @(
    [ordered]@{ name = 'RenoDX HDR'; file = 'cards/2-intermediate/renodx-hdr/renodx_hdr.json'
                short = 'HDR for games that never had it. Needs an HDR display.'; note = 'Card made by us.' },
    [ordered]@{ name = 'Neural Rendering'; file = 'cards/3-advanced/neural-rendering/neural_rendering.json'
                short = 'Neural Rendering (DLSS 5 and OpenNR) as ReShade add-ons.'; note = 'Card made by us.' },
    [ordered]@{ name = 'DOOM'; file = 'cards/4-expert/doom/doom_card_package.zip'
                short = 'DOOM mods, map packs and the original games, ready to play.'
                note = 'Card made by us. Brings two DLLs we made too: the Doomsday 3D Fix and the 3D crosshair dot.' }
)
# Written like Python's json.dump(..., indent=2): ASCII only, no newline at the end
function JsonAscii([string]$s)
{
    $sb = New-Object System.Text.StringBuilder '"'
    foreach ($c in $s.ToCharArray())
    {
        $code = [int]$c
        if ($c -eq '"') { [void]$sb.Append('\"') }
        elseif ($c -eq '\') { [void]$sb.Append('\\') }
        elseif ($code -eq 10) { [void]$sb.Append('\n') }
        elseif ($code -eq 13) { [void]$sb.Append('\r') }
        elseif ($code -eq 9) { [void]$sb.Append('\t') }
        elseif ($code -lt 32 -or $code -gt 126) { [void]$sb.Append('\u' + $code.ToString('x4')) }
        else { [void]$sb.Append($c) }
    }
    return $sb.Append('"').ToString()
}
$cardTexts = @()
foreach ($card in $house)
{
    $card['sha256'] = Sha256 (Join-Path $Site $card.file)
    $fields = @()
    foreach ($key in $card.Keys)
    {
        $fields += '      ' + (JsonAscii $key) + ': ' + (JsonAscii $card[$key])
    }
    $cardTexts += "    {`n" + ($fields -join ",`n") + "`n    }"
}
$houseJson = "{`n  `"version`": 1,`n  `"cards`": [`n" + ($cardTexts -join ",`n") + "`n  ]`n}"
[IO.File]::WriteAllText((Join-Path $Site 'house_cards.json'), $houseJson, $Utf8)
Write-Output ("docs/house_cards.json (" + $house.Count + " cards)")

# 4. The site pages
$template = ReadText (Join-Path $Here 'page_template.html')
$pages = [ordered]@{ 'index.html' = @('Home', 'HOME'); 'cards.html' = @('Example Cards', 'CARDS'); 'safety.html' = @('Safety', 'SAFETY') }
foreach ($name in $pages.Keys)
{
    $title = $pages[$name][0]
    $active = $pages[$name][1]
    $content = ReadText (Join-Path $Here "pages\$name")
    $page = $template.Replace('{{TITLE}}', $title).Replace('{{ACTIVE_' + $active + '}}', ' class="active"')
    $page = [regex]::Replace($page, '\{\{ACTIVE_[A-Z]+\}\}', '').Replace('{{CONTENT}}', $content)
    [IO.File]::WriteAllText((Join-Path $Site $name), $page.Replace("`r`n", "`n"), $Utf8)
    Write-Output "docs/$name"
}

# 5. GPU Selector's built-in Custom Mod Card Guide (src\mod_guide_handler.cpp) as docs\guide.html
$code = ReadText $GuideSource
$open = 'guideFile << R"('
$start = $code.IndexOf($open) + $open.Length
$end = $code.IndexOf(')";', $start)
$html = $code.Substring($start, $end - $start).Replace(')" R"(', '')    # the guide is split into several raw strings
$bodyStart = $html.IndexOf('<body>') + '<body>'.Length
$body = $html.Substring($bodyStart, $html.IndexOf('</body>') - $bodyStart)
$body = [regex]::Replace($body.Trim(), '<hr[^>]*>\s*<p[^>]*>GPU Selector - Custom Mod Card System[^<]*</p>\s*$', '')
$page = $template.Replace('{{TITLE}}', 'Card Guide').Replace('{{ACTIVE_GUIDE}}', ' class="active"')
$page = [regex]::Replace($page, '\{\{ACTIVE_[A-Z]+\}\}', '')
$page = $page.Replace('{{CONTENT}}', "<div class=`"guide-content card-guide`">`n" + $body + "`n</div>")
$page = $page.Replace("`r`n", "`n")
[IO.File]::WriteAllText((Join-Path $Site 'guide.html'), $page, $Utf8)
Write-Output ("docs/guide.html written, " + $page.Length + " bytes")

# No em dashes anywhere on the site
foreach ($file in Get-ChildItem $Site -Recurse -Filter *.html -File)
{
    $text = [IO.File]::ReadAllText($file.FullName, $Utf8)
    if ($text.Contains([string][char]0x2014) -or $text.Contains('&mdash;'))
    {
        Write-Output ("WARNING: em dash in " + $file.Name)
    }
}
