#pragma once

#include "CoreMinimal.h"
#include "Math/RandomStream.h"
#include "AlienIdleTuning.h"

enum class EUfoState : uint8
{
	Docked,
	FlyingOut,
	Capturing,
	Returning,
	WaitingToUnload,
};

enum class EIncident : uint8
{
	None,
	Brownout,             // energy regeneration drops
	TractorJam,           // one docked UFO cannot launch
	CoffeeFailure,        // labs slow down
	InventoryReorganized, // escaped accountant: containment +3 units (a good incident)
	CeaseAndDesist,       // escaped lawyer: launches cost more energy
	StaffMeeting,         // a crew member calls a mandatory meeting: labs pause
	AdoptedSquirrel,      // a crew member adopts a squirrel and goes off duty
	WhaleSong,            // Oceanic: a whale sings against the hull: pressure spike, labs slower
	HeatingCut,           // Antarctic: the heating budget has been reduced: heat x0.5
	ContainmentNewsletter, // case files: colleagues held together publish a newsletter: labs slower
	AcademicDispute,       // case files: rivals left waiting together argue: labs slower
};

/** What is limiting progress right now. */
enum class EBottleneck : uint8
{
	None,
	Crew,        // posts without staff
	Energy,      // labs or launches waiting for energy
	Collection,  // labs idle with nothing to study: need more cargo, ships or pilots
	Containment, // ships waiting or holding because containment is full
	LabCapacity, // specimens queue while every chamber is busy
	Count,
};

/** "While You Were Out": what happened between two moments. */
struct FAwayReport
{
	double Seconds = 0.0;
	int32 Captures = 0;
	int32 Experiments = 0;
	double ResearchEarned = 0.0;
	TArray<FString> Discoveries;
	int32 Contracts = 0;
	int32 Escapes = 0;
	int32 Incidents = 0;
	float LostSeconds[int32(EBottleneck::Count)] = {};
	FString Recommendation;
};

enum class EStation : uint8
{
	None,       // unassigned
	Ufo,        // Index = UFO
	Chamber,    // Index = lab chamber
	Containment,
	Power,
	MemoryWipe,
	Pressure,   // Oceanic: Pressure Control
	Boiler,     // Antarctic: the Boiler Room
	Calibration, // the Calibration Desk (optional post, once built): takes probe calibrations the player leaves
};

/** Antarctic: who gets the heat first. */
enum class EHeatPriority : uint8
{
	Even,
	LabsFirst,
	HangarFirst,
	Count,
};

struct FAssignment
{
	EStation Station = EStation::None;
	int32 Index = 0;

	bool operator==(const FAssignment& Other) const { return Station == Other.Station && (Index == Other.Index || !(Station == EStation::Ufo || Station == EStation::Chamber)); }
};

struct FCrewMember
{
	FString Name;
	ESpecialty Specialty = ESpecialty::Pilot;
	int32 Training = 0;
	float TrainingRemaining = 0.0f; // off duty while > 0
	FAssignment Assignment;
};

struct FSpecimen
{
	ESpecimen Type = ESpecimen::Squirrel;
	EOccupation Occupation = EOccupation::None;
	/** Region it was taken from (escapes and returns cause trouble there). */
	uint8 Region = 0;
	/** Seconds kept cold in containment (Antarctic, or Cryo Pods): preserved samples pay more. */
	float Held = 0.0f;
	/** A recurring case-file subject (AlienIdleTuning::CaseSubjects index), or -1 for an ordinary specimen. */
	int8 Case = -1;
};

struct FUfo
{
	FString Name;
	EUfoState State = EUfoState::Docked;
	float StateTime = 0.0f;
	float StateDuration = 0.0f;
	ESpecimen Destination = ESpecimen::Squirrel;
	int32 Region = 0;      // hunting ground for the next launch
	int32 TripRegion = 0;  // hunting ground of the current trip
	/** On the current trip (fixed at launch). None = whatever turns up. */
	ESpecimen TripTarget = ESpecimen::Squirrel;
	EOccupation TripOccupation = EOccupation::None;
	/** Player settings. */
	bool bAutoLaunch = false;
	bool bFollowContract = false;
	/** Scan for a recurring subject (case index) on the next human trips; -1 = none. Cleared once taken. */
	int8 CaseTarget = -1;
	/** The subject being scanned for on the current trip, or -1. */
	int8 TripCase = -1;
	TArray<FSpecimen> Cargo;
	int32 Levels[int32(EShipSystem::Count)] = {};
};

/** What the lab does with a specimen type. Player-set per type: the lab's priorities. */
struct FExperimentChoice
{
	EExperimentKind Kind = EExperimentKind::Routine;
	EDiscovery Discovery = EDiscovery::None;
	EDepartment Department = EDepartment::None; // signature experiments only
	uint8 CaseStep = 0; // case studies only: the step, or AlienIdleTuning::PeerReviewStep
};

struct FLabChamber
{
	bool bRunning = false;
	float Time = 0.0f;
	float Duration = 0.0f;
	FSpecimen Specimen;
	FExperimentChoice Experiment;
	/** Batch Experiments: two animals of one species studied together. */
	int32 Batch = 1;
	/** Peer Review: the second rival academic, studied alongside. Case -1 when unused. */
	FSpecimen Partner;
};

struct FRegionState
{
	double Suspicion = 0.0;
	bool bLockedDown = false;
	/** Earth's responses: pressure builds from what we do here and fades over time. */
	float Pressure[int32(EResponse::Count)] = {};
	bool Active[int32(EResponse::Count)] = {};
	int32 CounterUses[int32(EResponse::Count)] = {};
};

struct FContractNeed
{
	ESpecimen Type = ESpecimen::Squirrel;
	EOccupation Occupation = EOccupation::None; // None = any
	int32 Count = 1;
	bool bDistinctOccupations = false;
	int32 Progress = 0;
	TArray<EOccupation> Seen;

	bool IsMet() const { return Progress >= Count; }
	bool Accepts(const FSpecimen& Specimen) const;
};

struct FContract
{
	FString Title;
	FString Flavor;
	double Reward = 0.0;
	TArray<FContractNeed> Needs;
	/** Department contracts reward that department's playstyle. */
	EDepartment Department = EDepartment::None;

