#include "AlienIdleSim.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

// Save format: every field of FAlienIdleSim, in declaration order. Bump AlienIdleTuning::SaveVersion
// whenever a field is added, removed or reordered; older saves are then rejected rather than misread.

namespace
{
	template <typename EnumType>
	void SerializeEnum(FArchive& Ar, EnumType& Value)
	{
		uint8 Byte = uint8(Value);
		Ar << Byte;
		Value = EnumType(Byte);
	}

	template <typename ElementType, int32 Num>
	void SerializeFixed(FArchive& Ar, ElementType (&Values)[Num])
	{
		for (int32 i = 0; i < Num; ++i)
		{
			Ar << Values[i];
		}
	}

	constexpr uint32 SaveMagic = 0x41494453; // "AIDS": Alien Idle Data Save. The name was chosen by committee.
}

static FArchive& operator<<(FArchive& Ar, FSpecimen& Value)
{
	SerializeEnum(Ar, Value.Type);
	SerializeEnum(Ar, Value.Occupation);
	Ar << Value.Region << Value.Held << Value.Case;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FAssignment& Value)
{
	SerializeEnum(Ar, Value.Station);
	Ar << Value.Index;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FCrewMember& Value)
{
	Ar << Value.Name;
	SerializeEnum(Ar, Value.Specialty);
	Ar << Value.Training << Value.TrainingRemaining << Value.Assignment;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FUfo& Value)
{
	Ar << Value.Name;
	SerializeEnum(Ar, Value.State);
	Ar << Value.StateTime << Value.StateDuration;
	SerializeEnum(Ar, Value.Destination);
	Ar << Value.Region << Value.TripRegion;
	SerializeEnum(Ar, Value.TripTarget);
	SerializeEnum(Ar, Value.TripOccupation);
	Ar << Value.bAutoLaunch << Value.bFollowContract << Value.CaseTarget << Value.TripCase << Value.Cargo;
	SerializeFixed(Ar, Value.Levels);
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FExperimentChoice& Value)
{
	SerializeEnum(Ar, Value.Kind);
	SerializeEnum(Ar, Value.Discovery);
	SerializeEnum(Ar, Value.Department);
	Ar << Value.CaseStep;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FLabChamber& Value)
{
	Ar << Value.bRunning << Value.Time << Value.Duration << Value.Specimen << Value.Experiment << Value.Batch << Value.Partner;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FRegionState& Value)
{
	Ar << Value.Suspicion << Value.bLockedDown;
	SerializeFixed(Ar, Value.Pressure);
	SerializeFixed(Ar, Value.Active);
	SerializeFixed(Ar, Value.CounterUses);
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FContractNeed& Value)
{
	SerializeEnum(Ar, Value.Type);
	SerializeEnum(Ar, Value.Occupation);
	Ar << Value.Count << Value.bDistinctOccupations << Value.Progress;
	int32 SeenNum = Value.Seen.Num();
	Ar << SeenNum;
	if (Ar.IsLoading())
	{
		Value.Seen.SetNum(SeenNum);
	}
	for (EOccupation& Seen : Value.Seen)
	{
		SerializeEnum(Ar, Seen);
	}
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FContract& Value)
{
	Ar << Value.Title << Value.Flavor << Value.Reward << Value.Needs;
	SerializeEnum(Ar, Value.Department);
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FActiveIncident& Value)
{
	SerializeEnum(Ar, Value.Type);
	Ar << Value.Remaining << Value.Ufo << Value.Crew;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FEncyclopediaEntry& Value)
{
	Ar << Value.Title << Value.Text << Value.Time;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FAlienIdleLogEntry& Value)
{
	Ar << Value.Time << Value.Text;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FExperimentChoice& Value);
static FArchive& operator<<(FArchive& Ar, FAssignment& Value);

