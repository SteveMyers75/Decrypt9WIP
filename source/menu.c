#include "menu.h"
#include "draw.h"
#include "hid.h"
#include "fs.h"


void ProcessMenu(MenuInfo* info, u32 nMenus) {
    char* buttonName[] = { "A", "B", "X", "Y", "L", "R", "\x18", "\x19", "\x1A", "\x1B" };
    u32 buttonCode[] = { BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y, BUTTON_L1, BUTTON_R1, BUTTON_UP, BUTTON_DOWN, BUTTON_RIGHT, BUTTON_LEFT };
    MenuInfo* currMenu = info;
    MenuInfo* nextMenu = (nMenus > 1) ? info + 1 : NULL;
    bool drawMenu = true;
    
    while (true) {
        if (drawMenu) {
            DebugClear();
            Debug("%s", currMenu->name);
            Debug("--------------------");
            for (u32 i = 0; i < 4; i++) {
                char* name = currMenu->entries[i].name;
                if (name != NULL)
                    Debug("%s: %s", buttonName[i], name);
            }
            Debug("--------------------");
            Debug("");
            if (nextMenu != NULL) 
                Debug("R: %s", nextMenu->name);
            Debug("START: Reboot");
            Debug("");
            Debug("");
            #ifdef WORKDIR
            Debug("Working directory: %s", WORKDIR);
            #endif
            Debug("Remaining SD storage space: %llu MiB", RemainingStorageSpace() / 1024 / 1024);
            drawMenu = false;
        }
       
        u32 pad_state = InputWait();
        for (u32 i = 0; i < 4; i++) {
            char* name = currMenu->entries[i].name;
            u32 (*function)(void) = currMenu->entries[i].function;
            u32 dangerous = currMenu->entries[i].dangerous;
            if ((pad_state & buttonCode[i]) && (name != NULL) && (function != NULL)) {
                if (dangerous) {
                    u32 unlockSequence[] = { BUTTON_LEFT, BUTTON_RIGHT, BUTTON_DOWN, BUTTON_UP };
                    u32 unlockLvl = 0;
                    DebugClear();
                    Debug("You selected \"%s\".", name);
                    Debug("This feature is potentially dangerous!");
                    Debug("If you understand and wish to proceed, enter:");
                    Debug("<Left>, <Right>, <Down>, <Up>");
                    Debug("");
                    Debug("(SELECT to return, START to reboot)");
                    while (true) {
                        ShowProgress(unlockLvl, 4);
                        if (unlockLvl == 4)
                            break;
                        pad_state = InputWait();
                        if (pad_state & unlockSequence[unlockLvl])
                            unlockLvl++;
                        else if (pad_state & (BUTTON_SELECT | BUTTON_START))
                            break;
                    }
                    ShowProgress(0, 0);
                    if (unlockLvl < 4) {
                        drawMenu = true;
                        break;
                    }
                }
                DebugClear();
                Debug("%s: %s!", name, (*function)() == 0 ? "succeeded" : "failed");
                Debug("");
                Debug("Press SELECT to return, START to reboot.");
                while(!(pad_state = InputWait() & (BUTTON_SELECT | BUTTON_START)));
                drawMenu = true;
                break;
            }
        }
        if ((pad_state & BUTTON_R1) && (nextMenu != NULL)) {
            currMenu = nextMenu;
            if (++nextMenu - info >= nMenus) nextMenu -= nMenus;
            drawMenu = true;
        } else if (pad_state & BUTTON_START) {
            break;
        }
    }
}
