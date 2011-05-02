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

#include "Color.h"
#include "Quaternion.h"
#include "StringHash.h"
#include "Vector4.h"

#include <map>
#include <vector>

/// Supported variable types for Variant
enum VariantType
{
    VAR_NONE = 0,
    VAR_INT,
    VAR_BOOL,
    VAR_FLOAT,
    VAR_VECTOR2,
    VAR_VECTOR3,
    VAR_VECTOR4,
    VAR_QUATERNION,
    VAR_COLOR,
    VAR_STRING,
    VAR_BUFFER,
    VAR_PTR,
    VAR_RESOURCEREF,
    VAR_RESOURCEREFLIST,
    VAR_VARIANTVECTOR,
    VAR_VARIANTMAP
};

/// Union for holding all the possible Variant values
struct VariantValue
{
    union
    {
        int int_;
        bool bool_;
        float float_;
        void* ptr_;
    };
    
    float y_;
    float z_;
    float w_;
};

/// Typed resource reference
struct ResourceRef
{
    /// Construct
    ResourceRef()
    {
    }
    
    /// Construct with type only and empty id
    ResourceRef(ShortStringHash type) :
        type_(type)
    {
    }
    
    /// Construct with type and id
    ResourceRef(ShortStringHash type, StringHash id) :
        type_(type),
        id_(id)
    {
    }
    
    /// Object type
    ShortStringHash type_;
    /// Object identifier, for example name hash
    StringHash id_;
    
    /// Test for equality with another reference
    bool operator == (const ResourceRef& rhs) const
    {
        return (type_ == rhs.type_) && (id_ == rhs.id_);
    }
    
    /// Test for inequality with another reference
    bool operator != (const ResourceRef& rhs) const
    {
        return (type_ != rhs.type_) || (id_ != rhs.id_);
    }
};

/// List of resource references of a specific type
struct ResourceRefList
{
    /// Construct
    ResourceRefList()
    {
    }
    
    /// Construct with type only
    ResourceRefList(ShortStringHash type) :
        type_(type)
    {
    }
    /// Construct with type and id list
    ResourceRefList(ShortStringHash type, const std::vector<StringHash>& ids) :
        type_(type),
        ids_(ids)
    {
    }
    
    /// Object type
    ShortStringHash type_;
    /// List of object identifiers, for example name hashes
    std::vector<StringHash> ids_;
    
    /// Test for equality with another reference list
    bool operator == (const ResourceRefList& rhs) const
    {
        return (type_ == rhs.type_) && (ids_ == rhs.ids_);
    }
    
    /// Test for inequality with another reference list
    bool operator != (const ResourceRefList& rhs) const
    {
        return (type_ != rhs.type_) || (ids_ != rhs.ids_);
    }
};

class Variant;

/// Vector of variants
typedef std::vector<Variant> VariantVector;
/// Map of variants
typedef std::map<ShortStringHash, Variant> VariantMap;

/// Variable that supports a fixed set of types
class Variant
{
public:
    /// Construct with none value
    Variant() :
        type_(VAR_NONE)
    {
    }
    
