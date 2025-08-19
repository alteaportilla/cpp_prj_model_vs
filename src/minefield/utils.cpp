#include <minefield/utils.h>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <format>
#include <set>

namespace utils
{

unsigned int getRandomNumberInRange(int max)
{
    unsigned int number = rand() % max;
    return number;
}

namespace game
{

void enterMine(GameContext& context, Player& player)
{
    if (context.board.empty())
    {
        std::cout << std::format(utilsMsg::kEmptyBoard);
    }

    for (unsigned int i = 0; i < context.mines; i++)
    {
        std::cout << std::format(PuttingMines::kMessage, (i + 1), context.mines);

        MinePosition minePosition = utils::board::validBoardPositionState(context.width, context.height, player);
        context.board[minePosition.x][minePosition.y] = minePosition;

        std::cout << std::format(PuttingMines::kSuccessMessage, player.name, minePosition.x, minePosition.y);

        player.placedMines.push_back(minePosition);
    }

    utils::player::saveMines(player);
}

bool hasOnePlayer(Players const& players)
{
    if (players.size() > 1)
    {
        return false;
    }

    std::cout << Results::kHeaderGameOver;

    if (players.size() == 1)
    {
        std::cout << std::format(Results::kWinnerByElimination, players[0].name);
    }
    else
    {
        std::cout << Results::kNoPlayersRemainingTie;
    }

    return true;
}

void handleOwnMine(Player& player, MinePosition const& mine, Board& board)
{
    if (board.empty())
    {
        std::cout << std::format(utilsMsg::kEmptyBoard);
    }

    std::cout << std::format(ProcessingGuesses::kHitOwnMine, player.name, mine.x, mine.y);
    player.ownMinesDetected++;

    if (player.remainingMines > 0)
    {
        std::cout << std::format(ProcessingGuesses::kMinesRemaining, player.remainingMines);
        player.remainingMines--;
        board[mine.x][mine.y].state = PositionState::Removed;
    }
}

void handleOpponentMine(Player& player, MinePosition const& mine, Board& board, Players const& players)
{
    if (board.empty())
    {
        std::cout << std::format(utilsMsg::kEmptyBoard);
    }

    if (players.empty())
    {
        std::cout << std::format(utilsMsg::kEmptyPlayers);
    }

    // If the position has a mine, the player detected a mine from other player

    std::cout << std::format(ProcessingGuesses::kHitOpponentMine, player.name, mine.x, mine.y);
    player.opponentMinesDetected++;
    board[mine.x][mine.y].state = PositionState::GuessedMine;

    for (auto const& opponent : players)
    {
        if (opponent.name != player.name && utils::player::isMineFromPlayer(mine, opponent.placedMines))
        {
            std::cout << std::format(ProcessingGuesses::kItWasPlayersMine, opponent.name);
            break;
        }
    }
}

void handleMiss(Player const& player, MinePosition const& mine, Board& board)
{
    if (board.empty())
    {
        std::cout << std::format(utilsMsg::kEmptyBoard);
    }

    std::cout << std::format(ProcessingGuesses::kMiss, player.name, mine.x, mine.y);
    board[mine.x][mine.y].state = PositionState::GuessedEmpty;
}

} // namespace game

namespace player
{

Player getPCPlayer(unsigned int initialMines)
{
    char type = PlayerCreation::Options::kPC;
    Player player = utils::player::createPlayer(PlayerCreation::kPCName, initialMines, type);
    return player;
}

void addPlayers(Players& players, unsigned int initialMines)
{
    std::string message = std::format(PlayerCreation::kNamePrompt, PlayerCreation::Options::kStopCreation);
    auto name = utils::enterValue<std::string>(message);

    // PlayerCreation::Options::kStopCreation is a char '*'
    // It's casted to std::string to be compared with name (std::string)

    std::string stopCreation = std::string(1, PlayerCreation::Options::kStopCreation);

    while (name != stopCreation)
    {
        if (utils::player::nameExists(name, players))
        {
            std::cout << std::format(PlayerCreation::kRepeatedName, name);
        }
        else
        {
            char type = utils::player::getType(name);

            Player newPlayer = createPlayer(name, initialMines, type);

            players.push_back(newPlayer);

            std::cout << std::format(PlayerCreation::kAdded, name);
        }

        name = utils::enterValue<std::string>(message);
    }
}

bool nameExists(std::string const& name, std::vector<Player> const& players)
{
    for (auto const& player : players)
    {
        if (player.name == name)
        {
            return true;
        }
    }
    return false;
}

bool isTypeValid(char type)
{
    return (type == PlayerCreation::Options::kHuman || type == PlayerCreation::Options::kPC);
}

char getType(std::string const& name)
{
    std::string message = std::format(PlayerCreation::kTypePrompt, name, PlayerCreation::Options::kHuman, PlayerCreation::Options::kPC);
    auto type = utils::enterValue<char>(message);

    while (!isTypeValid(type))
    {
        message = std::format(PlayerCreation::kInvalidType, PlayerCreation::Options::kHuman, PlayerCreation::Options::kPC);
        type = utils::enterValue<char>(message);
    }

    return type;
}

Player createPlayer(std::string const& name, unsigned int initialMines, char type)
{
    Player player;

    player.name = name;
    player.remainingMines = initialMines;
    player.remainingGuesses = initialMines;
    player.enterMine = &utils::game::enterMine;
    player.type = (type == PlayerCreation::Options::kHuman) ? PlayerType::HumanPlayer : PlayerType::PC;

    return player;
}

void saveMines(Player& player)
{
    for (auto const& mine : player.placedMines)
    {
        player.minesHistory.push_back(mine);
    }
}

void saveGuesses(Player& player)
{
    for (auto const& mine : player.placedGuesses)
    {
        player.guessesHistory.push_back(mine);
    }
}

Player const* getTopScorer(Players const& players)
{
    Player const* topPlayer = nullptr;

    unsigned int maxScore = 0;

    for (auto const& player : players)
    {
        unsigned int score = player.opponentMinesDetected - player.ownMinesDetected;

        std::cout << std::format(Results::kScoreOfPlayer, player.name, score);

        if (score > maxScore)
        {
            maxScore = score;
            topPlayer = &player;
        }
    }

    return topPlayer;
}

bool areThereWinners(Players const& winners)
{
    if (winners.empty())
    {
        return false;
    }

    std::cout << Results::kHeaderGameOverWinner;

    if (winners.size() == 1)
    {
        std::cout << std::format(Results::kWinnerWins, winners[0].name);
        std::cout << Results::kCongratulations;
    }
    else
    {
        std::cout << Results::kTie;
        std::cout << Results::kWinnersListHeader;
        for (auto const& winner : winners)
        {
            std::cout << std::format(Results::kWinnerListItem, winner.name);
        }
    }

    return true;
}

Players getRemainigPlayers(Players const& players, Players const& removed)
{
    Players result;

    for (auto const& player : players)
    {
        if (std::find(removed.begin(), removed.end(), player) == removed.end())
        {
            result.push_back(player);
        }
    }

    return result;
}

int countOpponentMines(Player const& player, Players const& players)
{
    int totalOpponentMines = 0;

    for (auto const& other : players)
    {
        if (other.name != player.name)
        {
            totalOpponentMines += other.placedMines.size();
        }
    }

    return totalOpponentMines;
}

bool isMineFromPlayer(MinePosition const& guess, std::vector<MinePosition> const& minePositions)
{
    for (auto const& minePosition : minePositions)
    {
        if (minePosition == guess)
        {
            return true;
        }
    }
    return false;
}

unsigned int whoHasLessAvailableMines(Players const& players)
{
    unsigned int lessGuesses = std::numeric_limits<unsigned int>::max();
    for (auto const& player : players)
    {
        if (player.remainingMines < lessGuesses)
        {
            lessGuesses = player.remainingMines;
        }
    }
    return lessGuesses;
}

} // namespace players

namespace board
{

int getStateValue(PositionState state)
{
    return static_cast<int>(state);
}

bool hasEmptyPositions(Width width, Height height, Board const& board)
{
    for (unsigned int i = 0; i < width.getValue(); ++i)
    {
        for (unsigned int j = 0; j < height.getValue(); ++j)
        {
            if (board[i][j].state == PositionState::Empty)
            {
                return true;
            }
        }
    }

    return false;
}

bool isFull(Width width, Height height, Board const& board, Players const& players)
{
    if (utils::board::hasEmptyPositions(width, height, board))
    {
        return false;
    }

    // If the game ended because of the board being full,
    // the winner is determined by the number of mines it guessed

    std::cout << Results::kHeaderGameOverBoardFull;
    std::cout << Results::kNoMorePositions;
    std::cout << Results::kFinalScores;

    Player const* topPlayer = utils::player::getTopScorer(players);

    if (topPlayer != nullptr)
    {
        std::cout << std::format(Results::kWinnerByPoints, topPlayer->name);
    }

    return true;
} 

/*
    How the board would look like:

    0  1  2  3 ...
    0  0  0  0  0 ...
    1  1  1  0  0 ...
    2  0  0  0  0 ...
    3  0  0  0  0 ...
    .. .. .. .. .. ...
*/

void printPerPlayer(Width width, Height height, Board const& board, Player const& player)
{
    std::cout << std::setw(Display::kBoardColWidth) << "";

    for (unsigned int i = 0; i < width.getValue(); ++i)
    {
        std::cout << std::setw(Display::kBoardColWidth) << i;
    }

    std::cout << '\n';

    for (unsigned int j = 0; j < height.getValue(); ++j)
    {
        std::cout << std::setw(Display::kBoardColWidth) << j;

        for (unsigned int k = 0; k < width.getValue(); ++k)
        {
            MinePosition minePositionFromBoard = board[j][k];

            // If the player has guessed this position (empty, mine, or own mine), show its actual state value.
            // If the player has placed a mine here but hasn't guessed it, show '1'.
            // Otherwise, show '0'.

            if (utils::player::isMineFromPlayer(minePositionFromBoard, player.guessesHistory) || minePositionFromBoard.state == PositionState::Removed)
            {
                std::cout << std::setw(Display::kBoardColWidth) << getStateValue(minePositionFromBoard.state);
            }
            else if (utils::player::isMineFromPlayer(minePositionFromBoard, player.minesHistory))
            {
                std::cout << std::setw(Display::kBoardColWidth) << 1;
            }
            else
            {
                std::cout << std::setw(Display::kBoardColWidth) << 0;
            }
        }

        std::cout << '\n';
    }
}

MinePosition getRandomBoardPosition(Width width, Height height)
{
    unsigned int xPos = getRandomNumberInRange(width.getValue());
    unsigned int yPos = getRandomNumberInRange(height.getValue());
    return {xPos, yPos};
}

MinePosition enterBoardPosition(Width width, Height height, Player const& player, RandomPosFn randomPos)
{
    MinePosition minePosition;
    if (player.type == PlayerType::HumanPlayer)
    {
        auto xPos = utils::enterValueInRange<unsigned int>(utilsMsg::kEnterXValue, static_cast<unsigned int>(0), (width.getValue() - 1));
        auto yPos = utils::enterValueInRange<unsigned int>(utilsMsg::kEnterYValue, static_cast<unsigned int>(0), (height.getValue() - 1));
        minePosition = {xPos, yPos};
    }
    else if (player.type == PlayerType::PC)
    {
        minePosition = randomPos(width, height);
    }
    return minePosition;
}

std::string showInvalidBoardPositionStateReason(PositionState const& state)
{
    std::string message;
    switch (state)
    {
    case PositionState::GuessedEmpty:
        message = utilsMsg::kAlreadyGuessedMessage;
        break;
    case PositionState::GuessedMine:
        message = utilsMsg::kAlreadyDetectedMessage;
        break;
    case PositionState::Removed:
        message = utilsMsg::kRemovedMessage;
        break;
    default:
        break;
    }
    return message;
}

bool isInvalidBoardPositionState(PositionState const& state)
{
    return (state == PositionState::GuessedEmpty || state == PositionState::GuessedMine || state == PositionState::Removed);
}

MinePosition validBoardPositionState(Width width, Height height, Player const& player)
{
    MinePosition minePosition = enterBoardPosition(width, height, player, getRandomBoardPosition);

    while (isInvalidBoardPositionState(minePosition.state))
    {
        showInvalidBoardPositionStateReason(minePosition.state);
        minePosition = enterBoardPosition(width, height, player, getRandomBoardPosition);
    }

    minePosition.state = PositionState::WithMine;

    return minePosition;
}

void initialize(Board& board, Height height, Width width)
{
    board.resize(height.getValue());
    for (unsigned int j = 0; j < height.getValue(); ++j)
    {
        board[j].resize(width.getValue());
        for (unsigned int k = 0; k < width.getValue(); ++k)
        {
            board[j][k] = MinePosition{j, k, PositionState::Empty};
        }
    }
}

} // namespace board

} // namespace utils