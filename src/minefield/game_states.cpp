#include <minefield/game_states.h>
#include <minefield/types.h>
#include <minefield/constants.h>
#include <minefield/utils.h>

#include <minefield/json_utils.h>

#include <iostream>
#include <cstdio>
#include <format>
#include <string>

namespace GameStates
{
    NextState stateMainMenuUpdate(GameContext& context)
    {
        std::cout << context.language["MainMenu::kHeader"] << '\n';
        std::cout << std::vformat(context.language["MainMenu::kStart"], std::make_format_args(MainMenu::Options::kStart));
        std::cout << std::vformat(context.language["MainMenu::kQuit"], std::make_format_args(MainMenu::Options::kQuit));
        std::cout << std::vformat(context.language["MainMenu::kLanguage"], std::make_format_args(MainMenu::Options::kLanguage));

        std::cout << context.language["MainMenu::kPrompt"];

        int userSelection = 0;
        std::cin >> userSelection;

        NextState next = { nullptr };
        switch (userSelection)
        {
            case MainMenu::Options::kStart:
                next = { &stateEnteringBoardMeasures };
                break;
            case MainMenu::Options::kQuit:
                std::cout << context.language["MainMenu::kThanksForPlaying"];
                next = { nullptr };
                break;
            case MainMenu::Options::kLanguage:
                next = { &stateChangeLanguage };
                break;
            default:
                std::cout << context.language["MainMenu::kInvalidOption"];
                std::cout << context.language["MainMenu::kPrompt"];
                next = context.currentState;
                break;
        }
        return next;
    }

    NextState stateChangeLanguage(GameContext& context)
    {
        std::cout << context.language["languages::kHeader"];
        std::cout << std::vformat(context.language["languages::kEnglish"], std::make_format_args(languages::options::kEnglish));
        std::cout << std::vformat(context.language["languages::kSpanish"], std::make_format_args(languages::options::kSpanish));
        std::cout << std::vformat(context.language["languages::kFrench"], std::make_format_args(languages::options::kFrench));

        std::cout << context.language["MainMenu::kPrompt"];

        int languageSelected = 0;
        std::cin >> languageSelected;

        Language language;

        switch (languageSelected)
        {
        case languages::options::kEnglish:
            language = json_utils::loadLanguage("../resources/minefield/en.json");
            break;
        case languages::options::kSpanish:
            language = json_utils::loadLanguage("../resources/minefield/es.json");
            break;
        case languages::options::kFrench:
            language = json_utils::loadLanguage("../resources/minefield/fr.json");
            break;
        }
        
        context.language = language;

        std::cout << '\n' << context.language["languages::kSet"] << '\n';

        return { &stateMainMenuUpdate };
    }

    NextState stateEnteringBoardMeasures(GameContext& context)
    {
        std::cout << context.language["BoardConfig::kHeader"] << '\n';
        std::cout << context.language["BoardConfig::kConfigMsg"] << '\n';

        context.width = Width(utils::enterValueInRange(context.language, context.language["BoardConfig::kEnterWidth"], BoardConfig::Limits::kMinWidth, BoardConfig::Limits::kMaxdWidth));
        context.height = Height(utils::enterValueInRange(context.language, context.language["BoardConfig::kEnterHeight"], BoardConfig::Limits::kMinHeight, BoardConfig::Limits::kMaxHeight));
        
        utils::board::initialize(context.board, context.height, context.width);

        std::cout << std::vformat(context.language["BoardConfig::kSetMsg"], std::make_format_args(context.width.getValue(), context.height.getValue()));

        return { &stateEnteringMineCount };
    }

    NextState stateEnteringMineCount(GameContext &context)
    {
        std::cout << context.language["MineConfig::kHeader"] << '\n';
        std::cout << context.language["MineConfig::kExplain"] << '\n';

        context.initialMines.setValue(utils::enterValueInRange(context.language, context.language["MineConfig::kEnterMines"], MineConfig::Limits::kMin, MineConfig::Limits::kMax));
        context.mines = context.initialMines;
        
        return { &stateCreatingPlayers };
    }

