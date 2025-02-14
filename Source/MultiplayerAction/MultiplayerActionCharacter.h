// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "MultiplayerActionCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config = Game)
class AMultiplayerActionCharacter : public ACharacter
{
    GENERATED_BODY()

    /** Camera boom positioning the camera behind the character */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    /** MappingContext */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    /** Jump Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    /** Attack Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* BlockAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* CombatMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* CombatMontageAlt;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* ImpactMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere)
    float WeaponDamage = 20;

    UPROPERTY(EditAnywhere)
    float SphareTraceLength = 100;

    UPROPERTY(EditAnywhere)
    float SphareTraceRadius = 50;

    UPROPERTY(EditAnywhere)
    int Team = 0;

public:
    AMultiplayerActionCharacter();

    UFUNCTION(BlueprintPure)
    bool IsDead();

    UFUNCTION(BlueprintPure)
    float GetHeathPercent() const;

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_Attack();

protected:
    bool bIsAttacking = false;
    bool AttackAnim = false;
    bool IsBlocking = false;

    FOnMontageEnded AttackMontageEndedDelegate;

    FOnMontageEnded BlockMontageEndedDelegate;

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_Attack();

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_Block();

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_Block();

    UPROPERTY(EditAnywhere)
    float MaxHealth = 100;
    /** The player's current health. When reduced to 0, they are considered dead.*/
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float Health;

    /** RepNotify for changes made to current health.*/
    UFUNCTION()
    void OnRep_CurrentHealth();

    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    void Block(const FInputActionValue& Value);

    /** Called for Attack input */
    void AttackInputMapping(const FInputActionValue& Value);

    int GetTeam();

    float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

    void PlayImpactAnimation();

    /** Property replication */
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void OnHealthUpdate();

protected:
    // APawn interface
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // To add mapping context
    virtual void BeginPlay();

public:
    /** Returns CameraBoom subobject **/
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const
    {
        return CameraBoom;
    }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetFollowCamera() const
    {
        return FollowCamera;
    }
};
