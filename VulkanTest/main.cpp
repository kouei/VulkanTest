#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <unordered_set>

using std::vector;
using std::cerr;
using std::cout;
using std::endl;
using std::runtime_error;
using std::exception;
using std::optional;
using std::unordered_set;

class HelloTriangleApplication {

public:

    void run() {
        this->initWindow();
        this->initVulkan();
        this->mainLoop();
        this->cleanup();
    }

private:

    struct QueueFamilyIndices {
        optional<uint32_t> graphicsFamily;
        optional<uint32_t> presentFamily;

        bool isComplete() const {
            return this->graphicsFamily.has_value() && this->presentFamily.has_value();
        }
    };

    static const char * getSeverityString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            return "Error";
        } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            return "Warning";
        } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            return "Info";
        } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            return "Verbose";
        } else {
            return "Unknown Severity String";
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

        cerr << "[Validation Layer] " << "[Severity] " << getSeverityString(messageSeverity) << " [Message] " << pCallbackData->pMessage << endl;

        return VK_FALSE;
    }

    static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger) {

        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static VkDebugUtilsMessengerCreateInfoEXT populateDebugMessengerCreateInfo() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
        return createInfo;
    }

    static vector<VkLayerProperties> getAllAvailableLayers() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        return availableLayers;
    }

    static vector<VkExtensionProperties> getAllAvailableExtensions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        return extensions;
    }

    static void showAllAvailableExtensions() {
        vector<VkExtensionProperties> extensions = getAllAvailableExtensions();
        cout << "[All Vulkan Extensions]" << '\n';
        for (const auto& extension : extensions) {
            cout << '\t' << extension.extensionName << '\n';
        }
    }

    static vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static void showRequiredExtensions() {
        vector<const char*> extensions = getRequiredExtensions();
        cout << "[All Required Extensions]" << '\n';
        for (const char* p : extensions) {
            cout << '\t' << p << '\n';
        }
    }

    static VkApplicationInfo createApplicationInfo() {
        VkApplicationInfo appInfo;
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext = nullptr;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        return appInfo;
    }

    static VkInstanceCreateInfo createInstanceCreationInfo(
        VkApplicationInfo* pAppInfo,
        const vector<const char*>& extensions,
        VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo,
        const vector<const char*>& validationLayers) {

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = pAppInfo;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            createInfo.pNext = pDebugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        return createInfo;
    }

    static VkPhysicalDeviceProperties getDeviceProperties(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        return deviceProperties;
    }

    static VkPhysicalDeviceFeatures getDeviceFeatures(VkPhysicalDevice device) {
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        return deviceFeatures;
    }

    static vector<VkQueueFamilyProperties> getDeviceQueueFamilyProperties(VkPhysicalDevice device) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        return queueFamilies;
    }

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        
        vector<VkQueueFamilyProperties> queueFamilies = getDeviceQueueFamilyProperties(device);
        for (size_t i = 0; i < queueFamilies.size(); ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = static_cast<uint32_t>(i);
                break;
            }
        }
        // Please note that the presentation queue and the graphics queue can actually be the same queue.
        for (size_t i = 0; i < queueFamilies.size(); ++i) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, static_cast<uint32_t>(i), surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = static_cast<uint32_t>(i);
                break;
            }
        }

        return indices;
    }

    static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties deviceProperties = getDeviceProperties(device);
        VkPhysicalDeviceFeatures deviceFeatures = getDeviceFeatures(device);
        QueueFamilyIndices indices = findQueueFamilies(device, surface);

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                deviceFeatures.geometryShader &&
                indices.isComplete();
    }

    static vector<VkPhysicalDevice> getAllAvailableDevices(VkInstance instance) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        return devices;
    }

    void createSurface() {

        // Below is what exactly "glfwCreateWindowSurface()" does
        
        /*
        VkWin32SurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.hwnd = glfwGetWin32Window(this->window);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(this->instance, &createInfo, nullptr, &this->surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
        */
        

        if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice, this->surface);

        vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        unordered_set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.enabledExtensionCount = 0;
        createInfo.ppEnabledExtensionNames = nullptr;
        createInfo.pEnabledFeatures = &deviceFeatures;

        // The following code is for backward compatability.
        // In latest Vulkan implementation, the 2 parameters 'enabledLayerCount' and 'ppEnabledLayerNames' will be ignored.
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(this->validationLayers.size());
            createInfo.ppEnabledLayerNames = this->validationLayers.data();
        }

        if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(this->device, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
        vkGetDeviceQueue(this->device, indices.presentFamily.value(), 0, &this->presentQueue);
    }

    void pickPhysicalDevice() {
        vector<VkPhysicalDevice> devices = getAllAvailableDevices(this->instance);
        for (const auto& device : devices) {
            if (isDeviceSuitable(device, this->surface)) {
                this->physicalDevice = device;
                break;
            }
        }

        if (this->physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        getDeviceProperties(this->physicalDevice);
        getDeviceFeatures(this->physicalDevice);
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo = populateDebugMessengerCreateInfo();

        if (CreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS) {
            throw runtime_error("failed to set up debug messenger!");
        }
    }

    bool checkValidationLayerSupport() {
        vector<VkLayerProperties> availableLayers = getAllAvailableLayers();

        for (const char* layerName : this->validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = createApplicationInfo();
        
        vector<const char *> extensions = getRequiredExtensions();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = populateDebugMessengerCreateInfo();

        VkInstanceCreateInfo createInfo = createInstanceCreationInfo(&appInfo, extensions, &debugCreateInfo, this->validationLayers);

        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
            throw runtime_error("failed to create instance!");
        }

        showAllAvailableExtensions();
        showRequiredExtensions();
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        this->createInstance();
        this->setupDebugMessenger();
        this->createSurface();
        this->pickPhysicalDevice();
        this->createLogicalDevice();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        // Comment out following lines deliberately to show how validation layer works.
        // 
        // if (enableValidationLayers) {
        //     DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        // }

        vkDestroyDevice(this->device, nullptr);
        vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
        vkDestroyInstance(this->instance, nullptr);

        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

#ifdef NDEBUG
    static constexpr const bool enableValidationLayers = false;
#else
    static constexpr const bool enableValidationLayers = true;
#endif

    static constexpr const uint32_t WIDTH = 800;
    static constexpr const uint32_t HEIGHT = 600;

    const vector<const char*> validationLayers{
        "VK_LAYER_KHRONOS_validation"
    };

    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}