"""Generates the hunting-region backdrops and Earth's-response sprites (SVG), then you render them with
render_sprites.py. Regions are 220x210 frames drawn at canvas (x0, 100); the ground line is at local y 200.

    python -I regions_art.py
"""
import random
from pathlib import Path

HERE = Path(__file__).resolve().parent
rnd = random.Random(11)

DEFS = '''  <defs>
    <linearGradient id="hill" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#2f5a3a"/><stop offset="1" stop-color="#1c3a26"/></linearGradient>
    <linearGradient id="hillFar" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#243a46"/><stop offset="1" stop-color="#1a2a36"/></linearGradient>
    <linearGradient id="grass" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#4f8a3a"/><stop offset="1" stop-color="#2f5a26"/></linearGradient>
    <linearGradient id="walk" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#8a8c92"/><stop offset="1" stop-color="#5a5c62"/></linearGradient>
    <linearGradient id="wood" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#9a6a3c"/><stop offset="1" stop-color="#5e3a1c"/></linearGradient>
    <linearGradient id="barn" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#7a1e16"/><stop offset="0.5" stop-color="#a8302a"/><stop offset="1" stop-color="#6a1810"/></linearGradient>
    <linearGradient id="bldgR" x1="0" y1="0" x2="1" y2="0"><stop offset="0" stop-color="#2a3048"/><stop offset="0.5" stop-color="#3a4262"/><stop offset="1" stop-color="#232838"/></linearGradient>
    <linearGradient id="water" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#6fb3d6"/><stop offset="1" stop-color="#2f6a8a"/></linearGradient>
    <radialGradient id="glowY" cx="0.5" cy="0.5" r="0.5"><stop offset="0" stop-color="#ffe08a" stop-opacity="0.8"/><stop offset="1" stop-color="#ffe08a" stop-opacity="0"/></radialGradient>
    <radialGradient id="cone" cx="0" cy="0.5" r="1"><stop offset="0" stop-color="#fff6c4" stop-opacity="0.75"/><stop offset="1" stop-color="#fff6c4" stop-opacity="0"/></radialGradient>
    <filter id="tex" x="0" y="0" width="100%" height="100%">
      <feTurbulence type="fractalNoise" baseFrequency="0.5" numOctaves="2" seed="3"/>
      <feColorMatrix type="matrix" values="0 0 0 0 0  0 0 0 0 0.05  0 0 0 0 0  1.4 0 0 0 -0.75"/>
    </filter>
  </defs>
'''


def pine(x, y, s=1.0, snow=False):
    t = f'<g transform="translate({x} {y}) scale({s})" stroke="#0f1a14" stroke-width="0.5"><rect x="-1" y="-4" width="2" height="4" fill="#3b2a1a"/><path d="M0 -24 L 7 -11 L 3.5 -11 L 9 -4 L -9 -4 L -3.5 -11 L -7 -11 Z" fill="#1f4a32"/>'
    if snow:
        t += '<path d="M0 -24 L 2.6 -19 L -2.6 -19 Z" fill="#e6edf7"/>'
    return t + '</g>'


def oak(x, y, s=1.0):
    return (f'<g transform="translate({x} {y}) scale({s})" stroke="#0f1a14" stroke-width="0.5"><rect x="-1.6" y="-10" width="3.2" height="10" fill="#4a3220"/>'
            f'<circle cx="0" cy="-16" r="9" fill="#2f6a3a"/><circle cx="-6" cy="-12" r="6" fill="#2a5a32"/><circle cx="6" cy="-13" r="6.5" fill="#367a42"/></g>')


def house(x, y, w=28, h=18, wall='#c9b28a', roof='#5a2a1e', lit=True):
    win = '#ffe08a' if lit else '#2a2e3a'
    return (f'<g stroke="#14161a" stroke-width="0.5"><rect x="{x}" y="{y - h}" width="{w}" height="{h}" fill="{wall}"/>'
            f'<path d="M{x - 3} {y - h} L {x + w / 2} {y - h - 12} L {x + w + 3} {y - h} Z" fill="{roof}"/>'
            f'<rect x="{x + 4}" y="{y - h + 5}" width="6" height="5" fill="{win}"/><rect x="{x + w - 10}" y="{y - h + 5}" width="6" height="5" fill="{win}"/>'
            f'<rect x="{x + w / 2 - 3}" y="{y - 9}" width="6" height="9" fill="#4a3220"/></g>')