	bool IsComplete() const;
	FString GetProgressText() const;
};

struct FActiveIncident
{
	EIncident Type = EIncident::None;
	float Remaining = 0.0f;
	int32 Ufo = INDEX_NONE;
	int32 Crew = INDEX_NONE;
};

struct FEncyclopediaEntry
{
	FString Title;
	FString Text;
	double Time = 0.0;
};

struct FAlienIdleLogEntry
{
	double Time = 0.0;
	FString Text;
};

/** A saved fleet and staffing setup. */
struct FUfoPreset
{
	ESpecimen Destination = ESpecimen::Squirrel;
	int32 Region = 0;
	bool bAutoLaunch = false;
	bool bFollowContract = false;
};

struct FBuildPreset
{
	bool bSaved = false;
	FString Name;
	TArray<FUfoPreset> Ufos;
	TArray<FAssignment> Crew; // by crew index
	FExperimentChoice Choices[int32(ESpecimen::Count)];
	bool Sell[3] = {};
	bool Buy[2] = {};
	bool bReroute = true;
	bool bPauseWhenFull = true;
	bool bContribute = false;
	bool bAutoContracts = false;
};

/** A recurring subject's permanent record: kept through promotions. */
struct FCaseFile
{
	int32 Encounters = 0;
	/** Different assignments (runs) they have been met on. */
	int32 RunsMet = 0;
	int32 LastRun = 0;
	/** Most personal research steps ever finished in one run: repeating them later takes half the time. */
	int32 BestSteps = 0;
	bool bDiscovered = false;
	/** One line per encounter, newest last. */
	TArray<FString> History;
};

/** Everything that survives a promotion. */
struct FCareer
{
	int32 Run = 1;
	int32 Promotions = 0;
	int32 Grants = 0;
	int32 TotalGrants = 0;
	bool Upgrades[int32(EGrantUpgrade::Count)] = {};
	ESpecialty StartingSpecialty = ESpecialty::Technician;
	TArray<EDeptProject> PermanentProjects;
	bool PermanentSignature[int32(EDepartment::Count)] = {};
	bool LocationUnlocked[int32(ELocation::Count)] = { true, false, false, false };
	/** Location benefits kept forever (Oceanic: Submersible Hulls). */
	bool LocationPerk[int32(ELocation::Count)] = {};
	bool Published[int32(EDiscovery::Count)] = {};
	bool PublishedSignature[int32(EDepartment::Count)] = {};
	// Professional reputation with rival departments
	int32 RaceWins = 0;
	int32 RaceLosses = 0;
	int32 Trades = 0;
	int32 Collaborations = 0;
	/** Saved fleet and staffing presets (kept across runs; applied as far as the current base allows). */
	FBuildPreset Presets[AlienIdleTuning::PresetSlots];
	/** Recurring subjects: encounter history and case-file discoveries. */
	FCaseFile Cases[AlienIdleTuning::NumCaseSubjects];
	bool bPeerReviewed = false;
};

/** A scheduled rival event: announced, then active for a while, then over. */
struct FRivalEvent
{
	float NextAt = 0.0f;
	bool bAnnounced = false;
	bool bActive = false;
	float EndsAt = 0.0f;
	int32 Region = 0;
	int32 Count = 0;
};

/** The Efficiency Bureau: a rival that sweeps common specimens from your busiest hunting ground. */
struct FRivalState
{
	float NextSweep = AlienIdleTuning::RivalFirstSweepSeconds; // run time the next sweep starts
	bool bAnnounced = false;
	bool bSweeping = false;
	float SweepEnds = 0.0f;
	int32 SweepRegion = 0;
	ESpecimen SweepType = ESpecimen::Squirrel;
	int32 Sweeps = 0;
	// Standing trade orders (never expire)
	bool Sell[3] = {};     // squirrels, cows, elephants: sell surplus when containment is crowded
	bool Buy[2] = {};      // squirrels, cows: delivered on a timer while there is room
	float SellTimer = 0.0f;
	float BuyTimer = 0.0f;
	int32 TradesThisRun = 0;
	double TradeIncome = 0.0;
	// Optional race contract
	bool bRaceOffered = false;
	bool bRaceActive = false;
	float NextRaceOffer = AlienIdleTuning::RaceFirstOfferSeconds;
	ESpecimen RaceType = ESpecimen::Squirrel;
	int32 RaceCount = 0;
	int32 RaceProgress = 0;
	float RivalProgress = 0.0f;
	double RaceReward = 0.0;
	// Where we have been hunting recently (the Bureau targets the busiest ground)
	float Usage[AlienIdleTuning::NumRegions][int32(ESpecimen::Count)] = {};
	// Phase 2
	bool Active[int32(ERival::Count)] = { true, false, false };
	FRivalEvent Celebrity{ AlienIdleTuning::CelebrityFirstSeconds };
	FRivalEvent Breach{ AlienIdleTuning::BreachFirstSeconds };
	ERival RaceOpponent = ERival::EfficiencyBureau;
	bool bContribute = false;
	float ContributeTimer = 0.0f;
	int32 JointProgress[2] = {};
	bool JointDone[int32(ERival::Count)] = {};
};

struct FThesisObjective
{
	FString Text;
	bool bDone = false;
};

/** A benefit the player may carry into the next run. */
struct FPermanentOption
{
	bool bSignature = false;
	EDeptProject Project = EDeptProject::Count;
	EDepartment Department = EDepartment::None;
	/** A location benefit instead of a department one (ELocation::Count = none). */
	ELocation Perk = ELocation::Count;
	FString Name;
	FString Effect;
};

/** The Academy review shown before the player commits to a promotion. */
struct FPromotionReview
{
	TArray<FString> Stats;
	TArray<FString> GrantLines;
	int32 Grants = 0;
	FString Review;
	FString RankBefore;
	FString RankAfter;
	TArray<FString> Resets;
	TArray<FString> Keeps;
	TArray<FPermanentOption> Options;
};

