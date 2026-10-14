#include "UI/SAlienBaseView.h"
#include "AlienIdleSim.h"
#include "InputCoreTypes.h"
#include "HAL/IConsoleManager.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
	/** Colors are authored as sRGB (what you see in a color picker). */
	FLinearColor Srgb(float R, float G, float B, float A = 1.0f)
	{
		// Computed directly (not via FLinearColor's lookup table) so it is safe in static initializers.
		auto ToLinear = [](float C) { return C <= 0.04045f ? C / 12.92f : FMath::Pow((C + 0.055f) / 1.055f, 2.4f); };
		return FLinearColor(ToLinear(R), ToLinear(G), ToLinear(B), A);
	}

	// The map runs from y -300 (sky above the peak) down to 700; the ground scene starts at y 0.
	const FVector2f CanvasMin(0.0f, -300.0f);
	const FVector2f CanvasMax(2100.0f, 860.0f);
	const FVector2f Canvas = CanvasMax - CanvasMin;
	/** Canvas width shown across the widget at zoom 1; the rest scrolls. */
	constexpr float ViewWidth = 1300.0f;

	// The mountain and the base inside it are authored in base-local coordinates (the old 1300-wide
	// layout) and drawn shifted right by this much, which centers the base on the wider mountain.
	constexpr float BaseShift = 400.0f;

	// The rooms are spread out like a bunker (3 levels joined by tunnels and shafts in the mountain
	// art). Each room's drawing code still uses its original coordinates; these move it into place.
	const FVector2f ContainmentOffset(135.0f, 30.0f);
	const FVector2f LabOffset(-155.0f, 55.0f);
	const FVector2f GeneratorOffset(145.0f, 55.0f);
	const FVector2f BreakRoomOffset(15.0f, 115.0f);
	const FVector2f BaseOrigin(BaseShift, 0.0f);
	constexpr float GroundY = 300.0f;
	constexpr float MountainX = 660.0f;

	/** Probe Calibration tag over a lab chamber, in base-local coordinates (canvas = this + BaseOrigin). */
	FVector2f CalibrationTagPos(int32 Chamber) { return FVector2f(725.0f + Chamber * 92.0f, 458.0f) + LabOffset; }
	const FVector2f CalibrationTagSize(80.0f, 28.0f);
	/** The hunting grounds: three regions per location, region 0 nearest the base. */
	constexpr float RegionWidth = 220.0f;

	// The hangar opens onto the mountain's slope, high above the meadow.
	constexpr float HangarX = 780.0f;
	constexpr float HangarY = 110.0f;

	// UFO flight path in canvas space.
	constexpr float DockY = HangarY + 68.0f;
	const FVector2f BaseExitPos(HangarX - 70.0f, HangarY + 40.0f);
	constexpr float HoverY = 175.0f;
	constexpr float ExitFraction = 0.25f;

	// Palette (linear values, tuned by eye in the game viewport).
	const FLinearColor SkyColor(0.05f, 0.06f, 0.14f);
	const FLinearColor StarColor(0.8f, 0.85f, 1.0f, 0.7f);
	const FLinearColor RockColor(0.27f, 0.25f, 0.24f);
	const FLinearColor SnowColor(0.82f, 0.85f, 0.9f);
	const FLinearColor DirtColor(0.2f, 0.14f, 0.1f);
	const FLinearColor GrassColor(0.18f, 0.36f, 0.16f);
	const FLinearColor TreeColor(0.1f, 0.3f, 0.15f);
	const FLinearColor TrunkColor(0.3f, 0.2f, 0.12f);
	const FLinearColor RoomColor(0.13f, 0.14f, 0.17f);
	const FLinearColor RoomEdgeColor(0.4f, 0.42f, 0.46f);
	const FLinearColor LabelColor(0.65f, 0.9f, 0.7f);
	const FLinearColor WarnColor(1.0f, 0.45f, 0.2f);
	const FLinearColor SquirrelColor(0.6f, 0.35f, 0.15f);
	const FLinearColor CowColor(0.9f, 0.9f, 0.88f);
	const FLinearColor ElephantColor(0.45f, 0.45f, 0.5f);
	const FLinearColor SkinColor(0.85f, 0.6f, 0.45f);
	const FLinearColor UfoColor(0.62f, 0.64f, 0.68f);
	const FLinearColor DomeColor(0.55f, 0.85f, 0.95f, 0.85f);
	const FLinearColor AlienColor(0.45f, 0.85f, 0.4f);

	FLinearColor OccupationColor(EOccupation Occupation)
	{
		switch (Occupation)
		{
		case EOccupation::Accountant: return FLinearColor(0.35f, 0.38f, 0.42f);
		case EOccupation::Astronomer: return FLinearColor(0.15f, 0.2f, 0.55f);
		case EOccupation::Magician:   return FLinearColor(0.45f, 0.15f, 0.6f);
		case EOccupation::Lawyer:     return FLinearColor(0.05f, 0.05f, 0.08f);
		case EOccupation::Therapist:  return FLinearColor(0.35f, 0.6f, 0.45f);
		case EOccupation::MallSanta:  return FLinearColor(0.85f, 0.1f, 0.1f);
		case EOccupation::Politician: return FLinearColor(0.1f, 0.12f, 0.35f);
		case EOccupation::Influencer: return FLinearColor(1.0f, 0.3f, 0.7f);
		case EOccupation::ConspiracyTheorist: return FLinearColor(0.4f, 0.42f, 0.2f);
		case EOccupation::Celebrity:  return FLinearColor(0.95f, 0.75f, 0.2f);
		case EOccupation::PolarResearcher: return FLinearColor(0.95f, 0.35f, 0.1f);
		default:                      return FLinearColor(0.3f, 0.5f, 0.7f);
		}
	}

	float RegionCenterX(int32 Region)
	{
		return MountainX - RegionWidth * (Region + 0.5f);
	}

	float DockX(int32 Ufo)
	{
		return HangarX + 60.0f + Ufo * 90.0f;
	}

	struct FCanvasPainter
	{
		const FGeometry& Geometry;
		FSlateWindowElementList& Elements;
		int32 Layer;
		float Scale;
		FVector2f Offset;
		const FSlateBrush* Square;
		const TArray<FSlateRoundedBoxBrush>& Rounds;
		const FAlienSprites& Sprites;
		float Time = 0.0f;
		EAlienUniform Uniform = EAlienUniform::LabCoat;
		/** Added to every coordinate; BaseShift while drawing the mountain and the base. */
		FVector2f Shift = FVector2f::ZeroVector;

		FPaintGeometry ToPaint(float X, float Y, float W, float H) const
		{
			return Geometry.ToPaintGeometry(FVector2f(W, H), FSlateLayoutTransform(Scale, Offset + (FVector2f(X, Y) + Shift) * Scale));
		}

		FPaintGeometry operator()(float X, float Y, float W, float H) const { return ToPaint(X, Y, W, H); }

		/** Lets FAlienSprites draw into this canvas. */
		FSpriteCanvas SpriteCanvas() { return { Elements, Layer, *this, Scale * Geometry.GetAccumulatedLayoutTransform().GetScale() }; }

		void Box(float X, float Y, float W, float H, const FLinearColor& Color, bool bRound = false)
		{
			const FSlateBrush* Brush = Square;
			if (bRound)
			{
				Brush = &Rounds[FMath::Clamp(FMath::FloorToInt32(FMath::Min(W, H) * 0.5f), 1, Rounds.Num()) - 1];
			}
			FSlateDrawElement::MakeBox(Elements, ++Layer, ToPaint(X, Y, W, H), Brush, ESlateDrawEffect::None, Color);
		}

		/** Box positioned by its center. */
		void CBox(float CX, float CY, float W, float H, const FLinearColor& Color, bool bRound = false)
		{
			Box(CX - W * 0.5f, CY - H * 0.5f, W, H, Color, bRound);
		}

		void Text(float X, float Y, const FString& String, int32 Size, const FLinearColor& Color, const FName Style = TEXT("Bold"))
		{
			FSlateDrawElement::MakeText(Elements, ++Layer, ToPaint(X, Y, 600.0f, Size * 2.0f), String,
				FCoreStyle::GetDefaultFontStyle(Style, Size), ESlateDrawEffect::None, Color);
		}

		void Room(float X, float Y, float W, float H, const FString& Label, ERoomSprite Sprite)
		{
			Box(X - 3, Y - 3, W + 6, H + 6, RoomEdgeColor);
			if (Sprites.HasRoom(Sprite))
			{
				Sprites.DrawRoom(SpriteCanvas(), Sprite, X, Y, Time);
			}
			else
			{
				Box(X, Y, W, H, RoomColor);
			}
			Text(X + 8, Y + 4, Label, 9, LabelColor);
		}

		void Bar(float X, float Y, float W, float H, float Fraction, const FLinearColor& Color)
		{
			Box(X, Y, W, H, FLinearColor(0.03f, 0.03f, 0.03f));
			Box(X, Y, W * FMath::Clamp(Fraction, 0.0f, 1.0f), H, Color);
		}

		/** Draws a specimen standing on FeetY, centered on CX. */
		void Specimen(ESpecimen Type, float CX, float FeetY, EOccupation Occupation = EOccupation::None)
		{
			switch (Type)
			{
			case ESpecimen::Squirrel:
				if (Sprites.HasSquirrel())
				{
					Sprites.DrawSquirrel(SpriteCanvas(), CX, FeetY, Time);
					break;
				}
				CBox(CX + 8, FeetY - 11, 7, 14, SquirrelColor, true);
				CBox(CX, FeetY - 5, 15, 10, SquirrelColor, true);
				CBox(CX - 7, FeetY - 9, 6, 6, SquirrelColor, true);
				break;
			case ESpecimen::Cow:
				if (Sprites.HasCow())
				{
					Sprites.DrawCow(SpriteCanvas(), CX, FeetY, Time);
					break;
				}
				CBox(CX - 8, FeetY - 3, 3, 7, CowColor);
				CBox(CX + 8, FeetY - 3, 3, 7, CowColor);
				CBox(CX, FeetY - 13, 28, 14, CowColor, true);
				CBox(CX - 3, FeetY - 15, 7, 6, FLinearColor::Black, true);
				CBox(CX + 7, FeetY - 11, 5, 5, FLinearColor::Black, true);
				CBox(CX - 16, FeetY - 18, 10, 9, CowColor, true);
				break;
			case ESpecimen::Elephant:
				CBox(CX - 12, FeetY - 5, 7, 10, ElephantColor);
				CBox(CX + 12, FeetY - 5, 7, 10, ElephantColor);
				CBox(CX, FeetY - 20, 42, 24, ElephantColor, true);
				CBox(CX - 22, FeetY - 26, 16, 16, ElephantColor, true);
				CBox(CX - 29, FeetY - 14, 5, 16, ElephantColor, true);
				CBox(CX - 17, FeetY - 27, 9, 12, FLinearColor(0.38f, 0.38f, 0.43f), true);
				break;
			case ESpecimen::Pufferfish:
			case ESpecimen::Octopus:
			case ESpecimen::Whale:
				if (Sprites.HasSeaCreatures())
				{
					Sprites.DrawSeaCreature(SpriteCanvas(), Type == ESpecimen::Pufferfish ? 0 : Type == ESpecimen::Octopus ? 1 : 2, CX, FeetY, Time);
				}
				else
				{
					const float W = Type == ESpecimen::Whale ? 90.0f : Type == ESpecimen::Octopus ? 22.0f : 14.0f;
					CBox(CX, FeetY - W * 0.25f, W, W * 0.4f, Type == ESpecimen::Octopus ? FLinearColor(0.5f, 0.2f, 0.6f) : Type == ESpecimen::Whale ? FLinearColor(0.2f, 0.3f, 0.5f) : FLinearColor(0.9f, 0.7f, 0.2f), true);
				}
				break;
			case ESpecimen::Penguin:
			case ESpecimen::Seal:
			case ESpecimen::ElephantSeal:
				if (Sprites.HasIceCreatures())
				{
					Sprites.DrawIceCreature(SpriteCanvas(), Type == ESpecimen::Penguin ? 0 : Type == ESpecimen::Seal ? 1 : 2, CX, FeetY, Time);
				}
				else
				{
					const float W = Type == ESpecimen::ElephantSeal ? 70.0f : Type == ESpecimen::Seal ? 30.0f : 12.0f;
					CBox(CX, FeetY - W * 0.25f, W, W * (Type == ESpecimen::Penguin ? 1.4f : 0.45f), Type == ESpecimen::Penguin ? FLinearColor(0.1f, 0.1f, 0.15f) : FLinearColor(0.5f, 0.5f, 0.55f), true);
				}
				break;
			case ESpecimen::Human:
				if (Occupation == EOccupation::PolarResearcher)
				{
					// Three coats, one on top of the other; you can see the cuffs of all three.
					CBox(CX, FeetY - 10, 18, 21, FLinearColor(0.2f, 0.3f, 0.6f), true);
					CBox(CX, FeetY - 11, 16, 20, FLinearColor(0.3f, 0.55f, 0.3f), true);
					CBox(CX, FeetY - 12, 14, 19, OccupationColor(Occupation), true);
					CBox(CX, FeetY - 25, 14, 13, FLinearColor(0.85f, 0.8f, 0.7f), true); // fur hood
					CBox(CX, FeetY - 24, 8, 8, SkinColor, true);
					CBox(CX, FeetY - 21, 10, 3, FLinearColor(0.75f, 0.15f, 0.15f)); // scarf
					break;
				}
				CBox(CX, FeetY - 10, 10, 20, OccupationColor(Occupation), true);
				CBox(CX, FeetY - 24, 10, 10, SkinColor, true);
				if (Occupation == EOccupation::Magician)
				{
					CBox(CX, FeetY - 31, 8, 7, FLinearColor::Black);
				}
				else if (Occupation == EOccupation::MallSanta)
				{
					CBox(CX, FeetY - 18, 10, 6, FLinearColor::White, true);
				}
				else if (Occupation == EOccupation::ConspiracyTheorist)
				{
					CBox(CX, FeetY - 31, 6, 5, FLinearColor(0.8f, 0.82f, 0.86f)); // tinfoil hat
					CBox(CX, FeetY - 34, 3, 3, FLinearColor(0.8f, 0.82f, 0.86f));
				}
				else if (Occupation == EOccupation::Celebrity)
				{
					CBox(CX - 1, FeetY - 25, 9, 3, FLinearColor::Black); // sunglasses, at night
				}
				else if (Occupation == EOccupation::Influencer)
				{
					CBox(CX - 7, FeetY - 24, 3, 5, FLinearColor(0.1f, 0.1f, 0.12f)); // phone, recording
					CBox(CX - 7, FeetY - 25, 1, 1, FLinearColor(1.0f, 0.2f, 0.2f));
				}
				break;
			default:
				break;
			}
		}

		void Alien(float CX, float FeetY)
		{
			if (Sprites.HasAlien())
			{
				Sprites.DrawAlien(SpriteCanvas(), CX, FeetY, Time, Uniform);
				return;
			}
			CBox(CX, FeetY - 14, 14, 26, AlienColor, true);
			CBox(CX, FeetY - 34, 20, 18, AlienColor, true);
			CBox(CX - 4, FeetY - 35, 4, 6, FLinearColor::Black, true);
			CBox(CX + 4, FeetY - 35, 4, 6, FLinearColor::Black, true);
		}
	};

	FVector2f LerpPath(const FVector2f& A, const FVector2f& B, const FVector2f& C, float T)
	{
		if (T < ExitFraction)
		{
			return FMath::Lerp(A, B, T / ExitFraction);
		}
		return FMath::Lerp(B, C, (T - ExitFraction) / (1.0f - ExitFraction));
	}
}

