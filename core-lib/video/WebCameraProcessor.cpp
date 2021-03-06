#include "WebCameraProcessor.h"
#include "Logger.h"
#include "DateTimeUtils.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <cstdint>

WebCameraProcessor::WebCameraProcessor(int width, int height, std::uint32_t pixelformat)
: m_width(width)
, m_height(height)
, m_pixelformat(pixelformat)
, m_descriptor(0)
, m_buffer(nullptr)
{    
}

void WebCameraProcessor::Init()
{
    Open();
    IsCanCapture();
    SetImageFormat(m_width, m_height, m_pixelformat);
    RequestBuffer();
    QueryBuffer();
    if (!Start())
    {
        LOG_EX_WARN_WITH_CONTEXT("Couldn't start web camera");
        return;
    }
    LOG_EX_INFO("Start web camera");

    DataProcessor::Init();
}

void WebCameraProcessor::Destroy()
{
    if (!Close())
    {
        LOG_EX_WARN("Couldn't stop web camera");
        return;
    }
    LOG_EX_INFO("Stop web camera");

    DataProcessor::Destroy();
}

void WebCameraProcessor::Process(const media_packet_ptr& pkt)
{
    if (!GetFrame(pkt)) {
        LOG_EX_WARN_WITH_CONTEXT("Couldn't get frame from web camera");
    }
    
    DataProcessor::Process(pkt);
}

bool WebCameraProcessor::Open() {
    int fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        LOG_EX_ERROR("Failed to open device");
        return false;
    }
    m_descriptor = fd;
    return true;
}

bool WebCameraProcessor::IsCanCapture()
{
    v4l2_capability capability;
    if(ioctl(m_descriptor, VIDIOC_QUERYCAP, &capability) < 0) {
        LOG_EX_ERROR("Failed to get device capabilities, VIDIOC_QUERYCAP");
        return false;
    }
    return true;
}

bool WebCameraProcessor::SetImageFormat(int width, int height, std::uint32_t pixelformat) {
    v4l2_format imageFormat;
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = width;
    imageFormat.fmt.pix.height = height;
    imageFormat.fmt.pix.pixelformat = pixelformat;
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    if(ioctl(m_descriptor, VIDIOC_S_FMT, &imageFormat) < 0){
        LOG_EX_ERROR_WITH_CONTEXT("Device could not set format, VIDIOC_S_FMT");
        return false;
    }
    return true;
}

bool WebCameraProcessor::RequestBuffer()
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

bool WebCameraProcessor::QueryBuffer()
{
    m_queryBuffer = {0};
    m_queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_queryBuffer.memory = V4L2_MEMORY_MMAP;
    m_queryBuffer.index = 0;
    if(ioctl(m_descriptor, VIDIOC_QUERYBUF, &m_queryBuffer) < 0){
        LOG_EX_ERROR("Device did not return the buffer information, VIDIOC_QUERYBUF");
        return false;
    }
    m_buffer = (std::uint8_t*)mmap(
        nullptr, m_queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
        m_descriptor, m_queryBuffer.m.offset
    );
    memset(m_buffer, 0, m_queryBuffer.length);
    return true;
}

bool WebCameraProcessor::Start()
{
    memset(&m_bufferInfo, 0, sizeof(m_bufferInfo));
    m_bufferInfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_bufferInfo.memory = V4L2_MEMORY_MMAP;
    m_bufferInfo.index = 0;
    if(ioctl(m_descriptor, VIDIOC_STREAMON, &m_bufferInfo.type) < 0){
        LOG_EX_ERROR("Could not start streaming, VIDIOC_STREAMON");
        return false;
    }
    return true;
}

bool WebCameraProcessor::GetFrame(const media_packet_ptr& pkt)
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
    // LOG_EX_INFO("Frame was captured, size = " + std::to_string(m_bufferInfo.bytesused / 1024) + " KBytes");

    pkt->header.type = MediaPacketType::JPEG;
    pkt->header.ts = DateTimeUtils::GetCurrentTimeNanoseconds();
    pkt->header.size = m_bufferInfo.bytesused;
    std::copy(m_buffer, m_buffer + m_bufferInfo.bytesused, pkt->data);

    return true;
}

bool WebCameraProcessor::Close()
{
    if (m_buffer)
    {
        munmap(m_buffer, m_queryBuffer.length);
        m_buffer = nullptr;
    }
    if (ioctl(m_descriptor, VIDIOC_STREAMOFF, &m_bufferInfo.type) < 0) {
        LOG_EX_INFO("Could not end streaming, VIDIOC_STREAMOFF");
        return false;
    }
    if (m_descriptor) {
        close(m_descriptor);
        m_descriptor = 0;
    }

    return true;
}
