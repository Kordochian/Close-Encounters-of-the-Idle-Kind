#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "AlienIdleTuning.h"
#include "AlienIdleSim.h"

class FAlienIdleSim;
class SButton;
class SVerticalBox;

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

	/** Called when the player acknowledges the memo. */
	TFunction<void()> OnAwayReportDismissed;

private:
	TSharedRef<SWidget> MakeHeading(const FText& Text) const;
	TSharedRef<SWidget> MakeLine(TFunction<FText()> TextGetter, TFunction<FSlateColor()> ColorGetter = nullptr) const;
	TSharedRef<SButton> MakeButton(TFunction<FText()> Label, TFunction<bool()> IsHighlighted, TFunction<void()> OnClick) const;
	TSharedRef<SWidget> MakeStepper(TFunction<FText()> Label, TFunction<void(int32)> OnStep) const;

	void AddShipUpgradeButton(const TSharedRef<SVerticalBox>& Box, EShipSystem System);
	void AddBaseUpgradeButton(const TSharedRef<SVerticalBox>& Box, EBaseUpgrade Upgrade);

	FText GetLaunchLabel() const;
	FText GetLogLine(int32 IndexFromNewest) const;
	FText GetResultLine(int32 IndexFromNewest) const;
	FText GetAwayReportText() const;
	TSharedRef<SWidget> MakeAwayReportPanel();
	TSharedRef<SWidget> MakePromotionPanel();
	int32 GetPlannedGrantSpend() const;

	TSharedPtr<FAlienIdleSim> Sim;
	TSharedPtr<SButton> LaunchButton;
	/** UFO shown in the fleet section. */
	int32 SelectedUfo = 0;

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
	TSharedPtr<SButton> DismissButton;
};