/** The player's choices for the next run. */
struct FNextAssignment
{
	EThesis Thesis = EThesis::NobodyWillBelieveYou;
	ELocation Location = ELocation::Mountain;
	int32 PermanentOption = INDEX_NONE; // index into FPromotionReview::Options
	TArray<EGrantUpgrade> Buy;
	ESpecialty StartingSpecialty = ESpecialty::Technician;
	EEarthCondition Condition = EEarthCondition::DocumentaryCraze;
};

/**
 * The whole game state and rules, independent of rendering. The UI reads it and issues commands;
 * the player controller ticks it. Deterministic: seeded randomness, fixed-size steps.
 *
 * Two currencies on purpose: routine and extended experiments (and contracts) pay Research, which
 * buys upgrades; discovery projects pay nothing but unlock new targets and equipment.
 */
class ALIENIDLE_API FAlienIdleSim
{
public:
	FAlienIdleSim();

	void Tick(float DeltaSeconds);

	// Fleet commands
	bool CanLaunch(int32 Ufo, FString* OutReason = nullptr) const;
	bool Launch(int32 Ufo);
	/** Steps the destination to the next/previous specimen type (locked ones included, so the player sees what's next). */
	void CycleDestination(int32 Ufo, int32 Direction);
	/** Targets the specimen in its home region. */
	void SetDestination(int32 Ufo, ESpecimen Type);
	void SetDestinationRegion(int32 Ufo, int32 Region, ESpecimen Type);
	/** Targets the specimen in whichever region ChooseRegionFor prefers. */
	void SetDestinationBest(int32 Ufo, ESpecimen Type);
	void SetAutoLaunch(int32 Ufo, bool bEnabled);
	/** Contract mode: the ship picks its own target (and trait, with a scanner) from the active contract and lab priorities. */
	void SetFollowContract(int32 Ufo, bool bEnabled);
	/** The occupation a scanner ship would target for this destination right now, or None. */
	EOccupation GetScannerTarget(int32 Ufo, ESpecimen Type) const;

	// Upgrades
	bool CanBuyShipUpgrade(int32 Ufo, EShipSystem System, FString* OutReason = nullptr) const;
	bool BuyShipUpgrade(int32 Ufo, EShipSystem System);
	double GetShipUpgradeCost(int32 Ufo, EShipSystem System) const;
	int32 GetShipLevel(int32 Ufo, EShipSystem System) const { return Ufos[Ufo].Levels[int32(System)]; }
	int32 GetShipSlotsUsed(int32 Ufo) const;
	/** Strips every ship upgrade for a partial refund, so a full ship can change roles. Docked ships only. */
	bool CanRefitShip(int32 Ufo, FString* OutReason = nullptr) const;
	bool RefitShip(int32 Ufo);
	double GetRefitRefund(int32 Ufo) const;

	bool CanBuyBaseUpgrade(EBaseUpgrade Upgrade, FString* OutReason = nullptr) const;
	bool BuyBaseUpgrade(EBaseUpgrade Upgrade);
	double GetBaseUpgradeCost(EBaseUpgrade Upgrade) const;
	int32 GetBaseLevel(EBaseUpgrade Upgrade) const { return BaseLevels[int32(Upgrade)]; }

	// Lab priorities
	/** Options for a specimen type: routine, extended, then each open discovery project. */
	TArray<FExperimentChoice> GetExperimentOptions(ESpecimen Type) const;
	const FExperimentChoice& GetExperimentChoice(ESpecimen Type) const { return Choices[int32(Type)]; }
	void SetExperimentChoice(ESpecimen Type, const FExperimentChoice& Choice);
	void CycleExperimentChoice(ESpecimen Type, int32 Direction);
	FString DescribeExperiment(ESpecimen Type, const FExperimentChoice& Choice) const;
	bool IsDiscoveryOpen(EDiscovery Discovery, FString* OutReason = nullptr) const;
	bool IsDiscovered(EDiscovery Discovery) const { return Discovery != EDiscovery::None && Discovered[int32(Discovery)]; }
	int32 GetDiscoveryCount() const;
	bool IsMiddleManagementAvailable() const { return IsDiscovered(EDiscovery::VoluntaryMeetings); }
	bool IsMiddleManagementOn() const { return bMiddleManagement; }
	void SetMiddleManagement(bool bEnabled);
	bool IsInMeeting() const { return bMiddleManagement && MeetingRemaining > 0.0f; }
	float GetLabSpeed() const;

	// Thesis and promotion (prestige)
	const FCareer& GetCareer() const { return Career; }
	EThesis GetThesis() const { return Thesis; }
	ELocation GetLocation() const { return Location; }
	FString GetRankName() const;
	TArray<FThesisObjective> GetThesisObjectives() const;
	bool CanSubmitThesis(FString* OutReason = nullptr) const;
	FPromotionReview BuildPromotionReview() const;
	TArray<EThesis> GetNextThesisOffers() const;
	bool IsLocationUnlocked(ELocation Where) const { return Career.LocationUnlocked[int32(Where)]; }
	/** Awards grants, applies the choices, and starts the next run in place. Returns false if the thesis or choices are invalid. */
	bool Promote(const FNextAssignment& Next, FString* OutReason = nullptr);
	bool HasGrantUpgrade(EGrantUpgrade Upgrade) const { return Career.Upgrades[int32(Upgrade)]; }
	bool HasSignatureBonus(EDepartment Department) const { return SignatureDone[int32(Department)] || Career.PermanentSignature[int32(Department)]; }
	/** Dev/test: pretend the thesis objectives are met. */
	void DevCompleteThesis();

