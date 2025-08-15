#pragma once

#include "types.h"
#include "constants.h"

#include <iostream>
#include <set>
#include <string>
#include <format>

namespace utils
{

unsigned int enterNumberInRange(char const* message, unsigned int min, unsigned int max);
unsigned int createRandomNumberInRange(int max); // getRandomNumberInRange
//void checkPlayerGuessedOwnMine(Player& player, MinePosition const& mine, Board& board);

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

Player getPCPlayer(unsigned int initialMines);
void addPlayers(Players& players, unsigned int initialMines);
std::string getName();
bool nameExists(std::string const& name, std::vector<Player> const& players);
char getType(std::string const& name);
Player createPlayer(std::string const& name, unsigned int initialMines, char type);
void saveMines(Player& player);
void saveGuesses(Player& player);
Player const* getTopScorer(Players const& players);
bool areThereWinners(Players const& winners);
Players getRemainigPlayers(Players const& players, Players const& removed);
int countOpponentMines(Player const& player, Players const& players);
bool isMineFromPlayer(MinePosition const& guess, std::vector<MinePosition> const& minePositions);
int whoHasLessAvailableMines(Players const& players);

} // namespace players

namespace board
{

int getStateValue(PositionState state);
bool hasEmptyPositions(unsigned int width, unsigned int height, Board const& board);
bool isFull(unsigned int width, unsigned int height, Board const& board, Players const& players);
void printPerPlayer(unsigned int width, unsigned int height, Board const& board, Player const& player);
MinePosition createRandomBoardPosition(unsigned int width, unsigned int height); // getRandomBoardPosition
MinePosition enterBoardPosition(unsigned int width, unsigned int height, Player const& player, RandomPosFn randomPos);
std::string showInvalidPositionReason(PositionState const& state);
bool isInvalidPosition(PositionState const& state);
MinePosition validPosition(unsigned int width, unsigned int height, Player const& player); // validBoardPositionState
void initialize(Board& board, unsigned int height, unsigned int width);

} // namespace board

} // namespace utils