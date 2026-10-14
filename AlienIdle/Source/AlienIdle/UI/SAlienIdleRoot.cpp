#include "UI/SAlienIdleRoot.h"
#include "UI/SAlienBaseView.h"
#include "UI/SCalibrationGauge.h"
#include "AlienIdleSim.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SOverlay.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	/** Colors are authored as sRGB (what you see in a color picker). */
	FLinearColor Srgb(float R, float G, float B, float A = 1.0f)
	{
		// Computed directly (not via FLinearColor's lookup table) so it is safe in static initializers.
		auto ToLinear = [](float C) { return C <= 0.04045f ? C / 12.92f : FMath::Pow((C + 0.055f) / 1.055f, 2.4f); };
		return FLinearColor(ToLinear(R), ToLinear(G), ToLinear(B), A);
	}

	constexpr int32 VisibleLogLines = 6;
	constexpr int32 VisibleResultLines = 12;
	constexpr int32 MaxCrewShown = 16;
	constexpr int32 MaxUnclaimedShown = 4;

	const FLinearColor PanelColor = Srgb(0.035f, 0.04f, 0.055f);
	const FLinearColor ResultsColor = Srgb(0.02f, 0.025f, 0.035f);
	const FLinearColor HeadingColor = Srgb(0.55f, 0.95f, 0.6f);
	const FLinearColor TextColor = Srgb(0.85f, 0.87f, 0.9f);
	const FLinearColor MutedColor = Srgb(0.55f, 0.58f, 0.62f);
	const FLinearColor WarnColor = Srgb(1.0f, 0.65f, 0.35f);
	const FLinearColor TabColor = Srgb(0.35f, 0.37f, 0.42f);
	const FLinearColor TabActiveColor = Srgb(0.55f, 0.95f, 0.6f);

	FSlateFontInfo Font(int32 Size, const FName Style = TEXT("Regular"))
	{
		return FCoreStyle::GetDefaultFontStyle(Style, Size);
	}

	FText Str(const FString& String)
	{
		return FText::FromString(String);
	}

	const TCHAR* Roman(int32 Level)
	{
		static const TCHAR* Numerals[] = { TEXT("I"), TEXT("II"), TEXT("III"), TEXT("IV"), TEXT("V"), TEXT("VI") };
		return Numerals[FMath::Clamp(Level, 0, 5)];
	}
}

