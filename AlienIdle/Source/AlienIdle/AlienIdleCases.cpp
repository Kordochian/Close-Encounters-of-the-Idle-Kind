#include "AlienIdleSim.h"

using namespace AlienIdleTuning;

// "The Specimens Remember": a small roster of recurring subjects. Most captures stay ordinary; now and
// then (or on purpose, with a scanner) one of five people turns up, gets a case file, and remembers us.
// Encounter history and case-file discoveries live in FCareer and survive promotions; who is on base and
// this run's case studies do not.

namespace
{
	/** The other subject in a relationship, or INDEX_NONE if Case is not part of it. */
	int32 OtherInRelation(const FCaseRelationDef& Relation, int32 Case)
	{
		return Relation.A == Case ? Relation.B : Relation.B == Case ? Relation.A : INDEX_NONE;
	}

	const TCHAR* RequirementText(ECaseRequirement Requirement)
	{
		switch (Requirement)
		{
		case ECaseRequirement::Returning: return TEXT("needs a return visit");
		case ECaseRequirement::SecondRun: return TEXT("needs them on a second assignment");
		default: return TEXT("any visit");
		}
	}
}

FString FAlienIdleSim::GetCaseName(int32 Case)
{
	return (Case >= 0 && Case < NumCaseSubjects) ? FString(CaseSubjects[Case].Name) : FString();
}

bool FAlienIdleSim::IsCaseInContainment(int32 Case) const
{
	return Containment.ContainsByPredicate([Case](const FSpecimen& S) { return S.Case == Case; });
}

bool FAlienIdleSim::IsCaseOnBase(int32 Case) const
{
	if (Case < 0 || IsCaseInContainment(Case))
	{
		return Case >= 0;
	}
	for (const FLabChamber& Chamber : Chambers)
	{
		if (Chamber.bRunning && (Chamber.Specimen.Case == Case || Chamber.Partner.Case == Case))
		{
			return true;
		}
	}
	for (const FUfo& Ufo : Ufos)
	{
		if (Ufo.Cargo.ContainsByPredicate([Case](const FSpecimen& S) { return S.Case == Case; }))
		{
			return true;
		}
	}
	return false;
}

int32 FAlienIdleSim::GetNextCaseStep(int32 Case) const
{
	const int32 Step = CaseStepsDone[Case];
	if (Step >= CaseSteps)
	{
		return INDEX_NONE;
	}
	const FCaseFile& File = Career.Cases[Case];
	const FCaseStepDef& Def = CaseSubjects[Case].Steps[Step];
	if (Def.Research <= 0.0 && File.bDiscovered)
	{
		return INDEX_NONE; // the discovery is already in the file
	}
	switch (Def.Requirement)
	{
	case ECaseRequirement::Returning: return File.Encounters >= 2 ? Step : INDEX_NONE;
	case ECaseRequirement::SecondRun: return File.RunsMet >= 2 ? Step : INDEX_NONE;
	default: return Step;
	}
}

bool FAlienIdleSim::IsKindredHeld(int32 Case) const
{
	for (const FCaseRelationDef& Relation : CaseRelations)
	{
		const int32 Other = OtherInRelation(Relation, Case);
		if (Relation.Type == ECaseRelation::Kindred && Other != INDEX_NONE)
		{
			bool bHere = IsCaseInContainment(Other);
			for (const FLabChamber& Chamber : Chambers)
			{
				bHere |= Chamber.bRunning && (Chamber.Specimen.Case == Other || Chamber.Partner.Case == Other);
			}
			if (bHere)
			{
				return true;
			}
		}
	}
	return false;
}

double FAlienIdleSim::GetCaseResearchMultiplier(const FSpecimen& Subject) const
{
	if (Subject.Case < 0)
	{
		return 1.0;
	}
	// They recognise the equipment and produce better research...
	const int32 Earlier = FMath::Clamp(Career.Cases[Subject.Case].Encounters - 1, 0, CaseMaxEncounterEffect);
	double Multiplier = 1.0 + CaseResearchPerEncounter * Earlier;
	// ...and they work better in good company.
	if (IsKindredHeld(Subject.Case))
	{
		Multiplier *= KindredResearchMultiplier;
	}
	return Multiplier;
}

