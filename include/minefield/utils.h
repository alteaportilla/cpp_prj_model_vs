#pragma once

#include "types.h"
#include "constants.h"

#include <iostream>
#include <set>
#include <string>
#include <format>
#include <unordered_map>

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
T enterValueInRange(Language& language, std::string const& message, T min, T max)
{
    std::string msgWithMinMax = std::vformat(std::string(message), std::make_format_args(min, max));
    T value = utils::enterValue<T>(msgWithMinMax);

    while (!utils::isInRange(value, min, max))
    {
        std::string msg = language["utilsMsg::kTryAgain"] + msgWithMinMax;
        value = utils::enterValue<T>(msg);
    }

    return value;
}

unsigned int getRandomNumberInRange(int max); 

namespace game
{

void enterMine(GameContext& context, Player& player);
bool hasOnePlayer(Language& language, Players const& players);
void handleOwnMine(Language& language, Player& player, MinePosition const& mine, Board& board);
void handleOpponentMine(Language& language, Player& player, MinePosition const& mine, Board& board, Players const& players);
void handleMiss(Language& language, Player const& player, MinePosition const& mine, Board& board);

} // namespace game

namespace player
{

Player getPCPlayer(Language& language, MinesCount initialMines);
void addPlayers(Language& language, Players& players, MinesCount initialMines);
bool nameExists(std::string const& name, std::vector<Player> const& players);
char getType(Language& language, std::string const& name);
Player createPlayer(std::string const& name, MinesCount initialMines, char type);
void saveMines(Player& player);
void saveGuesses(Player& player);
Player const* getTopScorer(Language& language, Players const& players);
bool areThereWinners(Language& language, Players const& winners);
Players getRemainigPlayers(Players const& players, Players const& removed);
int countOpponentMines(Player const& player, Players const& players);
bool isMineFromPlayer(MinePosition const& guess, std::vector<MinePosition> const& minePositions);
GuessesCount whoHasLessAvailableMines(Players const& players);

} // namespace players

namespace board
{

int getStateValue(PositionState state);
bool hasEmptyPositions(Width width, Height height, Board const& board);
bool isFull(Language& language, Width width, Height height, Board const& board, Players const& players);
void printPerPlayer(Width width, Height height, Board const& board, Player const& player);
MinePosition getRandomBoardPosition(Width width, Height height);
MinePosition enterBoardPosition(Language& language, Width width, Height height, Player const& player, RandomPosFn randomPos);
std::string showInvalidBoardPositionStateReason(Language& language, PositionState const& state);
bool isInvalidBoardPositionState(PositionState const& state);
MinePosition validBoardPositionState(Language& language, Width width, Height height, Player const& player);
void initialize(Board& board, Height height, Width width);

} // namespace board

} // namespace utils