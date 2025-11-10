#include "renderer.h"
#include <stdio.h>

#define DESIRED_WIDTH 1280
#define DESIRED_HEIGHT 720
#define WINDOW_TITLE "Path Tracing Renderer"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s scene.gltf\n", argv[0]);
    return 1;
  }
  
  OpenGLContext ctx =
      OpenGLContext_new(WINDOW_TITLE, DESIRED_WIDTH, DESIRED_HEIGHT);
  Renderer renderer = Renderer_new(OpenGLContext_get_resolution(&ctx));

  GLFWUserData *user_data = OpenGLContext_get_user_data(&ctx);
  Renderer_load_gltf(&renderer, argv[1], user_data);

  // https://gameprogrammingpatterns.com/game-loop.html#event-loops
  while (!glfwWindowShouldClose(ctx.window)) {
    OpenGLContext_poll_events(&ctx);
    Renderer_update_state(&renderer, OpenGLContext_get_user_data(&ctx),
                          OpenGLContext_get_resolution(&ctx));
    Renderer_render_frame(&renderer, &ctx);
    OpenGLContext_swap_buffers(&ctx);
  }
  Renderer_delete(&renderer);
  OpenGLContext_delete(&ctx);

  return 0;
}