void SAlienBaseView::Construct(const FArguments& InArgs)
{
	Sim = InArgs._Sim;
	OnCalibrationClicked = InArgs._OnCalibrationClicked;
	check(Sim.IsValid());
	for (int32 Radius = 1; Radius <= 30; ++Radius)
	{
		RoundBrushes.Emplace(FLinearColor::White, float(Radius));
	}
	Sprites.Load();
	// Repaint every frame; the scene animates continuously.
	ForceVolatile(true);
	// Zoomed-in drawing must not spill over the console.
	SetClipping(EWidgetClipping::ClipToBounds);
}

namespace
{
	constexpr float MaxZoom = 4.0f;
	constexpr float ZoomStep = 1.15f;
}

namespace
{
	TAutoConsoleVariable<int32> CVarPreviewEarth(
		TEXT("AlienIdle.PreviewEarth"), 0,
		TEXT("Visual preview only: every Earth response active in every region, plus a condition: 1 business as usual, 2 documentary craze, 3 surveillance crackdown, 4 everything is advertising."));

	TAutoConsoleVariable<int32> CVarPreviewRival(
		TEXT("AlienIdle.PreviewRival"), 0,
		TEXT("Visual preview only: 1/2/3 shows the Efficiency Bureau sweeping that region (1 = nearest the base); 11/12/13 a Celebrity Studies stunt; 21/22/23 an Experimental Sciences breach."));

	TAutoConsoleVariable<int32> CVarPreviewLocation(
		TEXT("AlienIdle.PreviewLocation"), 0,
		TEXT("Visual preview only: 1 draws Metropolis, 2 the Oceanic base, 3 Station Permafrost (Antarctic), instead of the mountain."));

	TAutoConsoleVariable<int32> CVarPreviewIce(
		TEXT("AlienIdle.PreviewIce"), 0,
		TEXT("Visual preview only (Antarctic, and Cryo Pods / Insulated Labs anywhere): 1 freezing with the heating budget cut, 2 overheated (meltwater; the humans blame the cows). Both show geothermal taps, the cryo vault, cryo pods and insulated labs."));