	// Locations
	/** The local stand-in for a specimen (at Oceanic, elephant work is done on whales). */
	ESpecimen Local(ESpecimen Type) const;
	ESpecimen GetDiscoverySpecimen(EDiscovery Discovery) const;
	bool IsSpecimenAtLocation(ESpecimen Type) const;
	/** Specimens only found at this posting (Oceanic: pufferfish, octopuses, whales). */
	bool IsNative(ESpecimen Type) const;
	int32 GetLocalExperiments() const;
	bool IsBaseUpgradeAvailable(EBaseUpgrade Upgrade) const;
	bool IsSurfaceRegion(int32 Region) const { return GetRegionDef(Region).Medium == AlienIdleTuning::MediumSurface; }
	bool IsUnderwaterRegion(int32 Region) const { return GetRegionDef(Region).Medium == AlienIdleTuning::MediumUnderwater; }
	/** Academy achievements and what each unlocks. */
	FString GetAchievementText() const;
	// Oceanic hull pressure
	bool HasHull() const { return Location == ELocation::Oceanic; }
	float GetHullPressure() const { return HullPressure; }
	/** Load on the hull per second (containment, tanks, turbines, the thesis). */
	float GetHullStress() const;
	/** Pumps: base plus Pressure Control crew. Pressure settles at 50 x stress / relief. */
	float GetHullRelief() const;
	bool IsAirlockSealed() const { return bAirlockSealed; }
	/** 0 at low tide, 1 at high tide. */
	float GetTideFactor() const;
	FString GetHullText() const;
	void DevSetHullPressure(float Pressure) { HullPressure = Pressure; }
	void DevUnlockLocation(ELocation Where) { Career.LocationUnlocked[int32(Where)] = true; }
	// Antarctic heat
	bool HasHeat() const { return Location == ELocation::Antarctic; }
	float GetHeatSupply() const;
	float GetHeatDemandLabs() const;
	float GetHeatDemandHangar() const;
	/** 0..1: how much of their heat the labs and the hangar get. 1 away from the Antarctic. */
	float GetLabWarmth() const;
	float GetHangarWarmth() const;
	EHeatPriority GetHeatPriority() const { return HeatPriority; }
	void SetHeatPriority(EHeatPriority Priority) { HeatPriority = Priority; }
	FString GetHeatText() const;
	bool IsPreserving() const { return Location == ELocation::Antarctic || GetBaseLevel(EBaseUpgrade::CryoPods) > 0; }
	int32 GetPreservedStudied() const { return PreservedStudied; }
	/** Freezer mode: routine studies wait until a sample has been preserved for a minute (more research each, slower turnover). */
	bool IsAgingSamples() const { return bAgeSamples; }
	void SetAgeSamples(bool bEnabled) { bAgeSamples = bEnabled; }
	static const TCHAR* GetHeatPriorityName(EHeatPriority Priority);

	// Builds: presets, approach templates, explanations and behaviour reports
	void SavePreset(int32 Slot, const FString& Name);
	bool ApplyPreset(int32 Slot);
	FString GetPresetSummary(int32 Slot) const;
	/** Sets targets, automation, lab priorities and standing orders to suit an approach (crew stays put). */
	void ApplyApproachTemplate(AlienIdleTuning::EApproach Approach);
	/** What the selected UFO's target pays and costs, with every multiplier spelled out. */
	FString ExplainTarget(int32 Ufo) const;
	/** Which approach the current operation looks like, from its ships and recent work. */
	AlienIdleTuning::EApproach ClassifyApproach(bool* bOutClear = nullptr) const;
	double SuspicionScale(int32 Region, ESpecimen Kind) const;

	// Rivals
	const FRivalState& GetRival() const { return Rival; }
	bool IsRivalActive(ERival Type) const { return Rival.Active[int32(Type)]; }
	void DevSetRivals(bool bBureau, bool bCelebrity, bool bExperimental) { Rival.Active[0] = bBureau; Rival.Active[1] = bCelebrity; Rival.Active[2] = bExperimental; }
	/** Offers the next race now (needs an active rival and no race offered or running). */
	void DevOfferRace() { if (!Rival.bRaceOffered && !Rival.bRaceActive && (Rival.Active[0] || Rival.Active[1] || Rival.Active[2])) { OfferRace(); } }
	FString GetRivalEventText(ERival Type) const;
	bool IsCelebrityStunt(int32 Region) const { return Rival.Celebrity.bActive && Rival.Celebrity.Region == Region; }
	bool IsBreach(int32 Region) const { return Rival.Breach.bActive && Rival.Breach.Region == Region; }
	/** The rival whose joint experiment is open, or ERival::Count when none. */
	ERival GetJointRival() const;
	FString GetJointText() const;
	void SetContribute(bool bEnabled) { Rival.bContribute = bEnabled; }
	double GetSellFraction() const;
	double GetBuyCost(ESpecimen Type) const;
	int32 GetRaceGrants() const;
	FString GetReputationText() const;
	bool IsSwept(int32 Region, ESpecimen Type) const { return Rival.bSweeping && Rival.SweepRegion == Region && Rival.SweepType == Type; }
	FString GetRivalStatusText() const;
	void SetSellOrder(ESpecimen Type, bool bEnabled);
	void SetBuyOrder(ESpecimen Type, bool bEnabled);
	FString GetRaceText() const;
	bool AcceptRace();
	void DeclineRace();

	// Research departments
	EDepartment GetPrimaryDepartment() const { return PrimaryDepartment; }
	/** Once per run. The other departments stay available at a higher research cost. */
	bool ChooseDepartment(EDepartment Department);
	bool IsProjectDone(EDeptProject Project) const { return ProjectsDone[int32(Project)]; }
	double GetProjectCost(EDeptProject Project) const;
	bool CanBuyProject(EDeptProject Project, FString* OutReason = nullptr) const;
	bool BuyProject(EDeptProject Project);
	int32 GetProjectsDone(EDepartment Department) const;
	/** Queue the department's signature experiment; the lab starts it once specimens and staff are in place. */
	bool CanBeginSignature(EDepartment Department, FString* OutReason = nullptr) const;
	bool BeginSignature(EDepartment Department);
	EDepartment GetPendingSignature() const { return PendingSignature; }
	bool IsSignatureDone(EDepartment Department) const { return SignatureDone[int32(Department)]; }
	bool IsSignatureRunning(EDepartment Department) const;
	FString GetSignatureStatusText(EDepartment Department) const;
	/** Specimens the queued signature experiment still needs in containment. */
	TArray<FSpecimen> GetSignatureMissingSpecimens() const;
	int32 GetShipSlots() const;

