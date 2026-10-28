#include <iostream>
#include <memory>
using namespace std;

class Window {
public:
    virtual ~Window() = default;
    virtual void draw() const = 0;
};

class Button {
public:
    virtual ~Button() = default;
    virtual void draw() const = 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile() const = 0;
};


class AbstractFactory {
public:
    virtual ~AbstractFactory() = default;
    virtual unique_ptr<Window> createWindow() const = 0;
    virtual unique_ptr<Button> createButton() const = 0;
    virtual unique_ptr<Shader> createShader() const = 0;
};

class OpenGLWindow : public Window {
public:
    void draw() const override {
        cout << "[OpenGL] Dibujando ventana con contexto GL (glfwCreateWindow + glClear)";
        cout << endl;
    }
};

class OpenGLButton : public Button {
public:
    void draw() const override {
        cout << "[OpenGL] Dibujando boton con quads y glDrawArrays";
        cout << endl;
    }
};

class OpenGLShader : public Shader {
public:
    void compile() const override {
        cout << "[OpenGL] Compilando shader GLSL con glCompileShader";
        cout << endl;
    }
};

// ===================== PRODUCTOS VULKAN =====================

class VulkanWindow : public Window {
public:
    void draw() const override {
        cout << "[Vulkan] Dibujando ventana con VkSurfaceKHR y swapchain";
        cout << endl;
    }
};

class VulkanButton : public Button {
public:
    void draw() const override {
        cout << "[Vulkan] Dibujando boton con command buffers y vkCmdDraw";
        cout << endl;
    }
};

class VulkanShader : public Shader {
public:
    void compile() const override {
        cout << "[Vulkan] Compilando shader GLSL a SPIR-V y creando VkShaderModule";
        cout << endl;
    }
};


class OpenGLFactory : public AbstractFactory {
public:
    unique_ptr<Window> createWindow() const override {
        return make_unique<OpenGLWindow>();
    }
    unique_ptr<Button> createButton() const override {
        return make_unique<OpenGLButton>();
    }
    unique_ptr<Shader> createShader() const override {
        return make_unique<OpenGLShader>();
    }
};

class VulkanFactory : public AbstractFactory {
public:
    unique_ptr<Window> createWindow() const override {
        return make_unique<VulkanWindow>();
    }
    unique_ptr<Button> createButton() const override {
        return make_unique<VulkanButton>();
    }
    unique_ptr<Shader> createShader() const override {
        return make_unique<VulkanShader>();
    }
};

int main() {
    std::unique_ptr<AbstractFactory> factory =
    std::make_unique<OpenGLFactory>();
    auto window = factory->createWindow();
    auto button = factory->createButton();
    window->draw();
    button->draw();
    factory = std::make_unique<VulkanFactory>();
    auto shader = factory->createShader();
    shader->compile();
}