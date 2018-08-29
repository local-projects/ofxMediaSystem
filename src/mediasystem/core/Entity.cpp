//
//  Entity.cpp
//  WallTest
//
//  Created by Michael Allison on 6/14/18.
//

#include "Entity.h"
#include "ofMain.h"

namespace mediasystem {
    
    EntityGraph::EntityGraph(Entity& me):self(me){}
    EntityGraph::~EntityGraph()
    {
        children.clear();
        parent.reset();
    }
    
    void EntityGraph::setParent(EntityHandle p, bool keepGlobalPosition){
        if(auto p = parent.lock()){
            clearParent(keepGlobalPosition);
        }
        if(auto ent = p.lock()){
            parent = std::move(p);
            auto node = ent->getComponent<ofNode>();
            auto myNode = self.getComponent<ofNode>();
            myNode->setParent(*node, keepGlobalPosition);
            auto graph = ent->getComponent<EntityGraph>();
            graph->children.push_back(self.getScene().getEntity(self.getId()));
        }
    }
    
    void EntityGraph::clearParent(bool keepTransform)
    {
        if(auto p = parent.lock()){
            auto graph = p->getComponent<EntityGraph>();
            //remove self as child
            auto it = graph->children.begin();
            auto end = graph->children.end();
            while(it != end){
                if(auto ch = (*it).lock()){
                    if(ch.get() == &self){
                        auto node = ch->getComponent<ofNode>();
                        node->clearParent(keepTransform);
                        graph->children.erase(it);
                        parent.reset();
                        return;
                    }
                    ++it;
                }else{
                    it = graph->children.erase(it);
                }
            }
        }
    }
    
    void EntityGraph::addChild(EntityHandle child, bool keepGlobalPosition){
        auto myHandle = self.getScene().getEntity(self.getId());
        if(auto ent = child.lock()){
            ent->setParent(myHandle);
        }
    }
    
    void EntityGraph::removeChild(EntityHandle child, bool keepGlobalPosition){
        if(auto ent = child.lock()){
            auto it = children.begin();
            auto end = children.end();
            while(it != end){
                if(auto ch = (*it).lock()){
                    if(ch.get() == ent.get()){
                        ch->clearParent(keepGlobalPosition);
                        return;
                    }
                    ++it;
                }else{
                    it = children.erase(it);
                }
            }
        }
    }
    
    std::map<mediasystem::type_id_t, size_t> mediasystem::Entity::sComponentIds = {};
    size_t mediasystem::Entity::sNextComponentId = 0;
    
    Entity::Entity(Scene& scene, uint64_t id):
        mId(id),
        mScene(scene)
    {
        mComponents.reset();
    }
    
    void Entity::clearComponents()
    {
        if(hasComponent<EntityGraph>()){
            auto graph = getComponent<EntityGraph>();
            if(auto p = graph->parent.lock()){
                auto myNode = getComponent<ofNode>();
                if(myNode)
                    myNode->clearParent(true);
            }
            for(auto& child : graph->children){
                if(auto ent = child.lock()){
                    auto node = ent->getComponent<ofNode>();
                    if(node)
                        node->clearParent(true);
                }
            }
        }
        for(size_t i = 0; i < mComponents.size(); i++){
            if(mComponents[i]){
                if(auto compType = getType(i)){
                    mScene.destroyComponent(compType, mId);
                }
            }
        }
        mComponents.reset();
    }
    
    bool Entity::destroy(){
        if(isValid()){
            mScene.destroyEntity(mId);
            return true;
        }
        return false;
    }
    
    void Entity::setParent(EntityHandle parent, bool bMaintainGlobalTransform)
    {
        auto graph = getComponent<EntityGraph>();
        graph->setParent(std::move(parent), bMaintainGlobalTransform);
    }
    
    EntityHandle Entity::getParent() const
    {
        auto graph = getComponent<EntityGraph>();
        return graph->parent;
    }
    
    void Entity::clearParent(bool bMaintainGlobalTransform)
    {
        auto graph = getComponent<EntityGraph>();
        graph->clearParent(bMaintainGlobalTransform);
    }
    
    void Entity::addChild(EntityHandle child)
    {
        auto graph = getComponent<EntityGraph>();
        graph->addChild(child);
    }
    