void SAlienIdleRoot::Construct(const FArguments& InArgs)
{
	Sim = InArgs._Sim;
	check(Sim.IsValid());

	// Always visible above the tabs: where we are, what we have, and what is holding us back.
	TSharedRef<SVerticalBox> Header = SNew(SVerticalBox);
	auto AddHeader = [&Header](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0))
	{
		Header->AddSlot().AutoHeight().Padding(Padding)[Widget];
	};
	AddHeader(SNew(STextBlock).Font(Font(16, TEXT("Bold"))).ColorAndOpacity(HeadingColor)
		.Text_Lambda([this] { return Str(FString(AlienIdleTuning::LocationNames[int32(Sim->GetLocation())]).ToUpper()); }));
	AddHeader(SNew(STextBlock).Font(Font(10)).ColorAndOpacity(MutedColor)
		.AutoWrapText(true)
		.Text_Lambda([this]
		{
			return Str(FString::Printf(TEXT("Research Officer Console, %s grade. Run %d, Research Grants %d."),
				*Sim->GetRankName(), Sim->GetCareer().Run, Sim->GetCareer().Grants));
		}), FMargin(0, 0, 0, 4));
	AddHeader(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Energy %.0f / %.0f  (+%.0f/s)     Research %.0f"), Sim->GetEnergy(), Sim->GetEnergyCap(), Sim->GetEnergyPerSecond(), Sim->GetResearch()));
	}));
	AddHeader(MakeLine([this] { return Str(Sim->GetBottleneckText()); },
		[this] { return FSlateColor(Sim->GetCurrentBottleneck() == EBottleneck::None ? MutedColor : WarnColor); }), FMargin(0, 2, 0, 8));

	TSharedRef<SVerticalBox> Operations = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> Contracts = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> Thesis = SNew(SVerticalBox);
	TSharedRef<SVerticalBox> Collaborations = SNew(SVerticalBox);
	// Each tab's widgets are added to the box that is current; Panel starts on Operations.
	TSharedRef<SVerticalBox> Panel = Operations;
	auto Add = [&Panel](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0))
	{
		Panel->AddSlot().AutoHeight().Padding(Padding)[Widget];
	};
	auto Shown = [](TFunction<bool()> Condition)
	{
		return TAttribute<EVisibility>::CreateLambda([Condition = MoveTemp(Condition)] { return Condition() ? EVisibility::Visible : EVisibility::Collapsed; });
	};
	auto IsUfo = [this] { return Selected.Station == EStation::Ufo; };
	auto IsAt = [this](EStation Station) { return [this, Station] { return Selected.Station == Station; }; };

	// =============================================================================================
	// OPERATIONS: the selected UFO or station, its crew, its orders and its upgrades, then base-wide work.

	// Incidents interrupt everything, so they sit above the selector while they last.
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->GetIncident().Type != EIncident::None ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()[ MakeHeading(NSLOCTEXT("AlienIdle", "Incident", "INCIDENT")) ]
			+ SVerticalBox::Slot().AutoHeight()[ MakeLine([this] { return Str(Sim->GetIncidentText() + TEXT(". Clears by itself.")); }, [] { return FSlateColor(WarnColor); }) ]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 8)
			[
				MakeButton([this]
				{
					FString Reason;
					return Sim->CanResolveIncident(&Reason)
						? Str(FString::Printf(TEXT("Intervene now (%.0f research)"), AlienIdleTuning::IncidentResolveCost))
						: Str(FString::Printf(TEXT("Intervene: %s"), *Reason));
				}, [this] { return Sim->CanResolveIncident(); }, [this] { Sim->ResolveIncident(); })
			]
		]);

	// 1. Selector
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Select", "SELECT")));
	Add(MakeStepper([this] { return Str(GetSelectionLabel()); }, [this](int32 Direction) { StepSelection(Direction); }));
	Add(MakeLine([] { return Str(TEXT("Or click a ship or room in the base.")); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 1, 0, 8));

	// 2. Status: current activity, progress and any blocker
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "SelectedStatus", "STATUS")));
	Add(MakeLine([this] { return Str(GetSelectionStatus()); }, [this] { return FSlateColor(IsSelectionBlocked() ? WarnColor : TextColor); }), FMargin(0, 1));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return GetSelectionProgress() >= 0.0f ? EVisibility::Visible : EVisibility::Collapsed; })
		.HeightOverride(6.0f)
		[
			SNew(SProgressBar)
			.Percent_Lambda([this] { return TOptional<float>(FMath::Max(0.0f, GetSelectionProgress())); })
			.FillColorAndOpacity(FSlateColor(HeadingColor))
		], FMargin(0, 2, 0, 2));
	Add(SNew(SBox).Visibility(Shown(IsUfo))
		[
			MakeLine([this]
			{
				const int32 U = SelectedUfo;
				return Str(FString::Printf(TEXT("Slots %d/%d   Tractor %d   Cargo %d   Flight x%.2f   Suspicion x%.2f"),
					Sim->GetShipSlotsUsed(U), Sim->GetShipSlots(), Sim->GetTractorStrength(U), Sim->GetCargoCapacity(U),
					Sim->GetFlightSeconds(U, ESpecimen::Squirrel) / AlienIdleTuning::Specimen(ESpecimen::Squirrel).FlightSeconds, Sim->GetSuspicionMultiplier(U)));
			}, [] { return FSlateColor(MutedColor); })
		], FMargin(0, 1));
	// Probe Calibration: optional. The window lapses on its own and research carries on regardless.
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Selected.Station == EStation::Chamber && Sim->GetCalibrationChamber() == Selected.Index && !Sim->IsCalibrationOpen() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([this]
			{
				return Str(FString::Printf(TEXT("CALIBRATION AVAILABLE (%.0f s): open the probe for a five-second calibration"), Sim->GetCalibrationOfferRemaining()));
			}, [] { return true; }, [this] { OpenCalibration(); })
		], FMargin(0, 2));
	// A calibration window on another chamber is worth a pointer from wherever the player is looking.
	Add(SNew(SBox)
		.Visibility_Lambda([this]
		{
			const int32 Chamber = Sim->GetCalibrationChamber();
			return Chamber != INDEX_NONE && !Sim->IsCalibrationOpen() && !(Selected.Station == EStation::Chamber && Selected.Index == Chamber) ? EVisibility::Visible : EVisibility::Collapsed;
		})
		[
			MakeButton([this]
			{
				return Str(FString::Printf(TEXT("Calibration available on lab chamber %d (%.0f s): open it"), Sim->GetCalibrationChamber() + 1, Sim->GetCalibrationOfferRemaining()));
			}, [] { return false; }, [this] { OpenCalibration(); })
		], FMargin(0, 2));
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// 3. Crew: the summary always shows; the assignment list folds away
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Crew", "CREW")));
	Add(MakeLine([this] { return Str(Sim->GetStationStaffingText(Selected)); },
		[this] { return FSlateColor(Sim->IsStationStaffed(Selected) ? TextColor : WarnColor); }), FMargin(0, 1));
	Add(MakeButton([this]
	{
		int32 Here = 0;
		for (const FCrewMember& Member : Sim->GetCrew())
		{
			Here += Member.Assignment.Station != EStation::None && Member.Assignment == Selected ? 1 : 0;
		}
		return Str(FString::Printf(TEXT("%s assignments (%d here, %d of %d crew idle)"), bCrewExpanded ? TEXT("Hide") : TEXT("Show"),
			Here, Sim->GetIdleCrew(), Sim->GetCrew().Num()));
	}, [this] { return bCrewExpanded; }, [this] { bCrewExpanded = !bCrewExpanded; }), FMargin(0, 2));
	{
		TSharedRef<SVerticalBox> CrewList = SNew(SVerticalBox);
		CrewList->AddSlot().AutoHeight().Padding(0, 2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
			[
				MakeButton([this]
				{
					FString Reason;
					return Sim->CanRecruit(&Reason)
						? Str(FString::Printf(TEXT("Request Additional Personnel (%.0f research)"), Sim->GetRecruitCost()))
						: Str(FString::Printf(TEXT("Request Additional Personnel: %s"), *Reason));
				}, [this] { return Sim->CanRecruit(); }, [this] { Sim->Recruit(); })
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				MakeButton([this] { return Str(Sim->IsAutoAssignOn() ? TEXT("Auto-assign: ON") : TEXT("Auto-assign: OFF")); },
					[this] { return Sim->IsAutoAssignOn(); }, [this] { Sim->SetAutoAssign(!Sim->IsAutoAssignOn()); })
			]
		];
		CrewList->AddSlot().AutoHeight().Padding(0, 1)
		[
			MakeLine([this]
			{
				return Str(FString::Printf(TEXT("Wanted here: a %s. Anyone can staff a post; specialists add bonuses."),
					*FString(FAlienIdleSim::GetSpecialtyName(FAlienIdleSim::GetStationSpecialty(Selected.Station))).ToLower()));
			}, [] { return FSlateColor(MutedColor); })
		];
		for (int32 i = 0; i < MaxCrewShown; ++i)
		{
			auto IsHere = [this, i]
			{
				const TArray<FCrewMember>& Crew = Sim->GetCrew();
				return Crew.IsValidIndex(i) && Crew[i].Assignment.Station != EStation::None && Crew[i].Assignment == Selected;
			};
			CrewList->AddSlot().AutoHeight().Padding(0, 1)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([this, i] { return i < Sim->GetCrew().Num() ? EVisibility::Visible : EVisibility::Collapsed; })
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					MakeLine([this, i]
					{
						if (!Sim->GetCrew().IsValidIndex(i))
						{
							return FText::GetEmpty();
						}
						const FCrewMember& Member = Sim->GetCrew()[i];
						const FString Level = Member.Training > 0 ? FString::Printf(TEXT(" %d"), Member.Training) : FString();
						const FString Where = Member.TrainingRemaining > 0.0f ? FString::Printf(TEXT("in a seminar (%.0f s)"), Member.TrainingRemaining)
							: Sim->GetStationName(Member.Assignment);
						return Str(FString::Printf(TEXT("%s, %s%s: %s"), *Member.Name, FAlienIdleSim::GetSpecialtyName(Member.Specialty), *Level, *Where));
					}, [IsHere] { return FSlateColor(IsHere() ? HeadingColor : TextColor); })
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					MakeButton([IsHere] { return Str(IsHere() ? TEXT("Remove") : TEXT("Assign here")); },
						[this, i, IsHere] { return IsHere() || (Sim->GetCrew().IsValidIndex(i) && Sim->GetCrew()[i].Assignment.Station == EStation::None); },
						[this, i, IsHere] { Sim->Assign(i, IsHere() ? FAssignment() : Selected); })
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					MakeButton([this, i]
					{
						if (!Sim->GetCrew().IsValidIndex(i))
						{
							return FText::GetEmpty();
						}
						FString Reason;
						return Sim->CanTrain(i, &Reason) || Reason.Contains(TEXT("needs"))
							? Str(FString::Printf(TEXT("Train %.0f"), Sim->GetTrainCost(i)))
							: Str(Reason.Contains(TEXT("fully")) ? TEXT("Trained") : TEXT("Seminar"));
					}, [this, i] { return Sim->GetCrew().IsValidIndex(i) && Sim->CanTrain(i); }, [this, i] { Sim->Train(i); })
				]
			];
		}
		Add(SNew(SBox).Visibility_Lambda([this] { return bCrewExpanded ? EVisibility::Visible : EVisibility::Collapsed; })[CrewList], FMargin(0, 0, 0, 2));
	}
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// 4. Dispatch (UFOs) or experiment / production choices (stations)
	Add(SNew(STextBlock).Font(Font(11, TEXT("Bold"))).ColorAndOpacity(HeadingColor)
		.Text_Lambda([this]
		{
			switch (Selected.Station)
			{
			case EStation::Ufo: return Str(TEXT("DISPATCH"));
			case EStation::Chamber: return Str(TEXT("EXPERIMENTS"));
			default: return Str(TEXT("PRODUCTION"));
			}
		}));
	{
		TSharedRef<SVerticalBox> Dispatch = SNew(SVerticalBox);
		auto AddDispatch = [&Dispatch](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0))
		{
			Dispatch->AddSlot().AutoHeight().Padding(Padding)[Widget];
		};
		AddDispatch(MakeStepper([this]
		{
			const int32 R = Sim->GetUfo(SelectedUfo).Region;
			const AlienIdleTuning::FRegionDef& Def = Sim->GetRegionDef(R);
			const FRegionState& Region = Sim->GetRegionState(R);
			const TCHAR* State = Region.bLockedDown ? TEXT("LOCKDOWN") : Sim->IsRegionIndexOnAlert(R) ? TEXT("on alert") : TEXT("calm");
			return Str(FString::Printf(TEXT("Region: %s (value x%.1f, exposure x%.1f), suspicion %.0f, %s"), Def.Name, Def.Value, Def.Exposure, Region.Suspicion, State));
		}, [this](int32 Direction)
		{
			const FUfo& Ufo = Sim->GetUfo(SelectedUfo);
			const int32 Num = Sim->GetNumRegions();
			const int32 Region = ((Ufo.Region + Direction) % Num + Num) % Num;
			const AlienIdleTuning::FRegionDef& Def = Sim->GetRegionDef(Region);
			ESpecimen Target = Def.NumSpecimens > 0 ? Def.Specimens[0] : Ufo.Destination;
			for (int32 k = 0; k < Def.NumSpecimens; ++k)
			{
				Target = Def.Specimens[k] == Ufo.Destination ? Ufo.Destination : Target;
			}
			Sim->SetDestinationRegion(SelectedUfo, Region, Target);
		}), FMargin(0, 1));
		AddDispatch(MakeStepper([this]
		{
			const FUfo& Ufo = Sim->GetUfo(SelectedUfo);
			const FSpecimenDef& Def = AlienIdleTuning::Specimen(Ufo.Destination);
			return Str(FString::Printf(TEXT("Target: %s (wt %d)%s  %.0f energy"), Def.Plural, Def.Weight,
				Sim->IsSpecimenUnlocked(Ufo.Destination) ? TEXT("") : TEXT(", locked"), Sim->GetLaunchEnergy(SelectedUfo, Ufo.Destination)));
		}, [this](int32 Direction)
		{
			const FUfo& Ufo = Sim->GetUfo(SelectedUfo);
			const AlienIdleTuning::FRegionDef& Def = Sim->GetRegionDef(Ufo.Region);
			if (Def.NumSpecimens == 0)
			{
				return;
			}
			int32 Current = 0;
			for (int32 k = 0; k < Def.NumSpecimens; ++k)
			{
				Current = Def.Specimens[k] == Ufo.Destination ? k : Current;
			}
			const int32 Next = ((Current + Direction) % Def.NumSpecimens + Def.NumSpecimens) % Def.NumSpecimens;
			Sim->SetDestinationRegion(SelectedUfo, Ufo.Region, Def.Specimens[Next]);
		}), FMargin(0, 1));
		AddDispatch(MakeButton([this] { return Str(bShowTargetMath ? TEXT("Why these numbers: hide") : TEXT("Why these numbers: show research bonuses and mission penalties")); },
			[this] { return bShowTargetMath; }, [this] { bShowTargetMath = !bShowTargetMath; }), FMargin(0, 2, 0, 0));
		AddDispatch(SNew(SBox)
			.Visibility_Lambda([this] { return bShowTargetMath ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this] { return Str(Sim->ExplainTarget(SelectedUfo)); }, [] { return FSlateColor(MutedColor); })
			], FMargin(0, 1, 0, 2));

		LaunchButton = MakeButton([this] { return GetLaunchLabel(); }, [this] { return Sim->CanLaunch(SelectedUfo); }, [this] { Sim->Launch(SelectedUfo); });
		AddDispatch(SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
			[
				LaunchButton.ToSharedRef()
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				MakeButton([this]
				{
					if (!Sim->IsAutomationUnlocked())
					{
						return Str(FString::Printf(TEXT("Auto-repeat: locked (%d/%d experiments)"), Sim->GetTotalExperiments(), AlienIdleTuning::AutomationUnlockExperiments));
					}
					return Str(Sim->GetUfo(SelectedUfo).bAutoLaunch ? TEXT("Auto-repeat: ON") : TEXT("Auto-repeat: OFF"));
				}, [this] { return Sim->IsAutomationUnlocked() && Sim->GetUfo(SelectedUfo).bAutoLaunch; },
				[this] { Sim->SetAutoLaunch(SelectedUfo, !Sim->GetUfo(SelectedUfo).bAutoLaunch); })
			], FMargin(0, 4, 0, 2));
		AddDispatch(MakeButton([this]
		{
			if (!Sim->IsAutomationUnlocked())
			{
				return Str(TEXT("Contract mode: locked until automation"));
			}
			return Str(Sim->GetUfo(SelectedUfo).bFollowContract ? TEXT("Contract mode: ON (picks its own target for the active contract)") : TEXT("Contract mode: OFF"));
		}, [this] { return Sim->IsAutomationUnlocked() && Sim->GetUfo(SelectedUfo).bFollowContract; },
			[this] { Sim->SetFollowContract(SelectedUfo, !Sim->GetUfo(SelectedUfo).bFollowContract); }), FMargin(0, 0, 0, 2));
		AddDispatch(MakeLine([this]
		{
			if (Sim->GetShipLevel(SelectedUfo, EShipSystem::Scanner) == 0)
			{
				return Str(TEXT("Scanner: none fitted. Humans arrive with whatever job they had."));
			}
			const EOccupation Target = Sim->GetScannerTarget(SelectedUfo, ESpecimen::Human);
			return Str(Target == EOccupation::None ? FString(TEXT("Scanner: no contract or project needs a profession right now."))
				: FString::Printf(TEXT("Scanner targets: %s (for your contract or project)"), AlienIdleTuning::Occupation(Target).Name));
		}, [] { return FSlateColor(MutedColor); }), FMargin(0, 1));
		AddDispatch(SNew(SBox)
			.Visibility_Lambda([this] { return Sim->GetUfo(SelectedUfo).CaseTarget >= 0 ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this] { return Str(Sim->GetCaseTargetText(SelectedUfo)); }, [] { return FSlateColor(HeadingColor); })
			]);
		AddDispatch(MakeLine([] { return Str(TEXT("Fleet rules (every auto-repeating ship):")); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 6, 0, 1));
		AddDispatch(MakeButton([this]
		{
			return Str(Sim->IsRerouteOn()
				? TEXT("Reroute around Earth's responses: ON (picks the calmest region with the target)")
				: TEXT("Reroute around Earth's responses: OFF"));
		}, [this] { return Sim->IsRerouteOn(); }, [this] { Sim->SetReroute(!Sim->IsRerouteOn()); }), FMargin(0, 0, 0, 2));
		AddDispatch(MakeButton([this]
		{
			return Str(Sim->IsPauseWhenFullOn()
				? TEXT("Pause collection when containment is full: ON")
				: TEXT("Pause collection when containment is full: OFF"));
		}, [this] { return Sim->IsPauseWhenFullOn(); }, [this] { Sim->SetPauseWhenFull(!Sim->IsPauseWhenFullOn()); }));
		Add(SNew(SBox).Visibility(Shown(IsUfo))[Dispatch], FMargin(0, 1));
	}

	// Lab chambers: what to study. Priorities are per species and shared by every chamber.
	{
		TSharedRef<SVerticalBox> Lab = SNew(SVerticalBox);
		auto AddLab = [&Lab](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0))
		{
			Lab->AddSlot().AutoHeight().Padding(Padding)[Widget];
		};
		AddLab(MakeLine([this] { return Str(FString::Printf(TEXT("Lab speed x%.2f. Priorities apply to every chamber. Routine and extended studies pay research; discovery projects pay nothing and unlock targets and equipment."), Sim->GetLabSpeed())); },
			[] { return FSlateColor(MutedColor); }));
		for (int32 i = 0; i < int32(ESpecimen::Count); ++i)
		{
			const ESpecimen Type = ESpecimen(i);
			AddLab(SNew(SBox)
				.Visibility_Lambda([this, Type] { return Sim->IsSpecimenUnlocked(Type) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					MakeStepper([this, Type]
					{
						const int32 Open = Sim->GetExperimentOptions(Type).Num() - 2;
						return Str(FString::Printf(TEXT("%s: %s%s"), *FString(AlienIdleTuning::Specimen(Type).Plural).Left(1).ToUpper().Append(FString(AlienIdleTuning::Specimen(Type).Plural).Mid(1)),
							*Sim->DescribeExperiment(Type, Sim->GetExperimentChoice(Type)),
							Open > 0 && Sim->GetExperimentChoice(Type).Kind != EExperimentKind::Discovery ? *FString::Printf(TEXT("  [%d project%s open]"), Open, Open == 1 ? TEXT("") : TEXT("s")) : TEXT("")));
					}, [this, Type](int32 Direction) { Sim->CycleExperimentChoice(Type, Direction); })
				], FMargin(0, 2));
		}
		for (int32 d = 1; d < int32(EDepartment::Count); ++d)
		{
			const EDepartment Dept = EDepartment(d);
			AddLab(SNew(SBox)
				.Visibility_Lambda([this] { return Sim->GetPrimaryDepartment() != EDepartment::None ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					MakeButton([this, Dept]
					{
						FString Reason;
						const bool bCan = Sim->CanBeginSignature(Dept, &Reason);
						return Str(FString::Printf(TEXT("%sSignature: %s"), bCan ? TEXT("Begin ") : TEXT(""), *Sim->GetSignatureStatusText(Dept)));
					}, [this, Dept] { return Sim->CanBeginSignature(Dept) || Sim->GetPendingSignature() == Dept; }, [this, Dept] { Sim->BeginSignature(Dept); })
				], FMargin(0, 2));
		}
		AddLab(SNew(SBox)
			.Visibility_Lambda([this] { return Sim->IsMiddleManagementAvailable() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this]
				{
					return Str(Sim->IsMiddleManagementOn()
						? TEXT("Middle Management: ON (labs +20%, mandatory meetings every 2 min)")
						: TEXT("Middle Management: OFF (promote an alien: labs +20%, but meetings)"));
				}, [this] { return Sim->IsMiddleManagementOn(); }, [this] { Sim->SetMiddleManagement(!Sim->IsMiddleManagementOn()); })
			], FMargin(0, 2));
		AddLab(MakeLine([this] { return Str(Sim->GetCalibrationStatusText()); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 2, 0, 0));
		Add(SNew(SBox).Visibility(Shown(IsAt(EStation::Chamber)))[Lab], FMargin(0, 1));
	}

	// Freezer mode belongs to the lab and to containment (where the samples wait).
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->IsPreserving() && (Selected.Station == EStation::Chamber || Selected.Station == EStation::Containment) ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([this]
			{
				return Str(Sim->IsAgingSamples()
					? TEXT("Freezer mode: ON (routine studies wait until a sample has been preserved a minute: up to +50% research, slower turnover)")
					: TEXT("Freezer mode: OFF (study specimens as soon as they arrive)"));
			}, [this] { return Sim->IsAgingSamples(); }, [this] { Sim->SetAgeSamples(!Sim->IsAgingSamples()); })
		], FMargin(0, 2));
	Add(SNew(SBox).Visibility(Shown(IsAt(EStation::Containment)))
		[
			MakeButton([this]
			{
				return Str(Sim->IsPauseWhenFullOn()
					? TEXT("Pause collection when containment is full: ON (all auto-repeating ships)")
					: TEXT("Pause collection when containment is full: OFF"));
			}, [this] { return Sim->IsPauseWhenFullOn(); }, [this] { Sim->SetPauseWhenFull(!Sim->IsPauseWhenFullOn()); })
		], FMargin(0, 2));
	Add(SNew(SBox).Visibility(Shown(IsAt(EStation::Boiler)))
		[
			MakeButton([this] { return Str(FString::Printf(TEXT("Heat priority: %s (click to change)"), FAlienIdleSim::GetHeatPriorityName(Sim->GetHeatPriority()))); },
				[] { return false; },
				[this] { Sim->SetHeatPriority(EHeatPriority((int32(Sim->GetHeatPriority()) + 1) % int32(EHeatPriority::Count))); })
		], FMargin(0, 2));
	Add(SNew(SBox)
		.Visibility_Lambda([this]
		{
			const EStation S = Selected.Station;
			return S == EStation::Power || S == EStation::MemoryWipe || S == EStation::Pressure || S == EStation::Calibration ? EVisibility::Visible : EVisibility::Collapsed;
		})
		[
			MakeLine([] { return Str(TEXT("No choices to make here. Crew and upgrades decide its output.")); }, [] { return FSlateColor(MutedColor); })
		], FMargin(0, 2));
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// 5. Upgrades: compact cards with the cost and the next improvement
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Upgrades", "UPGRADES")));
	{
		TSharedRef<SVerticalBox> ShipUpgrades = SNew(SVerticalBox);
		for (EShipSystem System : { EShipSystem::Tractor, EShipSystem::Cargo, EShipSystem::Stealth, EShipSystem::Scanner, EShipSystem::Thrusters })
		{
			AddShipUpgradeButton(ShipUpgrades, System);
		}
		ShipUpgrades->AddSlot().AutoHeight().Padding(0, 2)
		[
			MakeButton([this]
			{
				FString Reason;
				if (!Sim->CanRefitShip(SelectedUfo, &Reason))
				{
					return Str(FString::Printf(TEXT("Refit ship: %s"), *Reason));
				}
				return Str(FString::Printf(TEXT("Refit ship: strip all upgrades, refund %.0f research"), Sim->GetRefitRefund(SelectedUfo)));
			}, [] { return false; }, [this] { Sim->RefitShip(SelectedUfo); })
		];
		Add(SNew(SBox).Visibility(Shown(IsUfo))[ShipUpgrades], FMargin(0, 1));
	}
	{
		TSharedRef<SVerticalBox> BaseUpgrades = SNew(SVerticalBox);
		for (int32 i = 0; i < int32(EBaseUpgrade::Count); ++i)
		{
			AddBaseUpgradeButton(BaseUpgrades, EBaseUpgrade(i));
		}
		Add(BaseUpgrades, FMargin(0, 0, 0, 2));
	}
	Add(SNew(SBox)
		.Visibility_Lambda([this]
		{
			for (int32 i = 0; i < int32(EBaseUpgrade::Count); ++i)
			{
				if (IsBaseUpgradeShown(EBaseUpgrade(i)))
				{
					return EVisibility::Collapsed;
				}
			}
			return Selected.Station == EStation::Ufo ? EVisibility::Collapsed : EVisibility::Visible;
		})
		[
			MakeLine([] { return Str(TEXT("Nothing to upgrade here yet.")); }, [] { return FSlateColor(MutedColor); })
		]);
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 14));

	// Base-wide work below the selected station
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "NextSteps", "WHAT TO DO NEXT")));
	Add(MakeLine([this] { return Str(Sim->GetRecommendation()); }, [] { return FSlateColor(HeadingColor); }), FMargin(0, 2, 0, 2));
	for (int32 i = 0; i < 6; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return Sim->GetBottleneckFixes().IsValidIndex(i) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this, i]
				{
					const TArray<FString> Fixes = Sim->GetBottleneckFixes();
					return Fixes.IsValidIndex(i) ? Str(FString::Printf(TEXT("  - %s"), *Fixes[i])) : FText::GetEmpty();
				}, [] { return FSlateColor(MutedColor); })
			]);
	}
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// Research department
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Department", "RESEARCH DEPARTMENT")));
	Add(MakeLine([this]
	{
		const EDepartment Primary = Sim->GetPrimaryDepartment();
		if (Primary == EDepartment::None)
		{
			return Str(TEXT("Choose a primary department. Its projects cost the listed price; the other two cost 2.5x but are never locked."));
		}
		return Str(FString::Printf(TEXT("Primary: %s (%s). Uniforms: %s."), AlienIdleTuning::Department(Primary).Name,
			AlienIdleTuning::Department(Primary).Focus, AlienIdleTuning::Department(Primary).UniformNote));
	}, [this] { return FSlateColor(Sim->GetPrimaryDepartment() == EDepartment::None ? WarnColor : HeadingColor); }));
	for (int32 d = 1; d < int32(EDepartment::Count); ++d)
	{
		const EDepartment Dept = EDepartment(d);
		Add(SNew(SBox)
			.Visibility_Lambda([this] { return Sim->GetPrimaryDepartment() == EDepartment::None ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([Dept]
				{
					const FDepartmentDef& Def = AlienIdleTuning::Department(Dept);
					return Str(FString::Printf(TEXT("Choose %s%s%s.%sUnlocks: %s."), Def.Name, TEXT(": "), Def.Focus, LINE_TERMINATOR, Def.Unlocks));
				}, [] { return true; }, [this, Dept] { Sim->ChooseDepartment(Dept); })
			], FMargin(0, 2));
	}
	for (int32 i = 0; i < int32(EDeptProject::Count); ++i)
	{
		const EDeptProject Project = EDeptProject(i);
		Add(SNew(SBox)
			.Visibility_Lambda([this] { return Sim->GetPrimaryDepartment() != EDepartment::None ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeUpgradeCard([Project]
				{
					const FDeptProjectDef& Def = AlienIdleTuning::DeptProject(Project);
					return FString::Printf(TEXT("%s: %s"), AlienIdleTuning::Department(Def.Department).Name, Def.Name);
				}, [this, Project]
				{
					return Sim->IsProjectDone(Project) ? FString(TEXT("done")) : FString::Printf(TEXT("%.0f research"), Sim->GetProjectCost(Project));
				}, [Project] { return FString(AlienIdleTuning::DeptProject(Project).Effect); }, [this, Project]
				{
					FString Reason;
					return Sim->IsProjectDone(Project) || Sim->CanBuyProject(Project, &Reason) ? FString() : Reason;
				}, [this, Project] { return Sim->CanBuyProject(Project); }, [this, Project] { Sim->BuyProject(Project); })
			], FMargin(0, 1));
	}
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// Regions and Earth's responses
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Regions", "REGIONS AND EARTH'S RESPONSES")));
	for (int32 r = 0; r < AlienIdleTuning::NumRegions; ++r)
	{
		Add(MakeLine([this, r]
		{
			const AlienIdleTuning::FRegionDef& Def = Sim->GetRegionDef(r);
			const FRegionState& Region = Sim->GetRegionState(r);
			TArray<FString> Kinds;
			for (int32 k = 0; k < Def.NumSpecimens; ++k)
			{
				Kinds.Add(AlienIdleTuning::Specimen(Def.Specimens[k]).Plural);
			}
			const TCHAR* State = Region.bLockedDown ? TEXT("LOCKDOWN") : Sim->IsRegionIndexOnAlert(r) ? TEXT("on alert") : TEXT("calm");
			TArray<FString> Pressures;
			for (int32 k = 0; k < int32(EResponse::Count); ++k)
			{
				if (Region.Pressure[k] >= 5.0f && !Region.Active[k])
				{
					Pressures.Add(FString::Printf(TEXT("%s %.0f%%"), AlienIdleTuning::Responses[k].Name, 100.0f * Region.Pressure[k] / Sim->GetResponseThreshold()));
				}
			}
			return Str(FString::Printf(TEXT("%s (%s; value x%.1f, exposure x%.1f): suspicion %.0f, %s%s"), Def.Name, *FString::Join(Kinds, TEXT(", ")),
				Def.Value, Def.Exposure, Region.Suspicion, State,
				Pressures.Num() > 0 ? *FString::Printf(TEXT(". Building: %s"), *FString::Join(Pressures, TEXT(", "))) : TEXT("")));
		}, [this, r]
		{
			const FRegionState& Region = Sim->GetRegionState(r);
			return FSlateColor(Region.bLockedDown || Sim->IsRegionIndexOnAlert(r) ? WarnColor : TextColor);
		}), FMargin(0, 3, 0, 1));
		for (int32 k = 0; k < int32(EResponse::Count); ++k)
		{
			const EResponse Response = EResponse(k);
			Add(SNew(SBox)
				.Visibility_Lambda([this, r, Response] { return Sim->IsResponseActive(r, Response) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					MakeButton([this, r, Response]
					{
						const AlienIdleTuning::FResponseDef& Def = AlienIdleTuning::Responses[int32(Response)];
						FString Reason;
						const bool bCan = Sim->CanCounter(r, Response, &Reason);
						return Str(FString::Printf(TEXT("%s (%s): %s.%s%s (%.0f research)%s"), Def.Name, Def.Cause, Def.Effect, LINE_TERMINATOR,
							Def.Counter, Sim->GetCounterCost(r, Response), bCan ? TEXT("") : *FString::Printf(TEXT(", %s"), *Reason)));
					}, [this, r, Response] { return Sim->CanCounter(r, Response); }, [this, r, Response] { Sim->Counter(r, Response); })
				], FMargin(12, 1, 0, 1));
		}
	}

	// Case files: recurring subjects, their relationships and research chains (kept through promotions)
	auto ShowCases = [this]
	{
		bool bAnyKnown = false;
		for (int32 c = 0; c < AlienIdleTuning::NumCaseSubjects; ++c)
		{
			bAnyKnown |= Sim->IsCaseKnown(c);
		}
		return bAnyKnown || Sim->IsSpecimenUnlocked(ESpecimen::Human) ? EVisibility::Visible : EVisibility::Collapsed;
	};
	TSharedRef<SVerticalBox> Cases = SNew(SVerticalBox);
	auto AddCase = [&Cases](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0))
	{
		Cases->AddSlot().AutoHeight().Padding(Padding)[Widget];
	};
	AddCase(MakeHeading(NSLOCTEXT("AlienIdle", "CaseFiles", "CASE FILES: PERSONS OF INTEREST")), FMargin(0, 10, 0, 2));
	AddCase(MakeLine([] { return Str(TEXT("Most humans are ordinary. These five are not: they remember us, they know each other, and their files survive promotions.")); },
		[] { return FSlateColor(MutedColor); }));
	AddCase(MakeButton([this]
	{
		return Str(Sim->IsCaseStudiesOn() ? TEXT("Case studies: ON (recurring subjects get their personal research chain)")
			: TEXT("Case studies: OFF (recurring subjects get routine studies)"));
	}, [this] { return Sim->IsCaseStudiesOn(); }, [this] { Sim->SetCaseStudies(!Sim->IsCaseStudiesOn()); }), FMargin(0, 2));
	for (int32 c = 0; c < AlienIdleTuning::NumCaseSubjects; ++c)
	{
		AddCase(MakeLine([this, c] { return Str(Sim->GetCaseFileText(c)); },
			[this, c] { return FSlateColor(Sim->HasCaseDiscovery(c) ? HeadingColor : Sim->IsCaseKnown(c) ? TextColor : MutedColor); }), FMargin(0, 4, 0, 0));
		for (int32 k = 0; k < 3; ++k)
		{
			AddCase(SNew(SBox)
				.Visibility_Lambda([this, c, k] { return Sim->GetCaseRelationText(c).IsValidIndex(k) ? EVisibility::Visible : EVisibility::Collapsed; })
				[
					MakeLine([this, c, k]
					{
						const TArray<FString> Lines = Sim->GetCaseRelationText(c);
						return Lines.IsValidIndex(k) ? Str(FString::Printf(TEXT("  - %s"), *Lines[k])) : FText::GetEmpty();
					}, [this, c, k]
					{
						const TArray<FString> Lines = Sim->GetCaseRelationText(c);
						return FSlateColor(Lines.IsValidIndex(k) && Lines[k].Contains(TEXT("WILL")) ? WarnColor : MutedColor);
					})
				]);
		}
		AddCase(MakeButton([this, c]
		{
			const FString Who = Sim->IsCaseKnown(c) ? FAlienIdleSim::GetCaseName(c) : FString(TEXT("this lead"));
			if (Sim->GetUfo(SelectedUfo).CaseTarget == c)
			{
				return Str(FString::Printf(TEXT("'%s' is scanning for %s (click to stop)"), *Sim->GetUfo(SelectedUfo).Name, *Who));
			}
			FString Reason;
			return Sim->CanTargetCase(SelectedUfo, c, &Reason)
				? Str(FString::Printf(TEXT("Scan for %s with '%s'"), *Who, *Sim->GetUfo(SelectedUfo).Name))
				: Str(FString::Printf(TEXT("Scan for %s: %s"), *Who, *Reason));
		}, [this, c] { return Sim->GetUfo(SelectedUfo).CaseTarget == c || Sim->CanTargetCase(SelectedUfo, c); }, [this, c]
		{
			Sim->SetCaseTarget(SelectedUfo, Sim->GetUfo(SelectedUfo).CaseTarget == c ? -1 : c);
		}), FMargin(0, 2));
	}
	Add(SNew(SBox).Visibility_Lambda(ShowCases)[Cases]);

	// Encyclopedia
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Encyclopedia", "ENCYCLOPEDIA (LATEST)")), FMargin(0, 10, 0, 2));
	for (int32 i = 0; i < 2; ++i)
	{
		Add(MakeLine([this, i]
		{
			const TArray<FEncyclopediaEntry>& Entries = Sim->GetEncyclopedia();
			const int32 Index = Entries.Num() - 1 - i;
			return Entries.IsValidIndex(Index) ? Str(FString::Printf(TEXT("%s: %s"), *Entries[Index].Title, *Entries[Index].Text)) : FText::GetEmpty();
		}, [] { return FSlateColor(MutedColor); }), FMargin(0, 0, 0, 4));
	}

	// Log
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Log", "DEPUTY'S LOG")), FMargin(0, 10, 0, 2));
	for (int32 i = 0; i < VisibleLogLines; ++i)
	{
		Add(SNew(STextBlock)
			.Font(Font(9))
			.AutoWrapText(true)
			.Text_Lambda([this, i] { return GetLogLine(i); })
			.ColorAndOpacity_Lambda([i] { return FSlateColor(i == 0 ? TextColor : MutedColor); }), FMargin(0, 0, 0, 4));
	}
	Add(SNew(STextBlock).Font(Font(9)).ColorAndOpacity(MutedColor).AutoWrapText(true)
		.Text(NSLOCTEXT("AlienIdle", "Controls", "Mouse, keyboard or gamepad. Arrows / D-pad to move, Enter / A to press, shoulder buttons switch tabs. Mouse wheel zooms the map, drag to pan, click a ship or room to select it, double-click to reset.")), FMargin(0, 6, 0, 0));

	// =============================================================================================
	// CONTRACTS: finished (claim), active, offers.
	Panel = Contracts;
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Finished", "FINISHED: CLAIM REWARDS")));
	Add(MakeLine([this]
	{
		const int32 Waiting = Sim->GetUnclaimedContracts().Num();
		return Waiting == 0 ? Str(FString::Printf(TEXT("Nothing waiting. Contracts done this run: %d."), Sim->GetContractsCompleted()))
			: Str(FString::Printf(TEXT("%d finished contract%s, %.0f research waiting. Contracts done this run: %d."), Waiting, Waiting == 1 ? TEXT("") : TEXT("s"),
				Sim->GetUnclaimedReward(), Sim->GetContractsCompleted()));
	}, [this] { return FSlateColor(Sim->GetUnclaimedContracts().Num() > 0 ? HeadingColor : MutedColor); }));
	for (int32 i = 0; i < MaxUnclaimedShown; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return Sim->GetUnclaimedContracts().IsValidIndex(i) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this, i]
				{
					const TArray<FContract>& Done = Sim->GetUnclaimedContracts();
					return Done.IsValidIndex(i) ? Str(FString::Printf(TEXT("Claim: %s  (+%.0f research)"), *Done[i].Title, Done[i].Reward)) : FText::GetEmpty();
				}, [] { return true; }, [this, i] { Sim->ClaimContract(i); })
			], FMargin(0, 2));
	}
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->GetUnclaimedContracts().Num() > 1 ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([] { return Str(TEXT("Claim all")); }, [] { return true; }, [this] { Sim->ClaimAllContracts(); })
		], FMargin(0, 2));
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Active", "ACCEPTED")));
	Add(MakeLine([this]
	{
		if (!Sim->HasActiveContract())
		{
			return Str(TEXT("No accepted contract. Pick one below."));
		}
		const FContract& Contract = Sim->GetActiveContract();
		return Str(FString::Printf(TEXT("%s  (%.0f research)%s%s%sProgress: %s"), *Contract.Title, Contract.Reward, LINE_TERMINATOR, *Contract.Flavor, LINE_TERMINATOR, *Contract.GetProgressText()));
	}, [this] { return FSlateColor(Sim->HasActiveContract() ? HeadingColor : MutedColor); }));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->HasActiveContract() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([] { return Str(TEXT("Abandon contract")); }, [] { return false; }, [this] { Sim->AbandonContract(); })
		], FMargin(0, 2));
	Add(MakeButton([this]
	{
		if (!Sim->IsAutomationUnlocked())
		{
			return Str(TEXT("Auto-accept contracts: locked until automation"));
		}
		return Str(Sim->IsAutoContractsOn() ? TEXT("Auto-accept contracts: ON (signs the best-paying offer your ships are set up for and claims finished ones, also while you are away)")
			: TEXT("Auto-accept contracts: OFF"));
	}, [this] { return Sim->IsAutomationUnlocked() && Sim->IsAutoContractsOn(); }, [this] { Sim->SetAutoContracts(!Sim->IsAutoContractsOn()); }), FMargin(0, 2, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Offers", "AVAILABLE")));
	for (int32 i = 0; i < AlienIdleTuning::ContractOffers; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return Sim->GetContractOffers().IsValidIndex(i) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this, i]
				{
					const TArray<FContract>& Offers = Sim->GetContractOffers();
					if (!Offers.IsValidIndex(i))
					{
						return FText::GetEmpty();
					}
					const FContract& Offer = Offers[i];
					return Str(FString::Printf(TEXT("%s%s  (%.0f research)%s%s"), Sim->HasActiveContract() ? TEXT("") : TEXT("Accept: "), *Offer.Title, Offer.Reward, LINE_TERMINATOR, *Offer.Flavor));
				}, [this] { return !Sim->HasActiveContract(); }, [this, i] { Sim->AcceptContract(i); })
			], FMargin(0, 2));
	}
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->HasActiveContract() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeLine([] { return Str(TEXT("One contract at a time. Finish or abandon the accepted one to sign another.")); }, [] { return FSlateColor(MutedColor); })
		], FMargin(0, 2));

	// =============================================================================================
	// THESIS: objectives, what a promotion would pay, and the promotion itself.
	Panel = Thesis;
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Thesis", "THESIS")));
	Add(MakeLine([this]
	{
		const AlienIdleTuning::FThesisDef& Def = AlienIdleTuning::Thesis(Sim->GetThesis());
		return Str(FString::Printf(TEXT("%s: %s Rule: %s"), Def.Name, Def.Brief, Def.Rule));
	}, [] { return FSlateColor(MutedColor); }));
	Add(MakeLine([this]
	{
		const AlienIdleTuning::FEarthConditionDef& Earth = AlienIdleTuning::EarthCondition(Sim->GetEarthCondition());
		return Str(FString::Printf(TEXT("Earth conditions: %s. %s"), Earth.Name, Earth.Preview));
	}, [this] { return FSlateColor(Sim->GetEarthCondition() == EEarthCondition::BusinessAsUsual ? MutedColor : WarnColor); }), FMargin(0, 2, 0, 6));
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Objectives", "OBJECTIVES")));
	for (int32 i = 0; i < 6; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return i < Sim->GetThesisObjectives().Num() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this, i]
				{
					const TArray<FThesisObjective> Objectives = Sim->GetThesisObjectives();
					return Objectives.IsValidIndex(i) ? Str(FString::Printf(TEXT("%s %s"), Objectives[i].bDone ? TEXT("[done]") : TEXT("[    ]"), *Objectives[i].Text)) : FText::GetEmpty();
				}, [this, i]
				{
					const TArray<FThesisObjective> Objectives = Sim->GetThesisObjectives();
					return FSlateColor(Objectives.IsValidIndex(i) && Objectives[i].bDone ? HeadingColor : TextColor);
				})
			]);
	}
	Add(MakeLine([this]
	{
		FString Line = TEXT("Experiments:");
		for (int32 i = 0; i < int32(ESpecimen::Count); ++i)
		{
			if (Sim->IsSpecimenUnlocked(ESpecimen(i)))
			{
				Line += FString::Printf(TEXT("  %s %d"), AlienIdleTuning::Specimens[i].Plural, Sim->GetExperimentsCompleted(ESpecimen(i)));
			}
		}
		return Str(Line + FString::Printf(TEXT("%sDiscoveries %d/%d   Encyclopedia %d/%d   Contracts done %d"), LINE_TERMINATOR, Sim->GetDiscoveryCount(), int32(EDiscovery::Count),
			Sim->GetEncyclopedia().Num(), FAlienIdleSim::GetEncyclopediaSize(), Sim->GetContractsCompleted()));
	}, [] { return FSlateColor(MutedColor); }), FMargin(0, 4, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Prestige", "PRESTIGE REWARDS IF SUBMITTED NOW")));
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("%d Research Grants. Rank: %s -> %s."), CachedReview.Grants, *CachedReview.RankBefore, *CachedReview.RankAfter));
	}, [] { return FSlateColor(HeadingColor); }));
	for (int32 i = 0; i < 8; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return CachedReview.GrantLines.IsValidIndex(i) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this, i] { return CachedReview.GrantLines.IsValidIndex(i) ? Str(TEXT("  - ") + CachedReview.GrantLines[i]) : FText::GetEmpty(); }, [] { return FSlateColor(MutedColor); })
			]);
	}
	Add(MakeLine([this] { return Str(TEXT("Academy achievements:") LINE_TERMINATOR + Sim->GetAchievementText()); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 4, 0, 0));
	Add(MakeButton([this]
	{
		FString Reason;
		return Sim->CanSubmitThesis(&Reason) ? Str(TEXT("Submit thesis to the Galactic Academy (review first)"))
			: Str(FString::Printf(TEXT("Submit thesis: %s"), *Reason));
	}, [this] { return Sim->CanSubmitThesis(); }, [this] { if (Sim->CanSubmitThesis()) { OpenPromotion(); } }), FMargin(0, 6, 0, 12));

	// Build: what kind of operation this is, approach templates and saved presets (each approach has its own thesis)
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Build", "BUILD: APPROACH AND PRESETS")));
	Add(MakeLine([this] { return Str(Sim->GetApproachReading()); }, [] { return FSlateColor(TextColor); }));
	Add(MakeButton([this] { return Str(bShowApproachGuide ? TEXT("Approach guide: hide") : TEXT("Approach guide: show the three approaches")); },
		[this] { return bShowApproachGuide; }, [this] { bShowApproachGuide = !bShowApproachGuide; }), FMargin(0, 2));
	for (int32 a = 0; a < int32(AlienIdleTuning::EApproach::Count); ++a)
	{
		const AlienIdleTuning::FApproachDef& Def = AlienIdleTuning::Approaches[a];
		const FString Guide = FString::Printf(TEXT("%s: %s%s  Strengths: %s%s  Weaknesses: %s%s  Thesis: %s (%s department)%s  Rival: %s%s  Keep forever: %s"),
			Def.Name, Def.Pitch, LINE_TERMINATOR, Def.Strengths, LINE_TERMINATOR, Def.Weaknesses, LINE_TERMINATOR,
			AlienIdleTuning::Thesis(Def.Thesis).Name, AlienIdleTuning::Department(Def.Department).Name, LINE_TERMINATOR,
			Def.RivalInteraction, LINE_TERMINATOR, Def.PermanentWhy);
		Add(SNew(SBox)
			.Visibility_Lambda([this] { return bShowApproachGuide ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeLine([Guide] { return Str(Guide); }, [] { return FSlateColor(MutedColor); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 6)
				[
					MakeButton([Def] { return Str(FString::Printf(TEXT("Apply the %s template (targets, automation, lab, trade orders)"), Def.Name)); },
						[] { return false; }, [this, a] { Sim->ApplyApproachTemplate(AlienIdleTuning::EApproach(a)); })
				]
			]);
	}
	for (int32 Slot = 0; Slot < AlienIdleTuning::PresetSlots; ++Slot)
	{
		Add(SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				MakeLine([this, Slot] { return Str(Sim->GetPresetSummary(Slot)); }, [this, Slot] { return FSlateColor(Sim->GetCareer().Presets[Slot].bSaved ? TextColor : MutedColor); })
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
			[
				MakeButton([] { return Str(TEXT("Save")); }, [] { return false; }, [this, Slot]
				{
					bool bClear = false;
					const AlienIdleTuning::EApproach Style = Sim->ClassifyApproach(&bClear);
					Sim->SavePreset(Slot, bClear ? FString(AlienIdleTuning::Approach(Style).Name) : FString(TEXT("Mixed operation")));
				})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
			[
				MakeButton([] { return Str(TEXT("Load")); }, [this, Slot] { return Sim->GetCareer().Presets[Slot].bSaved; }, [this, Slot] { Sim->ApplyPreset(Slot); })
			], FMargin(0, 1));
	}

	// =============================================================================================
	// COLLABORATIONS: rivals, joint experiments, races, trades and contributions.
	Panel = Collaborations;
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Rivals", "RIVALS ON THIS ASSIGNMENT")), FMargin(0, 0, 0, 1));
	for (int32 r = 0; r < int32(ERival::Count); ++r)
	{
		const ERival Type = ERival(r);
		Add(SNew(SBox)
			.Visibility_Lambda([this, Type] { return Sim->IsRivalActive(Type) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this, Type]
				{
					const AlienIdleTuning::FRivalDef& Def = AlienIdleTuning::RivalDef(Type);
					return Str(FString::Printf(TEXT("%s: %s%sNow: %s%sYour options: %s"), Def.Name, Def.Behavior, LINE_TERMINATOR, *Sim->GetRivalEventText(Type), LINE_TERMINATOR, Def.Response));
				}, [this, Type]
				{
					const FRivalState& R = Sim->GetRival();
					const bool bBusy = Type == ERival::EfficiencyBureau ? (R.bSweeping || R.bAnnounced)
						: Type == ERival::CelebrityStudies ? (R.Celebrity.bActive || R.Celebrity.bAnnounced) : (R.Breach.bActive || R.Breach.bAnnounced);
					return FSlateColor(bBusy ? WarnColor : TextColor);
				})
			], FMargin(0, 2));
	}
	Add(MakeLine([this] { return Str(Sim->GetReputationText()); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 2, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Joint", "JOINT EXPERIMENTS AND CONTRIBUTIONS")));
	Add(MakeLine([this] { return Str(Sim->GetJointText()); }, [] { return FSlateColor(TextColor); }), FMargin(0, 1));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->GetJointRival() != ERival::Count ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([this]
			{
				return Str(Sim->GetRival().bContribute ? TEXT("[on]  Contributing specimens to the joint experiment")
					: TEXT("[off] Contribute specimens to the joint experiment"));
			}, [this] { return Sim->GetRival().bContribute; }, [this] { Sim->SetContribute(!Sim->GetRival().bContribute); })
		], FMargin(0, 1, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Race", "RACES")));
	Add(MakeLine([this] { return Str(Sim->GetRaceText()); }, [this] { return FSlateColor(Sim->GetRival().bRaceActive || Sim->GetRival().bRaceOffered ? HeadingColor : TextColor); }), FMargin(0, 1));
	Add(SNew(SHorizontalBox)
		.Visibility_Lambda([this] { return Sim->GetRival().bRaceOffered ? EVisibility::Visible : EVisibility::Collapsed; })
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
		[
			MakeButton([] { return Str(TEXT("Accept the race")); }, [] { return true; }, [this] { Sim->AcceptRace(); })
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			MakeButton([] { return Str(TEXT("Decline")); }, [] { return false; }, [this] { Sim->DeclineRace(); })
		], FMargin(0, 2, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Trades", "TRADES")));
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Standing trade orders (never expire). Trades this run: %d, income %.0f research. Lifetime trades: %d."),
			Sim->GetRival().TradesThisRun, Sim->GetRival().TradeIncome, Sim->GetCareer().Trades));
	}, [] { return FSlateColor(MutedColor); }), FMargin(0, 1));
	for (int32 k = 0; k < 3; ++k)
	{
		const ESpecimen Type = ESpecimen(k);
		Add(SNew(SBox)
			.Visibility_Lambda([this, Type] { return Sim->IsSpecimenUnlocked(Type) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this, Type]
				{
					return Str(FString::Printf(TEXT("%s Sell surplus %s when containment is 75%% full (%.0f%% of their routine research)"),
						Sim->GetRival().Sell[int32(Type)] ? TEXT("[on] ") : TEXT("[off]"), AlienIdleTuning::Specimen(Type).Plural, 100.0 * Sim->GetSellFraction()));
				}, [this, Type] { return Sim->GetRival().Sell[int32(Type)]; }, [this, Type] { Sim->SetSellOrder(Type, !Sim->GetRival().Sell[int32(Type)]); })
			], FMargin(0, 1));
	}
	for (int32 k = 0; k < 2; ++k)
	{
		const ESpecimen Type = ESpecimen(k);
		Add(SNew(SBox)
			.Visibility_Lambda([this, Type] { return Sim->IsSpecimenUnlocked(Type) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this, Type]
				{
					return Str(FString::Printf(TEXT("%s Buy a %s every %.0f s for %.0f research while there is room"),
						Sim->GetRival().Buy[int32(Type)] ? TEXT("[on] ") : TEXT("[off]"), AlienIdleTuning::Specimen(Type).Name,
						AlienIdleTuning::TradeBuyIntervalSeconds, Sim->GetBuyCost(Type)));
				}, [this, Type] { return Sim->GetRival().Buy[int32(Type)]; }, [this, Type] { Sim->SetBuyOrder(Type, !Sim->GetRival().Buy[int32(Type)]); })
			], FMargin(0, 1));
	}

	// =============================================================================================
	// Tabs: one scroll box per tab inside a switcher, so each keeps its own scroll position.
	TSharedRef<SHorizontalBox> TabBar = SNew(SHorizontalBox);
	TabSwitcher = SNew(SWidgetSwitcher);
	const TSharedRef<SVerticalBox> TabPanels[] = { Operations, Contracts, Thesis, Collaborations };
	static const TCHAR* TabNames[] = { TEXT("Operations"), TEXT("Contracts"), TEXT("Thesis"), TEXT("Collaborations") };
	// Widths follow the label (plus room for a badge) so "Collaborations !" fits in the narrow console.
	static const float TabWidths[] = { 13.0f, 12.0f, 9.0f, 17.0f };
	for (int32 t = 0; t < int32(EConsoleTab::Count); ++t)
	{
		TabSwitcher->AddSlot()
		[
			SNew(SScrollBox)
			.ScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll)
			+ SScrollBox::Slot().Padding(0, 0, 10, 0)
			[
				TabPanels[t]
			]
		];
		TabBar->AddSlot().FillWidth(TabWidths[t]).Padding(t == 0 ? 0.0f : 2.0f, 0, 0, 0)
		[
			SAssignNew(TabButtons[t], SButton)
			.ContentPadding(FMargin(4, 5))
			.HAlign(HAlign_Center)
			.ButtonColorAndOpacity_Lambda([this, t] { return FSlateColor(ActiveTab == t ? TabActiveColor : TabColor); })
			.OnClicked_Lambda([this, t] { SetTab(t); return FReply::Handled(); })
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(Font(9, TEXT("Bold")))
					.Text(Str(TabNames[t]))
					.ColorAndOpacity_Lambda([this, t] { return FSlateColor(ActiveTab == t ? HeadingColor : MutedColor); })
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(3, 0, 0, 0)
				[
					SNew(SBorder)
					.Visibility_Lambda([this, t] { return GetTabBadge(t).IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
					.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.BorderBackgroundColor(WarnColor)
					.Padding(FMargin(3, 0))
					[
						SNew(STextBlock)
						.Font(Font(8, TEXT("Bold")))
						.ColorAndOpacity(PanelColor)
						.Text_Lambda([this, t] { return Str(GetTabBadge(t)); })
					]
				]
			]
		];
	}
	TabSwitcher->SetActiveWidgetIndex(ActiveTab);

	// Experiment results box along the bottom of the screen, newest first.
	TSharedRef<SVerticalBox> ResultLines = SNew(SVerticalBox);
	for (int32 i = 0; i < VisibleResultLines; ++i)
	{
		ResultLines->AddSlot().AutoHeight().Padding(0, 0, 0, 3)
		[
			SNew(STextBlock)
			.Font(Font(10))
			.AutoWrapText(true)
			.Text_Lambda([this, i] { return GetResultLine(i); })
			.ColorAndOpacity_Lambda([this, i] { return FSlateColor(i == 0 ? TextColor : MutedColor); })
		];
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SAlienBaseView).Sim(Sim)
				.OnCalibrationClicked([this] { OpenCalibration(); })
				.OnStationClicked([this](const FAssignment& Station) { SelectStation(Station, true); })
				.GetSelectedStation([this] { return Selected; })
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(460.0f)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.BorderBackgroundColor(PanelColor)
					.Padding(FMargin(16, 14, 6, 14))
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 10, 0)
						[
							Header
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 10, 8)
						[
							TabBar
						]
						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							TabSwitcher.ToSharedRef()
						]
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(150.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(ResultsColor)
				.Padding(FMargin(16, 8, 6, 8))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
					[
						MakeHeading(NSLOCTEXT("AlienIdle", "Results", "EXPERIMENT RESULTS"))
					]
					+ SVerticalBox::Slot().FillHeight(1.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot().Padding(0, 0, 10, 0)
						[
							ResultLines
						]
					]
				]
			]
		]
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			MakeAwayReportPanel()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			MakePromotionPanel()
		]
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			MakeCalibrationPanel()
		]
	];
}

