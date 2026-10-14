#include "AlienIdleSim.h"
#include "AlienIdleTuning.h"
#include "Misc/AutomationTest.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	constexpr EAutomationTestFlags SimTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter;

	/** Ticks until the predicate holds, in 0.05 s frames. Returns false on timeout. */
	template <typename PredicateType>
	bool RunUntil(FAlienIdleSim& Sim, PredicateType Predicate, float TimeoutSeconds = 120.0f)
	{
		for (float T = 0.0f; T < TimeoutSeconds; T += 0.05f)
		{
			if (Predicate())
			{
				return true;
			}
			Sim.Tick(0.05f);
		}
		return Predicate();
	}

	void BuyShip(FAlienIdleSim& Sim, int32 Ufo, EShipSystem System, int32 Levels)
	{
		for (int32 i = 0; i < Levels; ++i)
		{
			Sim.DevAddResearch(Sim.GetShipUpgradeCost(Ufo, System));
			Sim.BuyShipUpgrade(Ufo, System);
		}
	}

	void BuyBase(FAlienIdleSim& Sim, EBaseUpgrade Upgrade, int32 Levels)
	{
		for (int32 i = 0; i < Levels; ++i)
		{
			Sim.DevAddResearch(Sim.GetBaseUpgradeCost(Upgrade));
			Sim.BuyBaseUpgrade(Upgrade);
		}
	}

	FContract MakeContract(ESpecimen Type, EOccupation Occupation, int32 Count, double Reward)
	{
		FContract Contract;
		Contract.Title = TEXT("Test contract");
		Contract.Reward = Reward;
		FContractNeed Need;
		Need.Type = Type;
		Need.Occupation = Occupation;
		Need.Count = Count;
		Contract.Needs.Add(Need);
		return Contract;
	}

	/** A quiet sim: no random incidents, so timing tests are exact. */
	struct FQuietSim : FAlienIdleSim
	{
		FQuietSim() { DevSetIncidentsEnabled(false); }
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleSquirrelExpeditionTest, "AlienIdle.Sim.SquirrelExpedition", SimTestFlags)
bool FAlienIdleSquirrelExpeditionTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	TestEqual(TEXT("Starts with start energy"), Sim.GetEnergy(), AlienIdleTuning::StartEnergy);
	TestEqual(TEXT("Three contracts on offer"), Sim.GetContractOffers().Num(), AlienIdleTuning::ContractOffers);
	TestTrue(TEXT("Launch succeeds"), Sim.Launch(0));
	TestFalse(TEXT("Cannot launch while flying"), Sim.CanLaunch(0));
	TestTrue(TEXT("Captures and returns"), RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; }));
	TestTrue(TEXT("Docks"), RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Docked; }));
	TestTrue(TEXT("Lab is running"), RunUntil(Sim, [&] { return Sim.GetChambers()[0].bRunning; }, 1.0f));
	TestTrue(TEXT("Experiment completes"), RunUntil(Sim, [&] { return Sim.GetTotalExperiments() == 1; }));
	TestEqual(TEXT("Routine research earned"), Sim.GetResearch(), AlienIdleTuning::Specimen(ESpecimen::Squirrel).Research);
	TestTrue(TEXT("First experiment within 30 s"), Sim.GetElapsedSeconds() < 30.0);
	TestEqual(TEXT("First capture filed in the encyclopedia"), Sim.GetEncyclopedia().Num(), 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleThrustersUpgradeTest, "AlienIdle.Sim.ThrustersUpgrade", SimTestFlags)
bool FAlienIdleThrustersUpgradeTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	const double Cost = Sim.GetShipUpgradeCost(0, EShipSystem::Thrusters);
	while (Sim.GetResearch() < Cost)
	{
		TestTrue(TEXT("Expedition can launch"), RunUntil(Sim, [&] { return Sim.CanLaunch(0); }));
		Sim.Launch(0);
		const int32 Before = Sim.GetTotalExperiments();
		TestTrue(TEXT("Expedition finishes its experiment"), RunUntil(Sim, [&] { return Sim.GetTotalExperiments() > Before; }));
	}
	const float FlightBefore = Sim.GetFlightSeconds(0, ESpecimen::Squirrel);
	TestTrue(TEXT("Buys thrusters"), Sim.BuyShipUpgrade(0, EShipSystem::Thrusters));
	TestEqual(TEXT("Flight is faster"), Sim.GetFlightSeconds(0, ESpecimen::Squirrel), FlightBefore * AlienIdleTuning::ThrustersFlightMultiplier);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleDiscoveryUnlockTest, "AlienIdle.Sim.DiscoveryUnlocksTargets", SimTestFlags)
bool FAlienIdleDiscoveryUnlockTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	FString Reason;
	TestFalse(TEXT("Cows locked at start"), Sim.IsSpecimenUnlocked(ESpecimen::Cow));
	TestFalse(TEXT("Project closed without baseline data"), Sim.IsDiscoveryOpen(EDiscovery::AcornAmnesia, &Reason));
	TestTrue(TEXT("Reason asks for baseline data"), Reason.Contains(TEXT("baseline")));

	// Baseline routine squirrel studies open the project.
	const int32 Baseline = AlienIdleTuning::Discovery(EDiscovery::AcornAmnesia).RequiredExperiments;
	for (int32 i = 0; i < Baseline; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
	}
	TestTrue(TEXT("Baseline studies done"), RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetExperimentsCompleted(ESpecimen::Squirrel) >= Baseline; }, 240.0f));
	TestTrue(TEXT("Project open"), Sim.IsDiscoveryOpen(EDiscovery::AcornAmnesia));
	TestEqual(TEXT("Options: routine, extended, one project"), Sim.GetExperimentOptions(ESpecimen::Squirrel).Num(), 3);

	// Pick the project in lab priorities; it pays no research but unlocks cows.
	Sim.CycleExperimentChoice(ESpecimen::Squirrel, -1);
	TestEqual(TEXT("Project selected"), Sim.GetExperimentChoice(ESpecimen::Squirrel).Discovery, EDiscovery::AcornAmnesia);
	const double ResearchBefore = Sim.GetResearch();
	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	TestTrue(TEXT("Discovery completes"), RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.IsDiscovered(EDiscovery::AcornAmnesia); }, 60.0f));
	TestEqual(TEXT("Discovery pays no research"), Sim.GetResearch(), ResearchBefore);
	TestTrue(TEXT("Cows unlocked"), Sim.IsSpecimenUnlocked(ESpecimen::Cow));
	TestEqual(TEXT("Lab priority returns to routine"), Sim.GetExperimentChoice(ESpecimen::Squirrel).Kind, EExperimentKind::Routine);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleExperimentKindsTest, "AlienIdle.Sim.ExperimentKinds", SimTestFlags)
bool FAlienIdleExperimentKindsTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.SetExperimentChoice(ESpecimen::Squirrel, { EExperimentKind::Extended, EDiscovery::None });
	Sim.DevAddEnergy(1000.0);
	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	Sim.Tick(0.1f);
	TestTrue(TEXT("Extended study running"), Sim.GetChambers()[0].bRunning && Sim.GetChambers()[0].Experiment.Kind == EExperimentKind::Extended);
	TestEqual(TEXT("Extended takes 2.5x"), Sim.GetChambers()[0].Duration, AlienIdleTuning::Specimen(ESpecimen::Squirrel).LabSeconds * AlienIdleTuning::ExtendedDurationMultiplier);
	RunUntil(Sim, [&] { return Sim.GetTotalExperiments() == 1; });
	TestEqual(TEXT("Extended pays 3.5x"), Sim.GetResearch(), FMath::RoundToDouble(AlienIdleTuning::Specimen(ESpecimen::Squirrel).Research * AlienIdleTuning::ExtendedResearchMultiplier));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleTraitsTest, "AlienIdle.Sim.HumanTraits", SimTestFlags)
bool FAlienIdleTraitsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Magician needs 2 units"), FAlienIdleSim::GetUnits({ ESpecimen::Human, EOccupation::Magician }), 2);

	FQuietSim Sim;
	Sim.DevUnlockSpecimen(ESpecimen::Human);
	Sim.DevAddEnergy(1000.0);
	Sim.DevAddToContainment({ ESpecimen::Human, EOccupation::Lawyer });
	Sim.Tick(0.1f);
	TestEqual(TEXT("Lawyer delays the experiment pending the consent form"), Sim.GetChambers()[0].Duration, AlienIdleTuning::Specimen(ESpecimen::Human).LabSeconds * 1.5f);
	RunUntil(Sim, [&] { return Sim.GetTotalExperiments() == 1; });
	TestEqual(TEXT("Lawyer pays +25%"), Sim.GetResearch(), FMath::RoundToDouble(AlienIdleTuning::Specimen(ESpecimen::Human).Research * 1.25));

	Sim.DevAddToContainment({ ESpecimen::Human, EOccupation::Therapist });
	TestEqual(TEXT("A therapist on base speeds up the lab"), Sim.GetLabSpeed(), AlienIdleTuning::TherapistLabSpeed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleTractorGateTest, "AlienIdle.Sim.TractorGate", SimTestFlags)
bool FAlienIdleTractorGateTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.SetDestination(0, ESpecimen::Cow);
	FString Reason;
	TestFalse(TEXT("Cows locked at start"), Sim.CanLaunch(0, &Reason));
	TestTrue(TEXT("Reason names the discovery"), Reason.Contains(TEXT("Acorn Amnesia")));

	Sim.DevDiscover(EDiscovery::AcornAmnesia);
	TestFalse(TEXT("Weak tractor cannot lift cows"), Sim.CanLaunch(0, &Reason));
	TestTrue(TEXT("Reason names the tractor"), Reason.Contains(TEXT("tractor")));
	BuyShip(Sim, 0, EShipSystem::Tractor, 1);
	TestTrue(TEXT("Can launch for cows"), Sim.CanLaunch(0));

	Sim.DevAddResearch(10000.0);
	TestFalse(TEXT("Tractor III needs a discovery"), Sim.CanBuyShipUpgrade(0, EShipSystem::Tractor, &Reason));
	TestTrue(TEXT("Reason names Bovine Load Tolerance"), Reason.Contains(TEXT("Bovine Load Tolerance")));
	Sim.DevDiscover(EDiscovery::BovineLoadTolerance);
	TestTrue(TEXT("Tractor III available after the discovery"), Sim.CanBuyShipUpgrade(0, EShipSystem::Tractor));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleSlotBudgetTest, "AlienIdle.Sim.SlotBudget", SimTestFlags)
bool FAlienIdleSlotBudgetTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	BuyShip(Sim, 0, EShipSystem::Stealth, 3);
	TestEqual(TEXT("Stealth uses two slots per level"), Sim.GetShipSlotsUsed(0), 6);
	Sim.DevAddResearch(1000.0);
	FString Reason;
	TestFalse(TEXT("No slot left for cargo"), Sim.CanBuyShipUpgrade(0, EShipSystem::Cargo, &Reason));
	TestTrue(TEXT("Reason names slots"), Reason.Contains(TEXT("slots")));
	TestEqual(TEXT("Role follows the dominant system"), Sim.GetShipRole(0), FString(TEXT("Discreet abductor")));
	const double Refund = Sim.GetRefitRefund(0);
	const double Before = Sim.GetResearch();
	TestTrue(TEXT("Refit strips the ship"), Sim.RefitShip(0));
	TestEqual(TEXT("Refund paid"), Sim.GetResearch(), Before + Refund);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleContainmentFullTest, "AlienIdle.Sim.ContainmentFullWaits", SimTestFlags)
bool FAlienIdleContainmentFullTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.DevDiscover(EDiscovery::AcornAmnesia);
	Sim.SetDestination(0, ESpecimen::Cow);
	BuyShip(Sim, 0, EShipSystem::Tractor, 1);
	BuyShip(Sim, 0, EShipSystem::Cargo, 4);
	Sim.DevAddEnergy(1000.0);
	Sim.Launch(0);
	bool bSawWaiting = false;
	bool bOverCapacity = false;
	RunUntil(Sim, [&]
	{
		bSawWaiting |= Sim.GetUfo(0).State == EUfoState::WaitingToUnload;
		bOverCapacity |= Sim.GetContainmentUnitsUsed() > Sim.GetContainmentCapacity();
		return bSawWaiting && Sim.GetUfo(0).State == EUfoState::Docked;
	}, 300.0f);
	TestTrue(TEXT("UFO waited for containment space"), bSawWaiting);
	TestFalse(TEXT("Containment never over capacity"), bOverCapacity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleStealthLockdownTest, "AlienIdle.Sim.StealthAndLockdown", SimTestFlags)
bool FAlienIdleStealthLockdownTest::RunTest(const FString& Parameters)
{
	auto TripSuspicion = [](int32 StealthLevels)
	{
		FQuietSim Sim;
		BuyShip(Sim, 0, EShipSystem::Stealth, StealthLevels);
		Sim.Launch(0);
		RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
		return Sim.GetRegion(ESpecimen::Squirrel).Suspicion;
	};
	TestTrue(TEXT("Stealth reduces suspicion"), TripSuspicion(2) < TripSuspicion(0) * 0.5);

	FQuietSim Sim;
	Sim.DevUnlockSpecimen(ESpecimen::Human);
	Sim.DevDiscover(EDiscovery::UnscheduledDepartures);
	Sim.SetDestination(0, ESpecimen::Human);
	BuyShip(Sim, 0, EShipSystem::Cargo, 4);
	for (int32 i = 0; i < 4 && !Sim.GetRegion(ESpecimen::Human).bLockedDown; ++i)
	{
		Sim.DevAddEnergy(1000.0);
		BuyBase(Sim, EBaseUpgrade::Containment, 1);
		RunUntil(Sim, [&] { return Sim.CanLaunch(0); });
		Sim.Launch(0);
		RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
	}
	TestTrue(TEXT("Region locked down"), Sim.GetRegion(ESpecimen::Human).bLockedDown);
	RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Docked; });
	FString Reason;
	TestFalse(TEXT("Cannot launch into lockdown"), Sim.CanLaunch(0, &Reason));
	TestTrue(TEXT("Reason names lockdown"), Reason.Contains(TEXT("lockdown")));
	TestTrue(TEXT("Lockdown lifts as suspicion decays"), RunUntil(Sim, [&] { return !Sim.GetRegion(ESpecimen::Human).bLockedDown; }, 300.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleContractChainTest, "AlienIdle.Sim.ContractChain", SimTestFlags)
bool FAlienIdleContractChainTest::RunTest(const FString& Parameters)
{
	// Accept a contract -> configure a UFO -> find the right specimen -> experiment -> payout.
	FQuietSim Sim;
	Sim.DevDiscover(EDiscovery::TotalRecall); // humans and scanners
	Sim.DevSetActiveContract(MakeContract(ESpecimen::Human, EOccupation::Therapist, 1, 170.0));
	TestEqual(TEXT("No scanner, no target"), Sim.GetScannerTarget(0, ESpecimen::Human), EOccupation::None);
	BuyShip(Sim, 0, EShipSystem::Scanner, 1);
	TestEqual(TEXT("Scanner targets the contract's profession"), Sim.GetScannerTarget(0, ESpecimen::Human), EOccupation::Therapist);
	TestTrue(TEXT("Tradeoff: scanning adds capture time"), Sim.GetCaptureSeconds(0, ESpecimen::Human, true) > Sim.GetCaptureSeconds(0, ESpecimen::Human, false));

	Sim.SetDestination(0, ESpecimen::Human);
	Sim.DevAddEnergy(1000.0);
	Sim.Launch(0);
	RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
	TestEqual(TEXT("Captured a therapist"), Sim.GetUfo(0).Cargo[0].Occupation, EOccupation::Therapist);

	const double Before = Sim.GetResearch();
	Sim.DevAddEnergy(1000.0);
	TestTrue(TEXT("Contract completes"), RunUntil(Sim, [&] { return Sim.GetContractsCompleted() == 1; }));
	TestFalse(TEXT("No active contract after completion"), Sim.HasActiveContract());
	const double Value = Sim.GetRegionDef(Sim.GetHomeRegion(ESpecimen::Human)).Value;
	TestEqual(TEXT("Routine research (times region value) plus contract reward"), Sim.GetResearch() - Before,
		FMath::RoundToDouble(AlienIdleTuning::Specimen(ESpecimen::Human).Research * Value) + 170.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleContractModeTest, "AlienIdle.Sim.ContractModeAutomation", SimTestFlags)
bool FAlienIdleContractModeTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	// Three quick experiments unlock automation.
	for (int32 i = 0; i < AlienIdleTuning::AutomationUnlockExperiments; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
	}
	RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.IsAutomationUnlocked(); });
	TestTrue(TEXT("Automation unlocked"), Sim.IsAutomationUnlocked());

	Sim.DevDiscover(EDiscovery::AcornAmnesia);
	BuyShip(Sim, 0, EShipSystem::Tractor, 1);
	Sim.DevSetActiveContract(MakeContract(ESpecimen::Cow, EOccupation::None, 2, 100.0));
	Sim.SetDestination(0, ESpecimen::Squirrel);
	Sim.SetFollowContract(0, true);
	Sim.SetAutoLaunch(0, true);
	Sim.DevAddEnergy(100.0);
	Sim.Tick(0.1f);
	TestEqual(TEXT("Contract mode retargets the ship to cows"), Sim.GetUfo(0).TripTarget, ESpecimen::Cow);
	TestTrue(TEXT("Contract finished without player input"), RunUntil(Sim, [&] { return Sim.GetContractsCompleted() == 1; }, 300.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleIncidentsTest, "AlienIdle.Sim.IncidentsAreOccasional", SimTestFlags)
bool FAlienIdleIncidentsTest::RunTest(const FString& Parameters)
{
	FAlienIdleSim Sim; // incidents on
	Sim.DevUnlockSpecimen(ESpecimen::Human);
	Sim.DevDiscover(EDiscovery::UnscheduledDepartures);
	BuyBase(Sim, EBaseUpgrade::Containment, 3);
	// A containment full of magicians: the worst case for escapes.
	for (int32 i = 0; i < 6; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Human, EOccupation::Magician });
	}
	int32 MaxActiveSeconds = 0;
	int32 Active = 0;
	for (int32 Second = 0; Second < 30 * 60; ++Second)
	{
		Sim.Tick(1.0f);
		Active = Sim.GetIncident().Type != EIncident::None ? Active + 1 : 0;
		MaxActiveSeconds = FMath::Max(MaxActiveSeconds, Active);
	}
	AddInfo(FString::Printf(TEXT("30 min: %d incidents, %d escapes, longest %d s"), Sim.GetIncidentCount(), Sim.GetEscapes(), MaxActiveSeconds));
	TestTrue(TEXT("Something happened"), Sim.GetIncidentCount() >= 1);
	TestTrue(TEXT("Occasional: at most one every two minutes"), Sim.GetIncidentCount() <= 15);
	TestTrue(TEXT("Every incident clears by itself"), MaxActiveSeconds <= 120);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCrewMinimumsTest, "AlienIdle.Sim.CrewMinimums", SimTestFlags)
bool FAlienIdleCrewMinimumsTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	TestEqual(TEXT("Starts with three aliens"), Sim.GetCrew().Num(), 3);
	TestTrue(TEXT("Pilot staffs Old Reliable"), Sim.IsStationStaffed({ EStation::Ufo, 0 }));
	TestTrue(TEXT("Scientist staffs chamber 1"), Sim.IsStationStaffed({ EStation::Chamber, 0 }));
	TestTrue(TEXT("Technician staffs power"), Sim.IsStationStaffed({ EStation::Power, 0 }));
	TestFalse(TEXT("Containment starts unstaffed"), Sim.IsStationStaffed({ EStation::Containment, 0 }));

	// A new UFO does nothing until someone flies it.
	BuyBase(Sim, EBaseUpgrade::Hangar, 1);
	Sim.DevAddEnergy(1000.0);
	FString Reason;
	TestFalse(TEXT("New UFO has no crew"), Sim.CanLaunch(1, &Reason));
	TestTrue(TEXT("Reason names the crew"), Reason.Contains(TEXT("crew")));

	Sim.DevAddResearch(Sim.GetRecruitCost());
	TestTrue(TEXT("Request Additional Personnel"), Sim.Recruit());
	TestEqual(TEXT("Workforce grows"), Sim.GetCrew().Num(), 4);
	TestTrue(TEXT("Auto-assign staffs the idle UFO first"), Sim.IsStationStaffed({ EStation::Ufo, 1 }));
	TestTrue(TEXT("Second UFO can launch"), Sim.CanLaunch(1));

	// Free reassignment: move the pilot off Old Reliable and it is grounded.
	Sim.SetAutoAssign(false);
	TestTrue(TEXT("Unassign the pilot"), Sim.Assign(0, FAssignment()));
	TestFalse(TEXT("Old Reliable grounded"), Sim.CanLaunch(0));
	TestTrue(TEXT("Reassign the pilot"), Sim.Assign(0, { EStation::Ufo, 0 }));
	TestTrue(TEXT("Old Reliable flies again"), Sim.CanLaunch(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCrewBonusTest, "AlienIdle.Sim.CrewSpecialistsAndTraining", SimTestFlags)
bool FAlienIdleCrewBonusTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.SetAutoAssign(false);
	const FAssignment Ufo{ EStation::Ufo, 0 };
	TestEqual(TEXT("A specialist pilot gives a bonus"), Sim.GetStationBonus(Ufo), AlienIdleTuning::SpecialistBonus);
	const float SpecialistFlight = Sim.GetFlightSeconds(0, ESpecimen::Squirrel);

	// Swap the pilot for the technician: still flies, no specialist bonus.
	Sim.Assign(0, FAssignment());
	Sim.Assign(2, Ufo);
	TestEqual(TEXT("Anyone can fill a basic post"), Sim.GetStationBonus(Ufo), 0.0f);
	TestTrue(TEXT("The specialist was faster"), SpecialistFlight < Sim.GetFlightSeconds(0, ESpecimen::Squirrel));
	Sim.Assign(2, { EStation::Power, 0 });
	Sim.Assign(0, Ufo);

	// Training: off duty for a seminar, then better at the job.
	Sim.DevAddResearch(Sim.GetTrainCost(1));
	TestTrue(TEXT("Send the scientist to a seminar"), Sim.Train(1));
	TestFalse(TEXT("Chamber unstaffed during training"), Sim.IsStationStaffed({ EStation::Chamber, 0 }));
	Sim.DevAddEnergy(1000.0);
	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	Sim.Tick(1.0f);
	TestFalse(TEXT("Lab waits for its scientist"), Sim.GetChambers()[0].bRunning);
	RunUntil(Sim, [&] { return Sim.GetCrew()[1].TrainingRemaining <= 0.0f; }, 60.0f);
	TestEqual(TEXT("Training level 1"), Sim.GetCrew()[1].Training, 1);
	TestEqual(TEXT("Trained specialist bonus"), Sim.GetStationBonus({ EStation::Chamber, 0 }),
		AlienIdleTuning::SpecialistBonus + AlienIdleTuning::TrainingBonusPerLevel);
	TestTrue(TEXT("Lab resumes"), RunUntil(Sim, [&] { return Sim.GetChambers()[0].bRunning; }, 2.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleMemoryWipeTest, "AlienIdle.Sim.MemoryWipeStation", SimTestFlags)
bool FAlienIdleMemoryWipeTest::RunTest(const FString& Parameters)
{
	auto SuspicionAfterOneReturn = [](bool bStaffWipe)
	{
		FQuietSim Sim;
		Sim.SetAutoAssign(false);
		if (bStaffWipe)
		{
			Sim.DevAddResearch(Sim.GetRecruitCost());
			Sim.Recruit();
			Sim.Assign(3, { EStation::MemoryWipe, 0 });
		}
		Sim.DevAddEnergy(1000.0);
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
		RunUntil(Sim, [&] { return Sim.GetTotalExperiments() == 1; });
		return Sim.GetRegion(ESpecimen::Squirrel).Suspicion;
	};
	TestTrue(TEXT("Unwiped returns raise suspicion"), SuspicionAfterOneReturn(false) > 0.0);
	TestEqual(TEXT("Wiped returns do not"), SuspicionAfterOneReturn(true), 0.0);
	return true;
}

namespace
{
	enum class EBotStrategy : uint8
	{
		Balanced,
		BulkAnimals,     // cargo ships, extended studies on animals, animal contracts only, no humans
		SelectiveHumans, // rush to humans, scanner ship, human contracts
		Covert,          // humans, stealth on every ship, calmest region, counter responses, let Earth calm down before submitting
	};

	struct FBotResult
	{
		double Unlock[int32(ESpecimen::Count)] = {};
		double ThirdHumanExperiment = -1.0;
		double ResearchEarned = 0.0;
		int32 Discoveries = 0;
		int32 Contracts = 0;
		int32 Incidents = 0;
		int32 Escapes = 0;
		int32 Ufos = 0;
		int32 Crew = 0;
	};

	/** Optional: records what the bot bought, in order (for opening comparisons). */
	TArray<FString>* GBotPurchases = nullptr;
	/** Optional: the bot never staffs Pressure Control (plays the undersea base like the mountain). */
	bool GBotIgnoresHull = false;

	/** A simple greedy player. Not optimal; a real player should be at least this quick. */
	FBotResult RunBotOn(FAlienIdleSim& Sim, EBotStrategy Strategy, double Minutes, EDepartment Department = EDepartment::None)
	{
		if (Department != EDepartment::None)
		{
			Sim.ChooseDepartment(Department);
		}
		FBotResult Result;
		const double Start = Sim.GetElapsedSeconds();
		auto Note = [](const FString& What) { if (GBotPurchases) { GBotPurchases->Add(What); } return true; };
		auto TryShip = [&](int32 Ufo, EShipSystem System)
		{
			return Sim.CanBuyShipUpgrade(Ufo, System) && Sim.BuyShipUpgrade(Ufo, System) && Note(AlienIdleTuning::ShipSystems[int32(System)].Name);
		};
		auto TryBase = [&](EBaseUpgrade Upgrade)
		{
			return Sim.CanBuyBaseUpgrade(Upgrade) && Sim.BuyBaseUpgrade(Upgrade) && Note(AlienIdleTuning::BaseUpgrades[int32(Upgrade)].Name);
		};
		// Under the sea, tidal turbines are the cheap power (generators cost 2.5x).
		auto TryPower = [&]()
		{
			if (Sim.HasHull() && Sim.GetBaseLevel(EBaseUpgrade::TidalTurbines) <= Sim.GetBaseLevel(EBaseUpgrade::Power) + 1
				&& Sim.GetBaseLevel(EBaseUpgrade::TidalTurbines) < AlienIdleTuning::BaseUpgrades[int32(EBaseUpgrade::TidalTurbines)].MaxLevel)
			{
				return TryBase(EBaseUpgrade::TidalTurbines);
			}
			return TryBase(EBaseUpgrade::Power);
		};
		const bool bHumans = Strategy != EBotStrategy::BulkAnimals;

		while (Sim.GetElapsedSeconds() - Start < Minutes * 60.0)
		{
			Sim.Tick(0.5f);
			for (int32 t = 1; t < int32(ESpecimen::Count); ++t)
			{
				if (Result.Unlock[t] == 0.0 && Sim.IsSpecimenUnlocked(ESpecimen(t)))
				{
					Result.Unlock[t] = Sim.GetElapsedSeconds();
				}
			}
			if (Result.ThirdHumanExperiment < 0.0 && Sim.GetExperimentsCompleted(ESpecimen::Human) >= 3)
			{
				Result.ThirdHumanExperiment = Sim.GetElapsedSeconds();
			}

			// Lab priorities: run any open project the strategy wants; otherwise routine (bulk: extended when energy is plentiful).
			for (int32 t = 0; t < int32(ESpecimen::Count); ++t)
			{
				const ESpecimen Type = ESpecimen(t);
				FExperimentChoice Want;
				for (const FExperimentChoice& Option : Sim.GetExperimentOptions(Type))
				{
					if (Option.Kind != EExperimentKind::Discovery)
					{
						continue;
					}
					const bool bHumanPath = Option.Discovery == EDiscovery::TotalRecall || Option.Discovery == EDiscovery::VoluntaryMeetings || Option.Discovery == EDiscovery::SarcasmOrgan;
					if (bHumans || !bHumanPath)
					{
						Want = Option;
						break;
					}
				}
				if (Want.Kind != EExperimentKind::Discovery && Strategy == EBotStrategy::BulkAnimals && Type != ESpecimen::Squirrel
					&& Sim.GetEnergy() > Sim.GetEnergyCap() * 0.6)
				{
					Want.Kind = EExperimentKind::Extended;
				}
				Sim.SetExperimentChoice(Type, Want);
			}
			// Freezer mode while a thesis asks for preserved samples.
			Sim.SetAgeSamples(Sim.IsPreserving() && AlienIdleTuning::Thesis(Sim.GetThesis()).PreservedSamples > Sim.GetPreservedStudied());
			if (Sim.IsMiddleManagementAvailable())
			{
				Sim.SetMiddleManagement(true);
			}
			if (Sim.GetRival().bRaceOffered)
			{
				Sim.AcceptRace();
			}
			Sim.SetSellOrder(ESpecimen::Squirrel, true);
			Sim.SetSellOrder(ESpecimen::Cow, true);
			if (Strategy == EBotStrategy::Covert)
			{
				for (int32 r = 0; r < Sim.GetNumRegions(); ++r)
				{
					for (int32 k = 0; k < int32(EResponse::Count); ++k)
					{
						if (Sim.IsResponseActive(r, EResponse(k)) && Sim.CanCounter(r, EResponse(k)) && Sim.GetResearch() >= 3.0 * Sim.GetCounterCost(r, EResponse(k)))
						{
							Sim.Counter(r, EResponse(k));
						}
					}
				}
			}

			// Contracts: take the first one that fits the strategy.
			if (!Sim.HasActiveContract())
			{
				for (int32 o = 0; o < Sim.GetContractOffers().Num(); ++o)
				{
					bool bWantsHumans = false;
					for (const FContractNeed& Need : Sim.GetContractOffers()[o].Needs)
					{
						bWantsHumans |= Need.Type == ESpecimen::Human;
					}
					const bool bTake = Strategy == EBotStrategy::Balanced
						|| (Strategy == EBotStrategy::BulkAnimals && !bWantsHumans)
						|| ((Strategy == EBotStrategy::SelectiveHumans || Strategy == EBotStrategy::Covert) && (bWantsHumans || !Sim.IsSpecimenUnlocked(ESpecimen::Human)));
					if (bTake)
					{
						Sim.AcceptContract(o);
						break;
					}
				}
			}

			// Purchases. UFO 0 is the heavy lifter; later UFOs collect (or scan, for the human strategy).
			const ESpecimen NextHeavy = Sim.IsSpecimenUnlocked(Sim.Local(ESpecimen::Elephant)) ? Sim.Local(ESpecimen::Elephant) : Sim.Local(ESpecimen::Cow);
			// Oceanic: keep the hull below creaking with technicians on Pressure Control.
			const bool bHull = Sim.HasHull();
			// Antarctic: keep labs and hangar warm with Boiler Room technicians, then geothermal taps.
			if (Sim.HasHeat() && !GBotIgnoresHull && (Sim.GetLabWarmth() < 0.9f || Sim.GetHangarWarmth() < 0.9f))
			{
				if (Sim.GetStationCrew({ EStation::Boiler, 0 }) < 2)
				{
					int32 Idle = INDEX_NONE;
					for (int32 c = 0; c < Sim.GetCrew().Num() && Idle == INDEX_NONE; ++c)
					{
						Idle = Sim.GetCrew()[c].Assignment.Station == EStation::None ? c : INDEX_NONE;
					}
					if (Idle == INDEX_NONE && Sim.CanRecruit() && Sim.Recruit())
					{
						Note(TEXT("Recruit for the Boiler Room"));
						Idle = Sim.GetCrew().Num() - 1;
					}
					if (Idle != INDEX_NONE)
					{
						Sim.Assign(Idle, { EStation::Boiler, 0 });
					}
				}
				else
				{
					TryBase(EBaseUpgrade::GeothermalTaps);
				}
			}
			const EStation Neglected = Sim.HasHull() ? EStation::Pressure : EStation::Boiler;
			if ((Sim.HasHull() || Sim.HasHeat()) && GBotIgnoresHull)
			{
				for (int32 c = 0; c < Sim.GetCrew().Num(); ++c)
				{
					if (Sim.GetCrew()[c].Assignment.Station == Neglected)
					{
						Sim.Assign(c, FAssignment());
						Sim.SetAutoAssign(false);
					}
				}
			}
			if (bHull && !GBotIgnoresHull && Sim.GetHullPressure() > 45.0f && Sim.GetStationCrew({ EStation::Pressure, 0 }) < 2)
			{
				int32 Idle = INDEX_NONE;
				for (int32 c = 0; c < Sim.GetCrew().Num() && Idle == INDEX_NONE; ++c)
				{
					Idle = Sim.GetCrew()[c].Assignment.Station == EStation::None ? c : INDEX_NONE;
				}
				if (Idle == INDEX_NONE && Sim.CanRecruit() && Sim.Recruit())
				{
					Note(TEXT("Recruit for Pressure Control"));
					Idle = Sim.GetCrew().Num() - 1;
				}
				if (Idle != INDEX_NONE)
				{
					Sim.Assign(Idle, { EStation::Pressure, 0 });
				}
			}
			bool bWaiting = false;
			for (int32 u = 0; u < Sim.GetNumUfos(); ++u)
			{
				bWaiting |= Sim.GetUfo(u).State == EUfoState::WaitingToUnload;
			}
			FString TractorReason;
			Sim.CanBuyShipUpgrade(0, EShipSystem::Tractor, &TractorReason);
			const bool bNeedTractor = Sim.IsSpecimenUnlocked(NextHeavy) && Sim.GetTractorStrength(0) < AlienIdleTuning::Specimen(NextHeavy).Weight
				&& !TractorReason.Contains(TEXT("discovery"));
			bool bUnstaffed = false;
			for (const FAssignment& Station : Sim.GetAllStations())
			{
				bUnstaffed |= !Sim.IsStationStaffed(Station);
			}
			EDeptProject NextProject = EDeptProject::Count;
			for (int32 i = 0; i < int32(EDeptProject::Count); ++i)
			{
				if (AlienIdleTuning::DeptProjects[i].Department == Department && !Sim.IsProjectDone(EDeptProject(i)))
				{
					NextProject = EDeptProject(i);
					break;
				}
			}
			if (Department != EDepartment::None && Sim.CanBeginSignature(Department))
			{
				Sim.BeginSignature(Department);
			}
			// Signature experiments need two crew on power (Incompetence) or a warden and a wipe operator.
			bool bNeedSignatureCrew = false;
			if (Sim.GetPendingSignature() != EDepartment::None)
			{
				bNeedSignatureCrew = Department == EDepartment::Incompetence ? Sim.GetStationCrew({ EStation::Power, 0 }) < 2
					: !Sim.IsStationStaffed({ EStation::Containment, 0 }) || !Sim.IsStationStaffed({ EStation::MemoryWipe, 0 });
			}
			if ((bUnstaffed || bNeedSignatureCrew) && Sim.GetIdleCrew() == 0 && Sim.CanRecruit())
			{
				Sim.Recruit();
				Note(TEXT("Recruit"));
				if (bNeedSignatureCrew && Department == EDepartment::Incompetence)
				{
					Sim.Assign(Sim.GetCrew().Num() - 1, { EStation::Power, 0 });
				}
			}
			else if (NextProject != EDeptProject::Count && Sim.CanBuyProject(NextProject))
			{
				Sim.BuyProject(NextProject);
				Note(AlienIdleTuning::DeptProjects[int32(NextProject)].Name);
			}
			else if (Sim.GetPendingSignature() != EDepartment::None && Sim.GetEnergyCap() < AlienIdleTuning::SignatureEnergy && TryBase(EBaseUpgrade::Power)) {}
			else if (bNeedTractor)
			{
				TryShip(0, EShipSystem::Tractor); // save up for it
			}
			else if (Sim.GetContainment().Num() >= 2 && TryBase(EBaseUpgrade::Chambers)) {}
			else if (Sim.GetEnergy() < 15.0 && TryPower()) {}
			else if (bWaiting && TryBase(EBaseUpgrade::Containment)) {}
			else if (bWaiting && bHull && TryBase(EBaseUpgrade::DeepTanks)) {}
			else if (bWaiting && Sim.HasHeat() && TryBase(EBaseUpgrade::CryoVault)) {}
			else if (TryBase(EBaseUpgrade::Hangar)) {}
			else
			{
				for (int32 u = 1; u < Sim.GetNumUfos(); ++u)
				{
					if (Strategy == EBotStrategy::SelectiveHumans && u == 1 && Sim.IsDiscovered(EDiscovery::TotalRecall))
					{
						// Switching strategy: strip a bulk collector so it can carry a scanner.
						FString ScanReason;
						if (Sim.GetShipLevel(u, EShipSystem::Scanner) == 0 && !Sim.CanBuyShipUpgrade(u, EShipSystem::Scanner, &ScanReason)
							&& ScanReason.Contains(TEXT("slots")) && Sim.GetUfo(u).State == EUfoState::Docked)
						{
							Sim.RefitShip(u);
						}
						TryShip(u, EShipSystem::Scanner);
						TryShip(u, EShipSystem::Stealth);
					}
					else
					{
						if (Strategy == EBotStrategy::BulkAnimals && Sim.GetShipLevel(u, EShipSystem::Tractor) < 1)
						{
							TryShip(u, EShipSystem::Tractor);
						}
						TryShip(u, EShipSystem::Cargo);
					}
				}
				if (Strategy == EBotStrategy::Covert && Sim.GetEnergyCap() < 150.0)
				{
					TryPower();
				}
				if (Strategy == EBotStrategy::Covert)
				{
					for (int32 u = 0; u < Sim.GetNumUfos(); ++u)
					{
						if (Sim.GetShipLevel(u, EShipSystem::Stealth) < 2)
						{
							TryShip(u, EShipSystem::Stealth);
						}
					}
				}
				if ((Strategy == EBotStrategy::SelectiveHumans || Strategy == EBotStrategy::Covert) && Sim.GetNumUfos() == 1 && Sim.IsSpecimenUnlocked(ESpecimen::Human))
				{
					TryShip(0, EShipSystem::Scanner);
				}
				if (Sim.GetResearch() > 150.0)
				{
					TryPower();
				}
				if (Sim.GetShipLevel(0, EShipSystem::Thrusters) < 1)
				{
					TryShip(0, EShipSystem::Thrusters);
				}
			}

			// Launch everything at the best target it can lift and reach; first fetch what a queued signature experiment lacks.
			const TArray<FSpecimen> SignatureMissing = Sim.GetSignatureMissingSpecimens();
			bool bOnlyCalmLeft = Sim.GetActiveResponseCount() > 0;
			for (const FThesisObjective& Objective : Sim.GetThesisObjectives())
			{
				bOnlyCalmLeft &= Objective.bDone || Objective.Text.StartsWith(TEXT("Earth calm"));
			}
			for (int32 u = 0; u < Sim.GetNumUfos(); ++u)
			{
				if (Sim.GetUfo(u).State != EUfoState::Docked || (Strategy == EBotStrategy::Covert && bOnlyCalmLeft))
				{
					continue;
				}
				bool bSent = false;
				for (const FSpecimen& Want : SignatureMissing)
				{
					Sim.SetDestinationBest(u, Want.Type);
					if (Sim.CanLaunch(u) && (Want.Occupation == EOccupation::None || Sim.GetScannerTarget(u, Want.Type) == Want.Occupation || Sim.GetShipLevel(u, EShipSystem::Scanner) == 0))
					{
						Sim.Launch(u);
						bSent = true;
						break;
					}
				}
				if (bSent)
				{
					continue;
				}
				// Two passes: first anything not being swept by the Bureau, then whatever can go.
				for (int32 Pass = 0; Pass < 2 && Sim.GetUfo(u).State == EUfoState::Docked; ++Pass)
				{
					for (int32 t = int32(ESpecimen::Count) - 1; t >= 0; --t)
					{
						if (!bHumans && ESpecimen(t) == ESpecimen::Human)
						{
							continue;
						}
						Sim.SetDestinationBest(u, ESpecimen(t));
						if (Strategy == EBotStrategy::Covert)
						{
							// Rotate: the region with this target that has the most room before an alert, skipping any with active responses.
							int32 Best = INDEX_NONE;
							double BestRoom = 0.0;
							for (int32 r = 0; r < Sim.GetNumRegions(); ++r)
							{
								bool bResponding = false;
								for (int32 k = 0; k < int32(EResponse::Count); ++k)
								{
									bResponding |= Sim.IsResponseActive(r, EResponse(k));
								}
								const double Room = Sim.GetAlertSuspicion(r) - Sim.GetRegionState(r).Suspicion;
								if (Sim.RegionHasSpecimen(r, ESpecimen(t)) && !bResponding && Room > BestRoom)
								{
									Best = r;
									BestRoom = Room;
								}
							}
							if (Best == INDEX_NONE || BestRoom < Sim.GetAlertSuspicion(Best) * 0.4)
							{
								continue; // everywhere is warm: wait
							}
							Sim.SetDestinationRegion(u, Best, ESpecimen(t));
						}
						if (Pass == 0 && Sim.IsSwept(Sim.GetUfo(u).Region, ESpecimen(t)))
						{
							continue;
						}
						if (Sim.CanLaunch(u))
						{
							Sim.Launch(u);
							break;
						}
					}
				}
			}
		}

		Result.ResearchEarned = Sim.GetTotalResearchEarned();
		Result.Discoveries = Sim.GetDiscoveryCount();
		Result.Contracts = Sim.GetContractsCompleted();
		Result.Incidents = Sim.GetIncidentCount();
		Result.Escapes = Sim.GetEscapes();
		Result.Ufos = Sim.GetNumUfos();
		Result.Crew = Sim.GetCrew().Num();
		return Result;
	}

	FBotResult RunBot(EBotStrategy Strategy, double Minutes, EDepartment Department = EDepartment::None)
	{
		FAlienIdleSim Sim;
		return RunBotOn(Sim, Strategy, Minutes, Department);
	}

	FString Describe(const FBotResult& R)
	{
		return FString::Printf(TEXT("cows %.1f min, elephants %.1f, humans %.1f, 3rd human exp %.1f; research earned %.0f, discoveries %d, contracts %d, incidents %d (escapes %d), UFOs %d, crew %d"),
			R.Unlock[1] / 60.0, R.Unlock[2] / 60.0, R.Unlock[3] / 60.0, R.ThirdHumanExperiment / 60.0, R.ResearchEarned, R.Discoveries, R.Contracts, R.Incidents, R.Escapes, R.Ufos, R.Crew);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdlePacingBotTest, "AlienIdle.Sim.PacingBot", SimTestFlags)
bool FAlienIdlePacingBotTest::RunTest(const FString& Parameters)
{
	const FBotResult Result = RunBot(EBotStrategy::Balanced, 30.0);
	AddInfo(TEXT("Balanced: ") + Describe(Result));
	TestTrue(TEXT("Reaches cows"), Result.Unlock[1] > 0.0);
	TestTrue(TEXT("Reaches elephants"), Result.Unlock[2] > 0.0);
	TestTrue(TEXT("Reaches humans"), Result.Unlock[3] > 0.0);
	TestTrue(TEXT("Three human experiments within 30 minutes"), Result.ThirdHumanExperiment > 0.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleStrategyComparisonTest, "AlienIdle.Sim.StrategyComparison", SimTestFlags)
bool FAlienIdleStrategyComparisonTest::RunTest(const FString& Parameters)
{
	const FBotResult Bulk = RunBot(EBotStrategy::BulkAnimals, 25.0);
	const FBotResult Humans = RunBot(EBotStrategy::SelectiveHumans, 25.0);
	AddInfo(TEXT("Bulk animals:     ") + Describe(Bulk));
	AddInfo(TEXT("Selective humans: ") + Describe(Humans));
	const double Ratio = Humans.ResearchEarned / FMath::Max(1.0, Bulk.ResearchEarned);
	AddInfo(FString::Printf(TEXT("Research ratio humans/bulk after 25 min: %.2f"), Ratio));
	TestTrue(TEXT("Both strategies are viable (within 2x of each other)"), Ratio > 0.5 && Ratio < 2.0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleContainmentHoldTest, "AlienIdle.Sim.PauseCollectionWhenFull", SimTestFlags)
bool FAlienIdleContainmentHoldTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	for (int32 i = 0; i < AlienIdleTuning::AutomationUnlockExperiments; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
	}
	RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.IsAutomationUnlocked(); });

	// Take the scientist out of the lab so nothing drains containment, then let the UFO repeat.
	Sim.SetAutoAssign(false);
	Sim.Assign(1, FAssignment());
	Sim.SetAutoLaunch(0, true);
	bool bSawWaiting = false;
	RunUntil(Sim, [&]
	{
		Sim.DevAddEnergy(100.0);
		bSawWaiting |= Sim.GetUfo(0).State == EUfoState::WaitingToUnload;
		return false;
	}, 300.0f);
	TestEqual(TEXT("Containment filled"), Sim.GetContainmentUnitsUsed(), Sim.GetContainmentCapacity());
	TestFalse(TEXT("The UFO never queued at a full dock"), bSawWaiting);
	TestTrue(TEXT("The UFO is holding"), Sim.IsHeldForContainment(0));
	TestTrue(TEXT("Status explains the hold"), Sim.GetUfoStatusText(0).Contains(TEXT("collection paused")));

	Sim.SetPauseWhenFull(false);
	TestFalse(TEXT("Hold can be switched off"), Sim.IsHeldForContainment(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleBottleneckTest, "AlienIdle.Sim.BottleneckDiagnosis", SimTestFlags)
bool FAlienIdleBottleneckTest::RunTest(const FString& Parameters)
{
	{
		FQuietSim Sim; // labs idle, nothing launched: collection-limited
		Sim.Tick(60.0f);
		TestEqual(TEXT("Idle lab means collection is the bottleneck"), Sim.GetCurrentBottleneck(), EBottleneck::Collection);
		AddInfo(TEXT("Collection: ") + Sim.GetRecommendation());
		TestTrue(TEXT("Recommendation names cargo"), Sim.GetRecommendation().Contains(TEXT("Cargo Bay")));
	}
	{
		FQuietSim Sim; // a queue in containment with one busy chamber: lab capacity
		for (int32 i = 0; i < 4; ++i)
		{
			Sim.DevAddToContainment({ ESpecimen::Squirrel });
		}
		RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return false; }, 25.0f);
		TestEqual(TEXT("Queue behind a busy lab"), Sim.GetCurrentBottleneck(), EBottleneck::LabCapacity);
		AddInfo(TEXT("Lab: ") + Sim.GetRecommendation());
		TestTrue(TEXT("Recommendation names a chamber"), Sim.GetRecommendation().Contains(TEXT("Experiment Chamber")));
	}
	{
		FQuietSim Sim; // a new UFO nobody flies: crew
		Sim.SetAutoAssign(false);
		BuyBase(Sim, EBaseUpgrade::Hangar, 1);
		Sim.Launch(0);
		Sim.Tick(60.0f);
		TestEqual(TEXT("Unstaffed UFO is the bottleneck"), Sim.GetCurrentBottleneck(), EBottleneck::Crew);
		TestTrue(TEXT("Text names the post"), Sim.GetBottleneckText().Contains(TEXT("Second Opinion")));
		TestTrue(TEXT("Recommendation is to hire"), Sim.GetRecommendation().Contains(TEXT("Request Additional Personnel")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleStaffingIncidentTest, "AlienIdle.Sim.StaffingIncidents", SimTestFlags)
bool FAlienIdleStaffingIncidentTest::RunTest(const FString& Parameters)
{
	// Over a long run both staffing incidents occur, clear themselves, and can be resolved early.
	FAlienIdleSim Sim;
	bool bMeeting = false;
	bool bAdopted = false;
	bool bAdopterOffDuty = false;
	for (int32 Second = 0; Second < 3 * 60 * 60 && !(bMeeting && bAdopted); ++Second)
	{
		Sim.Tick(1.0f);
		const FActiveIncident& Incident = Sim.GetIncident();
		if (Incident.Type == EIncident::StaffMeeting)
		{
			bMeeting = true;
			TestEqual(TEXT("Meeting pauses the labs"), Sim.GetLabSpeed(), 0.0f);
		}
		if (Incident.Type == EIncident::AdoptedSquirrel && !bAdopted)
		{
			bAdopted = true;
			const FAssignment Post = Sim.GetCrew()[Incident.Crew].Assignment;
			const int32 Before = Sim.GetStationCrew(Post);
			Sim.DevAddResearch(AlienIdleTuning::IncidentResolveCost);
			TestTrue(TEXT("Intervene"), Sim.ResolveIncident());
			bAdopterOffDuty = Sim.GetStationCrew(Post) == Before + 1;
		}
	}
	TestTrue(TEXT("A mandatory meeting happened"), bMeeting);
	TestTrue(TEXT("A squirrel was adopted"), bAdopted);
	TestTrue(TEXT("The adopter was off duty until the intervention"), bAdopterOffDuty);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleUnattendedTest, "AlienIdle.Sim.UnattendedThirtyMinutes", SimTestFlags)
bool FAlienIdleUnattendedTest::RunTest(const FString& Parameters)
{
	// Configure the base like a player would, then walk away for 30 minutes.
	FAlienIdleSim Sim;
	RunBotOn(Sim, EBotStrategy::Balanced, 15.0);
	for (int32 u = 0; u < Sim.GetNumUfos(); ++u)
	{
		Sim.SetAutoLaunch(u, true);
		Sim.SetFollowContract(u, true);
	}
	const double ResearchBefore = Sim.GetResearch();
	Sim.BeginAwayReport();
	for (int32 Second = 0; Second < 30 * 60; ++Second)
	{
		Sim.Tick(1.0f);
	}
	const FAwayReport Report = Sim.BuildAwayReport();
	FString Lost;
	for (int32 i = 1; i < int32(EBottleneck::Count); ++i)
	{
		Lost += FString::Printf(TEXT(" %s %.1f;"), FAlienIdleSim::GetBottleneckName(EBottleneck(i)), Report.LostSeconds[i] / 60.0f);
	}
	AddInfo(FString::Printf(TEXT("Away 30 min: captures %d, experiments %d, research %.0f, contracts %d, discoveries %d, incidents %d (escapes %d). Lost station-minutes:%s"),
		Report.Captures, Report.Experiments, Report.ResearchEarned, Report.Contracts, Report.Discoveries.Num(), Report.Incidents, Report.Escapes, *Lost));
	AddInfo(TEXT("Recommendation: ") + Report.Recommendation);
	TestEqual(TEXT("Report covers 30 minutes"), int32(FMath::RoundToInt(Report.Seconds)), int32(30 * 60));
	TestTrue(TEXT("Specimens kept coming"), Report.Captures > 50);
	TestTrue(TEXT("Labs kept working"), Report.Experiments > 50);
	TestTrue(TEXT("Useful progress: research banked"), Sim.GetResearch() > ResearchBefore + 2000.0);
	TestTrue(TEXT("A clear next upgrade"), Report.Recommendation.StartsWith(TEXT("Next:")));
	TestTrue(TEXT("Ships did not sit at a full dock for long"), Report.LostSeconds[int32(EBottleneck::Containment)] < 0.5f * 30 * 60 * Sim.GetNumUfos());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleStrategySwitchTest, "AlienIdle.Sim.StrategySwitch", SimTestFlags)
bool FAlienIdleStrategySwitchTest::RunTest(const FString& Parameters)
{
	// Run a bulk animal operation, then pivot the same base to selective human studies.
	FAlienIdleSim Sim;
	const FBotResult Bulk = RunBotOn(Sim, EBotStrategy::BulkAnimals, 15.0);
	TestEqual(TEXT("Bulk phase studied no humans"), Sim.GetExperimentsCompleted(ESpecimen::Human), 0);
	const FBotResult Pivot = RunBotOn(Sim, EBotStrategy::SelectiveHumans, 15.0);
	AddInfo(TEXT("Bulk phase:  ") + Describe(Bulk));
	AddInfo(TEXT("After pivot: ") + Describe(Pivot));
	AddInfo(FString::Printf(TEXT("Human experiments after pivot: %d, scanner on UFO 2: %d"),
		Sim.GetExperimentsCompleted(ESpecimen::Human), Sim.GetNumUfos() > 1 ? Sim.GetShipLevel(1, EShipSystem::Scanner) : 0));
	TestTrue(TEXT("Humans unlocked after the pivot"), Sim.IsSpecimenUnlocked(ESpecimen::Human));
	TestTrue(TEXT("Human studies under way"), Sim.GetExperimentsCompleted(ESpecimen::Human) >= 3);
	TestTrue(TEXT("A ship was refitted for scanning"), Sim.GetNumUfos() > 1 && Sim.GetShipLevel(1, EShipSystem::Scanner) > 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleSaveRoundTripTest, "AlienIdle.Sim.SaveRoundTrip", SimTestFlags)
bool FAlienIdleSaveRoundTripTest::RunTest(const FString& Parameters)
{
	// A busy mid-game base: save it, load it into a second sim, and both must continue identically.
	FAlienIdleSim Live;
	RunBotOn(Live, EBotStrategy::Balanced, 12.0);
	TArray<uint8> Bytes;
	Live.SaveToBytes(Bytes);
	FAlienIdleSim Loaded;
	TestTrue(TEXT("Load succeeds"), Loaded.LoadFromBytes(Bytes));
	TestEqual(TEXT("Same research"), Loaded.GetResearch(), Live.GetResearch());
	TestEqual(TEXT("Same crew"), Loaded.GetCrew().Num(), Live.GetCrew().Num());
	TestEqual(TEXT("Same fleet"), Loaded.GetNumUfos(), Live.GetNumUfos());

	Live.CatchUp(600.0);
	Loaded.CatchUp(600.0);
	TestEqual(TEXT("Same elapsed time"), Loaded.GetElapsedSeconds(), Live.GetElapsedSeconds());
	TestEqual(TEXT("Same research 10 minutes later"), Loaded.GetResearch(), Live.GetResearch());
	TestEqual(TEXT("Same captures"), Loaded.GetCaptures(), Live.GetCaptures());
	TestEqual(TEXT("Same experiments"), Loaded.GetTotalExperiments(), Live.GetTotalExperiments());
	TestEqual(TEXT("Same incidents"), Loaded.GetIncidentCount(), Live.GetIncidentCount());
	TestEqual(TEXT("Same last log line"), Loaded.GetLog().Last().Text, Live.GetLog().Last().Text);

	TArray<uint8> Garbage = { 1, 2, 3, 4 };
	TestFalse(TEXT("Rejects a bad file"), Loaded.LoadFromBytes(Garbage));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleSaveFileCatchUpTest, "AlienIdle.Sim.SaveFileAndCatchUp", SimTestFlags)
bool FAlienIdleSaveFileCatchUpTest::RunTest(const FString& Parameters)
{
	// The idle flow: save at the 5-minute mark, keep playing live, then reload and recalculate the gap.
	FAlienIdleSim Live;
	RunBotOn(Live, EBotStrategy::Balanced, 10.0);
	for (int32 u = 0; u < Live.GetNumUfos(); ++u)
	{
		Live.SetAutoLaunch(u, true);
	}
	const FString Path = FPaths::ProjectSavedDir() / TEXT("Automation") / TEXT("SaveFileAndCatchUp.sav");
	TestTrue(TEXT("Save to disk"), Live.SaveToFile(Path));
	Live.CatchUp(20.0 * 60.0);

	FAlienIdleSim Reloaded;
	double SinceSave = -1.0;
	TestTrue(TEXT("Load from disk"), Reloaded.LoadFromFile(Path, SinceSave));
	TestTrue(TEXT("Wall-clock time since save is known"), SinceSave >= 0.0 && SinceSave < 600.0);
	TestEqual(TEXT("Recalculates the 20 minutes"), Reloaded.CatchUp(20.0 * 60.0), 20.0 * 60.0);
	TestEqual(TEXT("Recalculated research matches the live run"), Reloaded.GetResearch(), Live.GetResearch());
	TestEqual(TEXT("Recalculated experiments match"), Reloaded.GetTotalExperiments(), Live.GetTotalExperiments());
	TestEqual(TEXT("Offline catch-up is capped"), Reloaded.CatchUp(1.0e9), AlienIdleTuning::OfflineCapSeconds);
	IFileManager::Get().Delete(*Path);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleDepartmentChoiceTest, "AlienIdle.Sim.DepartmentChoice", SimTestFlags)
bool FAlienIdleDepartmentChoiceTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	FString Reason;
	TestFalse(TEXT("Projects need a department"), Sim.CanBuyProject(EDeptProject::ReinforcedHabitats, &Reason));
	TestTrue(TEXT("Choose Biological Studies"), Sim.ChooseDepartment(EDepartment::Biological));
	TestFalse(TEXT("Only one primary department"), Sim.ChooseDepartment(EDepartment::Human));
	TestEqual(TEXT("Primary price"), Sim.GetProjectCost(EDeptProject::ReinforcedHabitats), 120.0);
	TestEqual(TEXT("Other departments cost 2.5x"), Sim.GetProjectCost(EDeptProject::AdvancedScanners), 300.0);
	TestFalse(TEXT("Projects go in order"), Sim.CanBuyProject(EDeptProject::HeavyTractorArray, &Reason));

	const int32 Capacity = Sim.GetContainmentCapacity();
	const int32 Strength = Sim.GetTractorStrength(0);
	Sim.DevAddResearch(120.0 + 350.0);
	TestTrue(TEXT("Reinforced Habitats"), Sim.BuyProject(EDeptProject::ReinforcedHabitats));
	TestEqual(TEXT("+4 containment"), Sim.GetContainmentCapacity(), Capacity + 4);
	TestTrue(TEXT("Heavy Tractor Array"), Sim.BuyProject(EDeptProject::HeavyTractorArray));
	TestEqual(TEXT("+3 tractor strength"), Sim.GetTractorStrength(0), Strength + 3);

	// Not trapped: another department's project is still available, at the higher price.
	Sim.DevAddResearch(300.0);
	TestTrue(TEXT("Cross-department project"), Sim.BuyProject(EDeptProject::ExperimentalModules));
	TestEqual(TEXT("+2 ship slots"), Sim.GetShipSlots(), AlienIdleTuning::ShipSlots + 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleBatchTest, "AlienIdle.Sim.BatchExperiments", SimTestFlags)
bool FAlienIdleBatchTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.ChooseDepartment(EDepartment::Biological);
	Sim.DevAddResearch(120.0 + 350.0 + 800.0);
	Sim.BuyProject(EDeptProject::ReinforcedHabitats);
	Sim.BuyProject(EDeptProject::HeavyTractorArray);
	TestTrue(TEXT("Batch Experiments"), Sim.BuyProject(EDeptProject::BatchExperiments));
	const double Before = Sim.GetResearch();
	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	Sim.DevAddEnergy(100.0);
	Sim.Tick(0.1f);
	TestEqual(TEXT("Two squirrels in one chamber"), Sim.GetChambers()[0].Batch, 2);
	TestEqual(TEXT("Containment emptied"), Sim.GetContainment().Num(), 0);
	RunUntil(Sim, [&] { return Sim.GetTotalExperiments() >= 2; });
	TestEqual(TEXT("Double research"), Sim.GetResearch() - Before, 2.0 * AlienIdleTuning::Specimen(ESpecimen::Squirrel).Research);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleSignatureTest, "AlienIdle.Sim.SignatureExperiment", SimTestFlags)
bool FAlienIdleSignatureTest::RunTest(const FString& Parameters)
{
	// Human Behavior's "Who Is Responsible?": an accountant, a politician and a squirrel, plus warden and wipe operator.
	FQuietSim Sim;
	Sim.DevDiscover(EDiscovery::TotalRecall);
	Sim.ChooseDepartment(EDepartment::Human);
	BuyBase(Sim, EBaseUpgrade::Power, 1); // the experiment needs 60 energy
	TestTrue(TEXT("Queue the signature experiment"), Sim.BeginSignature(EDepartment::Human));
	TestTrue(TEXT("Status names what is missing"), Sim.GetSignatureStatusText(EDepartment::Human).Contains(TEXT("politician")));

	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	Sim.DevAddToContainment({ ESpecimen::Human, EOccupation::Accountant });
	Sim.DevAddToContainment({ ESpecimen::Human, EOccupation::Politician });
	Sim.DevAddEnergy(1000.0);
	Sim.Tick(1.0f);
	TestFalse(TEXT("Waits for the warden and wipe operator"), Sim.IsSignatureRunning(EDepartment::Human));
	TestEqual(TEXT("Reserved specimens are not used for routine studies"), Sim.GetContainment().Num(), 3);

	for (int32 i = 0; i < 2; ++i)
	{
		Sim.DevAddResearch(Sim.GetRecruitCost());
		Sim.Recruit();
	}
	TestTrue(TEXT("Facilities staffed by auto-assign"), Sim.IsStationStaffed({ EStation::Containment, 0 }) && Sim.IsStationStaffed({ EStation::MemoryWipe, 0 }));
	Sim.DevAddEnergy(1000.0);
	Sim.Tick(0.2f);
	TestTrue(TEXT("Signature experiment running"), Sim.IsSignatureRunning(EDepartment::Human));
	const double Before = Sim.GetResearch();
	TestTrue(TEXT("Signature completes"), RunUntil(Sim, [&] { return Sim.IsSignatureDone(EDepartment::Human); }, 200.0f));
	TestTrue(TEXT("Big research payout"), Sim.GetResearch() - Before >= AlienIdleTuning::SignatureResearch);
	TestTrue(TEXT("The squirrel accepted accountability"), Sim.GetResults().Last().Text.Contains(TEXT("accept accountability")));
	Sim.DevAddToContainment({ ESpecimen::Human, EOccupation::Therapist });
	const double BeforeHuman = Sim.GetResearch();
	Sim.DevAddEnergy(1000.0);
	RunUntil(Sim, [&] { return Sim.GetResearch() > BeforeHuman; }, 60.0f);
	TestEqual(TEXT("Human research +25% after the signature"), Sim.GetResearch() - BeforeHuman,
		FMath::RoundToDouble(AlienIdleTuning::Specimen(ESpecimen::Human).Research * AlienIdleTuning::SignatureResearchBonus));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleDepartmentRunsTest, "AlienIdle.Sim.DepartmentRunsDiffer", SimTestFlags)
bool FAlienIdleDepartmentRunsTest::RunTest(const FString& Parameters)
{
	// Two separate test runs, two departments: they should build different operations and each unlock something.
	auto Run = [this](EBotStrategy Strategy, EDepartment Dept, const TCHAR* Label)
	{
		FAlienIdleSim Sim;
		const FBotResult Result = RunBotOn(Sim, Strategy, 40.0, Dept);
		const int32 Animals = Sim.GetExperimentsCompleted(ESpecimen::Squirrel) + Sim.GetExperimentsCompleted(ESpecimen::Cow) + Sim.GetExperimentsCompleted(ESpecimen::Elephant);
		const int32 Humans = Sim.GetExperimentsCompleted(ESpecimen::Human);
		AddInfo(FString::Printf(TEXT("%s: projects %d/3, signature %s, animal experiments %d, human experiments %d, department contracts in board: %s; %s"),
			Label, Sim.GetProjectsDone(Dept), Sim.IsSignatureDone(Dept) ? TEXT("done") : *Sim.GetSignatureStatusText(Dept), Animals, Humans,
			Sim.GetContractOffers().ContainsByPredicate([Dept](const FContract& C) { return C.Department == Dept; }) ? TEXT("yes") : TEXT("no"), *Describe(Result)));
		TestTrue(FString::Printf(TEXT("%s: bought department projects"), Label), Sim.GetProjectsDone(Dept) >= 2);
		TestTrue(FString::Printf(TEXT("%s: signature experiment done"), Label), Sim.IsSignatureDone(Dept));
		return float(Humans) / float(FMath::Max(1, Animals + Humans));
	};
	const float BioHumanShare = Run(EBotStrategy::BulkAnimals, EDepartment::Biological, TEXT("Biological Studies"));
	const float HumanHumanShare = Run(EBotStrategy::SelectiveHumans, EDepartment::Human, TEXT("Human Behavior"));
	TestTrue(TEXT("The operations look different"), HumanHumanShare > BioHumanShare + 0.2f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdlePromotionTest, "AlienIdle.Sim.PromotionResetsAndKeeps", SimTestFlags)
bool FAlienIdlePromotionTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	FString Reason;
	TestFalse(TEXT("Thesis not complete at start"), Sim.CanSubmitThesis(&Reason));
	TestTrue(TEXT("Reason names an objective"), Reason.Contains(TEXT("signature")));

	Sim.Launch(0);
	RunUntil(Sim, [&] { return Sim.GetTotalExperiments() >= 1; });
	Sim.DevCompleteThesis();
	TestTrue(TEXT("Thesis complete"), Sim.CanSubmitThesis());
	const int32 Entries = Sim.GetEncyclopedia().Num();
	const FPromotionReview Review = Sim.BuildPromotionReview();
	AddInfo(FString::Printf(TEXT("Grants %d: %s"), Review.Grants, *FString::Join(Review.GrantLines, TEXT(" | "))));
	TestTrue(TEXT("Thesis grants plus discoveries"), Review.Grants >= AlienIdleTuning::ThesisGrants + 4);
	TestTrue(TEXT("Signature bonus can be kept"), Review.Options.ContainsByPredicate([](const FPermanentOption& O) { return O.bSignature; }));
	TestTrue(TEXT("Consequences listed"), Review.Resets.Num() > 0 && Review.Keeps.Num() > 0);
	TestFalse(TEXT("Review text"), Review.Review.IsEmpty());

	FNextAssignment Next;
	Next.Thesis = EThesis::CorporateWantsResults;
	Next.Location = ELocation::Metropolis;
	Next.PermanentOption = Review.Options.IndexOfByPredicate([](const FPermanentOption& O) { return O.bSignature; });
	Next.Buy = { EGrantUpgrade::BetterStartingUfo, EGrantUpgrade::SpecialistRecruit };
	Next.StartingSpecialty = ESpecialty::Scientist;
	TestTrue(TEXT("Promote"), Sim.Promote(Next, &Reason));

	// Resets
	TestEqual(TEXT("Run 2"), Sim.GetCareer().Run, 2);
	TestEqual(TEXT("Research reset"), Sim.GetResearch(), 0.0);
	TestEqual(TEXT("One UFO again"), Sim.GetNumUfos(), 1);
	TestEqual(TEXT("No discoveries yet"), Sim.GetDiscoveryCount(), 0);
	TestEqual(TEXT("No department yet"), Sim.GetPrimaryDepartment(), EDepartment::None);
	// Keeps
	TestEqual(TEXT("Encyclopedia kept"), Sim.GetEncyclopedia().Num(), Entries);
	TestEqual(TEXT("Grants kept minus upgrades"), Sim.GetCareer().Grants, Review.Grants - 4);
	TestTrue(TEXT("Permanent signature bonus"), Sim.HasSignatureBonus(EDepartment::Biological) && !Sim.IsSignatureDone(EDepartment::Biological));
	TestTrue(TEXT("Metropolis"), Sim.GetLocation() == ELocation::Metropolis && Sim.GetRegionName(ESpecimen::Human) == TEXT("Downtown"));
	// Grant upgrades
	TestEqual(TEXT("Better starting UFO"), Sim.GetShipLevel(0, EShipSystem::Cargo), 1);
	TestEqual(TEXT("Specialist recruit"), Sim.GetCrew().Num(), 4);
	TestEqual(TEXT("Recruit is a trained scientist"), Sim.GetCrew().Last().Specialty, ESpecialty::Scientist);
	// Thesis rule: two UFOs at most
	BuyBase(Sim, EBaseUpgrade::Hangar, 1);
	Sim.DevDiscover(EDiscovery::PachydermLogistics);
	Sim.DevAddResearch(100000.0);
	TestFalse(TEXT("Corporate Wants Results caps the hangar"), Sim.CanBuyBaseUpgrade(EBaseUpgrade::Hangar, &Reason));
	TestTrue(TEXT("Reason names the rule"), Reason.Contains(TEXT("thesis rule")));
	// Published discoveries are faster the second time
	Sim.DevAddExperiments(ESpecimen::Squirrel, AlienIdleTuning::Discovery(EDiscovery::AcornAmnesia).RequiredExperiments);
	Sim.SetExperimentChoice(ESpecimen::Squirrel, { EExperimentKind::Discovery, EDiscovery::AcornAmnesia });
	Sim.DevAddToContainment({ ESpecimen::Squirrel });
	Sim.DevAddEnergy(1000.0);
	Sim.Tick(0.1f);
	const FLabChamber* Running = Sim.GetChambers().FindByPredicate([](const FLabChamber& C) { return C.bRunning; });
	TestTrue(TEXT("Known discovery takes half the time"), Running && FMath::IsNearlyEqual(Running->Duration, AlienIdleTuning::Discovery(EDiscovery::AcornAmnesia).Seconds * 0.5f));

	// The save carries the career.
	TArray<uint8> Bytes;
	Sim.SaveToBytes(Bytes);
	FAlienIdleSim Loaded;
	TestTrue(TEXT("Career survives a save"), Loaded.LoadFromBytes(Bytes) && Loaded.GetCareer().Run == 2 && Loaded.GetLocation() == ELocation::Metropolis);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdlePrestigeCycleTest, "AlienIdle.Sim.PrestigeCycle", SimTestFlags)
bool FAlienIdlePrestigeCycleTest::RunTest(const FString& Parameters)
{
	// Run 1: a Biological base on the mountain, played until the thesis can be submitted.
	FAlienIdleSim Sim;
	FBotResult Run1;
	double Run1Minutes = 0.0;
	while (!Sim.CanSubmitThesis() && Run1Minutes < 60.0)
	{
		const FBotResult Chunk = RunBotOn(Sim, EBotStrategy::BulkAnimals, 5.0, EDepartment::Biological);
		for (int32 t = 1; t < int32(ESpecimen::Count); ++t)
		{
			Run1.Unlock[t] = Run1.Unlock[t] > 0.0 ? Run1.Unlock[t] : Chunk.Unlock[t];
		}
		Run1Minutes += 5.0;
	}
	const int32 Run1Ufos = Sim.GetNumUfos();
	const int32 Run1Humans = Sim.GetExperimentsCompleted(ESpecimen::Human);
	const int32 Run1Total = FMath::Max(1, Sim.GetTotalExperiments());
	FString Reason;
	TestTrue(FString::Printf(TEXT("Run 1 thesis complete within an hour (%.0f min)"), Run1Minutes), Sim.CanSubmitThesis(&Reason));
	if (!Sim.CanSubmitThesis())
	{
		AddInfo(TEXT("Run 1 blocked: ") + Reason);
		return true;
	}
	const FPromotionReview Review = Sim.BuildPromotionReview();
	AddInfo(FString::Printf(TEXT("Run 1 (%.0f min, %d UFOs): %s"), Run1Minutes, Run1Ufos, *FString::Join(Review.Stats, TEXT(" | "))));
	AddInfo(FString::Printf(TEXT("Grants %d: %s. Review: %s"), Review.Grants, *FString::Join(Review.GrantLines, TEXT(" | ")), *Review.Review));

	// Promotion: keep the Heavy Tractor Array, buy the better UFO and a veteran scientist, go to Metropolis.
	FNextAssignment Next;
	Next.Thesis = EThesis::CorporateWantsResults;
	Next.Location = ELocation::Metropolis;
	Next.PermanentOption = Review.Options.IndexOfByPredicate([](const FPermanentOption& O) { return !O.bSignature && O.Project == EDeptProject::HeavyTractorArray; });
	Next.PermanentOption = Next.PermanentOption == INDEX_NONE ? 0 : Next.PermanentOption;
	Next.Buy = { EGrantUpgrade::BetterStartingUfo, EGrantUpgrade::SpecialistRecruit };
	Next.StartingSpecialty = ESpecialty::Scientist;
	TestTrue(TEXT("Promote"), Sim.Promote(Next, &Reason));

	// Run 2: Human Behavior in the city, under "Corporate Wants Results".
	const FBotResult Run2 = RunBotOn(Sim, EBotStrategy::SelectiveHumans, 30.0, EDepartment::Human);
	const int32 Run2Humans = Sim.GetExperimentsCompleted(ESpecimen::Human);
	const int32 Run2Total = FMath::Max(1, Sim.GetTotalExperiments());
	TArray<FString> Objectives;
	for (const FThesisObjective& Objective : Sim.GetThesisObjectives())
	{
		Objectives.Add(FString::Printf(TEXT("%s%s"), Objective.bDone ? TEXT("done: ") : TEXT(""), *Objective.Text));
	}
	AddInfo(FString::Printf(TEXT("Run 1: cows %.1f min, humans %.1f min, %d UFOs, humans %d/%d experiments"),
		Run1.Unlock[1] / 60.0, Run1.Unlock[3] / 60.0, Run1Ufos, Run1Humans, Run1Total));
	AddInfo(FString::Printf(TEXT("Run 2: cows %.1f min, humans %.1f min, %d UFOs, crew %d, humans %d/%d experiments; thesis: %s"),
		Run2.Unlock[1] / 60.0, Run2.Unlock[3] / 60.0, Sim.GetNumUfos(), Sim.GetCrew().Num(), Run2Humans, Run2Total, *FString::Join(Objectives, TEXT("; "))));
	TestTrue(TEXT("Run 2 opens faster"), Run2.Unlock[1] > 0.0 && Run2.Unlock[1] < Run1.Unlock[1]);
	TestTrue(TEXT("Run 2 reaches humans sooner"), Run2.Unlock[3] > 0.0 && (Run1.Unlock[3] <= 0.0 || Run2.Unlock[3] < Run1.Unlock[3]));
	TestTrue(TEXT("Run 2 fleet obeys the new thesis"), Sim.GetNumUfos() <= 2 && Run1Ufos == 3);
	TestTrue(TEXT("Run 2 is a different operation (more human work)"), float(Run2Humans) / Run2Total > float(Run1Humans) / Run1Total + 0.1f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleThesisPacingTest, "AlienIdle.Sim.ThesisPacing", SimTestFlags)
bool FAlienIdleThesisPacingTest::RunTest(const FString& Parameters)
{
	// Spec target: the first thesis takes a 20-30 minute development run.
	struct FCase { EBotStrategy Strategy; EDepartment Department; const TCHAR* Label; };
	const FCase Cases[] = {
		{ EBotStrategy::Balanced, EDepartment::Biological, TEXT("Balanced + Biology") },
		{ EBotStrategy::BulkAnimals, EDepartment::Biological, TEXT("Bulk animals + Biology") },
		{ EBotStrategy::SelectiveHumans, EDepartment::Human, TEXT("Humans + Human Behavior") },
		{ EBotStrategy::Balanced, EDepartment::Incompetence, TEXT("Balanced + Incompetence") },
	};
	for (const FCase& Case : Cases)
	{
		FAlienIdleSim Sim;
		double Minutes = 0.0;
		while (!Sim.CanSubmitThesis() && Minutes < 60.0)
		{
			RunBotOn(Sim, Case.Strategy, 1.0, Case.Department);
			Minutes += 1.0;
			if (FMath::Fmod(Minutes, 5.0) == 0.0)
			{
				FString Waiting;
				Sim.CanSubmitThesis(&Waiting);
				AddInfo(FString::Printf(TEXT("  %s @%.0f min: %s | %s"), Case.Label, Minutes, *Waiting, *Sim.GetSignatureStatusText(Case.Department)));
			}
		}
		FString Reason;
		Sim.CanSubmitThesis(&Reason);
		AddInfo(FString::Printf(TEXT("%s: first thesis at %.0f min%s"), Case.Label, Minutes, Sim.CanSubmitThesis() ? TEXT("") : *(TEXT(" (not done: ") + Reason + TEXT(")"))));
		TestTrue(FString::Printf(TEXT("%s: first thesis within 16-32 minutes"), Case.Label), Sim.CanSubmitThesis() && Minutes >= 16.0 && Minutes <= 32.0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleRegionsTest, "AlienIdle.Sim.RegionsCompete", SimTestFlags)
bool FAlienIdleRegionsTest::RunTest(const FString& Parameters)
{
	// The same human is worth more at the fairgrounds, and the fairgrounds notice more.
	auto OneHuman = [](int32 Region, double& OutResearch, double& OutSuspicion)
	{
		FQuietSim Sim;
		Sim.DevUnlockSpecimen(ESpecimen::Human);
		Sim.SetDestinationRegion(0, Region, ESpecimen::Human);
		Sim.DevAddEnergy(1000.0);
		Sim.Launch(0);
		RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
		OutSuspicion = Sim.GetRegionState(Region).Suspicion;
		// Normalise the profession so only the region differs.
		FSpecimen Subject = Sim.GetUfo(0).Cargo[0];
		FQuietSim Lab;
		Lab.DevUnlockSpecimen(ESpecimen::Human);
		Subject.Occupation = EOccupation::Therapist;
		Lab.DevAddToContainment(Subject);
		Lab.DevAddEnergy(1000.0);
		RunUntil(Lab, [&] { return Lab.GetTotalExperiments() == 1; });
		OutResearch = Lab.GetResearch();
	};
	double TownResearch, TownSuspicion, FairResearch, FairSuspicion;
	OneHuman(1, TownResearch, TownSuspicion);
	OneHuman(2, FairResearch, FairSuspicion);
	AddInfo(FString::Printf(TEXT("Glenwhit: research %.0f, suspicion %.1f. Fairgrounds: research %.0f, suspicion %.1f"), TownResearch, TownSuspicion, FairResearch, FairSuspicion));
	TestTrue(TEXT("Valuable region pays more"), FairResearch > TownResearch);
	TestTrue(TEXT("Valuable region is more exposed"), FairSuspicion > TownSuspicion);

	FQuietSim Sim;
	FString Reason;
	Sim.DevUnlockSpecimen(ESpecimen::Human);
	Sim.SetDestinationRegion(0, 0, ESpecimen::Human);
	TestFalse(TEXT("No humans in Pine Ridge Valley"), Sim.CanLaunch(0, &Reason));
	TestTrue(TEXT("Reason names the region"), Reason.Contains(TEXT("Pine Ridge Valley")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleResponsesTest, "AlienIdle.Sim.EarthResponds", SimTestFlags)
bool FAlienIdleResponsesTest::RunTest(const FString& Parameters)
{
	// Repeated cow raids on the valley: farmers organise night patrols.
	FQuietSim Sim;
	Sim.DevDiscover(EDiscovery::AcornAmnesia);
	BuyShip(Sim, 0, EShipSystem::Tractor, 1);
	BuyShip(Sim, 0, EShipSystem::Cargo, 3);
	BuyBase(Sim, EBaseUpgrade::Containment, 2);
	Sim.SetDestinationRegion(0, 0, ESpecimen::Cow);
	const float CalmCapture = Sim.GetCaptureSeconds(0, ESpecimen::Cow, false);
	for (int32 Trip = 0; Trip < 6 && !Sim.IsResponseActive(0, EResponse::NightPatrols); ++Trip)
	{
		Sim.DevAddEnergy(1000.0);
		RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.CanLaunch(0); });
		Sim.Launch(0);
		RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
	}
	TestTrue(TEXT("Night patrols formed"), Sim.IsResponseActive(0, EResponse::NightPatrols));
	RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetUfo(0).State == EUfoState::Docked; });
	TestTrue(TEXT("Soft effect: captures slower, nothing lost"), Sim.GetCaptureSeconds(0, ESpecimen::Cow, false) > CalmCapture * 1.5f);
	TestTrue(TEXT("Log names the response"), Sim.GetLog().ContainsByPredicate([](const FAlienIdleLogEntry& E) { return E.Text.Contains(TEXT("night patrols")); }));

	// Counter-action: decoy cows. Credibility drops, so the second one costs more.
	const double FirstCost = Sim.GetCounterCost(0, EResponse::NightPatrols);
	Sim.DevAddResearch(FirstCost);
	TestTrue(TEXT("Deploy decoys"), Sim.Counter(0, EResponse::NightPatrols));
	TestFalse(TEXT("Patrols gone"), Sim.IsResponseActive(0, EResponse::NightPatrols));
	TestTrue(TEXT("Next decoy costs more"), Sim.GetCounterCost(0, EResponse::NightPatrols) > FirstCost);

	// Or simply leave the valley alone: pressure fades on its own.
	Sim.SetDestinationRegion(0, 2, ESpecimen::Cow);
	for (int32 Trip = 0; Trip < 3; ++Trip)
	{
		Sim.DevAddEnergy(1000.0);
		RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.CanLaunch(0); });
		Sim.Launch(0);
		RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetUfo(0).State == EUfoState::Docked; });
	}
	TestEqual(TEXT("Valley patrol pressure fades when we hunt elsewhere"), Sim.GetResponsePressure(0, EResponse::NightPatrols) < 1.0f, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleRerouteTest, "AlienIdle.Sim.AutomationReroutes", SimTestFlags)
bool FAlienIdleRerouteTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	for (int32 i = 0; i < AlienIdleTuning::AutomationUnlockExperiments; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
	}
	RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.IsAutomationUnlocked(); });
	Sim.DevDiscover(EDiscovery::AcornAmnesia);
	BuyShip(Sim, 0, EShipSystem::Tractor, 1);
	BuyShip(Sim, 0, EShipSystem::Cargo, 3);
	BuyBase(Sim, EBaseUpgrade::Containment, 2);
	Sim.SetDestinationRegion(0, 0, ESpecimen::Cow);
	Sim.SetAutoLaunch(0, true);
	TestTrue(TEXT("Patrols eventually form in the valley and the ship reroutes"),
		RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetReroutes() > 0; }, 900.0f));
	TestEqual(TEXT("Cows now hunted at the fairgrounds"), Sim.GetUfo(0).Region, 2);
	TestEqual(TEXT("Still after cows"), Sim.GetUfo(0).Destination, ESpecimen::Cow);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleEarthConditionsTest, "AlienIdle.Sim.EarthConditions", SimTestFlags)
