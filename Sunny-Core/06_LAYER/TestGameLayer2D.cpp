#include "TestGameLayer2D.h"

TestGameLayer2D::TestGameLayer2D()
: Layer2D(maths::mat4::Orthographic(0.0f, Application::GetApplication().GetWindowWidth(), 0.0f, Application::GetApplication().GetWindowHeight(), -1.0f, 1.0f))
, m_saved_packet_size(0), m_packet_size(0), m_io_flag(0)
{
	m_send_wsabuf.buf = m_send_buffer;
	m_send_wsabuf.len = MAX_BUFF_SIZE;
	m_recv_wsabuf.buf = m_recv_buffer;
	m_recv_wsabuf.len = MAX_BUFF_SIZE;
}

TestGameLayer2D::~TestGameLayer2D()
{

}

void TestGameLayer2D::OnInit(Renderer2D& renderer)
{
	m_panel = new Panel();
	m_bossHPBar = new Progressbar(maths::Rectangle(300, 830, 500, 20));
	m_bossHPBar->SetValue(1);
	m_panel->Add(m_bossHPBar);

	m_test = new Sprite(new Texture2D("/TEXTURE/cursor2.png"));

	Add(m_test);
}

void TestGameLayer2D::OnTick()
{

}

void TestGameLayer2D::OnUpdate(const utils::Timestep& ts)
{
	InputProcess();
}

void TestGameLayer2D::OnRender(Renderer2D& renderer)
{

}

void TestGameLayer2D::OnEvent(Event& event)
{
	Layer2D::OnEvent(event);
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<ServerPacketEvent>(METHOD(&TestGameLayer2D::OnServerPacketEvent));
	dispatcher.Dispatch<MouseMovedEvent>(METHOD(&TestGameLayer2D::OnMouseMovedEvent));
	dispatcher.Dispatch<KeyPressedEvent>(METHOD(&TestGameLayer2D::OnKeyPressedEvent));
	dispatcher.Dispatch<MouseReleasedEvent>(METHOD(&TestGameLayer2D::OnMouseReleasedEvent));
}

bool TestGameLayer2D::OnMouseMovedEvent(MouseMovedEvent& event)
{
	return false;
}

void TestGameLayer2D::ProcessPacket(char* ptr)
{
	switch (ptr[1])
	{
		case SC_PLAYER_DEGREE:
		{
			sc_packet_player_degree* packet = reinterpret_cast<sc_packet_player_degree*>(ptr);

			int       id = packet->id;
			float degree = packet->degree;


			BossLocker::players[id]->character->GetTransformComponent()->SetRotation(vec3( 0, degree + 90.0f, 0 ));
			break;
		}

		case SC_PLAYER_POS:
		{
			sc_packet_player_pos* packet = reinterpret_cast<sc_packet_player_pos*>(ptr);

			int       id = packet->id;
			float      x = packet->x;
			float      z = packet->z;


			BossLocker::players[id]->character->GetTransformComponent()->SetPosition(vec3(x, 0, z));
			break;
		}

		case SC_PLAYER_ANIMATION:
		{
			sc_packet_player_animation* packet = reinterpret_cast<sc_packet_player_animation*>(ptr);

			int           id = packet->id;
			int    animation = packet->animation;

			BossLocker::players[id]->character->SetAnimation(animation);
			break;
		}

		case SC_BULLET_PUT:
		{
			sc_packet_bullet_put* packet = reinterpret_cast<sc_packet_bullet_put*>(ptr);
			int           id = packet->id;
			int         kind = packet->kind;
			float         x  = packet->x;
			float         z  = packet->z;

			if (kind == BULLET_PLAYER)
			{
				BossLocker::player_bullets[id]->GetTransformComponent()->SetPosition(vec3(x, 100, z));
				BossLocker::player_bullets[id]->SetVisible(true);
			}
			break;
		}

		case SC_BULLET_POS:
		{
			sc_packet_bullet_pos* packet = reinterpret_cast<sc_packet_bullet_pos*>(ptr);
			int           id = packet->id;
			int         kind = packet->kind;
			float         x = packet->x;
			float         z = packet->z;

			if (kind == BULLET_PLAYER)
			{
				BossLocker::player_bullets[id]->GetTransformComponent()->SetPosition(vec3(x, 100, z));
			}
			break;
		}

		case SC_BULLET_REMOVE:
		{
			sc_packet_bullet_remove* packet = reinterpret_cast<sc_packet_bullet_remove*>(ptr);
			int           id = packet->id;
			int         kind = packet->kind;

			if (kind == BULLET_PLAYER)
			{
				BossLocker::player_bullets[id]->SetVisible(false);
			}
			break;
		}

		case SC_BOSS_HP:
		{
			sc_packet_boss_hp* packet = reinterpret_cast<sc_packet_boss_hp*>(ptr);
			float         hp = packet->hp;
			float max = BOSS_HP;
			m_bossHPBar->SetValue(hp / max );
			break;
		}

		case SC_PARTICLE_PUT:
		{
			std::cout << 1 << std::endl;

			sc_packet_particle_put* packet = reinterpret_cast<sc_packet_particle_put*>(ptr);

			vec3 pos = vec3(packet->x, 20, packet->z);

			if (!BossLocker::sc_bulletList[packet->id])
				BossLocker::sc_bulletList[packet->id] = new SCBullet(pos, packet->bullet_type);
			else
				BossLocker::sc_bulletList[packet->id]->m_position = pos;

			//new Bullet(BULLET_TYPE::DIRECTIONAL, pos, 0, 0, 0, 0);

			break;
		}
	}
}
bool TestGameLayer2D::OnMouseReleasedEvent(MouseReleasedEvent& event)
{
	SOCKET socket = Server::GetSocket();

	cs_packet_player_mouse_release* packet = reinterpret_cast<cs_packet_player_mouse_release*>(m_send_buffer);
	packet->type = CS_PLAYER_MOUSE_RELEASE;
	packet->size = sizeof(packet);
	m_send_wsabuf.len = sizeof(packet);

	DWORD ioByte;

	WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);
	return false;
}

