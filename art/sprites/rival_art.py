from pathlib import Path
HERE = Path(r'F:\Alien_Idle\art\sprites')
(HERE / 'rival_bureau.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 96 48" width="384" height="192" data-canvas-width="96">
  <!--
    The Efficiency Bureau's ship: grey, boxy, regulation. Anchor (48, 26) like the player's ships.
      part-body    the ship
      part-lights  a row of status lights that blink in strict sequence
  -->
  <defs>
    <linearGradient id="grey" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#b8bec6"/><stop offset="0.6" stop-color="#7a808a"/><stop offset="1" stop-color="#4a4f58"/></linearGradient>
  </defs>
  <g id="part-body" stroke="#14161a" stroke-width="0.8" stroke-linejoin="round">
    <path d="M10 26 L 18 18 L 78 18 L 86 26 L 78 34 L 18 34 Z" fill="url(#grey)"/>
    <path d="M10 26 H 86" stroke="#4a4f58" stroke-width="0.6"/>
    <rect x="36" y="8" width="24" height="10" fill="#5a6068"/>
    <rect x="39" y="10" width="18" height="6" fill="#9fb4c8" opacity="0.8"/>
    <path d="M22 18 V 34 M74 18 V 34" stroke="#5a6068" stroke-width="0.6"/>
    <!-- clipboard logo -->
    <rect x="42" y="20" width="12" height="12" rx="0.8" fill="#e9e1c6"/>
    <rect x="45.4" y="19" width="5.2" height="2.4" fill="#5a6068"/>
    <path d="M44 24 h 8 M44 26.6 h 8 M44 29.2 h 5" stroke="#5a6068" stroke-width="0.6"/>
    <path d="M51 28.4 l 1.4 1.4 l 2.6 -3" fill="none" stroke="#2b8a4a" stroke-width="0.8"/>
    <text x="30" y="28" font-family="Arial, sans-serif" font-size="3.2" font-weight="bold" text-anchor="middle" fill="#1d1f25" stroke="none">E.B.</text>
    <text x="66" y="28" font-family="Arial, sans-serif" font-size="2.6" font-weight="bold" text-anchor="middle" fill="#1d1f25" stroke="none">FORM 7-B</text>
  </g>
  <g id="part-lights" stroke="none">
    <circle cx="24" cy="31" r="1" fill="#7aff9a"/><circle cx="36" cy="31" r="1" fill="#7aff9a"/><circle cx="60" cy="31" r="1" fill="#7aff9a"/><circle cx="72" cy="31" r="1" fill="#7aff9a"/>
  </g>
</svg>
''', encoding='utf-8')
(HERE / 'rival_sign.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 60 26" width="240" height="104" data-canvas-width="60" data-densities="4 8">
  <!--
    The Bureau reserves a hunting ground: a queue barrier and a sign. Feet at y 26.
      part-body
  -->
  <g id="part-body" stroke="#14161a" stroke-width="0.5">
    <path d="M4 26 V 12 M30 26 V 12 M56 26 V 12" stroke="#c9ced6" stroke-width="1.2"/>
    <path d="M4 14 C 12 20, 22 20, 30 14 C 38 20, 48 20, 56 14" fill="none" stroke="#c8241c" stroke-width="1.2"/>
    <g fill="#8a9098"><circle cx="4" cy="12" r="1.2"/><circle cx="30" cy="12" r="1.2"/><circle cx="56" cy="12" r="1.2"/></g>
    <rect x="14" y="0" width="32" height="10" fill="#e9e1c6"/>
    <text x="30" y="4.2" font-family="Arial, sans-serif" font-size="2.8" font-weight="bold" text-anchor="middle" fill="#c8241c" stroke="none">RESERVED</text>
    <text x="30" y="8" font-family="Arial, sans-serif" font-size="2.3" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">BUREAU VEHICLES ONLY</text>
  </g>
</svg>
''', encoding='utf-8')
print('ok')
