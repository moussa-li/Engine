#pragma once

/// Controls symbol visibility when building or consuming the RenderEngine library.
#ifdef _WIN32
#ifdef RenderEngine
#define RenderEngineAPI __declspec(dllexport)
#elif defined(RENDERENGINE_STATIC)
#define RenderEngineAPI
#else
#define RenderEngineAPI __declspec(dllimport)
#endif
#else // Linux
#define RenderEngineAPI __attribute__((visibility("default")))
#endif