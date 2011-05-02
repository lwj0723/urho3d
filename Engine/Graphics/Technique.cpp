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

#include "Precompiled.h"
#include "Context.h"
#include "Log.h"
#include "Technique.h"
#include "PixelShader.h"
#include "Profiler.h"
#include "ResourceCache.h"
#include "StringUtils.h"
#include "VertexShader.h"
#include "XMLFile.h"

static const std::string passNames[] =
{
    "deferred",
    "prepass",
    "material",
    "base",
    "litbase",
    "light",
    "extra",
    "shadow"
};

static const std::string blendModeNames[] =
{
    "replace",
    "add",
    "multiply",
    "alpha",
    "addalpha",
    "premulalpha",
    "invdestalpha"
};

static const std::string CompareModeNames[] =
{
    "always",
    "equal",
    "notequal",
    "less",
    "lessequal",
    "greater",
    "greaterequal"
};

Pass::Pass()
{
}

Pass::Pass(PassType type) :
    type_(type),
    alphaMask_(false),
    alphaTest_(false),
    blendMode_(BLEND_REPLACE),
    depthTestMode_(CMP_LESSEQUAL),
    depthWrite_(true)
{
}

Pass::~Pass()
{
}

void Pass::SetAlphaMask(bool enable)
{
    alphaMask_ = enable;
}

void Pass::SetAlphaTest(bool enable)
{
    alphaTest_ = enable;
}

void Pass::SetBlendMode(BlendMode mode)
{
    blendMode_ = mode;
}

void Pass::SetDepthTestMode(CompareMode mode)
{
    depthTestMode_ = mode;
}

void Pass::SetDepthWrite(bool enable)
{
    depthWrite_ = enable;
}

void Pass::SetVertexShader(const std::string& name)
{
    vertexShaderName_ = name;
    ReleaseShaders();
}

void Pass::SetPixelShader(const std::string& name)
{
    pixelShaderName_ = name;
    ReleaseShaders();
}

void Pass::ReleaseShaders()
{
    vertexShaders_.clear();
    pixelShaders_.clear();
}

OBJECTTYPESTATIC(Technique);

Technique::Technique(Context* context) :
    Resource(context),
    isSM3_(false),
    shadersLoadedFrameNumber_(0)
{
}

Technique::~Technique()
{
}

void Technique::RegisterObject(Context* context)
{
    context->RegisterFactory<Technique>();
}

bool Technique::Load(Deserializer& source)
{
    PROFILE(LoadTechnique);
    
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    if (!xml->Load(source))
        return false;
    
    XMLElement rootElem = xml->GetRootElement();
    if (rootElem.HasAttribute("sm3"))
        isSM3_ = rootElem.GetBool("sm3");
    
    XMLElement passElem = rootElem.GetChildElement("pass");
    while (passElem)
    {
        PassType type = MAX_PASSES;
        if (passElem.HasAttribute("name"))
        {
            std::string name = passElem.GetStringLower("name");
            type = (PassType)GetStringListIndex(name, passNames, MAX_PASSES, MAX_PASSES);
            if (type == MAX_PASSES)
                LOGERROR("Unknown pass " + name);
        }
        else
            LOGERROR("Missing pass name");
        
        if (type != MAX_PASSES)
        {
            Pass& newPass = *CreatePass(type);
            
            if (passElem.HasAttribute("vs"))
                newPass.SetVertexShader(passElem.GetString("vs"));
            
            if (passElem.HasAttribute("ps"))
                newPass.SetPixelShader(passElem.GetString("ps"));
            
            if (passElem.HasAttribute("alphamask"))
                newPass.SetAlphaMask(passElem.GetBool("alphamask"));
            
            if (passElem.HasAttribute("alphatest"))
                newPass.SetAlphaTest(passElem.GetBool("alphatest"));
            
            if (passElem.HasAttribute("blend"))
            {
                std::string blend = passElem.GetStringLower("blend");
                newPass.SetBlendMode((BlendMode)GetStringListIndex(blend, blendModeNames, MAX_BLENDMODES, BLEND_REPLACE));
            }
            
            if (passElem.HasAttribute("depthtest"))
            {
                std::string depthTest = passElem.GetStringLower("depthtest");
                if (depthTest == "false")
                    newPass.SetDepthTestMode(CMP_ALWAYS);
                else
                    newPass.SetDepthTestMode((CompareMode)GetStringListIndex(depthTest, CompareModeNames, MAX_COMPAREMODES,
                        CMP_LESSEQUAL));
            }
            
            if (passElem.HasAttribute("depthwrite"))
                newPass.SetDepthWrite(passElem.GetBool("depthwrite"));
        }
        
        passElem = passElem.GetNextElement("pass");
    }
    
    // Calculate memory use
    unsigned memoryUse = 0;
    memoryUse += sizeof(Technique);
    for (std::map<PassType, Pass>::const_iterator j = passes_.begin(); j != passes_.end(); ++j)
        memoryUse += sizeof(Pass);
    
    SetMemoryUse(memoryUse);
    return true;
}

void Technique::SetIsSM3(bool enable)
{
    isSM3_ = enable;
}

void Technique::ReleaseShaders()
{
    for (std::map<PassType, Pass>::iterator i = passes_.begin(); i != passes_.end(); ++i)
        i->second.ReleaseShaders();
}

Pass* Technique::CreatePass(PassType pass)
{
    Pass* existing = GetPass(pass);
    if (existing)
        return existing;
    
    Pass newPass(pass);
    passes_[pass] = newPass;
    
    return GetPass(pass);
}

void Technique::RemovePass(PassType pass)
{
    passes_.erase(pass);
}

void Technique::MarkShadersLoaded(unsigned frameNumber)
{
    shadersLoadedFrameNumber_ = frameNumber;
}

const std::string& Technique::GetPassName(PassType pass)
{
    return passNames[pass];
}