#include "../nclGL/window.h"
#include "Renderer.h"
#include <algorithm>

int main() {
	Window w("Texturing!", 1280, 720, false);	 //This is all boring win32 window creation stuff!
	if (!w.HasInitialised()) {
		return -1;
	}

	Renderer renderer(w);	//This handles all the boring OGL 3.2 initialisation stuff, and sets up our tutorial!
	if (!renderer.HasInitialised()) {
		return -1;
	}

	float rotate = 0.0f;
	float mixFactor = 0.0f;
	while (w.UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT)) {
			--rotate;
			renderer.UpdateTextureMatrix(rotate);
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT)) {
			++rotate;
			renderer.UpdateTextureMatrix(rotate);
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
			renderer.ToggleFiltering();
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
			renderer.ToggleRepeating();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_M))
		{
			mixFactor += 0.1f;
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
		{
			mixFactor -= 0.1f;
		}
		mixFactor = std::max(0.0f, mixFactor);
		mixFactor = std::min(1.0f, mixFactor);


		renderer.setMixFactor(mixFactor);
		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}
	}

	return 0;
}