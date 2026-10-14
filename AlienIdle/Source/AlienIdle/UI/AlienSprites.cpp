#include "UI/AlienSprites.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Serialization/Archive.h"
#include "Misc/Paths.h"
#include "Rendering/DrawElements.h"

namespace
{
	// Sprite frames in SVG units (see art/sprites/*.svg), their size on the 1300x700 canvas, and pivots.
	constexpr float SquirrelFrame = 32.0f;
	constexpr float SquirrelSize = 24.0f;
	const FVector2f SquirrelTailPivot(20.0f, 27.0f);
	const FVector2f SquirrelNeckPivot(12.0f, 18.0f);

	const FVector2f CowFrame(48.0f, 32.0f);
	constexpr float CowScale = 0.75f;
	const FVector2f CowTailPivot(41.0f, 12.0f);
	const FVector2f CowNeckPivot(14.0f, 13.0f);

	const FVector2f AlienFrame(32.0f, 48.0f);
	constexpr float AlienScale = 0.875f;
	const FVector2f AlienNeckPivot(16.0f, 18.0f);
	const FVector2f AlienShoulderPivot(21.5f, 21.0f);

	const FVector2f UfoFrame(96.0f, 48.0f);
	const FVector2f UfoAnchor(48.0f, 26.0f);
	const FVector2f UfoBodyPivot(48.0f, 28.0f);

	const FVector2f MountainOrigin(240.0f, -300.0f);
	const FVector2f MountainSize(1460.0f, 1160.0f);
	const FVector2f HallFxOrigin(460.0f, 205.0f);
	const FVector2f HallFxSize(1000.0f, 600.0f);
	const FVector2f CabinOrigin(780.0f, 40.0f);
	const FVector2f CabinSize(140.0f, 56.0f);

	struct FPropDef
	{
		const TCHAR* Prefix;
		/** Top-left corner on the canvas and frame size, in canvas units. */
		FVector2f Pos;
		FVector2f Size;
	};
	const FPropDef Props[] = {
		{ TEXT("prop_sign_gohome"), FVector2f(786.0f, 68.5f), FVector2f(24.0f, 24.0f) },
		{ TEXT("prop_sign_keepout"), FVector2f(871.5f, 76.5f), FVector2f(12.0f, 9.0f) },
		{ TEXT("prop_mailbox"), FVector2f(902.0f, 76.4f), FVector2f(14.0f, 16.0f) },
	};
	static_assert(UE_ARRAY_COUNT(Props) == int32(EMountainProp::Count), "One prop definition per EMountainProp");

	TAutoConsoleVariable<int32> CVarMountainProps(
		TEXT("AlienIdle.MountainProps"), 0,
		TEXT("Preview the mountain props: 0 as set by the game, 1 snow, 2 on fire, 3 burnt, 4 burnt and snowed on."));

	struct FRoomDef
	{
		const TCHAR* Back;
		const TCHAR* Detail;
		FVector2f Frame;
	};
	const FRoomDef Rooms[] = {
		{ TEXT("room_hangar_back"), TEXT("room_hangar_beacon"), FVector2f(300.0f, 100.0f) },
		{ TEXT("room_containment_back"), TEXT("room_containment_bulb"), FVector2f(305.0f, 100.0f) },
		{ TEXT("room_lab_back"), TEXT("room_lab_light"), FVector2f(330.0f, 115.0f) },
		{ TEXT("room_generator_back"), TEXT("room_generator_needle"), FVector2f(255.0f, 115.0f) },
		{ TEXT("room_breakroom_back"), TEXT("room_breakroom_hand"), FVector2f(605.0f, 115.0f) },
	};
	static_assert(UE_ARRAY_COUNT(Rooms) == int32(ERoomSprite::Count), "One room definition per ERoomSprite");

	/** A smooth 0..1..0 bump during the first Width of every unit cycle of X. */
	float Bump(float X, float Width)
	{
		const float F = FMath::Frac(X);
		return F < Width ? FMath::Sin(F / Width * UE_PI) : 0.0f;
	}

	/** Pixel width of a PNG, read from its IHDR chunk; 0 if unreadable. */
	int32 PngWidth(const FString& Path)
	{
		TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Path));
		uint8 Header[24] = {};
		if (!Reader || Reader->TotalSize() < 24)
		{
			return 0;
		}
		Reader->Serialize(Header, 24);
		return (Header[16] << 24) | (Header[17] << 16) | (Header[18] << 8) | Header[19];
	}

	/** Stable pseudo-random 0..1 per integer step. */
	float Hash(float Step)
	{
		return FMath::Frac(FMath::Sin(Step * 12.9898f) * 43758.5453f);
	}

	FVector2f Rotate(const FVector2f& V, float Degrees)
	{
		float S, C;
		FMath::SinCos(&S, &C, FMath::DegreesToRadians(Degrees));
		return FVector2f(V.X * C - V.Y * S, V.X * S + V.Y * C);
	}
}

FVector2f FUfoPose::ToCanvas(const FVector2f& FramePoint) const
{
	return Pos - UfoAnchor + UfoBodyPivot + Rotate(FramePoint - UfoBodyPivot, TiltDegrees);
}

