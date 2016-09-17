# HotKeys

Keys that can be bind <br />
..Special Keys: CTRL, ALT, SHIFT  <br />
..Normal Keys: A - Z  <br />

Normal keys have to be uppercase always. HotKeys are configured through a json file (hotkeys.json) that has to be in the same directory as the application.
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
        "startin" : "c:\\windows\\system32"
    }
]
```