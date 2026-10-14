#pragma once

#include "CoreMinimal.h"

// Prototype numbers. Keep in sync with docs/prototype-spec.md.

enum class ESpecimen : uint8
{
	Squirrel,
	Cow,
	Elephant,
	Human,
	// Oceanic
	Pufferfish,
	Octopus,
	Whale,
	// Antarctic
	Penguin,
	Seal,
	ElephantSeal,
	Count,
};

enum class EShipSystem : uint8
{
	Thrusters,
	Tractor,
	Cargo,
	Stealth,
	Scanner,
	Count,
};

enum class EBaseUpgrade : uint8
{
	Containment,
	Chambers,
	Power,
	Hangar,
	Camouflage, // research camouflage: halves radar attention
	Cleanup,    // cleanup technology: halves the fallout of rival breaches
	TidalTurbines, // Oceanic only: power that rises and falls with the tide; loads the hull
	DeepTanks,     // Oceanic only: big pressurised containment; loads the hull
	SonarArray,    // any base once Whale Song Transcripts is published: instant scanning, more radar attention
	InkScreens,    // any base once Ink Diplomacy is published: quieter trips, hungrier launches
	GeothermalTaps, // Antarctic only: heat; the steam plume draws radar
	CryoVault,      // Antarctic only: containment that never escapes; preserves twice as fast; draws energy
	InsulatedLabs,  // any base once Huddle Theory is published: experiments use 30% less energy
	CryoPods,       // any base once Blubber Economics is published: preserved samples anywhere; draws energy
	CalibrationDesk, // after a few probe calibrations: a technician post that takes calibration windows you leave
	Count,
};

/** Probe Calibration: how well a calibration went. Better grades skip more of the experiment's remaining time. */
enum class ECalibrationGrade : uint8
{
	Missed,     // stopped outside the band, or ran out of time: any attempt still helps
	Steady,     // inside the band
	Precise,    // inside the bright core of the band
	Quick,      // the no-timing button (accessibility): same as a steady stop
	Technician, // automatic, from the Calibration Desk: weaker than any manual attempt but never sleeps
	Count,
};

/** Rival research teams. Each assignment features a pairing of them. */
enum class ERival : uint8
{
	EfficiencyBureau,
	CelebrityStudies,
	ExperimentalSciences,
	Count,
};

/** The human trait pool. Each human has one occupation, and it changes how they behave in the base. */
enum class EOccupation : uint8
{
	None,
	Accountant,
	Astronomer,
	Magician,
	Lawyer,
	Therapist,
	MallSanta,
	Politician,
	// Metropolis only
	Influencer,
	ConspiracyTheorist,
	Celebrity,
	// Antarctic only
	PolarResearcher,
	// Case-file subjects only (never rolled at random)
	Journalist,
	Count,
};

/** Where the base is. Metropolis unlocks with the first Academy promotion. */
enum class ELocation : uint8
{
	Mountain,
	Metropolis,
	Oceanic,
	Antarctic,
	Count,
};

/** Thesis assignments: the run's objectives and its one rule. */
enum class EThesis : uint8
{
	PreliminaryObservations, // run 1
	NobodyWillBelieveYou,
	CorporateWantsResults,
	ControlledExperiment,
	DeepEnd, // Oceanic only
	ColdStorage, // Antarctic only
	Count,
};

/** How Earth pushes back in a region. Effects are soft: slower or less profitable, never losses. */
enum class EResponse : uint8
{
	NightPatrols,  // repeated animal disappearances
	Investigators, // frequent human abductions
	Radar,         // heavy UFO traffic
	SupportGroup,  // containment escapes and unwiped returns
	Count,
};

/** World conditions chosen at the start of a run (after the first promotion). */
enum class EEarthCondition : uint8
{
	BusinessAsUsual,
	DocumentaryCraze,
	SurveillanceCrackdown,
	EverythingIsAdvertising,
	Count,
};

/** Grant upgrades: few and decisive, no small percentage bonuses. Bought with Research Grants, kept forever. */
enum class EGrantUpgrade : uint8
{
	BetterStartingUfo,
	SpecialistRecruit,
	ImprovedScanning,
	Count,
};

/** Crew specialties. Anyone can fill a basic post; specialists add bonuses at matching stations. */
enum class ESpecialty : uint8
{
	Pilot,      // UFOs
	Scientist,  // laboratories
	Technician, // power, containment, memory-wipe
	Count,
};

/** One-off discovery projects. They pay no research; they unlock targets and equipment. */
enum class EDiscovery : uint8
{
	AcornAmnesia,
	UnscheduledDepartures,
	HerdBehaviour,
	BovineLoadTolerance,
	TotalRecall,
	PachydermLogistics,
	VoluntaryMeetings,
	SarcasmOrgan,
	// Oceanic
	InflationPolicy,
	EightArmedBureaucracy,
	InkDiplomacy,
	WhaleSong,
	// Antarctic
	HuddleTheory,
	BlubberEconomics,
	Count,
	None = Count,
};

enum class EExperimentKind : uint8
{
	Routine,   // quick, pays research
	Extended,  // slower and hungrier, pays more research per minute
	Discovery, // one-off project that unlocks something
	Signature, // a department's signature experiment: several specimens, staffed facilities, big payoff
	CaseStudy, // a recurring subject's personal research chain (or the rivals' Peer Review)
};

/** Research departments. The primary one is cheap; the others cost more but are never locked out. */
enum class EDepartment : uint8
{
	None,
	Biological,   // animal collection and cross-species experiments
	Human,        // rare traits and specific occupations
	Incompetence, // risky experiments and questionable engineering
	Count,
};

/** Three projects per department, bought in order with research. Each changes a mechanic. */
enum class EDeptProject : uint8
{
	ReinforcedHabitats,
	HeavyTractorArray,
	BatchExperiments,
	AdvancedScanners,
	BehavioralLab,
	MemoryManipulation,
	ExperimentalModules,
	UnusualPower,
	AutomatedCoverStories,
	Count,
};

struct FSpecimenDef
{
	const TCHAR* Name;
	const TCHAR* Plural;
	const TCHAR* Region;
	float FlightSeconds;      // per leg, at thrusters level 0
	float CaptureSeconds;     // on site, before cargo/alert modifiers
	float LabSeconds;         // routine study
	double LaunchEnergy;      // before tractor modifier
	double ExperimentEnergy;  // routine study
	double Research;          // routine study
	int32 Weight;             // tractor strength needed
	int32 ContainmentUnits;   // habitat space taken
	double SuspicionPerSpecimen;
	double EscapePerSecond;   // chance per second while waiting in containment
	EDiscovery UnlockedBy;    // EDiscovery::None = available from the start
	const TCHAR* Encyclopedia;
};

struct FOccupationDef
{
	const TCHAR* Name;
	const TCHAR* TraitText;   // shown in the console
	double ResearchMultiplier;
	float DurationMultiplier;
	int32 ContainmentUnits;
	double SuspicionMultiplier;
	double EscapeMultiplier;
	float DiscoverySpeedMultiplier;
	const TCHAR* Encyclopedia;
};

struct FDiscoveryDef
{
	const TCHAR* Name;
	const TCHAR* Question;    // what the project investigates
	ESpecimen Specimen;
	EOccupation RequiredOccupation;
	int32 RequiredExperiments; // routine/extended experiments on this specimen before the project opens
	float Seconds;
	double Energy;
	const TCHAR* Unlocks;     // short effect text
	const TCHAR* Report;      // log + encyclopedia text
};

struct FDeptProjectDef
{
	const TCHAR* Name;
	EDepartment Department;
	int32 Tier;      // 0..2, bought in order within the department
	double Cost;     // for the primary department
	const TCHAR* Effect;
	const TCHAR* Flavor;
};

struct FDepartmentDef
{
	const TCHAR* Name;
	const TCHAR* Focus;
	const TCHAR* Unlocks;
	const TCHAR* UniformNote;
	const TCHAR* SignatureName;
	const TCHAR* SignatureQuestion;
	// The signature experiment needs these three specimens in containment at once...
	ESpecimen NeedType[3];
	EOccupation NeedOccupation[3];
	// ...and these facilities staffed (the console spells it out).
	const TCHAR* FacilityText;
	const TCHAR* SignatureReport;
	const TCHAR* SignatureBonus;
};

struct FUpgradeDef
{
	const TCHAR* Name;
	double BaseCost;
	double CostGrowth;
	int32 MaxLevel;
	int32 SlotsPerLevel;      // ship systems only
};