// ---------------------------------------------------------------------------------------------
// Console tabs and the Operations selection

void SAlienIdleRoot::SetTab(int32 Tab)
{
	ActiveTab = FMath::Clamp(Tab, 0, int32(EConsoleTab::Count) - 1);
	if (TabSwitcher.IsValid())
	{
		TabSwitcher->SetActiveWidgetIndex(ActiveTab);
	}
	if (ActiveTab == int32(EConsoleTab::Thesis))
	{
		CachedReview = Sim->BuildPromotionReview();
		ReviewCacheTime = FPlatformTime::Seconds();
	}
	// Focus follows the tab so a gamepad never sits on a control that is no longer shown.
	if (TabButtons[ActiveTab].IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(TabButtons[ActiveTab], EFocusCause::Navigation);
	}
}

FString SAlienIdleRoot::GetTabBadge(int32 Tab) const
{
	switch (EConsoleTab(Tab))
	{
	case EConsoleTab::Contracts:
		return Sim->GetUnclaimedContracts().Num() > 0 ? FString::FromInt(Sim->GetUnclaimedContracts().Num()) : FString();
	case EConsoleTab::Thesis:
		return Sim->CanSubmitThesis() ? FString(TEXT("!")) : FString();
	case EConsoleTab::Collaborations:
		return Sim->GetRival().bRaceOffered ? FString(TEXT("!")) : FString();
	default:
		return FString();
	}
}