void FAlienSprites::Load()
{
	const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Sprites"));
	TArray<FString> Files;
	IFileManager::Get().FindFiles(Files, *(Dir / TEXT("*.png")), true, false);
	for (const FString& File : Files)
	{
		// "cow_head@4" is the cow's head at 4 pixels per canvas unit.
		FString Name = FPaths::GetBaseFilename(File);
		FString DensityText;
		float Density = 4.0f;
		if (Name.Split(TEXT("@"), &Name, &DensityText))
		{
			Density = FMath::Max(FCString::Atof(*DensityText), 0.1f);
		}
		const FString Path = Dir / File;
		FPartVariant Variant;
		Variant.Density = Density;
		Variant.NominalWidth = FMath::Max(PngWidth(Path) / Density, 1.0f);
		// Dynamic brushes load straight from disk; plain image brushes only resolve inside style sets.
		// The image size is irrelevant: parts are always drawn into an explicit box.
		Variant.Brush = MakeUnique<FSlateDynamicImageBrush>(FName(*Path), FVector2D(32.0, 32.0));
		Parts.FindOrAdd(FName(*Name)).Add(MoveTemp(Variant));
	}
	for (TPair<FName, TArray<FPartVariant>>& Pair : Parts)
	{
		Pair.Value.Sort([](const FPartVariant& A, const FPartVariant& B) { return A.Density < B.Density; });
	}

	bHasSquirrel = HasAll({ TEXT("squirrel_tail"), TEXT("squirrel_body"), TEXT("squirrel_head"), TEXT("squirrel_pupils"), TEXT("squirrel_arms") });
	bHasCow = HasAll({ TEXT("cow_tail"), TEXT("cow_body"), TEXT("cow_jaw"), TEXT("cow_head"), TEXT("cow_lids"), TEXT("cow_tag") });
	bHasAlien = HasAll({ TEXT("alien_body"), TEXT("alien_head"), TEXT("alien_blink"), TEXT("alien_arm") });
	bHasUfo[0] = HasAll({ TEXT("ufo_old_reliable_wire"), TEXT("ufo_old_reliable_spark"), TEXT("ufo_old_reliable_antenna"), TEXT("ufo_old_reliable_body"), TEXT("smoke_puff") });
	bHasUfo[1] = HasAll({ TEXT("ufo_second_opinion_plate"), TEXT("ufo_second_opinion_body"), TEXT("ufo_second_opinion_siren") });
	bHasUfo[2] = HasAll({ TEXT("ufo_plausible_deniability_balloon"), TEXT("ufo_plausible_deniability_body"), TEXT("ufo_plausible_deniability_propa"), TEXT("ufo_plausible_deniability_propb") });
	bHasMountain = HasAll({ TEXT("mountain_back"), TEXT("hallfx_lights"), TEXT("hallfx_leds"), TEXT("hallfx_spark"),
		TEXT("cabin_back"), TEXT("cabin_glow"), TEXT("cabin_oldman"), TEXT("cabin_oldman_watch"), TEXT("cabin_dog_tail"), TEXT("cabin_dog"),
		TEXT("cabin_dog_head"), TEXT("smoke_puff") });
	bHasCity = bHasMountain && HasAll({ TEXT("city_back") });
	bHasOcean = bHasMountain && HasAll({ TEXT("ocean_back"), TEXT("ocean_turbine_a"), TEXT("ocean_turbine_b"), TEXT("ocean_tanks"), TEXT("ocean_cracks"), TEXT("ocean_needle") });
	bHasSea = HasAll({ TEXT("pufferfish_body"), TEXT("pufferfish_puffed"), TEXT("pufferfish_fin"), TEXT("octopus_body"), TEXT("octopus_arms_a"),
		TEXT("octopus_arms_b"), TEXT("whale_body"), TEXT("whale_tail"), TEXT("whale_notes") });
	bHasIce = bHasMountain && HasAll({ TEXT("ice_back"), TEXT("ice_aurora"), TEXT("ice_furnace_glow"), TEXT("ice_taps"), TEXT("ice_vault") });
	bHasIceCreatures = HasAll({ TEXT("penguin_body"), TEXT("penguin_flipper"), TEXT("seal_body"), TEXT("seal_tail"),
		TEXT("elephantseal_body"), TEXT("elephantseal_nose") });
	bHasRegions = HasAll({ TEXT("region_mountain_0_back"), TEXT("region_mountain_1_back"), TEXT("region_mountain_2_back"),
		TEXT("region_city_0_back"), TEXT("region_city_1_back"), TEXT("region_city_2_back"),
		TEXT("resp_patrol_body"), TEXT("resp_investigators_body"), TEXT("resp_radar_body"), TEXT("resp_support_body") });
	for (int32 r = 0; r < int32(ERoomSprite::Count); ++r)
	{
		bHasRoom[r] = HasAll({ Rooms[r].Back, Rooms[r].Detail, TEXT("smoke_puff") });
	}
}

bool FAlienSprites::HasAll(std::initializer_list<const TCHAR*> Names) const
{
	for (const TCHAR* Name : Names)
	{
		if (!Parts.Contains(FName(Name)))
		{
			return false;
		}
	}
	return true;
}

const FSlateBrush* FAlienSprites::Part(FName Name, float W, float PixelsPerUnit) const
{
	const TArray<FPartVariant>* Variants = Parts.Find(Name);
	if (!Variants || Variants->IsEmpty())
	{
		return nullptr;
	}
	// Smallest density that is (nearly) as sharp as the screen needs; downscaling a much bigger
	// texture without mipmaps would shimmer, upscaling a smaller one would blur.
	for (const FPartVariant& Variant : *Variants)
	{
		const float Needed = PixelsPerUnit * W / Variant.NominalWidth;
		if (Variant.Density >= Needed * 0.85f)
		{
			return Variant.Brush.Get();
		}
	}
	return Variants->Last().Brush.Get();
}

void FAlienSprites::DrawPart(const FSpriteCanvas& Canvas, FName Name, float X, float Y, float W, float H,
	float AngleDegrees, const FVector2f& LocalPivot, const FLinearColor& Tint) const
{
	if (const FSlateBrush* Brush = Part(Name, W, Canvas.PixelsPerUnit))
	{
		FSlateDrawElement::MakeRotatedBox(Canvas.Elements, ++Canvas.Layer, Canvas.ToPaint(X, Y, W, H), Brush,
			ESlateDrawEffect::None, FMath::DegreesToRadians(AngleDegrees), LocalPivot, FSlateDrawElement::RelativeToElement, Tint);
	}
}

void FAlienSprites::DrawPuffs(const FSpriteCanvas& Canvas, const FVector2f& From, const FVector2f& Drift, float MinSize, float MaxSize,
	float Rate, int32 Count, const FLinearColor& Color, float Time) const
{
	for (int32 i = 0; i < Count; ++i)
	{
		const float T = FMath::Frac(Time * Rate + float(i) / Count);
		const float Size = FMath::Lerp(MinSize, MaxSize, T);
		const FVector2f At = From + Drift * T;
		DrawPart(Canvas, TEXT("smoke_puff"), At.X - Size * 0.5f, At.Y - Size * 0.5f, Size, Size, 0.0f, FVector2f::ZeroVector,
			Color.CopyWithNewOpacity(Color.A * (1.0f - T)));
	}
}

