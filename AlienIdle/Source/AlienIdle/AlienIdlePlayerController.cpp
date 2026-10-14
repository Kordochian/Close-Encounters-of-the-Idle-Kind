#include "AlienIdlePlayerController.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
	/** No input for this long counts as being away. */
	constexpr double AwayThresholdSeconds = 300.0;
	/** Regular autosave, so a crash or a killed process loses at most this much. */
	constexpr double AutosaveIntervalSeconds = 120.0;
}

AAlienIdlePlayerController::AAlienIdlePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = true;
}

void AAlienIdlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	Sim = MakeShared<FAlienIdleSim>();
	RootWidget = SNew(SAlienIdleRoot).Sim(Sim);
	RootWidget->OnAwayReportDismissed = [this] { OnAwayReportDismissed(); };
	if (FSlateApplication::IsInitialized())
	{
		InputWatcher = MakeShared<FAlienInputWatcher>();
		FSlateApplication::Get().RegisterInputPreProcessor(InputWatcher);
	}

	if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
	{
		Viewport->AddViewportWidgetContent(RootWidget.ToSharedRef(), 10);
	}

	// Continue from the last save, simulating the time the game was closed (-AlienFresh starts a new game).
	const FString SavePath = FAlienIdleSim::GetDefaultSavePath();
	if (!FParse::Param(FCommandLine::Get(), TEXT("AlienFresh")) && IFileManager::Get().FileExists(*SavePath))
	{
		double SecondsClosed = 0.0;
		if (Sim->LoadFromFile(SavePath, SecondsClosed))
		{
			Sim->BeginAwayReport();
			const double Simulated = Sim->CatchUp(SecondsClosed);
			Sim->AddSystemLog(FString::Printf(TEXT("Welcome back. The station ran for %.1f minutes without you%s."), Simulated / 60.0,
				Simulated < SecondsClosed ? TEXT(" (the Academy only pays overtime for 8 hours)") : TEXT("")));
			UE_LOG(LogTemp, Display, TEXT("AlienIdle: loaded save, simulated %.1f of %.1f minutes closed"), Simulated / 60.0, SecondsClosed / 60.0);
			if (Simulated >= 60.0)
			{
				RootWidget->ShowAwayReport(Sim->BuildAwayReport());
			}
			Sim->BeginAwayReport();
		}
		else
		{
			Sim->AddSystemLog(TEXT("The previous save could not be read (it predates the current format). Starting a fresh assignment."));
		}
	}
	LastAutosave = FPlatformTime::Seconds();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(RootWidget->GetInitialFocusWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AAlienIdlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Closing the game saves it; the next launch simulates the time away.
	if (Sim.IsValid() && !FParse::Param(FCommandLine::Get(), TEXT("AlienFresh")))
	{
		Sim->SaveToFile(FAlienIdleSim::GetDefaultSavePath());
	}
	if (InputWatcher.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputWatcher);
		InputWatcher.Reset();
	}
	if (RootWidget.IsValid())
	{
		if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
		{
			Viewport->RemoveViewportWidgetContent(RootWidget.ToSharedRef());
		}
		RootWidget.Reset();
	}
	Super::EndPlay(EndPlayReason);
}

void AAlienIdlePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Sim.IsValid())
	{
		return;
	}
	Sim->Tick(DeltaSeconds);

	if (FPlatformTime::Seconds() - LastAutosave >= AutosaveIntervalSeconds && !FParse::Param(FCommandLine::Get(), TEXT("AlienFresh")))
	{
		LastAutosave = FPlatformTime::Seconds();
		Sim->SaveToFile(FAlienIdleSim::GetDefaultSavePath());
	}

	// "While You Were Out": the report covers everything since the last input before a long gap.
	if (InputWatcher.IsValid())
	{
		const double Now = FPlatformTime::Seconds();
		const double Interaction = InputWatcher->GetLastInputTime();

		// At the idle mark, save before the memo can appear.
		if (!bSavedThisIdle && LastSeenInteraction > 0.0 && Now - Interaction >= AwayThresholdSeconds)
		{
			bSavedThisIdle = Sim->SaveToFile(FAlienIdleSim::GetDefaultSavePath());
			UE_LOG(LogTemp, Display, TEXT("AlienIdle: autosave %s"), bSavedThisIdle ? TEXT("written") : TEXT("FAILED"));
			Sim->AddSystemLog(bSavedThisIdle
				? TEXT("Autosaved: nobody has touched anything for 5 minutes. Management assumes this is a strategy.")
				: TEXT("Autosave failed. Management has filed a ticket with itself."));
		}

		if (Interaction != LastSeenInteraction)
		{
			if (LastSeenInteraction > 0.0 && Interaction - LastSeenInteraction >= AwayThresholdSeconds && RootWidget.IsValid())
			{
				RootWidget->ShowAwayReport(Sim->BuildAwayReport());
				bReloadPending = bSavedThisIdle;
				CatchUpOverride = -1.0;
			}
			LastSeenInteraction = Interaction;
			bSavedThisIdle = false;
			Sim->BeginAwayReport();
		}
	}
}

