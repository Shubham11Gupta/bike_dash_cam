#include "camera/windows/WindowsMediaFoundationCameraBackend.hpp"

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>

#include <sstream>
#include <string>

namespace bike_dashcam::camera {
namespace {

std::string hresultToString(const HRESULT value) {
    std::ostringstream stream;
    stream << "HRESULT 0x" << std::hex << static_cast<unsigned long>(value);
    return stream.str();
}

class MediaFoundationScope {
public:
    MediaFoundationScope() {
        const HRESULT com_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        com_initialized_ = SUCCEEDED(com_result);
        if (FAILED(com_result) && com_result != RPC_E_CHANGED_MODE) {
            error_ = "COM initialization failed: " + hresultToString(com_result);
            return;
        }

        const HRESULT media_foundation_result = MFStartup(MF_VERSION);
        if (FAILED(media_foundation_result)) {
            error_ = "Media Foundation startup failed: " + hresultToString(media_foundation_result);
            return;
        }

        media_foundation_started_ = true;
    }

    ~MediaFoundationScope() {
        if (media_foundation_started_) {
            MFShutdown();
        }
        if (com_initialized_) {
            CoUninitialize();
        }
    }

    bool ready() const {
        return media_foundation_started_;
    }

    const std::string& error() const {
        return error_;
    }

private:
    bool com_initialized_{false};
    bool media_foundation_started_{false};
    std::string error_;
};

std::string toUtf8(const wchar_t* value, const UINT32 length) {
    if (value == nullptr || length == 0) {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, value, static_cast<int>(length), nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, value, static_cast<int>(length), result.data(), size, nullptr, nullptr);
    return result;
}

bool enumerateDevices(IMFActivate*** devices, UINT32* device_count, std::string& error_message) {
    *devices = nullptr;
    *device_count = 0;

    IMFAttributes* attributes = nullptr;
    HRESULT result = MFCreateAttributes(&attributes, 1);
    if (FAILED(result)) {
        error_message = "Could not create Media Foundation attributes: " + hresultToString(result);
        return false;
    }

    result = attributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (SUCCEEDED(result)) {
        result = MFEnumDeviceSources(attributes, devices, device_count);
    }
    attributes->Release();

    if (FAILED(result)) {
        error_message = "Could not enumerate video capture devices: " + hresultToString(result);
        return false;
    }

    return true;
}

void releaseDevices(IMFActivate** devices, const UINT32 device_count) {
    for (UINT32 index = 0; index < device_count; ++index) {
        devices[index]->Release();
    }
    CoTaskMemFree(devices);
}

bool readAllocatedString(IMFActivate* device, const GUID& key, std::string& result) {
    wchar_t* value = nullptr;
    UINT32 length = 0;
    const HRESULT status = device->GetAllocatedString(key, &value, &length);
    if (FAILED(status)) {
        return false;
    }

    result = toUtf8(value, length);
    CoTaskMemFree(value);
    return true;
}

}  // namespace

std::string WindowsMediaFoundationCameraBackend::backendName() const {
    return "Windows Media Foundation";
}

std::vector<CameraDescriptor> WindowsMediaFoundationCameraBackend::discover() {
    MediaFoundationScope scope;
    if (!scope.ready()) {
        throw std::runtime_error(scope.error());
    }

    IMFActivate** devices = nullptr;
    UINT32 device_count = 0;
    std::string error_message;
    if (!enumerateDevices(&devices, &device_count, error_message)) {
        throw std::runtime_error(error_message);
    }

    std::vector<CameraDescriptor> cameras;
    cameras.reserve(device_count);
    for (UINT32 index = 0; index < device_count; ++index) {
        CameraDescriptor camera{};
        camera.backend_name = backendName();
        camera.available = readAllocatedString(
            devices[index], MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, camera.id);
        if (!readAllocatedString(devices[index], MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, camera.name)) {
            camera.name = "Unnamed Windows camera";
        }
        cameras.push_back(std::move(camera));
    }

    releaseDevices(devices, device_count);
    return cameras;
}

bool WindowsMediaFoundationCameraBackend::initialize(const CameraDescriptor& camera, std::string& error_message) {
    MediaFoundationScope scope;
    if (!scope.ready()) {
        error_message = scope.error();
        return false;
    }

    IMFActivate** devices = nullptr;
    UINT32 device_count = 0;
    if (!enumerateDevices(&devices, &device_count, error_message)) {
        return false;
    }

    bool initialized = false;
    for (UINT32 index = 0; index < device_count; ++index) {
        std::string identifier;
        if (!readAllocatedString(
                devices[index], MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, identifier) ||
            identifier != camera.id) {
            continue;
        }

        IMFMediaSource* source = nullptr;
        const HRESULT result = devices[index]->ActivateObject(IID_PPV_ARGS(&source));
        if (SUCCEEDED(result)) {
            source->Shutdown();
            source->Release();
            initialized = true;
        } else {
            error_message = "Could not activate camera device: " + hresultToString(result);
        }
        break;
    }

    releaseDevices(devices, device_count);
    if (!initialized && error_message.empty()) {
        error_message = "Camera device disappeared before initialization.";
    }
    return initialized;
}

}  // namespace bike_dashcam::camera