void FAlienSprites::DrawSquirrel(const FSpriteCanvas& Canvas, float CX, float FeetY, float Time, const FLinearColor& Tint) const
{
	const float S = SquirrelSize / SquirrelFrame;
	const float X = CX - SquirrelSize * 0.5f;
	const float Y = FeetY + 1.0f - SquirrelSize;
	// Each squirrel gets its own rhythm from where it stands.
	const float Phase = CX * 0.031f + FeetY * 0.17f;

	// Fast, nervous breathing lifts everything above the waist.
	const float Breath = 0.35f * FMath::Sin(Time * 6.0f + Phase);
	// Tail swishes, with a sudden flick now and then.
	const float Tail = 7.0f * FMath::Sin(Time * 2.1f + Phase) - 18.0f * Bump(Time * 0.31f + Phase, 0.08f);
	// Head scans slowly, then twitches violently.
	const float Head = 4.0f * FMath::Sin(Time * 0.9f + Phase) + 9.0f * Bump(Time * 0.47f + Phase * 1.3f, 0.1f) * FMath::Sin(Time * 55.0f);
	// Pupils dart to a new spot a few times a second.
	const float Step = FMath::FloorToFloat(Time * 1.3f + Phase);
	const FVector2f Look(FMath::Lerp(-0.9f, 0.5f, Hash(Step)) * S, FMath::Lerp(-0.6f, 0.6f, Hash(Step + 71.0f)) * S);

	DrawPart(Canvas, TEXT("squirrel_tail"), X, Y, SquirrelSize, SquirrelSize, Tail, SquirrelTailPivot * S, Tint);
	DrawPart(Canvas, TEXT("squirrel_body"), X, Y, SquirrelSize, SquirrelSize, 0.0f, FVector2f::ZeroVector, Tint);
	DrawPart(Canvas, TEXT("squirrel_head"), X, Y + Breath, SquirrelSize, SquirrelSize, Head, SquirrelNeckPivot * S, Tint);
	DrawPart(Canvas, TEXT("squirrel_pupils"), X + Look.X, Y + Breath + Look.Y, SquirrelSize, SquirrelSize, Head, SquirrelNeckPivot * S - Look, Tint);
	DrawPart(Canvas, TEXT("squirrel_arms"), X, Y + Breath, SquirrelSize, SquirrelSize, 0.0f, FVector2f::ZeroVector, Tint);
}

void FAlienSprites::DrawCow(const FSpriteCanvas& Canvas, float CX, float FeetY, float Time, const FLinearColor& Tint) const
{
	const FVector2f Size = CowFrame * CowScale;
	const float X = CX - Size.X * 0.5f;
	const float Y = FeetY + 0.5f - Size.Y;
	const float Phase = CX * 0.029f + FeetY * 0.13f;

	// Tail swats at flies that aren't there.
	const float Tail = 10.0f * FMath::Sin(Time * 1.7f + Phase) + 22.0f * Bump(Time * 0.23f + Phase, 0.1f);
	// Slow, heavy head bob; the jaw chews side to side regardless of circumstances.
	const float Head = 2.0f * FMath::Sin(Time * 0.8f + Phase);
	const float Chew = FMath::Sin(Time * 7.0f + Phase);
	const FVector2f Jaw(0.5f * Chew * CowScale, 0.3f * FMath::Abs(Chew) * CowScale);
	const FVector2f Neck = CowNeckPivot * CowScale;
	// Blinks rarely, which is worse.
	const bool bBlink = FMath::Frac(Time * 0.19f + Phase * 0.3f) < 0.03f;
	const bool bTagOn = FMath::Frac(Time * 0.8f + Phase) < 0.15f;

	DrawPart(Canvas, TEXT("cow_tail"), X, Y, Size.X, Size.Y, Tail, CowTailPivot * CowScale, Tint);
	DrawPart(Canvas, TEXT("cow_body"), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, Tint);
	DrawPart(Canvas, TEXT("cow_jaw"), X + Jaw.X, Y + Jaw.Y, Size.X, Size.Y, Head, Neck - Jaw, Tint);
	DrawPart(Canvas, TEXT("cow_head"), X, Y, Size.X, Size.Y, Head, Neck, Tint);
	if (bBlink)
	{
		DrawPart(Canvas, TEXT("cow_lids"), X, Y, Size.X, Size.Y, Head, Neck, Tint);
	}
	if (bTagOn)
	{
		DrawPart(Canvas, TEXT("cow_tag"), X, Y, Size.X, Size.Y, Head, Neck, Tint);
	}
}

void FAlienSprites::DrawAlien(const FSpriteCanvas& Canvas, float CX, float FeetY, float Time, EAlienUniform Uniform) const
{
	const TCHAR* BodyPart = TEXT("alien_body");
	const TCHAR* ArmPart = TEXT("alien_arm");
	switch (Uniform)
	{
	case EAlienUniform::Biological: BodyPart = TEXT("alien_body_bio"); ArmPart = TEXT("alien_arm_bio"); break;
	case EAlienUniform::Human: BodyPart = TEXT("alien_body_human"); ArmPart = TEXT("alien_arm_clipboard"); break;
	case EAlienUniform::Incompetence: BodyPart = TEXT("alien_body_incomp"); ArmPart = TEXT("alien_arm_incomp"); break;
	default: break;
	}
	const FVector2f Size = AlienFrame * AlienScale;
	const float X = CX - Size.X * 0.5f;
	const float Y = FeetY + 0.5f - Size.Y;
	const FLinearColor Tint = FLinearColor::White;

	// Every ~8 s: a long, joyless sip of coffee, head tipping back, eyes closed.
	const float Sip = Bump(Time * 0.12f, 0.25f);
	const float Head = 3.0f * FMath::Sin(Time * 0.6f) - 6.0f * Sip;
	const FVector2f Neck = AlienNeckPivot * AlienScale;
	const float HeadY = 0.2f * FMath::Sin(Time * 2.0f);
	const bool bBlink = FMath::Frac(Time * 0.31f) < 0.04f || Sip > 0.7f;

	DrawPart(Canvas, BodyPart, X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, Tint);
	DrawPart(Canvas, TEXT("alien_head"), X, Y + HeadY, Size.X, Size.Y, Head, Neck, Tint);
	if (bBlink)
	{
		DrawPart(Canvas, TEXT("alien_blink"), X, Y + HeadY, Size.X, Size.Y, Head, Neck, Tint);
	}
	if (Uniform == EAlienUniform::Incompetence)
	{
		DrawPart(Canvas, TEXT("alien_hardhat"), X, Y + HeadY, Size.X, Size.Y, Head, Neck, Tint);
	}
	// In a suit there is no coffee: the clipboard just gets consulted now and then.
	DrawPart(Canvas, ArmPart, X, Y, Size.X, Size.Y, -40.0f * Sip, AlienShoulderPivot * AlienScale, Tint);
}