static FArchive& operator<<(FArchive& Ar, FUfoPreset& Value)
{
	SerializeEnum(Ar, Value.Destination);
	Ar << Value.Region << Value.bAutoLaunch << Value.bFollowContract;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FBuildPreset& Value)
{
	Ar << Value.bSaved << Value.Name << Value.Ufos << Value.Crew;
	SerializeFixed(Ar, Value.Choices);
	SerializeFixed(Ar, Value.Sell);
	SerializeFixed(Ar, Value.Buy);
	Ar << Value.bReroute << Value.bPauseWhenFull << Value.bContribute << Value.bAutoContracts;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FCaseFile& Value)
{
	Ar << Value.Encounters << Value.RunsMet << Value.LastRun << Value.BestSteps << Value.bDiscovered << Value.History;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FCareer& Value)
{
	Ar << Value.Run << Value.Promotions << Value.Grants << Value.TotalGrants;
	SerializeFixed(Ar, Value.Upgrades);
	SerializeEnum(Ar, Value.StartingSpecialty);
	int32 Num = Value.PermanentProjects.Num();
	Ar << Num;
	if (Ar.IsLoading())
	{
		Value.PermanentProjects.SetNum(Num);
	}
	for (EDeptProject& Project : Value.PermanentProjects)
	{
		SerializeEnum(Ar, Project);
	}
	SerializeFixed(Ar, Value.PermanentSignature);
	SerializeFixed(Ar, Value.LocationUnlocked);
	SerializeFixed(Ar, Value.LocationPerk);
	SerializeFixed(Ar, Value.Published);
	SerializeFixed(Ar, Value.PublishedSignature);
	Ar << Value.RaceWins << Value.RaceLosses << Value.Trades << Value.Collaborations;
	SerializeFixed(Ar, Value.Presets);
	SerializeFixed(Ar, Value.Cases);
	Ar << Value.bPeerReviewed;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FRivalEvent& Value)
{
	Ar << Value.NextAt << Value.bAnnounced << Value.bActive << Value.EndsAt << Value.Region << Value.Count;
	return Ar;
}

static FArchive& operator<<(FArchive& Ar, FRivalState& Value)
{
	Ar << Value.NextSweep << Value.bAnnounced << Value.bSweeping << Value.SweepEnds << Value.SweepRegion;
	SerializeEnum(Ar, Value.SweepType);
	Ar << Value.Sweeps;
	SerializeFixed(Ar, Value.Sell);
	SerializeFixed(Ar, Value.Buy);
	Ar << Value.SellTimer << Value.BuyTimer << Value.TradesThisRun << Value.TradeIncome;
	Ar << Value.bRaceOffered << Value.bRaceActive << Value.NextRaceOffer;
	SerializeEnum(Ar, Value.RaceType);
	Ar << Value.RaceCount << Value.RaceProgress << Value.RivalProgress << Value.RaceReward;
	for (int32 r = 0; r < AlienIdleTuning::NumRegions; ++r)
	{
		SerializeFixed(Ar, Value.Usage[r]);
	}
	SerializeFixed(Ar, Value.Active);
	Ar << Value.Celebrity << Value.Breach;
	SerializeEnum(Ar, Value.RaceOpponent);
	Ar << Value.bContribute << Value.ContributeTimer;
	SerializeFixed(Ar, Value.JointProgress);
	SerializeFixed(Ar, Value.JointDone);
	return Ar;
}

void FAlienIdleSim::Serialize(FArchive& Ar)
{
	int32 Seed = Rng.GetCurrentSeed();
	Ar << Seed;
	if (Ar.IsLoading())
	{
		Rng.Initialize(Seed);
	}

	Ar << Energy << Research << Elapsed;
	Ar << Ufos << Containment << Chambers;
	SerializeFixed(Ar, Regions);
	SerializeFixed(Ar, BaseLevels);
	SerializeFixed(Ar, Choices);

	SerializeFixed(Ar, ExperimentsByType);
	Ar << TotalExperiments;
	SerializeFixed(Ar, Discovered);
	SerializeFixed(Ar, DevUnlocked);
	SerializeFixed(Ar, AnnouncedUnlock);
	SerializeFixed(Ar, AnnouncedProject);
	Ar << bAnnouncedAutomation;

	Ar << bMiddleManagement << MeetingTimer << MeetingRemaining;

	Ar << Offers << ActiveContract << bHasActiveContract << ContractsCompleted << ContractsGenerated << Unclaimed;

	Ar << Incident << IncidentCooldown << bIncidentsEnabled << Escapes << IncidentCount << ResearchEarned;

	SerializeFixed(Ar, SeenSpecimen);
	SerializeFixed(Ar, SeenOccupation);
	Ar << Encyclopedia << Log << Results;

	Ar << Crew << Recruited << bAutoAssign << bWarnedUnwiped;

	Ar << Career;
	SerializeEnum(Ar, Thesis);
	SerializeEnum(Ar, Location);
	SerializeEnum(Ar, Condition);
	Ar << Rival;
	Ar << NextBehaviourMemo << BehaviourMemos;
	Ar << bReroute << ResponsesTriggered << Reroutes << ResponseSeconds;
	SerializeFixed(Ar, StudiedOccupation);
	Ar << RunNewPublications << RunNewEntries;

	SerializeEnum(Ar, PrimaryDepartment);
	SerializeFixed(Ar, ProjectsDone);
	SerializeEnum(Ar, PendingSignature);
	SerializeFixed(Ar, SignatureDone);

	Ar << bPauseWhenFull << bAutoContracts << Captures;
	Ar << HullPressure << bAirlockSealed << bHullCreaking;
	SerializeEnum(Ar, HeatPriority);
	Ar << PreservedStudied << bAgeSamples << bOverheating << OverheatJokes;
	SerializeFixed(Ar, BottleneckSeconds);
	SerializeFixed(Ar, BottleneckRecent);

	int32 CaseSeed = CaseRng.GetCurrentSeed();
	Ar << CaseSeed;
	if (Ar.IsLoading())
	{
		CaseRng.Initialize(CaseSeed);
	}
	SerializeFixed(Ar, CaseStepsDone);
	SerializeFixed(Ar, CaseVisit);
	Ar << bCaseStudies << bCaseAppearances << bPeerReviewDone;
	SerializeFixed(Ar, CaseTrouble);
	Ar << RunCaseDiscoveries;

	int32 CalibrationSeed = CalibrationRng.GetCurrentSeed();
	Ar << CalibrationSeed;
	if (Ar.IsLoading())
	{
		CalibrationRng.Initialize(CalibrationSeed);
		bCalibrationOpen = false; // the challenge screen does not survive a reload
	}
	Ar << CalibrationChamber << CalibrationTimer << CalibrationOfferAge << CalibrationBand << bCalibrationEnabled;
	Ar << bAnnouncedCalibration << bAnnouncedDesk << ManualCalibrations << TechnicianCalibrations << CalibrationSecondsSaved;
}

