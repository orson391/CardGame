// CardG.cpp: Defines the entry point for the application.

#include "CardG.h"

struct Button {
    SDL_Rect rect;
    SDL_Color color;
    std::string label;
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


int receiveCardIDs(TCPsocket client) {
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
            return 0;
        }

        std::cout << "Received card IDs: " << cardID << std::endl;
    }
        
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

void GameRoom(Player& server) {
    std::cout << "Server's cards:\n";
    for (const Card& card : server.myCards) {
        std::cout << card.toString() << "\n";
    }

    // Additional game logic can go here
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
    GameRoom(myserver);


    SDLNet_TCP_Close(server);
    SDLNet_Quit();
}







void joinClient() {
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
    while (gameStarted && running) {
        int get = receiveCardIDs(client);
        if (get == -1)
        {
            std::cout << "Error Reciving cards" << "\n";
        }
        else
        {
            break;
        }
        // Send acknowledgment
        //std::string action = "ACK";
        //SDLNet_TCP_Send(client, action.c_str(), action.length() + 1);
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






int main(int argc, char* argv[]) {

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

    SDL_Window* window = SDL_CreateWindow("SDL Button", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
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
    Button button1 = { {200, 200, 100, 30}, {0, 0, 0, 255}, "Start Server" };

    // Second button (red)
    Button button2 = { {200, 250, 100, 30}, {255, 0, 0, 255}, "Exit" };

    Button button3 = { {200, 300, 100, 30}, {0, 0, 0, 255}, "Join Client" };

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
                    joinClient();
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