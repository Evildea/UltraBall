// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishTarget.generated.h"

UCLASS()
class GOLF_API AFinishTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFinishTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Visible Components
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Base;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* UltraBallOuter;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* UltraBallInner;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintPure)
	bool GetHasFinishedLevel();

	UFUNCTION(BlueprintPure)
	FName GetNextLevel();

	UPROPERTY(EditAnywhere, Category = "Designer")
	FName NextLevel;

private:

	bool HasFinishedLevel;

};
