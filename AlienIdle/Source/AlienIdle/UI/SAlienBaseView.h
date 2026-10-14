#pragma once

#include "CoreMinimal.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "UI/AlienSprites.h"
#include "Widgets/SLeafWidget.h"

class FAlienIdleSim;

/**
 * Placeholder-art cutaway of the mountain base: sky and Pine Ridge on the left, rooms inside the
 * mountain on the right, the UFO flying between them. Everything is flat shapes drawn in a
 * 1300x700 virtual canvas that is scaled to fit.
 */
class SAlienBaseView : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SAlienBaseView) {}
		SLATE_ARGUMENT(TSharedPtr<FAlienIdleSim>, Sim)
		/** Clicking the "CALIBRATION AVAILABLE" tag on a lab chamber. */
		SLATE_ARGUMENT(TFunction<void()>, OnCalibrationClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(1300.0, 700.0); }

	// Mouse wheel zooms toward the cursor; dragging pans while zoomed; double-click resets.
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	/** Canvas scale and screen offset for the current zoom and pan. */
	void GetView(const FVector2f& Size, float& OutScale, FVector2f& OutOffset) const;
	/** Keeps the view inside the canvas. */
	void ClampViewCenter(const FVector2f& Size);

	float Zoom = 1.0f;
	/** Canvas point shown at the middle of the widget (clamped to the map); starts at the bottom, on the base. */
	FVector2f ViewCenter = FVector2f(1260.0f, 860.0f);
	bool bDragging = false;
	FVector2f LastDragPosition = FVector2f::ZeroVector;
	/** A press that never moved far is a click (calibration tags), not a drag. */
	FVector2f PressPosition = FVector2f::ZeroVector;
	bool bPressMoved = false;
	TFunction<void()> OnCalibrationClicked;

	TSharedPtr<FAlienIdleSim> Sim;
	/** Rounded brushes indexed by corner radius - 1; shapes pick the one matching half their short side. */
	TArray<FSlateRoundedBoxBrush> RoundBrushes;
	/** Animated sprites from Content/Sprites; anything missing falls back to placeholder shapes. */
	FAlienSprites Sprites;
};
