#include <minefield/game_states.h>
#include <minefield/types.h>
#include <minefield/constants.h>
#include <minefield/utils.h>

#include <iostream>
#include <cstdio>
#include <format>
#include <string>

namespace GameStates
{
    NextState stateMainMenuUpdate(GameContext& context)
    {
        std::cout << MainMenu::kHeader;
        std::cout << std::format(MainMenu::kStart, MainMenu::Options::kStart);
        std::cout << std::format(MainMenu::kQuit, MainMenu::Options::kQuit);

        std::cout << MainMenu::kPrompt;

        int userSelection = 0;
        std::cin >> userSelection;

        NextState next = { nullptr };
        switch (userSelection)
        {
            case MainMenu::Options::kStart:
                next = { &stateEnteringBoardMeasures };
                break;
            case MainMenu::Options::kQuit:
                std::cout << MainMenu::kThanksForPlaying;
                next = { nullptr };
                break;
            default:
                std::cout << MainMenu::kInvalidOption;
                std::cout << MainMenu::kPrompt;
                next = context.currentState;
                break;
        }
        return next;
    }

    NextState stateEnteringBoardMeasures(GameContext& context)
    {
        std::cout << BoardConfig::kHeader;
        std::cout << BoardConfig::kConfigMsg;

        context.width = Width(utils::enterValueInRange(BoardConfig::kEnterWidth, BoardConfig::Limits::kMinWidth, BoardConfig::Limits::kMaxdWidth));
        context.height = Height(utils::enterValueInRange(BoardConfig::kEnterHeight, BoardConfig::Limits::kMinHeight, BoardConfig::Limits::kMaxHeight));
        
        utils::board::initialize(context.board, context.height, context.width);

        std::cout << std::format(BoardConfig::kSetMsg, context.width.raw(), context.height.raw());

        return { &stateEnteringMineCount };
    }

    NextState stateEnteringMineCount(GameContext &context)
    {
        std::cout << MineConfig::kHeader;
        std::cout << MineConfig::kExplain;

        context.initialMines = utils::enterValueInRange(MineConfig::kEnterMines, MineConfig::Limits::kMin, MineConfig::Limits::kMax);
        context.mines = context.initialMines;
        
        return { &stateCreatingPlayers };
    }

    NextState stateCreatingPlayers(GameContext& context)
    {
        std::cout << PlayerCreation::kHeader;

        utils::player::addPlayers(context.players, context.initialMines);

        if (context.players.empty())
        {
            std::cout << PlayerCreation::kZeroAdded;
            return { nullptr };
        }
        else if (context.players.size() == 1)
        {
            Player player = utils::player::getPCPlayer(context.initialMines);
            context.players.push_back(player);

            std::cout << std::format(PlayerCreation::kPCAdded, context.players[0].name, player.name);
        }
        else
        {
            std::cout << std::format(PlayerCreation::kCreated, context.players.size());
        }

        return { &statePuttingMines };
    }

    NextState statePuttingMines(GameContext& context)
    {
        if (context.players.empty())
        {
            std::cout << utilsMsg::kEmptyPlayers;
            return { &stateCreatingPlayers };
        }

        std::cout << PuttingMines::kHeader;

        unsigned int minesToPlace = 0;

        if (context.round == 1)
        {
            minesToPlace = context.initialMines;

            std::cout << std::format(PuttingMines::kFirstRound);
            std::cout << std::format(PuttingMines::kPlayersWillPlaceMines, minesToPlace);
        }
        else
        {
            std::cout << std::format(PuttingMines::kRoundNumber, context.round);

            // If there are more than two players, the number of mines a player can guess 
            // is limited to the player with the fewest mines

            minesToPlace = utils::player::whoHasLessAvailableMines(context.players);

            if (minesToPlace == 0)
            {
                std::cout << std::format(PuttingMines::kNoAvailableMines);
                return { nullptr };
            }
            else
            {
                std::cout << std::format(PuttingMines::kPlayersWillPlaceMines, minesToPlace);
                context.mines = minesToPlace; 
            }
        }
        
        for (auto& player : context.players)
        {
            std::cout << std::format(PuttingMines::kPlayerTurn, player.name);

            player.enterMine(context, player);

            std::cout << std::format(utilsMsg::kBoardOfPlayerPrompt, player.name);
            utils::board::printPerPlayer(context.width, context.height, context.board, player);
        }

        context.round++;

        return { &stateProcessingMines };
    }

