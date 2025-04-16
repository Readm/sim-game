#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <queue>
#include <imgui.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>

// 确保定义OpenGL函数指针，避免链接错误
#ifndef GL_FUNCTIONS_DEFINED
#define GL_FUNCTIONS_DEFINED

// OpenGL常量定义
#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif

#ifndef GL_COLOR_ATTACHMENT0
#define GL_COLOR_ATTACHMENT0 0x8CE0
#endif

#ifndef GL_FRAMEBUFFER_COMPLETE
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#endif

// 定义函数指针类型
typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
typedef void (APIENTRY* PFNGLBINDFRAMEBUFFERPROC)(GLenum target, GLuint framebuffer);
typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSPROC)(GLsizei n, GLuint* framebuffers);
typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(GLenum target);
typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERSPROC)(GLsizei n, const GLuint* framebuffers);

// 声明函数指针
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;

// 在初始化函数中加载函数指针
bool InitOpenGLFunctions();
#endif

// 模拟事件类型
enum class GuiEventType {
    MouseClick,
    MouseMove,
    KeyPress,
    KeyRelease,
    DragStart,
    DragUpdate,
    DragEnd
};

// 模拟事件结构
struct GuiEvent {
    GuiEventType type;
    float x = 0.0f;
    float y = 0.0f;
    int button = 0;
    int key = 0;
    float timestamp = 0.0f;
};

class NetworkViewer; // 前向声明

class GuiTestFramework {
public:
    GuiTestFramework();
    ~GuiTestFramework();

    // 初始化和清理
    bool init();
    void shutdown();

    // 设置关联的网络查看器
    void setNetworkViewer(NetworkViewer* viewer) { m_NetworkViewer = viewer; }

    // 事件模拟
    void simulateMouseClick(int x, int y, int button = 0);
    void simulateMouseMove(int x, int y);
    void simulateKeyPress(int key);
    void simulateKeyRelease(int key);
    void simulateDragAndDrop(int startX, int startY, int endX, int endY, int button = 0);

    // 状态验证
    bool verifyWindowExists(const std::string& title);
    bool verifyNodeExists(int nodeId);
    bool verifyConnectionExists(int startPin, int endPin);
    bool verifyText(const std::string& text);
    bool verifyPixelColor(int x, int y, ImVec4 color, float tolerance = 0.1f);

    // 渲染控制
    void runOneFrame();
    void waitForSeconds(float seconds);
    void processEvents();

private:
    // OpenGL上下文
    GLFWwindow* m_Window;
    
    // 网络查看器指针
    NetworkViewer* m_NetworkViewer = nullptr;
    
    // 事件队列
    std::queue<GuiEvent> m_EventQueue;
    
    // 时间控制
    std::chrono::steady_clock::time_point m_StartTime;
    float m_CurrentTime;
    
    // 帧计数器
    int m_FrameCount = 0;

    // 内部方法
    void pushEvent(const GuiEvent& event);
    void processNextEvent();
    float getTime() const;
    void renderTestUI();

    int m_WindowWidth = 1280;  // 添加窗口宽度成员变量，默认1280
    int m_WindowHeight = 720;  // 添加窗口高度成员变量，默认720
}; 