bool FAlienIdleSim::IsWaitingForPartner(const FSpecimen& Subject) const
{
	if (Subject.Case < 0 || bPeerReviewDone)
	{
		return false;
	}
	for (const FCaseRelationDef& Relation : CaseRelations)
	{
		const int32 Other = OtherInRelation(Relation, Subject.Case);
		if (Relation.Type != ECaseRelation::Rivals || Other == INDEX_NONE)
		{
			continue;
		}
		if (IsCaseOnBase(Other))
		{
			return true;
		}
		for (int32 u = 0; u < Ufos.Num(); ++u)
		{
			const FUfo& Ufo = Ufos[u];
			const bool bSought = Ufo.State == EUfoState::Docked ? (Ufo.CaseTarget == Other && GetShipLevel(u, EShipSystem::Scanner) > 0) : Ufo.TripCase == Other;
			if (bSought)
			{
				return true;
			}
		}
	}
	return false;
}

int32 FAlienIdleSim::RollCaseAppearance(const TArray<FSpecimen>& Taken)
{
	if (!bCaseAppearances || CaseRng.FRand() >= CaseAppearChance)
	{
		return INDEX_NONE;
	}
	TArray<int32> Candidates;
	for (int32 c = 0; c < NumCaseSubjects; ++c)
	{
		if (!IsCaseOnBase(c) && !Taken.ContainsByPredicate([c](const FSpecimen& S) { return S.Case == c; }))
		{
			Candidates.Add(c);
		}
	}
	return Candidates.Num() > 0 ? Candidates[CaseRng.RandRange(0, Candidates.Num() - 1)] : INDEX_NONE;
}

void FAlienIdleSim::MeetCase(FSpecimen& Subject, int32 UfoIndex)
{
	const int32 Case = Subject.Case;
	const FCaseSubjectDef& Def = CaseSubjects[Case];
	FCaseFile& File = Career.Cases[Case];
	++File.Encounters;
	if (File.LastRun != Career.Run)
	{
		File.LastRun = Career.Run;
		++File.RunsMet;
	}
	const int32 Region = FMath::Clamp(int32(Subject.Region), 0, NumRegions - 1);
	CaseVisit[Case] = FString::Printf(TEXT("taken from %s"), *GetRegionNameAt(Region));

	if (File.Encounters == 1)
	{
		AddEncyclopediaEntry(FString::Printf(TEXT("Case File: %s"), Def.Name), Def.Profile);
		AddResult(FString::Printf(TEXT("NEW CASE FILE: %s, %s. %s They will remember this."), Def.Name, Def.Title, Def.Profile));
	}
	else
	{
		AddResult(FString::Printf(TEXT("RETURNING SUBJECT: %s (encounter %d). %s Research x%.2f, but harder to fool."),
			Def.Name, File.Encounters, Def.ReturnLine, GetCaseResearchMultiplier(Subject)));
	}

	// Colleagues notice when someone they know goes missing.
	for (const FCaseRelationDef& Relation : CaseRelations)
	{
		const int32 Other = OtherInRelation(Relation, Case);
		if (Relation.Type == ECaseRelation::Colleagues && Other != INDEX_NONE && Career.Cases[Other].Encounters > 0)
		{
			AddPressure(Region, EResponse::Investigators, ColleagueInvestigatorPressure);
			AddSuspicion(Region, ColleagueSuspicion, ESpecimen::Human);
			AddLog(FString::Printf(TEXT("%s has noticed %s is missing again and started asking questions in %s. Investigators +%.0f there."),
				CaseSubjects[Other].Name, Def.Name, *GetRegionNameAt(Region), ColleagueInvestigatorPressure));
		}
	}

	if (Ufos.IsValidIndex(UfoIndex) && Ufos[UfoIndex].CaseTarget == Case)
	{
		Ufos[UfoIndex].CaseTarget = -1;
		AddLog(FString::Printf(TEXT("'%s' found %s. The scanner has returned to judging job titles."), *Ufos[UfoIndex].Name, Def.Name));
	}
}

