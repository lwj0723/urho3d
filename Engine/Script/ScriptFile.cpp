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
#include "FileSystem.h"
#include "Log.h"
#include "Profiler.h"
#include "ResourceCache.h"
#include "Script.h"
#include "ScriptFile.h"
#include "ScriptInstance.h"
#include "SharedArrayPtr.h"
#include "StringUtils.h"

#include <angelscript.h>
#include <cstring>

#include "DebugNew.h"

OBJECTTYPESTATIC(ScriptFile);

ScriptFile::ScriptFile(Context* context) :
    Resource(context),
    script_(GetSubsystem<Script>()),
    scriptModule_(0),
    compiled_(false)
{
}

ScriptFile::~ScriptFile()
{
    ReleaseModule();
}

void ScriptFile::RegisterObject(Context* context)
{
    context->RegisterFactory<ScriptFile>();
}

bool ScriptFile::Load(Deserializer& source)
{
    PROFILE(LoadScript);
    
    ReleaseModule();
    
    // Create the module. Discard previous module if there was one
    if (scriptModule_)
        script_->GetModuleMap().erase(scriptModule_);
    asIScriptEngine* engine = script_->GetScriptEngine();
    scriptModule_ = engine->GetModule(GetName().c_str(), asGM_ALWAYS_CREATE);
    if (!scriptModule_)
    {
        LOGERROR("Failed to create script module " + GetName());
        return false;
    }
    
    // Add the initial section and check for includes
    if (!AddScriptSection(engine, source))
        return false;
    
    // Compile. Set script engine logging to retained mode so that potential exceptions can show all error info
    script_->SetLogMode(LOGMODE_RETAINED);
    script_->ClearLogMessages();
    int result = scriptModule_->Build();
    std::string errors = script_->GetLogMessages();
    script_->SetLogMode(LOGMODE_IMMEDIATE);
    if (result < 0)
    {
        LOGERROR("Failed to compile script module " + GetName() + ":\n" + errors);
        return false;
    }
    if (!errors.empty())
        LOGWARNING(errors);
    
    LOGINFO("Compiled script module " + GetName());
    compiled_ = true;
    script_->GetModuleMap()[scriptModule_] = this;
    
    return true;
}

void ScriptFile::AddEventHandler(StringHash eventType, const std::string& handlerName)
{
    if (!compiled_)
        return;
    
    std::string declaration = "void " + handlerName + "(StringHash, VariantMap&)";
    asIScriptFunction* function = GetFunction(declaration);
    if (!function)
    {
        declaration = "void " + handlerName + "()";
        function = GetFunction(declaration);
        if (!function)
        {
            LOGERROR("Event handler function " + handlerName + " not found in " + GetName());
            return;
        }
    }
    
    SubscribeToEvent(eventType, HANDLER_USERDATA(ScriptFile, HandleScriptEvent, (void*)function));
}

void ScriptFile::AddEventHandler(Object* sender, StringHash eventType, const std::string& handlerName)
{
    if (!compiled_)
        return;
    
    if (!sender)
    {
        LOGERROR("Null event sender for event " + ToString(eventType) + ", handler " + handlerName);
        return;
    }
    
    std::string declaration = "void " + handlerName + "(StringHash, VariantMap&)";
    asIScriptFunction* function = GetFunction(declaration);
    if (!function)
    {
        declaration = "void " + handlerName + "()";
        function = GetFunction(declaration);
        if (!function)
        {
            LOGERROR("Event handler function " + handlerName + " not found in " + GetName());
            return;
        }
    }
    
    SubscribeToEvent(sender, eventType, HANDLER_USERDATA(ScriptFile, HandleScriptEvent, (void*)function));
}

bool ScriptFile::Execute(const std::string& declaration, const VariantVector& parameters, bool unprepare)
{
    asIScriptFunction* function = GetFunction(declaration);
    if (!function)
    {
        LOGERROR("Function " + declaration + " not found in " + GetName());
        return false;
    }
    
    return Execute(GetFunction(declaration), parameters, unprepare);
}

