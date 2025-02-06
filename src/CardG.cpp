// CardG.cpp: Defines the entry point for the application.

#include "CardG.h"

struct Button {
    int id;
    SDL_Rect rect;
    SDL_Color color;
    std::string label;
    std::function<void()> onClick;
};

uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF) << 24) |
        ((hostlong & 0xFF00) << 8) |
        ((hostlong & 0xFF0000) >> 8) |
        ((hostlong & 0xFF000000) >> 24);
}
uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong); // Same operation as htonl
}

struct Card {
    int id;
    enum class Suit { Hearts, Diamonds, Clubs, Spades }; // Enum for suits
    enum class Rank { Two = 2, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace }; // Enum for ranks

    Suit suit;     // The suit of the card
    Rank rank;     // The rank of the card
    bool faceUp;   // True if the card is face-up, false if face-down

    // Constructor for initializing the card
    Card(int cardID, Suit s, Rank r, bool f = false) : id(cardID), suit(s), rank(r), faceUp(f) {}

    // Function to get a string representation of the card (for display or debugging)
    std::string toString() const {
        std::string rankStr;
        switch (rank) {
        case Rank::Two: rankStr = "2"; break;
        case Rank::Three: rankStr = "3"; break;
        case Rank::Four: rankStr = "4"; break;
        case Rank::Five: rankStr = "5"; break;
        case Rank::Six: rankStr = "6"; break;
        case Rank::Seven: rankStr = "7"; break;
        case Rank::Eight: rankStr = "8"; break;
        case Rank::Nine: rankStr = "9"; break;
        case Rank::Ten: rankStr = "10"; break;
        case Rank::Jack: rankStr = "Jack"; break;
        case Rank::Queen: rankStr = "Queen"; break;
        case Rank::King: rankStr = "King"; break;
        case Rank::Ace: rankStr = "Ace"; break;
        }

        std::string suitStr;
        switch (suit) {
        case Suit::Hearts: suitStr = "Hearts"; break;
        case Suit::Diamonds: suitStr = "Diamonds"; break;
        case Suit::Clubs: suitStr = "Clubs"; break;
        case Suit::Spades: suitStr = "Spades"; break;
        }

        return rankStr + " of " + suitStr + (faceUp ? " (Face-up)" : " (Face-down)") + " (ID: " + std::to_string(id) + ")";
    }
};

std::vector<Card> initializeDeck() {
    std::vector<Card> deck;
    int id = 0;  // Start ID from 0 for the first card

    // Loop over all suits and ranks to create all 52 cards
    for (int suit = 0; suit < 4; ++suit) {
        for (int rank = 2; rank <= 14; ++rank) {
            deck.push_back(Card(id++, static_cast<Card::Suit>(suit), static_cast<Card::Rank>(rank)));
        }
    }

    return deck;
}


struct Player {
    TCPsocket socket;                // Connection socket
    std::string name;                // Player name
    std::vector<Card> myCards;       // Cards in hand
    int score = 0;                   // Player's score (optional, for scoring games)
    bool isTurn = false;             // Whether it's the player's turn
    bool isConnected = true;         // Track if the player is connected
    bool isServer = false;
    bool isLose = false;
};


std::vector<Card> randdom(std::vector<Card> myne)
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(myne.begin(), myne.end(), g);

    //for (const auto& mcard : myne) {
    //    std::cout << mcard.toString() << std::endl;
    //}
    return myne;
}

void roundov(TCPsocket client, const std::string& message) {
    if (SDLNet_TCP_Send(client, message.c_str(), message.length() + 1) == 0) {
        std::cerr << "Error sending message: " << SDLNet_GetError() << std::endl;
    } else {
        std::cout << "Sent message: " << message << std::endl;
    }
}

std::string roundre(TCPsocket client) {
    char buffer[512];
    int bytesReceived = SDLNet_TCP_Recv(client, buffer, sizeof(buffer) - 1);
    
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0'; // Null-terminate the received data
        return std::string(buffer);
    } else {
        return "";
    }
}

