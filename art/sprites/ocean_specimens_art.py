"""Oceanic specimens: pufferfish, octopus, whale. Each faces left."""
from pathlib import Path
HERE = Path(__file__).resolve().parent

(HERE / 'pufferfish.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 20" width="192" height="160" data-canvas-width="16" data-densities="4 8">
  <!--
    Pufferfish, facing left, belly on y 19. Drawn 16x13.3 canvas units.
      part-body    relaxed
      part-puffed  stressed (shown now and then; staff relate)
      part-fin     tail fin, flaps about (19, 11)
  -->
  <defs>
    <radialGradient id="pf" cx="0.4" cy="0.35" r="0.7"><stop offset="0" stop-color="#fff2a8"/><stop offset="1" stop-color="#d6a02a"/></radialGradient>
  </defs>
  <g id="part-fin" stroke="#3a2a10" stroke-width="0.5" stroke-linejoin="round">
    <path d="M18 11 L 23 7 L 22.4 11 L 23 15 Z" fill="#e2b923"/>
  </g>
  <g id="part-body" stroke="#3a2a10" stroke-width="0.6" stroke-linejoin="round">
    <ellipse cx="11" cy="12" rx="8" ry="6" fill="url(#pf)"/>
    <path d="M5 14 C 8 17, 14 17, 17 14" fill="#fff8e0" stroke="none"/>
    <g fill="#8a6a2a" stroke="none"><circle cx="10" cy="9" r="0.6"/><circle cx="13" cy="10" r="0.6"/><circle cx="15" cy="13" r="0.6"/></g>
    <circle cx="6.6" cy="10" r="1.8" fill="#ffffff"/><circle cx="6.2" cy="10" r="0.8" fill="#111" stroke="none"/>
    <path d="M3.2 12.6 c 0.6 0.6, 1.4 0.6, 1.8 0" fill="none" stroke-width="0.5"/>
  </g>
  <g id="part-puffed" stroke="#3a2a10" stroke-width="0.6" stroke-linejoin="round">
    <circle cx="11" cy="10" r="9" fill="url(#pf)"/>
    <path d="''' + ' '.join(f'M{11 + 9 * __import__("math").cos(a * 0.3927):.1f} {10 + 9 * __import__("math").sin(a * 0.3927):.1f} l {2 * __import__("math").cos(a * 0.3927):.1f} {2 * __import__("math").sin(a * 0.3927):.1f}' for a in range(16)) + '''" stroke-width="0.7"/>
    <circle cx="6" cy="8" r="2.4" fill="#ffffff"/><circle cx="5.6" cy="8.2" r="1.4" fill="#111" stroke="none"/>
    <path d="M3 12 c 0.4 0.8, 1.4 0.8, 1.8 0" fill="none" stroke-width="0.6"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'octopus.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 28" width="256" height="224" data-canvas-width="26" data-densities="4 8">
  <!--
    Octopus, bottom on y 27. Drawn 26x22.75 canvas units.
      part-arms_a / part-arms_b  tentacles, two wriggle frames
      part-body                  head and eyes (the eyes are judging you)
  -->
  <defs>
    <radialGradient id="oc" cx="0.4" cy="0.3" r="0.75"><stop offset="0" stop-color="#e0a0ff"/><stop offset="1" stop-color="#7a3aa6"/></radialGradient>
  </defs>
  <g id="part-arms_a" fill="none" stroke="#5a2a7a" stroke-width="2.2" stroke-linecap="round">
    <path d="M10 17 C 6 20, 4 24, 1 25 M13 18 C 11 22, 10 25, 7 27 M16 18 C 16 22, 15 25, 13 27 M19 18 C 20 22, 22 25, 24 26 M22 17 C 25 20, 28 22, 31 22"/>
    <path d="M10 17 C 6 20, 4 24, 1 25 M13 18 C 11 22, 10 25, 7 27 M16 18 C 16 22, 15 25, 13 27 M19 18 C 20 22, 22 25, 24 26 M22 17 C 25 20, 28 22, 31 22" stroke="#c07ae8" stroke-width="1.2"/>
  </g>
  <g id="part-arms_b" fill="none" stroke="#5a2a7a" stroke-width="2.2" stroke-linecap="round">
    <path d="M10 17 C 7 21, 3 22, 2 26 M13 18 C 12 22, 8 24, 8 27 M16 18 C 17 22, 14 24, 15 27 M19 18 C 19 22, 23 23, 23 27 M22 17 C 26 19, 27 24, 30 25"/>
    <path d="M10 17 C 7 21, 3 22, 2 26 M13 18 C 12 22, 8 24, 8 27 M16 18 C 17 22, 14 24, 15 27 M19 18 C 19 22, 23 23, 23 27 M22 17 C 26 19, 27 24, 30 25" stroke="#c07ae8" stroke-width="1.2"/>
  </g>
  <g id="part-body" stroke="#3a1a5a" stroke-width="0.7">
    <path d="M8 16 C 6 6, 12 1, 16 1 C 22 1, 27 6, 24 17 C 20 20, 12 20, 8 16 Z" fill="url(#oc)"/>
    <g fill="#d08af0" stroke="none"><circle cx="14" cy="6" r="0.9"/><circle cx="19" cy="5" r="0.7"/><circle cx="21" cy="9" r="0.8"/></g>
    <ellipse cx="12.4" cy="12.4" rx="2.4" ry="2" fill="#fff8e0"/><ellipse cx="18.6" cy="12.4" rx="2.4" ry="2" fill="#fff8e0"/>
    <rect x="11" y="12" width="2.8" height="0.9" fill="#111" stroke="none"/><rect x="17.2" y="12" width="2.8" height="0.9" fill="#111" stroke="none"/>
    <path d="M10 10.4 l 4 0.8 M21 10.4 l -4 0.8" stroke-width="0.8"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'whale.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 44" width="480" height="176" data-canvas-width="96" data-densities="2 4 8">
  <!--
    Whale, facing left, belly near y 40. Drawn 96x35 canvas units, floating.
      part-tail   flukes, sway about (100, 22)
      part-body   the whale
      part-notes  music notes (whale song)
  -->
  <defs>
    <linearGradient id="wh" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#5a7aa8"/><stop offset="0.6" stop-color="#3a5a8a"/><stop offset="1" stop-color="#2a4068"/></linearGradient>
  </defs>
  <g id="part-tail" stroke="#14203a" stroke-width="0.9" stroke-linejoin="round">
    <path d="M98 22 C 104 20, 108 14, 116 8 C 116 14, 114 18, 110 22 C 114 26, 116 30, 116 36 C 108 30, 104 24, 98 24 Z" fill="url(#wh)"/>
  </g>
  <g id="part-body" stroke="#14203a" stroke-width="0.9" stroke-linejoin="round">
    <path d="M4 26 C 4 12, 24 6, 50 6 C 76 6, 96 14, 102 22 C 96 32, 76 40, 48 40 C 22 40, 4 36, 4 26 Z" fill="url(#wh)"/>
    <path d="M6 30 C 20 38, 60 40, 92 30 C 76 38, 52 40, 30 39 C 18 38, 10 35, 6 30 Z" fill="#d8e4f0" stroke="none"/>
    <path d="M10 31 h 40 M12 34 h 36 M16 37 h 26" stroke="#9fb4c8" stroke-width="0.6"/>
    <path d="M40 30 C 46 34, 52 36, 56 40 C 48 40, 42 36, 40 30 Z" fill="#2a4068"/>
    <circle cx="20" cy="22" r="2" fill="#ffffff"/><circle cx="19.4" cy="22" r="1" fill="#111" stroke="none"/>
    <path d="M17 19.6 l 5 -0.8" stroke-width="0.7"/>
    <path d="M30 8 C 34 6, 40 6, 44 7" fill="none" stroke="#9fb4e8" stroke-width="1" opacity="0.7"/>
    <g fill="#9fb4c8" stroke="none" opacity="0.7"><circle cx="60" cy="14" r="0.8"/><circle cx="66" cy="18" r="0.6"/><circle cx="72" cy="12" r="0.7"/></g>
  </g>
  <g id="part-notes" fill="#e8f2ff" stroke="none">
    <path d="M4 10 v -7 l 5 -1 v 6.4" fill="none" stroke="#e8f2ff" stroke-width="0.8"/><ellipse cx="3" cy="10" rx="1.6" ry="1.1"/><ellipse cx="8" cy="8.6" rx="1.6" ry="1.1"/>
    <path d="M16 6 v -5" stroke="#e8f2ff" stroke-width="0.8"/><ellipse cx="15" cy="6" rx="1.6" ry="1.1"/>
  </g>
</svg>
''', encoding='utf-8')
print('ok')