namespace AlienIdleTuning
{
	inline constexpr FSpecimenDef Specimens[] = {
		{ TEXT("squirrel"), TEXT("squirrels"), TEXT("Pine Ridge"),     6.0f,  4.0f, 10.0f, 4.0,  6.0,  12.0,  1, 1,  4.0, 0.003,  EDiscovery::None,
			TEXT("A small tree-dwelling accountant that stores assets underground and never audits them.") },
		{ TEXT("cow"),      TEXT("cows"),      TEXT("Dairy Flats"),    9.0f,  5.0f, 20.0f, 6.0,  15.0, 45.0,  3, 2,  8.0, 0.0005, EDiscovery::AcornAmnesia,
			TEXT("Grass-to-milk converter. Appears to be thinking about something. It is not.") },
		{ TEXT("elephant"), TEXT("elephants"), TEXT("Circus Grounds"), 14.0f, 8.0f, 35.0f, 10.0, 40.0, 160.0, 8, 4, 20.0, 0.0003, EDiscovery::HerdBehaviour,
			TEXT("Remembers everything, forgives nothing. Possibly middle management.") },
		{ TEXT("human"),    TEXT("humans"),    TEXT("Glenwhit"),       9.0f,  5.0f, 25.0f, 6.0,  30.0, 110.0, 1, 1, 15.0, 0.001,  EDiscovery::TotalRecall,
			TEXT("Dominant species. Spends a third of its life unconscious and much of the rest in meetings.") },
		{ TEXT("pufferfish"), TEXT("pufferfish"), TEXT("Coral Shallows"), 5.0f, 4.0f, 10.0f, 4.0,  6.0,  14.0,  1, 1,  2.0, 0.002,  EDiscovery::None,
			TEXT("Doubles in size when stressed. Staff find this relatable and have asked for the same benefit.") },
		{ TEXT("octopus"),  TEXT("octopuses"), TEXT("Coral Shallows"), 8.0f,  6.0f, 25.0f, 7.0,  18.0, 70.0,  3, 2,  5.0, 0.006,  EDiscovery::InflationPolicy,
			TEXT("Eight arms, three hearts, no respect for latches. Has already applied for a management position.") },
		{ TEXT("whale"),    TEXT("whales"),    TEXT("The Trench"),     12.0f, 9.0f, 40.0f, 12.0, 45.0, 200.0, 5, 5, 10.0, 0.0002, EDiscovery::EightArmedBureaucracy,
			TEXT("The largest animal ever studied, and surprisingly light in water. Sings about it constantly. The lyrics are mostly complaints about krill.") },
		{ TEXT("penguin"),  TEXT("penguins"),  TEXT("The Ice Shelf"),  6.0f,  4.0f, 12.0f, 5.0,  7.0,  13.0,  1, 1,  3.0, 0.0015, EDiscovery::None,
			TEXT("Dressed for a formal event it will never attend. Waddles with the confidence of senior management.") },
		{ TEXT("seal"),     TEXT("seals"),     TEXT("The Ice Shelf"),  9.0f,  5.0f, 22.0f, 7.0,  16.0, 50.0,  3, 2,  6.0, 0.0006, EDiscovery::AcornAmnesia,
			TEXT("A sausage with opinions. Claps when pleased, which is never, for us.") },
		{ TEXT("elephant seal"), TEXT("elephant seals"), TEXT("Elephant Seal Beach"), 14.0f, 8.0f, 35.0f, 11.0, 40.0, 170.0, 8, 4, 12.0, 0.0003, EDiscovery::HerdBehaviour,
			TEXT("Neither an elephant nor, emotionally, a seal. Roars at the paperwork.") },
	};
	static_assert(UE_ARRAY_COUNT(Specimens) == int32(ESpecimen::Count));
	inline const FSpecimenDef& Specimen(ESpecimen Type) { return Specimens[int32(Type)]; }

	// Index 0 (None) is a placeholder so the table lines up with EOccupation.
	inline constexpr FOccupationDef Occupations[] = {
		//  Name                 Trait                                                         Res   Dur   Units Susp Escape Disc
		{ TEXT("Unknown"),    TEXT(""),                                                          1.0, 1.0f, 1, 1.0, 1.0, 1.0f, TEXT("") },
		{ TEXT("Accountant"), TEXT("+50% research. Escapes trigger an inventory reorganisation."), 1.5, 1.0f, 1, 1.0, 1.0, 1.0f,
			TEXT("A human who converts anxiety into spreadsheets.") },
		{ TEXT("Astronomer"), TEXT("Discovery projects run twice as fast."),                      1.0, 1.0f, 1, 1.0, 1.0, 0.5f,
			TEXT("Looks at space for a living. Has not noticed us, which is professionally embarrassing for one of us.") },
		{ TEXT("Magician"),   TEXT("Needs 2 containment units. Escapes six times as often."),     1.2, 1.0f, 2, 1.0, 6.0, 1.0f,
			TEXT("Specialises in unscheduled departures. A professional rival.") },
		{ TEXT("Lawyer"),     TEXT("Experiments take +50% longer pending consent-form clarification. +25% research."), 1.25, 1.5f, 1, 1.0, 1.0, 1.0f,
			TEXT("Produces words that make other words unusable.") },
		{ TEXT("Therapist"),  TEXT("While on base, every chamber works 20% faster."),             1.0, 1.0f, 1, 1.0, 1.0, 1.0f,
			TEXT("Charges humans to listen to them. We do this for free and are called 'abductors.'") },
		{ TEXT("Mall Santa"), TEXT("+50% research, but double suspicion when taken."),           1.5, 1.0f, 1, 2.0, 1.0, 1.0f,
			TEXT("Seasonal deity. Worship involves photographs and crying.") },
		{ TEXT("Politician"), TEXT("+30% research. +50% suspicion when taken. Cannot be held responsible."), 1.3, 1.0f, 1, 1.5, 1.0, 1.0f,
			TEXT("Speaks for an hour without saying anything. Staff are taking notes.") },
		{ TEXT("Influencer"), TEXT("Double research. Livestreams the abduction: triple suspicion."), 2.0, 1.0f, 1, 3.0, 1.0, 1.0f,
			TEXT("Broadcasts every moment of its life, including this one. Engagement is up.") },
		{ TEXT("Conspiracy Theorist"), TEXT("Returning one removes 15 suspicion: nobody believes its story."), 1.2, 1.0f, 1, 1.0, 1.0, 1.0f,
			TEXT("Has warned everyone about us for years. Nobody listens. We intend to keep it that way.") },
		{ TEXT("Celebrity"), TEXT("Triple research. Media frenzy: quadruple suspicion."), 3.0, 1.0f, 1, 4.0, 1.0, 1.0f,
			TEXT("Famous for being famous. Its disappearance trended for six hours, then was replaced by a cat.") },
		{ TEXT("Polar Researcher"), TEXT("+40% research and discovery projects twice as fast. Already dressed for containment."), 1.4, 1.0f, 1, 1.0, 1.0, 0.5f,
			TEXT("Came to the end of the world to be alone with data. Is now data. Has asked to see the methodology.") },
		{ TEXT("Journalist"), TEXT("Case-file subject only. Asks questions and writes the answers down. +20% research, +50% suspicion when taken."), 1.2, 1.0f, 1, 1.5, 1.0, 1.0f,
			TEXT("A human who finds things out for a living and then tells everyone. Professionally, our exact opposite.") },
	};
	static_assert(UE_ARRAY_COUNT(Occupations) == int32(EOccupation::Count));
	inline const FOccupationDef& Occupation(EOccupation Type) { return Occupations[int32(Type)]; }

	inline constexpr FDiscoveryDef Discoveries[] = {
		{ TEXT("Acorn Amnesia"), TEXT("Why do squirrels forget where they buried things?"), ESpecimen::Squirrel, EOccupation::None, 8, 20.0f, 10.0,
			TEXT("Unlocks Dairy Flats (cows)."),
			TEXT("Squirrels do not forget. They simply stop caring. Cow studies approved.") },
		{ TEXT("Unscheduled Departures"), TEXT("How do squirrels keep escaping?"), ESpecimen::Squirrel, EOccupation::None, 12, 25.0f, 10.0,
			TEXT("Halves escapes. Unlocks Containment Habitat III+."),
			TEXT("Squirrels escape for the same reason staff do: the door was there. New latches ordered.") },
		{ TEXT("Herd Behaviour"), TEXT("Where do cows go when nobody is watching?"), ESpecimen::Cow, EOccupation::None, 6, 40.0f, 25.0,
			TEXT("Unlocks Circus Grounds (elephants)."),
			TEXT("Repeated cow disappearances are now blamed on 'agricultural migration.' Elephant studies approved.") },
		{ TEXT("Bovine Load Tolerance"), TEXT("How heavy is a cow, really?"), ESpecimen::Cow, EOccupation::None, 4, 40.0f, 25.0,
			TEXT("Unlocks Tractor Beam III+ (strength 5 and up)."),
			TEXT("A cow is heavier than it looks, mostly emotionally. Heavy tractor beams certified.") },
		{ TEXT("Total Recall"), TEXT("Does the elephant remember us?"), ESpecimen::Elephant, EOccupation::None, 4, 60.0f, 50.0,
			TEXT("Unlocks Glenwhit (humans) and the Trait Scanner."),
			TEXT("The elephant remembers everything, including our faces. Human studies approved, with scanners.") },
		{ TEXT("Pachyderm Logistics"), TEXT("How does a herd move without a project manager?"), ESpecimen::Elephant, EOccupation::None, 4, 60.0f, 50.0,
			TEXT("Unlocks Experiment Chamber III and Hangar Bay III."),
			TEXT("Elephants move as a committee and arrive anyway. We have ordered a third chamber and a third hangar out of spite.") },
		{ TEXT("Voluntary Meetings"), TEXT("Why do humans willingly attend meetings?"), ESpecimen::Human, EOccupation::Accountant, 2, 45.0f, 40.0,
			TEXT("Unlocks Middle Management: labs +20% faster, with mandatory meetings."),
			TEXT("Humans do not attend meetings willingly. They are summoned. An alien has been promoted to management to test this.") },
		{ TEXT("The Sarcasm Organ"), TEXT("Where is the human sarcasm organ?"), ESpecimen::Human, EOccupation::Lawyer, 2, 45.0f, 40.0,
			TEXT("Human research +50%."),
			TEXT("Located the sarcasm organ. It is the entire face. Human findings now read correctly, which is to say the opposite.") },
		{ TEXT("Inflation Policy"), TEXT("Why does the pufferfish panic-inflate?"), ESpecimen::Pufferfish, EOccupation::None, 6, 25.0f, 12.0,
			TEXT("Unlocks octopuses (Coral Shallows, The Trench)."),
			TEXT("The pufferfish inflates because it has been asked to do more with less. Finance has requested the technique. Octopus studies approved.") },
		{ TEXT("Eight-Armed Bureaucracy"), TEXT("How does an octopus do eight jobs at once?"), ESpecimen::Octopus, EOccupation::None, 4, 45.0f, 30.0,
			TEXT("Unlocks whales (The Trench)."),
			TEXT("It does not do eight jobs. It does one job eight times and bills for all of them. Whale studies approved.") },
		{ TEXT("Ink Diplomacy"), TEXT("Can a cloud of ink end a conversation?"), ESpecimen::Octopus, EOccupation::None, 3, 40.0f, 30.0,
			TEXT("Unlocks Harbour Town humans here; at every base, unlocks Ink Screen Launchers."),
			TEXT("A well-timed cloud of ink ends any meeting. Humans approved for study, and every base may now fit ink screens to its launch tubes.") },
		{ TEXT("Whale Song Transcripts"), TEXT("What is the whale singing about?"), ESpecimen::Whale, EOccupation::None, 3, 60.0f, 50.0,
			TEXT("At every base, unlocks the Sonar Array. Here: offers the Submersible Hulls benefit at promotion."),
			TEXT("Transcribed: forty minutes about krill, one verse about us. The verse is unflattering. Sonar Arrays approved for every base.") },
		{ TEXT("Huddle Theory"), TEXT("Why do penguins huddle?"), ESpecimen::Penguin, EOccupation::None, 6, 30.0f, 15.0,
			TEXT("Here: the base needs 25% less heat. At every base: unlocks Insulated Labs."),
			TEXT("Penguins huddle because the one in the middle is warm and the ones outside are waiting for a promotion. Insulated Labs approved for every base.") },
		{ TEXT("Blubber Economics"), TEXT("How does a seal stay warm on no budget?"), ESpecimen::Seal, EOccupation::None, 4, 40.0f, 25.0,
			TEXT("At every base: unlocks Cryo Pods (preserved samples anywhere). Offers Polar Crew Program at promotion here."),
			TEXT("The seal stays warm by being mostly insulation, a strategy Facilities has proposed for the staff. Cryo Pods approved for every base.") },
	};
	static_assert(UE_ARRAY_COUNT(Discoveries) == int32(EDiscovery::Count));
	inline const FDiscoveryDef& Discovery(EDiscovery Type) { return Discoveries[int32(Type)]; }

