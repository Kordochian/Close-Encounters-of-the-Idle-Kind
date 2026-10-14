# Close Encounters of the Idle Kind — Game Design Document

> Status: draft v0.1 · Owner: George · Last updated: 2026-10-14
>
> Engine: Unreal Engine 5.8.3
>
> Items marked **[Gap fill]** are reasonable defaults added to complete the design. Items under
> [Open questions](#13-open-questions) need a decision before production.

## Contents

1. [Pitch](#1-pitch)
2. [Pillars](#2-pillars)
3. [Core loop](#3-core-loop)
4. [Specimen stages](#4-specimen-stages)
5. [Human traits and contracts](#5-human-traits-and-contracts)
6. [Experiments](#6-experiments)
7. [Fleet](#7-fleet)
8. [Base](#8-base)
9. [Suspicion and cover stories](#9-suspicion-and-cover-stories)
10. [Prestige: Submit Your Thesis](#10-prestige-submit-your-thesis)
11. [Base themes](#11-base-themes)
12. [Tone and humor guide](#12-tone-and-humor-guide)
13. [Open questions](#13-open-questions)

---

## 1. Pitch

You are an alien research officer stationed inside a secret mountain base. Your assignment: study
Earth's lifeforms, submit your findings, and climb the ranks of the Galactic Academy.

You begin with a battered UFO, one laboratory, and a squirrel that has already escaped containment.

**Signature hook:** you are assembling an increasingly ridiculous alien understanding of Earth, and
your discoveries visibly change your base. After studying birthdays, your staff begin celebrating
every specimen's arrival with a mandatory cake.

## 2. Pillars

1. **The base is the punchline.** Every discovery should be visible somewhere: a decoration, a staff
   behavior, a new memo on the bulletin board.
2. **Choices over multipliers.** Upgrades and prestige unlocks change *how* you play, not just how
   fast numbers go up.
3. **Horror as paperwork.** The aliens treat unsettling circumstances as routine office
   administration. Experiments stay cartoonish and non-graphic.
4. **Respect idle time.** Fleet and labs keep working offline; the first run already includes
   meaningful automation.

## 3. Core loop

```
Scan for targets → Dispatch UFOs → Abduct specimens → Run experiments → Earn Research → Upgrade fleet & base
        ↑                                                     │
        └──────────── Memory-wipe & return specimen ──────────┘
```

- **Scan** a region to reveal targets. Scanner quality determines how many traits are visible
  before launch.
- **Dispatch** a UFO (or a mission group). Expedition time depends on speed and range; risk
  depends on stealth and regional suspicion.
- **Abduct** specimens up to the ship's tractor strength and cargo capacity.
- **Experiment** in laboratories. Each experiment consumes lab time and power and produces
  **Research** (soft currency) plus encyclopedia progress.
- **Return** specimens after a memory wipe, freeing containment space.
- **Upgrade** ships, rooms, and power, then repeat.

**Offline progress:** queued missions and experiments continue while the player is away, capped by
containment space and lab throughput. **[Gap fill]** Offline earnings cap at 8 hours by default,
extendable through Academy upgrades. The return screen is a "While You Were Out" memo from your
deputy.

### Currencies **[Gap fill]**

| Currency | Source | Spent on | Resets on prestige |
|---|---|---|---|
| Research | Experiments | Ships, rooms, upgrades | Yes |
| Power | Generators | Running labs and containment | Yes (capacity) |
| Research Grants (Academic Prestige) | Thesis submission | Academy upgrades | No |
| Encyclopedia entries | First-time discoveries | Unlocks, discovery equips | No |

## 4. Specimen stages

Each stage introduces a new challenge and unlocks a new system. Earlier animals remain useful
because their discoveries feed later studies (e.g., squirrel escape data improves human containment).

| Stage | Challenge | Requires | Unlocks |
|---|---|---|---|
| Squirrels | Easy to lift, hard to catch | Basic tractor beam | Tracking and containment |
| Cows | Heavy, bulky | Stronger tractor beams, bigger cargo bays | Bulk collection |
| Elephants | Very heavy, need room | Heavy-lift UFOs, reinforced habitats | Coordinated (multi-ship) missions |
| Humans | Physically easy, behaviorally complicated | Trait scanners, specialized labs | Specialized experiments and research contracts |

**[Gap fill]** Each stage is gated by encyclopedia progress in the previous one (for example, 10
squirrel experiments completed unlocks cow scanning), so the player never skips a lesson.

## 5. Human traits and contracts

Human targets carry combinations of traits:

- **Occupation** (astronomer, accountant, magician, dentist, mall Santa, …)
- **Intelligence**, **Fitness**, **Curiosity**, **Skepticism**
- **Unusual talents** (juggling, perfect pitch, can fold a fitted sheet)

Better scanners reveal more traits before launch; unscanned traits are discovered on arrival,
sometimes inconveniently.

### Trait families unlocked by prestige

New trait families arrive gradually across prestige runs. Each family creates new decisions rather
than bigger numbers.

| Research stage | Trait family | Gameplay effect |
|---|---|---|
| Initial operation (run 1) | Occupation, size, fitness | Learn capture and processing |
| Behavioral research | Skepticism, paranoia, curiosity | Different containment and experiment needs |
| Social research | Fame, influence, relationships | High-value subjects that create public attention |
| Anomaly research | Conspiracy obsession, improbable luck, alleged psychic ability | Unusual specimen abilities and experiment chains |

Examples:

- A **conspiracy theorist** may see through your camouflage immediately, then *reduce* suspicion by
  telling everyone a story nobody believes.
- A **celebrity** yields excellent research but triggers a missing-person media frenzy (large
  regional suspicion spike).

### Research contracts

Contracts request a specific trait combination for a bonus payout: a *skeptical astronomer*, a
*fearless magician*, an *athletic accountant*. **[Gap fill]** Three contracts are offered at a time,
refresh on a timer, and can be rerolled for Research. Contracts are the main reason to invest in
scanners.

## 6. Experiments

Experiments lean into absurd alien misunderstandings. They are cartoon science fiction, never
graphic.

- Locate the human sarcasm organ.
- Probe why "five more minutes" lasts forty minutes.
- Test resistance to elevator music.
- Determine whether a magician can escape alien containment.

**[Gap fill] Experiment structure**

- Each experiment has required specimen type/traits, lab type, duration, power cost, and an output
  table (Research, encyclopedia entry, chance of a bonus discovery).
- Matching traits improve results ("the accountant was *extremely* resistant to elevator music").
- Some experiments chain: finishing one unlocks a follow-up with stricter requirements.
- First completion of any experiment adds an encyclopedia entry and, often, a visible base change.

## 7. Fleet

UFO upgrades create real choices between specialized roles.

| Stat | Benefit |
|---|---|
| Speed | Shorter expeditions |
| Tractor strength | Heavier specimens |
| Cargo capacity | More specimens per trip |
| Stealth | Less detection |
| Scanners | Find rare traits |
| Range | Reach new regions |

### Ship roles

- **Scouts:** high scanners and stealth. Find valuable targets and reveal traits.
- **Collectors:** high cargo. Bring back large batches.
- **Heavy lifters:** high tractor strength. Handle cows and elephants.

**[Gap fill]** Ships have a limited number of upgrade slots, so a single ship cannot max every stat.
Coordinated missions (unlocked by elephants) let a scout tag a target for a lifter or collector.

## 8. Base

The base must keep up with what the fleet delivers. Bottlenecks are intentional and visible.

| Room | Role |
|---|---|
| Hangars | Ship count and repair |
| Containment rooms | Specimen capacity while waiting for experiments |
| Laboratories | Experiment throughput; specialized labs for later stages |
| Power | Runs labs and containment; overloading causes brownouts |
| Memory-wipe bay **[Gap fill]** | Processes returns; slow wipes back up containment |

**Visible discoveries:** completed research adds decorations, staff routines, and memos. Some are
cosmetic; some are tied to equipped discoveries (see 10.4) and carry a mechanical sidegrade.

## 9. Suspicion and cover stories

Each region tracks **suspicion**. Repeated raids raise it; high suspicion lowers mission success
and can lock a region temporarily.

Ways to manage it:

- Rotate hunting grounds.
- Improve ship stealth.
- Deploy **cover stories**: "weather balloon," "particularly ambitious owl," **[Gap fill]** "swamp
  gas," "a very confident drone," "local man simply wandered off."

**[Gap fill]** Cover stories are a consumable built in a Public Relations office. Each has a
strength and a credibility that drops if reused in the same region ("that's the fourth ambitious
owl this month").

## 10. Prestige: Submit Your Thesis

Complete a major research project and submit your findings to the Galactic Academy. Your local
operation resets, and you earn **Research Grants** (Academic Prestige).

### 10.1 What persists

- Specimen encyclopedia
- Permanent discoveries
- Unlocked base themes
- Academy upgrades

### 10.2 What Grants buy

- Better starting ships
- Automated logistics
- Faster laboratories
- New research disciplines (including new trait families, see section 5)

Grant rewards favor **discoveries and completed experiments** over raw Research, so each run has a
reason to chase different targets.

**Design rule:** essential automation (queues, auto-dispatch basics) is available in run 1.
Prestige reveals interesting decisions; it never gates convenience behind three resets.

### 10.3 The run setup: thesis + base + three discoveries

Each prestige begins with three choices that together define the run:

1. **A thesis** (the run's objective and rules)
2. **A base theme** (section 11)
3. **Three equipped discoveries** (sidegrades from the encyclopedia)

#### Thesis assignments

At the start of each run the Academy offers three theses. Examples:

| Thesis | Rule / focus |
|---|---|
| "Nobody Will Believe You" | Study conspiracy theorists and skeptics. Witnesses behave differently. |
| "Corporate Wants Results" | High-volume studies on a limited operating budget. |
| "The Human Resources Department" | Collect specific occupations. Your supervisor misunderstands what "human resources" means. |
| "A Controlled Experiment" | Use one UFO class only; bonus prestige on completion. |
| "Ethics Committee Inspection" | Experiments must meet absurd paperwork requirements. The committee mostly cares about improperly labeled storage. |

Each thesis should favor a different fleet, base layout, or specimen mix. Restrictive theses are
optional challenges with higher rewards.

### 10.4 Equipped discoveries (sidegrades)

The encyclopedia unlocks discoveries with both an upside and a cost. The player equips **three**
per run, turning the collection into a build system rather than a pile of bonuses.

| Discovery | Upside | Cost |
|---|---|---|
| Squirrel escape behavior | Faster specimen handling | Higher escape risk |
| Corporate managers | Laboratory coordination bonus | Mandatory meetings pause labs |
| Influencers | Public manipulation (suspicion control) | Some subjects livestream their abduction |
| Insomniacs | Overnight / offline research bonus | Harder containment |

**[Gap fill]** Additional equip slots can be earned at high Academy rank, capped at five.

### 10.5 The performance review

On submission the Academy grades the thesis and issues a performance review:

> "Excellent contribution to human biology. Unfortunately, your findings contradict the department
> head's published theory. Funding reduced."

**Rule:** the review is flavor. Rewards are calculated from transparent rules; a funny review never
unexpectedly invalidates a successful run.

### 10.6 Academy ranks

Junior Abductor → Senior Probe Technician → **[Gap fill]** Associate Professor of Applied Lifting →
Dean of Unscheduled Departures → Director of Involuntary Participation.

## 11. Base themes

Unlock themes through milestones, then revisit them freely. No fixed location sequence is required.
Each theme changes available specimens, constraints, and its own research branch.

| Theme | Setting | Specimens | Strengths | Constraints |
|---|---|---|---|---|
| Mountain | Cliff hangars, forests, farms, original station | Animals, rural humans | Low surveillance, strong heavy lifters | Fewer rare occupations |
| Oceanic | Underwater dome | Coastal humans, aquatic specimens | Secrecy, pressure-resistant ships | Constrained storage |
| Antarctic | Ice tunnels | Polar wildlife, isolated research crews | Scarce but valuable specimens | Long journeys, heat and energy management |
| Metropolis | Disguised skyscraper HQ | Abundant humans, rare occupations | Exceptional variety, fast collection | Heavy surveillance, constant attention |

## 12. Tone and humor guide

**Register:** cartoon science fiction with dark workplace comedy. The aliens are earnest,
bureaucratic, and completely wrong about Earth.

**Do**

- Treat horrifying situations as routine administration:
  - An escape is logged as "unscheduled specimen departure."
  - Overcrowding is "an exciting opportunity for social research."
  - Memory wiping comes with a customer satisfaction survey.
  - A failed experiment produces a report titled "Results Were Technically Obtained."
- Let jokes accumulate in the base (memos, cake, posters, staff habits).
- Punch at institutions: academia, corporate culture, HR, grant committees.

**Don't**

- Show gore, injury, or suffering. Experiments are absurd, not cruel.
- Make real groups the butt of the joke; occupations and quirks are fair game, identity is not.
- Let randomized humor change rewards (see 10.5).

**Voice samples**

- Deputy memo: "Specimen #0412 has requested 'five more minutes.' We have granted them. Study is
  now in its third day."
- Patch note on the bulletin board: "Cake is no longer optional."

## 13. Open questions

1. **Target platforms.** The engine is decided (Unreal Engine 5.8.3). PC only, or also mobile or
   console? This affects UI layout and offline-progress handling.
2. **Monetization.** Premium, ad-supported, or none? This affects offline caps and speed-up design.
3. **Art direction.** Pixel art, flat vector, or hand-drawn cartoon?
4. **Base view.** Side-on cutaway (ant-farm style) or top-down? The "discoveries change the base"
   hook leans toward a cutaway.
5. **Offline cap.** Is 8 hours the right default?
6. **First thesis.** Is run 1 a fixed tutorial thesis, or does it offer a choice immediately?
7. **Thesis completion length.** Target time for a first prestige (suggestion: 2–4 hours of active
   play, or 1–2 days idle).
8. **Equip slots.** Fixed at three, or growable (current gap fill: up to five)?