FUfoPose FAlienSprites::PoseUfo(int32 Ufo, const FVector2f& Pos, float Time, bool bEngineOn) const
{
	FUfoPose Pose;
	Pose.Ufo = Ufo;
	Pose.bEngineOn = bEngineOn;
	if (bEngineOn)
	{
		switch (Ufo)
		{
		case 0:
			// Every ~3 s Old Reliable's engine coughs: it lurches nose-down and drops.
			Pose.Sputter = Bump(Time * 0.33f, 0.07f);
			Pose.TiltDegrees = 2.0f * FMath::Sin(Time * 1.6f) - 4.0f * Pose.Sputter;
			break;
		case 1:
			// Second Opinion wallows gently, like it's reconsidering.
			Pose.TiltDegrees = 1.5f * FMath::Sin(Time * 1.1f + 1.0f);
			break;
		default:
			// Plausible Deniability holds suspiciously still.
			Pose.TiltDegrees = 0.5f * FMath::Sin(Time * 0.9f + 2.0f);
			break;
		}
	}
	Pose.Pos = Pos + FVector2f(0.0f, 2.0f * Pose.Sputter);
	return Pose;
}

void FAlienSprites::DrawUfo(const FSpriteCanvas& Canvas, const FUfoPose& Pose, float Time, const FLinearColor& Tint, bool bModules) const
{
	const FVector2f Origin = Pose.Pos - UfoAnchor;

	// Parts that hang from a point on the tilting hull.
	auto Hang = [&](const FVector2f& FramePivot, const TCHAR* Name, float Swing, const FLinearColor& PartTint)
	{
		const FVector2f Moved = Pose.ToCanvas(FramePivot) - (Origin + FramePivot);
		DrawPart(Canvas, Name, Origin.X + Moved.X, Origin.Y + Moved.Y, UfoFrame.X, UfoFrame.Y, Pose.TiltDegrees + Swing, FramePivot, PartTint);
	};
	auto Body = [&](const TCHAR* Name, const FLinearColor& PartTint)
	{
		DrawPart(Canvas, Name, Origin.X, Origin.Y, UfoFrame.X, UfoFrame.Y, Pose.TiltDegrees, UfoBodyPivot, PartTint);
	};

	switch (Pose.Ufo)
	{
	case 0:
	{
		// Exhaust: a steady trail of puffs from the stern, one fat dark one per cough.
		if (Pose.bEngineOn)
		{
			const FVector2f Stern = Pose.ToCanvas(FVector2f(86.0f, 29.0f));
			DrawPuffs(Canvas, Stern, FVector2f(16.0f, -14.0f), 6.0f, 16.0f, 0.9f, 4, FLinearColor(0.8f, 0.8f, 0.8f, 0.55f * Tint.A), Time);
			if (Pose.Sputter > 0.0f)
			{
				const float Size = 10.0f + 10.0f * Pose.Sputter;
				DrawPart(Canvas, TEXT("smoke_puff"), Stern.X - Size * 0.3f, Stern.Y - Size * 0.6f, Size, Size,
					0.0f, FVector2f::ZeroVector, FLinearColor(0.25f, 0.24f, 0.22f, 0.8f * Pose.Sputter * Tint.A));
			}
		}
		const FVector2f WirePivot(61.0f, 33.0f);
		const float WireSwing = 14.0f * FMath::Sin(Time * 2.4f) + (Pose.bEngineOn ? 10.0f : 0.0f) + 25.0f * Pose.Sputter;
		Hang(WirePivot, TEXT("ufo_old_reliable_wire"), WireSwing, Tint);
		if (FMath::Frac(Time * 1.7f) < 0.12f || FMath::Frac(Time * 2.9f + 0.3f) < 0.06f || Pose.Sputter > 0.3f)
		{
			Hang(WirePivot, TEXT("ufo_old_reliable_spark"), WireSwing, Tint);
		}
		Hang(FVector2f(22.0f, 22.0f), TEXT("ufo_old_reliable_antenna"), 6.0f * FMath::Sin(Time * 3.3f) + 20.0f * Pose.Sputter * FMath::Sin(Time * 30.0f), Tint);
		Body(TEXT("ufo_old_reliable_body"), Tint);
		break;
	}
	case 1:
	{
		Hang(FVector2f(81.0f, 31.0f), TEXT("ufo_second_opinion_plate"), 10.0f * FMath::Sin(Time * 2.0f) + (Pose.bEngineOn ? 12.0f : 0.0f), Tint);
		Body(TEXT("ufo_second_opinion_body"), Tint);
		if (Pose.bEngineOn)
		{
			// The siren pulses whenever it's out, for reasons nobody can explain.
			const float Glow = FMath::Square(0.5f + 0.5f * FMath::Sin(Time * 8.0f));
			Body(TEXT("ufo_second_opinion_siren"), Tint.CopyWithNewOpacity(Tint.A * Glow));
		}
		break;
	}
	default:
	{
		// The balloon trails behind when moving, and fools no one.
		Hang(FVector2f(15.0f, 27.0f), TEXT("ufo_plausible_deniability_balloon"), 8.0f * FMath::Sin(Time * 1.3f) - (Pose.bEngineOn ? 12.0f : 0.0f), Tint);
		Body(TEXT("ufo_plausible_deniability_body"), Tint);
		const bool bFrameB = Pose.bEngineOn && FMath::Frac(Time * 14.0f) < 0.5f;
		Body(bFrameB ? TEXT("ufo_plausible_deniability_propb") : TEXT("ufo_plausible_deniability_propa"), Tint);
		break;
	}
	}

	// Experimental UFO Modules: a box bolted on by someone in a hurry, its fan always spinning.
	if (bModules)
	{
		Body(TEXT("ufo_modules_module"), Tint);
		Body(FMath::Frac(Time * 12.0f + Pose.Ufo * 0.3f) < 0.5f ? TEXT("ufo_modules_fan_a") : TEXT("ufo_modules_fan_b"), Tint);
	}
}

