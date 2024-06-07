#include "Application.h"

#include "GLFW/glfw3.h"

#include <iostream>

#include "oglfv2/Util/GLFWEventWrapper.h"

#include "oglfv2/Renderer/Renderer.h"

#include "oglfv2/UI/Widgets/Frame.h"
#include "oglfv2/UI/Widgets/FrameButton.h"
#include "oglfv2/UI/Widgets/TexturedButton.h"
#include "oglfv2/UI/Widgets/NineSliceButton.h"
#include "oglfv2/UI/Widgets/TexturedFrame.h"
#include "oglfv2/UI/Widgets/TextLabel.h"
#include "oglfv2/UI/Widgets/NineSliceFrame.h"

#include "oglfv2/UI/Behaviors/Focusable.h"

#include "oglfv2/UI/IWidgetGroup.h"
#include "oglfv2/UI/Animators.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}

const char* museVideo = "D:/Random/Muse Concert/20230416_222204.mp4";

Application::Application()
	: m_Window(nullptr), m_Surface(), m_Mesh(), m_Shader("res/Render.shader", false), m_Texture(nullptr), m_PixelBuffer(nullptr) {}

void Application::LoadResources()
{
	// Turn on blending
	Renderer::UseBlending(true, Renderer::BlendFunction::SrcAlpha, Renderer::BlendFunction::OneMinusSrcAlpha);

	// Build mesh

	float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	m_Mesh.SetVertices(vertices, 4 * 4 * 4);
	m_Mesh.SetIndices(indices, 6);
	m_Mesh.BufferLayout.Push<float>(2);
	m_Mesh.BufferLayout.Push<float>(2);
	m_Mesh.Construct();

	m_Shader.SetUniform1i("u_Texture", 0);

	// Window event callbacks
	GLFWEventWrapper* wrapper = GLFWEventWrapper::GetWrapper(m_Window);
	wrapper->ConnectFramebufferSize([&](GLFWwindow*, int32_t width, int32_t height) 
		{
			if (width > 0 && height > 0)
				m_Surface.SetResolution({ width, height });
			Renderer::SetViewport(width, height);

			Draw();
		}
	);

	// Setup surface
	int32_t width, height;
	glfwGetFramebufferSize(m_Window, &width, &height);
	m_Surface.SetResolution({ width, height });
	m_Surface.SetTimeFunction([]() 
		{ return glfwGetTime(); }
	);

	wrapper->ConnectMouseCursorPos(m_Surface.GLFWMouseCursorPosCallback());
	wrapper->ConnectMouseButton(m_Surface.GLFWMouseButtonCallback());
	wrapper->ConnectMouseScroll(m_Surface.GLFWMouseScrollCallback());
	wrapper->ConnectKey(m_Surface.GLFWKeyCallback());
	wrapper->ConnectChar(m_Surface.GLFWCharCallback());
}

void Application::Draw()
{
	Renderer::Clear();

	m_Surface.Update();
	m_Surface.ShouldRender = true;
	m_Surface.Render();

	m_Surface.GetColorBuffer()->Bind();
	Renderer::SubmitMesh(m_Mesh, m_Shader);

	Renderer::DrawFrame();

	glfwSwapBuffers(m_Window);
}

AVPixelFormat formatChooser(AVCodecContext* context, const AVPixelFormat* fmt)
{
	for (int i = 0; fmt[i] != AV_PIX_FMT_NONE; i++)
		std::cout << "Format: " << av_get_pix_fmt_name(fmt[i]) << '\n';
	std::cout << '\n';

	return fmt[1];
}

