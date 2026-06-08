#pragma once

#ifndef PROJECT_ASSET_DIR
#error "PROJECT_ASSET_DIR must be defined by the build system"
#endif

#define ASSET_PATH(relativePath) PROJECT_ASSET_DIR "/" relativePath