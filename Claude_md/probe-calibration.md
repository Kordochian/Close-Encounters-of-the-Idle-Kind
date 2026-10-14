# Probe Calibration (optional interactions)

Last updated: 2026-10-14

George's request: optional short interactions that boost existing production while the base stays
productive unattended. Three were proposed; **Probe Calibration** is built, the other two are not yet.

| Interaction | Player action | Reward | State |
|---|---|---|---|
| Probe Calibration | Stop a cursor inside a highlighted band | Advance the current experiment | Built 2026-10-14 |
| Tractor Beam Assist | Keep a moving target inside a capture zone for a few seconds | Finish part of the current expedition early | Not started |
| Management Intervention | Click a distracted crew member | Brief station productivity boost | Not started |

## How it plays

- Now and then one running experiment (the one with the most time left, at least 12 s) shows a pulsing
  CALIBRATION AVAILABLE tag in the lab and a button under its chamber line in the console. One window at a
  time for the whole lab; it lasts 30 s, then lapses with no effect.
- Clicking opens a 5-second challenge: stop the sweeping cursor in the green band. Precise (bright core)
  skips 25% of the remaining time, steady (band) 15%, a miss or running out of time 8%.
- Accessibility: "Standard calibration (no timing)" gives 15%, the same as a steady stop.
- Shared cooldown: 60 s after any calibration, 30 s after an ignored window, so cycling labs gives nothing.
- Calibration Desk (base upgrade, 150 research; appears after 3 manual calibrations or 40 experiments):
  an optional technician post. The technician takes any window left alone for 15 s, for 6% (+2% per point of
  station bonus, max 12%), also offline. Manual calibration always does more. The desk never counts as a
  crew bottleneck.
- Dark-humour result lines go to the experiment results box.

All numbers: `AlienIdleTuning.h` "Probe Calibration" block, mirrored in `docs/prototype-spec.md` "Probe Calibration".

## Where in the source

All under `F:\Alien_Idle\AlienIdle\Source\AlienIdle\`:

- `AlienIdleCalibration.cpp` — all rules and text: `StepCalibration` (offer, lapse, technician), `OfferCalibration`,
  `BeginCalibration` (holds the window while the challenge is on screen), `GradeCalibration`, `Calibrate`,
  `GetCalibrationStatusText`, `GetTechnicianFraction`, `DevOfferCalibration`, result-line tables.
- `AlienIdleTuning.h` — `ECalibrationGrade`, `EBaseUpgrade::CalibrationDesk` (+ BaseUpgrades row), the
  `Calibration*` constants and `CalibrationFraction[]` by grade.
- `AlienIdleSim.h` — public API ("Probe Calibration" block before Encyclopedia); state at the end of the class
  (`CalibrationRng` with its own seed, so ignoring windows never changes the run). `EStation::Calibration`.
- `AlienIdleSim.cpp` — `StepCalibration` call after `StepLab` in `Step`; desk in `GetAllStations`,
  `GetStationName`, staffing text, `IsBaseUpgradeAvailable`/`CanBuyBaseUpgrade` text; skipped as optional in
  `StepBottlenecks`, `GetBottleneckText`, `GetRecommendation`.
- `AlienIdleSave.cpp` — calibration fields at the end of `Serialize` (save version 11). `bCalibrationOpen` is
  not saved: a reload never leaves the challenge stuck.
- `UI/SCalibrationGauge.h` — the gauge widget (track, band, core, cursor).
- `UI/SAlienIdleRoot.cpp` — console button per chamber + status line under "Lab speed"; challenge overlay
  `MakeCalibrationPanel`, `OpenCalibration`, `SubmitCalibration`, `Tick` (5 s timeout = missed attempt, result
  auto-closes after 6 s). Cursor uses real time (`GetCalibrationCursor`).
- `UI/SAlienBaseView.cpp` — pulsing tag over the chamber (`CalibrationTagPos`), click-vs-drag hit test in
  `OnMouseButtonUp`, desk crew badge anchor in the lab.
- `AlienIdlePlayerController` — dev command `AlienCalibrate`.
- Tests (`Tests/AlienIdleSimTests.cpp`, end of file): CalibrationGradesAndBoost, CalibrationIgnoredCostsNothing,
  CalibrationBalance (bot `GBotCalibrateGrade` answers every window), CalibrationTechnicians, CalibrationSaves.

## State

Code written 2026-10-14; build and test results are recorded in prototype-build.md "State after Probe Calibration".
Not yet checked by eye in the running game.