FReply SAlienIdleRoot::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Unhandled keys bubble up here from the focused control: shoulder buttons switch tabs.
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Gamepad_LeftShoulder || Key == EKeys::Gamepad_RightShoulder)
	{
		const int32 Num = int32(EConsoleTab::Count);
		SetTab(((ActiveTab + (Key == EKeys::Gamepad_RightShoulder ? 1 : -1)) % Num + Num) % Num);
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SAlienIdleRoot::SelectStation(const FAssignment& Station, bool bShowOperations)
{
	Selected = Station;
	if (Station.Station == EStation::Ufo)
	{
		SelectedUfo = Station.Index;
	}
	if (bShowOperations && ActiveTab != int32(EConsoleTab::Operations))
	{
		SetTab(int32(EConsoleTab::Operations));
	}
}

void SAlienIdleRoot::StepSelection(int32 Direction)
{
	const TArray<FAssignment> All = Sim->GetAllStations();
	if (All.Num() == 0)
	{
		return;
	}
	int32 Current = All.IndexOfByPredicate([this](const FAssignment& A) { return A.Station == Selected.Station && A == Selected; });
	Current = Current == INDEX_NONE ? 0 : Current;
	SelectStation(All[((Current + Direction) % All.Num() + All.Num()) % All.Num()], false);
}

void SAlienIdleRoot::ValidateSelection()
{
	SelectedUfo = FMath::Clamp(SelectedUfo, 0, FMath::Max(0, Sim->GetNumUfos() - 1));
	const bool bExists = Sim->GetAllStations().ContainsByPredicate([this](const FAssignment& A) { return A.Station == Selected.Station && A == Selected; });
	if (!bExists)
	{
		Selected = { EStation::Ufo, SelectedUfo };
	}
}

FString SAlienIdleRoot::GetSelectionLabel() const
{
	const TArray<FAssignment> All = Sim->GetAllStations();
	const int32 Current = All.IndexOfByPredicate([this](const FAssignment& A) { return A.Station == Selected.Station && A == Selected; });
	const FString Counter = FString::Printf(TEXT("%d/%d  "), Current + 1, All.Num());
	if (Selected.Station == EStation::Ufo)
	{
		return Counter + FString::Printf(TEXT("UFO '%s': %s"), *Sim->GetUfo(SelectedUfo).Name, *Sim->GetShipRole(SelectedUfo));
	}
	return Counter + Sim->GetStationName(Selected);
}

FString SAlienIdleRoot::GetSelectionStatus() const
{
	switch (Selected.Station)
	{
	case EStation::Ufo:
		return Sim->GetUfoStatusText(SelectedUfo);
	case EStation::Chamber:
		return Sim->GetChambers().IsValidIndex(Selected.Index) ? Sim->GetChamberStatusText(Selected.Index) : FString();
	case EStation::Containment:
		return FString::Printf(TEXT("Containment: %d / %d units, %d specimens waiting for a lab."), Sim->GetContainmentUnitsUsed(), Sim->GetContainmentCapacity(), Sim->GetContainment().Num());
	case EStation::Power:
		return FString::Printf(TEXT("Generating +%.0f energy/s. Stored %.0f / %.0f."), Sim->GetEnergyPerSecond(), Sim->GetEnergy(), Sim->GetEnergyCap());
	case EStation::Pressure:
		return Sim->GetHullText();
	case EStation::Boiler:
		return Sim->GetHeatText();
	case EStation::Calibration:
		return Sim->GetCalibrationStatusText();
	case EStation::MemoryWipe:
		return Sim->IsStationStaffed(Selected) ? FString(TEXT("Returning specimens with fewer memories and less suspicion."))
			: FString(TEXT("Idle: returned specimens keep their memories (and their suspicion)."));
	default:
		return FString();
	}
}

bool SAlienIdleRoot::IsSelectionBlocked() const
{
	if (!Sim->IsStationStaffed(Selected))
	{
		return true;
	}
	switch (Selected.Station)
	{
	case EStation::Ufo:
	{
		const EUfoState State = Sim->GetUfo(SelectedUfo).State;
		return State == EUfoState::WaitingToUnload || (State == EUfoState::Docked && !Sim->CanLaunch(SelectedUfo));
	}
	case EStation::Chamber:
		return Sim->GetChambers().IsValidIndex(Selected.Index) && !Sim->GetChambers()[Selected.Index].bRunning && Sim->GetContainment().Num() > 0;
	case EStation::Containment:
		return Sim->GetContainmentUnitsUsed() >= Sim->GetContainmentCapacity();
	case EStation::Pressure:
		return Sim->GetHullPressure() >= AlienIdleTuning::HullCreakPressure;
	case EStation::Boiler:
		return Sim->GetLabWarmth() < 0.8f || Sim->GetHangarWarmth() < 0.8f;
	default:
		return false;
	}
}

float SAlienIdleRoot::GetSelectionProgress() const
{
	switch (Selected.Station)
	{
	case EStation::Ufo:
	{
		const EUfoState State = Sim->GetUfo(SelectedUfo).State;
		return State == EUfoState::FlyingOut || State == EUfoState::Capturing || State == EUfoState::Returning ? Sim->GetUfoStateProgress(SelectedUfo) : -1.0f;
	}
	case EStation::Chamber:
	{
		if (!Sim->GetChambers().IsValidIndex(Selected.Index))
		{
			return -1.0f;
		}
		const FLabChamber& Chamber = Sim->GetChambers()[Selected.Index];
		return Chamber.bRunning && Chamber.Duration > 0.0f ? FMath::Clamp(Chamber.Time / Chamber.Duration, 0.0f, 1.0f) : -1.0f;
	}
	case EStation::Containment:
		return Sim->GetContainmentCapacity() > 0 ? FMath::Clamp(float(Sim->GetContainmentUnitsUsed()) / Sim->GetContainmentCapacity(), 0.0f, 1.0f) : -1.0f;
	default:
		return -1.0f;
	}
}

bool SAlienIdleRoot::IsBaseUpgradeShown(EBaseUpgrade Upgrade) const
{
	if (!Sim->IsBaseUpgradeAvailable(Upgrade))
	{
		return false;
	}
	// Each base upgrade appears with the station it improves.
	switch (Upgrade)
	{
	case EBaseUpgrade::Hangar:
	case EBaseUpgrade::Camouflage:
	case EBaseUpgrade::SonarArray:
	case EBaseUpgrade::InkScreens:
		return Selected.Station == EStation::Ufo;
	case EBaseUpgrade::Chambers:
	case EBaseUpgrade::InsulatedLabs:
		return Selected.Station == EStation::Chamber;
	case EBaseUpgrade::CalibrationDesk:
		return Selected.Station == EStation::Chamber || Selected.Station == EStation::Calibration;
	case EBaseUpgrade::Containment:
	case EBaseUpgrade::CryoVault:
	case EBaseUpgrade::CryoPods:
		return Selected.Station == EStation::Containment;
	case EBaseUpgrade::DeepTanks:
		return Selected.Station == EStation::Containment || Selected.Station == EStation::Pressure;
	case EBaseUpgrade::Power:
		return Selected.Station == EStation::Power;
	case EBaseUpgrade::TidalTurbines:
		return Selected.Station == EStation::Power || Selected.Station == EStation::Pressure;
	case EBaseUpgrade::GeothermalTaps:
		return Selected.Station == EStation::Power || Selected.Station == EStation::Boiler;
	case EBaseUpgrade::Cleanup:
		return Selected.Station == EStation::MemoryWipe;
	default:
		return true;
	}
}

// ---------------------------------------------------------------------------------------------
// Probe Calibration: a five-second timing challenge. Stop the cursor in the band; the bright core is
// precise. Running out of time still counts as an attempt. "Standard calibration" skips the timing.

void SAlienIdleRoot::OpenCalibration()
{
	if (bShowingCalibration || bShowingReport || bShowingPromotion || !Sim->BeginCalibration())
	{
		return;
	}
	bShowingCalibration = true;
	bCalibrationResolved = false;
	CalibrationResult.Reset();
	CalibrationChamberShown = Sim->GetCalibrationChamber();
	CalibrationStartTime = FSlateApplication::Get().GetCurrentTime();
	if (CalibrationStopButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(CalibrationStopButton, EFocusCause::SetDirectly);
	}
}

float SAlienIdleRoot::GetCalibrationCursor() const
{
	if (bCalibrationResolved)
	{
		return CalibrationStoppedAt;
	}
	const double Elapsed = FSlateApplication::Get().GetCurrentTime() - CalibrationStartTime;
	const float Phase = FMath::Fmod(float(Elapsed) / AlienIdleTuning::CalibrationSweepSeconds, 2.0f);
	return Phase <= 1.0f ? Phase : 2.0f - Phase;
}

void SAlienIdleRoot::SubmitCalibration(ECalibrationGrade Grade)
{
	if (!bShowingCalibration || bCalibrationResolved)
	{
		return;
	}
	CalibrationStoppedAt = GetCalibrationCursor();
	bCalibrationResolved = true;
	CalibrationResolvedTime = FSlateApplication::Get().GetCurrentTime();
	Sim->Calibrate(Grade, &CalibrationResult);
	if (CalibrationCloseButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(CalibrationCloseButton, EFocusCause::SetDirectly);
	}
}

void SAlienIdleRoot::CloseCalibration()
{
	bShowingCalibration = false;
	if (LaunchButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(LaunchButton, EFocusCause::SetDirectly);
	}
}

void SAlienIdleRoot::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	ValidateSelection();
	if (ActiveTab == int32(EConsoleTab::Thesis) && FPlatformTime::Seconds() - ReviewCacheTime >= 1.0)
	{
		CachedReview = Sim->BuildPromotionReview();
		ReviewCacheTime = FPlatformTime::Seconds();
	}
	if (!bShowingCalibration)
	{
		return;
	}
	const double Now = FSlateApplication::Get().GetCurrentTime();
	if (!bCalibrationResolved && Now - CalibrationStartTime >= AlienIdleTuning::CalibrationChallengeSeconds)
	{
		SubmitCalibration(ECalibrationGrade::Missed); // out of time: still an attempt
	}
	else if (bCalibrationResolved && Now - CalibrationResolvedTime >= 6.0)
	{
		CloseCalibration();
	}
}

