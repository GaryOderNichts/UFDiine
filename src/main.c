#include <coreinit/filesystem_fsa.h>
#include <coreinit/ios.h>
#include <coreinit/mcp.h>
#include <coreinit/screen.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <mocha/mocha.h>
#include <vpad/input.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_console.h>
#include <whb/proc.h>

#define OSScreenClearBuffer(color) ({ \
    OSScreenClearBufferEx(SCREEN_TV, color); \
    OSScreenClearBufferEx(SCREEN_DRC, color); \
})

#define OSScreenFlipBuffers() ({ \
    OSScreenFlipBuffersEx(SCREEN_TV); \
    OSScreenFlipBuffersEx(SCREEN_DRC); \
})

#define OSScreenPutFont(row, column, buffer) ({ \
    OSScreenPutFontEx(SCREEN_TV, row, column, buffer); \
    OSScreenPutFontEx(SCREEN_DRC, row, column, buffer); \
})

#define UPDATE_FOLDER_PATH "/vol/storage_mlc01/sys/update"

int gClient = -1;

BOOL updateFolderExists() {
    FSADirectoryHandle handle = -1;
    if (FSAOpenDir(gClient, UPDATE_FOLDER_PATH, &handle) != FS_ERROR_OK) {
        return FALSE;
    }

    FSACloseDir(gClient, handle);
    return TRUE;
}

void createUpdateFolder() {
    FSAMakeDir(gClient, UPDATE_FOLDER_PATH, 0777);
}

void deleteUpdateFolder() {
    FSARemove(gClient, UPDATE_FOLDER_PATH);
}

void drawMenu() {
    OSScreenClearBuffer(0);

    OSScreenPutFont(0, 0, "*************************************************");
    OSScreenPutFont(0, 1, "      UFDiine (Update Folder Deleter) v2.0");
    OSScreenPutFont(0, 2, "   Block updates by deleting the update folder   ");
    OSScreenPutFont(0, 3, "*************************************************");

    if (updateFolderExists()) {
        OSScreenPutFont(0, 4, "Update folder exists");
        OSScreenPutFont(0, 6, "Press A to delete the update folder");
    } else {
        OSScreenPutFont(0, 4, "Update folder is deleted");
        OSScreenPutFont(0, 6, "Press A to create the update folder");
    }

    OSScreenPutFont(0, 8, "Press HOME to exit");

    OSScreenFlipBuffers();
}

int main(int argc, char **argv) {
    WHBProcInit();
    WHBLogConsoleInit();
    WHBLogCafeInit();

    if (Mocha_InitLibrary() != MOCHA_RESULT_SUCCESS) {
        WHBLogPrintf("Mocha_InitLibrary failed");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(3000));
        goto exit;
    }


    FSAInit();
    gClient = FSAAddClient(NULL);
    if (gClient == 0) {
        WHBLogPrintf("Failed to add FSAClient");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(3000));
        goto exit;
    }
    if (Mocha_UnlockFSClientEx(gClient) != MOCHA_RESULT_SUCCESS) {
        FSADelClient(gClient);
        WHBLogPrintf("Failed to add FSAClient");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(3000));
        goto exit;
    }

    drawMenu();

    VPADStatus buffer;
    while (WHBProcIsRunning()) {
        VPADRead(VPAD_CHAN_0, &buffer, 1, NULL);

        if (buffer.trigger & VPAD_BUTTON_A) {
            if (updateFolderExists()) {
                deleteUpdateFolder();
            } else {
                createUpdateFolder();
            }
        }
        drawMenu();
        OSSleepTicks(OSMillisecondsToTicks(100));
    }

    FSAFlushVolume(gClient, "/vol/storage_mlc01");

    FSADelClient(gClient);

    Mocha_DeInitLibrary();


exit:
    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}