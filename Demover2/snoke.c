#include <SDL.h>
#include <iostream>
#include <vector>
#include<SDL_image.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int GRID_SIZE = 20;
const int INITIAL_SNAKE_LENGTH = 3;

SDL_Texture* snakeHeadTexture = NULL;
SDL_Texture* snakeBodyTexture = NULL;
SDL_Texture* foodTexture = NULL;

SDL_Texture *loadTexture(const char *filename, SDL_Renderer* renderer)
{
	SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO,
                     "Loading %s", filename);

	SDL_Texture *texture = IMG_LoadTexture(renderer, filename);
	if (texture == NULL) {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                       "Load texture %s", IMG_GetError());
      }

	return texture;
}

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

struct Point {
    int x, y;
};

enum class Direction { UP, DOWN, LEFT, RIGHT };

struct Snake {
    std::vector<Point> segments;
    Direction direction;
    SDL_Texture* headTexture;
    SDL_Texture* bodyTexture;
};

Point food;

void placeFood(Snake& snake) {
    bool onSnake = true;

    while (onSnake) {
        food.x = rand() % (SCREEN_WIDTH / GRID_SIZE);
        food.y = rand() % (SCREEN_HEIGHT / GRID_SIZE);

        onSnake = false;
        for (const auto& segment : snake.segments) {
            if (food.x == segment.x && food.y == segment.y) {
                onSnake = true;
                break;
            }
        }
    }
}

void initializeGame(Snake& snake) {
    snake.segments.clear();
    snake.headTexture = snakeHeadTexture;
    snake.bodyTexture = snakeBodyTexture;

    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++) {
        snake.segments.push_back({SCREEN_WIDTH / 2 / GRID_SIZE, SCREEN_HEIGHT / 2 / GRID_SIZE + i});
    }

    snake.direction = Direction::UP;
    placeFood(snake);
}

void handleInput(Snake& snake, bool& quit) {
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    if (snake.direction != Direction::DOWN) {
                        snake.direction = Direction::UP;
                    }
                    break;
                case SDLK_DOWN:
                    if (snake.direction != Direction::UP) {
                        snake.direction = Direction::DOWN;
                    }
                    break;
                case SDLK_LEFT:
                    if (snake.direction != Direction::RIGHT) {
                        snake.direction = Direction::LEFT;
                    }
                    break;
                case SDLK_RIGHT:
                    if (snake.direction != Direction::LEFT) {
                        snake.direction = Direction::RIGHT;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void updateSnake(Snake& snake) {
    Point newHead = snake.segments[0];

    switch (snake.direction) {
        case Direction::UP:
            newHead.y -= 1;
            break;
        case Direction::DOWN:
            newHead.y += 1;
            break;
        case Direction::LEFT:
            newHead.x -= 1;
            break;
        case Direction::RIGHT:
            newHead.x += 1;
            break;
    }

    if (newHead.x < 0 || newHead.x >= SCREEN_WIDTH / GRID_SIZE || newHead.y < 0 || newHead.y >= SCREEN_HEIGHT / GRID_SIZE) {
        initializeGame(snake);
        return;
    }

    if (newHead.x == food.x && newHead.y == food.y) {
        snake.segments.insert(snake.segments.begin(), newHead);
        placeFood(snake);
    } else {
        snake.segments.pop_back();
        snake.segments.insert(snake.segments.begin(), newHead);
    }

    for (size_t i = 1; i < snake.segments.size(); i++) {
        if (newHead.x == snake.segments[i].x && newHead.y == snake.segments[i].y) {
            initializeGame(snake);
            return;
        }
    }
}

void renderGame(Snake& snake) {
    SDL_RenderClear(gRenderer);

    for (size_t i = 0; i < snake.segments.size(); ++i) {
        SDL_Rect r = { snake.segments[i].x * GRID_SIZE, snake.segments[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        if (i == 0) {
            SDL_RenderCopy(gRenderer, snake.headTexture, NULL, &r);
        } else {
            SDL_RenderCopy(gRenderer, snake.bodyTexture, NULL, &r);
        }
    }

    SDL_Rect rFood = { food.x * GRID_SIZE, food.y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
    SDL_RenderCopy(gRenderer, foodTexture, NULL, &rFood);

    SDL_RenderPresent(gRenderer);
}


int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    gWindow = SDL_CreateWindow("Snoke", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    snakeHeadTexture = loadTexture("snake_head.png", gRenderer);
    snakeBodyTexture = loadTexture("snake_body.png", gRenderer);
    foodTexture = loadTexture("apple.png", gRenderer);

    if (snakeHeadTexture == NULL || snakeBodyTexture == NULL || foodTexture == NULL) {
        std::cerr << "Failed to load textures." << std::endl;
        return 1;
    }

    Snake snake;
    bool quit = false;

    initializeGame(snake);

    while (!quit) {
        handleInput(snake, quit);
        updateSnake(snake);
        renderGame(snake);

        SDL_Delay(200);
    }

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gRenderer = NULL;
    gWindow = NULL;

    SDL_Quit();

    return 0;
}
