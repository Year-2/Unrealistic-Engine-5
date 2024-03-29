#include "Model.h"

namespace UE{

	//=====	MODEL ======
	Model::Model(const char* modelPath, const char* texturePath)
	{
		m_Model = std::make_shared<Mesh>();
		const bool result = m_Model->LoadFromFile((g_ModelDirectory + std::string(modelPath)).c_str());
		if (!result) std::cerr << "Failed to load model!\n";

		m_Texture = std::make_shared<Texture>((g_TextureDirectory + std::string(texturePath)).c_str());

		m_ModelRenderer = std::make_unique<MeshRenderer>(m_Model);
		m_ModelRenderer->SetRotation({ 0.0F, 0.0F, 0.0F });
		m_ModelRenderer->SetScale({ 3,3,3 });
		m_ModelRenderer->SetMaterial(m_Texture);
	}

	void Model::Draw(const std::shared_ptr<Camera>& camera) {
		m_ModelRenderer->Draw(camera);
	}

	//=====	MESH ======
	Mesh::Mesh()
	{
		m_Vertices = {};
		m_NumVertices = 0;
	}

	bool Mesh::LoadFromFile(const char* filename)
	{
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs);

		if (pScene == nullptr) return false;

		for (unsigned int MeshIdx = 0; MeshIdx < pScene->mNumMeshes; MeshIdx++)
		{
			const aiMesh* mesh = pScene->mMeshes[MeshIdx];
			for (unsigned int faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++)
			{
				const aiFace& face = mesh->mFaces[faceIdx];
				for (int vertexIdx = 0; vertexIdx < 3; vertexIdx++)
				{
					const aiVector3D* pos = &mesh->mVertices[face.mIndices[vertexIdx]];
					const aiVector3D uv = mesh->mTextureCoords[0][face.mIndices[vertexIdx]];
					m_Vertices.emplace_back(Vertex<float>(pos->x, pos->y, pos->z, uv.x, uv.y));
				}
			}
		}

		m_NumVertices = m_Vertices.size();

		return true;
	}

	//=====	MESH RENDERER ======
	MeshRenderer::MeshRenderer(std::shared_ptr<Mesh> model)
	{
		m_Position = { 0.0F, 0.0F, 0.0F };
		m_Rotation = { 0.0F, 0.0F, 0.0F };
		m_Scale = { 1.0F, 1.0F, 1.0F };
		m_Mesh = std::move(model);
		Init();
	}

	MeshRenderer::~MeshRenderer() 
	{
		glDeleteProgram(m_ProgramID);
		glDeleteBuffers(1, &m_VboModel);
	}

	void MeshRenderer::Init()
	{
		GLuint v_id = glCreateShader(GL_VERTEX_SHADER);
		std::string v_shader_source = LoadShaderSourceCode(g_ShaderDirectory + "model.vert");
		std::vector<const GLchar*> v_source_array = { v_shader_source.c_str() };

		GLuint f_id = glCreateShader(GL_FRAGMENT_SHADER);
		std::string f_shader_source = LoadShaderSourceCode(g_ShaderDirectory + "model.frag");
		std::vector<const GLchar*> f_source_array = { f_shader_source.c_str() };

		if (!CompileProgram(v_id, v_source_array, f_id, f_source_array, m_ProgramID)) {
			std::cerr << "Failed to create model renderer program. Check console for errors: \n";
			return;
		}

		m_VertexPos3DLocation = glGetAttribLocation(m_ProgramID, "vertexPos3D");
		if (m_VertexPos3DLocation == -1) {
			std::cerr << "Problem getting vertexPos3D\n";
			return;
		}

		m_VertexUVLocation = glGetAttribLocation(m_ProgramID, "vUV");
		if (m_VertexUVLocation == -1) {
			std::cerr << "Problem getting vUV\n";
			return;
		}

		m_TransformUniformID = glGetUniformLocation(m_ProgramID, "transform");
		m_ViewUniformID = glGetUniformLocation(m_ProgramID, "view");
		m_ProjectionUniformID = glGetUniformLocation(m_ProgramID, "projection");
		m_SamplerID = glGetUniformLocation(m_ProgramID, "sampler");

		glGenBuffers(1, &m_VboModel);
		glBindBuffer(GL_ARRAY_BUFFER, m_VboModel);

		glBufferData(GL_ARRAY_BUFFER, m_Mesh->GetNumVertices() * sizeof(Vertex<float>), m_Mesh->GetVertices().data(), GL_STATIC_DRAW);

		glDeleteShader(v_id);
		glDeleteShader(f_id);
	}

	void MeshRenderer::Draw(const std::shared_ptr<Camera>& camera)
	{
		glEnable(GL_CULL_FACE);

		glm::mat4 transformationMatrix = glm::mat4(1.0F);
		transformationMatrix = glm::translate(transformationMatrix, glm::vec3(m_Position.x, m_Position.y, m_Position.z));
		transformationMatrix = glm::rotate(transformationMatrix, glm::radians(m_Rotation.x), glm::vec3(1.0F, 0.0F, 0.0F));
		transformationMatrix = glm::rotate(transformationMatrix, glm::radians(m_Rotation.y), glm::vec3(0.0F, 1.0F, 0.0F));
		transformationMatrix = glm::rotate(transformationMatrix, glm::radians(m_Rotation.z), glm::vec3(0.0F, 0.0F, 1.0F));
		transformationMatrix = glm::scale(transformationMatrix, glm::vec3(m_Scale.x, m_Scale.y, m_Scale.z));

		glm::mat4 viewMatrix = camera->GetViewMatrix();
		glm::mat4 projectionMatrix = camera->GetProjectionMatrix();

		glUseProgram(m_ProgramID);

		glUniformMatrix4fv(m_TransformUniformID, 1, GL_FALSE, glm::value_ptr(transformationMatrix));
		glUniformMatrix4fv(m_ViewUniformID, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(m_ProjectionUniformID, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		glBindBuffer(GL_ARRAY_BUFFER, m_VboModel);

		glVertexAttribPointer(m_VertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex<float>), reinterpret_cast<void*>(offsetof(Vertex<float>, x)));
		glEnableVertexAttribArray(m_VertexPos3DLocation);

		glVertexAttribPointer(m_VertexUVLocation, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex<float>), reinterpret_cast<void*>(offsetof(Vertex<float>, u)));
		glEnableVertexAttribArray(m_VertexUVLocation);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(m_SamplerID, 0);
		glBindTexture(GL_TEXTURE_2D, m_Texture->GetTextureName());

		glDrawArrays(GL_TRIANGLES, 0, m_Mesh->GetNumVertices());

		glDisableVertexAttribArray(m_VertexPos3DLocation);
		glDisableVertexAttribArray(m_VertexUVLocation);

		glUseProgram(0);

		glDisable(GL_CULL_FACE);
	}
}// namespace UE