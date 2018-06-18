//
//  RenderComponent.cpp
//  WallTest
//
//  Created by Michael Allison on 6/12/18.
//

#include "LayeredMediaComponent.h"
#include "mediasystem/core/Entity.h"
#include "ofMain.h"

namespace mediasystem {
    
    LayeredMediaComponent::LayeredMediaComponent(Entity& context, std::shared_ptr<MediaBase> media, RectMode mode, TextureMode tc ):
        mContext(context),
        mNode(context.getComponent<ofNode>()),
        mMedia(std::move(media)),
        mRectMode(mode),
        mTexMode(tc)
    {
        mMedia->init();
        auto size = mMedia->getMediaSize();
        setRect(size.x, size.y, size.x, size.y);
    }
    
    LayeredMediaComponent::LayeredMediaComponent(Entity& context, std::shared_ptr<MediaBase> media, float width, float height, RectMode mode, TextureMode tc ):
        mContext(context),
        mNode(context.getComponent<ofNode>()),
        mMedia(std::move(media)),
        mRectMode(mode),
        mTexMode(tc)
    {
        mMedia->init();
        auto size = mMedia->getMediaSize();
        setRect(width, height, size.x, size.y);
    }
    
    void LayeredMediaComponent::setRect(float width, float height, float mediaW, float mediaH)
    {
        mRect.clear();
        
        switch(mRectMode){
            case RECT_MODE_CENTER:
            {
                mRect.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
                
                mRect.addVertex(glm::vec3(-width/2.f,-height/2.f, 0.f));
                mRect.addVertex(glm::vec3(width/2.f,-height/2.f, 0.f));
                mRect.addVertex(glm::vec3(-width/2.f,height/2.f, 0.f));
                mRect.addVertex( glm::vec3(width/2.f,height/2.f, 0.f));
                
                mDebugRect = ofRectangle(-width/2.f,-height/2.f, width, height);
                
            }break;
            case RECT_MODE_TOP_LEFT:
            {
                mRect.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
                
                mRect.addVertex(glm::vec3(0.f));
                mRect.addVertex(glm::vec3(width, 0.f, 0.f));
                mRect.addVertex(glm::vec3(0,height, 0.f));
                mRect.addVertex( glm::vec3(width,height, 0.f));
                
                mDebugRect = ofRectangle(0,0, width, height);
                
            }break;
        }
        
        switch(mTexMode){
            case RECTANGLE:
            {
                mRect.addTexCoord(glm::vec2(0,0));
                mRect.addTexCoord(glm::vec2(mediaW,0));
                mRect.addTexCoord(glm::vec2(0,mediaH));
                mRect.addTexCoord(glm::vec2(mediaW,mediaH));
            }break;
            case NORMALIZED:
            {
                mRect.addTexCoord(glm::vec2(0,0));
                mRect.addTexCoord(glm::vec2(1,0));
                mRect.addTexCoord(glm::vec2(0,1));
                mRect.addTexCoord(glm::vec2(1,1));
            }break;
        }
    }
    
    glm::mat4 LayeredMediaComponent::getGlobalTransform()
    {
        glm::mat4 ret;
        if(auto node = mNode.lock()){
            ret = node->getGlobalTransformMatrix();
        }
        return ret;
    }
    
    void LayeredMediaComponent::setRectMode(RectMode mode)
    {
        mRectMode = mode;
        auto size = mMedia->getMediaSize();
        setRect(mDebugRect.width, mDebugRect.height, size.x, size.y);
    }
    
    LayeredMediaComponent::RectMode LayeredMediaComponent::getRectMode()const
    {
        return mRectMode;
    }
    
    glm::vec2 LayeredMediaComponent::getSize() const
    {
        return glm::vec2(mDebugRect.width, mDebugRect.height);
    }
    
    ofRectangle LayeredMediaComponent::getRect() const
    {
        return mDebugRect;
    }
    
    void LayeredMediaComponent::setSize(float w, float h)
    {
        auto size = mMedia->getMediaSize();
        setRect(w, h, size.x, size.y);
    }
    
    void LayeredMediaComponent::draw()
    {
        mMedia->bind();
        mRect.draw();
        mMedia->unbind();
    }
    
    void LayeredMediaComponent::debugDraw()
    {
        mMedia->debugDraw(mDebugRect, glm::min(mDebugRect.height/10.f, 12.f));
    }
    
    glm::vec2 LayeredMediaComponent::getMediaSize()
    {
        return mMedia->getMediaSize();
    }

    
}//end namespace mediasystem
