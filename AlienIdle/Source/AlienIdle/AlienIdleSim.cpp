#include "AlienIdleSim.h"

using namespace AlienIdleTuning;

namespace
{
	// Experiment reports cycle in order per specimen so the log stays readable.
	const TCHAR* SquirrelReports[] = {
		TEXT("Squirrel exposed to elevator music for 40 minutes. Squirrel was unbothered. Staff were not."),
		TEXT("Attempted to locate the squirrel's sense of personal space. Results Were Technically Obtained."),
		TEXT("Specimen buried the lab technician's pen. The pen is now considered a field sample."),
		TEXT("Measured tail-to-attitude ratio. Ratio exceeds instrument range."),
	};
	const TCHAR* PufferfishReports[] = {
		TEXT("Asked the pufferfish about its weekend. It inflated. Data excellent."),
		TEXT("Pufferfish shown a spreadsheet. Doubled in size. Finance took notes."),
		TEXT("Measured the pufferfish deflated and inflated. Both readings disappointing."),
	};
	const TCHAR* OctopusReports[] = {
		TEXT("Octopus solved the maze, then the clipboard, then the scientist's password."),
		TEXT("Octopus changed colour to match the lab coat. The lab coat changed colour back, out of solidarity."),
		TEXT("Octopus filled out all eight copies of the consent form at once. Signed 'Legal'."),
	};
	const TCHAR* WhaleReports[] = {
		TEXT("Whale studied for four hours. It noticed us in the final minute and seemed hurt."),
		TEXT("Recorded forty minutes of whale song. Translation: a performance review. Ours."),
		TEXT("Whale weighed. The scale has filed a grievance."),
	};
	const TCHAR* PenguinReports[] = {
		TEXT("Penguin attended the briefing in formal wear and left without comment. Data excellent."),
		TEXT("Asked the penguin to rank its colleagues. It ranked itself first, twice."),
		TEXT("Penguin thawed and studied. It looked at us like we were the strange ones."),
	};
	const TCHAR* SealReports[] = {
		TEXT("Seal clapped at the results. Nobody knows which ones."),
		TEXT("Seal measured for insulation. Facilities took notes, then took the seal's measurements for the staff."),
		TEXT("Seal barked through the whole interview. Transcript filed under 'feedback'."),
	};
	const TCHAR* ElephantSealReports[] = {
		TEXT("Elephant seal weighed. Scale lost. Elephant seal roared about it anyway."),
		TEXT("Elephant seal asserted dominance over the lab. The lab has accepted the new management."),
		TEXT("Studied the elephant seal's nose. The elephant seal studied ours. Inconclusive."),
	};
	const TCHAR* CowReports[] = {
		TEXT("Asked a cow to rate our hospitality. It mooed. Translation pending; tone hostile."),
		TEXT("Measured how long a cow can stare at a wall. Study ended when the wall looked away."),
		TEXT("Located the cow's sarcasm organ. It was the whole cow."),
		TEXT("Cow attended the quarterly all-hands. Asked the only relevant question."),
	};
	const TCHAR* ElephantReports[] = {
		TEXT("Tested whether an elephant fits in a standard filing cabinet. It does not. Cabinet reclassified as debris."),
		TEXT("Elephant attended mandatory team-building. Outperformed two managers."),
		TEXT("Elephant was shown our org chart. It sighed. We are told this is a good sign."),
	};
	const TCHAR* GenericHumanReports[] = {
		TEXT("Probed why 'five more minutes' lasts forty minutes. Study is now in its third day."),
		TEXT("Asked the subject what they do for fun. Subject described 'brunch.' Further funding requested."),
	};
	const TCHAR* ReturnNotes[] = {
		TEXT("Specimen memory-wiped and returned. Original address remains theoretical."),
		TEXT("Specimen memory-wiped and returned. Satisfaction survey: 4 out of 5 stars, would not be abducted again."),
		TEXT("Specimen memory-wiped and returned to the nearest location that looked about right."),
	};
	const TCHAR* MeetingLines[] = {
		TEXT("Mandatory meeting. Agenda item 1: why the lab is not running. Labs paused."),
		TEXT("Mandatory meeting about reducing meetings. Labs paused."),
		TEXT("Mandatory meeting. Management presented a slide titled 'Synergy?' Labs paused."),
	};

	const TCHAR* OccupationReport(EOccupation Occupation)
	{
		switch (Occupation)
		{
		case EOccupation::Accountant: return TEXT("Tested an accountant's resistance to elevator music. The accountant was *extremely* resistant.");
		case EOccupation::Astronomer: return TEXT("Showed the astronomer our star charts. Astronomer corrected three of them. Charts have been confiscated.");
		case EOccupation::Magician:   return TEXT("Determined whether a magician can escape alien containment. Yes, twice. The third escape was for an audience.");
		case EOccupation::Lawyer:     return TEXT("Experiment delayed pending clarification of the consent form. The lawyer has clarified it into four volumes.");
		case EOccupation::Therapist:  return TEXT("The therapist asked how the experiment made us feel. Staff morale is becoming measurable.");
		case EOccupation::MallSanta:  return TEXT("Mall Santa asked what staff wanted for Christmas. Staff requested 'a lighter workload.' Santa said no.");
		case EOccupation::Politician: return TEXT("Asked the politician a yes-or-no question. The answer is now in its fourth hour.");
		default: return nullptr;
		}
	}

	const TCHAR* AcquireFlavor(EOccupation Occupation)
	{
		switch (Occupation)
		{
		case EOccupation::Accountant: return TEXT("The budget has achieved sentience and must be audited.");
		case EOccupation::Astronomer: return TEXT("Someone keeps pointing at the sky and saying 'that's us.'");
		case EOccupation::Magician:   return TEXT("The Academy wants to know where the rabbit goes.");
		case EOccupation::Lawyer:     return TEXT("Legal would like a second opinion on the first opinion.");
		case EOccupation::Therapist:  return TEXT("Staff morale is becoming measurable.");
		case EOccupation::MallSanta:  return TEXT("Seasonal research. The Academy declined to specify which season.");
		case EOccupation::Politician: return TEXT("The Academy needs someone to blame. Ideally someone experienced.");
		default: return TEXT("");
		}
	}

	// Department flavour for routine studies: half of the reports come from the primary department.
	const TCHAR* BiologyReports[] = {
		TEXT("Cross-referenced the subject with a squirrel. The squirrel objected."),
		TEXT("Habitat review: the subject rated its cell 'roomy, emotionally.'"),
		TEXT("Measured the subject against the department's growth chart. The chart has been revised downward."),
	};
	const TCHAR* HumanReports[] = {
		TEXT("Interviewed the subject about career goals. The subject wept. Data excellent."),
		TEXT("Observed the subject reply-all to the entire station. The ethics committee has been cc'd."),
		TEXT("Asked the subject to rate their job satisfaction. Instruments recorded a number below zero."),
	};
	const TCHAR* IncompetenceReports[] = {
		TEXT("Experiment conducted with the safety off. The safety was never on."),
		TEXT("Equipment caught fire, then caught on. Results inconclusive, smoke conclusive."),
		TEXT("Calibrated the instruments with a hammer. Readings improved, morally."),
	};

	const TCHAR* UfoNames[] = { TEXT("Old Reliable"), TEXT("Second Opinion"), TEXT("Plausible Deniability") };

	double UpgradeCost(const FUpgradeDef& Def, int32 Level)
	{
		return FMath::RoundToDouble(Def.BaseCost * FMath::Pow(Def.CostGrowth, double(Level)));
	}

	FString Capitalized(const FString& Text)
	{
		FString Result(Text);
		if (!Result.IsEmpty())
		{
			Result[0] = FChar::ToUpper(Result[0]);
		}
		return Result;
	}

	FString WithArticle(const FString& Noun)
	{
		const bool bVowel = Noun.Len() > 0 && FString(TEXT("aeiouAEIOU")).Contains(Noun.Left(1));
		return (bVowel ? TEXT("an ") : TEXT("a ")) + Noun;
	}

	bool SameChoice(const FExperimentChoice& A, const FExperimentChoice& B)
	{
		return A.Kind == B.Kind && (A.Kind != EExperimentKind::Discovery || A.Discovery == B.Discovery);
	}
}

// ---------------------------------------------------------------------------------------------
// Contracts

bool FContractNeed::Accepts(const FSpecimen& Subject) const
{
	if (IsMet() || Subject.Type != Type)
	{
		return false;
	}
	if (Occupation != EOccupation::None && Subject.Occupation != Occupation)
	{
		return false;
	}
	if (bDistinctOccupations && (Subject.Occupation == EOccupation::None || Seen.Contains(Subject.Occupation)))
	{
		return false;
	}
	return true;
}

bool FContract::IsComplete() const
{
	for (const FContractNeed& Need : Needs)
	{
		if (!Need.IsMet())
		{
			return false;
		}
	}
	return Needs.Num() > 0;
}

FString FContract::GetProgressText() const
{
	TArray<FString> Parts;
	for (const FContractNeed& Need : Needs)
	{
		const FSpecimenDef& Def = Specimen(Need.Type);
		const FString What = Need.Occupation != EOccupation::None ? FString(AlienIdleTuning::Occupation(Need.Occupation).Name).ToLower()
			: Need.bDistinctOccupations ? FString(TEXT("different professions"))
			: FString(Need.Count == 1 ? Def.Name : Def.Plural);
		Parts.Add(FString::Printf(TEXT("%s %d/%d"), *What, Need.Progress, Need.Count));
	}
	return FString::Join(Parts, TEXT(", "));
}

// ---------------------------------------------------------------------------------------------

FAlienIdleSim::FAlienIdleSim()
{
	Rng.Initialize(RandomSeed);
	CaseRng.Initialize(CaseRandomSeed);
	Energy = StartEnergy;
	AddUfo();
	Chambers.AddDefaulted(1);
	AddCrew(ESpecialty::Pilot, { EStation::Ufo, 0 });
	AddCrew(ESpecialty::Scientist, { EStation::Chamber, 0 });
	AddCrew(ESpecialty::Technician, { EStation::Power, 0 });
	AnnouncedUnlock[int32(ESpecimen::Squirrel)] = true;
	FillContractOffers();
	AddLog(TEXT("Memo from your deputy: Specimen #0001 (squirrel) has completed an unscheduled departure. Please collect a replacement from Pine Ridge."));
	AddLog(TEXT("Welcome to Mountain Station 7. Routine studies fund the station; discovery projects get us promoted. Please do both."));
	AddLog(TEXT("Alien Resources: you have a pilot, a scientist and a technician. Containment and the memory-wipe station are unstaffed. Management considers this sustainable."));
}

void FAlienIdleSim::AddUfo()
{
	FUfo& Ufo = Ufos.AddDefaulted_GetRef();
	Ufo.Name = UfoNames[FMath::Min(Ufos.Num() - 1, int32(UE_ARRAY_COUNT(UfoNames)) - 1)];
}

// ---------------------------------------------------------------------------------------------
// Simulation

void FAlienIdleSim::Tick(float DeltaSeconds)
{
	float Remaining = FMath::Max(0.0f, DeltaSeconds);
	while (Remaining > 0.0f)
	{
		const float Dt = FMath::Min(Remaining, MaxStepSeconds);
		Step(Dt);
		Remaining -= Dt;
	}
}

void FAlienIdleSim::Step(float Dt)
{
	Elapsed += Dt;
	Energy = FMath::Min(GetEnergyCap(), Energy + GetEnergyPerSecond() * Dt);
	StepRegions(Dt);
	StepHull(Dt);
	StepOverheat();
	if (IsPreserving())
	{
		const float Rate = GetBaseLevel(EBaseUpgrade::CryoVault) > 0 ? 2.0f : 1.0f;
		for (FSpecimen& Held : Containment)
		{
			Held.Held += Dt * Rate;
		}
	}
	StepAchievements();
	StepCrew(Dt);
	StepRival(Dt);
	StepBehaviourMemo();
	StepAutoContract();
	StepIncidents(Dt);
	StepCaseDynamics(Dt);

	if (bMiddleManagement)
	{
		if (MeetingRemaining > 0.0f)
		{
			MeetingRemaining = FMath::Max(0.0f, MeetingRemaining - Dt);
		}
		else if ((MeetingTimer += Dt) >= MeetingIntervalSeconds)
		{
			MeetingTimer = 0.0f;
			MeetingRemaining = MeetingSeconds;
			AddLog(MeetingLines[int32(Elapsed / MeetingIntervalSeconds) % UE_ARRAY_COUNT(MeetingLines)]);
		}
	}

	for (int32 i = 0; i < Ufos.Num(); ++i)
	{
		StepUfo(i, Dt);
	}
	StepLab(Dt);
	StepBottlenecks(Dt);

	for (int32 i = 0; i < int32(ESpecimen::Count); ++i)
	{
		if (!AnnouncedUnlock[i] && IsSpecimenUnlocked(ESpecimen(i)))
		{
			AnnouncedUnlock[i] = true;
			// Refresh the oldest offer so the new target shows up on the contract board.
			if (Offers.Num() > 0)
			{
				Offers.RemoveAt(0);
				FillContractOffers();
			}
			const FSpecimenDef& Def = Specimens[i];
			AddLog(FString::Printf(TEXT("%s approved for study (first sought in %s). Check your tractor beam: %s weigh %d."),
				*FString(Def.Plural).Left(1).ToUpper().Append(FString(Def.Plural).Mid(1)), *GetRegionName(ESpecimen(i)), Def.Plural, Def.Weight));
		}
	}
	for (int32 i = 0; i < int32(EDiscovery::Count); ++i)
	{
		if (!AnnouncedProject[i] && IsDiscoveryOpen(EDiscovery(i)))
		{
			AnnouncedProject[i] = true;
			const FDiscoveryDef& Def = Discoveries[i];
			AddLog(FString::Printf(TEXT("Discovery project open: '%s' (%s) Pick it in Lab priorities. %s"), Def.Name, Def.Question, Def.Unlocks));
		}
	}
	if (!bAnnouncedAutomation && IsAutomationUnlocked())
	{
		bAnnouncedAutomation = true;
		AddLog(TEXT("Automation unlocked: auto-launch and contract mode. Pilots may now repeat their last mission without being asked, which they describe as 'a career.'"));
	}
}

void FAlienIdleSim::StepRegions(float Dt)
{
	for (int32 i = 0; i < NumRegions; ++i)
	{
		FRegionState& Region = Regions[i];
		double Decay = SuspicionDecayPerSecond * (IsProjectDone(EDeptProject::AutomatedCoverStories) ? 2.0 : 1.0);
		if (Region.Active[int32(EResponse::SupportGroup)])
		{
			Decay *= 0.5;
		}
		Region.Suspicion = FMath::Max(0.0, Region.Suspicion - Decay * Dt);
		if (Region.bLockedDown && Region.Suspicion < GetLockdownRelease())
		{
			Region.bLockedDown = false;
			AddLog(FString::Printf(TEXT("%s has calmed down. Residents have moved on to complaining about parking."), *GetRegionNameAt(i)));
		}
	}
	StepResponses(Dt);
}

void FAlienIdleSim::AddSuspicion(int32 RegionIndex, double Amount, ESpecimen Kind)
{
	FRegionState& Region = Regions[RegionIndex];
	const double Alert = GetAlertSuspicion(RegionIndex);
	const bool bWasAlert = Region.Suspicion >= Alert;
	// Exposure, thesis rules, Earth conditions, city surveillance and investigators make disappearances louder.
	if (Amount > 0.0)
	{
		Amount *= SuspicionScale(RegionIndex, Kind);
	}
	Region.Suspicion = FMath::Clamp(Region.Suspicion + Amount, 0.0, LockdownSuspicion);
	const FString RegionName = GetRegionNameAt(RegionIndex);
	const TCHAR* Name = *RegionName;
	if (!Region.bLockedDown && Region.Suspicion >= LockdownSuspicion)
	{
		Region.bLockedDown = true;
		AddLog(FString::Printf(TEXT("%s is in lockdown. Someone started a podcast. Missions there are suspended until it blows over."), Name));
	}
	else if (!bWasAlert && Region.Suspicion >= Alert)
	{
		static const TCHAR* Excuses[] = {
			TEXT("Local paper blames 'a particularly ambitious owl.'"),
			TEXT("Repeated cow disappearances are being blamed on 'agricultural migration.'"),
			TEXT("Officials blame 'a weather balloon with ambitions.'"),
			TEXT("Residents blame 'the new neighbours,' which is fair."),
			TEXT("A local scientist blames rising temperatures on cow farts. The cows have declined to comment."),
		};
		AddLog(FString::Printf(TEXT("%s is on alert. %s Captures there take longer."), Name, Excuses[(RegionIndex + int32(Kind)) % UE_ARRAY_COUNT(Excuses)]));
	}
}

// ---------------------------------------------------------------------------------------------
// Incidents

void FAlienIdleSim::StepIncidents(float Dt)
{
	if (Incident.Type != EIncident::None)
	{
		Incident.Remaining -= Dt;
		if (Incident.Remaining <= 0.0f)
		{
			switch (Incident.Type)
			{
			case EIncident::Brownout: AddLog(TEXT("Brownout over. Facilities has been thanked for doing nothing.")); break;
			case EIncident::TractorJam: AddLog(TEXT("Tractor beam unjammed. The squirrel responsible remains at large.")); break;
			case EIncident::CoffeeFailure: AddLog(TEXT("Coffee machine repaired. Productivity restored to its usual disappointing level.")); break;
			case EIncident::InventoryReorganized: AddLog(TEXT("Someone found the old labels. Containment is back to its original, worse layout.")); break;
			case EIncident::CeaseAndDesist: AddLog(TEXT("Legal replied to the cease-and-desist with a cease-and-desist. Launch costs back to normal.")); break;
			case EIncident::StaffMeeting: AddLog(TEXT("The meeting has ended. Action items: schedule a follow-up meeting. Labs resume.")); break;
			case EIncident::AdoptedSquirrel: AddLog(TEXT("The adopted squirrel has left for a better life in the ventilation system. Its owner is back on duty, quietly.")); break;
			case EIncident::WhaleSong: AddLog(TEXT("The whale has finished its song and left. Staff are asked not to hum it.")); break;
			case EIncident::HeatingCut: AddLog(TEXT("The heating budget has been restored after staff began filing their complaints in frost on the windows.")); break;
			case EIncident::ContainmentNewsletter: AddLog(TEXT("The containment newsletter has folded after its only advertiser (the cafeteria) withdrew. Labs back to normal.")); break;
			case EIncident::AcademicDispute: AddLog(TEXT("The academic dispute has been resolved: both parties agreed the other was wrong. Labs back to normal.")); break;
			default: break;
			}
			Incident = FActiveIncident();
			IncidentCooldown = IncidentCooldownSeconds;
		}
		return;
	}

	IncidentCooldown = FMath::Max(0.0f, IncidentCooldown - Dt);
	if (!bIncidentsEnabled || IncidentCooldown > 0.0f)
	{
		return;
	}

	const FAssignment Warden{ EStation::Containment, 0 };
	double EscapeScale = IsDiscovered(EDiscovery::UnscheduledDepartures) ? 0.5 : 1.0;
	EscapeScale *= IsStationStaffed(Warden) ? 1.0 / (1.0 + GetStationBonus(Warden)) : UnstaffedEscapeMultiplier;
	for (int32 i = 0; i < Containment.Num(); ++i)
	{
		const FSpecimen& Held = Containment[i];
		const double Habitat = ((Held.Type != ESpecimen::Human && IsProjectDone(EDeptProject::ReinforcedHabitats)) ? 0.5 : 1.0)
			* (Location == ELocation::Antarctic ? 0.5 : 1.0) * (GetBaseLevel(EBaseUpgrade::CryoVault) > 0 ? 0.5 : 1.0);
		// Recurring subjects are harder to fool every time: they know where the doors are.
		const double Repeat = Held.Case >= 0 ? 1.0 + CaseEscapePerEncounter * FMath::Min(Career.Cases[Held.Case].Encounters - 1, CaseMaxEncounterEffect) : 1.0;
		const double Chance = Specimen(Held.Type).EscapePerSecond * Occupation(Held.Occupation).EscapeMultiplier * EscapeScale * Habitat * Repeat * Dt;
		if (Rng.FRand() < Chance)
		{
			EscapeSpecimen(i);
			return;
		}
	}

	if (Rng.FRand() < EquipmentFailurePerSecond * (IsProjectDone(EDeptProject::UnusualPower) ? 1.5 : 1.0) * Dt)
	{
		if (HasHeat() && Rng.FRand() < 0.35f)
		{
			StartIncident(EIncident::HeatingCut, 45.0f);
			AddLog(TEXT("The heating budget has been reduced. Staff are encouraged to develop thicker skin. Heat output halved."));
			return;
		}
		if (HasHull() && Rng.FRand() < 0.35f)
		{
			StartIncident(EIncident::WhaleSong, 40.0f);
			HullPressure = FMath::Min(100.0f, HullPressure + WhaleIncidentPressure);
			AddLog(TEXT("A whale has taken an interest in the hull and is singing to it. Hull pressure +20, labs slower. The heating budget has nothing to do with this, for once."));
			return;
		}
		switch (Rng.RandRange(0, 4))
		{
		case 3:
		{
			int32 Caller = INDEX_NONE;
			for (int32 i = 0; i < Crew.Num(); ++i)
			{
				if (IsOnDuty(i) && Crew[i].Assignment.Station != EStation::None)
				{
					Caller = i;
					break;
				}
			}
			StartIncident(EIncident::StaffMeeting, StaffMeetingSeconds);
			Incident.Crew = Caller;
			AddLog(FString::Printf(TEXT("%s has initiated a mandatory meeting to discuss the agenda for the next meeting. Labs paused."),
				Caller != INDEX_NONE ? *Crew[Caller].Name : TEXT("Someone")));
			break;
		}
		case 4:
		{
			TArray<int32> Candidates;
			for (int32 i = 0; i < Crew.Num(); ++i)
			{
				if (IsOnDuty(i) && Crew[i].Assignment.Station != EStation::None)
				{
					Candidates.Add(i);
				}
			}
			if (Candidates.Num() == 0)
			{
				break;
			}
			const int32 Adopter = Candidates[Rng.RandRange(0, Candidates.Num() - 1)];
			StartIncident(EIncident::AdoptedSquirrel, AdoptedSquirrelSeconds);
			Incident.Crew = Adopter;
			const int32 Squirrel = Containment.IndexOfByPredicate([](const FSpecimen& S) { return S.Type == ESpecimen::Squirrel; });
			if (Squirrel != INDEX_NONE)
			{
				Containment.RemoveAt(Squirrel);
			}
			AddLog(FString::Printf(TEXT("%s has adopted a squirrel and named it 'Specimen'. %s is off duty, building it a tiny desk. Their post (%s) is short-staffed."),
				*Crew[Adopter].Name, *Crew[Adopter].Name, *GetStationName(Crew[Adopter].Assignment)));
			break;
		}
		case 0:
			StartIncident(EIncident::Brownout, 30.0f);
			AddLog(HasHeat() ? TEXT("Generator brownout. The generator froze. Someone suggested a blanket; the blanket also froze. Energy regeneration reduced.")
				: HasHull() ? TEXT("Generator brownout. A crab is in the intake again. It has been offered a job. Energy regeneration reduced.")
				: TEXT("Generator brownout. Facilities says it is 'resting its eyes.' Energy regeneration reduced."));
			break;
		case 1:
		{
			const int32 Ufo = Rng.RandRange(0, Ufos.Num() - 1);
			StartIncident(EIncident::TractorJam, 25.0f, Ufo);
			AddLog(FString::Printf(TEXT("'%s' has a jammed tractor beam. A squirrel is suspected. Grounded until it clears."), *Ufos[Ufo].Name));
			break;
		}
		default:
			StartIncident(EIncident::CoffeeFailure, 40.0f);
			AddLog(HasHull() ? TEXT("The coffee machine is making seawater again. Staff insist they cannot taste the difference. Labs slower.")
				: TEXT("The coffee machine has failed. Lab productivity is now measured in sighs."));
			break;
		}
	}
}

void FAlienIdleSim::StartIncident(EIncident Type, float Seconds, int32 Ufo)
{
	++IncidentCount;
	if (HasSignatureBonus(EDepartment::Incompetence))
	{
		Research += IncidentLessonResearch;
		ResearchEarned += IncidentLessonResearch;
		AddLog(FString::Printf(TEXT("Applied Incompetence logs the incident as a learning opportunity. Research +%.0f."), IncidentLessonResearch));
	}
	Incident.Type = Type;
	Incident.Remaining = Seconds;
	Incident.Ufo = Ufo;
}

void FAlienIdleSim::EscapeSpecimen(int32 ContainmentIndex)
{
	const FSpecimen Escapee = Containment[ContainmentIndex];
	Containment.RemoveAt(ContainmentIndex);
	++Escapes;
	NoteCaseEscape(Escapee);
	if (Escapee.Occupation != EOccupation::Accountant && Escapee.Occupation != EOccupation::Lawyer)
	{
		++IncidentCount; // the others start a timed incident, which counts itself
	}
	AddSuspicion(Escapee.Region, EscapeSuspicion, Escapee.Type);
	AddPressure(Escapee.Region, EResponse::SupportGroup, SupportPressurePerEscape);
	IncidentCooldown = IncidentCooldownSeconds;

	switch (Escapee.Occupation)
	{
	case EOccupation::Accountant:
		StartIncident(EIncident::InventoryReorganized, 120.0f);
		AddLog(TEXT("An escaped accountant has reorganised the inventory on the way out. Containment holds 3 more units until someone finds the old labels."));
		return;
	case EOccupation::Lawyer:
		StartIncident(EIncident::CeaseAndDesist, 60.0f);
		AddLog(TEXT("An escaped lawyer served the station a cease-and-desist. Launches cost 50% more energy while Legal drafts a reply."));
		return;
	case EOccupation::Magician:
		AddLog(TEXT("The magician has escaped. Also missing: the stapler, two watches and the concept of trust."));
		return;
	default:
		break;
	}
	switch (Escapee.Type)
	{
	case ESpecimen::Squirrel: AddLog(TEXT("Unscheduled specimen departure: one squirrel, via the ventilation system. It took a pen.")); break;
	case ESpecimen::Cow: AddLog(TEXT("Unscheduled specimen departure: one cow. Nobody knows how. Nobody wants to discuss it.")); break;
	case ESpecimen::Elephant: AddLog(TEXT("Unscheduled specimen departure: one elephant. In fairness, it did not fit.")); break;
	case ESpecimen::Pufferfish: AddLog(TEXT("Unscheduled specimen departure: one pufferfish, via the drain. It inflated on the way out as a statement.")); break;
	case ESpecimen::Octopus: AddLog(TEXT("Unscheduled specimen departure: one octopus. It unscrewed the tank, the vent and, somehow, the morale.")); break;
	case ESpecimen::Whale: AddLog(TEXT("Unscheduled specimen departure: one whale. Nobody can explain how. Everyone can explain the flooding.")); break;
	case ESpecimen::Penguin: AddLog(TEXT("Unscheduled specimen departure: one penguin, by sliding. It did not look back. They never do.")); break;
	case ESpecimen::Seal: AddLog(TEXT("Unscheduled specimen departure: one seal. It clapped once on the way out. It was not applause.")); break;
	case ESpecimen::ElephantSeal: AddLog(TEXT("Unscheduled specimen departure: one elephant seal, through the wall. The wall has been reclassified as a door.")); break;
	default: AddLog(TEXT("Unscheduled specimen departure: one human, who told the guard they were 'just going to the car.'")); break;
	}
}

FString FAlienIdleSim::GetIncidentText() const
{
	const float Left = FMath::Max(0.0f, Incident.Remaining);
	switch (Incident.Type)
	{
	case EIncident::Brownout: return FString::Printf(TEXT("Generator brownout: energy regeneration x0.25 (%.0f s)"), Left);
	case EIncident::TractorJam: return FString::Printf(TEXT("'%s' tractor beam jammed: grounded (%.0f s)"), *Ufos[Incident.Ufo].Name, Left);
	case EIncident::CoffeeFailure: return FString::Printf(TEXT("Coffee machine failure: labs x0.7 speed (%.0f s)"), Left);
	case EIncident::InventoryReorganized: return FString::Printf(TEXT("Inventory reorganised by an escaped accountant: containment +3 units (%.0f s)"), Left);
	case EIncident::CeaseAndDesist: return FString::Printf(TEXT("Cease-and-desist: launches cost +50%% energy (%.0f s)"), Left);
	case EIncident::StaffMeeting: return FString::Printf(TEXT("%s called a mandatory meeting: labs paused (%.0f s)"),
		Crew.IsValidIndex(Incident.Crew) ? *Crew[Incident.Crew].Name : TEXT("Someone"), Left);
	case EIncident::AdoptedSquirrel: return FString::Printf(TEXT("%s adopted a squirrel and is off duty (%.0f s)"),
		Crew.IsValidIndex(Incident.Crew) ? *Crew[Incident.Crew].Name : TEXT("Someone"), Left);
	case EIncident::WhaleSong: return FString::Printf(TEXT("A whale is singing to the hull: labs x0.85 speed (%.0f s)"), Left);
	case EIncident::HeatingCut: return FString::Printf(TEXT("Heating budget reduced: heat x0.5 (%.0f s)"), Left);
	case EIncident::ContainmentNewsletter: return FString::Printf(TEXT("Delgado and Pell are publishing a containment newsletter: labs x%.2f speed (%.0f s)"), NewsletterLabSpeed, Left);
	case EIncident::AcademicDispute: return FString::Printf(TEXT("Dr. Marsh and Prof. Pike are having an academic dispute: labs x%.2f speed (%.0f s)"), DisputeLabSpeed, Left);
	default: return FString();
	}
}

bool FAlienIdleSim::CanResolveIncident(FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	if (Incident.Type == EIncident::None)
	{
		return Fail(TEXT("no incident"));
	}
	if (Incident.Type == EIncident::InventoryReorganized)
	{
		return Fail(TEXT("this one is helping"));
	}
	if (Research < IncidentResolveCost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f research"), IncidentResolveCost));
	}
	return true;
}

bool FAlienIdleSim::ResolveIncident()
{
	if (!CanResolveIncident())
	{
		return false;
	}
	Research -= IncidentResolveCost;
	Incident = FActiveIncident();
	IncidentCooldown = IncidentCooldownSeconds;
	AddLog(FString::Printf(TEXT("Management intervened. It cost %.0f research and a strongly worded form."), IncidentResolveCost));
	return true;
}

// ---------------------------------------------------------------------------------------------
// Fleet

