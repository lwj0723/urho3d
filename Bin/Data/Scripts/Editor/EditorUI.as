// Urho3D editor user interface

XMLFile@ uiStyle;
UIElement@ uiMenuBar;
FileSelector@ uiFileSelector;

Array<String> uiSceneFilters = {"*.xml", "*.bin", "*.*"};
Array<String> uiAllFilter = {"*.*"};
Array<String> uiScriptFilter = {"*.as", "*.*"};
uint uiSceneFilter = 0;
String uiScenePath;
String uiImportPath;
String uiScriptPath;

void CreateUI()
{
    uiStyle = cache.GetResource("XMLFile", "UI/DefaultStyle.xml");

    CreateCursor();
    CreateMenuBar();
    CreateSceneWindow();
    CreateNodeWindow();
    CreateEditorSettingsDialog();
    CreateStatsBar();
    CreateConsole();
    CreateDebugHud();

    SubscribeToEvent("ScreenMode", "ResizeUI");
    SubscribeToEvent("MenuSelected", "HandleMenuSelected");
    SubscribeToEvent("KeyDown", "HandleKeyDown");
}

void ResizeUI()
{
    uiMenuBar.SetFixedWidth(graphics.width);
}

void CreateCursor()
{
    Cursor@ cursor = Cursor("Cursor");
    cursor.style = uiStyle;
    cursor.SetPosition(graphics.width / 2, graphics.height / 2);
    ui.cursor = cursor;
    if (GetPlatform() == "Android" || GetPlatform() == "iOS")
        ui.cursor.visible = false;
}

void CreateMenuBar()
{
    uiMenuBar = BorderImage("MenuBar");
    uiMenuBar.active = true;
    uiMenuBar.SetStyle(uiStyle, "EditorMenuBar");
    uiMenuBar.SetLayout(LM_HORIZONTAL, 4, IntRect(2, 2, 2, 2));
    ui.root.AddChild(uiMenuBar);

    {
        Menu@ fileMenu = CreateMenu("File");
        Window@ filePopup = fileMenu.popup;
        filePopup.AddChild(CreateMenuItem("New scene", 0, 0));
        filePopup.AddChild(CreateMenuItem("Open scene", 'O', QUAL_CTRL));
        filePopup.AddChild(CreateMenuItem("Save scene", 'S', QUAL_CTRL));
        filePopup.AddChild(CreateMenuItem("Save scene as", 'S', QUAL_SHIFT | QUAL_CTRL));
        filePopup.AddChild(CreateMenuDivider());

        Menu@ loadNodeMenu = CreateMenuItem("Load node", 0, 0);
        Window@ loadNodePopup = CreatePopup(loadNodeMenu);
        loadNodeMenu.popupOffset = IntVector2(loadNodeMenu.width, 0);
        loadNodePopup.AddChild(CreateMenuItem("As replicated", 0, 0));
        loadNodePopup.AddChild(CreateMenuItem("As local", 0, 0));
        filePopup.AddChild(loadNodeMenu);
        
        filePopup.AddChild(CreateMenuItem("Save node as", 0, 0));
        filePopup.AddChild(CreateMenuDivider());
        filePopup.AddChild(CreateMenuItem("Import model", 0, 0));
        filePopup.AddChild(CreateMenuItem("Import scene", 0, 0));
        filePopup.AddChild(CreateMenuItem("Run script", 0, 0));
        filePopup.AddChild(CreateMenuDivider());
        filePopup.AddChild(CreateMenuItem("Set resource path", 0, 0));
        filePopup.AddChild(CreateMenuItem("Reload resources", 'R', QUAL_CTRL));
        filePopup.AddChild(CreateMenuDivider());
        filePopup.AddChild(CreateMenuItem("Exit", 0, 0));
        uiMenuBar.AddChild(fileMenu);
    }

    {
        Menu@ editMenu = CreateMenu("Edit");
        Window@ editPopup = editMenu.popup;
        editPopup.AddChild(CreateMenuItem("Cut", 'X', QUAL_CTRL));
        editPopup.AddChild(CreateMenuItem("Copy", 'C', QUAL_CTRL));
        editPopup.AddChild(CreateMenuItem("Paste", 'V', QUAL_CTRL));
        editPopup.AddChild(CreateMenuItem("Delete", KEY_DELETE, QUAL_ANY));
        editPopup.AddChild(CreateMenuItem("Select all", 'A', QUAL_CTRL));
        editPopup.AddChild(CreateMenuDivider());
        editPopup.AddChild(CreateMenuItem("Reset position", 0, 0));
        editPopup.AddChild(CreateMenuItem("Reset rotation", 0, 0));
        editPopup.AddChild(CreateMenuItem("Reset scale", 0, 0));
        editPopup.AddChild(CreateMenuItem("Unparent", 'U', QUAL_CTRL));
        editPopup.AddChild(CreateMenuDivider());
        editPopup.AddChild(CreateMenuItem("Toggle update", 'P', QUAL_CTRL));
        uiMenuBar.AddChild(editMenu);
    }

    {
        Menu@ createMenu = CreateMenu("Create");
        Window@ createPopup = createMenu.popup;
        createPopup.AddChild(CreateMenuItem("Box", 0, 0));
        createPopup.AddChild(CreateMenuItem("Cone", 0, 0));
        createPopup.AddChild(CreateMenuItem("Cylinder", 0, 0));
        createPopup.AddChild(CreateMenuItem("Plane", 0, 0));
        createPopup.AddChild(CreateMenuItem("Pyramid", 0, 0));
        createPopup.AddChild(CreateMenuItem("Sphere", 0, 0));
        uiMenuBar.AddChild(createMenu);
    }

    {
        Menu@ fileMenu = CreateMenu("View");
        Window@ filePopup = fileMenu.popup;
        filePopup.AddChild(CreateMenuItem("Scene hierarchy", 'H', QUAL_CTRL));
        filePopup.AddChild(CreateMenuItem("Node / component edit", 'N', QUAL_CTRL));
        filePopup.AddChild(CreateMenuItem("Editor settings", 0, 0));
        uiMenuBar.AddChild(fileMenu);
    }

    UIElement@ spacer = UIElement("MenuBarSpacer");
    uiMenuBar.AddChild(spacer);

    ResizeUI();
}

