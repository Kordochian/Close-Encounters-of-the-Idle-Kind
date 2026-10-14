"""Re-lays the underground of mountain.svg as a spread-out bunker and rebuilds hallfx.svg to match."""
import re

DIR = 'F:/Alien_Idle/art/sprites/'
s = open(DIR + 'mountain.svg', encoding='utf-8').read()

# ------------------------------------------------------------------ layout (base-local coordinates)
ROOMS = {  # interior rects drawn by the game (x, y, w, h)
    'hangar': (780, 110, 300, 100),
    'containment': (1110, 330, 305, 100),
    'lab': (520, 480, 330, 115),
    'generator': (1170, 480, 255, 115),
    'break': (690, 680, 605, 115),
    'storage': (560, 320, 288, 106),   # drawn here, in the mountain art
}
# Tunnels: (x0, x1, top); 19 tall, floor grate in the bottom 2.6
TUNNELS = {
    'T1': (848, 1107, 407), 'T1W': (500, 557, 407), 'T1E': (1418, 1440, 411),
    'T2': (853, 1167, 576), 'T2W': (500, 517, 576), 'T2E': (1428, 1440, 576),
    'T3W': (500, 687, 776), 'T3E': (1298, 1440, 776),
}
BOTTOM = 860

# ------------------------------------------------------------------ helpers
def concrete_shell(x, y, w, h, pad=9, open_left=False):
    x0 = x - pad if not open_left else x
    return (f'    <rect x="{x0}" y="{y - pad}" width="{w + pad + (0 if open_left else pad)}" height="{h + 2 * pad}" fill="#4d5058" stroke="#2a2c31" stroke-width="0.8"/>\n'
            f'    <rect x="{x0}" y="{y - pad}" width="{w + pad + (0 if open_left else pad)}" height="{h + 2 * pad}" filter="url(#concrete)" opacity="0.6"/>\n')


def tunnel(x0, x1, top, lamps=()):
    w = x1 - x0
    seams = ' '.join(f'M{x} {top} V {top + 16}' for x in range(int(x0) + 24, int(x1), 48))
    out = [
        f'    <rect x="{x0}" y="{top - 3}" width="{w}" height="25" fill="#4d5058"/>',
        f'    <rect x="{x0}" y="{top}" width="{w}" height="19" fill="url(#wallH)"/>',
        f'    <rect x="{x0}" y="{top}" width="{w}" height="19" filter="url(#concrete)" opacity="0.5"/>',
        f'    <path d="{seams}" stroke="#30343b" stroke-width="0.4"/>' if seams else '',
        f'    <rect x="{x0}" y="{top + 0.5}" width="{w}" height="2.6" fill="url(#pipeH)"/>',
        f'    <rect x="{x0}" y="{top + 3.6}" width="{w}" height="1.4" fill="url(#pipeCu)"/>',
        f'    <rect x="{x0}" y="{top + 16.4}" width="{w}" height="2.6" fill="url(#grate)"/>',
        f'    <path d="M{x0} {top + 16.4} H {x1}" stroke="#5a606a" stroke-width="0.4"/>',
    ]
    for lx in lamps:
        out.append(f'    <path d="M{lx - 4} {top + 1} h 8 l -1 2 h -6 Z" fill="#2a2e34" stroke="#14161a" stroke-width="0.3"/>')
    return '\n'.join(o for o in out if o) + '\n'


def lamp_glow(lx, top):
    return (f'    <path d="M{lx - 4} {top + 3} L {lx - 16} {top + 17} L {lx + 16} {top + 17} L {lx + 4} {top + 3} Z" fill="url(#lampCone)"/>\n'
            f'    <rect x="{lx - 2.6}" y="{top + 2.6}" width="5.2" height="0.8" fill="#fff4cf"/>\n')


def blast_door(x, top, side):
    """A bulkhead frame where a tunnel meets a room; side = 'L' or 'R' tells which way it opens."""
    return (f'    <rect x="{x - 2}" y="{top - 4}" width="4" height="23" fill="#5a6068" stroke="#14161a" stroke-width="0.4"/>\n'
            f'    <rect x="{x - 2}" y="{top - 4}" width="4" height="23" fill="url(#hazardM)" opacity="0.55"/>\n'
            f'    <circle cx="{x}" cy="{top + 8}" r="1.2" fill="#2a2e34" stroke="#8a9098" stroke-width="0.4"/>\n')