void FAlienSprites::DrawProp(const FSpriteCanvas& Canvas, EMountainProp Prop, float Time) const
{
	const FPropDef& Def = Props[int32(Prop)];
	FPropState State = PropStates[int32(Prop)];
	switch (CVarMountainProps.GetValueOnGameThread())
	{
	case 1: State = { true, false, false }; break;
	case 2: State = { false, true, false }; break;
	case 3: State = { false, false, true }; break;
	case 4: State = { true, false, true }; break;
	default: break;
	}
	auto Layer = [&](const TCHAR* Suffix, const FLinearColor& Tint = FLinearColor::White)
	{
		DrawPart(Canvas, FName(*FString::Printf(TEXT("%s_%s"), Def.Prefix, Suffix)), Def.Pos.X, Def.Pos.Y, Def.Size.X, Def.Size.Y, 0.0f, FVector2f::ZeroVector, Tint);
	};
	Layer(State.bBurnt ? TEXT("burnt") : TEXT("base"));
	if (State.bSnow)
	{
		Layer(TEXT("snow"));
	}
	if (State.bOnFire)
	{
		// Flames flicker between two frames; black smoke rolls off the top.
		const float Phase = float(Prop) * 0.37f;
		DrawPuffs(Canvas, Def.Pos + FVector2f(Def.Size.X * 0.5f, 0.0f), FVector2f(-4.0f, -18.0f), Def.Size.X * 0.3f, Def.Size.X * 0.8f,
			0.8f, 3, FLinearColor(0.12f, 0.1f, 0.1f, 0.7f), Time + Phase);
		Layer(FMath::Frac(Time * 7.0f + Phase) < 0.5f ? TEXT("fire_a") : TEXT("fire_b"));
	}
}

