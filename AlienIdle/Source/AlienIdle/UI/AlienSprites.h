#pragma once

#include "CoreMinimal.h"
#include "Layout/PaintGeometry.h"
#include "Styling/SlateBrush.h"
#include "Templates/Function.h"

class FSlateWindowElementList;

/** Where sprites are drawn: the caller's element list, layer counter and canvas-to-paint mapping. */
struct FSpriteCanvas
{
	FSlateWindowElementList& Elements;
	int32& Layer;
	TFunctionRef<FPaintGeometry(float X, float Y, float W, float H)> ToPaint;
	/** Screen pixels per canvas unit; picks which resolution of each part to draw. */
	float PixelsPerUnit = 1.0f;
};

/** A UFO's pose for one frame; the base view uses it to place lights and pods on the tilting hull. */
struct FUfoPose
{
	int32 Ufo = 0;
	/** UFO position after any sputter drop (canvas units). */
	FVector2f Pos = FVector2f::ZeroVector;
	float TiltDegrees = 0.0f;
	/** 0..1 while Old Reliable's engine coughs; always 0 for the other ships. */
	float Sputter = 0.0f;
	bool bEngineOn = false;

	/** Maps a point in the 96x48 UFO sprite frame (anchor at 48, 26) to canvas space, following the hull tilt. */
	FVector2f ToCanvas(const FVector2f& FramePoint) const;
};

/** What the Oceanic base shows on and around its dome. */
struct FOceanLook
{
	float HullPressure = 0.0f; // 0..100; the dome cracks at 60+
	float Tide = 0.0f;         // 0..1; turbines spin faster at high tide
	bool bTurbines = false;
	bool bTanks = false;
	bool bWhaleSong = false;
};

/** What Station Permafrost (Antarctic) shows on the ice shelf. */
struct FIceLook
{
	float Fire = 1.0f;     // 0..1+; how hard the boiler burns (heat supply)
	bool bTaps = false;    // Geothermal Taps: vents puff steam
	bool bVault = false;   // Cryo Vault on the shelf
};

/** What the lab alien wears; follows the primary research department. */
enum class EAlienUniform : uint8
{
	LabCoat,
	Biological,   // green lab coat
	Human,        // grey suit and a clipboard
	Incompetence, // scorched overalls and a hard hat
};

/** Room backgrounds; each is drawn at its room's canvas rectangle (see art/sprites/room_*.svg). */
enum class ERoomSprite : uint8
{
	Hangar,
	Containment,
	Lab,
	Generator,
	BreakRoom,
	Count
};

/** Props on the mountain that are their own sprites, so weather and disasters can change them. */
enum class EMountainProp : uint8
{
	GoHomeSign,
	KeepOutSign,
	Mailbox,
	Count
};

/** How a prop looks right now; states combine (a burnt sign can be snowed on). */
struct FPropState
{
	bool bSnow = false;
	bool bOnFire = false;
	bool bBurnt = false;
};

/**
 * Cut-out puppet sprites: each sprite is a stack of full-frame PNG parts (rendered from
 * art/sprites/*.svg by render_sprites.py into Content/Sprites) rotated about pivots in code.
 * Every part exists at a few densities (name@D.png = D pixels per canvas unit); each draw uses the
 * smallest one that is still sharp at the current screen scale.
 * A sprite whose parts are missing reports false from Has*() and the caller draws placeholder shapes.
 */
class FAlienSprites
{
public:
	static constexpr int32 NumUfoSprites = 3;

	void Load();

	bool HasSquirrel() const { return bHasSquirrel; }
	bool HasCow() const { return bHasCow; }
	bool HasAlien() const { return bHasAlien; }
	bool HasUfo(int32 Ufo) const { return Ufo >= 0 && Ufo < NumUfoSprites && bHasUfo[Ufo]; }
	bool HasRoom(ERoomSprite Room) const { return bHasRoom[int32(Room)]; }
	bool HasMountain() const { return bHasMountain; }
	bool HasCity() const { return bHasCity; }
	bool HasRegions() const { return bHasRegions; }

	/** Specimens stand on FeetY, centered on CX, facing left, and fidget on their own. */
	void DrawSquirrel(const FSpriteCanvas& Canvas, float CX, float FeetY, float Time, const FLinearColor& Tint = FLinearColor::White) const;
	void DrawCow(const FSpriteCanvas& Canvas, float CX, float FeetY, float Time, const FLinearColor& Tint = FLinearColor::White) const;
	/** The lab alien, facing right, sipping coffee (or holding a clipboard, in the Human department's suit). */
	void DrawAlien(const FSpriteCanvas& Canvas, float CX, float FeetY, float Time, EAlienUniform Uniform = EAlienUniform::LabCoat) const;

	FUfoPose PoseUfo(int32 Ufo, const FVector2f& Pos, float Time, bool bEngineOn) const;
	/** bModules adds the Incompetence department's bolted-on experimental module. */
	void DrawUfo(const FSpriteCanvas& Canvas, const FUfoPose& Pose, float Time, const FLinearColor& Tint, bool bModules = false) const;