	TAutoConsoleVariable<int32> CVarPreviewOcean(
		TEXT("AlienIdle.PreviewOcean"), 0,
		TEXT("Visual preview only (Oceanic): 1 shows tidal turbines at high tide, deep tanks, a creaking hull at 80 pressure, the airlock leak and a whale singing."));

	TAutoConsoleVariable<int32> CVarPreviewDepartment(
		TEXT("AlienIdle.PreviewDepartment"), 0,
		TEXT("Visual preview only: 1 Biological, 2 Human, 3 Incompetence draws that department's uniform, all its equipment and its trophy. Add 10 to also show its signature experiment running."));

	TAutoConsoleVariable<float> CVarViewX(
		TEXT("AlienIdle.ViewX"), 0.0f,
		TEXT("Screenshots: when above 0, centre the map view on this canvas x (e.g. 330 for the hunting grounds)."));

	TAutoConsoleVariable<int32> CVarFitWholeMap(
		TEXT("AlienIdle.FitWholeMap"), 0,
		TEXT("1: show the whole map (peak to base) at zoom 1 instead of filling the width. Handy for screenshots."));

	/** Keeps a view centered on Center inside the map; an axis the view already covers is centered. */
	FVector2f ClampCenter(const FVector2f& Size, float Scale, FVector2f Center)
	{
		const FVector2f HalfView = Size * 0.5f / Scale;
		for (int32 Axis = 0; Axis < 2; ++Axis)
		{
			Center[Axis] = HalfView[Axis] * 2.0f >= Canvas[Axis]
				? (CanvasMin[Axis] + CanvasMax[Axis]) * 0.5f
				: FMath::Clamp(Center[Axis], CanvasMin[Axis] + HalfView[Axis], CanvasMax[Axis] - HalfView[Axis]);
		}
		return Center;
	}
}

void SAlienBaseView::GetView(const FVector2f& Size, float& OutScale, FVector2f& OutOffset) const
{
	// Show ViewWidth across the widget; the map is wider and taller, so the player drags to look around.
	const float FitScale = CVarFitWholeMap.GetValueOnGameThread() != 0
		? FMath::Min(Size.X / Canvas.X, Size.Y / Canvas.Y)
		: Size.X / ViewWidth;
	OutScale = FitScale * Zoom;
	FVector2f Center = ViewCenter;
	if (const float ForcedX = CVarViewX.GetValueOnGameThread(); ForcedX > 0.0f)
	{
		Center.X = ForcedX;
	}
	OutOffset = Size * 0.5f - ClampCenter(Size, OutScale, Center) * OutScale;
}

void SAlienBaseView::ClampViewCenter(const FVector2f& Size)
{
	float Scale;
	FVector2f Offset;
	GetView(Size, Scale, Offset);
	ViewCenter = ClampCenter(Size, Scale, ViewCenter);
}

FReply SAlienBaseView::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2f Size = MyGeometry.GetLocalSize();
	const FVector2f Cursor = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	float Scale;
	FVector2f Offset;
	GetView(Size, Scale, Offset);
	const FVector2f Anchor = (Cursor - Offset) / Scale; // canvas point under the cursor

	Zoom = FMath::Clamp(Zoom * FMath::Pow(ZoomStep, MouseEvent.GetWheelDelta()), 1.0f, MaxZoom);

	// Keep the anchor under the cursor at the new zoom.
	GetView(Size, Scale, Offset);
	const FVector2f NewOffset = Cursor - Anchor * Scale;
	ViewCenter = (Size * 0.5f - NewOffset) / Scale;
	ClampViewCenter(Size);
	return FReply::Handled();
}

FReply SAlienBaseView::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bDragging = true;
		LastDragPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		PressPosition = LastDragPosition;
		bPressMoved = false;
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	return FReply::Unhandled();
}

FReply SAlienBaseView::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bDragging)
	{
		bDragging = false;
		const int32 Chamber = Sim->GetCalibrationChamber();
		if (!bPressMoved && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Chamber != INDEX_NONE && !Sim->IsCalibrationOpen() && OnCalibrationClicked)
		{
			// A click on the chamber's calibration tag opens the challenge.
			float Scale;
			FVector2f Offset;
			GetView(MyGeometry.GetLocalSize(), Scale, Offset);
			const FVector2f Point = (MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - Offset) / Scale;
			const FVector2f TagPos = CalibrationTagPos(Chamber) + BaseOrigin;
			const float Slack = 6.0f;
			if (Point.X >= TagPos.X - Slack && Point.X <= TagPos.X + CalibrationTagSize.X + Slack && Point.Y >= TagPos.Y - Slack && Point.Y <= TagPos.Y + CalibrationTagSize.Y + Slack)
			{
				OnCalibrationClicked();
			}
		}
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply SAlienBaseView::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (!bDragging)
	{
		return FReply::Unhandled();
	}
	const FVector2f Size = MyGeometry.GetLocalSize();
	const FVector2f Position = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	bPressMoved |= FVector2f::Distance(Position, PressPosition) > 4.0f;
	float Scale;
	FVector2f Offset;
	GetView(Size, Scale, Offset);
	ViewCenter -= (Position - LastDragPosition) / Scale;
	LastDragPosition = Position;
	ClampViewCenter(Size);
	return FReply::Handled();
}

FReply SAlienBaseView::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Zoom = 1.0f;
	ViewCenter = FVector2f(1260.0f, CanvasMax.Y);
	return FReply::Handled();
}

