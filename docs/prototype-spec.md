# Prototype Spec: Mountain Station 7

> One-page spec for the first playable prototype. Status: v0.12 (next headquarters: Oceanic and Antarctic) · 2026-10-14
> Source of truth for numbers: `AlienIdle/Source/AlienIdle/AlienIdleTuning.h` (keep the two in sync).

## Goal

Prove that **abduct → experiment → upgrade** is fun in one mountain base. First milestone: a player
starts with a battered UFO, reaches human experiments, submits a thesis, and begins a second run
with a meaningful new choice. Development run length: **20–30 minutes** to the first thesis.

## Presentation and platform

- **Engine:** Unreal Engine 5.8.3, C++ project, UI built in Slate from code (no binary assets needed
  for the prototype).
- **View:** 2D side-on cutaway of the mountain. Sky and hunting grounds to the left, rooms inside the
  mountain to the right, UFOs fly visibly between them. Placeholder art is flat colored shapes.
- **Platform:** PC (Windows) first. Every action is a focusable button, so mouse, keyboard and
  gamepad (D-pad + A) all work. Later console port needs no layout rewrite.

## Resources

| Resource | Start | Source | Spent on | Prestige |
|---|---|---|---|---|
| Energy | 20 (cap 40) | Generator, 1/s | Launching UFOs, running experiments | Resets |
| Research | 0 | Experiments | Upgrades | Resets |
| Grants | 0 | Submitting a thesis | Academy upgrades | Kept |

Energy is the *operating* bottleneck: when it runs short, the UFO or lab visibly waits for it.

## Specimens and missions

Flight time is per leg (out and back). Capture happens on site. All times at upgrade level 0.

| Specimen | Region | Flight | Capture | Weight | Units | Lab time | Launch E | Exp. E | Research | Suspicion | Unlocked by |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Squirrel | Pine Ridge | 6 s | 4 s | 1 | 1 | 10 s | 4 | 6 | 12 | 4 | Start |
| Cow | Dairy Flats | 9 s | 5 s | 3 | 2 | 20 s | 6 | 15 | 45 | 8 | Discovery: Acorn Amnesia |
| Elephant | Circus Grounds | 14 s | 8 s | 8 | 4 | 35 s | 10 | 40 | 160 | 20 | Discovery: Herd Behaviour |
| Human | Glenwhit | 9 s | 5 s | 1 | 1 | 25 s | 6 | 30 | 110 | 15 | Discovery: Total Recall |

- **Weight** needs that much tractor strength. **Units** is containment space (start 4 units).
  **Suspicion** is added to the region per specimen taken.
- On return, specimens unload into containment first-in-first-out. If they don't fit, the UFO
  **waits at the dock** ("containment full") until space frees. Idle lab chambers pull the oldest
  specimen when there is energy for it; otherwise they show "waiting for energy".
- After the experiment the specimen is memory-wiped and returned; its space frees.

### Suspicion

Each region has suspicion 0–100, decaying 0.5/s. At **50** the region is *on alert* (captures take
×1.5). At **100** it goes into *lockdown*: no launches there until it drops below 40. Rotate
hunting grounds or fit stealth.

## The decision chain (strategy update, 2026-10-14)

**Accept a contract → configure a UFO → find the right specimen → choose an experiment → unlock something useful.**

### Two currencies

- **Research** comes from routine and extended studies and from contracts. It buys upgrades.
- **Discoveries** come only from discovery projects. They pay no research; they unlock new targets
  and equipment. So "fill the research bar" and "make progress" are different activities.

### Experiments (lab priorities)

The player sets, per specimen type, what the lab does with it. Chambers take whatever advances the
active contract first, then specimens that can run the chosen project, then the oldest.

| Kind | Time | Energy | Research |
|---|---|---|---|
| Routine | ×1 | ×1 | ×1 |
| Extended | ×2.5 | ×2.5 | ×3.5 (better per minute, but ties up chambers and energy) |
| Discovery project | fixed per project | fixed | 0, unlocks something |

### Discoveries

A project opens after enough routine/extended studies of its specimen (baseline data).

| Discovery | Specimen | Opens after | Time | Unlocks |
|---|---|---|---|---|
| Acorn Amnesia | squirrel | 5 | 20 s | Dairy Flats (cows) |
| Unscheduled Departures | squirrel | 8 | 25 s | Escapes halved; Containment Habitat III+ |
| Herd Behaviour | cow | 4 | 40 s | Circus Grounds (elephants) |
| Bovine Load Tolerance | cow | 2 | 40 s | Tractor Beam III+ (needed for elephants) |
| Total Recall | elephant | 3 | 60 s | Glenwhit (humans) and the Trait Scanner |
| Pachyderm Logistics | elephant | 2 | 60 s | Experiment Chamber III, Hangar Bay III |
| Voluntary Meetings | human (accountant) | 2 | 45 s | Middle Management: labs +20%, 10 s meeting every 2 min |
| The Sarcasm Organ | human (lawyer) | 2 | 45 s | Human research +50% |

