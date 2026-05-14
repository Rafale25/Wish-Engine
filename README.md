
# Setup

### Macos
```bash
cmake --preset clang-debug \
-DCMAKE_PREFIX_PATH="$(brew --prefix vulkan-loader);$(brew --prefix vulkan-headers)" \
-DCMAKE_BUILD_RPATH="$(brew --prefix)/lib"
```