	// Contracts
	const TArray<FContract>& GetContractOffers() const { return Offers; }
	bool HasActiveContract() const { return bHasActiveContract; }
	const FContract& GetActiveContract() const { return ActiveContract; }
	bool AcceptContract(int32 Offer);
	void AbandonContract();
	int32 GetContractsCompleted() const { return ContractsCompleted; }
	/** Finished contracts wait here until their reward is claimed. With auto-accept on, the contract office claims them itself. */
	const TArray<FContract>& GetUnclaimedContracts() const { return Unclaimed; }
	double GetUnclaimedReward() const;
	bool ClaimContract(int32 Index);
	/** Claims every finished contract; returns how many. */
	int32 ClaimAllContracts();

	// Incidents
	const FActiveIncident& GetIncident() const { return Incident; }
	FString GetIncidentText() const;
	bool CanResolveIncident(FString* OutReason = nullptr) const;
	bool ResolveIncident();
	int32 GetEscapes() const { return Escapes; }
	int32 GetIncidentCount() const { return IncidentCount; }

	// Crew (Alien Resources)
	const TArray<FCrewMember>& GetCrew() const { return Crew; }
	double GetRecruitCost() const;
	bool CanRecruit(FString* OutReason = nullptr) const;
	/** "Request Additional Personnel": a new alien with a random specialty joins the pool, unassigned. */
	bool Recruit();
	double GetTrainCost(int32 Member) const;
	bool CanTrain(int32 Member, FString* OutReason = nullptr) const;
	bool Train(int32 Member);
	/** Free reassignment. Fails only if the station is full or does not exist. */
	bool Assign(int32 Member, const FAssignment& Where);
	void CycleAssignment(int32 Member, int32 Direction);
	TArray<FAssignment> GetAllStations() const;
	FString GetStationName(const FAssignment& Where) const;
	int32 GetStationCrew(const FAssignment& Where) const;
	bool IsStationStaffed(const FAssignment& Where) const { return GetStationCrew(Where) > 0; }
	/** Skill beyond the first basic crew member: 0 with one non-specialist; specialists and training add more. */
	float GetStationBonus(const FAssignment& Where) const;
	FString GetStationStaffingText(const FAssignment& Where) const;
	int32 GetIdleCrew() const;
	bool IsAutoAssignOn() const { return bAutoAssign; }
	void SetAutoAssign(bool bEnabled) { bAutoAssign = bEnabled; }
	/** Puts idle crew where they are needed: unstaffed minimums first (matching specialists preferred), then extras. */
	void AutoAssignIdle();
	static const TCHAR* GetSpecialtyName(ESpecialty Specialty);
	static ESpecialty GetStationSpecialty(EStation Station);

	// Automation: auto-launching ships hold at the dock instead of overfilling containment.
	bool IsPauseWhenFullOn() const { return bPauseWhenFull; }
	void SetPauseWhenFull(bool bEnabled) { bPauseWhenFull = bEnabled; }
	/** Standing order: when no contract is active, accept the best-paying offer the fleet is set up to serve. */
	bool IsAutoContractsOn() const { return bAutoContracts; }
	void SetAutoContracts(bool bEnabled) { bAutoContracts = bEnabled; }
	bool IsHeldForContainment(int32 Ufo) const;

	// Bottlenecks and the next upgrade
	EBottleneck GetCurrentBottleneck() const;
	FString GetBottleneckText() const;
	FString GetRecommendation() const;
	/** Every upgrade that would ease the current bottleneck (and Earth's pressure), by approach. */
	TArray<FString> GetBottleneckFixes() const;
	/** One line on how the operation is behaving: which approach it looks like and why. */
	FString GetApproachReading() const;
	static const TCHAR* GetBottleneckName(EBottleneck Bottleneck);

	// While You Were Out
	void BeginAwayReport();
	FAwayReport BuildAwayReport() const;
	int32 GetCaptures() const { return Captures; }

	// Saving (AlienIdleSave.cpp). The save holds the whole state; loading replaces this sim in place.
	void SaveToBytes(TArray<uint8>& OutBytes) const;
	bool LoadFromBytes(const TArray<uint8>& Bytes);
	/** Writes the state plus the wall-clock time of the save. */
	bool SaveToFile(const FString& Path) const;
	/** Loads in place; OutSecondsSinceSave is the wall-clock time since the file was written. */
	bool LoadFromFile(const FString& Path, double& OutSecondsSinceSave);
	/** Simulates time the player was away (capped at AlienIdleTuning::OfflineCapSeconds). Returns seconds simulated. */
	double CatchUp(double Seconds);
	static FString GetDefaultSavePath();
	void AddSystemLog(const FString& Text) { AddLog(Text); }

	// Case files: recurring subjects (AlienIdleCases.cpp)
	const FCaseFile& GetCaseFile(int32 Case) const { return Career.Cases[Case]; }
	bool IsCaseKnown(int32 Case) const { return Career.Cases[Case].Encounters > 0; }
	/** In containment, in a lab chamber or in a ship's hold. */
	bool IsCaseOnBase(int32 Case) const;
	bool IsCaseInContainment(int32 Case) const;
	int32 GetCaseStepsDone(int32 Case) const { return CaseStepsDone[Case]; }
	bool HasCaseDiscovery(int32 Case) const { return Career.Cases[Case].bDiscovered; }
	/** The next personal research step this subject can do right now, or -1. */
	int32 GetNextCaseStep(int32 Case) const;
	bool IsPeerReviewDone() const { return bPeerReviewDone; }
	/** Multi-line console text: who they are, where they are, their history, relationships and research chain. */
	FString GetCaseFileText(int32 Case) const;
	/** Relationship lines that matter right now (revealed once either subject has a case file). */
	TArray<FString> GetCaseRelationText(int32 Case) const;
	bool CanTargetCase(int32 Ufo, int32 Case, FString* OutReason = nullptr) const;
	/** Points the ship's scanner at a recurring subject (sets a human target in the best region). -1 clears it. */
	void SetCaseTarget(int32 Ufo, int32 Case);
	FString GetCaseTargetText(int32 Ufo) const;
	bool IsCaseStudiesOn() const { return bCaseStudies; }
	void SetCaseStudies(bool bEnabled) { bCaseStudies = bEnabled; }
	/** Research multiplier from earlier encounters and kindred company. */
	double GetCaseResearchMultiplier(const FSpecimen& Subject) const;
	static FString GetCaseName(int32 Case);
	/** Dev/test: a recurring subject is taken from the given region and put straight into containment. */
	void DevAddCaseSubject(int32 Case, int32 Region = -1);
	void DevSetCaseAppearances(bool bEnabled) { bCaseAppearances = bEnabled; }

