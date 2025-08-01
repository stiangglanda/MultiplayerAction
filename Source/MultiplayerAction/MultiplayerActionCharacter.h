#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include <Components/SphereComponent.h>
#include "OutpostInteractable.h"
#include "InteractionProgressBarWidget.h"
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

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USkeletalMeshComponent> ShieldMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LockAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* BlockAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* RollAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* GroupControlAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* EscapeAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* HeavyAttackAction;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* HeavyAttackMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* CombatMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* CombatMontageAlt;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* ImpactMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* RollMontage;

    UPROPERTY(Replicated)
    TObjectPtr<UAnimMontage> CurrentInteractionMontage;

    UPROPERTY(EditAnywhere)
    float SphereTraceRadiusWeapon = 20;

    UPROPERTY(EditAnywhere)
    float SphereTraceRadiusLockOn = 900;

    UPROPERTY(Replicated, EditAnywhere)
    int Team = 0;

    UPROPERTY(Category = Chest, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USphereComponent> SphereCollider;

    UPROPERTY(EditAnywhere, Category = "Chest")
    float SphereColliderRadius = 200.0f;
    UPROPERTY(Replicated)
    bool allowCombat = true;

    UFUNCTION(Client, Reliable)
    void Client_OnBecameGroupLeader();

    UFUNCTION(Server, Reliable)
    void Server_RequestToggleGroupCombat();

public:
    UFUNCTION(Client, Reliable)
    void Client_OnInteractionSuccess();

    UPROPERTY(Replicated, EditAnywhere)
    TObjectPtr<class AAIGroupManager> AIGroupManager;

    void InitializeGroupMembership(TObjectPtr<class AAIGroupManager> AIGroupManager);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UWeapon> Weapon;

    UFUNCTION()
    void OnRep_WeaponClass();

    UPROPERTY(ReplicatedUsing = OnRep_WeaponClass, EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TSubclassOf<UWeapon> WeaponClass;

    AMultiplayerActionCharacter();

    UFUNCTION(BlueprintPure)
    bool IsDead();

    UFUNCTION(BlueprintPure)
    bool GetAllowCombat();

    UFUNCTION(BlueprintPure)
    float GetHeathPercent() const;

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_Attack();

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_HeavyAttack();

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_Block();

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_SwapWeapon(TSubclassOf<UWeapon> NewWeaponClass);

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_SwapWeapon(TSubclassOf<UWeapon> NewWeaponClass);

    UFUNCTION()
    TSubclassOf<UWeapon> SwapWeapon(TSubclassOf<UWeapon> NewWeaponClass);

    void SetLockedOnTarget(AMultiplayerActionCharacter* unit);

    UAudioComponent* GetMovementAudioComponent() const { return MovementAudioComponent; }

    USoundBase* GetMovementLoopSound() const { return MovementLoopSound; }

    void PlayInteractionMontage(UAnimMontage* MontageToPlay);

    void StopInteractionMontage();

    void SetActiveProgressBar(UInteractionProgressBarWidget* Widget);

    UFUNCTION(Server, Reliable)
    void Server_RequestStartInteract(AActor* InteractableActor);

    UFUNCTION(Server, Reliable)
    void Server_RequestStopInteract(AActor* InteractableActor);

    void SetMaxHealth(float NewValue);
    void SetHealth(float NewValue);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> MovementAudioComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> MovementLoopSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> JumpSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> RollSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> BlockSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> AttackSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> DeathSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> DamageSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> HeavyAttackGruntSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> AttackGruntSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> WeaponSwapSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> EscapeWidgetClass;

    UPROPERTY()
    UUserWidget* EscapeWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> InteractionWidgetClass;

    UPROPERTY()
    UUserWidget* InteractionWidget;

    bool bIsAttacking = false;
    bool IsRolling = false;
    bool AttackAnim = false;
    bool IsBlocking = false;
    bool IsLockedOn = false;

	FRotator RotationBeforeAttack;// Root moten messes with the rotation, so we store the rotation before attack to restore it after the attack animation ends.

	AMultiplayerActionCharacter* LockedOnTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    TScriptInterface<IOutpostInteractable> CurrentInteractable;

    UPROPERTY()
    TObjectPtr<UInteractionProgressBarWidget> ActiveProgressBarWidget;

    FOnMontageEnded AttackMontageEndedDelegate;

    FOnMontageEnded HeavyAttackMontageEndedDelegate;

    FOnMontageEnded BlockMontageEndedDelegate;

    FOnMontageEnded RollMontageEndedDelegate;

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnHeavyAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_Attack();

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_Block();

    UFUNCTION(Server, Reliable)
    void ServerReliableRPC_Roll();

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_Roll();

    UFUNCTION(NetMulticast, Reliable)
    void NetMulticastReliableRPC_HeavyAttack();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayInteractionMontage(UAnimMontage* MontageToPlay);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopInteractionMontage();

    FTimerHandle WeaponTraceTimer;
    virtual void PerformWeaponTrace();
    void StartWeaponTrace();
    void StopWeaponTrace();
    const float WeaponTraceInterval = 0.01f;// 100 traces per second

    TArray<AActor*> ActorsHit;

    UPROPERTY(EditDefaultsOnly, Category = "Health", ReplicatedUsing = OnRep_MaxHealth)
    float MaxHealth = 100;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float Health;

    UFUNCTION()
    void OnRep_MaxHealth();

    UFUNCTION()
    void OnRep_CurrentHealth();

    void Move(const FInputActionValue& Value);

    void Look(const FInputActionValue& Value);

    void Block(const FInputActionValue& Value);

    void Lock(const FInputActionValue& Value);

    void Roll(const FInputActionValue& Value);

    void HeavyAttack(const FInputActionValue& Value);

    void Interact(const FInputActionValue& Value);

    void StopInteract(const FInputActionValue& Value);

    void Escape(const FInputActionValue& Value);

    void GroupControl(const FInputActionValue& Value);

    void Jump() override;

    void AttackInputMapping(const FInputActionValue& Value);

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

public:
    int GetTeam();

    void SetTeam(int NewTeam);

    float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

    FORCEINLINE class USpringArmComponent* GetCameraBoom() const
    {
        return CameraBoom;
    }

    FORCEINLINE class UCameraComponent* GetFollowCamera() const
    {
        return FollowCamera;
    }
};
