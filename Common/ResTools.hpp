#pragma once

#ifdef WNT
#include <windows.h>
#endif

#include <string>

namespace EgLab
{
#ifdef WNT
    /**
     * @brief export the res to file
     * 
     * @param exportFilePath 
     * @param pBuffer 
     * @param bufferLength 
     * @return true 
     * @return false 
     */
    bool ExportToFile(const std::wstring& exportFilePath, const void* pBuffer, DWORD bufferLength);

    /**
     * @brief export the resouce files
     * 
     * @param exportPath 
     * @param resourceId 
     * @return true 
     * @return false 
     */
    bool ExportRes(const std::wstring& exportPath, DWORD resourceId);
    

#else

#endif
}