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

#include "Attribute.h"
#include "Object.h"

class Deserializer;
class Serializer;
class XMLElement;

/// Base class for objects with automatic serialization through attributes
class Serializable : public Object
{
    OBJECT(Serializable);
    
public:
    /// Construct
    Serializable(Context* context);
    /// Destruct
    virtual ~Serializable();
    
    /// Handle attribute write access. Default implementation writes to the variable at offset
    virtual void OnSetAttribute(const AttributeInfo& attr, const Variant& value);
    /// Handle attribute read access. Default implementation reads the variable at offset
    virtual Variant OnGetAttribute(const AttributeInfo& attr);
    /// Load from binary data. Return true if successful
    virtual bool Load(Deserializer& source);
    /// Save as binary data. Return true if successful
    virtual bool Save(Serializer& dest);
    /// Load from XML data. Return true if successful
    virtual bool LoadXML(const XMLElement& source);
    /// Save as XML data. Return true if successful
    virtual bool SaveXML(XMLElement& dest);
    /// Perform post-load after the whole scene has been loaded
    virtual void PostLoad() {}
    
    /// Set attribute by index
    void SetAttribute(unsigned index, const Variant& value);
    /// Set attribute by name
    void SetAttribute(const char* name, const Variant& value);
    
    /// Return attribute value by index. Return empty if illegal index
    Variant GetAttribute(unsigned index);
    /// Return attribute value by name. Return empty if not found
    Variant GetAttribute(const char* name);
    /// Return number of attributes
    unsigned GetNumAttributes() const;
    /// Return attribute descriptions, or null if none defined
    const std::vector<AttributeInfo>* GetAttributes() const;
    
protected:
    /// In serialization -flag
    bool inSerialization_;
    /// In network replication -flag
    bool inNetwork_;
};