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
        std::cout << context.language["utilsMsg::kEmptyBoard"];
    }

    for (unsigned int i = 0; i < context.mines.getValue(); i++)
    {
        unsigned int iPlus1 = i + 1;
        std::cout << std::vformat(context.language["PuttingMines::kMessage"], std::make_format_args(iPlus1, context.mines.getValue()));

        MinePosition minePosition = utils::board::validBoardPositionState(context.language, context.width, context.height, player);
        context.board[minePosition.x][minePosition.y] = minePosition;

        std::cout << std::vformat(context.language["PuttingMines::kSuccessMessage"], std::make_format_args(player.name, minePosition.x, minePosition.y));

        player.placedMines.push_back(minePosition);
    }

    utils::player::saveMines(player);
}

bool hasOnePlayer(Language& language, Players const& players)
{
    if (players.size() > 1)
    {
        return false;
    }

    std::cout << language["Results::kHeaderGameOver"];

    if (players.size() == 1)
    {
        std::cout << std::vformat(language["Results::kWinnerByElimination"], std::make_format_args(players[0].name));
    }
    else
    {
        std::cout << language["Results::kNoPlayersRemainingTie"];
    }

    return true;
}

void handleOwnMine(Language& language, Player& player, MinePosition const& mine, Board& board)
{
    if (board.empty())
    {
        std::cout << language["utilsMsg::kEmptyBoard"];
    }

    std::cout << std::vformat(language["ProcessingGuesses::kHitOwnMine"], std::make_format_args(player.name, mine.x, mine.y));
    player.ownMinesDetected.setValue(player.ownMinesDetected.getValue() + 1);

    if (player.remainingMines.getValue() > 0)
    {
        std::cout << std::vformat(language["ProcessingGuesses::kMinesRemaining"], std::make_format_args(player.remainingMines.getValue()));
        player.remainingMines.setValue(player.remainingMines.getValue() - 1);
        board[mine.x][mine.y].state = PositionState::Removed;
    }
}

void handleOpponentMine(Language& language, Player& player, MinePosition const& mine, Board& board, Players const& players)
{
    if (board.empty())
    {
        std::cout << language["utilsMsg::kEmptyBoard"];
    }

    if (players.empty())
    {
        std::cout << language["utilsMsg::kEmptyPlayers"];
    }

    // If the position has a mine, the player detected a mine from other player

    std::cout << std::vformat(language["ProcessingGuesses::kHitOpponentMine"], std::make_format_args(player.name, mine.x, mine.y));
    player.opponentMinesDetected.setValue(player.opponentMinesDetected.getValue() + 1);
    board[mine.x][mine.y].state = PositionState::GuessedMine;

    for (auto const& opponent : players)
    {
        if (opponent.name != player.name && utils::player::isMineFromPlayer(mine, opponent.placedMines))
        {
            std::cout << std::vformat(language["ProcessingGuesses::kItWasPlayersMine"], std::make_format_args(opponent.name));
            break;
        }
    }
}

void handleMiss(Language& language, Player const& player, MinePosition const& mine, Board& board)
{
    if (board.empty())
    {
        std::cout << language["utilsMsg::kEmptyBoard"];
    }

    std::cout << std::vformat(language["ProcessingGuesses::kMiss"], std::make_format_args(player.name, mine.x, mine.y));
    board[mine.x][mine.y].state = PositionState::GuessedEmpty;
}

} // namespace game

