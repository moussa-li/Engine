#include "ResTools.hpp"
#include <stdlib.h>

namespace EgLab
{

#ifdef WNT
    bool ExportToFile(const std::wstring& exportFilePath, const void* pBuffer, DWORD bufferLength) {
        return false;
    }

    bool ExportRes(const std::wstring& exportPath, DWORD resourceId) {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HRSRC hRes = FindResourceW(hInstance, MAKEINTRESOURCEW(resourceId),
            L"shader"
        );

        if(!hRes) return false;
        HGLOBAL hGlobal = LoadResource(hInstance, hRes);
        if(!hGlobal) return false;

        const void* pData = LockResource(hGlobal);
        DWORD size = SizeofResource(hInstance, hRes);
    
        bool success = ExportToFile(exportPath, pData, size);
        FreeResource(hGlobal);

        return success;
    }

#else

#endif
}