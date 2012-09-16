//
// Urho3D Engine
// Copyright (c) 2008-2012 Lasse Oorni
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
#include "HashSet.h"
#include "List.h"
#include "Object.h"
#include "Polyhedron.h"

namespace Urho3D
{

class Camera;
class DebugRenderer;
class Light;
class Drawable;
class OcclusionBuffer;
class Octree;
class RenderSurface;
class Technique;
class Texture2D;
class Viewport;
class Zone;
struct WorkItem;

/// Intermediate light processing result.
struct LightQueryResult
{
    /// Light.
    Light* light_;
    /// Lit geometries.
    PODVector<Drawable*> litGeometries_;
    /// Shadow casters.
    PODVector<Drawable*> shadowCasters_;
    /// Shadow cameras.
    Camera* shadowCameras_[MAX_LIGHT_SPLITS];
    /// Shadow caster start indices.
    unsigned shadowCasterBegin_[MAX_LIGHT_SPLITS];
    /// Shadow caster end indices.
    unsigned shadowCasterEnd_[MAX_LIGHT_SPLITS];
    /// Combined bounding box of shadow casters in light view or projection space.
    BoundingBox shadowCasterBox_[MAX_LIGHT_SPLITS];
    /// Shadow camera near splits (directional lights only.)
    float shadowNearSplits_[MAX_LIGHT_SPLITS];
    /// Shadow camera far splits (directional lights only.)
    float shadowFarSplits_[MAX_LIGHT_SPLITS];
    /// Shadow map split count.
    unsigned numSplits_;
};

/// 3D rendering view. Includes the main view(s) and any auxiliary views, but not shadow cameras.
class View : public Object
{
    friend void CheckVisibilityWork(const WorkItem* item, unsigned threadIndex);
    friend void ProcessLightWork(const WorkItem* item, unsigned threadIndex);
    
    OBJECT(View);
    
public:
    /// Construct.
    View(Context* context);
    /// Destruct.
    virtual ~View();
    
    /// Define with rendertarget and viewport. Return true if successful.
    bool Define(RenderSurface* renderTarget, Viewport* viewport);
    /// Update and cull objects and construct rendering batches.
    void Update(const FrameInfo& frame);
    /// Render batches.
    void Render();
    
    /// Return octree.
    Octree* GetOctree() const { return octree_; }
    /// Return camera.
    Camera* GetCamera() const { return camera_; }
    /// Return the rendertarget. 0 if using the backbuffer.
    RenderSurface* GetRenderTarget() const { return renderTarget_; }
    /// Return geometry objects.
    const PODVector<Drawable*>& GetGeometries() const { return geometries_; }
    /// Return occluder objects.
    const PODVector<Drawable*>& GetOccluders() const { return occluders_; }
    /// Return lights.
    const PODVector<Light*>& GetLights() const { return lights_; }
    /// Return light batch queues.
    const Vector<LightBatchQueue>& GetLightQueues() const { return lightQueues_; }
    
private:
    /// Query the octree for drawable objects.
    void GetDrawables();
    /// Construct batches from the drawable objects.
    void GetBatches();
    /// Update geometries and sort batches.
    void UpdateGeometries();
    /// Get pixel lit batches for a certain light and drawable.
    void GetLitBatches(Drawable* drawable, LightBatchQueue& lightQueue);
    /// Render batches using forward rendering.
    void RenderBatchesForward();
    /// Render batches using light pre-pass or deferred rendering.
    void RenderBatchesDeferred();
    /// Allocate needed screen buffers for post-processing and/or framebuffer blitting.
    void AllocateScreenBuffers();
    /// Blit the framebuffer to destination. Used in OpenGL deferred rendering modes.
    void BlitFramebuffer();
    /// Run post-processing effects.
    void RunPostProcesses();
    /// Query for occluders as seen from a camera.
    void UpdateOccluders(PODVector<Drawable*>& occluders, Camera* camera);
    /// Draw occluders to occlusion buffer.
    void DrawOccluders(OcclusionBuffer* buffer, const PODVector<Drawable*>& occluders);
    /// Query for lit geometries and shadow casters for a light.
    void ProcessLight(LightQueryResult& query, unsigned threadIndex);
    /// Process shadow casters' visibilities and build their combined view- or projection-space bounding box.
    void ProcessShadowCasters(LightQueryResult& query, const PODVector<Drawable*>& drawables, unsigned splitIndex);
    /// Set up initial shadow camera view(s).
    void SetupShadowCameras(LightQueryResult& query);
    /// Set up a directional light shadow camera
    void SetupDirLightShadowCamera(Camera* shadowCamera, Light* light, float nearSplit, float farSplit);
    /// Finalize shadow camera view after shadow casters and the shadow map are known.
    void FinalizeShadowCamera(Camera* shadowCamera, Light* light, const IntRect& shadowViewport, const BoundingBox& shadowCasterBox);
    /// Quantize a directional light shadow camera view to eliminate swimming.
    void QuantizeDirLightShadowCamera(Camera* shadowCamera, Light* light, const IntRect& shadowViewport, const BoundingBox& viewBox);
    /// Check visibility of one shadow caster.
    bool IsShadowCasterVisible(Drawable* drawable, BoundingBox lightViewBox, Camera* shadowCamera, const Matrix3x4& lightView, const Frustum& lightViewFrustum, const BoundingBox& lightViewFrustumBox);
    /// Return the viewport for a shadow map split.
    IntRect GetShadowMapViewport(Light* light, unsigned splitIndex, Texture2D* shadowMap);
    /// Optimize light rendering by setting up a scissor rectangle.
    void OptimizeLightByScissor(Light* light);
    /// Optimize spot or point light rendering by drawing its volume to the stencil buffer.
    void OptimizeLightByStencil(Light* light);
    /// Return scissor rectangle for a light.
    const Rect& GetLightScissor(Light* light);
    /// Split directional or point light for shadow rendering.
    unsigned SplitLight(Light* light);
    /// Find and set a new zone for a drawable when it has moved.
    void FindZone(Drawable* drawable);
    /// Return the drawable's zone, or camera zone if it has override mode enabled.
    Zone* GetZone(Drawable* drawable);
    /// Return the drawable's light mask, considering also its zone.
    unsigned GetLightMask(Drawable* drawable);
    /// Return the drawable's shadow mask, considering also its zone.
    unsigned GetShadowMask(Drawable* drawable);
    /// Return hash code for a vertex light queue.
    unsigned long long GetVertexLightQueueHash(const PODVector<Light*>& vertexLights);
    /// Return material technique, considering the drawable's LOD distance.
    Technique* GetTechnique(Drawable* drawable, Material* material);
    /// Check if material should render an auxiliary view (if it has a camera attached.)
    void CheckMaterialForAuxView(Material* material);
    /// Choose shaders for a batch and add it to queue.
    void AddBatchToQueue(BatchQueue& queue, Batch& batch, Technique* tech, bool allowInstancing = true, bool allowShadows = true);
    /// Prepare instancing buffer by filling it with all instance transforms.
    void PrepareInstancingBuffer();
    /// Set up a light volume rendering batch.
    void SetupLightVolumeBatch(Batch& batch);
    /// Draw a full screen quad (either near or far.) Shaders must have been set beforehand.
    void DrawFullscreenQuad(Camera* camera, bool nearQuad);
    /// Render a shadow map.
    void RenderShadowMap(const LightBatchQueue& queue);
    /// Return the proper depth-stencil surface to use for a rendertarget.
    RenderSurface* GetDepthStencil(RenderSurface* renderTarget);
    