TSharedRef<SWidget> SAlienIdleRoot::MakeCalibrationPanel()
{
	auto WhileTiming = [this] { return bCalibrationResolved ? EVisibility::Collapsed : EVisibility::Visible; };
	auto WhenResolved = [this] { return bCalibrationResolved ? EVisibility::Visible : EVisibility::Collapsed; };
	return SNew(SBox)
		.WidthOverride(640.0f)
		.Visibility_Lambda([this] { return bShowingCalibration ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(Srgb(0.05f, 0.08f, 0.09f))
			.Padding(22.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(18, TEXT("Bold"))).ColorAndOpacity(HeadingColor)
					.Text_Lambda([this] { return Str(FString::Printf(TEXT("PROBE CALIBRATION: CHAMBER %d"), CalibrationChamberShown + 1)); })
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 12)
				[
					SNew(STextBlock).Font(Font(10)).ColorAndOpacity(MutedColor).AutoWrapText(true)
					.Text_Lambda([]
					{
						return Str(FString::Printf(TEXT("Stop the probe inside the green band: %.0f%% of the experiment's remaining time, %.0f%% in the bright centre. Any attempt gives %.0f%%. Standard calibration needs no timing and gives %.0f%%."),
							100.0f * FAlienIdleSim::GetCalibrationFraction(ECalibrationGrade::Steady), 100.0f * FAlienIdleSim::GetCalibrationFraction(ECalibrationGrade::Precise),
							100.0f * FAlienIdleSim::GetCalibrationFraction(ECalibrationGrade::Missed), 100.0f * FAlienIdleSim::GetCalibrationFraction(ECalibrationGrade::Quick)));
					})
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
				[
					SNew(SCalibrationGauge)
					.Cursor_Lambda([this] { return GetCalibrationCursor(); })
					.Band_Lambda([this] { return Sim->GetCalibrationBand(); })
					.BandHalfWidth(AlienIdleTuning::CalibrationBandHalfWidth)
					.CoreHalfWidth(AlienIdleTuning::CalibrationCoreHalfWidth)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 12)
				[
					SNew(STextBlock).Font(Font(11)).AutoWrapText(true)
					.Text_Lambda([this]
					{
						if (bCalibrationResolved)
						{
							return Str(CalibrationResult);
						}
						const double Left = AlienIdleTuning::CalibrationChallengeSeconds - (FSlateApplication::Get().GetCurrentTime() - CalibrationStartTime);
						return Str(FString::Printf(TEXT("%.1f s left. The subject has been asked to hold still."), FMath::Max(0.0, Left)));
					})
					.ColorAndOpacity_Lambda([this] { return FSlateColor(bCalibrationResolved ? HeadingColor : TextColor); })
				]
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 8, 0)
					[
						SNew(SBox).Visibility_Lambda(WhileTiming)
						[
							SAssignNew(CalibrationStopButton, SButton)
							.ContentPadding(FMargin(14, 6))
							.OnClicked_Lambda([this]
							{
								SubmitCalibration(Sim->GradeCalibration(GetCalibrationCursor()));
								return FReply::Handled();
							})
							[
								SNew(STextBlock).Font(Font(11, TEXT("Bold"))).Text(NSLOCTEXT("AlienIdle", "CalibrationStop", "Stop the probe (Enter / A)"))
							]
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox).Visibility_Lambda(WhileTiming)
						[
							MakeButton([] { return Str(TEXT("Standard calibration (no timing)")); }, [] { return true; },
								[this] { SubmitCalibration(ECalibrationGrade::Quick); })
						]
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SBox).Visibility_Lambda(WhenResolved)
						[
							SAssignNew(CalibrationCloseButton, SButton)
							.ContentPadding(FMargin(14, 6))
							.OnClicked_Lambda([this] { CloseCalibration(); return FReply::Handled(); })
							[
								SNew(STextBlock).Font(Font(11)).Text(NSLOCTEXT("AlienIdle", "CalibrationClose", "Back to work"))
							]
						]
					]
				]
			]
		];
}