void Application::Run(GLFWwindow* window)
{
	m_Window = window;

	LoadResources();

	// Open the file
	AVFormatContext* formatContext = avformat_alloc_context();
	avformat_open_input(&formatContext, museVideo, nullptr, nullptr);

	std::cout << "Format " << formatContext->iformat->long_name << ", Duration " << formatContext->duration << " us\n\n";

	avformat_find_stream_info(formatContext, nullptr);

	// Print stream information and open a video codec
	AVCodecContext* codecContext = nullptr;
	AVStream* videoStream = nullptr;
	for (int32_t i = 0; i < formatContext->nb_streams; i++)
	{
		AVCodecParameters* localCodecParameters = formatContext->streams[i]->codecpar;
		const AVCodec* localCodec = avcodec_find_decoder(localCodecParameters->codec_id);

		if (localCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			std::cout << "\tVideo Codec: resolution " << localCodecParameters->width << " x " << localCodecParameters->height << '\n';

			videoStream = formatContext->streams[i];

			// Open the codec
			codecContext = avcodec_alloc_context3(localCodec);
			avcodec_parameters_to_context(codecContext, localCodecParameters);
			avcodec_open2(codecContext, localCodec, nullptr);
		}
		else if (localCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO)
			std::cout << "\tAudio Codec: " << localCodecParameters->ch_layout.nb_channels << " channels, sample rate " << localCodecParameters->sample_rate << '\n';
		std::cout << "\tCodec " << localCodec->long_name << " ID " << localCodec->id << "\n\n";
	}

	// Read a frame
	AVPacket* packet = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();


	while (av_read_frame(formatContext, packet) >= 0)
	{
		if (packet->stream_index == videoStream->index)
		{
			break;
		}

		av_packet_unref(packet);
	}

	avcodec_send_packet(codecContext, packet);
	avcodec_receive_frame(codecContext, frame);


	std::cout << "Frame " <<
		codecContext->frame_num << " [" <<
		av_get_pix_fmt_name(static_cast<AVPixelFormat>(frame->format)) << "] (" <<
		frame->width << ", " <<
		frame->height << ")\n";

	// Create render objects
	m_Texture = std::make_shared<Texture>(frame->width, frame->height);
	PixelBuffer::Spec pboSpec = {
		PixelBuffer::Type::CPUtoGPU,
		PixelBuffer::Order::RGB,
		PixelBuffer::Format::UINT8,
		frame->width, frame->height
	};
	m_PixelBuffer = new PixelBuffer(pboSpec);

	// Convert image to rgb format for rendering
	AVFrame* dstFrame = av_frame_alloc();
	dstFrame->format = AV_PIX_FMT_RGB24;
	dstFrame->width = frame->width;
	dstFrame->height = frame->height;
	av_image_alloc(dstFrame->data, dstFrame->linesize, dstFrame->width, dstFrame->height, static_cast<AVPixelFormat>(dstFrame->format), 1);

	SwsContext* conversion = sws_getContext(
		frame->width, frame->height, static_cast<AVPixelFormat>(frame->format),
		dstFrame->width, dstFrame->height, static_cast<AVPixelFormat>(dstFrame->format),
		SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

	sws_scale(conversion, frame->data, frame->linesize, 0, frame->height, dstFrame->data, dstFrame->linesize);
	sws_freeContext(conversion);

	std::cout << "Dst Frame " <<
		codecContext->frame_num << " [" <<
		av_get_pix_fmt_name(static_cast<AVPixelFormat>(dstFrame->format)) << "] (" <<
		dstFrame->width << ", " <<
		dstFrame->height << ")\n\n";

	// Upload image to gpu
	uint8_t* mappedMemory = reinterpret_cast<uint8_t*>(m_PixelBuffer->Map());
	for (uint32_t y = 0; y < dstFrame->height; y++)
	{
		for (uint32_t x = 0; x < dstFrame->width; x++)
		{
			uint32_t index = (x + y * dstFrame->width) * 3;

			mappedMemory[index] = dstFrame->data[0][(3 * x + y * dstFrame->linesize[0])];
			mappedMemory[index + 1] = dstFrame->data[0][(3 * x + y * dstFrame->linesize[0]) + 1];
			mappedMemory[index + 2] = dstFrame->data[0][(3 * x + y * dstFrame->linesize[0]) + 2];
		}
	}
	m_PixelBuffer->Unmap();
	m_Texture->UnpackPBO(*m_PixelBuffer);

	av_freep(dstFrame->data);
	av_frame_free(&dstFrame);

	av_packet_unref(packet);
	av_packet_free(&packet);
	av_frame_free(&frame);

	avcodec_free_context(&codecContext);
	avformat_close_input(&formatContext);

	std::shared_ptr<UI::TexturedFrame> display = std::make_shared<UI::TexturedFrame>();
	display->Position = glm::vec4(0.5f, 0.0f, 0.5f, 0.0f);
	display->Size = glm::vec4( 0.0f, 1280.0f, 0.0f, 720.0f );
	display->Rotation = glm::radians(90.0f);
	display->TextureObject = m_Texture;
	m_Surface.AddChild(display);

	while (!glfwWindowShouldClose(window))
	{
		Draw();
		UI::AnimationScheduler::CleanupFinishedAnimations();

		glfwPollEvents();
	}

	delete m_PixelBuffer;
}