void FAlienIdleSim::StepAutomation(int32 Index)
{
	FUfo& Ufo = Ufos[Index];
	if (!Ufo.bFollowContract || !IsAutomationUnlocked() || !bHasActiveContract)
	{
		return;
	}
	for (const FContractNeed& Need : ActiveContract.Needs)
	{
		if (Need.IsMet() || !IsSpecimenUnlocked(Need.Type))
		{
			continue;
		}
		if (GetTractorStrength(Index) < Specimen(Need.Type).Weight)
		{
			continue;
		}
		const int32 Best = ChooseRegionFor(Index, Need.Type);
		if (Regions[Best].bLockedDown)
		{
			continue;
		}
		Ufo.Destination = Need.Type;
		Ufo.Region = Best;
		return;
	}
}

void FAlienIdleSim::StepUfo(int32 Index, float Dt)
{
	FUfo& Ufo = Ufos[Index];
	switch (Ufo.State)
	{
	case EUfoState::Docked:
		StepAutomation(Index);
		if (Ufo.bAutoLaunch && IsAutomationUnlocked() && bReroute)
		{
			// Automation steers around Earth's responses instead of waiting for the player.
			const int32 Best = ChooseRegionFor(Index, Ufo.Destination);
			if (Best != Ufo.Region)
			{
				++Reroutes;
				AddLog(FString::Printf(TEXT("'%s' rerouted from %s to %s. The flight plan has been amended and nobody has read it."),
					*Ufo.Name, *GetRegionNameAt(Ufo.Region), *GetRegionNameAt(Best)));
				Ufo.Region = Best;
			}
		}
		if (Ufo.bAutoLaunch && IsAutomationUnlocked() && CanLaunch(Index) && !IsHeldForContainment(Index))
		{
			Launch(Index);
		}
		return;
	case EUfoState::WaitingToUnload:
		TryUnload(Index);
		return;
	default:
		break;
	}

	Ufo.StateTime += Dt;
	if (Ufo.StateTime < Ufo.StateDuration)
	{
		return;
	}

	switch (Ufo.State)
	{
	case EUfoState::FlyingOut:
		SetUfoState(Ufo, EUfoState::Capturing, GetCaptureSeconds(Index, Ufo.TripTarget, Ufo.TripOccupation != EOccupation::None));
		break;
	case EUfoState::Capturing:
		FinishCapture(Index);
		break;
	case EUfoState::Returning:
		SetUfoState(Ufo, EUfoState::WaitingToUnload, 0.0f);
		TryUnload(Index);
		if (Ufo.State == EUfoState::WaitingToUnload)
		{
			AddLog(FString::Printf(TEXT("'%s' is waiting at the dock: containment is full. Overcrowding is an exciting opportunity for social research."), *Ufo.Name));
		}
		break;
	default:
		break;
	}
}

void FAlienIdleSim::FinishCapture(int32 Index)
{
	FUfo& Ufo = Ufos[Index];
	const FSpecimenDef& Def = Specimen(Ufo.TripTarget);
	// The Bureau got there first: a sweep halves what is left to take.
	// Rival breach: the region is full of the Experimental Sciences Office's escapees, so every trip brings one extra.
	const int32 Count = (IsSwept(Ufo.TripRegion, Ufo.TripTarget) ? FMath::Max(1, GetCargoCapacity(Index) / 2) : GetCargoCapacity(Index))
		+ (IsBreach(Ufo.TripRegion) ? 1 : 0);
	Rival.Usage[FMath::Clamp(Ufo.TripRegion, 0, NumRegions - 1)][int32(Ufo.TripTarget)] += float(Count);
	double Suspicion = 0.0;
	TArray<FString> Names;
	for (int32 i = 0; i < Count; ++i)
	{
		FSpecimen& Captured = Ufo.Cargo.AddDefaulted_GetRef();
		Captured.Type = Ufo.TripTarget;
		Captured.Region = uint8(Ufo.TripRegion);
		if (Ufo.TripTarget == ESpecimen::Human)
		{
			const EOccupation Featured = GetRegionDef(Ufo.TripRegion).Featured;
			Captured.Occupation = Ufo.TripOccupation != EOccupation::None ? Ufo.TripOccupation
				: (Featured != EOccupation::None && Rng.FRand() < 0.3f) ? Featured
				: RandomOccupation();
			// A recurring subject: the one the scanner was looking for, or (rarely) one who turns up by chance.
			int32 Case = (i == 0 && Ufo.TripCase >= 0 && !IsCaseOnBase(Ufo.TripCase)) ? int32(Ufo.TripCase) : INDEX_NONE;
			if (Case == INDEX_NONE && Ufo.TripOccupation == EOccupation::None)
			{
				Case = RollCaseAppearance(Ufo.Cargo);
			}
			if (Case != INDEX_NONE)
			{
				Captured.Case = int8(Case);
				Captured.Occupation = CaseSubjects[Case].Occupation;
				Names.Add(CaseSubjects[Case].Name);
			}
			else
			{
				Names.Add(WithArticle(FString(Occupation(Captured.Occupation).Name).ToLower()));
			}
		}
		++Captures;
		Suspicion += Def.SuspicionPerSpecimen * Occupation(Captured.Occupation).SuspicionMultiplier;
		RecordEncyclopedia(Captured);
		if (Captured.Case >= 0)
		{
			MeetCase(Captured, Index);
		}
	}
	AddSuspicion(Ufo.TripRegion, Suspicion * GetSuspicionMultiplier(Index), Ufo.TripTarget);
	const float Stealthy = FMath::Pow(0.7f, float(GetShipLevel(Index, EShipSystem::Stealth)));
	if (Ufo.TripTarget == ESpecimen::Human)
	{
		AddPressure(Ufo.TripRegion, EResponse::Investigators, InvestigatorPressurePerHuman * Count * Stealthy);
	}
	else
	{
		AddPressure(Ufo.TripRegion, EResponse::NightPatrols, PatrolPressurePerAnimal * Count * Stealthy * (IsUnderwaterRegion(Ufo.TripRegion) ? 0.5f : 1.0f));
	}

	if (Ufo.TripTarget == ESpecimen::Human)
	{
		AddLog(FString::Printf(TEXT("'%s' acquired %s%s. Each was told this is 'a quick survey.'"), *Ufo.Name, *FString::Join(Names, TEXT(", ")),
			Ufo.TripOccupation != EOccupation::None ? TEXT(" (scanned)") : TEXT("")));
	}
	else
	{
		AddLog(FString::Printf(TEXT("'%s' acquired %d %s. Pilot reports being outmaneuvered twice, emotionally."), *Ufo.Name, Count, Count == 1 ? Def.Name : Def.Plural));
	}
	SetUfoState(Ufo, EUfoState::Returning, GetFlightSeconds(Index, Ufo.TripTarget));
}

void FAlienIdleSim::TryUnload(int32 Index)
{
	FUfo& Ufo = Ufos[Index];
	while (Ufo.Cargo.Num() > 0)
	{
		if (GetContainmentUnitsUsed() + GetUnits(Ufo.Cargo[0]) > GetContainmentCapacity())
		{
			return;
		}
		Containment.Add(Ufo.Cargo[0]);
		Ufo.Cargo.RemoveAt(0);
	}
	SetUfoState(Ufo, EUfoState::Docked, 0.0f);
}

void FAlienIdleSim::SetUfoState(FUfo& Ufo, EUfoState NewState, float Duration)
{
	Ufo.State = NewState;
	Ufo.StateTime = 0.0f;
	Ufo.StateDuration = Duration;
}

bool FAlienIdleSim::CanLaunch(int32 Index, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};

	const FUfo& Ufo = Ufos[Index];
	const FSpecimenDef& Def = Specimen(Ufo.Destination);
	if (Ufo.State == EUfoState::WaitingToUnload)
	{
		return Fail(TEXT("waiting for containment space"));
	}
	if (Ufo.State != EUfoState::Docked)
	{
		return Fail(TEXT("out on an expedition"));
	}
	if (!IsStationStaffed({ EStation::Ufo, Index }))
	{
		return Fail(TEXT("no crew. Assign a pilot in Alien Resources. Management considers this sustainable"));
	}
	if (Incident.Type == EIncident::TractorJam && Incident.Ufo == Index)
	{
		return Fail(TEXT("tractor beam jammed (incident)"));
	}
	if (!IsSpecimenUnlocked(Ufo.Destination))
	{
		return Fail(GetUnlockRequirementText(Ufo.Destination));
	}
	if (GetTractorStrength(Index) < Def.Weight)
	{
		return Fail(FString::Printf(TEXT("tractor beam too weak for %s (strength %d, need %d)"), Def.Plural, GetTractorStrength(Index), Def.Weight));
	}
	if (!RegionHasSpecimen(Ufo.Region, Ufo.Destination))
	{
		return Fail(FString::Printf(TEXT("no %s in %s"), Def.Plural, *GetRegionNameAt(Ufo.Region)));
	}
	if (IsSurfaceRegion(Ufo.Region) && bAirlockSealed)
	{
		return Fail(FString::Printf(TEXT("surface airlock sealed: hull pressure %.0f (reopens below %.0f)"), HullPressure, HullCreakPressure));
	}
	if (Regions[Ufo.Region].bLockedDown)
	{
		return Fail(FString::Printf(TEXT("%s is in lockdown (suspicion %.0f, reopens below %.0f)"), *GetRegionNameAt(Ufo.Region), Regions[Ufo.Region].Suspicion, GetLockdownRelease()));
	}
	const double Cost = GetLaunchEnergy(Index, Ufo.Destination);
	if (Energy < Cost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f energy"), Cost));
	}
	return true;
}

bool FAlienIdleSim::Launch(int32 Index)
{
	if (!CanLaunch(Index))
	{
		return false;
	}
	FUfo& Ufo = Ufos[Index];
	Energy -= GetLaunchEnergy(Index, Ufo.Destination);
	Ufo.TripTarget = Ufo.Destination;
	Ufo.TripRegion = Ufo.Region;
	// Heavy traffic draws radar; small, stealthy ships and camouflage draw less. Nobody's radar looks underwater.
	if (!IsUnderwaterRegion(Ufo.TripRegion))
	{
		AddPressure(Ufo.TripRegion, EResponse::Radar, RadarPressurePerLaunch * FMath::Pow(0.6f, float(GetShipLevel(Index, EShipSystem::Stealth)))
			* (GetBaseLevel(EBaseUpgrade::Camouflage) > 0 ? 0.5f : 1.0f) * (GetBaseLevel(EBaseUpgrade::SonarArray) > 0 ? 1.5f : 1.0f)
			* (1.0f + 0.25f * GetBaseLevel(EBaseUpgrade::GeothermalTaps)));
	}
	if (HasHull() && IsSurfaceRegion(Ufo.TripRegion))
	{
		HullPressure = FMath::Min(100.0f, HullPressure + HullStressPerSurfaceLaunch * AlienIdleTuning::Thesis(Thesis).HullStressMultiplier);
	}
	if (Regions[Ufo.TripRegion].Active[int32(EResponse::Radar)])
	{
		AddSuspicion(Ufo.TripRegion, 4.0, Ufo.TripTarget);
	}
	Ufo.TripOccupation = GetScannerTarget(Index, Ufo.Destination);
	Ufo.TripCase = -1;
	if (Ufo.CaseTarget >= 0 && Ufo.Destination == ESpecimen::Human && GetShipLevel(Index, EShipSystem::Scanner) > 0 && !IsCaseOnBase(Ufo.CaseTarget))
	{
		Ufo.TripCase = Ufo.CaseTarget;
		Ufo.TripOccupation = CaseSubjects[Ufo.CaseTarget].Occupation;
	}
	SetUfoState(Ufo, EUfoState::FlyingOut, GetFlightSeconds(Index, Ufo.TripTarget));
	if (IsProjectDone(EDeptProject::ExperimentalModules) && Rng.FRand() < 0.1f)
	{
		Ufo.StateDuration *= 1.5f;
		AddLog(FString::Printf(TEXT("'%s' experienced an experimental module 'learning experience'. The outbound flight will take 50%% longer."), *Ufo.Name));
	}
	if (!(Ufo.bAutoLaunch && IsAutomationUnlocked()))
	{
		AddLog(FString::Printf(TEXT("'%s' dispatched to %s%s. The seatbelt sign remains decorative."), *Ufo.Name, *GetRegionNameAt(Ufo.TripRegion),
			Ufo.TripCase >= 0 ? *FString::Printf(TEXT(", scanning for %s"), CaseSubjects[Ufo.TripCase].Name)
			: Ufo.TripOccupation != EOccupation::None ? *FString::Printf(TEXT(", scanning for %s"), *WithArticle(FString(Occupation(Ufo.TripOccupation).Name).ToLower())) : TEXT("")));
	}
	return true;
}

EOccupation FAlienIdleSim::GetScannerTarget(int32 Index, ESpecimen Type) const
{
	if (Type != ESpecimen::Human || GetShipLevel(Index, EShipSystem::Scanner) == 0)
	{
		return EOccupation::None;
	}
	if (bHasActiveContract)
	{
		for (const FContractNeed& Need : ActiveContract.Needs)
		{
			if (!Need.IsMet() && Need.Type == ESpecimen::Human && Need.Occupation != EOccupation::None)
			{
				return Need.Occupation;
			}
		}
		for (const FContractNeed& Need : ActiveContract.Needs)
		{
			if (!Need.IsMet() && Need.Type == ESpecimen::Human && Need.bDistinctOccupations)
			{
				for (int32 o = 1; o < int32(EOccupation::Count); ++o)
				{
					if (!Need.Seen.Contains(EOccupation(o)) && !IsCaseOccupation(EOccupation(o)))
					{
						return EOccupation(o);
					}
				}
			}
		}
	}
	if (PendingSignature != EDepartment::None)
	{
		TArray<int32> Missing;
		GetSignatureReservation(&Missing);
		for (int32 Need : Missing)
		{
			const FDepartmentDef& Dept = Department(PendingSignature);
			if (Dept.NeedType[Need] == ESpecimen::Human && Dept.NeedOccupation[Need] != EOccupation::None)
			{
				return Dept.NeedOccupation[Need];
			}
		}
	}
	const FExperimentChoice& Choice = Choices[int32(ESpecimen::Human)];
	if (Choice.Kind == EExperimentKind::Discovery && IsDiscoveryOpen(Choice.Discovery))
	{
		return Discovery(Choice.Discovery).RequiredOccupation;
	}
	return EOccupation::None;
}

void FAlienIdleSim::CycleDestination(int32 Index, int32 Direction)
{
	// Every (region, specimen) pair, locked specimens included so the player sees what is next.
	TArray<TPair<int32, ESpecimen>> Pairs;
	for (int32 r = 0; r < NumRegions; ++r)
	{
		const FRegionDef& Def = GetRegionDef(r);
		for (int32 k = 0; k < Def.NumSpecimens; ++k)
		{
			Pairs.Add({ r, Def.Specimens[k] });
		}
	}
	int32 Current = Pairs.IndexOfByPredicate([this, Index](const TPair<int32, ESpecimen>& P) { return P.Key == Ufos[Index].Region && P.Value == Ufos[Index].Destination; });
	Current = Current == INDEX_NONE ? 0 : Current;
	const int32 Num = Pairs.Num();
	const TPair<int32, ESpecimen>& Next = Pairs[((Current + Direction) % Num + Num) % Num];
	Ufos[Index].Region = Next.Key;
	Ufos[Index].Destination = Next.Value;
}

void FAlienIdleSim::SetDestination(int32 Index, ESpecimen Type)
{
	SetDestinationRegion(Index, GetHomeRegion(Type), Type);
}

void FAlienIdleSim::SetDestinationRegion(int32 Index, int32 Region, ESpecimen Type)
{
	Ufos[Index].Destination = Type;
	Ufos[Index].Region = FMath::Clamp(Region, 0, NumRegions - 1);
}

void FAlienIdleSim::SetDestinationBest(int32 Index, ESpecimen Type)
{
	SetDestinationRegion(Index, ChooseRegionFor(Index, Type), Type);
}

void FAlienIdleSim::SetAutoLaunch(int32 Index, bool bEnabled)
{
	Ufos[Index].bAutoLaunch = bEnabled;
}

void FAlienIdleSim::SetFollowContract(int32 Index, bool bEnabled)
{
	Ufos[Index].bFollowContract = bEnabled;
}

// ---------------------------------------------------------------------------------------------
// Lab

int32 FAlienIdleSim::GetUnits(const FSpecimen& Held)
{
	return Held.Type == ESpecimen::Human ? Occupation(Held.Occupation).ContainmentUnits : Specimen(Held.Type).ContainmentUnits;
}

int32 FAlienIdleSim::PickNextSpecimen() const
{
	if (Containment.Num() == 0)
	{
		return INDEX_NONE;
	}
	TArray<int32> Reserved = GetSignatureReservation();
	// Rival academics wait for each other so they can be studied together (Peer Review).
	for (int32 i = 0; i < Containment.Num(); ++i)
	{
		if (IsWaitingForPartner(Containment[i]))
		{
			Reserved.AddUnique(i);
		}
	}
	const bool bAging = bAgeSamples && IsPreserving();
	auto Young = [this, bAging](int32 i) { return bAging && Containment[i].Held < PreservedSampleSeconds; };
	// 1. Whatever moves the active contract forward.
	if (bHasActiveContract)
	{
		for (int32 i = 0; i < Containment.Num(); ++i)
		{
			if (Reserved.Contains(i) || Young(i))
			{
				continue;
			}
			for (const FContractNeed& Need : ActiveContract.Needs)
			{
				if (Need.Accepts(Containment[i]))
				{
					return i;
				}
			}
		}
	}
	// 2. A specimen that can run its type's chosen discovery project.
	for (int32 i = 0; i < Containment.Num(); ++i)
	{
		const FExperimentChoice& Choice = Choices[int32(Containment[i].Type)];
		if (!Reserved.Contains(i) && Choice.Kind == EExperimentKind::Discovery && ResolveExperiment(Containment[i]).Kind == EExperimentKind::Discovery)
		{
			return i;
		}
	}
	// 3. Oldest first, skipping specimens held for a signature experiment (and, in freezer mode, ones still curing).
	for (int32 i = 0; i < Containment.Num(); ++i)
	{
		if (!Reserved.Contains(i) && !Young(i))
		{
			return i;
		}
	}
	// 4. Never deadlock: if specimens held for the signature experiment block a ship from unloading, study the oldest anyway.
	const bool bShipWaiting = Ufos.ContainsByPredicate([](const FUfo& U) { return U.State == EUfoState::WaitingToUnload; });
	if (Reserved.Num() > 0 && (GetContainmentUnitsUsed() >= GetContainmentCapacity() || bShipWaiting))
	{
		return Reserved[0];
	}
	return INDEX_NONE;
}

FExperimentChoice FAlienIdleSim::ResolveExperiment(const FSpecimen& Subject) const
{
	FExperimentChoice Choice = Choices[int32(Subject.Type)];
	// Recurring subjects get their personal research chain unless the player picked a project they can run.
	if (Subject.Case >= 0 && bCaseStudies)
	{
		const int32 Step = GetNextCaseStep(Subject.Case);
		const bool bProjectFits = Choice.Kind == EExperimentKind::Discovery && IsDiscoveryOpen(Choice.Discovery)
			&& Discovery(Choice.Discovery).RequiredOccupation == Subject.Occupation && Subject.Occupation != EOccupation::None;
		if (Step != INDEX_NONE && !bProjectFits)
		{
			FExperimentChoice CaseStudy;
			CaseStudy.Kind = EExperimentKind::CaseStudy;
			CaseStudy.CaseStep = uint8(Step);
			return CaseStudy;
		}
	}
	if (Choice.Kind != EExperimentKind::Discovery)
	{
		return Choice;
	}
	const FDiscoveryDef& Def = Discovery(Choice.Discovery);
	bool bRunningElsewhere = false;
	for (const FLabChamber& Chamber : Chambers)
	{
		bRunningElsewhere |= Chamber.bRunning && Chamber.Experiment.Kind == EExperimentKind::Discovery && Chamber.Experiment.Discovery == Choice.Discovery;
	}
	if (!IsDiscoveryOpen(Choice.Discovery) || bRunningElsewhere
		|| (Def.RequiredOccupation != EOccupation::None && Subject.Occupation != Def.RequiredOccupation))
	{
		return FExperimentChoice(); // routine instead
	}
	return Choice;
}

float FAlienIdleSim::GetExperimentSeconds(const FSpecimen& Subject, const FExperimentChoice& Choice) const
{
	const FOccupationDef& Occ = Occupation(Subject.Occupation);
	const bool bBehavioral = Subject.Type == ESpecimen::Human && IsProjectDone(EDeptProject::BehavioralLab);
	const float Duration = bBehavioral ? FMath::Min(Occ.DurationMultiplier, 1.0f) * 0.7f : Occ.DurationMultiplier;
	switch (Choice.Kind)
	{
	case EExperimentKind::Signature: return SignatureSeconds;
	case EExperimentKind::CaseStudy:
		if (Choice.CaseStep >= PeerReviewStep || Subject.Case < 0)
		{
			return PeerReviewSeconds;
		}
		return CaseSubjects[Subject.Case].Steps[Choice.CaseStep].Seconds * (Career.Cases[Subject.Case].BestSteps > Choice.CaseStep ? CaseRepeatStepSpeedup : 1.0f);
	case EExperimentKind::Discovery: return Discovery(Choice.Discovery).Seconds * Occ.DiscoverySpeedMultiplier * Duration
		* (Career.Published[int32(Choice.Discovery)] ? PublishedDiscoverySpeedup : 1.0f) * (HasCaseDiscovery(CaseMarsh) ? CaseDiscoverySpeed : 1.0f);
	case EExperimentKind::Extended: return Specimen(Subject.Type).LabSeconds * ExtendedDurationMultiplier * Duration;
	default: return Specimen(Subject.Type).LabSeconds * Duration;
	}
}

double FAlienIdleSim::GetExperimentEnergy(const FSpecimen& Subject, const FExperimentChoice& Choice) const
{
	switch (Choice.Kind)
	{
	case EExperimentKind::Signature: return SignatureEnergy;
	case EExperimentKind::CaseStudy:
		return (Choice.CaseStep >= PeerReviewStep || Subject.Case < 0) ? PeerReviewEnergy : CaseSubjects[Subject.Case].Steps[Choice.CaseStep].Energy;
	case EExperimentKind::Discovery: return Discovery(Choice.Discovery).Energy;
	case EExperimentKind::Extended: return Specimen(Subject.Type).ExperimentEnergy * ExtendedEnergyMultiplier * (GetBaseLevel(EBaseUpgrade::InsulatedLabs) > 0 ? 0.7 : 1.0);
	default: return Specimen(Subject.Type).ExperimentEnergy * (GetBaseLevel(EBaseUpgrade::InsulatedLabs) > 0 ? 0.7 : 1.0);
	}
}

double FAlienIdleSim::GetExperimentResearch(const FSpecimen& Subject, const FExperimentChoice& Choice) const
{
	if (Choice.Kind == EExperimentKind::Discovery || Choice.Kind == EExperimentKind::Signature)
	{
		return 0.0;
	}
	if (Choice.Kind == EExperimentKind::CaseStudy)
	{
		// Personal research pays a fixed amount (the last step pays in a discovery instead); kindred company still helps.
		const double Base = (Choice.CaseStep >= PeerReviewStep || Subject.Case < 0) ? PeerReviewResearch : CaseSubjects[Subject.Case].Steps[Choice.CaseStep].Research;
		return FMath::RoundToDouble(Base * (Subject.Case >= 0 && IsKindredHeld(Subject.Case) ? KindredResearchMultiplier : 1.0));
	}
	const int32 From = FMath::Clamp(int32(Subject.Region), 0, NumRegions - 1);
	double Amount = Specimen(Subject.Type).Research * Occupation(Subject.Occupation).ResearchMultiplier * GetRegionDef(From).Value;
	if (Subject.Type != ESpecimen::Human && Regions[From].Active[int32(EResponse::NightPatrols)])
	{
		Amount *= 0.8; // patrol-stressed specimens
	}
	if (Subject.Type == ESpecimen::Human)
	{
		Amount *= EarthCondition(Condition).HumanResearch * (Regions[From].Active[int32(EResponse::SupportGroup)] ? 0.9 : 1.0);
	}
	if (Subject.Type == ESpecimen::Human)
	{
		Amount *= (IsProjectDone(EDeptProject::AdvancedScanners) ? 1.25 : 1.0) * (HasSignatureBonus(EDepartment::Human) ? SignatureResearchBonus : 1.0);
	}
	else if (HasSignatureBonus(EDepartment::Biological))
	{
		Amount *= SignatureResearchBonus;
	}
	if (Choice.Kind == EExperimentKind::Extended)
	{
		Amount *= ExtendedResearchMultiplier;
	}
	if (Subject.Type == ESpecimen::Human && IsDiscovered(EDiscovery::SarcasmOrgan))
	{
		Amount *= SarcasmHumanResearchMultiplier;
	}
	if (IsPreserving())
	{
		Amount *= 1.0 + PreserveResearchBonus * FMath::Min(1.0f, Subject.Held / PreserveSeconds);
	}
	Amount *= GetCaseResearchMultiplier(Subject);
	return FMath::RoundToDouble(Amount);
}

float FAlienIdleSim::GetLabSpeed() const
{
	if (IsInMeeting() || Incident.Type == EIncident::StaffMeeting)
	{
		return 0.0f;
	}
	float Speed = 1.0f;
	bool bTherapist = Containment.ContainsByPredicate([](const FSpecimen& S) { return S.Occupation == EOccupation::Therapist; });
	for (const FLabChamber& Chamber : Chambers)
	{
		bTherapist |= Chamber.bRunning && Chamber.Specimen.Occupation == EOccupation::Therapist;
	}
	if (bTherapist)
	{
		Speed *= TherapistLabSpeed;
	}
	if (bMiddleManagement)
	{
		Speed *= MiddleManagementLabSpeed;
	}
	if (Incident.Type == EIncident::CoffeeFailure)
	{
		Speed *= 0.7f;
	}
	if (Incident.Type == EIncident::WhaleSong)
	{
		Speed *= 0.85f;
	}
	if (Incident.Type == EIncident::ContainmentNewsletter)
	{
		Speed *= NewsletterLabSpeed;
	}
	if (Incident.Type == EIncident::AcademicDispute)
	{
		Speed *= DisputeLabSpeed;
	}
	if (HasHull() && HullPressure >= HullCreakPressure)
	{
		Speed *= HullCreakLabSpeed;
	}
	if (HasHeat())
	{
		Speed *= ColdLabSpeedFloor + (1.0f - ColdLabSpeedFloor) * GetLabWarmth();
	}
	if (GetBaseLevel(EBaseUpgrade::InsulatedLabs) > 0)
	{
		Speed *= 0.95f; // too cosy: scientists nap
	}
	return Speed;
}

void FAlienIdleSim::StepLab(float Dt)
{
	const float Speed = GetLabSpeed();
	bool bFreedSpace = false;
	for (int32 c = 0; c < Chambers.Num(); ++c)
	{
		FLabChamber& Chamber = Chambers[c];
		const FAssignment Post{ EStation::Chamber, c };
		const bool bStaffed = IsStationStaffed(Post);
		if (Chamber.bRunning)
		{
			Chamber.Time += Dt * Speed * (bStaffed ? 1.0f + LabSpeedPerBonus * GetStationBonus(Post) : 0.0f);
			if (Chamber.Time >= Chamber.Duration)
			{
				FinishExperiment(Chamber);
			}
		}

		if (!Chamber.bRunning && !IsInMeeting() && bStaffed && TryStartSignature(Chamber))
		{
			bFreedSpace = true;
		}
		if (!Chamber.bRunning && !IsInMeeting() && bStaffed && TryStartPeerReview(Chamber))
		{
			bFreedSpace = true;
		}
		if (!Chamber.bRunning && !IsInMeeting() && bStaffed)
		{
			const int32 Pick = PickNextSpecimen();
			if (Pick != INDEX_NONE)
			{
				const FSpecimen Subject = Containment[Pick];
				const FExperimentChoice Choice = ResolveExperiment(Subject);
				const double Cost = GetExperimentEnergy(Subject, Choice);
				if (Energy >= Cost)
				{
					Energy -= Cost;
					Chamber.bRunning = true;
					Chamber.Time = 0.0f;
					Chamber.Duration = GetExperimentSeconds(Subject, Choice);
					Chamber.Specimen = Subject;
					Chamber.Experiment = Choice;
					Chamber.Batch = 1;
					Containment.RemoveAt(Pick);
					// Batch Experiments: take a second animal of the same species along.
					if (Subject.Type != ESpecimen::Human && Choice.Kind != EExperimentKind::Discovery && IsProjectDone(EDeptProject::BatchExperiments))
					{
						const TArray<int32> Reserved = GetSignatureReservation();
						for (int32 i = 0; i < Containment.Num(); ++i)
						{
							if (Containment[i].Type == Subject.Type && !Reserved.Contains(i))
							{
								Containment.RemoveAt(i);
								Chamber.Batch = 2;
								break;
							}
						}
					}
					bFreedSpace = true;
				}
			}
		}
	}

	if (bFreedSpace)
	{
		for (int32 i = 0; i < Ufos.Num(); ++i)
		{
			if (Ufos[i].State == EUfoState::WaitingToUnload)
			{
				TryUnload(i);
			}
		}
	}
}