bool FAlienIdleEarthConditionsTest::RunTest(const FString& Parameters)
{
	// Same flights, different worlds: radar reacts very differently.
	auto RadarAfterLaunches = [](EEarthCondition Earth)
	{
		FQuietSim Sim;
		Sim.DevSetEarthCondition(Earth);
		for (int32 i = 0; i < 8; ++i)
		{
			Sim.DevAddEnergy(1000.0);
			RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.CanLaunch(0); });
			Sim.Launch(0);
		}
		return Sim.GetResponsePressure(Sim.GetUfo(0).TripRegion, EResponse::Radar);
	};
	const float Usual = RadarAfterLaunches(EEarthCondition::BusinessAsUsual);
	const float Craze = RadarAfterLaunches(EEarthCondition::DocumentaryCraze);
	const float Advertising = RadarAfterLaunches(EEarthCondition::EverythingIsAdvertising);
	AddInfo(FString::Printf(TEXT("Radar pressure after 8 launches: usual %.1f, documentary craze %.1f, everything is advertising %.1f"), Usual, Craze, Advertising));
	TestTrue(TEXT("Documentary craze: radar reacts faster"), Craze > Usual);
	TestTrue(TEXT("Everything is advertising: radar barely cares"), Advertising < Usual * 0.5f);

	FQuietSim Crackdown;
	Crackdown.DevSetEarthCondition(EEarthCondition::SurveillanceCrackdown);
	TestTrue(TEXT("Crackdown lowers the response threshold"), Crackdown.GetResponseThreshold() < AlienIdleTuning::ResponseThreshold);

	// Chosen at promotion, previewed, and kept for the run.
	FQuietSim Sim;
	Sim.DevCompleteThesis();
	FNextAssignment Next;
	Next.Thesis = EThesis::NobodyWillBelieveYou;
	Next.PermanentOption = 0;
	Next.Condition = EEarthCondition::EverythingIsAdvertising;
	TestTrue(TEXT("Promote"), Sim.Promote(Next));
	TestEqual(TEXT("Condition applied"), Sim.GetEarthCondition(), EEarthCondition::EverythingIsAdvertising);
	TestFalse(TEXT("Preview text"), FString(AlienIdleTuning::EarthCondition(EEarthCondition::EverythingIsAdvertising).Preview).IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleEarthNoticesRunTest, "AlienIdle.Sim.EarthNoticesRun", SimTestFlags)
