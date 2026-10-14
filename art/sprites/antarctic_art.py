"""Antarctic location (Station Permafrost): ice.svg backdrop (bunker copied from mountain.svg with the
storage hall swapped for the Boiler Room), three regions, polar specimens, frost/melt overlays, cryo pods,
and the boiler stoker.

    python -I antarctic_art.py
"""
import math
import random
from pathlib import Path

HERE = Path(__file__).resolve().parent
rnd = random.Random(23)

# ------------------------------------------------------------------ specimens
(HERE / 'penguin.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 20 26" width="160" height="208" data-canvas-width="14" data-densities="4 8">
  <!--
    Penguin, facing left, feet on y 25.6. Drawn 14x18.2 canvas units; the game waddles it.
      part-body
      part-flipper  flaps about the shoulder (11, 11)
  -->
  <g id="part-body" stroke="#0e1018" stroke-width="0.6" stroke-linejoin="round">
    <path d="M10 2 C 15 2, 17 8, 17 15 C 17 22, 14 25, 10 25 C 6 25, 3 22, 3 15 C 3 8, 5 2, 10 2 Z" fill="#1c2030"/>
    <path d="M10 8 C 13 8, 14 13, 14 17 C 14 22, 12 24, 10 24 C 7 24, 6 21, 6 17 C 6 12, 7 8, 10 8 Z" fill="#f4f6f8" stroke="none"/>
    <path d="M3 7 L 0.4 8.4 L 3.4 9.4 Z" fill="#e8a02a"/>
    <circle cx="6" cy="6" r="1.4" fill="#ffffff" stroke-width="0.3"/><circle cx="5.6" cy="6" r="0.6" fill="#111" stroke="none"/>
    <path d="M7 11 C 8.6 10, 11 10, 12.4 11" fill="none" stroke="#e8c02a" stroke-width="0.8"/>
    <path d="M6 25.4 h 3 M11 25.4 h 3" stroke="#e8a02a" stroke-width="1.2"/>
  </g>
  <g id="part-flipper" stroke="#0e1018" stroke-width="0.5">
    <path d="M12 11 C 15 13, 16 17, 15 20 C 13 18, 12 15, 12 11 Z" fill="#1c2030"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'seal.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 20" width="320" height="160" data-canvas-width="32" data-densities="4 8">
  <!--
    Seal, facing left, belly on y 19.6. Drawn 32x16 canvas units.
      part-body
      part-tail  rear flippers, wave about (34, 15)
  -->
  <defs><linearGradient id="sl" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#9aa6b2"/><stop offset="1" stop-color="#5a6672"/></linearGradient></defs>
  <g id="part-tail" stroke="#1c2430" stroke-width="0.6" stroke-linejoin="round">
    <path d="M33 15 L 39 11 L 38 15 L 39 19 Z" fill="#5a6672"/>
  </g>
  <g id="part-body" stroke="#1c2430" stroke-width="0.7" stroke-linejoin="round">
    <path d="M4 12 C 4 6, 10 4, 16 6 C 24 8, 30 11, 34 15 C 30 19, 18 20, 10 19.4 C 5 19, 4 16, 4 12 Z" fill="url(#sl)"/>
    <g fill="#4a5662" stroke="none"><circle cx="18" cy="11" r="0.8"/><circle cx="23" cy="13" r="0.7"/><circle cx="14" cy="14" r="0.6"/></g>
    <circle cx="7.4" cy="9.6" r="1.6" fill="#111" stroke="none"/><circle cx="7" cy="9.2" r="0.5" fill="#fff" stroke="none"/>
    <circle cx="4.4" cy="11.6" r="0.8" fill="#111" stroke="none"/>
    <path d="M4 12.6 l -3 -0.6 M4 13.2 l -3 0.6" stroke-width="0.3"/>
    <path d="M14 18 C 16 20, 19 20, 20 19" fill="#4a5662"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'elephantseal.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 90 40" width="360" height="160" data-canvas-width="72" data-densities="4 8">
  <!--
    Elephant seal, facing left, belly on y 39.4. Drawn 72x32 canvas units. Big, loud, smelly.
      part-body
      part-nose  the trunk-nose, wobbles about (14, 14)
  -->
  <defs><linearGradient id="es" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#9a8a78"/><stop offset="1" stop-color="#5a4a3a"/></linearGradient></defs>
  <g id="part-body" stroke="#2a1e14" stroke-width="0.9" stroke-linejoin="round">
    <path d="M12 26 C 10 12, 22 6, 36 8 C 58 10, 76 18, 86 30 C 80 38, 50 40, 30 39.4 C 16 39, 12 34, 12 26 Z" fill="url(#es)"/>
    <path d="M30 18 C 40 16, 52 18, 60 22 M28 26 C 40 25, 56 27, 66 31" fill="none" stroke="#6a5a4a" stroke-width="0.8"/>
    <path d="M82 28 L 89 24 L 88 30 L 89 36 Z" fill="#5a4a3a"/>
    <circle cx="20" cy="16" r="2" fill="#111" stroke="none"/><circle cx="19.4" cy="15.4" r="0.6" fill="#fff" stroke="none"/>
    <path d="M30 38 C 34 40, 40 40, 42 38" fill="#4a3a2a"/>
    <path d="M20 21 C 18 24, 14 26, 13 30" fill="none" stroke="#2a1e14" stroke-width="0.7"/>
  </g>
  <g id="part-nose" stroke="#2a1e14" stroke-width="0.8" stroke-linejoin="round">
    <path d="M14 14 C 8 12, 3 15, 2 20 C 1 24, 4 27, 7 25 C 8 22, 10 20, 14 20 Z" fill="#8a7a68"/>
    <circle cx="4" cy="23" r="0.8" fill="#2a1e14" stroke="none"/>
  </g>
</svg>
''', encoding='utf-8')

# ------------------------------------------------------------------ backdrop
m = (HERE / 'mountain.svg').read_text(encoding='utf-8')
defs = m[m.index('  <defs>'):m.index('  </defs>') + len('  </defs>')]
b0 = m.index('    <!-- ===================== Bunker')
b1 = m.index('\n  </g>', b0)
bunker = m[b0:b1 + 1]
s0 = bunker.index('    <g transform="translate(-112 23)">\n    <!-- Storage hall under the hangar -->')
depth, i = 0, s0
while True:
    o, c = bunker.find('<g', i), bunker.find('</g>', i)
    if o != -1 and o < c:
        depth, i = depth + 1, o + 2
    else:
        depth, i = depth - 1, c + 4
        if depth == 0:
            break
s1 = bunker.index('\n', i) + 1
boiler = '''    <!-- Boiler Room (replaces the storage hall in the ice) -->
    <rect x="560" y="320" width="288" height="106" fill="#3a2e2a"/>
    <rect x="560" y="320" width="288" height="106" filter="url(#concrete)" opacity="0.5"/>
    <rect x="560" y="421.4" width="288" height="4.6" fill="url(#grate)"/>
    <text x="704" y="338" font-family="Arial Black, Arial, sans-serif" font-size="8" font-weight="bold" text-anchor="middle" fill="#e8c8a8" opacity="0.2" letter-spacing="1">BOILER ROOM</text>
    <!-- The furnace (its glow is a part), coal, pipes going everywhere -->
    <g stroke="#14161a" stroke-width="0.8">
      <rect x="690" y="356" width="70" height="65" rx="6" fill="#4a4048"/>
      <rect x="704" y="380" width="42" height="30" rx="3" fill="#1a1010"/>
      <path d="M690 368 h 70 M690 400 h 70" stroke="#2a2428"/>
      <rect x="716" y="320" width="18" height="36" fill="#5a5058"/>
      <path d="M560 344 H 690 M760 344 H 848 M760 336 H 848" stroke="#8a6a4a" stroke-width="4"/>
      <path d="M600 421 c 6 -14, 30 -16, 44 0 Z" fill="#1a1a1e"/>
      <circle cx="790" cy="380" r="14" fill="#f0ebdc"/>
      <path d="M778 384 A 14 14 0 0 1 790 366 A 14 14 0 0 1 802 384" fill="none" stroke="#c8241c" stroke-width="2"/>
    </g>
    <text x="790" y="392" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#14161a">HEAT</text>
    <rect x="808" y="350" width="34" height="22" fill="#e9e1c6" stroke="#14161a" stroke-width="0.5"/>
    <text x="825" y="357" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#c8241c">FEED THE BOILER</text>
    <text x="825" y="362" font-family="Arial, sans-serif" font-size="2.6" text-anchor="middle" fill="#14161a">it is the only one</text>
    <text x="825" y="366" font-family="Arial, sans-serif" font-size="2.6" text-anchor="middle" fill="#14161a">that loves you</text>
'''
bunker = bunker[:s0] + boiler + bunker[s1:]

ice_defs = defs.replace('  </defs>', '''    <linearGradient id="iceSky" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#0a1428" stop-opacity="0"/><stop offset="1" stop-color="#2a3a5a" stop-opacity="0.6"/>
    </linearGradient>
    <linearGradient id="iceTop" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#f4faff"/><stop offset="1" stop-color="#b8d4e8"/>
    </linearGradient>
    <linearGradient id="iceDeep" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#8ab8d8"/><stop offset="0.3" stop-color="#4a6a88"/><stop offset="1" stop-color="#2a2a32"/>
    </linearGradient>
    <linearGradient id="hut" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#c8ccd0"/><stop offset="1" stop-color="#8a9098"/>
    </linearGradient>
    <linearGradient id="auroraG" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#7affb0" stop-opacity="0"/><stop offset="0.5" stop-color="#7affb0" stop-opacity="0.45"/><stop offset="1" stop-color="#7a9aff" stop-opacity="0"/>
    </linearGradient>
  </defs>''')

drifts = ''.join(f'<ellipse cx="{x}" cy="{y}" rx="{rx}" ry="{ry}" fill="#f4faff"/>' for x, y, rx, ry in
                 [(rnd.uniform(260, 1680), rnd.uniform(252, 262), rnd.uniform(20, 60), rnd.uniform(4, 9)) for _ in range(18)])
ice_svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="240 -300 1460 1160" width="2190" height="1740" data-canvas-width="1460" data-densities="1.5 2.5">
  <!--
    Station Permafrost (Antarctic): drawn instead of the mountain, same frame and base-local coordinates.
    An ice shelf; the hangar is a snow-covered hut (780..1080 x 110..210, open to the left) on stilts above
    the shelf; the bunker is cut into the ice and permafrost, with the Boiler Room where storage was.
      part-back          sky haze, ice shelf, hut, flag, snowmobile, bunker
      part-aurora        aurora curtains (shimmer)
      part-furnace_glow  the boiler's fire (flickers)
      part-taps          Geothermal Taps: vents in the ice (the game puffs steam from (1250, 236) and (1330, 240))
      part-vault         Cryo Vault: a frosted vault door on the shelf
  -->
{ice_defs}
  <g id="part-back">
    <rect x="240" y="-300" width="1460" height="560" fill="url(#iceSky)"/>
    <!-- Distant icebergs -->
    <path d="M1300 250 L 1340 170 L 1370 190 L 1400 140 L 1450 250 Z M1480 250 L 1520 200 L 1560 215 L 1600 250 Z" fill="#c8dcec" opacity="0.7"/>
    <!-- Ice shelf -->
    <path d="M240 300 L 240 252 C 400 246, 700 250, 1000 248 C 1300 246, 1500 252, 1700 250 L 1700 300 Z" fill="url(#iceTop)"/>
    {drifts}
    <rect x="240" y="296" width="1460" height="564" fill="url(#iceDeep)"/>
    <rect x="240" y="296" width="1460" height="564" filter="url(#rockTex)" opacity="0.5"/>
    <path d="M240 330 C 600 320, 1200 340, 1700 326 M240 380 C 700 370, 1100 390, 1700 376" fill="none" stroke="#c8e4f4" stroke-width="1" opacity="0.4"/>
    <!-- The hangar hut on stilts -->
    <path d="M790 248 V 214 M1070 248 V 214 M930 248 V 214" stroke="#5a6068" stroke-width="4"/>
    <path d="M776 214 L 776 112 C 800 70, 1060 70, 1084 112 L 1084 214 Z" fill="url(#hut)" stroke="#14161a" stroke-width="1"/>
    <path d="M790 92 C 860 72, 1000 72, 1070 92" fill="none" stroke="#f4faff" stroke-width="10"/>
    <path d="M800 84 l 2 8 l 2 -8 M860 78 l 2 9 l 2 -9 M1000 78 l 2 10 l 2 -10 M1060 86 l 2 7 l 2 -7" fill="#dff0ff"/>
    <text x="930" y="66" font-family="Arial Black, Arial, sans-serif" font-size="8" font-weight="bold" text-anchor="middle" fill="#e8f2ff">STATION PERMAFROST</text>
    <text x="930" y="58" font-family="Arial, sans-serif" font-size="4" text-anchor="middle" fill="#9fc8e8">definitely a weather station</text>
    <!-- Flag, snowmobile -->
    <path d="M740 252 V 196" stroke="#8a9098" stroke-width="1.4"/>
    <path d="M740 198 h 18 l -3 5 l 3 5 h -18 Z" fill="#7fc25b" stroke="#14161a" stroke-width="0.4"/>
    <g stroke="#14161a" stroke-width="0.6"><path d="M700 252 h 26 l -4 -6 h -14 l -4 -4 h -4 Z" fill="#c8241c"/><path d="M698 254 h 32" stroke-width="1.6"/></g>
    <!-- Warning sign about the cold -->
    <rect x="1140" y="222" width="40" height="18" fill="#e9e1c6" stroke="#14161a" stroke-width="0.6"/>
    <text x="1160" y="229" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#2b5a8a">OUTSIDE TEMP:</text>
    <text x="1160" y="235" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#c8241c">NO</text>
    <path d="M1160 240 v 12" stroke="#5a6068" stroke-width="1.2"/>

{bunker}  </g>
  <g id="part-aurora">
    <path d="M240 -260 C 500 -200, 800 -280, 1100 -210 C 1400 -150, 1600 -240, 1700 -200 L 1700 -60 C 1500 -110, 1300 -20, 1000 -80 C 700 -140, 500 -60, 240 -120 Z" fill="url(#auroraG)"/>
  </g>
  <g id="part-furnace_glow">
    <rect x="704" y="380" width="42" height="30" rx="3" fill="#ff8a2a"/>
    <path d="M708 410 c 4 -12, 8 -6, 10 -16 c 4 10, 8 4, 10 14 c 4 -10, 8 -4, 12 -12 c 2 6, 2 10, 0 14 Z" fill="#ffe08a"/>
    <circle cx="725" cy="395" r="40" fill="#ff8a2a" opacity="0.15"/>
  </g>
  <g id="part-taps" stroke="#14161a" stroke-width="0.8">
    <rect x="1242" y="236" width="16" height="16" fill="#5a6068"/><rect x="1322" y="240" width="16" height="12" fill="#5a6068"/>
    <path d="M1250 252 V 296 M1330 252 V 296" stroke="#c96a2a" stroke-width="3"/>
    <text x="1290" y="232" font-family="Arial, sans-serif" font-size="3.6" font-weight="bold" text-anchor="middle" fill="#c96a2a" stroke="none">GEOTHERMAL</text>
  </g>
  <g id="part-vault" stroke="#14161a" stroke-width="0.8">
    <rect x="1190" y="200" width="44" height="52" rx="4" fill="#8aa8c0"/>
    <circle cx="1212" cy="226" r="14" fill="#b8d4e8"/><circle cx="1212" cy="226" r="4" fill="#5a6068"/>
    <path d="M1212 212 V 240 M1198 226 H 1226" stroke="#5a6068" stroke-width="1.6"/>
    <path d="M1190 204 c 6 4, 12 -2, 18 2 c 6 4, 12 -2, 18 2 c 4 2, 6 0, 8 -2" fill="none" stroke="#f4faff" stroke-width="3"/>
    <text x="1212" y="250" font-family="Arial, sans-serif" font-size="3.4" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">CRYO VAULT</text>
  </g>
</svg>
'''
(HERE / 'ice.svg').write_text(ice_svg, encoding='utf-8')

# ------------------------------------------------------------------ overlays: frost, melt, insulation, cryo pods, stoker
(HERE / 'frost.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 330 115" width="660" height="230" data-canvas-width="330" data-densities="2 4">
  <!--
    Room overlays in a 330x115 frame (lab size; the hangar draws it at 300x100):
      part-frost     frosted edges and icicles (alpha = how cold)
      part-melt      meltwater: drips, puddles and a news ticker blaming cows
      part-cladding  Insulated Labs: penguin-style black-and-white cladding
  -->
  <g id="part-frost">
    <path d="M0 0 H 330 V 10 C 280 16, 220 8, 160 14 C 100 8, 40 16, 0 10 Z" fill="#e8f6ff" opacity="0.85"/>
    <path d="M0 0 V 115 H 10 C 16 80, 6 40, 12 0 Z M330 0 V 115 H 320 C 314 80, 324 40, 318 0 Z" fill="#e8f6ff" opacity="0.7"/>
    <g fill="#dff0ff">''' + ''.join(f'<path d="M{x} 10 l 2 {h} l 2 -{h} Z"/>' for x, h in [(rnd.uniform(14, 310), rnd.uniform(6, 16)) for _ in range(22)]) + '''</g>
    <g fill="#ffffff" opacity="0.7">''' + ''.join(f'<path d="M{x} {y} l 1 3 l 3 1 l -3 1 l -1 3 l -1 -3 l -3 -1 l 3 -1 Z"/>' for x, y in [(rnd.uniform(10, 320), rnd.uniform(14, 100)) for _ in range(14)]) + '''</g>
  </g>
  <g id="part-melt">
    <g fill="#6fb8e8" opacity="0.8">''' + ''.join(f'<path d="M{x} {y} c -1.4 2, -1.4 4, 0 4 c 1.4 0, 1.4 -2, 0 -4 Z"/>' for x, y in [(rnd.uniform(20, 310), rnd.uniform(10, 60)) for _ in range(16)]) + '''</g>
    <g fill="#6fb8e8" opacity="0.5">''' + ''.join(f'<ellipse cx="{x}" cy="112" rx="{rx}" ry="2"/>' for x, rx in [(rnd.uniform(20, 310), rnd.uniform(8, 20)) for _ in range(6)]) + '''</g>
    <rect x="0" y="100" width="330" height="9" fill="#14161a" opacity="0.8"/>
    <text x="165" y="106.6" font-family="Arial, sans-serif" font-size="5" font-weight="bold" text-anchor="middle" fill="#ffe08a">LOCAL NEWS: scientists blame unexplained Antarctic heatwave on cow flatulence</text>
  </g>
  <g id="part-cladding" stroke="#14161a" stroke-width="0.6">
    <path d="M0 0 H 330 V 6 H 0 Z" fill="#1c2030"/>
    <path d="M0 6 H 330 V 9 H 0 Z" fill="#f4f6f8"/>
    <path d="M0 0 V 115 H 5 V 0 Z M325 0 V 115 H 330 V 0 Z" fill="#1c2030"/>
    <g fill="#e8a02a" stroke="none">''' + ''.join(f'<circle cx="{x}" cy="3" r="0.8"/>' for x in range(10, 330, 20)) + '''</g>
    <text x="300" y="5" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#f4f6f8" stroke="none">INSULATED</text>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'cryopods.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 60 48" width="240" height="192" data-canvas-width="60" data-densities="4 8">
  <!--
    Cryo Pods by containment: three frosted pods, one with a frozen squirrel inside. Feet at y 48.
      part-body
      part-lights  status lights (blink)
  -->
  <g id="part-body" stroke="#14161a" stroke-width="0.7">
    <rect x="0" y="2" width="60" height="46" fill="#3a3e46"/>
    ''' + ''.join(f'<rect x="{x}" y="6" width="14" height="40" rx="7" fill="#b8d4e8"/><rect x="{x + 2}" y="10" width="10" height="30" rx="5" fill="#dff0ff" opacity="0.8"/>' for x in (4, 23, 42)) + '''
    <g transform="translate(26 26) scale(0.35)" opacity="0.8"><ellipse cx="10" cy="10" rx="6" ry="6" fill="#a9582a"/><circle cx="4" cy="4" r="4" fill="#a9582a"/><path d="M14 12 C 22 10, 22 0, 16 -4" fill="none" stroke="#a9582a" stroke-width="4"/></g>
    <text x="30" y="5.6" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#9fd0e8" stroke="none">CRYO PODS</text>
  </g>
  <g id="part-lights" stroke="none">
    <circle cx="11" cy="44" r="1" fill="#7aff9a"/><circle cx="30" cy="44" r="1" fill="#7aff9a"/><circle cx="49" cy="44" r="1" fill="#ff4a3a"/>
  </g>
</svg>
''', encoding='utf-8')

j = (HERE / 'janitor.svg').read_text(encoding='utf-8')
stoker = j.replace('Cleanup Crew: an alien janitor with a mop and bucket, facing left.', 'Boiler Room stoker: an alien in coveralls shovelling coal, facing right (mirrored by the art).')
stoker = stoker.replace('<text x="16" y="29" font-family="Arial, sans-serif" font-size="2.2" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">CLEANUP</text>',
                        '<text x="17.4" y="29" font-family="Arial, sans-serif" font-size="1.8" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">BOILER</text>')
stoker = stoker.replace('    <path d="M2 40 h 9 l -1 7 h -7 Z" fill="#c9a224"/>\n    <path d="M2.6 41.4 h 7.8" stroke="#8a6a10" stroke-width="0.6"/>\n    <ellipse cx="6.5" cy="40" rx="4.5" ry="0.8" fill="#6fa8c8" stroke-width="0.3"/>\n', '')
k0 = stoker.index('  <g id="part-mop"')
k1 = stoker.index('  </g>\n', k0) + len('  </g>\n')
stoker = stoker[:k0] + '''  <g id="part-shovel" stroke="#1c2318" stroke-width="0.6" stroke-linecap="round">
    <!-- swings about the hands (12, 28) -->
    <path d="M12 28 L 2 40" stroke="#a87a4a" stroke-width="1.4"/>
    <path d="M-1 38 L 3 36 L 6 42 L 2 44 Z" fill="#5a6068"/>
    <path d="M0 39 l 2 -1 l 1 2 Z" fill="#1a1a1e"/>
    <circle cx="12" cy="28" r="1.2" fill="#8fd16a"/>
  </g>
''' + stoker[k1:]
stoker = stoker.replace('part-mop   the mop, swings about the hands (12, 28)', 'part-shovel  the coal shovel, swings about the hands (12, 28)')
(HERE / 'stoker.svg').write_text(stoker, encoding='utf-8')

# ------------------------------------------------------------------ regions
rdefs_src = (HERE / 'region_mountain_0.svg').read_text(encoding='utf-8')
rdefs = rdefs_src[rdefs_src.index('  <defs>'):rdefs_src.index('  </defs>') + len('  </defs>')]


def region(name, doc, back, extra=''):
    (HERE / f'{name}.svg').write_text(f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 220 210" width="440" height="420" data-canvas-width="220" data-densities="2 4">
  <!--
    {doc}
    Hunting region backdrop, drawn at canvas (x0, 100), 220x210; ground at local y 200.
  -->
{rdefs}
  <g id="part-back">
{back}
  </g>
{extra}</svg>
''', encoding='utf-8')


ice_ground = '<rect x="0" y="200" width="220" height="10" fill="#f4faff"/><path d="M0 200 H 220" stroke="#b8d4e8" stroke-width="0.8"/>'
region('region_ice_0', 'The Ice Shelf: flat ice, floes and bergs (penguins, seals).', f'''
    <path d="M0 160 L 30 130 L 50 150 L 70 120 L 100 160 Z M140 170 L 170 140 L 200 170 Z" fill="#c8dcec"/>
    <path d="M0 184 C 60 180, 140 186, 220 184 L 220 200 L 0 200 Z" fill="#dff0ff"/>
    <path d="M20 196 h 30 l -4 4 h -22 Z M120 194 h 40 l -6 6 h -30 Z" fill="#2a4a6a"/>
    <text x="110" y="176" font-family="Arial, sans-serif" font-size="3.6" font-weight="bold" text-anchor="middle" fill="#5a7a9a">nothing for 1,200 km in any direction</text>
    {ice_ground}''')
region('region_ice_1', 'Research Outpost: prefab huts, a satellite dish, twelve scientists and one cook (humans, penguins).', f'''
    <path d="M0 184 C 60 180, 140 186, 220 184 L 220 200 L 0 200 Z" fill="#dff0ff"/>
    <g stroke="#14161a" stroke-width="0.6">
      <rect x="10" y="172" width="40" height="28" fill="#c8241c"/><path d="M8 172 C 20 160, 40 160, 52 172 Z" fill="#f4faff"/>
      <rect x="60" y="176" width="36" height="24" fill="#e2b923"/><path d="M58 176 C 70 166, 86 166, 98 176 Z" fill="#f4faff"/>
      <rect x="20" y="182" width="8" height="6" fill="#ffe08a"/><rect x="70" y="184" width="8" height="6" fill="#ffe08a"/>
      <path d="M150 200 V 168" stroke="#8a9098" stroke-width="2"/>
      <rect x="104" y="186" width="34" height="14" fill="#e9e1c6"/>
    </g>
    <text x="121" y="192" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#14161a">12 SCIENTISTS</text>
    <text x="121" y="197" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#14161a">1 COOK (TIRED)</text>
    <path d="M190 200 V 150 M184 152 h 12" stroke="#8a9098" stroke-width="1.4"/>
    {ice_ground}''',
       extra='''  <g id="part-dish" stroke="#14161a" stroke-width="0.6">
    <!-- Satellite dish, turns about (150, 168) -->
    <path d="M136 160 C 140 174, 160 174, 164 160 Z" fill="#d8dde2"/>
    <path d="M150 170 L 150 156" stroke="#8a9098" stroke-width="0.8"/>
    <circle cx="150" cy="156" r="1.4" fill="#c8241c"/>
  </g>
''')
region('region_ice_2', 'Elephant Seal Beach: shingle, rocks and grey surf (elephant seals, seals).', f'''
    <rect x="0" y="150" width="220" height="40" fill="#2a4a6a"/>
    <path d="M0 152 {' '.join(f'Q {x + 7} 148, {x + 14} 152' for x in range(0, 220, 14))}" fill="none" stroke="#dff0ff" stroke-width="1"/>
    <path d="M0 178 C 60 172, 140 176, 220 172 L 220 200 L 0 200 Z" fill="#6a6a70"/>
    <g fill="#8a8a90">''' + ''.join(f'<ellipse cx="{rnd.uniform(4, 216):.0f}" cy="{rnd.uniform(180, 198):.0f}" rx="{rnd.uniform(1, 3):.1f}" ry="1"/>' for _ in range(40)) + '''</g>
    <path d="M160 178 L 176 150 L 196 160 L 210 178 Z" fill="#3a3a42"/>
    <text x="60" y="168" font-family="Arial, sans-serif" font-size="3.6" font-weight="bold" fill="#dff0ff" opacity="0.8">smell advisory in effect</text>
    <rect x="0" y="200" width="220" height="10" fill="#7a7a80"/>''')
print('ok')
