//
// Copyright (c) 2008-2013 the Urho3D project.
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
#include "Cursor.h"
#include "InputEvents.h"
#include "Window.h"

#include "DebugNew.h"

namespace Urho3D
{

static const int DEFAULT_RESIZE_BORDER = 4;

OBJECTTYPESTATIC(Window);

Window::Window(Context* context) :
    BorderImage(context),
    movable_(false),
    resizable_(false),
    resizeBorder_(DEFAULT_RESIZE_BORDER, DEFAULT_RESIZE_BORDER, DEFAULT_RESIZE_BORDER, DEFAULT_RESIZE_BORDER),
    dragMode_(DRAG_NONE)
{
    bringToFront_ = true;
    clipChildren_ = true;
    active_ = true;
}

Window::~Window()
{
}

void Window::RegisterObject(Context* context)
{
    context->RegisterFactory<Window>();
    
    REF_ACCESSOR_ATTRIBUTE(Window, VAR_INTRECT, "Resize Border", GetResizeBorder, SetResizeBorder, IntRect, IntRect(DEFAULT_RESIZE_BORDER, \
        DEFAULT_RESIZE_BORDER, DEFAULT_RESIZE_BORDER, DEFAULT_RESIZE_BORDER), AM_FILE);
    ACCESSOR_ATTRIBUTE(Window, VAR_BOOL, "Is Movable", IsMovable, SetMovable, bool, false, AM_FILE);
    ACCESSOR_ATTRIBUTE(Window, VAR_BOOL, "Is Resizable", IsResizable, SetResizable, bool, false, AM_FILE);
    COPY_BASE_ATTRIBUTES(Window, BorderImage);
}

void Window::OnHover(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
{
    if (dragMode_ == DRAG_NONE)
    {
        WindowDragMode mode = GetDragMode(position);
        SetCursorShape(mode, cursor);
    }
    else
        SetCursorShape(dragMode_, cursor);
}

void Window::OnDragBegin(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
{
    if (buttons != MOUSEB_LEFT || !CheckAlignment())
    {
        dragMode_ = DRAG_NONE;
        return;
    }
    
    dragBeginCursor_ = screenPosition;
    dragBeginPosition_ = GetPosition();
    dragBeginSize_ = GetSize();
    dragMode_ = GetDragMode(position);
    SetCursorShape(dragMode_, cursor);
}

void Window::OnDragMove(const IntVector2& position, const IntVector2& screenPosition, int buttons, int qualifiers, Cursor* cursor)
{
    if (dragMode_ == DRAG_NONE)
        return;
    
    IntVector2 delta = screenPosition - dragBeginCursor_;

    const IntVector2& position_ = GetPosition();
    const IntVector2& size_ = GetSize();
    const IntVector2& minSize_ = GetMinSize();
    const IntVector2& maxSize_ = GetMaxSize();
    
    switch (dragMode_)
    {
    case DRAG_MOVE:
        SetPosition(dragBeginPosition_ + delta);
        break;
        
    case DRAG_RESIZE_TOPLEFT:
        SetPosition(Clamp(dragBeginPosition_.x_ + delta.x_, position_.x_ - (maxSize_.x_ - size_.x_), position_.x_ + (size_.x_ - minSize_.x_)), 
            Clamp(dragBeginPosition_.y_ + delta.y_, position_.y_ - (maxSize_.y_ - size_.y_), position_.y_ + (size_.y_ - minSize_.y_)));
        SetSize(dragBeginSize_ - delta);
        break;
        
    case DRAG_RESIZE_TOP:
        SetPosition(dragBeginPosition_.x_, Clamp(dragBeginPosition_.y_ + delta.y_, position_.y_ - (maxSize_.y_ - size_.y_), position_.y_ + (size_.y_ - minSize_.y_)));
        SetSize(dragBeginSize_.x_, dragBeginSize_.y_ - delta.y_);
        break;
        
    case DRAG_RESIZE_TOPRIGHT:
        SetPosition(dragBeginPosition_.x_, dragBeginPosition_.y_ + delta.y_);
        SetSize(dragBeginSize_.x_ + delta.x_, dragBeginSize_.y_ - delta.y_);
        break;
        
    case DRAG_RESIZE_RIGHT:
        SetSize(dragBeginSize_.x_ + delta.x_, dragBeginSize_.y_);
        break;
        
    case DRAG_RESIZE_BOTTOMRIGHT:
        SetSize(dragBeginSize_ + delta);
        break;
        
    case DRAG_RESIZE_BOTTOM:
        SetSize(dragBeginSize_.x_, dragBeginSize_.y_ + delta.y_);
        break;
        
    case DRAG_RESIZE_BOTTOMLEFT:
        SetPosition(Clamp(dragBeginPosition_.x_ + delta.x_, position_.x_ - (maxSize_.x_ - size_.x_), position_.x_ + (size_.x_ - minSize_.x_)), dragBeginPosition_.y_);
        SetSize(dragBeginSize_.x_ - delta.x_, dragBeginSize_.y_ + delta.y_);
        break;
        
    case DRAG_RESIZE_LEFT:
        SetPosition(Clamp(dragBeginPosition_.x_ + delta.x_, position_.x_ - (maxSize_.x_ - size_.x_), position_.x_ + (size_.x_ - minSize_.x_)), dragBeginPosition_.y_);
        SetSize(dragBeginSize_.x_ - delta.x_, dragBeginSize_.y_);
        break;

    default:
        break;
    }
    
    ValidatePosition();
    SetCursorShape(dragMode_, cursor);
}

void Window::OnDragEnd(const IntVector2& position, const IntVector2& screenPosition, Cursor* cursor)
{
    dragMode_ = DRAG_NONE;
}

void Window::SetMovable(bool enable)
{
    movable_ = enable;
}

void Window::SetResizable(bool enable)
{
    resizable_ = enable;
}

void Window::SetResizeBorder(const IntRect& rect)
{
    resizeBorder_.left_ = Max(rect.left_, 0);
    resizeBorder_.top_ = Max(rect.top_, 0);
    resizeBorder_.right_ = Max(rect.right_, 0);
    resizeBorder_.bottom_ = Max(rect.bottom_, 0);
}

WindowDragMode Window::GetDragMode(const IntVector2& position) const
{
    WindowDragMode mode = DRAG_NONE;
    
    // Top row
    if (position.y_ < resizeBorder_.top_)
    {
        if (movable_)
            mode = DRAG_MOVE;
        if (resizable_)
        {
            mode = DRAG_RESIZE_TOP;
            if (position.x_ < resizeBorder_.left_)
                mode = DRAG_RESIZE_TOPLEFT;
            if (position.x_ >= GetWidth() - resizeBorder_.right_)
                mode = DRAG_RESIZE_TOPRIGHT;
        }
    }
    // Bottom row
    else if (position.y_ >= GetHeight() - resizeBorder_.bottom_)
    {
        if (movable_)
            mode = DRAG_MOVE;
        if (resizable_)
        {
            mode = DRAG_RESIZE_BOTTOM;
            if (position.x_ < resizeBorder_.left_)
                mode = DRAG_RESIZE_BOTTOMLEFT;
            if (position.x_ >= GetWidth() - resizeBorder_.right_)
                mode = DRAG_RESIZE_BOTTOMRIGHT;
        }
    }
    // Middle
    else
    {
        if (movable_)
            mode = DRAG_MOVE;
        if (resizable_)
        {
            if (position.x_ < resizeBorder_.left_)
                mode = DRAG_RESIZE_LEFT;
            if (position.x_ >= GetWidth() - resizeBorder_.right_)
                mode = DRAG_RESIZE_RIGHT;
        }
    }
    
    return mode;
}

void Window::SetCursorShape(WindowDragMode mode, Cursor* cursor) const
{
    if (!cursor)
        return;
    
    switch (mode)
    {
    case DRAG_RESIZE_TOP:
    case DRAG_RESIZE_BOTTOM:
        cursor->SetShape(CS_RESIZEVERTICAL);
        break;
        
    case DRAG_RESIZE_LEFT:
    case DRAG_RESIZE_RIGHT:
        cursor->SetShape(CS_RESIZEHORIZONTAL);
        break;

    case DRAG_RESIZE_TOPRIGHT:
    case DRAG_RESIZE_BOTTOMLEFT:
        cursor->SetShape(CS_RESIZEDIAGONAL_TOPRIGHT);
        break;
        
    case DRAG_RESIZE_TOPLEFT:
    case DRAG_RESIZE_BOTTOMRIGHT:
        cursor->SetShape(CS_RESIZEDIAGONAL_TOPLEFT);
        break;
    
    default:
        break;
    }
}

void Window::ValidatePosition()
{
    // Check that window does not go more than halfway outside its parent in either dimension
    if (!parent_)
        return;
    
    const IntVector2& parentSize = parent_->GetSize();
    IntVector2 position = GetPosition();
    IntVector2 halfSize = GetSize() / 2;
    
    position.x_ = Clamp(position.x_, -halfSize.x_, parentSize.x_ - halfSize.x_);
    position.y_ = Clamp(position.y_, -halfSize.y_, parentSize.y_ - halfSize.y_);
    
    SetPosition(position);
}

bool Window::CheckAlignment() const
{
    // Only top left-alignment is supported for move and resize
    if (GetHorizontalAlignment() == HA_LEFT && GetVerticalAlignment() == VA_TOP)
        return true;
    else
        return false;
}

}