# ------------------------------------------------------------------ collect props from the old bands
hall_start = s.index('    <!-- ===================== Hallways ===================== -->')
hall_end = s.index('\n  </g>', hall_start)
old_hall = s[hall_start:hall_end]


def block(comment, nth=0):
    """Text of the nth block that starts with this comment, up to the next comment."""
    idx = -1
    for _ in range(nth + 1):
        idx = old_hall.index('    <!-- ' + comment, idx + 1)
    end = old_hall.index('\n    <!-- ', idx + 1)
    return old_hall[idx:end + 1]


def moved(text, dx, dy):
    return f'    <g transform="translate({dx} {dy})">\n{text}    </g>\n'


storage_text = old_hall[old_hall.index('    <!-- Storage hall under the hangar -->'):old_hall.index('    <!-- Rock wall between the storage hall and the cliff -->')]
A = 403  # old band A top
B = 543  # old band B top
props = []
T = TUNNELS
props.append(moved(block('electrical panel with hanging wires'), 120, T['T1'][2] - A))
props.append(moved(block('poster', 0), 181, T['T1'][2] - A))                      # SMILE
props.append(moved(block('gas cylinders'), 29, T['T1'][2] - A))
props.append(moved(block('fire extinguisher, alien edition'), 52, T['T1'][2] - A))
props.append(moved(block('poster', 1), 194, T['T1'][2] - A))                      # LOOSE LIPS
props.append(moved(block('vending machine, one snack stuck forever'), 66, T['T1'][2] - A))
props.append(moved(block('poster', 2), 146, T['T1'][2] - A))                      # HAVE YOU WIPED
props.append(moved(block('crates, one upside-down'), -166, T['T1W'][2] - A))
props.append(moved(block('drooping wire bundle'), 648, T['T1E'][2] - A))
props.append(moved(block('mop bucket and a wet-floor sign'), 66, T['T2'][2] - A))
props.append(moved(block('poster: employee of the month (nobody)'), -159, T['T2'][2] - A))
props.append(moved(block('vertical pipes with a valve'), -165, T['T2'][2] - A))
props.append(moved(block('second panel'), -178, T['T2'][2] - A))
props.append(moved(block('crates of unused probes'), -118, T['T2'][2] - A))
props.append(moved(block('the last intern'), -132, T['T2'][2] - A))
props.append(moved(block('poster', 6), 44, T['T2'][2] - B))                       # REPORT HUMAN-LIKE
props.append(moved(block('barrels of something glowing'), 30, T['T2'][2] - B))
props.append(moved(block('water cooler, empty'), -179, T['T2W'][2] - B))
props.append(moved(block('poster', 4), -180, T['T3W'][2] - B))                    # MANDATORY FUN
props.append(moved(block('coat rack with spare lab coats'), -182, T['T3W'][2] - B))
props.append(moved(block('stretcher with straps'), -188, T['T3W'][2] - B))
props.append(moved(block('suggestion box, padlocked, overflowing'), -191, T['T3W'][2] - B))
props.append(moved(block('specimen cooler'), -197, T['T3W'][2] - B))
props.append(moved(block('tubes: a manifold of glass tubes with green fluid'), -206, T['T3W'][2] - B))
props.append(moved(block('poster', 5), -220, T['T3W'][2] - B))                   # NOT FOREVER
props.append(moved(block('fuse box with a loose wire'), 352, T['T3E'][2] - B))
props.append(moved(block('stacked chairs'), 295, T['T3E'][2] - B))
props.append(moved(block('poster', 7), 188, T['T3E'][2] - B))                     # MIND THE PROBE