    NextState stateCreatingPlayers(GameContext& context)
    {
        std::cout << context.language["PlayerCreation::kHeader"] << '\n';

        utils::player::addPlayers(context.language, context.players, context.initialMines);

        if (context.players.empty())
        {
            std::cout << context.language["PlayerCreation::kZeroAdded"];
            return { nullptr };
        }
        else if (context.players.size() == 1)
        {
            Player player = utils::player::getPCPlayer(context.language, context.initialMines);
            context.players.push_back(player);

            std::cout << std::vformat(context.language["PlayerCreation::kPCAdded"], std::make_format_args(context.players[0].name, player.name));
        }
        else
        {
            unsigned int size = context.players.size();
            std::cout << std::vformat(context.language["PlayerCreation::kCreated"], std::make_format_args(size));
        }

        return { &statePuttingMines };
    }

    NextState statePuttingMines(GameContext& context)
    {
        if (context.players.empty())
        {
            std::cout << context.language["utilsMsg::kEmptyPlayers"];
            return { &stateCreatingPlayers };
        }

        std::cout << context.language["PuttingMines::kHeader"];

        unsigned int minesToPlace = 0;

        if (context.round.getValue() == 1)
        {
            minesToPlace = context.initialMines.getValue();

            std::cout << context.language["PuttingMines::kFirstRound"];
            std::cout << std::vformat(context.language["PuttingMines::kPlayersWillPlaceMines"], std::make_format_args(minesToPlace));
        }
        else
        {
            std::cout << std::vformat(context.language["PuttingMines::kRoundNumber"], std::make_format_args(context.round.getValue()));

            // If there are more than two players, the number of mines a player can guess 
            // is limited to the player with the fewest mines

            minesToPlace = utils::player::whoHasLessAvailableMines(context.players).getValue();

            if (minesToPlace == 0)
            {
                std::cout << context.language["PuttingMines::kNoAvailableMines"];
                return { nullptr };
            }
            else
            {
                std::cout << std::vformat(context.language["PuttingMines::kPlayersWillPlaceMines"], std::make_format_args(minesToPlace));
                context.mines.setValue(minesToPlace); 
            }
        }
        
        for (auto& player : context.players)
        {
            std::cout << std::vformat(context.language["PuttingMines::kPlayerTurn"], std::make_format_args(player.name));

            player.enterMine(context, player);

            std::cout << std::vformat(context.language["utilsMsg::kBoardOfPlayerPrompt"], std::make_format_args(player.name));
            utils::board::printPerPlayer(context.width, context.height, context.board, player);
        }

        auto currentRound = context.round.getValue();
        context.round.setValue(currentRound + 1);

        return { &stateProcessingMines };
    }

    NextState stateProcessingMines(GameContext& context)
    {
        std::cout << context.language["ProcessingMines::kHeader"];

        std::set<MinePosition> mineSet;
        std::set<MinePosition> duplicateMinesSet;

        for (auto const& player : context.players)
        {
            for (auto const& mine : player.placedMines)
            {
                auto insertion = mineSet.insert(mine);
                if (!insertion.second)
                {
                    duplicateMinesSet.insert(mine);
                }
            }
        }

        if (duplicateMinesSet.empty())
        {
            std::cout << context.language["ProcessingMines::kNoCollisions"];
        }
        else
        {
            for (auto& player : context.players)
            {
                for (auto& mine : player.placedMines)
                {
                    // If two players placed a mine in the same position, it is removed
                    
                    if (duplicateMinesSet.count(mine) > 0)
                    {
                        std::cout << std::vformat(context.language["ProcessingMines::kColissionMsg"], std::make_format_args(mine.x, mine.y));

                        context.board[mine.x][mine.y].state = PositionState::Removed;

                        duplicateMinesSet.erase(mine);
                    }
                }
            }
        }

        for (auto& player : context.players)
        {
            utils::player::saveMines(player);
        }

        return { &stateGuessingMines };
    }