	// Research departments
	inline constexpr FDepartmentDef Departments[] = {
		{ TEXT("None"), TEXT(""), TEXT(""), TEXT(""), TEXT(""), TEXT(""),
			{ ESpecimen::Squirrel, ESpecimen::Squirrel, ESpecimen::Squirrel }, { EOccupation::None, EOccupation::None, EOccupation::None },
			TEXT(""), TEXT(""), TEXT("") },
		{ TEXT("Biological Studies"), TEXT("Animal collection and cross-species experiments"),
			TEXT("improved habitats, heavy tractor beams, batch experiments"), TEXT("green lab coats"),
			TEXT("The Great Chain of Being"), TEXT("Which animal is in charge?"),
			{ ESpecimen::Squirrel, ESpecimen::Cow, ESpecimen::Elephant }, { EOccupation::None, EOccupation::None, EOccupation::None },
			TEXT("a containment warden and a memory-wipe operator"),
			TEXT("The squirrel, cow and elephant were asked to form a hierarchy. The elephant declined, the cow abstained, and the squirrel declared itself regional manager."),
			TEXT("animal research +25%") },
		{ TEXT("Human Behavior"), TEXT("Rare traits and specific occupations"),
			TEXT("advanced scanners, behavioral labs, memory manipulation"), TEXT("grey suits and clipboards"),
			TEXT("Who Is Responsible?"), TEXT("Which participant will accept accountability?"),
			{ ESpecimen::Human, ESpecimen::Human, ESpecimen::Squirrel }, { EOccupation::Accountant, EOccupation::Politician, EOccupation::None },
			TEXT("a containment warden and a memory-wipe operator"),
			TEXT("Responsibility study: the accountant blamed the budget, the politician blamed the accountant. The squirrel was the only participant willing to accept accountability. Further investigation required."),
			TEXT("human research +25%") },
		{ TEXT("Applied Incompetence"), TEXT("Risky experiments and questionable engineering"),
			TEXT("experimental UFO modules, unusual power systems, automated cover stories"), TEXT("scorched overalls and hard hats"),
			TEXT("Controlled Explosion"), TEXT("Can something explode exactly as planned?"),
			{ ESpecimen::Squirrel, ESpecimen::Cow, ESpecimen::Human }, { EOccupation::None, EOccupation::None, EOccupation::Magician },
			TEXT("two crew on the power station"),
			TEXT("The experiment exploded exactly as planned. Nobody had planned it. The magician claims it was part of the act and has asked for a fee."),
			TEXT("every incident teaches a lesson worth 60 research") },
	};
	static_assert(UE_ARRAY_COUNT(Departments) == int32(EDepartment::Count));
	inline const FDepartmentDef& Department(EDepartment Type) { return Departments[int32(Type)]; }

	inline constexpr FDeptProjectDef DeptProjects[] = {
		{ TEXT("Reinforced Habitats"), EDepartment::Biological, 0, 120.0, TEXT("+4 containment units; animals escape half as often."),
			TEXT("Habitats reinforced. The elephant has been told it is 'a feature wall.'") },
		{ TEXT("Heavy Tractor Array"), EDepartment::Biological, 1, 350.0, TEXT("+3 tractor strength on every ship; tractor energy penalty halved."),
			TEXT("Heavy Tractor Array online. Cows are now lifted with dignity, or at least with less effort.") },
		{ TEXT("Batch Experiments"), EDepartment::Biological, 2, 800.0, TEXT("Animal studies take two of the same species at once for double research."),
			TEXT("Batch processing approved. The specimens have formed a union.") },
		{ TEXT("Advanced Scanners"), EDepartment::Human, 0, 120.0, TEXT("Scanning takes no extra time; human research +25%."),
			TEXT("Advanced Scanners installed. They read job titles from orbit and judge them silently.") },
		{ TEXT("Behavioral Lab"), EDepartment::Human, 1, 350.0, TEXT("Human experiments 30% faster; lawyers no longer delay them."),
			TEXT("Behavioral Lab opened. It has a two-way mirror, and both sides are uncomfortable.") },
		{ TEXT("Memory Manipulation"), EDepartment::Human, 2, 800.0, TEXT("Each returned human removes 6 suspicion from its region (needs a wipe operator)."),
			TEXT("Memory Scrambler online. Subjects retain only a vague sense that customer service was disappointing.") },
		{ TEXT("Experimental UFO Modules"), EDepartment::Incompetence, 0, 120.0, TEXT("+2 upgrade slots on every ship; 10% of launches suffer a slow 'learning experience'."),
			TEXT("Experimental modules fitted. The instructions were in a language nobody speaks, including the authors.") },
		{ TEXT("Unusual Power Systems"), EDepartment::Incompetence, 1, 350.0, TEXT("+3 energy/s and +60 storage; equipment fails 50% more often."),
			TEXT("Unusual Power Systems installed. The generator now runs on spite and two hamsters with ambition.") },
		{ TEXT("Automated Cover Stories"), EDepartment::Incompetence, 2, 800.0, TEXT("Suspicion fades twice as fast; lockdowns lift at 60 instead of 40."),
			TEXT("Automated Cover Stories online. Tonight's explanation: 'a particularly ambitious owl, again.'") },
	};
	static_assert(UE_ARRAY_COUNT(DeptProjects) == int32(EDeptProject::Count));
	inline const FDeptProjectDef& DeptProject(EDeptProject Type) { return DeptProjects[int32(Type)]; }

	inline bool IsAntarcticOccupation(EOccupation Occupation) { return Occupation == EOccupation::PolarResearcher; }
	/** Professions only recurring case-file subjects have: never rolled at random, never scanned for. */
	inline bool IsCaseOccupation(EOccupation Occupation) { return Occupation == EOccupation::Journalist; }
	inline bool IsMetropolisOccupation(EOccupation Occupation)
	{
		return Occupation == EOccupation::Influencer || Occupation == EOccupation::ConspiracyTheorist || Occupation == EOccupation::Celebrity;
	}

	// Locations: region names per specimen and how the city changes things.
	inline constexpr const TCHAR* LocationNames[] = { TEXT("Mountain Station 7"), TEXT("Metropolis Tower"), TEXT("Trench Station 9"), TEXT("Station Permafrost") };
	inline constexpr const TCHAR* LocationBlurbs[] = {
		TEXT("Cliff hangars, forests, farms. Low surveillance, few rare occupations."),
		TEXT("A disguised office tower. Humans everywhere (three extra professions), short flights downtown, heavy surveillance: suspicion x1.5 and alerts at 35."),
		TEXT("A pressurised dome on the sea floor. Aquatic specimens and tidal power; cramped (habitat capped at II), generators cost 1.5x but a tidal turbine comes free; hull pressure needs technicians; trips to land are costly surface expeditions."),
		TEXT("An ice-shelf outpost. Penguins, seals and polar researchers; heat must be shared between labs and hangar; long supply routes: upgrades cost 25% more and flights are long, but recruits arrive trained and containment preserves samples."),
	};
	static_assert(UE_ARRAY_COUNT(LocationNames) == int32(ELocation::Count) && UE_ARRAY_COUNT(LocationBlurbs) == int32(ELocation::Count));