	/**
	 * The mountain with its hallways and the cabin on top (frame 660x1000 at canvas 640, -300, plus the hallway-effects and cabin sprites). The old
	 * man raises his binoculars and the dog looks up while any UFO is out.
	 */
	/** Location 0 mountain, 1 Metropolis, 2 Oceanic (Ocean says what the dome shows), 3 Antarctic (Ice says what the shelf shows). */
	void DrawMountain(const FSpriteCanvas& Canvas, float Time, bool bUfoOut, int32 Location = 0, const FOceanLook& Ocean = FOceanLook(),
		const FIceLook& Ice = FIceLook()) const;
	/** Antarctic specimens: 0 penguin, 1 seal, 2 elephant seal; standing on FeetY, centered on CX. */
	void DrawIceCreature(const FSpriteCanvas& Canvas, int32 Kind, float CX, float FeetY, float Time) const;
	bool HasIceCreatures() const { return bHasIceCreatures; }
	bool HasIce() const { return bHasIce; }
	/** Oceanic specimens: 0 pufferfish, 1 octopus, 2 whale; standing (or floating) on FeetY, centered on CX. */
	void DrawSeaCreature(const FSpriteCanvas& Canvas, int32 Kind, float CX, float FeetY, float Time) const;
	bool HasSeaCreatures() const { return bHasSea; }

	/**
	 * Sets a mountain prop's look (snow, fire, burnt). The console variable AlienIdle.MountainProps
	 * overrides every prop for previewing: 1 snow, 2 on fire, 3 burnt, 4 burnt and snowed on.
	 */
	void SetPropState(EMountainProp Prop, const FPropState& State) { PropStates[int32(Prop)] = State; }
	const FPropState& GetPropState(EMountainProp Prop) const { return PropStates[int32(Prop)]; }

	/** Draws a room background (and its animated details) with its top-left corner at X, Y. */
	void DrawRoom(const FSpriteCanvas& Canvas, ERoomSprite Room, float X, float Y, float Time) const;
	/** A hunting region's backdrop (220x210 frame, ground at local y 200) with its top-left at X, Y. */
	void DrawRegion(const FSpriteCanvas& Canvas, int32 Location, int32 Region, float X, float Y, float Time) const;
	/** Earth's responses (night patrols, investigators, radar, support group), feet on FeetY. */
	void DrawResponse(const FSpriteCanvas& Canvas, int32 Response, float X, float FeetY, float Time) const;
	/** Earth-condition jokes: 1 documentary crew, 2 agency surveillance, 3 billboard. */
	void DrawCondition(const FSpriteCanvas& Canvas, int32 Condition, float X, float FeetY, float Time, float Scale = 1.0f) const;

	/** Draws any loaded part by name into a box (for one-off sprites such as the rival's ship). */
	void DrawSprite(const FSpriteCanvas& Canvas, FName Name, float X, float Y, float W, float H,
		float AngleDegrees = 0.0f, const FVector2f& LocalPivot = FVector2f::ZeroVector, const FLinearColor& Tint = FLinearColor::White) const
	{
		DrawPart(Canvas, Name, X, Y, W, H, AngleDegrees, LocalPivot, Tint);
	}

	/** Draws one optional part of a room sprite (e.g. "bars", "mirror", "printer_a") in the room's frame at X, Y. */
	void DrawRoomPart(const FSpriteCanvas& Canvas, ERoomSprite Room, const TCHAR* Part, float X, float Y, const FLinearColor& Tint = FLinearColor::White) const;

private:
	/** The best resolution of a part for a box W canvas units wide, or null if the part is missing. */
	const FSlateBrush* Part(FName Name, float W, float PixelsPerUnit) const;
	bool HasAll(std::initializer_list<const TCHAR*> Names) const;
	void DrawPart(const FSpriteCanvas& Canvas, FName Name, float X, float Y, float W, float H,
		float AngleDegrees, const FVector2f& LocalPivot, const FLinearColor& Tint) const;
	void DrawProp(const FSpriteCanvas& Canvas, EMountainProp Prop, float Time) const;
	/** Rising, fading puffs from a point (exhaust, steam). */
	void DrawPuffs(const FSpriteCanvas& Canvas, const FVector2f& From, const FVector2f& Drift, float MinSize, float MaxSize,
		float Rate, int32 Count, const FLinearColor& Color, float Time) const;

	struct FPartVariant
	{
		/** Pixels per canvas unit at the part's nominal size. */
		float Density = 1.0f;
		/** Width the part was authored to be drawn at, in canvas units. */
		float NominalWidth = 1.0f;
		TUniquePtr<FSlateBrush> Brush;
	};
	/** Variants sorted by ascending density. */
	TMap<FName, TArray<FPartVariant>> Parts;
	bool bHasSquirrel = false;
	bool bHasCow = false;
	bool bHasAlien = false;
	bool bHasUfo[NumUfoSprites] = {};
	bool bHasRoom[int32(ERoomSprite::Count)] = {};
	bool bHasMountain = false;
	bool bHasCity = false;
	bool bHasRegions = false;
	bool bHasOcean = false;
	bool bHasSea = false;
	bool bHasIce = false;
	bool bHasIceCreatures = false;
	FPropState PropStates[int32(EMountainProp::Count)];
};
