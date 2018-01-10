//
// Created by adunstudio on 2018-01-10.
//

#pragma once

#include "../sunny.h"
#include "../app/Window.h"
#include "../utils/Timer.h"
#include "../utils/Timestep.h"
#include "../maths/vec2.h"

namespace sunny
{
    namespace app
    {
        class Application
        {
        public:
            Window* window;

            inline static Application& GetApplication() { return *s_instance; }
        private:
            static Application* s_instance;

        private:
            bool m_running, m_paused;
            utils::Timer* m_timer;

            int m_updatesPerSecond, m_framesPerSecond;
            float m_frameTime;

            std::string m_name;
            WindowProperties m_properties;

        public:
            Application(const std::string& name, const WindowProperties& properties);
            ~Application();

            void Init();

            void Start();
            void Pause();
            void Resume();
            void Stop();

            inline int GetFPS() const { return m_framesPerSecond;   }
            inline int GetUPS() const { return m_updatesPerSecond;  }
            inline float GetFrameTime() const { return m_frameTime; }

            inline int GetWindowWidth()  const { window->GetWidth();  }
            inline int getWindowHeight() const { window->GetHeight(); }
            inline maths::vec2 GetWindowSize() const
            {
                return maths::vec2(static_cast<float>(window->GetWidth()), static_cast<float>(window->GetHeight()));
            }

        private:
            void Run();

            void OnTick();
            void OnUpdate(const utils::Timestep& ts);
            void OnRender();
        };
    }
}
