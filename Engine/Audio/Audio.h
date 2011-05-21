//
// Urho3D Engine
// Copyright (c) 2008-2011 Lasse ��rni
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "AudioDefs.h"
#include "Mutex.h"
#include "Object.h"
#include "Quaternion.h"
#include "Thread.h"
#include "SharedArrayPtr.h"

class AudioImpl;
class Sound;
class SoundSource;

/// Audio subsystem. Produces sound output using DirectSound
class Audio : public Object, public Thread
{
    OBJECT(Audio);
    
public:
    /// Construct
    Audio(Context* context);
    /// Destruct. Terminate the audio thread and free the audio buffer
    virtual ~Audio();
    
    /// Initialize sound output with specified buffer length and output mode
    bool SetMode(int bufferLengthMSec, int mixRate, bool sixteenBit, bool stereo, bool interpolate = true);
    /// Run update on sound sources. Not required for continued playback, but frees unused sound sources & sounds and updates 3D positions.
    void Update(float timeStep);
    /// Restart sound output
    bool Play();
    /// Suspend sound output
    void Stop();
    /// Set master gain on a specific sound type such as sound effects, music or voice.
    void SetMasterGain(SoundType type, float gain);
    /// Set listener position
    void SetListenerPosition(const Vector3& position);
    /// Set listener rotation
    void SetListenerRotation(const Quaternion& rotation);
    /// Set listener position and rotation
    void SetListenerTransform(const Vector3& position, const Quaternion& rotation);
    /// Stop any sound source playing a certain sound clip
    void StopSound(Sound* sound);
    
    /// Return audio implementation, which contains the actual DirectSound objects
    AudioImpl* GetImpl() { return impl_; }
    /// Return sound buffer size in samples
    unsigned GetBufferSamples() const { return bufferSamples_; }
    /// Return sound buffer size in bytes
    unsigned GetBufferSize() const { return bufferSize_; }
    /// Return byte size of one sample
    unsigned GetSampleSize() const { return sampleSize_; }
    /// Return mixing rate
    int GetMixRate() const { return mixRate_; }
    /// Return whether output is sixteen bit
    bool IsSixteenBit() const { return sixteenBit_; }
    /// Return whether output is stereo
    bool IsStereo() const { return stereo_; }
    /// Return whether output is interpolated
    bool IsInterpolated() const { return interpolate_; }
    /// Return whether audio is being output
    bool IsPlaying() const { return playing_; }
    /// Return whether an audio buffer has been reserved
    bool IsInitialized() const;
    /// Return master gain for a specific sound source type
    float GetMasterGain(SoundType type) const;
    /// Return listener position
    const Vector3& GetListenerPosition() const { return listenerPosition_; }
    /// Return listener rotation
    const Quaternion& GetListenerRotation() const { return listenerRotation_; }
    /// Return all sound sources
    const PODVector<SoundSource*>& GetSoundSources() const { return soundSources_; }
    
    /// Add a sound source to keep track of. Called by SoundSource
    void AddSoundSource(SoundSource* soundSource);
    /// Remove a sound source. Called by SoundSource
    void RemoveSoundSource(SoundSource* soundSource);
    /// Return audio thread mutex
    Mutex& GetMutex() { return audioMutex_; }
    /// Return sound type specific gain multiplied by master gain
    float GetSoundSourceMasterGain(SoundType type) const { return masterGain_[SOUND_MASTER] * masterGain_[type]; }
    
    /// Mixing thread function
    virtual void ThreadFunction();
    
private:
    /// Initialize when screen mode initially set
    void Initialize();
    /// Mix sound sources into the buffer
    void MixOutput(void* dest, unsigned bytes);
    /// Release the sound buffer
    void ReleaseBuffer();
    /// Handle screen mode event
    void HandleScreenMode(StringHash eventType, VariantMap& eventData);
    /// Handle render update event
    void HandleRenderUpdate(StringHash eventType, VariantMap& eventData);
    
    /// Implementation
    AudioImpl* impl_;
    /// Clipping buffer for mixing
    SharedArrayPtr<int> clipBuffer_;
    /// Audio thread mutex
    Mutex audioMutex_;
    /// Window handle
    unsigned windowHandle_;
    /// Sound buffer size in samples
    unsigned bufferSamples_;
    /// Sound buffer size in bytes
    unsigned bufferSize_;
    /// Sample size
    unsigned sampleSize_;
    /// Mixing rate
    int mixRate_;
    /// Sixteen bit flag
    bool sixteenBit_;
    /// Stereo flag
    bool stereo_;
    /// Interpolation flag
    bool interpolate_;
    /// Playing flag
    bool playing_;
    /// Master gain by sound source type
    float masterGain_[MAX_SOUND_TYPES];
    /// Sound sources
    PODVector<SoundSource*> soundSources_;
    /// Listener position
    Vector3 listenerPosition_;
    /// Listener rotation
    Quaternion listenerRotation_;
};

/// Register Sound library objects
void RegisterAudioLibrary(Context* context);
