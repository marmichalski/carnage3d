#include "stdafx.h"
#include "AudioManager.h"
#include "GameMapManager.h"
#include "AudioDevice.h"
#include "CarnageGame.h"

AudioManager gAudioManager;

bool AudioManager::Initialize()
{
    if (!AllocateAudioSources())
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot allocate audio sources");
        return false;
    }

    return true;
}

void AudioManager::Deinit()
{
    StopAllSounds();
    FreeLevelSounds();

    ReleaseAudioSources();
}

void AudioManager::UpdateFrame()
{

}

bool AudioManager::LoadLevelSounds()
{
    FreeLevelSounds();

    gConsole.LogMessage(eLogMessage_Debug, "Loading level sounds...");
    if (!mVoiceSounds.LoadArchive("AUDIO/VOCALCOM"))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load Voice sounds");
    }

    std::string audioBankFileName = cxx::va("AUDIO/LEVEL%03d", gGameMap.mAudioFileNumber); 
    if (!mLevelSounds.LoadArchive(audioBankFileName))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load Level sounds");
    }

    mLevelSoundsBuffers.resize(mLevelSounds.GetEntriesCount());
    mVoiceSoundsBuffers.resize(mVoiceSounds.GetEntriesCount());

    return true;
}

void AudioManager::FreeLevelSounds()
{
    mLevelSounds.FreeArchive();
    mVoiceSounds.FreeArchive();

    // detach buffers
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource)
        {
            currSource->SetupSourceBuffer(nullptr);
        }
    }

    // free audio buffers
    for (AudioBuffer* currBuffer: mLevelSoundsBuffers)
    {
        if (currBuffer)
        {
            gAudioDevice.DestroyAudioBuffer(currBuffer);
        }
    }
    mLevelSoundsBuffers.clear();

    // free audio buffers
    for (AudioBuffer* currBuffer: mVoiceSoundsBuffers)
    {
        if (currBuffer)
        {
            gAudioDevice.DestroyAudioBuffer(currBuffer);
        }
    }
    mVoiceSoundsBuffers.clear();
}

void AudioManager::ReleaseAudioSources()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource)
        {
            gAudioDevice.DestroyAudioSource(currSource);
        }
    }
    mAudioSources.clear();
}

AudioSource* AudioManager::PlaySfxLevel(int sfxIndex, const glm::vec3& position, bool enableLoop)
{
    if (sfxIndex < 0 || sfxIndex >= mLevelSounds.GetEntriesCount())
    {
        debug_assert(false);
        return nullptr;
    }

    AudioSource* audioSource = GetFreeSfxAudioSource();
    if (audioSource == nullptr)
        return nullptr; // out of free audio channels

    if (mLevelSoundsBuffers[sfxIndex] == nullptr)
    {
        SfxArchiveEntry archiveEntry;
        if (!mLevelSounds.GetEntryData(sfxIndex, archiveEntry))
        {
            debug_assert(false);
            return nullptr;
        }
        // upload audio data
        AudioBuffer* audioBuffer = gAudioDevice.CreateAudioBuffer(
            archiveEntry.mSampleRate,
            archiveEntry.mBitsPerSample,
            archiveEntry.mChannelsCount,
            archiveEntry.mDataLength,
            archiveEntry.mData);

        // free source data
        mLevelSounds.FreeEntryData(sfxIndex);

        debug_assert(audioBuffer && !audioBuffer->IsBufferError());

        mLevelSoundsBuffers[sfxIndex] = audioBuffer;
    }
    // get buffer
    if (!audioSource->SetupSourceBuffer(mLevelSoundsBuffers[sfxIndex]))
    {
        debug_assert(false);
    }
    audioSource->SetPitch(1.0f);
    audioSource->SetPosition3D(position.x, position.y, position.z);
    audioSource->Start(enableLoop);
    return audioSource;
}

AudioSource* AudioManager::PlaySfxVoice(int sfxIndex, const glm::vec3& position, bool enableLoop)
{
    if (sfxIndex < 0 || sfxIndex >= mVoiceSounds.GetEntriesCount())
    {
        debug_assert(false);
        return nullptr;
    }

    AudioSource* audioSource = GetFreeSfxAudioSource();
    if (audioSource == nullptr)
        return nullptr; // out of free audio channels

    if (mVoiceSoundsBuffers[sfxIndex] == nullptr)
    {
        SfxArchiveEntry archiveEntry;
        if (!mVoiceSounds.GetEntryData(sfxIndex, archiveEntry))
        {
            debug_assert(false);
            return nullptr;
        }
        // upload audio data
        AudioBuffer* audioBuffer = gAudioDevice.CreateAudioBuffer(
            archiveEntry.mSampleRate,
            archiveEntry.mBitsPerSample,
            archiveEntry.mChannelsCount,
            archiveEntry.mDataLength,
            archiveEntry.mData);

        // free source data
        mVoiceSounds.FreeEntryData(sfxIndex);

        debug_assert(audioBuffer && !audioBuffer->IsBufferError());

        mVoiceSoundsBuffers[sfxIndex] = audioBuffer;
    }
    // get buffer
    if (!audioSource->SetupSourceBuffer(mVoiceSoundsBuffers[sfxIndex]))
    {
        debug_assert(false);
    }
    audioSource->SetPitch(1.0f);
    audioSource->SetPosition3D(position.x, position.y, position.z);
    audioSource->Start(enableLoop);
    return audioSource;
}

AudioSource* AudioManager::GetFreeSfxAudioSource() const
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource && !currSource->IsPlaying() && !currSource->IsPaused())
            return currSource;
    }

    return nullptr;
}

bool AudioManager::AllocateAudioSources()
{
    const int MaxAudioSources = 32;
    for (int icurr = 0; icurr < MaxAudioSources; ++icurr)
    {
        AudioSource* audioSource = gAudioDevice.CreateAudioSource();
        if (audioSource == nullptr)
            break;

        mAudioSources.push_back(audioSource);
    }
    return true;
}

void AudioManager::StopAllSounds()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource->IsPlaying() || currSource->IsPaused())
        {
            currSource->Stop();
        }
    }
}

void AudioManager::PauseAllSounds()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource->IsPlaying())
        {
            currSource->Pause();
        }
    }
}

void AudioManager::ResumeAllSounds()
{
    for (AudioSource* currSource: mAudioSources)
    {
        if (currSource->IsPaused())
        {
            currSource->Resume();
        }
    }
}