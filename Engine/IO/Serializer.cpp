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
#include "BoundingBox.h"
#include "Serializer.h"
#include "Variant.h"

#include "DebugNew.h"

static const float q = 32767.0f;

Serializer::~Serializer()
{
}

bool Serializer::WriteInt(int value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteShort(short value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteByte(signed char value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteUInt(unsigned value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteUShort(unsigned short value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteUByte(unsigned char value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteBool(bool value)
{
    return WriteUByte(value ? 1 : 0) == 1;
}

bool Serializer::WriteFloat(float value)
{
    return Write(&value, sizeof value) == sizeof value;
}

bool Serializer::WriteIntRect(const IntRect& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WriteIntVector2(const IntVector2& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WriteRect(const Rect& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WriteVector2(const Vector2& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WriteVector3(const Vector3& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WritePackedVector3(const Vector3& value, float maxAbsCoord)
{
    short coords[3];
    float v = 32767.0f / maxAbsCoord;
    
    coords[0] = (short)(Clamp(value.x_, -maxAbsCoord, maxAbsCoord) * v + 0.5f);
    coords[1] = (short)(Clamp(value.y_, -maxAbsCoord, maxAbsCoord) * v + 0.5f);
    coords[2] = (short)(Clamp(value.z_, -maxAbsCoord, maxAbsCoord) * v + 0.5f);
    
    return Write(&coords[0], sizeof coords) == sizeof coords;
}

bool Serializer::WriteVector4(const Vector4& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WriteQuaternion(const Quaternion& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WritePackedQuaternion(const Quaternion& value)
{
    short coords[4];
    Quaternion norm = value.GetNormalized();

    coords[0] = (short)(Clamp(norm.w_, -1.0f, 1.0f) * q + 0.5f);
    coords[1] = (short)(Clamp(norm.x_, -1.0f, 1.0f) * q + 0.5f);
    coords[2] = (short)(Clamp(norm.y_, -1.0f, 1.0f) * q + 0.5f);
    coords[3] = (short)(Clamp(norm.z_, -1.0f, 1.0f) * q + 0.5f);
    return Write(&coords[0], sizeof coords) == sizeof value;
}

bool Serializer::WriteColor(const Color& value)
{
    return Write(value.GetData(), sizeof value) == sizeof value;
}

bool Serializer::WriteBoundingBox(const BoundingBox& value)
{
    bool success = true;
    
    success &= WriteVector3(value.min_);
    success &= WriteVector3(value.max_);
    return success;
}

bool Serializer::WriteString(const std::string& value)
{
    return Write(value.c_str(), value.length() + 1) == value.length() + 1;
}

bool Serializer::WriteID(const std::string& value)
{
    bool success = true;
    unsigned length = Min((int)value.length(), 4);
    
    success &= Write(value.c_str(), length) == length;
    for (unsigned i = value.length(); i < 4; ++i)
        success &= WriteByte(' ');
    return success;
}

bool Serializer::WriteStringHash(const StringHash& value)
{
    return WriteUInt(value.GetValue());
}

bool Serializer::WriteShortStringHash(const ShortStringHash& value)
{
    return WriteUShort(value.GetValue());
}

bool Serializer::WriteBuffer(const std::vector<unsigned char>& value)
{
    bool success = true;
    unsigned size = value.size();
    
    success &= WriteVLE(size);
    if (size)
        success &= Write(&value[0], size) == size;
    return success;
}

bool Serializer::WriteResourceRef(const ResourceRef& value)
{
    bool success = true;
    
    success &= WriteShortStringHash(value.type_);
    success &= WriteStringHash(value.id_);
    return success;
}

bool Serializer::WriteResourceRefList(const ResourceRefList& value)
{
    bool success = true;
    unsigned size = value.ids_.size() * sizeof(StringHash);
    
    success &= WriteShortStringHash(value.type_);
    success &= WriteVLE(value.ids_.size());
    if (size)
        success &= Write(&value.ids_[0], size) == size;
    return success;
}

bool Serializer::WriteVariant(const Variant& value)
{
    bool success = true;
    
    VariantType type = value.GetType();
    success &= WriteUByte((unsigned char)type);
    success &= WriteVariantData(value);
    return success;
}

bool Serializer::WriteVariantData(const Variant& value)
{
    switch (value.GetType())
    {
    case VAR_NONE:
        return true;
        
    case VAR_INT:
        return WriteInt(value.GetInt());
        
    case VAR_BOOL:
        return WriteBool(value.GetBool());
        
    case VAR_FLOAT:
        return WriteFloat(value.GetFloat());
        
    case VAR_VECTOR2:
        return WriteVector2(value.GetVector2());
        
    case VAR_VECTOR3:
        return WriteVector3(value.GetVector3());
        
    case VAR_VECTOR4:
        return WriteVector4(value.GetVector4());
        
    case VAR_QUATERNION:
        return WriteQuaternion(value.GetQuaternion());
        
    case VAR_COLOR:
        return WriteColor(value.GetColor());
        
    case VAR_STRING:
        return WriteString(value.GetString());
        
    case VAR_BUFFER:
        return WriteBuffer(value.GetBuffer());
        
    case VAR_PTR:
        return WriteUInt(0);
        
    case VAR_RESOURCEREF:
        return WriteResourceRef(value.GetResourceRef());
        
    case VAR_RESOURCEREFLIST:
        return WriteResourceRefList(value.GetResourceRefList());
        
    case VAR_VARIANTVECTOR:
        return WriteVariantVector(value.GetVariantVector());
        
    case VAR_VARIANTMAP:
        return WriteVariantMap(value.GetVariantMap());
    }
    
    return false;
}

bool Serializer::WriteVariantVector(const VariantVector& value)
{
    bool success = true;
    
    success &= WriteVLE(value.size());
    for (VariantVector::const_iterator i = value.begin(); i != value.end(); ++i)
        success &= WriteVariant(*i);
    return success;
}

bool Serializer::WriteVariantMap(const VariantMap& value)
{
    bool success = true;
    
    success &= WriteVLE(value.size());
    for (VariantMap::const_iterator i = value.begin(); i != value.end(); ++i)
    {
        WriteShortStringHash(i->first);
        WriteVariant(i->second);
    }
    return success;
}

bool Serializer::WriteVLE(unsigned value)
{
    unsigned char data[4];
    
    if (value < 0x80)
        return WriteUByte(value);
    else if (value < 0x4000)
    {
        data[0] = value | 0x80;
        data[1] = value >> 7;
        return Write(data, 2) == 2;
    }
    else if (value < 0x200000)
    {
        data[0] = value | 0x80;
        data[1] = (value >> 7) | 0x80;
        data[2] = value >> 14;
        return Write(data, 3) == 3;
    }
    else
    {
        data[0] = value | 0x80;
        data[1] = (value >> 7) | 0x80;
        data[2] = (value >> 14) | 0x80;
        data[3] = (value >> 21);
        return Write(data, 4) == 4;
    }
}

bool Serializer::WriteLine(const std::string& value)
{
    bool success = true;
    
    success &= Write(value.c_str(), value.length()) == value.length();
    success &= WriteUByte(13);
    success &= WriteUByte(10);
    return success;
}