bool FAlienIdleEarthNoticesRunTest::RunTest(const FString& Parameters)
{
	// Over a normal 30-minute run Earth should react, and the operation should move around because of it.
	auto Run = [this](EEarthCondition Earth, const TCHAR* Label)
	{
		FAlienIdleSim Sim;
		Sim.DevSetEarthCondition(Earth);
		RunBotOn(Sim, EBotStrategy::Balanced, 30.0, EDepartment::Biological);
		TArray<int32> Used;
		for (const FAlienIdleLogEntry& Entry : Sim.GetLog())
		{
			for (int32 r = 0; r < Sim.GetNumRegions(); ++r)
			{
				if (Entry.Text.Contains(Sim.GetRegionNameAt(r)))
				{
					Used.AddUnique(r);
				}
			}
		}
		AddInfo(FString::Printf(TEXT("%s: responses triggered %d (active now %d), response-minutes %.0f, regions mentioned %d, research earned %.0f"),
			Label, Sim.GetResponsesTriggered(), Sim.GetActiveResponseCount(), Sim.GetResponseSeconds() / 60.0, Used.Num(), Sim.GetTotalResearchEarned()));
		return Sim.GetResponseSeconds();
	};
	const double Usual = Run(EEarthCondition::BusinessAsUsual, TEXT("Business as usual"));
	const double Crackdown = Run(EEarthCondition::SurveillanceCrackdown, TEXT("Surveillance crackdown"));
	const double Advertising = Run(EEarthCondition::EverythingIsAdvertising, TEXT("Everything is advertising"));
	TestTrue(TEXT("Earth notices a normal operation"), Usual > 0.0);
	TestTrue(TEXT("A crackdown provokes more responses"), Crackdown > Usual);
	TestTrue(TEXT("Advertising cover provokes fewer"), Advertising < Crackdown);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleRivalSweepTest, "AlienIdle.Sim.RivalSweep", SimTestFlags)
bool FAlienIdleRivalSweepTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	BuyShip(Sim, 0, EShipSystem::Cargo, 2);
	BuyShip(Sim, 0, EShipSystem::Stealth, 1); // keeps the valley calm, so it stays a sweep target
	BuyBase(Sim, EBaseUpgrade::Containment, 2);
	Sim.SetDestinationRegion(0, 0, ESpecimen::Squirrel);
	// Hunt squirrels in the valley until the Bureau announces its sweep.
	bool bAnnouncedEarly = false;
	while (!Sim.GetRival().bAnnounced && Sim.GetElapsedSeconds() < 400.0)
	{
		Sim.DevAddEnergy(1000.0);
		if (Sim.CanLaunch(0))
		{
			Sim.Launch(0);
		}
		Sim.Tick(0.5f);
	}
	bAnnouncedEarly = Sim.GetRival().bAnnounced && !Sim.GetRival().bSweeping
		&& Sim.GetRival().NextSweep - Sim.GetElapsedSeconds() >= AlienIdleTuning::RivalSweepWarningSeconds - 1.0;
	TestTrue(TEXT("Sweep announced well ahead"), bAnnouncedEarly);
	TestEqual(TEXT("Aimed at our busiest ground"), Sim.GetRival().SweepRegion, 0);
	TestEqual(TEXT("Aimed at squirrels"), Sim.GetRival().SweepType, ESpecimen::Squirrel);

	RunUntil(Sim, [&] { return Sim.GetRival().bSweeping; }, 120.0f);
	TestTrue(TEXT("Sweep under way"), Sim.IsSwept(0, ESpecimen::Squirrel));
	RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetUfo(0).State == EUfoState::Docked; }, 120.0f);
	Sim.DevAddEnergy(1000.0);
	Sim.Launch(0);
	RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
	TestEqual(TEXT("Half the catch during a sweep"), Sim.GetUfo(0).Cargo.Num(), FMath::Max(1, Sim.GetCargoCapacity(0) / 2));
	TestEqual(TEXT("Planning ahead: the best squirrel ground is now elsewhere"), Sim.ChooseRegionFor(0, ESpecimen::Squirrel), 1);

	const double SuspicionDuring = Sim.GetRegionState(0).Suspicion;
	TestTrue(TEXT("Sweep ends by itself"), RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return !Sim.GetRival().bSweeping; }, 200.0f));
	TestTrue(TEXT("The region comes back calmer"), Sim.GetRegionState(0).Suspicion < SuspicionDuring);
	TestTrue(TEXT("Next sweep scheduled"), Sim.GetRival().NextSweep > Sim.GetElapsedSeconds());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleTradeTest, "AlienIdle.Sim.RivalTrading", SimTestFlags)
bool FAlienIdleTradeTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.SetAutoAssign(false);
	Sim.Assign(1, FAssignment()); // no scientist: the lab cannot drain containment
	for (int32 i = 0; i < 3; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
	}
	Sim.SetSellOrder(ESpecimen::Squirrel, true);
	const double Before = Sim.GetResearch();
	Sim.Tick(1.0f);
	TestEqual(TEXT("Surplus squirrel sold"), Sim.GetContainment().Num(), 2);
	TestTrue(TEXT("Paid in research"), Sim.GetResearch() > Before);
	Sim.Tick(10.0f);
	TestEqual(TEXT("Keeps one of each species, stops below 75% full"), Sim.GetContainment().Num(), 2);

	FQuietSim Buyer;
	Buyer.DevDiscover(EDiscovery::AcornAmnesia);
	Buyer.DevAddResearch(100.0);
	Buyer.SetBuyOrder(ESpecimen::Cow, true);
	Buyer.SetAutoAssign(false);
	Buyer.Assign(1, FAssignment());
	Buyer.Tick(1.0f);
	TestTrue(TEXT("A cow delivered without flying"), Buyer.GetContainment().ContainsByPredicate([](const FSpecimen& S) { return S.Type == ESpecimen::Cow; }));
	TestEqual(TEXT("Cost charged"), Buyer.GetResearch(), 100.0 - AlienIdleTuning::TradeBuyCost[1]);
	TestTrue(TEXT("Standing orders never expire"), Buyer.GetRival().Buy[1]);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleRaceTest, "AlienIdle.Sim.RivalRace", SimTestFlags)
bool FAlienIdleRaceTest::RunTest(const FString& Parameters)
{
	// Win: study squirrels faster than the Bureau's pace.
	FQuietSim Sim;
	RunUntil(Sim, [&] { return Sim.GetRival().bRaceOffered; }, AlienIdleTuning::RaceFirstOfferSeconds + 5.0f);
	TestTrue(TEXT("Race offered"), Sim.GetRival().bRaceOffered);
	TestTrue(TEXT("Accept"), Sim.AcceptRace());
	const int32 GrantsBefore = Sim.GetCareer().Grants;
	BuyBase(Sim, EBaseUpgrade::Chambers, 1);
	for (int32 i = 0; i < Sim.GetRival().RaceCount; ++i)
	{
		Sim.DevAddToContainment({ Sim.GetRival().RaceType });
	}
	BuyBase(Sim, EBaseUpgrade::Containment, 3);
	Sim.DevAddResearch(Sim.GetRecruitCost());
	Sim.Recruit();
	TestTrue(TEXT("Race won"), RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetCareer().RaceWins == 1; }, 300.0f));
	TestEqual(TEXT("Extra grants"), Sim.GetCareer().Grants, GrantsBefore + AlienIdleTuning::RaceGrants);

	// Lose: no penalty. And the Bureau does not race while the player is away.
	FQuietSim Idle;
	RunUntil(Idle, [&] { return Idle.GetRival().bRaceOffered; }, AlienIdleTuning::RaceFirstOfferSeconds + 5.0f);
	Idle.AcceptRace();
	Idle.CatchUp(3600.0);
	TestTrue(TEXT("Race paused while offline"), Idle.GetRival().bRaceActive && Idle.GetRival().RivalProgress == 0.0f);
	const double Research = Idle.GetResearch();
	TestTrue(TEXT("Lost while playing"), RunUntil(Idle, [&] { return Idle.GetCareer().RaceLosses == 1; }, 600.0f));
	TestTrue(TEXT("No penalty for losing"), Idle.GetResearch() >= Research);
	TestTrue(TEXT("The spelling-mistakes line"), Idle.GetLog().ContainsByPredicate([](const FAlienIdleLogEntry& E) { return E.Text.Contains(TEXT("spelling mistakes")); }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleRivalRunTest, "AlienIdle.Sim.RivalRun", SimTestFlags)
bool FAlienIdleRivalRunTest::RunTest(const FString& Parameters)
{
	// A normal 30-minute run with the Bureau around: sweeps happen, we steer around them, trades and races pay.
	FAlienIdleSim Sim;
	int32 FlightsDuringSweeps = 0;
	int32 IntoSweep = 0;
	int32 FlightsOtherwise = 0;
	int32 IntoSamePairOtherwise = 0;
	TArray<TPair<int32, ESpecimen>> SweptPairs;
	for (int32 Tick = 0; Tick < 30 * 12; ++Tick)
	{
		RunBotOn(Sim, EBotStrategy::Balanced, 1.0 / 12.0, EDepartment::Biological);
		const FRivalState& R = Sim.GetRival();
		if (R.bSweeping)
		{
			SweptPairs.AddUnique({ R.SweepRegion, R.SweepType });
		}
		for (int32 u = 0; u < Sim.GetNumUfos(); ++u)
		{
			const FUfo& Ufo = Sim.GetUfo(u);
			if (Ufo.State != EUfoState::FlyingOut || Ufo.StateTime > 5.0f)
			{
				continue; // count each flight once, near its launch
			}
			const bool bSweptPair = SweptPairs.Contains(TPair<int32, ESpecimen>(Ufo.TripRegion, Ufo.TripTarget));
			if (R.bSweeping)
			{
				++FlightsDuringSweeps;
				IntoSweep += (Ufo.TripRegion == R.SweepRegion && Ufo.TripTarget == R.SweepType) ? 1 : 0;
			}
			else
			{
				++FlightsOtherwise;
				IntoSamePairOtherwise += bSweptPair ? 1 : 0;
			}
		}
	}
	const FRivalState& Rival = Sim.GetRival();
	const float ShareDuring = float(IntoSweep) / FMath::Max(1, FlightsDuringSweeps);
	const float ShareOtherwise = float(IntoSamePairOtherwise) / FMath::Max(1, FlightsOtherwise);
	AddInfo(FString::Printf(TEXT("30 min: sweeps %d; flights into the swept ground: %.0f%% during sweeps (%d/%d) vs %.0f%% to those grounds otherwise (%d/%d); trades %d for %.0f research; races won %d, lost %d"),
		Rival.Sweeps, 100.0f * ShareDuring, IntoSweep, FlightsDuringSweeps, 100.0f * ShareOtherwise, IntoSamePairOtherwise, FlightsOtherwise,
		Rival.TradesThisRun, Rival.TradeIncome, Sim.GetCareer().RaceWins, Sim.GetCareer().RaceLosses));
	TestTrue(TEXT("The Bureau swept at least twice"), Rival.Sweeps >= 2);
	TestTrue(TEXT("Decision changed: far fewer flights to a ground while the Bureau sweeps it"), FlightsDuringSweeps > 0 && ShareDuring < ShareOtherwise * 0.5f);
	TestTrue(TEXT("Benefit: trading paid"), Rival.TradeIncome > 0.0);
	TestTrue(TEXT("Benefit: at least one race won"), Sim.GetCareer().RaceWins >= 1);
	TestTrue(TEXT("Core thesis still achievable"), Sim.CanSubmitThesis());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCelebrityTest, "AlienIdle.Sim.RivalCelebrityStunt", SimTestFlags)
bool FAlienIdleCelebrityTest::RunTest(const FString& Parameters)
{
	auto GlenwhitSuspicionFromOneHuman = [](bool bStunt)
	{
		FQuietSim Sim;
		Sim.DevSetRivals(false, bStunt, false);
		Sim.DevUnlockSpecimen(ESpecimen::Human);
		RunUntil(Sim, [&] { return !bStunt || Sim.GetRival().Celebrity.bActive; }, AlienIdleTuning::CelebrityFirstSeconds + 5.0f);
		const double Before = Sim.GetRegionState(1).Suspicion;
		Sim.SetDestinationRegion(0, 1, ESpecimen::Human);
		Sim.DevAddEnergy(1000.0);
		Sim.Launch(0);
		RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
		return Sim.GetRegionState(1).Suspicion - Before;
	};
	FQuietSim Sim;
	Sim.DevSetRivals(false, true, false);
	RunUntil(Sim, [&] { return Sim.GetRival().Celebrity.bAnnounced; }, AlienIdleTuning::CelebrityFirstSeconds);
	TestTrue(TEXT("Stunt announced ahead"), Sim.GetRival().Celebrity.bAnnounced && !Sim.GetRival().Celebrity.bActive);
	TestEqual(TEXT("In the most valuable human region"), Sim.GetRival().Celebrity.Region, 2);
	RunUntil(Sim, [&] { return Sim.GetRival().Celebrity.bActive; }, 120.0f);
	Sim.Tick(60.0f);
	TestTrue(TEXT("Investigators gather at the stunt"), Sim.GetResponsePressure(2, EResponse::Investigators) > 30.0f);

	const double Normal = GlenwhitSuspicionFromOneHuman(false);
	const double Covered = GlenwhitSuspicionFromOneHuman(true);
	AddInfo(FString::Printf(TEXT("Human taken from Glenwhit: suspicion +%.1f normally, +%.1f during a stunt elsewhere"), Normal, Covered));
	TestTrue(TEXT("Opportunity: quieter elsewhere during the stunt"), Covered < Normal * 0.6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleBreachTest, "AlienIdle.Sim.RivalBreach", SimTestFlags)
bool FAlienIdleBreachTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.DevSetRivals(false, false, true);
	RunUntil(Sim, [&] { return Sim.GetRival().Breach.bActive; }, AlienIdleTuning::BreachFirstSeconds + 5.0f);
	TestTrue(TEXT("Breach under way"), Sim.IsBreach(0));
	Sim.SetDestinationRegion(0, 0, ESpecimen::Squirrel);
	Sim.DevAddEnergy(1000.0);
	Sim.Launch(0);
	RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Returning; });
	TestEqual(TEXT("Escapees: one extra specimen"), Sim.GetUfo(0).Cargo.Num(), Sim.GetCargoCapacity(0) + 1);
	Sim.Tick(60.0f);
	const float Pressure = Sim.GetResponsePressure(0, EResponse::SupportGroup);
	TestTrue(TEXT("Support group pressure from the breach"), Pressure > 30.0f);

	FQuietSim Clean;
	Clean.DevSetRivals(false, false, true);
	BuyBase(Clean, EBaseUpgrade::Cleanup, 1);
	RunUntil(Clean, [&] { return Clean.GetRival().Breach.bActive; }, AlienIdleTuning::BreachFirstSeconds + 5.0f);
	Clean.Tick(60.0f);
	TestTrue(TEXT("Cleanup Crew halves the fallout"), Clean.GetResponsePressure(0, EResponse::SupportGroup) < Pressure * 0.7f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleJointTest, "AlienIdle.Sim.RivalJointExperiment", SimTestFlags)
