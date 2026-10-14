#include "UI/SAlienIdleRoot.h"
#include "UI/SAlienBaseView.h"
#include "UI/SCalibrationGauge.h"
#include "AlienIdleSim.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
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
	constexpr int32 MaxStationsShown = 12;
	constexpr int32 MaxCrewShown = 16;
	constexpr int32 MaxChambersShown = 3;

	const FLinearColor PanelColor = Srgb(0.035f, 0.04f, 0.055f);
	const FLinearColor ResultsColor = Srgb(0.02f, 0.025f, 0.035f);
	const FLinearColor HeadingColor = Srgb(0.55f, 0.95f, 0.6f);
	const FLinearColor TextColor = Srgb(0.85f, 0.87f, 0.9f);
	const FLinearColor MutedColor = Srgb(0.55f, 0.58f, 0.62f);
	const FLinearColor WarnColor = Srgb(1.0f, 0.65f, 0.35f);

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

	TSharedRef<SVerticalBox> Panel = SNew(SVerticalBox);
	auto Add = [&Panel](const TSharedRef<SWidget>& Widget, const FMargin& Padding = FMargin(0))
	{
		Panel->AddSlot().AutoHeight().Padding(Padding)[Widget];
	};

	// Header and resources
	Add(SNew(STextBlock).Font(Font(16, TEXT("Bold"))).ColorAndOpacity(HeadingColor)
		.Text_Lambda([this] { return Str(FString(AlienIdleTuning::LocationNames[int32(Sim->GetLocation())]).ToUpper()); }));
	Add(SNew(STextBlock).Font(Font(10)).ColorAndOpacity(MutedColor)
		.Text_Lambda([this]
		{
			return Str(FString::Printf(TEXT("Research Officer Console, %s grade. Run %d, Research Grants %d."),
				*Sim->GetRankName(), Sim->GetCareer().Run, Sim->GetCareer().Grants));
		}), FMargin(0, 0, 0, 10));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Resources", "RESOURCES")));
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Energy %.0f / %.0f  (+%.0f/s)     Research %.0f"), Sim->GetEnergy(), Sim->GetEnergyCap(), Sim->GetEnergyPerSecond(), Sim->GetResearch()));
	}));
	Add(MakeLine([this]
	{
		FString Line = TEXT("Experiments:");
		for (int32 i = 0; i < int32(ESpecimen::Count); ++i)
		{
			if (Sim->IsSpecimenUnlocked(ESpecimen(i)))
			{
				const FSpecimenDef& Def = AlienIdleTuning::Specimens[i];
				Line += FString::Printf(TEXT("  %s %d"), Def.Plural, Sim->GetExperimentsCompleted(ESpecimen(i)));
			}
		}
		return Str(Line);
	}, [] { return FSlateColor(MutedColor); }));
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Discoveries %d/%d   Encyclopedia %d/%d   Contracts done %d"), Sim->GetDiscoveryCount(), int32(EDiscovery::Count),
			Sim->GetEncyclopedia().Num(), FAlienIdleSim::GetEncyclopediaSize(), Sim->GetContractsCompleted()));
	}, [] { return FSlateColor(MutedColor); }), FMargin(0, 0, 0, 10));

	// Status: what limits progress and what to buy next
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Status", "BOTTLENECK")));
	Add(MakeLine([this] { return Str(Sim->GetBottleneckText()); },
		[this] { return FSlateColor(Sim->GetCurrentBottleneck() == EBottleneck::None ? TextColor : WarnColor); }));
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

	// Build: what kind of operation this is, approach templates and saved presets
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
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 10));

	// Thesis
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
	}, [this] { return FSlateColor(Sim->GetEarthCondition() == EEarthCondition::BusinessAsUsual ? MutedColor : WarnColor); }));
	Add(MakeLine([this] { return Str(TEXT("Academy achievements:") LINE_TERMINATOR + Sim->GetAchievementText()); }, [] { return FSlateColor(MutedColor); }));
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
	Add(MakeButton([this]
	{
		FString Reason;
		return Sim->CanSubmitThesis(&Reason) ? Str(TEXT("Submit thesis to the Galactic Academy (review first)"))
			: Str(FString::Printf(TEXT("Submit thesis: %s"), *Reason));
	}, [this] { return Sim->CanSubmitThesis(); }, [this] { if (Sim->CanSubmitThesis()) { OpenPromotion(); } }), FMargin(0, 2, 0, 10));

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
				MakeButton([this, Project]
				{
					const FDeptProjectDef& Def = AlienIdleTuning::DeptProject(Project);
					const TCHAR* DeptName = AlienIdleTuning::Department(Def.Department).Name;
					if (Sim->IsProjectDone(Project))
					{
						return Str(FString::Printf(TEXT("%s: %s (done)%s%s"), DeptName, Def.Name, LINE_TERMINATOR, Def.Effect));
					}
					FString Reason;
					const bool bCan = Sim->CanBuyProject(Project, &Reason);
					return Str(FString::Printf(TEXT("%s: %s  %.0f research%s%s%s"), DeptName, Def.Name, Sim->GetProjectCost(Project), LINE_TERMINATOR, Def.Effect,
						bCan ? TEXT("") : *FString::Printf(TEXT("%sUnavailable: %s"), LINE_TERMINATOR, *Reason)));
				}, [this, Project] { return Sim->CanBuyProject(Project); }, [this, Project] { Sim->BuyProject(Project); })
			], FMargin(0, 2));
	}
	for (int32 d = 1; d < int32(EDepartment::Count); ++d)
	{
		const EDepartment Dept = EDepartment(d);
		Add(SNew(SBox)
			.Visibility_Lambda([this, Dept] { return Sim->GetPrimaryDepartment() != EDepartment::None ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this, Dept]
				{
					FString Reason;
					const bool bCan = Sim->CanBeginSignature(Dept, &Reason);
					return Str(FString::Printf(TEXT("%sSignature: %s"), bCan ? TEXT("Begin ") : TEXT(""), *Sim->GetSignatureStatusText(Dept)));
				}, [this, Dept] { return Sim->CanBeginSignature(Dept) || Sim->GetPendingSignature() == Dept; }, [this, Dept] { Sim->BeginSignature(Dept); })
			], FMargin(0, 2));
	}
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// Contracts
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Contracts", "CONTRACTS")));
	Add(MakeLine([this]
	{
		if (!Sim->HasActiveContract())
		{
			return Str(TEXT("No active contract. Accept one below."));
		}
		const FContract& Contract = Sim->GetActiveContract();
		return Str(FString::Printf(TEXT("Active: %s  (%.0f research)%sProgress: %s"), *Contract.Title, Contract.Reward, LINE_TERMINATOR, *Contract.GetProgressText()));
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
		return Str(Sim->IsAutoContractsOn() ? TEXT("Auto-accept contracts: ON (best-paying offer your ships are set up for, also while you are away)")
			: TEXT("Auto-accept contracts: OFF"));
	}, [this] { return Sim->IsAutomationUnlocked() && Sim->IsAutoContractsOn(); }, [this] { Sim->SetAutoContracts(!Sim->IsAutoContractsOn()); }), FMargin(0, 0, 0, 2));
	for (int32 i = 0; i < AlienIdleTuning::ContractOffers; ++i)
	{
		Add(MakeButton([this, i]
		{
			const TArray<FContract>& Offers = Sim->GetContractOffers();
			if (!Offers.IsValidIndex(i))
			{
				return FText::GetEmpty();
			}
			const FContract& Offer = Offers[i];
			return Str(FString::Printf(TEXT("%s%s  (%.0f research)%s%s"), Sim->HasActiveContract() ? TEXT("") : TEXT("Accept: "), *Offer.Title, Offer.Reward, LINE_TERMINATOR, *Offer.Flavor));
		}, [this] { return !Sim->HasActiveContract(); }, [this, i] { Sim->AcceptContract(i); }), FMargin(0, 2));
	}
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// Fleet
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Fleet", "FLEET")));
	Add(MakeStepper([this]
	{
		SelectedUfo = FMath::Clamp(SelectedUfo, 0, Sim->GetNumUfos() - 1);
		return Str(FString::Printf(TEXT("UFO %d/%d  '%s'  %s"), SelectedUfo + 1, Sim->GetNumUfos(), *Sim->GetUfo(SelectedUfo).Name, *Sim->GetShipRole(SelectedUfo)));
	}, [this](int32 Direction)
	{
		const int32 Num = Sim->GetNumUfos();
		SelectedUfo = ((SelectedUfo + Direction) % Num + Num) % Num;
	}));
	Add(MakeLine([this] { return Str(Sim->GetUfoStatusText(SelectedUfo)); }, [this]
	{
		const EUfoState State = Sim->GetUfo(SelectedUfo).State;
		return FSlateColor(State == EUfoState::WaitingToUnload || (State == EUfoState::Docked && !Sim->CanLaunch(SelectedUfo)) ? WarnColor : TextColor);
	}), FMargin(0, 2));
	Add(MakeStepper([this]
	{
		const FUfo& Ufo = Sim->GetUfo(SelectedUfo);
		const FSpecimenDef& Def = AlienIdleTuning::Specimen(Ufo.Destination);
		const AlienIdleTuning::FRegionDef& Region = Sim->GetRegionDef(Ufo.Region);
		return Str(FString::Printf(TEXT("Target: %s in %s (value x%.1f, exposure x%.1f, wt %d)  %.0f energy"), Def.Plural, Region.Name, Region.Value, Region.Exposure,
			Def.Weight, Sim->GetLaunchEnergy(SelectedUfo, Ufo.Destination)));
	}, [this](int32 Direction) { Sim->CycleDestination(SelectedUfo, Direction); }));
	Add(MakeButton([this] { return Str(bShowTargetMath ? TEXT("Why these numbers: hide") : TEXT("Why these numbers: show research bonuses and mission penalties")); },
		[this] { return bShowTargetMath; }, [this] { bShowTargetMath = !bShowTargetMath; }), FMargin(0, 2, 0, 0));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return bShowTargetMath ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeLine([this] { return Str(Sim->ExplainTarget(SelectedUfo)); }, [] { return FSlateColor(MutedColor); })
		], FMargin(0, 1, 0, 2));

	LaunchButton = MakeButton([this] { return GetLaunchLabel(); }, [this] { return Sim->CanLaunch(SelectedUfo); }, [this] { Sim->Launch(SelectedUfo); });
	Add(LaunchButton.ToSharedRef(), FMargin(0, 4, 0, 2));

	Add(SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
		[
			MakeButton([this]
			{
				if (!Sim->IsAutomationUnlocked())
				{
					return Str(FString::Printf(TEXT("Auto-launch: locked (%d/%d experiments)"), Sim->GetTotalExperiments(), AlienIdleTuning::AutomationUnlockExperiments));
				}
				return Str(Sim->GetUfo(SelectedUfo).bAutoLaunch ? TEXT("Auto-launch: ON") : TEXT("Auto-launch: OFF"));
			}, [this] { return Sim->IsAutomationUnlocked() && Sim->GetUfo(SelectedUfo).bAutoLaunch; },
			[this] { Sim->SetAutoLaunch(SelectedUfo, !Sim->GetUfo(SelectedUfo).bAutoLaunch); })
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			MakeButton([this]
			{
				if (!Sim->IsAutomationUnlocked())
				{
					return Str(TEXT("Contract mode: locked"));
				}
				return Str(Sim->GetUfo(SelectedUfo).bFollowContract ? TEXT("Contract mode: ON") : TEXT("Contract mode: OFF"));
			}, [this] { return Sim->IsAutomationUnlocked() && Sim->GetUfo(SelectedUfo).bFollowContract; },
			[this] { Sim->SetFollowContract(SelectedUfo, !Sim->GetUfo(SelectedUfo).bFollowContract); })
		], FMargin(0, 0, 0, 4));

	Add(MakeButton([this]
	{
		return Str(Sim->IsRerouteOn()
			? TEXT("Reroute around Earth's responses: ON (auto-launching ships pick the calmest region with their target)")
			: TEXT("Reroute around Earth's responses: OFF"));
	}, [this] { return Sim->IsRerouteOn(); }, [this] { Sim->SetReroute(!Sim->IsRerouteOn()); }), FMargin(0, 0, 0, 2));
	Add(MakeButton([this]
	{
		return Str(Sim->IsPauseWhenFullOn()
			? TEXT("Pause collection when containment is full: ON (all auto-launching ships)")
			: TEXT("Pause collection when containment is full: OFF"));
	}, [this] { return Sim->IsPauseWhenFullOn(); }, [this] { Sim->SetPauseWhenFull(!Sim->IsPauseWhenFullOn()); }), FMargin(0, 0, 0, 4));

	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Slots %d/%d   Tractor %d   Cargo %d   Flight x%.2f   Suspicion x%.2f"),
			Sim->GetShipSlotsUsed(SelectedUfo), Sim->GetShipSlots(), Sim->GetTractorStrength(SelectedUfo), Sim->GetCargoCapacity(SelectedUfo),
			Sim->GetFlightSeconds(SelectedUfo, ESpecimen::Squirrel) / AlienIdleTuning::Specimen(ESpecimen::Squirrel).FlightSeconds, Sim->GetSuspicionMultiplier(SelectedUfo)));
	}, [] { return FSlateColor(MutedColor); }));

	Add(MakeLine([this]
	{
		if (Sim->GetShipLevel(SelectedUfo, EShipSystem::Scanner) == 0)
		{
			return Str(TEXT("Scanner: none fitted. Humans arrive with whatever job they had."));
		}
		const EOccupation Target = Sim->GetScannerTarget(SelectedUfo, ESpecimen::Human);
		return Str(Target == EOccupation::None ? FString(TEXT("Scanner: no contract or project needs a profession right now."))
			: FString::Printf(TEXT("Scanner targets: %s (for your contract or project)"), AlienIdleTuning::Occupation(Target).Name));
	}, [] { return FSlateColor(MutedColor); }));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->GetUfo(SelectedUfo).CaseTarget >= 0 ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeLine([this] { return Str(Sim->GetCaseTargetText(SelectedUfo)); }, [] { return FSlateColor(HeadingColor); })
		]);

	TSharedRef<SVerticalBox> ShipUpgrades = SNew(SVerticalBox);
	for (EShipSystem System : { EShipSystem::Tractor, EShipSystem::Cargo, EShipSystem::Stealth, EShipSystem::Scanner, EShipSystem::Thrusters })
	{
		AddShipUpgradeButton(ShipUpgrades, System);
	}
	Add(ShipUpgrades, FMargin(0, 2, 0, 2));
	Add(MakeButton([this]
	{
		FString Reason;
		if (!Sim->CanRefitShip(SelectedUfo, &Reason))
		{
			return Str(FString::Printf(TEXT("Refit ship: %s"), *Reason));
		}
		return Str(FString::Printf(TEXT("Refit ship: strip all upgrades, refund %.0f research"), Sim->GetRefitRefund(SelectedUfo)));
	}, [] { return false; }, [this] { Sim->RefitShip(SelectedUfo); }), FMargin(0, 0, 0, 10));

	// Alien Resources (crew)
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "AlienResources", "ALIEN RESOURCES")));
	Add(MakeLine([this]
	{
		int32 InSeminar = 0;
		for (const FCrewMember& Member : Sim->GetCrew())
		{
			InSeminar += Member.TrainingRemaining > 0.0f ? 1 : 0;
		}
		return Str(FString::Printf(TEXT("Crew %d: %d idle, %d in seminars. Anyone can staff a post; specialists add bonuses."),
			Sim->GetCrew().Num(), Sim->GetIdleCrew(), InSeminar));
	}, [] { return FSlateColor(MutedColor); }));
	Add(SNew(SHorizontalBox)
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
		], FMargin(0, 2));
	for (int32 i = 0; i < MaxStationsShown; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return i < Sim->GetAllStations().Num() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this, i]
				{
					const TArray<FAssignment> Stations = Sim->GetAllStations();
					return Stations.IsValidIndex(i) ? Str(Sim->GetStationStaffingText(Stations[i])) : FText::GetEmpty();
				}, [this, i]
				{
					const TArray<FAssignment> Stations = Sim->GetAllStations();
					return FSlateColor(Stations.IsValidIndex(i) && !Sim->IsStationStaffed(Stations[i]) ? WarnColor : TextColor);
				})
			]);
	}
	for (int32 i = 0; i < MaxCrewShown; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return i < Sim->GetCrew().Num() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f)
				[
					MakeStepper([this, i]
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
					}, [this, i](int32 Direction)
					{
						if (Sim->GetCrew().IsValidIndex(i))
						{
							Sim->CycleAssignment(i, Direction);
						}
					})
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
			], FMargin(0, 1));
	}
	Add(SNullWidget::NullWidget, FMargin(0, 0, 0, 8));

	// Base
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Base", "BASE")));
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Containment: %d / %d units, %d specimens waiting"), Sim->GetContainmentUnitsUsed(), Sim->GetContainmentCapacity(), Sim->GetContainment().Num()));
	}));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->HasHull() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeLine([this] { return Str(Sim->GetHullText()); }, [this]
			{
				return FSlateColor(Sim->GetHullPressure() >= AlienIdleTuning::HullCreakPressure ? WarnColor : TextColor);
			})
		], FMargin(0, 2));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->HasHeat() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeLine([this] { return Str(Sim->GetHeatText()); }, [this]
				{
					return FSlateColor(Sim->GetLabWarmth() < 0.8f || Sim->GetHangarWarmth() < 0.8f ? WarnColor : TextColor);
				})
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
			[
				MakeButton([this] { return Str(FString::Printf(TEXT("Heat priority: %s (click to change)"), FAlienIdleSim::GetHeatPriorityName(Sim->GetHeatPriority()))); },
					[] { return false; },
					[this] { Sim->SetHeatPriority(EHeatPriority((int32(Sim->GetHeatPriority()) + 1) % int32(EHeatPriority::Count))); })
			]
		], FMargin(0, 2));
	for (int32 i = 0; i < MaxChambersShown; ++i)
	{
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return i < Sim->GetChambers().Num() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeLine([this, i]
				{
					return i < Sim->GetChambers().Num() ? Str(FString::Printf(TEXT("Chamber %d: %s"), i + 1, *Sim->GetChamberStatusText(i))) : FText::GetEmpty();
				}, [this, i]
				{
					const bool bIdleWithQueue = i < Sim->GetChambers().Num() && !Sim->GetChambers()[i].bRunning && Sim->GetContainment().Num() > 0;
					return FSlateColor(bIdleWithQueue ? WarnColor : TextColor);
				})
			]);
		// Probe Calibration: optional. The window lapses on its own and research carries on regardless.
		Add(SNew(SBox)
			.Visibility_Lambda([this, i] { return Sim->GetCalibrationChamber() == i && !Sim->IsCalibrationOpen() ? EVisibility::Visible : EVisibility::Collapsed; })
			[
				MakeButton([this]
				{
					return Str(FString::Printf(TEXT("CALIBRATION AVAILABLE (%.0f s): open the probe for a five-second calibration"), Sim->GetCalibrationOfferRemaining()));
				}, [] { return true; }, [this] { OpenCalibration(); })
			], FMargin(12, 1, 0, 2));
	}
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Lab speed x%.2f"), Sim->GetLabSpeed()));
	}, [] { return FSlateColor(MutedColor); }));
	Add(MakeLine([this] { return Str(Sim->GetCalibrationStatusText()); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 2, 0, 0));

	Add(MakeHeading(NSLOCTEXT("AlienIdle", "LabPriorities", "LAB PRIORITIES")), FMargin(0, 8, 0, 0));
	Add(MakeLine([] { return Str(TEXT("Routine and extended studies pay research. Discovery projects pay nothing and unlock targets and equipment.")); },
		[] { return FSlateColor(MutedColor); }));
	for (int32 i = 0; i < int32(ESpecimen::Count); ++i)
	{
		const ESpecimen Type = ESpecimen(i);
		Add(SNew(SBox)
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
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->IsMiddleManagementAvailable() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([this]
			{
				return Str(Sim->IsMiddleManagementOn()
					? TEXT("Middle Management: ON (labs +20%, mandatory meetings every 2 min)")
					: TEXT("Middle Management: OFF (promote an alien: labs +20%, but meetings)"));
			}, [this] { return Sim->IsMiddleManagementOn(); }, [this] { Sim->SetMiddleManagement(!Sim->IsMiddleManagementOn()); })
		], FMargin(0, 2, 0, 8));

	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->IsPreserving() ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([this]
			{
				return Str(Sim->IsAgingSamples()
					? TEXT("Freezer mode: ON (routine studies wait until a sample has been preserved a minute: up to +50% research, slower turnover)")
					: TEXT("Freezer mode: OFF (study specimens as soon as they arrive)"));
			}, [this] { return Sim->IsAgingSamples(); }, [this] { Sim->SetAgeSamples(!Sim->IsAgingSamples()); })
		], FMargin(0, 2, 0, 8));

	TSharedRef<SVerticalBox> BaseUpgrades = SNew(SVerticalBox);
	for (int32 i = 0; i < int32(EBaseUpgrade::Count); ++i)
	{
		AddBaseUpgradeButton(BaseUpgrades, EBaseUpgrade(i));
	}
	Add(BaseUpgrades, FMargin(0, 4, 0, 10));

	// Incident
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

	// Rivals
	Add(MakeHeading(NSLOCTEXT("AlienIdle", "Rivals", "RIVALS ON THIS ASSIGNMENT")), FMargin(0, 10, 0, 1));
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
	Add(MakeLine([this] { return Str(Sim->GetJointText()); }, [] { return FSlateColor(TextColor); }), FMargin(0, 4, 0, 1));
	Add(SNew(SBox)
		.Visibility_Lambda([this] { return Sim->GetJointRival() != ERival::Count ? EVisibility::Visible : EVisibility::Collapsed; })
		[
			MakeButton([this]
			{
				return Str(Sim->GetRival().bContribute ? TEXT("[on]  Contributing specimens to the joint experiment")
					: TEXT("[off] Contribute specimens to the joint experiment"));
			}, [this] { return Sim->GetRival().bContribute; }, [this] { Sim->SetContribute(!Sim->GetRival().bContribute); })
		], FMargin(0, 1));
	Add(MakeLine([this] { return Str(Sim->GetReputationText()); }, [] { return FSlateColor(MutedColor); }), FMargin(0, 2));
	Add(MakeLine([this] { return Str(Sim->GetRaceText()); }, [this] { return FSlateColor(Sim->GetRival().bRaceActive ? HeadingColor : TextColor); }), FMargin(0, 2));
	Add(SNew(SHorizontalBox)
		.Visibility_Lambda([this] { return Sim->GetRival().bRaceOffered ? EVisibility::Visible : EVisibility::Collapsed; })
		+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
		[
			MakeButton([] { return Str(TEXT("Accept the race")); }, [] { return true; }, [this] { Sim->AcceptRace(); })
		]
		+ SHorizontalBox::Slot().FillWidth(1.0f)
		[
			MakeButton([] { return Str(TEXT("Decline")); }, [] { return false; }, [this] { Sim->DeclineRace(); })
		], FMargin(0, 2));
	Add(MakeLine([this]
	{
		return Str(FString::Printf(TEXT("Standing trade orders (never expire). Trades this run: %d, income %.0f research. Lifetime trades: %d."),
			Sim->GetRival().TradesThisRun, Sim->GetRival().TradeIncome, Sim->GetCareer().Trades));
	}, [] { return FSlateColor(MutedColor); }), FMargin(0, 4, 0, 1));
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
		.Text(NSLOCTEXT("AlienIdle", "Controls", "Mouse, keyboard or gamepad. Arrows / D-pad to move, Enter / A to press. Mouse wheel zooms the map, drag to pan, double-click to reset.")), FMargin(0, 6, 0, 0));

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
				SNew(SAlienBaseView).Sim(Sim).OnCalibrationClicked([this] { OpenCalibration(); })
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
						SNew(SScrollBox)
						.ScrollWhenFocusChanges(EScrollWhenFocusChanges::AnimatedScroll)
						+ SScrollBox::Slot().Padding(0, 0, 10, 0)
						[
							Panel
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
	Box->AddSlot().AutoHeight().Padding(0, 2)
	[
		MakeButton([this, System]
		{
			const FUpgradeDef& Def = AlienIdleTuning::ShipSystems[int32(System)];
			const int32 Level = Sim->GetShipLevel(SelectedUfo, System);
			FString Reason;
			const bool bCan = Sim->CanBuyShipUpgrade(SelectedUfo, System, &Reason);
			FString Title = Level >= Def.MaxLevel
				? FString::Printf(TEXT("%s %s (max)"), Def.Name, Roman(Level))
				: FString::Printf(TEXT("%s %s -> %s   %.0f research%s"), Def.Name, Roman(Level), Roman(Level + 1), Sim->GetShipUpgradeCost(SelectedUfo, System),
					Def.SlotsPerLevel > 1 ? *FString::Printf(TEXT(", %d slots"), Def.SlotsPerLevel) : TEXT(""));
			FString Text = Title + TEXT("\n") + FAlienIdleSim::GetShipUpgradeEffectText(System);
			if (!bCan && Level < Def.MaxLevel)
			{
				Text += TEXT("\nUnavailable: ") + Reason;
			}
			return Str(Text);
		}, [this, System] { return Sim->CanBuyShipUpgrade(SelectedUfo, System); },
		[this, System] { Sim->BuyShipUpgrade(SelectedUfo, System); })
	];
}