# ------------------------------------------------------------------ new underground
lamps = {'T1': (880, 1040), 'T1W': (528,), 'T2': (900, 1060, 1135), 'T3W': (540, 640), 'T3E': (1370,)}
parts = ['    <!-- ===================== Bunker (base-local; rooms are drawn by the game on top) ===================== -->\n']
# Concrete shells around every room
for name, (x, y, w, h) in ROOMS.items():
    parts.append(concrete_shell(x, y, w, h, open_left=(name == 'hangar')))
# Shafts first so tunnels draw over their ends
parts.append('''    <!-- Cargo lift from the hangar floor down to level 1 -->
    <rect x="924" y="213" width="42" height="194" fill="#4d5058"/>
    <rect x="930" y="213" width="30" height="194" fill="#25282e"/>
    <path d="M932 213 V 407 M958 213 V 407" stroke="#8a9098" stroke-width="1"/>
    <path d="M938 213 V 300 M952 213 V 300" stroke="#14161a" stroke-width="0.6"/>
    <rect x="931" y="300" width="28" height="3" fill="#5a6068" stroke="#14161a" stroke-width="0.4"/>
    <rect x="931" y="300" width="28" height="1" fill="url(#hazardM)"/>
    <g stroke="#2a1d0e" stroke-width="0.3"><rect x="935" y="292" width="9" height="8" fill="url(#crate)"/><rect x="945" y="294" width="7" height="6" fill="url(#crate)"/></g>
    <path d="M936 292 l 8 8 M944 292 l -8 8" stroke="#5a4326" stroke-width="0.3"/>
    <text x="945" y="226" font-family="Arial, sans-serif" font-size="2.4" font-weight="bold" text-anchor="middle" fill="#e2b923">CARGO LIFT</text>
    <text x="945" y="229" font-family="Arial, sans-serif" font-size="1.6" text-anchor="middle" fill="#c9ced6">max 1 cow</text>
    <!-- Pipe bundle from the hangar down to level 1 -->
    <rect x="1058" y="213" width="16" height="194" fill="#4d5058"/>
    <rect x="1061" y="213" width="4" height="194" fill="url(#pipeV)"/><rect x="1066" y="213" width="3" height="194" fill="url(#pipeCu)"/><rect x="1070" y="213" width="2" height="194" fill="#2b5a8a"/>
    <!-- Power conduit from the generator up to containment -->
    <rect x="1296" y="433" width="16" height="44" fill="#4d5058"/>
    <rect x="1299" y="433" width="4" height="44" fill="url(#pipeV)"/><rect x="1305" y="433" width="4" height="44" fill="#c9a224"/>
    <path d="M1299 445 h 10 M1299 465 h 10" stroke="#14161a" stroke-width="0.6"/>
    <!-- Stairwell (west): flights between the three levels -->
    <rect x="464" y="398" width="42" height="400" fill="#4d5058"/>
    <rect x="470" y="404" width="30" height="391" fill="url(#wallH)"/>
    <rect x="470" y="404" width="30" height="391" filter="url(#concrete)" opacity="0.5"/>
    <g fill="none" stroke="#8a9098" stroke-width="1.2">
      <path d="M472 426 L 498 470 L 472 510 L 498 552 L 472 595"/>
      <path d="M472 595 L 498 640 L 472 685 L 498 735 L 472 795"/>
    </g>
    <g fill="none" stroke="#5a606a" stroke-width="0.4">
      <path d="M472 420 L 498 464 L 472 504 L 498 546 L 472 589 L 498 634 L 472 679 L 498 729 L 472 789"/>
    </g>
    <g fill="#5a606a"><rect x="470" y="424" width="30" height="2"/><rect x="470" y="593" width="30" height="2"/><rect x="470" y="793" width="30" height="2"/></g>
    <text x="485" y="418" font-family="Arial Black, Arial, sans-serif" font-size="7" font-weight="bold" text-anchor="middle" fill="#e2b923" opacity="0.5">B1</text>
    <text x="485" y="588" font-family="Arial Black, Arial, sans-serif" font-size="7" font-weight="bold" text-anchor="middle" fill="#e2b923" opacity="0.5">B2</text>
    <text x="485" y="788" font-family="Arial Black, Arial, sans-serif" font-size="7" font-weight="bold" text-anchor="middle" fill="#e2b923" opacity="0.5">B3</text>
    <!-- Elevator shaft (centre): from level 1 down to the break room, car stuck halfway -->
    <rect x="984" y="426" width="36" height="254" fill="#4d5058"/>
    <rect x="990" y="426" width="24" height="254" fill="#22252a"/>
    <path d="M992 426 V 680 M1012 426 V 680" stroke="#8a9098" stroke-width="0.9"/>
    <path d="M1000 426 V 612 M1004 426 V 612" stroke="#14161a" stroke-width="0.5"/>
    <rect x="991" y="612" width="22" height="26" fill="#5a6068" stroke="#14161a" stroke-width="0.5"/>
    <path d="M1002 612 v 26" stroke="#14161a" stroke-width="0.5"/>
    <rect x="993" y="620" width="18" height="5" fill="#e9e1c6" transform="rotate(-6 1002 622.5)"/>
    <text x="1002" y="623.6" font-family="Arial, sans-serif" font-size="1.9" font-weight="bold" text-anchor="middle" fill="#b0261e" transform="rotate(-6 1002 622.5)">OUT OF ORDER</text>
    <text x="1002" y="634" font-family="Arial, sans-serif" font-size="1.4" text-anchor="middle" fill="#c9ced6">since 1987</text>
    <!-- Ladder shaft (east): containment, generator and the break room level -->
    <rect x="1434" y="402" width="28" height="396" fill="#4d5058"/>
    <rect x="1440" y="408" width="16" height="387" fill="#22252a"/>
    <path d="M1442 408 V 795 M1454 408 V 795" stroke="#8a9098" stroke-width="0.8"/>
    <path d="''' + ' '.join(f'M1442 {y} H 1454' for y in range(412, 795, 5)) + '''" stroke="#8a9098" stroke-width="0.6"/>
    <path d="''' + ' '.join(f'M1440 {y} C 1440 {y - 6}, 1456 {y - 6}, 1456 {y}' for y in range(440, 795, 30)) + '''" fill="none" stroke="#5a606a" stroke-width="0.5"/>
''')
# Tunnels and their lamps
for name, (x0, x1, top) in TUNNELS.items():
    parts.append(f'    <!-- Tunnel {name} -->\n' + tunnel(x0, x1, top, lamps.get(name, ())))
