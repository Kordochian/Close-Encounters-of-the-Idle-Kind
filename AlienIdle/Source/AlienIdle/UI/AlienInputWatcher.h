#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "HAL/PlatformTime.h"

/**
 * Records when the player last acted on the game: a key, a mouse click, the wheel or a pushed
 * stick. Mouse movement does not count, because the cursor crosses the window during other work on
 * the computer. (Slate's own last-interaction time counts those moves, so the game never went idle.)
 * Never consumes input.
 */
class FAlienInputWatcher : public IInputProcessor
{
public:
	double GetLastInputTime() const { return LastInputTime; }
	const TCHAR* GetLastInputKind() const { return LastInputKind; }

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { Touch(TEXT("key")); return false; }
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { Touch(TEXT("button")); return false; }
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override { Touch(TEXT("wheel")); return false; }
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override
	{
		if (FMath::Abs(InAnalogInputEvent.GetAnalogValue()) > 0.3f)
		{
			Touch(TEXT("analog"));
		}
		return false;
	}
	virtual const TCHAR* GetDebugName() const override { return TEXT("AlienInputWatcher"); }

private:
	void Touch(const TCHAR* What) { LastInputTime = FPlatformTime::Seconds(); LastInputKind = What; }

	double LastInputTime = FPlatformTime::Seconds();
	const TCHAR* LastInputKind = TEXT("start");
};