void FAlienSprites::DrawMountain(const FSpriteCanvas& Canvas, float Time, bool bUfoOut, int32 Location, const FOceanLook& Ocean,
	const FIceLook& Ice) const
{
	const bool bCity = Location == 1 && bHasCity;
	const bool bOcean = Location == 2 && bHasOcean;
	const bool bIce = Location == 3 && bHasIce;
	// Three frames: the mountain itself, the hallway effects and the cabin, each sized to what it holds.
	auto Layer = [&](const FVector2f& Origin, const FVector2f& Size, const TCHAR* Name, float Alpha = 1.0f, float Angle = 0.0f,
		const FVector2f& CanvasPivot = FVector2f::ZeroVector)
	{
		DrawPart(Canvas, Name, Origin.X, Origin.Y, Size.X, Size.Y, Angle, CanvasPivot - Origin, FLinearColor::White.CopyWithNewOpacity(Alpha));
	};
	auto Hall = [&](const TCHAR* Name, float Alpha = 1.0f) { Layer(HallFxOrigin, HallFxSize, Name, Alpha); };
	auto Cabin = [&](const TCHAR* Name, float Alpha = 1.0f, float Angle = 0.0f, const FVector2f& CanvasPivot = FVector2f::ZeroVector)
	{
		Layer(CabinOrigin, CabinSize, Name, Alpha, Angle, CanvasPivot);
	};

	// Metropolis and the ocean swap the mountain for their own backdrop (same frame, same bunker underground).
	Layer(MountainOrigin, MountainSize, bIce ? TEXT("ice_back") : bOcean ? TEXT("ocean_back") : bCity ? TEXT("city_back") : TEXT("mountain_back"));
	// Hallway lamps hum along, then brown out together now and then.
	Hall(TEXT("hallfx_lights"), Hash(FMath::FloorToFloat(Time * 8.0f)) < 0.08f ? 0.3f : 1.0f);
	if (FMath::Frac(Time * 1.3f) < 0.6f)
	{
		Hall(TEXT("hallfx_leds"));
	}
	if (FMath::Frac(Time * 1.1f) < 0.1f || FMath::Frac(Time * 2.7f + 0.4f) < 0.05f)
	{
		Hall(TEXT("hallfx_spark"));
	}

	if (bIce)
	{
		// The aurora shimmers; nobody up here has time to look at it.
		Layer(MountainOrigin, MountainSize, TEXT("ice_aurora"), 0.55f + 0.3f * FMath::Sin(Time * 0.5f) + 0.15f * FMath::Sin(Time * 1.7f));
		// The boiler burns as hard as the heat supply; it flickers either way.
		const float Fire = FMath::Clamp(Ice.Fire, 0.15f, 1.0f);
		Layer(MountainOrigin, MountainSize, TEXT("ice_furnace_glow"), Fire * (0.75f + 0.25f * Hash(FMath::FloorToFloat(Time * 10.0f))));
		if (Ice.bVault)
		{
			Layer(MountainOrigin, MountainSize, TEXT("ice_vault"));
		}
		if (Ice.bTaps)
		{
			// Geothermal vents puff steam. Radar loves steam.
			Layer(MountainOrigin, MountainSize, TEXT("ice_taps"));
			const FLinearColor Steam(0.92f, 0.95f, 1.0f, 0.6f);
			DrawPuffs(Canvas, FVector2f(1250.0f, 234.0f), FVector2f(-14.0f, -60.0f), 6.0f, 22.0f, 0.5f, 5, Steam, Time);
			DrawPuffs(Canvas, FVector2f(1330.0f, 238.0f), FVector2f(-10.0f, -50.0f), 5.0f, 18.0f, 0.6f, 4, Steam, Time + 0.7f);
		}
		return;
	}
	if (bOcean)
	{
		// Tidal turbines turn with the tide; deep tanks sit on the sea bed; the hull gauge reads the pressure.
		if (Ocean.bTurbines)
		{
			const float Spin = Time * (40.0f + 280.0f * Ocean.Tide);
			Layer(MountainOrigin, MountainSize, TEXT("ocean_turbine_a"), 1.0f, Spin, FVector2f(1460.0f, 196.0f));
			Layer(MountainOrigin, MountainSize, TEXT("ocean_turbine_b"), 1.0f, -Spin * 1.2f, FVector2f(1580.0f, 214.0f));
		}
		if (Ocean.bTanks)
		{
			Layer(MountainOrigin, MountainSize, TEXT("ocean_tanks"));
		}
		if (Ocean.HullPressure >= 60.0f && Hash(FMath::FloorToFloat(Time * 3.0f)) < 0.7f)
		{
			Layer(MountainOrigin, MountainSize, TEXT("ocean_cracks"), 0.5f + 0.5f * (Ocean.HullPressure - 60.0f) / 40.0f);
		}
		const float Needle = -120.0f + 240.0f * FMath::Clamp(Ocean.HullPressure / 100.0f, 0.0f, 1.0f) + (Ocean.HullPressure >= 60.0f ? 3.0f * FMath::Sin(Time * 25.0f) : 0.0f);
		Layer(MountainOrigin, MountainSize, TEXT("ocean_needle"), 1.0f, Needle, FVector2f(780.0f, 360.0f));
		if (Ocean.bWhaleSong)
		{
			// A whale sings against the dome. It has notes.
			const FVector2f Whale(1200.0f + 20.0f * FMath::Sin(Time * 0.3f), 0.0f + 6.0f * FMath::Sin(Time * 0.8f));
			DrawPart(Canvas, TEXT("whale_tail"), Whale.X - 70.0f, Whale.Y - 26.0f, 140.0f, 51.0f, 8.0f * FMath::Sin(Time * 1.2f), FVector2f(100.0f, 22.0f) * (140.0f / 120.0f), FLinearColor::White);
			DrawPart(Canvas, TEXT("whale_body"), Whale.X - 70.0f, Whale.Y - 26.0f, 140.0f, 51.0f, 0.0f, FVector2f::ZeroVector, FLinearColor::White);
			const float Rise = FMath::Frac(Time * 0.4f);
			DrawPart(Canvas, TEXT("whale_notes"), Whale.X - 90.0f - 20.0f * Rise, Whale.Y - 40.0f - 30.0f * Rise, 140.0f, 51.0f, 0.0f, FVector2f::ZeroVector, FLinearColor::White.CopyWithNewOpacity(1.0f - Rise));
		}
		return;
	}
	if (bCity)
	{
		return; // The old man and his cabin stay on the mountain.
	}

	// The cabin: TV flicker in the window, smoke from the chimney.
	Cabin(TEXT("cabin_back"));
	Cabin(TEXT("cabin_glow"), 0.45f + 0.55f * Hash(FMath::FloorToFloat(Time * 6.0f)));
	DrawPuffs(Canvas, FVector2f(876.5f, 48.0f), FVector2f(-10.0f, -26.0f), 3.0f, 11.0f, 0.35f, 4, FLinearColor(0.75f, 0.75f, 0.78f, 0.6f), Time);

	// Signs and the mailbox are their own sprites (snow, fire, burnt).
	for (int32 p = 0; p < int32(EMountainProp::Count); ++p)
	{
		DrawProp(Canvas, EMountainProp(p), Time);
	}

	// The old man rocks; when a UFO is out he has his binoculars up. Nobody believes him.
	Cabin(bUfoOut ? TEXT("cabin_oldman_watch") : TEXT("cabin_oldman"), 1.0f, 4.0f * FMath::Sin(Time * 1.4f), FVector2f(825.0f, 92.0f));
	// The dog wags lazily, or excitedly with its nose up while a UFO is out.
	const float Tail = bUfoOut ? 25.0f * FMath::Sin(Time * 14.0f) : 15.0f * FMath::Sin(Time * 3.0f);
	Cabin(TEXT("cabin_dog_tail"), 1.0f, Tail, FVector2f(898.0f, 89.0f));
	Cabin(TEXT("cabin_dog"));
	const float Head = bUfoOut ? 20.0f + 3.0f * FMath::Sin(Time * 9.0f) : 3.0f * FMath::Sin(Time * 0.7f);
	Cabin(TEXT("cabin_dog_head"), 1.0f, Head, FVector2f(892.0f, 88.0f));
}

void FAlienSprites::DrawRegion(const FSpriteCanvas& Canvas, int32 Location, int32 Region, float X, float Y, float Time) const
{
	const bool bCity = Location == 1;
	static const TCHAR* Names[] = { TEXT("mountain"), TEXT("city"), TEXT("ocean"), TEXT("ice") };
	const FString Prefix = FString::Printf(TEXT("region_%s_%d_"), Names[FMath::Clamp(Location, 0, 3)], Region);
	const FVector2f Size(220.0f, 210.0f);
	DrawPart(Canvas, FName(*(Prefix + TEXT("back"))), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, FLinearColor::White);
	if (Location == 3)
	{
		if (Region == 1)
		{
			// The outpost's satellite dish hunts for a signal. It is picking us up. Nobody reads it.
			DrawPart(Canvas, FName(*(Prefix + TEXT("dish"))), X, Y, Size.X, Size.Y, 15.0f * FMath::Sin(Time * 0.4f), FVector2f(150.0f, 168.0f), FLinearColor::White);
		}
		return;
	}
	if (Location == 2)
	{
		if (Region == 1)
		{
			// The Harbour Town lighthouse sweeps the bay.
			DrawPart(Canvas, FName(*(Prefix + TEXT("beam"))), X, Y, Size.X, Size.Y, 25.0f * FMath::Sin(Time * 0.7f), FVector2f(96.0f, 115.0f), FLinearColor::White);
		}
		if (Region == 2)
		{
			// Something with a lure is waiting in the trench.
			DrawPart(Canvas, FName(*(Prefix + TEXT("lure"))), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, FLinearColor::White.CopyWithNewOpacity(0.5f + 0.5f * FMath::Sin(Time * 2.0f)));
		}
		return;
	}
	if (!bCity && Region == 2)
	{
		// The fairground Ferris wheel never stops, even when nobody is on it.
		DrawPart(Canvas, FName(*(Prefix + TEXT("wheel"))), X, Y, Size.X, Size.Y, Time * 12.0f, FVector2f(172.0f, 150.0f), FLinearColor::White);
	}
	if (bCity && Region == 2 && Hash(FMath::FloorToFloat(Time * 5.0f)) > 0.12f)
	{
		DrawPart(Canvas, FName(*(Prefix + TEXT("neon"))), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, FLinearColor::White);
	}
}

