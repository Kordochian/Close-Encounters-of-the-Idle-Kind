# Console UI (right-hand panel tabs)

Last updated: 2026-10-14

George's request: four tabs across the right panel, day-to-day controls together in Operations so the
player can upgrade, assign aliens and dispatch without switching tabs.

## Layout

Always visible above the tabs: location, rank/run/grants, energy and research, the bottleneck line.

| Tab | Contents |
|---|---|
| Operations | Incident (while one lasts), then 1 Select, 2 Status, 3 Crew, 4 Dispatch / Experiments / Production, 5 Upgrades; then base-wide: what to do next (bottleneck fixes), research department, regions and Earth's responses, case files, encyclopedia, log |
| Contracts | Finished contracts to claim (Claim / Claim all), the accepted contract with progress and Abandon, auto-accept, available offers |
| Thesis | Thesis and rule, Earth conditions, objectives, run counts, prestige rewards if submitted now (grants and rank, refreshed once a second), achievements, Submit thesis; build approach guide and presets |
| Collaborations | Rivals and their current event, reputation, joint experiment + contribute toggle, races (accept/decline), trade orders |

Operations details:
- Selector: a stepper over every UFO and station (`GetAllStations` order). Clicking a ship or room in the
  cutaway selects it and switches to Operations; the selection gets a pulsing green bracket in the base.
- Status: what it is doing, a progress bar (trip phase, experiment, containment fill), warning color when blocked.
- Crew: `GetStationStaffingText` always shows; "Show assignments" folds out recruit/auto-assign and one row
  per alien with Assign here / Remove and Train.
- UFOs: region stepper, target stepper (within that region), why these numbers, Dispatch + Auto-repeat
  (= auto-launch), contract mode, scanner/case target, fleet rules (reroute, pause when full).
- Stations: chambers get lab priorities, signature experiments, Middle Management, calibration status;
  containment gets freezer mode and pause-when-full; the Boiler Room gets heat priority; others say there is nothing to choose.
- Upgrades: compact cards (name and level, cost, next improvement, blocker). Base upgrades are listed with the
  station they improve (`IsBaseUpgradeShown`): UFO = hangar, camouflage, sonar, ink; chamber = chambers,
  insulated labs, calibration desk; containment = containment, deep tanks, cryo vault, cryo pods; power = power,
  tidal, geothermal; memory wipe = cleanup; Pressure Control = deep tanks, tidal; Boiler = geothermal.

Badges (only when an action is waiting): Contracts = number of unclaimed contracts, Thesis = "!" when the
thesis can be submitted, Collaborations = "!" while a race offer awaits an answer.
Each tab is its own scroll box in an `SWidgetSwitcher`, so scroll position and selections survive tab switches.
Gamepad shoulder buttons cycle tabs.

Contract claims (new rule): a finished contract counts as done at once (thesis objectives, `GetContractsCompleted`)
but its reward waits in `Unclaimed` until claimed. Auto-accept claims automatically (also offline). Bots in
tests call `ClaimAllContracts` every step. Save version 12.

## Where in the source

- `UI/SAlienIdleRoot.cpp` `Construct`: header, then one `SVerticalBox` per tab (Operations, Contracts, Thesis,
  Collaborations; the `Panel`/`Add` helper switches between them), then the tab bar + `TabSwitcher`.
  Helpers after Construct ("Console tabs and the Operations selection"): `SetTab`, `GetTabBadge`, `OnKeyDown`,
  `SelectStation`, `StepSelection`, `ValidateSelection` (called in `Tick`), `GetSelectionLabel/Status/Progress`,
  `IsSelectionBlocked`, `IsBaseUpgradeShown`. Cards: `MakeUpgradeCard`, `AddShipUpgradeButton`, `AddBaseUpgradeButton`.
- `UI/SAlienIdleRoot.h`: `EConsoleTab`, `Selected` (FAssignment), `SelectedUfo`, `bCrewExpanded`, `ActiveTab`, `CachedReview`.
- `UI/SAlienBaseView`: `OnStationClicked` / `GetSelectedStation` arguments, `StationBounds` (room click areas in
  base-local coordinates), `UfoBounds` (recorded each paint, so ships in flight are clickable), `HitStation`,
  selection bracket at the end of `OnPaint`.
- Sim: `GetUnclaimedContracts`, `GetUnclaimedReward`, `ClaimContract`, `ClaimAllContracts` (after `CountTowardContract`
  in AlienIdleSim.cpp); auto-claim in `StepAutoContract`.

## State

Verified 2026-10-14 on branch `claude/project-thread-gtnfyj` (on top of PR #2, Probe Calibration), built in a
separate worktree `F:\Alien_Idle_tabs`: build clean first time, 63/63 tests pass (no bot or tuning changes needed).
Bots: Balanced 35064 research earned (cows 2.4 min, humans 11.8); humans vs bulk at 25 min 1.61x; Bulk 1052/min,
Precision 434/min, Covert 426/min; calibration unchanged (9345 ignoring, 9558 precise).
Layout fixes after looking at captures: tab widths follow label length (Collaborations badge was clipped), the UFO
bracket starts below the ship name, the console subtitle wraps.
Dev commands for captures: `AlienTab N` (0..3), `AlienSelect <station> <index>` (EStation number: 1 UFO, 2 chamber...),
`AlienBadges` (thesis ready, one-cow contract, race offer; uses `DevOfferRace`).
Screenshots: `docs/screenshots/console-tabs-*.png` (operations-ufo, operations-chamber, contracts, thesis, collaborations).
Not yet: a human playtest of gamepad focus across tabs. Known older art issue: the "BULK COLLECTION" hangar label
runs past the hangar frame into the ship name.