	// Probe Calibration (AlienIdleCalibration.cpp): an optional five-second timing challenge on a running
	// experiment. Any attempt skips part of its remaining time; ignoring the window costs nothing.
	/** The chamber with an open calibration window, or INDEX_NONE. */
	int32 GetCalibrationChamber() const { return CalibrationChamber; }
	/** Seconds before the open window lapses (it waits while the challenge is on screen). */
	float GetCalibrationOfferRemaining() const;
	/** Centre of the highlighted band on the 0..1 gauge for the open window. */
	float GetCalibrationBand() const { return CalibrationBand; }
	bool IsCalibrationOpen() const { return bCalibrationOpen; }
	/** The player opened the challenge: holds the window until Calibrate is called. */
	bool BeginCalibration();
	/** Grades a stop of the cursor at this gauge position against the open window's band. */
	ECalibrationGrade GradeCalibration(float Cursor) const;
	/** Applies a calibration to the open window's experiment. OutResult gets the result line (or why it failed). */
	bool Calibrate(ECalibrationGrade Grade, FString* OutResult = nullptr);
	/** One console line: window open, cooldown, or why none is offered. */
	FString GetCalibrationStatusText() const;
	int32 GetManualCalibrations() const { return ManualCalibrations; }
	int32 GetTechnicianCalibrations() const { return TechnicianCalibrations; }
	/** Experiment seconds skipped by calibrations this run. */
	float GetCalibrationSecondsSaved() const { return CalibrationSecondsSaved; }
	/** Fraction of the remaining time a Calibration Desk technician skips (0 when the desk is unstaffed). */
	float GetTechnicianFraction() const;
	static float GetCalibrationFraction(ECalibrationGrade Grade);
	static const TCHAR* GetCalibrationGradeName(ECalibrationGrade Grade);
	/** Dev/test: opens a window now on this chamber (or the one with the most time left when INDEX_NONE). */
	bool DevOfferCalibration(int32 Chamber = INDEX_NONE);
	/** Dev/test: no windows at all. */
	void DevSetCalibrationEnabled(bool bEnabled) { bCalibrationEnabled = bEnabled; if (!bEnabled) { CalibrationChamber = INDEX_NONE; bCalibrationOpen = false; } }

	// Encyclopedia
	const TArray<FEncyclopediaEntry>& GetEncyclopedia() const { return Encyclopedia; }
	static int32 GetEncyclopediaSize();

	// Dev / test helpers
	void DevAddResearch(double Amount) { Research += FMath::Max(0.0, Amount); }
	void DevAddEnergy(double Amount) { Energy = FMath::Min(GetEnergyCap(), Energy + FMath::Max(0.0, Amount)); }
	void DevDiscover(EDiscovery Discovery);
	void DevUnlockSpecimen(ESpecimen Type);
	void DevSetIncidentsEnabled(bool bEnabled) { bIncidentsEnabled = bEnabled; }
	void DevAddToContainment(const FSpecimen& Specimen) { Containment.Add(Specimen); }
	/** Counts experiments as done (unlocks automation and opens discovery projects) without running them. */
	void DevAddExperiments(ESpecimen Type, int32 Count) { ExperimentsByType[int32(Type)] += Count; TotalExperiments += Count; }
	void DevSetActiveContract(const FContract& Contract) { ActiveContract = Contract; bHasActiveContract = true; }

	// Resources
	double GetEnergy() const { return Energy; }
	double GetEnergyCap() const;
	double GetEnergyPerSecond() const;
	double GetResearch() const { return Research; }
	/** Research earned from experiments and contracts this run (ignores spending and refunds). */
	double GetTotalResearchEarned() const { return ResearchEarned; }

	// Fleet
	int32 GetNumUfos() const { return Ufos.Num(); }
	const FUfo& GetUfo(int32 Ufo) const { return Ufos[Ufo]; }
	float GetUfoStateProgress(int32 Ufo) const;
	int32 GetTractorStrength(int32 Ufo) const;
	int32 GetCargoCapacity(int32 Ufo) const;
	float GetFlightSeconds(int32 Ufo, ESpecimen Type) const;
	float GetCaptureSeconds(int32 Ufo, ESpecimen Type, bool bScanning) const;
	double GetLaunchEnergy(int32 Ufo, ESpecimen Type) const;
	double GetSuspicionMultiplier(int32 Ufo) const;
	/** "Heavy lifter", "Bulk collector"... from the ship's dominant system. */
	FString GetShipRole(int32 Ufo) const;
	FString GetUfoStatusText(int32 Ufo) const;
	bool IsAutomationUnlocked() const { return TotalExperiments >= AlienIdleTuning::AutomationUnlockExperiments; }

	// Specimens and regions
	bool IsSpecimenUnlocked(ESpecimen Type) const;
	FString GetUnlockRequirementText(ESpecimen Type) const;
	// Regions: three competing hunting grounds per location.
	int32 GetNumRegions() const { return AlienIdleTuning::NumRegions; }
	const AlienIdleTuning::FRegionDef& GetRegionDef(int32 Region) const { return AlienIdleTuning::RegionDefs[int32(Location)][Region]; }
	const FRegionState& GetRegionState(int32 Region) const { return Regions[Region]; }
	FString GetRegionNameAt(int32 Region) const { return GetRegionDef(Region).Name; }
	bool RegionHasSpecimen(int32 Region, ESpecimen Type) const;
	bool IsRegionIndexOnAlert(int32 Region) const { return Regions[Region].Suspicion >= GetAlertSuspicion(Region); }
	double GetAlertSuspicion(int32 Region) const;
	int32 GetHomeRegion(ESpecimen Type) const { return AlienIdleTuning::HomeRegion[int32(Location)][int32(Type)]; }
	/** The best region for this specimen right now: value against exposure and Earth's responses. */
	int32 ChooseRegionFor(int32 Ufo, ESpecimen Type) const;
	// Older per-specimen views report the specimen's home region.
	const FRegionState& GetRegion(ESpecimen Type) const { return Regions[GetHomeRegion(Type)]; }
	bool IsRegionOnAlert(ESpecimen Type) const { return IsRegionIndexOnAlert(GetHomeRegion(Type)); }
	FString GetRegionName(ESpecimen Type) const { return GetRegionNameAt(GetHomeRegion(Type)); }