def ground(kind='grass'):
    if kind == 'walk':
        return '<rect x="0" y="200" width="220" height="10" fill="url(#walk)"/><path d="M0 200 H 220" stroke="#b8bcc2" stroke-width="0.8"/>'
    return '<rect x="0" y="200" width="220" height="10" fill="url(#grass)"/><path d="M0 200 H 220" stroke="#7fbf5a" stroke-width="0.8"/>'


def region(name, doc, back, extra_parts=''):
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 220 210" width="440" height="420" data-canvas-width="220" data-densities="2 4">
  <!--
    {doc}
    Hunting region backdrop, drawn at canvas (x0, 100), 220x210; ground line at local y 200 (canvas 300).
    The game draws the specimens, Earth's responses and labels on top.
  -->
{DEFS}  <g id="part-back">
{back}
  </g>
{extra_parts}</svg>
'''
    (HERE / f'{name}.svg').write_text(svg, encoding='utf-8')


# ------------------------------------------------------------------ Mountain regions
region('region_mountain_0', 'Pine Ridge Valley: forest, a creek, a barn and a pasture (squirrels, cows).', f'''
    <path d="M0 150 C 40 120, 90 130, 130 145 C 170 160, 200 140, 220 135 L 220 200 L 0 200 Z" fill="url(#hillFar)"/>
    {''.join(pine(x, 150 + rnd.randint(-6, 6), 0.8, True) for x in range(8, 130, 14))}
    <path d="M0 175 C 50 160, 120 170, 220 165 L 220 200 L 0 200 Z" fill="url(#hill)"/>
    {''.join(pine(x, 182 + rnd.randint(-3, 3), 1.0) for x in (10, 26, 40, 54))}
    <path d="M70 200 C 80 192, 96 194, 104 188 C 112 182, 128 186, 140 180" fill="none" stroke="url(#water)" stroke-width="3"/>
    <path d="M74 198 l 6 -3 M96 192 l 6 -2" stroke="#c9eaff" stroke-width="0.5"/>
    <g stroke="#14161a" stroke-width="0.6">
      <rect x="150" y="166" width="40" height="34" fill="url(#barn)"/>
      <path d="M146 166 L 170 150 L 194 166 Z" fill="#3a2a22"/>
      <rect x="162" y="178" width="16" height="22" fill="#5a1410"/>
      <path d="M162 178 L 178 200 M178 178 L 162 200" stroke="#e8dcc0" stroke-width="1"/>
      <rect x="166" y="158" width="8" height="6" fill="#ffe08a"/>
    </g>
    <path d="M104 200 V 186 M118 200 V 186 M132 200 V 186 M146 200 V 186 M104 190 H 146 M104 196 H 146" stroke="#c9b28a" stroke-width="1.2"/>
    <text x="40" y="140" font-family="Arial, sans-serif" font-size="5" font-weight="bold" fill="#9aa6bc" opacity="0.6">elev. 4,200 ft</text>
    {ground()}''')

region('region_mountain_1', 'Glenwhit: a small town, a diner, a water tower (squirrels, humans).', f'''
    <path d="M0 160 C 60 150, 140 155, 220 150 L 220 200 L 0 200 Z" fill="url(#hillFar)"/>
    <g stroke="#14161a" stroke-width="0.6">
      <path d="M176 200 L 180 140 M196 200 L 192 140 M178 170 H 194 M176 186 H 196" stroke="#6a6e78" stroke-width="1.4"/>
      <rect x="170" y="118" width="32" height="24" rx="4" fill="#8a9aac"/>
      <path d="M170 118 L 186 108 L 202 118 Z" fill="#5a6a7a"/>
    </g>
    <text x="186" y="133" font-family="Arial Black, Arial, sans-serif" font-size="5.4" font-weight="bold" text-anchor="middle" fill="#e8f0f6">GLENWHIT</text>
    <text x="186" y="139" font-family="Arial, sans-serif" font-size="2.6" text-anchor="middle" fill="#e8f0f6">pop. 312 (and falling)</text>
    {house(8, 200)}{house(42, 200, 26, 20, '#a8b8c8', '#2a3a5a')}{house(132, 200, 30, 18, '#d6c2a0', '#6a2a1e', False)}
    <g stroke="#14161a" stroke-width="0.6">
      <rect x="76" y="176" width="48" height="24" fill="#c8d8e0"/>
      <rect x="76" y="172" width="48" height="5" fill="#c8241c"/>
      <rect x="80" y="182" width="40" height="9" fill="#ffe08a"/>
      <rect x="94" y="190" width="10" height="10" fill="#4a3220"/>
    </g>
    <text x="100" y="176" font-family="Arial Black, Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#fff4cf">EAT</text>
    <text x="100" y="161" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#ff8a7a">PIE OF THE DAY: UNKNOWN</text>
    {oak(30, 200, 0.9)}{oak(165, 200, 0.8)}
    {ground()}''')

region('region_mountain_2', 'County Fairgrounds: big top, Ferris wheel, livestock pens (elephants, cows, humans).', f'''
    <path d="M0 165 C 70 158, 150 162, 220 156 L 220 200 L 0 200 Z" fill="url(#hillFar)"/>
    <g stroke="#14161a" stroke-width="0.6">
      <path d="M20 200 L 30 150 L 100 150 L 110 200 Z" fill="#e8e2d4"/>
      <path d="M30 150 L 38 200 M46 150 L 50 200 M62 150 L 63 200 M78 150 L 76 200 M94 150 L 90 200" stroke="#c8241c" stroke-width="5"/>
      <path d="M26 152 L 65 118 L 104 152 Z" fill="#c8241c"/>
      <path d="M65 118 V 106" stroke="#5a6068" stroke-width="1"/>
      <path d="M65 106 l 8 3 l -8 3 Z" fill="#e2b923"/>
      <path d="M58 200 L 58 182 C 58 176, 72 176, 72 182 L 72 200 Z" fill="#2a1a12"/>
    </g>
    <path d="M0 140 Q 30 150, 60 136 Q 90 150, 120 136" fill="none" stroke="#2a2e34" stroke-width="0.5"/>
    <g fill="#ffe08a">{''.join(f'<circle cx="{x}" cy="{140 + 6 * abs(((x - 0) % 60) - 30) / 30}" r="1"/>' for x in range(4, 120, 8))}</g>
    <path d="M150 200 L 172 150 L 194 200 M160 176 H 184" stroke="#6a6e78" stroke-width="1.6" fill="none"/>
    <g stroke="#14161a" stroke-width="0.6">
      <path d="M126 200 V 186 M140 200 V 186 M126 190 H 146 M126 196 H 146" stroke="#c9b28a" stroke-width="1.2"/>
      <rect x="196" y="182" width="22" height="18" fill="#d6b33a"/>
      <path d="M194 182 L 207 172 L 220 182 Z" fill="#8a2a24"/>
    </g>
    <text x="207" y="193" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#2a1a12">FUNNEL</text>
    <text x="207" y="197" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#2a1a12">CAKE</text>
    {ground()}''',
       extra_parts='''  <g id="part-wheel" stroke="#9aa1a8" stroke-width="0.8" fill="none">
    <!-- Ferris wheel: rotates about (172, 150) -->
    <circle cx="172" cy="150" r="26"/><circle cx="172" cy="150" r="3" fill="#5a6068"/>
    ''' + ''.join(f'<path d="M172 150 L {172 + 26 * __import__("math").cos(a * 0.5236):.1f} {150 + 26 * __import__("math").sin(a * 0.5236):.1f}"/>' for a in range(12)) + '''
    ''' + ''.join(f'<rect x="{172 + 26 * __import__("math").cos(a * 1.0472) - 3:.1f}" y="{150 + 26 * __import__("math").sin(a * 1.0472) - 2:.1f}" width="6" height="5" fill="{["#c8241c", "#2b5a8a", "#e2b923"][a % 3]}" stroke="#14161a" stroke-width="0.4"/>' for a in range(6)) + '''
    ''' + ''.join(f'<circle cx="{172 + 26 * __import__("math").cos(a * 0.5236):.1f}" cy="{150 + 26 * __import__("math").sin(a * 0.5236):.1f}" r="0.9" fill="#ffe08a" stroke="none"/>' for a in range(12)) + '''
  </g>