bool ScriptFile::Execute(asIScriptFunction* function, const VariantVector& parameters, bool unprepare)
{
    PROFILE(ExecuteFunction);
    
    if ((!compiled_) || (!function))
        return false;
    
    asIScriptContext* context = script_->GetScriptFileContext();
    if (!context)
    {
        LOGERROR("Maximum script execution nesting level exceeded");
        return false;
    }
    
    if (context->Prepare(function->GetId()) < 0)
        return false;
    
    SetParameters(context, function, parameters);
    
    script_->IncScriptNestingLevel();
    bool success = context->Execute() >= 0;
    if (unprepare)
        context->Unprepare();
    script_->DecScriptNestingLevel();
    
    return success;
}

bool ScriptFile::Execute(asIScriptObject* object, const std::string& declaration, const VariantVector& parameters, bool unprepare)
{
    asIScriptFunction* method = GetMethod(object, declaration);
    if (!method)
    {
        LOGERROR("Method " + declaration + " not found in " + GetName());
        return false;
    }
    
    return Execute(object, method, parameters, unprepare);
}

bool ScriptFile::Execute(asIScriptObject* object, asIScriptFunction* method, const VariantVector& parameters, bool unprepare)
{
    PROFILE(ExecuteMethod);
    
    if ((!compiled_) || (!object) || (!method))
        return false;
    
    asIScriptContext* context = script_->GetScriptFileContext();
    if (!context)
    {
        LOGERROR("Maximum script execution nesting level exceeded");
        return false;
    }
    
    if (context->Prepare(method->GetId()) < 0)
        return false;
    
    context->SetObject(object);
    SetParameters(context, method, parameters);
    
    script_->IncScriptNestingLevel();
    bool success = context->Execute() >= 0;
    if (unprepare)
        context->Unprepare();
    script_->DecScriptNestingLevel();
    
    return success;
}

asIScriptObject* ScriptFile::CreateObject(const std::string& className)
{
    PROFILE(CreateObject);
    
    if (!IsCompiled())
        return 0;
    
    asIScriptContext* context = script_->GetScriptFileContext();
    if (!context)
    {
        LOGERROR("Maximum script execution nesting level exceeded, can not create object");
        return 0;
    }
    
    asIScriptEngine* engine = script_->GetScriptEngine();
    asIObjectType *type = engine->GetObjectTypeById(scriptModule_->GetTypeIdByDecl(className.c_str()));
    if (!type)
        return 0;
    
    // Ensure that the type implements the "ScriptObject" interface, so it can be returned to script properly
    bool found = false;
    std::map<asIObjectType*, bool>::const_iterator i = checkedClasses_.find(type);
    if (i != checkedClasses_.end())
        found = i->second;
    else
    {
        unsigned numInterfaces = type->GetInterfaceCount();
        for (unsigned j = 0; j < numInterfaces; ++j)
        {
            asIObjectType* interfaceType = type->GetInterface(j);
            if (!strcmp(interfaceType->GetName(), "ScriptObject"))
            {
                found = true;
                break;
            }
        }
        checkedClasses_[type] = found;
    }
    if (!found)
    {
        LOGERROR("Script class " + className + " does not implement the ScriptObject interface");
        return 0;
    }
    
    // Get the factory function id from the object type
    std::string factoryName = className + "@ " + className + "()";
    int factoryId = type->GetFactoryIdByDecl(factoryName.c_str());
    if ((factoryId < 0) || (context->Prepare(factoryId) < 0) || (context->Execute() < 0))
        return 0;
    
    asIScriptObject* obj = *(static_cast<asIScriptObject**>(context->GetAddressOfReturnValue()));
    if (obj)
        obj->AddRef();
    
    return obj;
}

asIScriptFunction* ScriptFile::GetFunction(const std::string& declaration)
{
    if (!compiled_)
        return 0;
    
    std::map<std::string, asIScriptFunction*>::const_iterator i = functions_.find(declaration);
    if (i != functions_.end())
        return i->second;
    
    int id = scriptModule_->GetFunctionIdByDecl(declaration.c_str());
    asIScriptFunction* function = scriptModule_->GetFunctionDescriptorById(id);
    functions_[declaration] = function;
    return function;
}

