#include <minefield/game_states.h>
#include <minefield/types.h>
#include <minefield/utils.h>
#include <minefield/json_utils.h>

void runMainLoop()
{
    bool quit = false;
    GameContext context;
    context.language = json_utils::loadLanguage("../resources/minefield/en.json");
    context.currentState = { &GameStates::stateMainMenuUpdate };
    while (!quit)
    {
        if (context.currentState.updateFunction != nullptr)
        {
            context.currentState = (*context.currentState.updateFunction)(context);
        }
        quit = context.currentState.updateFunction == nullptr;
    }
}

void initializeRandomNumberGenerator()
{
    srand(static_cast<unsigned int>(time(0)));
}

int main()
{
    initializeRandomNumberGenerator();
    runMainLoop();
    return 0;
}