    /// Graphics subsystem.
    WeakPtr<Graphics> graphics_;
    /// Renderer subsystem.
    WeakPtr<Renderer> renderer_;
    /// Octree to use.
    Octree* octree_;
    /// Camera to use.
    Camera* camera_;
    /// Camera's scene node.
    Node* cameraNode_;
    /// Zone the camera is inside, or default zone if not assigned.
    Zone* cameraZone_;
    /// Zone at far clip plane.
    Zone* farClipZone_;
    /// Occlusion buffer for the main camera.
    OcclusionBuffer* occlusionBuffer_;
    /// Color rendertarget to use.
    RenderSurface* renderTarget_;
    /// Viewport rectangle.
    IntRect viewRect_;
    /// Viewport size.
    IntVector2 viewSize_;
    /// Rendertarget size.
    IntVector2 rtSize_;
    /// Information of the frame being rendered.
    FrameInfo frame_;
    /// Combined bounding box of visible geometries.
    BoundingBox sceneBox_;
    /// Minimum Z value of the visible scene.
    float minZ_;
    /// Maximum Z value of the visible scene.
    float maxZ_;
    /// Rendering mode.
    RenderMode renderMode_;
    /// Material quality level.
    int materialQuality_;
    /// Maximum number of occluder triangles.
    int maxOccluderTriangles_;
    /// Highest zone priority currently visible.
    int highestZonePriority_;
    /// Camera zone's override flag.
    bool cameraZoneOverride_;
    /// Draw shadows flag.
    bool drawShadows_;
    /// Whether objects with zero lightmask exist. In light pre-pass mode this means skipping some optimizations.
    bool hasZeroLightMask_;
    /// Post-processing effects.
    Vector<SharedPtr<PostProcess> > postProcesses_;
    /// Intermediate screen buffers used in postprocessing and OpenGL deferred framebuffer blit.
    PODVector<Texture2D*> screenBuffers_;
    /// Per-thread octree query results.
    Vector<PODVector<Drawable*> > tempDrawables_;
    /// Visible zones.
    PODVector<Zone*> zones_;
    /// Visible geometry objects.
    PODVector<Drawable*> geometries_;
    /// Geometry objects visible in shadow maps.
    PODVector<Drawable*> shadowGeometries_;
    /// Geometry objects that will be updated in the main thread.
    PODVector<Drawable*> nonThreadedGeometries_;
    /// Geometry objects that will be updated in worker threads.
    PODVector<Drawable*> threadedGeometries_;
    /// Occluder objects.
    PODVector<Drawable*> occluders_;
    /// Lights.
    PODVector<Light*> lights_;
    /// Drawables that limit their maximum light count.
    HashSet<Drawable*> maxLightsDrawables_;
    /// Intermediate light processing results.
    Vector<LightQueryResult> lightQueryResults_;
    /// Per-pixel light queues.
    Vector<LightBatchQueue> lightQueues_;
    /// Per-vertex light queues.
    HashMap<unsigned long long, LightBatchQueue> vertexLightQueues_;
    /// Base pass batches.
    BatchQueue baseQueue_;
    /// Deferred rendering G-buffer batches.
    BatchQueue gbufferQueue_;
    /// Pre-transparent pass batches.
    BatchQueue preAlphaQueue_;
    /// Transparent geometry batches.
    BatchQueue alphaQueue_;
    /// Post-transparent pass batches.
    BatchQueue postAlphaQueue_;
};

}
