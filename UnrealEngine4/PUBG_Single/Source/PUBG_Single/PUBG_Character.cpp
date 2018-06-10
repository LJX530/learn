﻿/*
    Author: 刘臣轩
    Date: 2018.5.29
*/

#include "PUBG_Character.h"

#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
APUBG_Character::APUBG_Character()
{
    // Set this character to call Tick() every frame.
    PrimaryActorTick.bCanEverTick = true;

    // Don't use controller to update the Pawn's Yaw
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // SkeletalMesh
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> CharacterMesh(TEXT("/Game/Girl_01/meshes/girl_01_a"));
    USkeletalMeshComponent* Mesh = GetMesh();
    Mesh->SetSkeletalMesh(CharacterMesh.Object);
    Mesh->SetRelativeLocation(FVector(0, 0, -90));
    Mesh->SetRelativeRotation(FRotator(0, -90, 0));

    // SpringArm
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 220;
    SpringArm->SetRelativeLocation(FVector(0, 0, 110));
    SpringArm->SocketOffset = FVector(0, 20, 0);
    SpringArm->bUsePawnControlRotation = true;

    // FollowCamera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    // Animation Blueprint
    static ConstructorHelpers::FClassFinder<UAnimInstance> PUBG_AnimBP(TEXT("/Game/Animation/PUBG_AnimBP"));
    GetMesh()->SetAnimInstanceClass(PUBG_AnimBP.Class);

    // TurnBackCurve
    static ConstructorHelpers::FObjectFinder<UCurveFloat> FindTurnBackCurve(TEXT("/Game/Animation/TurnBackCurve"));
    TurnBackCurve = FindTurnBackCurve.Object;

    // CharacterMovement
    UCharacterMovementComponent* CharacterMovement = GetCharacterMovement();
    CharacterMovement->MaxWalkSpeed = 450;
    CharacterMovement->BrakingFrictionFactor = 0.1;
    CharacterMovement->BrakingDecelerationWalking = 1024;
    CharacterMovement->GroundFriction = 0.1;
    CharacterMovement->bUseControllerDesiredRotation = true;
    CharacterMovement->RotationRate = FRotator(0, 180, 0);
    
}

// Called when the game starts or when spawned
void APUBG_Character::BeginPlay()
{
    Super::BeginPlay();
    if (TurnBackCurve) {
        FOnTimelineFloat TurnBackTimelineCallBack;
        FOnTimelineEventStatic TurnBackTimelineFinishedCallback;

        TurnBackTimelineCallBack.BindUFunction(this, FName("UpdateController"));
        TurnBackTimelineFinishedCallback.BindLambda([this]() { GetCharacterMovement()->bUseControllerDesiredRotation = true; });
        TurnBackTimeline.AddInterpFloat(TurnBackCurve, TurnBackTimelineCallBack);
        TurnBackTimeline.SetTimelineFinishedFunc(TurnBackTimelineFinishedCallback);
    }
}

// Called every frame
void APUBG_Character::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TurnBackTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void APUBG_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

    PlayerInputComponent->BindAxis("MoveForward", this, &APUBG_Character::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APUBG_Character::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &ACharacter::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &ACharacter::AddControllerPitchInput);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &APUBG_Character::SprintPressed);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &APUBG_Character::SprintReleased);

    PlayerInputComponent->BindAction("Freelook", IE_Pressed, this, &APUBG_Character::FreelookPressed);
    PlayerInputComponent->BindAction("Freelook", IE_Released, this, &APUBG_Character::FreelookReleased);
}

void APUBG_Character::MoveForward(float AxisValue)
{
    if (AxisValue)
        AddMovementInput(GetActorForwardVector(), AxisValue);
}

void APUBG_Character::MoveRight(float AxisValue)
{
    if (AxisValue)
        AddMovementInput(GetActorRightVector(), AxisValue);
}

void APUBG_Character::SprintPressed()
{
    GetCharacterMovement()->MaxWalkSpeed = 600;
}

void APUBG_Character::SprintReleased()
{
    GetCharacterMovement()->MaxWalkSpeed = 450;
}

void APUBG_Character::FreelookPressed()
{
    TargetControlRotation = GetControlRotation();
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void APUBG_Character::FreelookReleased()
{
    CurrentContrtolRotation = GetControlRotation();
    TurnBackTimeline.PlayFromStart();
}

void APUBG_Character::UpdateController(float Value)
{
    FRotator NewRotation = FMath::Lerp(CurrentContrtolRotation, TargetControlRotation, Value);
    Controller->SetControlRotation(NewRotation);
}
