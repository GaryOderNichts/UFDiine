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
    OSScreenClearBufferEx(SCREEN_TV, 0);
    OSScreenClearBufferEx(SCREEN_DRC, 0);

    OSScreenPutFontEx(SCREEN_TV, 0, 0, "*************************************************");
    OSScreenPutFontEx(SCREEN_TV, 0, 1, "UFDiine (Update Folder Deleter) by GaryOderNichts");
    OSScreenPutFontEx(SCREEN_TV, 0, 2, "   Block updates by deleting the update folder   ");
    OSScreenPutFontEx(SCREEN_TV, 0, 3, "*************************************************");

    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "*************************************************");
    OSScreenPutFontEx(SCREEN_DRC, 0, 1, "UFDiine (Update Folder Deleter) by GaryOderNichts");
    OSScreenPutFontEx(SCREEN_DRC, 0, 2, "   Block updates by deleting the update folder   ");
    OSScreenPutFontEx(SCREEN_DRC, 0, 3, "*************************************************");

    if (updateFolderExists()) {
        OSScreenPutFontEx(SCREEN_TV, 0, 4, "Update folder exists");
        OSScreenPutFontEx(SCREEN_DRC, 0, 4, "Update folder exists");

        OSScreenPutFontEx(SCREEN_TV, 0, 6, "Press A to delete the update folder");
        OSScreenPutFontEx(SCREEN_DRC, 0, 6, "Press A to delete the update folder");
    } else {
        OSScreenPutFontEx(SCREEN_TV, 0, 4, "Update folder is deleted");
        OSScreenPutFontEx(SCREEN_DRC, 0, 4, "Update folder is deleted");

        OSScreenPutFontEx(SCREEN_TV, 0, 6, "Press A to create the update folder");
        OSScreenPutFontEx(SCREEN_DRC, 0, 6, "Press A to create the update folder");
    }

    OSScreenPutFontEx(SCREEN_TV, 0, 8, "Press HOME to exit");
    OSScreenPutFontEx(SCREEN_DRC, 0, 8, "Press HOME to exit");

    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
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