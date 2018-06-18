//
//  SceneManager.cpp
//  ProjectionDemo
//
//  Created by Michael Allison on 4/25/18.
//

#include "SceneManager.h"
#include "ofMain.h"
#include "mediasystem/core/Scene.h"
#include "mediasystem/util/Log.h"
#include "mediasystem/util/Util.h"

namespace mediasystem {
    
    SceneManager::SceneManager()
    {}
    
    SceneManager::~SceneManager()
    {
        shutdownScenes();
    }

    void SceneManager::shutdownScenes()
    {
        for(auto & scene : mScenes){
            scene->shutdown();
        }
        mScenes.clear();
    }
    
    void SceneManager::resetScenes()
    {
        for(auto & scene : mScenes){
            scene->reset();
        }
    }
    
    void SceneManager::initScenes()
    {
        for(auto & scene : mScenes){
            scene->init();
        }
        for(auto & scene : mScenes){
            scene->postInit();
        }
    }
    
    std::shared_ptr<Scene> SceneManager::createScene(const std::string& name, int eventDequeuTimeLimit)
    {
        auto scene = std::make_shared<Scene>(name, eventDequeuTimeLimit);
        addScene(scene);
        return scene;
    }
    
    void SceneManager::changeScene(const std::string& nextScene)
    {
        mNextScene = nullptr;
        mNextScene = getScene(nextScene);
        if(!mNextScene){
            MS_LOG_ERROR("dont have a scene by the name: " + nextScene);
        }else{
            transition();
        }
    }
    
    void SceneManager::changeScene(std::shared_ptr<Scene> scene)
    {
        mNextScene = scene;
        transition();
    }
    
    void SceneManager::addScene(std::shared_ptr<Scene> scene)
    {
        scene->addDelegate<SceneChange>(EventDelegate::create<SceneManager, &SceneManager::onChangeScene>(this));
        mScenes.emplace_back(std::move(scene));
    }
    
    EventStatus SceneManager::swapScenes(const IEventRef&)
    {
        mCurrentScene = mNextScene;
        mNextScene = nullptr;
        return EventStatus::REMOVE_THIS_DELEGATE;
    }
    
    void SceneManager::transition()
    {
        if(mCurrentScene){
            if(mNextScene)
                mNextScene->transitionIn();
            
            mCurrentScene->transitionOut();
            if(mCurrentScene->isTransitioning()){
                mCurrentScene->addDelegate<Stop>(EventDelegate::create<SceneManager, &SceneManager::swapScenes>(this));
            }else{
                mCurrentScene = mNextScene;
                mNextScene = nullptr;
            }
            
        }else{
            mCurrentScene = mNextScene;
            mCurrentScene->transitionIn();
        }
    }
    
    std::shared_ptr<Scene> SceneManager::getScene(const std::string& name) const
    {
        for (size_t i = 0; i < mScenes.size(); i++ ) {
            auto scene = mScenes[i];
            if (scene->getName().compare(name) == 0) {
                return scene;
            }
        }
        MS_LOG_ERROR("dont have a scene by the name: " + name);
        return nullptr;
    }
    
    EventStatus SceneManager::onChangeScene(const IEventRef& sceneChange)
    {
        if(mCurrentScene && mCurrentScene->isTransitioning()){
            return EventStatus::DEFER_EVENT;
        }
        auto cast = std::static_pointer_cast<SceneChange>(sceneChange);
        mNextScene = cast->getNextScene();
        if(mNextScene){
            transition();
        }else{
            changeScene(cast->getNextSceneName());
        }
        return EventStatus::SUCCESS;
    }
    
    void SceneManager::update()
    {
        if(mCurrentScene)
            mCurrentScene->update();
        
        if(mNextScene)
            mNextScene->update();
    }
    
    void SceneManager::draw()
    {
        if(mNextScene)
            mNextScene->draw();
        
        if(mCurrentScene)
            mCurrentScene->draw();
    }

    void SceneManager::destroyScene(const std::string& name)
    {
        auto found = std::find_if(mScenes.begin(), mScenes.end(),[&name](const std::shared_ptr<Scene>& scene){
            return scene->getName() == name;
        });
        if(found != mScenes.end()){
            mScenes.erase(found);
        }else{
            MS_LOG_ERROR("dont have a scene by the name: " + name);
        }
    }
    
    void SceneManager::destroyScene(const std::shared_ptr<Scene>& scene)
    {
        auto found = std::find(mScenes.begin(), mScenes.end(), scene);
        if(found != mScenes.end()){
            mScenes.erase(found);
        }else{
            MS_LOG_ERROR("dont have a scene by the name: " + scene->getName());
        }
    }
    
}//end namespace mediasystem