void FAlienIdleSim::FinishExperiment(FLabChamber& Chamber)
{
	const FSpecimen Subject = Chamber.Specimen;
	const FSpecimen Partner = Chamber.Partner;
	const FExperimentChoice Choice = Chamber.Experiment;
	const int32 Batch = FMath::Max(1, Chamber.Batch);
	Chamber = FLabChamber();

	if (Choice.Kind == EExperimentKind::Signature)
	{
		CompleteSignature(Choice.Department);
		return;
	}

	const int32 TypeCount = ExperimentsByType[int32(Subject.Type)];
	if (IsPreserving() && Subject.Held >= PreservedSampleSeconds)
	{
		PreservedStudied += Batch;
	}
	const int32 Studied = Batch + (Partner.Case >= 0 ? 1 : 0);
	ExperimentsByType[int32(Subject.Type)] += Studied;
	TotalExperiments += Studied;

	if (Choice.Kind == EExperimentKind::CaseStudy)
	{
		CompleteCaseStudy(Subject, Partner, Choice.CaseStep);
	}
	else if (Choice.Kind == EExperimentKind::Discovery)
	{
		Discover(Choice.Discovery);
		if (SameChoice(Choices[int32(Subject.Type)], Choice))
		{
			Choices[int32(Subject.Type)] = FExperimentChoice();
		}
	}
	else
	{
		const double Earned = GetExperimentResearch(Subject, Choice) * Batch;
		Research += Earned;
		ResearchEarned += Earned;

		FString Report;
		switch (Subject.Type)
		{
		case ESpecimen::Squirrel: Report = SquirrelReports[TypeCount % UE_ARRAY_COUNT(SquirrelReports)]; break;
		case ESpecimen::Cow:      Report = CowReports[TypeCount % UE_ARRAY_COUNT(CowReports)]; break;
		case ESpecimen::Elephant: Report = ElephantReports[TypeCount % UE_ARRAY_COUNT(ElephantReports)]; break;
		case ESpecimen::Pufferfish: Report = PufferfishReports[TypeCount % UE_ARRAY_COUNT(PufferfishReports)]; break;
		case ESpecimen::Octopus: Report = OctopusReports[TypeCount % UE_ARRAY_COUNT(OctopusReports)]; break;
		case ESpecimen::Whale: Report = WhaleReports[TypeCount % UE_ARRAY_COUNT(WhaleReports)]; break;
		case ESpecimen::Penguin: Report = PenguinReports[TypeCount % UE_ARRAY_COUNT(PenguinReports)]; break;
		case ESpecimen::Seal: Report = SealReports[TypeCount % UE_ARRAY_COUNT(SealReports)]; break;
		case ESpecimen::ElephantSeal: Report = ElephantSealReports[TypeCount % UE_ARRAY_COUNT(ElephantSealReports)]; break;
		case ESpecimen::Human:
		{
			const TCHAR* ByJob = OccupationReport(Subject.Occupation);
			Report = (ByJob && TypeCount % 3 != 2) ? ByJob : GenericHumanReports[TypeCount % UE_ARRAY_COUNT(GenericHumanReports)];
			break;
		}
		default: break;
		}
		if (PrimaryDepartment != EDepartment::None && TotalExperiments % 2 == 0)
		{
			switch (PrimaryDepartment)
			{
			case EDepartment::Biological: Report = BiologyReports[(TotalExperiments / 2) % UE_ARRAY_COUNT(BiologyReports)]; break;
			case EDepartment::Human: Report = HumanReports[(TotalExperiments / 2) % UE_ARRAY_COUNT(HumanReports)]; break;
			default: Report = IncompetenceReports[(TotalExperiments / 2) % UE_ARRAY_COUNT(IncompetenceReports)]; break;
			}
		}
		if (Subject.Case >= 0)
		{
			AddCaseVisit(Subject.Case, Choice.Kind == EExperimentKind::Extended ? TEXT("extended study") : TEXT("routine study"));
			if (Career.Cases[Subject.Case].Encounters > 1)
			{
				Report = FString::Printf(TEXT("%s recognised the equipment and helped, a little too much."), CaseSubjects[Subject.Case].Short);
			}
		}
		const FString What = Batch > 1 ? FString::Printf(TEXT("batch of %d %s"), Batch, Specimen(Subject.Type).Plural) : GetSpecimenLabel(Subject);
		AddResult(FString::Printf(TEXT("%s #%d (%s): %s Research +%.0f. %s"),
			Choice.Kind == EExperimentKind::Extended ? TEXT("Extended study") : TEXT("Experiment"), TotalExperiments, *What,
			*Report, Earned, ReturnNotes[TotalExperiments % UE_ARRAY_COUNT(ReturnNotes)]));
	}

	for (int32 i = 0; i < Batch; ++i)
	{
		ReturnSpecimen(Subject);
		CountTowardContract(Subject);
		CountTowardRace(Subject);
	}
	if (Partner.Case >= 0)
	{
		ReturnSpecimen(Partner);
		CountTowardContract(Partner);
		CountTowardRace(Partner);
	}
}

// ---------------------------------------------------------------------------------------------
// Discoveries and encyclopedia

bool FAlienIdleSim::IsDiscoveryOpen(EDiscovery Type, FString* OutReason) const
{
	if (Type == EDiscovery::None)
	{
		return false;
	}
	const FDiscoveryDef& Def = Discovery(Type);
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	if (IsDiscovered(Type))
	{
		return Fail(TEXT("already discovered"));
	}
	const ESpecimen On = GetDiscoverySpecimen(Type);
	if (!IsSpecimenUnlocked(On))
	{
		return Fail(FString::Printf(TEXT("%s not yet approved"), Specimen(On).Plural));
	}
	const int32 Done = ExperimentsByType[int32(On)];
	if (Done < Def.RequiredExperiments)
	{
		return Fail(FString::Printf(TEXT("needs baseline data: %d/%d %s experiments"), Done, Def.RequiredExperiments, Specimen(On).Name));
	}
	return true;
}

int32 FAlienIdleSim::GetDiscoveryCount() const
{
	int32 Count = 0;
	for (bool bDone : Discovered)
	{
		Count += bDone ? 1 : 0;
	}
	return Count;
}

void FAlienIdleSim::Discover(EDiscovery Type)
{
	if (Type == EDiscovery::None || Discovered[int32(Type)])
	{
		return;
	}
	Discovered[int32(Type)] = true;
	AnnouncedProject[int32(Type)] = true;
	if (!Career.Published[int32(Type)])
	{
		Career.Published[int32(Type)] = true;
		++RunNewPublications;
	}
	const FDiscoveryDef& Def = Discovery(Type);
	AddResult(FString::Printf(TEXT("DISCOVERY: '%s'. %s (%s)"), Def.Name, Def.Report, Def.Unlocks));
	AddEncyclopediaEntry(FString::Printf(TEXT("Discovery: %s"), Def.Name), Def.Report);
}

void FAlienIdleSim::DevDiscover(EDiscovery Type)
{
	Discover(Type);
}

void FAlienIdleSim::DevUnlockSpecimen(ESpecimen Type)
{
	DevUnlocked[int32(Type)] = true;
}

void FAlienIdleSim::SetMiddleManagement(bool bEnabled)
{
	if (!IsMiddleManagementAvailable() || bEnabled == bMiddleManagement)
	{
		return;
	}
	bMiddleManagement = bEnabled;
	MeetingTimer = 0.0f;
	MeetingRemaining = 0.0f;
	AddLog(bEnabled
		? TEXT("An alien has been promoted to Middle Management. Productivity is up 20%. A meeting has been scheduled to discuss this.")
		: TEXT("Middle Management has been reassigned to 'special projects.' Nobody can find it."));
}

void FAlienIdleSim::RecordEncyclopedia(const FSpecimen& Captured)
{
	if (!SeenSpecimen[int32(Captured.Type)])
	{
		SeenSpecimen[int32(Captured.Type)] = true;
		const FSpecimenDef& Def = Specimen(Captured.Type);
		AddEncyclopediaEntry(Capitalized(Def.Name), Def.Encyclopedia);
	}
	if (Captured.Occupation != EOccupation::None && !SeenOccupation[int32(Captured.Occupation)])
	{
		SeenOccupation[int32(Captured.Occupation)] = true;
		const FOccupationDef& Occ = Occupation(Captured.Occupation);
		AddEncyclopediaEntry(Occ.Name, Occ.Encyclopedia);
	}
}

void FAlienIdleSim::AddEncyclopediaEntry(const FString& Title, const FString& Text)
{
	if (Encyclopedia.ContainsByPredicate([&Title](const FEncyclopediaEntry& E) { return E.Title == Title; }))
	{
		return; // already filed in an earlier run
	}
	Encyclopedia.Add({ Title, Text, Elapsed });
	if (!Title.StartsWith(TEXT("Discovery:")))
	{
		++RunNewEntries;
		AddLog(FString::Printf(TEXT("Encyclopedia entry filed, '%s': %s"), *Title, *Text));
	}
}

int32 FAlienIdleSim::GetEncyclopediaSize()
{
	// Plus a case file and a case-file discovery per recurring subject, and the rivals' Peer Review.
	return int32(ESpecimen::Count) + int32(EOccupation::Count) - 1 + int32(EDiscovery::Count) + 2 * NumCaseSubjects + 1;
}

// ---------------------------------------------------------------------------------------------
// Lab priorities

TArray<FExperimentChoice> FAlienIdleSim::GetExperimentOptions(ESpecimen Type) const
{
	TArray<FExperimentChoice> Options;
	Options.Add({ EExperimentKind::Routine, EDiscovery::None });
	Options.Add({ EExperimentKind::Extended, EDiscovery::None });
	for (int32 d = 0; d < int32(EDiscovery::Count); ++d)
	{
		if (GetDiscoverySpecimen(EDiscovery(d)) == Type && IsDiscoveryOpen(EDiscovery(d)))
		{
			Options.Add({ EExperimentKind::Discovery, EDiscovery(d) });
		}
	}
	return Options;
}

void FAlienIdleSim::SetExperimentChoice(ESpecimen Type, const FExperimentChoice& Choice)
{
	Choices[int32(Type)] = Choice;
}

void FAlienIdleSim::CycleExperimentChoice(ESpecimen Type, int32 Direction)
{
	const TArray<FExperimentChoice> Options = GetExperimentOptions(Type);
	int32 Current = Options.IndexOfByPredicate([this, Type](const FExperimentChoice& C) { return SameChoice(C, Choices[int32(Type)]); });
	Current = Current == INDEX_NONE ? 0 : Current;
	const int32 Num = Options.Num();
	Choices[int32(Type)] = Options[((Current + Direction) % Num + Num) % Num];
}

FString FAlienIdleSim::DescribeExperiment(ESpecimen Type, const FExperimentChoice& Choice) const
{
	const FSpecimen Typical{ Type, EOccupation::None };
	switch (Choice.Kind)
	{
	case EExperimentKind::Routine:
		return FString::Printf(TEXT("Routine study: %.0f s, %.0f energy, %.0f research"),
			GetExperimentSeconds(Typical, Choice), GetExperimentEnergy(Typical, Choice), GetExperimentResearch(Typical, Choice));
	case EExperimentKind::Extended:
		return FString::Printf(TEXT("Extended study: %.0f s, %.0f energy, %.0f research"),
			GetExperimentSeconds(Typical, Choice), GetExperimentEnergy(Typical, Choice), GetExperimentResearch(Typical, Choice));
	case EExperimentKind::Discovery:
	{
		const FDiscoveryDef& Def = Discovery(Choice.Discovery);
		const FString Needs = Def.RequiredOccupation != EOccupation::None
			? FString::Printf(TEXT(", needs %s"), *WithArticle(FString(Occupation(Def.RequiredOccupation).Name).ToLower())) : FString();
		return FString::Printf(TEXT("Discovery '%s': %.0f s, %.0f energy%s. %s"), Def.Name, Def.Seconds, Def.Energy, *Needs, Def.Unlocks);
	}
	}
	return FString();
}

// ---------------------------------------------------------------------------------------------
// Contracts

void FAlienIdleSim::FillContractOffers()
{
	while (Offers.Num() < ContractOffers)
	{
		FContract Candidate = MakeContract();
		for (int32 Retry = 0; Retry < 6 && Offers.ContainsByPredicate([&](const FContract& C) { return C.Title == Candidate.Title; }); ++Retry)
		{
			Candidate = MakeContract();
		}
		Offers.Add(MoveTemp(Candidate));
	}
}

FContract FAlienIdleSim::MakeContract()
{
	++ContractsGenerated;
	if (PrimaryDepartment != EDepartment::None && Rng.FRand() < 0.5f)
	{
		bool bMade = false;
		FContract DeptContract = MakeDepartmentContract(PrimaryDepartment, bMade);
		if (bMade)
		{
			// Titles name the local stand-ins (penguins for squirrels on the ice, whales for elephants at sea).
			for (ESpecimen Type : { ESpecimen::Elephant, ESpecimen::Squirrel, ESpecimen::Cow })
			{
				if (Local(Type) != Type)
				{
					FString& Title = DeptContract.Title;
					Title = Title.Replace(Specimen(Type).Plural, Specimen(Local(Type)).Plural, ESearchCase::CaseSensitive)
						.Replace(*FString::Printf(TEXT("%s "), Specimen(Type).Name), *FString::Printf(TEXT("%s "), Specimen(Local(Type)).Name), ESearchCase::CaseSensitive)
						.Replace(*FString::Printf(TEXT("%s,"), Specimen(Type).Name), *FString::Printf(TEXT("%s,"), Specimen(Local(Type)).Name), ESearchCase::CaseSensitive);
					if (Title.EndsWith(Specimen(Type).Name))
					{
						Title = Title.LeftChop(FCString::Strlen(Specimen(Type).Name)) + Specimen(Local(Type)).Name;
					}
				}
			}
			return DeptContract;
		}
	}
	TArray<int32> Templates;
	Templates.Add(0); // census of an unlocked animal
	if (IsSpecimenUnlocked(Local(ESpecimen::Cow)) && IsSpecimenUnlocked(Local(ESpecimen::Squirrel))) { Templates.Add(1); }
	if (IsSpecimenUnlocked(Local(ESpecimen::Elephant)) && IsSpecimenUnlocked(Local(ESpecimen::Cow))) { Templates.Add(2); }
	if (IsSpecimenUnlocked(ESpecimen::Octopus)) { Templates.Append({ 6, 6 }); }
	if (IsSpecimenUnlocked(ESpecimen::Human)) { Templates.Append({ 3, 3, 4, 5 }); }

	FContract Contract;
	switch (Templates[Rng.RandRange(0, Templates.Num() - 1)])
	{
	case 0:
	{
		TArray<ESpecimen> Animals;
		for (ESpecimen Type : { ESpecimen::Squirrel, ESpecimen::Cow, ESpecimen::Elephant, ESpecimen::Pufferfish, ESpecimen::Octopus, ESpecimen::Whale,
			ESpecimen::Penguin, ESpecimen::Seal, ESpecimen::ElephantSeal })
		{
			if (IsSpecimenUnlocked(Type))
			{
				Animals.Add(Type);
			}
		}
		const ESpecimen Type = Animals[Rng.RandRange(0, Animals.Num() - 1)];
		static const int32 Counts[][2] = { { 3, 5 }, { 2, 3 }, { 1, 2 }, { 0, 0 }, { 3, 5 }, { 2, 3 }, { 1, 2 }, { 3, 5 }, { 2, 3 }, { 1, 2 } };
		static_assert(UE_ARRAY_COUNT(Counts) == int32(ESpecimen::Count), "one census size per specimen");
		FContractNeed Need;
		Need.Type = Type;
		Need.Count = Counts[int32(Type)][Rng.RandRange(0, 1)];
		Contract.Title = FString::Printf(TEXT("Census: %d %s"), Need.Count, Need.Count == 1 ? Specimen(Type).Name : Specimen(Type).Plural);
		static const TCHAR* Flavors[] = {
			TEXT("Quantity is a kind of quality. (Department memo)"),
			TEXT("The Academy has misplaced last year's numbers and would like new ones."),
			TEXT("A committee needs a bar chart. Any bar chart."),
		};
		Contract.Flavor = Flavors[Rng.RandRange(0, UE_ARRAY_COUNT(Flavors) - 1)];
		Contract.Reward = FMath::RoundToDouble(Need.Count * Specimen(Type).Research * 1.2);
		Contract.Needs.Add(Need);
		break;
	}
	case 1:
	{
		Contract.Title = FString::Printf(TEXT("Comparative zoology: 2 %s and %s"), Specimen(Local(ESpecimen::Squirrel)).Plural, *WithArticle(Specimen(Local(ESpecimen::Cow)).Name));
		Contract.Flavor = TEXT("Determine which is more suspicious of us. Preliminary answer: both.");
		Contract.Reward = 100.0;
		FContractNeed Squirrels;
		Squirrels.Type = Local(ESpecimen::Squirrel);
		Squirrels.Count = 2;
		FContractNeed Cow;
		Cow.Type = Local(ESpecimen::Cow);
		Contract.Needs.Add(Squirrels);
		Contract.Needs.Add(Cow);
		break;
	}
	case 2:
	{
		const ESpecimen Big = Local(ESpecimen::Elephant);
		Contract.Title = FString::Printf(TEXT("Large mammal comparison: %s and %s"), *WithArticle(Specimen(Local(ESpecimen::Cow)).Name), *WithArticle(Specimen(Big).Name));
		Contract.Flavor = TEXT("Settle the office bet about which one is 'the big one.'");
		Contract.Reward = Big == ESpecimen::Whale ? 300.0 : 260.0;
		FContractNeed Cow;
		Cow.Type = Local(ESpecimen::Cow);
		FContractNeed Elephant;
		Elephant.Type = Big;
		Contract.Needs.Add(Cow);
		Contract.Needs.Add(Elephant);
		break;
	}
	case 6:
	{
		Contract.Title = TEXT("Tide pool survey: 3 pufferfish and an octopus");
		Contract.Flavor = TEXT("Marine Affairs wants to know who is in charge of the reef. Early answer: whoever has more arms.");
		Contract.Reward = 150.0;
		FContractNeed Puffers;
		Puffers.Type = ESpecimen::Pufferfish;
		Puffers.Count = 3;
		FContractNeed Octopus;
		Octopus.Type = ESpecimen::Octopus;
		Contract.Needs.Add(Puffers);
		Contract.Needs.Add(Octopus);
		break;
	}
	case 3:
	{
		const EOccupation Wanted = RandomOccupation();
		Contract.Title = FString::Printf(TEXT("Acquire a qualified %s"), *FString(Occupation(Wanted).Name).ToLower());
		Contract.Flavor = AcquireFlavor(Wanted);
		Contract.Reward = 170.0;
		FContractNeed Need;
		Need.Type = ESpecimen::Human;
		Need.Occupation = Wanted;
		Contract.Needs.Add(Need);
		break;
	}
	case 4:
	{
		Contract.Title = TEXT("Human resources: 3 different professions");
		Contract.Flavor = TEXT("Your supervisor would like to see some human resources. Your supervisor has misunderstood the term.");
		Contract.Reward = 380.0;
		FContractNeed Need;
		Need.Type = ESpecimen::Human;
		Need.Count = 3;
		Need.bDistinctOccupations = true;
		Contract.Needs.Add(Need);
		break;
	}
	default:
	{
		Contract.Title = TEXT("Rush job: 2 humans, any kind");
		Contract.Flavor = TEXT("Corporate wants results. Corporate has not specified which results.");
		Contract.Reward = 230.0;
		FContractNeed Need;
		Need.Type = ESpecimen::Human;
		Need.Count = 2;
		Contract.Needs.Add(Need);
		break;
	}
	}
	return Contract;
}

bool FAlienIdleSim::AcceptContract(int32 Offer)
{
	if (bHasActiveContract || !Offers.IsValidIndex(Offer))
	{
		return false;
	}
	ActiveContract = Offers[Offer];
	bHasActiveContract = true;
	Offers.RemoveAt(Offer);
	FillContractOffers();
	AddLog(FString::Printf(TEXT("Contract accepted: '%s'. %s Reward %.0f research."), *ActiveContract.Title, *ActiveContract.Flavor, ActiveContract.Reward));
	return true;
}

void FAlienIdleSim::AbandonContract()
{
	if (!bHasActiveContract)
	{
		return;
	}
	bHasActiveContract = false;
	AddLog(FString::Printf(TEXT("Contract '%s' abandoned. The Academy has noted this in a file it keeps for exactly this purpose."), *ActiveContract.Title));
}

void FAlienIdleSim::CountTowardContract(const FSpecimen& Subject)
{
	if (!bHasActiveContract)
	{
		return;
	}
	for (FContractNeed& Need : ActiveContract.Needs)
	{
		if (Need.Accepts(Subject))
		{
			++Need.Progress;
			if (Need.bDistinctOccupations)
			{
				Need.Seen.Add(Subject.Occupation);
			}
			break;
		}
	}
	if (ActiveContract.IsComplete())
	{
		ActiveContract.Reward = FMath::RoundToDouble(ActiveContract.Reward * EarthCondition(Condition).ContractReward
			* (HasCaseDiscovery(CasePell) ? CaseContractMultiplier : 1.0));
		Research += ActiveContract.Reward;
		ResearchEarned += ActiveContract.Reward;
		++ContractsCompleted;
		bHasActiveContract = false;
		AddLog(FString::Printf(TEXT("Contract complete: '%s'. Research +%.0f. The Academy thanks you with a form."), *ActiveContract.Title, ActiveContract.Reward));
	}
}

// ---------------------------------------------------------------------------------------------
// Upgrades

bool FAlienIdleSim::CanBuyShipUpgrade(int32 Index, EShipSystem System, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};

	const FUpgradeDef& Def = ShipSystems[int32(System)];
	const int32 Level = GetShipLevel(Index, System);
	if (Level >= Def.MaxLevel)
	{
		return Fail(TEXT("fully upgraded"));
	}
	if (System == EShipSystem::Tractor && Level >= TractorLevelWithoutDiscovery && !IsDiscovered(EDiscovery::BovineLoadTolerance))
	{
		return Fail(TEXT("needs discovery 'Bovine Load Tolerance' (cow project)"));
	}
	if (System == EShipSystem::Scanner && !IsDiscovered(EDiscovery::TotalRecall) && !HasGrantUpgrade(EGrantUpgrade::ImprovedScanning))
	{
		return Fail(TEXT("needs discovery 'Total Recall' (elephant project)"));
	}
	if (GetShipSlotsUsed(Index) + Def.SlotsPerLevel > GetShipSlots())
	{
		return Fail(FString::Printf(TEXT("no free upgrade slots (%d/%d used)"), GetShipSlotsUsed(Index), GetShipSlots()));
	}
	const double Cost = GetShipUpgradeCost(Index, System);
	if (Research < Cost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f research"), Cost));
	}
	return true;
}

bool FAlienIdleSim::BuyShipUpgrade(int32 Index, EShipSystem System)
{
	if (!CanBuyShipUpgrade(Index, System))
	{
		return false;
	}
	Research -= GetShipUpgradeCost(Index, System);
	const int32 NewLevel = ++Ufos[Index].Levels[int32(System)];
	AddLog(FString::Printf(TEXT("'%s': %s"), *Ufos[Index].Name, *GetShipUpgradeFlavor(System, NewLevel)));
	return true;
}

double FAlienIdleSim::GetShipUpgradeCost(int32 Index, EShipSystem System) const
{
	return FMath::RoundToDouble(UpgradeCost(ShipSystems[int32(System)], GetShipLevel(Index, System)) * (Location == ELocation::Antarctic ? AntarcticUpgradeCostMultiplier : 1.0));
}

int32 FAlienIdleSim::GetShipSlotsUsed(int32 Index) const
{
	int32 Used = 0;
	for (int32 i = 0; i < int32(EShipSystem::Count); ++i)
	{
		Used += Ufos[Index].Levels[i] * ShipSystems[i].SlotsPerLevel;
	}
	return Used;
}