void FAlienSprites::DrawIceCreature(const FSpriteCanvas& Canvas, int32 Kind, float CX, float FeetY, float Time) const
{
	const FLinearColor White = FLinearColor::White;
	const float Phase = CX * 0.037f + FeetY * 0.11f;
	switch (Kind)
	{
	case 0:
	{
		// Penguin: waddles side to side and flaps, going nowhere with great purpose.
		const FVector2f Size(14.0f, 18.2f);
		const float Waddle = 6.0f * FMath::Sin(Time * 5.0f + Phase);
		const float X = CX - Size.X * 0.5f;
		const float Y = FeetY - Size.Y + 0.3f;
		const FVector2f Feet(Size.X * 0.5f, Size.Y);
		DrawPart(Canvas, TEXT("penguin_body"), X, Y, Size.X, Size.Y, Waddle, Feet, White);
		DrawPart(Canvas, TEXT("penguin_flipper"), X, Y, Size.X, Size.Y, Waddle + 18.0f * FMath::Max(0.0f, FMath::Sin(Time * 7.0f + Phase)), FVector2f(11.0f, 11.0f) * 0.7f, White);
		break;
	}
	case 1:
	{
		// Seal: lies there, waves its tail, breathes.
		const FVector2f Size(32.0f, 16.0f);
		const float X = CX - Size.X * 0.5f;
		const float Y = FeetY - Size.Y + 0.3f;
		DrawPart(Canvas, TEXT("seal_tail"), X, Y, Size.X, Size.Y, 15.0f * FMath::Sin(Time * 2.0f + Phase), FVector2f(34.0f, 15.0f) * 0.8f, White);
		DrawPart(Canvas, TEXT("seal_body"), X, Y - 0.3f * FMath::Sin(Time * 1.5f + Phase), Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, White);
		break;
	}
	case 2:
	{
		// Elephant seal: enormous, loud, and its nose wobbles when it bellows.
		const FVector2f Size(72.0f, 32.0f);
		const float X = CX - Size.X * 0.5f;
		const float Y = FeetY - Size.Y + 0.6f;
		const float Bellow = FMath::Frac(Time * 0.25f + Phase) < 0.2f ? 12.0f * FMath::Sin(Time * 18.0f) : 2.0f * FMath::Sin(Time * 1.2f + Phase);
		DrawPart(Canvas, TEXT("elephantseal_body"), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, White);
		DrawPart(Canvas, TEXT("elephantseal_nose"), X, Y, Size.X, Size.Y, Bellow, FVector2f(14.0f, 17.0f) * 0.8f, White);
		break;
	}
	default:
		break;
	}
}

void FAlienSprites::DrawSeaCreature(const FSpriteCanvas& Canvas, int32 Kind, float CX, float FeetY, float Time) const
{
	const FLinearColor White = FLinearColor::White;
	const float Phase = CX * 0.037f + FeetY * 0.11f;
	switch (Kind)
	{
	case 0:
	{
		// Pufferfish: hovers, flaps its fin, and puffs up when stressed (often).
		const FVector2f Size(16.0f, 13.33f);
		const float Y = FeetY - Size.Y - 2.0f + 1.5f * FMath::Sin(Time * 2.0f + Phase);
		const float X = CX - Size.X * 0.5f;
		DrawPart(Canvas, TEXT("pufferfish_fin"), X, Y, Size.X, Size.Y, 20.0f * FMath::Sin(Time * 9.0f + Phase), FVector2f(19.0f, 11.0f) * (16.0f / 24.0f), White);
		const bool bPuffed = FMath::Frac(Time * 0.2f + Phase) < 0.18f;
		DrawPart(Canvas, bPuffed ? TEXT("pufferfish_puffed") : TEXT("pufferfish_body"), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, White);
		break;
	}
	case 1:
	{
		// Octopus: wriggles, and is clearly planning something.
		const FVector2f Size(26.0f, 22.75f);
		const float X = CX - Size.X * 0.5f;
		const float Y = FeetY + 0.5f - Size.Y;
		DrawPart(Canvas, FMath::Frac(Time * 1.6f + Phase) < 0.5f ? TEXT("octopus_arms_a") : TEXT("octopus_arms_b"), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, White);
		DrawPart(Canvas, TEXT("octopus_body"), X, Y + 0.4f * FMath::Sin(Time * 3.0f + Phase), Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, White);
		break;
	}
	case 2:
	{
		// Whale: enormous, yet it floats a little above the floor, gently.
		const FVector2f Size(96.0f, 35.2f);
		const float X = CX - Size.X * 0.5f;
		const float Y = FeetY - Size.Y - 3.0f + 2.0f * FMath::Sin(Time * 0.7f + Phase);
		DrawPart(Canvas, TEXT("whale_tail"), X, Y, Size.X, Size.Y, 7.0f * FMath::Sin(Time * 1.1f + Phase), FVector2f(100.0f, 22.0f) * 0.8f, White);
		DrawPart(Canvas, TEXT("whale_body"), X, Y, Size.X, Size.Y, 0.0f, FVector2f::ZeroVector, White);
		break;
	}
	default:
		break;
	}
}