	/** Academy achievements unlock new postings (Metropolis simply opens with the first promotion). */
	struct FAchievementDef
	{
		const TCHAR* Name;
		const TCHAR* Requirement;
		ELocation Unlocks;
	};
	inline constexpr FAchievementDef Achievements[] = {
		{ TEXT("Heavy Lifting"), TEXT("study 8 elephants in one run"), ELocation::Oceanic },
		{ TEXT("Tenure Track"), TEXT("submit two theses"), ELocation::Antarctic },
	};
	inline constexpr int32 TenureTrackTheses = 2;

	// Antarctic: heat is made by a base supply, the Boiler Room crew and geothermal taps, and shared between labs and hangar.
	inline constexpr float HeatBase = 2.0f;
	inline constexpr float HeatPerBoilerCrew = 4.0f;     // x (1 + bonus)
	inline constexpr float HeatPerTap = 3.0f;
	inline constexpr float HeatPerChamber = 1.5f;        // demand
	inline constexpr float HeatPerUfo = 1.5f;            // demand
	inline constexpr float HuddleHeatDemand = 0.75f;
	inline constexpr float ColdLabSpeedFloor = 0.4f;     // labs at no heat run at 40%
	inline constexpr float ColdFlightPenalty = 0.6f;     // a frozen hangar: flights +60%
	inline constexpr double ColdLaunchEnergyPenalty = 0.5; // and launches +50% energy (de-icing)
	inline constexpr double AntarcticUpgradeCostMultiplier = 1.25; // long supply routes
	inline constexpr double AntarcticRecruitCostMultiplier = 1.4;
	inline constexpr float PreserveSeconds = 120.0f;     // held this long, a sample is fully preserved
	inline constexpr double PreserveResearchBonus = 0.5; // +50% research when fully preserved
	inline constexpr float PreservedSampleSeconds = 60.0f; // counts as a preserved sample for the thesis
	inline constexpr int32 CryoVaultUnits = 4;
	inline constexpr int32 CryoPodUnits = 3;
	inline constexpr double CryoEnergyPerLevel = 0.5;    // per second
	inline constexpr float HeatingCutMultiplier = 0.5f;
	inline constexpr float OverheatRatio = 1.6f;          // heat supply this far above demand: the base overheats (flavour, and a little suspicion)
	inline constexpr int32 HeavyLiftingElephants = 8;

	// Oceanic: hull pressure (0..100). It moves toward 50 x stress / relief, so the meter settles where the load and pumps balance.
	inline constexpr float HullStressPerUnit = 0.04f;      // per containment unit in use, per second
	inline constexpr float HullStressPerTank = 0.15f;      // per Deep-Water Tank level
	inline constexpr float HullStressPerTurbine = 0.08f;   // per Tidal Turbine level
	inline constexpr float HullStressPerSurfaceLaunch = 7.0f; // airlock cycle, instant
	inline constexpr float HullBaseRelief = 0.3f;
	inline constexpr float HullCrewRelief = 0.9f;          // per staffed Pressure Control, x (1 + bonus)
	inline constexpr float HullCreakPressure = 60.0f;      // labs 20% slower at or above
	inline constexpr float HullLeakPressure = 85.0f;       // surface airlock seals until below the creak level
	inline constexpr float HullCreakLabSpeed = 0.8f;
	inline constexpr double SurfaceLaunchEnergyMultiplier = 2.0;
	inline constexpr int32 OceanicContainmentMaxLevel = 2;
	inline constexpr double OceanicPowerCostMultiplier = 1.5; // pressure-rated generators: tidal turbines are the cheap power down here
	inline constexpr int32 DeepTankUnits = 5;
	inline constexpr double TidalEnergyPerLevel = 3.0;    // at high tide; nothing at low tide
	inline constexpr float TidePeriodSeconds = 240.0f;
	inline constexpr float WhaleIncidentPressure = 20.0f;
	inline constexpr float MetropolisHumanFlightMultiplier = 0.6f;
	inline constexpr double MetropolisSuspicionMultiplier = 1.5;
	inline constexpr double MetropolisAlertSuspicion = 35.0;

	struct FThesisDef
	{
		const TCHAR* Name;
		const TCHAR* Brief;
		const TCHAR* Rule;
		int32 Discoveries;          // objectives (0 = not required)
		int32 Contracts;
		int32 TotalExperiments;
		int32 DistinctProfessions;
		int32 DepartmentProjects;
		int32 BonusGrants;
		int32 MaxUfos;              // 0 = no limit
		int32 ExtraSlots;
		double SuspicionMultiplier;
		double RecruitCostMultiplier;
		bool bNoActiveResponses = false; // objective: submit with Earth calm (no active responses anywhere)
		ELocation RequiredLocation = ELocation::Count; // Count = any posting
		int32 LocalExperiments = 0;  // objective: experiments on the posting's native specimens
		float HullStressMultiplier = 1.0f;
		int32 PreservedSamples = 0;  // objective: samples studied after at least a minute in the freezer
		float HeatMultiplier = 1.0f;
	};
	inline constexpr FThesisDef Theses[] = {
		{ TEXT("Preliminary Observations of Earth"), TEXT("Establish that Earth exists and contains things."),
			TEXT("No special rules. The Academy is watching, mostly out of boredom."), 5, 3, 100, 0, 0, 0, 0, 0, 1.0, 1.0 },
		{ TEXT("Nobody Will Believe You"), TEXT("Covert: study six different professions and leave Earth calm when you submit."),
			TEXT("Witnesses talk: all suspicion x1.5. Stealth ships, a memory-wipe crew and regional rotation matter."), 0, 3, 0, 6, 0, 2, 0, 0, 1.5, 1.0, true },
		{ TEXT("Corporate Wants Results"), TEXT("Bulk: high-volume studies on a limited fleet."),
			TEXT("Hangar capped at 2 UFOs, but recruitment costs 30% less: big cargo bays, big containment, busy labs."), 2, 2, 150, 0, 0, 2, 2, 0, 1.0, 0.7 },
		{ TEXT("A Controlled Experiment"), TEXT("Precision: one fully specialised ship hunting the right people for the right contracts."),
			TEXT("Exactly one UFO, with 4 extra upgrade slots: scanners and specialists."), 0, 3, 0, 5, 2, 3, 1, 4, 1.0, 1.0 },
		{ TEXT("The Deep End"), TEXT("Oceanic only: 60 aquatic experiments, 3 discoveries and 2 contracts, without leaving the water much."),
			TEXT("Deep posting: hull stress x1.5. Dive ships, Deep-Water Tanks and a Pressure Control crew of technicians matter; surface trips hurt."),
			3, 2, 0, 0, 0, 2, 0, 0, 1.0, 1.0, false, ELocation::Oceanic, 60, 1.5f },
		{ TEXT("Cold Storage"), TEXT("Antarctic only: study 25 preserved samples, make 3 discoveries and finish 2 contracts on a reduced heating budget."),
			TEXT("Heating budget cut: heat x0.75. Boiler Room technicians, geothermal taps and patient containment matter; every extra ship is another mouth to heat."),
			3, 2, 0, 0, 0, 2, 0, 0, 1.0, 1.0, false, ELocation::Antarctic, 0, 1.0f, 25, 0.75f },
	};
	static_assert(UE_ARRAY_COUNT(Theses) == int32(EThesis::Count));
	inline const FThesisDef& Thesis(EThesis Type) { return Theses[int32(Type)]; }

	struct FGrantUpgradeDef
	{
		const TCHAR* Name;
		const TCHAR* Effect;
		int32 Cost;
	};
	inline constexpr FGrantUpgradeDef GrantUpgrades[] = {
		{ TEXT("Better Starting UFO"), TEXT("Old Reliable starts every run with Thrusters II, Tractor Beam II and Cargo Bay II."), 2 },
		{ TEXT("Specialist Starting Recruit"), TEXT("Every run starts with a fourth alien: a level-2 specialist of your choice."), 2 },
		{ TEXT("Improved Trait Scanning"), TEXT("Old Reliable starts with a Trait Scanner, scanners need no discovery, and scanning time is halved."), 3 },
	};
	static_assert(UE_ARRAY_COUNT(GrantUpgrades) == int32(EGrantUpgrade::Count));

	inline constexpr const TCHAR* Ranks[] = {
		TEXT("Junior Abductor"), TEXT("Senior Probe Technician"), TEXT("Associate Professor of Applied Lifting"),
		TEXT("Dean of Unscheduled Departures"), TEXT("Director of Involuntary Participation"),
	};

	// Grants
	inline constexpr int32 ThesisGrants = 4;
	inline constexpr int32 GrantsPerNewDiscovery = 1;
	inline constexpr float PublishedDiscoverySpeedup = 0.5f; // known discoveries take half the time next run