bool FAlienIdleSim::CanRefitShip(int32 Index, FString* OutReason) const
{
	auto Fail = [OutReason](const TCHAR* Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	if (GetShipSlotsUsed(Index) == 0)
	{
		return Fail(TEXT("nothing to strip"));
	}
	if (Ufos[Index].State != EUfoState::Docked)
	{
		return Fail(TEXT("ship must be docked"));
	}
	return true;
}

double FAlienIdleSim::GetRefitRefund(int32 Index) const
{
	double Spent = 0.0;
	for (int32 i = 0; i < int32(EShipSystem::Count); ++i)
	{
		for (int32 Level = 0; Level < Ufos[Index].Levels[i]; ++Level)
		{
			Spent += UpgradeCost(ShipSystems[i], Level);
		}
	}
	return FMath::FloorToDouble(Spent * RefitRefundFraction);
}

bool FAlienIdleSim::RefitShip(int32 Index)
{
	if (!CanRefitShip(Index))
	{
		return false;
	}
	const double Refund = GetRefitRefund(Index);
	Research += Refund;
	FUfo& Ufo = Ufos[Index];
	for (int32& Level : Ufo.Levels)
	{
		Level = 0;
	}
	AddLog(FString::Printf(TEXT("'%s' stripped for refit. Accounting refunded %.0f research and kept the rest as a 'handling fee.'"), *Ufo.Name, Refund));
	return true;
}

bool FAlienIdleSim::CanBuyBaseUpgrade(EBaseUpgrade Upgrade, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	const FUpgradeDef& Def = BaseUpgrades[int32(Upgrade)];
	const int32 Level = GetBaseLevel(Upgrade);
	if (!IsBaseUpgradeAvailable(Upgrade))
	{
		return Fail(Upgrade == EBaseUpgrade::SonarArray ? TEXT("needs discovery 'Whale Song Transcripts' (Oceanic)")
			: Upgrade == EBaseUpgrade::InkScreens ? TEXT("needs discovery 'Ink Diplomacy' (Oceanic)")
			: Upgrade == EBaseUpgrade::InsulatedLabs ? TEXT("needs discovery 'Huddle Theory' (Antarctic)")
			: Upgrade == EBaseUpgrade::CryoPods ? TEXT("needs discovery 'Blubber Economics' (Antarctic)")
			: Upgrade == EBaseUpgrade::GeothermalTaps || Upgrade == EBaseUpgrade::CryoVault ? TEXT("Antarctic only") : TEXT("Oceanic only"));
	}
	if (Level >= Def.MaxLevel)
	{
		return Fail(TEXT("fully upgraded"));
	}
	if (Upgrade == EBaseUpgrade::Containment && Location == ELocation::Oceanic && Level >= OceanicContainmentMaxLevel)
	{
		return Fail(TEXT("no room down here: habitat capped at II. Deep-Water Tanks add space, at a cost to the hull"));
	}
	if (Upgrade == EBaseUpgrade::Containment && Level >= ContainmentLevelWithoutDiscovery && !IsDiscovered(EDiscovery::UnscheduledDepartures))
	{
		return Fail(TEXT("needs discovery 'Unscheduled Departures' (squirrel project)"));
	}
	const int32 MaxUfos = AlienIdleTuning::Thesis(Thesis).MaxUfos;
	if (Upgrade == EBaseUpgrade::Hangar && MaxUfos > 0 && Ufos.Num() >= MaxUfos)
	{
		return Fail(FString::Printf(TEXT("thesis rule: at most %d UFO%s"), MaxUfos, MaxUfos == 1 ? TEXT("") : TEXT("s")));
	}
	if ((Upgrade == EBaseUpgrade::Chambers || Upgrade == EBaseUpgrade::Hangar) && Level >= 1 && !IsDiscovered(EDiscovery::PachydermLogistics))
	{
		return Fail(TEXT("needs discovery 'Pachyderm Logistics' (elephant project)"));
	}
	const double Cost = GetBaseUpgradeCost(Upgrade);
	if (Research < Cost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f research"), Cost));
	}
	return true;
}

bool FAlienIdleSim::BuyBaseUpgrade(EBaseUpgrade Upgrade)
{
	if (!CanBuyBaseUpgrade(Upgrade))
	{
		return false;
	}
	Research -= GetBaseUpgradeCost(Upgrade);
	const int32 NewLevel = ++BaseLevels[int32(Upgrade)];
	switch (Upgrade)
	{
	case EBaseUpgrade::Chambers: Chambers.AddDefaulted(1); break;
	case EBaseUpgrade::Hangar: AddUfo(); break;
	default: break;
	}
	AddLog(GetBaseUpgradeFlavor(Upgrade, NewLevel));
	return true;
}

double FAlienIdleSim::GetBaseUpgradeCost(EBaseUpgrade Upgrade) const
{
	const bool bExclusive = Upgrade == EBaseUpgrade::GeothermalTaps || Upgrade == EBaseUpgrade::CryoVault;
	return FMath::RoundToDouble(UpgradeCost(BaseUpgrades[int32(Upgrade)], GetBaseLevel(Upgrade))
		* (Upgrade == EBaseUpgrade::Power && Location == ELocation::Oceanic ? OceanicPowerCostMultiplier : 1.0)
		* (Location == ELocation::Antarctic && !bExclusive ? AntarcticUpgradeCostMultiplier : 1.0));
}

// ---------------------------------------------------------------------------------------------
// Queries

double FAlienIdleSim::GetEnergyCap() const
{
	return EnergyCap + PowerCapPerLevel * GetBaseLevel(EBaseUpgrade::Power) + (IsProjectDone(EDeptProject::UnusualPower) ? 60.0 : 0.0);
}

double FAlienIdleSim::GetEnergyPerSecond() const
{
	const FAssignment Power{ EStation::Power, 0 };
	double Rate = EnergyPerSecond + PowerEnergyPerLevel * GetBaseLevel(EBaseUpgrade::Power) + (IsProjectDone(EDeptProject::UnusualPower) ? 3.0 : 0.0);
	Rate *= IsStationStaffed(Power) ? 1.0 + PowerOutputPerBonus * GetStationBonus(Power) : UnstaffedPowerMultiplier;
	Rate += TidalEnergyPerLevel * GetBaseLevel(EBaseUpgrade::TidalTurbines) * GetTideFactor();
	Rate -= CryoEnergyPerLevel * (GetBaseLevel(EBaseUpgrade::CryoVault) + GetBaseLevel(EBaseUpgrade::CryoPods));
	return Incident.Type == EIncident::Brownout ? Rate * 0.25 : Rate;
}

int32 FAlienIdleSim::GetTractorStrength(int32 Index) const
{
	return TractorStrength[GetShipLevel(Index, EShipSystem::Tractor)] + (IsProjectDone(EDeptProject::HeavyTractorArray) ? 3 : 0);
}

int32 FAlienIdleSim::GetCargoCapacity(int32 Index) const
{
	return StartCargo + GetShipLevel(Index, EShipSystem::Cargo);
}

float FAlienIdleSim::GetFlightSeconds(int32 Index, ESpecimen Type) const
{
	const int32 RegionIndex = ActiveRegion(Index);
	return Specimen(Type).FlightSeconds * FMath::Pow(ThrustersFlightMultiplier, float(GetShipLevel(Index, EShipSystem::Thrusters)))
		* (Location == ELocation::Metropolis && Type == ESpecimen::Human ? MetropolisHumanFlightMultiplier : 1.0f)
		* GetRegionDef(RegionIndex).Flight * (Regions[RegionIndex].Active[int32(EResponse::Radar)] ? 1.3f : 1.0f)
		* (HasHeat() ? 1.0f + ColdFlightPenalty * (1.0f - GetHangarWarmth()) : 1.0f)
		/ (1.0f + UfoSpeedPerBonus * GetStationBonus({ EStation::Ufo, Index }));
}

float FAlienIdleSim::GetCaptureSeconds(int32 Index, ESpecimen Type, bool bScanning) const
{
	float Seconds = Specimen(Type).CaptureSeconds * (1.0f + CargoCapturePerLevel * GetShipLevel(Index, EShipSystem::Cargo));
	const int32 RegionIndex = ActiveRegion(Index);
	if (IsRegionIndexOnAlert(RegionIndex))
	{
		Seconds *= AlertCaptureMultiplier;
	}
	if (Type != ESpecimen::Human && Regions[RegionIndex].Active[int32(EResponse::NightPatrols)])
	{
		Seconds *= 1.6f;
	}
	if (IsSwept(RegionIndex, Type))
	{
		Seconds *= 1.5f; // queueing behind the Bureau
	}
	if (Type == ESpecimen::Human && Regions[RegionIndex].Active[int32(EResponse::Investigators)])
	{
		Seconds *= 1.3f;
	}
	const int8 Sought = Ufos[Index].State == EUfoState::Docked ? Ufos[Index].CaseTarget : Ufos[Index].TripCase;
	if (bScanning && Type == ESpecimen::Human && Sought >= 0)
	{
		// Harder to fool: a subject who has been taken before hides better every time.
		Seconds *= 1.0f + CaseCapturePerEncounter * FMath::Min(Career.Cases[Sought].Encounters, CaseMaxEncounterEffect);
	}
	if (bScanning)
	{
		Seconds += IsProjectDone(EDeptProject::AdvancedScanners) || GetBaseLevel(EBaseUpgrade::SonarArray) > 0 ? 0.0f
			: ScannerSeconds[GetShipLevel(Index, EShipSystem::Scanner)] * (HasGrantUpgrade(EGrantUpgrade::ImprovedScanning) ? 0.5f : 1.0f);
	}
	return Seconds / (1.0f + UfoSpeedPerBonus * GetStationBonus({ EStation::Ufo, Index }));
}

double FAlienIdleSim::GetLaunchEnergy(int32 Index, ESpecimen Type) const
{
	const double Penalty = TractorLaunchEnergyPerLevel * (IsProjectDone(EDeptProject::HeavyTractorArray) ? 0.5 : 1.0);
	double Cost = Specimen(Type).LaunchEnergy * (1.0 + Penalty * GetShipLevel(Index, EShipSystem::Tractor));
	if (Incident.Type == EIncident::CeaseAndDesist)
	{
		Cost *= 1.5;
	}
	const int32 RegionIndex = Ufos.IsValidIndex(Index) ? ActiveRegion(Index) : 0;
	if (IsSurfaceRegion(RegionIndex))
	{
		Cost *= SurfaceLaunchEnergyMultiplier;
	}
	else if (IsUnderwaterRegion(RegionIndex) && Career.LocationPerk[int32(ELocation::Oceanic)])
	{
		Cost *= 0.5;
	}
	if (GetBaseLevel(EBaseUpgrade::InkScreens) > 0)
	{
		Cost += 3.0;
	}
	if (HasHeat())
	{
		Cost *= 1.0 + ColdLaunchEnergyPenalty * (1.0 - GetHangarWarmth());
	}
	return FMath::RoundToDouble(Cost);
}

double FAlienIdleSim::GetSuspicionMultiplier(int32 Index) const
{
	return FMath::Pow(StealthSuspicionMultiplier, double(GetShipLevel(Index, EShipSystem::Stealth))) * (GetBaseLevel(EBaseUpgrade::InkScreens) > 0 ? 0.75 : 1.0);
}

FString FAlienIdleSim::GetShipRole(int32 Index) const
{
	// Ties go to the rarer specialty.
	static const EShipSystem Priority[] = { EShipSystem::Scanner, EShipSystem::Stealth, EShipSystem::Tractor, EShipSystem::Cargo, EShipSystem::Thrusters };
	int32 BestSlots = 0;
	EShipSystem Best = EShipSystem::Count;
	for (EShipSystem System : Priority)
	{
		const int32 Slots = GetShipLevel(Index, System) * ShipSystems[int32(System)].SlotsPerLevel;
		if (Slots > BestSlots)
		{
			BestSlots = Slots;
			Best = System;
		}
	}
	switch (Best)
	{
	case EShipSystem::Scanner: return TEXT("Rare-specimen hunter");
	case EShipSystem::Stealth: return TEXT("Discreet abductor");
	case EShipSystem::Tractor: return TEXT("Heavy lifter");
	case EShipSystem::Cargo: return TEXT("Bulk collector");
	case EShipSystem::Thrusters: return TEXT("Courier");
	default: return TEXT("Generalist");
	}
}

float FAlienIdleSim::GetUfoStateProgress(int32 Index) const
{
	const FUfo& Ufo = Ufos[Index];
	return Ufo.StateDuration > 0.0f ? FMath::Clamp(Ufo.StateTime / Ufo.StateDuration, 0.0f, 1.0f) : 0.0f;
}

FString FAlienIdleSim::GetUfoStatusText(int32 Index) const
{
	const FUfo& Ufo = Ufos[Index];
	const FSpecimenDef& Def = Specimen(Ufo.TripTarget);
	const float Left = FMath::Max(0.0f, Ufo.StateDuration - Ufo.StateTime);
	switch (Ufo.State)
	{
	case EUfoState::Docked:
	{
		FString Reason;
		if (CanLaunch(Index, &Reason))
		{
			if (Ufo.bAutoLaunch && IsAutomationUnlocked())
			{
				return IsHeldForContainment(Index) ? TEXT("Docked. Holding: containment is full, collection paused.") : TEXT("Docked. Auto-launching.");
			}
			return TEXT("Docked. Ready to launch.");
		}
		return FString::Printf(TEXT("Docked. Waiting: %s."), *Reason);
	}
	case EUfoState::FlyingOut: return FString::Printf(TEXT("Flying to %s (%.1f s)"), *GetRegionNameAt(Ufo.TripRegion), Left);
	case EUfoState::Capturing:
	{
		const FString Scan = Ufo.TripOccupation != EOccupation::None
			? FString::Printf(TEXT("Scanning for %s among the "), *WithArticle(FString(Occupation(Ufo.TripOccupation).Name).ToLower())) : FString(TEXT("Chasing "));
		return FString::Printf(TEXT("%s%s%s (%.1f s)"), *Scan, Def.Plural, IsRegionIndexOnAlert(Ufo.TripRegion) ? TEXT(", region on alert") : TEXT(""), Left);
	}
	case EUfoState::Returning: return FString::Printf(TEXT("Returning with %d %s (%.1f s)"), Ufo.Cargo.Num(), Ufo.Cargo.Num() == 1 ? Def.Name : Def.Plural, Left);
	case EUfoState::WaitingToUnload:
		return FString::Printf(TEXT("Waiting at dock: containment full (%d/%d units, %d still aboard)."),
			GetContainmentUnitsUsed(), GetContainmentCapacity(), Ufo.Cargo.Num());
	}
	return FString();
}

EDiscovery FAlienIdleSim::GetUnlockDiscovery(ESpecimen Type) const
{
	if (Type == ESpecimen::Human && Location == ELocation::Metropolis)
	{
		return EDiscovery::HerdBehaviour;
	}
	if (Type == ESpecimen::Human && Location == ELocation::Oceanic)
	{
		return EDiscovery::InkDiplomacy;
	}
	if (Location == ELocation::Antarctic && (Type == ESpecimen::Seal || Type == ESpecimen::ElephantSeal))
	{
		return Type == ESpecimen::Seal ? EDiscovery::AcornAmnesia : EDiscovery::HerdBehaviour;
	}
	return Specimen(Type).UnlockedBy;
}

bool FAlienIdleSim::IsSpecimenUnlocked(ESpecimen Type) const
{
	if (!IsSpecimenAtLocation(Type))
	{
		return false;
	}
	const EDiscovery UnlockedBy = GetUnlockDiscovery(Type);
	return DevUnlocked[int32(Type)] || UnlockedBy == EDiscovery::None || IsDiscovered(UnlockedBy);
}

FString FAlienIdleSim::GetUnlockRequirementText(ESpecimen Type) const
{
	const FSpecimenDef& Def = Specimen(Type);
	if (!IsSpecimenAtLocation(Type))
	{
		return FString::Printf(TEXT("no %s near %s"), Def.Plural, LocationNames[int32(Location)]);
	}
	const EDiscovery UnlockedBy = GetUnlockDiscovery(Type);
	if (UnlockedBy == EDiscovery::None)
	{
		return FString();
	}
	const FDiscoveryDef& Needed = Discovery(UnlockedBy);
	return FString::Printf(TEXT("%s not approved: needs discovery '%s' (%s project)"), Def.Plural, Needed.Name, Specimen(GetDiscoverySpecimen(UnlockedBy)).Name);
}

int32 FAlienIdleSim::GetContainmentUnitsUsed() const
{
	int32 Units = 0;
	for (const FSpecimen& Held : Containment)
	{
		Units += GetUnits(Held);
	}
	return Units;
}

int32 FAlienIdleSim::GetContainmentCapacity() const
{
	const FAssignment Warden{ EStation::Containment, 0 };
	const int32 StaffUnits = IsStationStaffed(Warden) ? FMath::FloorToInt32(GetStationBonus(Warden) * ContainmentUnitsPerBonus) : 0;
	return StartContainmentUnits + ContainmentUnitsPerLevel * GetBaseLevel(EBaseUpgrade::Containment) + StaffUnits
		+ (IsProjectDone(EDeptProject::ReinforcedHabitats) ? 4 : 0)
		+ (Incident.Type == EIncident::InventoryReorganized ? 3 : 0)
		+ DeepTankUnits * GetBaseLevel(EBaseUpgrade::DeepTanks)
		+ CryoVaultUnits * GetBaseLevel(EBaseUpgrade::CryoVault) + CryoPodUnits * GetBaseLevel(EBaseUpgrade::CryoPods);
}

FString FAlienIdleSim::GetChamberStatusText(int32 Index) const
{
	const FLabChamber& Chamber = Chambers[Index];
	if (!IsStationStaffed({ EStation::Chamber, Index }))
	{
		return Chamber.bRunning ? TEXT("Paused: no scientist. Management considers this sustainable.") : TEXT("Unstaffed. Management considers this sustainable.");
	}
	if (IsInMeeting())
	{
		return FString::Printf(TEXT("In a mandatory meeting (%.0f s)"), MeetingRemaining);
	}
	if (Chamber.bRunning)
	{
		const float Speed = FMath::Max(GetLabSpeed() * (1.0f + LabSpeedPerBonus * GetStationBonus({ EStation::Chamber, Index })), 0.01f);
		const FString Who = Chamber.Specimen.Case >= 0 ? GetSpecimenLabel(Chamber.Specimen) : WithArticle(GetSpecimenLabel(Chamber.Specimen));
		const float Left = FMath::Max(0.0f, Chamber.Duration - Chamber.Time) / Speed;
		switch (Chamber.Experiment.Kind)
		{
		case EExperimentKind::Discovery:
			return FString::Printf(TEXT("Discovery '%s' on %s (%.1f s)"), Discovery(Chamber.Experiment.Discovery).Name, *Who, Left);
		case EExperimentKind::Extended:
			return FString::Printf(TEXT("Extended study of %s (%.1f s)"), *Who, Left);
		case EExperimentKind::CaseStudy:
			if (Chamber.Experiment.CaseStep >= PeerReviewStep)
			{
				return FString::Printf(TEXT("Peer Review: %s and %s (%.1f s)"), *Who, *GetSpecimenLabel(Chamber.Partner), Left);
			}
			return FString::Printf(TEXT("Case study '%s' on %s (%.1f s)"), CaseSubjects[Chamber.Specimen.Case].Steps[Chamber.Experiment.CaseStep].Name, *Who, Left);
		default:
			return FString::Printf(TEXT("Routine study of %s (%.1f s)"), *Who, Left);
		}
	}
	const int32 Pick = PickNextSpecimen();
	if (Pick == INDEX_NONE)
	{
		return TEXT("Idle. Waiting for a specimen.");
	}
	const FSpecimen& Next = Containment[Pick];
	return FString::Printf(TEXT("Idle. Waiting for energy: next %s needs %.0f."), *GetSpecimenLabel(Next), GetExperimentEnergy(Next, ResolveExperiment(Next)));
}

void FAlienIdleSim::AddResult(const FString& Text)
{
	AddLog(Text);
	Results.Add({ Elapsed, Text });
	if (Results.Num() > MaxLogEntries)
	{
		Results.RemoveAt(0, Results.Num() - MaxLogEntries);
	}
}

void FAlienIdleSim::AddLog(const FString& Text)
{
	Log.Add({ Elapsed, Text });
	if (Log.Num() > MaxLogEntries)
	{
		Log.RemoveAt(0, Log.Num() - MaxLogEntries);
	}
}

// ---------------------------------------------------------------------------------------------
// Text

FString FAlienIdleSim::GetSpecimenLabel(const FSpecimen& Subject)
{
	if (Subject.Case >= 0 && Subject.Case < NumCaseSubjects)
	{
		return CaseSubjects[Subject.Case].Name;
	}
	return Subject.Occupation != EOccupation::None ? FString(Occupation(Subject.Occupation).Name).ToLower() : FString(Specimen(Subject.Type).Name);
}

FString FAlienIdleSim::GetShipUpgradeEffectText(EShipSystem System)
{
	switch (System)
	{
	case EShipSystem::Thrusters: return TEXT("Flights 15% shorter.");
	case EShipSystem::Tractor: return TEXT("Lifts heavier specimens. Tradeoff: +25% launch energy.");
	case EShipSystem::Cargo: return TEXT("+1 specimen per trip. Tradeoff: +20% loading time.");
	case EShipSystem::Stealth: return TEXT("Suspicion per capture x0.6. Tradeoff: takes 2 slots that could hold cargo.");
	case EShipSystem::Scanner: return TEXT("Targets the occupation your contract or project needs. Tradeoff: adds scan time.");
	default: return FString();
	}
}

FString FAlienIdleSim::GetShipUpgradeFlavor(EShipSystem System, int32 NewLevel)
{
	static const TCHAR* Thrusters[] = {
		TEXT("Thrusters Mk 2: Duct tape replaced with premium duct tape."),
		TEXT("Thrusters Mk 3: Exhaust now meets the emissions standard of a planet we do not live on."),
		TEXT("Thrusters Mk 4: The ship now outpaces the pilot's regrets."),
		TEXT("Thrusters Mk 5: Top speed limited only by the warranty, which has expired."),
	};
	static const TCHAR* Tractor[] = {
		TEXT("Tractor Beam II: Now lifts cows. Cows have not been consulted."),
		TEXT("Tractor Beam III: Rated for one large mammal or several disappointed interns."),
		TEXT("Tractor Beam IV: Elephant-certified. The beam hums a little tune. Nobody knows which tune."),
		TEXT("Tractor Beam V: Can lift anything except staff morale."),
	};
	static const TCHAR* Cargo[] = {
		TEXT("Cargo Bay II: Occupancy limits have been revised following a successful appeal from Accounting."),
		TEXT("Cargo Bay III: Passenger comfort package removed to accommodate additional passengers."),
		TEXT("Cargo Bay IV: The seating chart has been abandoned. Specimens are encouraged to network."),
		TEXT("Cargo Bay V: Legally a conference room."),
	};
	static const TCHAR* Stealth[] = {
		TEXT("Stealth Panels I: The hull is now painted the color of 'nothing to see here.'"),
		TEXT("Stealth Panels II: Witnesses now describe the ship as 'probably a plane, or a feeling.'"),
		TEXT("Stealth Panels III: The ship is so discreet that Facilities filed it as missing."),
	};
	static const TCHAR* Scanner[] = {
		TEXT("Trait Scanner I: Reads occupations from orbit. Accuracy confirmed by one lawyer, who would like to discuss terms."),
		TEXT("Trait Scanner II: Scan time halved after the scanner was told it was being evaluated."),
	};
	auto Pick = [NewLevel](const TCHAR* const* Lines, int32 Num) { return FString(Lines[FMath::Clamp(NewLevel - 1, 0, Num - 1)]); };
	switch (System)
	{
	case EShipSystem::Thrusters: return Pick(Thrusters, UE_ARRAY_COUNT(Thrusters));
	case EShipSystem::Tractor: return Pick(Tractor, UE_ARRAY_COUNT(Tractor));
	case EShipSystem::Cargo: return Pick(Cargo, UE_ARRAY_COUNT(Cargo));
	case EShipSystem::Stealth: return Pick(Stealth, UE_ARRAY_COUNT(Stealth));
	case EShipSystem::Scanner: return Pick(Scanner, UE_ARRAY_COUNT(Scanner));
	default: return FString();
	}
}

FString FAlienIdleSim::GetBaseUpgradeEffectText(EBaseUpgrade Upgrade)
{
	switch (Upgrade)
	{
	case EBaseUpgrade::Containment: return TEXT("+3 containment units. Cows take 2, elephants 4, magicians 2.");
	case EBaseUpgrade::Chambers: return TEXT("+1 experiment at a time.");
	case EBaseUpgrade::Power: return TEXT("+1 energy/s and +30 energy storage.");
	case EBaseUpgrade::Hangar: return TEXT("+1 UFO with its own upgrade slots.");
	case EBaseUpgrade::Camouflage: return TEXT("Launches draw half as much radar attention.");
	case EBaseUpgrade::Cleanup: return TEXT("Rival breaches cause half the suspicion and support-group pressure.");
	case EBaseUpgrade::TidalTurbines: return TEXT("+3 energy/s at high tide, nothing at low tide (4-minute tides). Loads the hull a little.");
	case EBaseUpgrade::DeepTanks: return TEXT("+5 containment units (fits whales). Each level loads the hull: more Pressure Control needed.");
	case EBaseUpgrade::SonarArray: return TEXT("Scanning takes no time at all. Launches draw 50% more radar attention.");
	case EBaseUpgrade::InkScreens: return TEXT("Every trip raises 25% less suspicion. Launches cost +3 energy.");
	case EBaseUpgrade::GeothermalTaps: return TEXT("+3 heat. The steam plume is visible: launches draw 25% more radar attention per tap.");
	case EBaseUpgrade::CryoVault: return TEXT("+4 containment units; escapes halved; uses 0.5 energy/s per level.");
	case EBaseUpgrade::InsulatedLabs: return TEXT("Experiments use 30% less energy. Labs 5% slower: the scientists keep napping.");
	case EBaseUpgrade::CryoPods: return TEXT("+3 units; at any base, held specimens become preserved samples (up to +50% research after 2 min). Uses 0.5 energy/s.");
	default: return FString();
	}
}

FString FAlienIdleSim::GetBaseUpgradeFlavor(EBaseUpgrade Upgrade, int32 NewLevel)
{
	switch (Upgrade)
	{
	case EBaseUpgrade::Containment:
		return NewLevel % 2 == 1
			? TEXT("Containment habitat expanded. Overcrowding has been rebranded as 'community.'")
			: TEXT("Containment habitat expanded. Each cell now includes a motivational poster about patience.");
	case EBaseUpgrade::Chambers:
		return NewLevel == 1
			? TEXT("Experiment Chamber II: A second table, so findings can now contradict each other in parallel.")
			: TEXT("Experiment Chamber III: Three experiments at once. The ethics committee asked for a fourth so it could watch.");
	case EBaseUpgrade::Power:
		return TEXT("Power generation upgraded. Brownouts are now scheduled, which staff find reassuring.");
	case EBaseUpgrade::Cleanup:
		return TEXT("Cleanup Crew hired. Their motto: 'It was like that when we got here.'");
	case EBaseUpgrade::Camouflage:
		return TEXT("Radar Camouflage researched. On radar, every UFO now looks like a flock of geese with excellent formation discipline.");
	case EBaseUpgrade::Hangar:
		return NewLevel == 1
			? TEXT("Hangar bay opened. New UFO 'Second Opinion' delivered with someone else's lunch still inside.")
			: TEXT("Hangar bay opened. 'Plausible Deniability' joins the fleet. Nobody remembers ordering it.");
	case EBaseUpgrade::TidalTurbines:
		return TEXT("Tidal turbines installed. Power now depends on the moon, which has never once returned our calls.");
	case EBaseUpgrade::DeepTanks:
		return TEXT("Deep-Water Tank flooded. The hull groans. Facilities describe the groan as 'structural feedback.'");
	case EBaseUpgrade::SonarArray:
		return TEXT("Sonar Array online. It reads job titles through walls and pings like a disappointed parent.");
	case EBaseUpgrade::InkScreens:
		return TEXT("Ink Screen Launchers fitted. Every departure now leaves a small cloud of plausible deniability.");
	case EBaseUpgrade::GeothermalTaps:
		return TEXT("Geothermal tap drilled. The base is warmer, and a column of steam now announces our position to anyone who looks up.");
	case EBaseUpgrade::CryoVault:
		return TEXT("Cryo Vault commissioned. Specimens inside report feeling 'preserved', which is both accurate and not a feeling.");
	case EBaseUpgrade::InsulatedLabs:
		return TEXT("Labs insulated with penguin-inspired cladding. Energy bills down; afternoon naps up.");
	case EBaseUpgrade::CryoPods:
		return TEXT("Cryo Pods installed. The specimens are chilled, the staff are chilled, the budget is not.");
	default:
		return FString();
	}
}

// ---------------------------------------------------------------------------------------------
// Crew (Alien Resources)

namespace
{
	const TCHAR* CrewNames[] = {
		TEXT("Zorp"), TEXT("Glimmox"), TEXT("Mxyl"), TEXT("Bleep-7"), TEXT("Ondraxx"), TEXT("Quillo"), TEXT("Vex"), TEXT("Thrrp"),
		TEXT("Nub"), TEXT("Kalgor"), TEXT("Pim"), TEXT("Ursk"), TEXT("Dree"), TEXT("Oglo"), TEXT("Fennimax"), TEXT("Sprool"),
	};
}

void FAlienIdleSim::AddCrew(ESpecialty Specialty, const FAssignment& Where)
{
	FCrewMember& Member = Crew.AddDefaulted_GetRef();
	const int32 Number = Crew.Num() - 1;
	const int32 NameCount = UE_ARRAY_COUNT(CrewNames);
	Member.Name = Number < NameCount ? FString(CrewNames[Number]) : FString::Printf(TEXT("%s %d"), CrewNames[Number % NameCount], Number / NameCount + 1);
	Member.Specialty = Specialty;
	Member.Assignment = Where;
}

const TCHAR* FAlienIdleSim::GetSpecialtyName(ESpecialty Specialty)
{
	switch (Specialty)
	{
	case ESpecialty::Pilot: return TEXT("Pilot");
	case ESpecialty::Scientist: return TEXT("Scientist");
	default: return TEXT("Technician");
	}
}

ESpecialty FAlienIdleSim::GetStationSpecialty(EStation Station)
{
	switch (Station)
	{
	case EStation::Ufo: return ESpecialty::Pilot;
	case EStation::Chamber: return ESpecialty::Scientist;
	default: return ESpecialty::Technician;
	}
}

TArray<FAssignment> FAlienIdleSim::GetAllStations() const
{
	TArray<FAssignment> Stations;
	for (int32 u = 0; u < Ufos.Num(); ++u)
	{
		Stations.Add({ EStation::Ufo, u });
	}
	for (int32 c = 0; c < Chambers.Num(); ++c)
	{
		Stations.Add({ EStation::Chamber, c });
	}
	Stations.Add({ EStation::Containment, 0 });
	Stations.Add({ EStation::Power, 0 });
	Stations.Add({ EStation::MemoryWipe, 0 });
	if (HasHull())
	{
		Stations.Add({ EStation::Pressure, 0 });
	}
	if (HasHeat())
	{
		Stations.Add({ EStation::Boiler, 0 });
	}
	return Stations;
}

FString FAlienIdleSim::GetStationName(const FAssignment& Where) const
{
	switch (Where.Station)
	{
	case EStation::Ufo: return FString::Printf(TEXT("UFO '%s'"), Ufos.IsValidIndex(Where.Index) ? *Ufos[Where.Index].Name : TEXT("?"));
	case EStation::Chamber: return FString::Printf(TEXT("Lab chamber %d"), Where.Index + 1);
	case EStation::Containment: return TEXT("Containment warden");
	case EStation::Power: return TEXT("Power station");
	case EStation::MemoryWipe: return TEXT("Memory-wipe station");
	case EStation::Pressure: return TEXT("Pressure Control");
	case EStation::Boiler: return TEXT("Boiler Room");
	default: return TEXT("Unassigned");
	}
}

int32 FAlienIdleSim::GetStationCrew(const FAssignment& Where) const
{
	int32 Count = 0;
	for (int32 i = 0; i < Crew.Num(); ++i)
	{
		Count += (IsOnDuty(i) && Crew[i].Assignment.Station != EStation::None && Crew[i].Assignment == Where) ? 1 : 0;
	}
	return Count;
}

float FAlienIdleSim::GetStationBonus(const FAssignment& Where) const
{
	const ESpecialty Wanted = GetStationSpecialty(Where.Station);
	float Skill = 0.0f;
	int32 Count = 0;
	for (int32 i = 0; i < Crew.Num(); ++i)
	{
		const FCrewMember& Member = Crew[i];
		if (!IsOnDuty(i) || Member.Assignment.Station == EStation::None || !(Member.Assignment == Where))
		{
			continue;
		}
		++Count;
		Skill += 1.0f;
		if (Member.Specialty == Wanted)
		{
			Skill += SpecialistBonus + TrainingBonusPerLevel * Member.Training;
		}
	}
	return Count > 0 ? FMath::Max(0.0f, Skill - 1.0f) : 0.0f;
}

FString FAlienIdleSim::GetStationStaffingText(const FAssignment& Where) const
{
	const int32 Count = GetStationCrew(Where);
	if (Count == 0)
	{
		return FString::Printf(TEXT("%s: 0/%d crew, needs 1. Management considers this sustainable."), *GetStationName(Where), MaxCrewPerStation);
	}
	const float Bonus = GetStationBonus(Where);
	FString Effect;
	switch (Where.Station)
	{
	case EStation::Ufo: Effect = FString::Printf(TEXT("expeditions +%.0f%% faster"), 100.0f * UfoSpeedPerBonus * Bonus); break;
	case EStation::Chamber: Effect = FString::Printf(TEXT("experiments +%.0f%% faster"), 100.0f * LabSpeedPerBonus * Bonus); break;
	case EStation::Containment: Effect = FString::Printf(TEXT("+%d units, escapes x%.2f"), FMath::FloorToInt32(Bonus * ContainmentUnitsPerBonus), 1.0f / (1.0f + Bonus)); break;
	case EStation::Power: Effect = FString::Printf(TEXT("energy +%.0f%%"), 100.0f * PowerOutputPerBonus * Bonus); break;
	case EStation::MemoryWipe: Effect = FString::Printf(TEXT("-%.1f suspicion per return"), WipeSuspicionPerBonus * Bonus); break;
	default: break;
	}
	return FString::Printf(TEXT("%s: %d/%d crew, %s"), *GetStationName(Where), Count, MaxCrewPerStation, *Effect);
}

int32 FAlienIdleSim::GetIdleCrew() const
{
	int32 Idle = 0;
	for (const FCrewMember& Member : Crew)
	{
		Idle += (Member.Assignment.Station == EStation::None && Member.TrainingRemaining <= 0.0f) ? 1 : 0;
	}
	return Idle;
}

double FAlienIdleSim::GetRecruitCost() const
{
	return FMath::RoundToDouble(RecruitBaseCost * FMath::Pow(RecruitCostGrowth, double(Recruited)) * AlienIdleTuning::Thesis(Thesis).RecruitCostMultiplier
		* (Location == ELocation::Antarctic ? AntarcticRecruitCostMultiplier : 1.0));
}

bool FAlienIdleSim::CanRecruit(FString* OutReason) const
{
	const double Cost = GetRecruitCost();
	if (Research < Cost)
	{
		if (OutReason)
		{
			*OutReason = FString::Printf(TEXT("needs %.0f research"), Cost);
		}
		return false;
	}
	return true;
}

bool FAlienIdleSim::Recruit()
{
	if (!CanRecruit())
	{
		return false;
	}
	Research -= GetRecruitCost();
	++Recruited;
	const ESpecialty Specialty = ESpecialty(Rng.RandRange(0, int32(ESpecialty::Count) - 1));
	AddCrew(Specialty, FAssignment());
	// Rare research crews: only the dedicated volunteer for the ice (and the Polar Crew Program trains everyone).
	if (Location == ELocation::Antarctic || Career.LocationPerk[int32(ELocation::Antarctic)])
	{
		Crew.Last().Training = Location == ELocation::Antarctic && Rng.FRand() < 0.25f ? 2 : 1;
		AddLog(FString::Printf(TEXT("%s arrives already trained (level %d). Nobody comes this far by accident."), *Crew.Last().Name, Crew.Last().Training));
	}
	static const TCHAR* Arrivals[] = {
		TEXT("arrived with a CV written entirely in the passive voice."),
		TEXT("arrived early, which Alien Resources has flagged as suspicious."),
		TEXT("arrived and asked where the snacks are. There are no snacks."),
		TEXT("arrived with three references, all of them from itself."),
	};
	AddLog(FString::Printf(TEXT("Request for Additional Personnel approved. %s (%s) %s"), *Crew.Last().Name, GetSpecialtyName(Specialty),
		Arrivals[Recruited % UE_ARRAY_COUNT(Arrivals)]));
	if (bAutoAssign)
	{
		AutoAssignIdle();
	}
	return true;
}

double FAlienIdleSim::GetTrainCost(int32 Member) const
{
	return FMath::RoundToDouble(TrainBaseCost * FMath::Pow(TrainCostGrowth, double(Crew[Member].Training)));
}

bool FAlienIdleSim::CanTrain(int32 Member, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	const FCrewMember& Alien = Crew[Member];
	if (Alien.TrainingRemaining > 0.0f)
	{
		return Fail(FString::Printf(TEXT("in a seminar (%.0f s)"), Alien.TrainingRemaining));
	}
	if (Alien.Training >= MaxTraining)
	{
		return Fail(TEXT("fully trained"));
	}
	const double Cost = GetTrainCost(Member);
	if (Research < Cost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f research"), Cost));
	}
	return true;
}

bool FAlienIdleSim::Train(int32 Member)
{
	if (!CanTrain(Member))
	{
		return false;
	}
	Research -= GetTrainCost(Member);
	FCrewMember& Alien = Crew[Member];
	Alien.TrainingRemaining = TrainSeconds;
	AddLog(FString::Printf(TEXT("%s has been sent to a mandatory %s seminar. Off duty for %.0f s. Attendance is compulsory; learning is optional."),
		*Alien.Name, *FString(GetSpecialtyName(Alien.Specialty)).ToLower(), TrainSeconds));
	return true;
}

void FAlienIdleSim::StepCrew(float Dt)
{
	for (FCrewMember& Member : Crew)
	{
		if (Member.TrainingRemaining > 0.0f)
		{
			Member.TrainingRemaining -= Dt;
			if (Member.TrainingRemaining <= 0.0f)
			{
				Member.TrainingRemaining = 0.0f;
				Member.Training = FMath::Min(Member.Training + 1, MaxTraining);
				AddLog(FString::Printf(TEXT("%s completed training (level %d). Certificate issued. Certificate laminated."), *Member.Name, Member.Training));
			}
		}
	}
	if (bAutoAssign && GetIdleCrew() > 0)
	{
		AutoAssignIdle();
	}
}

bool FAlienIdleSim::Assign(int32 Member, const FAssignment& Where)
{
	if (!Crew.IsValidIndex(Member))
	{
		return false;
	}
	if (Where.Station != EStation::None)
	{
		const bool bExists = GetAllStations().ContainsByPredicate([&](const FAssignment& A) { return A == Where; });
		if (!bExists || (!(Crew[Member].Assignment == Where) && GetStationCrew(Where) >= MaxCrewPerStation))
		{
			return false;
		}
	}
	Crew[Member].Assignment = Where;
	return true;
}

void FAlienIdleSim::CycleAssignment(int32 Member, int32 Direction)
{
	TArray<FAssignment> Options;
	Options.Add(FAssignment());
	Options.Append(GetAllStations());
	int32 Current = Options.IndexOfByPredicate([&](const FAssignment& A) { return A.Station == Crew[Member].Assignment.Station && A == Crew[Member].Assignment; });
	Current = Current == INDEX_NONE ? 0 : Current;
	for (int32 Step = 1; Step <= Options.Num(); ++Step)
	{
		const int32 Next = ((Current + Direction * Step) % Options.Num() + Options.Num()) % Options.Num();
		if (Assign(Member, Options[Next]))
		{
			return;
		}
	}
}

void FAlienIdleSim::AutoAssignIdle()
{
	const TArray<FAssignment> Stations = GetAllStations();
	auto IsIdle = [this](int32 i) { return Crew[i].Assignment.Station == EStation::None && Crew[i].TrainingRemaining <= 0.0f; };

	// Pass 1: unstaffed stations get someone, matching specialists preferred.
	for (const FAssignment& Station : Stations)
	{
		if (IsStationStaffed(Station))
		{
			continue;
		}
		const ESpecialty Wanted = GetStationSpecialty(Station.Station);
		int32 Pick = INDEX_NONE;
		for (int32 i = 0; i < Crew.Num(); ++i)
		{
			if (IsIdle(i) && (Pick == INDEX_NONE || Crew[i].Specialty == Wanted))
			{
				Pick = i;
				if (Crew[i].Specialty == Wanted)
				{
					break;
				}
			}
		}
		if (Pick != INDEX_NONE)
		{
			Crew[Pick].Assignment = Station;
		}
	}
	// Pass 2: remaining idle specialists join a matching station with room.
	for (int32 i = 0; i < Crew.Num(); ++i)
	{
		if (!IsIdle(i))
		{
			continue;
		}
		for (const FAssignment& Station : Stations)
		{
			if (GetStationSpecialty(Station.Station) == Crew[i].Specialty && GetStationCrew(Station) < MaxCrewPerStation)
			{
				Crew[i].Assignment = Station;
				break;
			}
		}
	}
}

void FAlienIdleSim::ReturnSpecimen(const FSpecimen& Subject)
{
	if (Subject.Occupation == EOccupation::ConspiracyTheorist)
	{
		FRegionState& Calm = Regions[FMath::Clamp(int32(Subject.Region), 0, NumRegions - 1)];
		Calm.Suspicion = FMath::Max(0.0, Calm.Suspicion - 15.0);
	}
	if (Subject.Occupation != EOccupation::None)
	{
		StudiedOccupation[int32(Subject.Occupation)] = true;
	}
	const FAssignment Wipe{ EStation::MemoryWipe, 0 };
	NoteCaseReturn(Subject, IsStationStaffed(Wipe));
	if (!IsStationStaffed(Wipe))
	{
		AddSuspicion(Subject.Region, UnwipedReturnSuspicion, Subject.Type);
		if (Subject.Type == ESpecimen::Human)
		{
			AddPressure(Subject.Region, EResponse::SupportGroup, SupportPressurePerUnwipedHuman);
		}
		if (!bWarnedUnwiped)
		{
			bWarnedUnwiped = true;
			AddLog(TEXT("Nobody is staffing the memory-wipe station. Specimens are being returned with their memories intact. They are telling people."));
		}
		return;
	}
	FRegionState& Region = Regions[FMath::Clamp(int32(Subject.Region), 0, NumRegions - 1)];
	double Removed = WipeSuspicionPerBonus * GetStationBonus(Wipe);
	if (Subject.Type == ESpecimen::Human)
	{
		// Manipulated witness memories cool the investigators down.
		Region.Pressure[int32(EResponse::Investigators)] = FMath::Max(0.0f, Region.Pressure[int32(EResponse::Investigators)] - 2.0f * (1.0f + GetStationBonus(Wipe)));
	}
	if (Subject.Type == ESpecimen::Human && IsProjectDone(EDeptProject::MemoryManipulation))
	{
		Removed += 6.0;
	}
	Region.Suspicion = FMath::Max(0.0, Region.Suspicion - Removed);
}

// ---------------------------------------------------------------------------------------------
// Unattended play: duty, holding, bottlenecks, away report

bool FAlienIdleSim::IsOnDuty(int32 Member) const
{
	if (!Crew.IsValidIndex(Member) || Crew[Member].TrainingRemaining > 0.0f)
	{
		return false;
	}
	return !(Incident.Type == EIncident::AdoptedSquirrel && Incident.Crew == Member);
}

int32 FAlienIdleSim::GetIncomingUnits() const
{
	int32 Units = 0;
	for (int32 u = 0; u < Ufos.Num(); ++u)
	{
		const FUfo& Ufo = Ufos[u];
		if (Ufo.State == EUfoState::FlyingOut || Ufo.State == EUfoState::Capturing)
		{
			Units += GetCargoCapacity(u) * GetUnits({ Ufo.TripTarget, EOccupation::None });
		}
		for (const FSpecimen& Aboard : Ufo.Cargo)
		{
			Units += GetUnits(Aboard);
		}
	}
	return Units;
}

bool FAlienIdleSim::IsHeldForContainment(int32 Index) const
{
	if (!bPauseWhenFull || Ufos[Index].State != EUfoState::Docked)
	{
		return false;
	}
	// Hold if there is no room for even one specimen once everything already on its way has landed.
	const int32 Free = GetContainmentCapacity() - GetContainmentUnitsUsed() - GetIncomingUnits();
	return Free < GetUnits({ Ufos[Index].Destination, EOccupation::None });
}

void FAlienIdleSim::StepBottlenecks(float Dt)
{
	float Add[int32(EBottleneck::Count)] = {};

	for (const FAssignment& Station : GetAllStations())
	{
		if (Station.Station == EStation::Containment && Containment.Num() == 0)
		{
			continue; // an empty habitat needs no warden
		}
		// Weight by impact: an idle ship, lab or generator stops work; a missing warden or wipe operator only adds risk.
		const float Impact = Station.Station == EStation::MemoryWipe ? 0.25f : Station.Station == EStation::Containment ? 0.5f : 1.0f;
		Add[int32(EBottleneck::Crew)] += IsStationStaffed(Station) ? 0.0f : Impact;
	}

	const bool bLabsPaused = GetLabSpeed() <= 0.0f;
	bool bAllBusy = true;
	for (int32 c = 0; c < Chambers.Num(); ++c)
	{
		if (!IsStationStaffed({ EStation::Chamber, c }))
		{
			continue;
		}
		if (Chambers[c].bRunning)
		{
			continue;
		}
		bAllBusy = false;
		if (!bLabsPaused)
		{
			Add[int32(Containment.Num() == 0 ? EBottleneck::Collection : EBottleneck::Energy)] += 1.0f;
		}
	}
	if (Containment.Num() > 0 && bAllBusy)
	{
		Add[int32(EBottleneck::LabCapacity)] += 1.0f;
	}

	for (int32 u = 0; u < Ufos.Num(); ++u)
	{
		const FUfo& Ufo = Ufos[u];
		if (Ufo.State == EUfoState::WaitingToUnload || (Ufo.bAutoLaunch && IsHeldForContainment(u)))
		{
			Add[int32(EBottleneck::Containment)] += 1.0f;
		}
		else if (Ufo.State == EUfoState::Docked && Ufo.bAutoLaunch && IsAutomationUnlocked())
		{
			FString Reason;
			if (!CanLaunch(u, &Reason) && Reason.Contains(TEXT("energy")))
			{
				Add[int32(EBottleneck::Energy)] += 1.0f;
			}
		}
	}

	const float Blend = FMath::Clamp(Dt / BottleneckWindowSeconds, 0.0f, 1.0f);
	for (int32 i = 1; i < int32(EBottleneck::Count); ++i)
	{
		BottleneckSeconds[i] += Add[i] * Dt;
		BottleneckRecent[i] += (Add[i] - BottleneckRecent[i]) * Blend;
	}
}

EBottleneck FAlienIdleSim::GetCurrentBottleneck() const
{
	EBottleneck Best = EBottleneck::None;
	float BestValue = 0.15f; // ignore brief blips
	for (int32 i = 1; i < int32(EBottleneck::Count); ++i)
	{
		if (BottleneckRecent[i] > BestValue)
		{
			BestValue = BottleneckRecent[i];
			Best = EBottleneck(i);
		}
	}
	return Best;
}

const TCHAR* FAlienIdleSim::GetBottleneckName(EBottleneck Bottleneck)
{
	switch (Bottleneck)
	{
	case EBottleneck::Crew: return TEXT("Crew");
	case EBottleneck::Energy: return TEXT("Energy");
	case EBottleneck::Collection: return TEXT("Collection (cargo)");
	case EBottleneck::Containment: return TEXT("Containment");
	case EBottleneck::LabCapacity: return TEXT("Laboratory capacity");
	default: return TEXT("None");
	}
}

FString FAlienIdleSim::GetBottleneckText() const
{
	switch (GetCurrentBottleneck())
	{
	case EBottleneck::Crew:
		for (const FAssignment& Station : GetAllStations())
		{
			if (!IsStationStaffed(Station) && !(Station.Station == EStation::Containment && Containment.Num() == 0))
			{
				return FString::Printf(TEXT("Crew: %s is unstaffed. Management considers this sustainable."), *GetStationName(Station));
			}
		}
		return TEXT("Crew: a post is unstaffed.");
	case EBottleneck::Energy:
		return FString::Printf(TEXT("Energy: labs or launches are waiting for power (%.0f / %.0f, +%.1f/s)."), Energy, GetEnergyCap(), GetEnergyPerSecond());
	case EBottleneck::Collection:
		return TEXT("Collection: labs are idle with nothing to study. More cargo, ships or pilots would help.");
	case EBottleneck::Containment:
		return FString::Printf(TEXT("Containment: ships are waiting at the dock (%d / %d units)."), GetContainmentUnitsUsed(), GetContainmentCapacity());
	case EBottleneck::LabCapacity:
		return FString::Printf(TEXT("Laboratory capacity: %d specimens waiting and every chamber is busy."), Containment.Num());
	default:
		return TEXT("No bottleneck. Everything is busy and nothing is stuck.");
	}
}

FString FAlienIdleSim::GetRecommendation() const
{
	auto BaseOption = [this](EBaseUpgrade Upgrade) -> FString
	{
		FString Reason;
		const FUpgradeDef& Def = BaseUpgrades[int32(Upgrade)];
		if (!CanBuyBaseUpgrade(Upgrade, &Reason) && !Reason.Contains(TEXT("research")))
		{
			return FString::Printf(TEXT("%s (%s)"), Def.Name, *Reason);
		}
		return FString::Printf(TEXT("%s (%.0f research)"), Def.Name, GetBaseUpgradeCost(Upgrade));
	};
	auto CheapestShip = [this](EShipSystem System) -> FString
	{
		int32 Best = INDEX_NONE;
		for (int32 u = 0; u < Ufos.Num(); ++u)
		{
			FString Reason;
			if ((CanBuyShipUpgrade(u, System, &Reason) || Reason.Contains(TEXT("research")))
				&& (Best == INDEX_NONE || GetShipUpgradeCost(u, System) < GetShipUpgradeCost(Best, System)))
			{
				Best = u;
			}
		}
		if (Best == INDEX_NONE)
		{
			return FString();
		}
		return FString::Printf(TEXT("%s on '%s' (%.0f research)"), ShipSystems[int32(System)].Name, *Ufos[Best].Name, GetShipUpgradeCost(Best, System));
	};
	const FString Hire = FString::Printf(TEXT("Request Additional Personnel (%.0f research)"), GetRecruitCost());

	switch (GetCurrentBottleneck())
	{
	case EBottleneck::Crew:
	{
		FString Where = TEXT("the empty post");
		for (const FAssignment& Station : GetAllStations())
		{
			if (!IsStationStaffed(Station) && !(Station.Station == EStation::Containment && Containment.Num() == 0))
			{
				Where = GetStationName(Station);
				break;
			}
		}
		return GetIdleCrew() > 0
			? FString::Printf(TEXT("Next: assign an idle alien to %s (free)."), *Where)
			: FString::Printf(TEXT("Next: %s for %s."), *Hire, *Where);
	}
	case EBottleneck::Energy:
		return FString::Printf(TEXT("Next: %s, or another technician on the power station."), *BaseOption(EBaseUpgrade::Power));
	case EBottleneck::Collection:
	{
		const FString Cargo = CheapestShip(EShipSystem::Cargo);
		FString Reason;
		const bool bHangarOpen = CanBuyBaseUpgrade(EBaseUpgrade::Hangar, &Reason) || Reason.Contains(TEXT("research"));
		return FString::Printf(TEXT("Next: %s%s%s."), Cargo.IsEmpty() ? TEXT("another pilot on a ship") : *Cargo,
			bHangarOpen ? TEXT(", or ") : TEXT(""), bHangarOpen ? *(BaseOption(EBaseUpgrade::Hangar) + TEXT(" plus a pilot")) : TEXT(""));
	}
	case EBottleneck::Containment:
		return FString::Printf(TEXT("Next: %s, or a second warden (+2 units each)."), *BaseOption(EBaseUpgrade::Containment));
	case EBottleneck::LabCapacity:
		return FString::Printf(TEXT("Next: %s plus a scientist, or another scientist in an existing chamber (%s)."), *BaseOption(EBaseUpgrade::Chambers), *Hire);
	default:
		return TEXT("Next: nothing is stuck. Take a contract or start a discovery project.");
	}
}

void FAlienIdleSim::BeginAwayReport()
{
	AwaySnapshot.Elapsed = Elapsed;
	AwaySnapshot.Captures = Captures;
	AwaySnapshot.Experiments = TotalExperiments;
	AwaySnapshot.ResearchEarned = ResearchEarned;
	FMemory::Memcpy(AwaySnapshot.Discovered, Discovered, sizeof(Discovered));
	AwaySnapshot.Contracts = ContractsCompleted;
	AwaySnapshot.Escapes = Escapes;
	AwaySnapshot.Incidents = IncidentCount;
	FMemory::Memcpy(AwaySnapshot.BottleneckSeconds, BottleneckSeconds, sizeof(BottleneckSeconds));
}

FAwayReport FAlienIdleSim::BuildAwayReport() const
{
	FAwayReport Report;
	Report.Seconds = Elapsed - AwaySnapshot.Elapsed;
	Report.Captures = Captures - AwaySnapshot.Captures;
	Report.Experiments = TotalExperiments - AwaySnapshot.Experiments;
	Report.ResearchEarned = ResearchEarned - AwaySnapshot.ResearchEarned;
	for (int32 d = 0; d < int32(EDiscovery::Count); ++d)
	{
		if (Discovered[d] && !AwaySnapshot.Discovered[d])
		{
			Report.Discoveries.Add(Discoveries[d].Name);
		}
	}
	Report.Contracts = ContractsCompleted - AwaySnapshot.Contracts;
	Report.Escapes = Escapes - AwaySnapshot.Escapes;
	Report.Incidents = IncidentCount - AwaySnapshot.Incidents;
	for (int32 i = 0; i < int32(EBottleneck::Count); ++i)
	{
		Report.LostSeconds[i] = BottleneckSeconds[i] - AwaySnapshot.BottleneckSeconds[i];
	}
	Report.Recommendation = GetRecommendation();
	return Report;
}

// ---------------------------------------------------------------------------------------------
// Research departments

bool FAlienIdleSim::ChooseDepartment(EDepartment Choice)
{
	if (PrimaryDepartment != EDepartment::None || Choice == EDepartment::None || Choice == EDepartment::Count)
	{
		return false;
	}
	PrimaryDepartment = Choice;
	const FDepartmentDef& Def = Department(Choice);
	AddLog(FString::Printf(TEXT("Primary department: %s. Staff have been issued %s. The other departments remain available, at 'inter-departmental' prices."),
		Def.Name, Def.UniformNote));
	// Department contracts start appearing straight away.
	if (Offers.Num() > 0)
	{
		Offers.RemoveAt(0);
		FillContractOffers();
	}
	return true;
}

double FAlienIdleSim::GetProjectCost(EDeptProject Project) const
{
	const FDeptProjectDef& Def = DeptProject(Project);
	return Def.Cost * (Def.Department == PrimaryDepartment ? 1.0 : OtherDepartmentCostMultiplier);
}

int32 FAlienIdleSim::GetProjectsDone(EDepartment Dept) const
{
	int32 Count = 0;
	for (int32 i = 0; i < int32(EDeptProject::Count); ++i)
	{
		Count += (DeptProjects[i].Department == Dept && ProjectsDone[i]) ? 1 : 0;
	}
	return Count;
}

bool FAlienIdleSim::CanBuyProject(EDeptProject Project, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	const FDeptProjectDef& Def = DeptProject(Project);
	if (IsProjectDone(Project))
	{
		return Fail(TEXT("complete"));
	}
	if (PrimaryDepartment == EDepartment::None)
	{
		return Fail(TEXT("choose a primary department first"));
	}
	if (GetProjectsDone(Def.Department) < Def.Tier)
	{
		return Fail(TEXT("finish the previous project in this department"));
	}
	const double Cost = GetProjectCost(Project);
	if (Research < Cost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f research"), Cost));
	}
	return true;
}

bool FAlienIdleSim::BuyProject(EDeptProject Project)
{
	if (!CanBuyProject(Project))
	{
		return false;
	}
	Research -= GetProjectCost(Project);
	ProjectsDone[int32(Project)] = true;
	const FDeptProjectDef& Def = DeptProject(Project);
	AddResult(FString::Printf(TEXT("%s project complete: %s %s"), Department(Def.Department).Name, Def.Flavor, Def.Effect));
	return true;
}

int32 FAlienIdleSim::GetShipSlots() const
{
	return ShipSlots + (IsProjectDone(EDeptProject::ExperimentalModules) ? 2 : 0) + AlienIdleTuning::Thesis(Thesis).ExtraSlots
		+ (Career.LocationPerk[int32(ELocation::Oceanic)] ? 1 : 0);
}

double FAlienIdleSim::GetLockdownRelease() const
{
	return IsProjectDone(EDeptProject::AutomatedCoverStories) ? 60.0 : LockdownReleaseSuspicion;
}

bool FAlienIdleSim::HasSignatureFacilities(EDepartment Dept, FString* OutMissing) const
{
	TArray<FString> Missing;
	if (Dept == EDepartment::Incompetence)
	{
		if (GetStationCrew({ EStation::Power, 0 }) < 2)
		{
			Missing.Add(TEXT("2 crew on the power station"));
		}
	}
	else
	{
		if (!IsStationStaffed({ EStation::Containment, 0 }))
		{
			Missing.Add(TEXT("a containment warden"));
		}
		if (!IsStationStaffed({ EStation::MemoryWipe, 0 }))
		{
			Missing.Add(TEXT("a memory-wipe operator"));
		}
	}
	if (OutMissing)
	{
		*OutMissing = FString::Join(Missing, TEXT(", "));
	}
	return Missing.Num() == 0;
}

TArray<int32> FAlienIdleSim::GetSignatureReservation(TArray<int32>* OutMissing) const
{
	TArray<int32> Reserved;
	if (PendingSignature == EDepartment::None)
	{
		return Reserved;
	}
	const FDepartmentDef& Def = Department(PendingSignature);
	for (int32 Need = 0; Need < 3; ++Need)
	{
		int32 Found = INDEX_NONE;
		for (int32 i = 0; i < Containment.Num(); ++i)
		{
			const FSpecimen& Held = Containment[i];
			if (!Reserved.Contains(i) && Held.Type == Local(Def.NeedType[Need])
				&& (Def.NeedOccupation[Need] == EOccupation::None || Held.Occupation == Def.NeedOccupation[Need]))
			{
				Found = i;
				break;
			}
		}
		if (Found != INDEX_NONE)
		{
			Reserved.Add(Found);
		}
		else if (OutMissing)
		{
			OutMissing->Add(Need);
		}
	}
	return Reserved;
}

bool FAlienIdleSim::CanBeginSignature(EDepartment Dept, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	if (IsSignatureDone(Dept))
	{
		return Fail(TEXT("completed"));
	}
	if (PrimaryDepartment == EDepartment::None)
	{
		return Fail(TEXT("choose a primary department first"));
	}
	if (Dept != PrimaryDepartment && GetProjectsDone(Dept) == 0)
	{
		return Fail(TEXT("needs this department's first project"));
	}
	if (PendingSignature != EDepartment::None)
	{
		return Fail(PendingSignature == Dept ? TEXT("already queued") : TEXT("another signature experiment is queued"));
	}
	return true;
}

bool FAlienIdleSim::BeginSignature(EDepartment Dept)
{
	if (!CanBeginSignature(Dept))
	{
		return false;
	}
	PendingSignature = Dept;
	const FDepartmentDef& Def = Department(Dept);
	AddLog(FString::Printf(TEXT("Signature experiment '%s' approved. The lab will hold the right specimens aside until everything is in place. %s"),
		Def.SignatureName, Def.SignatureQuestion));
	return true;
}

bool FAlienIdleSim::IsSignatureRunning(EDepartment Dept) const
{
	return Chambers.ContainsByPredicate([Dept](const FLabChamber& C)
	{
		return C.bRunning && C.Experiment.Kind == EExperimentKind::Signature && C.Experiment.Department == Dept;
	});
}

bool FAlienIdleSim::TryStartSignature(FLabChamber& Chamber)
{
	if (PendingSignature == EDepartment::None || IsSignatureRunning(PendingSignature) || !HasSignatureFacilities(PendingSignature)
		|| Energy < SignatureEnergy)
	{
		return false;
	}
	TArray<int32> Missing;
	TArray<int32> Reserved = GetSignatureReservation(&Missing);
	if (Missing.Num() > 0)
	{
		return false;
	}
	Energy -= SignatureEnergy;
	Chamber.bRunning = true;
	Chamber.Time = 0.0f;
	Chamber.Duration = SignatureSeconds;
	Chamber.Specimen = Containment[Reserved[0]];
	Chamber.Experiment = { EExperimentKind::Signature, EDiscovery::None, PendingSignature };
	Chamber.Batch = 1;
	Reserved.Sort([](int32 A, int32 B) { return A > B; });
	for (int32 Index : Reserved)
	{
		Containment.RemoveAt(Index);
	}
	AddLog(FString::Printf(TEXT("Signature experiment '%s' has begun. Please do not tap on the glass."), Department(PendingSignature).SignatureName));
	return true;
}

void FAlienIdleSim::CompleteSignature(EDepartment Dept)
{
	SignatureDone[int32(Dept)] = true;
	if (!Career.PublishedSignature[int32(Dept)])
	{
		Career.PublishedSignature[int32(Dept)] = true;
		++RunNewPublications;
	}
	if (PendingSignature == Dept)
	{
		PendingSignature = EDepartment::None;
	}
	++TotalExperiments;
	Research += SignatureResearch;
	ResearchEarned += SignatureResearch;
	const FDepartmentDef& Def = Department(Dept);
	AddResult(FString::Printf(TEXT("SIGNATURE EXPERIMENT '%s': %s Research +%.0f. Permanent: %s."), Def.SignatureName, Def.SignatureReport, SignatureResearch, Def.SignatureBonus));
	AddEncyclopediaEntry(FString::Printf(TEXT("Discovery: %s"), Def.SignatureName), Def.SignatureReport);
}

FString FAlienIdleSim::GetSignatureStatusText(EDepartment Dept) const
{
	const FDepartmentDef& Def = Department(Dept);
	if (IsSignatureDone(Dept))
	{
		return FString::Printf(TEXT("'%s' complete. Permanent: %s."), Def.SignatureName, Def.SignatureBonus);
	}
	for (const FLabChamber& Chamber : Chambers)
	{
		if (Chamber.bRunning && Chamber.Experiment.Kind == EExperimentKind::Signature && Chamber.Experiment.Department == Dept)
		{
			return FString::Printf(TEXT("'%s' running (%.0f s left)."), Def.SignatureName, FMath::Max(0.0f, Chamber.Duration - Chamber.Time));
		}
	}
	TArray<FString> Needs;
	for (int32 i = 0; i < 3; ++i)
	{
		Needs.Add(Def.NeedOccupation[i] != EOccupation::None ? FString(Occupation(Def.NeedOccupation[i]).Name).ToLower() : FString(Specimen(Local(Def.NeedType[i])).Name));
	}
	const FString Requirement = FString::Printf(TEXT("needs %s in containment, plus %s"), *FString::Join(Needs, TEXT(", ")), Def.FacilityText);
	if (PendingSignature != Dept)
	{
		return FString::Printf(TEXT("'%s': %s. 1500 research and %s."), Def.SignatureName, *Requirement, Def.SignatureBonus);
	}
	TArray<int32> Missing;
	GetSignatureReservation(&Missing);
	TArray<FString> Waiting;
	for (int32 Need : Missing)
	{
		Waiting.Add(Needs[Need]);
	}
	FString Facilities;
	HasSignatureFacilities(Dept, &Facilities);
	if (!Facilities.IsEmpty())
	{
		Waiting.Add(Facilities);
	}
	if (Energy < SignatureEnergy)
	{
		Waiting.Add(FString::Printf(TEXT("%.0f energy"), SignatureEnergy));
	}
	return Waiting.Num() > 0
		? FString::Printf(TEXT("'%s' queued, waiting for: %s."), Def.SignatureName, *FString::Join(Waiting, TEXT(", ")))
		: FString::Printf(TEXT("'%s' queued, waiting for a free staffed chamber."), Def.SignatureName);
}

FContract FAlienIdleSim::MakeDepartmentContract(EDepartment Dept, bool& bOutMade)
{
	bOutMade = true;
	FContract Contract;
	Contract.Department = Dept;
	auto Need = [this](ESpecimen Type, EOccupation Occ, int32 Count, bool bDistinct = false)
	{
		FContractNeed N;
		N.Type = Local(Type);
		N.Occupation = Occ;
		N.Count = Count;
		N.bDistinctOccupations = bDistinct;
		return N;
	};
	const bool bCows = IsSpecimenUnlocked(Local(ESpecimen::Cow));
	const bool bElephants = IsSpecimenUnlocked(Local(ESpecimen::Elephant));
	const bool bHumans = IsSpecimenUnlocked(ESpecimen::Human);
	const int32 Roll = Rng.RandRange(0, 1);
	switch (Dept)
	{
	case EDepartment::Biological:
		if (bElephants && Roll == 0)
		{
			Contract.Title = TEXT("[Biology] Menagerie: a squirrel, a cow and an elephant");
			Contract.Flavor = TEXT("The department wants one of everything, 'for the brochure.'");
			Contract.Reward = 450.0;
			Contract.Needs = { Need(ESpecimen::Squirrel, EOccupation::None, 1), Need(ESpecimen::Cow, EOccupation::None, 1), Need(ESpecimen::Elephant, EOccupation::None, 1) };
		}
		else if (bCows)
		{
			Contract.Title = TEXT("[Biology] Cross-species study: 3 squirrels and 2 cows");
			Contract.Flavor = TEXT("Determine whether squirrels are small cows. Preliminary answer: no, but keep going.");
			Contract.Reward = 260.0;
			Contract.Needs = { Need(ESpecimen::Squirrel, EOccupation::None, 3), Need(ESpecimen::Cow, EOccupation::None, 2) };
		}
		else
		{
			Contract.Title = TEXT("[Biology] Field survey: 6 squirrels");
			Contract.Flavor = TEXT("The department is building a squirrel family tree. It is mostly one squirrel.");
			Contract.Reward = 110.0;
			Contract.Needs = { Need(ESpecimen::Squirrel, EOccupation::None, 6) };
		}
		break;
	case EDepartment::Human:
		if (!bHumans)
		{
			bOutMade = false;
			return Contract;
		}
		if (Roll == 0)
		{
			Contract.Title = TEXT("[Human Behavior] Responsibility panel: an accountant and a lawyer");
			Contract.Flavor = TEXT("Find out who signs off on things. Nobody, so far.");
			Contract.Reward = 400.0;
			Contract.Needs = { Need(ESpecimen::Human, EOccupation::Accountant, 1), Need(ESpecimen::Human, EOccupation::Lawyer, 1) };
		}
		else
		{
			Contract.Title = TEXT("[Human Behavior] Focus group: 4 different professions");
			Contract.Flavor = TEXT("The department needs a representative sample. It will settle for a sample.");
			Contract.Reward = 560.0;
			Contract.Needs = { Need(ESpecimen::Human, EOccupation::None, 4, true) };
		}
		break;
	default:
		if (bHumans && Roll == 0)
		{
			Contract.Title = TEXT("[Applied Incompetence] Hazard pay: 2 magicians");
			Contract.Flavor = TEXT("Study the escape artists. Containment has been told to 'be brave.'");
			Contract.Reward = 420.0;
			Contract.Needs = { Need(ESpecimen::Human, EOccupation::Magician, 2) };
		}
		else if (bElephants)
		{
			Contract.Title = TEXT("[Applied Incompetence] Load test: 2 elephants");
			Contract.Flavor = TEXT("Engineering would like to know what the floor is rated for. So would the floor.");
			Contract.Reward = 520.0;
			Contract.Needs = { Need(ESpecimen::Elephant, EOccupation::None, 2) };
		}
		else
		{
			Contract.Title = TEXT("[Applied Incompetence] Stress test: 5 squirrels");
			Contract.Flavor = TEXT("Run the lab hot. The fire extinguisher is 'mostly decorative.'");
			Contract.Reward = 100.0;
			Contract.Needs = { Need(ESpecimen::Squirrel, EOccupation::None, 5) };
		}
		break;
	}
	return Contract;
}

TArray<FSpecimen> FAlienIdleSim::GetSignatureMissingSpecimens() const
{
	TArray<FSpecimen> Result;
	if (PendingSignature == EDepartment::None)
	{
		return Result;
	}
	TArray<int32> Missing;
	GetSignatureReservation(&Missing);
	const FDepartmentDef& Def = Department(PendingSignature);
	for (int32 Need : Missing)
	{
		Result.Add({ Local(Def.NeedType[Need]), Def.NeedOccupation[Need] });
	}
	return Result;
}

// ---------------------------------------------------------------------------------------------
// Thesis, Academy review and promotion (prestige)

double FAlienIdleSim::GetAlertSuspicion(int32 Region) const
{
	return Location == ELocation::Metropolis ? MetropolisAlertSuspicion : AlertSuspicion;
}

FString FAlienIdleSim::GetRankName() const
{
	return Ranks[FMath::Clamp(Career.Promotions, 0, int32(UE_ARRAY_COUNT(Ranks)) - 1)];
}

EOccupation FAlienIdleSim::RandomOccupation()
{
	TArray<EOccupation> Pool;
	for (int32 o = int32(EOccupation::None) + 1; o < int32(EOccupation::Count); ++o)
	{
		if ((Location == ELocation::Metropolis || !IsMetropolisOccupation(EOccupation(o))) && (Location == ELocation::Antarctic || !IsAntarcticOccupation(EOccupation(o)))
			&& !IsCaseOccupation(EOccupation(o)))
		{
			Pool.Add(EOccupation(o));
		}
	}
	return Pool[Rng.RandRange(0, Pool.Num() - 1)];
}

int32 FAlienIdleSim::CountStudiedProfessions() const
{
	int32 Count = 0;
	for (bool bStudied : StudiedOccupation)
	{
		Count += bStudied ? 1 : 0;
	}
	return Count;
}

TArray<FThesisObjective> FAlienIdleSim::GetThesisObjectives() const
{
	const FThesisDef& Def = AlienIdleTuning::Thesis(Thesis);
	TArray<FThesisObjective> Objectives;
	{
		FThesisObjective Signature;
		if (PrimaryDepartment == EDepartment::None)
		{
			Signature.Text = TEXT("Choose a department and complete its signature experiment");
		}
		else
		{
			Signature.Text = FString::Printf(TEXT("Complete the signature experiment '%s'"), Department(PrimaryDepartment).SignatureName);
			Signature.bDone = IsSignatureDone(PrimaryDepartment);
		}
		Objectives.Add(Signature);
	}
	auto Add = [&Objectives](int32 Needed, int32 Have, const TCHAR* What)
	{
		if (Needed > 0)
		{
			Objectives.Add({ FString::Printf(TEXT("%s: %d/%d"), What, FMath::Min(Have, Needed), Needed), Have >= Needed });
		}
	};
	Add(Def.Discoveries, GetDiscoveryCount(), TEXT("Discoveries this run"));
	Add(Def.Contracts, ContractsCompleted, TEXT("Contracts completed"));
	Add(Def.TotalExperiments, TotalExperiments, TEXT("Experiments completed"));
	Add(Def.DistinctProfessions, CountStudiedProfessions(), TEXT("Different professions studied"));
	Add(Def.DepartmentProjects, PrimaryDepartment != EDepartment::None ? GetProjectsDone(PrimaryDepartment) : 0, TEXT("Department projects"));
	Add(Def.LocalExperiments, GetLocalExperiments(), TEXT("Aquatic experiments (pufferfish, octopuses, whales)"));
	Add(Def.PreservedSamples, PreservedStudied, TEXT("Preserved samples studied (held a minute or more in the cold)"));
	if (Def.bNoActiveResponses)
	{
		const int32 Active = GetActiveResponseCount();
		Objectives.Add({ FString::Printf(TEXT("Earth calm when you submit: %d active response%s (counter them or let them fade)"), Active, Active == 1 ? TEXT("") : TEXT("s")), Active == 0 });
	}
	return Objectives;
}

bool FAlienIdleSim::CanSubmitThesis(FString* OutReason) const
{
	for (const FThesisObjective& Objective : GetThesisObjectives())
	{
		if (!Objective.bDone)
		{
			if (OutReason)
			{
				*OutReason = FString::Printf(TEXT("still to do: %s"), *Objective.Text);
			}
			return false;
		}
	}
	return true;
}

void FAlienIdleSim::DevCompleteThesis()
{
	for (FRegionState& Region : Regions)
	{
		for (int32 k = 0; k < int32(EResponse::Count); ++k)
		{
			Region.Active[k] = false;
			Region.Pressure[k] = 0.0f;
		}
	}
	if (PrimaryDepartment == EDepartment::None)
	{
		ChooseDepartment(EDepartment::Biological);
	}
	SignatureDone[int32(PrimaryDepartment)] = true;
	const FThesisDef& Def = AlienIdleTuning::Thesis(Thesis);
	for (int32 d = 0; d < int32(EDiscovery::Count) && GetDiscoveryCount() < Def.Discoveries; ++d)
	{
		Discover(EDiscovery(d));
	}
	ContractsCompleted = FMath::Max(ContractsCompleted, Def.Contracts);
	TotalExperiments = FMath::Max(TotalExperiments, Def.TotalExperiments);
	PreservedStudied = FMath::Max(PreservedStudied, Def.PreservedSamples);
	if (GetLocalExperiments() < Def.LocalExperiments)
	{
		DevAddExperiments(ESpecimen::Pufferfish, Def.LocalExperiments - GetLocalExperiments());
	}
	for (int32 o = 1; o < int32(EOccupation::Count) && CountStudiedProfessions() < Def.DistinctProfessions; ++o)
	{
		StudiedOccupation[o] = true;
	}
	for (int32 p = 0; p < int32(EDeptProject::Count) && GetProjectsDone(PrimaryDepartment) < Def.DepartmentProjects; ++p)
	{
		if (DeptProjects[p].Department == PrimaryDepartment)
		{
			ProjectsDone[p] = true;
		}
	}
}

TArray<EThesis> FAlienIdleSim::GetNextThesisOffers() const
{
	TArray<EThesis> ThesisOffers = { EThesis::NobodyWillBelieveYou, EThesis::CorporateWantsResults, EThesis::ControlledExperiment };
	if (IsLocationUnlocked(ELocation::Oceanic))
	{
		ThesisOffers.Add(EThesis::DeepEnd);
	}
	if (IsLocationUnlocked(ELocation::Antarctic))
	{
		ThesisOffers.Add(EThesis::ColdStorage);
	}
	return ThesisOffers;
}

FPromotionReview FAlienIdleSim::BuildPromotionReview() const
{
	FPromotionReview Review;
	const FThesisDef& Def = AlienIdleTuning::Thesis(Thesis);

	// The operation, reviewed.
	Review.Stats.Add(FString::Printf(TEXT("Thesis: %s (%s, run %d)"), Def.Name, LocationNames[int32(Location)], Career.Run));
	{
		TArray<FString> Studied;
		for (int32 t = 0; t < int32(ESpecimen::Count); ++t)
		{
			if (ExperimentsByType[t] > 0)
			{
				Studied.Add(FString::Printf(TEXT("%d %s"), ExperimentsByType[t], Specimens[t].Plural));
			}
		}
		Review.Stats.Add(FString::Printf(TEXT("Specimens studied: %s (%d professions)"), Studied.Num() > 0 ? *FString::Join(Studied, TEXT(", ")) : TEXT("none"), CountStudiedProfessions()));
	}
	TArray<FString> Found;
	for (int32 d = 0; d < int32(EDiscovery::Count); ++d)
	{
		if (Discovered[d])
		{
			Found.Add(Discoveries[d].Name);
		}
	}
	Review.Stats.Add(FString::Printf(TEXT("Discoveries: %s"), Found.Num() > 0 ? *FString::Join(Found, TEXT(", ")) : TEXT("none")));
	Review.Stats.Add(FString::Printf(TEXT("Captures %d, contracts %d, incidents %d (escapes %d), research earned %.0f"),
		Captures, ContractsCompleted, IncidentCount, Escapes, ResearchEarned));

	// The reward, line by line.
	const int32 Routine = FMath::FloorToInt32(FMath::Log2(1.0 + TotalExperiments / 10.0));
	const int32 Entries = RunNewEntries / 2;
	Review.GrantLines.Add(FString::Printf(TEXT("Thesis completed: +%d"), ThesisGrants));
	if (Def.BonusGrants > 0)
	{
		Review.GrantLines.Add(FString::Printf(TEXT("Demanding thesis bonus: +%d"), Def.BonusGrants));
	}
	Review.GrantLines.Add(FString::Printf(TEXT("New discoveries published (first time ever): %d x %d = +%d"), RunNewPublications, GrantsPerNewDiscovery, RunNewPublications * GrantsPerNewDiscovery));
	Review.GrantLines.Add(FString::Printf(TEXT("New encyclopedia entries: %d, one grant per two = +%d"), RunNewEntries, Entries));
	Review.GrantLines.Add(FString::Printf(TEXT("Routine experiments: %d, with diminishing returns = +%d"), TotalExperiments, Routine));
	Review.Grants = ThesisGrants + Def.BonusGrants + RunNewPublications * GrantsPerNewDiscovery + Entries + Routine;
	if (RunCaseDiscoveries > 0)
	{
		Review.GrantLines.Add(FString::Printf(TEXT("Case-file discoveries: %d x %d = +%d"), RunCaseDiscoveries, GrantsPerNewDiscovery, RunCaseDiscoveries * GrantsPerNewDiscovery));
		Review.Grants += RunCaseDiscoveries * GrantsPerNewDiscovery;
	}
	if (HasCaseDiscovery(CasePike))
	{
		Review.GrantLines.Add(TEXT("Hostile Peer Review (Prof. Pike's rebuttals): +1"));
		Review.Grants += 1;
	}

	// The review itself: flavour, never a change to the reward.
	if (Escapes > 0)
	{
		Review.Review = TEXT("\"Your department reported zero containment failures after reclassifying escaped specimens as field researchers. Promotion approved.\"");
	}
	else if (IncidentCount >= 5)
	{
		Review.Review = FString::Printf(TEXT("\"The panel counted %d incidents and praised your commitment to 'learning experiences.' Promotion approved, reluctantly.\""), IncidentCount);
	}
	else
	{
		Review.Review = TEXT("\"Excellent contribution to the study of Earth. Unfortunately your findings contradict the department head's published theory. Promotion approved; the theory has been quietly retired.\"");
	}
	Review.RankBefore = GetRankName();
	Review.RankAfter = Ranks[FMath::Clamp(Career.Promotions + 1, 0, int32(UE_ARRAY_COUNT(Ranks)) - 1)];

	Review.Resets = {
		TEXT("Research, energy and contracts"),
		TEXT("Ships and their upgrades (except grant upgrades)"),
		TEXT("Facilities: containment, chambers, power, hangar"),
		TEXT("Hired and trained staff (you start again with three)"),
		TEXT("Discovery unlocks: re-run them, at half the time for anything already published"),
		TEXT("Department choice, projects and signature experiment (except the one benefit you keep)"),
		TEXT("Recurring subjects' captures and this run's case studies (they go home; finished steps repeat at half the time)"),
	};
	Review.Keeps = {
		FString::Printf(TEXT("Encyclopedia (%d/%d entries)"), Encyclopedia.Num(), GetEncyclopediaSize()),
		FString::Printf(TEXT("Research Grants (%d + %d from this thesis)"), Career.Grants, Review.Grants),
		TEXT("Unlocked locations (Metropolis unlocks now; others through Academy achievements) and published discoveries' cross-base upgrades"),
		TEXT("Grant upgrades and permanent department benefits"),
		TEXT("Case files: every recurring subject's encounter history, relationships and case-file discoveries"),
	};

	// One department benefit to carry forward.
	if (PrimaryDepartment != EDepartment::None)
	{
		for (int32 p = 0; p < int32(EDeptProject::Count); ++p)
		{
			if (DeptProjects[p].Department == PrimaryDepartment && ProjectsDone[p] && !Career.PermanentProjects.Contains(EDeptProject(p)))
			{
				FPermanentOption Option;
				Option.Project = EDeptProject(p);
				Option.Department = PrimaryDepartment;
				Option.Name = DeptProjects[p].Name;
				Option.Effect = DeptProjects[p].Effect;
				Review.Options.Add(Option);
			}
		}
		if (SignatureDone[int32(PrimaryDepartment)] && !Career.PermanentSignature[int32(PrimaryDepartment)])
		{
			FPermanentOption Option;
			Option.bSignature = true;
			Option.Department = PrimaryDepartment;
			Option.Name = FString::Printf(TEXT("%s (signature bonus)"), Department(PrimaryDepartment).SignatureName);
			Option.Effect = Department(PrimaryDepartment).SignatureBonus;
			Review.Options.Add(Option);
		}
	}
	// A location benefit: the Oceanic posting's permanent discovery.
	if (Location == ELocation::Oceanic && (Thesis == EThesis::DeepEnd || IsDiscovered(EDiscovery::WhaleSong)) && !Career.LocationPerk[int32(ELocation::Oceanic)])
	{
		FPermanentOption Option;
		Option.Perk = ELocation::Oceanic;
		Option.Name = TEXT("Submersible Hulls (Oceanic)");
		Option.Effect = TEXT("Every ship at every base gets +1 upgrade slot; at Oceanic, dives cost half energy.");
		Review.Options.Add(Option);
	}
	if (Location == ELocation::Antarctic && (Thesis == EThesis::ColdStorage || IsDiscovered(EDiscovery::BlubberEconomics)) && !Career.LocationPerk[int32(ELocation::Antarctic)])
	{
		FPermanentOption Option;
		Option.Perk = ELocation::Antarctic;
		Option.Name = TEXT("Polar Crew Program (Antarctic)");
		Option.Effect = TEXT("At every base, every recruit arrives trained (level 1): fewer, better staff.");
		Review.Options.Add(Option);
	}
	return Review;
}

bool FAlienIdleSim::Promote(const FNextAssignment& Next, FString* OutReason)
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	FString Why;
	if (!CanSubmitThesis(&Why))
	{
		return Fail(Why);
	}
	const FPromotionReview Review = BuildPromotionReview();
	const ELocation Required = AlienIdleTuning::Thesis(Next.Thesis).RequiredLocation;
	const ELocation Where = Required != ELocation::Count ? Required : Next.Location;
	if (!IsLocationUnlocked(Where) && Where != ELocation::Metropolis)
	{
		return Fail(TEXT("location locked"));
	}
	if (Review.Options.Num() > 0 && !Review.Options.IsValidIndex(Next.PermanentOption))
	{
		return Fail(TEXT("choose a permanent discovery"));
	}
	FCareer NewCareer = Career;
	NewCareer.Grants += Review.Grants;
	NewCareer.TotalGrants += Review.Grants;
	NewCareer.Promotions += 1;
	NewCareer.Run += 1;
	NewCareer.LocationUnlocked[int32(ELocation::Metropolis)] = true;
	NewCareer.StartingSpecialty = Next.StartingSpecialty;
	for (EGrantUpgrade Upgrade : Next.Buy)
	{
		const int32 Cost = GrantUpgrades[int32(Upgrade)].Cost;
		if (NewCareer.Upgrades[int32(Upgrade)])
		{
			continue;
		}
		if (NewCareer.Grants < Cost)
		{
			return Fail(TEXT("not enough grants for those upgrades"));
		}
		NewCareer.Grants -= Cost;
		NewCareer.Upgrades[int32(Upgrade)] = true;
	}
	FString Kept;
	if (Review.Options.IsValidIndex(Next.PermanentOption))
	{
		const FPermanentOption& Option = Review.Options[Next.PermanentOption];
		if (Option.Perk != ELocation::Count)
		{
			NewCareer.LocationPerk[int32(Option.Perk)] = true;
		}
		else if (Option.bSignature)
		{
			NewCareer.PermanentSignature[int32(Option.Department)] = true;
		}
		else
		{
			NewCareer.PermanentProjects.AddUnique(Option.Project);
		}
		Kept = Option.Name;
	}

	// Start the next run in place, keeping the career and the encyclopedia.
	FAlienIdleSim Fresh;
	Fresh.Rng.Initialize(Rng.GetCurrentSeed());
	Fresh.CaseRng.Initialize(CaseRng.GetCurrentSeed());
	Fresh.bCaseStudies = bCaseStudies;
	Fresh.bCaseAppearances = bCaseAppearances;
	Fresh.Career = NewCareer;
	Fresh.Thesis = Next.Thesis;
	Fresh.Location = Where;
	Fresh.Condition = Next.Condition;
	for (int32 r = 0; r < int32(ERival::Count); ++r)
	{
		Fresh.Rival.Active[r] = RivalPairs[int32(Next.Thesis)][r];
	}
	Fresh.Encyclopedia = Encyclopedia;
	FMemory::Memcpy(Fresh.SeenSpecimen, SeenSpecimen, sizeof(SeenSpecimen));
	FMemory::Memcpy(Fresh.SeenOccupation, SeenOccupation, sizeof(SeenOccupation));
	Fresh.bIncidentsEnabled = bIncidentsEnabled;
	Fresh.ApplyCareerStart();
	Fresh.AddLog(FString::Printf(TEXT("PROMOTED to %s. %s Grants awarded: %d."), *Review.RankAfter, *Review.Review, Review.Grants));
	if (!Kept.IsEmpty())
	{
		Fresh.AddLog(FString::Printf(TEXT("Permanent benefit carried forward: %s."), *Kept));
	}
	Fresh.AddLog(FString::Printf(TEXT("New assignment: '%s' at %s. %s Rule: %s"), AlienIdleTuning::Thesis(Next.Thesis).Name,
		LocationNames[int32(Where)], AlienIdleTuning::Thesis(Next.Thesis).Brief, AlienIdleTuning::Thesis(Next.Thesis).Rule));
	Fresh.AddLog(FString::Printf(TEXT("Earth conditions: %s. %s"), EarthCondition(Next.Condition).Name, EarthCondition(Next.Condition).Preview));
	if (Where == ELocation::Antarctic)
	{
		Fresh.AddLog(TEXT("Welcome to Station Permafrost. Heat is shared between the labs and the hangar; the Boiler Room needs a technician. Supplies take weeks, recruits arrive trained, and the freezer is free."));
	}
	if (Where == ELocation::Oceanic)
	{
		Fresh.BaseLevels[int32(EBaseUpgrade::TidalTurbines)] = 1;
		Fresh.AddLog(TEXT("Welcome to Trench Station 9. Hull pressure is everyone's responsibility, specifically the technician on Pressure Control. Trips to Harbour Town are surface expeditions: double energy and an airlock cycle. One tidal turbine is already spinning."));
	}
	{
		TArray<FString> Followers;
		for (int32 c = 0; c < NumCaseSubjects; ++c)
		{
			if (NewCareer.Cases[c].Encounters > 0)
			{
				Followers.Add(CaseSubjects[c].Short);
			}
		}
		if (Followers.Num() > 0)
		{
			Fresh.AddLog(FString::Printf(TEXT("Case files transferred to %s. %s %s followed the sightings here. Nobody asked how."), LocationNames[int32(Where)],
				*FString::Join(Followers, TEXT(", ")), Followers.Num() == 1 ? TEXT("has") : TEXT("have")));
		}
	}
	Fresh.AddResult(FString::Printf(TEXT("ACADEMY REVIEW: %s Promotion to %s. Grants +%d."), *Review.Review, *Review.RankAfter, Review.Grants));
	*this = MoveTemp(Fresh);
	BeginAwayReport();
	return true;
}