	// Earth's responses
	bool IsResponseActive(int32 Region, EResponse Response) const { return Regions[Region].Active[int32(Response)]; }
	float GetResponsePressure(int32 Region, EResponse Response) const { return Regions[Region].Pressure[int32(Response)]; }
	float GetResponseThreshold() const;
	double GetCounterCost(int32 Region, EResponse Response) const;
	bool CanCounter(int32 Region, EResponse Response, FString* OutReason = nullptr) const;
	/** Pay to make a response go away (decoys, jurisdiction disputes, weather balloons, absurd cover stories). */
	bool Counter(int32 Region, EResponse Response);
	int32 GetActiveResponseCount() const;
	int32 GetResponsesTriggered() const { return ResponsesTriggered; }
	int32 GetReroutes() const { return Reroutes; }
	/** Sum over time of active responses (response-seconds), a measure of how hard Earth pushed back. */
	double GetResponseSeconds() const { return ResponseSeconds; }
	bool IsRerouteOn() const { return bReroute; }
	void SetReroute(bool bEnabled) { bReroute = bEnabled; }
	EEarthCondition GetEarthCondition() const { return Condition; }
	void DevSetEarthCondition(EEarthCondition NewCondition) { Condition = NewCondition; }

	// Containment and lab
	const TArray<FSpecimen>& GetContainment() const { return Containment; }
	int32 GetContainmentUnitsUsed() const;
	int32 GetContainmentCapacity() const;
	static int32 GetUnits(const FSpecimen& Specimen);
	const TArray<FLabChamber>& GetChambers() const { return Chambers; }
	FString GetChamberStatusText(int32 Chamber) const;
	int32 GetExperimentsCompleted(ESpecimen Type) const { return ExperimentsByType[int32(Type)]; }
	int32 GetTotalExperiments() const { return TotalExperiments; }

	double GetElapsedSeconds() const { return Elapsed; }
	const TArray<FAlienIdleLogEntry>& GetLog() const { return Log; }
	/** Experiment and discovery reports only, newest last (shown in the results box). */
	const TArray<FAlienIdleLogEntry>& GetResults() const { return Results; }

	// Static text
	static FString GetSpecimenLabel(const FSpecimen& Specimen);
	static FString GetShipUpgradeEffectText(EShipSystem System);
	static FString GetShipUpgradeFlavor(EShipSystem System, int32 NewLevel);
	static FString GetBaseUpgradeEffectText(EBaseUpgrade Upgrade);
	static FString GetBaseUpgradeFlavor(EBaseUpgrade Upgrade, int32 NewLevel);

private:
	void Serialize(FArchive& Ar);
	void Step(float Dt);
	void StepUfo(int32 Index, float Dt);
	void StepLab(float Dt);
	void StepRegions(float Dt);
	void StepIncidents(float Dt);
	void StepAutomation(int32 Index);
	void TryUnload(int32 Index);
	void FinishCapture(int32 Index);
	void FinishExperiment(FLabChamber& Chamber);
	int32 PickNextSpecimen() const;
	FExperimentChoice ResolveExperiment(const FSpecimen& Specimen) const;
	float GetExperimentSeconds(const FSpecimen& Specimen, const FExperimentChoice& Choice) const;
	double GetExperimentEnergy(const FSpecimen& Specimen, const FExperimentChoice& Choice) const;
	double GetExperimentResearch(const FSpecimen& Specimen, const FExperimentChoice& Choice) const;
	void AddSuspicion(int32 Region, double Amount, ESpecimen Kind);
	void AddPressure(int32 Region, EResponse Response, float Amount);
	void StepResponses(float Dt);
	int32 ActiveRegion(int32 Ufo) const;
	bool HasRelevantResponse(int32 Region, ESpecimen Type) const;
	void StepRival(float Dt);
	void StepBehaviourMemo();
	void StepAutoContract();
	void StepHull(float Dt);
	void StepAchievements();
	void StepTenure();
	void StepOverheat();
	void StepCalibration(float Dt);
	void OfferCalibration(int32 Chamber);
	void StepCaseDynamics(float Dt);
	void MeetCase(FSpecimen& Subject, int32 Ufo);
	int32 RollCaseAppearance(const TArray<FSpecimen>& Taken);
	void NoteCaseReturn(const FSpecimen& Subject, bool bWiped);
	void NoteCaseEscape(const FSpecimen& Subject);
	void AddCaseVisit(int32 Case, const FString& Text);
	void CloseCaseVisit(int32 Case, const FString& Ending);
	void CompleteCaseStudy(const FSpecimen& Subject, const FSpecimen& Partner, uint8 Step);
	bool TryStartPeerReview(FLabChamber& Chamber);
	bool IsWaitingForPartner(const FSpecimen& Subject) const;
	bool IsKindredHeld(int32 Case) const;
	EDiscovery GetUnlockDiscovery(ESpecimen Type) const;
	void StepRivalEvents(float Dt);
	void StepJoint(float Dt);
	void OfferRace();
	void CountTowardRace(const FSpecimen& Subject);
	void SetUfoState(FUfo& Ufo, EUfoState NewState, float Duration);
	void AddUfo();
	void Discover(EDiscovery Discovery);
	void RecordEncyclopedia(const FSpecimen& Specimen);
	void AddEncyclopediaEntry(const FString& Title, const FString& Text);
	void FillContractOffers();
	FContract MakeContract();
	void CountTowardContract(const FSpecimen& Specimen);
	void StartIncident(EIncident Type, float Seconds, int32 Ufo = INDEX_NONE);
	void EscapeSpecimen(int32 ContainmentIndex);
	void AddLog(const FString& Text);
	void AddResult(const FString& Text);
	void AddCrew(ESpecialty Specialty, const FAssignment& Where);
	void StepCrew(float Dt);
	void ReturnSpecimen(const FSpecimen& Subject);
	bool IsOnDuty(int32 Member) const;
	bool HasSignatureFacilities(EDepartment Department, FString* OutMissing = nullptr) const;
	/** Containment indices held back for the pending signature experiment (one per requirement found). */
	TArray<int32> GetSignatureReservation(TArray<int32>* OutMissing = nullptr) const;
	bool TryStartSignature(FLabChamber& Chamber);
	void CompleteSignature(EDepartment Department);
	double GetLockdownRelease() const;
	FContract MakeDepartmentContract(EDepartment Department, bool& bOutMade);
	EOccupation RandomOccupation();
	void ApplyCareerStart();
	int32 CountStudiedProfessions() const;
	void StepBottlenecks(float Dt);
	int32 GetIncomingUnits() const;

