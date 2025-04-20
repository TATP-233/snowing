# Snow Animation Program Technical Report

## 1. Project Dependencies

This project is a snowflake falling animation program developed using modern OpenGL technology, with the following main dependencies:

### Core Dependencies

- **OpenGL**: Provides the low-level graphics rendering API
- **GLEW (OpenGL Extension Wrangler Library)**: Used to load and manage OpenGL extensions, simplifying access to modern OpenGL features
- **GLFW (Graphics Library Framework)**: Provides a cross-platform solution for window creation, input handling, and rendering context management

### Image Processing Dependencies

- **stb_image.h**: Single-header image loading library, used to load texture images (background and snowflakes)
  - Supports multiple image formats (PNG, JPEG, etc.)
  - Provides image data processing functions

### Compilation and Build Tools

- **CMake 3.10+**: Cross-platform build system
- **C++17 Compiler**: Uses modern C++ features to improve code readability and performance

### System Requirements

- **Operating System**: Windows, macOS, or Linux
- **Graphics Hardware**: Graphics card and drivers supporting OpenGL 2.1 or higher

## 2. Shader Implementation

Although this project does not use separate shader files but rather OpenGL's fixed pipeline functionality, it still implements various rendering effects:

### Texture Rendering

The program uses OpenGL's texture mapping functionality, implemented through the following steps:

1. **Texture Loading**:
   ```cpp
   unsigned int loadTexture(const char* path)
   {
       unsigned int textureID;
       glGenTextures(1, &textureID);
       // Load image data...
       glBindTexture(GL_TEXTURE_2D, textureID);
       // Configure texture parameters...
       return textureID;
   }
   ```

2. **Texture Parameter Settings**:
   ```cpp
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   ```

3. **Transparency Processing**:
   For snowflake textures, the program dynamically calculates the Alpha channel by analyzing pixel brightness, achieving smooth transparency transitions:
   ```cpp
   unsigned char brightness = (data[i * 3 + 0] + data[i * 3 + 1] + data[i * 3 + 2]) / 3;
   float alpha = 1.0f - (brightness / 255.0f) * 0.7f;
   alpha = alpha * 1.5f;
   if (alpha > 1.0f) alpha = 1.0f;
   tempData[i * 4 + 3] = static_cast<unsigned char>(alpha * 255);
   ```

### Blending Mode

To achieve transparency effects, the program configures appropriate blending functions:
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

### Transformations and Matrix Operations

Snowflake rendering utilizes OpenGL's matrix stack operations to implement combined transformations of position, rotation, and scaling:
```cpp
glPushMatrix();
glTranslatef(flake.x, flake.y, 0.0f);
glRotatef(flake.angle, 0.0f, 0.0f, 1.0f);
glScalef(flake.size, flake.size, 1.0f);
// Render snowflake...
glPopMatrix();
```

## 3. Snowflake Particle Motion Formulas

The motion simulation of snowflakes employs various physical models including gravity, wind force, and air resistance, achieving realistic falling effects.

### Initialization Parameters

Each snowflake receives random attributes during initialization to ensure visual diversity:
```cpp
void init() {
    x = static_cast<float>(rand() % SCR_WIDTH) / SCR_WIDTH * 3.0f - 1.5f;
    y = 1.1f;  // Just above the top of the screen
    
    vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
    vy = -0.001f - static_cast<float>(rand()) / RAND_MAX * 0.3f;
    
    size = 0.02f + static_cast<float>(rand()) / RAND_MAX * 0.08f;
    
    angle = static_cast<float>(rand() % 360);
    rotation_speed = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
}
```

### Motion Update Algorithm

Snowflake position updates are based on time increments (dt), applying the following physical formulas:

1. **Gravity Effect**:
   ```cpp
   float gravity = -0.98f * 2.0f;  // Modified to negative value to accelerate snowflakes downward
   vy += gravity * dt;  // Apply gravity
   ```

2. **Wind Effect**:
   Wind simulation uses a sine function to create lateral swaying effects, making snowflake falling paths more natural:
   ```cpp
   float wind = sin((x * 0.2f + y * 1.0f - 0.5f) * M_PI * 2.0f) * 0.01f;
   vx += wind;
   ```
   
   In this formula:
   - `x * 0.2f + y * 1.0f - 0.5f` creates spatial variation in the wind field
   - `M_PI * 2.0f` controls the frequency of wind fluctuations
   - `0.01f` adjusts the intensity of the wind

3. **Air Resistance**:
   ```cpp
   float air_resistance = 0.98f;
   vx *= air_resistance;
   vy *= air_resistance;
   ```
   The air resistance coefficient (0.98) reduces snowflake velocity by approximately 2% each frame, simulating the damping effect of air on snowflake movement.

4. **Position Update**:
   ```cpp
   x += vx * dt;
   y += vy * dt;
   ```

5. **Rotation Angle Update**:
   ```cpp
   angle += rotation_speed * dt;
   ```

6. **Boundary Handling**:
   When snowflakes float off-screen, they restart from the top, creating a continuous snowfall effect:
   ```cpp
   if (y < -1.1f || x < -1.1f || x > 1.1f) {
       x = static_cast<float>(rand() % SCR_WIDTH) / SCR_WIDTH * 2.0f - 1.0f;
       y = 1.1f;
       vx = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.001f;
       vy = -0.001f - static_cast<float>(rand()) / RAND_MAX * 0.003f;
   }
   ```

### Dynamic Adjustment of Snowflake Count

To enhance the visual effect, the program gradually increases the number of snowflakes:
```cpp
if (currentMaxSnowflakes < MAX_SNOWFLAKES && 
    currentTime - lastSnowflakeIncrease > 1.0) { // Increase once every 1 second
    currentMaxSnowflakes += snowflakeIncreaseRate;
    // Add new snowflakes...
}
```

## Conclusion

This project achieves a realistic snowfall effect by combining OpenGL rendering techniques with physical simulation algorithms. The random parameters of snowflakes, physical motion models, and transparency processing together create a visually pleasing dynamic scene. In particular, the application of wind simulation and air resistance makes the snowflake movement more natural, rather than simple linear falling. 