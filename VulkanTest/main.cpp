#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

using std::vector;
using std::cerr;
using std::cout;
using std::endl;
using std::runtime_error;
using std::exception;
using std::optional;

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

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;
        
        vector<VkQueueFamilyProperties> queueFamilies = getDeviceQueueFamilyProperties(device);
        for (size_t i = 0; i < queueFamilies.size(); ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = static_cast<uint32_t>(i);
                break;
            }
        }

        return indices;
    }

    static bool isDeviceSuitable(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties = getDeviceProperties(device);
        VkPhysicalDeviceFeatures deviceFeatures = getDeviceFeatures(device);
        QueueFamilyIndices indices = findQueueFamilies(device);

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
               deviceFeatures.geometryShader &&
               indices.graphicsFamily.has_value();
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

    void pickPhysicalDevice() {
        vector<VkPhysicalDevice> devices = getAllAvailableDevices(this->instance);
        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
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
        this->pickPhysicalDevice();
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

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

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
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
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