    NextState stateProcessingMines(GameContext& context)
    {
        std::cout << ProcessingMines::kHeader;

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
            std::cout << ProcessingMines::kNoCollisions;
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
                        std::cout << std::format(ProcessingMines::kColissionMsg, mine.x, mine.y);

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
        std::cout << GuessingMines::kHeader;
        
        // The number of guesses the players can make is the same
        // to the number of mines they can place
        
        std::cout << std::format(GuessingMines::kTotalMsg, context.mines);

        for (auto& player : context.players)
        {
            std::cout << std::format(GuessingMines::kPlayerTurn, player.name);

            for (unsigned int i = 0; i < context.mines; i++)
            {
                MinePosition minePosition = utils::board::validBoardPositionState(context.width, context.height, player);

                std::cout << std::format(GuessingMines::kSuccess, player.name, minePosition.x, minePosition.y);
                
                player.placedGuesses.push_back(minePosition);
            }
        }

        return { &stateProcessingGuesses };
    }

    NextState stateProcessingGuesses(GameContext& context)
    {
        std::cout << ProcessingGuesses::kHeader;

        for (auto& player : context.players)
        {
            std::cout << std::format(ProcessingGuesses::kPlayerHeader, player.name);

            for (auto const& guess : player.placedGuesses)
            {
                // If the mine is from the player, it reduces the amount of mines it can place

                if (utils::player::isMineFromPlayer(guess, player.minesHistory)) 
                {
                    utils::game::handleOwnMine(player, guess, context.board);
                } 
                else if (context.board[guess.x][guess.y].state == PositionState::WithMine) 
                {
                    utils::game::handleOpponentMine(player, guess, context.board, context.players);
                } 
                else 
                {
                    utils::game::handleMiss(player, guess, context.board);
                }
            }

            utils::player::saveGuesses(player);
            
            std::cout << std::format(utilsMsg::kBoardOfPlayerPrompt, player.name);
            utils::board::printPerPlayer(context.width, context.height, context.board, player);
        }

        std::cout << ProcessingGuesses::kCurrentScoresHeader;
            
        for (auto const& player : context.players)
        {
            std::cout << std::format(ProcessingGuesses::kScoreLine, player.name, player.opponentMinesDetected, player.ownMinesDetected);
        }

        return { &stateCheckingNextTurn };
    }


    NextState stateCheckingNextTurn(GameContext& context)
    {
        std::cout << std::format(Results::kHeader, (context.round - 1));

        Players winners;
        Players eliminated;

        for (auto const& player : context.players)
        {
            unsigned int totalOpponentMines = utils::player::countOpponentMines(player, context.players);

            std::cout << std::format(Results::kPlayerInformation, player.name, player.opponentMinesDetected, totalOpponentMines, player.remainingMines);

            if (player.opponentMinesDetected >= totalOpponentMines && totalOpponentMines > 0)
            {
                winners.push_back(player);
            }
            
            // Players who can't place more mines are removed

            if (player.remainingMines == 0)
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
    
        if (utils::player::areThereWinners(winners) 
            || utils::game::hasOnePlayer(context.players) 
            || utils::board::isFull(context.width, context.height, context.board, context.players))
        {
            return { nullptr };
        }

        std::cout << std::format(Results::kProceedRound, context.round);
        
        return { &statePuttingMines };
    }
}