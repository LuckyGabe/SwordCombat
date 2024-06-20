// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com


#include "SwordCombatSystemGameModeBase.h"
#include "HUD/PlayerHUD.h"

ASwordCombatSystemGameModeBase::ASwordCombatSystemGameModeBase()
{
	HUDClass = APlayerHUD::StaticClass();
}