bool TestGameLayer2D::OnKeyPressedEvent(KeyPressedEvent& event)
{
	switch (event.GetKeyCode())
	{
		case SUNNY_KEY_SPACE:
			if (BossLocker::player->character->GetAnimation() != ANIMATION_ROLL_BASIC)
			{
				SOCKET socket = Server::GetSocket();

				cs_packet_player_roll* packet = reinterpret_cast<cs_packet_player_roll*>(m_send_buffer);
				packet->type = CS_PLAYER_ROLL;
				packet->size = sizeof(packet);
				m_send_wsabuf.len = sizeof(packet);


				DWORD ioByte;

				WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);
			}
			break;

	}

	return false;
}

void TestGameLayer2D::InputProcess()
{
	SOCKET socket = Server::GetSocket();
	
	cs_packet_player_move* packet = reinterpret_cast<cs_packet_player_move*>(m_send_buffer);
	packet->type = CS_PLAYER_MOVE;
	packet->size = sizeof(packet);
	packet->dir = MOVE_LEFT;
	m_send_wsabuf.len = sizeof(packet);
	
	if (Input::IsKeyPressed(SUNNY_KEY_A) && Input::IsKeyPressed(SUNNY_KEY_W))
	{
		packet->dir = MOVE_LEFT_UP;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_A) && Input::IsKeyPressed(SUNNY_KEY_S))
	{
		packet->dir = MOVE_LEFT_DOWN;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_D) && Input::IsKeyPressed(SUNNY_KEY_W))
	{
		packet->dir = MOVE_RIGHT_UP;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_D) && Input::IsKeyPressed(SUNNY_KEY_S))
	{
		packet->dir = MOVE_RIGHT_DOWN;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_W))
	{
		packet->dir = MOVE_UP;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_S))
	{
		packet->dir = MOVE_DOWN;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_A))
	{
		packet->dir = MOVE_LEFT;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (Input::IsKeyPressed(SUNNY_KEY_D))
	{
		packet->dir = MOVE_RIGHT;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}

	if (!Input::IsMouseButtonPressed(SUNNY_MOUSE_LEFT) && BossLocker::player->character->GetAnimation() != ANIMATION_IDLE_BASIC && BossLocker::player->character->GetAnimation() !=  ANIMATION_ROLL_BASIC)
	{
		packet->dir = MOVE_STOP;

		DWORD ioByte;

		WSASend(socket, &m_send_wsabuf, 1, &m_io_flag, 0, NULL, NULL);

		return;
	}
}









bool TestGameLayer2D::OnServerPacketEvent(ServerPacketEvent& event)
{
	unsigned long ioByte = 0;

	int ret = WSARecv(event.socket, &m_recv_wsabuf, 1, &ioByte, &m_io_flag, NULL, NULL);
	if (ret)
	{
		// Debug System
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	unsigned char* ptr = reinterpret_cast<unsigned char*>(m_recv_buffer);

	while (ioByte != 0)
	{
		if (m_packet_size == 0) m_packet_size = ptr[0];

		if (ioByte + m_saved_packet_size >= m_packet_size)
		{
			memcpy(m_packet_buffer + m_saved_packet_size, ptr, m_packet_size - m_saved_packet_size);
			ProcessPacket(m_packet_buffer);


			ptr += m_packet_size - m_saved_packet_size;
			ioByte -= m_packet_size - m_saved_packet_size;
			m_packet_size = 0;
			m_saved_packet_size = 0;
		}
		else
		{
			memcpy(m_packet_buffer + m_saved_packet_size, ptr, ioByte);
			m_saved_packet_size += ioByte;
			ioByte = 0;
		}
	}

	return true;
}