void FAlienIdleSim::AddCaseVisit(int32 Case, const FString& Text)
{
	CaseVisit[Case] += CaseVisit[Case].IsEmpty() ? Text : TEXT("; ") + Text;
}

void FAlienIdleSim::CloseCaseVisit(int32 Case, const FString& Ending)
{
	AddCaseVisit(Case, Ending);
	TArray<FString>& History = Career.Cases[Case].History;
	History.Add(FString::Printf(TEXT("Run %d, %s: %s."), Career.Run, LocationNames[int32(Location)], *CaseVisit[Case]));
	while (History.Num() > MaxCaseHistory)
	{
		History.RemoveAt(0);
	}
	CaseVisit[Case].Empty();
}

void FAlienIdleSim::NoteCaseReturn(const FSpecimen& Subject, bool bWiped)
{
	if (Subject.Case < 0)
	{
		return;
	}
	const int32 Encounters = Career.Cases[Subject.Case].Encounters;
	if (bWiped && Encounters > 1)
	{
		// The memory wipe works less every time.
		AddSuspicion(Subject.Region, CaseWipeSuspicionPerEncounter * FMath::Min(Encounters - 1, CaseMaxEncounterEffect), Subject.Type);
		AddLog(FString::Printf(TEXT("Memory wipe unsuccessful. %s remembers everything but has elected to forget the mandatory orientation."), CaseSubjects[Subject.Case].Short));
	}
	CloseCaseVisit(Subject.Case, !bWiped ? TEXT("returned unwiped") : Encounters > 1 ? TEXT("returned; the memory wipe did not take") : TEXT("memory-wiped and returned"));
}

void FAlienIdleSim::NoteCaseEscape(const FSpecimen& Subject)
{
	if (Subject.Case < 0)
	{
		return;
	}
	AddLog(FString::Printf(TEXT("%s walked out during orientation. They know the way by now."), CaseSubjects[Subject.Case].Name));
	CloseCaseVisit(Subject.Case, TEXT("escaped"));
}

bool FAlienIdleSim::TryStartPeerReview(FLabChamber& Chamber)
{
	if (bPeerReviewDone || Energy < PeerReviewEnergy)
	{
		return false;
	}
	for (const FCaseRelationDef& Relation : CaseRelations)
	{
		if (Relation.Type != ECaseRelation::Rivals)
		{
			continue;
		}
		const int32 A = Containment.IndexOfByPredicate([&Relation](const FSpecimen& S) { return S.Case == Relation.A; });
		const int32 B = Containment.IndexOfByPredicate([&Relation](const FSpecimen& S) { return S.Case == Relation.B; });
		if (A == INDEX_NONE || B == INDEX_NONE)
		{
			continue;
		}
		Energy -= PeerReviewEnergy;
		Chamber.bRunning = true;
		Chamber.Time = 0.0f;
		Chamber.Specimen = Containment[A];
		Chamber.Partner = Containment[B];
		Chamber.Experiment = FExperimentChoice();
		Chamber.Experiment.Kind = EExperimentKind::CaseStudy;
		Chamber.Experiment.CaseStep = PeerReviewStep;
		Chamber.Batch = 1;
		Chamber.Duration = GetExperimentSeconds(Chamber.Specimen, Chamber.Experiment);
		Containment.RemoveAt(FMath::Max(A, B));
		Containment.RemoveAt(FMath::Min(A, B));
		AddLog(FString::Printf(TEXT("Peer Review begins: %s and %s have been placed in one chamber with one microphone."), CaseSubjects[Relation.A].Name, CaseSubjects[Relation.B].Name));
		return true;
	}
	return false;
}