TSharedRef<SWidget> SAlienIdleRoot::MakeAwayReportPanel()
{
	return SNew(SBox)
		.WidthOverride(680.0f)
		.Visibility_Lambda([this] { return bShowingReport ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(Srgb(0.06f, 0.08f, 0.07f))
			.Padding(24.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock).Font(Font(18, TEXT("Bold"))).ColorAndOpacity(HeadingColor)
					.Text(NSLOCTEXT("AlienIdle", "AwayTitle", "WHILE YOU WERE OUT"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 12)
				[
					SNew(STextBlock).Font(Font(11)).ColorAndOpacity(MutedColor)
					.Text(NSLOCTEXT("AlienIdle", "AwaySubtitle", "Operations continued during your absence. Management has taken credit."))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)
				[
					SNew(STextBlock).Font(Font(11)).ColorAndOpacity(TextColor).AutoWrapText(true)
					.Text_Lambda([this] { return GetAwayReportText(); })
				]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
				[
					SAssignNew(DismissButton, SButton)
					.ContentPadding(FMargin(14, 6))
					.OnClicked_Lambda([this]
					{
						bShowingReport = false;
						if (OnAwayReportDismissed)
						{
							OnAwayReportDismissed();
						}
						if (LaunchButton.IsValid())
						{
							FSlateApplication::Get().SetAllUserFocus(LaunchButton, EFocusCause::SetDirectly);
						}
						return FReply::Handled();
					})
					[
						SNew(STextBlock).Font(Font(11)).Text(NSLOCTEXT("AlienIdle", "AwayDismiss", "Acknowledge memo"))
					]
				]
			]
		];
}

void SAlienIdleRoot::ShowAwayReport(const FAwayReport& Report)
{
	AwayReport = Report;
	bShowingReport = true;
	if (DismissButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(DismissButton, EFocusCause::SetDirectly);
	}
}

FText SAlienIdleRoot::GetAwayReportText() const
{
	const FAwayReport& R = AwayReport;
	FString Text = FString::Printf(TEXT("You were away for %.0f minutes.\n\n"), R.Seconds / 60.0);
	Text += FString::Printf(TEXT("Specimens captured: %d\nExperiments completed: %d\nResearch earned: %.0f\nContracts completed: %d\n"),
		R.Captures, R.Experiments, R.ResearchEarned, R.Contracts);
	if (Sim->GetUnclaimedContracts().Num() > 0)
	{
		Text += FString::Printf(TEXT("Contract rewards waiting in the Contracts tab: %.0f research\n"), Sim->GetUnclaimedReward());
	}
	Text += R.Discoveries.Num() > 0
		? FString::Printf(TEXT("Discoveries: %s\n"), *FString::Join(R.Discoveries, TEXT(", ")))
		: FString(TEXT("Discoveries: none. Nobody chose a project, so nobody was promoted.\n"));
	Text += FString::Printf(TEXT("Incidents: %d (escapes: %d), all resolved themselves eventually.\n\n"), R.Incidents, R.Escapes);

	Text += TEXT("Lost productivity:\n");
	bool bAny = false;
	static const TCHAR* Labels[] = {
		TEXT(""),
		TEXT("Posts without crew"),
		TEXT("Labs and launches waiting for energy"),
		TEXT("Labs idle with nothing to study"),
		TEXT("Ships held because containment was full"),
		TEXT("Specimens queued with every chamber busy"),
	};
	for (int32 i = 1; i < int32(EBottleneck::Count); ++i)
	{
		const float Minutes = R.LostSeconds[i] / 60.0f;
		if (Minutes >= 0.5f)
		{
			Text += FString::Printf(TEXT("  - %s: %.1f station-minutes\n"), Labels[i], Minutes);
			bAny = true;
		}
	}
	if (!bAny)
	{
		Text += TEXT("  - None worth mentioning. Management is suspicious.\n");
	}
	Text += TEXT("\n") + R.Recommendation;
	return Str(Text);
}

void SAlienIdleRoot::AddShipUpgradeButton(const TSharedRef<SVerticalBox>& Box, EShipSystem System)
{
	Box->AddSlot().AutoHeight().Padding(0, 1)
	[
		MakeUpgradeCard([this, System]
		{
			const FUpgradeDef& Def = AlienIdleTuning::ShipSystems[int32(System)];
			const int32 Level = Sim->GetShipLevel(SelectedUfo, System);
			return Level >= Def.MaxLevel ? FString::Printf(TEXT("%s %s"), Def.Name, Roman(Level))
				: FString::Printf(TEXT("%s %s -> %s"), Def.Name, Roman(Level), Roman(Level + 1));
		}, [this, System]
		{
			const FUpgradeDef& Def = AlienIdleTuning::ShipSystems[int32(System)];
			if (Sim->GetShipLevel(SelectedUfo, System) >= Def.MaxLevel)
			{
				return FString(TEXT("max"));
			}
			return FString::Printf(TEXT("%.0f research%s"), Sim->GetShipUpgradeCost(SelectedUfo, System),
				Def.SlotsPerLevel > 1 ? *FString::Printf(TEXT(", %d slots"), Def.SlotsPerLevel) : TEXT(""));
		}, [System] { return FAlienIdleSim::GetShipUpgradeEffectText(System); }, [this, System]
		{
			FString Reason;
			const bool bMax = Sim->GetShipLevel(SelectedUfo, System) >= AlienIdleTuning::ShipSystems[int32(System)].MaxLevel;
			return bMax || Sim->CanBuyShipUpgrade(SelectedUfo, System, &Reason) ? FString() : Reason;
		}, [this, System] { return Sim->CanBuyShipUpgrade(SelectedUfo, System); }, [this, System] { Sim->BuyShipUpgrade(SelectedUfo, System); })
	];
}

void SAlienIdleRoot::AddBaseUpgradeButton(const TSharedRef<SVerticalBox>& Box, EBaseUpgrade Upgrade)
{
	Box->AddSlot().AutoHeight().Padding(0, 1)
	[
		SNew(SBox)
		.Visibility_Lambda([this, Upgrade] { return IsBaseUpgradeShown(Upgrade) ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeUpgradeCard([this, Upgrade]
			{
				const FUpgradeDef& Def = AlienIdleTuning::BaseUpgrades[int32(Upgrade)];
				const int32 Level = Sim->GetBaseLevel(Upgrade);
				return Level >= Def.MaxLevel ? FString::Printf(TEXT("%s %s"), Def.Name, Roman(Level))
					: FString::Printf(TEXT("%s %s -> %s"), Def.Name, Roman(Level), Roman(Level + 1));
			}, [this, Upgrade]
			{
				return Sim->GetBaseLevel(Upgrade) >= AlienIdleTuning::BaseUpgrades[int32(Upgrade)].MaxLevel ? FString(TEXT("max"))
					: FString::Printf(TEXT("%.0f research"), Sim->GetBaseUpgradeCost(Upgrade));
			}, [Upgrade] { return FAlienIdleSim::GetBaseUpgradeEffectText(Upgrade); }, [this, Upgrade]
			{
				FString Reason;
				const bool bMax = Sim->GetBaseLevel(Upgrade) >= AlienIdleTuning::BaseUpgrades[int32(Upgrade)].MaxLevel;
				return bMax || Sim->CanBuyBaseUpgrade(Upgrade, &Reason) ? FString() : Reason;
			}, [this, Upgrade] { return Sim->CanBuyBaseUpgrade(Upgrade); }, [this, Upgrade] { Sim->BuyBaseUpgrade(Upgrade); })
		]
	];
}

TSharedRef<SButton> SAlienIdleRoot::MakeUpgradeCard(TFunction<FString()> Title, TFunction<FString()> Cost, TFunction<FString()> Next,
	TFunction<FString()> Blocker, TFunction<bool()> CanBuy, TFunction<void()> OnClick) const
{
	// One compact card: name and level on the left, cost on the right, the next improvement under it,
	// and why it cannot be bought (if it cannot) in the warning color.
	TSharedRef<TFunction<FString()>> BlockerText = MakeShared<TFunction<FString()>>(MoveTemp(Blocker));
	TSharedRef<TFunction<bool()>> Affordable = MakeShared<TFunction<bool()>>(MoveTemp(CanBuy));
	return SNew(SButton)
		.ContentPadding(FMargin(8, 4))
		.OnClicked_Lambda([OnClick = MoveTemp(OnClick)] { OnClick(); return FReply::Handled(); })
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(Font(10, TEXT("Bold")))
					.Text_Lambda([Title = MoveTemp(Title)] { return Str(Title()); })
					.ColorAndOpacity_Lambda([Affordable] { return FSlateColor((*Affordable)() ? HeadingColor : TextColor); })
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 0, 0)
				[
					SNew(STextBlock)
					.Font(Font(10, TEXT("Bold")))
					.Text_Lambda([Cost = MoveTemp(Cost)] { return Str(Cost()); })
					.ColorAndOpacity_Lambda([Affordable] { return FSlateColor((*Affordable)() ? HeadingColor : MutedColor); })
				]
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Font(Font(9))
				.AutoWrapText(true)
				.ColorAndOpacity(MutedColor)
				.Text_Lambda([Next = MoveTemp(Next)] { return Str(Next()); })
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Font(Font(9))
				.AutoWrapText(true)
				.ColorAndOpacity(WarnColor)
				.Visibility_Lambda([BlockerText] { return (*BlockerText)().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible; })
				.Text_Lambda([BlockerText] { return Str(FString::Printf(TEXT("Unavailable: %s"), *(*BlockerText)())); })
			]
		];
}