Player checkturn(std::vector<Player>& pelyas)
{
 
   
        for(auto &p:pelyas)
        {
            for (size_t i = 0; i < p.myCards.size(); i++)
            {
                if (p.myCards[i].id == 51)
                {
                    p.isTurn = true;
                    return p;
                }
            }
            
        }
    
    
}

int nextTurn(std::vector<Player>& pelyas, int currentPlayerIndex) {
    // Set the current player's turn to false
    pelyas[currentPlayerIndex].isTurn = false;

    // Move to the next player (circular)
    currentPlayerIndex = (currentPlayerIndex + 1) % pelyas.size();

    // Set the next player's turn to true
    pelyas[currentPlayerIndex].isTurn = true;

    // Output the current player's turn using cout
    std::cout << "It's Player " << pelyas[currentPlayerIndex].name << "'s turn now!" << std::endl;
    return currentPlayerIndex;
}










int sendCardIDs(TCPsocket client, const std::vector<Card>& deck) {
    // Create a vector to hold just the card IDs
    std::vector<int32_t> cardIDs;

    // Extract card IDs from the deck (0-51)
    for (const Card& card : deck) {
        cardIDs.push_back(htonl(card.id)); // Convert card ID to network byte order
    }

    // Add the termination value (-1) at the end
    cardIDs.push_back(htonl(-1));  // Terminator to indicate end of data

    // Send the entire vector as one message
    int dataSize = cardIDs.size() * sizeof(int32_t);
    if (SDLNet_TCP_Send(client, cardIDs.data(), dataSize) < dataSize) {
        std::cerr << "Failed to send card IDs: " << SDLNet_GetError() << std::endl;
        return -1;
    }

    std::cout << "Sent all card IDs in one message with termination marker.\n";
    return 0;
}


int receiveCardIDs(TCPsocket client,Player& me,std::vector<Card> deck) {
    // Receive the entire message with card IDs
    std::vector<int32_t> networkCardIDs;

    // Define a buffer size large enough to hold the expected data
    int bufferSize = 1024;  // Adjust the buffer size as needed
    std::vector<int32_t> buffer(bufferSize);

    // Receive the data
    int bytesReceived = SDLNet_TCP_Recv(client, buffer.data(), bufferSize * sizeof(int32_t));
    if (bytesReceived < 0) {
        std::cerr << "Error receiving card IDs: " << SDLNet_GetError() << std::endl;
        return -1;
    }

    // Convert received data from network byte order to host byte order
    for (int i = 0; i < bytesReceived / sizeof(int32_t); ++i) {
        int cardID = ntohl(buffer[i]);  // Convert from network byte order to host byte order
        if (cardID == -1) {
            std::cout << "Termination marker received. End of card IDs.\n";
            break;
            //return 0;
        }
        else
        {

            std::cout << "Received card IDs: " << cardID << std::endl;
            for (const auto& card : deck) {
                if (card.id == cardID) {
                    me.myCards.push_back(card);
                    break;
                }
            }
        }
         
    }
    return 0;
        
}


void sendInt(TCPsocket socket, int value) {
    SDLNet_TCP_Send(socket, &value, sizeof(value));
}

int receiveInt(TCPsocket socket) {
    int value;
    SDLNet_TCP_Recv(socket, &value, sizeof(value));
    return value;
}









// Broadcast message to all players (excluding the server)
void broadcastMessage(std::vector<Player>& players, const char* message, TCPsocket serverSocket) {
    for (Player& player : players) {
        if (player.socket != serverSocket) {  // Don't send message to the server
            int result = SDLNet_TCP_Send(player.socket, message, strlen(message) + 1);
            if (result < strlen(message) + 1) {
                std::cerr << "Failed to send message to player: " << player.name << std::endl;
            }
            else {
                std::cout << "Sent message to " << player.name << std::endl;
            }
        }
    }
}