void FAlienIdleSim::CompleteCaseStudy(const FSpecimen& Subject, const FSpecimen& Partner, uint8 Step)
{
	FExperimentChoice Choice;
	Choice.Kind = EExperimentKind::CaseStudy;
	Choice.CaseStep = Step;
	const double Earned = GetExperimentResearch(Subject, Choice);
	Research += Earned;
	ResearchEarned += Earned;

	if (Step >= PeerReviewStep)
	{
		bPeerReviewDone = true;
		const FString Report = FString::Printf(TEXT("%s and %s reviewed each other's work for an hour. Both papers were rejected. Both reviewers were right."),
			*GetSpecimenLabel(Subject), *GetSpecimenLabel(Partner));
		if (!Career.bPeerReviewed)
		{
			Career.bPeerReviewed = true;
			AddEncyclopediaEntry(TEXT("Case File: Peer Review"), Report);
		}
		AddResult(FString::Printf(TEXT("PEER REVIEW #%d: %s Research +%.0f."), TotalExperiments, *Report, Earned));
		for (const FSpecimen& Who : { Subject, Partner })
		{
			if (Who.Case >= 0)
			{
				AddCaseVisit(Who.Case, TEXT("Peer Review"));
			}
		}
		return;
	}

	const int32 Case = Subject.Case;
	const FCaseSubjectDef& Def = CaseSubjects[Case];
	const FCaseStepDef& StepDef = Def.Steps[Step];
	FCaseFile& File = Career.Cases[Case];
	CaseStepsDone[Case] = FMath::Max(CaseStepsDone[Case], int32(Step) + 1);
	File.BestSteps = FMath::Max(File.BestSteps, CaseStepsDone[Case]);
	AddCaseVisit(Case, StepDef.Name);
	if (StepDef.Research <= 0.0)
	{
		File.bDiscovered = true;
		++RunCaseDiscoveries;
		AddResult(FString::Printf(TEXT("CASE-FILE DISCOVERY: '%s' (%s). %s (%s)"), Def.DiscoveryName, Def.Name, StepDef.Report, Def.DiscoveryEffect));
		AddEncyclopediaEntry(FString::Printf(TEXT("Case Discovery: %s"), Def.DiscoveryName), FString::Printf(TEXT("%s %s"), StepDef.Report, Def.DiscoveryEffect));
		return;
	}
	AddResult(FString::Printf(TEXT("Case study #%d (%s, '%s'): %s Research +%.0f."), TotalExperiments, Def.Name, StepDef.Name, StepDef.Report, Earned));
}

void FAlienIdleSim::StepCaseDynamics(float Dt)
{
	for (int32 r = 0; r < NumCaseRelations; ++r)
	{
		const FCaseRelationDef& Relation = CaseRelations[r];
		if (Relation.Type == ECaseRelation::Kindred)
		{
			continue;
		}
		float& Timer = CaseTrouble[r];
		if (Timer < 0.0f)
		{
			Timer = FMath::Min(0.0f, Timer + Dt); // they need a while before the next row
			continue;
		}
		const bool bTogether = IsCaseInContainment(Relation.A) && IsCaseInContainment(Relation.B);
		Timer = bTogether ? Timer + Dt : 0.0f;
		if (Timer < CaseTroubleSeconds || Incident.Type != EIncident::None)
		{
			continue;
		}
		Timer = -CaseTroubleRepeatSeconds;
		if (Relation.Type == ECaseRelation::Colleagues)
		{
			StartIncident(EIncident::ContainmentNewsletter, NewsletterSeconds);
			const int32 Index = Containment.IndexOfByPredicate([&Relation](const FSpecimen& S) { return S.Case == Relation.A; });
			if (Index != INDEX_NONE)
			{
				AddSuspicion(Containment[Index].Region, NewsletterSuspicion, ESpecimen::Human);
			}
			AddLog(FString::Printf(TEXT("%s and %s, held in the same block, have started a containment newsletter. Issue 1: 'Is The Food Getting Worse?' Labs slower while staff read it."),
				CaseSubjects[Relation.A].Short, CaseSubjects[Relation.B].Short));
		}
		else
		{
			StartIncident(EIncident::AcademicDispute, DisputeSeconds);
			AddLog(FString::Printf(TEXT("%s and %s, left waiting together, have started an academic dispute. Staff have been asked to take sides. Labs slower."),
				CaseSubjects[Relation.A].Short, CaseSubjects[Relation.B].Short));
		}
	}
}