asIScriptFunction* ScriptFile::GetMethod(asIScriptObject* object, const std::string& declaration)
{
    if ((!compiled_) || (!object))
        return 0;
    
    asIObjectType* type = object->GetObjectType();
    if (!type)
        return 0;
    std::map<asIObjectType*, std::map<std::string, asIScriptFunction*> >::const_iterator i = methods_.find(type);
    if (i != methods_.end())
    {
        std::map<std::string, asIScriptFunction*>::const_iterator j = i->second.find(declaration);
        if (j != i->second.end())
            return j->second;
    }
    
    int id = type->GetMethodIdByDecl(declaration.c_str());
    asIScriptFunction* function = scriptModule_->GetFunctionDescriptorById(id);
    methods_[type][declaration] = function;
    return function;
}

bool ScriptFile::AddScriptSection(asIScriptEngine* engine, Deserializer& source)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    
    unsigned dataSize = source.GetSize();
    SharedArrayPtr<char> buffer(new char[dataSize]);
    source.Read((void*)buffer.GetPtr(), dataSize);
    
    // Pre-parse for includes
    // Adapted from Angelscript's scriptbuilder add-on
    std::vector<std::string> includeFiles;
    unsigned pos = 0;
    while(pos < dataSize)
    {
        int len;
        asETokenClass t = engine->ParseToken(&buffer[pos], dataSize - pos, &len);
        if ((t == asTC_COMMENT) || (t == asTC_WHITESPACE))
        {
            pos += len;
            continue;
        }
        // Is this a preprocessor directive?
        if (buffer[pos] == '#')
        {
            int start = pos++;
            asETokenClass t = engine->ParseToken(&buffer[pos], dataSize - pos, &len);
            if (t == asTC_IDENTIFIER)
            {
                std::string token(&buffer[pos], len);
                if (token == "include")
                {
                    pos += len;
                    t = engine->ParseToken(&buffer[pos], dataSize - pos, &len);
                    if (t == asTC_WHITESPACE)
                    {
                        pos += len;
                        t = engine->ParseToken(&buffer[pos], dataSize - pos, &len);
                    }
                    
                    if ((t == asTC_VALUE) && (len > 2) && (buffer[pos] == '"'))
                    {
                        // Get the include file
                        std::string includeFile(&buffer[pos+1], len - 2);
                        pos += len;
                        
                        // If the file is not found as it is, add the path of current file
                        if (!cache->Exists(includeFile))
                            includeFile = GetPath(GetName()) + includeFile;
                        
                        std::string includeFileLower = ToLower(includeFile);
                        
                        // If not included yet, store it for later processing
                        if (includeFiles_.find(includeFileLower) == includeFiles_.end())
                        {
                            includeFiles_.insert(includeFileLower);
                            includeFiles.push_back(includeFile);
                        }
                        
                        // Overwrite the include directive with space characters to avoid compiler error
                        memset(&buffer[start], ' ', pos - start);
                    }
                }
            }
        }
        // Don't search includes within statement blocks or between tokens in statements
        else
        {
            int len;
            // Skip until ; or { whichever comes first
            while ((pos < dataSize) && (buffer[pos] != ';') && (buffer[pos] != '{' ))
            {
                engine->ParseToken(&buffer[pos], 0, &len);
                pos += len;
            }
            // Skip entire statement block
            if ((pos < dataSize) && (buffer[pos] == '{' ))
            {
                ++pos;
                // Find the end of the statement block
                int level = 1;
                while ((level > 0) && (pos < dataSize))
                {
                    asETokenClass t = engine->ParseToken(&buffer[pos], 0, &len);
                    if (t == asTC_KEYWORD)
                    {
                        if (buffer[pos] == '{')
                            level++;
                        else if(buffer[pos] == '}')
                            level--;
                    }
                    pos += len;
                }
            }
            else
                ++pos;
        }
    }
    
    // Process includes first
    for (unsigned i = 0; i < includeFiles.size(); ++i)
    {
        SharedPtr<File> file = cache->GetFile(includeFiles[i]);
        if (!AddScriptSection(engine, *file))
            return false;
    }
    
    // Then add this section
    if (scriptModule_->AddScriptSection(source.GetName().c_str(), (const char*)buffer.GetPtr(), dataSize) < 0)
    {
        LOGERROR("Failed to add script section " + source.GetName());
        return false;
    }
    
    SetMemoryUse(GetMemoryUse() + dataSize);
    return true;
}

