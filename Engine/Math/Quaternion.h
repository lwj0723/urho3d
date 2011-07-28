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

#include "Matrix3.h"

/// Rotation represented as a four-dimensional normalized vector
class Quaternion
{
public:
    /// Construct identity quaternion
    Quaternion() :
        w_(1.0f),
        x_(0.0f),
        y_(0.0f),
        z_(0.0f)
    {
    }
    
    /// Copy-construct from another quaternion
    Quaternion(const Quaternion& quat) :
        w_(quat.w_),
        x_(quat.x_),
        y_(quat.y_),
        z_(quat.z_)
    {
    }
    
    /// Construct from values
    Quaternion(float w, float x, float y, float z) :
        w_(w),
        x_(x),
        y_(y),
        z_(z)
    {
    }
    
    /// Construct from a float array
    Quaternion(const float* data) :
        w_(data[0]),
        x_(data[1]),
        y_(data[2]),
        z_(data[3])
    {
    }
    
    /// Construct from an angle (in degrees) and axis
    Quaternion(float angle, const Vector3& axis);
    /// Construct from Euler angles (in degrees)
    Quaternion(float x, float y, float z);
    /// Construct from the rotation difference between two vectors
    Quaternion(const Vector3& start, const Vector3& end);
    /// Construct from a rotation matrix
    Quaternion(const Matrix3& matrix);
    
    /// Assign from another quaternion
    Quaternion& operator = (const Quaternion& rhs)
    {
        w_ = rhs.w_;
        x_ = rhs.x_;
        y_ = rhs.y_;
        z_ = rhs.z_;
        return *this;
    }
    
    /// Add-assign a quaternion
    Quaternion& operator += (const Quaternion& rhs)
    {
        w_ += rhs.w_;
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }
    
    /// Test for equality with another quaternion
    bool operator == (const Quaternion& rhs) const { return Equals(w_, rhs.w_) && Equals(x_, rhs.x_) && Equals(y_, rhs.y_) && Equals(z_, rhs.z_); }
    /// Test for inequality with another quaternion
    bool operator != (const Quaternion& rhs) const { return !Equals(w_, rhs.w_) || !Equals(x_, rhs.x_) || !Equals(y_, rhs.y_) || !Equals(z_, rhs.z_); }
    /// Multiply with a scalar
    Quaternion operator * (float rhs) const { return Quaternion(w_ * rhs, x_ * rhs, y_ * rhs, z_ * rhs); }
    /// Return negation
    Quaternion operator - () const { return Quaternion(-w_, -x_, -y_, -z_); }
    /// Add a quaternion
    Quaternion operator + (const Quaternion& rhs) const { return Quaternion(w_ + rhs.w_, x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_); }
    /// Subtract a quaternion
    Quaternion operator - (const Quaternion& rhs) const { return Quaternion(w_ - rhs.w_, x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_); }
    
    /// Multiply a quaternion
    Quaternion operator * (const Quaternion& rhs) const
    {
        return Quaternion(
            w_ * rhs.w_ - x_ * rhs.x_ - y_ * rhs.y_ - z_ * rhs.z_,
            w_ * rhs.x_ + x_ * rhs.w_ + y_ * rhs.z_ - z_ * rhs.y_,
            w_ * rhs.y_ + y_ * rhs.w_ + z_ * rhs.x_ - x_ * rhs.z_,
            w_ * rhs.z_ + z_ * rhs.w_ + x_ * rhs.y_ - y_ * rhs.x_
        );
    }
    
    /// Multiply a Vector3
    Vector3 operator * (const Vector3& rhs) const
    {
        Vector3 qVec(x_,y_,z_);
        Vector3 cross1(qVec.CrossProduct(rhs));
        Vector3 cross2(qVec.CrossProduct(cross1));
        
        return rhs + 2.0f * (cross1 * w_ + cross2);
    }
    
    /// Normalize to unit length and return the previous length
    float Normalize()
    {
        float len = sqrtf(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_);
        if (len >= M_EPSILON)
        {
            float invLen = 1.0f / len;
            w_ *= invLen;
            x_ *= invLen;
            y_ *= invLen;
            z_ *= invLen;
        }
        
        return len;
    }
    
    /// Normalize to unit length using fast inverse square root
    void NormalizeFast()
    {
        float invLen = FastInvSqrt(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_);
        w_ *= invLen;
        x_ *= invLen;
        y_ *= invLen;
        z_ *= invLen;
    }
    
    /// Return normalized to unit length
    Quaternion Normalized() const
    {
        float len = sqrtf(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_);
        if (len >= M_EPSILON)
            return *this * (1.0f / len);
        else
            return IDENTITY;
    }
    
    /// Return normalized to unit length, using fast inverse square root
    Quaternion NormalizedFast() const { return *this * FastInvSqrt(w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_); }
    
    /// Return inverse
    Quaternion Inverse() const
    {
        float lenSquared = w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_;
        if (lenSquared >= M_EPSILON)
        {
            float invLenSquared = 1.0f / lenSquared;
            return Quaternion(
                w_ * invLenSquared,
                -x_ * invLenSquared,
                -y_ * invLenSquared,
                -z_ * invLenSquared
            );
        }
        else
            return IDENTITY;
    }
    
    /// Return squared length
    float LengthSquared() const { return w_ * w_ + x_ * x_ + y_ * y_ + z_ * z_; }
    /// Calculate dot product
    float DotProduct(const Quaternion& rhs) const { return w_ * rhs.w_ + x_ * rhs.x_ + y_ * rhs.y_ + z_ * rhs.z_; }
    /// Normalized interpolation with another quaternion
    Quaternion Nlerp(const Quaternion& rhs, float t) const { return (*this * (1.0f - t) + rhs * t).Normalized(); }
    /// Normalized interpolation with another quaternion, using fast inverse square root
    Quaternion NlerpFast(const Quaternion& rhs, float t) const { return (*this * (1.0f - t) + rhs * t).NormalizedFast(); }
    
    /// Return Euler angles in degrees
    Vector3 EulerAngles() const;
    /// Return yaw angle in degrees
    float YawAngle() const;
    /// Return pitch angle in degrees
    float PitchAngle() const;
    /// Return roll angle in degrees
    float RollAngle() const;
    /// Return the rotation matrix that corresponds to this quaternion
    Matrix3 RotationMatrix() const;
    /// Spherical interpolation with another quaternion
    Quaternion Slerp(Quaternion rhs, float t) const;
    /// Return float data
    const float* GetData() const { return &w_; }
    /// Return as string
    String ToString() const;
    
    /// W coordinate
    float w_;
    /// X coordinate
    float x_;
    /// Y coordinate
    float y_;
    /// Z coordinate
    float z_;
    
    /// Identity quaternion
    static const Quaternion IDENTITY;
};
