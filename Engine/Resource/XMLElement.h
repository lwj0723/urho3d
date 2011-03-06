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

#ifndef RESOURCE_XMLELEMENT_H
#define RESOURCE_XMLELEMENT_H

#include "BoundingBox.h"
#include "Color.h"
#include "Rect.h"
#include "Quaternion.h"
#include "Variant.h"
#include "Vector4.h"

#include <string>

class TiXmlElement;

//! An element in an XML file
class XMLElement
{
public:
    //! Construct null element
    XMLElement();
    //! Construct with element pointer
    XMLElement(TiXmlElement* element);
    //! Copy-construct from another element
    XMLElement(const XMLElement& rhs);
    //! Destruct
    ~XMLElement();
    
    //! Create a child element
    XMLElement createChildElement(const std::string& name);
    //! Remove a child element, either first or last of them if several exist
    bool removeChildElement(const std::string& name = std::string(), bool last = true);
    //! Remove child elements of certain name, or all child elements if name is empty
    bool removeChildElements(const std::string& name = std::string());
    //! Set an attribute
    bool setAttribute(const std::string& name, const std::string& value);
    //! Set a bool attribute
    bool setBool(const std::string& name, bool value);
    //! Set a BoundingBox attribute
    bool setBoundingBox(const BoundingBox& value);
    //! Set a buffer attribute
    bool setBuffer(const std::string& name, const void* data, unsigned size);
    //! Set a buffer attribute
    bool setBuffer(const std::string& name, const std::vector<unsigned char>& value);
    //! Set a Color attribute
    bool setColor(const std::string& name, const Color& value);
    //! Set a float attribute
    bool setFloat(const std::string& name, float value);
    //! Set an integer attribute
    bool setInt(const std::string& name, int value);
    //! Set an IntRect attribute
    bool setIntRect(const std::string& name, const IntRect& value);
    //! Set an IntVector2 attribute
    bool setIntVector2(const std::string& name, const IntVector2& value);
    //! Set a Rect attribute
    bool setRect(const std::string& name, const Rect& value);
    //! Set a Quaternion attribute
    bool setQuaternion(const std::string& name, const Quaternion& value);
    //! Set a string attribute
    bool setString(const std::string& name, const std::string& value);
    //! Set a Variant attribute
    bool setVariant(const Variant& value);
    //! Set a VariantVector attribute. Creates child elements as necessary
    bool setVariantVector(const VariantVector& value);
    //! Set a VariantMap attribute. Creates child elements as necessary
    bool setVariantMap(const VariantMap& value);
    //! Set a Vector2 attribute
    bool setVector2(const std::string& name, const Vector2& value);
    //! Set a Vector3 attribute
    bool setVector3(const std::string& name, const Vector3& value);
    //! Set a Vector4 attribute
    bool setVector4(const std::string& name, const Vector4& value);
    
    //! Return whether does not refer to an element
    bool isNull() const { return mElement == 0; }
    //! Return whether refers to an element
    bool notNull() const { return mElement != 0; }
    //! Return true if refers to an element
    operator bool () const { return mElement != 0; }
    //! Return element name
    std::string getName() const;
    //! Return element contents
    std::string getText() const;
    //! Return whether has a child element
    bool hasChildElement(const std::string& name) const;
    //! Return child element, or null if missing
    XMLElement getChildElement(const std::string& name = std::string()) const;
    //! Return next sibling element
    XMLElement getNextElement(const std::string& name = std::string()) const;
    //! Return parent element
    XMLElement getParentElement() const;
    //! Return whether has an attribute
    bool hasAttribute(const std::string& name) const;
    //! Return attribute, or empty if missing
    std::string getAttribute(const std::string& name) const;
    //! Return names of all attributes
    std::vector<std::string> getAttributeNames() const;
    //! Return bool attribute, or false if missing
    bool getBool(const std::string& name) const;
    //! Return buffer attribute, or empty if missing
    std::vector<unsigned char> getBuffer(const std::string& name) const;
    //! Copy buffer attribute into a supplied buffer. Throw exception if not big enough
    void getBuffer(const std::string& name, void* dest, unsigned size) const;
    //! Return bounding box attribute, or empty if missing
    BoundingBox getBoundingBox() const;
    //! Return a Color attribute, or default if missing
    Color getColor(const std::string& name) const;
    //! Return a float attribute, or zero if missing
    float getFloat(const std::string& name) const;
    //! Return an integer attribute, or zero if missing
    int getInt(const std::string& name) const;
    //! Return an IntRect attribute, or default if missing
    IntRect getIntRect(const std::string& name) const;
    //! Return an IntVector2 attribute, or default if missing
    IntVector2 getIntVector2(const std::string& name) const;
    //! Return a Rect attribute, or default if missing
    Rect getRect(const std::string& name) const;
    //! Return a Quaternion attribute, or default if missing
    Quaternion getQuaternion(const std::string& name) const;
    //! Return a string attribute, or empty if missing
    std::string getString(const std::string& name) const;
    //! Return a string attribute in lowercase, or empty if missing
    std::string getStringLower(const std::string& name) const;
    //! Return a string attribute in uppercase, or empty if missing
    std::string getStringUpper(const std::string& name) const;
    //! Return a Variant attribute, or empty if missing
    Variant getVariant() const;
    //! Return a VariantVector attribute, or empty if missing
    VariantVector getVariantVector() const;
    //! Return a VariantMap attribute, or empty if missing
    VariantMap getVariantMap() const;
    //! Return a Vector2 attribute, or default if missing
    Vector2 getVector2(const std::string& name) const;
    //! Return a Vector3 attribute, or default if missing
    Vector3 getVector3(const std::string& name) const;
    //! Return a Vector4 attribute, or default if missing
    Vector4 getVector4(const std::string& name) const;
    //! Return any Vector attribute as Vector4. Missing coordinates will be zero
    Vector4 getVector(const std::string& name) const;
    
    //! Return TinyXML element
    TiXmlElement* getElement() const { return mElement; }
    
private:
    //! TinyXML element
    TiXmlElement* mElement;
};

#endif // RESOURCE_XMLELEMENT_H
