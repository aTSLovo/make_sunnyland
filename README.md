- engine/core 引擎核心组件
GameAPP文件控制主循环
每次更新调用下面三个函数
handleEvenets
update
render
init函数调用initSDL、initTime、initResourceManager、initCamera、initRenderer(传入两个指针)
完成SDL窗口、渲染器初始化，时间初始化，资源管理模块初始化
init中使用testResourceManager测试资源管理模块
update中使用testCamera测试相机模块
render中使用testRenderer测试渲染器模块

帧率控制在Time文件中
delta_time记录帧间时间差，用于记录在不同帧率下每帧不同的时间
在Unity中，Update函数每帧调用一次，如果直接按每帧移动物体，帧率不同会导致物体移动速度不一致。
通过将移动量乘以delta_time，可以将每帧的变化量转换为每秒的变化量
从而保证无论帧率高低，物体每秒移动的距离保持一致。

Config管理配置文件
如果存在config.json文件，那么从json文件中读取配置并设置类中pubilc成员变量
如不存在，那么将现有默认配置转换成json对象，再写入json文件中

Context管理上下文

- engine/resource 游戏引擎资源管理组件
音乐资源管理
SDL3_mixer更新后API变更，原Chunk和Music数据文件统一用Audio数据存储
需要创建MIX_Mixer，再在当前mixer上创建MIX_Track(轨道)，在轨道上播放音频
字体资源模块、材质资源模块
通用loadXXX、getXXX、unloadXXX、clearXXX进行资源管理
资源使用unordered_map，根据键值对存储，unique_ptr自定义删除器完成资源清理
使用ResourceManager类统一管理资源，实现不同资源的指针以及调用接口，这样在GameApp里用指针管理ResourceManager就可以管理所有资源了

- engine/render
Renderer类
构造函数传入裸指针SDL_Renderer* engine::resource::ResourceManager*,这俩个指针由GameApp管理，为了方便，这里只是拿过来用
draw逻辑
1. 根据精灵的材质id获取对应材质
2. 获取精灵的材质矩形
3. 绘画

Camera类
worldToScreen 把游戏大世界里的坐标，转换成屏幕上显示的坐标。可以认为屏幕左上角是(0,0)，得到是相对位置
screenToWorld 将屏幕点击的坐标，转换为游戏世界的坐标
worldToScreenWithParallax 2D 视差滚动背景

- engine/input
  InputManager类将Config类中的 动作->按键 映射复制一份
  把映射动作转成动作状态
  计算映射按键对应SDL的代码并将代码作为key存储动作，方便updateActionState

- engine/component
  Component作为抽象基类，后续所有组件都继承它
  SpriteComponent管理GameObject的视觉表示，通过持有一个 Sprite 对象
  TransformComponent管理GameObject的位置、旋转和缩放

- engine/object
  管理游戏对象的组件，并提供添加、获取、检查和移除组件的功能