    /// Construct from integer
    Variant(int value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from unsigned integer
    Variant(unsigned value) :
        type_(VAR_NONE)
    {
        *this = (int)value;
    }
    
    /// Construct from a StringHash (convert to integer)
    Variant(const StringHash& value) :
        type_(VAR_NONE)
    {
        *this = (int)value.GetValue();
    }
    
    /// Construct from a ShortStringHash (convert to integer)
    Variant(const ShortStringHash& value) :
        type_(VAR_NONE)
    {
        *this = (int)value.GetValue();
    }
    
    /// Construct from a bool
    Variant(bool value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a float
    Variant(float value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a Vector2
    Variant(const Vector2& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a Vector3
    Variant(const Vector3& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a Vector4
    Variant(const Vector4& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a Quaternion
    Variant(const Quaternion& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a Color
    Variant(const Color& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a string
    Variant(const std::string& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a C string
    Variant(const char* value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a buffer
    Variant(const std::vector<unsigned char>& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a pointer
    Variant(void* value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from an object reference
    Variant(const ResourceRef& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from an object reference list
    Variant(const ResourceRefList& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a variant vector
    Variant(const VariantVector& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from a variant map
    Variant(const VariantMap& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Construct from type and value
    Variant(const std::string& type, const std::string& value) :
        type_(VAR_NONE)
    {
        FromString(type, value);
    }
    
    /// Copy-construct from another variant
    Variant(const Variant& value) :
        type_(VAR_NONE)
    {
        *this = value;
    }
    
    /// Destruct
    ~Variant()
    {
        SetType(VAR_NONE);
    }
    
    /// Reset to none type
    void Clear()
    {
        SetType(VAR_NONE);
    }
    
    /// Assign from another variant
    Variant& operator = (const Variant& rhs);
    
    /// Assign from an integer
    Variant& operator = (int rhs)
    {
        SetType(VAR_INT);
        value_.int_ = rhs;
        return *this;
    }
    
    /// Assign from an unsigned integer
    Variant& operator = (unsigned rhs)
    {
        SetType(VAR_INT);
        value_.int_ = (int)rhs;
        return *this;
    }
    
    /// Assign from a StringHash (convert to integer)
    Variant& operator = (const StringHash& rhs)
    {
        SetType(VAR_INT);
        value_.int_ = (int)rhs.GetValue();
        return *this;
    }
    
    /// Assign from a ShortStringHash (convert to integer)
    Variant& operator = (const ShortStringHash& rhs)
    {
        SetType(VAR_INT);
        value_.int_ = (int)rhs.GetValue();
        return *this;
    }
    
    /// Assign from a bool
    Variant& operator = (bool rhs)
    {
        SetType(VAR_BOOL);
        value_.bool_ = rhs;
        return *this;
    }
    
    /// Assign from a float
    Variant& operator = (float rhs)
    {
        SetType(VAR_FLOAT);
        value_.float_ = rhs;
        return *this;
    }
    
    /// Assign from a Vector2
    Variant& operator = (const Vector2& rhs)
    {
        SetType(VAR_VECTOR2);
        *(reinterpret_cast<Vector2*>(&value_)) = rhs;
        return *this;
    }
    
    /// Assign from a Vector3
    Variant& operator = (const Vector3& rhs)
    {
        SetType(VAR_VECTOR3);
        *(reinterpret_cast<Vector3*>(&value_)) = rhs;
        return *this;
    }
    
    /// Assign from a Vector4
    Variant& operator = (const Vector4& rhs)
    {
        SetType(VAR_VECTOR4);
        *(reinterpret_cast<Vector4*>(&value_)) = rhs;
        return *this;
    }
    
    /// Assign from a Quaternion
    Variant& operator = (const Quaternion& rhs)
    {
        SetType(VAR_QUATERNION);
        *(reinterpret_cast<Quaternion*>(&value_)) = rhs;
        return *this;
    }
    
    /// Assign from a Color
    Variant& operator = (const Color& rhs)
    {
        SetType(VAR_COLOR);
        *(reinterpret_cast<Color*>(&value_)) = rhs;
        return *this;
    }
    
    /// Assign from a string
    Variant& operator = (const std::string& rhs)
    {
        SetType(VAR_STRING);
        *(reinterpret_cast<std::string*>(value_.ptr_)) = rhs;
        return *this;
    }
    
    /// Assign from a C string
    Variant& operator = (const char* rhs)
    {
        SetType(VAR_STRING);
        *(reinterpret_cast<std::string*>(value_.ptr_)) = std::string(rhs);
        return *this;
    }

    /// Assign from a buffer
    Variant& operator = (const std::vector<unsigned char>& rhs)
    {
        SetType(VAR_BUFFER);
        *(reinterpret_cast<std::vector<unsigned char>*>(value_.ptr_)) = rhs;
        return *this;
    }
    
    /// Assign from a pointer
    Variant& operator = (void* rhs)
    {
        SetType(VAR_PTR);
        value_.ptr_ = rhs;
        return *this;
    }
    
    /// Assign from an object reference
    Variant& operator = (const ResourceRef& rhs)
    {
        SetType(VAR_RESOURCEREF);
        *(reinterpret_cast<ResourceRef*>(value_.ptr_)) = rhs;
        return *this;
    }
    
    /// Assign from an object reference list
    Variant& operator = (const ResourceRefList& rhs)
    {
        SetType(VAR_RESOURCEREFLIST);
        *(reinterpret_cast<ResourceRefList*>(value_.ptr_)) = rhs;
        return *this;
    }
    
    /// Assign from a variant vector
    Variant& operator = (const VariantVector& rhs)
    {
        SetType(VAR_VARIANTVECTOR);
        *(reinterpret_cast<VariantVector*>(value_.ptr_)) = rhs;
        return *this;
    }
    
    /// Assign from a variant map
    Variant& operator = (const VariantMap& rhs)
    {
        SetType(VAR_VARIANTMAP);
        *(reinterpret_cast<VariantMap*>(value_.ptr_)) = rhs;
        return *this;
    }
    
    /// Test for equality with another variant
    bool operator == (const Variant& rhs) const;
    
    /// Test for inequality with another variant
    bool operator != (const Variant& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for equality with an integer. To return true, both the type and value must match
    bool operator == (int rhs) const
    {
        if (type_ == VAR_INT)
            return value_.int_ == rhs;
        else
            return false;
    }
    
    /// Test for equality with an unsigned integer. To return true, both the type and value must match
    bool operator == (unsigned rhs) const
    {
        if (type_ == VAR_INT)
            return value_.int_ == (int)rhs;
        else
            return false;
    }
    
    /// Test for equality with a bool. To return true, both the type and value must match
    bool operator == (bool rhs) const
    {
        if (type_ == VAR_BOOL)
            return value_.bool_ == rhs;
        else
            return false;
    }
    
    /// Test for equality with a float. To return true, both the type and value must match
    bool operator == (float rhs) const
    {
        if (type_ == VAR_FLOAT)
            return value_.float_ == rhs;
        else
            return false;
    }
    
    /// Test for equality with a Vector2. To return true, both the type and value must match
    bool operator == (const Vector2& rhs) const
    {
        if (type_ == VAR_VECTOR2)
            return *(reinterpret_cast<const Vector2*>(&value_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a Vector3. To return true, both the type and value must match
    bool operator == (const Vector3& rhs) const
    {
        if (type_ == VAR_VECTOR3)
            return *(reinterpret_cast<const Vector3*>(&value_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a Vector4. To return true, both the type and value must match
    bool operator == (const Vector4& rhs) const
    {
        if (type_ == VAR_VECTOR4)
            return *(reinterpret_cast<const Vector4*>(&value_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a Quaternion. To return true, both the type and value must match
    bool operator == (const Quaternion& rhs) const
    {
        if (type_ == VAR_QUATERNION)
            return *(reinterpret_cast<const Quaternion*>(&value_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a Color. To return true, both the type and value must match
    bool operator == (const Color& rhs) const
    {
        if (type_ == VAR_COLOR)
            return *(reinterpret_cast<const Color*>(&value_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a string. To return true, both the type and value must match
    bool operator == (const std::string& rhs) const
    {
        if (type_ == VAR_STRING)
            return *(reinterpret_cast<const std::string*>(value_.ptr_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a buffer. To return true, both the type and value must match
    bool operator == (const std::vector<unsigned char>& rhs) const
    {
        if (type_ == VAR_BUFFER)
            return *(reinterpret_cast<const std::vector<unsigned char>*>(value_.ptr_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a pointer. To return true, both the type and value must match
    bool operator == (void* rhs) const
    {
        if (type_ == VAR_PTR)
            return value_.ptr_ == rhs;
        else
            return false;
    }
    
    /// Test for equality with an object reference. To return true, both the type and value must match
    bool operator == (const ResourceRef& rhs) const
    {
        if (type_ == VAR_RESOURCEREF)
            return *(reinterpret_cast<ResourceRef*>(value_.ptr_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with an object reference list. To return true, both the type and value must match
    bool operator == (const ResourceRefList& rhs) const
    {
        if (type_ == VAR_RESOURCEREFLIST)
            return *(reinterpret_cast<ResourceRefList*>(value_.ptr_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a variant vector. To return true, both the type and value must match
    bool operator == (const VariantVector& rhs) const
    {
        if (type_ == VAR_VARIANTVECTOR)
            return *(reinterpret_cast<VariantVector*>(value_.ptr_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a variant map. To return true, both the type and value must match
    bool operator == (const VariantMap& rhs) const
    {
        if (type_ == VAR_VARIANTMAP)
            return *(reinterpret_cast<VariantMap*>(value_.ptr_)) == rhs;
        else
            return false;
    }
    
    /// Test for equality with a StringHash. To return true, both the type and value must match
    bool operator == (const StringHash& rhs) const
    {
        if (type_ == VAR_INT)
            return value_.int_ == rhs.GetValue();
        else
            return false;
    }
    
    /// Test for equality with a ShortStringHash. To return true, both the type and value must match
    bool operator == (const ShortStringHash& rhs) const
    {
        if (type_ == VAR_INT)
            return value_.int_ == rhs.GetValue();
        else
            return false;
    }
    
    /// Test for inequality with an integer
    bool operator != (int rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with an unsigned integer
    bool operator != (unsigned rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a bool
    bool operator != (bool rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a float
    bool operator != (float rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a Vector2
    bool operator != (const Vector2& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a Vector3
    bool operator != (const Vector3& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with an Vector4
    bool operator != (const Vector4& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a Quaternion
    bool operator != (const Quaternion& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a string
    bool operator != (const std::string& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a buffer
    bool operator != (const std::vector<unsigned char>& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a pointer
    bool operator != (void* rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with an object reference
    bool operator != (const ResourceRef& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with an object reference list
    bool operator != (const ResourceRefList& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a variant vector
    bool operator != (const VariantVector& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a variant map
    bool operator != (const VariantMap& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a StringHash
    bool operator != (const StringHash& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Test for inequality with a ShortStringHash
    bool operator != (const ShortStringHash& rhs) const
    {
        return !(*this == rhs);
    }
    
    /// Return an integer, 0 if type mismatch
    int GetInt() const
    {
        if (type_ != VAR_INT)
            return 0;
        return value_.int_;
    }
    
    /// Return an unsigned integer, 0 if type mismatch
    unsigned GetUInt() const
    {
        if (type_ != VAR_INT)
            return 0;
        return (unsigned)value_.int_;
    }
    
    /// Return a StringHash, zero hash if type mismatch
    StringHash GetStringHash() const
    {
        if (type_ != VAR_INT)
            return StringHash::ZERO;
        return StringHash(value_.int_);
    }
    
    /// Return a ShortStringHash, zero hash if type mismatch
    ShortStringHash GetShortStringHash() const
    {
        if (type_ != VAR_INT)
            return ShortStringHash::ZERO;
        return ShortStringHash(value_.int_);
    }
    
    /// Return a bool, false if type mismatch
    bool GetBool() const
    {
        if (type_ != VAR_BOOL)
            return false;
        return value_.bool_;
    }
    
    /// Return a float, 0 if type mismatch
    float GetFloat() const
    {
        if (type_ != VAR_FLOAT)
            return 0.0f;
        return value_.float_;
    }
    
    /// Return a Vector2, zero vector if type mismatch
    const Vector2& GetVector2() const
    {
        if (type_ != VAR_VECTOR2)
            return Vector2::ZERO;
        return *reinterpret_cast<const Vector2*>(&value_);
    }
    
    /// Return a Vector3, zero vector if type mismatch
    const Vector3& GetVector3() const
    {
        if (type_ != VAR_VECTOR3)
            return Vector3::ZERO;
        return *reinterpret_cast<const Vector3*>(&value_);
    }
    
    /// Return a Vector4, zero vector if type mismatch
    const Vector4& GetVector4() const
    {
        if (type_ != VAR_VECTOR4)
            return Vector4::ZERO;
        return *reinterpret_cast<const Vector4*>(&value_);
    }
    
    /// Return a Quaternion, identity if type mismatch
    const Quaternion& GetQuaternion() const
    {
        if (type_ != VAR_QUATERNION)
            return Quaternion::IDENTITY;
        return *reinterpret_cast<const Quaternion*>(&value_);
    }
    
    /// Return a Color, default if type mismatch
    const Color& GetColor() const
    {
        if (type_ != VAR_COLOR)
            return Color::WHITE;
        return *reinterpret_cast<const Color*>(&value_);
    }
    
    /// Return a string, empty if type mismatch
    const std::string& GetString() const
    {
        if (type_ != VAR_STRING)
            return emptyString;
        return *reinterpret_cast<const std::string*>(value_.ptr_);
    }
    
    /// Return a buffer, empty if type mismatch
    const std::vector<unsigned char>& GetBuffer() const
    {
        if (type_ != VAR_BUFFER)
            return emptyBuffer;
        return *reinterpret_cast<const std::vector<unsigned char>*>(value_.ptr_);
    }
    
    /// Return a pointer, null if type mismatch
    void* GetPtr() const
    {
        if (type_ != VAR_PTR)
            return 0;
        return value_.ptr_;
    }
    
    /// Return an object reference, empty if type mismatch
    const ResourceRef& GetResourceRef() const
    {
        if (type_ != VAR_RESOURCEREF)
            return emptyResourceRef;
        return *reinterpret_cast<ResourceRef*>(value_.ptr_);
    }
    
    /// Return an object reference list, empty if type mismatch
    const ResourceRefList& GetResourceRefList() const
    {
        if (type_ != VAR_RESOURCEREFLIST)
            return emptyResourceRefList;
        return *reinterpret_cast<ResourceRefList*>(value_.ptr_);
    }
    
    /// Return a variant vector, empty if type mismatch
    const VariantVector& GetVariantVector() const
    {
        if (type_ != VAR_VARIANTVECTOR)
            return emptyVariantVector;
        return *reinterpret_cast<VariantVector*>(value_.ptr_);
    }
    
    /// Return a variant map, empty if type mismatch
    const VariantMap& GetVariantMap() const
    {
        if (type_ != VAR_VARIANTMAP)
            return emptyVariantMap;
        return *reinterpret_cast<VariantMap*>(value_.ptr_);
    }
    
    /// Set from type and value strings. Pointers will be set to null, and VariantBuffer or VariantMap types are not supported
    void FromString(const std::string& type, const std::string& value);
    /// Set buffer type from a memory area
    void SetBuffer(const void* data, unsigned size);
    
    /// Return type
    VariantType GetType() const { return type_; }
    /// Return type name
    const std::string& GetTypeName() const;
    /// Convert value to string. Pointers are returned as null, and VariantBuffer or VariantMap are not supported and return empty
    std::string ToString() const;
    
    /// Empty variant
    static const Variant EMPTY;
    
private:
    /// Set new type and allocate/deallocate memory as necessary
    void SetType(VariantType newType);
    
    /// Variant type
    VariantType type_;
    /// Variant value
    VariantValue value_;
    
    /// Empty string
    static const std::string emptyString;
    /// Empty buffer
    static const std::vector<unsigned char> emptyBuffer;
    /// Empty object reference
    static const ResourceRef emptyResourceRef;
    /// Empty object reference list
    static const ResourceRefList emptyResourceRefList;
    /// Empty variant map
    static const VariantMap emptyVariantMap;
    /// Empty variant vector
    static const VariantVector emptyVariantVector;
};