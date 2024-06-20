// Code & Blueprints by Gabriel Spytkowski. spytkowskidev.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PlayerHUD.generated.h"



UCLASS()
class SWORDCOMBATSYSTEM_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

public:
    UUserWidget* PlayerHUDWidget;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    UUserWidget* GetPlayerHUDWidget() const { return PlayerHUDWidget; };
};
