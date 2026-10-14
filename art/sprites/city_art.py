"""Builds city.svg (Metropolis): skyline, the disguised HQ tower, a street, and the same bunker underground."""
import random

DIR = 'F:/Alien_Idle/art/sprites/'
m = open(DIR + 'mountain.svg', encoding='utf-8').read()
defs = m[m.index('  <defs>'):m.index('  </defs>') + len('  </defs>')]
b0 = m.index('    <!-- ===================== Bunker')
b1 = m.index('\n  </g>', b0)
bunker = m[b0:b1 + 1]

rnd = random.Random(7)


def windows(x0, y0, x1, y1, dx, dy, w, h, lit=0.35, colors=('#ffe08a', '#fff4c4', '#9fd0ff')):
    out = []
    y = y0
    while y + h <= y1:
        x = x0
        while x + w <= x1:
            if rnd.random() < lit:
                out.append(f'<rect x="{x:.1f}" y="{y:.1f}" width="{w}" height="{h}" fill="{rnd.choice(colors)}"/>')
            x += dx
        y += dy
    return ''.join(out)


far = []
x = 240
while x < 1700:
    w = rnd.randint(30, 70)
    top = rnd.randint(-120, 160)
    far.append(f'<rect x="{x}" y="{top}" width="{w}" height="{300 - top}" fill="#1c2236"/>')
    far.append(f'<g opacity="0.55">{windows(x + 4, top + 6, x + w - 3, 296, 7, 9, 3, 4, 0.22)}</g>')
    if rnd.random() < 0.3:
        far.append(f'<path d="M{x + w / 2} {top} v -14" stroke="#1c2236" stroke-width="1.4"/><circle cx="{x + w / 2}" cy="{top - 14}" r="1" fill="#ff4a3a"/>')
    x += w + rnd.randint(2, 10)

near = []
for x, w, top in [(250, 90, 120), (350, 70, 60), (430, 80, 150), (520, 60, 20), (590, 110, 170), (680, 70, 90),
                  (1140, 80, 40), (1230, 60, 140), (1300, 100, 70), (1410, 70, 180), (1490, 90, 30), (1590, 110, 130)]:
    near.append(f'<rect x="{x}" y="{top}" width="{w}" height="{300 - top}" fill="url(#bldg)" stroke="#0e1018" stroke-width="0.8"/>')
    near.append(windows(x + 5, top + 8, x + w - 4, 290, 9, 11, 4, 6, 0.4))
    near.append(f'<rect x="{x - 2}" y="{top - 3}" width="{w + 4}" height="3" fill="#2a3048"/>')

city_defs = defs.replace('  </defs>', '''    <linearGradient id="bldg" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0" stop-color="#2a3048"/><stop offset="0.5" stop-color="#343b58"/><stop offset="1" stop-color="#232838"/>
    </linearGradient>
    <linearGradient id="glassT" x1="0" y1="0" x2="1" y2="0">
      <stop offset="0" stop-color="#2c4a66"/><stop offset="0.45" stop-color="#4f7aa0"/><stop offset="1" stop-color="#22384e"/>
    </linearGradient>
    <linearGradient id="earth" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#4a4038"/><stop offset="1" stop-color="#2a2420"/>
    </linearGradient>
    <linearGradient id="asphalt" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#3a3c42"/><stop offset="1" stop-color="#2a2c30"/>
    </linearGradient>
    <radialGradient id="streetGlow" cx="0.5" cy="0" r="1">
      <stop offset="0" stop-color="#ffd27a" stop-opacity="0.45"/><stop offset="1" stop-color="#ffd27a" stop-opacity="0"/>
    </radialGradient>
  </defs>''')

tower_windows = windows(1084, -226, 1118, 106, 8, 10, 5, 6, 0.55) + windows(786, -226, 1080, 104, 10, 10, 6, 6, 0.5)

svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="240 -300 1460 1160" width="2190" height="1740" data-canvas-width="1460" data-densities="1.5 2.5">
  <!--
    Metropolis (second location): drawn instead of the mountain, same frame and BASE-LOCAL coordinates
    as mountain.svg (drawn at canvas (640, -300) with the +400 base shift).
    Above ground: a night skyline, and Metropolis Tower, the HQ, disguised as an insurance company.
    The hangar (780..1080 x 110..210) is a floor of the tower with its bay open to the left.
    Below ground: bedrock, a subway line, and the same bunker as under the mountain (copied from
    mountain.svg by city_art.py, so regenerate this file after changing the bunker).
      part-back   everything (hallway effects come from hallfx.svg)
  -->
{city_defs}
  <g id="part-back">
    <!-- Far skyline -->
    {''.join(far)}
    <!-- Near buildings -->
    {''.join(near)}

    <!-- Metropolis Tower: the HQ -->
    <rect x="776" y="-240" width="348" height="540" fill="url(#glassT)" stroke="#0e1018" stroke-width="1.2"/>
    <g fill="#3f6688" opacity="0.6">{''.join(f'<rect x="776" y="{y}" width="348" height="1.2"/>' for y in range(-230, 296, 10))}</g>
    <g opacity="0.8">{tower_windows}</g>
    <path d="M776 -240 h 348 v 6 h -348 Z" fill="#1e2c3c"/>
    <!-- Crown, antenna and the sign -->
    <path d="M800 -240 L 820 -262 L 1080 -262 L 1100 -240 Z" fill="#2a3a4e" stroke="#0e1018" stroke-width="0.8"/>
    <path d="M950 -262 V -296" stroke="#9aa1a8" stroke-width="2"/>
    <circle cx="950" cy="-297" r="2" fill="#ff4a3a"/>
    <rect x="840" y="-232" width="220" height="26" fill="#0e1624" stroke="#9fd0ff" stroke-width="0.8"/>
    <text x="950" y="-214" font-family="Arial Black, Arial, sans-serif" font-size="13" font-weight="bold" text-anchor="middle" fill="#e8f2ff">NORMAL INSURANCE CO.</text>
    <text x="950" y="-200" font-family="Arial, sans-serif" font-size="5" font-style="italic" text-anchor="middle" fill="#9fd0ff">staffed entirely by humans since 1947</text>
    <!-- Hangar floor: the bay opens left over a landing ledge -->
    <rect x="740" y="213" width="40" height="5" fill="#5a6068" stroke="#14161a" stroke-width="0.5"/>
    <rect x="740" y="213" width="40" height="1.6" fill="url(#hazardM)"/>
    <path d="M744 218 l 4 8 M776 218 l -4 8" stroke="#5a6068" stroke-width="1.4"/>
    <text x="760" y="234" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#e2b923">WINDOW CLEANING</text>
    <text x="760" y="238" font-family="Arial, sans-serif" font-size="3" text-anchor="middle" fill="#c9ced6">platform (do not ask)</text>
    <!-- Lobby -->
    <rect x="776" y="250" width="348" height="50" fill="#1a2430"/>
    <rect x="790" y="258" width="320" height="38" fill="#a8d4f0" opacity="0.35"/>
    <path d="M830 258 v 38 M870 258 v 38 M990 258 v 38 M1030 258 v 38 M1070 258 v 38" stroke="#14161a" stroke-width="1.2"/>
    <circle cx="950" cy="284" r="12" fill="#2a3a4e" stroke="#9fd0ff" stroke-width="0.8"/>
    <path d="M938 284 h 24 M950 272 v 24" stroke="#9fd0ff" stroke-width="0.8"/>
    <text x="950" y="254" font-family="Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#e8f2ff">LOBBY - VISITORS PLEASE DO NOT</text>
    <!-- Corporate art: a bronze cow being abducted -->
    <path d="M880 296 h 16 v -4 h -16 Z" fill="#5a4a30"/>
    <path d="M882 292 l 2 -10 h 8 l 2 10 Z" fill="#c9a224" opacity="0.4"/>
    <rect x="884" y="284" width="8" height="4" rx="1.6" fill="#a8783a"/>

    <!-- Street level -->
    <rect x="240" y="296" width="1460" height="4" fill="#5a5c62"/>
    <rect x="240" y="300" width="1460" height="10" fill="url(#asphalt)"/>
    <path d="{' '.join(f'M{x} 305 h 10' for x in range(250, 1700, 24))}" stroke="#e2d58a" stroke-width="0.8"/>
    <g>
      {''.join(f'<path d="M{x} 296 v -26 h 6" fill="none" stroke="#2a2e34" stroke-width="1.2"/><rect x="{x + 4}" y="268" width="5" height="2" fill="#fff1c2"/><path d="M{x + 6} 270 L {x - 12} 296 L {x + 24} 296 Z" fill="url(#streetGlow)"/>' for x in (300, 480, 660, 1180, 1360, 1560))}
    </g>
    <!-- Parked cars, a hot dog cart, a hydrant -->
    <g stroke="#14161a" stroke-width="0.5">
      <path d="M380 296 h 24 v -5 l -4 -4 h -12 l -5 4 h -3 Z" fill="#8a2a24"/><circle cx="385" cy="296" r="2.4" fill="#1a1a1a"/><circle cx="399" cy="296" r="2.4" fill="#1a1a1a"/>
      <path d="M1240 296 h 26 v -6 l -5 -4 h -12 l -5 4 h -4 Z" fill="#2a5a8a"/><circle cx="1246" cy="296" r="2.4" fill="#1a1a1a"/><circle cx="1260" cy="296" r="2.4" fill="#1a1a1a"/>
      <rect x="1140" y="284" width="14" height="9" fill="#d6b33a"/><path d="M1138 284 h 18 l -2 -5 h -14 Z" fill="#c8241c"/><circle cx="1143" cy="295" r="1.6" fill="#1a1a1a"/><circle cx="1151" cy="295" r="1.6" fill="#1a1a1a"/>
      <text x="1147" y="290" font-family="Arial, sans-serif" font-size="2.2" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">HOT DOGS</text>
      <rect x="560" y="289" width="3" height="7" rx="1" fill="#c8241c"/>
    </g>
    <!-- A billboard with a message from management -->
    <rect x="1312" y="40" width="80" height="26" fill="#f3efe2" stroke="#14161a" stroke-width="0.8"/>
    <text x="1352" y="51" font-family="Arial Black, Arial, sans-serif" font-size="6" font-weight="bold" text-anchor="middle" fill="#14161a">NOTHING IS</text>
    <text x="1352" y="60" font-family="Arial Black, Arial, sans-serif" font-size="6" font-weight="bold" text-anchor="middle" fill="#c8241c">HAPPENING</text>

    <!-- Underground: bedrock, utility lines, a subway -->
    <rect x="240" y="310" width="1460" height="550" fill="url(#earth)"/>
    <rect x="240" y="310" width="1460" height="550" filter="url(#rockTex)" opacity="0.6"/>
    <path d="M240 318 H 1700" stroke="#3b6bd1" stroke-width="2"/>
    <path d="M240 324 H 1700" stroke="#5a6068" stroke-width="3"/>
    <path d="M240 330 H 1700" stroke="#c9a224" stroke-width="1"/>
    <!-- Subway line on the far right, train waiting forever -->
    <rect x="1470" y="600" width="230" height="44" fill="#22252a" stroke="#14161a" stroke-width="0.8"/>
    <path d="M1470 640 H 1700" stroke="#8a9098" stroke-width="1.2"/>
    <rect x="1490" y="612" width="120" height="24" rx="3" fill="#b8c0c8" stroke="#14161a" stroke-width="0.6"/>
    <g fill="#9fd0ff">{''.join(f'<rect x="{x}" y="616" width="10" height="8"/>' for x in range(1496, 1600, 16))}</g>
    <text x="1550" y="633" font-family="Arial, sans-serif" font-size="4" font-weight="bold" text-anchor="middle" fill="#c8241c">LINE 7 - NOT IN SERVICE</text>
    <!-- Sewer on the far left -->
    <circle cx="350" cy="560" r="20" fill="#22252a" stroke="#5a6068" stroke-width="2"/>
    <path d="M330 566 h 40" stroke="#4f8a5a" stroke-width="3"/>
    <text x="350" y="592" font-family="Arial, sans-serif" font-size="4" text-anchor="middle" fill="#8a9098">do not feed the alligator</text>

{bunker}  </g>
</svg>
'''
open(DIR + 'city.svg', 'w', encoding='utf-8', newline='').write(svg)
print('ok')