Menu@ CreateMenuItem(const String&in title, int accelKey, int accelQual)
{
    Menu@ menu = Menu(title);
    menu.style = uiStyle;
    menu.SetLayout(LM_HORIZONTAL, 0, IntRect(4, 0, 4, 0));
    if (accelKey != 0)
        menu.SetAccelerator(accelKey, accelQual);

    Text@ menuText = Text();
    menuText.SetStyle(uiStyle, "EditorMenuText");
    menuText.text = title;
    menu.AddChild(menuText);

    return menu;
}

BorderImage@ CreateMenuDivider()
{
    BorderImage@ divider = BorderImage();
    divider.SetStyle(uiStyle, "EditorDivider");
    divider.SetFixedHeight(4);

    return divider;
}

Window@ CreatePopup(Menu@ baseMenu)
{
    Window@ popup = Window();
    popup.style = uiStyle;
    popup.SetLayout(LM_VERTICAL, 4, IntRect(4, 4, 4, 4));
    baseMenu.popup = popup;
    baseMenu.popupOffset = IntVector2(0, baseMenu.height);

    return popup;
}

Menu@ CreateMenu(const String&in title)
{
    Menu@ menu = CreateMenuItem(title, 0, 0);
    menu.name = "";
    menu.SetFixedWidth(menu.width);
    CreatePopup(menu);

    return menu;
}

void CreateFileSelector(const String&in title, const String&in ok, const String&in cancel, const String&in initialPath, Array<String>@ filters,
    uint initialFilter)
{
    // Within the editor UI, the file selector is a kind of a "singleton". When the previous one is overwritten, also
    // the events subscribed from it are disconnected, so new ones are safe to subscribe.
    uiFileSelector = FileSelector();
    uiFileSelector.style = uiStyle;
    uiFileSelector.title = title;
    uiFileSelector.path = initialPath;
    uiFileSelector.SetButtonTexts(ok, cancel);
    uiFileSelector.SetFilters(filters, initialFilter);

    CenterDialog(uiFileSelector.window);
}

void CloseFileSelector()
{
    uiFileSelector = null;
}

void CreateConsole()
{
    Console@ console = engine.CreateConsole();
    console.style = uiStyle;
    console.numRows = 16;
}

void CreateDebugHud()
{
    engine.CreateDebugHud();
    debugHud.style = uiStyle;
    debugHud.mode = DEBUGHUD_SHOW_NONE;
}