void FAlienIdleSim::ApplyCareerStart()
{
	for (EDeptProject Project : Career.PermanentProjects)
	{
		ProjectsDone[int32(Project)] = true;
	}
	if (HasGrantUpgrade(EGrantUpgrade::BetterStartingUfo) && Ufos.Num() > 0)
	{
		Ufos[0].Levels[int32(EShipSystem::Thrusters)] = FMath::Max(Ufos[0].Levels[int32(EShipSystem::Thrusters)], 1);
		Ufos[0].Levels[int32(EShipSystem::Tractor)] = FMath::Max(Ufos[0].Levels[int32(EShipSystem::Tractor)], 1);
		Ufos[0].Levels[int32(EShipSystem::Cargo)] = FMath::Max(Ufos[0].Levels[int32(EShipSystem::Cargo)], 1);
	}
	if (HasGrantUpgrade(EGrantUpgrade::ImprovedScanning) && Ufos.Num() > 0)
	{
		Ufos[0].Levels[int32(EShipSystem::Scanner)] = FMath::Max(Ufos[0].Levels[int32(EShipSystem::Scanner)], 1);
	}
	if (HasGrantUpgrade(EGrantUpgrade::SpecialistRecruit))
	{
		AddCrew(Career.StartingSpecialty, FAssignment());
		Crew.Last().Training = 2;
		AutoAssignIdle();
	}
	// Contracts and announcements follow the new location.
	Offers.Reset();
	FillContractOffers();
}