void AAlienIdlePlayerController::OnAwayReportDismissed()
{
	if (!bReloadPending || !Sim.IsValid())
	{
		return;
	}
	bReloadPending = false;
	const double LiveResearch = Sim->GetResearch();
	const int32 LiveExperiments = Sim->GetTotalExperiments();
	double SecondsSinceSave = 0.0;
	if (!Sim->LoadFromFile(FAlienIdleSim::GetDefaultSavePath(), SecondsSinceSave))
	{
		Sim->AddSystemLog(TEXT("Could not reload the autosave. Carrying on with the live operation."));
		return;
	}
	const double Recalculated = Sim->CatchUp(CatchUpOverride >= 0.0 ? CatchUpOverride : SecondsSinceSave);
	CatchUpOverride = -1.0;
	const FString Message = FString::Printf(TEXT("Reloaded the idle save and recalculated %.1f minutes: research %.0f, experiments %d (live operation had %.0f and %d)."),
		Recalculated / 60.0, Sim->GetResearch(), Sim->GetTotalExperiments(), LiveResearch, LiveExperiments);
	Sim->AddSystemLog(Message);
	UE_LOG(LogTemp, Display, TEXT("AlienIdle: %s"), *Message);
}

void AAlienIdlePlayerController::AlienAway(float Minutes)
{
	if (!Sim.IsValid() || !RootWidget.IsValid())
	{
		return;
	}
	// Same flow as a real absence: save, time passes, memo, and acknowledging reloads + recalculates.
	bReloadPending = Sim->SaveToFile(FAlienIdleSim::GetDefaultSavePath());
	CatchUpOverride = FMath::Max(0.0f, Minutes) * 60.0;
	Sim->BeginAwayReport();
	Sim->CatchUp(CatchUpOverride);
	RootWidget->ShowAwayReport(Sim->BuildAwayReport());
}

void AAlienIdlePlayerController::AlienLaunch(int32 Ufo)
{
	if (Sim.IsValid() && Ufo >= 0 && Ufo < Sim->GetNumUfos())
	{
		Sim->Launch(Ufo);
	}
}

void AAlienIdlePlayerController::AlienUnlockAll()
{
	if (Sim.IsValid())
	{
		// Discoveries unlock every target and piece of equipment.
		for (int32 i = 0; i < int32(EDiscovery::Count); ++i)
		{
			Sim->DevDiscover(EDiscovery(i));
		}
	}
}

void AAlienIdlePlayerController::AlienAddResearch(float Amount)
{
	if (Sim.IsValid())
	{
		Sim->DevAddResearch(Amount);
	}
}

void AAlienIdlePlayerController::AlienDemo()
{
	if (!Sim.IsValid())
	{
		return;
	}
	FAlienIdleSim& S = *Sim;
	AlienUnlockAll();
	auto Base = [&S](EBaseUpgrade Upgrade, int32 Levels)
	{
		for (int32 i = 0; i < Levels; ++i)
		{
			S.DevAddResearch(S.GetBaseUpgradeCost(Upgrade));
			S.BuyBaseUpgrade(Upgrade);
		}
	};
	auto Ship = [&S](int32 Ufo, EShipSystem System, int32 Levels)
	{
		for (int32 i = 0; i < Levels; ++i)
		{
			S.DevAddResearch(S.GetShipUpgradeCost(Ufo, System));
			S.BuyShipUpgrade(Ufo, System);
		}
	};
	Base(EBaseUpgrade::Hangar, 2);
	Base(EBaseUpgrade::Chambers, 2);
	Base(EBaseUpgrade::Containment, 4);
	Base(EBaseUpgrade::Power, 4);
	Ship(0, EShipSystem::Tractor, 3);
	Ship(0, EShipSystem::Thrusters, 1);
	Ship(1, EShipSystem::Tractor, 1);
	Ship(1, EShipSystem::Cargo, 3);
	Ship(1, EShipSystem::Stealth, 1);
	Ship(2, EShipSystem::Scanner, 2);
	Ship(2, EShipSystem::Stealth, 1);
	for (int32 Hire = 0; Hire < 8; ++Hire)
	{
		S.DevAddResearch(S.GetRecruitCost());
		S.Recruit();
	}
	S.SetDestination(0, ESpecimen::Elephant);
	S.SetDestination(1, ESpecimen::Cow);
	S.SetDestination(2, ESpecimen::Human);
	for (int32 Offer = 0; Offer < S.GetContractOffers().Num(); ++Offer)
	{
		if (S.GetContractOffers()[Offer].Needs[0].Type == ESpecimen::Human)
		{
			S.AcceptContract(Offer);
			break;
		}
	}
	S.SetFollowContract(2, true);
	// Unlock automation and keep the whole fleet flying.
	if (!S.IsAutomationUnlocked())
	{
		S.DevAddExperiments(ESpecimen::Squirrel, AlienIdleTuning::AutomationUnlockExperiments);
	}
	for (int32 Ufo = 0; Ufo < S.GetNumUfos(); ++Ufo)
	{
		S.SetAutoLaunch(Ufo, true);
	}
	S.DevAddEnergy(1000.0);
	for (int32 Ufo = 0; Ufo < S.GetNumUfos(); ++Ufo)
	{
		S.Launch(Ufo);
		S.DevAddEnergy(1000.0);
	}
}

void AAlienIdlePlayerController::AlienCase(int32 Case)
{
	if (!Sim.IsValid())
	{
		return;
	}
	for (int32 c = 0; c < AlienIdleTuning::NumCaseSubjects; ++c)
	{
		if ((Case < 0 || Case == c) && !Sim->IsCaseOnBase(c))
		{
			Sim->DevAddCaseSubject(c);
		}
	}
}

void AAlienIdlePlayerController::AlienThesis()
{
	if (Sim.IsValid() && RootWidget.IsValid())
	{
		Sim->DevCompleteThesis();
		RootWidget->OpenPromotion();
	}
}