''')

# ------------------------------------------------------------------ Metropolis regions
region('region_city_0', 'Central Park: trees, a fountain, benches and lamp posts (squirrels, humans).', f'''
    <path d="M0 120 h 30 v 80 h -30 Z M34 100 h 26 v 100 h -26 Z M170 110 h 24 v 90 h -24 Z M196 130 h 24 v 70 h -24 Z" fill="url(#bldgR)" opacity="0.7"/>
    <path d="M0 175 C 60 168, 150 172, 220 168 L 220 200 L 0 200 Z" fill="url(#hill)"/>
    {''.join(oak(x, 196, 0.9 + 0.2 * rnd.random()) for x in (14, 40, 160, 190, 210))}
    <g stroke="#14161a" stroke-width="0.6">
      <ellipse cx="100" cy="196" rx="26" ry="4" fill="#8a9098"/>
      <ellipse cx="100" cy="195" rx="22" ry="3" fill="url(#water)"/>
      <rect x="97" y="178" width="6" height="16" fill="#a8b0b8"/>
      <ellipse cx="100" cy="178" rx="9" ry="2" fill="#8a9098"/>
      <path d="M100 176 C 96 168, 92 172, 90 180 M100 176 C 104 168, 108 172, 110 180" fill="none" stroke="#9fd0ff" stroke-width="1"/>
      <path d="M60 196 h 14 M62 196 v 3 M72 196 v 3 M60 192 h 14" stroke="#6a4020" stroke-width="1.4"/>
      <path d="M134 196 h 14 M136 196 v 3 M146 196 v 3 M134 192 h 14" stroke="#6a4020" stroke-width="1.4"/>
    </g>
    {''.join(f'<path d="M{x} 200 v -20" stroke="#2a2e34" stroke-width="1"/><circle cx="{x}" cy="179" r="2" fill="#fff1c2"/><circle cx="{x}" cy="179" r="8" fill="url(#glowY)"/>' for x in (52, 152))}
    <text x="100" y="166" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#9fd0ff" opacity="0.8">PLEASE DO NOT FEED THE SQUIRRELS (THEY TALK)</text>
    {ground()}''')

region('region_city_1', 'The Suburbs: identical houses, picket fences, lawn flamingos (cows, humans).', f'''
    <path d="M0 170 C 60 165, 150 168, 220 164 L 220 200 L 0 200 Z" fill="url(#hillFar)"/>
    {house(4, 200, 30, 18, '#d8d0c0', '#5a4a3a')}{house(50, 200, 30, 18, '#d8d0c0', '#5a4a3a', False)}{house(96, 200, 30, 18, '#d8d0c0', '#5a4a3a')}{house(142, 200, 30, 18, '#d8d0c0', '#5a4a3a')}{house(188, 200, 30, 18, '#d8d0c0', '#5a4a3a', False)}
    <path d="{' '.join(f'M{x} 200 v -6' for x in range(0, 220, 4))} M0 196 H 220" stroke="#f4f1e6" stroke-width="0.8"/>
    {''.join(f'<g stroke="#14161a" stroke-width="0.3"><path d="M{x} 200 v -6" stroke="#2a2e34" stroke-width="0.5"/><ellipse cx="{x}" cy="193" rx="2.4" ry="1.6" fill="#ff6fa8"/><path d="M{x + 1.6} 192 c 1 -3, 0 -4, 1 -5" fill="none" stroke="#ff6fa8" stroke-width="0.9"/></g>' for x in (40, 132))}
    <g stroke="#14161a" stroke-width="0.5"><path d="M84 200 h 18 v -6 l -3 -4 h -11 l -4 4 Z" fill="#8a9aac"/><circle cx="88" cy="200" r="2" fill="#1a1a1a"/><circle cx="98" cy="200" r="2" fill="#1a1a1a"/></g>
    <text x="110" y="156" font-family="Arial, sans-serif" font-size="3.6" font-weight="bold" text-anchor="middle" fill="#c9d4e0" opacity="0.8">HOMEOWNERS' ASSOCIATION: NO LIGHTS IN THE SKY</text>
    {ground()}''')

region('region_city_2', 'Downtown: towers, neon, a crosswalk (humans, elephants).', f'''
    {''.join(f'<rect x="{x}" y="{t}" width="{w}" height="{200 - t}" fill="url(#bldgR)" stroke="#0e1018" stroke-width="0.6"/>' + ''.join(f'<rect x="{x + 3 + i * 6}" y="{t + 5 + j * 8}" width="3" height="4" fill="#ffe08a"/>' for i in range(int((w - 4) / 6)) for j in range(int((190 - t) / 8)) if rnd.random() < 0.35) for x, w, t in ((0, 36, 70), (38, 30, 110), (70, 44, 50), (116, 30, 100), (148, 40, 80), (190, 30, 120)))}
    <rect x="0" y="190" width="220" height="10" fill="#2a2c30"/>
    <path d="{' '.join(f'M{x} 192 v 6' for x in range(96, 130, 4))}" stroke="#e8e8e8" stroke-width="2"/>
    <g stroke="#14161a" stroke-width="0.5"><rect x="150" y="183" width="14" height="9" fill="#d6b33a"/><path d="M148 183 h 18 l -2 -5 h -14 Z" fill="#c8241c"/></g>
    {ground('walk')}''',
       extra_parts='''  <g id="part-neon">
    <!-- Neon signs, flicker on and off -->
    <rect x="74" y="80" width="36" height="12" rx="2" fill="none" stroke="#ff4fa8" stroke-width="1.2"/>
    <text x="92" y="89" font-family="Arial Black, Arial, sans-serif" font-size="7" font-weight="bold" text-anchor="middle" fill="#ff8fc8">BAR</text>
    <text x="168" y="98" font-family="Arial Black, Arial, sans-serif" font-size="5" font-weight="bold" text-anchor="middle" fill="#7affd8">OPEN 24/7</text>
    <rect x="70" y="76" width="44" height="20" fill="#ff4fa8" opacity="0.12"/>
  </g>
