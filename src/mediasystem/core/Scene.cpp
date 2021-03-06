#include "Scene.h"
#include "mediasystem/core/Entity.h"
#include "mediasystem/util/Util.h"

namespace mediasystem {

    Scene::Scene(const std::string & name, AllocationManager&& allocationManager):
        mName(name),
        mAllocationManager(std::move(allocationManager))
    {}

    Scene::~Scene()
    {}
    
    static size_t sNextEntityId = 0;
    
    EntityHandle Scene::createEntity()
    {
        auto next = sNextEntityId++;
        auto it = mEntities.emplace(next, allocateStrongHandle<Entity>(Allocator<Entity>( &mAllocationManager ), *this, next));
        if(it.second){
            queueEvent<NewEntity>(it.first->second);
            //everyone gets a node component, because why not
            it.first->second->createComponent<ofNode>();
            it.first->second->createComponent<EntityGraph>(*it.first->second);
            return it.first->second;
        }else{
            return EntityHandle();
        }
    }
    
    void Scene::clearSystems(){
        mSystems.clear();
    }
    
    bool Scene::destroyEntity(size_t id)
    {
        auto found = mEntities.find(id);
        if(found != mEntities.end()){
            mDestroyedEntities.push_back(id);
            return true;
        }
        return false;
    }
    
    void Scene::clearComponents(){
        mComponents.clear();
    }
    
    bool Scene::destroyComponent(type_id_t type, size_t entity_id){
        auto found = mComponents.find(type);
        if(found != mComponents.end()){
            auto foundComponent = found->second.find(entity_id);
            if(foundComponent != found->second.end()){
                found->second.erase(foundComponent);
                return true;
            }else{
                ofLogError("Scene") << ("ComponentManager: Entity id: " + std::to_string(entity_id) + " DOES NOT HAVE COMPONENT");
                return false;
            }
        }
        ofLogError("Scene") << ("ComponentManager: Entity id: " + std::to_string(entity_id) + " DOES NOT HAVE COMPONENT");
        return false;
    }
    
    void Scene::collectEntities()
    {
        for(size_t i = 0; i < mDestroyedEntities.size(); i++){
            auto entId = mDestroyedEntities.front();
            auto ent = mEntities[entId];
            triggerEvent<DestroyEntity>(ent);
            ent->clearComponents();
            mEntities.erase(entId);
            mDestroyedEntities.pop_front();
        }
    }
    
    bool Scene::destroyEntity(EntityHandle handle)
    {
        if(auto ent = handle.lock()){
            return destroyEntity(ent->getId());
        }
        return false;
    }
    
    EntityHandle Scene::getEntity(size_t id)
    {
        auto found = mEntities.find(id);
        if(found != mEntities.end()){
            return EntityHandle(found->second);
        }else{
            return EntityHandle();
        }
    }
    
    float Scene::getPercentTransitionComplete() const
    {
        switch(mTransitionDirection){
            case TRANSITION_IN:
                return (ofGetElapsedTimef() - mTransitionStart) / mTransitionInDuration;
            case TRANSITION_OUT:
                return (ofGetElapsedTimef() - mTransitionStart) / mTransitionOutDuration;
            default: return 0;
        }
    }
    
    void Scene::notifyTransitionIn()
    {
        if(mTransitionInDuration > 0){
            mTransitionDirection = TRANSITION_IN;
            mIsTransitioning = true;
            mTransitionStart = ofGetElapsedTimef();
            transitionIn();
            triggerEvent<TransitionInBegin>(*this);
        }
        notifyStart();
    }
    
    void Scene::notifyTransitionOut()
    {
        if(mTransitionOutDuration > 0){
            mTransitionDirection = TRANSITION_OUT;
            mIsTransitioning = true;
            mTransitionStart = ofGetElapsedTimef();
            transitionOut();
            triggerEvent<TransitionOutBegin>(*this);
        }else{
            notifyStop();
        }
    }
    
    void Scene::notifyTransitionInComplete()
    {
        mIsTransitioning = false;
        transitionInComplete();
        triggerEvent<TransitionInComplete>(*this);
    }
    
    void Scene::notifyTransitionOutComplete()
    {
        mIsTransitioning = false;
        transitionOutComplete();
        triggerEvent<TransitionOutComplete>(*this);
        notifyStop();
    }
    
