#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "AlienIdleTuning.h"
#include "AlienIdleSim.h"

class FAlienIdleSim;
class SButton;
class SVerticalBox;
class SWidgetSwitcher;

/** The console's tabs, left to right. */
enum class EConsoleTab : uint8
{
	Operations,
	Contracts,
	Thesis,
	Collaborations,
	Count,
};

/** Full-screen layout: the mountain cutaway on the left, the officer's console on the right. */
class SAlienIdleRoot : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAlienIdleRoot) {}
		SLATE_ARGUMENT(TSharedPtr<FAlienIdleSim>, Sim)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Gamepad and keyboard focus starts here. */
	TSharedPtr<SWidget> GetInitialFocusWidget() const { return LaunchButton; }

	/** Shows the "While You Were Out" memo over everything until dismissed. */
	void ShowAwayReport(const FAwayReport& Report);
	bool IsShowingAwayReport() const { return bShowingReport; }
	/** Opens the Academy thesis review (only meaningful once the thesis can be submitted). */
	void OpenPromotion();
	/** Opens the five-second Probe Calibration challenge for the open calibration window, if any. */
	void OpenCalibration();
	/** Shows a console tab (EConsoleTab order). */
	void SetTab(int32 Tab);
	/** Selects a UFO or station; bShowOperations also switches to the Operations tab. */
	void SelectStation(const FAssignment& Station, bool bShowOperations);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	/** Gamepad shoulder buttons switch tabs (keys bubble up here from the focused control). */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** Called when the player acknowledges the memo. */
	TFunction<void()> OnAwayReportDismissed;

private:
	TSharedRef<SWidget> MakeHeading(const FText& Text) const;
	TSharedRef<SWidget> MakeLine(TFunction<FText()> TextGetter, TFunction<FSlateColor()> ColorGetter = nullptr) const;
	TSharedRef<SButton> MakeButton(TFunction<FText()> Label, TFunction<bool()> IsHighlighted, TFunction<void()> OnClick) const;
	TSharedRef<SWidget> MakeStepper(TFunction<FText()> Label, TFunction<void(int32)> OnStep) const;
	/** A compact upgrade card: title and cost on one line, the next improvement under it, then any blocker. */
	TSharedRef<SButton> MakeUpgradeCard(TFunction<FString()> Title, TFunction<FString()> Cost, TFunction<FString()> Next,
		TFunction<FString()> Blocker, TFunction<bool()> CanBuy, TFunction<void()> OnClick) const;

	// Tabs and the Operations selection
	/** Short badge text when the tab has an action waiting (finished contract, ready promotion, race offer); empty otherwise. */
	FString GetTabBadge(int32 Tab) const;
	void StepSelection(int32 Direction);
	/** Falls back to the first UFO when the selected station no longer exists (promotion, fewer chambers). */
	void ValidateSelection();
	FString GetSelectionLabel() const;
	FString GetSelectionStatus() const;
	bool IsSelectionBlocked() const;
	/** 0..1 for the selection's current activity, or negative when there is nothing in progress. */
	float GetSelectionProgress() const;
	/** Base upgrades are listed with the station they improve. */
	bool IsBaseUpgradeShown(EBaseUpgrade Upgrade) const;

	void AddShipUpgradeButton(const TSharedRef<SVerticalBox>& Box, EShipSystem System);
	void AddBaseUpgradeButton(const TSharedRef<SVerticalBox>& Box, EBaseUpgrade Upgrade);

	FText GetLaunchLabel() const;
	FText GetLogLine(int32 IndexFromNewest) const;
	FText GetResultLine(int32 IndexFromNewest) const;
	FText GetAwayReportText() const;
	TSharedRef<SWidget> MakeAwayReportPanel();
	TSharedRef<SWidget> MakePromotionPanel();
	TSharedRef<SWidget> MakeCalibrationPanel();
	/** Where the challenge cursor is now (or where it stopped): 0..1, sweeping back and forth. */
	float GetCalibrationCursor() const;
	void SubmitCalibration(ECalibrationGrade Grade);
	void CloseCalibration();
	int32 GetPlannedGrantSpend() const;

	TSharedPtr<FAlienIdleSim> Sim;
	TSharedPtr<SButton> LaunchButton;
	/** The UFO or station shown in Operations. SelectedUfo is the last UFO chosen (case-file scans use it too). */
	FAssignment Selected = { EStation::Ufo, 0 };
	int32 SelectedUfo = 0;
	bool bCrewExpanded = false;
	int32 ActiveTab = 0;
	TSharedPtr<SWidgetSwitcher> TabSwitcher;
	TSharedPtr<SButton> TabButtons[int32(EConsoleTab::Count)];
	/** What a promotion would pay right now, refreshed once a second while the Thesis tab is open. */
	FPromotionReview CachedReview;
	double ReviewCacheTime = 0.0;

	FAwayReport AwayReport;
	bool bShowingReport = false;

	// Promotion screen state
	bool bShowingPromotion = false;
	FPromotionReview Promotion;
	int32 SelectedPermanent = INDEX_NONE;
	int32 SelectedThesis = 0;
	bool bShowApproachGuide = false;
	bool bShowTargetMath = true;
	ELocation SelectedLocation = ELocation::Metropolis;
	bool PlannedUpgrades[int32(EGrantUpgrade::Count)] = {};
	ESpecialty PlannedSpecialty = ESpecialty::Technician;
	EEarthCondition SelectedCondition = EEarthCondition::DocumentaryCraze;
	FString PromotionError;
	TSharedPtr<SButton> PromotionConfirmButton;

	// Probe Calibration challenge state
	bool bShowingCalibration = false;
	bool bCalibrationResolved = false;
	double CalibrationStartTime = 0.0;
	double CalibrationResolvedTime = 0.0;
	float CalibrationStoppedAt = 0.0f;
	int32 CalibrationChamberShown = INDEX_NONE;
	FString CalibrationResult;
	TSharedPtr<SButton> CalibrationStopButton;
	TSharedPtr<SButton> CalibrationCloseButton;
	TSharedPtr<SButton> DismissButton;
};
