#pragma once

#include "CoreMinimal.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"

/**
 * Probe Calibration gauge: a horizontal track with the target band (and its bright precise core) and
 * the moving cursor the player stops. Positions are 0..1 across the track.
 */
class SCalibrationGauge : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCalibrationGauge)
		: _Cursor(0.0f)
		, _Band(0.5f)
		, _BandHalfWidth(0.1f)
		, _CoreHalfWidth(0.03f)
	{}
		SLATE_ATTRIBUTE(float, Cursor)
		SLATE_ATTRIBUTE(float, Band)
		SLATE_ARGUMENT(float, BandHalfWidth)
		SLATE_ARGUMENT(float, CoreHalfWidth)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Cursor = InArgs._Cursor;
		Band = InArgs._Band;
		BandHalfWidth = InArgs._BandHalfWidth;
		CoreHalfWidth = InArgs._CoreHalfWidth;
	}

	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(560.0, 48.0); }

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const FVector2f Size = AllottedGeometry.GetLocalSize();
		const FSlateBrush* White = FCoreStyle::Get().GetBrush("GenericWhiteBox");
		auto Rect = [&](float X, float Y, float W, float H, const FLinearColor& Color)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, ++LayerId,
				AllottedGeometry.ToPaintGeometry(FVector2f(FMath::Max(W, 1.0f), FMath::Max(H, 1.0f)), FSlateLayoutTransform(1.0f, FVector2f(X, Y))),
				White, ESlateDrawEffect::None, Color);
		};
		const float TrackY = 10.0f;
		const float TrackH = Size.Y - 20.0f;
		const float Centre = Band.Get() * Size.X;

		Rect(0.0f, TrackY, Size.X, TrackH, FLinearColor(0.012f, 0.014f, 0.02f));
		Rect(Centre - BandHalfWidth * Size.X, TrackY, 2.0f * BandHalfWidth * Size.X, TrackH, FLinearColor(0.03f, 0.3f, 0.08f));
		Rect(Centre - CoreHalfWidth * Size.X, TrackY, 2.0f * CoreHalfWidth * Size.X, TrackH, FLinearColor(0.3f, 0.95f, 0.35f));
		// Tick marks every tenth, so the band's position reads at a glance.
		for (int32 i = 1; i < 10; ++i)
		{
			Rect(Size.X * i / 10.0f - 0.5f, TrackY + TrackH - 6.0f, 1.0f, 6.0f, FLinearColor(0.2f, 0.22f, 0.25f));
		}
		const float X = FMath::Clamp(Cursor.Get(), 0.0f, 1.0f) * Size.X;
		Rect(X - 2.0f, 0.0f, 4.0f, Size.Y, FLinearColor(1.0f, 0.55f, 0.15f));
		return LayerId;
	}

private:
	TAttribute<float> Cursor;
	TAttribute<float> Band;
	float BandHalfWidth = 0.1f;
	float CoreHalfWidth = 0.03f;
};