### Human traits (occupations)

Each human has one occupation. Without a scanner it is random; a scanner ship targets whatever the
active contract (or the chosen human project) needs.

| Occupation | Effect |
|---|---|
| Accountant | +50% research; escaping reorganises inventory (containment +3 for 2 min) |
| Astronomer | Discovery projects twice as fast |
| Magician | Takes 2 containment units; escapes 6× as often |
| Lawyer | Experiments +50% longer (consent form), +25% research; escaping serves a cease-and-desist (launches +50% energy for 1 min) |
| Therapist | While on base, all chambers +20% speed ("staff morale is becoming measurable") |
| Mall Santa | +50% research, double suspicion when taken |

### Contracts

Three offers on a board; one active at a time; abandon any time. Templates: census (N of one
animal), comparative zoology (2 squirrels + cow), large mammals (cow + elephant), acquire a
qualified *occupation*, three different professions, rush job (2 humans). Rewards 40–380 research.
New targets refresh the oldest offer.

### Automation (unlocks after 3 experiments)

- **Auto-launch** per UFO: relaunch whenever possible.
- **Contract mode** per UFO: the ship retargets itself to what the active contract still needs
  and that it can lift; with a scanner it hunts the needed profession.
- **Lab priorities** (above) are the lab's standing orders.

### Incidents

Occasional and self-recovering: never more than one at a time, none in the first 3 minutes, 2-minute
cooldown after each. Escapes (chance per second per specimen waiting in containment), generator
brownout (30 s), tractor jam on one UFO (25 s), coffee machine failure (labs ×0.7, 40 s). Each
clears by itself; "Call Facilities" fixes it now for 25 research. In a 30-minute bot run: 5–10
incidents, of which 1–7 escapes.

### Encyclopedia

18 entries: 4 species, 6 occupations, 8 discoveries, each a confident alien misunderstanding
("Therapist: Charges humans to listen to them. We do this for free and are called 'abductors.'").
Filed on first capture or discovery; shown as books on the break-room shelf.

## Crew: Alien Resources (2026-10-14)

Nothing works without staff. The run starts with three aliens: a pilot on Old Reliable, a scientist
in lab chamber 1 and a technician on the power station. Containment and the memory-wipe station
start unstaffed ("Management considers this sustainable"). Every new UFO and lab chamber needs
someone before it does anything, so each purchase raises the question: buy another ship, improve
this one, or hire someone to operate it.

| Station | Minimum (1 crew) | Without it | Extra staff / specialists improve |
|---|---|---|---|
| UFO | Pilot enables expeditions | Grounded | Flight and capture +15% faster per bonus point |
| Lab chamber | Scientist enables experiments | Idle / paused | Experiments +20% faster per bonus point |
| Containment | Warden manages habitats | Escapes ×4 | +2 units per bonus point, escapes ÷(1+bonus) |
| Power station | Technician runs generators | Energy ×0.5 | Energy +20% per bonus point |
| Memory-wipe station | Operator processes returns | Each return +3 suspicion ("they are telling people") | −3 suspicion per return per bonus point |

- **Bonus** = total skill − 1. Each crew member gives 1 skill; a specialist at a matching station
  adds +0.5, and each training level +0.25 more. So one generalist runs a post; extras and
  specialists improve it. Max 3 per station.
- **Specialties:** pilots (UFOs), scientists (labs), technicians (power, containment, memory-wipe).
  Anyone can fill any post, so a "wrong" recruit is never useless.
- **Recruitment** ("Request Additional Personnel"): 40 research ×1.5 per hire, random specialty.
- **Training** (separate from recruitment): 30 research ×2 per level, max 3; the alien is off duty
  for a 30 s seminar.
- **Reassignment** is free and instant (stepper per alien). **Auto-assign** (on by default) puts
  idle crew on unstaffed posts first, matching specialists preferred, then extra specialists on
  matching stations. It never moves someone the player assigned.

### Recruit traits (later, from George)

Each recruit gets one useful trait and one ridiculous drawback. Ordinary hires stay dependable;
rare recruits offer interesting alternatives.

| Trait | Upside | Drawback |
|---|---|---|
| Overqualified | Excellent researcher | Uses twice the energy on unnecessary equipment |
| Gentle Tentacles | Fewer containment incidents | Slower specimen handling |
| Company Loyalist | Cheaper to recruit | Occasionally calls a productivity meeting |
| Human Enthusiast | Excellent human research | Keeps trying to adopt subjects |

Recruitment upgrades (later): more applicants to choose from, unlock specialist hiring, better
training.

## Unattended play milestone (2026-10-14)

