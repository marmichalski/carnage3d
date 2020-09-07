#pragma once

#include "GameDefs.h"

// defines manager that hold all game logic parameters
class GameParams
{
public:
    GameParams();

    // reset all rules to default values
    void SetToDefaults();

public:
    // pedestrians
    float mPedestrianBoundsSphereRadius; // bounding sphere radius, meters
    float mPedestrianTurnSpeed; // degrees per second
    float mPedestrianTurnSpeedSlideOnCar; // degrees per second
    float mPedestrianSlideOnCarSpeed; // meters per second
    float mPedestrianWalkSpeed; // meters per second
    float mPedestrianRunSpeed; // meters per second
    float mPedestrianKnockedDownTime; // knocked down duration after the punch in face, seconds
    float mPedestrianSpotTheCarDistance; // max distance to detect the car, meters
    float mPedestrianFallDeathHeight; // falling distance which causes pedestrian death, meters
    float mPedestrianDrowningTime; // seconds
    float mPedestrianBurnDuration; // time while pedestrian can live while burning, seconds

    // traffic - pedestrians
    int mTrafficGenMaxPeds; // max number of traffic pedestrians around the player
    int mTrafficGenPedsChance; // chance to generate new traffic pedestrian on current turn
    int mTrafficGenPedsMaxDistance; // maximum distance from player camera, blocks
    float mTrafficGenPedsCooldownTime; // seconds between traffic generation

    // traffic - cars
    int mTrafficGenMaxCars; // max number of traffic cars around the player
    int mTrafficGenCarsChance; // chance to generate new traffic car on current turn
    int mTrafficGenCarsMaxDistance; // maximum distance from player camera, blocks
    float mTrafficGenCarsCooldownTime; // seconds between traffic generation

    // explosion
    float mExplosionPrimaryDamageDistance; // how far explosion can do maximum damage, meters
    float mExplosionSecondaryDamageDistance; // how far explosion can do significant damage, meters
    int mExplosionPrimaryDamage; // hit points to injure at Primary Damage Distance
    int mExplosionSecondaryDamage; // hit points to injure at Secondary Damage Distance

    // vehicles
    float mVehicleBurnDuration; // time before flame will go out by itself, seconds

   
};

extern GameParams gGameParams;