''')


# ------------------------------------------------------------------ Earth's responses and conditions
def sprite(name, w, h, canvas_w, doc, parts):
    svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w * 4}" height="{h * 4}" data-canvas-width="{canvas_w}" data-densities="4 8">
  <!--
    {doc}
  -->
{DEFS}{parts}</svg>
'''
    (HERE / f'{name}.svg').write_text(svg, encoding='utf-8')


sprite('resp_patrol', 40, 30, 40, 'Night patrols: a farmer with a thermos and a flashlight. Feet at y 30. part-beam sweeps about the torch (12, 15).', '''  <g id="part-beam">
    <path d="M12 15 L 40 6 L 40 26 Z" fill="url(#cone)"/>
  </g>
  <g id="part-body" stroke="#14161a" stroke-width="0.5" stroke-linejoin="round">
    <path d="M6 30 V 22 M10 30 V 22" stroke="#3a4a6a" stroke-width="2.4"/>
    <rect x="4.6" y="12" width="7" height="11" rx="1.4" fill="#4a6a3a"/>
    <path d="M5 13 h 6 M5 16 h 6 M5 19 h 6" stroke="#2f4a26" stroke-width="0.4"/>
    <circle cx="8" cy="9" r="3" fill="#e8b996"/>
    <path d="M4.6 7.4 h 7 l -1 -3 h -5 Z" fill="#8a6a3e"/>
    <path d="M11 15 l 2 0" stroke="#4a6a3a" stroke-width="2"/>
    <rect x="11.5" y="13.6" width="3" height="2.8" rx="0.6" fill="#2a2e34"/>
    <rect x="1" y="18" width="3" height="6" rx="1" fill="#8a9aac"/>
    <path d="M1.4 18 h 2.2" stroke="#c8241c" stroke-width="0.8"/>
  </g>
''')
sprite('resp_investigators', 64, 36, 64, 'Investigators: a van and a corkboard of red string. Feet at y 36.', '''  <g id="part-body" stroke="#14161a" stroke-width="0.6" stroke-linejoin="round">
    <path d="M2 34 V 14 h 30 l 8 8 v 12 Z" fill="#e8e2d4"/>
    <rect x="32" y="16" width="6" height="6" fill="#9fd0ff"/>
    <rect x="6" y="18" width="20" height="8" fill="#c9c2b2"/>
    <text x="16" y="23.6" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#2a2e34" stroke="none">NOT THE FBI</text>
    <circle cx="10" cy="34" r="3" fill="#1a1a1a"/><circle cx="32" cy="34" r="3" fill="#1a1a1a"/>
    <path d="M6 14 l 4 -6 M10 8 l 8 0" stroke="#5a6068" stroke-width="0.8"/>
    <path d="M44 36 l 3 -14 M60 36 l -3 -14" stroke="#6a4020" stroke-width="1"/>
    <rect x="44" y="10" width="16" height="14" fill="#b98b5a"/>
    <g fill="#f4f1e6" stroke-width="0.3"><rect x="45.6" y="11.4" width="4" height="3"/><rect x="54" y="12" width="4" height="3"/><rect x="49" y="18" width="4" height="3"/><rect x="55" y="19" width="3.4" height="3"/></g>
    <circle cx="51" cy="14" r="1.2" fill="#7fc25b" stroke-width="0.3"/>
    <path d="M47.6 13 L 56 13.6 L 51 19.6 L 56.6 20.6 M47.6 13 L 51 19.6 M51 14 L 56 13.6" stroke="#d6332a" stroke-width="0.4" fill="none"/>
  </g>
''')
sprite('resp_radar', 30, 40, 30, 'Radar: a dish on a trailer. part-dish nods about (15, 18) to sweep the sky.', '''  <g id="part-body" stroke="#14161a" stroke-width="0.6">
    <rect x="3" y="30" width="24" height="6" fill="#5a6068"/>
    <circle cx="8" cy="37" r="2.6" fill="#1a1a1a"/><circle cx="22" cy="37" r="2.6" fill="#1a1a1a"/>
    <path d="M15 30 V 18" stroke="#8a9098" stroke-width="2"/>
    <rect x="10" y="26" width="10" height="4" fill="#3a4048"/>
    <circle cx="18" cy="28" r="0.7" fill="#4fff6a" stroke="none"/>
  </g>
  <g id="part-dish" stroke="#14161a" stroke-width="0.6">
    <path d="M3 10 C 6 20, 24 20, 27 10 Z" fill="#d8dde2"/>
    <path d="M15 17 L 15 6" stroke="#8a9098" stroke-width="0.8"/>
    <circle cx="15" cy="6" r="1.2" fill="#c8241c"/>
    <path d="M6 12 C 10 16, 20 16, 24 12" fill="none" stroke="#9aa1a8" stroke-width="0.4"/>
  </g>
''')
sprite('resp_support', 56, 26, 56, 'Support group: a circle of folding chairs. Feet at y 26.', '''  <g id="part-body" stroke="#14161a" stroke-width="0.5">
    <rect x="2" y="2" width="30" height="8" fill="#f4f1e6"/>
    <text x="17" y="5.4" font-family="Arial, sans-serif" font-size="2.4" font-weight="bold" text-anchor="middle" fill="#2a2e34" stroke="none">"I WAS TAKEN"</text>
    <text x="17" y="8.6" font-family="Arial, sans-serif" font-size="2.2" text-anchor="middle" fill="#2a2e34" stroke="none">support group - free coffee</text>
    <path d="M10 10 v 16" stroke="#6a4020" stroke-width="0.8"/>
    ''' + ''.join(f'<path d="M{x} 26 l 2 -6 h 4 l 1 6 M{x + 2} 20 v -5" fill="none" stroke="#8a9098" stroke-width="0.8"/>' for x in (14, 24, 34, 44)) + '''
    <g><circle cx="27" cy="13" r="2" fill="#e8b996"/><rect x="25" y="15" width="4" height="6" rx="1" fill="#7d5aa6"/></g>
    <g><circle cx="47" cy="13" r="2" fill="#c99a6a"/><rect x="45" y="15" width="4" height="6" rx="1" fill="#2b5a8a"/></g>
    <rect x="38" y="22" width="3" height="4" fill="#c9c2b2"/>
  </g>
''')
sprite('cond_crew', 36, 34, 36, 'Documentary craze: a camera crew. Feet at y 34.', '''  <g id="part-body" stroke="#14161a" stroke-width="0.5">
    <path d="M6 34 V 26 M10 34 V 26" stroke="#2a2e34" stroke-width="2.2"/>
    <rect x="4.4" y="16" width="7" height="11" rx="1.4" fill="#5a5c62"/>
    <circle cx="8" cy="13" r="3" fill="#c99a6a"/>
    <rect x="9" y="12" width="9" height="6" rx="1" fill="#1a1a1a"/><circle cx="18" cy="15" r="2" fill="#3a6a9a"/>
    <circle cx="11" cy="11" r="0.8" fill="#ff4a3a" stroke="none"/>
    <path d="M24 34 V 26 M28 34 V 26" stroke="#2a2e34" stroke-width="2.2"/>
    <rect x="22.4" y="16" width="7" height="11" rx="1.4" fill="#8a6a3e"/>
    <circle cx="26" cy="13" r="3" fill="#e8b996"/>
    <path d="M28 18 L 34 4" stroke="#5a6068" stroke-width="0.8"/>
    <ellipse cx="34" cy="4" rx="2" ry="1.4" fill="#8a8a8a"/>
    <rect x="12" y="28" width="8" height="5" fill="#1a1a1a"/><path d="M12 28 l 8 -2" stroke="#e8e8e8" stroke-width="0.8"/>
  </g>
''')
sprite('cond_agency', 64, 46, 64, 'Surveillance crackdown: an agency van and a camera on a pole. Feet at y 46. part-cam pans about (6, 8).', '''  <g id="part-body" stroke="#14161a" stroke-width="0.6">
    <path d="M6 46 V 8" stroke="#3a3e46" stroke-width="1.4"/>
    <path d="M18 44 V 26 h 34 l 8 8 v 10 Z" fill="#1a1c22"/>
    <rect x="52" y="28" width="6" height="5" fill="#3a5a7a"/>
    <text x="34" y="36" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#9aa1a8" stroke="none">DEFINITELY NOT</text>
    <text x="34" y="39.4" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#9aa1a8" stroke="none">SURVEILLANCE</text>
    <circle cx="26" cy="44" r="3" fill="#0a0a0a"/><circle cx="52" cy="44" r="3" fill="#0a0a0a"/>
    <path d="M40 26 V 20 M36 20 h 8" stroke="#5a6068" stroke-width="0.8"/>
  </g>
  <g id="part-cam" stroke="#14161a" stroke-width="0.5">
    <rect x="6" y="6" width="9" height="4" rx="0.6" fill="#d8dde2"/>
    <circle cx="15" cy="8" r="1.2" fill="#1a1a1a"/>
    <circle cx="8" cy="7" r="0.6" fill="#ff4a3a" stroke="none"/>
  </g>
''')
sprite('cond_billboard', 56, 46, 56, 'Everything is advertising: a billboard on posts. Feet at y 46.', '''  <g id="part-body" stroke="#14161a" stroke-width="0.6">
    <path d="M12 46 V 24 M44 46 V 24" stroke="#5a6068" stroke-width="1.6"/>
    <rect x="2" y="2" width="52" height="24" fill="#f3efe2"/>
    <rect x="4" y="4" width="20" height="20" fill="#2b5a8a"/>
    <ellipse cx="14" cy="10" rx="7" ry="2" fill="#a9b1ba"/><path d="M10 11 L 18 11 L 20 22 L 8 22 Z" fill="#fff3a0" opacity="0.6"/>
    <text x="39" y="11" font-family="Arial Black, Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#c8241c" stroke="none">ABDUCTED?</text>
    <text x="39" y="17" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">You may be owed</text>
    <text x="39" y="21" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">compensation!</text>
    <path d="M2 26 h 52" stroke="#8a9098" stroke-width="1"/>
  </g>
''')
print('ok')
