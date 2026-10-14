#include "AlienIdleGameMode.h"
#include "AlienIdlePlayerController.h"

AAlienIdleGameMode::AAlienIdleGameMode()
{
	PlayerControllerClass = AAlienIdlePlayerController::StaticClass();
	// The whole game is the 2D cutaway UI; no pawn is needed.
	DefaultPawnClass = nullptr;
}
