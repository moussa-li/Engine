#pragma once

#ifdef MeshEngine
#ifdef _WIN32
#ifdef MeshEngine
#define MeshEngineAPI __declspec(dllexport)
#else
#define MeshEngineAPI __declspec(dllimport)
#endif
#else // Linux
#define CommonAPI __attribute__((visibility("default")))
#endif
#else
#define MeshEngineAPI
#endif