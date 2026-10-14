"""Oceanic location (Trench Station 9): ocean.svg backdrop, the three ocean regions, the airlock leak.
ocean.svg reuses the bunker from mountain.svg (rerun this after bunker changes) but swaps the storage
hall for Pressure Control.

    python -I ocean_art.py
"""
import math
import random
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
rnd = random.Random(5)
m = (HERE / 'mountain.svg').read_text(encoding='utf-8')
defs = m[m.index('  <defs>'):m.index('  </defs>') + len('  </defs>')]
b0 = m.index('    <!-- ===================== Bunker')
b1 = m.index('\n  </g>', b0)
bunker = m[b0:b1 + 1]

# Swap the storage hall for Pressure Control (same spot: 560..848 x 320..426).
s0 = bunker.index('    <g transform="translate(-112 23)">\n    <!-- Storage hall under the hangar -->')
# The storage group has nested groups, so find its matching </g> by counting nesting.
depth, i = 0, s0
while True:
    nxt_open = bunker.find('<g', i)
    nxt_close = bunker.find('</g>', i)
    if nxt_open != -1 and nxt_open < nxt_close:
        depth += 1
        i = nxt_open + 2
    else:
        depth -= 1
        i = nxt_close + 4
        if depth == 0:
            break
s1 = bunker.index('\n', i) + 1
pressure_room = '''    <!-- Pressure Control (replaces the storage hall under the sea) -->
    <rect x="560" y="320" width="288" height="106" fill="url(#wallH)"/>
    <rect x="560" y="320" width="288" height="106" filter="url(#concrete)" opacity="0.5"/>
    <rect x="560" y="320.5" width="288" height="3" fill="url(#pipeH)"/>
    <rect x="560" y="421.4" width="288" height="4.6" fill="url(#grate)"/>
    <text x="704" y="338" font-family="Arial Black, Arial, sans-serif" font-size="8" font-weight="bold" text-anchor="middle" fill="#c9ced6" opacity="0.18" letter-spacing="1">PRESSURE CONTROL</text>
    <!-- Console with dials and a big red valve -->
    <g stroke="#14161a" stroke-width="0.6">
      <path d="M600 421 V 392 L 610 384 H 720 L 730 392 V 421 Z" fill="#4a5058"/>
      <rect x="612" y="388" width="106" height="10" fill="#2a2e34"/>
      ''' + ''.join(f'<circle cx="{x}" cy="393" r="3.6" fill="#f0ebdc"/><path d="M{x} 393 l {rnd.uniform(-3, 3):.1f} {rnd.uniform(-3, -1):.1f}" stroke="#c8241c" stroke-width="0.6"/>' for x in range(620, 715, 12)) + '''
      <g fill="#4fff6a" stroke="none">''' + ''.join(f'<circle cx="{x}" cy="404" r="0.9"/>' for x in range(616, 714, 7)) + '''</g>
      <circle cx="745" cy="404" r="9" fill="none" stroke="#c8241c" stroke-width="2.4"/>
      <path d="M736 404 h 18 M745 395 v 18" stroke="#c8241c" stroke-width="1.6"/>
      <path d="M745 413 V 421" stroke="#5a6068" stroke-width="2"/>
    </g>
    <!-- Periscope, pointed at the fish -->
    <path d="M790 320 V 380 h 8" fill="none" stroke="#5a6068" stroke-width="4"/>
    <rect x="796" y="376" width="10" height="8" fill="#2a2e34"/>
    <!-- The big hull gauge (its needle is a separate part) -->
    <circle cx="780" cy="360" r="23" fill="#2a2e34" stroke="#14161a" stroke-width="0.8"/>
    <circle cx="780" cy="360" r="20" fill="#f0ebdc" stroke="#14161a" stroke-width="0.8"/>
    <path d="M764.7 372.9 A 20 20 0 0 1 765.9 345.9" fill="none" stroke="#3a9a4a" stroke-width="3"/>
    <path d="M765.9 345.9 A 20 20 0 0 1 794.1 345.9" fill="none" stroke="#e2b923" stroke-width="3"/>
    <path d="M794.1 345.9 A 20 20 0 0 1 795.3 372.9" fill="none" stroke="#c8241c" stroke-width="3"/>
    <text x="780" y="374" font-family="Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#14161a">HULL</text>
    <!-- Motivational poster -->
    <rect x="574" y="344" width="22" height="30" fill="#1e3a5a" stroke="#14161a" stroke-width="0.6"/>
    <path d="M578 366 c 4 -6, 10 -6, 14 0" fill="none" stroke="#9fd0ff" stroke-width="1"/>
    <text x="585" y="352" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#e8f2ff">KEEP CALM</text>
    <text x="585" y="356" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#e8f2ff">AND</text>
    <text x="585" y="360" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#e8f2ff">EQUALIZE</text>
    <!-- Lamps -->
    <g fill="#2a2e34" stroke="#14161a" stroke-width="0.3"><path d="M626 326 h 8 l -1 2 h -6 Z"/><path d="M716 326 h 8 l -1 2 h -6 Z"/></g>
'''
bunker = bunker[:s0] + pressure_room + bunker[s1:]