    void Scene::notifyUpdate(size_t elapsedFrames, float elapsedTime, float prevFrameTime)
    {
        mCurrentTime = elapsedTime;
        
        if(!mStagedCues.empty())
            mCues.splice(mCues.end(), mStagedCues);
        
        auto it = mCues.begin();
        auto end = mCues.end();
        while(it!=end){
            auto & cue = *it;
            if(elapsedTime >= cue.executionTime){
                cue.handler();
                if(cue.repeats){
                    cue.executionTime += cue.interval;
                    ++it;
                }else{
                    it = mCues.erase(it);
                }
            }else {
                ++it;
            }
        }
        
        if(mIsTransitioning){
            auto perc = getPercentTransitionComplete();
            if(perc >= 1.){
                switch(mTransitionDirection){
                    case TRANSITION_IN:
                        notifyTransitionInComplete();
                        return;
                    case TRANSITION_OUT:
                        notifyTransitionOutComplete();
                        return;
                }
            }else{
                transitionUpdate();
                triggerEvent<TransitionUpdate>(*this);
            }
        }
        mSequence.update(elapsedFrames,elapsedTime,prevFrameTime);
        update(elapsedFrames, elapsedTime, prevFrameTime);
        triggerEvent<Update>(*this, elapsedFrames, elapsedTime, prevFrameTime);
        //process any events queued by other systems and components, etc.
        processEvents();
        collectEntities();
    }
    
    void Scene::notifyStart()
    {
        mHasStarted = true;
        start();
        triggerEvent<Start>(*this);
    }

    void Scene::notifyStop()
    {
        mStagedCues.clear();
        mCues.clear();
        mHasStarted = false;
        mIsTransitioning = false;
        stop();
        triggerEvent<Stop>(*this);
    }
    
    float Scene::getTransitionDuration(TransitionDir direction)const
    {
        switch(direction){
            case TRANSITION_IN:
                return mTransitionInDuration;
            case TRANSITION_OUT:
                return mTransitionOutDuration;
        }
        return 0.f;
    }
    
    void Scene::setTransitionDuration(TransitionDir direction, float duration)
    {
        switch(direction){
            case TRANSITION_IN:
                mTransitionInDuration = duration;
                return;
            case TRANSITION_OUT:
                mTransitionOutDuration = duration;
                return;
        }
    }
    
    void Scene::notifyInit()
    {
        init();
        triggerEvent<Init>(*this);
    }
    
    void Scene::notifyPostInit()
    {
        postInit();
        triggerEvent<PostInit>(*this);
    }
    
    void Scene::notifyShutdown()
    {
        mStagedCues.clear();
        mCues.clear();
        shutdown();
        triggerEvent<Shutdown>(*this);
        for(auto & ent : mEntities){
            ent.second->clearComponents();
        }
        clearComponents();
        mEntities.clear();
        clearSystems();
        clearQueues();
        clearDelegates();
    }
    
    void Scene::notifyDraw()
    {
        draw();
        triggerEvent<Draw>(*this);
    }
    
    void Scene::notifyReset()
    {
        reset();
        queueEvent<Reset>(*this);
    }
    
    void Scene::addState(StateMachine::State&& state)
    {
        mSequence.addState(std::move(state));
    }
    
    void Scene::addChildState(std::string parent, StateMachine::State&& state)
    {
        mSequence.addChildState(std::move(parent), std::move(state));
    }
    
    void Scene::requestState(std::string state)
    {
        mSequence.requestState(std::move(state));
    }

    static size_t sCueIds = 0;
    
    CueId Scene::cueAtTime(float seconds, std::function<void()> handler)
    {
        if(seconds > mCurrentTime){
            auto id = sCueIds++;
            Cue c;
            c.interval = seconds;
            c.executionTime = mCurrentTime + seconds;
            c.handler = std::move(handler);
            c.id = id;
            mStagedCues.emplace_back(std::move(c));
            return id;
        }
        
        ofLogError("Scene") << "Could not create a cue at time: " << seconds << " that has already passed, current time: " << mCurrentTime;
        
        return -1;
    }
    
    CueId Scene::cueFromNow(float seconds, std::function<void()> handler)
    {
        auto id = sCueIds++;
        Cue c;
        c.interval = seconds;
        c.executionTime = mCurrentTime + seconds;
        c.handler = std::move(handler);
        c.id = id;
        mStagedCues.emplace_back(std::move(c));
        return id;
    }
    
    CueId Scene::cueInterval(float seconds, std::function<void()> handler)
    {
        auto id = sCueIds++;
        Cue c;
        c.interval = seconds;
        c.executionTime = mCurrentTime + seconds;
        c.handler = std::move(handler);
        c.id = id;
        mStagedCues.emplace_back(std::move(c));
        return id;
    }
    
    void Scene::cancelCue(CueId id)
    {
        auto found = std::find_if(mCues.begin(), mCues.end(), [id](const Cue& cue){
            return cue.id == id;
        });
        if(found!=mCues.end()){
            mCues.erase(found);
            return;
        }
        found = std::find_if(mStagedCues.begin(), mStagedCues.end(), [id](const Cue& cue){
            return cue.id == id;
        });
        if(found!=mStagedCues.end()){
            mStagedCues.erase(found);
            return;
        }
        ofLogWarning("Scene") << "There is no Cue for id: " << id;
    }
    
}//end namespace mediasystem
