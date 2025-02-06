#include "Screen.h"
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define NUM_BUTTONS 10

int main()
{
    int run = 1;
    SDL_Event e;
    // running;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL Initialization Failed");
        return -1;
    }

    if (TTF_Init() < 0)
    {
        printf("TTF Initialization Failed");
        SDL_Quit();
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Kashutha", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!window || !renderer)
    {
        printf("SDL Initialization failed");
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    TTF_Font *font = TTF_OpenFont("assets/fonts/arial.ttf", 16);
    // Make sure to have a font file in your directory
    if (!font)
    {
        printf("Failed to load font");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    SDL_Surface *imageSurface = IMG_Load("assets/fonts/image.png"); // Replace with your image path
    // SDL_Surface* imageSurface = IMG_Load("image.png");
    if (!imageSurface)
    {
        printf("Failed to load image: %s\n", IMG_GetError());
        // return 1;
    }

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    if (!imageTexture)
    {
        printf("Failed to create texture: %s\n", SDL_GetError());
        // return 1;
    }
    SDL_FreeSurface(imageSurface);

    // Define multiple buttons
    SDL_Rect buttons[NUM_BUTTONS];
    char *buttonLabels[NUM_BUTTONS] = {"Button 1", "Button 2", "Button 3", "Button 4", "Button 5"};
    SDL_Texture *buttonTextures[NUM_BUTTONS];

    int buttonWidth = 30, buttonHeight = 60; // Button size
    int spacingX = 20, spacingY = 20;         // Space between buttons
    int buttonsPerRow = 6;                    // Number of buttons per row
    int startX = 100;                         // Starting X position
    int startY = 450;                         // Starting Y position

    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        int row = i / buttonsPerRow; // Determines the row (0 or 1)
        int col = i % buttonsPerRow; // Determines the column (0 to 5)

        buttons[i].x = startX + col * (buttonWidth + spacingX);  // Move right for columns
        buttons[i].y = startY + row * (buttonHeight + spacingY); // Move down for rows
        buttons[i].w = buttonWidth;
        buttons[i].h = buttonHeight;

        // Create button text
        SDL_Color textColor = {255, 255, 255, 255}; // White text
        SDL_Surface *textSurface = TTF_RenderText_Solid(font, buttonLabels[i], textColor);
        buttonTextures[i] = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FreeSurface(textSurface);
    }

    while (run)
    {

        SDL_Rect buttonRect = {125, 400, 250, 100};

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                run = 0;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int x = e.button.x;
                int y = e.button.y;

                // Check if any button is clicked
                for (int i = 0; i < NUM_BUTTONS; i++)
                {
                    if (x > buttons[i].x && x < buttons[i].x + buttons[i].w &&
                        y > buttons[i].y && y < buttons[i].y + buttons[i].h)
                    {
                        printf("%s Clicked!\n", buttonLabels[i]);
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Draw table
        SDL_Rect tablerect = {0, 0, SCREEN_WIDTH, 400};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &tablerect);

        // Draw message area
        SDL_Rect messagerect = {0, 400, SCREEN_WIDTH, 40};
        SDL_SetRenderDrawColor(renderer, 25, 10, 100, 255);
        SDL_RenderFillRect(renderer, &messagerect);

        // Draw cards area
        SDL_Rect cardsrect = {0, 440, SCREEN_WIDTH, 360};
        SDL_SetRenderDrawColor(renderer, 125, 10, 10, 255);
        SDL_RenderFillRect(renderer, &cardsrect);

        // Draw image
        if (imageTexture)
        {
            SDL_RenderCopy(renderer, imageTexture, NULL, &tablerect);
        }

        // Draw button
        for (int i = 0; i < NUM_BUTTONS; i++)
        {
            // Draw button (filled rectangle)
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue color
            SDL_RenderFillRect(renderer, &buttons[i]);

            // Render text centered inside the button
            int textWidth, textHeight;
            SDL_QueryTexture(buttonTextures[i], NULL, NULL, &textWidth, &textHeight);
            SDL_Rect textRect = {buttons[i].x + (buttons[i].w - textWidth) / 2,
                                 buttons[i].y + (buttons[i].h - textHeight) / 2,
                                 textWidth, textHeight};
            SDL_RenderCopy(renderer, buttonTextures[i], NULL, &textRect);
        }

        // Update the screen
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        SDL_DestroyTexture(buttonTextures[i]);
    }
    if (imageTexture)
        SDL_DestroyTexture(imageTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}