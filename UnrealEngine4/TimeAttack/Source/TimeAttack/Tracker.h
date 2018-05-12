// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"

#include "Tracker.generated.h"

UCLASS()
class TIMEATTACK_API ATracker : public AActor {

    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ATracker();

protected:
    // Called when the game starts or when spawned
    void BeginPlay() override;

public:
    // Called every frame
    void Tick(float DeltaTime) override;
};