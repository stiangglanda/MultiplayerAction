#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/UMG.h"
#include <Components/SphereComponent.h>
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

    /** Lock on Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LockAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* BlockAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* RollAction;

    /** Move Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    /** Look Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    /** Interact Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* InteractAction;

    /** Escape Input Action */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* EscapeAction;
    
    /** Heavy Attack Input Action */
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

    UPROPERTY(EditAnywhere)
    float WeaponDamage = 20;

    UPROPERTY(EditAnywhere)
    float SphereTraceRadiusWeapon = 20;

    UPROPERTY(EditAnywhere)
    float SphereTraceRadiusLockOn = 900;

    UPROPERTY(EditAnywhere)
    int Team = 0;

    UPROPERTY(Category = Chest, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USphereComponent> SphereCollider;

    UPROPERTY(EditAnywhere, Category = "Chest")
    float SphereColliderRadius = 200.0f;

public:
    UPROPERTY(EditAnywhere)
    TObjectPtr<class AAIGroupManager> AIGroupManager;

    void InitializeGroupMembership(TObjectPtr<class AAIGroupManager> AIGroupManager);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UWeapon> Weapon;

    UFUNCTION()
    void OnRep_WeaponClass();

    UPROPERTY(ReplicatedUsing = OnRep_WeaponClass, EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    TSubclassOf<UWeapon> WeaponClass;

    AMultiplayerActionCharacter();

    ~AMultiplayerActionCharacter();

    UFUNCTION(BlueprintPure)
    bool IsDead();

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

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> MovementAudioComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USoundBase> MovementLoopSound;

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

	class AChest* OverlappingChest;

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

    FTimerHandle WeaponTraceTimer;
    virtual void PerformWeaponTrace();
    void StartWeaponTrace();
    void StopWeaponTrace();
    const float WeaponTraceInterval = 0.01f;// 100 traces per second

    TArray<AActor*> ActorsHit;

    UPROPERTY(EditAnywhere)
    float MaxHealth = 100;

    /** RepNotify for changes made to current health.*/
    UFUNCTION()
    void OnRep_CurrentHealth();

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float Health;

    /** Called for movement input */
    void Move(const FInputActionValue& Value);

    /** Called for looking input */
    void Look(const FInputActionValue& Value);

    void Block(const FInputActionValue& Value);

    void Lock(const FInputActionValue& Value);

    void Roll(const FInputActionValue& Value);

    void HeavyAttack(const FInputActionValue& Value);

    void Interact(const FInputActionValue& Value);

    void Escape(const FInputActionValue& Value);

    /** Called for Attack input */
    void AttackInputMapping(const FInputActionValue& Value);

    //void PlayImpactAnimation();

    /** Property replication */
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    //void OnHealthUpdate();

    UFUNCTION()
    virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    // APawn interface
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // To add mapping context
    virtual void BeginPlay();

    virtual void Tick(float DeltaTime) override;

public:
    int GetTeam();

    float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

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