TSharedRef<SWidget> SAlienIdleRoot::MakeHeading(const FText& Text) const
{
	return SNew(STextBlock)
		.Font(Font(11, TEXT("Bold")))
		.ColorAndOpacity(HeadingColor)
		.Text(Text);
}

TSharedRef<SWidget> SAlienIdleRoot::MakeLine(TFunction<FText()> TextGetter, TFunction<FSlateColor()> ColorGetter) const
{
	TSharedRef<STextBlock> Line = SNew(STextBlock)
		.Font(Font(10))
		.AutoWrapText(true)
		.Text_Lambda(MoveTemp(TextGetter));
	if (ColorGetter)
	{
		Line->SetColorAndOpacity(TAttribute<FSlateColor>::CreateLambda(MoveTemp(ColorGetter)));
	}
	else
	{
		Line->SetColorAndOpacity(TextColor);
	}
	return Line;
}

TSharedRef<SButton> SAlienIdleRoot::MakeButton(TFunction<FText()> Label, TFunction<bool()> IsHighlighted, TFunction<void()> OnClick) const
{
	// Buttons stay enabled so gamepad focus never lands on a dead control; the label says why an action is unavailable.
	return SNew(SButton)
		.ContentPadding(FMargin(8, 5))
		.OnClicked_Lambda([OnClick = MoveTemp(OnClick)] { OnClick(); return FReply::Handled(); })
		[
			SNew(STextBlock)
			.Font(Font(10))
			.AutoWrapText(true)
			.Text_Lambda(MoveTemp(Label))
			.ColorAndOpacity_Lambda([IsHighlighted = MoveTemp(IsHighlighted)] { return FSlateColor(IsHighlighted() ? HeadingColor : MutedColor); })
		];
}

TSharedRef<SWidget> SAlienIdleRoot::MakeStepper(TFunction<FText()> Label, TFunction<void(int32)> OnStep) const
{
	TSharedRef<TFunction<void(int32)>> Step = MakeShared<TFunction<void(int32)>>(MoveTemp(OnStep));
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakeButton([] { return Str(TEXT("<")); }, [] { return true; }, [Step] { (*Step)(-1); })
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(8, 0)
		[
			MakeLine(MoveTemp(Label))
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakeButton([] { return Str(TEXT(">")); }, [] { return true; }, [Step] { (*Step)(1); })
		];
}

FText SAlienIdleRoot::GetLaunchLabel() const
{
	FString Reason;
	if (Sim->CanLaunch(SelectedUfo, &Reason))
	{
		return Str(FString::Printf(TEXT("Dispatch '%s'"), *Sim->GetUfo(SelectedUfo).Name));
	}
	return Str(FString::Printf(TEXT("Dispatch: %s"), *Reason));
}

FText SAlienIdleRoot::GetLogLine(int32 IndexFromNewest) const
{
	const TArray<FAlienIdleLogEntry>& Log = Sim->GetLog();
	const int32 Index = Log.Num() - 1 - IndexFromNewest;
	if (!Log.IsValidIndex(Index))
	{
		return FText::GetEmpty();
	}
	const int32 Seconds = FMath::FloorToInt32(Log[Index].Time);
	return Str(FString::Printf(TEXT("[%02d:%02d] %s"), Seconds / 60, Seconds % 60, *Log[Index].Text));
}

FText SAlienIdleRoot::GetResultLine(int32 IndexFromNewest) const
{
	const TArray<FAlienIdleLogEntry>& Results = Sim->GetResults();
	const int32 Index = Results.Num() - 1 - IndexFromNewest;
	if (!Results.IsValidIndex(Index))
	{
		return IndexFromNewest == 0 ? Str(TEXT("No results yet. Specimens must first be acquired, then studied, then returned with no memory of it.")) : FText::GetEmpty();
	}
	const int32 Seconds = FMath::FloorToInt32(Results[Index].Time);
	return Str(FString::Printf(TEXT("[%02d:%02d] %s"), Seconds / 60, Seconds % 60, *Results[Index].Text));
}

void SAlienIdleRoot::OpenPromotion()
{
	Promotion = Sim->BuildPromotionReview();
	SelectedPermanent = Promotion.Options.Num() > 0 ? 0 : INDEX_NONE;
	SelectedThesis = 0;
	SelectedLocation = ELocation::Metropolis;
	for (bool& Planned : PlannedUpgrades)
	{
		Planned = false;
	}
	PlannedSpecialty = Sim->GetCareer().StartingSpecialty;
	PromotionError.Reset();
	bShowingPromotion = true;
	if (PromotionConfirmButton.IsValid())
	{
		FSlateApplication::Get().SetAllUserFocus(PromotionConfirmButton, EFocusCause::SetDirectly);
	}
}

