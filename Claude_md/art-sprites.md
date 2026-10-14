# Art Sprites

Last updated: 2026-10-14 (bunker layout: rooms spread over 3 levels; verified in game at 900p)

Style: cartoon sci-fi, dark comedy. Thick dark outlines (#1d1a20 / #2a1810), flat fills with a
light gradient, small sad details (rust, duct tape, missing rivets, bloodshot eyes, bandages).

## Where in the source

- `F:\Alien_Idle\art\sprites\*.svg` — hand-written SVG sources. Each `<g id="part-NAME">` is one
  animatable part; each SVG's header comment lists its parts, pivots and areas the game draws over.
- `F:\Alien_Idle\art\sprites\render_sprites.py` — renders every part to full-frame PNGs
  `Content/Sprites/<sprite>_<part>@<D>.png` via headless Edge (`python -I render_sprites.py [sprite ...]`).
  D = pixels per canvas unit. Each SVG root carries `data-canvas-width` (how wide it is drawn on the
  1300x700 canvas) and optional `data-densities` (default "2 4 8"; rooms and smoke use "2 4").
- `F:\Alien_Idle\AlienIdle\Content\Sprites\*.png` — the PNGs the game loads from disk at runtime
  (plain files, not .uasset imports). The game loads every PNG in the folder by base name.
- `Source/AlienIdle/UI/AlienSprites.h/.cpp` — `FAlienSprites`: `Load`, `Has*`, `DrawSquirrel`,
  `DrawCow`, `DrawAlien`, `PoseUfo` + `DrawUfo` (`FUfoPose::ToCanvas` maps a sprite-frame point onto
  the tilted hull), `DrawRoom(ERoomSprite)`, `DrawPuffs` (smoke/steam). All motion math lives here.
  Parts are drawn with `FSlateDrawElement::MakeRotatedBox` (pivot = SVG units x draw scale, local to
  the part box). Missing parts → `Has*()` false → the base view's placeholder shapes.
  Resolution: `Load` reads every `name@D.png` (the PNG header width gives the nominal size);
  `Part()` picks the smallest D >= 0.85 x the screen's pixels per canvas unit
  (`FSpriteCanvas::PixelsPerUnit`, set by the painter from its scale x the geometry's layout scale).
  Slate dynamic textures have no mipmaps, so this avoids shimmer from heavy downscaling.
- `Source/AlienIdle/UI/SAlienBaseView.cpp` — owns `FAlienSprites Sprites`. Hooks: squirrel/cow cases in
  `FCanvasPainter::Specimen`, `FCanvasPainter::Alien`, `FCanvasPainter::Room(..., ERoomSprite)`, the
  hangar block, and the UFO loop (`bSprite`, `Pose`, `OnHull` for cargo, pods and lights).
- `docs/screenshots/bunker-whole.png` (AlienIdle.FitWholeMap 1), `bunker-default.png`, `bunker.gif`,
  `props-on-fire.gif` — latest in-game captures. Always launch captures with `-AlienFresh` (the game
  otherwise loads and autosaves George's real save). Capture at 1600x900: 2560x1440 windows on this PC
  open with the view in the wrong place (window larger than the screen).
- The prototype thread added zoom (wheel, up to 4x) and pan to the base view; `PixelsPerUnit` includes
  the zoom, so zooming in switches to the sharper PNGs automatically.

## Sprites, parts and animation

| Sprite | Frame (SVG units) | Drawn at (canvas) | Parts, back to front |
|---|---|---|---|
| `squirrel` | 32x32, feet bottom | 24x24 | tail, body, head, pupils, arms |
| `cow` | 48x32, feet y 31.5 | 36x24 | tail, body, jaw, head, lids (blink), tag (blink) |
| `alien` | 32x48, feet bottom, collar y 18 | 28x42 | body, head, blink, arm |
| `ufo_old_reliable` | 96x48, anchor (48,26) | 96x48 | wire, spark, antenna, body (+ smoke) |
| `ufo_second_opinion` | same | 96x48 | plate, body, siren |
| `ufo_plausible_deniability` | same | 96x48 | balloon, body, propa/propb |
| `room_hangar` | 300x100 at (660,300) | 1:1 | back, beacon |
| `room_containment` | 305x100 at (975,300) | 1:1 | back, bulb |
| `room_lab` | 330x115 at (675,425) | 1:1 | back, light |
| `room_generator` | 255x115 at (1025,425) | 1:1 | back, needle (+ steam puffs) |
| `room_breakroom` | 605x115 at (675,565) | 1:1 | back, hand (+ coffee steam) |
| `smoke` | 16x16 | any | puff |
| `mountain` | 1460x1160 at base-local (240,-300) = canvas (640,-300); viewBox uses base-local coords | 1:1, densities 1.5 2.5 | back (one static part) |
| `hallfx` | 1000x600 at base-local (460,205) | 1:1, densities 2 3 | lights, leds, spark |
| `cabin` | 140x56 at (780,40), canvas coords | 1:1, densities 4 8 | back, glow, oldman / oldman_watch, dog_tail, dog, dog_head (+ chimney smoke) |
| `prop_sign_gohome` | 24x24 at (786,68.5) | 1:1, densities 4 8 | base / burnt, snow, fire_a / fire_b |
| `prop_sign_keepout` | 12x9 at (871.5,76.5), on the cabin wall | 1:1 | same |
| `prop_mailbox` | 14x16 at (902,76.4) | 1:1 | same |

Detail pass (2026-10-14, George asked for higher definition): gradients and cel shading, rim lights,
procedural grime/fur/hide texture via SVG `feTurbulence` filters clipped to shapes, panel seams,
rivets, chrome bezels, fur strands, glossy eyes, extra props per room. Room props: hangar fuel drums
("FUEL?"), tool cart, work lamps, "BAY 7" stencil; containment one-way window with eyes, tally marks,
bowl #0042, a lone sock; lab monitor "BPM 3", eye-wash station, annotated poster; generator hamster
wheel "BACKUP", "MK II (MK I EXPLODED)", "PSI?" gauge, dripping pipe; break room peeling wallpaper,
lamp pools, clock moved to (425, 14) beside the prototype thread's "MEMORY-WIPE STATION" label.
Crew badges (prototype thread) sit over the room tops: hangar y 24 per ship, lab (60/152/244, 27),
containment (230..290, 7..17), generator (210, 12), break room (550..600, 16..30). Keep art clear there.

- Squirrel: nervous breathing, tail swish + flicks, head scans then twitches, pupils dart.
- Cow: tail swats, slow head bob, jaw chews side to side, rare blink, ear tracking tag blinks green.
- Alien: lab coat, coffee; nods; every ~8 s a long sip (arm lifts, head tips back, eyes close); blinks.
  The management tie (prototype thread, `P.Box(698, 494, ...)`) lands on its shirt.
- UFOs (engine on = flying or capturing; docked = still): Old Reliable sways, coughs every ~3 s
  (dips, drops, black puff, lights die), exhaust trail, swinging wire + spark, wobbly antenna, light 3
  dying. Second Opinion: mint clinic saucer, gentle wallow, orange siren pulses, learner plate swings.
  Plausible Deniability: matte black, "WEATHER BALLOON" stencil (peeling), tethered balloon trails,
  fake propeller alternates two frames. Code-drawn upgrades follow the pose position but don't tilt.
- Mountain (2026-10-14; reshaped several times that day at George's request: taller, hangar near the
  top, "more nature, no sharp drop-off", map expanded upward, then wider with the base centered).
  BASE-LOCAL COORDINATES: the mountain, rooms, hallways, cabin, props, banners and crew badges are all
  authored in the old 1300-wide layout and drawn with `P.Shift` = `BaseShift` (+400 x) in
  SAlienBaseView.cpp, so local x 240 = canvas 640. All mountain/cabin/hallfx/prop coordinates in
  this note are base-local. The map is canvas x 0..2100, y -300..700 (see prototype-build.md).
  The mountain spans local 240..1700 with feet at 260 and 1684, centered on the base (660..1283).
  A long gentle slope with boulders, scree, pines and a zigzag footpath climbs from the meadow to a
  hazard-striped landing apron at the hangar mouth (780, 107..213; `HangarX` 780 / `HangarY` 110).
  Above the mouth: overhang lip with icicles, the cliff-top plateau (790..915, y 92) with the cabin,
  a ridge trail up the steep left flank to the snowy peak (1105, -236). The right flank descends past
  pines and an antenna on its shoulder (1440, 30) with a "DEFINITELY A CELL TOWER" sign, to a meadow
  at the right foot. Underground: a fossilised cow (left, ~460, 480) and a buried saucer marked 1947
  with its pilot (right, ~1535, 505); roots and a worm near the surface. Strata, cracks, ore vein.
  BUNKER LAYOUT (George, 2026-10-14: "spread the rooms out ... underground bunker"). Three levels in
  base-local coordinates, rooms set in concrete shells with rock between them:
  hangar (780,110) on the slope; B1: storage hall (560..848 x 320..426, drawn in the mountain art) and
  containment (1110,330); B2: lab (520,480) and generator (1170,480); B3: break room (690,680).
  The game draws rooms at their original coordinates plus `ContainmentOffset` (135,30), `LabOffset`
  (-155,55), `GeneratorOffset` (145,55), `BreakRoomOffset` (15,115) via `P.Shift`; crew badges, the
  meeting banner and the MEMORY-WIPE label use the same offsets. Map bottom is now y 860.
  Connections (all in mountain.svg, generated by a script; tunnels are 19 tall with blast doors):
  cargo lift (930..960, 213..407, "max 1 cow") hangar -> B1; pipe bundle (1058..1074) hangar -> B1;
  tunnels T1 (848..1107, y 407), T1W (500..557), T1E (1418..1440, y 411), T2 (853..1167, y 576),
  T2W (500..517), T2E (1428..1440), T3W (500..687, y 776), T3E (1298..1440, y 776); west stairwell
  (470..500, 404..795, landings stencilled B1/B2/B3); central elevator shaft (990..1014, 426..680)
  with the car stuck halfway ("OUT OF ORDER since 1987"); east ladder shaft (1440..1456, 408..795);
  power conduit generator -> containment (1296..1312, 433..477). The fossil cow moved to ~(350, 650)
  and the 1947 saucer to ~(1555, 735). Tunnel props (moved from the old bands):
  crates ("THIS SIDE UP" upside down), electrical panels with blinking LEDs, wire bundles, pipes, gas
  cylinders, glass tubes, mop + wet sign, vending machine, water cooler, stretcher, padlocked IDEAS box,
  "DO NOT EAT" cooler, glowing barrels, stairs/elevator ("OUT OF ORDER"), the last intern's skeleton
  ("REVIEW PENDING"), fuse box with a sparking wire, posters (SMILE you're undercover, LOOSE LIPS SINK
  MOTHERSHIPS, HAVE YOU WIPED A MEMORY TODAY?, EMPLOYEE OF THE MONTH ?, MANDATORY FUN, REPORT HUMAN-LIKE
  BEHAVIOUR, MIND THE PROBE). Lamps brown out together now and then.
  Cabin (cabin.svg, authored at its first spot and shifted by translate(-50 -58)): log walls, "I KNOW" painted on the roof, tinfoil antenna,
  TV flicker in the window, chimney smoke from (876.5, 48). Old man in
  a tinfoil hat rocks in his chair; while any UFO is out (`bUfoOut`, computed in the base view) he
  raises binoculars and the scruffy dog lifts its head and wags fast (verified in game). Pivots:
  rocker (825, 92), dog tail (898, 89), dog head (892, 88).
- Mountain props (George, 2026-10-14: "so we can add snow, set them on fire, etc."): the GO HOME sign
  (on the plateau edge, aimed down at the hangar mouth below), the KEEP OUT (esp. you) plate and the mailbox
  (J. HOLLIS, flag up) are separate sprites, drawn by `DrawProp` from `DrawMountain`. Each has parts
  base, burnt (replaces base), snow (overlay), fire_a/fire_b (alternating flames, plus black smoke
  puffs). State: `FAlienSprites::SetPropState(EMountainProp, FPropState{bSnow, bOnFire, bBurnt})`,
  meant to be driven later by seasons/weather/catastrophes. Console preview:
  `AlienIdle.MountainProps 0..4` (0 game state, 1 snow, 2 fire, 3 burnt, 4 burnt + snow);
  e.g. `-ExecCmds="AlienDemo,AlienIdle.MountainProps 2"`. Capture: docs/screenshots/props-on-fire.gif.
- Rooms: hangar amber beacon flashes, "days without an incident: 0" sign; containment padded cell,
  caged bulb stutters and blacks out; lab tiles, jars with eyeballs, "HUMAN (?)" poster, tube light
  flicker bursts; generator pipes, leaking valve steam, gauge needle jitters and slams into red,
  "DO NOT LICK (AGAIN)"; break room "HANG IN THERE" cow poster, ERR 7 coffee machine with steam, dead
  plant, clock running backwards. Room labels and all game content are still drawn by code on top.

## Research department visuals (2026-10-14, handed over by the prototype thread)

All drawn from `SAlienBaseView::OnPaint`, which reads `S.GetPrimaryDepartment()`, `S.IsProjectDone(...)`,
`S.IsSignatureRunning/Done(d)` and `FLabChamber::Batch`. Preview without touching the sim:
`AlienIdle.PreviewDepartment 1|2|3` (Biological / Human / Incompetence: uniform, all three projects'
equipment, that department's trophy); add 10 (11/12/13) to also show its signature experiment running.

- Uniform of the lab alien (`EAlienUniform`, `DrawAlien(..., Uniform)`): alien.svg parts body_bio + arm_bio
  (green lab coat), body_human + arm_clipboard (grey suit, clipboard instead of coffee), body_incomp +
  arm_incomp + hardhat (scorched overalls, hard hat; the hat rotates with the head).
- Biological: Reinforced Habitats -> `room_containment_bars` drawn over the specimens; Heavy Tractor Array
  -> code-drawn emitter 1.5x wider with a glowing ring on every ship; Batch Experiments -> a second tier
  under every exam table, and chambers with `Batch >= 2` show two specimens (x +22 and +58).
- Human: Advanced Scanners -> taller mast and 1.8x dish on ships (code-drawn); Behavioral Lab ->
  `room_lab_mirror` (two-way mirror with silhouettes, "OBSERVATION. ACT NATURAL."); Memory Manipulation
  -> `room_breakroom_helmet` + blinking `helmet_lights` (scrambler hanging over the memory-wipe station).
- Incompetence: Experimental Modules -> `ufo_modules.svg` (bolted-on BETA box with a spinning fan and a
  coffee-can exhaust) on all three ships via `DrawUfo(..., bModules)`; Unusual Power -> two more hamster
  wheels (`hamsters_a/b`) and random coil arcs (`sparks_a/b`) in the generator; Automated Cover Stories ->
  `room_breakroom_printer` ("COVER-O-MATIC" on the coffee machine) spewing "AMBITIOUS OWL" memos
  (`printer_a/b`).
- Signatures: running -> `room_lab_signature` strobes the lab in the department colour (green / blue /
  orange); done -> `trophy_shelf` above the bulletin board with a golden cow (Bio), golden brain in a jar
  (Human), golden empty fire extinguisher (Incompetence).
- Generic helper: `FAlienSprites::DrawRoomPart(Canvas, Room, "part", X, Y, Tint)` draws any extra part of a
  room sprite in that room's frame.

## Metropolis (second location, 2026-10-14, requested by the prototype thread)

- Generators kept in art/sprites: `city_art.py` (rerun after bunker changes) and `bunker_layout.py`
  (one-off: it expected the pre-bunker mountain.svg; kept as a record of the layout).
- `city.svg` (built by a script from mountain.svg's bunker block, so regenerate it after changing the
  bunker): same 1460x1160 frame and base-local coordinates as the mountain. Night skyline (far and near
  buildings with lit windows), Metropolis Tower as the HQ disguised as "NORMAL INSURANCE CO. - staffed
  entirely by humans since 1947", the hangar as a tower floor with a "WINDOW CLEANING platform (do not
  ask)" ledge at its open bay, a lobby with a bronze abducted-cow sculpture, a street with streetlights,
  parked cars, a hot dog cart and a "NOTHING IS HAPPENING" billboard; underground bedrock, utility lines,
  a sewer ("do not feed the alligator"), the subway "LINE 7 - NOT IN SERVICE", and the same bunker.
- `DrawMountain(..., bCity)` draws `city_back` instead of `mountain_back` and skips the cabin, old man,
  dog and props (they stay on the mountain). Hallway effects are shared.
- Base view: `Location` = `S.GetLocation()`, zone labels use `S.GetRegionName(Type)` (Central Park,
  Petting Zoo, City Zoo, Downtown in the city). Preview: `AlienIdle.PreviewLocation 1`.
- New occupation colours: Politician navy, Influencer hot pink (holding a recording phone),
  Conspiracy Theorist olive (tinfoil hat), Celebrity gold (sunglasses at night).

## Hunting regions and Earth's reactions (2026-10-14, "Earth Notices" milestone, from the prototype thread)

- The left side of the map is three regions per location (`RegionWidth` 220; region 0 nearest the base at
  canvas x 440..660, region 2 at 0..220), replacing the four specimen zones. UFOs fly to
  `RegionCenterX(Ufo.TripRegion)`.
- Backdrops (`regions_art.py` generates the SVGs): region_mountain_0 Pine Ridge Valley (forest, creek, barn,
  pasture), _1 Glenwhit (town, "EAT" diner with "PIE OF THE DAY: UNKNOWN", water tower "pop. 312 (and
  falling)"), _2 County Fairgrounds (big top, a Ferris wheel that turns, funnel cake); region_city_0 Central
  Park (fountain, benches, "DO NOT FEED THE SQUIRRELS (THEY TALK)"), _1 The Suburbs (identical houses,
  flamingos, "HOA: NO LIGHTS IN THE SKY"), _2 Downtown (towers, flickering neon). Frame 220x210 at
  (x0, 100); ground at local y 200. `DrawRegion`.
- Each region draws its unlocked specimens loitering, its active responses (`DrawResponse`): night patrols
  (farmer, thermos, sweeping flashlight), investigators ("NOT THE FBI" van, red-string corkboard), radar
  (dish nodding across the sky), support group (folding chairs, "I WAS TAKEN"); labels list active
  responses in orange and responses with pressure >= 0.5 as "(organising)".
- Earth condition jokes in every region (`DrawCondition`): documentary crew, agency van "DEFINITELY NOT
  SURVEILLANCE" with a panning camera, "ABDUCTED? You may be owed compensation!" billboard.
- Rival (Efficiency Bureau): while a sweep is announced or running, a "RESERVED - BUREAU VEHICLES ONLY"
  queue barrier stands in `SweepRegion`; while sweeping, the grey boxy Bureau ship (clipboard logo, FORM
  7-B) hovers over it with a grey scanning beam and the label "EFFICIENCY BUREAU: sweeping <specimen>s".
  Sprites rival_bureau.svg / rival_sign.svg (`rival_art.py`).
- Rivals phase 2 (`rival2_art.py`): Celebrity Studies stunt (`S.IsCelebrityStunt(r)`) -> red carpet,
  velvet ropes, paparazzi with random flashes and a "CH.7 LIVE" news van in the region, plus a glossy pink
  "CSD" ship with sparkles and a spotlight ("CELEBRITY STUDIES: live from the red carpet"). Experimental
  Sciences breach (`S.IsBreach(r)`) -> dented lab-green "E.S.O. TRIAL 39" ship dripping goo, a
  three-eyed purple squirrel and a glowing cow wandering the region; BIOHAZARD tape while the breach is
  announced or active. Cleanup Crew (`EBaseUpgrade::Cleanup` bought) -> `janitor.svg` alien mopping in
  front of the break-room bulletin board.
- Approach badge ("Every Build Has a Purpose"): a dark plate on the hangar wall reading "APPROACH: BULK /
  PRECISION / COVERT" (orange / cyan / purple) from `S.ClassifyApproach(&bClear)`, or "APPROACH:
  UNDECIDED" in grey when not clear. Drawn in the hangar block of SAlienBaseView.cpp.
- Previews: `AlienIdle.PreviewRival` 1..3 Bureau, 11..13 Celebrity stunt, 21..23 breach (any preview also
  shows the janitor). `AlienIdle.PreviewEarth 1..4` (all responses everywhere; condition 1 usual, 2 documentary, 3
  surveillance, 4 advertising), `AlienIdle.PreviewRival 1..3` (Bureau sweeping that region),
  `AlienIdle.ViewX 600` centres the view on the hunting grounds for screenshots.
- Capture gotcha: if another Unreal instance (e.g. the prototype thread's headless tests) is running,
  a captured game instance can die at startup in the DerivedDataCache. Wait until no UnrealEditor runs.

## Oceanic (Trench Station 9, 2026-10-14, from the prototype thread)

- `ocean_art.py` builds `ocean.svg` (copies the bunker from mountain.svg, swapping the storage hall for
  Pressure Control: console, red valve, periscope, "KEEP CALM AND EQUALIZE", the big HULL gauge), the three
  regions and `leak.svg`. `ocean_specimens_art.py` builds pufferfish/octopus/whale.
- ocean.svg: sea with light rays, fish, bubbles, kelp and coral; sand floor; a glass dome with an AIRLOCK
  around the hangar ("TRENCH STATION 9 - a perfectly normal research reef"). Parts: turbine_a/b (Tidal
  Turbines; spin 40 + 280 x tide deg/s), tanks (Deep Tanks), cracks (hull pressure 60+, flicker; the base
  view adds "*creeeak*"), needle (hull gauge, -120..+120 deg for 0..100, jitters at 60+).
  `DrawMountain(..., Location 2, FOceanLook)`; whale song (`EIncident::WhaleSong`) draws a big whale with
  rising notes beside the dome.
- Regions: region_ocean_0 Coral Shallows (reef, "SUNKEN CHEST (empty, sorry)"), _1 Harbour Town (on land:
  yachts, lighthouse whose beam sweeps, YACHT CLUB, FISH MARKET "no questions asked"), _2 The Trench (dark
  cliffs, bioluminescence, an anglerfish lure that pulses, "depth: yes"). Regions 0 and 2 get a blue
  water tint over their sky (the trench darker).
- Specimens (`DrawSeaCreature`): pufferfish hovers and puffs up now and then; octopus wriggles (two arm
  frames, judging eyes); whale 96x35, floats above the floor and sways its tail. Whales (like elephants)
  are not drawn in the UFO dome.
- Pressure Control technician: the lab alien sprite at the console (base-local 645, 421); crew badge
  anchor for `EStation::Pressure` at (640, 335).
- Airlock sealed (`S.IsAirlockSealed()`): `leak.svg` in the hangar: ceiling drip into a bucket and the
  poster "EVERY DROP IS A LEARNING OPPORTUNITY".
- Previews: `AlienIdle.PreviewLocation 2` (Oceanic; previewing a location also shows every specimen),
  `AlienIdle.PreviewOcean 1` (turbines at high tide, tanks, pressure 80, leak, whale song).

## Antarctic (Station Permafrost, 2026-10-14, from the prototype thread)

- `antarctic_art.py` builds everything: penguin/seal/elephantseal, `ice.svg` (bunker copied from
  mountain.svg with the storage hall swapped for the Boiler Room: furnace, coal pile, HEAT gauge, "FEED THE
  BOILER / it is the only one that loves you"), `frost.svg`, `cryopods.svg`, `stoker.svg` (janitor.svg with
  a shovel) and region_ice_0..2. Rerun it after bunker changes in mountain.svg.
- ice.svg: ice shelf at y ~250, icebergs, the hangar is a snow-roofed hut on stilts ("STATION PERMAFROST -
  definitely a weather station"), flag, snowmobile, "OUTSIDE TEMP: NO" sign. Parts: aurora (shimmers),
  furnace_glow (alpha = heat supply / demand, flickers), taps (Geothermal Taps; the code puffs steam from
  1250,234 and 1330,238), vault (Cryo Vault). `DrawMountain(..., Location 3, FOceanLook, FIceLook)`.
- Regions: region_ice_0 The Ice Shelf ("nothing for 1,200 km in any direction"), _1 Research Outpost
  (huts, "12 SCIENTISTS / 1 COOK (TIRED)", part-dish turns), _2 Elephant Seal Beach (shingle, surf, "smell
  advisory in effect"). Ground under the regions is ice blue with a snow strip.
- Specimens (`DrawIceCreature` 0 penguin 14x18 waddles and flaps, 1 seal 32x16, 2 elephant seal 72x32 whose
  nose wobbles when it bellows). Elephant seals are not drawn in the UFO dome.
- PolarResearcher: drawn as boxes, three coats (blue, green, orange) with a fur hood and red scarf.
- Boiler Room: stoker shovels between the coal pile and furnace at base-local (652, 379); crew badge anchor
  `EStation::Boiler` (600, 335). HeatingCut shows "HEATING BUDGET CUT" by the gauge.
- Heat: `frost_frost` over the hangar (300x100) and lab (330x115) with alpha 1 - warmth (labels get a dark
  plate); `frost_melt` (drips, puddles, ticker "scientists blame unexplained Antarctic heatwave on cow
  flatulence") on the lab when supply > 1.6 x demand. Any base: `frost_cladding` on the lab for
  InsulatedLabs, `cryopods` at the back of containment (1222, 358 containment-local) for CryoPods.
- Previews: `AlienIdle.PreviewLocation 3`; `AlienIdle.PreviewIce 1` freezing + heating cut, `2` overheated;
  both also show taps, vault, cryo pods and insulated labs. Screenshots: docs/screenshots/antarctic-*.png,
  antarctic.gif.

## Gotchas

- Load sprite files with `FSlateDynamicImageBrush(FName(FullPath), Size)`. A plain `FSlateImageBrush`
  with a file path is only resolved for style-set brushes and draws as a solid white box in game.
- At the game's 1600x900 window the UFO is ~90 px wide, so fine details read as texture, not detail.

## Re-render

Edit the SVG, then `python -I F:\Alien_Idle\art\sprites\render_sprites.py` (no Inkscape or
ImageMagick on this PC; it drives headless Edge). PNG size = the SVG's width/height attributes
(4x the viewBox for the UFO, 8x for the squirrel). Each Edge call needs its own `--user-data-dir`
or it hands off to the previous instance and writes nothing; Edge also sometimes exits without
writing, so the script deletes the sprite's old PNGs first and retries until each file appears.
A full render of all sprites (~100 PNGs) takes a few minutes. No rebuild is needed after
re-rendering; the game reads the PNGs at startup. New parts
also need a line in `AlienSprites.cpp` (part list + draw call).

## Not yet

- Packaging: loose PNGs under Content are not cooked; add `+DirectoriesToAlwaysStageAsNonUFS` or
  import as textures before shipping.
- Sprites for elephants, humans, the hunting grounds (houses, circus, barn, trees), sky,
  containment floor tiles, lab tables, generator coil.

## Animation direction (agreed with George 2026-10-14)

George wants animated sprites and backgrounds that change with seasons, weather and catastrophes.
Proposal: SVG stays the source; Unreal can't render SVG at runtime. Split sprites into named parts
rendered as separate PNGs and animate them in code (cut-out puppets), use short flipbooks only
for effects (explosions, smoke), and build backgrounds as layered PNGs. Seasons swap or tint
layers, weather (rain, snow, fog, lightning) is drawn in code on top, and catastrophes add
overlay layers. Spine or Rive could come later if puppets aren't enough.
All three UFOs, squirrel, cow, lab alien, the five rooms and the mountain (hallways, cabin) are puppets (done). Seasons/weather/catastrophe
backgrounds are not started.
