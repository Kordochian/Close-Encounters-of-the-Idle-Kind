#include "AlienIdleSim.h"

using namespace AlienIdleTuning;

// Probe Calibration: now and then a running experiment offers a short timing challenge. Stopping the
// cursor in the band skips part of the remaining time; any attempt helps a little; ignoring it costs
// nothing. One window at a time across all chambers with a shared cooldown, so there is no bonus to farm
// by cycling labs. The Calibration Desk adds a technician who takes the windows the player leaves alone,
// for a weaker boost that also works while nobody is watching.

namespace
{
	const TCHAR* PreciseLines[] = {
		TEXT("Calibration successful. Previous results have been retroactively declared accurate."),
		TEXT("Needle dead centre. Three earlier findings have been upgraded from 'guess' to 'law'."),
		TEXT("Perfect alignment. The probe has been promoted above two of its operators."),
		TEXT("Flawless. The error bars have been sent home early with full pay."),
	};
	const TCHAR* SteadyLines[] = {
		TEXT("Acceptable precision. Subject disagreement has been excluded from the dataset."),
		TEXT("Within tolerance. The tolerance was widened beforehand, to be sure."),
		TEXT("Close enough. The margin of error has been reclassified as a margin of confidence."),
		TEXT("Adequate. The subject's objections have been filed under 'noise'."),
	};
	const TCHAR* MissedLines[] = {
		TEXT("Calibration missed. The probe was recalibrated by hitting it, which also counts."),
		TEXT("Off target. The readings have been rounded toward the conclusion."),
		TEXT("Outside the band. The experiment was told it did well anyway and sped up out of politeness."),
		TEXT("No stop recorded. The probe calibrated itself out of spite. Some progress was made."),
	};
	const TCHAR* QuickLines[] = {
		TEXT("Standard calibration: the probe was switched off and on again. Science agrees this helps."),
		TEXT("Standard calibration applied from the manual. Nobody read the manual; it was applied anyway."),
		TEXT("Standard calibration. The subject was asked to hold still and, remarkably, did."),
	};
	const TCHAR* TechnicianLines[] = {
		TEXT("calibrated it unprompted. Adequate. They would like this noted in their file."),
		TEXT("calibrated it by ear. The probe hummed back. Nobody asked what it said."),
		TEXT("calibrated it during their break, and has asked for the break back."),
	};

	template <int32 Num>
	const TCHAR* PickCalibrationLine(const TCHAR* (&Lines)[Num], int32 Index)
	{
		return Lines[FMath::Abs(Index) % Num];
	}
}

float FAlienIdleSim::GetCalibrationFraction(ECalibrationGrade Grade)
{
	return CalibrationFraction[FMath::Clamp(int32(Grade), 0, int32(ECalibrationGrade::Count) - 1)];
}

const TCHAR* FAlienIdleSim::GetCalibrationGradeName(ECalibrationGrade Grade)
{
	switch (Grade)
	{
	case ECalibrationGrade::Precise: return TEXT("precise");
	case ECalibrationGrade::Steady: return TEXT("steady");
	case ECalibrationGrade::Quick: return TEXT("standard");
	case ECalibrationGrade::Technician: return TEXT("technician");
	default: return TEXT("missed");
	}
}

float FAlienIdleSim::GetTechnicianFraction() const
{
	const FAssignment Desk{ EStation::Calibration, 0 };
	if (GetBaseLevel(EBaseUpgrade::CalibrationDesk) <= 0 || !IsStationStaffed(Desk))
	{
		return 0.0f;
	}
	return FMath::Min(CalibrationTechnicianMax, GetCalibrationFraction(ECalibrationGrade::Technician) + CalibrationTechnicianPerBonus * GetStationBonus(Desk));
}

float FAlienIdleSim::GetCalibrationOfferRemaining() const
{
	if (CalibrationChamber == INDEX_NONE)
	{
		return 0.0f;
	}
	return bCalibrationOpen ? CalibrationOfferSeconds : FMath::Max(0.0f, CalibrationOfferSeconds - CalibrationOfferAge);
}