ocean_defs = defs.replace('  </defs>', '''    <linearGradient id="sea" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#2f7aa8"/><stop offset="0.45" stop-color="#174a72"/><stop offset="1" stop-color="#081a30"/>
    </linearGradient>
    <linearGradient id="ray" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#bff0ff" stop-opacity="0.22"/><stop offset="1" stop-color="#bff0ff" stop-opacity="0"/>
    </linearGradient>
    <linearGradient id="sand" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#8a7a5a"/><stop offset="1" stop-color="#4a4030"/>
    </linearGradient>
    <radialGradient id="domeG" cx="0.4" cy="0.3" r="0.8">
      <stop offset="0" stop-color="#d8f6ff" stop-opacity="0.35"/><stop offset="1" stop-color="#6fb8d8" stop-opacity="0.2"/>
    </radialGradient>
    <linearGradient id="tankG" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0" stop-color="#4a5058"/><stop offset="0.4" stop-color="#9aa1a8"/><stop offset="1" stop-color="#3a3e46"/>
    </linearGradient>
  </defs>''')

fish = ''.join(f'<path d="M{x} {y} l 4 -2 v 4 Z M{x + 4} {y} c 2 -2, 5 -2, 6 0 c -1 2, -4 2, -6 0 Z" fill="#9fd0e8" opacity="0.6"/>'
               for x, y in [(rnd.uniform(260, 1680), rnd.uniform(-240, 200)) for _ in range(40)])
kelp = ''.join(f'<path d="M{x} 262 C {x - 6} 240, {x + 6} 220, {x} {262 - h}" fill="none" stroke="#2f6a3a" stroke-width="2.4"/>'
               for x, h in [(rnd.uniform(260, 700), rnd.uniform(30, 70)) for _ in range(10)] + [(rnd.uniform(1180, 1680), rnd.uniform(30, 70)) for _ in range(10)])
rays = ''.join(f'<path d="M{x} -270 L {x + 60} 260 L {x + 90} 260 L {x + 20} -270 Z" fill="url(#ray)"/>' for x in range(300, 1700, 160))
bubbles = ''.join(f'<circle cx="{rnd.uniform(260, 1690):.0f}" cy="{rnd.uniform(-250, 250):.0f}" r="{rnd.uniform(0.6, 2):.1f}" fill="none" stroke="#bff0ff" stroke-width="0.5" opacity="0.6"/>' for _ in range(60))