Goal (George): configure the base, leave for 30 minutes, return to useful progress and a clear next
upgrade; switch between bulk-animal and selective-human strategies by reassigning crew and equipment.

- **Pause collection when containment is full** (global toggle, on): an auto-launching ship holds at
  the dock when containment has no room left for even one specimen after everything already in
  flight lands. Status: "Holding: containment is full, collection paused."
- **Bottleneck line** (top of console): the dominant limiter over roughly the last minute, one of
  Crew, Energy, Collection (labs idle with nothing to study), Containment (ships waiting or holding),
  Laboratory capacity (queue with every chamber busy), plus a **Next:** recommendation with a cost
  (for example "Next: Cargo Bay on 'Old Reliable' (25 research), or Hangar Bay (120 research) plus a
  pilot."). Unstaffed posts count by impact: ship, lab or power 1, warden 0.5, wipe operator 0.25.
- **Staffing incidents** join the incident roll: a crew member calls a mandatory meeting (labs paused
  25 s) or adopts a squirrel (that alien off duty 45 s; a squirrel leaves containment). Both clear by
  themselves; "Intervene" ends any incident for 25 research.
- **While You Were Out** memo: shown when the player returns after 5+ minutes without input (game left
  running), or via the `AlienAway <minutes>` console command. Lists captures, experiments, research,
  contracts, discoveries, incidents and escapes, and lost productivity in station-minutes per
  bottleneck, then the next upgrade. "Operations continued during your absence. Management has taken
  credit." True offline progress (game closed) needs saving, which is step 8.

Verified by tests: a base configured by the bot for 15 minutes then left alone for 30 minutes
captured 338 specimens, ran 347 experiments and earned 8.2k research; the memo's next step named the
unstaffed third UFO. A bulk-animal base pivoted to selective humans (refit a cargo ship to a scanner,
same crew) ran 28 human experiments in the following 15 minutes.

## Research departments: "Choose Your Research Department" (2026-10-14)

Pick one primary department (once per run). Its projects cost the listed research; the other two
departments' projects cost 2.5x but are never locked. Half of new contract offers become department
contracts. Half of routine experiment reports use the department's voice. Staff uniforms per
department (green lab coats / grey suits / scorched overalls) are for the art pass.

| Department | Project (tier, cost) | Effect |
|---|---|---|
| Biological Studies | Reinforced Habitats (1, 120) | +4 containment units; animals escape half as often |
| | Heavy Tractor Array (2, 350) | +3 tractor strength on every ship; tractor energy penalty halved |
| | Batch Experiments (3, 800) | Animal studies take two of one species at once for double research |
| Human Behavior | Advanced Scanners (1, 120) | Scanning takes no extra time; human research +25% |
| | Behavioral Lab (2, 350) | Human experiments 30% faster; lawyers no longer delay |
| | Memory Manipulation (3, 800) | Each returned human removes 6 suspicion (needs a wipe operator) |
| Applied Incompetence | Experimental UFO Modules (1, 120) | +2 upgrade slots per ship; 10% of launches fly out 50% slower |
| | Unusual Power Systems (2, 350) | +3 energy/s, +60 storage; equipment fails 50% more often |
| | Automated Cover Stories (3, 800) | Suspicion fades twice as fast; lockdowns lift at 60 instead of 40 |

**Signature experiments** (one per department, queued from the console; the lab holds the needed
specimens aside and starts when everything is in place; 120 s, 60 energy, 1500 research):

| Department | Signature | Needs in containment | Staffed | Permanent bonus |
|---|---|---|---|---|
| Biological | The Great Chain of Being | squirrel, cow, elephant | warden + wipe operator | animal research +25% |
| Human Behavior | Who Is Responsible? | accountant, politician, squirrel | warden + wipe operator | human research +25% |
| Applied Incompetence | Controlled Explosion | squirrel, cow, magician | 2 crew on power | each incident pays 60 research |

The primary department's signature is open from the start; another department's needs its first
project. A new occupation, **Politician** (+30% research, +50% suspicion when taken), joins the pool
for the responsibility study. Scanner ships target whatever a queued signature still needs.

Department contracts (reward reflects 1.5x): Biology: menagerie (squirrel + cow + elephant, 450),
cross-species (3 squirrels + 2 cows, 260), field survey (6 squirrels, 110). Human Behavior:
responsibility panel (accountant + lawyer, 400), focus group (4 professions, 560). Applied
Incompetence: hazard pay (2 magicians, 420), load test (2 elephants, 520), stress test (5 squirrels, 100).

Verified by tests (`AlienIdle.Sim.DepartmentRunsDiffer`): over 40 minutes a Biological run built a pure
animal operation (205 animal experiments, 0 human) and a Human Behavior run a mixed one (150 human
experiments, 32% of all); both bought all three projects and completed their signature experiment.
Research totals (53k and 59k) are now well above earlier pacing and need tuning with prestige.

## Earth Notices (2026-10-14)

**Three competing regions per location.** A UFO's target is now a specimen *in a region*. Value
multiplies research from specimens taken there; exposure multiplies the suspicion they cause.

| Location | Region | Specimens | Value | Exposure | Notes |
|---|---|---|---|---|---|
| Mountain | Pine Ridge Valley | squirrels, cows | x1.0 | x0.8 | quiet farms |
| | Glenwhit | squirrels, humans | x1.2 | x1.2 | accountants more common |
| | County Fairgrounds | elephants, cows, humans | x1.5 | x1.5 | flights x1.3, magicians more common |
| Metropolis | Central Park | squirrels, humans | x1.0 | x1.0 | conspiracy theorists more common |
| | The Suburbs | cows, humans | x1.1 | x0.9 | accountants more common |
| | Downtown | humans, elephants | x1.6 | x1.6 | celebrities more common |

**Earth's responses.** Each region builds pressure from what we do there; at 60 (45 in a crackdown)
a response starts, and it ends when pressure fades to 20 (0.15/s). Effects are soft, so offline play
gets slower or less profitable but never loses anything.

| Your activity | Response | Effect | Counter (60 research, x1.5 per reuse in that region) |
|---|---|---|---|
| Animals taken (+7 each) | Night patrols | animal captures x1.6, animal research -20% | Deploy decoy cows |
| Humans taken (+9 each) | Investigators | human captures x1.3, double suspicion from humans | Start a jurisdiction dispute |
| Launches (+5 each) | Radar coverage | flights x1.3, +4 suspicion per launch | File a weather balloon report |
| Escapes (+30), unwiped human returns (+4) | Support group | suspicion fades at half speed, human research -10% | Fund an absurd cover story |

Other options: rotate regions (pressure fades), stealth (radar x0.6 and patrols/investigators x0.7
per level), **Radar Camouflage** base upgrade (300 research, radar x0.5), and a staffed memory-wipe
station (cools investigators on each human return). **Reroute** (on by default): auto-launching ships
move to the best region offering their target when their region has a relevant response or lockdown.

**Earth conditions** are chosen with each new assignment after the first promotion, with previews:
UFO Documentary Craze (radar and investigators x1.5; human research +25%), Surveillance Crackdown
(suspicion +30%, responses start at 45; contracts +30%), Everything Is Advertising (radar x0.3,
investigators x0.5; human research -15%, contracts -20%). Run 1 is Business as Usual.

Verified by tests (`AlienIdle.Sim.EarthNoticesRun` and others): over a 30-minute bot run Earth
triggered 7 responses (65 response-minutes) as usual, 79 response-minutes in a crackdown, and 3
responses (44 response-minutes) when everything is advertising; an auto-launching cow ship rerouted
from the patrolled valley to the fairgrounds; leaving a region lets its pressure fade.

## The Academy Has Competition: phase 1 (2026-10-14)

Scoped as George asked: one rival, trading and one optional competition contract first; the other two
rivals, collaboration and reputation effects come after this proves itself.

**The Efficiency Bureau.** Collects common specimens in bulk. First sweep at 5:00, then every 7
minutes. Each sweep is announced 90 s ahead in the log and console, targets the squirrel or cow ground
we have used most in the last few minutes that is not already on alert (so a calmer alternative is
left), and lasts 150 s. While it runs: captures there take 50% longer and bring back half the cargo
(minimum 1). Routing (reroute automation, contract mode, bots) treats a swept ground as hostile.
Afterwards the Bureau's paperwork buries local rumours (that region's suspicion -25) and it has surplus.

