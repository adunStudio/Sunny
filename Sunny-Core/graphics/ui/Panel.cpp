#include "Panel.h"

#include "Widget.h"

#include "../../app/Application.h"

namespace sunny
{
	namespace graphics
	{
		namespace ui
		{
			Panel::Panel() : Layer2D(maths::mat4::Orthographic(0.0f, 32.0f, 0.0f, 18.0f, -1.0f, 1.0f))
			{
				Application::GetApplication().PushOverlay(this);
			}

			Panel::~Panel()
			{
				for (unsigned int i = 0; i < m_widgets.size(); ++i)
					delete m_widgets[i];

				Application::GetApplication().PopOverlay(this);
			}

			Widget* Panel::Add(Widget* widget)
			{
				m_widgets.push_back(widget);

				return widget;
			}

			void Panel::Remove(Widget* widget)
			{
				unsigned int index = 0;

				for (unsigned int i = 0; i < m_widgets.size(); ++i)
				{
					if (m_widgets[i] == widget)
					{
						delete m_widgets[i];
						m_widgets.erase(m_widgets.begin() + i);
						break;
					}
				}
			}

			void Panel::Clear()
			{
				for (unsigned int i = 0; i < m_widgets.size(); ++i)
					delete m_widgets[i];

				m_widgets.clear();
			}

			void Panel::OnUpdate(const utils::Timestep& ts)
			{
				for (Widget* widget : m_widgets)
				{
					if (widget->IsActive())
						widget->OnUpdate();
				}
			}

			void Panel::OnRender(Renderer2D& renderer)
			{
				for (Widget* widget : m_widgets)
				{
					if (widget->IsActive())
						widget->OnRender(renderer);
				}
			}

			void Panel::OnEvent(events::Event& e)
			{
				events::EventDispatcher dispathcer(e);
				dispathcer.Dispatch<events::MousePressedEvent>(METHOD(&Panel::OnMousePressedEvent));
				dispathcer.Dispatch<events::MouseReleasedEvent>(METHOD(&Panel::OnMouseReleasedEvent));
				dispathcer.Dispatch<events::MouseMovedEvent>(METHOD(&Panel::OnMouseMovedEvent));
			
				dispathcer.Dispatch<events::ResizeEvent>([this](events::ResizeEvent& e) {
					return Layer2D::OnResize(e.GetWidth(), e.GetHeight());
				});
			}

			bool Panel::OnMousePressedEvent(events::MousePressedEvent& e)
			{
				maths::vec2 mouse(e.GetX() * (32.0f / Window::GetWindowClass(nullptr)->GetWidth()) - 16.0f, 18.0f - e.GetY() * (18.0f / Window::GetWindowClass(nullptr)->GetHeight()) - 9.0f);

				for (unsigned int i = 0; i < m_widgets.size(); ++i)
				{
					Widget* widget = m_widgets[i];

					if (widget->GetBounds().Contains(mouse))
					{
						if (widget->OnMousePressed(e))
							return true;
					}
				}

				return false;
			}

			bool Panel::OnMouseReleasedEvent(events::MouseReleasedEvent& e)
			{
				maths::vec2 mouse(e.GetX() * (32.0f / Window::GetWindowClass(nullptr)->GetWidth()) - 16.0f, 18.0f - e.GetY() * (18.0f / Window::GetWindowClass(nullptr)->GetHeight()) - 9.0f);

				for (unsigned int i = 0; i < m_widgets.size(); ++i)
				{
					Widget* widget = m_widgets[i];

					if (widget->GetBounds().Contains(mouse))
					{
						if (widget->OnMouseReleased(e))
							return true;
					}
				}

				return false;
			}

			bool Panel::OnMouseMovedEvent(events::MouseMovedEvent& e)
			{
				for (unsigned int i = 0; i < m_widgets.size(); ++i)
				{
					Widget* widget = m_widgets[i];

					if (widget->OnMouseMoved(e))
						return true;
				}

				return false;
			}
		}
	}
}