#include "ResTools.hpp"

namespace EgLab
{

#ifdef WNT
    bool ExportToFile(const std::wstring& exportFilePath, const void* pBuffer, DWORD bufferLength) {

    }

    bool ExportRes(const std::wstring& exportPath, DWORD resourceId) {
        HINSTANCE hInstance = GetModuleHandle(NULL);
        HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(resourceId),
        L"shader"
    );

    if(!hRes) return false;
    HGLOBAL hGlobal = LoadResource(hInstance, hRes);
    if(!hGobal) return false;

    const void* pData = LockResource(hGlobal);
    DWORD size = SizeofResource(hInstance, hRes);
    
    bool success = ExportToFile(exportPath, pData, size);
    FreeResource(hGlboal);

    return success;
    }

#else

#endif
}