	FRandomStream Rng;

	double Energy = 0.0;
	double Research = 0.0;
	double Elapsed = 0.0;

	TArray<FUfo> Ufos;
	TArray<FSpecimen> Containment;
	TArray<FLabChamber> Chambers;
	FRegionState Regions[AlienIdleTuning::NumRegions];
	int32 BaseLevels[int32(EBaseUpgrade::Count)] = {};
	FExperimentChoice Choices[int32(ESpecimen::Count)];

	int32 ExperimentsByType[int32(ESpecimen::Count)] = {};
	int32 TotalExperiments = 0;
	bool Discovered[int32(EDiscovery::Count)] = {};
	bool DevUnlocked[int32(ESpecimen::Count)] = {};
	bool AnnouncedUnlock[int32(ESpecimen::Count)] = {};
	bool AnnouncedProject[int32(EDiscovery::Count)] = {};
	bool bAnnouncedAutomation = false;

	bool bMiddleManagement = false;
	float MeetingTimer = 0.0f;
	float MeetingRemaining = 0.0f;

	TArray<FContract> Offers;
	FContract ActiveContract;
	bool bHasActiveContract = false;
	int32 ContractsCompleted = 0;
	int32 ContractsGenerated = 0;
	TArray<FContract> Unclaimed;

	FActiveIncident Incident;
	float IncidentCooldown = AlienIdleTuning::FirstIncidentSeconds;
	bool bIncidentsEnabled = true;
	int32 Escapes = 0;
	int32 IncidentCount = 0;
	double ResearchEarned = 0.0;

	bool SeenSpecimen[int32(ESpecimen::Count)] = {};
	bool SeenOccupation[int32(EOccupation::Count)] = {};
	TArray<FEncyclopediaEntry> Encyclopedia;

	TArray<FAlienIdleLogEntry> Log;
	TArray<FAlienIdleLogEntry> Results;

	TArray<FCrewMember> Crew;
	int32 Recruited = 0;
	bool bAutoAssign = true;
	bool bWarnedUnwiped = false;

	FCareer Career;
	EThesis Thesis = EThesis::PreliminaryObservations;
	ELocation Location = ELocation::Mountain;
	bool StudiedOccupation[int32(EOccupation::Count)] = {};
	int32 RunNewPublications = 0;
	int32 RunNewEntries = 0;

	EEarthCondition Condition = EEarthCondition::BusinessAsUsual;
	FRivalState Rival;
	float NextBehaviourMemo = AlienIdleTuning::BehaviourMemoSeconds;
	int32 BehaviourMemos = 0;
	/** True while simulating time the player was away: rival races pause so nothing is lost offline. */
	bool bCatchingUp = false;
	bool bReroute = true;
	int32 ResponsesTriggered = 0;
	int32 Reroutes = 0;
	double ResponseSeconds = 0.0;

	EDepartment PrimaryDepartment = EDepartment::None;
	bool ProjectsDone[int32(EDeptProject::Count)] = {};
	EDepartment PendingSignature = EDepartment::None;
	bool SignatureDone[int32(EDepartment::Count)] = {};

	bool bAutoContracts = false;
	float HullPressure = 0.0f;
	EHeatPriority HeatPriority = EHeatPriority::Even;
	int32 PreservedStudied = 0;
	bool bAgeSamples = false;
	bool bOverheating = false;
	int32 OverheatJokes = 0;
	bool bAirlockSealed = false;
	bool bHullCreaking = false;
	bool bPauseWhenFull = true;
	int32 Captures = 0;
	float BottleneckSeconds[int32(EBottleneck::Count)] = {};
	float BottleneckRecent[int32(EBottleneck::Count)] = {};

	// Case files (per run; the permanent record is FCareer::Cases)
	FRandomStream CaseRng;
	int32 CaseStepsDone[AlienIdleTuning::NumCaseSubjects] = {};
	/** What happened on the current encounter, closed into the history when they leave. */
	FString CaseVisit[AlienIdleTuning::NumCaseSubjects];
	bool bCaseStudies = true;
	bool bCaseAppearances = true;
	bool bPeerReviewDone = false;
	float CaseTrouble[AlienIdleTuning::NumCaseRelations] = {};
	int32 RunCaseDiscoveries = 0;

	// Probe Calibration (per run)
	FRandomStream CalibrationRng;
	int32 CalibrationChamber = INDEX_NONE;
	/** Until the next window when none is open. */
	float CalibrationTimer = AlienIdleTuning::CalibrationFirstSeconds;
	/** Age of the open window; while the challenge is on screen, time since it opened. */
	float CalibrationOfferAge = 0.0f;
	float CalibrationBand = 0.5f;
	/** The challenge is on screen. Not saved: a reload never leaves a window stuck open. */
	bool bCalibrationOpen = false;
	bool bCalibrationEnabled = true;
	bool bAnnouncedCalibration = false;
	bool bAnnouncedDesk = false;
	int32 ManualCalibrations = 0;
	int32 TechnicianCalibrations = 0;
	float CalibrationSecondsSaved = 0.0f;

	struct FAwaySnapshot
	{
		double Elapsed = 0.0;
		int32 Captures = 0;
		int32 Experiments = 0;
		double ResearchEarned = 0.0;
		bool Discovered[int32(EDiscovery::Count)] = {};
		int32 Contracts = 0;
		int32 Escapes = 0;
		int32 Incidents = 0;
		float BottleneckSeconds[int32(EBottleneck::Count)] = {};
	} AwaySnapshot;
};