# Storage hall moved into its new shell
parts.append(moved(storage_text, ROOMS['storage'][0] - 672, ROOMS['storage'][1] - 297))
# Blast doors where tunnels meet rooms
for x, top in [(848, 407), (1107, 407), (557, 407), (1418, 411), (853, 576), (1167, 576), (517, 576), (1428, 576), (687, 776), (1298, 776)]:
    parts.append(blast_door(x, top, 'L'))
# Hatches where shafts meet tunnels
parts.append('''    <rect x="930" y="403" width="30" height="2.6" fill="url(#hazardM)"/>
    <rect x="990" y="423.4" width="24" height="2.6" fill="url(#hazardM)"/>
    <rect x="990" y="592.4" width="24" height="2.6" fill="url(#hazardM)"/>
''')
parts.extend(props)
new_hall = ''.join(parts)
s = s[:hall_start] + new_hall + s[hall_end:]

# Map goes deeper: mountain frame to y 860
s = s.replace('viewBox="240 -300 1460 1000" width="2190" height="1500"', 'viewBox="240 -300 1460 1160" width="2190" height="1740"')
s = s.replace('<rect x="240" y="-300" width="1460" height="1000"', '<rect x="240" y="-300" width="1460" height="1160"')
s = s.replace(' L 1700 700 Z', f' L 1700 {BOTTOM} Z').replace('M240 700 L 240 302', f'M240 {BOTTOM} L 240 302')
# Fossil and saucer move clear of the new rooms
for start, end, t in [('<!-- Underground, left:', '<!-- Underground, right:', 'translate(-110 170)'),
                      ('<!-- Underground, right:', '<!-- Roots and a worm', 'translate(20 230)')]:
    a = s.index(start); a = s.rindex('\n', 0, a) + 1
    b = s.index(end); b = s.rindex('\n', 0, b) + 1
    s = s[:a] + f'      <g transform="{t}">\n' + s[a:b] + '      </g>\n' + s[b:]
