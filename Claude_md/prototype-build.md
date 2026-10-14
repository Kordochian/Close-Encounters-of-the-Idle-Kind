# Prototype Build (UE 5.8.3)

Last updated: 2026-10-14 (console tabs)

## Where in the source

All under `F:\Alien_Idle\AlienIdle\Source\AlienIdle\`:

- `AlienIdleTuning.h` — enums (`ESpecimen`, `EShipSystem`, `EBaseUpgrade`, `EOccupation`, `EDiscovery`,
  `EExperimentKind`) and every number: specimen, occupation (trait), discovery and upgrade tables,
  suspicion, incident and contract constants, encyclopedia texts. Mirror of `docs/prototype-spec.md`.
- `AlienIdleSim.h/.cpp` — `FAlienIdleSim`: whole game state + rules. Per-UFO state machine
  (Docked → FlyingOut → Capturing → Returning → WaitingToUnload if containment full), FIFO
  containment in units, lab chambers pull the oldest specimen when energy allows, regions with
  suspicion/alert/lockdown, contract board (3 offers, 1 active, `MakeContract` templates), lab
  priorities per specimen (`FExperimentChoice`: routine/extended/discovery) with chamber pick order
  contract → project → oldest, discoveries (`Discover`, gates in `CanBuy*`/`IsSpecimenUnlocked`),
  traits via `FOccupationDef`, scanner targeting (`GetScannerTarget`), contract mode
  (`StepAutomation`), Middle Management meetings, incidents (`StepIncidents`, seeded `FRandomStream`,
  `DevSetIncidentsEnabled(false)` in tests), encyclopedia, ship slots + refit, all log text.
  Crew lives at the end of AlienIdleSim.cpp ("Crew (Alien Resources)"): `FCrewMember`, `FAssignment`
  (`EStation` + index), `GetStationBonus`, `Recruit`, `Train`, `Assign`/`CycleAssignment`,
  `AutoAssignIdle`, `ReturnSpecimen` (memory-wipe effect). Staffing checks are in `CanLaunch`,
  `StepLab`, `GetEnergyPerSecond`, `GetFlightSeconds`/`GetCaptureSeconds`, escapes and capacity.
  Unattended play is the last block of AlienIdleSim.cpp: `IsOnDuty`, `IsHeldForContainment`
  (pause-when-full), `StepBottlenecks` (cumulative + 60 s decayed per `EBottleneck`),
  `GetBottleneckText`/`GetRecommendation`, `BeginAwayReport`/`BuildAwayReport` (`FAwayReport`).
  Staffing incidents (StaffMeeting, AdoptedSquirrel) are cases 3-4 of the equipment roll in `StepIncidents`.
  Every "why is it waiting" string comes from the sim (`GetUfoStatusText`, `GetChamberStatusText`,
  `CanLaunch`/`CanBuy*` reasons).
- Research departments live near the end of AlienIdleSim.cpp ("Research departments"): `ChooseDepartment`,
  `BuyProject` (tiers in order, 2.5x outside the primary), signature flow (`BeginSignature` ->
  `GetSignatureReservation` holds specimens -> `TryStartSignature` in `StepLab` -> `CompleteSignature`),
  `MakeDepartmentContract`. Project effects are inline where the mechanic lives (search `IsProjectDone`).
  Data tables `Departments` / `DeptProjects` are in AlienIdleTuning.h. Save version 2.
- Prestige is the last block of AlienIdleSim.cpp ("Thesis, Academy review and promotion"):
  `FCareer` (kept across runs), `GetThesisObjectives`, `BuildPromotionReview` (grants, review text,
  resets/keeps, permanent options), `Promote` (builds a fresh sim, copies career + encyclopedia,
  `ApplyCareerStart`). Thesis rules and Metropolis effects are inline (search `Thesis(Thesis)` and
  `ELocation::Metropolis`). Tables in AlienIdleTuning.h: `Theses`, `GrantUpgrades`, `Ranks`,
  `RegionNames`, `LocationNames`. Save version 3. UI: THESIS section in the console and the promotion
  overlay `MakePromotionPanel` in SAlienIdleRoot.cpp. Dev command `AlienThesis` completes the
  objectives and opens the review.
- Regions and Earth's responses: the last block of AlienIdleSim.cpp ("Regions and Earth's responses"):
  `ChooseRegionFor` (reroute + bots), `AddPressure`/`StepResponses`, `Counter`. Destinations are
  (`FUfo::Region`, `FUfo::Destination`); trips use `TripRegion`; specimens remember `FSpecimen::Region`.
  `AddSuspicion(Region, Amount, Kind)` applies exposure, thesis, Earth condition, Metropolis and
  investigators. Old per-specimen getters (GetRegion/IsRegionOnAlert/GetRegionName with ESpecimen)
  report the specimen's home region. Tables: `RegionDefs`, `HomeRegion`, `Responses`, `EarthConditions`.
  Save version 4. Bots target regions via `SetDestinationBest`.
- Rivals: last block of AlienIdleSim.cpp ("Rivals: the Efficiency Bureau"): `StepRival` (sweeps, standing
  orders, race clock; skips the race while `bCatchingUp`), `OfferRace`/`AcceptRace`/`CountTowardRace`.
  State in `FRivalState` (sim member `Rival`); reputation counters in `FCareer`. Sweep hooks: cargo halved
  in `FinishCapture`, capture x1.5 in `GetCaptureSeconds`, routing penalty in `ChooseRegionFor`. Save v5.
- Rivals phase 2: last block of AlienIdleSim.cpp: `StepRivalEvents` (celebrity stunts, breaches, as
  `FRivalEvent`), `StepJoint` (contributions), `GetSellFraction`/`GetBuyCost`/`GetRaceGrants` (reputation),
  `GetRivalEventText`. Pairings: `AlienIdleTuning::RivalPairs` applied in `Promote`. Tables: `Rivals`.
  Save version 6.
- Builds: `AlienIdleTuning::Approaches` (EApproach, FApproachDef), `PresetSlots`, `BehaviourMemoSeconds`,
  `FThesisDef::bNoActiveResponses`. Sim (end of AlienIdleSim.cpp): `SuspicionScale`, `ExplainTarget`,
  `SavePreset`/`ApplyPreset`/`GetPresetSummary` (FBuildPreset in FCareer), `ApplyApproachTemplate`,
  `ClassifyApproach`, `StepBehaviourMemo`, `GetBottleneckFixes`, `GetApproachReading`, `StepAutoContract`.
  UI: "BUILD: APPROACH AND PRESETS" section and "Why these numbers" toggle in SAlienIdleRoot.cpp.
  Tests: bot strategy `Covert`, `ApproachCycles`, `PresetsAndExplanations`. Save version 7.
- Locations (Oceanic, Antarctic): tables in AlienIdleTuning.h (specimens 4-9, RegionDefs[2..3], HomeRegion,
  Achievements, hull and heat constants, theses DeepEnd/ColdStorage). Sim, end of AlienIdleSim.cpp: `Local`
  (stand-in species), `GetDiscoverySpecimen`, `IsSpecimenAtLocation`, `IsBaseUpgradeAvailable`, `StepHull`,
  `GetHullText`, `StepAchievements`/`StepTenure`, heat (`GetHeatSupply`, `GetLabWarmth`, `GetHangarWarmth`,
  `GetHeatText`), `StepOverheat`. Preserved samples: `FSpecimen::Held`, freezer mode in `PickNextSpecimen`.
  Location perks: `FCareer::LocationPerk`, `FPermanentOption::Perk`. Tests: Oceanic*/Antarctic* and helper
  `StartAt`/`PlayOpening` in AlienIdleSimTests.cpp. Save version 9.
- Probe Calibration: `AlienIdleCalibration.cpp`, `UI/SCalibrationGauge.h`, Calibration Desk post; see
  probe-calibration.md "Where in the source". Save version 11.
- `AlienIdleSave.cpp` — `Serialize` writes every FAlienIdleSim field in declaration order (bump
  `AlienIdleTuning::SaveVersion` when fields change; mismatched saves are rejected), `SaveToBytes`/
  `LoadFromBytes` (in place), `SaveToFile`/`LoadFromFile` (file = UTC ticks + payload),
  `CatchUp` (capped at `OfflineCapSeconds`, 8 h). AwaySnapshot is not saved; loading resets it.
- `UI/AlienInputWatcher.h` — input preprocessor recording deliberate input only (keys, clicks, wheel,
  analog). Mouse movement never counts (George's call: the cursor crosses the window during other
  work). Slate's own last-interaction time counted those moves, so the game never went idle.
- `AlienIdlePlayerController.h/.cpp` — owns the sim, ticks it, adds the Slate UI to the viewport.
  Idle flow in `Tick`: 300 s without real input -> autosave; next input -> memo; `OnAwayReportDismissed`
  reloads the save and `CatchUp`s the wall-clock time since it (AlienAway uses its own minutes).
  Dev console commands: `AlienLaunch [ufo]`, `AlienAddResearch <n>`, `AlienUnlockAll`, `AlienAway [minutes]`, `AlienDemo`
  (jumps to a 3-UFO mid-game base, unlocks automation and turns on auto-launch so ships keep flying; good for screenshots).
- `AlienIdleGameMode.h/.cpp` — sets the controller, no pawn. Set as GlobalDefaultGameMode in `Config/DefaultEngine.ini`; map is `/Engine/Maps/Entry`.
- `UI/SAlienIdleRoot` — layout: cutaway left, 460 px console right (header + four tabs: Operations,
  Contracts, Thesis, Collaborations; see console-ui.md), and a 150 px "EXPERIMENT RESULTS" box across the
  bottom (newest first, from `FAlienIdleSim::GetResults()`, which holds experiment and discovery reports only).
  All buttons stay enabled (gamepad focus) and say why an action is unavailable.
- `UI/SAlienBaseView` — `OnPaint` draws the cutaway in a virtual canvas x 0..2100, y -300..860
  (`CanvasMin`/`CanvasMax`, widened and raised 2026-10-14). Zoom 1 shows `ViewWidth` 1300 units across
  and starts at the bottom around x 1100; drag to scroll sideways and up. Hunting grounds use canvas
  coordinates (x 0..660); the mountain and everything in the base are drawn in base-local coordinates
  with `FCanvasPainter::Shift` = `BaseShift` (+400 x), so the base sits centered on the wider mountain.
  Rooms are spread out like a bunker: each room block adds its own offset (`ContainmentOffset`,
  `LabOffset`, `GeneratorOffset`, `BreakRoomOffset`) to `P.Shift`; crew badge anchors add the same.
  Default view center x 1260 (shows the whole base).
  UFOs fly in canvas coordinates (`DockX(u) + BaseShift`, `BaseExitPos + BaseShift`). Console
  `AlienIdle.FitWholeMap 1` fits the whole map instead (screenshots):
  four hunting grounds left (Glenwhit, Circus Grounds, Dairy Flats, Pine Ridge) with suspicion bars,
  mountain right with the hangar opening onto its slope (`HangarX` 780, `HangarY` 110; docks, pads,
  exit point and the UFO crew badge derive from them), unit-tiled containment, lab tables, generator,
  memo board.
  Ship upgrades are drawn on each UFO. Mouse wheel zooms toward the cursor (1x to 4x), left/right
  drag pans, double-click resets (`GetView`, `ClampViewCenter`, `OnMouseWheel`...); the widget clips
  to its bounds so zoomed art never covers the console.
  Colors authored as sRGB via `Srgb()`; rounded shapes use per-radius `FSlateRoundedBoxBrush`es.
- `UI/AlienSprites` — animated cut-out sprites (Old Reliable, squirrel) used by the base view; see art-sprites.md.
- `Tests/AlienIdleSimTests.cpp` — 63 automation tests under `AlienIdle.Sim.*` (see file); `RunBot`
  plays Balanced / BulkAnimals / SelectiveHumans strategies and prints unlock times, research,
  contracts, incidents via AddInfo (PacingBot, StrategyComparison).

## Build, test, run

```powershell
# Build (≈1–2 min incremental)
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" AlienIdleEditor Win64 Development "-Project=F:\Alien_Idle\AlienIdle\AlienIdle.uproject" -WaitMutex
# Tests (headless)
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "F:\Alien_Idle\AlienIdle\AlienIdle.uproject" -ExecCmds="Automation RunTests AlienIdle;Quit" -unattended -nullrhi -nosplash -abslog=<path>
# Play
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "F:\Alien_Idle\AlienIdle\AlienIdle.uproject" -game -windowed -ResX=1600 -ResY=900
```

Never run headless tests or captures while another thread's game instance is running (concurrent
instances crash at startup in the DerivedDataCache); wait for it to exit. Close any running game instance before building (the DLL is locked otherwise). The game takes
~20 s to show the UI after the window appears. For screenshots, `-ExecCmds=AlienLaunch` plus
PrintWindow capture works; SendKeys into the window is unreliable.

## Gotchas learned

- Module root is not an include path by default: `PrivateIncludePaths.Add(ModuleDirectory)` in Build.cs.
- Slate in the game viewport treats `FLinearColor` as linear: dark colors look washed out unless converted from sRGB.
- `FSlateRoundedBoxBrush` with a radius larger than the shape draws nothing; pick radius ≤ half the short side.

## State after Probe Calibration

Code written 2026-10-14 (5 new tests: CalibrationGradesAndBoost, CalibrationIgnoredCostsNothing,
CalibrationBalance, CalibrationTechnicians, CalibrationSaves). Verified: build clean (after renaming shadowing
locals in SAlienBaseView.cpp), all 63 tests pass. CalibrationBalance, 20 min Balanced/Biology: ignoring windows
9345 research, missing every one 9746, steady/standard 8846, precise 9523; 16 windows, 93 s of lab work skipped.
The boost is small next to run-to-run divergence (a skipped second reshuffles later random rolls), so research
totals are noise here; seconds skipped is the real measure. Technician (specialist, untrained): 7%.
With 30 s cooldown (George chose "more often"): ignoring 9345, missing 9122, steady/standard 9431 (x1.009),
precise 9558 (x1.023); 27 windows, 166 s of lab work skipped. 63/63 pass.

## State after "The Specimens Remember"

Verified 2026-10-14: build clean, all 58 tests pass (new: 6 Case* tests). ApproachCycles now runs without random
case appearances (see case-files.md "State"). Screenshot `docs/screenshots/case-files.png`.

## State after "Build Your Next Headquarters"

Verified 2026-10-14: build clean, all 52 tests pass. bt.ps1 now waits for (never closes) other editors and
crash dialogs. Console shows hull and heat lines, heat priority and freezer-mode buttons, achievements in the
thesis section, up to 5 thesis offers. Not yet checked by eye.

## State after "Every Build Has a Purpose"

Verified 2026-10-14: build clean, all 46 tests pass. ApproachCycles: bulk 25 min / 919 research/min /
80 response-min; precision 16 / 370 / 9; covert 20 / 484 / 7. Offline 30 min: bulk 1009/min, precision
376/min, covert 203/min. Console UI not yet checked by eye (no screenshot taken this milestone).

## State after rivals phase 2

Verified 2026-10-14: build clean, all 44 tests pass (new: RivalCelebrityStunt, RivalBreach,
RivalJointExperiment, RivalReputationAndPairing). Console "RIVALS ON THIS ASSIGNMENT" shows each active
rival, its event, the joint experiment with a contribute toggle, and reputation.

## State after rivals phase 1

Verified 2026-10-14: build clean, all 40 tests pass (new: RivalSweep, RivalTrading, RivalRace, RivalRun).
Console section "RIVALS: THE EFFICIENCY BUREAU" (sweep status, race accept/decline, 5 standing-order toggles).

## State after Earth Notices

Verified 2026-10-14: build clean, all 36 tests pass (new: RegionsCompete, EarthResponds,
AutomationReroutes, EarthConditions, EarthNoticesRun). Console: target stepper cycles region x
specimen pairs with value/exposure; "REGIONS AND EARTH'S RESPONSES" shows building pressure and a
counter-action button per active response; reroute toggle; Earth condition line under THESIS and a
picker in the promotion screen. First-thesis pacing bound relaxed to 16-32 min (Balanced+Biology now 17).

## State after pacing tune + closed-game progress

Verified 2026-10-14: all 31 tests pass (new: ThesisPacing). In-game: quit via window close saved,
relaunch 90 s later logged "loaded save, simulated 1.6 of 1.6 minutes closed". Startup load is in
`AAlienIdlePlayerController::BeginPlay`; `-AlienFresh` skips loading and saving.

## State after first prestige

Verified 2026-10-14: build clean, all 30 tests pass (new: PromotionResetsAndKeeps, PrestigeCycle).
`AlienDemo,AlienThesis` capture: `docs/screenshots/academy-review.png`.

## State after research departments

Verified 2026-10-14: build clean, all 28 tests pass (new: DepartmentChoice, BatchExperiments,
SignatureExperiment, DepartmentRunsDiffer). Console "RESEARCH DEPARTMENT" section: choose buttons, 9
project buttons, 3 signature buttons with live status (`docs/screenshots/choose-department.png`).
Bots take a department argument (`RunBotOn(Sim, Strategy, Minutes, Department)`).

## State after idle autosave + reload

Verified 2026-10-14: build clean, all 24 tests pass (new: SaveRoundTrip, SaveFileAndCatchUp; both
match a live run exactly after 10/20 minutes). In-game: game minimised and idle, autosave at 5:00,
a key press showed the memo, Enter acknowledged it and the log read "Reloaded the idle save and
recalculated 0.6 minutes: research 546, experiments 6 (live operation had 546 and 6)". To test idle
in-game, just don't click or type in it; mouse movement no longer counts as input.

## State after the unattended-play milestone

Verified 2026-10-14: build clean, all 22 tests pass (new: PauseCollectionWhenFull, BottleneckDiagnosis,
StaffingIncidents, UnattendedThirtyMinutes, StrategySwitch). `AlienDemo,AlienAway 30` capture shows the
memo over the game (`docs/screenshots/while-you-were-out.png`). Tip: if the module DLL is locked by
another thread's game window, copy Source/Config/Content/uproject to the scratchpad and build/test there.

## State after crew staffing

Verified 2026-10-14: build clean, all 17 tests pass (new: CrewMinimums, CrewSpecialistsAndTraining,
MemoryWipeStation). Cutaway shows colored crew dots per station (blue pilot, green scientist,
orange technician) and UNSTAFFED labels (`docs/screenshots/crew-start-of-run.png`). Bots now hire
when a post is unstaffed; 25-min research: selective humans 13.1k vs bulk animals 9.8k (1.33x).

## State after the strategy update

Verified 2026-10-14: build clean, all 14 tests pass. Console now has Contracts, Fleet (auto-launch,
contract mode, scanner target), Base, Lab priorities (steppers per specimen, Middle Management),
Incident (with Call Facilities), Regions, Encyclopedia, Log. Cutaway adds incident/meeting banners,
management tie, encyclopedia shelf (`docs/screenshots/strategy-update-console.png`).
Bot numbers are in prototype-spec.md "Pacing targets".

## State at end of step 4

Verified 2026-10-14: build clean, all 8 tests pass. `AlienDemo` capture shows three specialised
UFOs flying to three regions, beams, containment with cows/elephant, three lab chambers
(`docs/screenshots/step4-*.png`). Pacing bot: cows 2.2 min, elephants 5.6, humans 13.3, 3rd human
experiment 15.3 min. Bot lesson: putting thrusters on the tractor ship can block Tractor IV for
elephants (slots), which is why refit exists.

Not yet: prestige/thesis (step 5), discoveries, saving/offline, cover stories, escapes.

## State at end of step 3

Verified 2026-10-14: build clean, both tests pass, in-game capture shows fly-out, tractor beam,
return, lab run, memo on the break-room board (`docs/screenshots/step3-*.png`). Expedition is
6 s out, 4 s capture, 6 s back, 10 s lab → first research at ~26 s. Thrusters (15 research after two
squirrels) shorten flights 15% and add visible thruster pods/flames on the UFO.

Not yet: other specimens, cargo/tractor/containment/lab/generator upgrades, auto-launch, prestige, saving.
