# HotKeys

Keys that can be bind:
    Special Keys: CTRL, ALT
    Normal Keys: A - Z

Normal keys have to be uppercase always. HotKeys are configured through a json (hotkeys.json) file that has to be in the same directory as the application. 

## hotkeys.json
```
[
    {
        "name" : "Windows console (cmd.exe)",
        "hotkeys" : "CTRL+ALT+T",
        "path" : "c:\\windows\\system32\\cmd.exe"
    },
    {
        "name" : "Sublimete Text 3",
        "hotkeys" : "CTRL+ALT+S",
        "path" : "C:\\Program Files\\Sublime Text 3\\sublime_text.exe"
    }
]
```