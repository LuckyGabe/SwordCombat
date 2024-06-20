// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#include "HUD/PlayerHUD.h"


#include "Blueprint/UserWidget.h"
#include "Character/BaseCharacter.h"
#include "GameFramework/PlayerController.h"

void APlayerHUD::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();

    if (!IsValid(World) || !World->GetFirstPlayerController()) { return; }

    ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(World->GetFirstPlayerController()->GetPawn());

    if (!IsValid(PlayerCharacter) || !IsValid(PlayerCharacter->WBP_PlayerHUD)) { return; }

        PlayerHUDWidget = CreateWidget<UUserWidget>(World, PlayerCharacter->WBP_PlayerHUD);

        if (PlayerHUDWidget)
        {
            PlayerHUDWidget->AddToViewport();
        }
    
}