**Trading: standing orders** (never expire, keep working offline): sell surplus squirrels/cows/
elephants when containment is 75% full (keeps one of each; 70% of their routine research), buy a
squirrel (10) or cow (30) every 45 s while there is room.

**Competition: race contract** (optional, offered at 8:00 and 10 minutes after each race ends; the offer
never expires): study 8 squirrels (or 6 cows once unlocked) before the Bureau, which studies one every 45 s
of active play and pauses while the game is closed. Winning: 2 Research Grants (kept) and 1.5x the
specimens' research. Losing: no penalty, and "A rival department has published your findings
independently. Unfortunately, they also copied your spelling mistakes."

**Reputation** (kept across runs): races won and lost, lifetime trades. Effects come in phase 2.

Measured (`AlienIdle.Sim.RivalRun`, 30 minutes, balanced bot, after George chose both sweep fixes):
4 sweeps; 0% of flights went to a ground while the Bureau swept it (0 of 44), against 26% to those same
grounds at other times; 48 trades worth 1,074 research; 1 race won, 1 lost; thesis still complete.

## The Academy Has Competition: phase 2 (2026-10-14)

**Three rivals, paired per assignment** (previewed on the promotion screen): Preliminary Observations has
the Bureau alone; Nobody Will Believe You: Celebrity Studies + Experimental Sciences; Corporate Wants
Results: Bureau + Celebrity Studies; A Controlled Experiment: Bureau + Experimental Sciences.