bool FAlienIdleJointTest::RunTest(const FString& Parameters)
{
	FQuietSim Sim;
	Sim.DevDiscover(EDiscovery::AcornAmnesia);
	Sim.SetAutoAssign(false);
	Sim.Assign(1, FAssignment()); // no scientist, so containment is not studied
	BuyBase(Sim, EBaseUpgrade::Containment, 4);
	TestEqual(TEXT("Joint experiment with the Bureau"), Sim.GetJointRival(), ERival::EfficiencyBureau);
	for (int32 i = 0; i < 6; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Squirrel });
	}
	for (int32 i = 0; i < 4; ++i)
	{
		Sim.DevAddToContainment({ ESpecimen::Cow });
	}
	const double SellBefore = Sim.GetSellFraction();
	const int32 Grants = Sim.GetCareer().Grants;
	Sim.SetContribute(true);
	TestTrue(TEXT("Joint experiment completes"), RunUntil(Sim, [&] { return Sim.GetCareer().Collaborations == 1; }, 120.0f));
	TestEqual(TEXT("Shared discovery grant"), Sim.GetCareer().Grants, Grants + AlienIdleTuning::JointGrants);
	TestTrue(TEXT("Reputation: better trades"), Sim.GetSellFraction() > SellBefore);
	TestEqual(TEXT("Specimens contributed"), Sim.GetContainment().Num(), 0);
	TestEqual(TEXT("No more joint experiments this run with the Bureau"), Sim.GetJointRival(), ERival::Count);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleReputationTest, "AlienIdle.Sim.RivalReputationAndPairing", SimTestFlags)
bool FAlienIdleReputationTest::RunTest(const FString& Parameters)
{
	// A win makes the next challenge bigger and better paid.
	FQuietSim Sim;
	RunUntil(Sim, [&] { return Sim.GetRival().bRaceOffered; }, AlienIdleTuning::RaceFirstOfferSeconds + 5.0f);
	const int32 FirstCount = Sim.GetRival().RaceCount;
	const int32 FirstGrants = Sim.GetRaceGrants();
	Sim.AcceptRace();
	BuyBase(Sim, EBaseUpgrade::Chambers, 1);
	BuyBase(Sim, EBaseUpgrade::Containment, 3);
	Sim.DevAddResearch(Sim.GetRecruitCost());
	Sim.Recruit();
	for (int32 i = 0; i < FirstCount; ++i)
	{
		Sim.DevAddToContainment({ Sim.GetRival().RaceType });
	}
	RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.GetCareer().RaceWins == 1; }, 300.0f);
	RunUntil(Sim, [&] { return Sim.GetRival().bRaceOffered; }, AlienIdleTuning::RaceOfferIntervalSeconds + 5.0f);
	AddInfo(FString::Printf(TEXT("Race size %d -> %d, grants %d -> %d"), FirstCount, Sim.GetRival().RaceCount, FirstGrants, Sim.GetRaceGrants()));
	TestTrue(TEXT("Harder race after a win"), Sim.GetRival().RaceCount > FirstCount);
	TestTrue(TEXT("Better prize after a win"), Sim.GetRaceGrants() > FirstGrants);

	// Each assignment features a different pairing.
	FQuietSim Next;
	Next.DevCompleteThesis();
	FNextAssignment Assignment;
	Assignment.Thesis = EThesis::NobodyWillBelieveYou;
	Assignment.PermanentOption = 0;
	TestTrue(TEXT("Promote"), Next.Promote(Assignment));
	TestTrue(TEXT("Nobody Will Believe You: Celebrity Studies and Experimental Sciences"),
		!Next.IsRivalActive(ERival::EfficiencyBureau) && Next.IsRivalActive(ERival::CelebrityStudies) && Next.IsRivalActive(ERival::ExperimentalSciences));
	Next.Tick(AlienIdleTuning::RivalFirstSweepSeconds + 60.0f);
	TestEqual(TEXT("No Bureau sweeps on this assignment"), Next.GetRival().Sweeps, 0);
	return true;
}

namespace
{
	struct FApproachRun
	{
		double Run1Minutes = 0.0;
		double Minutes = 0.0;
		bool bDone = false;
		double ResearchPerMinute = 0.0;
		double ResponseMinutes = 0.0;
		int32 Responses = 0;
		int32 Incidents = 0;
		double OfflinePerMinute = 0.0;
		int32 OfflineExperiments = 0;
		int32 OfflineContracts = 0;
		double OfflineResponseMinutes = 0.0;
		FString OfflineNote;
		bool bPermanentOffered = false;
		FString Reading;
		FString Objectives;
	};