ocean_svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="240 -300 1460 1160" width="2190" height="1740" data-canvas-width="1460" data-densities="1.5 2.5">
  <!--
    Trench Station 9 (Oceanic): drawn instead of the mountain, same frame and base-local coordinates.
    A pressurised glass dome on the sea floor holds the hangar (780..1080 x 110..210); the rest of the
    base is the same bunker below the sea bed, with Pressure Control where the storage hall was.
      part-back         sea, light rays, fish, kelp, sea floor, dome, bunker
      part-turbine_a/b  tidal turbine rotors, hubs at (1460, 196) and (1580, 214) (spin with the tide)
      part-tanks        Deep Tanks: big pressurised water tanks on the sea bed
      part-cracks       hairline cracks in the dome (hull pressure 60+)
      part-needle       the hull gauge needle in Pressure Control, pivot (780, 360)
  -->
{ocean_defs}
  <g id="part-back">
    <rect x="240" y="-300" width="1460" height="1160" fill="url(#sea)"/>
    <path d="M240 -276 {' '.join(f'Q {x + 15} -282, {x + 30} -276' for x in range(240, 1700, 30))}" fill="none" stroke="#bff0ff" stroke-width="1.2" opacity="0.6"/>
    {rays}
    <g>{fish}</g>
    {bubbles}
    <!-- Sea floor -->
    <path d="M240 300 L 240 270 C 400 256, 600 268, 760 262 C 900 256, 1000 262, 1150 258 C 1350 252, 1500 266, 1700 260 L 1700 300 Z" fill="url(#sand)"/>
    <rect x="240" y="296" width="1460" height="564" fill="#2a2420"/>
    <rect x="240" y="296" width="1460" height="564" filter="url(#rockTex)" opacity="0.6"/>
    <g>{kelp}</g>
    <g fill="#c86a8a" stroke="#14161a" stroke-width="0.4">{''.join(f'<path d="M{x} 262 c -3 -8, -1 -14, 2 -16 c 1 4, 3 4, 4 0 c 3 3, 4 8, 0 16 Z"/>' for x in (300, 352, 610, 1240, 1650))}</g>
    <!-- Tidal turbine towers (rotors are parts) -->
    <path d="M1460 260 V 196 M1580 262 V 214" stroke="#5a6068" stroke-width="3"/>
    <circle cx="1460" cy="196" r="4" fill="#3a3e46" stroke="#14161a" stroke-width="0.6"/>
    <circle cx="1580" cy="214" r="4" fill="#3a3e46" stroke="#14161a" stroke-width="0.6"/>
    <path d="M1460 260 C 1400 280, 1300 290, 1296 300" fill="none" stroke="#c9a224" stroke-width="1.2"/>
    <!-- The dome: a pressurised bubble around the hangar, with an airlock on the left -->
    <path d="M708 236 C 708 120, 800 58, 930 58 C 1060 58, 1152 120, 1152 236 Z" fill="url(#domeG)" stroke="#9fd0e8" stroke-width="1.6"/>
    <path d="M760 236 C 770 150, 840 90, 930 82 M1100 236 C 1090 150, 1020 90, 930 82 M930 58 V 236" fill="none" stroke="#9fd0e8" stroke-width="0.8" opacity="0.6"/>
    <path d="M740 140 C 780 100, 840 74, 900 66" fill="none" stroke="#ffffff" stroke-width="1.2" opacity="0.5"/>
    <rect x="700" y="228" width="460" height="14" fill="#4d5058" stroke="#14161a" stroke-width="0.8"/>
    <rect x="700" y="228" width="460" height="3" fill="url(#hazardM)"/>
    <rect x="712" y="152" width="22" height="58" rx="3" fill="#3a3e46" stroke="#14161a" stroke-width="0.8"/>
    <circle cx="723" cy="181" r="7" fill="#9fd0e8" stroke="#14161a" stroke-width="0.6" opacity="0.8"/>
    <text x="723" y="150" font-family="Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#e2b923">AIRLOCK</text>
    <text x="930" y="52" font-family="Arial Black, Arial, sans-serif" font-size="8" font-weight="bold" text-anchor="middle" fill="#e8f2ff">TRENCH STATION 9</text>
    <text x="930" y="44" font-family="Arial, sans-serif" font-size="4" text-anchor="middle" fill="#9fd0e8">a perfectly normal research reef</text>

{bunker}  </g>
  <g id="part-turbine_a" stroke="#14161a" stroke-width="0.6">
    {''.join(f'<path d="M1460 196 L {1460 + 26 * math.cos(a * 2.094):.1f} {196 + 26 * math.sin(a * 2.094):.1f} L {1460 + 24 * math.cos(a * 2.094 + 0.2):.1f} {196 + 24 * math.sin(a * 2.094 + 0.2):.1f} Z" fill="#d8dde2"/>' for a in range(3))}
    <circle cx="1460" cy="196" r="3" fill="#e2b923"/>
  </g>
  <g id="part-turbine_b" stroke="#14161a" stroke-width="0.6">
    {''.join(f'<path d="M1580 214 L {1580 + 22 * math.cos(a * 2.094 + 1):.1f} {214 + 22 * math.sin(a * 2.094 + 1):.1f} L {1580 + 20 * math.cos(a * 2.094 + 1.2):.1f} {214 + 20 * math.sin(a * 2.094 + 1.2):.1f} Z" fill="#d8dde2"/>' for a in range(3))}
    <circle cx="1580" cy="214" r="3" fill="#e2b923"/>
  </g>
  <g id="part-tanks" stroke="#14161a" stroke-width="0.7">
    {''.join(f'<rect x="{x}" y="186" width="40" height="72" rx="10" fill="url(#tankG)"/><rect x="{x + 14}" y="198" width="12" height="48" rx="3" fill="#0e2a44"/><rect x="{x + 15}" y="{198 + lvl}" width="10" height="{47 - lvl}" fill="#2f8ac8"/><path d="M{x + 20} 258 V 296" stroke="#5a6068" stroke-width="3"/>' for x, lvl in ((1180, 10), (1232, 22), (1284, 6)))}
    <text x="1252" y="182" font-family="Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#e2b923" stroke="none">DEEP TANKS (do not tap)</text>
  </g>
  <g id="part-cracks" fill="none" stroke="#e8fbff" stroke-width="0.9">
    <path d="M820 96 l 10 14 l -6 10 l 12 16 M1040 100 l -8 16 l 8 8 l -6 18 M930 64 l 4 18 l -6 12"/>
  </g>
  <g id="part-needle">
    <path d="M780 360 L 780 342" stroke="#14161a" stroke-width="1.4" stroke-linecap="round"/>
    <circle cx="780" cy="360" r="2" fill="#14161a"/>
  </g>
