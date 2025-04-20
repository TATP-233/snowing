# 飘雪动画程序技术报告

## 1. 项目依赖

本项目是一个基于现代OpenGL技术开发的雪花飘落动画程序，主要依赖以下库和工具：

### 核心依赖

- **OpenGL**: 提供底层图形渲染API
- **GLEW (OpenGL Extension Wrangler Library)**: 用于加载和管理OpenGL扩展，简化了对现代OpenGL特性的访问
- **GLFW (Graphics Library Framework)**: 提供窗口创建、输入处理和渲染上下文管理的跨平台解决方案

### 图像处理依赖

- **stb_image.h**: 单头文件图像加载库，用于加载纹理图像（背景和雪花）
  - 支持多种图像格式（PNG、JPEG等）
  - 提供图像数据处理功能

### 编译和构建工具

- **CMake 3.10+**: 跨平台构建系统
- **C++17 编译器**: 使用现代C++特性，提高代码可读性和性能

### 系统要求

- **操作系统**: Windows、macOS 或 Linux
- **图形硬件**: 支持OpenGL 2.1及以上的图形卡和驱动

## 2. 着色器(Shader)实现

虽然本项目没有使用独立的着色器文件，而是使用了OpenGL固定管线功能，但仍然实现了多种渲染效果：

### 纹理渲染

程序使用OpenGL的纹理映射功能，通过以下步骤实现：

1. **纹理加载**:
   ```cpp
   unsigned int loadTexture(const char* path)
   {
       unsigned int textureID;
       glGenTextures(1, &textureID);
       // 加载图像数据...
       glBindTexture(GL_TEXTURE_2D, textureID);
       // 配置纹理参数...
       return textureID;
   }
   ```

2. **纹理参数设置**:
   ```cpp
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   ```

3. **透明度处理**:
   对于雪花纹理，程序通过分析像素亮度动态计算Alpha通道，实现平滑的透明度过渡：
   ```cpp
   unsigned char brightness = (data[i * 3 + 0] + data[i * 3 + 1] + data[i * 3 + 2]) / 3;
   float alpha = 1.0f - (brightness / 255.0f) * 0.7f;
   alpha = alpha * 1.5f;
   if (alpha > 1.0f) alpha = 1.0f;
   tempData[i * 4 + 3] = static_cast<unsigned char>(alpha * 255);
   ```

### 混合模式

为了实现透明效果，程序配置了适当的混合函数：
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

### 变换和矩阵操作

雪花的渲染利用了OpenGL的矩阵栈操作，实现位置、旋转和缩放的组合变换：
```cpp
glPushMatrix();
glTranslatef(flake.x, flake.y, 0.0f);
glRotatef(flake.angle, 0.0f, 0.0f, 1.0f);
glScalef(flake.size, flake.size, 1.0f);
// 渲染雪花...
glPopMatrix();
```

## 3. 雪花粒子运动公式

雪花的运动模拟采用了物理学中的多种力学模型，包括重力、风力和空气阻力，实现了逼真的飘落效果。

### 初始化参数

每个雪花在初始化时获得随机的属性，确保视觉效果的多样性：
```cpp
void init() {
    x = static_cast<float>(rand() % SCR_WIDTH) / SCR_WIDTH * 3.0f - 1.5f;
    y = 1.1f;  // 屏幕顶部以上一点
    
    vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
    vy = -0.001f - static_cast<float>(rand()) / RAND_MAX * 0.3f;
    
    size = 0.02f + static_cast<float>(rand()) / RAND_MAX * 0.08f;
    
    angle = static_cast<float>(rand() % 360);
    rotation_speed = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
}
```

### 运动更新算法

雪花位置的更新基于时间增量(dt)，应用以下物理公式：

1. **重力作用**:
   ```cpp
   float gravity = -0.98f * 2.0f;  // 修改为负值，使雪花向下加速
   vy += gravity * dt;  // 施加重力
   ```

2. **风力效果**:
   风力模拟使用正弦函数创建横向摆动效果，使雪花飘落路径更自然：
   ```cpp
   float wind = sin((x * 0.2f + y * 1.0f - 0.5f) * M_PI * 2.0f) * 0.01f;
   vx += wind;
   ```
   
   这个公式中：
   - `x * 0.2f + y * 1.0f - 0.5f` 创建风力场的空间变化
   - `M_PI * 2.0f` 控制风力波动的频率
   - `0.01f` 调整风力的强度

3. **空气阻力**:
   ```cpp
   float air_resistance = 0.98f;
   vx *= air_resistance;
   vy *= air_resistance;
   ```
   空气阻力系数(0.98)使雪花的速度每帧减少约2%，模拟了空气对雪花运动的阻尼效果。

4. **位置更新**:
   ```cpp
   x += vx * dt;
   y += vy * dt;
   ```

5. **旋转角度更新**:
   ```cpp
   angle += rotation_speed * dt;
   ```

6. **边界处理**:
   当雪花飘出屏幕时，重新从顶部开始飘落，形成连续的雪花效果：
   ```cpp
   if (y < -1.1f || x < -1.1f || x > 1.1f) {
       x = static_cast<float>(rand() % SCR_WIDTH) / SCR_WIDTH * 2.0f - 1.0f;
       y = 1.1f;
       vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.001f;
       vy = -0.001f - static_cast<float>(rand()) / RAND_MAX * 0.003f;
   }
   ```

### 雪花数量动态调整

为增强视觉效果，程序逐渐增加雪花数量：
```cpp
if (currentMaxSnowflakes < MAX_SNOWFLAKES && 
    currentTime - lastSnowflakeIncrease > 1.0) { // 每1秒增加一次
    currentMaxSnowflakes += snowflakeIncreaseRate;
    // 添加新雪花...
}
```

## 总结

本项目通过结合OpenGL渲染技术和物理模拟算法，实现了逼真的雪花飘落效果。雪花的随机参数、物理运动模型和透明度处理共同创造出一个视觉上令人愉悦的动态场景。特别是通过风力模拟和空气阻力的应用，使雪花的运动更加自然，而不是简单的线性下落。 