	// Regions: three competing hunting grounds per location. Value raises research, exposure raises suspicion.
	struct FRegionDef
	{
		const TCHAR* Name;
		const TCHAR* Blurb;
		ESpecimen Specimens[3];
		int32 NumSpecimens;
		double Value;
		double Exposure;
		float Flight;
		EOccupation Featured; // three times as likely among humans taken here
		uint8 Medium = 0;     // 0 land, 1 surface expedition (from an undersea base), 2 underwater
	};
	inline constexpr uint8 MediumLand = 0;
	inline constexpr uint8 MediumSurface = 1;
	inline constexpr uint8 MediumUnderwater = 2;
	inline constexpr int32 NumRegions = 3;
	inline constexpr FRegionDef RegionDefs[int32(ELocation::Count)][NumRegions] = {
		{
			{ TEXT("Pine Ridge Valley"), TEXT("Forests and dairy farms. Nobody counts the squirrels; the farmers count the cows."),
				{ ESpecimen::Squirrel, ESpecimen::Cow, ESpecimen::Cow }, 2, 1.0, 0.8, 1.0f, EOccupation::None },
			{ TEXT("Glenwhit"), TEXT("A small town with one of everything and a very active community newsletter."),
				{ ESpecimen::Squirrel, ESpecimen::Human, ESpecimen::Human }, 2, 1.2, 1.2, 1.0f, EOccupation::Accountant },
			{ TEXT("County Fairgrounds"), TEXT("Circus, fair and petting barn. Valuable, crowded and watched."),
				{ ESpecimen::Elephant, ESpecimen::Cow, ESpecimen::Human }, 3, 1.5, 1.5, 1.3f, EOccupation::Magician },
		},
		{
			{ TEXT("Central Park"), TEXT("Squirrels, joggers and one man who has been feeding pigeons since 1987."),
				{ ESpecimen::Squirrel, ESpecimen::Human, ESpecimen::Human }, 2, 1.0, 1.0, 1.0f, EOccupation::ConspiracyTheorist },
			{ TEXT("The Suburbs"), TEXT("Petting zoos, cul-de-sacs and neighbourhood watch committees."),
				{ ESpecimen::Cow, ESpecimen::Human, ESpecimen::Human }, 2, 1.1, 0.9, 1.0f, EOccupation::Accountant },
			{ TEXT("Downtown"), TEXT("The city zoo, the studios and city hall. Rare professions, cameras everywhere."),
				{ ESpecimen::Human, ESpecimen::Elephant, ESpecimen::Elephant }, 2, 1.6, 1.6, 1.0f, EOccupation::Celebrity },
		},
		{
			{ TEXT("Coral Shallows"), TEXT("A reef full of pufferfish and octopuses. Nobody up there counts fish, but the divers are nosy."),
				{ ESpecimen::Pufferfish, ESpecimen::Octopus, ESpecimen::Octopus }, 2, 1.0, 0.5, 0.8f, EOccupation::None, MediumUnderwater },
			{ TEXT("Harbour Town"), TEXT("Fish market, yacht club, lighthouse. Land specimens, reached by surface expedition: double energy and an airlock cycle."),
				{ ESpecimen::Squirrel, ESpecimen::Cow, ESpecimen::Human }, 3, 1.2, 1.0, 1.3f, EOccupation::Politician, MediumSurface },
			{ TEXT("The Trench"), TEXT("Dark, deep and valuable. Octopuses and whales, and absolutely no witnesses."),
				{ ESpecimen::Octopus, ESpecimen::Whale, ESpecimen::Whale }, 2, 1.6, 0.4, 1.4f, EOccupation::None, MediumUnderwater },
		},
		{
			{ TEXT("The Ice Shelf"), TEXT("Penguins and seals as far as the eye can see, and nobody to see it."),
				{ ESpecimen::Penguin, ESpecimen::Seal, ESpecimen::Seal }, 2, 1.0, 0.4, 1.4f, EOccupation::None },
			{ TEXT("Research Outpost"), TEXT("Twelve scientists, one cook and a satellite dish. Everybody notices when someone leaves."),
				{ ESpecimen::Human, ESpecimen::Penguin, ESpecimen::Penguin }, 2, 1.3, 1.1, 1.5f, EOccupation::PolarResearcher },
			{ TEXT("Elephant Seal Beach"), TEXT("Loud, smelly and enormously valuable. A long, cold flight."),
				{ ESpecimen::ElephantSeal, ESpecimen::Seal, ESpecimen::Seal }, 2, 1.5, 0.5, 1.7f, EOccupation::None },
		},
	};
	/** The region each specimen is first sought in (and the one older code paths report). */
	inline constexpr int32 HomeRegion[int32(ELocation::Count)][int32(ESpecimen::Count)] = {
		{ 0, 0, 2, 1, 0, 0, 0, 0, 0, 0 },
		{ 0, 1, 2, 2, 0, 0, 0, 0, 0, 0 },
		{ 1, 1, 2, 1, 0, 0, 2, 0, 0, 0 },
		{ 0, 0, 2, 1, 0, 0, 0, 0, 0, 2 },
	};

	struct FResponseDef
	{
		const TCHAR* Name;
		const TCHAR* Cause;
		const TCHAR* Effect;
		const TCHAR* Counter;
		const TCHAR* StartLog;   // %s = region
		const TCHAR* CounterLog; // %s = region
		const TCHAR* EndLog;     // %s = region
	};
	inline constexpr FResponseDef Responses[] = {
		{ TEXT("Night patrols"), TEXT("repeated animal disappearances"), TEXT("animal captures 60% slower, animal research -20%"),
			TEXT("Deploy decoy cows"),
			TEXT("Farmers in %s have organised night patrols. They carry flashlights, thermoses and a strong sense of grievance."),
			TEXT("Inflatable cows deployed in %s. The patrols are now guarding a field of balloons, very professionally."),
			TEXT("The %s night patrols have disbanded after a dispute about whose turn it was to bring snacks.") },
		{ TEXT("Investigators"), TEXT("frequent human abductions"), TEXT("human captures 30% slower, double suspicion from humans"),
			TEXT("Start a jurisdiction dispute"),
			TEXT("Investigators are now monitoring %s. They have a corkboard, red string and no budget."),
			TEXT("The investigation in %s has stalled. Both agencies insist the disappearance falls under the other agency's jurisdiction."),
			TEXT("The %s investigation has been closed and reclassified as 'people moving away.'") },
		{ TEXT("Radar coverage"), TEXT("heavy UFO traffic"), TEXT("flights 30% longer, +4 suspicion per launch"),
			TEXT("File a weather balloon report"),
			TEXT("Radar coverage over %s has been increased. Somewhere, an air traffic controller has started a spreadsheet."),
			TEXT("A weather balloon report for %s has been filed in triplicate. The radar team is now studying weather balloons."),
			TEXT("Radar coverage over %s has been cut to fund a new logo.") },
		{ TEXT("Support group"), TEXT("escapes and unwiped returns"), TEXT("suspicion fades at half speed, human research -10%"),
			TEXT("Fund an absurd cover story"),
			TEXT("Former subjects in %s have formed a support group. They meet on Tuesdays and compare probe stories."),
			TEXT("The %s support group has been told it was a shared dream sponsored by a mattress company. Attendance has dropped."),
			TEXT("The %s support group has dissolved into a book club. The book is about us. Nobody finishes it.") },
	};
	static_assert(UE_ARRAY_COUNT(Responses) == int32(EResponse::Count));

	inline constexpr float ResponseThreshold = 60.0f;
	inline constexpr float ResponseClear = 20.0f;
	inline constexpr float ResponsePressureDecay = 0.15f;    // per second
	inline constexpr float PatrolPressurePerAnimal = 7.0f;
	inline constexpr float InvestigatorPressurePerHuman = 9.0f;
	inline constexpr float RadarPressurePerLaunch = 5.0f;
	inline constexpr float SupportPressurePerEscape = 30.0f;
	inline constexpr float SupportPressurePerUnwipedHuman = 4.0f;
	inline constexpr double CounterBaseCost = 60.0;
	inline constexpr double CounterCostGrowth = 1.5;          // credibility drops with each reuse in a region

	struct FEarthConditionDef
	{
		const TCHAR* Name;
		const TCHAR* Preview;
		float PatrolPressure;
		float InvestigatorPressure;
		float RadarPressure;
		float SupportPressure;
		double Suspicion;
		float Threshold;
		double HumanResearch;
		double ContractReward;
	};
	inline constexpr FEarthConditionDef EarthConditions[] = {
		{ TEXT("Business as Usual"), TEXT("Earth is mostly busy with itself."), 1.0f, 1.0f, 1.0f, 1.0f, 1.0, ResponseThreshold, 1.0, 1.0 },
		{ TEXT("UFO Documentary Craze"),
			TEXT("Everyone owns a camera and a theory. Radar and investigators react 50% faster, but abducted humans are thrilled to be on camera: human research +25%."),
			1.0f, 1.5f, 1.5f, 1.0f, 1.0, ResponseThreshold, 1.25, 1.0 },
		{ TEXT("Surveillance Crackdown"),
			TEXT("A new agency with a new budget. All suspicion +30%, every response builds 20% faster and starts at 45 instead of 60, but the Academy pays 30% more for contracts."),
			1.2f, 1.2f, 1.2f, 1.2f, 1.3, 45.0f, 1.0, 1.3 },
		{ TEXT("Everything Is Advertising"),
			TEXT("The public assumes every sighting is a marketing stunt. Radar and investigators barely care (x0.3 and x0.5), but subjects keep asking for free samples: human research -15%, contracts -20%."),
			1.0f, 0.5f, 0.3f, 1.0f, 1.0, ResponseThreshold, 0.85, 0.8 },
	};
	static_assert(UE_ARRAY_COUNT(EarthConditions) == int32(EEarthCondition::Count));
	inline const FEarthConditionDef& EarthCondition(EEarthCondition Type) { return EarthConditions[int32(Type)]; }

	// Rivals (phase 1: the Efficiency Bureau)
	inline constexpr float RivalFirstSweepSeconds = 300.0f;
	inline constexpr float RivalSweepIntervalSeconds = 420.0f;  // from one sweep's start to the next
	inline constexpr float RivalSweepWarningSeconds = 90.0f;    // announced this long before it starts
	inline constexpr float RivalSweepSeconds = 150.0f;
	inline constexpr double RivalSweepCooling = 25.0;          // the Bureau's paperwork buries local rumours afterwards
	inline constexpr double TradeSellFraction = 0.7;           // of the routine research a specimen would earn
	inline constexpr float TradeSellIntervalSeconds = 5.0f;
	inline constexpr float TradeBuyIntervalSeconds = 45.0f;
	inline constexpr double TradeBuyCost[2] = { 10.0, 30.0 };   // squirrel, cow
	inline constexpr float RaceFirstOfferSeconds = 480.0f;
	inline constexpr float RaceOfferIntervalSeconds = 600.0f;
	inline constexpr float RivalRaceSecondsPerSpecimen = 45.0f; // the Bureau's pace (paused while offline)
	inline constexpr int32 RaceGrants = 2;