TArray<FString> FAlienIdleSim::GetCaseRelationText(int32 Case) const
{
	TArray<FString> Lines;
	for (const FCaseRelationDef& Relation : CaseRelations)
	{
		const int32 Other = OtherInRelation(Relation, Case);
		if (Other == INDEX_NONE || (!IsCaseKnown(Case) && !IsCaseKnown(Other)))
		{
			continue;
		}
		const FString OtherName = IsCaseKnown(Other) ? FString(CaseSubjects[Other].Name) : FString::Printf(TEXT("an unidentified lead (%s)"), CaseSubjects[Other].Lead);
		FString Live;
		if (Relation.Type == ECaseRelation::Colleagues && IsCaseKnown(Other) && !IsCaseOnBase(Case))
		{
			Live = TEXT(" Taking them now WILL send investigators.");
		}
		else if (Relation.Type == ECaseRelation::Rivals && !bPeerReviewDone)
		{
			Live = IsCaseOnBase(Case) && IsCaseOnBase(Other) ? TEXT(" Both on base: Peer Review is ready.") : TEXT(" Peer Review not done this run.");
		}
		else if (Relation.Type == ECaseRelation::Kindred && IsKindredHeld(Case) && IsCaseOnBase(Case))
		{
			Live = TEXT(" Together now: research x1.3.");
		}
		Lines.Add(FString::Printf(TEXT("%s with %s: %s%s"), CaseRelationNames[int32(Relation.Type)], *OtherName, Relation.Text, *Live));
	}
	return Lines;
}

FString FAlienIdleSim::GetCaseFileText(int32 Case) const
{
	const FCaseSubjectDef& Def = CaseSubjects[Case];
	const FCaseFile& File = Career.Cases[Case];
	if (!IsCaseKnown(Case))
	{
		return FString::Printf(TEXT("Unidentified lead: %s. A Trait Scanner on a human trip can find them (or they may turn up)."), Def.Lead);
	}
	FString Where = TEXT("at large");
	if (IsCaseInContainment(Case))
	{
		Where = IsWaitingForPartner({ ESpecimen::Human, Def.Occupation, 0, 0.0f, int8(Case) }) ? TEXT("in containment, waiting for their rival") : TEXT("in containment");
	}
	else if (IsCaseOnBase(Case))
	{
		Where = TEXT("on base (lab or ship)");
	}
	else
	{
		for (const FUfo& Ufo : Ufos)
		{
			if (Ufo.CaseTarget == Case || (Ufo.State != EUfoState::Docked && Ufo.TripCase == Case))
			{
				Where = FString::Printf(TEXT("at large, sought by '%s'"), *Ufo.Name);
				break;
			}
		}
	}
	const int32 Earlier = FMath::Clamp(File.Encounters, 0, CaseMaxEncounterEffect);
	FString Text = FString::Printf(TEXT("%s, %s. Encounters %d on %d assignment%s; %s."), Def.Name, Def.Title, File.Encounters, File.RunsMet,
		File.RunsMet == 1 ? TEXT("") : TEXT("s"), *Where);
	Text += FString::Printf(TEXT("%sNext time: research x%.2f, escapes x%.1f, scanning +%.0f%% (they remember us)."), LINE_TERMINATOR,
		1.0 + CaseResearchPerEncounter * Earlier, 1.0 + CaseEscapePerEncounter * Earlier, 100.0f * CaseCapturePerEncounter * Earlier);
	TArray<FString> Chain;
	for (int32 s = 0; s < CaseSteps; ++s)
	{
		const FCaseStepDef& Step = Def.Steps[s];
		const bool bDone = s < CaseStepsDone[Case] || (Step.Research <= 0.0 && File.bDiscovered);
		Chain.Add(FString::Printf(TEXT("%s [%s]"), Step.Name, bDone ? TEXT("done") : RequirementText(Step.Requirement)));
	}
	Text += FString::Printf(TEXT("%sResearch chain: %s -> '%s'%s: %s"), LINE_TERMINATOR, *FString::Join(Chain, TEXT(" > ")), Def.DiscoveryName,
		File.bDiscovered ? TEXT(" (DISCOVERED)") : TEXT(""), Def.DiscoveryEffect);
	if (File.History.Num() > 0)
	{
		Text += FString::Printf(TEXT("%sLast seen: %s"), LINE_TERMINATOR, *File.History.Last());
	}
	return Text;
}