s = s.replace('lab (672..1008, 422..543), generator (1022..1283, 422..543), break room (672..1283, 562..683)',
              'lab (517..853, 477..598), generator (1167..1428, 477..598), break room (687..1298, 677..798)')
s = s.replace('The game draws on top: hangar (777..1083, 107..213), containment (972..1283, 297..403),',
              'The game draws on top (spread out like a bunker, 2026-10-14): hangar (777..1083, 107..213), containment (1107..1418, 327..433),')
open(DIR + 'mountain.svg', 'w', encoding='utf-8', newline='').write(s)

# ------------------------------------------------------------------ hallfx.svg
fx = open(DIR + 'hallfx.svg', encoding='utf-8').read()
defs = fx[fx.index('  <defs>'):fx.index('  </defs>') + len('  </defs>')]
lights = []
for name, (x0, x1, top) in TUNNELS.items():
    for lx in lamps.get(name, ()):
        lights.append(lamp_glow(lx, top))
st = (ROOMS['storage'][0] - 672, ROOMS['storage'][1] - 297)
for lx in (706, 796, 886):  # storage lamps
    x, y = lx + 4 + st[0], 305 + st[1]
    lights.append(f'    <path d="M{x - 4} {y} L {x - 30} {y + 93} L {x + 30} {y + 93} L {x + 4} {y} Z" fill="url(#lampCone)" opacity="0.7"/>\n'
                  f'    <rect x="{x - 2.6}" y="{y - 0.4}" width="5.2" height="0.8" fill="#fff4cf"/>\n')
for x, y in [(485, 470), (485, 650), (1448, 520), (1448, 700), (1002, 470)]:  # shaft lamps
    lights.append(f'    <rect x="{x - 3}" y="{y - 2}" width="6" height="2" fill="#2a2e34"/>\n'
                  f'    <path d="M{x - 3} {y} L {x - 8} {y + 22} L {x + 8} {y + 22} L {x + 3} {y} Z" fill="url(#lampCone)"/>\n')
leds = [(863, 414.6, '#4fff6a'), (869, 414.6, '#ff4a3a'), (872, 414.6, '#4fff6a'),
        (943, 583.6, '#ffd23a'), (949, 583.6, '#4fff6a'),
        (1313, 782.6, '#ff4a3a'), (1319, 782.6, '#4fff6a'), (1057.8, 413, '#ff4a3a')]
led_svg = ''.join(f'    <circle cx="{x}" cy="{y}" r="0.6" fill="{c}"/>\n    <circle cx="{x}" cy="{y}" r="1.6" fill="#ffffff" opacity="0.3"/>\n' for x, y, c in leds)
spark = (1324, 789.4)
fx_new = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="460 205 1000 600" width="2000" height="1200" data-canvas-width="1000" data-densities="2 3">
  <!--
    Animated bunker effects, drawn over the mountain at base-local (460, 205), 1000x600.
      part-lights  tunnel, shaft and storage lamp glows, brown out together
      part-leds    blinking LEDs on the electrical panels and the vending machine
      part-spark   a loose wire sparking by the fuse box ({spark[0]}, {spark[1]})
  -->
{defs}
  <g id="part-lights">
{''.join(lights)}  </g>
  <g id="part-leds">
{led_svg}  </g>
  <g id="part-spark" stroke-linecap="round">
    <circle cx="{spark[0]}" cy="{spark[1]}" r="2.4" fill="#ffe680" opacity="0.35"/>
    <path d="M{spark[0]} {spark[1]} l -1.8 1.4 M{spark[0]} {spark[1]} l 2 0.8 M{spark[0]} {spark[1]} l 0.4 2 M{spark[0]} {spark[1]} l -0.6 -1.6" stroke="#ffd84a" stroke-width="0.45"/>
    <circle cx="{spark[0]}" cy="{spark[1]}" r="0.5" fill="#fffbe0"/>
  </g>
</svg>
'''
open(DIR + 'hallfx.svg', 'w', encoding='utf-8', newline='').write(fx_new)
print('ok')
