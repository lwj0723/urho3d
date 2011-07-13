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
#include "Deserializer.h"
#include "FileSystem.h"
#include "Graphics.h"
#include "GraphicsImpl.h"
#include "Log.h"
#include "Profiler.h"
#include "Shader.h"
#include "ShaderVariation.h"

/// Shader constant (uniform or sampler) structure for loading
struct Constant
{
    /// Parameter name
    String name_;
    /// Constant register or sampler
    unsigned register_;
};

OBJECTTYPESTATIC(Shader);

Shader::Shader(Context* context) :
    Resource(context),
    shaderType_(VS),
    isSM3_(false)
{
}

Shader::~Shader()
{
}

void Shader::RegisterObject(Context* context)
{
    context->RegisterFactory<Shader>();
}

bool Shader::Load(Deserializer& source)
{
    PROFILE(LoadShader);
    
    // Clear existing variations
    variations_.Clear();
    
    Graphics* graphics = GetSubsystem<Graphics>();
    if (!graphics)
        return false;
    
    // Check ID
    if (source.ReadID() != "USHD")
    {
        LOGERROR(source.GetName() + " is not a valid shader file");
        return false;
    }
    
    String fileName = GetFileName(source.GetName());
    shaderType_ = (ShaderType)source.ReadShort();
    isSM3_ = (source.ReadShort() == 3);
    
    Vector<Constant> parameters;
    Vector<Constant> textureUnits;
    
    // Read the parameters and texture units used by this shader; use information is specified in terms of them
    unsigned numParameters = source.ReadUInt();
    for (unsigned i = 0; i < numParameters; ++i)
    {
        Constant newParameter;
        newParameter.name_ = source.ReadString();
        newParameter.register_ = source.ReadUByte();
        parameters.Push(newParameter);
        
        graphics->DefineShaderParameter(StringHash(newParameter.name_), shaderType_, newParameter.register_);
    }
    
    unsigned numTextureUnits = source.ReadUInt();
    for (unsigned i = 0; i < numTextureUnits; ++i)
    {
        Constant newTextureUnit;
        newTextureUnit.name_ = source.ReadString();
        newTextureUnit.register_ = source.ReadUByte();
        textureUnits.Push(newTextureUnit);
        
        TextureUnit tuIndex = graphics->GetTextureUnit(newTextureUnit.name_);
        if (tuIndex == MAX_TEXTURE_UNITS)
            LOGWARNING("Unknown texture unit " + newTextureUnit.name_);
    }
    
    // Read the variations
    unsigned numVariations = source.ReadUInt();
    for (unsigned i = 0; i < numVariations; ++i)
    {
        SharedPtr<ShaderVariation> variation(new ShaderVariation(this, shaderType_, isSM3_));
        String variationName = source.ReadString();
        StringHash nameHash(variationName);
        if (!variationName.Empty())
            variation->SetName(fileName + "_" + variationName);
        else
            variation->SetName(fileName);
        
        // Fill the parameter & texture unit use information
        for (unsigned j = 0; j < numParameters; ++j)
        {
            if (source.ReadBool())
                variation->SetUseParameter(StringHash(parameters[j].name_), true);
        }
        
        for (unsigned j = 0; j < numTextureUnits; ++j)
        {
            if (source.ReadBool())
            {
                TextureUnit tuIndex = graphics->GetTextureUnit(textureUnits[j].name_);
                if (tuIndex != MAX_TEXTURE_UNITS)
                    variation->SetUseTextureUnit(tuIndex, true);
            }
        }
        
        // Read the bytecode
        unsigned dataSize = source.ReadUInt();
        if (dataSize)
        {
            SharedArrayPtr<unsigned char> byteCode(new unsigned char[dataSize]);
            source.Read(byteCode.RawPtr(), dataSize);
            variation->SetByteCode(byteCode);
        }
        
        // Store the variation
        variations_[nameHash] = variation;
    }
    
    // This is not exactly accurate, but a reasonable estimate
    SetMemoryUse(source.GetSize());
    return true;
}

ShaderVariation* Shader::GetVariation(const String& name)
{
    return GetVariation(StringHash(name));
}

ShaderVariation* Shader::GetVariation(StringHash nameHash)
{
    Map<StringHash, SharedPtr<ShaderVariation> >::Iterator i = variations_.Find(nameHash);
    if (i != variations_.End())
        return i->second_;
    else
        return 0;
}
