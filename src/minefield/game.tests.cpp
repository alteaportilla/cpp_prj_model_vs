#include <gtest/gtest.h>
#include <minefield/utils.h>
#include <minefield/game_states.h>
#include <iostream>

namespace minefield::game::tests
{
class GameTestSuit : public ::testing::Test
{
protected:
    void addPlayer(std::string name, unsigned int remainingMines)
    {
        Player player;
        player.name = name;
        player.remainingMines = remainingMines;
        context.players.push_back(player);
    }

    void setBoard(unsigned int width, unsigned int height) 
    {
        context.board;
        context.width = width;
        context.height = height;
        utils::board::initialize(context.board, context.width, context.height);
    }

    GameContext context;
};

TEST_F(GameTestSuit, should_continue_game_if_two_players_have_mines)
{
    setBoard(10, 10);
    addPlayer("p1", 3);
    addPlayer("p2",3);
    NextState nextState = GameStates::stateCheckingNextTurn(context);

    EXPECT_EQ(nextState.updateFunction, GameStates::statePuttingMines);
}

TEST_F(GameTestSuit, should_continue_game_if_at_least_two_players_have_mines)
{
    setBoard(10, 10);
    addPlayer("p1", 3);
    addPlayer("p2", 5);
    addPlayer("p3", 0);
    addPlayer("p4", 0);
    NextState nextState = GameStates::stateCheckingNextTurn(context);

    EXPECT_EQ(nextState.updateFunction, GameStates::statePuttingMines);
} 

TEST_F(GameTestSuit, should_finish_game_if_board_is_not_initialized)
{
    addPlayer("p1", 3);
    addPlayer("p2", 3);

    EXPECT_EQ(GameStates::stateCheckingNextTurn(context).updateFunction, nullptr);
}

TEST_F(GameTestSuit, should_finish_game_if_only_one_player_has_mines)
{
    setBoard(10, 10);
    addPlayer("p1", 3);
    addPlayer("p2", 0);
    NextState nextState = GameStates::stateCheckingNextTurn(context);
    
    EXPECT_EQ(nextState.updateFunction, nullptr);
}  

TEST_F(GameTestSuit, should_finish_game_if_players_are_not_initialized)
{
    setBoard(10, 10);
    NextState nextState = GameStates::stateCheckingNextTurn(context);

    EXPECT_EQ(nextState.updateFunction, nullptr);
}
}