void FAlienIdleSim::OfferCalibration(int32 Chamber)
{
	CalibrationChamber = Chamber;
	CalibrationOfferAge = 0.0f;
	bCalibrationOpen = false;
	CalibrationBand = CalibrationRng.FRandRange(CalibrationBandMin, 1.0f - CalibrationBandMin);
	if (!bAnnouncedCalibration)
	{
		bAnnouncedCalibration = true;
		AddLog(FString::Printf(TEXT("Calibration available in chamber %d. Click it in the lab or in the console for a five-second probe calibration that speeds up the experiment. Ignoring it is also fine, as with most of management's suggestions."),
			Chamber + 1));
	}
}

bool FAlienIdleSim::DevOfferCalibration(int32 Chamber)
{
	if (Chamber == INDEX_NONE)
	{
		float Best = 0.0f;
		for (int32 c = 0; c < Chambers.Num(); ++c)
		{
			const float Left = Chambers[c].Duration - Chambers[c].Time;
			if (Chambers[c].bRunning && Left > Best)
			{
				Best = Left;
				Chamber = c;
			}
		}
	}
	if (!Chambers.IsValidIndex(Chamber) || !Chambers[Chamber].bRunning)
	{
		return false;
	}
	OfferCalibration(Chamber);
	return true;
}

void FAlienIdleSim::StepCalibration(float Dt)
{
	if (!bAnnouncedDesk && IsBaseUpgradeAvailable(EBaseUpgrade::CalibrationDesk))
	{
		bAnnouncedDesk = true;
		AddLog(FString::Printf(TEXT("Calibration Desk approved (%.0f research, under Base). A technician there takes the calibration windows you are too busy for. They are slower about it, and smug."),
			GetBaseUpgradeCost(EBaseUpgrade::CalibrationDesk)));
	}
	if (!bCalibrationEnabled)
	{
		return;
	}

	if (CalibrationChamber != INDEX_NONE)
	{
		CalibrationOfferAge += Dt;
		if (bCalibrationOpen)
		{
			// The challenge is on screen. If the screen went away without an answer, let the window run on.
			if (CalibrationOfferAge > CalibrationChallengeSeconds * 3.0f)
			{
				bCalibrationOpen = false;
				CalibrationOfferAge = 0.0f;
			}
			return;
		}
		if (!Chambers.IsValidIndex(CalibrationChamber) || !Chambers[CalibrationChamber].bRunning)
		{
			CalibrationChamber = INDEX_NONE;
			CalibrationTimer = CalibrationRetrySeconds;
			return;
		}
		if (CalibrationOfferAge >= CalibrationTechnicianDelay && GetTechnicianFraction() > 0.0f)
		{
			Calibrate(ECalibrationGrade::Technician);
			return;
		}
		if (CalibrationOfferAge >= CalibrationOfferSeconds)
		{
			// Ignored: research carries on exactly as before.
			CalibrationChamber = INDEX_NONE;
			CalibrationTimer = CalibrationRetrySeconds;
		}
		return;
	}

	CalibrationTimer -= Dt;
	if (CalibrationTimer > 0.0f)
	{
		return;
	}
	int32 Best = INDEX_NONE;
	float BestLeft = CalibrationMinRemaining;
	for (int32 c = 0; c < Chambers.Num(); ++c)
	{
		const FLabChamber& Chamber = Chambers[c];
		const float Left = Chamber.Duration - Chamber.Time;
		if (Chamber.bRunning && Left >= BestLeft && IsStationStaffed({ EStation::Chamber, c }))
		{
			Best = c;
			BestLeft = Left;
		}
	}
	if (Best == INDEX_NONE)
	{
		CalibrationTimer = 5.0f; // look again shortly
		return;
	}
	OfferCalibration(Best);
}

bool FAlienIdleSim::BeginCalibration()
{
	if (CalibrationChamber == INDEX_NONE || bCalibrationOpen || !Chambers.IsValidIndex(CalibrationChamber) || !Chambers[CalibrationChamber].bRunning)
	{
		return false;
	}
	bCalibrationOpen = true;
	CalibrationOfferAge = 0.0f;
	return true;
}

ECalibrationGrade FAlienIdleSim::GradeCalibration(float Cursor) const
{
	const float Distance = FMath::Abs(Cursor - CalibrationBand);
	return Distance <= CalibrationCoreHalfWidth ? ECalibrationGrade::Precise
		: Distance <= CalibrationBandHalfWidth ? ECalibrationGrade::Steady
		: ECalibrationGrade::Missed;
}

