#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <coreinit/ios.h>
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <coreinit/screen.h>
#include <vpad/input.h>
#include <iosuhax.h>

#define UPDATE_FOLDER_PATH "/vol/storage_mlc01/sys/update"

int fsaFd = -1;

//just to be able to call async
void someFunc(IOSError err, void *arg){(void)arg;}

static int mcp_hook_fd = -1;
int MCPHookOpen()
{
	//take over mcp thread
	mcp_hook_fd = MCP_Open();
	if(mcp_hook_fd < 0)
		return -1;
	IOS_IoctlAsync(mcp_hook_fd, 0x62, (void*)0, 0, (void*)0, 0, someFunc, (void*)0);
	//let wupserver start up
	OSSleepTicks(OSMillisecondsToTicks(500));
	if(IOSUHAX_Open("/dev/mcp") < 0)
		return -1;
	return 0;
}

void MCPHookClose()
{
	if(mcp_hook_fd < 0)
		return;
	//close down wupserver, return control to mcp
	IOSUHAX_Close();
	//wait for mcp to return
	OSSleepTicks(OSMillisecondsToTicks(500));
	MCP_Close(mcp_hook_fd);
	mcp_hook_fd = -1;
}

BOOL updateFolderExists()
{
    int handle;
    IOSUHAX_FSA_OpenDir(fsaFd, UPDATE_FOLDER_PATH, &handle);
    if (handle < 0)
        return FALSE;

    IOSUHAX_FSA_CloseDir(fsaFd, handle);
    return TRUE;
}

void createUpdateFolder()
{
    IOSUHAX_FSA_MakeDir(fsaFd, UPDATE_FOLDER_PATH, 0755);
}

void deleteUpdateFolder()
{
    IOSUHAX_FSA_Remove(fsaFd, UPDATE_FOLDER_PATH);
}

void drawMenu()
{
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

    if (updateFolderExists())
    {
        OSScreenPutFontEx(SCREEN_TV, 0, 4, "Update folder exists");
        OSScreenPutFontEx(SCREEN_DRC, 0, 4, "Update folder exists");

        OSScreenPutFontEx(SCREEN_TV, 0, 6, "Press A to delete the update folder");
        OSScreenPutFontEx(SCREEN_DRC, 0, 6, "Press A to delete the update folder");
    }
    else
    {
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

int main(int argc, char** argv)
{
    WHBProcInit();
    WHBLogConsoleInit();

	int res = IOSUHAX_Open(NULL);
	if (res < 0)
		res = MCPHookOpen();
	if(res < 0)
	{
        WHBLogPrintf("Error opening IOSUHAX");
        WHBLogPrintf("Make sure to run CFW");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(3000));
        goto exit;
	}

	fsaFd = IOSUHAX_FSA_Open();
	if(fsaFd < 0)
	{
        WHBLogPrintf("IOSUHAX_FSA_Open Error");
        WHBLogConsoleDraw();
        OSSleepTicks(OSMillisecondsToTicks(3000));
        goto exit;
	}

    drawMenu();

    VPADStatus buffer;
    while (WHBProcIsRunning())
    {
        VPADRead(VPAD_CHAN_0, &buffer, 1, NULL);
        
        if (buffer.trigger & VPAD_BUTTON_A)
        {
            if (updateFolderExists())
            {
                deleteUpdateFolder();
            }
            else
            {
                createUpdateFolder();
            }
            drawMenu();
        }
    }

    if(mcp_hook_fd >= 0)
		MCPHookClose();
	else
		IOSUHAX_Close();

exit:
    WHBLogConsoleFree();
    WHBProcShutdown();
    return 0;
}