</svg>
'''
(HERE / 'ocean.svg').write_text(ocean_svg, encoding='utf-8')

# ------------------------------------------------------------------ regions
DEFS = (HERE / 'region_mountain_0.svg').read_text(encoding='utf-8')
rdefs = DEFS[DEFS.index('  <defs>'):DEFS.index('  </defs>') + len('  </defs>')]


def region(name, doc, back, extra=''):
    (HERE / f'{name}.svg').write_text(f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 220 210" width="440" height="420" data-canvas-width="220" data-densities="2 4">
  <!--
    {doc}
    Hunting region backdrop, drawn at canvas (x0, 100), 220x210; ground (or sea floor) at local y 200.
  -->
{rdefs}
  <g id="part-back">
{back}
  </g>
{extra}</svg>
''', encoding='utf-8')


region('region_ocean_0', 'Coral Shallows: a reef on the sea floor (pufferfish, octopus). The game tints the water above.', f'''
    <path d="M0 196 C 40 188, 90 194, 130 190 C 170 186, 200 192, 220 188 L 220 210 L 0 210 Z" fill="#c8b48a"/>
    {''.join(f'<path d="M{x} 198 c -4 -12, -1 -22, 3 -26 c 1 6, 4 6, 5 0 c 4 5, 6 14, 0 26 Z" fill="{c}" stroke="#14161a" stroke-width="0.5"/>' for x, c in ((14, '#e86a8a'), (52, '#ffa04a'), (96, '#c86ae8'), (150, '#e86a8a'), (196, '#4ad8c8')))}
    {''.join(f'<path d="M{x} 198 C {x - 4} 180, {x + 4} 170, {x} {150 + (x % 30)}" fill="none" stroke="#3a8a4a" stroke-width="2"/>' for x in (30, 78, 122, 176))}
    {''.join(f'<circle cx="{x}" cy="{y}" r="1.4" fill="none" stroke="#bff0ff" stroke-width="0.5"/>' for x, y in ((40, 150), (44, 140), (130, 160), (133, 148), (190, 120)))}
    <g stroke="#14161a" stroke-width="0.5"><rect x="110" y="182" width="16" height="14" rx="2" fill="#8a7a5a"/><path d="M110 186 h 16" stroke="#5a4a30"/><circle cx="118" cy="190" r="1.4" fill="#e2b923"/></g>
    <text x="118" y="179" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#e8f2ff">SUNKEN CHEST (empty, sorry)</text>
''')
region('region_ocean_1', 'Harbour Town: fish market, lighthouse, yacht club, on land at the surface (squirrels, cows, humans).', f'''
    <rect x="0" y="176" width="70" height="34" fill="#1f4a72"/>
    <path d="M0 178 {' '.join(f'Q {x + 5} 175, {x + 10} 178' for x in range(0, 70, 10))}" fill="none" stroke="#9fd0e8" stroke-width="0.8"/>
    <g stroke="#14161a" stroke-width="0.6">
      <path d="M8 178 l 6 -12 v 12 Z M18 178 l -2 -10 l 10 10 Z" fill="#f4f1e6"/><path d="M4 178 h 26 l -3 5 h -20 Z" fill="#e8e2d4"/>
      <path d="M38 178 l 5 -10 v 10 Z" fill="#f4f1e6"/><path d="M34 178 h 18 l -2 4 h -14 Z" fill="#c8241c"/>
      <path d="M60 200 V 176 h 12 v 24" fill="#6a4020"/>
    </g>
    <path d="M70 200 L 74 196 L 220 196 L 220 200 Z" fill="#5a4a3a"/>
    <g stroke="#14161a" stroke-width="0.6">
      <path d="M86 200 L 90 120 L 102 120 L 106 200 Z" fill="#f4f1e6"/>
      <path d="M88 160 h 16 M89 140 h 14" stroke="#c8241c" stroke-width="5"/>
      <rect x="88" y="110" width="16" height="10" fill="#e9e1c6"/><path d="M86 110 L 96 100 L 106 110 Z" fill="#c8241c"/>
      <rect x="112" y="178" width="44" height="22" fill="#9ab8c8"/><path d="M110 178 L 134 166 L 158 178 Z" fill="#2b5a8a"/>
      <rect x="164" y="176" width="50" height="24" fill="#e9e1c6"/>
      ''' + ''.join(f'<path d="M{x} 176 h 8" stroke="{c}" stroke-width="4"/>' for x, c in ((164, '#c8241c'), (172, '#f4f1e6'), (180, '#c8241c'), (188, '#f4f1e6'), (196, '#c8241c'), (204, '#f4f1e6'))) + '''
    </g>
    <text x="134" y="190" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#14161a">YACHT CLUB</text>
    <text x="189" y="188" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#14161a">FISH MARKET</text>
    <text x="189" y="194" font-family="Arial, sans-serif" font-size="2.4" text-anchor="middle" fill="#8a2a24">no questions asked</text>
    <rect x="0" y="200" width="220" height="10" fill="#8a8c92"/>''',
       extra='''  <g id="part-beam">
    <!-- Lighthouse beam, sweeps about (96, 115) -->
    <path d="M96 115 L 220 100 L 220 130 Z" fill="#fff6c4" opacity="0.35"/>
    <circle cx="96" cy="115" r="3" fill="#fff6c4"/>
  </g>
''')
region('region_ocean_2', 'The Trench: dark cliffs, glowing things, no witnesses (octopus, whales). The game tints the water above.', f'''
    <path d="M0 120 L 30 130 L 40 200 L 0 210 Z M220 110 L 186 124 L 176 200 L 220 210 Z" fill="#0e1622"/>
    <path d="M40 200 C 80 196, 140 198, 176 200 L 176 210 L 40 210 Z" fill="#1a2230"/>
    <path d="M30 130 L 40 200 M186 124 L 176 200" stroke="#2a3a4a" stroke-width="1"/>
    {''.join(f'<circle cx="{rnd.uniform(40, 176):.0f}" cy="{rnd.uniform(110, 196):.0f}" r="{rnd.uniform(0.5, 1.4):.1f}" fill="#7affd8" opacity="0.7"/>' for _ in range(24))}
    <g stroke="#14161a" stroke-width="0.5">
      <ellipse cx="120" cy="160" rx="10" ry="7" fill="#2a2a3a"/><path d="M112 162 l -4 4 l 5 0 Z" fill="#e8e8e8"/>
      <path d="M124 154 C 128 144, 136 144, 136 150" fill="none" stroke="#2a2a3a" stroke-width="1"/>
    </g>
    <text x="108" y="206" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#3a5a7a">depth: yes</text>''',
       extra='''  <g id="part-lure">
    <circle cx="136" cy="150" r="2" fill="#fff6a0"/><circle cx="136" cy="150" r="7" fill="#fff6a0" opacity="0.25"/>
  </g>
''')