    NextState stateGuessingMines(GameContext& context)
    {
        std::cout << context.language["GuessingMines::kHeader"];
        
        // The number of guesses the players can make is the same
        // to the number of mines they can place
        
        std::cout << std::vformat(context.language["GuessingMines::kTotalMsg"], std::make_format_args(context.mines.getValue()));

        for (auto& player : context.players)
        {
            std::cout << std::vformat(context.language["GuessingMines::kPlayerTurn"], std::make_format_args(player.name));

            for (unsigned int i = 0; i < context.mines.getValue(); i++)
            {
                MinePosition minePosition = utils::board::validBoardPositionState(context.language, context.width, context.height, player);

                std::cout << std::vformat(context.language["GuessingMines::kSuccess"], std::make_format_args(player.name, minePosition.x, minePosition.y));
                
                player.placedGuesses.push_back(minePosition);
            }
        }

        return { &stateProcessingGuesses };
    }

    NextState stateProcessingGuesses(GameContext& context)
    {
        std::cout << context.language["ProcessingGuesses::kHeader"];

        for (auto& player : context.players)
        {
            std::cout << std::vformat(context.language["ProcessingGuesses::kPlayerHeader"], std::make_format_args(player.name));

            for (auto const& guess : player.placedGuesses)
            {
                // If the mine is from the player, it reduces the amount of mines it can place

                if (utils::player::isMineFromPlayer(guess, player.minesHistory)) 
                {
                    utils::game::handleOwnMine(context.language, player, guess, context.board);
                } 
                else if (context.board[guess.x][guess.y].state == PositionState::WithMine) 
                {
                    utils::game::handleOpponentMine(context.language, player, guess, context.board, context.players);
                } 
                else 
                {
                    utils::game::handleMiss(context.language, player, guess, context.board);
                }
            }

            utils::player::saveGuesses(player);
            
            std::cout << std::vformat(context.language["utilsMsg::kBoardOfPlayerPrompt"], std::make_format_args(player.name));
            utils::board::printPerPlayer(context.width, context.height, context.board, player);
        }

        std::cout << context.language["ProcessingGuesses::kCurrentScoresHeader"];
            
        for (auto const& player : context.players)
        {
            std::cout << std::vformat(context.language["ProcessingGuesses::kScoreLine"], std::make_format_args(player.name, player.opponentMinesDetected.getValue(), player.ownMinesDetected.getValue()));
        }

        return { &stateCheckingNextTurn };
    }


    NextState stateCheckingNextTurn(GameContext& context)
    {
        unsigned int round = context.round.getValue() - 1;
        std::cout << std::vformat(context.language["Results::kHeader"], std::make_format_args(round));

        Players winners;
        Players eliminated;

        for (auto const& player : context.players)
        {
            unsigned int totalOpponentMines = utils::player::countOpponentMines(player, context.players);

            std::cout << std::vformat(context.language["Results::kPlayerInformation"], std::make_format_args(player.name, player.opponentMinesDetected.getValue(), totalOpponentMines, player.remainingMines.getValue()));

            if (player.opponentMinesDetected.getValue() >= totalOpponentMines && totalOpponentMines > 0)
            {
                winners.push_back(player);
            }
            
            // Players who can't place more mines are removed

            if (player.remainingMines.getValue() == 0)
            {
                eliminated.push_back(player);
            } 
        }

        context.players = utils::player::getRemainigPlayers(context.players, eliminated);

        /*
            Game finishes if:
            - A player found all oponnent mines
            - All players were removed
            - The board has no more available positions
        */
    
        if (utils::player::areThereWinners(context.language, winners) 
            || utils::game::hasOnePlayer(context.language, context.players) 
            || utils::board::isFull(context.language, context.width, context.height, context.board, context.players))
        {
            return { nullptr };
        }

        std::cout << std::vformat(context.language["Results::kProceedRound"], std::make_format_args(context.round.getValue()));
        
        return { &statePuttingMines };
    }
}