bool FAlienIdleSim::CanTargetCase(int32 UfoIndex, int32 Case, FString* OutReason) const
{
	auto Fail = [OutReason](const FString& Reason)
	{
		if (OutReason)
		{
			*OutReason = Reason;
		}
		return false;
	};
	if (!Ufos.IsValidIndex(UfoIndex) || Case < 0 || Case >= NumCaseSubjects)
	{
		return Fail(TEXT("no such ship or subject"));
	}
	if (!IsSpecimenUnlocked(ESpecimen::Human))
	{
		return Fail(TEXT("humans are not yet approved for study"));
	}
	if (GetShipLevel(UfoIndex, EShipSystem::Scanner) == 0)
	{
		return Fail(FString::Printf(TEXT("'%s' needs a Trait Scanner"), *Ufos[UfoIndex].Name));
	}
	if (IsCaseOnBase(Case))
	{
		return Fail(TEXT("already on base"));
	}
	return true;
}

void FAlienIdleSim::SetCaseTarget(int32 UfoIndex, int32 Case)
{
	if (!Ufos.IsValidIndex(UfoIndex))
	{
		return;
	}
	if (Case < 0)
	{
		Ufos[UfoIndex].CaseTarget = -1;
		return;
	}
	if (!CanTargetCase(UfoIndex, Case))
	{
		return;
	}
	Ufos[UfoIndex].CaseTarget = int8(Case);
	SetDestinationBest(UfoIndex, ESpecimen::Human);
	AddLog(FString::Printf(TEXT("'%s' will scan %s for %s. The pilot has been given a photograph and told not to make it weird."),
		*Ufos[UfoIndex].Name, *GetRegionNameAt(Ufos[UfoIndex].Region), IsCaseKnown(Case) ? CaseSubjects[Case].Name : CaseSubjects[Case].Lead));
}

FString FAlienIdleSim::GetCaseTargetText(int32 UfoIndex) const
{
	const FUfo& Ufo = Ufos[UfoIndex];
	if (Ufo.CaseTarget < 0)
	{
		return TEXT("Case target: none. Pick a person of interest in CASE FILES to scan for them.");
	}
	const FCaseFile& File = Career.Cases[Ufo.CaseTarget];
	const FString Who = IsCaseKnown(Ufo.CaseTarget) ? FString(CaseSubjects[Ufo.CaseTarget].Name) : FString::Printf(TEXT("the lead (%s)"), CaseSubjects[Ufo.CaseTarget].Lead);
	return FString::Printf(TEXT("Case target: %s on the next human trip (capture +%.0f%%: they remember us)%s"), *Who,
		100.0f * CaseCapturePerEncounter * FMath::Min(File.Encounters, CaseMaxEncounterEffect),
		GetShipLevel(UfoIndex, EShipSystem::Scanner) == 0 ? TEXT(". Needs a Trait Scanner.") : TEXT("."));
}

void FAlienIdleSim::DevAddCaseSubject(int32 Case, int32 Region)
{
	FSpecimen Subject;
	Subject.Type = ESpecimen::Human;
	Subject.Occupation = CaseSubjects[Case].Occupation;
	Subject.Region = uint8(Region >= 0 ? Region : GetHomeRegion(ESpecimen::Human));
	Subject.Case = int8(Case);
	RecordEncyclopedia(Subject);
	MeetCase(Subject, INDEX_NONE);
	Containment.Add(Subject);
}