	// Phase 2: three rivals, pairings, collaboration and reputation
	struct FRivalDef
	{
		const TCHAR* Name;
		const TCHAR* Behavior;
		const TCHAR* Response;
		const TCHAR* JointName;
		ESpecimen JointType[2];
		int32 JointCount[2];
		const TCHAR* JointReport;
		ESpecimen RaceType;
		int32 RaceCount;
	};
	inline constexpr FRivalDef Rivals[] = {
		{ TEXT("The Efficiency Bureau"), TEXT("Collects common specimens in bulk; sweeps your busiest calm squirrel or cow ground."),
			TEXT("Hunt elsewhere or switch species while it sweeps; trade with it afterwards."),
			TEXT("Joint Census"), { ESpecimen::Squirrel, ESpecimen::Cow }, { 6, 4 },
			TEXT("Both departments counted the same squirrel twice. The figures have been averaged into a new squirrel."),
			ESpecimen::Cow, 6 },
		{ TEXT("The Celebrity Studies Division"), TEXT("Stages abductions of famous humans in your most valuable human region, drawing the public's attention."),
			TEXT("While the cameras point at its stunt, other regions are quiet: human suspicion there is halved."),
			TEXT("Fame Under Glass"), { ESpecimen::Human, ESpecimen::Human }, { 2, 1 },
			TEXT("Fame was found to be contagious but not fatal. The subject has asked for a signed copy of the report."),
			ESpecimen::Human, 4 },
		{ TEXT("The Experimental Sciences Office"), TEXT("Its containment breaches spill escaped specimens into a nearby region."),
			TEXT("Study its escapees (every trip there brings back one extra specimen), or research a Cleanup Crew."),
			TEXT("Controlled Spillage"), { ESpecimen::Elephant, ESpecimen::Cow }, { 2, 2 },
			TEXT("Both offices agree the spill was controlled. They disagree on by whom."),
			ESpecimen::Elephant, 3 },
	};
	static_assert(UE_ARRAY_COUNT(Rivals) == int32(ERival::Count));
	inline const FRivalDef& RivalDef(ERival Type) { return Rivals[int32(Type)]; }

	/** Which rivals appear on each assignment (a different pairing per thesis). */
	inline constexpr bool RivalPairs[int32(EThesis::Count)][int32(ERival::Count)] = {
		{ true, false, false },  // Preliminary Observations: the Bureau alone
		{ false, true, true },   // Nobody Will Believe You (covert): celebrity stunts give cover
		{ true, false, true },   // Corporate Wants Results (bulk): Bureau trades, breach escapees
		{ true, true, false },   // A Controlled Experiment (precision): celebrity races on humans
		{ true, false, true },   // The Deep End (Oceanic): the Bureau trawls the reef; the Experimental Sciences Office leaks into the trench
		{ false, true, true },   // Cold Storage (Antarctic): documentary crews and a very cold breach; the Bureau will not ship this far
	};

	inline constexpr float CelebrityFirstSeconds = 360.0f;
	inline constexpr float CelebrityIntervalSeconds = 480.0f;
	inline constexpr float BreachFirstSeconds = 420.0f;
	inline constexpr float BreachIntervalSeconds = 540.0f;
	inline constexpr float RivalEventWarningSeconds = 90.0f;
	inline constexpr float RivalEventSeconds = 150.0f;
	inline constexpr float CelebrityInvestigatorPressurePerSecond = 1.0f;
	inline constexpr double CelebritySuspicionPerSecond = 0.15;
	inline constexpr double CelebrityCoverMultiplier = 0.5;   // human suspicion elsewhere while the cameras are busy
	inline constexpr float BreachSupportPressurePerSecond = 1.0f;
	inline constexpr double BreachSuspicionPerSecond = 0.15;
	inline constexpr float ContributeIntervalSeconds = 5.0f;
	inline constexpr double JointResearch = 400.0;
	inline constexpr int32 JointGrants = 1;
	// Reputation: collaboration improves trades, competition unlocks harder (better-paid) races.
	inline constexpr double CollaboratorSellFraction = 0.85;
	inline constexpr double TrustedBuyMultiplier = 0.7;

	// The three approaches: each has a thesis, a rival interaction and a permanent discovery that suit it.
	enum class EApproach : uint8 { Bulk, Precision, Covert, Count };
	struct FApproachDef
	{
		const TCHAR* Name;
		const TCHAR* Pitch;
		const TCHAR* Strengths;
		const TCHAR* Weaknesses;
		EThesis Thesis;
		EDepartment Department;
		ERival Rival;
		const TCHAR* RivalInteraction;
		EDeptProject Permanent;
		const TCHAR* PermanentWhy;
	};
	inline constexpr FApproachDef Approaches[] = {
		{ TEXT("Bulk Collection"), TEXT("Cargo ships, animals, big containment, batch experiments."),
			TEXT("Lots of cheap specimens; easy to leave running; research piles up."),
			TEXT("Each animal pays little; farmers' night patrols; containment fills; the Bureau sweeps the same squirrels and cows."),
			EThesis::CorporateWantsResults, EDepartment::Biological, ERival::EfficiencyBureau,
			TEXT("Sell surplus to the Efficiency Bureau and run its Joint Census; study the Experimental Sciences Office's escapees."),
			EDeptProject::BatchExperiments, TEXT("Batch Experiments doubles every animal study from the first minute.") },
		{ TEXT("Precision Research"), TEXT("Scanners, specialist crews, rare human traits, high-value contracts."),
			TEXT("Highest research per specimen; contracts and races on humans pay big; few ships needed."),
			TEXT("Slow captures (scanning); lab time is the limit; investigators gather where you work."),
			EThesis::ControlledExperiment, EDepartment::Human, ERival::CelebrityStudies,
			TEXT("Race the Celebrity Studies Division on humans and join its Fame Under Glass study."),
			EDeptProject::AdvancedScanners, TEXT("Advanced Scanners: no scan delay and +25% human research from the start.") },
		{ TEXT("Covert Operations"), TEXT("Stealth ships, memory manipulation, careful regional rotation."),
			TEXT("Rarely provokes Earth; safe under a crackdown; keeps every region open."),
			TEXT("Stealth eats cargo slots, so fewer specimens per trip; rotating takes attention or good automation."),
			EThesis::NobodyWillBelieveYou, EDepartment::Incompetence, ERival::CelebrityStudies,
			TEXT("Hunt while the Celebrity Studies Division has every camera pointed at its stunt."),
			EDeptProject::AutomatedCoverStories, TEXT("Automated Cover Stories: suspicion fades twice as fast, lockdowns lift early.") },
	};
	static_assert(UE_ARRAY_COUNT(Approaches) == int32(EApproach::Count));
	inline const FApproachDef& Approach(EApproach Type) { return Approaches[int32(Type)]; }

	// ---------------------------------------------------------------------------------------------
	// The Specimens Remember: recurring subjects, their relationships and persistent case files.

	/** A personal research step: what the lab does with a recurring subject, in order. */
	enum class ECaseRequirement : uint8
	{
		Any,       // any encounter
		Returning, // the subject has been taken before (second encounter or later)
		SecondRun, // the subject has been met on two different assignments (prestige runs)
	};

	struct FCaseStepDef
	{
		const TCHAR* Name;
		ECaseRequirement Requirement;
		float Seconds;
		double Energy;
		double Research;   // 0 = the final step: it makes the case-file discovery instead
		const TCHAR* Report;
	};

	inline constexpr int32 NumCaseSubjects = 5;
	inline constexpr int32 CaseSteps = 3;

	struct FCaseSubjectDef
	{
		const TCHAR* Name;
		const TCHAR* Short;      // used in log lines
		const TCHAR* Title;
		EOccupation Occupation;  // counts for contracts and professions
		const TCHAR* Lead;       // what you know before you meet them
		const TCHAR* Profile;    // their encyclopedia entry
		const TCHAR* ReturnLine; // when they are taken again
		FCaseStepDef Steps[CaseSteps];
		const TCHAR* DiscoveryName;
		const TCHAR* DiscoveryEffect;
	};

	inline constexpr int32 CaseMarsh = 0;
	inline constexpr int32 CasePike = 1;
	inline constexpr int32 CaseDelgado = 2;
	inline constexpr int32 CasePell = 3;
	inline constexpr int32 CaseFenn = 4;

