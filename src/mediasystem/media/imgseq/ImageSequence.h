#pragma once

#include "ofMain.h"
#include "mediasystem/util/Playable.h"

namespace mediasystem {
    
    class ImageSequenceBase : public Playable {
    public:
        
        ImageSequenceBase(std::filesystem::path&& pathToImgDir, float fps, const Playable::Options& options);
        virtual ~ImageSequenceBase() = default;
        
        void setImgDir(std::filesystem::path&& pathToImgDir);
        
        virtual void init() = 0;
        virtual ofTexture* getCurrentTexture() = 0;
        virtual glm::vec2 getSize() const  = 0;
        
        bool isInit() const { return mIsInit; }
        
        inline void setTextureLocation(int bindLocation){ mTextureLocation = bindLocation; }
        void bind();
        void unbind();
        
    protected:
        
        virtual void initPaths(const std::filesystem::path& imageSequenceDir);
        
        static void parseSource(const std::filesystem::path& path, std::vector<std::filesystem::path>& imageFiles, const std::vector<std::filesystem::path> allowedExt = { ".png", ".jpg", ".gif", ".JPG", ".PNG", ".jpeg", ".JPEG" });
        
        std::filesystem::path mImagesDir;
        std::vector<std::filesystem::path> mSeqPaths;
        bool mIsInit{false};
        int mTextureLocation{0};
    };
    
    class ImageSequence {
    public:
        
        enum Type { SEQ_PRELOAD, SEQ_DISK_STREAMING };
        
        class Options {
        public:
            
            Options():mType(SEQ_PRELOAD){}
            
            inline Options& streaming(){ mType = SEQ_DISK_STREAMING; return *this; }
            inline Options& preload(){ mType = SEQ_PRELOAD; return *this; }
            inline Options& disableLooping(){ mPlayableOptions.disableLooping(); return *this; }
            inline Options& keyFrame(uint32_t frame, const Playable::KeyFrameCallback& callback){ mPlayableOptions.keyFrame(frame,callback); return *this; }
            inline Options& boomarang(){ mPlayableOptions.boomarang(); return *this; }
            inline Options& reverse(){ mPlayableOptions.reverse(); return *this; }
            inline Options& loopPoints(uint32_t beginFrame, uint32_t endFrame){ mPlayableOptions.loopPoints(beginFrame,endFrame); return *this; }
            
        private:
            ImageSequence::Type mType;
            Playable::Options mPlayableOptions;
            friend ImageSequence;
        };
        
        ImageSequence(std::filesystem::path pathToImgDir, float fps, const ImageSequence::Options& options = ImageSequence::Options());
        
        inline void setImgDir(std::filesystem::path pathToImgDir){ mImpl->setImgDir(std::move(pathToImgDir)); }
        inline void init(){ return mImpl->init(); };
        inline ofTexture* getCurrentTexture(){ return mImpl->getCurrentTexture(); }
        inline glm::vec2 getSize() const { return mImpl->getSize(); }
        inline bool isInit() const { return mImpl->isInit(); }
        inline void setTextureLocation(int bindLocation){ mImpl->setTextureLocation(bindLocation); }
        inline void bind(){ mImpl->bind(); }
        inline void unbind(){ mImpl->unbind(); }
        inline void play(){ mImpl->play(); }
        inline void stop(){ mImpl->stop(); }
        inline void pause(){ mImpl->pause(); }
        inline void advanceFrame() { mImpl->advanceFrame(); }
        inline void step(double amount){ mImpl->step(amount); }
        inline void reset(){ mImpl->reset(); }
        
        inline void setKeyFrame(uint32_t frame, const Playable::KeyFrameCallback& callback){ mImpl->setKeyFrame(frame, callback); }
        
        inline const Playable::State& getState()const{ return mImpl->getState(); }
        
        inline bool isPaused()const { return mImpl->isPaused(); }
        inline bool isPlaying()const { return mImpl->isPlaying(); }
        inline bool isReversed()const { return mImpl->isReversed(); }
        inline bool isLooping()const { return mImpl->isLooping(); }
        inline bool isBoomarang()const { return mImpl->isBoomarang(); }
        inline bool isFinished()const { return mImpl->isFinished(); }
        
        inline void setLoop(bool flag = true){ mImpl->setLoop(flag); }
        inline void setReverse(bool flag = true){ mImpl->setReverse(flag); }
        inline void setBoomarang(bool flag = true){ mImpl->setBoomarang(flag); }
        inline void setLoopPointIn(uint32_t frame){ mImpl->setLoopPointIn(frame); }
        inline void setLoopPointOut(uint32_t frame){ mImpl->setLoopPointOut(frame); }
        inline void setLoopPoints(uint32_t inFrame, uint32_t outFrame ){ mImpl->setLoopPoints(inFrame, outFrame); }
        
        inline void setFinished(bool flag = true){ mImpl->setFinished(flag); }
        
        inline float getPlaybackPosition() const { return mImpl->getPlaybackPosition(); }
        
        inline uint32_t getCurrentFrame() const { return mImpl->getCurrentFrame(); }
        inline uint32_t getTotalFrames() const { return mImpl->getTotalFrames(); }
        inline uint32_t getLoopPointIn() const { return mImpl->getLoopPointIn(); }
        inline uint32_t getLoopPointOut() const { return mImpl->getLoopPointOut(); }
        inline float getDuration() const { return mImpl->getDuration(); }
        inline void setFramerate(const float rate) { mImpl->setFramerate(rate); }
        inline float getFramerate() const { return mImpl->getFramerate(); }
        
    private:
        Type mType{SEQ_PRELOAD};
        std::unique_ptr<ImageSequenceBase> mImpl;
    };

}//end namespace mediasystem
