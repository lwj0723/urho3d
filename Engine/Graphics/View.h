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

#include "Batch.h"
#include "Object.h"

class Camera;
class DebugRenderer;
class Light;
class Drawable;
class OcclusionBuffer;
class RenderSurface;
class Zone;
struct Viewport;

static const int MAX_LIGHT_SPLITS = 6;

/// Geometry view space depth minimum and maximum values
struct GeometryDepthBounds
{
    float min_;
    float max_;
};


/// Helper structure for checking whether a transparent object is already lit by a certain light
struct LitTransparencyCheck
{
    LitTransparencyCheck(Light* light, Drawable* drawable, unsigned batchIndex) :
        light_(light),
        drawable_(drawable),
        batchIndex_(batchIndex)
    {
    }
    
    /// Test if less than another lit transparency check
    bool operator < (const LitTransparencyCheck& rhs) const
    {
        if (light_ == rhs.light_)
        {
            if (drawable_ == rhs.drawable_)
                return batchIndex_ < rhs.batchIndex_;
            else
                return drawable_ < rhs.drawable_;
        }
        else
            return light_ < rhs.light_;
    }
    
    /// Test if greater than another lit transparency check
    bool operator > (const LitTransparencyCheck& rhs) const
    {
        if (light_ == rhs.light_)
        {
            if (drawable_ == rhs.drawable_)
                return batchIndex_ > rhs.batchIndex_;
            else
                return drawable_ > rhs.drawable_;
        }
        else
            return light_ > rhs.light_;
    }
    
    Light* light_;
    Drawable* drawable_;
    unsigned batchIndex_;
};

/// A rendering view. Includes the main view(s) and any auxiliary views, but not shadow cameras
class View : public Object
{
    OBJECT(View);
    
public:
    /// Construct
    View(Context* context);
    /// Destruct
    virtual ~View();
    
    /// Define with rendertarget and viewport. Return true if successful
    bool Define(RenderSurface* renderTarget, const Viewport& viewport);
    /// Update culling and construct rendering batches
    void Update(const FrameUpdate& frame);
    /// Render batches
    void Render();
    
    /// Return octree
    Octree* GetOctree() const { return octree_; }
    /// Return camera
    Camera* GetCamera() const { return camera_; }
    /// Return zone
    Zone* GetZone() const { return zone_; }
    /// Return the render target. 0 if using the backbuffer
    RenderSurface* GetRenderTarget() const { return renderTarget_; }
    /// Return the depth stencil. 0 if using the backbuffer's depth stencil
    RenderSurface* GetDepthStencil() const { return depthStencil_; }
    /// Return geometry objects
    const std::vector<Drawable*>& GetGeometries() const { return geometries_; }
    /// Return occluder objects
    const std::vector<Drawable*>& GetOccluders() const { return occluders_; }
    /// Return directional light shadow rendering occluders
    const std::vector<Drawable*>& GetShadowOccluders() const { return shadowOccluders_; }
    /// Return lights
    const std::vector<Light*>& GetLights() const { return lights_; }
    /// Return light batch queues
    const std::vector<LightBatchQueue>& GetLightQueues() const { return lightQueues_; }
    
private:
    /// Query the octree for scene nodes
    void GetDrawables();
    /// Construct batches from the scene nodes
    void GetBatches();
    /// Get lit batches for a certain light and drawable
    void GetLitBatches(Drawable* drawable, Light* light, Light* SplitLight, LightBatchQueue* lightQueue, std::set<LitTransparencyCheck>& litTransparencies, PassType gBufferPass);
    /// Render batches, forward mode
    void RenderBatcheforward();
    /// Render batches, deferred mode
    void RenderBatchesDeferred();
    /// Query for occluders as seen from a camera
    void UpdateOccluders(std::vector<Drawable*>& occluders, Camera* camera);
    /// Draw occluders to occlusion buffer
    void DrawOccluders(OcclusionBuffer* buffer, const std::vector<Drawable*>& occluders);
    /// Query for lit geometries and shadow casters for a light
    unsigned ProcessLight(Light* light);
    /// Generate combined bounding boxes for lit geometries and shadow casters and check shadow caster visibility
    void ProcessLightQuery(unsigned splitIndex, const std::vector<Drawable*>& result, BoundingBox& geometryBox, BoundingBox& shadowSpaceBox, bool getLitGeometries, bool GetShadowCasters);
    /// Check visibility of one shadow caster
    bool IsShadowCasterVisible(Drawable* drawable, BoundingBox lightViewBox, Camera* shadowCamera, const Matrix4x3& lightView, const Frustum& lightViewFrustum, const BoundingBox& lightViewFrustumBox);
    /// Set up initial shadow camera view
    void SetupShadowCamera(Light* light, bool shadowOcclusion = false);
    /// Focus shadow camera to use shadow map texture space more optimally
    void FocusShadowCamera(Light* light, const BoundingBox& geometryBox, const BoundingBox& shadowCasterBox);
    /// Quantize the directional light shadow camera view to eliminate artefacts
    void QuantizeDirShadowCamera(Light* light, const BoundingBox& viewBox);
    /// Optimize light rendering by setting up a scissor rectangle
    void OptimizeLightByScissor(Light* light);
    /// Return scissor rectangle for a light
    const Rect& GetLightScissor(Light* light);
    /// Split directional or point light for shadow rendering
    unsigned SplitLight(Light* light);
    /// Return material technique, considering the drawable's LOD distance
    Technique* GetTechnique(Drawable* drawable, Material*& material);
    /// Check if material should render an auxiliary view (if it has a camera attached)
    void CheckMaterialForAuxView(Material* material);
    /// Sort all batches
    void SortBatches();
    /// Prepare instancing buffer by filling it with all instance transforms
    void PrepareInstancingBuffer();
    /// Set per-view shader parameters
    void SetShaderParameters();
    /// Draw a split light to stencil buffer
    void DrawSplitLightToStencil(Camera& camera, Light* light, bool clear = false);
    /// Draw everything in a batch queue, priority batches first
    void RenderBatchQueue(const BatchQueue& queue, bool useScissor = false, bool useLightBuffer = false);
    /// Draw a forward (shadowed) light batch queue
    void RenderForwardLightBatchQueue(const BatchQueue& queue, Light* forwardQueueLight);
    /// Render a shadow map
    void RenderShadowMap(const LightBatchQueue& queue);
    
