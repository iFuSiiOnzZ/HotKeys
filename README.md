# HotKeys

Keys that can be bind <br />
&nbsp;&nbsp;&nbsp;&nbsp;Special Keys: CTRL, ALT, SHIFT  <br />
&nbsp;&nbsp;&nbsp;&nbsp;Normal Keys: A - Z, 0 - 9  <br />

Normal keys letters have to be uppercase always, letters and numbers are mutually exclusive. HotKeys are configured through a json file (hotkeys.json) that has to be in the same directory as the application.
If json file is modified during the execution there is no need to restart the application, the json file is automatically parsed again.

## hotkeys.json
```
[
    {
        "name" : "Windows console (cmd.exe)",
        "hotkeys" : "CTRL+ALT+T",
        
        "path" : "c:\\windows\\system32\\cmd.exe",
        "startin" : "c:\\windows\\system32"
    },
    {
        "name" : "Sublimete Text 3",
        "hotkeys" : "CTRL+ALT+S",

        "path" : "C:\\Program Files\\Sublime Text 3\\sublime_text.exe",
        "startin" : "C:\\Program Files\\Sublime Text 3\\"
    },
    {
        "name" : "EA Origin",
        "hotkeys" : "CTRL+SHIFT+O",

        "path" : "C:\\Program Files (x86)\\Origin\\Origin.exe",
        "startin" : "C:\\Program Files (x86)\\Origin"
    },
    {
        "name" : "Valve Steam",
        "hotkeys" : "CTRL+SHIFT+S",

        "path" : "C:\\Program Files (x86)\\Steam\\Steam.exe",
        "startin" : "C:\\Program Files (x86)\\Steam"
    }
]
```