// ---------------------------------------------------------------------------------------------
// Regions and Earth's responses

bool FAlienIdleSim::RegionHasSpecimen(int32 Region, ESpecimen Type) const
{
	const FRegionDef& Def = GetRegionDef(Region);
	for (int32 k = 0; k < Def.NumSpecimens; ++k)
	{
		if (Def.Specimens[k] == Type)
		{
			return true;
		}
	}
	return false;
}

int32 FAlienIdleSim::ActiveRegion(int32 Index) const
{
	const FUfo& Ufo = Ufos[Index];
	return FMath::Clamp(Ufo.State == EUfoState::Docked ? Ufo.Region : Ufo.TripRegion, 0, NumRegions - 1);
}

bool FAlienIdleSim::HasRelevantResponse(int32 Region, ESpecimen Type) const
{
	const FRegionState& State = Regions[Region];
	return IsSwept(Region, Type) || State.Active[int32(EResponse::Radar)]
		|| (Type == ESpecimen::Human ? State.Active[int32(EResponse::Investigators)] : State.Active[int32(EResponse::NightPatrols)]);
}

int32 FAlienIdleSim::ChooseRegionFor(int32 Index, ESpecimen Type) const
{
	const int32 Current = Ufos.IsValidIndex(Index) ? Ufos[Index].Region : GetHomeRegion(Type);
	// Stay put unless the current region is hostile or does not offer the specimen.
	if (RegionHasSpecimen(Current, Type) && !Regions[Current].bLockedDown && !HasRelevantResponse(Current, Type) && !(IsSurfaceRegion(Current) && bAirlockSealed))
	{
		return Current;
	}
	int32 Best = RegionHasSpecimen(Current, Type) ? Current : INDEX_NONE;
	double BestScore = -1.0e9;
	for (int32 r = 0; r < NumRegions; ++r)
	{
		if (!RegionHasSpecimen(r, Type))
		{
			continue;
		}
		const FRegionState& State = Regions[r];
		const bool bResponse = State.Active[int32(EResponse::Radar)]
			|| (Type == ESpecimen::Human ? State.Active[int32(EResponse::Investigators)] : State.Active[int32(EResponse::NightPatrols)]);
		const double Score = GetRegionDef(r).Value * 100.0 - (bResponse ? 60.0 : 0.0) - (IsSwept(r, Type) ? 90.0 : 0.0)
			- (IsSurfaceRegion(r) && bAirlockSealed ? 1000.0 : 0.0)
			- State.Suspicion * 0.5 - (State.bLockedDown ? 1000.0 : 0.0)
			- (r == Current ? 0.0 : 5.0); // a little stickiness
		if (Score > BestScore)
		{
			BestScore = Score;
			Best = r;
		}
	}
	return Best == INDEX_NONE ? GetHomeRegion(Type) : Best;
}

float FAlienIdleSim::GetResponseThreshold() const
{
	return EarthCondition(Condition).Threshold;
}

void FAlienIdleSim::AddPressure(int32 RegionIndex, EResponse Response, float Amount)
{
	const FEarthConditionDef& Earth = EarthCondition(Condition);
	const float Scale[int32(EResponse::Count)] = { Earth.PatrolPressure, Earth.InvestigatorPressure, Earth.RadarPressure, Earth.SupportPressure };
	FRegionState& Region = Regions[FMath::Clamp(RegionIndex, 0, NumRegions - 1)];
	float& Pressure = Region.Pressure[int32(Response)];
	if (Amount > 0.0f && ((Response == EResponse::Investigators && HasCaseDiscovery(CaseDelgado)) || (Response == EResponse::SupportGroup && HasCaseDiscovery(CaseFenn))))
	{
		Amount *= CasePressureMultiplier;
	}
	Pressure = FMath::Clamp(Pressure + Amount * Scale[int32(Response)], 0.0f, 100.0f);
	if (!Region.Active[int32(Response)] && Pressure >= GetResponseThreshold())
	{
		Region.Active[int32(Response)] = true;
		++ResponsesTriggered;
		AddLog(FString::Printf(TEXT("EARTH NOTICES: %s Effect: %s. Options: rotate regions, improve stealth, or '%s'."),
			*FString(Responses[int32(Response)].StartLog).Replace(TEXT("%s"), *GetRegionNameAt(RegionIndex)), Responses[int32(Response)].Effect, Responses[int32(Response)].Counter));
	}
}

void FAlienIdleSim::StepResponses(float Dt)
{
	for (int32 r = 0; r < NumRegions; ++r)
	{
		FRegionState& Region = Regions[r];
		for (int32 k = 0; k < int32(EResponse::Count); ++k)
		{
			Region.Pressure[k] = FMath::Max(0.0f, Region.Pressure[k] - ResponsePressureDecay * Dt);
			ResponseSeconds += Region.Active[k] ? Dt : 0.0f;
			if (Region.Active[k] && Region.Pressure[k] <= ResponseClear)
			{
				Region.Active[k] = false;
				AddLog(FString(Responses[k].EndLog).Replace(TEXT("%s"), *GetRegionNameAt(r)));
			}
		}
	}
}

double FAlienIdleSim::GetCounterCost(int32 Region, EResponse Response) const
{
	return FMath::RoundToDouble(CounterBaseCost * FMath::Pow(CounterCostGrowth, double(Regions[Region].CounterUses[int32(Response)])));
}

bool FAlienIdleSim::CanCounter(int32 Region, EResponse Response, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	if (!Regions[Region].Active[int32(Response)])
	{
		return Fail(TEXT("not active"));
	}
	const double Cost = GetCounterCost(Region, Response);
	if (Research < Cost)
	{
		return Fail(FString::Printf(TEXT("needs %.0f research"), Cost));
	}
	return true;
}

bool FAlienIdleSim::Counter(int32 Region, EResponse Response)
{
	if (!CanCounter(Region, Response))
	{
		return false;
	}
	Research -= GetCounterCost(Region, Response);
	FRegionState& State = Regions[Region];
	++State.CounterUses[int32(Response)];
	State.Active[int32(Response)] = false;
	State.Pressure[int32(Response)] = 0.0f;
	State.Suspicion = FMath::Max(0.0, State.Suspicion - 20.0);
	AddLog(FString(Responses[int32(Response)].CounterLog).Replace(TEXT("%s"), *GetRegionNameAt(Region))
		+ (State.CounterUses[int32(Response)] > 1 ? FString(TEXT(" It is getting harder to sell; the next one will cost more.")) : FString()));
	return true;
}

int32 FAlienIdleSim::GetActiveResponseCount() const
{
	int32 Count = 0;
	for (const FRegionState& Region : Regions)
	{
		for (bool bActive : Region.Active)
		{
			Count += bActive ? 1 : 0;
		}
	}
	return Count;
}

// ---------------------------------------------------------------------------------------------
// Rivals: the Efficiency Bureau

