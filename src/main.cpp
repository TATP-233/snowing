#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

// 窗口大小
const unsigned int SCR_WIDTH = 1586 / 2;
const unsigned int SCR_HEIGHT = 1186 / 2;

// 纹理ID
unsigned int backgroundTexture, snowTexture;

// 雪花参数
const int MAX_SNOWFLAKES = 100;  // 最大雪花数量
int currentMaxSnowflakes = 20;   // 当前允许的最大雪花数
int snowflakeIncreaseRate = 10;   // 每次增加的雪花数
int lastSnowflakeIncrease = 0;    // 上次增加雪花的时间

// 雪花结构体
struct Snowflake {
    float x, y;           // 位置
    float vx, vy;         // 速度
    float size;           // 大小
    float angle;          // 旋转角度
    float rotation_speed; // 旋转速度
    
    // 初始化雪花
    void init() {
        x = static_cast<float>(rand() % SCR_WIDTH) / SCR_WIDTH * 3.0f - 1.5f;  // 范围在 -1.0 到 1.0
        y = 1.1f;  // 屏幕顶部以上一点
        
        // 横向速度较小，纵向速度随机（确保是负值，向下移动）
        vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
        vy = -0.001f - static_cast<float>(rand()) / RAND_MAX * 0.3f;
        
        // 随机大小
        size = 0.02f + static_cast<float>(rand()) / RAND_MAX * 0.08f;
        
        // 随机初始角度和旋转速度
        angle = static_cast<float>(rand() % 360);
        rotation_speed = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
    }
    
    // 更新雪花位置
    void update(float dt) {
        // 模拟重力和空气阻力
        float gravity = -0.98f * 2.0f;  // 修改为负值，使雪花向下加速
        float air_resistance = 0.98f;
        
        // 横向摆动（模拟风的效果）
        float wind = sin((x * 0.2f + y * 1.0f - 0.5f) * M_PI * 2.0f) * 0.01f;
        vx += wind;
        
        vy += gravity * dt;  // 施加重力
        
        // 应用空气阻力
        vx *= air_resistance;
        vy *= air_resistance;
        
        // 更新位置
        x += vx * dt;
        y += vy * dt;
        
        // 更新旋转角度
        angle += rotation_speed * dt;
        
        // 如果雪花飘出屏幕底部，重新从顶部开始
        if (y < -1.1f || x < -1.1f || x > 1.1f) {
            // 重新初始化位置，但保持其他属性
            x = static_cast<float>(rand() % SCR_WIDTH) / SCR_WIDTH * 2.0f - 1.0f;
            y = 1.1f;  // 屏幕顶部以上一点
            // 可以稍微变化一下速度，使下落看起来更自然
            vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.001f;
            vy = -0.001f - static_cast<float>(rand()) / RAND_MAX * 0.003f;
        }
    }
};

// 雪花数组
std::vector<Snowflake> snowflakes;

// 雪花图像没有alpha通道，需要手动创建透明度
float snowAlpha = 1.0f; // 提高默认不透明度

// 计时器变量，用于动画
double lastTime = 0;
double currentTime = 0;

// GLFW窗口指针
GLFWwindow* window = nullptr;

// 错误回调函数
void error_callback(int error, const char* description) {
    std::cerr << "Error: " << description << std::endl;
}

