#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>

int main(int argc, char* argv[])
{
    // Initialisation de SDL avec support OpenGL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur SDL_Init: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Version d'OpenGL (ici 3.3 Core)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Création de la fenêtre
    SDL_Window* window = SDL_CreateWindow(
        "SDL2 + OpenGL test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Erreur SDL_CreateWindow: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Création du contexte OpenGL
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Erreur SDL_GL_CreateContext: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Initialisation de GLEW (nécessaire pour charger les fonctions OpenGL modernes)
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Erreur GLEW: " << glewGetErrorString(glewError) << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "OpenGL version : " << glGetString(GL_VERSION) << std::endl;

    bool running = true;
    SDL_Event event;

    // Boucle principale
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // Nettoyage de l’écran avec une couleur bleu ciel
        glClearColor(0.2f, 0.4f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Échange des buffers
        SDL_GL_SwapWindow(window);
    }

    // Nettoyage
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
