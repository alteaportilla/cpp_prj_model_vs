#pragma once

#include "types.h"
#include "constants.h"

#include <iostream>
#include <set>
#include <string>
#include <format>

namespace utils
{

template <typename T, typename U>
T enterValue(U message)
{
    T value;
    std::cout << message;
    std::cin >> value;
    return value;
}

template <typename T>
bool isInRange(T value, T min, T max)
{
    return (value >= min && value <= max);
}

template <typename T>
T enterValueInRange(std::string const& message, T min, T max)
{
    std::string msgWithMinMax = std::vformat(std::string(message), std::make_format_args(min, max));
    T value = utils::enterValue<T>(msgWithMinMax);

    while (!utils::isInRange(value, min, max))
    {
        std::string msg = utilsMsg::kTryAgain + msgWithMinMax;
        value = utils::enterValue<T>(msg);
    }

    return value;
}

unsigned int getRandomNumberInRange(int max); 

namespace game
{

void enterMine(GameContext& context, Player& player);
bool hasOnePlayer(Players const& players);
void handleOwnMine(Player& player, MinePosition const& mine, Board& board);
void handleOpponentMine(Player& player, MinePosition const& mine, Board& board, Players const& players);
void handleMiss(Player const& player, MinePosition const& mine, Board& board);

} // namespace game

namespace player
{

Player getPCPlayer(MinesCount initialMines);
void addPlayers(Players& players, MinesCount initialMines);
bool nameExists(std::string const& name, std::vector<Player> const& players);
char getType(std::string const& name);
Player createPlayer(std::string const& name, MinesCount initialMines, char type);
void saveMines(Player& player);
void saveGuesses(Player& player);
Player const* getTopScorer(Players const& players);
bool areThereWinners(Players const& winners);
Players getRemainigPlayers(Players const& players, Players const& removed);
int countOpponentMines(Player const& player, Players const& players);
bool isMineFromPlayer(MinePosition const& guess, std::vector<MinePosition> const& minePositions);
GuessesCount whoHasLessAvailableMines(Players const& players);

} // namespace players

namespace board
{

int getStateValue(PositionState state);
bool hasEmptyPositions(Width width, Height height, Board const& board);
bool isFull(Width width, Height height, Board const& board, Players const& players);
void printPerPlayer(Width width, Height height, Board const& board, Player const& player);
MinePosition getRandomBoardPosition(Width width, Height height);
MinePosition enterBoardPosition(Width width, Height height, Player const& player, RandomPosFn randomPos);
std::string showInvalidBoardPositionStateReason(PositionState const& state);
bool isInvalidBoardPositionState(PositionState const& state);
MinePosition validBoardPositionState(Width width, Height height, Player const& player);
void initialize(Board& board, Height height, Width width);

} // namespace board

} // namespace utils