namespace player
{

Player getPCPlayer(Language& language, MinesCount initialMines)
{
    char type = PlayerCreation::Options::kPC;
    std::string PCName = language["PlayerCreation::kPCName"];
    Player player = utils::player::createPlayer(PCName, initialMines, type);
    return player;
}

void addPlayers(Language& language, Players& players, MinesCount initialMines)
{
    std::string message = std::vformat(language["PlayerCreation::kNamePrompt"], std::make_format_args(PlayerCreation::Options::kStopCreation));
    auto name = utils::enterValue<std::string>(message);

    // PlayerCreation::Options::kStopCreation is a char '*'
    // It's casted to std::string to be compared with name (std::string)

    std::string stopCreation = std::string(1, PlayerCreation::Options::kStopCreation);

    while (name != stopCreation)
    {
        if (utils::player::nameExists(name, players))
        {
            std::cout << std::vformat(language["PlayerCreation::kRepeatedName"], std::make_format_args(name));
        }
        else
        {
            char type = utils::player::getType(language, name);

            Player newPlayer = createPlayer(name, initialMines, type);

            players.push_back(newPlayer);

            std::cout << std::vformat(language["PlayerCreation::kAdded"], std::make_format_args(name));
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

char getType(Language& language, std::string const& name)
{
    std::string message = std::vformat(language["PlayerCreation::kTypePrompt"], std::make_format_args(name, PlayerCreation::Options::kHuman, PlayerCreation::Options::kPC));
    auto type = utils::enterValue<char>(message);

    while (!isTypeValid(type))
    {
        message = std::vformat(language["PlayerCreation::kInvalidType"], std::make_format_args(PlayerCreation::Options::kHuman, PlayerCreation::Options::kPC));
        type = utils::enterValue<char>(message);
    }

    return type;
}

Player createPlayer(std::string const& name, MinesCount initialMines, char type)
{
    Player player;

    player.name = name;
    player.remainingMines = initialMines;
    player.remainingGuesses.setValue(initialMines.getValue());
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

Player const* getTopScorer(Language& language, Players const& players)
{
    Player const* topPlayer = nullptr;

    unsigned int maxScore = 0;

    for (auto const& player : players)
    {
        unsigned int score = player.opponentMinesDetected.getValue() - player.ownMinesDetected.getValue();

        std::cout << std::vformat(language["Results::kScoreOfPlayer"], std::make_format_args(player.name, score));

        if (score > maxScore)
        {
            maxScore = score;
            topPlayer = &player;
        }
    }

    return topPlayer;
}

bool areThereWinners(Language& language, Players const& winners)
{
    if (winners.empty())
    {
        return false;
    }

    std::cout << language["Results::kHeaderGameOverWinner"];

    if (winners.size() == 1)
    {
        std::cout << std::vformat(language["Results::kWinnerWins"], std::make_format_args(winners[0].name));
        std::cout << language["Results::kCongratulations"];
    }
    else
    {
        std::cout << language["Results::kTie"];
        std::cout << language["Results::kWinnersListHeader"];
        for (auto const& winner : winners)
        {
            std::cout << std::vformat(language["Results::kWinnerListItem"], std::make_format_args(winner.name));
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

GuessesCount whoHasLessAvailableMines(Players const& players)
{
    GuessesCount lessGuesses{std::numeric_limits<unsigned int>::max()};
    for (auto const& player : players)
    {
        if (player.remainingMines.getValue() < lessGuesses.getValue())
        {
            lessGuesses.setValue(player.remainingMines.getValue());
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

bool isFull(Language& language, Width width, Height height, Board const& board, Players const& players)
{
    if (utils::board::hasEmptyPositions(width, height, board))
    {
        return false;
    }

    // If the game ended because of the board being full,
    // the winner is determined by the number of mines it guessed

    std::cout << language["Results::kHeaderGameOverBoardFull"];
    std::cout << language["Results::kNoMorePositions"];
    std::cout << language["Results::kFinalScores"];

    Player const* topPlayer = utils::player::getTopScorer(language, players);

    if (topPlayer != nullptr)
    {
        std::cout << std::vformat(language["Results::kWinnerByPoints"], std::make_format_args(topPlayer->name));
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

MinePosition enterBoardPosition(Language& language, Width width, Height height, Player const& player, RandomPosFn randomPos)
{
    MinePosition minePosition;
    if (player.type == PlayerType::HumanPlayer)
    {
        std::string msgX = language["utilsMsg::kEnterXValue"];
        auto xPos = utils::enterValueInRange<unsigned int>(language, msgX, static_cast<unsigned int>(0), (width.getValue() - 1));
        std::string msgY = language["utilsMsg::kEnterYValue"];
        auto yPos = utils::enterValueInRange<unsigned int>(language, msgY, static_cast<unsigned int>(0), (height.getValue() - 1));
        minePosition = {xPos, yPos};
    }
    else if (player.type == PlayerType::PC)
    {
        minePosition = randomPos(width, height);
    }
    return minePosition;
}

std::string showInvalidBoardPositionStateReason(Language& language, PositionState const& state)
{
    std::string message;
    switch (state)
    {
    case PositionState::GuessedEmpty:
        message = language["utilsMsg::kAlreadyGuessedMessage"];
        break;
    case PositionState::GuessedMine:
        message = language["utilsMsg::kAlreadyDetectedMessage"];
        break;
    case PositionState::Removed:
        message = language["utilsMsg::kRemovedMessage"];
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

MinePosition validBoardPositionState(Language& language, Width width, Height height, Player const& player)
{
    MinePosition minePosition = enterBoardPosition(language, width, height, player, getRandomBoardPosition);

    while (isInvalidBoardPositionState(minePosition.state))
    {
        showInvalidBoardPositionStateReason(language, minePosition.state);
        minePosition = enterBoardPosition(language, width, height, player, getRandomBoardPosition);
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