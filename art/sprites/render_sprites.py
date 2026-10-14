"""Renders every <g id="part-NAME"> of each sprite SVG here to Content/Sprites/<sprite>_<NAME>@<D>.png.

Each part keeps the full sprite frame (same size and origin as the SVG), so the game can stack parts
and rotate them about pivots given in SVG units. Every part is rendered at several densities D
(pixels per canvas unit); the game picks the smallest one that is at least as sharp as the screen
needs, so sprites stay crisp from a small window up to 4K without heavy downscaling.

The SVG root says how wide the sprite is drawn on the game's 1300x700 canvas (data-canvas-width) and,
optionally, which densities to render (data-densities, default "2 4 8"). Uses headless Edge.

    python -I render_sprites.py            # all sprites
    python -I render_sprites.py squirrel   # one sprite
"""
import subprocess
import sys
import tempfile
import time
import xml.etree.ElementTree as ET
from pathlib import Path

HERE = Path(__file__).resolve().parent
OUT = HERE.parent.parent / "AlienIdle" / "Content" / "Sprites"
EDGE = Path(r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe")
SVG_NS = "http://www.w3.org/2000/svg"
ET.register_namespace("", SVG_NS)


def render(svg_path: Path, png_path: Path, width: int, height: int, profile: Path) -> None:
    # Edge occasionally exits without writing anything, so delete the old file and retry until it appears.
    png_path.unlink(missing_ok=True)
    for attempt in range(4):
        subprocess.run([str(EDGE), "--headless=new", "--disable-gpu", "--hide-scrollbars", "--no-first-run",
                        f"--user-data-dir={profile}{attempt}", "--default-background-color=00000000",
                        f"--window-size={width},{height}", f"--screenshot={png_path}", svg_path.as_uri()],
                       check=True, capture_output=True)
        for _ in range(20):
            if png_path.exists():
                return
            time.sleep(0.25)
    raise RuntimeError(f"Edge did not write {png_path}")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    names = sys.argv[1:] or [p.stem for p in sorted(HERE.glob("*.svg"))]
    with tempfile.TemporaryDirectory(ignore_cleanup_errors=True) as tmp:  # Edge may still hold its profile
        tmp = Path(tmp)
        for name in names:
            root = ET.parse(HERE / f"{name}.svg").getroot()
            _, _, vb_w, vb_h = (float(v) for v in root.get("viewBox").split())
            canvas_w = float(root.get("data-canvas-width", vb_w))
            densities = [float(d) for d in root.get("data-densities", "2 4 8").split()]
            parts = [g.get("id")[5:] for g in root.iter(f"{{{SVG_NS}}}g") if (g.get("id") or "").startswith("part-")]
            for old in OUT.glob(f"{name}_*.png"):
                old.unlink()
            for part in parts:
                for d in densities:
                    single = ET.parse(HERE / f"{name}.svg").getroot()
                    for g in list(single):
                        if g.tag == f"{{{SVG_NS}}}g" and g.get("id") != f"part-{part}":
                            single.remove(g)
                    width = round(canvas_w * d)
                    height = round(canvas_w * d * vb_h / vb_w)
                    single.set("width", str(width))
                    single.set("height", str(height))
                    tag = f"{d:g}"
                    svg = tmp / f"{name}_{part}@{tag}.svg"
                    ET.ElementTree(single).write(svg, encoding="utf-8", xml_declaration=True)
                    png = OUT / f"{name}_{part}@{tag}.png"
                    # A fresh profile per call: a shared one hands off to the previous Edge and writes nothing.
                    render(svg, png, width, height, tmp / f"profile_{name}_{part}_{tag}_")
                    print(f"{png.name} ({width}x{height})")


if __name__ == "__main__":
    main()
