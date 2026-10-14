# Project Status: Close Encounters of the Idle Kind

Last updated: 2026-10-14

## The plan (George's 8 steps, mountain base only)

First milestone: start with a battered UFO, reach human experiments, submit a thesis, begin a
second run with a meaningful new choice.

| # | Step | State |
|---|---|---|
| 1 | Decide presentation and platform | Done: UE 5.8.3 C++, 2D Slate cutaway, PC first, gamepad-friendly |
| 2 | One-page prototype spec | Done: `docs/prototype-spec.md` |
| 3 | One complete squirrel expedition + visible upgrade | Done (thrusters), see prototype-build.md |
| 4 | Cows/elephants/humans, tractor/cargo upgrades, collect-vs-process choices, waiting reasons | Done: 4 specimens, 3 UFOs, slots, tractor/cargo/stealth/scanner, suspicion, contracts |
| 4b | Strategy update (George, before prestige): contracts, traits, selectable experiments, automation, research/discovery split, incidents, encyclopedia | Done 2026-10-14 |
| 4c | Map zoom + experiment results box (George) | Done 2026-10-14 |
| 4d | Crew staffing, "Alien Resources" (George, before prestige) | Done 2026-10-14; recruit traits recorded for later |
| 4e | Unattended-play milestone (George): bottlenecks, pause-when-full, staffing incidents, away report, strategy switch | Done 2026-10-14 |
| 4f | Idle autosave + reload-and-recalculate on acknowledging the memo (George) | Done 2026-10-14 |
| 4g | "Choose Your Research Department" (George): 3 departments x 3 projects, signature experiments, department contracts/reports | Done 2026-10-14 (visual uniforms/equipment with the sprites thread) |
| 5 | First prestige playable: "Your First Academy Promotion" (thesis, review, grants, permanent benefit, 3 grant upgrades, 3 theses, Metropolis) | Done 2026-10-14 |
| 5b | "Earth Notices" (George): 3 regions per location, 4 regional responses with counters, reroute automation, Earth conditions per run | Done 2026-10-14 (region, response and Bureau visuals done by the sprites thread) |
| 5c | "The Academy Has Competition" phase 1 (George): Efficiency Bureau sweeps, standing trade orders, race contract, reputation counters | Done 2026-10-14; sweeps now avoid alert grounds and cool suspicion (George chose both fixes); phase 2 done 2026-10-14: Celebrity Studies, Experimental Sciences, joint experiments, reputation effects, pairing per thesis |
| 5d | "Every Build Has a Purpose" (George): 3 approaches (bulk/precision/covert) each with thesis, rival, permanent pick; bottleneck fixes; presets; "why these numbers"; auto-accept contracts; behaviour memos; per-approach prestige + offline tests | Done 2026-10-14; needs a human playtest per approach |
| 5e | "Build Your Next Headquarters" (George): Oceanic (hull pressure) then Antarctic (heat distribution), achievements unlock them, location theses and keep-forever perks, cross-base upgrades | Done 2026-10-14; Oceanic and Antarctic art done by the sprites thread |
| 5f | "The Specimens Remember" (George): 5 recurring case-file subjects, 3 relationship types, Peer Review, case discoveries kept through prestige | Done 2026-10-14 (see case-files.md) |
| 6 | Test second run for real choices | Bot check done (PrestigeCycle); needs a human playtest |
| 7 | Personality pass (reports, incidents, reviews) | Not started (a few log lines exist) |
| 8 | Saving, offline progress, polish | Saving + offline progress done 2026-10-14; polish not started |

## Folder locations

