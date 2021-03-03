#pragma once
namespace UE {
	struct Vertex {
		Vertex(float x, float y, float z, float u, float v) : x(x), y(y), z(z), u(u), v(v) {}
		Vertex() { x = y = z = 0.0f; u = v = 0.0f; }

		float x, y, z;
		float u, v;
	};

	class Mesh {
	public:
		Mesh() {
			m_Vertices = nullptr;
			m_NumVertices = 0;
		}

		~Mesh() {
			delete m_Vertices;
		}

		bool LoadFromFile(const char* filename);

		void* GetVertices() {
			return (void*)m_Vertices;
		}

		int GetNumVertices() {
			return m_NumVertices;
		}

	private:
		Vertex* m_Vertices;
		int m_NumVertices;
	};

}