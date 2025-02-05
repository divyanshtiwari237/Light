#include "light.hpp"
#include "core/entrypoint.hpp"

#include "imgui.h"

#include "Objects.hpp"

class ExampleLayer : public Light::Layer
{
public:
	ExampleLayer(): Light::Layer("TestLayer"),
                    m_camera(45.0f, 1.6f / 0.9f, 0.001f, 100.0f),
                    m_lightPos(-1, 2, 1.5),
                    m_floor(glm::vec3(0, -1, 0), glm::vec3(0), glm::vec3(2, 0.1, 2))
	{
	    Light::FramebufferSpec fbspec;
		fbspec.width = 1280;
		fbspec.height = 720;
        m_framebuffer = Light::Framebuffer::create(fbspec);
	}
	~ExampleLayer() = default;

	void onUpdate(Light::Timestep ts) override
	{
		if(m_resizeViewport)
		{
			m_camera.setViewportSize(m_viewportPanelSize.x, m_viewportPanelSize.y);
			m_framebuffer->resize(m_viewportPanelSize.x, m_viewportPanelSize.y);
            m_resizeViewport = false;
		}
		m_frameCount++;
        m_time += ts.getMilliSeconds();
		if(m_time >= 500.0f)
		{
            m_lastTime = m_time;
            m_lastFrameCount = m_frameCount;
            m_time = 0.0f;
            m_frameCount = 0;
		}

		m_camera.onUpdate(ts);
		m_cube.onUpdate(ts);

		m_framebuffer->bind();

		Light::RenderCommand::setClearColor({0.2f,0.2f,0.2f,1.0f});
		Light::RenderCommand::clear();

		Light::Renderer::beginScene(m_camera, m_lightPos);
		
		m_skybox.render();
		m_cube.render();
		m_floor.render();

		Light::Renderer::endScene();

		m_framebuffer->unbind();
	}

	bool onWindowResize(Light::WindowResizeEvent& e)
	{
		auto[width, height] = e.getSize();
		if(width == 0 || height == 0)
		{
			return false;
		}
		m_camera.setViewportSize(width, height);
		return false;
	}


	void onEvent(Light::Event& e) override
	{
		Light::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Light::WindowResizeEvent>(BIND_EVENT_FN(ExampleLayer::onWindowResize));
		m_camera.onEvent(e);
		m_cube.onEvent(e);
	}

	void onImguiRender() override
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());


		if(ImGui::BeginMainMenuBar())
		{
			if(ImGui::BeginMenu("File"))
			{
				if(ImGui::MenuItem("Exit")) Light::Application::get().close();
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
		ImGui::Begin("Viewport");

		Light::Application::get().getImguiLayer()->blockFocusEvents(!ImGui::IsWindowFocused());
		Light::Application::get().getImguiLayer()->blockHoverEvents(!ImGui::IsWindowHovered());

		ImVec2 panelSize = ImGui::GetContentRegionAvail();
		if(m_viewportPanelSize != *(glm::vec2*)&panelSize)
		{
            m_resizeViewport = true;
            m_viewportPanelSize = glm::vec2(panelSize.x, panelSize.y);
		}
		ImGui::Image(INT2VOIDP(m_framebuffer->getColorAttachmentRendererId()), panelSize, ImVec2(0, 1), ImVec2(1, 0));
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Scene Settings");
		ImGui::DragFloat3("Light Position", &(m_lightPos.x), 0.01f);
		ImGui::End();

		ImGui::Begin("Camera Controls");
		ImGui::Text("Left Alt + LMB to Orbit");
		ImGui::Text("Left Alt + MMB to Pan");
		ImGui::Text("Left Alt + RMB to Zoom");
		ImGui::Text("Scroll to Zoom");
		ImGui::End();

		ImGui::Begin("Performance Statistics");
		ImGui::Text("MSPF: %0.2f\nSPF: %0.4f\nFPS: %d",
                    m_lastTime / m_lastFrameCount,
                    m_lastTime * 0.001f / m_lastFrameCount,
                    int(m_lastFrameCount * 1000 / m_lastTime));
		ImGui::End();

	}

private:
	Light::EditorCamera m_camera;
	Cube m_cube;
	Cube m_floor;
	Skybox m_skybox;
	glm::vec3 m_lightPos;
	std::shared_ptr<Light::Framebuffer> m_framebuffer;
	glm::vec2 m_viewportPanelSize;
	bool m_resizeViewport = false;
	float m_time = 0.0f;
	int m_frameCount = 0;
	float m_lastTime = 0.0f;
	int m_lastFrameCount = 0;
};

class Editor : public Light::Application
{
public:
	Editor()
	{
		pushLayer(new ExampleLayer());
	}
	~Editor() = default;
};

Light::Application* Light::createApplication()
{
	return new Editor();
}