- `F:\Alien_Idle\docs\game-design-document.md` — full GDD (also PR #1 on GitHub, Kordochian/Close-Encounters-of-the-Idle-Kind)
- `F:\Alien_Idle\docs\prototype-spec.md` — one-page prototype numbers
- `F:\Alien_Idle\docs\screenshots\` — in-game captures
- `F:\Alien_Idle\AlienIdle\` — the Unreal project (`AlienIdle.uproject`)
- `F:\Alien_Idle\art\sprites\` — SVG sprite sources (PNGs in `AlienIdle\Content\Sprites\`)
- `F:\Alien_Idle\Claude_md\` — these notes
- Engine: `C:\Program Files\Epic Games\UE_5.8` · VS 2026 (18) Community

## Decisions

- Engine UE 5.8.3, PC (Windows) first; UI must stay gamepad-navigable for a later console port.
- Presentation: 2D side-on cutaway mountain base drawn in Slate from C++. Art (George, 2026-10-14):
  cartoon sci-fi dark-comedy sprites, SVG sources in `art/sprites` split into parts and animated in
  code as cut-out puppets (PNGs in `Content/Sprites`). Done: 3 UFOs, squirrel, cow, lab alien, 5 rooms, mountain with hallways and the old man's cabin.
  Planned: seasons/weather/catastrophe backgrounds as layers. See art-sprites.md.
- Game rules live in a plain C++ sim (`FAlienIdleSim`) separate from UI; deterministic fixed steps.
- Numbers live in `AlienIdleTuning.h` and must match `docs/prototype-spec.md`.
- Save compatibility (George, 2026-10-14): old saves breaking is fine while the game is in development; just bump SaveVersion, no need to mention it.
- Keep everything in `F:\Alien_Idle`; do not push to GitHub unless George says so. Not a git repo yet.
- Fleet roles (George, 2026-10-14): ships have 6 upgrade slots; tractor, cargo, stealth, scanner
  built first (heavy lifter / bulk collector / discreet abductor / rare-specimen hunter). Each
  upgrade has a tradeoff and a dark-office-humor line per level. Refit = strip for 50% refund.
- Remaining upgrades from George's list are recorded in prototype-spec.md "Later upgrades".
- Two currencies (George, 2026-10-14): routine/extended studies and contracts pay Research (upgrades);
  discovery projects pay nothing and unlock targets/equipment. Specimen unlocks are discovery-gated.
- Incidents must stay occasional and self-recovering (one at a time, 2-min cooldown, "Call Facilities" optional).
- Crew (George, 2026-10-14): every UFO/lab chamber/station needs at least one alien; three specialties
  (pilot, scientist, technician); recruitment separate from training; free reassignment; auto-assign on
  by default. Recruit traits/drawbacks are deferred (prototype-spec.md "Recruit traits").
- Saving (George, 2026-10-14): after 5 min with no real input the game autosaves to
  `AlienIdle/Saved/SaveGames/MountainStation7.sav`; on return the memo shows, and "Acknowledge memo"
  reloads that save in place and recalculates the time since (capped at 8 h). The game also saves on
  quit and every 2 minutes, and at startup loads the save and simulates the time it was closed (memo
  shown if over a minute). Launch with `-AlienFresh` for captures/tests so George's save is untouched.
  `AlienAway <min>` runs the idle flow with a fast-forward.
- Departments (George, 2026-10-14): one primary department per run, others at 2.5x cost (never locked).
  Later, prestige should let the player carry one department discovery into the next assignment.
- Prestige (George, 2026-10-14): only three grant upgrades, no small % bonuses; review text is flavour,
  rewards are transparent. Metropolis is the second location (city skyline art done by the sprites thread).
- Earth Notices (George, 2026-10-14): responses must stay soft (slower/less profitable, no sudden
  losses); a military/combat system is explicitly deferred.
- Rivals (George, 2026-10-14): visible, predictable, temporary; core thesis always achievable; nothing
  expires offline (race offers wait, the Bureau's race clock pauses during catch-up).
- Builds (George, 2026-10-14): three viable approaches; none may be fastest, safest and most profitable at
  once (test ApproachCycles enforces it); offline must stay useful for all three (>= 30% of active rate).
- Locations (George, 2026-10-14): each posting must change opening purchases, crew posts and research
  priorities, or it needs a stronger mechanic (tests OceanicChangesTheOpening, AntarcticChangesTheOpening).
  Humour request: overheating and alert excuses where humans blame cow farts.
- Goal check before prestige: two strategies must both work in one run (bulk animals vs selective humans).

## Open items

- GDD open questions (monetization, art style, offline cap, etc.) still open; see GDD section 13.
- Keyboard/gamepad focus on the console buttons is wired but not yet verified by hand.
- First thesis pacing (2026-10-14 tune): bots finish thesis 1 in 18-27 min (test ThesisPacing). Levers:
  thesis 1 objectives (5 discoveries, 3 contracts, 100 experiments) and discovery baselines in AlienIdleTuning.h.
- After departments, 40-minute bot runs earn 53k-59k research (batch experiments and signature payouts);
  costs and rewards need a balance pass alongside prestige.
- Strategy bots after 25 min: selective humans 13.6k research vs bulk animals 9.6k (1.4x). Bulk is
  limited by few animal-only contracts once humans exist; consider more animal contract templates.

- Covert's offline rate is thin and seed-sensitive (ApproachCycles: 44% of active in the tested path, 16% when one
  random case subject shifted run 1); worth an energy/covert-template balance look. See case-files.md "State".

## Topic notes

- [prototype-build.md](prototype-build.md) — how the UE prototype is structured, built, tested, run
- [case-files.md](case-files.md) — recurring human subjects, relationships, case studies, scanning for them
- [art-sprites.md](art-sprites.md) — animated sprites (UFOs, squirrel, cow, alien, rooms, mountain): sources, parts, code, re-rendering