// 键盘回调函数
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// 窗口大小变化回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 加载纹理
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // 翻转Y轴
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        std::cout << "成功加载纹理: " << path << ", 宽度=" << width << ", 高度=" << height 
                  << ", 通道数=" << nrChannels << std::endl;

        // 处理雪花纹理 - 将纯白色转换为透明
        bool isSnowTexture = (strstr(path, "snow") != nullptr);
        if (isSnowTexture && nrChannels == 3) {
            // 为雪花图像创建一个带有Alpha通道的临时缓冲区
            unsigned char* tempData = new unsigned char[width * height * 4];
            
            // 将白色背景设为透明
            for (int i = 0; i < width * height; i++) {
                // 复制RGB通道
                tempData[i * 4 + 0] = data[i * 3 + 0]; // R
                tempData[i * 4 + 1] = data[i * 3 + 1]; // G
                tempData[i * 4 + 2] = data[i * 3 + 2]; // B
                
                // 检测纯白色或接近白色的像素，设置为透明
                if (data[i * 3 + 0] > 245 && data[i * 3 + 1] > 245 && data[i * 3 + 2] > 245) {
                    tempData[i * 4 + 3] = 0; // 完全透明
                } else {
                    // 基于像素亮度计算Alpha值 - 越暗的部分越不透明
                    // 调整透明度计算方式，使雪花不那么透明
                    unsigned char brightness = (data[i * 3 + 0] + data[i * 3 + 1] + data[i * 3 + 2]) / 3;
                    float alpha = 1.0f - (brightness / 255.0f) * 0.7f; // 减小亮度对透明度的影响
                    alpha = alpha * 1.5f; // 整体增加不透明度
                    if (alpha > 1.0f) alpha = 1.0f; // 限制最大值为1.0
                    tempData[i * 4 + 3] = static_cast<unsigned char>(alpha * 255); 
                }
            }
            
            // 使用新的RGBA数据并更改格式
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempData);
            glGenerateMipmap(GL_TEXTURE_2D);
            
            delete[] tempData;
        } else {
            // 正常处理其他纹理
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "无法加载纹理: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 初始化雪花
void initSnowflakes() {
    snowflakes.clear();
    
    // 创建初始雪花
    for (int i = 0; i < currentMaxSnowflakes; i++) {
        Snowflake flake;
        flake.init();
        // 让初始雪花分布在屏幕的不同位置
        flake.y = static_cast<float>(rand() % SCR_HEIGHT) / SCR_HEIGHT * 2.0f - 1.0f;
        snowflakes.push_back(flake);
    }
}

// 初始化
bool init() 
{
    // 初始化随机种子
    srand(static_cast<unsigned int>(time(nullptr)));
    
    // 清除颜色
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // 启用混合
    glEnable(GL_BLEND);
    // 使用预乘alpha的混合方式，对于带有透明度的雪花效果更好
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 启用纹理
    glEnable(GL_TEXTURE_2D);
    
    // 加载纹理
    backgroundTexture = loadTexture("../textures/background.png");
    snowTexture = loadTexture("../textures/snow.png");
    
    // 初始化雪花
    initSnowflakes();
    
    // 初始化时间
    lastTime = glfwGetTime();
    
    return true;
}

// 绘制背景
void drawBackground() 
{
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    
    // 设置完全不透明
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
    glEnd();
}

// 绘制单个雪花
void drawSnowflake(const Snowflake& flake) 
{
    glPushMatrix();
    
    // 移动到雪花位置
    glTranslatef(flake.x, flake.y, 0.0f);
    
    // 旋转
    glRotatef(flake.angle, 0.0f, 0.0f, 1.0f);
    
    // 缩放到雪花大小
    glScalef(flake.size, flake.size, 1.0f);
    
    // 设置白色，但允许雪花纹理本身的Alpha来控制透明度
    // 调整这里的透明度只影响雪花的整体可见度
    glColor4f(1.0f, 1.0f, 1.0f, snowAlpha);
    
    // 绘制雪花（以中心为原点的正方形）
    glBindTexture(GL_TEXTURE_2D, snowTexture);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
    glEnd();
    
    glPopMatrix();
}

// 更新雪花位置
void updateSnowflakes() {
    currentTime = glfwGetTime();
    float dt = static_cast<float>(currentTime - lastTime);
    lastTime = currentTime;
    
    // 随着时间增加雪花数量，直到达到最大值
    if (currentMaxSnowflakes < MAX_SNOWFLAKES && 
        currentTime - lastSnowflakeIncrease > 1.0) { // 每1秒增加一次
        currentMaxSnowflakes += snowflakeIncreaseRate;
        if (currentMaxSnowflakes > MAX_SNOWFLAKES) 
            currentMaxSnowflakes = MAX_SNOWFLAKES;
        
        // 添加新雪花
        for (int i = 0; i < snowflakeIncreaseRate; i++) {
            if (snowflakes.size() < currentMaxSnowflakes) {
                Snowflake flake;
                flake.init();
                snowflakes.push_back(flake);
            }
        }
        
        lastSnowflakeIncrease = currentTime;
        std::cout << "当前雪花数量: " << snowflakes.size() << std::endl;
    }
    
    // 确保始终有最小数量的雪花
    if (snowflakes.size() < currentMaxSnowflakes / 2) {
        int snowflakesToAdd = currentMaxSnowflakes / 10;  // 一次添加10%
        for (int i = 0; i < snowflakesToAdd; i++) {
            Snowflake flake;
            flake.init();
            snowflakes.push_back(flake);
        }
        std::cout << "添加新雪花，当前数量: " << snowflakes.size() << std::endl;
    }
    
    // 更新所有雪花
    for (auto& flake : snowflakes) {
        flake.update(dt);
    }
}

// 绘制所有雪花
void drawSnowflakes() 
{
    for (const auto& flake : snowflakes) {
        drawSnowflake(flake);
    }
}

// 渲染场景
void render() 
{
    // 清除缓冲
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 绘制背景（不透明）
    glDisable(GL_BLEND);
    drawBackground();
    
    // 绘制雪花（半透明）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawSnowflakes();
}

int main(int argc, char** argv) 
{
    // 初始化GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW初始化失败" << std::endl;
        return -1;
    }
    
    // 设置错误回调
    glfwSetErrorCallback(error_callback);
    
    // 设置OpenGL版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    // 创建窗口
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Snowing", NULL, NULL);
    if (!window) {
        std::cerr << "窗口创建失败" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // 设置当前上下文
    glfwMakeContextCurrent(window);
    
    // 初始化GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW初始化失败" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // 设置视口
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    // 设置回调函数
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // 应用程序初始化
    if (!init()) {
        std::cerr << "应用程序初始化失败" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    
    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 更新雪花
        updateSnowflakes();
        
        // 渲染
        render();
        
        // 交换缓冲并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理资源
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}