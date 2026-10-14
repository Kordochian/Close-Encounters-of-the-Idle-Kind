#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AlienIdleSim.h"
#include "UI/SAlienIdleRoot.h"
#include "UI/AlienInputWatcher.h"
#include "AlienIdlePlayerController.generated.h"

/** Owns the simulation, ticks it, and puts the Slate UI on screen. */
UCLASS()
class ALIENIDLE_API AAlienIdlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAlienIdlePlayerController();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

	/** Dev console: launch a UFO (same as its Launch button). */
	UFUNCTION(Exec)
	void AlienLaunch(int32 Ufo = 0);

	/** Dev console: unlock every specimen type. */
	UFUNCTION(Exec)
	void AlienUnlockAll();

	/** Dev console: jump to a mid-game base with three specialised UFOs, all launched. */
	UFUNCTION(Exec)
	void AlienDemo();

	/** Dev console: mark the thesis objectives complete and open the Academy review. */
	UFUNCTION(Exec)
	void AlienThesis();

	/** Dev console: fast-forward N minutes as if the player stepped away, then show the report. */
	UFUNCTION(Exec)
	void AlienAway(float Minutes = 30.0f);

	/** Dev console: a recurring subject (0 Marsh, 1 Pike, 2 Delgado, 3 Pell, 4 Fenn; -1 = all five) is put straight into containment. */
	UFUNCTION(Exec)
	void AlienCase(int32 Case = -1);

	/** Dev console: open a probe calibration window now (on the chamber with the most time left) and show the challenge. */
	UFUNCTION(Exec)
	void AlienCalibrate();

	/** Dev console: show a console tab (0 Operations, 1 Contracts, 2 Thesis, 3 Collaborations). */
	UFUNCTION(Exec)
	void AlienTab(int32 Tab = 0);

	/** Dev console: select a station in Operations (EStation: 1 UFO, 2 lab chamber, 3 containment, ...) and its index. */
	UFUNCTION(Exec)
	void AlienSelect(int32 Station = 1, int32 Index = 0);

	/** Dev console: give every console tab its badge (thesis ready, a one-cow contract that finishes on the next cow, a race offer). */
	UFUNCTION(Exec)
	void AlienBadges();

	/** Dev console: grant research for testing upgrades. */
	UFUNCTION(Exec)
	void AlienAddResearch(float Amount);

private:
	TSharedPtr<FAlienIdleSim> Sim;
	TSharedPtr<SAlienIdleRoot> RootWidget;
	/** Last Slate input time we have seen; a long gap before the next input means the player was away. */
	double LastSeenInteraction = 0.0;
	/** Saved at the idle mark during the current absence. */
	bool bSavedThisIdle = false;
	double LastAutosave = 0.0;
	/** The memo is up; acknowledging it reloads the idle save and recalculates the time since. */
	bool bReloadPending = false;
	/** Seconds to recalculate instead of wall-clock time (AlienAway fast-forwards without real time passing). */
	double CatchUpOverride = -1.0;

	void OnAwayReportDismissed();

	TSharedPtr<FAlienInputWatcher> InputWatcher;
};