void FAlienIdleSim::StepRival(float Dt)
{
	const float Now = float(Elapsed);

	// Recent hunting grounds fade (about a five-minute memory).
	for (int32 r = 0; r < NumRegions; ++r)
	{
		for (float& Used : Rival.Usage[r])
		{
			Used = FMath::Max(0.0f, Used - Used * Dt / 300.0f);
		}
	}

	// Sweeps: announced well ahead, aimed at our busiest common-specimen ground, always temporary.
	StepRivalEvents(Dt);
	StepJoint(Dt);

	if (IsRivalActive(ERival::EfficiencyBureau) && !Rival.bSweeping && !Rival.bAnnounced && Now >= Rival.NextSweep - RivalSweepWarningSeconds)
	{
		// Busiest ground first, but the Bureau avoids regions already on alert (it has a reputation to protect),
		// which leaves the player a calmer alternative to move to.
		float Busiest = -1.0f;
		for (int32 Pass = 0; Pass < 2 && Busiest < 0.0f; ++Pass)
		{
			for (int32 r = 0; r < NumRegions; ++r)
			{
				if (Pass == 0 && (IsRegionIndexOnAlert(r) || Regions[r].bLockedDown))
				{
					continue;
				}
				for (ESpecimen Type : { Local(ESpecimen::Squirrel), Local(ESpecimen::Cow), ESpecimen::Pufferfish })
				{
					if (RegionHasSpecimen(r, Type) && IsSpecimenUnlocked(Type) && Rival.Usage[r][int32(Type)] > Busiest)
					{
						Busiest = Rival.Usage[r][int32(Type)];
						Rival.SweepRegion = r;
						Rival.SweepType = Type;
					}
				}
			}
		}
		Rival.bAnnounced = true;
		AddLog(FString::Printf(TEXT("EFFICIENCY BUREAU memo: a collection sweep of %s in %s begins in %.0f s and lasts %.0f s. Availability will be 'temporarily optimised.'"),
			Specimen(Rival.SweepType).Plural, *GetRegionNameAt(Rival.SweepRegion), RivalSweepWarningSeconds, RivalSweepSeconds));
	}
	if (Rival.bAnnounced && !Rival.bSweeping && Now >= Rival.NextSweep)
	{
		Rival.bSweeping = true;
		Rival.SweepEnds = Now + RivalSweepSeconds;
		++Rival.Sweeps;
		AddLog(FString::Printf(TEXT("The Efficiency Bureau's sweep of %s has begun. Pilots report a queue, a ticket machine and a laminated sign reading 'Bureau vehicles only.'"),
			*GetRegionNameAt(Rival.SweepRegion)));
	}
	if (Rival.bSweeping && Now >= Rival.SweepEnds)
	{
		Rival.bSweeping = false;
		Rival.bAnnounced = false;
		Rival.NextSweep = Rival.NextSweep + RivalSweepIntervalSeconds;
		// The Bureau's paperwork drowns out the rumours: the region comes back calmer than before.
		Regions[Rival.SweepRegion].Suspicion = FMath::Max(0.0, Regions[Rival.SweepRegion].Suspicion - RivalSweepCooling);
		AddLog(FString::Printf(TEXT("The Bureau's sweep of %s is over. Its forms have buried the local rumours (suspicion -%.0f), so the area is quieter for us now. The Bureau has surplus %s to trade."),
			*GetRegionNameAt(Rival.SweepRegion), RivalSweepCooling, Specimen(Rival.SweepType).Plural));
	}

	// Standing orders: sell surplus when containment is crowded, buy common specimens while there is room.
	Rival.SellTimer = FMath::Max(0.0f, Rival.SellTimer - Dt);
	if (Rival.SellTimer <= 0.0f && GetContainmentUnitsUsed() * 4 >= GetContainmentCapacity() * 3)
	{
		for (int32 k = 0; k < 3; ++k)
		{
			if (!Rival.Sell[k])
			{
				continue;
			}
			const ESpecimen Type = Local(ESpecimen(k));
			int32 Have = 0;
			int32 Last = INDEX_NONE;
			const TArray<int32> Reserved = GetSignatureReservation();
			for (int32 i = 0; i < Containment.Num(); ++i)
			{
				if (Containment[i].Type == Type && !Reserved.Contains(i))
				{
					++Have;
					Last = i;
				}
			}
			if (Have >= 2 && Last != INDEX_NONE)
			{
				const FSpecimen Sold = Containment[Last];
				Containment.RemoveAt(Last);
				const double Price = FMath::RoundToDouble(GetExperimentResearch(Sold, FExperimentChoice()) * GetSellFraction());
				Research += Price;
				ResearchEarned += Price;
				Rival.TradeIncome += Price;
				++Rival.TradesThisRun;
				++Career.Trades;
				Rival.SellTimer = TradeSellIntervalSeconds;
				if (Rival.TradesThisRun == 1 || Rival.TradesThisRun % 10 == 0)
				{
					AddLog(FString::Printf(TEXT("Standing order: sold a surplus %s to a rival department for %.0f research. They will study it faster and worse."),
						Specimen(Type).Name, Price));
				}
				break;
			}
		}
	}
	Rival.BuyTimer = FMath::Max(0.0f, Rival.BuyTimer - Dt);
	if (Rival.BuyTimer <= 0.0f)
	{
		for (int32 k = 0; k < 2; ++k)
		{
			const ESpecimen Type = Local(ESpecimen(k));
			if (Rival.Buy[k] && IsSpecimenUnlocked(Type) && Research >= GetBuyCost(ESpecimen(k))
				&& GetContainmentUnitsUsed() + GetUnits({ Type }) <= GetContainmentCapacity() - GetIncomingUnits())
			{
				Research -= GetBuyCost(ESpecimen(k));
				Containment.Add({ Type, EOccupation::None, uint8(GetHomeRegion(Type)) });
				++Rival.TradesThisRun;
				++Career.Trades;
				Rival.BuyTimer = TradeBuyIntervalSeconds;
				if (Rival.TradesThisRun == 1 || Rival.TradesThisRun % 10 == 0)
				{
					AddLog(FString::Printf(TEXT("Standing order: a rival department delivered a %s for %.0f research. It arrived in a box labelled 'Fragile: Possibly'."),
						Specimen(Type).Name, GetBuyCost(Type)));
				}
				break;
			}
		}
	}

	// Races: offered on a timer; the Bureau only works while the player is around.
	if (!Rival.bRaceOffered && !Rival.bRaceActive && Now >= Rival.NextRaceOffer
		&& (IsRivalActive(ERival::EfficiencyBureau) || IsRivalActive(ERival::CelebrityStudies) || IsRivalActive(ERival::ExperimentalSciences)))
	{
		OfferRace();
	}
	if (Rival.bRaceActive && !bCatchingUp)
	{
		Rival.RivalProgress += Dt / RivalRaceSecondsPerSpecimen;
		if (Rival.RivalProgress >= float(Rival.RaceCount))
		{
			Rival.bRaceActive = false;
			Rival.NextRaceOffer = Now + RaceOfferIntervalSeconds;
			++Career.RaceLosses;
			AddLog(TEXT("RACE LOST: A rival department has published your findings independently. Unfortunately, they also copied your spelling mistakes. No penalty; the Academy has filed it under 'character building.'"));
		}
	}
}

void FAlienIdleSim::OfferRace()
{
	// Rivals take turns challenging us, each on its own speciality (falling back to what we can catch).
	TArray<ERival> Opponents;
	for (int32 r = 0; r < int32(ERival::Count); ++r)
	{
		if (Rival.Active[r])
		{
			Opponents.Add(ERival(r));
		}
	}
	Rival.RaceOpponent = Opponents[(Career.RaceWins + Career.RaceLosses + Rival.Sweeps) % Opponents.Num()];
	const FRivalDef& Opponent = RivalDef(Rival.RaceOpponent);
	const ESpecimen Speciality = Local(Opponent.RaceType);
	Rival.RaceType = IsSpecimenUnlocked(Speciality) ? Speciality : IsSpecimenUnlocked(Local(ESpecimen::Cow)) ? Local(ESpecimen::Cow)
		: IsSpecimenUnlocked(ESpecimen::Pufferfish) ? ESpecimen::Pufferfish : Local(ESpecimen::Squirrel);
	const int32 BaseCount = Rival.RaceType == Speciality ? Opponent.RaceCount : (Rival.RaceType == ESpecimen::Cow ? 6 : 8);
	// Reputation as a competitor: each win (up to two) makes the next challenge 50% bigger and worth a grant more.
	Rival.RaceCount = FMath::CeilToInt32(BaseCount * (1.0f + 0.5f * FMath::Min(Career.RaceWins, 2)));
	Rival.RaceReward = FMath::RoundToDouble(Rival.RaceCount * Specimen(Rival.RaceType).Research * 1.5);
	Rival.bRaceOffered = true;
	AddLog(FString::Printf(TEXT("%s challenges you: who can study %d %s first? Optional. Winner gets %d Research Grants and %.0f research."),
		Opponent.Name, Rival.RaceCount, Specimen(Rival.RaceType).Plural, GetRaceGrants(), Rival.RaceReward));
}

bool FAlienIdleSim::AcceptRace()
{
	if (!Rival.bRaceOffered)
	{
		return false;
	}
	Rival.bRaceOffered = false;
	Rival.bRaceActive = true;
	Rival.RaceProgress = 0;
	Rival.RivalProgress = 0.0f;
	AddLog(TEXT("Race accepted. The Bureau has already printed a press release. It is undated, for flexibility."));
	return true;
}

void FAlienIdleSim::DeclineRace()
{
	if (Rival.bRaceOffered)
	{
		Rival.bRaceOffered = false;
		Rival.NextRaceOffer = float(Elapsed) + RaceOfferIntervalSeconds;
		AddLog(TEXT("Race declined. The Bureau has noted your 'lack of competitive spirit' in a memo to itself."));
	}
}

void FAlienIdleSim::CountTowardRace(const FSpecimen& Subject)
{
	if (!Rival.bRaceActive || Subject.Type != Rival.RaceType)
	{
		return;
	}
	if (++Rival.RaceProgress >= Rival.RaceCount)
	{
		Rival.bRaceActive = false;
		Rival.NextRaceOffer = float(Elapsed) + RaceOfferIntervalSeconds;
		Research += Rival.RaceReward;
		ResearchEarned += Rival.RaceReward;
		const int32 Grants = GetRaceGrants();
		Career.Grants += Grants;
		Career.TotalGrants += Grants;
		++Career.RaceWins;
		AddResult(FString::Printf(TEXT("RACE WON against %s: research +%.0f and %d Research Grants. They have requested a recount, a re-run and a quiet room."),
			RivalDef(Rival.RaceOpponent).Name, Rival.RaceReward, Grants));
	}
}

void FAlienIdleSim::SetSellOrder(ESpecimen Type, bool bEnabled)
{
	if (int32(Type) < 3)
	{
		Rival.Sell[int32(Type)] = bEnabled;
	}
}

void FAlienIdleSim::SetBuyOrder(ESpecimen Type, bool bEnabled)
{
	if (int32(Type) < 2)
	{
		Rival.Buy[int32(Type)] = bEnabled;
	}
}

FString FAlienIdleSim::GetRivalStatusText() const
{
	const float Now = float(Elapsed);
	if (Rival.bSweeping)
	{
		return FString::Printf(TEXT("Sweeping %s in %s now (%.0f s left): half the catch there, captures 50%% slower."),
			Specimen(Rival.SweepType).Plural, *GetRegionNameAt(Rival.SweepRegion), Rival.SweepEnds - Now);
	}
	if (Rival.bAnnounced)
	{
		return FString::Printf(TEXT("Sweep of %s in %s starts in %.0f s, lasts %.0f s."),
			Specimen(Rival.SweepType).Plural, *GetRegionNameAt(Rival.SweepRegion), Rival.NextSweep - Now, RivalSweepSeconds);
	}
	return FString::Printf(TEXT("Next sweep in about %.0f s, aimed at your busiest squirrel or cow ground."), FMath::Max(0.0f, Rival.NextSweep - Now));
}

FString FAlienIdleSim::GetRaceText() const
{
	if (Rival.bRaceActive)
	{
		return FString::Printf(TEXT("Race vs %s: %s studied, you %d/%d vs them %.0f/%d. Prize: %d grants + %.0f research. (They pause while you are away.)"),
			RivalDef(Rival.RaceOpponent).Name, Specimen(Rival.RaceType).Plural, Rival.RaceProgress, Rival.RaceCount, FMath::FloorToFloat(Rival.RivalProgress), Rival.RaceCount,
			GetRaceGrants(), Rival.RaceReward);
	}
	if (Rival.bRaceOffered)
	{
		return FString::Printf(TEXT("Challenge from %s: study %d %s first (about %.0f s at their pace). Prize: %d grants + %.0f research. No penalty for losing."),
			RivalDef(Rival.RaceOpponent).Name, Rival.RaceCount, Specimen(Rival.RaceType).Plural, Rival.RaceCount * RivalRaceSecondsPerSpecimen, GetRaceGrants(), Rival.RaceReward);
	}
	return FString::Printf(TEXT("No race open. Next challenge in about %.0f s. Record: %d won, %d lost."),
		FMath::Max(0.0f, Rival.NextRaceOffer - float(Elapsed)), Career.RaceWins, Career.RaceLosses);
}

// ---------------------------------------------------------------------------------------------
// Rivals phase 2: Celebrity Studies Division, Experimental Sciences Office, collaboration, reputation

void FAlienIdleSim::StepRivalEvents(float Dt)
{
	const float Now = float(Elapsed);
	const float Cleanup = GetBaseLevel(EBaseUpgrade::Cleanup) > 0 ? 0.5f : 1.0f;

	// Celebrity stunts happen in the most valuable region that has humans: predictable by design.
	if (IsRivalActive(ERival::CelebrityStudies))
	{
		FRivalEvent& Event = Rival.Celebrity;
		if (!Event.bAnnounced && Now >= Event.NextAt - RivalEventWarningSeconds)
		{
			Event.Region = 0;
			for (int32 r = 0; r < NumRegions; ++r)
			{
				if (RegionHasSpecimen(r, ESpecimen::Human) && (!RegionHasSpecimen(Event.Region, ESpecimen::Human) || GetRegionDef(r).Value > GetRegionDef(Event.Region).Value))
				{
					Event.Region = r;
				}
			}
			Event.bAnnounced = true;
			AddLog(FString::Printf(TEXT("CELEBRITY STUDIES DIVISION press release: a 'celebrity wellness retreat' in %s begins in %.0f s and lasts %.0f s. Every camera on Earth will be pointed at it, which means not at anywhere else."),
				*GetRegionNameAt(Event.Region), RivalEventWarningSeconds, RivalEventSeconds));
		}
		if (Event.bAnnounced && !Event.bActive && Now >= Event.NextAt)
		{
			Event.bActive = true;
			Event.EndsAt = Now + RivalEventSeconds;
			++Event.Count;
			AddLog(FString::Printf(TEXT("The Celebrity Studies Division has abducted someone famous in %s. Investigators are pouring in; elsewhere, human disappearances barely make the news."),
				*GetRegionNameAt(Event.Region)));
		}
		if (Event.bActive)
		{
			AddPressure(Event.Region, EResponse::Investigators, CelebrityInvestigatorPressurePerSecond * Dt);
			Regions[Event.Region].Suspicion = FMath::Min(LockdownSuspicion, Regions[Event.Region].Suspicion + CelebritySuspicionPerSecond * Dt);
			if (Now >= Event.EndsAt)
			{
				Event.bActive = false;
				Event.bAnnounced = false;
				Event.NextAt += CelebrityIntervalSeconds;
				AddLog(TEXT("The celebrity has been returned and has announced a memoir. The Celebrity Studies Division has announced a sequel."));
			}
		}
	}

	// Experimental Sciences breaches cycle through the regions.
	if (IsRivalActive(ERival::ExperimentalSciences))
	{
		FRivalEvent& Event = Rival.Breach;
		if (!Event.bAnnounced && Now >= Event.NextAt - RivalEventWarningSeconds)
		{
			Event.Region = Event.Count % NumRegions;
			Event.bAnnounced = true;
			AddLog(FString::Printf(TEXT("EXPERIMENTAL SCIENCES OFFICE notice: a 'scheduled unscheduled release' will affect %s in %.0f s for %.0f s. Escaped specimens will be studyable by anyone. Please do not feed them."),
				*GetRegionNameAt(Event.Region), RivalEventWarningSeconds, RivalEventSeconds));
		}
		if (Event.bAnnounced && !Event.bActive && Now >= Event.NextAt)
		{
			Event.bActive = true;
			Event.EndsAt = Now + RivalEventSeconds;
			++Event.Count;
			AddLog(FString::Printf(TEXT("Containment breach at the Experimental Sciences Office. Their escapees are loose in %s: every trip there brings back one extra specimen, but the locals are forming a support group."),
				*GetRegionNameAt(Event.Region)));
		}
		if (Event.bActive)
		{
			AddPressure(Event.Region, EResponse::SupportGroup, BreachSupportPressurePerSecond * Cleanup * Dt);
			Regions[Event.Region].Suspicion = FMath::Min(LockdownSuspicion, Regions[Event.Region].Suspicion + BreachSuspicionPerSecond * Cleanup * Dt);
			if (Now >= Event.EndsAt)
			{
				Event.bActive = false;
				Event.bAnnounced = false;
				Event.NextAt += BreachIntervalSeconds;
				AddLog(TEXT("The breach is over. The Experimental Sciences Office describes it as 'a successful field trial of our containment's limits.'"));
			}
		}
	}
}

FString FAlienIdleSim::GetRivalEventText(ERival Type) const
{
	const float Now = float(Elapsed);
	switch (Type)
	{
	case ERival::EfficiencyBureau:
		return GetRivalStatusText();
	case ERival::CelebrityStudies:
	{
		const FRivalEvent& Event = Rival.Celebrity;
		if (Event.bActive)
		{
			return FString::Printf(TEXT("Celebrity stunt in %s (%.0f s left): investigators gather there; human suspicion elsewhere is halved."), *GetRegionNameAt(Event.Region), Event.EndsAt - Now);
		}
		if (Event.bAnnounced)
		{
			return FString::Printf(TEXT("Celebrity stunt in %s starts in %.0f s, lasts %.0f s."), *GetRegionNameAt(Event.Region), Event.NextAt - Now, RivalEventSeconds);
		}
		return FString::Printf(TEXT("Next stunt in about %.0f s, in your most valuable human region."), FMath::Max(0.0f, Event.NextAt - Now));
	}
	default:
	{
		const FRivalEvent& Event = Rival.Breach;
		if (Event.bActive)
		{
			return FString::Printf(TEXT("Breach in %s (%.0f s left): +1 specimen per trip there; support group pressure building%s."),
				*GetRegionNameAt(Event.Region), Event.EndsAt - Now, GetBaseLevel(EBaseUpgrade::Cleanup) > 0 ? TEXT(" (halved by the Cleanup Crew)") : TEXT(""));
		}
		if (Event.bAnnounced)
		{
			return FString::Printf(TEXT("Breach in %s starts in %.0f s, lasts %.0f s."), *GetRegionNameAt(Event.Region), Event.NextAt - Now, RivalEventSeconds);
		}
		return FString::Printf(TEXT("Next breach in about %.0f s (it works through the regions in order)."), FMath::Max(0.0f, Event.NextAt - Now));
	}
	}
}

ERival FAlienIdleSim::GetJointRival() const
{
	for (int32 r = 0; r < int32(ERival::Count); ++r)
	{
		if (Rival.Active[r] && !Rival.JointDone[r])
		{
			return ERival(r);
		}
	}
	return ERival::Count;
}

void FAlienIdleSim::StepJoint(float Dt)
{
	const ERival Partner = GetJointRival();
	if (Partner == ERival::Count || !Rival.bContribute)
	{
		return;
	}
	Rival.ContributeTimer = FMath::Max(0.0f, Rival.ContributeTimer - Dt);
	if (Rival.ContributeTimer > 0.0f)
	{
		return;
	}
	const FRivalDef& Def = RivalDef(Partner);
	const TArray<int32> Reserved = GetSignatureReservation();
	for (int32 n = 0; n < 2; ++n)
	{
		// Both needs may name the same species; count them as one combined quota in that case.
		const bool bShared = n == 1 && Def.JointType[1] == Def.JointType[0];
		if (bShared || Rival.JointProgress[n] >= Def.JointCount[n] + (n == 0 && Def.JointType[1] == Def.JointType[0] ? Def.JointCount[1] : 0))
		{
			continue;
		}
		const int32 Index = Containment.IndexOfByPredicate([&](const FSpecimen& S) { return S.Type == Local(Def.JointType[n]); });
		if (Index != INDEX_NONE && !Reserved.Contains(Index))
		{
			Containment.RemoveAt(Index);
			++Rival.JointProgress[n];
			Rival.ContributeTimer = ContributeIntervalSeconds;
			break;
		}
	}
	const bool bSame = Def.JointType[1] == Def.JointType[0];
	const bool bDone = bSame ? Rival.JointProgress[0] >= Def.JointCount[0] + Def.JointCount[1]
		: Rival.JointProgress[0] >= Def.JointCount[0] && Rival.JointProgress[1] >= Def.JointCount[1];
	if (bDone)
	{
		Rival.JointDone[int32(Partner)] = true;
		Rival.JointProgress[0] = Rival.JointProgress[1] = 0;
		++Career.Collaborations;
		Career.Grants += JointGrants;
		Career.TotalGrants += JointGrants;
		Research += JointResearch;
		ResearchEarned += JointResearch;
		AddResult(FString::Printf(TEXT("JOINT EXPERIMENT '%s' with %s: %s Shared discovery: research +%.0f, %d Research Grant. Collaborations: %d."),
			Def.JointName, Def.Name, Def.JointReport, JointResearch, JointGrants, Career.Collaborations));
		AddEncyclopediaEntry(FString::Printf(TEXT("Discovery: %s"), Def.JointName), Def.JointReport);
	}
}

FString FAlienIdleSim::GetJointText() const
{
	const ERival Partner = GetJointRival();
	if (Partner == ERival::Count)
	{
		return TEXT("No joint experiment open on this assignment.");
	}
	const FRivalDef& Def = RivalDef(Partner);
	const bool bSame = Def.JointType[1] == Def.JointType[0];
	const FString Needs = bSame
		? FString::Printf(TEXT("%s %d/%d"), Specimen(Def.JointType[0]).Plural, Rival.JointProgress[0], Def.JointCount[0] + Def.JointCount[1])
		: FString::Printf(TEXT("%s %d/%d, %s %d/%d"), Specimen(Def.JointType[0]).Plural, Rival.JointProgress[0], Def.JointCount[0],
			Specimen(Def.JointType[1]).Plural, Rival.JointProgress[1], Def.JointCount[1]);
	return FString::Printf(TEXT("Joint experiment '%s' with %s: contribute %s from containment (no research for them). Shared discovery: %.0f research + %d grant, and better trades."),
		Def.JointName, Def.Name, *Needs, JointResearch, JointGrants);
}

double FAlienIdleSim::GetSellFraction() const
{
	return Career.Collaborations >= 1 ? CollaboratorSellFraction : TradeSellFraction;
}

double FAlienIdleSim::GetBuyCost(ESpecimen Type) const
{
	const int32 Index = FMath::Clamp(int32(Type), 0, 1);
	return FMath::RoundToDouble(TradeBuyCost[Index] * (Career.Collaborations >= 2 ? TrustedBuyMultiplier : 1.0));
}

int32 FAlienIdleSim::GetRaceGrants() const
{
	return RaceGrants + FMath::Min(Career.RaceWins, 2);
}

FString FAlienIdleSim::GetReputationText() const
{
	const TCHAR* Collaborator = Career.Collaborations >= 2 ? TEXT("trusted partner: sell at 85%, buy 30% cheaper")
		: Career.Collaborations == 1 ? TEXT("collaborator: sell surplus at 85%") : TEXT("unknown: standard trade terms");
	const TCHAR* Competitor = Career.RaceWins >= 2 ? TEXT("feared: races twice as big, +2 grants")
		: Career.RaceWins == 1 ? TEXT("noticed: races 50% bigger, +1 grant") : TEXT("unproven: standard races");
	return FString::Printf(TEXT("Reputation (kept across runs): %d collaborations, %s. %d races won, %d lost, %s."),
		Career.Collaborations, Collaborator, Career.RaceWins, Career.RaceLosses, Competitor);
}

// ---------------------------------------------------------------------------------------------
// Builds: suspicion scale, explanations, presets, approach templates, behaviour reports

double FAlienIdleSim::SuspicionScale(int32 RegionIndex, ESpecimen Kind) const
{
	const FRegionState& Region = Regions[FMath::Clamp(RegionIndex, 0, NumRegions - 1)];
	return GetRegionDef(RegionIndex).Exposure * AlienIdleTuning::Thesis(Thesis).SuspicionMultiplier * EarthCondition(Condition).Suspicion
		* (Location == ELocation::Metropolis && Kind == ESpecimen::Human ? MetropolisSuspicionMultiplier : 1.0)
		* (Kind == ESpecimen::Human && Region.Active[int32(EResponse::Investigators)] ? 2.0 : 1.0)
		* (Kind == ESpecimen::Human && Rival.Celebrity.bActive && Rival.Celebrity.Region != RegionIndex ? CelebrityCoverMultiplier : 1.0);
}

FString FAlienIdleSim::ExplainTarget(int32 Index) const
{
	const FUfo& Ufo = Ufos[Index];
	const ESpecimen Type = Ufo.Destination;
	const int32 RegionIndex = Ufo.Region;
	const FRegionDef& Region = GetRegionDef(RegionIndex);
	const FRegionState& State = Regions[RegionIndex];
	const FSpecimenDef& Def = Specimen(Type);
	TArray<FString> Lines;

	// Research per specimen (routine study).
	{
		const EOccupation Scan = GetScannerTarget(Index, Type);
		const FSpecimen Typical{ Type, Scan, uint8(RegionIndex) };
		TArray<FString> Factors;
		Factors.Add(FString::Printf(TEXT("base %.0f"), Def.Research));
		Factors.Add(FString::Printf(TEXT("%s value x%.1f"), Region.Name, Region.Value));
		if (Scan != EOccupation::None)
		{
			Factors.Add(FString::Printf(TEXT("%s x%.2f"), Occupation(Scan).Name, Occupation(Scan).ResearchMultiplier));
		}
		if (Type == ESpecimen::Human)
		{
			if (IsProjectDone(EDeptProject::AdvancedScanners)) { Factors.Add(TEXT("Advanced Scanners x1.25")); }
			if (HasSignatureBonus(EDepartment::Human)) { Factors.Add(TEXT("signature x1.25")); }
			if (IsDiscovered(EDiscovery::SarcasmOrgan)) { Factors.Add(TEXT("Sarcasm Organ x1.5")); }
			if (EarthCondition(Condition).HumanResearch != 1.0) { Factors.Add(FString::Printf(TEXT("%s x%.2f"), EarthCondition(Condition).Name, EarthCondition(Condition).HumanResearch)); }
			if (State.Active[int32(EResponse::SupportGroup)]) { Factors.Add(TEXT("support group x0.9")); }
		}
		else
		{
			if (HasSignatureBonus(EDepartment::Biological)) { Factors.Add(TEXT("signature x1.25")); }
			if (State.Active[int32(EResponse::NightPatrols)]) { Factors.Add(TEXT("night patrols x0.8")); }
			if (IsProjectDone(EDeptProject::BatchExperiments)) { Factors.Add(TEXT("batch: two per chamber")); }
		}
		Lines.Add(FString::Printf(TEXT("Research per %s: %.0f (%s). Extended studies x3.5 for 2.5x time and energy."),
			Def.Name, GetExperimentResearch(Typical, FExperimentChoice()), *FString::Join(Factors, TEXT(", "))));
	}

	// Trip time.
	{
		TArray<FString> Factors;
		const int32 Thrusters = GetShipLevel(Index, EShipSystem::Thrusters);
		if (Thrusters > 0) { Factors.Add(FString::Printf(TEXT("thrusters x%.2f"), FMath::Pow(ThrustersFlightMultiplier, float(Thrusters)))); }
		const float CrewBonus = GetStationBonus({ EStation::Ufo, Index });
		if (CrewBonus > 0.0f) { Factors.Add(FString::Printf(TEXT("crew x%.2f"), 1.0f / (1.0f + UfoSpeedPerBonus * CrewBonus))); }
		if (Region.Flight != 1.0f) { Factors.Add(FString::Printf(TEXT("distance x%.1f"), Region.Flight)); }
		if (State.Active[int32(EResponse::Radar)]) { Factors.Add(TEXT("radar x1.3")); }
		if (IsSurfaceRegion(RegionIndex)) { Factors.Add(FString::Printf(TEXT("surface expedition: launch energy x2, +%.0f hull pressure"), HullStressPerSurfaceLaunch * AlienIdleTuning::Thesis(Thesis).HullStressMultiplier)); }
		if (IsUnderwaterRegion(RegionIndex)) { Factors.Add(TEXT("underwater: no radar, half patrol attention")); }
		const float Capture = GetCaptureSeconds(Index, Type, GetScannerTarget(Index, Type) != EOccupation::None);
		TArray<FString> CaptureFactors;
		if (GetShipLevel(Index, EShipSystem::Cargo) > 0) { CaptureFactors.Add(FString::Printf(TEXT("cargo loading x%.1f"), 1.0f + CargoCapturePerLevel * GetShipLevel(Index, EShipSystem::Cargo))); }
		if (IsRegionIndexOnAlert(RegionIndex)) { CaptureFactors.Add(TEXT("alert x1.5")); }
		if (Type != ESpecimen::Human && State.Active[int32(EResponse::NightPatrols)]) { CaptureFactors.Add(TEXT("patrols x1.6")); }
		if (Type == ESpecimen::Human && State.Active[int32(EResponse::Investigators)]) { CaptureFactors.Add(TEXT("investigators x1.3")); }
		if (IsSwept(RegionIndex, Type)) { CaptureFactors.Add(TEXT("Bureau sweep x1.5, half cargo")); }
		if (GetScannerTarget(Index, Type) != EOccupation::None) { CaptureFactors.Add(TEXT("scanning adds time")); }
		Lines.Add(FString::Printf(TEXT("Trip: %.1f s each way%s; capture %.1f s%s; %d per trip%s."),
			GetFlightSeconds(Index, Type), Factors.Num() > 0 ? *FString::Printf(TEXT(" (%s)"), *FString::Join(Factors, TEXT(", "))) : TEXT(""),
			Capture, CaptureFactors.Num() > 0 ? *FString::Printf(TEXT(" (%s)"), *FString::Join(CaptureFactors, TEXT(", "))) : TEXT(""),
			GetCargoCapacity(Index), IsBreach(RegionIndex) ? TEXT(" +1 escapee from the breach") : TEXT("")));
	}

	// Suspicion per trip.
	{
		const double PerTrip = Def.SuspicionPerSpecimen * GetCargoCapacity(Index) * GetSuspicionMultiplier(Index) * SuspicionScale(RegionIndex, Type);
		TArray<FString> Factors;
		Factors.Add(FString::Printf(TEXT("%.0f each x %d"), Def.SuspicionPerSpecimen, GetCargoCapacity(Index)));
		Factors.Add(FString::Printf(TEXT("exposure x%.1f"), Region.Exposure));
		if (GetShipLevel(Index, EShipSystem::Stealth) > 0) { Factors.Add(FString::Printf(TEXT("stealth x%.2f"), GetSuspicionMultiplier(Index))); }
		if (AlienIdleTuning::Thesis(Thesis).SuspicionMultiplier != 1.0) { Factors.Add(FString::Printf(TEXT("thesis x%.1f"), AlienIdleTuning::Thesis(Thesis).SuspicionMultiplier)); }
		if (EarthCondition(Condition).Suspicion != 1.0) { Factors.Add(FString::Printf(TEXT("%s x%.1f"), EarthCondition(Condition).Name, EarthCondition(Condition).Suspicion)); }
		if (Type == ESpecimen::Human && State.Active[int32(EResponse::Investigators)]) { Factors.Add(TEXT("investigators x2")); }
		if (Type == ESpecimen::Human && Rival.Celebrity.bActive && Rival.Celebrity.Region != RegionIndex) { Factors.Add(TEXT("celebrity stunt elsewhere x0.5")); }
		Lines.Add(FString::Printf(TEXT("Suspicion per trip: +%.1f in %s (%s); now %.0f, alert at %.0f."),
			PerTrip, Region.Name, *FString::Join(Factors, TEXT(", ")), State.Suspicion, GetAlertSuspicion(RegionIndex)));
	}
	return FString::Join(Lines, LINE_TERMINATOR);
}

