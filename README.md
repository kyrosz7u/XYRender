# XYRender
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE) 

XYRender是使用C++17+Vulkan1.3实现的一个高性能实时渲染器，可以用来学习和实战现代图形API开发，或者用于开发游戏引擎。
## Features
- 自带完整的第三方库，包括Vulkan SDK、GLFW、stb_image、assimp、imgui、spdlog等，无需手动下载和配置
- 自带glsl shader编译脚本，只需将shader文件放入shaders目录即可编译，且能够自动检测文件变化并重新编译，渲染器编译后会自动加载最新的shader，无需手动管理shader加载
- 具有完整的一套渲染管线，包括平行光shadowmap，前向渲染和延迟渲染两种光照计算方法，天空盒绘制、UI Pass
- 窗口管理、输入系统、日志系统、资源加载器、渲染器管理、UI系统等均已封装，使用简单
- 可以使用IMGUI进行调试，支持在IMGUI中渲染器信息、相机参数等
- 支持Vulkan的一些高级特性，如多线程渲染、One Pass Defer、Descriptor Indexing、Dynamic Buffer等
- 组件化的封装了图形API，可以方便的添加新的渲染器、新的Pass、新的渲染资源类型等
## 系统要求

要构建此XYRender，您必须首先确保您的GPU支持Vulkan 1.3 或更高版本（在[此处](https://vulkan.gpuinfo.org/listdevices.php)检查您的GPU支持）并安装以下工具:

### Windows 10/11
- Visual Studio 2019 (or more recent)
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

### macOS >= 11 (arm64 only)
- Xcode 13.0 (or more recent)
- CMake 3.19 (or more recent)
- Git 2.1 (or more recent)

## 构建XYRender


## 文档

## TODO List
- [ ] 优化渲染器的组件化，使得渲染器的添加更加方便
- [ ] 点光源shadowmap
- [ ] PBR渲染

## References
