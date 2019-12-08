// Fill out your copyright notice in the Description page of Project Settings.

#include "GravityWell.h"
#include "Components/SphereComponent.h"

// Sets default values
AGravityWell::AGravityWell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Colider = CreateDefaultSubobject<USphereComponent>("Colider");
	Colider->SetWorldScale3D(FVector(4.0f));
	RootComponent = Colider;

}

// Called when the game starts or when spawned
void AGravityWell::BeginPlay()
{
	Super::BeginPlay();
	
}