void FAlienSprites::DrawResponse(const FSpriteCanvas& Canvas, int32 Response, float X, float FeetY, float Time) const
{
	const FLinearColor White = FLinearColor::White;
	switch (Response)
	{
	case 0: // night patrols: the flashlight sweeps
		DrawPart(Canvas, TEXT("resp_patrol_beam"), X, FeetY - 30.0f, 40.0f, 30.0f, 15.0f * FMath::Sin(Time * 1.3f + X), FVector2f(12.0f, 15.0f), White);
		DrawPart(Canvas, TEXT("resp_patrol_body"), X, FeetY - 30.0f, 40.0f, 30.0f, 0.0f, FVector2f::ZeroVector, White);
		break;
	case 1: // investigators
		DrawPart(Canvas, TEXT("resp_investigators_body"), X, FeetY - 36.0f, 64.0f, 36.0f, 0.0f, FVector2f::ZeroVector, White);
		break;
	case 2: // radar: the dish nods back and forth across the sky
		DrawPart(Canvas, TEXT("resp_radar_body"), X, FeetY - 40.0f, 30.0f, 40.0f, 0.0f, FVector2f::ZeroVector, White);
		DrawPart(Canvas, TEXT("resp_radar_dish"), X, FeetY - 40.0f, 30.0f, 40.0f, 35.0f * FMath::Sin(Time * 0.8f + X), FVector2f(15.0f, 18.0f), White);
		break;
	case 3: // support group
		DrawPart(Canvas, TEXT("resp_support_body"), X, FeetY - 26.0f, 56.0f, 26.0f, 0.0f, FVector2f::ZeroVector, White);
		break;
	default:
		break;
	}
}

void FAlienSprites::DrawCondition(const FSpriteCanvas& Canvas, int32 Condition, float X, float FeetY, float Time, float Scale) const
{
	const FLinearColor White = FLinearColor::White;
	switch (Condition)
	{
	case 1:
		DrawPart(Canvas, TEXT("cond_crew_body"), X, FeetY - 34.0f * Scale, 36.0f * Scale, 34.0f * Scale, 0.0f, FVector2f::ZeroVector, White);
		break;
	case 2:
		DrawPart(Canvas, TEXT("cond_agency_body"), X, FeetY - 46.0f * Scale, 64.0f * Scale, 46.0f * Scale, 0.0f, FVector2f::ZeroVector, White);
		DrawPart(Canvas, TEXT("cond_agency_cam"), X, FeetY - 46.0f * Scale, 64.0f * Scale, 46.0f * Scale, 20.0f * FMath::Sin(Time * 0.6f + X), FVector2f(6.0f, 8.0f) * Scale, White);
		break;
	case 3:
		DrawPart(Canvas, TEXT("cond_billboard_body"), X, FeetY - 46.0f * Scale, 56.0f * Scale, 46.0f * Scale, 0.0f, FVector2f::ZeroVector, White);
		break;
	default:
		break;
	}
}

void FAlienSprites::DrawRoomPart(const FSpriteCanvas& Canvas, ERoomSprite Room, const TCHAR* Part, float X, float Y, const FLinearColor& Tint) const
{
	const FRoomDef& Def = Rooms[int32(Room)];
	const FString Prefix = FString(Def.Back).LeftChop(4); // "room_lab_back" -> "room_lab_"
	DrawPart(Canvas, FName(*(Prefix + Part)), X, Y, Def.Frame.X, Def.Frame.Y, 0.0f, FVector2f::ZeroVector, Tint);
}

void FAlienSprites::DrawRoom(const FSpriteCanvas& Canvas, ERoomSprite Room, float X, float Y, float Time) const
{
	const FRoomDef& Def = Rooms[int32(Room)];
	const FLinearColor White = FLinearColor::White;
	DrawPart(Canvas, Def.Back, X, Y, Def.Frame.X, Def.Frame.Y, 0.0f, FVector2f::ZeroVector, White);

	switch (Room)
	{
	case ERoomSprite::Hangar:
	{
		// Warning beacon: always on, always warning.
		const float Flash = FMath::Square(0.5f + 0.5f * FMath::Sin(Time * 5.0f));
		DrawPart(Canvas, Def.Detail, X, Y, Def.Frame.X, Def.Frame.Y, 0.0f, FVector2f::ZeroVector, White.CopyWithNewOpacity(Flash));
		break;
	}
	case ERoomSprite::Containment:
	{
		// The bulb buzzes and stutters, with the odd long blackout.
		float Glow = Hash(FMath::FloorToFloat(Time * 12.0f)) < 0.12f ? 0.25f : 0.9f;
		if (FMath::Frac(Time * 0.09f) < 0.03f)
		{
			Glow = 0.0f;
		}
		DrawPart(Canvas, Def.Detail, X, Y, Def.Frame.X, Def.Frame.Y, 0.0f, FVector2f::ZeroVector, White.CopyWithNewOpacity(Glow));
		break;
	}
	case ERoomSprite::Lab:
	{
		// Fluorescent tube: steady, then a burst of flickering every few seconds.
		const bool bBurst = FMath::Frac(Time * 0.15f) < 0.08f;
		const float Glow = bBurst && Hash(FMath::FloorToFloat(Time * 20.0f)) < 0.5f ? 0.1f : 1.0f;
		DrawPart(Canvas, Def.Detail, X, Y, Def.Frame.X, Def.Frame.Y, 0.0f, FVector2f::ZeroVector, White.CopyWithNewOpacity(Glow));
		break;
	}
	case ERoomSprite::Generator:
	{
		// Leaking valve, and a needle that jitters, then slams into the red.
		DrawPuffs(Canvas, FVector2f(X + 186.0f, Y + 11.0f), FVector2f(14.0f, 7.0f), 4.0f, 12.0f, 0.7f, 3, FLinearColor(1.0f, 1.0f, 1.0f, 0.45f), Time);
		const float Needle = 25.0f * FMath::Sin(Time * 0.7f) + 3.0f * FMath::Sin(Time * 37.0f) + 75.0f * Bump(Time * 0.17f, 0.12f);
		DrawPart(Canvas, Def.Detail, X, Y, Def.Frame.X, Def.Frame.Y, Needle, FVector2f(227.0f, 98.0f), White);
		break;
	}
	case ERoomSprite::BreakRoom:
	{
		// Coffee steam, and a clock that runs backwards.
		DrawPuffs(Canvas, FVector2f(X + 574.0f, Y + 72.0f), FVector2f(-2.0f, -16.0f), 2.0f, 6.0f, 0.5f, 3, FLinearColor(1.0f, 1.0f, 1.0f, 0.35f), Time);
		DrawPart(Canvas, Def.Detail, X, Y, Def.Frame.X, Def.Frame.Y, -30.0f * Time, FVector2f(425.0f, 14.0f), White);
		break;
	}
	default:
		break;
	}
}
