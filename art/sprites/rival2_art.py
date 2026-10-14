"""Rivals phase 2 sprites: Celebrity Studies Division, Experimental Sciences Office, and the Cleanup Crew janitor."""
from pathlib import Path
HERE = Path(__file__).resolve().parent

(HERE / 'rival_celebrity.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 96 48" width="384" height="192" data-canvas-width="96">
  <!--
    Celebrity Studies Division ship: glossy pink, a star on the hull, a spotlight. Anchor (48, 26).
      part-body     the ship
      part-sparkle  glints (flicker)
  -->
  <defs>
    <linearGradient id="pink" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#ffd0ea"/><stop offset="0.5" stop-color="#ff6fb8"/><stop offset="1" stop-color="#b8306f"/></linearGradient>
    <radialGradient id="dome" cx="0.4" cy="0.35" r="0.75"><stop offset="0" stop-color="#ffffff" stop-opacity="0.8"/><stop offset="1" stop-color="#ff9ad0" stop-opacity="0.5"/></radialGradient>
  </defs>
  <g id="part-body" stroke="#3a0a24" stroke-width="0.9" stroke-linejoin="round">
    <path d="M34 22 C 34 10, 40 6, 48 6 C 56 6, 62 10, 62 22 Z" fill="url(#dome)"/>
    <path d="M8 28 C 12 20, 30 18, 48 18 C 66 18, 84 20, 88 28 C 84 34, 66 37, 48 37 C 30 37, 12 34, 8 28 Z" fill="url(#pink)"/>
    <path d="M14 23 C 26 20, 40 19.6, 48 19.6" fill="none" stroke="#ffffff" stroke-width="1.2" stroke-opacity="0.9"/>
    <path d="M48 23 l 1.6 3.6 l 3.8 0.3 l -3 2.4 l 1 3.8 l -3.4 -2.2 l -3.4 2.2 l 1 -3.8 l -3 -2.4 l 3.8 -0.3 Z" fill="#ffe08a" stroke-width="0.4"/>
    <text x="28" y="31" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#ffffff" stroke="none">CSD</text>
    <text x="69" y="31" font-family="Arial, sans-serif" font-size="2.6" font-style="italic" text-anchor="middle" fill="#ffffff" stroke="none">xoxo</text>
  </g>
  <g id="part-sparkle" stroke="none" fill="#ffffff">
    <path d="M20 21 l 0.6 -2 l 0.6 2 l 2 0.6 l -2 0.6 l -0.6 2 l -0.6 -2 l -2 -0.6 Z"/>
    <path d="M70 20 l 0.5 -1.6 l 0.5 1.6 l 1.6 0.5 l -1.6 0.5 l -0.5 1.6 l -0.5 -1.6 l -1.6 -0.5 Z"/>
    <path d="M44 9 l 0.5 -1.6 l 0.5 1.6 l 1.6 0.5 l -1.6 0.5 l -0.5 1.6 l -0.5 -1.6 l -1.6 -0.5 Z"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'rival_carpet.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 40" width="480" height="160" data-canvas-width="120" data-densities="4 8">
  <!--
    A Celebrity Studies stunt: red carpet, velvet ropes, paparazzi and a news van. Feet at y 40.
      part-body     carpet, ropes, photographers, van
      part-flashes  camera flashes (shown at random)
  -->
  <g id="part-body" stroke="#14161a" stroke-width="0.5">
    <path d="M30 40 L 34 34 L 86 34 L 90 40 Z" fill="#c8241c"/>
    <path d="M36 34 v -8 M60 34 v -8 M84 34 v -8" stroke="#e2b923" stroke-width="1"/>
    <path d="M36 27 C 44 31, 52 31, 60 27 C 68 31, 76 31, 84 27" fill="none" stroke="#7a1030" stroke-width="1.2"/>
    <g><circle cx="22" cy="24" r="2" fill="#e8b996"/><rect x="20" y="26" width="4" height="9" rx="1" fill="#2a2e34"/><rect x="23" y="24" width="4" height="3" fill="#1a1a1a"/><path d="M20.6 35 v 5 M23.4 35 v 5" stroke="#2a2e34" stroke-width="1.4"/></g>
    <g><circle cx="96" cy="24" r="2" fill="#c99a6a"/><rect x="94" y="26" width="4" height="9" rx="1" fill="#5a4a3a"/><rect x="91" y="24" width="4" height="3" fill="#1a1a1a"/><path d="M94.6 35 v 5 M97.4 35 v 5" stroke="#5a4a3a" stroke-width="1.4"/></g>
    <g><circle cx="12" cy="25" r="2" fill="#e8b996"/><rect x="10" y="27" width="4" height="8" rx="1" fill="#8a2a24"/><rect x="13" y="25" width="4" height="3" fill="#1a1a1a"/><path d="M10.6 35 v 5 M13.4 35 v 5" stroke="#8a2a24" stroke-width="1.4"/></g>
    <path d="M100 40 V 22 h 18 v 18 Z" fill="#e8e2d4"/>
    <path d="M109 22 V 12 M105 12 h 8" stroke="#5a6068" stroke-width="0.8"/>
    <circle cx="109" cy="11" r="2.4" fill="#d8dde2"/>
    <text x="109" y="30" font-family="Arial, sans-serif" font-size="3.2" font-weight="bold" text-anchor="middle" fill="#c8241c" stroke="none">CH.7</text>
    <text x="109" y="34" font-family="Arial, sans-serif" font-size="2.2" text-anchor="middle" fill="#14161a" stroke="none">LIVE</text>
    <circle cx="104" cy="40" r="2" fill="#1a1a1a"/><circle cx="114" cy="40" r="2" fill="#1a1a1a"/>
  </g>
  <g id="part-flashes" stroke="none">
    <circle cx="25" cy="25.5" r="5" fill="#ffffff" opacity="0.7"/><circle cx="25" cy="25.5" r="1.6" fill="#ffffff"/>
    <circle cx="15" cy="26.5" r="4" fill="#ffffff" opacity="0.6"/>
    <circle cx="93" cy="25.5" r="5" fill="#ffffff" opacity="0.7"/><circle cx="93" cy="25.5" r="1.6" fill="#ffffff"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'rival_experimental.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 96 48" width="384" height="192" data-canvas-width="96">
  <!--
    Experimental Sciences Office ship: dented, lab-green, leaking. Anchor (48, 26).
      part-body  the ship
      part-goo   dripping goo (bobs)
  -->
  <defs>
    <linearGradient id="labg" x1="0" y1="0" x2="0" y2="1"><stop offset="0" stop-color="#c8f0b0"/><stop offset="0.55" stop-color="#6fb85a"/><stop offset="1" stop-color="#2f6a2a"/></linearGradient>
  </defs>
  <g id="part-body" stroke="#14261a" stroke-width="0.9" stroke-linejoin="round">
    <path d="M36 22 C 36 12, 41 8, 48 8 C 55 8, 60 12, 60 22 Z" fill="#9fe08a" opacity="0.6"/>
    <path d="M44 9 l 3 6 l -4 4" fill="none" stroke="#e8ffe0" stroke-width="0.6"/>
    <path d="M8 28 C 12 21, 28 18, 48 18 C 64 18, 76 20, 80 22 L 84 21 L 82 25 C 86 26, 88 27, 88 28 C 84 34, 66 37, 48 37 C 30 37, 12 34, 8 28 Z" fill="url(#labg)"/>
    <path d="M26 21 c 3 2, 7 2, 10 0" fill="none" stroke="#2f6a2a" stroke-width="1"/>
    <path d="M60 30 c 2 -2, 6 -2, 7 1 c -2 2, -6 2, -7 -1 Z" fill="#14261a" opacity="0.5"/>
    <rect x="40" y="24" width="16" height="7" fill="#e2b923"/>
    <path d="M44 24 l -3 7 M50 24 l -3 7 M56 24 l -3 7" stroke="#14161a" stroke-width="1.2"/>
    <text x="20" y="31" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#14261a" stroke="none">E.S.O.</text>
    <text x="74" y="31" font-family="Arial, sans-serif" font-size="2.2" font-weight="bold" text-anchor="middle" fill="#14261a" stroke="none">TRIAL 39</text>
  </g>
  <g id="part-goo" stroke="none">
    <path d="M58 36 c 0 3, 2 4, 2 6 c 0 2, -3 2, -3 0 c 0 -2, 1 -3, 1 -6 Z" fill="#9dff7a" opacity="0.9"/>
    <path d="M34 36 c 0 2, 1.4 3, 1.4 4.4 c 0 1.4, -2 1.4, -2 0 c 0 -1.4, 0.6 -2.4, 0.6 -4.4 Z" fill="#9dff7a" opacity="0.9"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'rival_creatures.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 30 24" width="240" height="192" data-canvas-width="30" data-densities="4 8">
  <!--
    Escaped experiments. Each part is its own creature, feet at y 24, facing left.
      part-squirrel  a three-eyed squirrel
      part-cow       a glowing cow
      part-glow      the cow's glow (pulses)
  -->
  <g id="part-squirrel" stroke="#2a1810" stroke-width="0.6" stroke-linejoin="round">
    <path d="M18 23 C 26 22, 27 14, 24 10 C 22 8, 19 9, 20 12 C 22 14, 20 18, 18 23 Z" fill="#7a5aa6"/>
    <ellipse cx="14" cy="19" rx="5" ry="4.6" fill="#8a6ab6"/>
    <circle cx="10" cy="13" r="4" fill="#8a6ab6"/>
    <g fill="#ffffff"><circle cx="8" cy="12" r="1.2"/><circle cx="10.4" cy="11" r="1.2"/><circle cx="12.4" cy="12.4" r="1.2"/></g>
    <g fill="#111" stroke="none"><circle cx="8" cy="12" r="0.5"/><circle cx="10.4" cy="11" r="0.5"/><circle cx="12.4" cy="12.4" r="0.5"/></g>
    <path d="M10 23 h 3 M15 23 h 3" stroke-width="1.2"/>
  </g>
  <g id="part-glow" stroke="none">
    <ellipse cx="15" cy="16" rx="15" ry="9" fill="#9dff7a" opacity="0.35"/>
  </g>
  <g id="part-cow" stroke="#14261a" stroke-width="0.6" stroke-linejoin="round">
    <rect x="9" y="10" width="18" height="9" rx="4" fill="#d8ffd0"/>
    <path d="M12 10 c 3 0, 4 3, 2 5 c -2 1, -4 0, -4 -3 Z M20 13 c 3 -1, 5 1, 4 3 c -2 1, -4 0, -4 -3 Z" fill="#6fdf6a" stroke="none"/>
    <rect x="3" y="7" width="7" height="7" rx="2" fill="#d8ffd0"/>
    <rect x="2" y="11" width="5" height="3" rx="1" fill="#9dffb0"/>
    <circle cx="5.6" cy="9.4" r="0.8" fill="#9dff7a"/>
    <path d="M12 19 v 5 M15 19 v 5 M22 19 v 5 M25 19 v 5" stroke-width="1.6"/>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'rival_tape.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 120 20" width="480" height="80" data-canvas-width="120" data-densities="4 8">
  <!--
    Hazard tape around a breach. Feet at y 20.
      part-body
  -->
  <defs>
    <pattern id="tapeP" width="6" height="3" patternUnits="userSpaceOnUse"><rect width="3" height="3" fill="#e2b923"/><rect x="3" width="3" height="3" fill="#14161a"/></pattern>
  </defs>
  <g id="part-body" stroke="#14161a" stroke-width="0.5">
    <path d="M4 20 V 6 M44 20 V 6 M80 20 V 6 M116 20 V 6" stroke="#c96a2a" stroke-width="1.4"/>
    <path d="M4 8 L 44 10 L 80 8 L 116 10 L 116 13 L 80 11 L 44 13 L 4 11 Z" fill="url(#tapeP)"/>
    <rect x="48" y="0" width="28" height="6" fill="#e9e1c6"/>
    <text x="62" y="4.4" font-family="Arial, sans-serif" font-size="3" font-weight="bold" text-anchor="middle" fill="#c8241c" stroke="none">BIOHAZARD</text>
  </g>
</svg>
''', encoding='utf-8')

(HERE / 'janitor.svg').write_text('''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 32 48" width="256" height="384" data-canvas-width="28">
  <!--
    Cleanup Crew: an alien janitor with a mop and bucket, facing left. Feet at y 47.5, centered on x 16.
      part-body  alien in grey coveralls with a bucket
      part-mop   the mop, swings about the hands (12, 28)
  -->
  <g id="part-body" stroke="#1c2318" stroke-width="0.7" stroke-linejoin="round" stroke-linecap="round">
    <path d="M14 39 L 14 46 M18 39 L 18 46" stroke="#5a6068" stroke-width="2.6"/>
    <path d="M12 46 h 4 v 1.6 h -4 Z M16.6 46 h 4 v 1.6 h -4 Z" fill="#2b2b33"/>
    <path d="M10 19 C 12 18, 20 18, 22 19 L 23 40 L 9 40 Z" fill="#8a9098"/>
    <rect x="12" y="26" width="8" height="4" fill="#e2b923" stroke-width="0.4"/>
    <text x="16" y="29" font-family="Arial, sans-serif" font-size="2.2" font-weight="bold" text-anchor="middle" fill="#14161a" stroke="none">CLEANUP</text>
    <path d="M16 0.6 C 23 0.6, 27 4.5, 27 9 C 27 13.5, 22 17, 17 17.4 C 12 17, 6 14, 6 9 C 6 4.5, 9.5 0.6, 16 0.6 Z" fill="#8fd16a"/>
    <path d="M7.4 9.6 C 8 7.4, 12.4 6.2, 14.8 7.6 C 15 10.4, 11.4 12.6, 7.4 9.6 Z M17.6 7.8 C 19.6 6.6, 23 7.4, 23.6 9.4 C 21 11.8, 18 10.4, 17.6 7.8 Z" fill="#141414"/>
    <path d="M9 14.4 h 3.6" stroke-width="0.6"/>
    <path d="M6 4 C 10 -1, 22 -1, 26 4 L 26 5 L 6 5 Z" fill="#2b5a8a"/>
    <path d="M2 40 h 9 l -1 7 h -7 Z" fill="#c9a224"/>
    <path d="M2.6 41.4 h 7.8" stroke="#8a6a10" stroke-width="0.6"/>
    <ellipse cx="6.5" cy="40" rx="4.5" ry="0.8" fill="#6fa8c8" stroke-width="0.3"/>
  </g>
  <g id="part-mop" stroke="#1c2318" stroke-width="0.6" stroke-linecap="round">
    <path d="M12 28 L 4 46" stroke="#a87a4a" stroke-width="1.4"/>
    <path d="M1 45 C 2 48, 7 48, 8 45 L 6 44 L 3 44 Z" fill="#e8e2d4"/>
    <circle cx="12" cy="28" r="1.2" fill="#8fd16a"/>
    <circle cx="9" cy="34" r="1.2" fill="#8fd16a"/>
  </g>
</svg>
''', encoding='utf-8')
print('ok')
