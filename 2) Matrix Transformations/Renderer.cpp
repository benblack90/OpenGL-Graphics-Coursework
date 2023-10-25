#include "Renderer.h"

Renderer::Renderer(Window& parent)
	: OGLRenderer(parent)
{
	triangle = Mesh::GenerateTriangle();
	matrixShader = new Shader("MatrixVertex.glsl", "colourFragment.glsl");
	camera = new Camera();
	if (!matrixShader->LoadSuccess())
	{
		return;
	}

	init = true;
	SwitchToOrthographic();
}

Renderer::~Renderer(void)
{
	delete triangle;
	delete matrixShader;
}

void Renderer::SwitchToPerspective()
{
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
}

void Renderer::SwitchToOrthographic()
{
	projMatrix = Matrix4::Orthographic(-1.0f, 10000.0f, width / 2.0f, -width / 2.0f, height / 2.0f, -height / 2.0f);
}

void Renderer::RenderScene()
{
	//clear the buffer to dark grey
	glClear(GL_COLOR_BUFFER_BIT);

	BindShader(matrixShader);

	//the -1 value in the 11th place identifies it as using perspective rather than orthographic projection
	if(projMatrix.values[11] == -1)
		//adjust fov value
		projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, fov);

	//update the matrix uniform variable in the shader - first with the projection matrix, then the view matrix
	UpdateShaderMatrices();

	//render 3 triangles offset in the x and y directions, and increasingly 'in the distance' in the z axis
	for (int i = 0; i < 3; ++i)
	{
		Vector3 tempPos = position;
		tempPos.z += (i * 500.0f);
		tempPos.x -= (i * 100.0f);
		tempPos.y -= (i * 100.0f);

		//create transformation matrix by multiplying the constituent matrices together, and then apply that new transformation matrix
		modelMatrix =   Matrix4::Translation(tempPos) * Matrix4::Scale(Vector3(scale, scale, scale)) * Matrix4::Rotation(rotation, Vector3(0, 1, 0));
		glUniformMatrix4fv(glGetUniformLocation(matrixShader->GetProgram(), "modelMatrix"), 1, false, modelMatrix.values);

		triangle->Draw();
	}
}

void Renderer::UpdateScene(float dt)
{
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}