void SAlienIdleRoot::AddBaseUpgradeButton(const TSharedRef<SVerticalBox>& Box, EBaseUpgrade Upgrade)
{
	Box->AddSlot().AutoHeight().Padding(0, 2)
	[
		SNew(SBox)
		.Visibility_Lambda([this, Upgrade] { return Sim->IsBaseUpgradeAvailable(Upgrade) ? EVisibility::Visible : EVisibility::Collapsed; })
		[
		MakeButton([this, Upgrade]
		{
			const FUpgradeDef& Def = AlienIdleTuning::BaseUpgrades[int32(Upgrade)];
			const int32 Level = Sim->GetBaseLevel(Upgrade);
			FString Reason;
			const bool bCan = Sim->CanBuyBaseUpgrade(Upgrade, &Reason);
			FString Title = Level >= Def.MaxLevel
				? FString::Printf(TEXT("%s %s (max)"), Def.Name, Roman(Level))
				: FString::Printf(TEXT("%s %s -> %s   %.0f research"), Def.Name, Roman(Level), Roman(Level + 1), Sim->GetBaseUpgradeCost(Upgrade));
			FString Text = Title + TEXT("\n") + FAlienIdleSim::GetBaseUpgradeEffectText(Upgrade);
			if (!bCan && Level < Def.MaxLevel)
			{
				Text += TEXT("\nUnavailable: ") + Reason;
			}
			return Str(Text);
		}, [this, Upgrade] { return Sim->CanBuyBaseUpgrade(Upgrade); },
		[this, Upgrade] { Sim->BuyBaseUpgrade(Upgrade); })
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
		return Str(FString::Printf(TEXT("Launch '%s'"), *Sim->GetUfo(SelectedUfo).Name));
	}
	return Str(FString::Printf(TEXT("Launch unavailable: %s"), *Reason));
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
					SelectedUfo = 0;
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