// Function to render a button with text
void renderButtonWithText(SDL_Renderer* renderer, const Button& button, TTF_Font* font) {
    SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
    SDL_RenderFillRect(renderer, &button.rect);

    // Render button label
    SDL_Color textColor = { 255, 255, 255, 255 };  // White text
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, button.label.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect textRect = {
        button.rect.x + (button.rect.w - textWidth) / 2,
        button.rect.y + (button.rect.h - textHeight) / 2,
        textWidth, textHeight
    };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}


// Server function
// Assuming the Player structure has been updated as needed.
void renderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Function to check if a point is inside a rectangle
bool isInside(const SDL_Rect& rect, int x, int y) {
    return x >= rect.x && x <= (rect.x + rect.w) && y >= rect.y && y <= (rect.y + rect.h);
}

// Function to generate card buttons
void generateCardButtons(SDL_Renderer* renderer, TTF_Font* font, const Player& player, std::vector<Button>& buttons) {
    int buttonWidth = 70;
    int buttonHeight = 30;
    int margin = 15;
    TCPsocket me = player.socket;
    int buttonsPerRow = 5;
    int startX = 10;
    int startY = 10;

    buttons.clear(); // Clear previous buttons

    for (size_t i = 0; i < player.myCards.size(); ++i) {
        const Card& card = player.myCards[i];

        // Create card label
        std::string rankStr;
        switch (card.rank) {
        case Card::Rank::Two: rankStr = "2"; break;
        case Card::Rank::Three: rankStr = "3"; break;
        case Card::Rank::Four: rankStr = "4"; break;
        case Card::Rank::Five: rankStr = "5"; break;
        case Card::Rank::Six: rankStr = "6"; break;
        case Card::Rank::Seven: rankStr = "7"; break;
        case Card::Rank::Eight: rankStr = "8"; break;
        case Card::Rank::Nine: rankStr = "9"; break;
        case Card::Rank::Ten: rankStr = "10"; break;
        case Card::Rank::Jack: rankStr = "J"; break;
        case Card::Rank::Queen: rankStr = "Q"; break;
        case Card::Rank::King: rankStr = "K"; break;
        case Card::Rank::Ace: rankStr = "A"; break;
        }

        std::string suitStr;
        switch (card.suit) {
        case Card::Suit::Hearts: suitStr = "H"; break;
        case Card::Suit::Diamonds: suitStr = "D"; break;
        case Card::Suit::Clubs: suitStr = "C"; break;
        case Card::Suit::Spades: suitStr = "S"; break;
        }

        std::string label = rankStr + " " + suitStr;

        int row = static_cast<int>(i) / buttonsPerRow;
        int col = static_cast<int>(i) % buttonsPerRow;

        Button button;
        button.rect = {
            startX + col * (buttonWidth + margin),
            startY + row * (buttonHeight + margin),
            buttonWidth,
            buttonHeight
        };
        button.color = { 50, 150, 250, 255 }; // Blue
        button.label = label;
        button.id = card.id;

        // Define click action for the button
        button.onClick = [label]() {
            
            std::cout << "Button clicked: " << label << std::endl;
        };

        buttons.push_back(button);

        // Render button
        SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
        SDL_RenderFillRect(renderer, &button.rect);

        // Center the text
        int textWidth, textHeight;
        TTF_SizeText(font, button.label.c_str(), &textWidth, &textHeight);
        int textX = button.rect.x + (button.rect.w - textWidth) / 2;
        int textY = button.rect.y + (button.rect.h - textHeight) / 2;

        renderText(renderer, font, button.label, { 255, 255, 255, 255 }, textX, textY); // White text
    }
}