# ------------------------------------------------------------------ airlock leak
(HERE / 'leak.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 60" width="160" height="240" data-canvas-width="40" data-densities="4 8">
  <!--
    The airlock is sealed and the hangar ceiling leaks into a bucket. Top of the frame at the ceiling.
      part-body   drip line, bucket, puddle, motivational poster
      part-drop   a falling drop (the game moves it)
  -->
  <g id="part-body" stroke="#14161a" stroke-width="0.5">
    <path d="M10 0 v 4" stroke="#9fd0e8" stroke-width="1.2"/>
    <path d="M4 48 h 12 l -1.4 10 h -9.2 Z" fill="#8a9aac"/>
    <ellipse cx="10" cy="48" rx="6" ry="1.2" fill="#2f8ac8"/>
    <ellipse cx="12" cy="59" rx="9" ry="1" fill="#2f8ac8" opacity="0.6"/>
    <rect x="20" y="10" width="18" height="24" fill="#f3efe2"/>
    <path d="M24 26 c 2 -6, 8 -6, 10 0" fill="none" stroke="#2b5a8a" stroke-width="1"/>
    <text x="29" y="16" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">EVERY DROP</text>
    <text x="29" y="19.6" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">IS A LEARNING</text>
    <text x="29" y="23.2" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">OPPORTUNITY</text>
  </g>
  <g id="part-drop" stroke="none">
    <path d="M10 4 c -1.4 2, -1.4 4, 0 4 c 1.4 0, 1.4 -2, 0 -4 Z" fill="#9fd0e8"/>
  </g>
</svg>
''', encoding='utf-8')
print('ok')