int32 SAlienIdleRoot::GetPlannedGrantSpend() const
{
	int32 Spend = 0;
	for (int32 i = 0; i < int32(EGrantUpgrade::Count); ++i)
	{
		if (PlannedUpgrades[i] && !Sim->HasGrantUpgrade(EGrantUpgrade(i)))
		{
			Spend += AlienIdleTuning::GrantUpgrades[i].Cost;
		}
	}
	return Spend;
}

TSharedRef<SWidget> SAlienIdleRoot::MakePromotionPanel()
{
	TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
	auto Add = [&Body](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0, 1))
	{
		Body->AddSlot().AutoHeight().Padding(Padding)[Widget];
	};
	auto Lines = [this](TFunction<TArray<FString>()> Getter, const TCHAR* Bullet)
	{
		return MakeLine([Getter = MoveTemp(Getter), Bullet]
		{
			FString Text;
			for (const FString& Line : Getter())
			{
				Text += FString(Bullet) + Line + LINE_TERMINATOR;
			}
			return Str(Text.TrimEnd());
		});
	};

	Add(SNew(STextBlock).Font(Font(18, TEXT("Bold"))).ColorAndOpacity(HeadingColor).Text(NSLOCTEXT("AlienIdle", "PromoTitle", "ACADEMY THESIS REVIEW")));
	Add(MakeLine([this] { return Str(FString::Printf(TEXT("%s  ->  %s"), *Promotion.RankBefore, *Promotion.RankAfter)); }, [] { return FSlateColor(HeadingColor); }));
	Add(MakeLine([this] { return Str(Promotion.Review); }, [] { return FSlateColor(TextColor); }), FMargin(0, 6));
	Add(Lines([this] { return Promotion.Stats; }, TEXT("")), FMargin(0, 2, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoGrants", "RESEARCH GRANTS")));
	Add(Lines([this] { return Promotion.GrantLines; }, TEXT("  ")));
	Add(MakeLine([this] { return Str(FString::Printf(TEXT("Total awarded: %d grants (balance after: %d)"), Promotion.Grants, Sim->GetCareer().Grants + Promotion.Grants)); },
		[] { return FSlateColor(HeadingColor); }), FMargin(0, 2, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoResets", "WHAT RESETS")));
	Add(Lines([this] { return Promotion.Resets; }, TEXT("  - ")), FMargin(0, 1, 0, 4));
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoKeeps", "WHAT YOU KEEP")));
	Add(Lines([this] { return Promotion.Keeps; }, TEXT("  - ")), FMargin(0, 1, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoPermanent", "KEEP ONE DEPARTMENT BENEFIT FOREVER")));
	for (int32 i = 0; i < 5; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return Promotion.Options.IsValidIndex(i) ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this, i]
				{
					if (!Promotion.Options.IsValidIndex(i))
					{
						return FText::GetEmpty();
					}
					const FPermanentOption& Option = Promotion.Options[i];
					FString Suits;
					for (const AlienIdleTuning::FApproachDef& Approach : AlienIdleTuning::Approaches)
					{
						Suits += !Option.bSignature && Approach.Permanent == Option.Project ? FString::Printf(TEXT(" (suits %s)"), Approach.Name) : FString();
					}
					return Str(FString::Printf(TEXT("%s %s: %s%s"), SelectedPermanent == i ? TEXT("[keep]") : TEXT("[    ]"), *Option.Name, *Option.Effect, *Suits));
				}, [this, i] { return SelectedPermanent == i; }, [this, i] { SelectedPermanent = i; })
			]);
	}
	Add(SNullWidget::NullWidget, FMargin(0, 4));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoUpgrades", "GRANT UPGRADES (KEPT FOREVER)")));
	for (int32 i = 0; i < int32(EGrantUpgrade::Count); ++i)
	{
		const EGrantUpgrade Upgrade = EGrantUpgrade(i);
		Add(MakeButton([this, i, Upgrade]
		{
			const AlienIdleTuning::FGrantUpgradeDef& Def = AlienIdleTuning::GrantUpgrades[i];
			const TCHAR* State = Sim->HasGrantUpgrade(Upgrade) ? TEXT("[owned]") : PlannedUpgrades[i] ? TEXT("[buy]  ") : TEXT("[    ] ");
			FString Extra;
			if (Upgrade == EGrantUpgrade::SpecialistRecruit)
			{
				Extra = FString::Printf(TEXT(" Specialty: %s (press again to change)."), FAlienIdleSim::GetSpecialtyName(PlannedSpecialty));
			}
			return Str(FString::Printf(TEXT("%s %s (%d grants): %s%s"), State, Def.Name, Def.Cost, Def.Effect, *Extra));
		}, [this, i, Upgrade] { return Sim->HasGrantUpgrade(Upgrade) || PlannedUpgrades[i]; }, [this, i, Upgrade]
		{
			if (Sim->HasGrantUpgrade(Upgrade))
			{
				if (Upgrade == EGrantUpgrade::SpecialistRecruit)
				{
					PlannedSpecialty = ESpecialty((int32(PlannedSpecialty) + 1) % int32(ESpecialty::Count));
				}
				return;
			}
			if (PlannedUpgrades[i] && Upgrade == EGrantUpgrade::SpecialistRecruit && int32(PlannedSpecialty) + 1 < int32(ESpecialty::Count))
			{
				PlannedSpecialty = ESpecialty(int32(PlannedSpecialty) + 1);
				return;
			}
			if (PlannedUpgrades[i])
			{
				PlannedUpgrades[i] = false;
				PlannedSpecialty = ESpecialty::Pilot;
				return;
			}
			const int32 Budget = Sim->GetCareer().Grants + Promotion.Grants - GetPlannedGrantSpend();
			if (Budget >= AlienIdleTuning::GrantUpgrades[i].Cost)
			{
				PlannedUpgrades[i] = true;
			}
		}));
	}
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Grants left after these upgrades: %d"), Sim->GetCareer().Grants + Promotion.Grants - GetPlannedGrantSpend()));
	}, [] { return FSlateColor(MutedColor); }), FMargin(0, 1, 0, 8));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoNext", "NEXT ASSIGNMENT")));
	for (int32 i = 0; i < 5; ++i)
	{
		Add(SNew(SBox)
		.Visibility_Lambda([this, i] { return Sim->GetNextThesisOffers().IsValidIndex(i) ? EVisibility::Visible : EVisibility::Collapsed; })
		[
		MakeButton([this, i]
		{
			const TArray<EThesis> Offers = Sim->GetNextThesisOffers();
			if (!Offers.IsValidIndex(i))
			{
				return FText::GetEmpty();
			}
			const AlienIdleTuning::FThesisDef& Def = AlienIdleTuning::Thesis(Offers[i]);
			TArray<FString> RivalNames;
			for (int32 r = 0; r < int32(ERival::Count); ++r)
			{
				if (AlienIdleTuning::RivalPairs[int32(Offers[i])][r])
				{
					RivalNames.Add(AlienIdleTuning::Rivals[r].Name);
				}
			}
			FString Fit;
			for (const AlienIdleTuning::FApproachDef& Approach : AlienIdleTuning::Approaches)
			{
				if (Approach.Thesis == Offers[i])
				{
					Fit = FString::Printf(TEXT("%sSuits %s: %s Watch out: %s%sBest pairing: %s department, keep %s. %s"), LINE_TERMINATOR, Approach.Name, Approach.Strengths,
						Approach.Weaknesses, LINE_TERMINATOR, AlienIdleTuning::Department(Approach.Department).Name, AlienIdleTuning::DeptProjects[int32(Approach.Permanent)].Name,
						Approach.RivalInteraction);
				}
			}
			return Str(FString::Printf(TEXT("%s %s: %s%sRule: %s%sRivals: %s%s"), SelectedThesis == i ? TEXT("[pick]") : TEXT("[    ]"), Def.Name, Def.Brief, LINE_TERMINATOR, Def.Rule,
				LINE_TERMINATOR, *FString::Join(RivalNames, TEXT(" and ")), *Fit));
		}, [this, i] { return SelectedThesis == i; }, [this, i]
		{
			SelectedThesis = i;
			const TArray<EThesis> Offers = Sim->GetNextThesisOffers();
			if (Offers.IsValidIndex(i) && AlienIdleTuning::Thesis(Offers[i]).RequiredLocation != ELocation::Count)
			{
				SelectedLocation = AlienIdleTuning::Thesis(Offers[i]).RequiredLocation;
			}
		})
		]);
	}
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoEarth", "EARTH CONDITIONS FOR THE NEXT RUN")), FMargin(0, 6, 0, 1));
	for (int32 c = 1; c < int32(EEarthCondition::Count); ++c)
	{
		const EEarthCondition Earth = EEarthCondition(c);
		Add(MakeButton([this, Earth]
		{
			const AlienIdleTuning::FEarthConditionDef& Def = AlienIdleTuning::EarthCondition(Earth);
			return Str(FString::Printf(TEXT("%s %s: %s"), SelectedCondition == Earth ? TEXT("[pick]") : TEXT("[    ]"), Def.Name, Def.Preview));
		}, [this, Earth] { return SelectedCondition == Earth; }, [this, Earth] { SelectedCondition = Earth; }));
	}
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "PromoWhere", "LOCATION")), FMargin(0, 6, 0, 1));
	for (int32 l = 0; l < int32(ELocation::Count); ++l)
	{
		const ELocation Where = ELocation(l);
		Add(MakeButton([this, Where]
		{
			const bool bNew = Where == ELocation::Metropolis && !Sim->IsLocationUnlocked(Where);
			const bool bLocked = !bNew && !Sim->IsLocationUnlocked(Where);
			FString Note = bNew ? TEXT(" (unlocked by this promotion)") : FString();
			for (const AlienIdleTuning::FAchievementDef& Achievement : AlienIdleTuning::Achievements)
			{
				Note += bLocked && Achievement.Unlocks == Where ? FString::Printf(TEXT(" (locked: Academy achievement '%s', %s)"), Achievement.Name, Achievement.Requirement) : FString();
			}
			return Str(FString::Printf(TEXT("%s %s%s: %s"), SelectedLocation == Where ? TEXT("[here]") : TEXT("[    ]"),
				AlienIdleTuning::LocationNames[int32(Where)], *Note, AlienIdleTuning::LocationBlurbs[int32(Where)]));
		}, [this, Where] { return SelectedLocation == Where; }, [this, Where]
		{
			if (Sim->IsLocationUnlocked(Where) || Where == ELocation::Metropolis)
			{
				SelectedLocation = Where;
			}
		}));
	}
	Add(MakeLine([this] { return Str(PromotionError); }, [] { return FSlateColor(WarnColor); }), FMargin(0, 6, 0, 2));

	Add(SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 6, 0)
		[
			SAssignNew(PromotionConfirmButton, SButton)
			.ContentPadding(FMargin(12, 8))
			.OnClicked_Lambda([this]
			{
				FNextAssignment Next;
				Next.Thesis = Sim->GetNextThesisOffers()[SelectedThesis];
				Next.Location = SelectedLocation;
				Next.PermanentOption = SelectedPermanent;
				Next.StartingSpecialty = PlannedSpecialty;
				Next.Condition = SelectedCondition;
				for (int32 i = 0; i < int32(EGrantUpgrade::Count); ++i)
				{
					if (PlannedUpgrades[i])
					{
						Next.Buy.Add(EGrantUpgrade(i));
					}
				}
				FString Reason;
				if (Sim->Promote(Next, &Reason))
				{
					bShowingPromotion = false;
					SelectStation({ EStation::Ufo, 0 }, true);
					if (LaunchButton.IsValid())
					{
						FSlateApplication::Get().SetAllUserFocus(LaunchButton, EFocusCause::SetDirectly);
					}
				}
				else
				{
					PromotionError = TEXT("Cannot submit yet: ") + Reason;
				}
				return FReply::Handled();
			})
			[
				SNew(STextBlock).Font(Font(11, TEXT("Bold"))).Text(NSLOCTEXT("AlienIdle", "PromoConfirm", "Submit thesis and begin the next assignment"))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			MakeButton([] { return Str(TEXT("Not yet")); }, [] { return false; }, [this] { bShowingPromotion = false; })
		], FMargin(0, 4));

	return SNew(SBox)
		.WidthOverride(820.0f)
		.MaxDesiredHeight(940.0f)
		.Visibility_Lambda([this] { return bShowingPromotion ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(Srgb(0.05f, 0.07f, 0.09f))
			.Padding(22.0f)
			[
				SNew(SScrollBox)
				.ScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll)
				+ SScrollBox::Slot().Padding(0, 0, 10, 0)
				[
					Body
				]
			]
		];
}