| Rival | What it does (announced 90 s ahead, 150 s long) | Opportunity / response |
|---|---|---|
| Efficiency Bureau | Sweeps your busiest calm squirrel/cow ground: half catch, slower | Hunt elsewhere or switch species; it cools the region afterwards; trade |
| Celebrity Studies Division | Stunt in the most valuable human region (first at 6:00, every 8 min): investigators pressure there | Human suspicion everywhere else is halved: exploit quieter regions |
| Experimental Sciences Office | Breach in each region in turn (first at 7:00, every 9 min): support-group pressure and suspicion there | Every trip there brings back one extra (escaped) specimen; Cleanup Crew (250) halves the fallout |

**Compete:** races now come from any rival on the assignment, each on its speciality (Bureau 6 cows,
Celebrity 4 humans, Experimental 3 elephants; falls back to what you can catch).
**Trade:** standing orders work with whichever rivals are present.
**Collaborate:** each rival has a joint experiment (Joint Census: 6 squirrels + 4 cows; Fame Under
Glass: 3 humans; Controlled Spillage: 2 elephants + 2 cows). Switch on "contribute" and containment
feeds it; completion gives a shared discovery (encyclopedia), 400 research and 1 Research Grant.

**Reputation (kept across runs):** 1 collaboration: sell surplus at 85% instead of 70%; 2: buy 30%
cheaper. 1 race won: races 50% bigger and +1 grant; 2: twice as big and +2 grants.

Measured: a human taken from Glenwhit raised suspicion by 18 normally and 9 during a celebrity stunt in
the fairgrounds; a breach trip returned cargo + 1; after one race win the next race grew from 8 to 12
specimens and from 2 to 3 grants.

## Every Build Has a Purpose (2026-10-14)

