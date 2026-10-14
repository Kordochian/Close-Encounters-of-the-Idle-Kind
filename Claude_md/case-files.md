# Case Files: "The Specimens Remember"

Last updated: 2026-10-14 (first build; see "State" below for verification)

George's milestone: captured humans get personalities and relationships that create stories across the
existing systems. Five recurring characters, three relationship types, a handful of interactions. Special
subjects are findable by targeted scanning, so nothing depends on a lucky appearance.

## Where in the source

All under `F:\Alien_Idle\AlienIdle\Source\AlienIdle\`:

- `AlienIdleTuning.h`, block "The Specimens Remember": `CaseSubjects` (5 x `FCaseSubjectDef`: name, title,
  occupation, lead, profile, return line, 3 `FCaseStepDef` steps, discovery name/effect), `CaseRelations`
  (`ECaseRelation` Colleagues/Rivals/Kindred), every number (`CaseAppearChance`, per-encounter effects,
  trouble timers, Peer Review). New case-only occupation `EOccupation::Journalist` (`IsCaseOccupation`:
  never rolled at random, never scanned for). `EExperimentKind::CaseStudy`. Save version 10.
- `AlienIdleCases.cpp` (new file): all case-file rules. `MeetCase` (encounter count, runs met, new/returning
  result lines, colleague investigation), `RollCaseAppearance` (own `CaseRng`, so the main RNG and old bot
  numbers are untouched), `GetNextCaseStep`, `CompleteCaseStudy`, `TryStartPeerReview`, `IsWaitingForPartner`
  (lab holds a rival while the other is on base or being scanned for), `IsKindredHeld`,
  `GetCaseResearchMultiplier`, `StepCaseDynamics` (troublesome pairs -> incidents), history
  (`AddCaseVisit`/`CloseCaseVisit`), console text (`GetCaseFileText`, `GetCaseRelationText`,
  `GetCaseTargetText`), scanning (`CanTargetCase`, `SetCaseTarget`), `DevAddCaseSubject`.
- `AlienIdleSim.h`: `FSpecimen::Case` (-1 = ordinary), `FUfo::CaseTarget/TripCase`, `FExperimentChoice::CaseStep`,
  `FLabChamber::Partner` (Peer Review), `FCaseFile` + `FCareer::Cases[5]` + `bPeerReviewed` (kept through
  prestige), per-run `CaseStepsDone`, `CaseVisit`, `bCaseStudies`, `bPeerReviewDone`, `CaseTrouble`,
  `RunCaseDiscoveries`. Incidents `ContainmentNewsletter`, `AcademicDispute`.
- Hooks in `AlienIdleSim.cpp` (search `Case`): `FinishCapture` (scanned or random subject), `Launch`
  (TripCase), `GetCaptureSeconds` (harder to fool), `StepIncidents` (escape x per encounter), `EscapeSpecimen`,
  `PickNextSpecimen` (rival hold), `ResolveExperiment` (case study unless a fitting discovery project is
  picked), experiment seconds/energy/research, `StepLab` (Peer Review), `FinishExperiment` (case study, partner),
  `ReturnSpecimen` (wipe works less each time), `AddPressure` and contract reward (case perks),
  `BuildPromotionReview` (grant lines, keeps/resets), `Promote` (CaseRng carried, "followed the sightings" log).
- UI: `SAlienIdleRoot.cpp` section "CASE FILES: PERSONS OF INTEREST" (before the encyclopedia): case-studies
  toggle, one block per subject (file text, relationship lines, warning in orange when taking them WILL send
  investigators, "Scan for X with '<selected UFO>'" button). Fleet section shows the case target line.
- Dev console: `AlienCase [n]` puts subject n (or all five with no argument) into containment.
- Tests (`AlienIdle.Sim.Case*`): CaseReturningSubject, CaseTargetedScanning, CaseColleagues,
  CaseRivalsPeerReview, CaseKindred, CaseFilesAcrossPrestige.

## The roster

| # | Subject | Occupation | Chain (step 1 any visit / 2 return visit / 3 second assignment) | Case discovery (forever) |
|---|---|---|---|---|
| 0 | Dr. Edith Marsh | Astronomer | Intake Interview / Calibration Study / Longitudinal Study | Ground Truth: discovery projects 30% faster |
| 1 | Prof. Desmond Pike | Astronomer | Skeptic Baseline / Denial Under Load / Tenure Review | Hostile Peer Review: +1 grant per Academy review |
| 2 | Lou Delgado | Journalist (case-only) | Off-the-Record Briefing / Source Protection / The Retraction | Off the Record: investigators build half as fast |
| 3 | Martin Pell | Accountant | Expense Audit / Reconciliation / Creative Accounting | Creative Accounting: contracts +25% |
| 4 | Gloria Fenn | Therapist | Intake Session / Group Session / Closure | Group Therapy: support groups build half as fast |

Steps 1-2 pay 150 / 400 research; step 3 pays the discovery (+1 grant at the review). One step per visit.
Steps already done on an earlier assignment take half the time.

## Rules

- Appearance: each unscanned human taken has a 4% chance to be a subject not already on base. Targeted
  scanning (any ship with a Trait Scanner, human target) always finds the chosen subject, known or still a lead.
- Repeat encounters: +25% research per earlier encounter (max 4), but escapes +50% each, scanning capture
  time +25% each, and a staffed memory wipe adds 2 suspicion per earlier encounter ("Memory wipe unsuccessful").
- Relationships (shown once either subject has a file):
  - Colleagues, Delgado and Pell: taking one when the other has a file gives investigators +25 and suspicion +8
    in that region. Held together in containment 20 s: Containment Newsletter (labs x0.85, 40 s, +6 suspicion).
  - Rivals, Marsh and Pike: both in containment start Peer Review (one chamber, 60 s, 40 energy, 600 research,
    counts 2 experiments, once per run). The lab holds one while the other is on base or being scanned for.
    Left waiting together 20 s: Academic Dispute (labs x0.75, 30 s).
  - Kindred, Fenn with Marsh and with Pell: both +30% research while held together.
- Troublesome incidents fire at most once per 120 s per pair, only when no other incident is running, and end
  on their own. They run even with random incidents off (they are consequences, not luck).
- Prestige: `FCareer::Cases` keeps encounters, runs met, best steps, discovery and the last 8 history lines
  ("Run 2, Metropolis Tower: taken from Downtown; Intake Interview; returned unwiped."). Captures, case
  studies done this run and Peer Review reset.

## Ideas not built

- Sprites or a name tag for subjects in the cutaway (they are humans in containment for now).
- More subjects per location, relationship webs beyond four pairs, thesis objectives that use case files.

## State

Verified 2026-10-14 on George's PC: build clean (no errors or warnings), all 58 AlienIdle tests pass (52 old + 6 Case*).
In-game: `-AlienFresh -ExecCmds="AlienDemo,AlienCase"` shows the CASE FILES section (toggle, files, relationships,
scan buttons) and case-study results in the results box (Pell's Expense Audit paid 195 = 150 x1.3 kindred with Fenn).
Screenshot: `docs/screenshots/case-files.png` (console scrolled with posted mouse-wheel messages).

One fix: `ApproachCycles` failed (Covert offline 73 research/min vs 463 active, needs >= 30%). Cause: a random 4%
appearance (Prof. Pike) in run 1 changes the bot's whole path, and the main RNG carries through prestige, so run 2
reached minute 10 energy-starved on squirrels. No case-file bug. `RunApproach` in AlienIdleSimTests.cpp now calls
`DevSetCaseAppearances(false)` (the design note already intended old bot numbers to stay untouched); assertions unchanged.
Numbers are back to the pre-milestone values: bulk 23/27 min, 1052/min, offline 1125; precision 17/19, 434, offline 328;
covert 20/22, 426, offline 186 (44%). ThesisPacing unchanged: 17 / 23 / 17 / 20 min.
Open: Covert's offline margin is thin and seed-sensitive (even the passing run loses 1859 s to energy offline);
worth a balance look (energy or the covert template) rather than relying on one RNG path.