void FAlienIdleSim::SaveToBytes(TArray<uint8>& OutBytes) const
{
	FBufferArchive Ar;
	uint32 Magic = SaveMagic;
	int32 Version = AlienIdleTuning::SaveVersion;
	Ar << Magic << Version;
	const_cast<FAlienIdleSim*>(this)->Serialize(Ar);
	OutBytes = MoveTemp(Ar);
}

bool FAlienIdleSim::LoadFromBytes(const TArray<uint8>& Bytes)
{
	FMemoryReader Ar(Bytes);
	uint32 Magic = 0;
	int32 Version = 0;
	Ar << Magic << Version;
	if (Magic != SaveMagic || Version != AlienIdleTuning::SaveVersion)
	{
		return false;
	}
	FAlienIdleSim Loaded;
	Loaded.Serialize(Ar);
	if (Ar.IsError())
	{
		return false;
	}
	*this = MoveTemp(Loaded);
	BeginAwayReport();
	return true;
}

FString FAlienIdleSim::GetDefaultSavePath()
{
	return FPaths::ProjectSavedDir() / TEXT("SaveGames") / TEXT("MountainStation7.sav");
}

bool FAlienIdleSim::SaveToFile(const FString& Path) const
{
	TArray<uint8> Payload;
	SaveToBytes(Payload);
	FBufferArchive Ar;
	int64 Ticks = FDateTime::UtcNow().GetTicks();
	Ar << Ticks << Payload;
	return FFileHelper::SaveArrayToFile(Ar, *Path);
}

bool FAlienIdleSim::LoadFromFile(const FString& Path, double& OutSecondsSinceSave)
{
	OutSecondsSinceSave = 0.0;
	TArray<uint8> File;
	if (!FFileHelper::LoadFileToArray(File, *Path))
	{
		return false;
	}
	FMemoryReader Ar(File);
	int64 Ticks = 0;
	TArray<uint8> Payload;
	Ar << Ticks << Payload;
	if (Ar.IsError() || !LoadFromBytes(Payload))
	{
		return false;
	}
	OutSecondsSinceSave = FMath::Max(0.0, (FDateTime::UtcNow() - FDateTime(Ticks)).GetTotalSeconds());
	return true;
}

double FAlienIdleSim::CatchUp(double Seconds)
{
	const double Simulated = FMath::Clamp(Seconds, 0.0, AlienIdleTuning::OfflineCapSeconds);
	// Tick in one-minute chunks; each chunk is still split into the usual fixed steps.
	TGuardValue<bool> CatchingUp(bCatchingUp, true);
	double Remaining = Simulated;
	while (Remaining > 0.0)
	{
		const float Chunk = float(FMath::Min(Remaining, 60.0));
		Tick(Chunk);
		Remaining -= Chunk;
	}
	return Simulated;
}