Three approaches, each with a thesis, a rival interaction and a permanent discovery that suit it
(table `AlienIdleTuning::Approaches`, shown in the console's approach guide and on the promotion screen):

| Approach | Thesis (rule) | Rival interaction | Keep forever | Strengths | Weaknesses |
|---|---|---|---|---|---|
| Bulk Collection | Corporate Wants Results: 2 discoveries, 2 contracts, 150 experiments; max 2 UFOs, recruits -30% | Efficiency Bureau: sell surplus, Joint Census; Experimental Sciences escapees (pairing Bureau + Experimental) | Batch Experiments | Lots of cheap specimens, easy to leave running | Low value each, patrols, containment fills, Bureau sweeps |
| Precision Research | A Controlled Experiment: 3 contracts, 5 professions, 2 department projects; exactly 1 UFO, +4 slots | Celebrity Studies races on humans, Fame Under Glass (pairing Bureau + Celebrity) | Advanced Scanners | Highest research per specimen, contracts pay big | Slow captures, lab time, investigators |
| Covert Operations | Nobody Will Believe You: 3 contracts, 6 professions, **Earth calm when you submit**; suspicion x1.5 | Celebrity stunts give cover elsewhere (pairing Celebrity + Experimental) | Automated Cover Stories | Rarely provokes Earth, keeps regions open | Stealth eats cargo slots; rotation needs attention |

**Console additions:** bottleneck panel lists every upgrade that would ease it (plus Earth-pressure fixes);
"BUILD: APPROACH AND PRESETS" reads the operation's actual style, shows the approach guide with a
template button per approach (targets, automation, lab, trade orders, auto-contracts), and 3 preset
slots (save/load fleet targets, automation flags, crew posts, lab priorities, trade orders; kept across
runs, applied as far as the current base allows). "Why these numbers" under the fleet target spells out
research multipliers (region value, profession, scanners, signature, Sarcasm Organ, Earth condition,
support group, patrols, batch) and mission penalties (distance, radar, alert, patrols, investigators,
sweeps, scanning, cargo loading, exposure, stealth, thesis, Earth condition, celebrity cover).
**Auto-accept contracts** (unlocks with automation): when idle, signs the best-paying offer the fleet is
set up for, also while the game is closed. **Department memos** every 5 minutes classify the operation
from its ships and work (e.g. "Your stealth division remains unrecognized. They have filed a complaint.").

Measured (test ApproachCycles: run 1 in the approach's department, promote into its thesis keeping its
permanent pick, Business as Usual, no grant purchases):

| Approach | Run 2 | Research/min | Earth response-min | Offline (30 min from minute 10, template automation) |
|---|---|---|---|---|
| Bulk | 25 min | 919 | 80 | 1009/min (145 experiments) |
| Precision | 16 min | 370 | 9 | 376/min (19 contracts) |
| Covert | 20 min | 484 | 7 | 203/min (energy-limited; Earth responds 64 min unattended) |

Fastest = precision, richest = bulk, safest = covert; the test fails if one approach ever wins all three.

## Build Your Next Headquarters (2026-10-14)

Two new postings, unlocked by Academy achievements and then freely chosen at promotion (their theses
take you there automatically). Existing discovery chains run on local stand-ins, so every department
signature still works: Oceanic elephant work is done on whales; on the ice, squirrel/cow/elephant work
is done on penguins/seals/elephant seals.

| | Oceanic: Trench Station 9 | Antarctic: Station Permafrost |
|---|---|---|
| Unlock | Achievement "Heavy Lifting": study 8 elephants in one run | Achievement "Tenure Track": submit two theses |
| Local system | **Hull pressure** (0-100) settles at 50 x load / pumps. Load: 0.04/s per containment unit, Deep-Water Tanks, turbines; +7 per surface launch. Pumps: 0.3 + 0.9 x (1 + bonus) with Pressure Control staffed. 60+: labs 20% slower; 85+: surface airlock sealed until < 60 | **Heat distribution**: supply 2 + Boiler Room 4 x (1 + bonus) + 3 per geothermal tap; demand 1.5 per chamber and per UFO; priority even / labs first / hangar first. Cold labs down to 40% speed; cold hangar flights +60%, launch energy +50%. Over 1.6x demand: overheating (humans blame cow farts, +4 suspicion at the outpost) |
| Main challenge | Cramped (habitat capped at II), generators x1.5, surface expeditions to Harbour Town cost double energy | Long supply routes: upgrades +25%, recruits +40%, long flights |
| Opportunity | Aquatic chain, one free tidal turbine, underwater regions draw no radar and half patrol attention | Recruits arrive trained (25% level 2), Polar Researchers (+40% research, 2x discovery speed), containment preserves samples (+50% research after 2 min; freezer mode waits a minute) |
| Specimens / traits | Pufferfish, octopus (escape artist), whale (5 units, buoyant: weight 5) | Penguin, seal, elephant seal; Polar Researcher |
| New chain | Inflation Policy (6 puffers) -> octopus; Eight-Armed Bureaucracy -> whale; Ink Diplomacy -> humans here; Whale Song Transcripts | Huddle Theory (6 penguins): heat demand -25% here; Blubber Economics (4 seals) |
| Exclusive facilities | Tidal Turbines (+3 energy at high tide, 0 at low, 4-min tides; hull load); Deep-Water Tanks (+5 units; hull load) | Geothermal Taps (+3 heat; plume: +25% radar per tap); Cryo Vault (+4 units, escapes halved, preserves 2x; 0.5 energy/s) |
| Cross-base options | Ink Screen Launchers (Ink Diplomacy): trips -25% suspicion, launches +3 energy. Sonar Array (Whale Song): instant scanning, +50% radar | Insulated Labs (Huddle Theory): experiments -30% energy, labs 5% slower. Cryo Pods (Blubber Economics): +3 units and preserved samples at any base; 0.5 energy/s |
| Thesis | The Deep End: 60 aquatic experiments, 3 discoveries, 2 contracts; hull stress x1.5 | Cold Storage: 25 preserved samples, 3 discoveries, 2 contracts; heat x0.75 |
| Keep forever | Submersible Hulls: +1 upgrade slot on every ship everywhere; dives half energy | Polar Crew Program: every recruit everywhere arrives trained |
| Incidents and humour | Whale sings to the hull (+20 pressure, labs slower); crab in the intake; seawater coffee; creaking and leak lines | "The heating budget has been reduced. Staff are encouraged to develop thicker skin." (heat x0.5); frozen generator; overheating cow-fart jokes |

Measured (same bot, 20-minute opening after a first thesis):
- Mountain: 11,652 research; buys thrusters, generator, recruits, habitats; discovery chain starts with acorns.
- Oceanic: 8,353; staffs Pressure Control with two technicians, adds turbines and tanks, runs the aquatic chain.
  Leaving Pressure Control empty: 2,707 (hull creaks and the airlock seals).
- Antarctic: 8,259; recruits for the Boiler Room first, runs Huddle Theory and Blubber Economics; leaving the
  Boiler Room empty: 5,148 (labs at 53%, flights x1.47).
- Full loops: Heavy Lifting unlocks Oceanic in a 27-minute first run; The Deep End finishes in 20 minutes;
  Cold Storage in 25 minutes (freezer mode on); both perks and one cross-base upgrade carry back to the mountain.
- Fix found on the way: specimens held for a signature experiment no longer block a ship from unloading.
- Surveillance Crackdown now also makes every response build 20% faster (it had stopped provoking more
  responses than Business as Usual).
- Alert excuse added everywhere: "A local scientist blames rising temperatures on cow farts."

## Probe Calibration (2026-10-14)

George's first "optional short interaction": it boosts existing production, and the base stays fully
productive when nobody plays it (test CalibrationIgnoredCostsNothing: an unattended run is identical with
or without calibration windows).

| Rule | Number |
|---|---|
| Window | One at a time across all chambers, on the running experiment with the most time left (at least 12 s of work). First after 45 s; lasts 30 s, then lapses with no effect |
| Shared cooldown | 60 s after any calibration (manual or technician); 30 s after an ignored window |
| Challenge | 5 s; cursor sweeps the 0..1 gauge every 0.9 s; band half-width 0.09, precise core 0.03, band centre random in 0.2..0.8 (own seed, never the main random stream) |
| Precise (core) | skips 25% of the experiment's remaining time |
| Steady (band) | 15% |
| Missed / out of time | 8% (any attempt helps) |
| Standard calibration (accessibility: one click, no timing) | 15%, same as a steady stop |
| Calibration Desk (base upgrade, 150 research, unlocks after 3 manual calibrations or 40 experiments) | Optional technician post. Takes any window left alone for 15 s: 6% + 2% per point of station bonus, at most 12%. Works offline |

Opened from the pulsing CALIBRATION AVAILABLE tag on the lab chamber, or the console button under the
chamber line. Results go to the experiment results box, for example *"Calibration successful. Previous
results have been retroactively declared accurate."* and *"Acceptable precision. Subject disagreement has
been excluded from the dataset."* Dev command `AlienCalibrate` opens a window and the challenge.

Later candidates from the same request (not built): **Tractor Beam Assist** (keep a moving target in a
capture zone for a few seconds: finish part of the current expedition early) and **Management
Intervention** (click a distracted crew member: brief station productivity boost).

## Ship upgrades (per UFO; cost = base × growth^level)

Each UFO has **6 upgrade slots**, so no ship maxes everything. That gives four fleet roles: heavy
lifters (tractor), bulk collectors (cargo), discreet abductors (stealth), rare-specimen hunters
(scanner). The console names each ship's role from its dominant system. **Refit** strips a docked
ship's upgrades for a 50% refund, so a full ship can change jobs.

| Upgrade | Effect per level | Tradeoff | Base | Growth | Max | Slots/level |
|---|---|---|---|---|---|---|
| Tractor beam | Strength 1 → 3 → 5 → 8 → 12 (III+ needs Bovine Load Tolerance) | +25% launch energy | 40 | 2.2 | 4 | 1 |
| Cargo bay | +1 specimen per trip | +20% loading (capture) time | 25 | 2.0 | 4 | 1 |
| Stealth panels | Suspicion ×0.6 | 2 slots that could hold cargo | 35 | 2.0 | 3 | 2 |
| Trait scanner | Target the occupation your contract/project needs (needs Total Recall) | +4 s scan (2 s at level II) | 60 | 2.5 | 2 | 1 |
| Thrusters | Flight time ×0.85 | Uses slots | 15 | 1.8 | 4 | 1 |

## Base upgrades

| Upgrade | Effect | Base | Growth | Max |
|---|---|---|---|---|
| Containment habitat | +3 units | 20 | 1.7 | 8 |
| Experiment chamber | +1 experiment at a time (up to 3) | 60 | 4.0 | 2 |
| Power generation | +1 energy/s, +30 cap | 20 | 1.7 | 10 |
| Hangar bay | +1 UFO (up to 3) | 120 | 3.75 | 2 |

Every upgrade is **visible** in the base: thruster pods and flames, a wider purple tractor
emitter, cargo pods on the rim, a darker fading hull for stealth, a scanner dish, extra containment
tiles, extra lab tables, a bigger generator coil, more UFOs on the pads.

The core decision is *collect more* (cargo, thrusters, more UFOs) vs *process faster* (chambers,
habitats, power), with suspicion punishing over-collection from one region.

**Automation in run 1:** an auto-launch toggle per UFO unlocks after the 3rd experiment.

### Upgrade copy (dark office humor)

Every level has its own log line, for example *Cargo Bay II: "Occupancy limits have been revised
following a successful appeal from Accounting."* and *Stealth Panels III: "The ship is so discreet
that Facilities filed it as missing."* All lines live in `AlienIdleSim.cpp`
(`GetShipUpgradeFlavor`, `GetBaseUpgradeFlavor`).

### Later upgrades (from George's list, not built yet)

| Ship upgrade | Effect | Tradeoff |
|---|---|---|
| Containment systems | Fewer escapes; carry difficult specimens safely | Higher operating cost |
| Fuel efficiency | Less energy per expedition | Competes with performance upgrades |
| Flight range | New regions and rare targets | Longer journeys |
| Memory manipulation ("Memory Scrambler") | Less suspicion when returning specimens | Some traits resist it |

Base: **Laboratory precision** (chance of useful discoveries), **Automation** beyond auto-launch
(assign ships to target traits, route specimens to suitable experiments). Memory Scrambler copy:
*"Subjects retain only a vague sense that customer service was disappointing."*

## Prestige: "Your First Academy Promotion" (2026-10-14)

**Thesis.** Every run has one. All theses require the primary department's signature experiment,
plus a few objectives. The console's THESIS section tracks them, and "Submit thesis" opens the
Academy review when they are all done.

| Thesis | Objectives (plus signature) | Rule | Bonus grants |
|---|---|---|---|
| Preliminary Observations of Earth (run 1) | 5 discoveries, 3 contracts, 100 experiments | none | 0 |
| Nobody Will Believe You | 6 different professions, 3 contracts | all suspicion x1.5 | +2 |
| Corporate Wants Results | 3 discoveries, 120 experiments | at most 2 UFOs; recruitment 30% cheaper | +2 |
| A Controlled Experiment | 4 discoveries, all 3 department projects | exactly one UFO, +4 upgrade slots | +3 |

**Academy review** (shown before committing): rank change, a review line (flavour only, e.g.
"Your department reported zero containment failures after reclassifying escaped specimens as field
researchers. Promotion approved."), the run's specimens, discoveries, captures, contracts and
incidents, the exact grant breakdown, what resets and what is kept.

**Research Grants** = 4 (thesis) + thesis bonus + 1 per discovery published for the first time ever
(signature experiments count) + 1 per 2 new encyclopedia entries + floor(log2(1 + experiments/10))
for routine work (50 experiments = 2, 500 = 5: diminishing returns).

**Keep one department benefit forever:** one completed project of the primary department, or its
signature bonus. **Grant upgrades** (kept forever, three only): Better Starting UFO (2: Old Reliable
starts with Thrusters II, Tractor II, Cargo II), Specialist Starting Recruit (2: a fourth, level-2
alien of your chosen specialty), Improved Trait Scanning (3: starting scanner, no discovery needed,
half scan time).

**Resets:** research, energy, contracts, ships and upgrades, facilities, staff, discovery unlocks,
department choice, projects and signature. **Kept:** encyclopedia, grants, grant upgrades, permanent
benefits, unlocked locations. Discoveries already published run at half time, so openings are faster.

**Metropolis Tower** unlocks with the first promotion: Central Park, Petting Zoo, City Zoo,
Downtown. Humans are approved after Herd Behaviour (no elephant step), human flights are 40%
shorter, three city-only professions appear (Influencer: x2 research, x3 suspicion; Conspiracy
Theorist: returning one removes 15 suspicion; Celebrity: x3 research, x4 suspicion), and
surveillance is heavier (human suspicion x1.5, alert at 35).

Verified by `AlienIdle.Sim.PrestigeCycle`: run 1 (Biological, mountain) finished its thesis in about
15 minutes with 3 UFOs and 6% human work, earning 19 grants. After promotion (kept Heavy Tractor Array,
bought Better UFO and a veteran scientist, chose Corporate Wants Results in Metropolis, Human Behavior)
run 2 reached cows at 1.4 min (vs 2.0) and humans at 2.8 min (vs 12.2), ran with 2 UFOs and 8 crew,
did 28% human work, and completed its thesis inside 30 minutes. After the pacing tune (discovery
baselines raised: Acorn Amnesia 8, Unscheduled Departures 12, Herd Behaviour 6, Bovine Load Tolerance 4,
Total Recall 4, Pachyderm Logistics 4) bots finish the first thesis in 18-27 minutes across four strategies.

**Saving:** the game saves on quit, every 2 minutes and at the 5-minute idle mark, and on launch it
continues from the save, simulating the time it was closed (up to 8 hours) and showing the memo.

## Out of scope for the prototype

Cover stories, other base themes, more than one active contract, ship-to-trait assignment rules
beyond contract mode, saving and offline progress (step 8), audio, final art.

## Pacing targets

Bot runs (`AlienIdle.Sim.PacingBot`, `StrategyComparison`) after the strategy update: cows 1.9 min,
elephants 5.8, humans 11.6, 3rd human experiment 14 min. After 25 minutes a **bulk-animal** bot
(cargo ships, extended studies, no humans) has earned 9.6k research and a **selective-human** bot
(scanner ship, human contracts) 13.6k: both viable, humans ahead by 1.4×. Expect to slow early
unlocks or raise the thesis bar in step 5.

| Moment | Target time |
|---|---|
| First squirrel in the lab | < 30 s |
| First upgrade bought | ~ 1.5 min |
| Cows | ~ 5 min |
| Elephants | ~ 12 min |
| Humans | ~ 18 min |
| First thesis submitted | 20–30 min |