void FAlienIdleSim::SavePreset(int32 Slot, const FString& Name)
{
	if (Slot < 0 || Slot >= PresetSlots)
	{
		return;
	}
	FBuildPreset& Preset = Career.Presets[Slot];
	Preset = FBuildPreset();
	Preset.bSaved = true;
	Preset.Name = Name;
	for (const FUfo& Ufo : Ufos)
	{
		Preset.Ufos.Add({ Ufo.Destination, Ufo.Region, Ufo.bAutoLaunch, Ufo.bFollowContract });
	}
	for (const FCrewMember& Member : Crew)
	{
		Preset.Crew.Add(Member.Assignment);
	}
	FMemory::Memcpy(Preset.Choices, Choices, sizeof(Choices));
	FMemory::Memcpy(Preset.Sell, Rival.Sell, sizeof(Rival.Sell));
	FMemory::Memcpy(Preset.Buy, Rival.Buy, sizeof(Rival.Buy));
	Preset.bReroute = bReroute;
	Preset.bPauseWhenFull = bPauseWhenFull;
	Preset.bContribute = Rival.bContribute;
	Preset.bAutoContracts = bAutoContracts;
	AddLog(FString::Printf(TEXT("Preset %d saved as '%s'. Alien Resources has laminated it."), Slot + 1, *Name));
}

bool FAlienIdleSim::ApplyPreset(int32 Slot)
{
	if (Slot < 0 || Slot >= PresetSlots || !Career.Presets[Slot].bSaved)
	{
		return false;
	}
	const FBuildPreset& Preset = Career.Presets[Slot];
	// Apply as far as this base allows: missing ships and crew are skipped, locked targets stay chosen.
	for (int32 u = 0; u < FMath::Min(Ufos.Num(), Preset.Ufos.Num()); ++u)
	{
		Ufos[u].Destination = Preset.Ufos[u].Destination;
		Ufos[u].Region = FMath::Clamp(Preset.Ufos[u].Region, 0, NumRegions - 1);
		Ufos[u].bAutoLaunch = Preset.Ufos[u].bAutoLaunch;
		Ufos[u].bFollowContract = Preset.Ufos[u].bFollowContract;
	}
	for (int32 c = 0; c < FMath::Min(Crew.Num(), Preset.Crew.Num()); ++c)
	{
		Assign(c, Preset.Crew[c]);
	}
	FMemory::Memcpy(Choices, Preset.Choices, sizeof(Choices));
	FMemory::Memcpy(Rival.Sell, Preset.Sell, sizeof(Rival.Sell));
	FMemory::Memcpy(Rival.Buy, Preset.Buy, sizeof(Rival.Buy));
	bReroute = Preset.bReroute;
	bPauseWhenFull = Preset.bPauseWhenFull;
	Rival.bContribute = Preset.bContribute;
	bAutoContracts = Preset.bAutoContracts;
	AddLog(FString::Printf(TEXT("Preset '%s' applied. Staff have been reassigned and told it was their idea."), *Preset.Name));
	return true;
}

FString FAlienIdleSim::GetPresetSummary(int32 Slot) const
{
	const FBuildPreset& Preset = Career.Presets[Slot];
	if (!Preset.bSaved)
	{
		return FString::Printf(TEXT("Slot %d: empty"), Slot + 1);
	}
	int32 Auto = 0;
	for (const FUfoPreset& Ufo : Preset.Ufos)
	{
		Auto += Ufo.bAutoLaunch ? 1 : 0;
	}
	return FString::Printf(TEXT("Slot %d: '%s' (%d ships, %d auto; %d crew posts)"), Slot + 1, *Preset.Name, Preset.Ufos.Num(), Auto, Preset.Crew.Num());
}

void FAlienIdleSim::ApplyApproachTemplate(EApproach Which)
{
	auto Target = [this](int32 u, std::initializer_list<ESpecimen> Wanted)
	{
		for (ESpecimen Type : Wanted)
		{
			if (IsSpecimenUnlocked(Type) && GetTractorStrength(u) >= Specimen(Type).Weight)
			{
				SetDestinationBest(u, Type);
				return;
			}
		}
	};
	for (int32 u = 0; u < Ufos.Num(); ++u)
	{
		Ufos[u].bAutoLaunch = true;
		switch (Which)
		{
		case EApproach::Bulk:
			Ufos[u].bFollowContract = false;
			Target(u, { ESpecimen::Whale, ESpecimen::ElephantSeal, ESpecimen::Cow, ESpecimen::Seal, ESpecimen::Elephant, ESpecimen::Pufferfish, ESpecimen::Penguin, ESpecimen::Squirrel });
			break;
		case EApproach::Precision:
			Ufos[u].bFollowContract = true;
			Target(u, { ESpecimen::Human, ESpecimen::Octopus, ESpecimen::Whale, ESpecimen::ElephantSeal, ESpecimen::Elephant, ESpecimen::Cow, ESpecimen::Seal, ESpecimen::Pufferfish, ESpecimen::Penguin, ESpecimen::Squirrel });
			break;
		default:
			Ufos[u].bFollowContract = true;
			Target(u, { ESpecimen::Human, ESpecimen::Octopus, ESpecimen::Pufferfish, ESpecimen::Penguin, ESpecimen::Cow, ESpecimen::Seal, ESpecimen::Squirrel });
			break;
		}
	}
	for (int32 t = 0; t < int32(ESpecimen::Count); ++t)
	{
		if (Choices[t].Kind == EExperimentKind::Discovery)
		{
			continue; // never cancel a chosen discovery project
		}
		const bool bAnimal = ESpecimen(t) != ESpecimen::Human;
		Choices[t].Kind = (Which == EApproach::Bulk && bAnimal && ESpecimen(t) != Local(ESpecimen::Squirrel) && ESpecimen(t) != ESpecimen::Pufferfish) ? EExperimentKind::Extended : EExperimentKind::Routine;
	}
	const bool bBulk = Which == EApproach::Bulk;
	Rival.Sell[0] = Rival.Sell[1] = bBulk;
	Rival.Sell[2] = false;
	Rival.Buy[0] = false;
	Rival.Buy[1] = bBulk;
	bReroute = true;
	bPauseWhenFull = true;
	Rival.bContribute = Which != EApproach::Covert;
	bAutoContracts = true;
	AddLog(FString::Printf(TEXT("Approach template applied: %s. %s"), Approach(Which).Name, Approach(Which).Pitch));
}

EApproach FAlienIdleSim::ClassifyApproach(bool* bOutClear) const
{
	float Score[int32(EApproach::Count)] = {};
	for (int32 u = 0; u < Ufos.Num(); ++u)
	{
		Score[int32(EApproach::Bulk)] += GetShipLevel(u, EShipSystem::Cargo);
		Score[int32(EApproach::Precision)] += 1.5f * GetShipLevel(u, EShipSystem::Scanner);
		Score[int32(EApproach::Covert)] += 1.5f * GetShipLevel(u, EShipSystem::Stealth);
	}
	const int32 Humans = ExperimentsByType[int32(ESpecimen::Human)];
	const int32 Animals = TotalExperiments - Humans;
	Score[int32(EApproach::Bulk)] += IsProjectDone(EDeptProject::BatchExperiments) ? 2.0f : 0.0f;
	Score[int32(EApproach::Bulk)] += Animals > 3 * FMath::Max(1, Humans) ? 2.0f : 0.0f;
	Score[int32(EApproach::Precision)] += Humans > Animals / 3 ? 2.0f : 0.0f;
	Score[int32(EApproach::Covert)] += ResponsesTriggered == 0 && TotalExperiments > 10 ? 2.0f : 0.0f;
	int32 Best = 0;
	for (int32 a = 1; a < int32(EApproach::Count); ++a)
	{
		Best = Score[a] > Score[Best] ? a : Best;
	}
	if (bOutClear)
	{
		float Second = 0.0f;
		for (int32 a = 0; a < int32(EApproach::Count); ++a)
		{
			Second = a != Best ? FMath::Max(Second, Score[a]) : Second;
		}
		*bOutClear = Score[Best] >= 2.0f && Score[Best] >= Second + 1.0f;
	}
	return EApproach(Best);
}

void FAlienIdleSim::StepBehaviourMemo()
{
	if (float(Elapsed) < NextBehaviourMemo)
	{
		return;
	}
	NextBehaviourMemo += BehaviourMemoSeconds;
	bool bClear = false;
	const EApproach Style = ClassifyApproach(&bClear);
	static const TCHAR* Bulk[] = {
		TEXT("Containment is so full the squirrels have formed a residents' committee."),
		TEXT("The Efficiency Bureau has asked how we are so efficient. Legal advises silence."),
		TEXT("Batch processing has reached the point where specimens are studied in alphabetical order, approximately."),
	};
	static const TCHAR* Precision[] = {
		TEXT("Your scanner crew has started rating humans out of ten. Alien Resources is concerned."),
		TEXT("Precision update: we abducted exactly the right accountant. He is very proud of this."),
		TEXT("The contract office reports record payouts and a shortage of the word 'qualified.'"),
	};
	static const TCHAR* Covert[] = {
		TEXT("Your stealth division remains unrecognized. They have filed a complaint."),
		TEXT("Earth has still not noticed us. Morale in the stealth division is low, but nobody can find them to raise it."),
		TEXT("The stealth division's award ceremony was held in secret. Nobody attended, which they took as a compliment."),
	};
	static const TCHAR* Mixed[] = {
		TEXT("Your operation defies classification. The Academy has created a new form for it, and a second form to describe the first."),
		TEXT("Internal review: the station does a bit of everything. Management calls this 'agile'. Staff call it Tuesday."),
	};
	const int32 Pick = BehaviourMemos++;
	FString Line;
	if (!bClear)
	{
		Line = Mixed[Pick % UE_ARRAY_COUNT(Mixed)];
	}
	else
	{
		switch (Style)
		{
		case EApproach::Bulk: Line = Bulk[Pick % UE_ARRAY_COUNT(Bulk)]; break;
		case EApproach::Precision: Line = Precision[Pick % UE_ARRAY_COUNT(Precision)]; break;
		default: Line = Covert[Pick % UE_ARRAY_COUNT(Covert)]; break;
		}
	}
	AddLog(FString::Printf(TEXT("DEPARTMENT MEMO (%s): %s"), bClear ? Approach(Style).Name : TEXT("mixed operation"), *Line));
}

TArray<FString> FAlienIdleSim::GetBottleneckFixes() const
{
	TArray<FString> Fixes;
	auto Base = [this](EBaseUpgrade Upgrade, const TCHAR* Why)
	{
		return FString::Printf(TEXT("%s, %.0f research: %s"), BaseUpgrades[int32(Upgrade)].Name, GetBaseUpgradeCost(Upgrade), Why);
	};
	auto Project = [this](EDeptProject Which, const TCHAR* Why)
	{
		return IsProjectDone(Which) ? FString() : FString::Printf(TEXT("%s (department project): %s"), DeptProjects[int32(Which)].Name, Why);
	};
	auto AddIf = [&Fixes](const FString& Line)
	{
		if (!Line.IsEmpty())
		{
			Fixes.Add(Line);
		}
	};
	switch (GetCurrentBottleneck())
	{
	case EBottleneck::Crew:
		AddIf(FString::Printf(TEXT("Request Additional Personnel, %.0f research: fills the empty post"), GetRecruitCost()));
		break;
	case EBottleneck::Energy:
		AddIf(Base(EBaseUpgrade::Power, TEXT("more energy per second")));
		AddIf(TEXT("A technician on the power station: +energy (any approach)"));
		AddIf(Project(EDeptProject::UnusualPower, TEXT("cheaper energy for everything")));
		break;
	case EBottleneck::Collection:
		AddIf(TEXT("Cargo bays (bulk): more specimens per trip"));
		AddIf(TEXT("Thrusters or a pilot: shorter trips (any approach)"));
		AddIf(Base(EBaseUpgrade::Hangar, TEXT("another ship")));
		AddIf(Project(EDeptProject::HeavyTractorArray, TEXT("heavier targets for every ship")));
		break;
	case EBottleneck::Containment:
		AddIf(Base(EBaseUpgrade::Containment, TEXT("+units (bulk)")));
		AddIf(TEXT("A second warden: +2 units"));
		AddIf(Project(EDeptProject::ReinforcedHabitats, TEXT("big animals take less room")));
		AddIf(TEXT("Sell surplus to a rival (trade orders), or switch to fewer, richer specimens (precision)"));
		break;
	case EBottleneck::LabCapacity:
		AddIf(Base(EBaseUpgrade::Chambers, TEXT("another chamber, plus a scientist")));
		AddIf(Project(EDeptProject::BatchExperiments, TEXT("two animals per chamber (bulk)")));
		AddIf(TEXT("Routine instead of Extended studies: faster throughput, less research each"));
		break;
	default:
		break;
	}
	if (HasHeat() && (GetLabWarmth() < 0.8f || GetHangarWarmth() < 0.8f))
	{
		AddIf(FString::Printf(TEXT("Cold base (labs %.0f%%, hangar %.0f%% warm): a technician in the Boiler Room, Geothermal Taps, a different heat priority, or fewer ships and chambers to heat"),
			GetLabWarmth() * 100.0f, GetHangarWarmth() * 100.0f));
	}
	if (HasHull() && HullPressure >= HullCreakPressure * 0.8f)
	{
		AddIf(FString::Printf(TEXT("Hull pressure %.0f (labs slow at %.0f, airlock seals at %.0f): a technician on Pressure Control, fewer surface trips, or fewer Deep-Water Tanks"),
			HullPressure, HullCreakPressure, HullLeakPressure));
	}
	if (GetActiveResponseCount() > 0)
	{
		AddIf(FString::Printf(TEXT("Earth is responding (%d active): Stealth on ships, Camouflage, a memory-wipe crew, or rotate regions (covert)"), GetActiveResponseCount()));
		AddIf(Project(EDeptProject::AutomatedCoverStories, TEXT("suspicion fades twice as fast")));
	}
	return Fixes;
}

FString FAlienIdleSim::GetApproachReading() const
{
	bool bClear = false;
	const EApproach Style = ClassifyApproach(&bClear);
	if (!bClear)
	{
		return TEXT("Your operation reads as: mixed. Upgrades split across cargo, scanners and stealth give a bit of everything and none of the bonuses.");
	}
	const FApproachDef& Def = Approach(Style);
	const bool bMatches = Def.Thesis == Thesis;
	return FString::Printf(TEXT("Your operation reads as: %s. %s%s"), Def.Name, Def.Strengths,
		bMatches ? TEXT(" It suits this thesis.") : TEXT(" It is not what this thesis was written for."));
}

void FAlienIdleSim::StepAutoContract()
{
	if (!bAutoContracts || bHasActiveContract || !IsAutomationUnlocked() || Offers.Num() == 0)
	{
		return;
	}
	int32 Best = INDEX_NONE;
	for (int32 o = 0; o < Offers.Num(); ++o)
	{
		bool bServable = true;
		for (const FContractNeed& Need : Offers[o].Needs)
		{
			bool bShip = false;
			for (int32 u = 0; u < Ufos.Num(); ++u)
			{
				bShip |= (Ufos[u].bFollowContract || Ufos[u].Destination == Need.Type) && GetTractorStrength(u) >= Specimen(Need.Type).Weight;
			}
			bServable &= bShip && IsSpecimenUnlocked(Need.Type);
		}
		if (bServable && (Best == INDEX_NONE || Offers[o].Reward > Offers[Best].Reward))
		{
			Best = o;
		}
	}
	if (Best != INDEX_NONE)
	{
		AddLog(TEXT("Standing order: the contract office signed the best offer the fleet can serve. Nobody read the small print, as is tradition."));
		AcceptContract(Best);
	}
}

// ---------------------------------------------------------------------------------------------
// Locations: Oceanic hull pressure, local stand-ins, cross-base upgrades, achievements

ESpecimen FAlienIdleSim::Local(ESpecimen Type) const
{
	if (Location == ELocation::Oceanic && Type == ESpecimen::Elephant)
	{
		return ESpecimen::Whale;
	}
	if (Location == ELocation::Antarctic)
	{
		switch (Type)
		{
		case ESpecimen::Squirrel: return ESpecimen::Penguin;
		case ESpecimen::Cow: return ESpecimen::Seal;
		case ESpecimen::Elephant: return ESpecimen::ElephantSeal;
		default: break;
		}
	}
	return Type;
}

ESpecimen FAlienIdleSim::GetDiscoverySpecimen(EDiscovery Type) const
{
	return Type == EDiscovery::None ? ESpecimen::Squirrel : Local(Discovery(Type).Specimen);
}

bool FAlienIdleSim::IsSpecimenAtLocation(ESpecimen Type) const
{
	for (int32 r = 0; r < NumRegions; ++r)
	{
		if (RegionHasSpecimen(r, Type))
		{
			return true;
		}
	}
	return false;
}

bool FAlienIdleSim::IsNative(ESpecimen Type) const
{
	return (Location == ELocation::Oceanic && (Type == ESpecimen::Pufferfish || Type == ESpecimen::Octopus || Type == ESpecimen::Whale))
		|| (Location == ELocation::Antarctic && (Type == ESpecimen::Penguin || Type == ESpecimen::Seal || Type == ESpecimen::ElephantSeal));
}

int32 FAlienIdleSim::GetLocalExperiments() const
{
	int32 Count = 0;
	for (int32 t = 0; t < int32(ESpecimen::Count); ++t)
	{
		Count += IsNative(ESpecimen(t)) ? ExperimentsByType[t] : 0;
	}
	return Count;
}

bool FAlienIdleSim::IsBaseUpgradeAvailable(EBaseUpgrade Upgrade) const
{
	switch (Upgrade)
	{
	case EBaseUpgrade::TidalTurbines:
	case EBaseUpgrade::DeepTanks:
		return Location == ELocation::Oceanic;
	case EBaseUpgrade::GeothermalTaps:
	case EBaseUpgrade::CryoVault:
		return Location == ELocation::Antarctic;
	case EBaseUpgrade::InsulatedLabs:
		return Career.Published[int32(EDiscovery::HuddleTheory)];
	case EBaseUpgrade::CryoPods:
		return Career.Published[int32(EDiscovery::BlubberEconomics)];
	case EBaseUpgrade::SonarArray:
		return Career.Published[int32(EDiscovery::WhaleSong)];
	case EBaseUpgrade::InkScreens:
		return Career.Published[int32(EDiscovery::InkDiplomacy)];
	default:
		return true;
	}
}

float FAlienIdleSim::GetTideFactor() const
{
	return 0.5f + 0.5f * FMath::Sin(2.0f * PI * float(FMath::Fmod(Elapsed, double(TidePeriodSeconds))) / TidePeriodSeconds);
}

float FAlienIdleSim::GetHullStress() const
{
	if (!HasHull())
	{
		return 0.0f;
	}
	return (HullStressPerUnit * GetContainmentUnitsUsed() + HullStressPerTank * GetBaseLevel(EBaseUpgrade::DeepTanks)
		+ HullStressPerTurbine * GetBaseLevel(EBaseUpgrade::TidalTurbines)) * AlienIdleTuning::Thesis(Thesis).HullStressMultiplier;
}

float FAlienIdleSim::GetHullRelief() const
{
	const FAssignment Pumps{ EStation::Pressure, 0 };
	return HullBaseRelief + (IsStationStaffed(Pumps) ? HullCrewRelief * (1.0f + GetStationBonus(Pumps)) : 0.0f);
}

void FAlienIdleSim::StepHull(float Dt)
{
	if (!HasHull())
	{
		return;
	}
	HullPressure = FMath::Clamp(HullPressure + (GetHullStress() - GetHullRelief() * HullPressure / 50.0f) * Dt, 0.0f, 100.0f);
	if (!bHullCreaking && HullPressure >= HullCreakPressure)
	{
		bHullCreaking = true;
		AddLog(TEXT("The hull is creaking. Staff have been asked to creak more quietly in solidarity. Labs 20% slower until pressure drops."));
	}
	else if (bHullCreaking && HullPressure < HullCreakPressure - 5.0f)
	{
		bHullCreaking = false;
		AddLog(TEXT("The hull has stopped creaking. Staff have resumed creaking on their own time."));
	}
	if (!bAirlockSealed && HullPressure >= HullLeakPressure)
	{
		bAirlockSealed = true;
		AddLog(TEXT("Small leak at the surface airlock. Facilities has placed a bucket and a motivational poster. Surface expeditions suspended until pressure drops below 60."));
	}
	else if (bAirlockSealed && HullPressure < HullCreakPressure)
	{
		bAirlockSealed = false;
		AddLog(TEXT("Surface airlock reopened. The bucket has been promoted to 'emergency infrastructure.'"));
	}
}

FString FAlienIdleSim::GetHullText() const
{
	const float Stress = GetHullStress();
	const float Relief = GetHullRelief();
	const float Settles = FMath::Min(100.0f, 50.0f * Stress / FMath::Max(0.01f, Relief));
	const FString State = bAirlockSealed ? FString(TEXT("LEAKING: surface airlock sealed until below 60"))
		: HullPressure >= HullCreakPressure ? FString(TEXT("creaking: labs 20% slower")) : FString(TEXT("holding"));
	return FString::Printf(TEXT("Hull pressure %.0f/100 (%s); settles near %.0f. Load %.2f/s: containment %d units, %d Deep-Water Tank%s, %d turbine%s%s; each surface launch +%.0f. Pumps %.2f%s.")
		TEXT(" Tide %.0f%% (turbines give +%.1f energy/s now)."),
		HullPressure, *State, Settles, Stress, GetContainmentUnitsUsed(), GetBaseLevel(EBaseUpgrade::DeepTanks), GetBaseLevel(EBaseUpgrade::DeepTanks) == 1 ? TEXT("") : TEXT("s"),
		GetBaseLevel(EBaseUpgrade::TidalTurbines), GetBaseLevel(EBaseUpgrade::TidalTurbines) == 1 ? TEXT("") : TEXT("s"),
		AlienIdleTuning::Thesis(Thesis).HullStressMultiplier != 1.0f ? TEXT(", deep posting x1.5") : TEXT(""),
		HullStressPerSurfaceLaunch * AlienIdleTuning::Thesis(Thesis).HullStressMultiplier, Relief,
		IsStationStaffed({ EStation::Pressure, 0 }) ? TEXT(" (Pressure Control staffed)") : TEXT(" (Pressure Control unstaffed: assign a technician)"),
		GetTideFactor() * 100.0f, TidalEnergyPerLevel * GetBaseLevel(EBaseUpgrade::TidalTurbines) * GetTideFactor());
}

void FAlienIdleSim::StepAchievements()
{
	StepTenure();
	if (!Career.LocationUnlocked[int32(ELocation::Oceanic)] && ExperimentsByType[int32(ESpecimen::Elephant)] >= HeavyLiftingElephants)
	{
		Career.LocationUnlocked[int32(ELocation::Oceanic)] = true;
		AddResult(TEXT("ACADEMY ACHIEVEMENT 'Heavy Lifting': you have studied eight elephants in one run. The Academy offers you Trench Station 9 (Oceanic) at your next promotion. It is very wet."));
		AddLog(TEXT("Achievement: Heavy Lifting. Oceanic posting available when you promote, with a new thesis: 'The Deep End'."));
	}
}

void FAlienIdleSim::StepTenure()
{
	if (!Career.LocationUnlocked[int32(ELocation::Antarctic)] && Career.Promotions >= TenureTrackTheses)
	{
		Career.LocationUnlocked[int32(ELocation::Antarctic)] = true;
		AddResult(TEXT("ACADEMY ACHIEVEMENT 'Tenure Track': two theses submitted. The Academy offers you Station Permafrost (Antarctic) at your next promotion. Pack a jumper."));
		AddLog(TEXT("Achievement: Tenure Track. Antarctic posting available when you promote, with a new thesis: 'Cold Storage'."));
	}
}

FString FAlienIdleSim::GetAchievementText() const
{
	TArray<FString> Lines;
	for (const AlienIdleTuning::FAchievementDef& Def : AlienIdleTuning::Achievements)
	{
		const bool bDone = Career.LocationUnlocked[int32(Def.Unlocks)];
		FString Progress;
		if (!bDone && Def.Unlocks == ELocation::Oceanic)
		{
			Progress = FString::Printf(TEXT(" (%d/%d this run)"), FMath::Min(ExperimentsByType[int32(ESpecimen::Elephant)], HeavyLiftingElephants), HeavyLiftingElephants);
		}
		if (!bDone && Def.Unlocks == ELocation::Antarctic)
		{
			Progress = FString::Printf(TEXT(" (%d/%d)"), FMath::Min(Career.Promotions, TenureTrackTheses), TenureTrackTheses);
		}
		Lines.Add(FString::Printf(TEXT("%s '%s': %s%s, unlocks %s"), bDone ? TEXT("[done]") : TEXT("[    ]"), Def.Name, Def.Requirement, *Progress, LocationNames[int32(Def.Unlocks)]));
	}
	return FString::Join(Lines, LINE_TERMINATOR);
}

// ---------------------------------------------------------------------------------------------
// Antarctic: heat distribution and preserved samples

const TCHAR* FAlienIdleSim::GetHeatPriorityName(EHeatPriority Priority)
{
	switch (Priority)
	{
	case EHeatPriority::LabsFirst: return TEXT("labs first");
	case EHeatPriority::HangarFirst: return TEXT("hangar first");
	default: return TEXT("shared evenly");
	}
}

float FAlienIdleSim::GetHeatSupply() const
{
	if (!HasHeat())
	{
		return 0.0f;
	}
	const FAssignment Boiler{ EStation::Boiler, 0 };
	float Heat = HeatBase + (IsStationStaffed(Boiler) ? HeatPerBoilerCrew * (1.0f + GetStationBonus(Boiler)) : 0.0f) + HeatPerTap * GetBaseLevel(EBaseUpgrade::GeothermalTaps);
	Heat *= AlienIdleTuning::Thesis(Thesis).HeatMultiplier;
	return Incident.Type == EIncident::HeatingCut ? Heat * HeatingCutMultiplier : Heat;
}

float FAlienIdleSim::GetHeatDemandLabs() const
{
	return HeatPerChamber * Chambers.Num() * (IsDiscovered(EDiscovery::HuddleTheory) ? HuddleHeatDemand : 1.0f);
}

float FAlienIdleSim::GetHeatDemandHangar() const
{
	return HeatPerUfo * Ufos.Num() * (IsDiscovered(EDiscovery::HuddleTheory) ? HuddleHeatDemand : 1.0f);
}

float FAlienIdleSim::GetLabWarmth() const
{
	if (!HasHeat())
	{
		return 1.0f;
	}
	const float Supply = GetHeatSupply();
	const float Labs = GetHeatDemandLabs();
	const float Hangar = GetHeatDemandHangar();
	float Given = 0.0f;
	switch (HeatPriority)
	{
	case EHeatPriority::LabsFirst: Given = FMath::Min(Labs, Supply); break;
	case EHeatPriority::HangarFirst: Given = FMath::Max(0.0f, Supply - Hangar); break;
	default: Given = Supply * Labs / FMath::Max(0.01f, Labs + Hangar); break;
	}
	return FMath::Clamp(Given / FMath::Max(0.01f, Labs), 0.0f, 1.0f);
}

float FAlienIdleSim::GetHangarWarmth() const
{
	if (!HasHeat())
	{
		return 1.0f;
	}
	const float Supply = GetHeatSupply();
	const float Labs = GetHeatDemandLabs();
	const float Hangar = GetHeatDemandHangar();
	float Given = 0.0f;
	switch (HeatPriority)
	{
	case EHeatPriority::LabsFirst: Given = FMath::Max(0.0f, Supply - Labs); break;
	case EHeatPriority::HangarFirst: Given = FMath::Min(Hangar, Supply); break;
	default: Given = Supply * Hangar / FMath::Max(0.01f, Labs + Hangar); break;
	}
	return FMath::Clamp(Given / FMath::Max(0.01f, Hangar), 0.0f, 1.0f);
}

FString FAlienIdleSim::GetHeatText() const
{
	return FString::Printf(TEXT("Heat %.1f supplied (base %.0f, Boiler Room %s, %d geothermal tap%s%s) for %.1f wanted (labs %.1f, hangar %.1f), %s. Labs %.0f%% warm (speed x%.2f); hangar %.0f%% warm (flights x%.2f, launch energy x%.2f). Containment is the freezer: %d preserved samples studied.%s"),
		GetHeatSupply(), HeatBase, IsStationStaffed({ EStation::Boiler, 0 }) ? TEXT("staffed") : TEXT("UNSTAFFED: assign a technician"),
		GetBaseLevel(EBaseUpgrade::GeothermalTaps), GetBaseLevel(EBaseUpgrade::GeothermalTaps) == 1 ? TEXT("") : TEXT("s"),
		AlienIdleTuning::Thesis(Thesis).HeatMultiplier != 1.0f ? TEXT(", heating budget x0.75") : TEXT(""),
		GetHeatDemandLabs() + GetHeatDemandHangar(), GetHeatDemandLabs(), GetHeatDemandHangar(), GetHeatPriorityName(HeatPriority),
		GetLabWarmth() * 100.0f, ColdLabSpeedFloor + (1.0f - ColdLabSpeedFloor) * GetLabWarmth(),
		GetHangarWarmth() * 100.0f, 1.0f + ColdFlightPenalty * (1.0f - GetHangarWarmth()), 1.0 + ColdLaunchEnergyPenalty * (1.0 - GetHangarWarmth()),
		PreservedStudied, bOverheating ? TEXT(" OVERHEATING: heat is wasted and the meltwater is noticed (the humans blame cow farts).") : TEXT(""));
}

void FAlienIdleSim::StepOverheat()
{
	if (!HasHeat())
	{
		return;
	}
	const float Demand = GetHeatDemandLabs() + GetHeatDemandHangar();
	const bool bHot = GetHeatSupply() > Demand * OverheatRatio;
	if (bHot && !bOverheating)
	{
		static const TCHAR* Jokes[] = {
			TEXT("The base is overheating. The ice shelf outside is now a puddle with ambitions. Human climate scientists have blamed it on cow farts. There are no cows here. We have not corrected them."),
			TEXT("Too much heat: the penguins are wearing sunglasses. The Research Outpost has published a paper blaming the thaw on cow farts. Peer review consisted of one cow, who abstained."),
			TEXT("Overheating again. A human documentary crew has filmed the melting and blamed 'the global cow-fart crisis.' Our steam plume has been credited as 'a very large cow.'"),
			TEXT("The Boiler Room is running hot. Staff are sunbathing in the Cryo Vault. Earth's newspapers blame the warm spell on cow farts, which Facilities has adopted as the official explanation."),
		};
		AddLog(Jokes[OverheatJokes++ % UE_ARRAY_COUNT(Jokes)]);
		// A small, visible cost: the meltwater is noticed near the outpost.
		AddSuspicion(1, 4.0, ESpecimen::Human);
	}
	else if (!bHot && bOverheating)
	{
		AddLog(TEXT("The base has cooled to a sensible temperature. The humans are still blaming the cows."));
	}
	bOverheating = bHot;
}