	/** Run 1 in the approach's department, promote into its thesis keeping its permanent pick, then play run 2 to the end. */
	FApproachRun RunApproach(AlienIdleTuning::EApproach Which)
	{
		const AlienIdleTuning::FApproachDef& Def = AlienIdleTuning::Approach(Which);
		const EBotStrategy Bot = Which == AlienIdleTuning::EApproach::Bulk ? EBotStrategy::BulkAnimals
			: Which == AlienIdleTuning::EApproach::Precision ? EBotStrategy::SelectiveHumans : EBotStrategy::Covert;
		const EBotStrategy Run1Bot = Which == AlienIdleTuning::EApproach::Bulk ? EBotStrategy::BulkAnimals
			: Which == AlienIdleTuning::EApproach::Precision ? EBotStrategy::SelectiveHumans : EBotStrategy::Balanced;
		FApproachRun Out;
		FAlienIdleSim Sim;
		// Approach balance is measured without chance recurring subjects: one turning up in run 1 changes the whole RNG path
		// of run 2 (the RNG carries through prestige), which says nothing about the approach. Case files have their own tests.
		Sim.DevSetCaseAppearances(false);
		while (!Sim.CanSubmitThesis() && Out.Run1Minutes < 60.0)
		{
			RunBotOn(Sim, Run1Bot, 1.0, Def.Department);
			Out.Run1Minutes += 1.0;
		}
		if (!Sim.CanSubmitThesis())
		{
			Sim.DevCompleteThesis();
		}
		const FPromotionReview Review = Sim.BuildPromotionReview();
		FNextAssignment Next;
		Next.Thesis = Def.Thesis;
		Next.Condition = EEarthCondition::BusinessAsUsual;
		Next.PermanentOption = Review.Options.IndexOfByPredicate([&Def](const FPermanentOption& O) { return !O.bSignature && O.Project == Def.Permanent; });
		Out.bPermanentOffered = Next.PermanentOption != INDEX_NONE;
		Next.PermanentOption = Out.bPermanentOffered ? Next.PermanentOption : 0;
		Sim.Promote(Next);

		const double Research0 = Sim.GetTotalResearchEarned();
		const double Response0 = Sim.GetResponseSeconds();
		const int32 Responses0 = Sim.GetResponsesTriggered();
		const int32 Incidents0 = Sim.GetIncidentCount();
		while (!Sim.CanSubmitThesis() && Out.Minutes < 60.0)
		{
			RunBotOn(Sim, Bot, 1.0, Def.Department);
			Out.Minutes += 1.0;
			if (Out.Minutes == 10.0)
			{
				// Offline check: from this moment, 30 minutes closed with the approach template's automation.
				TArray<uint8> Bytes;
				Sim.SaveToBytes(Bytes);
				FAlienIdleSim Away;
				Away.LoadFromBytes(Bytes);
				Away.ApplyApproachTemplate(Which);
				const double Before = Away.GetTotalResearchEarned();
				const int32 Experiments = Away.GetTotalExperiments();
				const int32 Contracts = Away.GetContractsCompleted();
				const double Responding = Away.GetResponseSeconds();
				Away.BeginAwayReport();
				Out.OfflineNote += FString::Printf(TEXT("at 10 min: energy %.0f/%.0f +%.2f/s, launch %.0f, crew %d (idle %d), research %.0f; "), Away.GetEnergy(), Away.GetEnergyCap(),
					Away.GetEnergyPerSecond(), Away.GetLaunchEnergy(0, Away.GetUfo(0).Destination), Away.GetCrew().Num(), Away.GetIdleCrew(), Away.GetResearch());
				Away.CatchUp(30.0 * 60.0);
				Out.OfflineContracts = Away.GetContractsCompleted() - Contracts;
				Out.OfflineResponseMinutes = (Away.GetResponseSeconds() - Responding) / 60.0;
				const FAwayReport Report = Away.BuildAwayReport();
				for (int32 i = 1; i < int32(EBottleneck::Count); ++i)
				{
					Out.OfflineNote += FString::Printf(TEXT("%s %.0fs; "), FAlienIdleSim::GetBottleneckName(EBottleneck(i)), Report.LostSeconds[i]);
				}
				for (int32 u = 0; u < Away.GetNumUfos(); ++u)
				{
					Out.OfflineNote += FString::Printf(TEXT("UFO %d: %s; "), u, *Away.GetUfoStatusText(u));
				}
				Out.OfflinePerMinute = (Away.GetTotalResearchEarned() - Before) / 30.0;
				Out.OfflineExperiments = Away.GetTotalExperiments() - Experiments;
			}
		}
		Out.bDone = Sim.CanSubmitThesis();
		Out.ResearchPerMinute = (Sim.GetTotalResearchEarned() - Research0) / FMath::Max(1.0, Out.Minutes);
		Out.ResponseMinutes = (Sim.GetResponseSeconds() - Response0) / 60.0;
		Out.Responses = Sim.GetResponsesTriggered() - Responses0;
		Out.Incidents = Sim.GetIncidentCount() - Incidents0;
		Out.Reading = Sim.GetApproachReading();
		TArray<FString> Lines;
		for (const FThesisObjective& Objective : Sim.GetThesisObjectives())
		{
			Lines.Add(FString::Printf(TEXT("%s%s"), Objective.bDone ? TEXT("done: ") : TEXT(""), *Objective.Text));
		}
		Out.Objectives = FString::Join(Lines, TEXT("; "));
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleApproachCyclesTest, "AlienIdle.Sim.ApproachCycles", SimTestFlags)
bool FAlienIdleApproachCyclesTest::RunTest(const FString& Parameters)
{
	FApproachRun Runs[int32(AlienIdleTuning::EApproach::Count)];
	for (int32 a = 0; a < int32(AlienIdleTuning::EApproach::Count); ++a)
	{
		const AlienIdleTuning::FApproachDef& Def = AlienIdleTuning::Approaches[a];
		FApproachRun& Run = Runs[a];
		Run = RunApproach(AlienIdleTuning::EApproach(a));
		AddInfo(FString::Printf(TEXT("%s: run 1 %.0f min (permanent offered: %s); run 2 %s in %.0f min; %.0f research/min; Earth responses %d (%.1f response-min); incidents %d; offline %.0f research/min (%d experiments in 30 min)"),
			Def.Name, Run.Run1Minutes, Run.bPermanentOffered ? TEXT("yes") : TEXT("no"), Run.bDone ? TEXT("finished") : TEXT("NOT finished"), Run.Minutes,
			Run.ResearchPerMinute, Run.Responses, Run.ResponseMinutes, Run.Incidents, Run.OfflinePerMinute, Run.OfflineExperiments));
		AddInfo(FString::Printf(TEXT("  %s | %s"), *Run.Reading, *Run.Objectives));
		AddInfo(FString::Printf(TEXT("  Offline: %d contracts, %.1f response-min; %s"), Run.OfflineContracts, Run.OfflineResponseMinutes, *Run.OfflineNote));
		TestTrue(FString::Printf(TEXT("%s: permanent discovery offered after run 1"), Def.Name), Run.bPermanentOffered);
		TestTrue(FString::Printf(TEXT("%s: prestige cycle finishes within 45 minutes"), Def.Name), Run.bDone && Run.Minutes <= 45.0);
		TestTrue(FString::Printf(TEXT("%s: offline progress stays useful (>= 30%% of active research rate)"), Def.Name),
			Run.OfflineExperiments > 0 && Run.OfflinePerMinute >= Run.ResearchPerMinute * 0.3);
	}
	// No approach may be fastest, safest and most profitable all at once.
	for (int32 a = 0; a < int32(AlienIdleTuning::EApproach::Count); ++a)
	{
		bool bFastest = true, bSafest = true, bRichest = true;
		for (int32 b = 0; b < int32(AlienIdleTuning::EApproach::Count); ++b)
		{
			if (a != b)
			{
				bFastest &= Runs[a].Minutes < Runs[b].Minutes;
				bSafest &= Runs[a].ResponseMinutes < Runs[b].ResponseMinutes;
				bRichest &= Runs[a].ResearchPerMinute > Runs[b].ResearchPerMinute;
			}
		}
		TestFalse(FString::Printf(TEXT("%s does not dominate on speed, safety and profit"), AlienIdleTuning::Approaches[a].Name), bFastest && bSafest && bRichest);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdlePresetsTest, "AlienIdle.Sim.PresetsAndExplanations", SimTestFlags)
bool FAlienIdlePresetsTest::RunTest(const FString& Parameters)
{
	FAlienIdleSim Sim;
	RunBotOn(Sim, EBotStrategy::Balanced, 12.0);
	Sim.SetAutoLaunch(0, true);
	Sim.SavePreset(0, TEXT("Test"));
	const ESpecimen Saved = Sim.GetUfo(0).Destination;
	Sim.ApplyApproachTemplate(AlienIdleTuning::EApproach::Bulk);
	Sim.SetAutoLaunch(0, false);
	TestTrue(TEXT("Preset applies"), Sim.ApplyPreset(0));
	TestTrue(TEXT("Auto-launch restored"), Sim.GetUfo(0).bAutoLaunch);
	TestEqual(TEXT("Target restored"), Sim.GetUfo(0).Destination, Saved);
	TestFalse(TEXT("Empty slot does nothing"), Sim.ApplyPreset(1));

	// Presets survive a save and a promotion.
	TArray<uint8> Bytes;
	Sim.SaveToBytes(Bytes);
	FAlienIdleSim Loaded;
	Loaded.LoadFromBytes(Bytes);
	TestTrue(TEXT("Preset saved to disk"), Loaded.GetCareer().Presets[0].bSaved);
	Loaded.DevCompleteThesis();
	FNextAssignment Next;
	Next.PermanentOption = 0;
	Loaded.Promote(Next);
	TestTrue(TEXT("Preset kept after promotion"), Loaded.ApplyPreset(0));

	const FString Why = Sim.ExplainTarget(0);
	AddInfo(Why);
	TestTrue(TEXT("Explains research"), Why.Contains(TEXT("Research per")));
	TestTrue(TEXT("Explains suspicion"), Why.Contains(TEXT("Suspicion per trip")));

	// Behaviour memos describe what the base actually does.
	FAlienIdleSim Stealthy;
	Stealthy.DevSetIncidentsEnabled(false);
	Stealthy.DevAddResearch(5000.0);
	Stealthy.BuyShipUpgrade(0, EShipSystem::Stealth);
	Stealthy.BuyShipUpgrade(0, EShipSystem::Stealth);
	Stealthy.DevAddExperiments(ESpecimen::Squirrel, 12);
	bool bClear = false;
	TestEqual(TEXT("Stealth fleet reads as covert"), Stealthy.ClassifyApproach(&bClear), AlienIdleTuning::EApproach::Covert);
	Stealthy.Tick(AlienIdleTuning::BehaviourMemoSeconds + 1.0f);
	bool bMemo = false;
	for (const auto& Line : Stealthy.GetLog())
	{
		bMemo |= Line.Text.Contains(TEXT("DEPARTMENT MEMO (Covert"));
	}
	TestTrue(TEXT("Covert memo filed"), bMemo);
	return true;
}

namespace
{
	/** A fresh run at a location after a (dev-completed) first thesis, keeping nothing. */
	void StartAt(FAlienIdleSim& Sim, ELocation Where, EThesis Thesis = EThesis::PreliminaryObservations)
	{
		Sim.DevUnlockLocation(Where);
		Sim.DevCompleteThesis();
		FNextAssignment Next;
		Next.Thesis = Thesis;
		Next.Location = Where;
		Next.Condition = EEarthCondition::BusinessAsUsual;
		Next.PermanentOption = Sim.BuildPromotionReview().Options.Num() > 0 ? 0 : INDEX_NONE;
		Sim.Promote(Next);
	}

	struct FOpening
	{
		TArray<FString> Purchases;
		TArray<FString> Posts;
		TArray<FString> Discoveries;
		double Research = 0.0;
		float CreakingSeconds = 0.0f;
		FString Note;
	};

	FOpening PlayOpening(ELocation Where, double Minutes, bool bIgnoreHull = false)
	{
		FAlienIdleSim Sim;
		StartAt(Sim, Where);
		FOpening Out;
		GBotPurchases = &Out.Purchases;
		GBotIgnoresHull = bIgnoreHull;
		for (double m = 0.0; m < Minutes; m += 1.0 / 6.0)
		{
			RunBotOn(Sim, EBotStrategy::Balanced, 1.0 / 6.0, EDepartment::Biological);
			Out.CreakingSeconds += Sim.GetHullPressure() >= AlienIdleTuning::HullCreakPressure ? 10.0f : 0.0f;
		}
		GBotPurchases = nullptr;
		GBotIgnoresHull = false;
		for (const FAssignment& Station : Sim.GetAllStations())
		{
			if (Sim.IsStationStaffed(Station))
			{
				Out.Posts.Add(FString::Printf(TEXT("%s x%d"), *Sim.GetStationName(Station), Sim.GetStationCrew(Station)));
			}
		}
		for (int32 d = 0; d < int32(EDiscovery::Count); ++d)
		{
			if (Sim.IsDiscovered(EDiscovery(d)))
			{
				Out.Discoveries.Add(AlienIdleTuning::Discoveries[d].Name);
			}
		}
		Out.Research = Sim.GetTotalResearchEarned();
		Out.Note = FString::Printf(TEXT("%s | %s | energy %.0f/%.0f +%.1f | %s"), *Sim.GetBottleneckText(), *Sim.GetUfoStatusText(0), Sim.GetEnergy(), Sim.GetEnergyCap(),
			Sim.GetEnergyPerSecond(), Sim.HasHeat() ? *Sim.GetHeatText() : Sim.HasHull() ? *Sim.GetHullText() : TEXT(""));
		return Out;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleOceanicHullTest, "AlienIdle.Sim.OceanicHullPressure", SimTestFlags)
bool FAlienIdleOceanicHullTest::RunTest(const FString& Parameters)
{
	FAlienIdleSim Sim;
	Sim.DevSetIncidentsEnabled(false);
	StartAt(Sim, ELocation::Oceanic);
	TestEqual(TEXT("Oceanic posting"), Sim.GetLocation(), ELocation::Oceanic);
	TestTrue(TEXT("Pufferfish from the start"), Sim.IsSpecimenUnlocked(ESpecimen::Pufferfish));
	TestFalse(TEXT("No elephants under the sea"), Sim.IsSpecimenUnlocked(ESpecimen::Elephant));
	TestTrue(TEXT("Pressure Control is a post"), Sim.GetAllStations().Contains(FAssignment{ EStation::Pressure, 0 }));
	TestTrue(TEXT("Exclusive facilities available"), Sim.IsBaseUpgradeAvailable(EBaseUpgrade::TidalTurbines) && Sim.IsBaseUpgradeAvailable(EBaseUpgrade::DeepTanks));

	// Surface expeditions cost double energy and load the hull.
	Sim.SetDestinationRegion(0, 1, ESpecimen::Squirrel);
	const double SurfaceCost = Sim.GetLaunchEnergy(0, ESpecimen::Squirrel);
	Sim.SetDestinationRegion(0, 0, ESpecimen::Pufferfish);
	const double DiveCost = Sim.GetLaunchEnergy(0, ESpecimen::Pufferfish);
	AddInfo(FString::Printf(TEXT("Launch energy: squirrel by surface expedition %.0f, pufferfish dive %.0f"), SurfaceCost, DiveCost));
	TestTrue(TEXT("Surface expeditions cost double"), SurfaceCost >= 2.0 * AlienIdleTuning::Specimen(ESpecimen::Squirrel).LaunchEnergy);
	Sim.SetDestinationRegion(0, 1, ESpecimen::Squirrel);
	Sim.DevAddEnergy(1000.0);
	const float Before = Sim.GetHullPressure();
	Sim.Launch(0);
	TestTrue(TEXT("Airlock cycle loads the hull"), Sim.GetHullPressure() >= Before + AlienIdleTuning::HullStressPerSurfaceLaunch - 0.1f);

	// Where pressure settles: full containment without and with a technician on Pressure Control.
	auto Settled = [](bool bStaffed)
	{
		FAlienIdleSim Base;
		Base.DevSetIncidentsEnabled(false);
		StartAt(Base, ELocation::Oceanic);
		Base.SetAutoAssign(false);
		Base.DevAddResearch(5000.0);
		Base.BuyBaseUpgrade(EBaseUpgrade::DeepTanks);
		Base.BuyBaseUpgrade(EBaseUpgrade::DeepTanks);
		Base.Recruit();
		if (bStaffed)
		{
			Base.Assign(Base.GetCrew().Num() - 1, { EStation::Pressure, 0 });
		}
		for (int32 c = 0; c < Base.GetCrew().Num(); ++c)
		{
			if (Base.GetCrew()[c].Assignment.Station == EStation::Chamber)
			{
				Base.Assign(c, FAssignment()); // nobody studies, so containment stays full
			}
		}
		for (int32 i = 0; i < 12; ++i)
		{
			Base.DevAddToContainment({ ESpecimen::Pufferfish });
		}
		Base.Tick(400.0f);
		return Base.GetHullPressure();
	};
	const float Unstaffed = Settled(false);
	const float Staffed = Settled(true);
	AddInfo(FString::Printf(TEXT("Hull with 12 units held and 2 Deep-Water Tanks: %.0f unstaffed, %.0f with a technician on Pressure Control"), Unstaffed, Staffed));
	TestTrue(TEXT("Unstaffed hull creaks"), Unstaffed >= AlienIdleTuning::HullCreakPressure);
	TestTrue(TEXT("A technician keeps it calm"), Staffed < AlienIdleTuning::HullCreakPressure * 0.7f);

	// A leak seals the airlock (soft and temporary): dives continue, surface trips wait.
	FAlienIdleSim Leak;
	Leak.DevSetIncidentsEnabled(false);
	StartAt(Leak, ELocation::Oceanic);
	Leak.DevSetHullPressure(95.0f);
	Leak.Tick(0.2f);
	Leak.DevAddEnergy(1000.0);
	Leak.SetDestinationRegion(0, 1, ESpecimen::Squirrel);
	FString Why;
	TestFalse(TEXT("Airlock sealed"), Leak.CanLaunch(0, &Why));
	AddInfo(TEXT("Sealed: ") + Why);
	Leak.SetDestinationRegion(0, 0, ESpecimen::Pufferfish);
	TestTrue(TEXT("Dives still go"), Leak.CanLaunch(0));
	Leak.Tick(300.0f);
	TestFalse(TEXT("Airlock reopens on its own"), Leak.IsAirlockSealed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleOceanicOpeningTest, "AlienIdle.Sim.OceanicChangesTheOpening", SimTestFlags)
bool FAlienIdleOceanicOpeningTest::RunTest(const FString& Parameters)
{
	const FOpening Mountain = PlayOpening(ELocation::Mountain, 20.0);
	const FOpening Ocean = PlayOpening(ELocation::Oceanic, 20.0);
	const FOpening Careless = PlayOpening(ELocation::Oceanic, 20.0, true);
	auto Show = [this](const TCHAR* Label, const FOpening& O)
	{
		TArray<FString> First;
		for (int32 i = 0; i < FMath::Min(10, O.Purchases.Num()); ++i)
		{
			First.Add(O.Purchases[i]);
		}
		AddInfo(FString::Printf(TEXT("%s: research %.0f, hull creaking %.0f s; first buys: %s; posts: %s; discoveries: %s"), Label, O.Research, O.CreakingSeconds, *FString::Join(First, TEXT(", ")),
			*FString::Join(O.Posts, TEXT(", ")), *FString::Join(O.Discoveries, TEXT(", "))));
	};
	Show(TEXT("Mountain"), Mountain);
	Show(TEXT("Oceanic"), Ocean);
	Show(TEXT("Oceanic, nobody on Pressure Control"), Careless);
	TestTrue(TEXT("Opening purchases differ: tidal turbines or deep tanks"), Ocean.Purchases.Contains(TEXT("Tidal Turbines")) || Ocean.Purchases.Contains(TEXT("Deep-Water Tanks")));
	TestTrue(TEXT("Crew assignments differ: Pressure Control staffed"), Ocean.Posts.ContainsByPredicate([](const FString& P) { return P.StartsWith(TEXT("Pressure Control")); }));
	auto Aquatic = [](const FOpening& O)
	{
		int32 Count = 0;
		for (const TCHAR* Name : { TEXT("Inflation Policy"), TEXT("Eight-Armed Bureaucracy"), TEXT("Ink Diplomacy"), TEXT("Whale Song Transcripts") })
		{
			Count += O.Discoveries.Contains(Name) ? 1 : 0;
		}
		return Count;
	};
	TestTrue(TEXT("Research priorities differ: the aquatic chain"), Aquatic(Ocean) >= 2 && Aquatic(Mountain) == 0);
	TestTrue(TEXT("Leaving Pressure Control empty costs research"), Careless.Research < Ocean.Research * 0.95);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleOceanicLoopTest, "AlienIdle.Sim.OceanicPrestigeLoop", SimTestFlags)
bool FAlienIdleOceanicLoopTest::RunTest(const FString& Parameters)
{
	// Run 1 at the mountain until the thesis is done and the Academy achievement opens Oceanic.
	FAlienIdleSim Sim;
	double Run1 = 0.0;
	while ((!Sim.CanSubmitThesis() || !Sim.IsLocationUnlocked(ELocation::Oceanic)) && Run1 < 60.0)
	{
		RunBotOn(Sim, EBotStrategy::BulkAnimals, 1.0, EDepartment::Biological);
		Run1 += 1.0;
	}
	AddInfo(FString::Printf(TEXT("Run 1: %.0f min, %d elephant experiments, Oceanic %s"), Run1, Sim.GetExperimentsCompleted(ESpecimen::Elephant),
		Sim.IsLocationUnlocked(ELocation::Oceanic) ? TEXT("unlocked") : TEXT("locked")));
	TestTrue(TEXT("Achievement unlocks Oceanic in run 1"), Sim.IsLocationUnlocked(ELocation::Oceanic));
	TestTrue(TEXT("The Deep End is offered"), Sim.GetNextThesisOffers().Contains(EThesis::DeepEnd));
	if (!Sim.CanSubmitThesis() || !Sim.IsLocationUnlocked(ELocation::Oceanic))
	{
		return true;
	}
	FNextAssignment Next;
	Next.Thesis = EThesis::DeepEnd;
	Next.Location = ELocation::Mountain; // the thesis insists on Oceanic
	Next.Condition = EEarthCondition::BusinessAsUsual;
	Next.PermanentOption = 0;
	TestTrue(TEXT("Promote to The Deep End"), Sim.Promote(Next));
	TestEqual(TEXT("The thesis takes you to Oceanic"), Sim.GetLocation(), ELocation::Oceanic);

	// Run 2 under the sea.
	double Run2 = 0.0;
	float PeakHull = 0.0f;
	while (!Sim.CanSubmitThesis() && Run2 < 60.0)
	{
		RunBotOn(Sim, EBotStrategy::Balanced, 1.0, EDepartment::Biological);
		Run2 += 1.0;
		PeakHull = FMath::Max(PeakHull, Sim.GetHullPressure());
		if (FMath::Fmod(Run2, 10.0) == 0.0)
		{
			AddInfo(FString::Printf(TEXT("  @%.0f min: %s"), Run2, *Sim.GetHullText()));
		}
	}
	TArray<FString> Objectives;
	for (const FThesisObjective& Objective : Sim.GetThesisObjectives())
	{
		Objectives.Add(FString::Printf(TEXT("%s%s"), Objective.bDone ? TEXT("done: ") : TEXT(""), *Objective.Text));
	}
	const FPromotionReview Review = Sim.BuildPromotionReview();
	AddInfo(FString::Printf(TEXT("Run 2 (The Deep End): %.0f min, peak hull %.0f; %s"), Run2, PeakHull, *FString::Join(Objectives, TEXT("; "))));
	AddInfo(FString::Join(Review.Stats, TEXT(" | ")));
	TestTrue(TEXT("The Deep End completes within 45 minutes"), Sim.CanSubmitThesis() && Run2 <= 45.0);
	const int32 Perk = Review.Options.IndexOfByPredicate([](const FPermanentOption& O) { return O.Perk == ELocation::Oceanic; });
	TestTrue(TEXT("Submersible Hulls offered"), Perk != INDEX_NONE);
	if (!Sim.CanSubmitThesis() || Perk == INDEX_NONE)
	{
		return true;
	}
	const int32 Slots = Sim.GetShipSlots();

	// Back to the mountain: what Oceanic taught carries over.
	Next.Thesis = EThesis::CorporateWantsResults;
	Next.Location = ELocation::Mountain;
	Next.PermanentOption = Perk;
	TestTrue(TEXT("Promote back to the mountain"), Sim.Promote(Next));
	TestEqual(TEXT("Submersible Hulls: +1 slot everywhere"), Sim.GetShipSlots(), Slots + 1 + AlienIdleTuning::Thesis(EThesis::CorporateWantsResults).ExtraSlots);
	TestTrue(TEXT("An Oceanic discovery opens a new upgrade at the mountain (Ink Screens or Sonar Array)"),
		Sim.IsBaseUpgradeAvailable(EBaseUpgrade::InkScreens) || Sim.IsBaseUpgradeAvailable(EBaseUpgrade::SonarArray));
	TestFalse(TEXT("Tidal turbines stay at sea"), Sim.IsBaseUpgradeAvailable(EBaseUpgrade::TidalTurbines));
	TestFalse(TEXT("No pressure post on land"), Sim.GetAllStations().Contains(FAssignment{ EStation::Pressure, 0 }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleAntarcticHeatTest, "AlienIdle.Sim.AntarcticHeat", SimTestFlags)
bool FAlienIdleAntarcticHeatTest::RunTest(const FString& Parameters)
{
	FAlienIdleSim Sim;
	Sim.DevSetIncidentsEnabled(false);
	StartAt(Sim, ELocation::Antarctic);
	TestEqual(TEXT("Antarctic posting"), Sim.GetLocation(), ELocation::Antarctic);
	TestTrue(TEXT("Penguins from the start"), Sim.IsSpecimenUnlocked(ESpecimen::Penguin));
	TestFalse(TEXT("No squirrels on the ice"), Sim.IsSpecimenUnlocked(ESpecimen::Squirrel));
	TestEqual(TEXT("Acorn Amnesia is studied on penguins here"), Sim.GetDiscoverySpecimen(EDiscovery::AcornAmnesia), ESpecimen::Penguin);
	TestTrue(TEXT("Boiler Room is a post"), Sim.GetAllStations().Contains(FAssignment{ EStation::Boiler, 0 }));

	// Heat distribution: a cold base slows labs or flights depending on the priority.
	Sim.SetAutoAssign(false);
	for (int32 c = 0; c < Sim.GetCrew().Num(); ++c)
	{
		if (Sim.GetCrew()[c].Assignment.Station == EStation::Boiler)
		{
			Sim.Assign(c, FAssignment());
		}
	}
	Sim.DevAddResearch(5000.0);
	Sim.BuyBaseUpgrade(EBaseUpgrade::Chambers);
	Sim.BuyBaseUpgrade(EBaseUpgrade::Hangar);
	const float ColdLabs = Sim.GetLabSpeed();
	const float ColdFlight = Sim.GetFlightSeconds(0, ESpecimen::Penguin);
	Sim.SetHeatPriority(EHeatPriority::LabsFirst);
	const float LabsFirstLabs = Sim.GetLabSpeed();
	const float LabsFirstFlight = Sim.GetFlightSeconds(0, ESpecimen::Penguin);
	AddInfo(Sim.GetHeatText());
	Sim.SetHeatPriority(EHeatPriority::Even);
	const int32 Tech = FMath::Max(0, Sim.GetCrew().IndexOfByPredicate([](const FCrewMember& M) { return M.Assignment.Station == EStation::None || M.Specialty == ESpecialty::Technician; }));
	Sim.Assign(Tech, { EStation::Boiler, 0 });
	const float WarmLabs = Sim.GetLabSpeed();
	AddInfo(FString::Printf(TEXT("Labs x%.2f cold (even split), x%.2f labs-first (flights %.1f s -> %.1f s), x%.2f with the Boiler Room staffed"), ColdLabs, LabsFirstLabs, ColdFlight, LabsFirstFlight, WarmLabs));
	TestTrue(TEXT("Labs-first warms the labs"), LabsFirstLabs > ColdLabs);
	TestTrue(TEXT("...and freezes the hangar"), LabsFirstFlight > ColdFlight);
	TestTrue(TEXT("A Boiler Room technician warms the base"), WarmLabs > ColdLabs);

	// Preserved samples pay more.
	FSpecimen Kept{ ESpecimen::Penguin, EOccupation::None, 0 };
	Kept.Held = AlienIdleTuning::PreserveSeconds;
	Sim.DevAddToContainment(Kept);
	TestTrue(TEXT("Preservation counts at the Antarctic"), Sim.IsPreserving());

	// Long supply routes: everything costs more; recruits arrive trained.
	FAlienIdleSim Mountain;
	StartAt(Mountain, ELocation::Mountain);
	TestTrue(TEXT("Upgrades cost more on the ice"), Sim.GetShipUpgradeCost(0, EShipSystem::Thrusters) > Mountain.GetShipUpgradeCost(0, EShipSystem::Thrusters));
	const int32 Before = Sim.GetCrew().Num();
	Sim.Recruit();
	TestTrue(TEXT("Recruits arrive trained"), Sim.GetCrew().Num() == Before + 1 && Sim.GetCrew().Last().Training >= 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleAntarcticOpeningTest, "AlienIdle.Sim.AntarcticChangesTheOpening", SimTestFlags)
bool FAlienIdleAntarcticOpeningTest::RunTest(const FString& Parameters)
{
	const FOpening Mountain = PlayOpening(ELocation::Mountain, 20.0);
	const FOpening Ice = PlayOpening(ELocation::Antarctic, 20.0);
	const FOpening Careless = PlayOpening(ELocation::Antarctic, 20.0, true);
	auto Show = [this](const TCHAR* Label, const FOpening& O)
	{
		TArray<FString> First;
		for (int32 i = 0; i < FMath::Min(10, O.Purchases.Num()); ++i)
		{
			First.Add(O.Purchases[i]);
		}
		AddInfo(FString::Printf(TEXT("%s: research %.0f; first buys: %s; posts: %s; discoveries: %s"), Label, O.Research, *FString::Join(First, TEXT(", ")),
			*FString::Join(O.Posts, TEXT(", ")), *FString::Join(O.Discoveries, TEXT(", "))));
	};
	Show(TEXT("Mountain"), Mountain);
	Show(TEXT("Antarctic"), Ice);
	Show(TEXT("Antarctic, nobody in the Boiler Room"), Careless);
	AddInfo(TEXT("Antarctic end state: ") + Ice.Note);
	AddInfo(TEXT("Careless end state: ") + Careless.Note);
	TestTrue(TEXT("Opening purchases differ: geothermal taps or a recruit for the Boiler Room"),
		Ice.Purchases.Contains(TEXT("Geothermal Taps")) || Ice.Purchases.Contains(TEXT("Recruit for the Boiler Room")));
	TestTrue(TEXT("Crew assignments differ: Boiler Room staffed"), Ice.Posts.ContainsByPredicate([](const FString& P) { return P.StartsWith(TEXT("Boiler Room")); }));
	TestTrue(TEXT("Research priorities differ: Huddle Theory or Blubber Economics"), Ice.Discoveries.Contains(TEXT("Huddle Theory")) || Ice.Discoveries.Contains(TEXT("Blubber Economics")));
	TestTrue(TEXT("Leaving the Boiler Room empty costs research"), Careless.Research < Ice.Research * 0.95);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleAntarcticLoopTest, "AlienIdle.Sim.AntarcticPrestigeLoop", SimTestFlags)
bool FAlienIdleAntarcticLoopTest::RunTest(const FString& Parameters)
{
	// Two theses earn tenure (the Antarctic achievement), then Cold Storage on the ice.
	FAlienIdleSim Sim;
	for (int32 Thesis = 0; Thesis < AlienIdleTuning::TenureTrackTheses; ++Thesis)
	{
		Sim.DevCompleteThesis();
		FNextAssignment Next;
		Next.Thesis = EThesis::CorporateWantsResults;
		Next.Condition = EEarthCondition::BusinessAsUsual;
		Next.PermanentOption = Sim.BuildPromotionReview().Options.Num() > 0 ? 0 : INDEX_NONE;
		Sim.Promote(Next);
	}
	Sim.Tick(1.0f);
	TestTrue(TEXT("Tenure Track unlocks the Antarctic"), Sim.IsLocationUnlocked(ELocation::Antarctic));
	TestTrue(TEXT("Cold Storage is offered"), Sim.GetNextThesisOffers().Contains(EThesis::ColdStorage));
	Sim.DevCompleteThesis();
	FNextAssignment Next;
	Next.Thesis = EThesis::ColdStorage;
	Next.Condition = EEarthCondition::BusinessAsUsual;
	Next.PermanentOption = Sim.BuildPromotionReview().Options.Num() > 0 ? 0 : INDEX_NONE;
	TestTrue(TEXT("Promote to Cold Storage"), Sim.Promote(Next));
	TestEqual(TEXT("The thesis takes you to the Antarctic"), Sim.GetLocation(), ELocation::Antarctic);

	double Minutes = 0.0;
	while (!Sim.CanSubmitThesis() && Minutes < 60.0)
	{
		RunBotOn(Sim, EBotStrategy::Balanced, 1.0, EDepartment::Biological);
		Minutes += 1.0;
		if (FMath::Fmod(Minutes, 10.0) == 0.0)
		{
			AddInfo(FString::Printf(TEXT("  @%.0f min: %s"), Minutes, *Sim.GetHeatText()));
		}
	}
	TArray<FString> Objectives;
	for (const FThesisObjective& Objective : Sim.GetThesisObjectives())
	{
		Objectives.Add(FString::Printf(TEXT("%s%s"), Objective.bDone ? TEXT("done: ") : TEXT(""), *Objective.Text));
	}
	const FPromotionReview Review = Sim.BuildPromotionReview();
	AddInfo(FString::Printf(TEXT("Cold Storage: %.0f min; %s"), Minutes, *FString::Join(Objectives, TEXT("; "))));
	AddInfo(FString::Join(Review.Stats, TEXT(" | ")));
	TestTrue(TEXT("Cold Storage completes within 45 minutes"), Sim.CanSubmitThesis() && Minutes <= 45.0);
	const int32 Perk = Review.Options.IndexOfByPredicate([](const FPermanentOption& O) { return O.Perk == ELocation::Antarctic; });
	TestTrue(TEXT("Polar Crew Program offered"), Perk != INDEX_NONE);
	if (!Sim.CanSubmitThesis() || Perk == INDEX_NONE)
	{
		return true;
	}
	Next.Thesis = EThesis::ControlledExperiment;
	Next.Location = ELocation::Mountain;
	Next.PermanentOption = Perk;
	TestTrue(TEXT("Promote back to the mountain"), Sim.Promote(Next));
	Sim.DevAddResearch(1000.0);
	Sim.Recruit();
	TestTrue(TEXT("Polar Crew Program: trained recruits at the mountain"), Sim.GetCrew().Last().Training >= 1);
	TestTrue(TEXT("An Antarctic discovery opens a new upgrade at the mountain"),
		Sim.IsBaseUpgradeAvailable(EBaseUpgrade::InsulatedLabs) || Sim.IsBaseUpgradeAvailable(EBaseUpgrade::CryoPods));
	TestFalse(TEXT("No Boiler Room on the mountain"), Sim.GetAllStations().Contains(FAssignment{ EStation::Boiler, 0 }));
	return true;
}

// ---------------------------------------------------------------------------------------------
// The Specimens Remember: recurring subjects and case files

namespace
{
	bool ResultsContain(const FAlienIdleSim& Sim, const TCHAR* Text)
	{
		return Sim.GetResults().ContainsByPredicate([Text](const FAlienIdleLogEntry& E) { return E.Text.Contains(Text); });
	}

	bool LogContains(const FAlienIdleSim& Sim, const TCHAR* Text)
	{
		return Sim.GetLog().ContainsByPredicate([Text](const FAlienIdleLogEntry& E) { return E.Text.Contains(Text); });
	}

	/** Lets the lab study whoever is in containment until the subject has gone home. */
	bool StudyUntilGone(FAlienIdleSim& Sim, int32 Case)
	{
		return RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return !Sim.IsCaseOnBase(Case); }, 180.0f);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCaseReturningSubjectTest, "AlienIdle.Sim.CaseReturningSubject", SimTestFlags)
bool FAlienIdleCaseReturningSubjectTest::RunTest(const FString& Parameters)
{
	using namespace AlienIdleTuning;
	FQuietSim Sim;
	Sim.DevAddCaseSubject(CaseMarsh, 1);
	TestEqual(TEXT("First encounter"), Sim.GetCaseFile(CaseMarsh).Encounters, 1);
	TestTrue(TEXT("Case file in the encyclopedia"), Sim.GetEncyclopedia().ContainsByPredicate([](const FEncyclopediaEntry& E) { return E.Title == TEXT("Case File: Dr. Edith Marsh"); }));
	TestTrue(TEXT("New case file announced"), ResultsContain(Sim, TEXT("NEW CASE FILE: Dr. Edith Marsh")));
	TestEqual(TEXT("Label is the name"), FAlienIdleSim::GetSpecimenLabel(Sim.GetContainment().Last()), FString(TEXT("Dr. Edith Marsh")));
	TestEqual(TEXT("Next step: intake"), Sim.GetNextCaseStep(CaseMarsh), 0);

	double Before = Sim.GetResearch();
	TestTrue(TEXT("Intake interview, then home"), StudyUntilGone(Sim, CaseMarsh));
	TestEqual(TEXT("Intake done"), Sim.GetCaseStepsDone(CaseMarsh), 1);
	TestEqual(TEXT("Intake pays its fixed research"), Sim.GetResearch() - Before, CaseSubjects[CaseMarsh].Steps[0].Research);
	TestEqual(TEXT("One history line"), Sim.GetCaseFile(CaseMarsh).History.Num(), 1);
	TestTrue(TEXT("History records the visit"), Sim.GetCaseFile(CaseMarsh).History[0].Contains(TEXT("Intake Interview")));

	// She comes back: the player recognises her, and she recognises the equipment.
	Sim.DevAddCaseSubject(CaseMarsh, 1);
	TestEqual(TEXT("Second encounter"), Sim.GetCaseFile(CaseMarsh).Encounters, 2);
	TestTrue(TEXT("Returning subject announced"), ResultsContain(Sim, TEXT("RETURNING SUBJECT: Dr. Edith Marsh (encounter 2)")));
	TestEqual(TEXT("Next step: needs a return visit"), Sim.GetNextCaseStep(CaseMarsh), 1);
	TestEqual(TEXT("Recognises the equipment: research x1.25"), Sim.GetCaseResearchMultiplier(Sim.GetContainment().Last()), 1.25);
	Before = Sim.GetResearch();
	TestTrue(TEXT("Calibration study, then home"), StudyUntilGone(Sim, CaseMarsh));
	TestEqual(TEXT("Calibration pays more"), Sim.GetResearch() - Before, CaseSubjects[CaseMarsh].Steps[1].Research);
	TestEqual(TEXT("Final step waits for a second assignment"), Sim.GetNextCaseStep(CaseMarsh), int32(INDEX_NONE));
	TestTrue(TEXT("Console says what the last step needs"), Sim.GetCaseFileText(CaseMarsh).Contains(TEXT("second assignment")));

	// Third visit this run: a routine study, better each time.
	Sim.DevAddCaseSubject(CaseMarsh, 1);
	TestEqual(TEXT("Research x1.5 on the third visit"), Sim.GetCaseResearchMultiplier(Sim.GetContainment().Last()), 1.5);
	Before = Sim.GetResearch();
	TestTrue(TEXT("Routine study, then home"), StudyUntilGone(Sim, CaseMarsh));
	const double Routine = Specimen(ESpecimen::Human).Research * Occupation(EOccupation::Astronomer).ResearchMultiplier * RegionDefs[0][1].Value;
	TestEqual(TEXT("Routine study pays the recognition bonus"), Sim.GetResearch() - Before, FMath::RoundToDouble(Routine * 1.5));
	TestEqual(TEXT("Three visits on file"), Sim.GetCaseFile(CaseMarsh).History.Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCaseScanningTest, "AlienIdle.Sim.CaseTargetedScanning", SimTestFlags)
bool FAlienIdleCaseScanningTest::RunTest(const FString& Parameters)
{
	using namespace AlienIdleTuning;
	FQuietSim Sim;
	FString Reason;
	TestFalse(TEXT("Needs humans first"), Sim.CanTargetCase(0, CasePell, &Reason));
	Sim.DevDiscover(EDiscovery::TotalRecall);
	TestFalse(TEXT("Needs a scanner"), Sim.CanTargetCase(0, CasePell, &Reason));
	TestTrue(TEXT("Reason names the scanner"), Reason.Contains(TEXT("Trait Scanner")));
	BuyShip(Sim, 0, EShipSystem::Scanner, 1);
	TestTrue(TEXT("Can scan for an unidentified lead"), Sim.CanTargetCase(0, CasePell));
	TestTrue(TEXT("Lead text before meeting"), Sim.GetCaseFileText(CasePell).Contains(TEXT("Unidentified lead")));

	Sim.SetCaseTarget(0, CasePell);
	TestEqual(TEXT("Ship now targets humans"), Sim.GetUfo(0).Destination, ESpecimen::Human);
	Sim.DevAddEnergy(100.0);
	TestTrue(TEXT("Launch"), Sim.Launch(0));
	TestEqual(TEXT("This trip scans for Pell"), int32(Sim.GetUfo(0).TripCase), CasePell);
	TestTrue(TEXT("Pell is found (no luck involved)"), RunUntil(Sim, [&] { return Sim.IsCaseOnBase(CasePell); }, 60.0f));
	TestEqual(TEXT("Scanner target cleared"), int32(Sim.GetUfo(0).CaseTarget), -1);
	TestEqual(TEXT("Met once"), Sim.GetCaseFile(CasePell).Encounters, 1);
	TestTrue(TEXT("Studied and returned"), StudyUntilGone(Sim, CasePell));

	// Harder to fool: scanning for him again takes longer.
	Sim.SetCaseTarget(0, CasePell);
	RunUntil(Sim, [&] { return Sim.GetUfo(0).State == EUfoState::Docked; }, 60.0f);
	const float Sought = Sim.GetCaptureSeconds(0, ESpecimen::Human, true);
	Sim.SetCaseTarget(0, -1);
	const float Anyone = Sim.GetCaptureSeconds(0, ESpecimen::Human, true);
	TestTrue(TEXT("A returning subject hides better"), Sought > Anyone);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCaseColleaguesTest, "AlienIdle.Sim.CaseColleagues", SimTestFlags)
bool FAlienIdleCaseColleaguesTest::RunTest(const FString& Parameters)
{
	using namespace AlienIdleTuning;
	FQuietSim Sim;
	Sim.SetAutoAssign(false);
	Sim.Assign(1, FAssignment()); // no scientist: they stay in containment together
	Sim.DevAddCaseSubject(CaseDelgado, 1);
	TestTrue(TEXT("Relationship revealed once one has a file"), Sim.GetCaseRelationText(CasePell).Num() > 0);
	TestTrue(TEXT("Console warns before taking Pell"), Sim.GetCaseRelationText(CasePell)[0].Contains(TEXT("WILL send investigators")));
	const float Before = Sim.GetResponsePressure(1, EResponse::Investigators);
	Sim.DevAddCaseSubject(CasePell, 1);
	TestTrue(TEXT("Delgado goes digging"), Sim.GetResponsePressure(1, EResponse::Investigators) >= Before + ColleagueInvestigatorPressure - 0.01f);
	TestTrue(TEXT("Logged"), LogContains(Sim, TEXT("started asking questions")));
	TestTrue(TEXT("Held together, they start a newsletter"), RunUntil(Sim, [&] { return Sim.GetIncident().Type == EIncident::ContainmentNewsletter; }, CaseTroubleSeconds + 5.0f));
	TestTrue(TEXT("Labs slowed"), Sim.GetLabSpeed() < 1.0f);
	TestTrue(TEXT("It resolves itself"), RunUntil(Sim, [&] { return Sim.GetIncident().Type == EIncident::None; }, NewsletterSeconds + 5.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCasePeerReviewTest, "AlienIdle.Sim.CaseRivalsPeerReview", SimTestFlags)
bool FAlienIdleCasePeerReviewTest::RunTest(const FString& Parameters)
{
	using namespace AlienIdleTuning;
	{
		// Left waiting together with no free chamber, rivals argue.
		FQuietSim Sim;
		Sim.SetAutoAssign(false);
		Sim.Assign(1, FAssignment());
		Sim.DevAddCaseSubject(CaseMarsh);
		Sim.DevAddCaseSubject(CasePike);
		TestTrue(TEXT("Academic dispute"), RunUntil(Sim, [&] { return Sim.GetIncident().Type == EIncident::AcademicDispute; }, CaseTroubleSeconds + 5.0f));
	}
	FQuietSim Sim;
	Sim.DevDiscover(EDiscovery::TotalRecall);
	BuyShip(Sim, 0, EShipSystem::Scanner, 1);
	Sim.DevAddCaseSubject(CaseMarsh);
	// While a ship is out looking for Pike, the lab holds Dr. Marsh for him.
	Sim.SetCaseTarget(0, CasePike);
	Sim.Tick(5.0f);
	TestFalse(TEXT("Marsh waits for her rival"), Sim.GetChambers()[0].bRunning);
	TestTrue(TEXT("Console says she is waiting"), Sim.GetCaseFileText(CaseMarsh).Contains(TEXT("waiting for their rival")));
	Sim.DevAddEnergy(100.0);
	TestTrue(TEXT("Launch for Pike"), Sim.Launch(0));
	TestTrue(TEXT("Peer Review starts when Pike arrives"), RunUntil(Sim, [&]
	{
		Sim.DevAddEnergy(100.0);
		return Sim.GetChambers()[0].bRunning && Sim.GetChambers()[0].Experiment.CaseStep == PeerReviewStep;
	}, 90.0f));
	TestEqual(TEXT("Both in one chamber"), int32(Sim.GetChambers()[0].Partner.Case), CasePike);
	const int32 Experiments = Sim.GetTotalExperiments();
	const double Before = Sim.GetResearch();
	TestTrue(TEXT("Peer Review completes"), RunUntil(Sim, [&] { Sim.DevAddEnergy(100.0); return Sim.IsPeerReviewDone(); }, PeerReviewSeconds + 10.0f));
	TestEqual(TEXT("Unique experiment pays"), Sim.GetResearch() - Before, PeerReviewResearch);
	TestEqual(TEXT("Two subjects studied"), Sim.GetTotalExperiments(), Experiments + 2);
	TestTrue(TEXT("Filed in the encyclopedia"), Sim.GetEncyclopedia().ContainsByPredicate([](const FEncyclopediaEntry& E) { return E.Title == TEXT("Case File: Peer Review"); }));
	TestFalse(TEXT("Both went home"), Sim.IsCaseOnBase(CaseMarsh) || Sim.IsCaseOnBase(CasePike));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCaseKindredTest, "AlienIdle.Sim.CaseKindred", SimTestFlags)
bool FAlienIdleCaseKindredTest::RunTest(const FString& Parameters)
{
	using namespace AlienIdleTuning;
	FQuietSim Sim;
	Sim.SetAutoAssign(false);
	Sim.Assign(1, FAssignment());
	Sim.DevAddCaseSubject(CasePell);
	TestEqual(TEXT("Alone: no bonus"), Sim.GetCaseResearchMultiplier(Sim.GetContainment().Last()), 1.0);
	Sim.DevAddCaseSubject(CaseFenn);
	TestEqual(TEXT("With Fenn: research x1.3"), Sim.GetCaseResearchMultiplier(Sim.GetContainment()[0]), KindredResearchMultiplier);
	TestEqual(TEXT("Fenn too"), Sim.GetCaseResearchMultiplier(Sim.GetContainment().Last()), KindredResearchMultiplier);
	Sim.Tick(CaseTroubleSeconds + 5.0f);
	TestEqual(TEXT("Kindred pairs cause no incident"), Sim.GetIncident().Type, EIncident::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAlienIdleCaseFilesPrestigeTest, "AlienIdle.Sim.CaseFilesAcrossPrestige", SimTestFlags)
bool FAlienIdleCaseFilesPrestigeTest::RunTest(const FString& Parameters)
{
	using namespace AlienIdleTuning;
	FQuietSim Sim;
	// Run 1: meet Delgado twice.
	for (int32 Visit = 0; Visit < 2; ++Visit)
	{
		Sim.DevAddCaseSubject(CaseDelgado);
		TestTrue(TEXT("Studied"), StudyUntilGone(Sim, CaseDelgado));
	}
	TestEqual(TEXT("Two steps done in run 1"), Sim.GetCaseStepsDone(CaseDelgado), 2);
	TestFalse(TEXT("No discovery in one run"), Sim.HasCaseDiscovery(CaseDelgado));

	Sim.DevCompleteThesis();
	const FPromotionReview Review = Sim.BuildPromotionReview();
	TestTrue(TEXT("Review says case files are kept"), Review.Keeps.ContainsByPredicate([](const FString& S) { return S.Contains(TEXT("Case files")); }));
	FNextAssignment Next;
	Next.Thesis = EThesis::NobodyWillBelieveYou;
	Next.Location = ELocation::Metropolis;
	Next.Condition = EEarthCondition::BusinessAsUsual;
	Next.PermanentOption = Review.Options.Num() > 0 ? 0 : INDEX_NONE;
	FString Reason;
	TestTrue(TEXT("Promote"), Sim.Promote(Next, &Reason));

	// Run 2: the history is kept, this run's case studies are not.
	TestEqual(TEXT("Encounters kept"), Sim.GetCaseFile(CaseDelgado).Encounters, 2);
	TestEqual(TEXT("History kept"), Sim.GetCaseFile(CaseDelgado).History.Num(), 2);
	TestEqual(TEXT("Case studies reset"), Sim.GetCaseStepsDone(CaseDelgado), 0);
	TestFalse(TEXT("Nobody on base"), Sim.IsCaseOnBase(CaseDelgado));
	TestTrue(TEXT("They followed us"), LogContains(Sim, TEXT("followed the sightings")));

	Sim.DevAddCaseSubject(CaseDelgado);
	TestEqual(TEXT("Met on two assignments"), Sim.GetCaseFile(CaseDelgado).RunsMet, 2);
	TestTrue(TEXT("Run 2 visits"), StudyUntilGone(Sim, CaseDelgado));
	Sim.DevAddCaseSubject(CaseDelgado);
	TestTrue(TEXT("Run 2 visits"), StudyUntilGone(Sim, CaseDelgado));
	TestEqual(TEXT("Final step open"), Sim.GetNextCaseStep(CaseDelgado), 2);
	Sim.DevAddCaseSubject(CaseDelgado);
	TestTrue(TEXT("Longitudinal step"), StudyUntilGone(Sim, CaseDelgado));
	TestTrue(TEXT("Case-file discovery"), Sim.HasCaseDiscovery(CaseDelgado));
	TestTrue(TEXT("Announced"), ResultsContain(Sim, TEXT("CASE-FILE DISCOVERY: 'Off the Record'")));
	TestTrue(TEXT("Grant line"), Sim.BuildPromotionReview().GrantLines.ContainsByPredicate([](const FString& S) { return S.Contains(TEXT("Case-file discoveries")); }));

	// Off the Record: investigators build half as fast from now on.
	const float Before = Sim.GetResponsePressure(0, EResponse::Investigators);
	Sim.DevAddCaseSubject(CasePell, 0);
	TestTrue(TEXT("Delgado's corrections halve the digging"),
		FMath::IsNearlyEqual(Sim.GetResponsePressure(0, EResponse::Investigators) - Before, ColleagueInvestigatorPressure * CasePressureMultiplier, 0.01f));

	// And the record survives a save.
	TArray<uint8> Bytes;
	Sim.SaveToBytes(Bytes);
	FAlienIdleSim Loaded;
	TestTrue(TEXT("Load"), Loaded.LoadFromBytes(Bytes));
	TestEqual(TEXT("Saved history"), Loaded.GetCaseFile(CaseDelgado).History.Num(), Sim.GetCaseFile(CaseDelgado).History.Num());
	TestTrue(TEXT("Saved discovery"), Loaded.HasCaseDiscovery(CaseDelgado));
	TestTrue(TEXT("Pell still in containment after load"), Loaded.IsCaseInContainment(CasePell));
	return true;
}

#endif