    void Entity::removeChild(EntityHandle child)
    {
        auto graph = getComponent<EntityGraph>();
        graph->removeChild(child);
    }
    
    std::list<EntityHandle>& Entity::getChildren()
    {
        auto graph = getComponent<EntityGraph>();
        return graph->children;
    }
    
    
    //node component pass through
    void Entity::setPosition(float px, float py, float pz)
    {
        auto node = getComponent<ofNode>();
        node->setPosition(px, py, pz);
    }
    
    void Entity::setPosition(const glm::vec3& p)
    {
        auto node = getComponent<ofNode>();
        node->setPosition(p);
    }
    
    glm::vec3 Entity::getPosition() const
    {
        auto node = getComponent<ofNode>();
        return node->getPosition();
    }
    
    void Entity::setGlobalPosition(float px, float py, float pz)
    {
        auto node = getComponent<ofNode>();
        node->setGlobalPosition(px, py, pz);
    }
    
    void Entity::setGlobalPosition(const glm::vec3& p)
    {
        auto node = getComponent<ofNode>();
        node->setGlobalPosition(p);
    }
    
    glm::vec3 Entity::getGlobalPosition() const
    {
        auto node = getComponent<ofNode>();
        return node->getGlobalPosition();
    }
    
    
    void Entity::setOrientation(const glm::quat& q)
    {
        auto node = getComponent<ofNode>();
        node->setOrientation(q);
    }
    
    void Entity::setOrientation(const glm::vec3& eulerAngles)
    {
        auto node = getComponent<ofNode>();
        node->setOrientation(eulerAngles);
    }
    
    glm::vec3 Entity::getOrientationEulerRad() const
    {
        auto node = getComponent<ofNode>();
        return node->getOrientationEulerRad();
    }
    
    glm::vec3 Entity::getOrientationEulerDeg() const
    {
        auto node = getComponent<ofNode>();
        return node->getOrientationEulerDeg();
    }
    
    glm::quat Entity::getOrientationQuat() const
    {
        auto node = getComponent<ofNode>();
        return node->getOrientationQuat();
    }
    
    glm::vec3 Entity::getUpDir() const
    {
        auto node = getComponent<ofNode>();
        return node->getUpDir();
    }
    
    glm::vec3 Entity::getLookAtDir()const
    {
        auto node = getComponent<ofNode>();
        return node->getLookAtDir();
    }
    
    glm::vec3 Entity::getZAxis() const
    {
        auto node = getComponent<ofNode>();
        return node->getZAxis();
    }
    
    glm::vec3 Entity::getYAxis() const
    {
        auto node = getComponent<ofNode>();
        return node->getYAxis();
    }
    
    glm::vec3 Entity::getXAxis() const
    {
        auto node = getComponent<ofNode>();
        return node->getXAxis();
    }
    
    void Entity::setGlobalOrientation(const glm::quat& q)
    {
        auto node = getComponent<ofNode>();
        node->setGlobalOrientation(q);
    }
    
    glm::quat Entity::getGlobalOrientation() const
    {
        auto node = getComponent<ofNode>();
        return node->getGlobalOrientation();
    }
    
    void Entity::setScale(float s)
    {
        auto node = getComponent<ofNode>();
        node->setScale(s);
    }
    
    void Entity::setScale(float sx, float sy, float sz)
    {
        auto node = getComponent<ofNode>();
        node->setScale(sx, sy, sz);
    }
    
    void Entity::setScale(const glm::vec3& s)
    {
        auto node = getComponent<ofNode>();
        node->setScale(s);
    }
    
    glm::vec3 Entity::getScale() const
    {
        auto node = getComponent<ofNode>();
        return node->getScale();
    }
    
    glm::vec3 Entity::getGlobalScale() const
    {
        auto node = getComponent<ofNode>();
        return node->getGlobalScale();
    }
    
    glm::mat4 Entity::getGlobalTransformMatrix() const
    {
        auto node = getComponent<ofNode>();
        return node->getGlobalTransformMatrix();
    }
    
    const glm::mat4& Entity::getLocalTransformMatrix() const
    {
        auto node = getComponent<ofNode>();
        return node->getLocalTransformMatrix();
    }
    
}//end namespace mediasystem