	inline constexpr FCaseSubjectDef CaseSubjects[NumCaseSubjects] = {
		{ TEXT("Dr. Edith Marsh"), TEXT("Dr. Marsh"), TEXT("astronomer"), EOccupation::Astronomer,
			TEXT("an astronomer who keeps filing UFO reports nobody reads"),
			TEXT("Has reported us 41 times, correctly every time. Nobody believes her, which she has started to take personally."),
			TEXT("Dr. Marsh recognised the tractor beam and asked whether the pilot had been promoted yet."),
			{
				{ TEXT("Intake Interview"), ECaseRequirement::Any, 30.0f, 20.0, 150.0,
					TEXT("Asked Dr. Marsh what she saw. She drew our ship accurately, then corrected the pilot's posture.") },
				{ TEXT("Calibration Study"), ECaseRequirement::Returning, 40.0f, 30.0, 400.0,
					TEXT("Dr. Marsh recognised the equipment and recalibrated it. Readings improved 40%. She has asked to be credited.") },
				{ TEXT("Longitudinal Study"), ECaseRequirement::SecondRun, 60.0f, 40.0, 0.0,
					TEXT("Two postings later, Dr. Marsh's notes match ours exactly. The Academy has quietly adopted her methodology.") },
			},
			TEXT("Ground Truth"), TEXT("Discovery projects run 30% faster at every base, forever.") },
		{ TEXT("Prof. Desmond Pike"), TEXT("Prof. Pike"), TEXT("astrophysicist"), EOccupation::Astronomer,
			TEXT("a professor who insists, loudly, that there is no life out there"),
			TEXT("Has spent thirty years proving Dr. Marsh wrong. Being abducted has not changed his position."),
			TEXT("Prof. Pike recognised the containment room and filed a complaint about the room, not the abduction."),
			{
				{ TEXT("Skeptic Baseline"), ECaseRequirement::Any, 30.0f, 20.0, 150.0,
					TEXT("Showed Prof. Pike a live alien. He called it a weather balloon. Data excellent.") },
				{ TEXT("Denial Under Load"), ECaseRequirement::Returning, 40.0f, 30.0, 400.0,
					TEXT("Prof. Pike has now been abducted twice and denied it twice. Staff are studying how.") },
				{ TEXT("Tenure Review"), ECaseRequirement::SecondRun, 60.0f, 40.0, 0.0,
					TEXT("Across two postings Prof. Pike has rejected every one of our findings. The Academy considers this peer review, and funds it.") },
			},
			TEXT("Hostile Peer Review"), TEXT("Every Academy review awards +1 Research Grant, forever: Pike writes the rebuttals now.") },
		{ TEXT("Lou Delgado"), TEXT("Delgado"), TEXT("reporter, the Gazette"), EOccupation::Journalist,
			TEXT("a reporter chasing a missing-persons story that is mostly us"),
			TEXT("Writes the Gazette's 'Unexplained' column. Lately it has been very explained."),
			TEXT("Lou Delgado recognised the pilot and asked for a comment. The pilot said 'no comment', confirming everything."),
			{
				{ TEXT("Off-the-Record Briefing"), ECaseRequirement::Any, 30.0f, 20.0, 150.0,
					TEXT("Gave Lou Delgado an exclusive. The editor rejected it as 'too plausible'.") },
				{ TEXT("Source Protection"), ECaseRequirement::Returning, 40.0f, 30.0, 400.0,
					TEXT("Delgado refused to name sources, including us. Staff are oddly touched.") },
				{ TEXT("The Retraction"), ECaseRequirement::SecondRun, 60.0f, 40.0, 0.0,
					TEXT("Two postings, one retraction: Delgado has publicly apologised for believing in us.") },
			},
			TEXT("Off the Record"), TEXT("Investigators build half as fast in every region, forever: Delgado writes the corrections now.") },
		{ TEXT("Martin Pell"), TEXT("Pell"), TEXT("accountant, the Gazette"), EOccupation::Accountant,
			TEXT("a newspaper's accountant who has noticed some odd expense reports"),
			TEXT("Audits the Gazette, and now us. Found three discrepancies in our containment log. All three were him."),
			TEXT("Subject requested their usual containment room. Staff have interpreted this as informed consent."),
			{
				{ TEXT("Expense Audit"), ECaseRequirement::Any, 30.0f, 20.0, 150.0,
					TEXT("Martin Pell audited the lab. The lab owes itself 40 research, which has been paid.") },
				{ TEXT("Reconciliation"), ECaseRequirement::Returning, 40.0f, 30.0, 400.0,
					TEXT("Pell reconciled our books with his. We now owe the Gazette a subscription.") },
				{ TEXT("Creative Accounting"), ECaseRequirement::SecondRun, 60.0f, 40.0, 0.0,
					TEXT("Pell has reclassified two postings of abductions as 'consulting'. The Academy now pays us like consultants.") },
			},
			TEXT("Creative Accounting"), TEXT("Contracts pay 25% more at every base, forever.") },
		{ TEXT("Gloria Fenn"), TEXT("Fenn"), TEXT("therapist"), EOccupation::Therapist,
			TEXT("a therapist whose clients keep describing the same dream"),
			TEXT("Runs a support group for people who have met us. Has started taking notes on the staff."),
			TEXT("Gloria Fenn recognised the containment room and asked whether we had considered talking about it."),
			{
				{ TEXT("Intake Session"), ECaseRequirement::Any, 30.0f, 20.0, 150.0,
					TEXT("Gloria Fenn asked the scientist how the probe made them feel. The session ran over. She billed us.") },
				{ TEXT("Group Session"), ECaseRequirement::Returning, 40.0f, 30.0, 400.0,
					TEXT("Fenn ran a group session for the containment block. Escape attempts are now 'processed'.") },
				{ TEXT("Closure"), ECaseRequirement::SecondRun, 60.0f, 40.0, 0.0,
					TEXT("After two postings, Fenn has concluded the abductions are 'a healthy boundary'. Her groups now recommend acceptance.") },
			},
			TEXT("Group Therapy"), TEXT("Support groups build half as fast in every region, forever: Fenn runs them now.") },
	};

	/** How two recurring subjects affect each other. */
	enum class ECaseRelation : uint8
	{
		Colleagues, // taking one sends the other digging; held together they cause trouble
		Rivals,     // held together they unlock Peer Review (and argue if left waiting)
		Kindred,    // held together both produce more research
		Count,
	};
	inline constexpr const TCHAR* CaseRelationNames[] = { TEXT("Colleagues"), TEXT("Rivals"), TEXT("Kindred") };

	struct FCaseRelationDef
	{
		int32 A;
		int32 B;
		ECaseRelation Type;
		const TCHAR* Text;
	};
	inline constexpr FCaseRelationDef CaseRelations[] = {
		{ CaseDelgado, CasePell, ECaseRelation::Colleagues,
			TEXT("Gazette colleagues. Taking one when the other has a case file sends Delgado digging (investigators +25, suspicion +8 there). Held together, they start a containment newsletter.") },
		{ CaseMarsh, CasePike, ECaseRelation::Rivals,
			TEXT("Rival academics. Held together, they unlock the one-off Peer Review experiment. Left waiting together, they start an academic dispute.") },
		{ CaseFenn, CaseMarsh, ECaseRelation::Kindred,
			TEXT("Fenn has been Dr. Marsh's therapist for years. Held together, both produce 30% more research.") },
		{ CaseFenn, CasePell, ECaseRelation::Kindred,
			TEXT("Pell does Fenn's taxes. Held together, both produce 30% more research.") },
	};
	inline constexpr int32 NumCaseRelations = UE_ARRAY_COUNT(CaseRelations);

	inline constexpr double CaseAppearChance = 0.04;            // per unscanned human taken: a recurring subject instead
	inline constexpr double CaseResearchPerEncounter = 0.25;    // they recognise the equipment: +25% research per earlier encounter...
	inline constexpr int32 CaseMaxEncounterEffect = 4;          // ...counting up to four
	inline constexpr double CaseEscapePerEncounter = 0.5;       // harder to fool: escapes +50% per earlier encounter
	inline constexpr float CaseCapturePerEncounter = 0.25f;     // and scanning them out takes +25% per earlier encounter
	inline constexpr double CaseWipeSuspicionPerEncounter = 2.0; // the memory wipe works less each time
	inline constexpr float ColleagueInvestigatorPressure = 25.0f;
	inline constexpr double ColleagueSuspicion = 8.0;
	inline constexpr double KindredResearchMultiplier = 1.3;
	inline constexpr float CaseTroubleSeconds = 20.0f;          // troublesome pairs held together this long cause an incident
	inline constexpr float CaseTroubleRepeatSeconds = 120.0f;
	inline constexpr float NewsletterSeconds = 40.0f;
	inline constexpr float NewsletterLabSpeed = 0.85f;
	inline constexpr double NewsletterSuspicion = 6.0;
	inline constexpr float DisputeSeconds = 30.0f;
	inline constexpr float DisputeLabSpeed = 0.75f;
	inline constexpr uint8 PeerReviewStep = CaseSteps;          // CaseStep value of the rivals' joint experiment
	inline constexpr float PeerReviewSeconds = 60.0f;
	inline constexpr double PeerReviewEnergy = 40.0;
	inline constexpr double PeerReviewResearch = 600.0;
	inline constexpr float CaseRepeatStepSpeedup = 0.5f;        // steps done on an earlier assignment take half the time
	inline constexpr int32 MaxCaseHistory = 8;
	inline constexpr float CaseDiscoverySpeed = 0.7f;           // Ground Truth
	inline constexpr double CaseContractMultiplier = 1.25;      // Creative Accounting
	inline constexpr float CasePressureMultiplier = 0.5f;       // Off the Record, Group Therapy
	inline constexpr int32 CaseRandomSeed = 1307;

	inline constexpr int32 PresetSlots = 3;
	inline constexpr float BehaviourMemoSeconds = 300.0f;

	inline constexpr double OtherDepartmentCostMultiplier = 2.5;
	inline constexpr float SignatureSeconds = 120.0f;
	inline constexpr double SignatureEnergy = 60.0;
	inline constexpr double SignatureResearch = 1500.0;
	inline constexpr double SignatureResearchBonus = 1.25;
	inline constexpr double IncidentLessonResearch = 60.0;
	inline constexpr double DepartmentContractMultiplier = 1.5;

