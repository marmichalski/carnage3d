#include "stdafx.h"
#include "Explosion.h"
#include "TimeManager.h"
#include "SpriteManager.h"
#include "DebugRenderer.h"
#include "PhysicsManager.h"
#include "Pedestrian.h"
#include "Vehicle.h"
#include "BroadcastEventsManager.h"
#include "GameObjectsManager.h"
#include "AudioManager.h"

Explosion::Explosion() 
    : GameObject(eGameObjectClass_Explosion, GAMEOBJECT_ID_NULL)
{
}

void Explosion::PreDrawFrame()
{
}

void Explosion::UpdateFrame()
{
    float deltaTime = gTimeManager.mGameFrameDelta;
    if (mAnimationState.UpdateFrame(deltaTime))
    {
        gSpriteManager.GetExplosionTexture(mAnimationState.GetSpriteIndex(), mDrawSprite);

        if (mAnimationState.mFrameCursor == 6) // todo: magic numbers
        {
            glm::vec3 currentPosition = GetPosition();
            // create smoke effect
            Decoration* bigSmoke = gGameObjectsManager.CreateBigSmoke(currentPosition);
            debug_assert(bigSmoke);
        }
    }

    if (!IsDamageDone())
    {
        ProcessPrimaryDamage();
        ProcessSecondaryDamage();
    }

    if (!mAnimationState.IsActive())
    {
        MarkForDeletion();
    }
}

void Explosion::DebugDraw(DebugRenderer& debugRender)
{
    float damageRadiusA = gGameParams.mExplosionPrimaryDamageDistance;
    float damageRadiusB = gGameParams.mExplosionSecondaryDamageDistance;

    debugRender.DrawSphere(mSpawnPosition, damageRadiusA, Color32_Red, false);
    debugRender.DrawSphere(mSpawnPosition, damageRadiusB, Color32_Yellow, false);
}

glm::vec3 Explosion::GetPosition() const
{
    return mSpawnPosition;
}

glm::vec2 Explosion::GetPosition2() const
{
    return { mSpawnPosition.x, mSpawnPosition.z };
}

void Explosion::Spawn(const glm::vec3& position, cxx::angle_t heading)
{
    mSpawnPosition = position;
    mSpawnHeading = heading;

    mDrawSprite.mPosition.x = position.x;
    mDrawSprite.mPosition.y = position.z;
    mDrawSprite.mHeight = position.y;
    mDrawSprite.mDrawOrder = eSpriteDrawOrder_Explosion;

    mAnimationState.Clear();
    // todo
    int numFrames = gSpriteManager.GetExplosionFramesCount();
    mAnimationState.mAnimDesc.SetFrames(0, numFrames);
    mAnimationState.PlayAnimation(eSpriteAnimLoop_FromStart);
    mAnimationState.SetMaxRepeatCycles(1);

    if (!gSpriteManager.GetExplosionTexture(0, mDrawSprite))
    {
        debug_assert(false);
    }

    mPrimaryDamageDone = false;
    mSecondaryDamageDone = false;

    // broadcast event
    glm::vec2 position2 (position.x, position.z);
    gBroadcastEvents.RegisterEvent(eBroadcastEvent_Explosion, position2, gGameParams.mBroadcastExplosionEventDuration);

    gAudioManager.PlaySfxLevel(SfxLevel_HugeExplosion, GetPosition(), false);
}

bool Explosion::IsDamageDone() const
{
    return mPrimaryDamageDone && mSecondaryDamageDone;
}

void Explosion::ProcessPrimaryDamage()
{
    if (mPrimaryDamageDone)
        return;

    mPrimaryDamageDone = true;

    // primary damage
    glm::vec2 centerPoint (mSpawnPosition.x, mSpawnPosition.z);
    glm::vec2 extents (
        gGameParams.mExplosionPrimaryDamageDistance,
        gGameParams.mExplosionPrimaryDamageDistance
    );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        DamageInfo damageInfo;
        damageInfo.SetDamageFromExplosion(gGameParams.mExplosionPrimaryDamage, this);

        if (CarPhysicsBody* carPhysics = currElement.mCarComponent)
        {
            // todo: temporary implementation
            carPhysics->mReferenceCar->ReceiveDamage(damageInfo);
            continue;
        }

        if (PedPhysicsBody* pedPhysics = currElement.mPedComponent)
        {
            // todo: temporary implementation
            pedPhysics->mReferencePed->ReceiveDamage(damageInfo);
            continue;
        }
    }

    queryResult.Clear();
}

void Explosion::ProcessSecondaryDamage()
{
    if (mSecondaryDamageDone)
        return;

    // do secondary damage
    glm::vec2 centerPoint (mSpawnPosition.x, mSpawnPosition.z);
    glm::vec2 extents (
        gGameParams.mExplosionSecondaryDamageDistance,
        gGameParams.mExplosionSecondaryDamageDistance
    );

    PhysicsQueryResult queryResult;
    gPhysics.QueryObjectsWithinBox(centerPoint, extents, queryResult);

    for (int icurr = 0; icurr < queryResult.mElementsCount; ++icurr)
    {
        PhysicsQueryElement& currElement = queryResult.mElements[icurr];

        DamageInfo damageInfo;
        damageInfo.SetDamageFromFire(gGameParams.mExplosionSecondaryDamage, this);

        if (CarPhysicsBody* carPhysics = currElement.mCarComponent)
        {
            // todo: temporary implementation
            carPhysics->mReferenceCar->ReceiveDamage(damageInfo);
            continue;
        }

        if (PedPhysicsBody* pedPhysics = currElement.mPedComponent)
        {
            // todo: temporary implementation
            pedPhysics->mReferencePed->ReceiveDamage(damageInfo);
            continue;
        }
    }

    queryResult.Clear();
}

void Explosion::DisablePrimaryDamage()
{
    mPrimaryDamageDone = true;
}

void Explosion::DisableSecondaryDamage()
{
    mSecondaryDamageDone = true;
}

void Explosion::SetIsCarExplosion(Vehicle* carObject)
{
    debug_assert(carObject);
    mIsCarExplosion = true;
}

bool Explosion::IsCarExplosion() const
{
    return mIsCarExplosion;
}
