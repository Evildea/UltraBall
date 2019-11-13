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
	UStaticMeshComponent* UltraBall;
	
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	bool GetHasFinishedLevel();

	UFUNCTION(BlueprintCallable)
	FName GetNextLevel();

	UPROPERTY(EditAnywhere, Category = "Designer")
	FName NextLevel;

private:

	bool HasFinishedLevel;
};