void CenterDialog(UIElement@ element)
{
    IntVector2 size = element.size;
    element.SetPosition((graphics.width - size.x) / 2, (graphics.height - size.y) / 2);
}

void UpdateWindowTitle()
{
    String sceneName = GetFileNameAndExtension(sceneFileName);
    if (sceneName.empty)
        sceneName = "Untitled";
    if (sceneModified)
        sceneName += "*";

    graphics.windowTitle = "Urho3D editor - " + sceneName;
}

Menu@ GetTopLevelMenu(Menu@ menu)
{
    for (;;)
    {
        UIElement@ menuParent = menu.parent;
        if (menuParent is null)
            break;

        Menu@ nextMenu = menuParent.vars["Origin"].GetUIElement();
        if (nextMenu is null)
            break;
        else
            menu = nextMenu;
    }

    if (menu.parent is uiMenuBar)
        return menu;
    else
        return null;
}

void HandleMenuSelected(StringHash eventType, VariantMap& eventData)
{
    Menu@ menu = eventData["Element"].GetUIElement();
    if (menu is null)
        return;

    String action = menu.name;
    if (action.empty)
        return;

    Menu@ topLevelMenu = GetTopLevelMenu(menu);
    // Close the top level menu now
    if (topLevelMenu !is null && action != "Load node") // Instantiate has a submenu, so do not close in that case
        topLevelMenu.showPopup = false;

    if (uiFileSelector is null)
    {
        if (action == "New scene")
            CreateScene();

        if (action == "Open scene")
        {
            CreateFileSelector("Open scene", "Open", "Cancel", uiScenePath, uiSceneFilters, uiSceneFilter);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleOpenSceneFile");
        }

        if (action == "Save scene")
            SaveScene(sceneFileName);

        if (action == "Save scene as")
        {
            CreateFileSelector("Save scene as", "Save", "Cancel", uiScenePath, uiSceneFilters, uiSceneFilter);
            uiFileSelector.fileName = GetFileNameAndExtension(sceneFileName);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleSaveSceneFile");
        }

        if (action == "As replicated")
        {
            instantiateMode = REPLICATED;
            CreateFileSelector("Load node", "Load", "Cancel", uiScenePath, uiSceneFilters, uiSceneFilter);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleLoadNodeFile");
        }

        if (action == "As local")
        {
            instantiateMode = LOCAL;
            CreateFileSelector("Load node", "Load", "Cancel", uiScenePath, uiSceneFilters, uiSceneFilter);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleLoadNodeFile");
        }

        if (action == "Save node as" && selectedNodes.length == 1 && selectedNodes[0] !is editorScene)
        {
            CreateFileSelector("Save node", "Save", "Cancel", uiScenePath, uiSceneFilters, uiSceneFilter);
            uiFileSelector.fileName = GetFileNameAndExtension(instantiateFileName);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleSaveNodeFile");
        }

        if (action == "Import model")
        {
            CreateFileSelector("Import model", "Import", "Cancel", uiImportPath, uiAllFilter, 0);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleImportModel");
        }

        if (action == "Import scene")
        {
            CreateFileSelector("Import scene", "Import", "Cancel", uiImportPath, uiAllFilter, 0);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleImportScene");
        }

        if (action == "Run script")
        {
            CreateFileSelector("Run script", "Run", "Cancel", uiScriptPath, uiScriptFilter, 0);
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleRunScript");
        }

        if (action == "Set resource path")
        {
            CreateFileSelector("Set resource path", "Set", "Cancel", sceneResourcePath, uiAllFilter, 0);
            uiFileSelector.directoryMode = true;
            SubscribeToEvent(uiFileSelector, "FileSelected", "HandleResourcePath");
        }
    }

    if (action == "Reload resources")
        ReloadResources();

    if (action == "Scene hierarchy")
        ShowSceneWindow();

    if (action == "Node / component edit")
        ShowNodeWindow();

    if (action == "Editor settings")
        ShowEditorSettingsDialog();

    if (action == "Cut")
        SceneCut();

    if (action == "Copy")
        SceneCopy();

    if (action == "Paste")
        ScenePaste();

    if (action == "Delete")
        SceneDelete();

    if (action == "Reset position")
        SceneResetPosition();
    
    if (action == "Reset rotation")
        SceneResetRotation();
    
    if (action == "Reset scale")
        SceneResetScale();

    if (action == "Unparent")
        SceneUnparent();

    if (action == "Select all")
        SceneSelectAll();

    if (action == "Toggle update")
        ToggleUpdate();
        
    if (action == "Box" || action == "Cone" || action == "Cylinder" || action == "Plane" || 
        action == "Pyramid" || action == "Sphere")
        CreateBuiltinObject(action);

    if (action == "Exit")
        engine.Exit();
}

