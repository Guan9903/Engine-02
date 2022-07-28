#include "../NCLGL/window.h"
#include "Renderer.h"

int main()	{
	Window w("Make your own project!", 1920, 1080, true);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	float scale = 100.0f;
	float rotation = 0.0f;
	Vector3 position(0, 0, -1500.0f);

	w.LockMouseToWindow(true);
	w.ShowOSPointer(false);

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE))
	{
		renderer.SetRotation(rotation);
		renderer.SetScale(scale);
		renderer.SetPosition(position);

		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
		renderer.RenderScene();
		renderer.SwapBuffers();
		
		/*if (Window::GetKeyboard()->KeyDown(KEYBOARD_1))
			renderer.SwitchToOrthographic();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_2))
			renderer.SwitchToPerspective();*/
		
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))
		{
			if (renderer.isSplit)
				renderer.isSplit = false;
			else
				renderer.isSplit = true;
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))
			renderer.SwitchToAuto();

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}

	}
	return 0;
}