void serGameRoom(Player& pl, std::vector<Player>& players, std::vector<Card> deck) {
    std::cout << "Your cards:\n";
    for (const Card& card : pl.myCards) {
        std::cout << card.toString() << "\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL or TTF: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("Player's Cards", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24); // Path to .ttf font file
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    std::vector<Button> buttons;
    bool running = true;
    SDL_Event event;
    std::vector<Card> tableCards;
    bool cardsUpdated = false;  // Track card updates

    // Initial check for the player whose turn it is
    Player turnPlayer = checkturn(players);  // Check whose turn it is
    roundov(turnPlayer.socket, "YouRTurn");  // Notify the current player it's their turn

    while (running) {
        if (pl.isTurn) {
            // Wait for player input during their turn
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    // Check which button (card) was clicked
                    for (Button& button : buttons) {
                        if (isInside(button.rect, mouseX, mouseY) && button.onClick) {
                            sendInt(pl.socket, button.id);  // Send the selected card ID to the server

                            // Find the selected card from the deck and add it to table
                            for (const Card& card : deck) {
                                if (card.id == button.id) {
                                    tableCards.push_back(card);
                                    break;
                                }
                            }

                            // Erase the played card from the player's hand
                            pl.myCards.erase(std::remove_if(pl.myCards.begin(), pl.myCards.end(),
                                [buttonId = button.id](const Card& card) {
                                    return card.id == buttonId;
                                }),
                                pl.myCards.end());

                            cardsUpdated = true;
                            pl.isTurn = false;  // End player's turn
                            button.onClick();  // Trigger button action (if any)
                            break;
                        }
                    }
                }
            }
        } else {
            // If it's not the player's turn, wait for the card ID from the server
            int cardId = receiveInt(pl.socket);
            for (const Card& card : deck) {
                if (card.id == cardId) {
                    tableCards.push_back(card);
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        // Regenerate buttons if cards were updated
        if (cardsUpdated) {
            generateCardButtons(renderer, font, pl, buttons);
            cardsUpdated = false;  // Reset the flag after regenerating buttons
        }

        // Server-side handling of the round and player status
        std::string roundStatus = roundre(pl.socket);
        if (roundStatus == "YouRTurn") {
            std::cout << "The round is over! Waiting for the next round...\n";
            tableCards.clear();
        } else if (roundStatus == "ROUND_OVER") {
            std::cout << "The round is over! Waiting for the next round...\n";
            tableCards.clear();
        } else if (roundStatus == "Fail") {
            std::cerr << "You " << roundStatus << "\n";
            pl.myCards.insert(pl.myCards.end(), tableCards.begin(), tableCards.end());  // Return cards to the player
            tableCards.clear();
        } else {
            std::cerr << "Unexpected message: " << roundStatus << "\n";
        }

        // Rendering phase
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);  // Background color
        SDL_RenderClear(renderer);

        // Render buttons (cards)
        for (const Button& button : buttons) {
            SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
            SDL_RenderFillRect(renderer, &button.rect);

            // Render text on the button (card label)
            int textWidth, textHeight;
            TTF_SizeText(font, button.label.c_str(), &textWidth, &textHeight);
            int textX = button.rect.x + (button.rect.w - textWidth) / 2;
            int textY = button.rect.y + (button.rect.h - textHeight) / 2;
            renderText(renderer, font, button.label, {255, 255, 255, 255}, textX, textY);  // White text
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}




void GameRoom(Player& pl, std::vector<Card> deck) {
    std::cout << "Your cards:\n";
    for (const Card& card : pl.myCards) {
        std::cout << card.toString() << "\n";
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // SDL and TTF initialization
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        std::cerr << "Failed to initialize SDL or TTF: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("Player's Cards", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 600, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 24); // Path to .ttf font file
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return;
    }

    std::vector<Button> buttons;
    bool running = true;
    SDL_Event event;
    std::vector<Card> tableCards;
    bool cardsUpdated = false;  // Track card updates

    while (running) {
        // Check for round status and turn change
        std::string roundStatus = roundre(pl.socket);
        if (roundStatus == "YouRTurn") {
            pl.isTurn = true;
            std::cout << "It's your turn!\n";
        } else if (roundStatus == "ROUND_OVER") {
            std::cout << "The round is over! Waiting for the next round...\n";
            tableCards.clear();
        } else if (roundStatus == "Fail") {
            std::cerr << "You " << roundStatus << "\n";
            pl.myCards.insert(pl.myCards.end(), tableCards.begin(), tableCards.end());
            tableCards.clear();
        } else {
            std::cerr << "Unexpected message: " << roundStatus << "\n";
        }

        if (pl.isTurn) {
            // Client-side: waiting for player input during their turn
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = event.button.x;
                    int mouseY = event.button.y;

                    for (Button& button : buttons) {
                        if (isInside(button.rect, mouseX, mouseY) && button.onClick) {
                            sendInt(pl.socket, button.id);  // Send the card ID to the server
                            for (const Card& card : deck) {
                                if (card.id == button.id) {
                                    tableCards.push_back(card);  // Add the card to the table
                                    break;
                                }
                            }

                            // Remove the card from the player's hand
                            pl.myCards.erase(std::remove_if(pl.myCards.begin(), pl.myCards.end(),
                                [buttonId = button.id](const Card& card) {
                                    return card.id == buttonId;
                                }),
                                pl.myCards.end());

                            cardsUpdated = true;
                            pl.isTurn = false;  // End the turn
                            break;
                        }
                    }
                }
            }
        } else {
            // Waiting for the server's response for non-turn players
            int cardId = receiveInt(pl.socket);
            for (const Card& card : deck) {
                if (card.id == cardId) {
                    tableCards.push_back(card);  // Add the card to the table
                    break;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
        if (cardsUpdated) {
            generateCardButtons(renderer, font, pl, buttons);
            cardsUpdated = false;  // Reset flag
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);  // Background color
        SDL_RenderClear(renderer);

        // Render buttons and cards
        for (const Button& button : buttons) {
            SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, button.color.a);
            SDL_RenderFillRect(renderer, &button.rect);

            int textWidth, textHeight;
            TTF_SizeText(font, button.label.c_str(), &textWidth, &textHeight);
            int textX = button.rect.x + (button.rect.w - textWidth) / 2;
            int textY = button.rect.y + (button.rect.h - textHeight) / 2;
            renderText(renderer, font, button.label, {255, 255, 255, 255}, textX, textY);  // White text
        }

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}






// Add server as a player
void createServer(std::vector<Card> deck) {
    if (SDLNet_Init() == -1) {
        std::cerr << "SDLNet_Init Error: " << SDLNet_GetError() << std::endl;
        return;
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, NULL, 12345) == -1) {
        std::cerr << "SDLNet_ResolveHost Error: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        return;
    }

    TCPsocket server = SDLNet_TCP_Open(&ip);
    if (!server) {
        std::cerr << "SDLNet_TCP_Open Error: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        return;
    }

    std::cout << "Cards Shuffled" << std::endl;
    std::vector<Card> RandCard = randdom(deck);

    std::cout << "Room created. Waiting for players on port 12345...\n";

    std::vector<Player> players;
    TCPsocket serverSocket = server;
    int currentPlayerIndex = 0;
    const int MAX_PLAYERS = 6;
    const int JOIN_TIME_LIMIT = 10;  // Time limit in seconds
    bool joiningClosed = false;  // Flag to track if joining period has ended
    auto startTime = std::chrono::steady_clock::now();
    std::set<TCPsocket> sentPlayers;  // Track players who have received their cards

    // Wait for players to join
    while (!joiningClosed) {
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<int> elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

        int remainingTime = JOIN_TIME_LIMIT - elapsedTime.count();
        if (remainingTime > 0) {
            std::cout << "Time remaining to join: " << remainingTime << " seconds.\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        else {
            std::cout << "Joining time has ended. No more players can join.\n";
            joiningClosed = true;
        }

        if (remainingTime > 0) {
            // Accept new players
            TCPsocket client = SDLNet_TCP_Accept(server);
            if (client) {
                if (players.size() < MAX_PLAYERS) {
                    char welcomeMessage[] = "Welcome to the game!";
                    SDLNet_TCP_Send(client, welcomeMessage, strlen(welcomeMessage) + 1);

                    players.push_back({ client, "Player " + std::to_string(players.size() + 1) });
                    std::cout << "Player joined. Total players: " << players.size() << "\n";

                    std::string playerCountMessage = "PLAYERS|" + std::to_string(players.size());
                    broadcastMessage(players, playerCountMessage.c_str(),serverSocket);
                }
                else {
                    char rejectionMessage[] = "Sorry, the room is full. Please try again later.";
                    SDLNet_TCP_Send(client, rejectionMessage, strlen(rejectionMessage) + 1);
                    SDLNet_TCP_Close(client);
                    std::cout << "Rejected a player, room is full.\n";
                }
            }
        }
    }

    // Add the server as a player
    players.push_back({ server, "Server" });

    // Distribute cards after all players have joined
    // Distribute cards after all players have joined
    if (joiningClosed && !players.empty()) {
        int pl = players.size();
        std::vector<std::vector<Card>> playerCards(pl); // Create a vector for each player

        for (size_t i = 0; i < RandCard.size(); ++i) {
            int playerIndex = i % pl; // Cycle through players (including the server)
            playerCards[playerIndex].push_back(RandCard[i]);
        }

        // Send 'GAME_STARTED' message to each player
        for (Player& player : players) {
            std::string gameStartMessage = "GAME_STARTED";
            int result = SDLNet_TCP_Send(player.socket, gameStartMessage.c_str(), gameStartMessage.size() + 1);
            if (result < gameStartMessage.size() + 1) {
                std::cerr << "Failed to send 'game started' message to player: " << player.name << "\n";
                continue;  // Skip this player if the message failed to send
            }
            std::cout << "Sent 'GAME_STARTED' to " << player.name << "\n";
        }

        // Send cards to each player
        for (size_t i = 0; i < players.size(); ++i) {
            Player& player = players[i];

            // Assign cards to player (including the server)
            player.myCards = playerCards[i];

            // Send card IDs
            int result = sendCardIDs(player.socket, player.myCards);
            if (result == -1) {
                std::cerr << "Failed to send cards to player: " << player.name << "\n";
            }
            else {
                std::cout << "Sent cards to " << player.name << "\n";
                sentPlayers.insert(player.socket);  // Mark this player as having received their cards
            }
        }



    }

    
    // Main Game Loop: Turn handling
    // Main Game Loop: Server plays like any other player
    
    Player& myserver = players.back();

    myserver.isServer = true;
    serGameRoom(myserver,players,deck);
        
                
            
        
        
    
    



    SDLNet_TCP_Close(server);
    SDLNet_Quit();
}
void joinClient(std::vector<Card> deck) {
    // Initialize SDL_net
    if (SDLNet_Init() == -1) {
        std::cerr << "SDLNet_Init Error: " << SDLNet_GetError() << std::endl;
        return;
    }

    std::string serverIP = "127.0.0.1"; // Change this to the host's IP
    int serverPort = 12345;

    // Resolve the host (server IP and port)
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, serverIP.c_str(), serverPort) == -1) {
        std::cerr << "SDLNet_ResolveHost Error: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        return;
    }

    // Open the TCP connection
    TCPsocket client = SDLNet_TCP_Open(&ip);
    if (!client) {
        std::cerr << "SDLNet_TCP_Open Error: " << SDLNet_GetError() << std::endl;
        SDLNet_Quit();
        return;
    }

    std::cout << "Connected to server at " << serverIP << ":" << serverPort << std::endl;

    char buffer[1024];
    bool running = true;

    // Wait for "GAME_STARTED" message
    std::cout << "Waiting for 'GAME_STARTED' message...\n";
    bool gameStarted = false;

    while (!gameStarted && running) {
        int bytesReceived = SDLNet_TCP_Recv(client, buffer, sizeof(buffer) - 1);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data
            std::string receivedData(buffer);

            if (receivedData == "GAME_STARTED") {
                std::cout << "Game has started! Waiting for cards...\n";
                gameStarted = true;
            }
            else {
                std::cerr << "Unexpected message: " << receivedData << "\n";
            }
        }
        else if (bytesReceived == 0) {
            std::cerr << "Connection closed by server." << std::endl;
            running = false;
        }
        else {
            std::cerr << "SDLNet_TCP_Recv Error: " << SDLNet_GetError() << std::endl;
            running = false;
        }
    }

    // Now receive card data
    Player me;
    me.socket = client;
    me.isConnected = true;
    me.name = "You";
    me.socket = client;
    int get = receiveCardIDs(client,me,deck);
    if (get == -1)
    {
        std::cout << "Error Reciving cards" << "\n";
    }
    else
    {
        std::cout << "Sucess Reciving cards" << "\n";
        GameRoom(me,deck);
    }
    

    
    
    

    // Cleanup
    SDLNet_TCP_Close(client);
    SDLNet_Quit();
    std::cout << "Disconnected from server." << std::endl;
}