void HandleOpenSceneFile(StringHash eventType, VariantMap& eventData)
{
    // Save filter & path for next time
    uiSceneFilter = uiFileSelector.filterIndex;
    uiScenePath = uiFileSelector.path;
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    LoadScene(fileName);
}

void HandleSaveSceneFile(StringHash eventType, VariantMap& eventData)
{
    // Save filter for next time
    uiSceneFilter = uiFileSelector.filterIndex;
    uiScenePath = uiFileSelector.path;
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    SaveScene(fileName);
}

void HandleLoadNodeFile(StringHash eventType, VariantMap& eventData)
{
    // Save filter for next time
    uiSceneFilter = uiFileSelector.filterIndex;
    uiScenePath = uiFileSelector.path;
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    LoadNode(fileName);
}

void HandleSaveNodeFile(StringHash eventType, VariantMap& eventData)
{
    // Save filter for next time
    uiSceneFilter = uiFileSelector.filterIndex;
    uiScenePath = uiFileSelector.path;
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    SaveNode(fileName);
}

void HandleImportModel(StringHash eventType, VariantMap& eventData)
{
    // Save path for next time
    uiImportPath = uiFileSelector.path;
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    ImportModel(fileName);
}

void HandleImportScene(StringHash eventType, VariantMap& eventData)
{
    // Save path for next time
    uiImportPath = uiFileSelector.path;
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    ImportScene(fileName);
}

void HandleRunScript(StringHash eventType, VariantMap& eventData)
{
    // Save path for next time
    uiScriptPath = uiFileSelector.path;
    CloseFileSelector();
    
    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    String fileName = eventData["FileName"].GetString();
    File@ file = File(fileName, FILE_READ);
    if (file.open)
    {
        String scriptCode;
        while (!file.eof)
            scriptCode += file.ReadLine() + "\n";
        file.Close();
        
        if (script.Execute(scriptCode))
            log.Info("Script " + fileName + " run successfully");
    }
}

void HandleResourcePath(StringHash eventType, VariantMap& eventData)
{
    CloseFileSelector();

    // Check for cancel
    if (!eventData["OK"].GetBool())
        return;

    SetResourcePath(eventData["FileName"].GetString(), false);
}

void HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();
    
    if (key == KEY_F1)
        console.Toggle();

    if (key == KEY_ESC)
    {
        UIElement@ front = ui.frontElement;
        if (console.visible)
            console.visible = false;
        else if (uiFileSelector !is null && front is uiFileSelector.window)
            CloseFileSelector();
        else if (front is settingsDialog)
        {
            ui.focusElement = null;
            front.visible = false;
        }
    }

    if (key == KEY_F2)
        ToggleRenderingDebug();
    if (key == KEY_F3)
        TogglePhysicsDebug();
    if (key == KEY_F4)
        ToggleOctreeDebug();

    if (eventData["Qualifiers"].GetInt() == QUAL_CTRL)
    {
        if (key == '1')
            editMode = EDIT_MOVE;
        else if (key == '2')
            editMode = EDIT_ROTATE;
        else if (key == '3')
            editMode = EDIT_SCALE;
        else if (key == '4')
            axisMode = AxisMode(axisMode ^ AXIS_LOCAL);
        else if (key == '5')
        {
            --pickMode;
            if (pickMode < PICK_GEOMETRIES)
                pickMode = MAX_PICK_MODES - 1;
        }
        else if (key == '6')
        {
            ++pickMode;
            if (pickMode >= MAX_PICK_MODES)
                pickMode = PICK_GEOMETRIES;
        }
        else if (key == '7')
        {
            fillMode = FillMode(fillMode + 1);
            if (fillMode > FILL_POINT)
                fillMode = FILL_SOLID;
        }
        else
            SteppedObjectManipulation(key);
    }
}
