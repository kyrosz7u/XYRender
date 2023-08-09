# XYRender
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/Naereen/StrapDown.js/blob/master/LICENSE) 

XYRender是使用C++17+Vulkan1.3实现的一个高性能实时渲染器，可以用来学习和实战现代图形API开发，或者用于开发游戏引擎。
## 特性
- 自带完整的第三方库，包括Vulkan SDK、GLFW、stb_image、assimp、imgui、spdlog等，无需手动下载和配置
- 自带glsl shader编译脚本，只需将shader文件放入shaders目录即可编译，且能够自动检测文件变化并重新编译，渲染器编译后会自动加载最新的shader，无需手动管理shader加载
- 具有完整的一套渲染管线，包括平行光shadowmap，前向渲染和延迟渲染两种光照计算方法，天空盒绘制、UI Pass
- 窗口管理、输入系统、日志系统、资源加载器、渲染器管理、UI系统等均已封装，使用简单
- 可以使用IMGUI进行调试，支持在IMGUI中渲染器信息、相机参数等
- 支持Vulkan的一些高级特性，如多线程渲染、One Pass Defer、Descriptor Indexing、Dynamic Buffer等
- 组件化的封装了图形API，可以方便的添加新的渲染器、新的Pass、新的渲染资源类型等
- 渲染器采用左手坐标系，支持导入obj和fbx模型，能够自动加载模型中的材质纹理

![系统架构图](images/arch.png)

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
首先确保已经安装了上述的所有工具，并在本地克隆仓库，然后按照以下步骤构建XYRender
### Windows 10/11
#### 使用Visual Studio 2019构建
1. 在XYRender目录下新建build文件夹，用于存放构建文件，然后打开build文件夹，在此处打开CMD窗口，输入以下命令
```shell
cmake -G "Visual Studio 16 2019" -A x64 ..
```
2. 打开build文件夹，双击XYRender.sln，选择Release模式，右键ALL_BUILD，点击生成
3. 设置工作路径为XYRender项目所在根目录
4. 运行XYRender.exe

#### 使用Clion构建
1. 打开Clion，选择Open，选择XYRender目录，点击OK
2. 点击Build->Build Project
3. 设置工作路径为XYRender项目所在根目录
4. 点击运行按钮

### macOS >= 11 (arm64 only)

#### 使用Xcode构建
1. 在XYRender目录下新建build文件夹，用于存放构建文件，然后打开build文件夹，在此处打开终端，输入以下命令
```shell
cmake -G "Xcode" ..
```
2. 打开build文件夹，双击XYRender.xcodeproj，选择Release模式，点击Build
3. 设置工作路径为XYRender项目所在根目录
4. 运行XYRender.app

#### 使用Clion构建
与Windows相同

## 使用说明
### 修改渲染管线
可以在CMakeLists.txt中修改渲染管线，只需要增加宏定义即可，如下所示
```cmake
add_definitions(-DFORWARD_RENDERING)  # 前向渲染
add_definitions(-DDEFERRED_RENDERING) # 延迟渲染
```

### 开启多线程渲染
只需要在CMakeLists.txt中增加宏定义即可，如下所示
```cmake
add_definitions(-DENABLE_MULTITHREADED_RENDERING)  # 开启多线程渲染
```

### 模型和贴图导入说明
模型和贴图均放在assets目录下，模型支持obj和fbx格式，贴图支持jpg、png、bmp、tga等格式，模型和贴图的导入均使用assimp库，导入后会自动加载模型中的材质贴图，如果模型中没有贴图，则会加载失败，模型和贴图的导入代码在scene/model.cpp中，可以根据自己的需求修改导入代码

模型统一存放在assets/models目录下，贴图统一存放在assets/textures目录下，贴图的会根据模型名和其中的材质名进行匹配查找，比如模型名为test.obj，其中有一个材质名为mat1，那么会自动加载assets/textures/test目录下的mat1.png。以下是渲染器自带的模型和贴图的文件结构，可以作为参考和测试用：
```
assets
├── fonts
│   └── PingFang Regular.ttf
├── models
│   ├── capsule.obj
│   ├── cube.obj
│   ├── cylinder.obj
│   ├── Kong.fbx
└── textures
    ├── capsule
    │   └── Default.png
    ├── cube
    │   └── Default.png
    ├── cylinder
    │   └── Default.png
    └── Kong
        ├── Cloth.png
        ├── Emoji.png
        ├── Eye.png
        ├── Face.png
        ├── Face_Unit.png
        ├── Hair.png
        └── TCP2_CustomRamp.png
```
## 结果展示
![结果展示](images/demo.png)

## TODO List
- [ ] 优化渲染器的组件化，使得渲染器的添加更加方便
- [ ] 点光源shadowmap
- [ ] PBR渲染
- [ ] Mac x86_64支持

## License
XYRender is licensed under the [MIT License](LICENSE). 

## References