bool isMouseOver(const SDL_Rect& rect, int mouseX, int mouseY) {
    return mouseX > rect.x && mouseX < rect.x + rect.w &&
        mouseY > rect.y && mouseY < rect.y + rect.h;
}
int mainnn(int argc, char* argv[]) {

    std::vector<Card> deck = initializeDeck();
    std::cout << "Cards initalized" << std::endl;

    // Print all cards in the deck
    for (const auto& card : deck) {
        std::cout << card.toString() << std::endl;
    }
    

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (TTF_Init() < 0) {
        std::cerr << "TTF Initialization failed: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("SDL Button", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,500 , 500, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer) {
        std::cerr << "SDL Initialization failed: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("assets/fonts/arial.ttf", 16);
    // Make sure to have a font file in your directory
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    // First button (black)
    Button button1 = {200, {200, 200, 100, 30}, {0, 0, 0, 255}, "Start Server" };

    // Second button (red)
    Button button2 = { 221,{200, 250, 100, 30}, {255, 0, 0, 255}, "Exit" };

    Button button3 = { 222,{200, 300, 100, 30}, {0, 0, 0, 255}, "Join Client" };

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }

            if (e.type == SDL_MOUSEMOTION) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                // Check for hover on button 1
                if (isMouseOver(button1.rect, mouseX, mouseY)) {
                    button1.color = { 0, 255, 0, 255 };  // Hover color (green)
                }
                else {
                    button1.color = { 0, 0, 0, 255 };  // Default color (black)
                }

                // Check for hover on button 2
                if (isMouseOver(button2.rect, mouseX, mouseY)) {
                    button2.color = { 0, 255, 0, 255 };  // Hover color (green)
                }
                else {
                    button2.color = { 255, 0, 0, 255 };  // Default color (red)
                }

                // Check for hover on button 3
                if (isMouseOver(button3.rect, mouseX, mouseY)) {
                    button3.color = { 0, 255, 0, 255 };  // Hover color (green)
                }
                else {
                    button3.color = { 0, 0, 0, 255 };  // Default color (black)
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                // Check for button 1 click
                if (isMouseOver(button1.rect, mouseX, mouseY)) {
                    std::cout << "Button 1 clicked! Starting server...\n";
                    createServer(deck);
                }

                // Check for button 2 click
                if (isMouseOver(button2.rect, mouseX, mouseY)) {
                    std::cout << "Button 2 clicked! Exiting...\n";
                    running = false;
                }
                if (isMouseOver(button3.rect, mouseX, mouseY)) {
                    std::cout << "Button 3 clicked! Joining...\n";
                    joinClient(deck);
                    //running = false;
                }
            }
        }

        // Clear the screen with a light color (white)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Set background color to white
        SDL_RenderClear(renderer);  // Clear screen with white background

        // Render both buttons
        renderButtonWithText(renderer, button1, font);
        renderButtonWithText(renderer, button2, font);
        renderButtonWithText(renderer, button3, font);

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}