void ScriptFile::SetParameters(asIScriptContext* context, asIScriptFunction* function, const VariantVector& parameters)
{
    unsigned paramCount = function->GetParamCount();
    for (unsigned i = 0; (i < parameters.size()) && (i < paramCount); ++i)
    {
        int paramType = function->GetParamTypeId(i);
        
        switch (paramType)
        {
        case asTYPEID_BOOL:
            context->SetArgByte(i, (unsigned char)parameters[i].GetBool());
            break;
            
        case asTYPEID_INT8:
        case asTYPEID_UINT8:
            context->SetArgByte(i, parameters[i].GetInt());
            break;
            
        case asTYPEID_INT16:
        case asTYPEID_UINT16:
            context->SetArgWord(i, parameters[i].GetInt());
            break;
            
        case asTYPEID_INT32:
        case asTYPEID_UINT32:
            context->SetArgDWord(i, parameters[i].GetInt());
            break;
            
        case asTYPEID_FLOAT:
            context->SetArgFloat(i, parameters[i].GetFloat());
            break;
            
        default:
            if (paramType & asTYPEID_APPOBJECT)
            {
                switch (parameters[i].GetType())
                {
                case VAR_VECTOR2:
                    context->SetArgObject(i, (void *)&parameters[i].GetVector2());
                    break;
                case VAR_VECTOR3:
                    context->SetArgObject(i, (void *)&parameters[i].GetVector3());
                    break;
                case VAR_VECTOR4:
                    context->SetArgObject(i, (void *)&parameters[i].GetVector4());
                    break;
                case VAR_QUATERNION:
                    context->SetArgObject(i, (void *)&parameters[i].GetQuaternion());
                    break;
                case VAR_STRING:
                    context->SetArgObject(i, (void *)&parameters[i].GetString());
                    break;
                case VAR_PTR:
                    context->SetArgObject(i, (void *)parameters[i].GetPtr());
                    break;
                }
            }
            break;
        }
    }
}

void ScriptFile::ReleaseModule()
{
    if (scriptModule_)
    {
        // Clear search caches, event handlers and function-to-file mappings
        includeFiles_.clear();
        checkedClasses_.clear();
        functions_.clear();
        methods_.clear();
        UnsubscribeFromAllEventsWithUserData();
        
        // Perform a full garbage collection cycle now
        script_->GarbageCollect(true);
        
        // Remove the module
        script_->GetModuleMap().erase(scriptModule_);
        asIScriptEngine* engine = script_->GetScriptEngine();
        engine->DiscardModule(GetName().c_str());
        scriptModule_ = 0;
        compiled_ = false;
        SetMemoryUse(0);
    }
}

void ScriptFile::HandleScriptEvent(StringHash eventType, VariantMap& eventData)
{
    if (!compiled_)
        return;
    
    asIScriptFunction* function = static_cast<asIScriptFunction*>(context_->GetHandler()->GetUserData());
    
    VariantVector parameters;
    if (function->GetParamCount() > 0)
    {
        parameters.push_back(Variant((void*)&eventType));
        parameters.push_back(Variant((void*)&eventData));
    }
    
    Execute(function, parameters);
}

ScriptFile* GetScriptContextFile()
{
    asIScriptContext* context = asGetActiveContext();
    asIScriptFunction* function = context->GetFunction();
    asIScriptModule* module = function->GetEngine()->GetModule(function->GetModuleName());
    std::map<asIScriptModule*, ScriptFile*>& moduleMap = static_cast<Script*>(context->GetEngine()->GetUserData())->GetModuleMap();
    std::map<asIScriptModule*, ScriptFile*>::const_iterator i = moduleMap.find(module);
    if (i != moduleMap.end())
        return i->second;
    else
        return 0;
}