bool FAlienIdleSim::Calibrate(ECalibrationGrade Grade, FString* OutResult)
{
	auto Fail = [OutResult](const FString& Reason)
	{
		if (OutResult)
		{
			*OutResult = Reason;
		}
		return false;
	};
	if (CalibrationChamber == INDEX_NONE || !Chambers.IsValidIndex(CalibrationChamber))
	{
		bCalibrationOpen = false;
		return Fail(TEXT("No calibration window is open. The probe is calibrated enough, says the probe."));
	}
	const int32 Index = CalibrationChamber;
	FLabChamber& Chamber = Chambers[Index];
	CalibrationChamber = INDEX_NONE;
	bCalibrationOpen = false;
	if (!Chamber.bRunning)
	{
		CalibrationTimer = CalibrationRetrySeconds;
		return Fail(TEXT("The experiment finished before the probe did. The results were accepted out of fatigue."));
	}

	const bool bTechnician = Grade == ECalibrationGrade::Technician;
	const float Fraction = bTechnician ? GetTechnicianFraction() : GetCalibrationFraction(Grade);
	const float Skipped = FMath::Max(0.0f, Chamber.Duration - Chamber.Time) * Fraction;
	Chamber.Time += Skipped;
	CalibrationSecondsSaved += Skipped;
	CalibrationTimer = CalibrationCooldownSeconds;

	FString Line;
	if (bTechnician)
	{
		FString Who = TEXT("A technician");
		for (int32 i = 0; i < Crew.Num(); ++i)
		{
			if (IsOnDuty(i) && Crew[i].Assignment.Station == EStation::Calibration)
			{
				Who = Crew[i].Name;
				break;
			}
		}
		Line = FString::Printf(TEXT("%s %s"), *Who, PickCalibrationLine(TechnicianLines, TechnicianCalibrations));
		++TechnicianCalibrations;
	}
	else
	{
		switch (Grade)
		{
		case ECalibrationGrade::Precise: Line = PickCalibrationLine(PreciseLines, ManualCalibrations); break;
		case ECalibrationGrade::Steady: Line = PickCalibrationLine(SteadyLines, ManualCalibrations); break;
		case ECalibrationGrade::Quick: Line = PickCalibrationLine(QuickLines, ManualCalibrations); break;
		default: Line = PickCalibrationLine(MissedLines, ManualCalibrations); break;
		}
		++ManualCalibrations;
	}

	const FString Text = FString::Printf(TEXT("Probe calibration, chamber %d (%s): %.0f%% of the remaining time skipped (%.1f s of work). %s"),
		Index + 1, GetCalibrationGradeName(Grade), 100.0f * Fraction, Skipped, *Line);
	AddResult(Text);
	if (OutResult)
	{
		*OutResult = Text;
	}
	return true;
}

FString FAlienIdleSim::GetCalibrationStatusText() const
{
	if (!bCalibrationEnabled)
	{
		return TEXT("Probe calibration: off.");
	}
	const FString Desk = GetTechnicianFraction() > 0.0f
		? FString::Printf(TEXT(" Calibration Desk staffed: windows left for %.0f s are taken at %.0f%%."), CalibrationTechnicianDelay, 100.0f * GetTechnicianFraction())
		: FString();
	if (CalibrationChamber != INDEX_NONE)
	{
		return FString::Printf(TEXT("Probe calibration: window open in chamber %d (%.0f s left). Precise %.0f%%, steady or standard %.0f%%, any attempt %.0f%% of the remaining time.%s"),
			CalibrationChamber + 1, GetCalibrationOfferRemaining(), 100.0f * GetCalibrationFraction(ECalibrationGrade::Precise),
			100.0f * GetCalibrationFraction(ECalibrationGrade::Steady), 100.0f * GetCalibrationFraction(ECalibrationGrade::Missed), *Desk);
	}
	return FString::Printf(TEXT("Probe calibration: next window in about %.0f s, on an experiment with at least %.0f s to go (shared by all chambers). %d by hand, %d by technicians, %.0f s of work skipped.%s"),
		FMath::Max(0.0f, CalibrationTimer), CalibrationMinRemaining, ManualCalibrations, TechnicianCalibrations, CalibrationSecondsSaved, *Desk);
}