	// Experiment kinds
	inline constexpr float ExtendedDurationMultiplier = 2.5f;
	inline constexpr double ExtendedEnergyMultiplier = 2.5;
	inline constexpr double ExtendedResearchMultiplier = 3.5;
	inline constexpr double SarcasmHumanResearchMultiplier = 1.5;
	inline constexpr float TherapistLabSpeed = 1.2f;
	inline constexpr float MiddleManagementLabSpeed = 1.2f;
	inline constexpr float MeetingIntervalSeconds = 120.0f;
	inline constexpr float MeetingSeconds = 10.0f;

	// Energy
	inline constexpr double StartEnergy = 20.0;
	inline constexpr double EnergyCap = 40.0;
	inline constexpr double EnergyPerSecond = 1.0;
	inline constexpr double PowerEnergyPerLevel = 1.0;
	inline constexpr double PowerCapPerLevel = 30.0;

	// Ship systems. Each ship has a limited number of upgrade slots, so no ship maxes everything.
	inline constexpr int32 ShipSlots = 6;
	inline constexpr FUpgradeDef ShipSystems[] = {
		{ TEXT("Thrusters"),     15.0, 1.8, 4, 1 },
		{ TEXT("Tractor Beam"),  40.0, 2.2, 4, 1 },
		{ TEXT("Cargo Bay"),     25.0, 2.0, 4, 1 },
		{ TEXT("Stealth Panels"), 35.0, 2.0, 3, 2 },
		{ TEXT("Trait Scanner"), 60.0, 2.5, 2, 1 },
	};
	static_assert(UE_ARRAY_COUNT(ShipSystems) == int32(EShipSystem::Count));

	inline constexpr double RefitRefundFraction = 0.5;
	inline constexpr float ThrustersFlightMultiplier = 0.85f;
	inline constexpr int32 TractorStrength[] = { 1, 3, 5, 8, 12 };
	inline constexpr int32 TractorLevelWithoutDiscovery = 1;      // Tractor III+ needs Bovine Load Tolerance
	inline constexpr double TractorLaunchEnergyPerLevel = 0.25;   // tradeoff: +25% launch energy per level
	inline constexpr int32 StartCargo = 1;
	inline constexpr float CargoCapturePerLevel = 0.2f;           // tradeoff: +20% capture (loading) time per level
	inline constexpr double StealthSuspicionMultiplier = 0.6;     // tradeoff: 2 slots per level
	inline constexpr float ScannerSeconds[] = { 0.0f, 4.0f, 2.0f }; // tradeoff: scan time when targeting a trait

	// Base
	inline constexpr int32 StartContainmentUnits = 4;
	inline constexpr int32 ContainmentUnitsPerLevel = 3;
	inline constexpr int32 ContainmentLevelWithoutDiscovery = 2;  // Habitat III+ needs Unscheduled Departures
	inline constexpr FUpgradeDef BaseUpgrades[] = {
		{ TEXT("Containment Habitat"), 20.0, 1.7, 8, 0 },
		{ TEXT("Experiment Chamber"),  60.0, 4.0, 2, 0 },
		{ TEXT("Power Generation"),    20.0, 1.7, 10, 0 },
		{ TEXT("Hangar Bay"),          120.0, 3.75, 2, 0 },
		{ TEXT("Radar Camouflage"),    300.0, 1.0, 1, 0 },
		{ TEXT("Cleanup Crew"),        250.0, 1.0, 1, 0 },
		{ TEXT("Tidal Turbines"),      35.0, 1.8, 4, 0 },
		{ TEXT("Deep-Water Tanks"),    50.0, 1.9, 4, 0 },
		{ TEXT("Sonar Array"),         260.0, 1.0, 1, 0 },
		{ TEXT("Ink Screen Launchers"), 220.0, 1.0, 1, 0 },
		{ TEXT("Geothermal Taps"),     45.0, 1.9, 3, 0 },
		{ TEXT("Cryo Vault"),          70.0, 2.0, 3, 0 },
		{ TEXT("Insulated Labs"),      200.0, 1.0, 1, 0 },
		{ TEXT("Cryo Pods"),           180.0, 1.0, 1, 0 },
		{ TEXT("Calibration Desk"),    150.0, 1.0, 1, 0 },
	};
	static_assert(UE_ARRAY_COUNT(BaseUpgrades) == int32(EBaseUpgrade::Count));

	// Suspicion (per region, 0..100)
	inline constexpr double SuspicionDecayPerSecond = 0.5;
	inline constexpr double AlertSuspicion = 50.0;
	inline constexpr float AlertCaptureMultiplier = 1.5f;
	inline constexpr double LockdownSuspicion = 100.0;
	inline constexpr double LockdownReleaseSuspicion = 40.0;
	inline constexpr double EscapeSuspicion = 10.0;

	// Contracts
	inline constexpr int32 ContractOffers = 3;

	// Incidents: occasional, self-recovering, never more than one at a time.
	inline constexpr float FirstIncidentSeconds = 180.0f;
	inline constexpr float IncidentCooldownSeconds = 120.0f;
	inline constexpr double EquipmentFailurePerSecond = 1.0 / 240.0;
	inline constexpr double IncidentResolveCost = 25.0;

	// Automation (auto-launch, contract mode) unlocks after this many experiments in total.
	inline constexpr int32 AutomationUnlockExperiments = 3;

	// Crew (Alien Resources)
	inline constexpr int32 MaxCrewPerStation = 3;
	inline constexpr float SpecialistBonus = 0.5f;          // skill added by a specialist at a matching station
	inline constexpr float TrainingBonusPerLevel = 0.25f;   // per training level, matching station only
	inline constexpr int32 MaxTraining = 3;
	inline constexpr double RecruitBaseCost = 40.0;
	inline constexpr double RecruitCostGrowth = 1.5;
	inline constexpr double TrainBaseCost = 30.0;
	inline constexpr double TrainCostGrowth = 2.0;
	inline constexpr float TrainSeconds = 30.0f;
	// How station bonus (skill beyond the first basic crew member) turns into effects.
	inline constexpr float UfoSpeedPerBonus = 0.15f;        // flight and capture faster
	inline constexpr float LabSpeedPerBonus = 0.2f;
	inline constexpr float ContainmentUnitsPerBonus = 2.0f; // floor(bonus x 2) extra units
	inline constexpr float PowerOutputPerBonus = 0.2f;
	inline constexpr float UnstaffedPowerMultiplier = 0.5f;
	inline constexpr double UnstaffedEscapeMultiplier = 4.0;
	inline constexpr double UnwipedReturnSuspicion = 3.0;   // memory-wipe unstaffed: returns add suspicion
	inline constexpr double WipeSuspicionPerBonus = 3.0;    // extra memory-wipe staff remove suspicion per return

	// Bottlenecks: recent values decay with this time constant (seconds) so the headline does not flicker.
	inline constexpr float BottleneckWindowSeconds = 60.0f;
	// Staffing incidents
	inline constexpr float StaffMeetingSeconds = 25.0f;
	inline constexpr float AdoptedSquirrelSeconds = 45.0f;

	// Probe Calibration (AlienIdleCalibration.cpp): now and then a running experiment offers a five-second
	// timing challenge. Any attempt skips part of the remaining time; ignoring it costs nothing. One window
	// at a time across all chambers, with a shared cooldown, so cycling labs gives no constant bonus.
	inline constexpr float CalibrationFirstSeconds = 45.0f;    // first window of a run
	inline constexpr float CalibrationCooldownSeconds = 60.0f; // after any calibration, manual or automatic
	inline constexpr float CalibrationRetrySeconds = 30.0f;    // after a window is ignored and lapses
	inline constexpr float CalibrationOfferSeconds = 30.0f;    // how long a window stays open
	inline constexpr float CalibrationMinRemaining = 12.0f;    // experiment seconds left before a window is worth offering
	inline constexpr float CalibrationChallengeSeconds = 5.0f; // the timing challenge; running out counts as a missed attempt
	inline constexpr float CalibrationSweepSeconds = 0.9f;     // one pass of the cursor across the gauge
	inline constexpr float CalibrationBandHalfWidth = 0.09f;   // gauge is 0..1
	inline constexpr float CalibrationCoreHalfWidth = 0.03f;
	inline constexpr float CalibrationBandMin = 0.2f;          // band centres are placed in [Min, 1 - Min]
	/** Fraction of the remaining experiment time skipped, by ECalibrationGrade. */
	inline constexpr float CalibrationFraction[] = { 0.08f, 0.15f, 0.25f, 0.15f, 0.06f };
	static_assert(UE_ARRAY_COUNT(CalibrationFraction) == int32(ECalibrationGrade::Count));
	// Calibration Desk: a technician takes windows the player leaves alone for a while.
	inline constexpr float CalibrationTechnicianDelay = 15.0f;     // seconds the window waits for the player first
	inline constexpr float CalibrationTechnicianPerBonus = 0.02f;  // extra fraction per point of station bonus
	inline constexpr float CalibrationTechnicianMax = 0.12f;       // never as good as a careful manual stop (0.15+)
	inline constexpr int32 CalibrationDeskManual = 3;              // desk unlocks after this many manual calibrations...
	inline constexpr int32 CalibrationDeskExperiments = 40;        // ...or this many experiments in the run
	inline constexpr int32 CalibrationRandomSeed = 4242;           // band placement only; never touches the main stream

	// Saving: offline catch-up is capped (GDD default: 8 hours).
	inline constexpr double OfflineCapSeconds = 8.0 * 60.0 * 60.0;
	inline constexpr int32 SaveVersion = 11;

	inline constexpr int32 RandomSeed = 7;

	// Simulation step; large deltas (hitches, later offline catch-up) are split into steps this size.
	inline constexpr float MaxStepSeconds = 0.1f;

	inline constexpr int32 MaxLogEntries = 80;
}
