#include "WebCamera.h"
#include "Logger.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

WebCamera::WebCamera()
: m_descriptor(0)
, m_buffer(nullptr)
{    
}

bool WebCamera::Initialize(int width, int height, unsigned int pixelformat)
{
    Open();
    IsCanCapture();
    SetImageFormat(width, height, pixelformat);
    RequestBuffer();
    QueryBuffer();
    return Start();
}

bool WebCamera::Deinitialize()
{
    return Stop();
}

bool WebCamera::Open() {
    int fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        LOG_EX_ERROR("Failed to open device");
        return false;
    }
    m_descriptor = fd;
    return true;
}

bool WebCamera::IsCanCapture()
{
    v4l2_capability capability;
    if(ioctl(m_descriptor, VIDIOC_QUERYCAP, &capability) < 0) {
        LOG_EX_ERROR("Failed to get device capabilities, VIDIOC_QUERYCAP");
        return false;
    }
    return true;
}

bool WebCamera::SetImageFormat(int width, int height, unsigned int pixelformat) {
    v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = width;
    imageFormat.fmt.pix.height = height;
    imageFormat.fmt.pix.pixelformat = pixelformat;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    if(ioctl(m_descriptor, VIDIOC_S_FMT, &imageFormat) < 0){
        LOG_EX_ERROR("Device could not set format, VIDIOC_S_FMT");
        return false;
    }
    return true;
}

bool WebCamera::RequestBuffer()
{
    v4l2_requestbuffers requestBuffer = {0};
    requestBuffer.count = 1; 
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffer.memory = V4L2_MEMORY_MMAP;
    if(ioctl(m_descriptor, VIDIOC_REQBUFS, &requestBuffer) < 0){
        LOG_EX_ERROR("Could not request buffer from device, VIDIOC_REQBUFS");
        return false;
    }
    return true;
}

bool WebCamera::QueryBuffer()
{
    v4l2_buffer queryBuffer = {0};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    if(ioctl(m_descriptor, VIDIOC_QUERYBUF, &queryBuffer) < 0){
        LOG_EX_ERROR("Device did not return the buffer information, VIDIOC_QUERYBUF");
        return false;
    }
    m_buffer = (std::uint8_t*)mmap(
        nullptr, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
        m_descriptor, queryBuffer.m.offset
    );
    memset(m_buffer, 0, queryBuffer.length);
    return true;
}

bool WebCamera::Start()
{
    v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof(bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;
    if(ioctl(m_descriptor, VIDIOC_STREAMON, &bufferinfo.type) < 0){
        LOG_EX_ERROR("Could not start streaming, VIDIOC_STREAMON");
        return false;
    }
    m_bufferInfo = bufferinfo;
    return true;
}

bool WebCamera::GetFrame(Image& image)
{
    // Queue the buffer
    if(ioctl(m_descriptor, VIDIOC_QBUF, &m_bufferInfo) < 0){
        LOG_EX_ERROR("Could not queue buffer, VIDIOC_QBUF");
        return false;
    }

    // Dequeue the buffer
    if(ioctl(m_descriptor, VIDIOC_DQBUF, &m_bufferInfo) < 0){
        LOG_EX_ERROR("Could not dequeue the buffer, VIDIOC_DQBUF");
        return false;
    }

    // Frames get written after dequeuing the buffer
    LOG_EX_INFO("Frame was captured, size = " + std::to_string(m_bufferInfo.bytesused / 1024) + " KBytes");

    std::uint8_t* buffer = new std::uint8_t[m_bufferInfo.bytesused * sizeof(std::uint8_t)];
    std::copy(m_buffer, m_buffer + m_bufferInfo.bytesused, buffer);
    
    image.type = ImageType::JPEG;
    image.size = m_bufferInfo.bytesused;
    image.data = buffer;

    return true;
}

bool WebCamera::Stop()
{
    if(ioctl(m_descriptor, VIDIOC_STREAMOFF, &m_bufferInfo.type) < 0){
        LOG_EX_INFO("Could not end streaming, VIDIOC_STREAMOFF");
        return false;
    }
    if (m_descriptor)
    {
        close(m_descriptor);
    }

    return true;
}