int32 SAlienBaseView::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2f Size = AllottedGeometry.GetLocalSize();
	float Scale;
	FVector2f Offset;
	GetView(Size, Scale, Offset);

	FCanvasPainter P{ AllottedGeometry, OutDrawElements, LayerId, Scale, Offset,
		FCoreStyle::Get().GetBrush("GenericWhiteBox"), RoundBrushes, Sprites };

	const FAlienIdleSim& S = *Sim;
	const float Time = float(S.GetElapsedSeconds());
	P.Time = Time;

	// Research departments change what the base looks like: uniforms, equipment, trophies.
	// AlienIdle.PreviewDepartment fakes a department for screenshots without touching the sim.
	EDepartment Dept = S.GetPrimaryDepartment();
	bool ProjectDone[int32(EDeptProject::Count)];
	bool SignatureDone[int32(EDepartment::Count)];
	bool SignatureRunning[int32(EDepartment::Count)];
	for (int32 i = 0; i < int32(EDeptProject::Count); ++i)
	{
		ProjectDone[i] = S.IsProjectDone(EDeptProject(i));
	}
	for (int32 d = 0; d < int32(EDepartment::Count); ++d)
	{
		SignatureDone[d] = d != 0 && S.IsSignatureDone(EDepartment(d));
		SignatureRunning[d] = d != 0 && S.IsSignatureRunning(EDepartment(d));
	}
	if (const int32 Preview = CVarPreviewDepartment.GetValueOnGameThread(); Preview > 0)
	{
		Dept = EDepartment(FMath::Clamp(Preview % 10, 1, 3));
		for (int32 k = 0; k < 3; ++k)
		{
			ProjectDone[(int32(Dept) - 1) * 3 + k] = true;
		}
		SignatureDone[int32(Dept)] = true;
		SignatureRunning[int32(Dept)] = Preview >= 10;
	}
	auto Done = [&](EDeptProject Project) { return ProjectDone[int32(Project)]; };
	const int32 PreviewLocation = CVarPreviewLocation.GetValueOnGameThread();
	const ELocation Location = PreviewLocation == 1 ? ELocation::Metropolis : PreviewLocation == 2 ? ELocation::Oceanic
		: PreviewLocation == 3 ? ELocation::Antarctic : S.GetLocation();
	const bool bOcean = Location == ELocation::Oceanic;
	const bool bPreviewOcean = CVarPreviewOcean.GetValueOnGameThread() > 0;
	// Station Permafrost: how warm the labs and hangar are, and whether the heat is overdone (meltwater).
	const bool bIce = Location == ELocation::Antarctic;
	const int32 PreviewIce = CVarPreviewIce.GetValueOnGameThread();
	const bool bHeat = bIce && (S.HasHeat() || PreviewIce > 0);
	const float HeatDemand = S.HasHeat() ? S.GetHeatDemandLabs() + S.GetHeatDemandHangar() : 0.0f;
	const bool bHeatingCut = PreviewIce == 1 || (bIce && S.GetIncident().Type == EIncident::HeatingCut);
	const float LabWarmth = PreviewIce == 1 ? 0.0f : PreviewIce == 2 ? 1.0f : S.HasHeat() ? S.GetLabWarmth() : 1.0f;
	const float HangarWarmth = PreviewIce == 1 ? 0.0f : PreviewIce == 2 ? 1.0f : S.HasHeat() ? S.GetHangarWarmth() : 1.0f;
	const bool bMelt = bHeat && (PreviewIce == 2 || (PreviewIce == 0 && HeatDemand > 0.0f && S.GetHeatSupply() > 1.6f * HeatDemand));
	P.Uniform = Dept == EDepartment::Biological ? EAlienUniform::Biological
		: Dept == EDepartment::Human ? EAlienUniform::Human
		: Dept == EDepartment::Incompetence ? EAlienUniform::Incompetence
		: EAlienUniform::LabCoat;

	// Letterbox fill, then sky.
	FSlateDrawElement::MakeBox(OutDrawElements, ++P.Layer, AllottedGeometry.ToPaintGeometry(), P.Square, ESlateDrawEffect::None, FLinearColor(0.02f, 0.02f, 0.04f));
	P.Box(CanvasMin.X, CanvasMin.Y, Canvas.X, Canvas.Y, SkyColor);
	for (int32 i = 0; i < 180; ++i)
	{
		P.Box(float((i * 7919) % 2100), CanvasMin.Y + float((i * 104729) % 530), 2, 2, StarColor);
	}
	P.CBox(430, 45, 50, 50, Srgb(0.9f, 0.9f, 0.8f), true);

	// Hunting grounds, nearest the mountain first.
	P.Box(0, GroundY, MountainX, CanvasMax.Y - GroundY, bIce ? FLinearColor::FromSRGBColor(FColor(0x5a, 0x7e, 0x9c)) : DirtColor); // the ice shelf goes down a long way
	P.Box(0, GroundY, MountainX, 10, bIce ? Srgb(0.95f, 0.98f, 1.0f) : GrassColor);
	// Earth notices: each region shows the responses it has organised, and the planet's mood shows everywhere.
	const int32 PreviewEarth = CVarPreviewEarth.GetValueOnGameThread();
	const int32 Condition = PreviewEarth > 0 ? FMath::Clamp(PreviewEarth - 1, 0, 3) : int32(S.GetEarthCondition());
	static const TCHAR* ResponseNames[] = { TEXT("night patrols"), TEXT("investigators"), TEXT("radar"), TEXT("support group") };
	// The Efficiency Bureau, our rival, reserves a hunting ground and sweeps it on a schedule.
	const FRivalState& Rival = S.GetRival();
	const int32 PreviewRival = CVarPreviewRival.GetValueOnGameThread();
	const int32 PreviewWho = PreviewRival / 10;       // 0 Bureau, 1 Celebrity Studies, 2 Experimental Sciences
	const int32 PreviewWhere = FMath::Clamp(PreviewRival % 10 - 1, 0, 2);
	const bool bRivalSweeping = (PreviewRival > 0 && PreviewWho == 0) || Rival.bSweeping;
	const bool bRivalReserved = bRivalSweeping || Rival.bAnnounced;
	const int32 RivalRegion = PreviewRival > 0 && PreviewWho == 0 ? PreviewWhere : Rival.SweepRegion;
	// Celebrity Studies stages stunts; Experimental Sciences has breaches.
	const bool bPreviewCeleb = PreviewRival > 0 && PreviewWho == 1;
	const bool bPreviewBreach = PreviewRival > 0 && PreviewWho == 2;
	auto IsStunt = [&](int32 R) { return bPreviewCeleb ? R == PreviewWhere : S.IsCelebrityStunt(R); };
	auto IsBreach = [&](int32 R) { return bPreviewBreach ? R == PreviewWhere : S.IsBreach(R); };
	auto IsBreachTaped = [&](int32 R) { return IsBreach(R) || (Rival.Breach.bAnnounced && Rival.Breach.Region == R); };
	for (int32 r = 0; r < S.GetNumRegions(); ++r)
	{
		const AlienIdleTuning::FRegionDef& Def = AlienIdleTuning::RegionDefs[int32(Location)][r];
		const FRegionState& Region = S.GetRegionState(r);
		const float X0 = MountainX - RegionWidth * (r + 1);

		if (P.Sprites.HasRegions())
		{
			if (bOcean && r != 1)
			{
				// Coral Shallows and The Trench are under water; Harbour Town is up on land.
				P.Box(X0, CanvasMin.Y, RegionWidth, GroundY - CanvasMin.Y, r == 2 ? FLinearColor(0.0f, 0.01f, 0.04f, 0.85f) : FLinearColor(0.02f, 0.12f, 0.25f, 0.6f));
			}
			P.Sprites.DrawRegion(P.SpriteCanvas(), int32(Location), r, X0, GroundY - 200.0f, Time);
		}
		else
		{
			for (int32 t = 0; t < 5; ++t)
			{
				const float X = X0 + 8 + t * 42;
				P.Box(X + 8, GroundY - 25, 8, 25, TrunkColor);
				P.Box(X, GroundY - 55, 24, 32, TreeColor);
			}
		}

		// The planet's mood: a joke in the background of every region.
		if (Condition > 0 && P.Sprites.HasRegions())
		{
			const float CondX[] = { 0.0f, 96.0f, 120.0f, 92.0f };
			P.Sprites.DrawCondition(P.SpriteCanvas(), Condition, X0 + CondX[Condition], GroundY - 4.0f, Time, 0.8f);
		}

		// Specimens loitering, daring you.
		bool bAnyUnlocked = false;
		ESpecimen FirstLocked = ESpecimen::Count;
		for (int32 k = 0; k < Def.NumSpecimens; ++k)
		{
			const ESpecimen Type = Def.Specimens[k];
			if (!S.IsSpecimenUnlocked(Type) && PreviewLocation == 0)
			{
				FirstLocked = FirstLocked == ESpecimen::Count ? Type : FirstLocked;
				continue;
			}
			bAnyUnlocked = true;
			const float CX = X0 + RegionWidth * (k + 1) / (Def.NumSpecimens + 1);
			P.Specimen(Type, CX + 18.0f * FMath::Sin(Time * 0.5f + r * 1.7f + k), GroundY + 2.0f);
		}

		// Earth's organised responses, each in its own spot.
		const float RespX[] = { 6.0f, 50.0f, 118.0f, 156.0f };
		for (int32 e = 0; e < int32(EResponse::Count); ++e)
		{
			if ((Region.Active[e] || PreviewEarth > 0) && P.Sprites.HasRegions())
			{
				P.Sprites.DrawResponse(P.SpriteCanvas(), e, X0 + RespX[e], GroundY + 4.0f, Time);
			}
		}

		if (bRivalReserved && r == RivalRegion)
		{
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_sign_body"), X0 + 80.0f, GroundY - 22.0f, 60.0f, 26.0f);
		}
		if (IsStunt(r))
		{
			// Red carpet, velvet ropes, a live news van, and paparazzi who never stop.
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_carpet_body"), X0 + 50.0f, GroundY - 36.0f, 120.0f, 40.0f);
			if (FMath::Frac(FMath::Sin(FMath::FloorToFloat(Time * 7.0f) * 12.9898f) * 43758.5453f) < 0.35f)
			{
				P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_carpet_flashes"), X0 + 50.0f, GroundY - 36.0f, 120.0f, 40.0f);
			}
		}
		if (IsBreachTaped(r))
		{
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_tape_body"), X0 + 50.0f, GroundY - 16.0f, 120.0f, 20.0f);
		}
		if (IsBreach(r))
		{
			// Escaped experiments wander about, unbothered.
			const float SqX = X0 + 110.0f + 80.0f * FMath::Sin(Time * 0.45f + r);
			const float CowX = X0 + 110.0f + 70.0f * FMath::Sin(Time * 0.3f + r + 2.0f);
			const float Glow = 0.5f + 0.5f * FMath::Sin(Time * 3.0f);
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_creatures_glow"), CowX - 15.0f, GroundY - 22.0f, 30.0f, 24.0f, 0.0f, FVector2f::ZeroVector, FLinearColor::White.CopyWithNewOpacity(Glow));
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_creatures_cow"), CowX - 15.0f, GroundY - 22.0f, 30.0f, 24.0f);
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_creatures_squirrel"), SqX - 10.0f, GroundY - 14.0f, 20.0f, 16.0f);
		}

		// Labels: name, suspicion, alert state, and what Earth has organised here.
		P.Text(X0 + 8, GroundY + 16, FString(Def.Name).ToUpper(), 9, LabelColor);
		if (bAnyUnlocked)
		{
			const float Fraction = float(Region.Suspicion / AlienIdleTuning::LockdownSuspicion);
			const bool bAlert = S.IsRegionIndexOnAlert(r);
			const FLinearColor BarColor = Region.bLockedDown ? FLinearColor(0.9f, 0.05f, 0.05f) : bAlert ? WarnColor : FLinearColor(0.3f, 0.7f, 0.3f);
			P.Text(X0 + 8, GroundY + 36, TEXT("suspicion"), 8, LabelColor, TEXT("Regular"));
			P.Bar(X0 + 8, GroundY + 56, RegionWidth - 20, 6, Fraction, BarColor);
			if (Region.bLockedDown)
			{
				P.Text(X0 + 8, GroundY + 66, TEXT("LOCKDOWN"), 9, WarnColor);
			}
			else if (bAlert)
			{
				P.Text(X0 + 8, GroundY + 66, TEXT("ON ALERT"), 9, WarnColor);
			}
			int32 Line = 0;
			for (int32 e = 0; e < int32(EResponse::Count); ++e)
			{
				const bool bActive = Region.Active[e] || PreviewEarth > 0;
				if (bActive || Region.Pressure[e] >= 0.5f)
				{
					const FString Text = bActive ? FString(ResponseNames[e]) : FString(ResponseNames[e]) + TEXT(" (organising)");
					P.Text(X0 + 8, GroundY + 88 + Line * 16, Text, 8, bActive ? WarnColor : LabelColor, TEXT("Regular"));
					++Line;
				}
			}
		}
		else
		{
			P.Box(X0, CanvasMin.Y, RegionWidth, Canvas.Y, FLinearColor(0.0f, 0.0f, 0.0f, 0.55f));
			P.Text(X0 + 8, GroundY + 36, TEXT("NOT YET APPROVED"), 8, LabelColor);
			if (FirstLocked != ESpecimen::Count && AlienIdleTuning::Specimen(FirstLocked).UnlockedBy != EDiscovery::None)
			{
				P.Text(X0 + 8, GroundY + 54, FString::Printf(TEXT("needs '%s'"), AlienIdleTuning::Discovery(AlienIdleTuning::Specimen(FirstLocked).UnlockedBy).Name), 8, LabelColor, TEXT("Regular"));
			}
		}
	}

	// The Bureau's ship hovers over the region it is sweeping, scanning it with a grey, regulation beam.
	if (bRivalSweeping)
	{
		const float Bob = 3.0f * FMath::Sin(Time * 1.5f);
		const FVector2f Ship(RegionCenterX(RivalRegion) + 50.0f * FMath::Sin(Time * 0.35f), HoverY - 45.0f + Bob);
		const float BeamX = Ship.X + 30.0f * FMath::Sin(Time * 1.1f);
		P.Box(BeamX - 18.0f, Ship.Y + 8.0f, 36.0f, GroundY - Ship.Y - 8.0f, FLinearColor(0.75f, 0.78f, 0.82f, 0.18f));
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_bureau_body"), Ship.X - 48.0f, Ship.Y - 26.0f, 96.0f, 48.0f);
		if (FMath::Frac(Time * 2.0f) < 0.5f)
		{
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_bureau_lights"), Ship.X - 48.0f, Ship.Y - 26.0f, 96.0f, 48.0f);
		}
		const FString What = PreviewRival > 0 ? FString(TEXT("specimens")) : FString(AlienIdleTuning::Specimen(Rival.SweepType).Name).ToLower() + TEXT("s");
		P.Text(Ship.X - 60.0f, Ship.Y - 40.0f, FString::Printf(TEXT("EFFICIENCY BUREAU: sweeping %s"), *What), 7, FLinearColor(0.85f, 0.88f, 0.92f));
	}
	// The other rivals' ships hover over their stunt or breach.
	for (int32 r = 0; r < S.GetNumRegions(); ++r)
	{
		const bool bStunt = IsStunt(r);
		const bool bBreach = IsBreach(r);
		if (!bStunt && !bBreach)
		{
			continue;
		}
		const float Bob = 3.0f * FMath::Sin(Time * 1.7f + r);
		const FVector2f Ship(RegionCenterX(r) + 40.0f * FMath::Sin(Time * 0.4f + r), HoverY - 70.0f + Bob);
		if (bStunt)
		{
			// A spotlight on the red carpet, from orbit.
			P.Box(Ship.X - 10.0f, Ship.Y + 8.0f, 20.0f, GroundY - Ship.Y - 8.0f, FLinearColor(1.0f, 0.6f, 0.85f, 0.15f));
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_celebrity_body"), Ship.X - 48.0f, Ship.Y - 26.0f, 96.0f, 48.0f);
			if (FMath::Frac(Time * 1.5f) < 0.5f)
			{
				P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_celebrity_sparkle"), Ship.X - 48.0f, Ship.Y - 26.0f, 96.0f, 48.0f);
			}
			P.Text(Ship.X - 60.0f, Ship.Y - 40.0f, TEXT("CELEBRITY STUDIES: live from the red carpet"), 7, FLinearColor(1.0f, 0.75f, 0.9f));
		}
		else
		{
			const float Tilt = 6.0f * FMath::Sin(Time * 0.9f);
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_experimental_goo"), Ship.X - 48.0f, Ship.Y - 26.0f + 2.0f * FMath::Frac(Time), 96.0f, 48.0f, Tilt, FVector2f(48.0f, 28.0f));
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("rival_experimental_body"), Ship.X - 48.0f, Ship.Y - 26.0f, 96.0f, 48.0f, Tilt, FVector2f(48.0f, 28.0f));
			P.Text(Ship.X - 60.0f, Ship.Y - 40.0f, TEXT("EXPERIMENTAL SCIENCES: containment is a suggestion"), 7, FLinearColor(0.7f, 1.0f, 0.6f));
		}
	}

	// Everything from here to the UFOs is in base-local coordinates.
	P.Shift = FVector2f(BaseShift, 0.0f);

	// The mountain: hallways between the rooms and the cabin on top (or a stepped placeholder).
	if (P.Sprites.HasMountain())
	{
		bool bUfoOut = false;
		for (int32 u = 0; u < S.GetNumUfos(); ++u)
		{
			bUfoOut |= S.GetUfo(u).State != EUfoState::Docked && S.GetUfo(u).State != EUfoState::WaitingToUnload;
		}
		FOceanLook Ocean;
		if (bOcean)
		{
			Ocean.HullPressure = bPreviewOcean ? 80.0f : (S.HasHull() ? S.GetHullPressure() : 0.0f);
			Ocean.Tide = bPreviewOcean ? 1.0f : S.GetTideFactor();
			Ocean.bTurbines = bPreviewOcean || S.GetBaseLevel(EBaseUpgrade::TidalTurbines) > 0;
			Ocean.bTanks = bPreviewOcean || S.GetBaseLevel(EBaseUpgrade::DeepTanks) > 0;
			Ocean.bWhaleSong = bPreviewOcean || S.GetIncident().Type == EIncident::WhaleSong;
		}
		FIceLook Ice;
		if (bIce)
		{
			Ice.Fire = PreviewIce == 1 ? 0.3f : PreviewIce == 2 ? 1.0f : HeatDemand > 0.0f ? S.GetHeatSupply() / HeatDemand : 1.0f;
			Ice.bTaps = PreviewIce > 0 || S.GetBaseLevel(EBaseUpgrade::GeothermalTaps) > 0;
			Ice.bVault = PreviewIce > 0 || S.GetBaseLevel(EBaseUpgrade::CryoVault) > 0;
		}
		P.Sprites.DrawMountain(P.SpriteCanvas(), Time, bUfoOut, int32(Location), Ocean, Ice);
		if (bIce && P.Sprites.HasIce())
		{
			// The Boiler Room stoker shovels coal from the pile into the furnace, all day, every day.
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("stoker_shovel"), 652, 379, 28, 42, 25.0f * FMath::Sin(Time * 3.0f), FVector2f(12.0f, 28.0f) * 0.875f);
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("stoker_body"), 652, 379, 28, 42);
			if (bHeatingCut)
			{
				P.Box(764, 400, 76, 16, FLinearColor(0.05f, 0.06f, 0.08f, 0.85f));
				P.Text(768, 401, TEXT("HEATING BUDGET CUT"), 7, WarnColor);
			}
		}
		if (bOcean)
		{
			// The Pressure Control technician minds the console; the dome complains above 60.
			P.Alien(645, 421);
			if (Ocean.HullPressure >= 60.0f && FMath::Frac(Time * 0.7f) < 0.35f)
			{
				P.Text(990, 66, TEXT("*creeeak*"), 9, FLinearColor(0.9f, 0.95f, 1.0f));
			}
		}
	}
	else
	{
		P.Box(MountainX - BaseShift, HangarY - 10, 1460, CanvasMax.Y - HangarY + 10, RockColor);
		P.Box(720, 210, 560, 50, RockColor);
		P.Box(780, 160, 480, 50, RockColor);
		P.Box(850, 115, 400, 45, RockColor);
		P.Box(920, 80, 300, 35, RockColor);
		P.Box(980, 55, 160, 25, SnowColor);
		P.Box(920, 80, 300, 10, SnowColor);
	}

	// Hangar: open on the cliff face, one pad per UFO.
	if (P.Sprites.HasRoom(ERoomSprite::Hangar))
	{
		P.Sprites.DrawRoom(P.SpriteCanvas(), ERoomSprite::Hangar, HangarX, HangarY, Time);
	}
	else
	{
		P.Box(HangarX, HangarY, 300, 100, RoomColor);
	}
	P.Box(HangarX, HangarY - 3, 303, 3, RoomEdgeColor);
	P.Box(HangarX, HangarY + 100, 303, 3, RoomEdgeColor);
	P.Box(HangarX + 300, HangarY - 3, 3, 106, RoomEdgeColor);
	if (bOcean && (bPreviewOcean || S.IsAirlockSealed()))
	{
		// Airlock sealed: the hangar ceiling leaks into a bucket, and someone put up a poster.
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("leak_body"), HangarX + 205.0f, HangarY + 34.0f, 40.0f, 60.0f);
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("leak_drop"), HangarX + 205.0f, HangarY + 34.0f + 40.0f * FMath::Frac(Time * 1.3f), 40.0f, 60.0f);
	}
	if (bHeat && HangarWarmth < 0.95f)
	{
		// A cold hangar frosts over; the icicles grow as the warmth drops.
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("frost_frost"), HangarX, HangarY, 300.0f, 100.0f, 0.0f, FVector2f::ZeroVector, FLinearColor::White.CopyWithNewOpacity(1.0f - HangarWarmth));
		P.Box(HangarX + 6, HangarY + 4, 96, 26, FLinearColor(0.05f, 0.06f, 0.08f, 0.8f));
	}
	P.Text(HangarX + 10, HangarY + 5, FString::Printf(TEXT("HANGAR  %d UFO%s"), S.GetNumUfos(), S.GetNumUfos() == 1 ? TEXT("") : TEXT("s")), 9, LabelColor);
	// The base's play style, as the Academy sees it: a badge on the hangar wall.
	{
		bool bClear = false;
		const AlienIdleTuning::EApproach Approach = S.ClassifyApproach(&bClear);
		static const FLinearColor ApproachColors[] = { FLinearColor(1.0f, 0.55f, 0.15f), FLinearColor(0.3f, 0.85f, 1.0f), FLinearColor(0.7f, 0.45f, 1.0f) };
		const FLinearColor Color = bClear ? ApproachColors[int32(Approach)] : FLinearColor(0.6f, 0.62f, 0.66f);
		P.Box(HangarX + 176, HangarY + 4, 118, 15, FLinearColor(0.05f, 0.06f, 0.08f, 0.85f));
		P.Box(HangarX + 176, HangarY + 4, 3, 15, Color);
		const FString Badge = bClear ? FString::Printf(TEXT("APPROACH: %s"), AlienIdleTuning::Approach(Approach).Name).ToUpper() : FString(TEXT("APPROACH: UNDECIDED"));
		P.Text(HangarX + 183, HangarY + 5, Badge, 7, Color);
	}
	for (int32 i = 0; i < 3; ++i)
	{
		P.CBox(DockX(i), HangarY + 90, 76, 6, i < S.GetNumUfos() ? FLinearColor(0.85f, 0.7f, 0.1f) : FLinearColor(0.25f, 0.22f, 0.1f));
	}

	// Containment: one floor tile per unit; specimens stand on the tiles they occupy.
	P.Shift = BaseOrigin + ContainmentOffset;
	const int32 Capacity = S.GetContainmentCapacity();
	P.Room(975, 300, 305, 100, FString::Printf(TEXT("CONTAINMENT  %d/%d units"), S.GetContainmentUnitsUsed(), Capacity), ERoomSprite::Containment);
	constexpr int32 TilesPerRow = 14;
	constexpr float TileW = 20.0f;
	for (int32 u = 0; u < Capacity; ++u)
	{
		P.Box(985 + (u % TilesPerRow) * TileW, 326 + (u / TilesPerRow) * 36, TileW - 2, 32, FLinearColor(0.07f, 0.08f, 0.09f));
	}
	if (S.GetBaseLevel(EBaseUpgrade::CryoPods) > 0 || PreviewIce > 0)
	{
		// Cryo Pods at the back of containment: samples keep forever. One of them is a squirrel.
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("cryopods_body"), 1222, 358, 50, 40);
		if (FMath::Frac(Time * 0.8f) < 0.6f)
		{
			P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("cryopods_lights"), 1222, 358, 50, 40);
		}
	}
	{
		int32 Unit = 0;
		for (const FSpecimen& Held : S.GetContainment())
		{
			const int32 Units = AlienIdleTuning::Specimen(Held.Type).ContainmentUnits;
			if (Unit % TilesPerRow + Units > TilesPerRow)
			{
				Unit += TilesPerRow - Unit % TilesPerRow; // don't split a specimen across rows
			}
			const float CX = 985 + (Unit % TilesPerRow) * TileW + Units * TileW * 0.5f - 1.0f;
			P.Specimen(Held.Type, CX, 326 + (Unit / TilesPerRow) * 36 + 32, Held.Occupation);
			Unit += Units;
		}
	}
	if (S.GetContainmentUnitsUsed() >= Capacity)
	{
		P.Text(1160, 305, TEXT("FULL"), 9, WarnColor);
	}

	// Reinforced Habitats: heavy bars in front of the specimens.
	if (Done(EDeptProject::ReinforcedHabitats))
	{
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::Containment, TEXT("bars"), 975, 300);
	}

	// Lab: one table per experiment chamber.
	P.Shift = BaseOrigin + LabOffset;
	const TArray<FLabChamber>& Chambers = S.GetChambers();
	P.Room(675, 425, 330, 115, FString::Printf(TEXT("LAB  %d chamber%s"), Chambers.Num(), Chambers.Num() == 1 ? TEXT("") : TEXT("s")), ERoomSprite::Lab);
	if (S.GetBaseLevel(EBaseUpgrade::InsulatedLabs) > 0 || PreviewIce > 0)
	{
		// Insulated Labs: penguin-style cladding. Huddle Theory paid off.
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("frost_cladding"), 675, 425, 330, 115);
	}
	if (bHeat && LabWarmth < 0.95f)
	{
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("frost_frost"), 675, 425, 330, 115, 0.0f, FVector2f::ZeroVector, FLinearColor::White.CopyWithNewOpacity(1.0f - LabWarmth));
	}
	if (bMelt)
	{
		// Too much heat: the walls sweat, puddles form, and the humans blame the cows.
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("frost_melt"), 675, 425, 330, 115);
	}
	if (bHeat && LabWarmth < 0.95f)
	{
		P.Box(680, 429, 96, 14, FLinearColor(0.05f, 0.06f, 0.08f, 0.8f));
		P.Text(683, 429, FString::Printf(TEXT("LAB  %d chamber%s"), Chambers.Num(), Chambers.Num() == 1 ? TEXT("") : TEXT("s")), 9, LabelColor); // over the frost
	}
	// Behavioral Lab: a two-way mirror, with someone always behind it.
	if (Done(EDeptProject::BehavioralLab))
	{
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::Lab, TEXT("mirror"), 675, 425);
	}
	P.Alien(700, 520);
	if (S.IsMiddleManagementOn())
	{
		P.Box(698, 494, 4, 14, FLinearColor(0.8f, 0.1f, 0.1f)); // management tie
	}
	for (int32 c = 0; c < 3; ++c)
	{
		const float X = 725.0f + c * 92.0f;
		if (c >= Chambers.Num())
		{
			P.Box(X, 495, 80, 10, FLinearColor(0.2f, 0.21f, 0.24f));
			continue;
		}
		const FLabChamber& Chamber = Chambers[c];
		P.Box(X, 495, 80, 10, FLinearColor(0.75f, 0.78f, 0.82f));
		if (Done(EDeptProject::BatchExperiments))
		{
			// Batch Experiments: a second tier on every exam table.
			P.Box(X + 4, 512, 72, 4, FLinearColor(0.6f, 0.63f, 0.68f));
		}
		P.Box(X + 6, 505, 5, 20, RoomEdgeColor);
		P.Box(X + 69, 505, 5, 20, RoomEdgeColor);
		if (Chamber.bRunning)
		{
			if (Chamber.Batch >= 2)
			{
				// Two of the same species, studied together. They have formed a union.
				P.Specimen(Chamber.Specimen.Type, X + 22, 495, Chamber.Specimen.Occupation);
				P.Specimen(Chamber.Specimen.Type, X + 58, 495, Chamber.Specimen.Occupation);
			}
			else
			{
				P.Specimen(Chamber.Specimen.Type, X + 40, 495, Chamber.Specimen.Occupation);
			}
			const float SweepX = X + 37.0f * (1.0f + FMath::Sin(Time * 3.0f + c));
			P.Box(SweepX, 445, 5, 48, FLinearColor(0.3f, 1.0f, 0.5f, 0.3f));
			P.Bar(X, 530, 80, 5, Chamber.Duration > 0.0f ? Chamber.Time / Chamber.Duration : 0.0f, LabelColor);
		}
		else
		{
			P.Text(X + 22, 470, TEXT("idle"), 8, FLinearColor(0.9f, 0.6f, 0.3f), TEXT("Regular"));
		}
	}

	// A signature experiment strobes the lab in its department's colour.
	for (int32 d = 1; d < int32(EDepartment::Count); ++d)
	{
		if (SignatureRunning[d])
		{
			static const FLinearColor DeptColors[] = { FLinearColor::White, FLinearColor(0.3f, 1.0f, 0.4f), FLinearColor(0.5f, 0.6f, 1.0f), FLinearColor(1.0f, 0.5f, 0.1f) };
			const float Strobe = 0.25f + 0.35f * FMath::Square(0.5f + 0.5f * FMath::Sin(Time * 6.0f));
			P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::Lab, TEXT("signature"), 675, 425, DeptColors[d].CopyWithNewOpacity(Strobe));
		}
	}

	// Generator: the coil grows with power level.
	P.Shift = BaseOrigin + GeneratorOffset;
	const int32 PowerLevel = S.GetBaseLevel(EBaseUpgrade::Power);
	const float EnergyFraction = float(S.GetEnergy() / S.GetEnergyCap());
	P.Room(1025, 425, 255, 115, FString::Printf(TEXT("GENERATOR  +%.0f/s"), S.GetEnergyPerSecond()), ERoomSprite::Generator);
	const float Pulse = 0.6f + 0.4f * FMath::Sin(Time * (5.0f + PowerLevel));
	const float Coil = 36.0f + 4.0f * PowerLevel;
	P.CBox(1075, 488, Coil, Coil, Srgb(0.2f, 0.6f * Pulse, 1.0f * Pulse), true);
	P.Bar(1125, 470, 140, 14, EnergyFraction, FLinearColor(0.3f, 0.7f, 1.0f));
	P.Text(1125, 492, FString::Printf(TEXT("%.0f / %.0f"), S.GetEnergy(), S.GetEnergyCap()), 9, LabelColor, TEXT("Regular"));
	// Unusual Power Systems: more hamsters, and the coil arcs at random.
	if (Done(EDeptProject::UnusualPower))
	{
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::Generator, FMath::Frac(Time * 6.0f) < 0.5f ? TEXT("hamsters_a") : TEXT("hamsters_b"), 1025, 425);
		const float Arc = FMath::Frac(FMath::Sin(FMath::FloorToFloat(Time * 9.0f) * 12.9898f) * 43758.5453f);
		if (Arc < 0.45f)
		{
			P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::Generator, Arc < 0.22f ? TEXT("sparks_a") : TEXT("sparks_b"), 1025, 425);
		}
	}

	// Break room bulletin board: one memo per completed experiment, colored by specimen.
	P.Shift = BaseOrigin + BreakRoomOffset;
	P.Room(675, 565, 605, 115, TEXT("BREAK ROOM"), ERoomSprite::BreakRoom);
	P.Box(695, 590, 250, 80, FLinearColor(0.45f, 0.32f, 0.2f));
	{
		static const FLinearColor MemoColors[] = { FLinearColor(0.95f, 0.9f, 0.4f), FLinearColor(0.9f, 0.9f, 0.85f), FLinearColor(0.6f, 0.75f, 0.95f), FLinearColor(0.95f, 0.55f, 0.6f) };
		int32 Memo = 0;
		for (int32 t = 0; t < int32(ESpecimen::Count) && Memo < 24; ++t)
		{
			for (int32 k = 0; k < S.GetExperimentsCompleted(ESpecimen(t)) && Memo < 24; ++k, ++Memo)
			{
				P.Box(702.0f + (Memo % 8) * 30.0f, 596.0f + (Memo / 8) * 24.0f, 24, 18, MemoColors[t]);
			}
		}
	}
	P.Text(965, 600, TEXT("Findings are posted here."), 9, LabelColor, TEXT("Regular"));
	P.Text(965, 620, TEXT("Reading them is optional."), 9, LabelColor, TEXT("Regular"));
	P.Text(965, 640, TEXT("Understanding them is discouraged."), 9, LabelColor, TEXT("Regular"));

	// Encyclopedia shelf: one spine per entry, discoveries in gold.
	{
		int32 Spine = 0;
		for (const FEncyclopediaEntry& Entry : S.GetEncyclopedia())
		{
			const bool bDiscovery = Entry.Title.StartsWith(TEXT("Discovery:"));
			P.Box(965.0f + Spine * 15.0f, 655, 12, 20, bDiscovery ? FLinearColor(0.85f, 0.65f, 0.1f) : FLinearColor(0.2f, 0.35f, 0.6f));
			++Spine;
		}
	}

	// Break room extras: the cover-story printer, the scrambler helmet, and signature trophies.
	if (Done(EDeptProject::AutomatedCoverStories))
	{
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::BreakRoom, TEXT("printer"), 675, 565);
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::BreakRoom, FMath::Frac(Time * 2.5f) < 0.5f ? TEXT("printer_a") : TEXT("printer_b"), 675, 565);
	}
	if (S.GetBaseLevel(EBaseUpgrade::Cleanup) > 0 || CVarPreviewRival.GetValueOnGameThread() > 0)
	{
		// Cleanup Crew: a janitor mops in front of the bulletin board, forever.
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("janitor_mop"), 676, 633, 28, 42, 10.0f * FMath::Sin(Time * 2.2f), FVector2f(12.0f, 28.0f) * 0.875f);
		P.Sprites.DrawSprite(P.SpriteCanvas(), TEXT("janitor_body"), 676, 633, 28, 42);
	}
	if (Done(EDeptProject::MemoryManipulation))
	{
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::BreakRoom, TEXT("helmet"), 675, 565);
		if (FMath::Frac(Time * 1.7f) < 0.6f)
		{
			P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::BreakRoom, TEXT("helmet_lights"), 675, 565);
		}
	}
	if (SignatureDone[1] || SignatureDone[2] || SignatureDone[3])
	{
		P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::BreakRoom, TEXT("trophy_shelf"), 675, 565);
		static const TCHAR* Trophies[] = { nullptr, TEXT("trophy_bio"), TEXT("trophy_human"), TEXT("trophy_incomp") };
		for (int32 d = 1; d < int32(EDepartment::Count); ++d)
		{
			if (SignatureDone[d])
			{
				P.Sprites.DrawRoomPart(P.SpriteCanvas(), ERoomSprite::BreakRoom, Trophies[d], 675, 565);
			}
		}
	}

	// UFOs fly in canvas coordinates: from the (shifted) hangar out to the hunting grounds.
	P.Shift = FVector2f::ZeroVector;
	bool bAnyWaiting = false;
	for (int32 u = 0; u < S.GetNumUfos(); ++u)
	{
		const FUfo& Ufo = S.GetUfo(u);
		const FVector2f Dock(DockX(u) + BaseShift, DockY);
		const FVector2f ExitPos = BaseExitPos + FVector2f(BaseShift, 0.0f);
		const FVector2f Target(RegionCenterX(Ufo.TripRegion) + (u - 1) * 14.0f, HoverY + u * 6.0f);
		const float Progress = S.GetUfoStateProgress(u);
		FVector2f Pos = Dock;
		switch (Ufo.State)
		{
		case EUfoState::FlyingOut: Pos = LerpPath(Dock, ExitPos, Target, Progress); break;
		case EUfoState::Capturing: Pos = Target + FVector2f(0.0f, 4.0f * FMath::Sin(Time * 4.0f + u)); break;
		case EUfoState::Returning: Pos = LerpPath(Dock, ExitPos, Target, 1.0f - Progress); break;
		case EUfoState::WaitingToUnload: bAnyWaiting = true; break;
		default: break;
		}
		const bool bFlying = Ufo.State == EUfoState::FlyingOut || Ufo.State == EUfoState::Returning;
		// The first three ships are animated sprites that tilt (and, for Old Reliable, cough and drop),
		// so everything bolted onto them follows their pose.
		const bool bSprite = Sprites.HasUfo(u);
		FUfoPose Pose;
		if (bSprite)
		{
			Pose = Sprites.PoseUfo(u, Pos, Time, bFlying || Ufo.State == EUfoState::Capturing);
			Pos = Pose.Pos;
		}
		auto OnHull = [&](float DX, float DY) { return bSprite ? Pose.ToCanvas(FVector2f(48.0f + DX, 26.0f + DY)) : Pos + FVector2f(DX, DY); };
		const int32 Thrusters = S.GetShipLevel(u, EShipSystem::Thrusters);
		const int32 Tractor = S.GetShipLevel(u, EShipSystem::Tractor);
		const int32 Cargo = S.GetShipLevel(u, EShipSystem::Cargo);
		const int32 Stealth = S.GetShipLevel(u, EShipSystem::Stealth);
		const int32 Scanner = S.GetShipLevel(u, EShipSystem::Scanner);

		if (Ufo.State == EUfoState::Capturing)
		{
			const float BeamW = 34.0f + 8.0f * Tractor;
			P.Box(Pos.X - BeamW * 0.5f, Pos.Y + 10, BeamW, GroundY - Pos.Y - 10, FLinearColor(1.0f, 0.95f, 0.4f, 0.28f));
			const float RiseY = FMath::Lerp(GroundY, Pos.Y + 34.0f, Progress);
			P.Specimen(Ufo.TripTarget, Pos.X + 8.0f * FMath::Sin(Time * 9.0f), RiseY);
		}

		// Stealth darkens and fades the hull.
		const float Fade = 1.0f - 0.2f * Stealth;
		const FLinearColor Hull = FLinearColor::LerpUsingHSV(UfoColor, FLinearColor(0.08f, 0.1f, 0.14f), 0.3f * Stealth).CopyWithNewOpacity(Fade);

		// Thruster pods (speed), flames while flying.
		for (int32 i = 0; i < Thrusters; ++i)
		{
			const float X = Pos.X - (Thrusters - 1) * 6.0f + i * 12.0f;
			P.CBox(X, Pos.Y + 12, 8, 6, Srgb(0.9f, 0.4f, 0.1f).CopyWithNewOpacity(Fade), true);
			if (bFlying)
			{
				const float Flame = 8.0f + 4.0f * FMath::Sin(Time * 30.0f + i);
				P.CBox(X, Pos.Y + 15 + Flame * 0.5f, 6, Flame, Srgb(1.0f, 0.8f, 0.2f, 0.85f * Fade), true);
			}
		}
		// Tractor emitter grows with strength.
		if (Tractor > 0)
		{
			// Heavy Tractor Array: a much bigger emitter with a glowing ring.
			const bool bHeavy = Done(EDeptProject::HeavyTractorArray);
			const float EmitterW = (10.0f + 7.0f * Tractor) * (bHeavy ? 1.5f : 1.0f);
			if (bHeavy)
			{
				P.CBox(Pos.X, Pos.Y + 12, EmitterW + 6.0f, 10, FLinearColor(0.8f, 0.5f, 1.0f, 0.35f * Fade), true);
			}
			P.CBox(Pos.X, Pos.Y + 10, EmitterW, bHeavy ? 8.0f : 6.0f, FLinearColor(0.6f, 0.3f, 0.9f, Fade), true);
		}
		// Scanner antenna and dish.
		if (Scanner > 0)
		{
			// Advanced Scanners: a taller mast and a much larger dish.
			const bool bBigDish = Done(EDeptProject::AdvancedScanners);
			const float Mast = bBigDish ? 22.0f : 14.0f;
			P.Box(Pos.X + 8, Pos.Y - 16 - Mast, 2, Mast, FLinearColor(0.8f, 0.8f, 0.85f, Fade));
			P.CBox(Pos.X + 9, Pos.Y - 17 - Mast, (6.0f + 4.0f * Scanner) * (bBigDish ? 1.8f : 1.0f), bBigDish ? 7.0f : 5.0f, FLinearColor(0.3f, 0.9f, 0.8f, Fade), true);
		}
		// Sprite domes are translucent, so cargo drawn before them shows through.
		if (!bSprite)
		{
			P.CBox(Pos.X, Pos.Y - 8, 32, 18, DomeColor.CopyWithNewOpacity(DomeColor.A * Fade), true);
		}
		if (Ufo.Cargo.Num() > 0 && Ufo.TripTarget != ESpecimen::Elephant && Ufo.TripTarget != ESpecimen::Whale && Ufo.TripTarget != ESpecimen::ElephantSeal)
		{
			const FVector2f Seat = OnHull(0.0f, -1.0f);
			P.Specimen(Ufo.TripTarget, Seat.X, Seat.Y, Ufo.Cargo[0].Occupation);
		}
		if (bSprite)
		{
			const FLinearColor Tint = FLinearColor::LerpUsingHSV(FLinearColor::White, FLinearColor(0.15f, 0.18f, 0.25f), 0.3f * Stealth).CopyWithNewOpacity(Fade);
			Sprites.DrawUfo(P.SpriteCanvas(), Pose, Time, Tint, Done(EDeptProject::ExperimentalModules));
		}
		else
		{
			P.CBox(Pos.X, Pos.Y + 2, 80, 16, Hull, true);
		}
		// Cargo pods on the rim.
		for (int32 i = 0; i < Cargo; ++i)
		{
			const float Side = i % 2 == 0 ? -1.0f : 1.0f;
			const FVector2f Pod = OnHull(Side * (44.0f + (i / 2) * 9.0f), 2.0f);
			P.CBox(Pod.X, Pod.Y, 9, 11, FLinearColor(0.75f, 0.6f, 0.3f, Fade), true);
		}
		for (int32 i = 0; i < 4; ++i)
		{
			// One of Old Reliable's lights has been dying for decades.
			// It also dies whenever the engine coughs.
			const bool bOn = bSprite && u == 0 && (i == 2 || Pose.Sputter > 0.2f) ? FMath::Frac(Time * 0.37f) < 0.06f : (int32(Time * 4.0f) + i + u) % 2 == 0;
			const FVector2f Light = OnHull(-24.0f + i * 16.0f, 3.0f);
			P.CBox(Light.X, Light.Y, 5, 5, (bOn ? FLinearColor(1.0f, 0.3f, 0.3f) : FLinearColor(0.3f, 0.1f, 0.1f)).CopyWithNewOpacity(Fade), true);
		}
		P.Text(Pos.X - 34, Pos.Y - 50, FString::Printf(TEXT("%d %s"), u + 1, *Ufo.Name), 7, LabelColor);
	}

	// Back to base-local coordinates for the banners and crew badges.
	P.Shift = FVector2f(BaseShift, 0.0f);

	// Incident and meeting banners across the top of the base.
	if (S.GetIncident().Type != EIncident::None)
	{
		P.Box(675, 22, 605, 22, FLinearColor(0.35f, 0.05f, 0.02f, 0.85f));
		P.Text(683, 24, TEXT("INCIDENT: ") + S.GetIncidentText(), 9, WarnColor);
	}
	if (S.IsInMeeting())
	{
		P.Box(675 + LabOffset.X, 548 + LabOffset.Y, 330, 14, FLinearColor(0.2f, 0.1f, 0.3f, 0.9f));
		P.Text(683 + LabOffset.X, 548 + LabOffset.Y, TEXT("MANDATORY MEETING IN PROGRESS"), 8, LabelColor);
	}

	if (bAnyWaiting)
	{
		P.Text(HangarX + 140, HangarY + 20, TEXT("WAITING: CONTAINMENT FULL"), 9, WarnColor);
	}

	// Crew badges: one dot per assigned alien, colored by specialty; unstaffed posts say so.
	P.Text(1110 + BreakRoomOffset.X, 570 + BreakRoomOffset.Y, TEXT("MEMORY-WIPE STATION"), 8, LabelColor);
	for (const FAssignment& Station : S.GetAllStations())
	{
		FVector2f Anchor;
		switch (Station.Station)
		{
		case EStation::Ufo: Anchor = FVector2f(DockX(Station.Index) + 16.0f, HangarY + 38.0f); break;
		case EStation::Chamber: Anchor = FVector2f(735.0f + Station.Index * 92.0f, 452.0f) + LabOffset; break;
		case EStation::Containment: Anchor = FVector2f(1210.0f, 312.0f) + ContainmentOffset; break;
		case EStation::Power: Anchor = FVector2f(1235.0f, 437.0f) + GeneratorOffset; break;
		case EStation::Pressure: Anchor = FVector2f(640.0f, 335.0f); break;
		case EStation::Boiler: Anchor = FVector2f(600.0f, 335.0f); break;
		case EStation::Calibration: Anchor = FVector2f(688.0f, 470.0f) + LabOffset; break;
		default: Anchor = FVector2f(1200.0f, 590.0f) + BreakRoomOffset; break;
		}
		int32 Dot = 0;
		for (const FCrewMember& Member : S.GetCrew())
		{
			if (Member.TrainingRemaining > 0.0f || Member.Assignment.Station == EStation::None || !(Member.Assignment == Station))
			{
				continue;
			}
			static const FLinearColor SpecialtyColors[] = { FLinearColor(0.3f, 0.6f, 1.0f), FLinearColor(0.4f, 1.0f, 0.4f), FLinearColor(1.0f, 0.6f, 0.2f) };
			P.CBox(Anchor.X + Dot * 11.0f, Anchor.Y, 9, 9, SpecialtyColors[int32(Member.Specialty)], true);
			++Dot;
		}
		if (Dot == 0)
		{
			P.Text(Anchor.X - 6.0f, Anchor.Y - 7.0f, TEXT("UNSTAFFED"), 7, WarnColor);
		}
	}

	// Probe Calibration: a pulsing tag over the chamber with an open window. Clicking it opens the challenge.
	const int32 CalibrationChamber = S.GetCalibrationChamber();
	if (CalibrationChamber != INDEX_NONE && !S.IsCalibrationOpen())
	{
		const FVector2f TagPos = CalibrationTagPos(CalibrationChamber);
		const float TagPulse = 0.55f + 0.45f * FMath::Abs(FMath::Sin(float(FPlatformTime::Seconds()) * 4.0f));
		P.Box(TagPos.X - 2, TagPos.Y - 2, CalibrationTagSize.X + 4, CalibrationTagSize.Y + 4, FLinearColor(0.02f, 0.05f, 0.04f, 0.9f));
		P.Box(TagPos.X, TagPos.Y, CalibrationTagSize.X, CalibrationTagSize.Y, FLinearColor(0.15f, 0.9f, 0.6f, TagPulse));
		P.Text(TagPos.X + 6, TagPos.Y + 1, TEXT("CALIBRATION"), 7, FLinearColor(0.01f, 0.03f, 0.02f));
		P.Text(TagPos.X + 6, TagPos.Y + 13, TEXT("AVAILABLE"), 7, FLinearColor(0.01f, 0.03f, 0.02f));
	}

	return P.Layer;
}
