#pragma once

/// Controls symbol visibility when building or consuming the GeometryEngine library.
#ifdef _WIN32
#ifdef GeometryEngine
#define GeometryEngineAPI __declspec(dllexport)
#elif defined(GEOMETRYENGINE_STATIC)
#define GeometryEngineAPI
#else
#define GeometryEngineAPI __declspec(dllimport)
#endif
#else // Linux
#define GeometryEngineAPI __attribute__((visibility("default")))
#endif