    /// Graphics
    WeakPtr<Graphics> graphics_;
    /// Renderer
    WeakPtr<Renderer> renderer_;
    /// Octree to use
    Octree* octree_;
    /// Camera to use
    Camera* camera_;
    /// Zone to get global rendering settings from
    Zone* zone_;
    /// Color buffer to use
    RenderSurface* renderTarget_;
    /// Depth buffer to use
    RenderSurface* depthStencil_;
    /// Screen rectangle
    IntRect screenRect_;
    /// Render target width
    int width_;
    /// Render target height
    int height_;
    /// Rendering mode
    RenderMode mode_;
    /// Draw shadows flag
    bool drawShadows_;
    /// Shader Model 3 support flag
    bool hasSM3_;
    /// Material quality level
    int materialQuality_;
    /// Maximum number of occluder triangles
    int maxOccluderTriangles_;
    /// Jitter counter for temporal antialiasing
    int jitterCounter_;
    /// Previous view matrix for temporal antialiasing
    Matrix4x3 lastCameraView_;
    /// Information of the frame being rendered
    FrameUpdate frame_;
    /// Combined bounding box of visible geometries
    BoundingBox sceneBox_;
    /// Combined bounding box of visible geometries in view space
    BoundingBox sceneViewBox_;
    /// Cache for light scissor queries
    std::map<Light*, Rect> lightScissorCache_;
    /// Current split lights being processed
    Light* splitLights_[MAX_LIGHT_SPLITS];
    /// Current lit geometries being processed
    std::vector<Drawable*> litGeometries_[MAX_LIGHT_SPLITS];
    /// Current shadow casters being processed
    std::vector<Drawable*> shadowCasters_[MAX_LIGHT_SPLITS];
    /// Temporary drawable query result
    std::vector<Drawable*> tempDrawables_;
    /// Geometry objects
    std::vector<Drawable*> geometries_;
    /// Occluder objects
    std::vector<Drawable*> occluders_;
    /// Directional light shadow rendering occluders
    std::vector<Drawable*> shadowOccluders_;
    /// Depth minimum and maximum values for visible geometries
    std::vector<GeometryDepthBounds> geometryDepthBounds_;
    /// Lights
    std::vector<Light*> lights_;
    /// G-buffer size error displayed
    std::set<RenderSurface*> gBufferErrorDisplayed_;
    
    /// G-buffer batches
    BatchQueue gBufferQueue_;
    /// Base pass batches
    BatchQueue baseQueue_;
    /// Extra pass batches
    BatchQueue extraQueue_;
    /// Transparent geometry batches
    BatchQueue transparentQueue_;
    /// Unshadowed light volume batches
    BatchQueue noShadowLightQueue_;
    /// Shadowed light queues
    std::vector<LightBatchQueue> lightQueues_;
};