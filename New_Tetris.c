#include "raylib.h"
#include "math.h"

#define SETFPS 120
#define BOXSIZE 30
#define BOARDWIDTH 10
#define BOARDHEIGHT 21

enum windowStates
{
    WINDOWSTATE_PLAY,
    WINDOWSTATE_PAUSE,
    WINDOWSTATE_GAMEOVER,
    WINDOWSTATE_CONTROLS
};

enum gameModes
{
    GAMEMODE_PRACTICE,
    GAMEMODE_TIME_ATTACK,
    GAMEMODE_SPRINT,
    GAMEMODE_DEATH_RUN
};

enum pieceShapes
{
    O_PIECE,
    I_PIECE,
    L_PIECE,
    RL_PIECE,
    S_PIECE,
    RS_PIECE,
    T_PIECE
};

struct board
{
    bool IsBlank;
    Vector2 Coords;
    Color color;
};

struct piece
{
    enum pieceShapes shape;
    Vector2 Coords[4];
    Color color;  
};

struct frameTimer
{
    int counter;
    int max;
};

struct clock
{
    int hour;
    int minute;
    int seconds;
    int timer;
};

struct gameStats
{
    struct clock Time;
    int Lines;
    int PiecesDrop;
};

static const int screenWidth = 660;
static const int screenHeight = 750;

struct frameTimer slow_Timer, drop_Timer, click_move_Timer, hold_move_Timer;
struct gameStats Stats;

bool CanPieceRotate;
int PieceRotation;

struct board Board[BOARDWIDTH][BOARDHEIGHT];
struct piece MainPiece, GhostPiece, TempPiece;
struct piece NxtPiece[5], HoldPiece, RotatePiece;

enum windowStates CurrentWindow;
enum gameModes CurrentGameMode;

void Initialize();
void functionTester();
//
void PlayGameModes();
void ModeTimeAttack();
void ModeSprint();
void ModeDeathRun();
//
void GamePlay();
void GamePause();
void GameOver();
void GameControls();
//
void DrawGame();
void DrawBoard();
void DrawPiece();
void DrawTexts();
void DrawNextPiece();
//
void PlayerControl();
void MoveX();
void MoveX_();
void Drop();
void Down();
void Swap();
void Rotate();
//
struct piece GetRandomPiece();
struct piece GetNewPiece(enum pieceShapes);
void GetMainPiece(bool);
void NextPiece();
//
void MovePiece(struct piece*, int, int);
void MoveBlock(Vector2*, int, int);
void MoveGhostPiece();
//
bool CheckCollision(struct piece*);
bool CheckBorderKicks();
//
void WhenCollideVertically();
void VerticalDrop();
//
void ClearBlock(int, int);
void ClearLines(int);
//
void isGameOver(bool);
//
void AddTime(int);
int ConvertTime();

int main()
{
    InitWindow(screenWidth, screenHeight, "New_Tetris.exe");
    SetTargetFPS(SETFPS);

    Initialize();

    while(!WindowShouldClose())
    {
        if(IsKeyPressed(KEY_R)) Initialize();

        BeginDrawing();
            ClearBackground((Color){30, 30, 30, 255});
            DrawGame();

            switch(CurrentWindow)
            {
                case WINDOWSTATE_PLAY: GamePlay(); break;
                case WINDOWSTATE_PAUSE: GamePause(); break;
                case WINDOWSTATE_GAMEOVER: GameOver(); break;
                case WINDOWSTATE_CONTROLS: GameControls(); break;
                default: break;
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void functionTester()
{

}

void Initialize()
{
    for(int h = 0; h < BOARDHEIGHT; h++)
    {
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            Board[w][h] = (struct board){
                true, 
                {
                    (w + 6) * BOXSIZE,
                    h * BOXSIZE
                },
                BLANK
            };
        }
    }

    CurrentWindow = WINDOWSTATE_PLAY;
    CurrentGameMode = GAMEMODE_PRACTICE;
    PieceRotation = 1;

    //simplified from SETFPS / X * (SETFPS/60);

    slow_Timer = (struct frameTimer){0, 120 / 3};
    drop_Timer = (struct frameTimer){0, 120 / 40};
    click_move_Timer = (struct frameTimer){0, 120 / 5};
    hold_move_Timer = (struct frameTimer){0, 120 / 60};

    Stats = (struct gameStats){{0, 0, 0, 0}, 0, 0};

    for(int i = 0; i < 5; i++)
        NxtPiece[i] = GetRandomPiece();
    NextPiece();
    HoldPiece = GetRandomPiece();
    NxtPiece[0] = GetRandomPiece();
}


// ----- GAMEMODE-RELATED FUNCTIONS ----- //

void PlayGameModes()
{
    switch(CurrentGameMode)
    {
        case GAMEMODE_PRACTICE: break;
        case GAMEMODE_TIME_ATTACK: ModeTimeAttack(); break;
        case GAMEMODE_SPRINT: ModeSprint(); break;
        case GAMEMODE_DEATH_RUN: ModeDeathRun(); break;
        default: break;
    }
}

void ModeTimeAttack()
{
    isGameOver((Stats.Time.minute == 1));
}

void ModeSprint()
{
    isGameOver((Stats.Lines == 100));
}

void ModeDeathRun()
{
    int BlankSpace;

    if(Stats.Time.timer == 0)
    {
        BlankSpace = GetRandomValue(1, 3);

        for(int w = 0; w < BOARDWIDTH; w++)
            for(int h = 0; h < BOARDHEIGHT; h++)
            {
                Board[w][h].color = Board[w][h + 1].color;
                Board[w][h].IsBlank = Board[w][h + 1].IsBlank;
            }
        
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            if(GetRandomValue(1, 2) == 1 && BlankSpace > 0)
            {
                ClearBlock(w, BOARDHEIGHT - 1);
                BlankSpace--;
            }
            else
            {
                Board[w][BOARDHEIGHT - 1].color = GRAY;
                Board[w][BOARDHEIGHT - 1].IsBlank = false;
            }
        }
    }
}


// ----- WINDOW-RELATED FUNCTIONS ----- //

void GamePlay()
{
    if(IsKeyPressed(KEY_P)) CurrentWindow = WINDOWSTATE_PAUSE;
    if(IsKeyPressed(KEY_O)) CurrentWindow = WINDOWSTATE_CONTROLS;
    PlayerControl();
    MoveGhostPiece();    

    if(slow_Timer.counter >= slow_Timer.max)
    {
        VerticalDrop();
        slow_Timer.counter = 0;
    }

    if(Stats.Time.timer >= SETFPS)
    {
        AddTime(1);
        Stats.Time.timer = 0;
    }

    PlayGameModes();

    slow_Timer.counter++;
    Stats.Time.timer++;
}

void GamePause()
{
    if(IsKeyPressed(KEY_P)) CurrentWindow = WINDOWSTATE_PLAY;

    DrawRectangle(BOXSIZE * 6, BOXSIZE, BOARDWIDTH * BOXSIZE, (BOARDHEIGHT - 1) * BOXSIZE, (Color){80, 80, 80, 155});
    DrawRectangle(BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 4, (Color){80, 80, 80, 155});
    DrawRectangle((BOARDWIDTH + 7) * BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 16, (Color){80, 80, 80, 155});

    DrawText("II", (screenWidth / 2) - (MeasureText("II", 90) / 2), (BOXSIZE * 11) - (MeasureText("II", 90) / 2), 90, WHITE);
}

void GameOver()
{
    DrawRectangle(BOXSIZE * 6, BOXSIZE, BOARDWIDTH * BOXSIZE, (BOARDHEIGHT - 1) * BOXSIZE, (Color){80, 80, 80, 155});
    DrawText("GAME OVER", (screenWidth / 2) - (MeasureText("GAME OVER", 45) / 2), (BOXSIZE * 11) - 15 , 45, WHITE);
}

void GameControls()
{
    if(IsKeyPressed(KEY_O)) CurrentWindow = WINDOWSTATE_PLAY;
    DrawRectangle(BOXSIZE * 6, BOXSIZE, BOARDWIDTH * BOXSIZE, (BOARDHEIGHT - 1) * BOXSIZE, DARKGRAY);
    DrawRectangle(BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 4, DARKGRAY);
    DrawRectangle((BOARDWIDTH + 7) * BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 16, DARKGRAY);
    
    DrawText("CONTROLS", (screenWidth / 2) - (MeasureText("CONTROLS", 40) / 2), BOXSIZE * 2, 40, WHITE);

    DrawText("Movement: Left/Right keys\n\nDown: S/Down Keys\n\nDrop: Space Bar\n\nHold: D/C Keys\n\nRotate: UP Key\n\nPause/Unpause: P Key\n\nReset: R Key\n\nControls: O Key",
            (screenWidth / 2) - (MeasureText("Movement: Left/Right keys", 20) / 2), BOXSIZE * 4, 20, WHITE);
}

// ----- DRAWING-RELATED FUNCTIONS ----- //

void DrawGame()
{
    DrawPiece();
    DrawBoard();
    DrawTexts();
    DrawNextPiece();
}

void DrawBoard()
{
    for(int h = 1; h < BOARDHEIGHT; h++)
    {
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            //It draw the block (this does not include the mainpiece)
            if(!Board[w][h].IsBlank)
                DrawRectangle(Board[w][h].Coords.x, Board[w][h].Coords.y, BOXSIZE, BOXSIZE, Board[w][h].color);
        }
    }

    DrawRectangleLines(BOXSIZE * 6, BOXSIZE, BOARDWIDTH * BOXSIZE, (BOARDHEIGHT - 1) * BOXSIZE, LIGHTGRAY);
    DrawRectangleLines(BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 4, LIGHTGRAY);
    DrawRectangleLines((BOARDWIDTH + 7) * BOXSIZE, BOXSIZE * 2, BOXSIZE * 4, BOXSIZE * 16, LIGHTGRAY);
}

void DrawTexts()
{
    int h = Stats.Time.hour;
    int m = Stats.Time.minute;
    int s = Stats.Time.seconds;

    DrawText(TextFormat("%02i:%02i:%02i", h, m, s),
        (screenWidth / 2) - (MeasureText(TextFormat("%02i:%02i:%02i", h, m, s), 40) / 2),
        BOXSIZE * 22, 40, GRAY);

    DrawText(TextFormat("%.2f", ((float)Stats.PiecesDrop / ConvertTime())), 
        (screenWidth / 2) - (MeasureText(TextFormat("%.2f", ((float)Stats.PiecesDrop / ConvertTime() + 1)), 25) / 2),
        (BOXSIZE * 23) + 15, 25, GRAY);

    DrawText(TextFormat("%03i", Stats.Lines),
        (BOXSIZE * 16) + (BOXSIZE * 3) - (MeasureText(TextFormat("%03i", Stats.Lines), 50) / 2),
        BOXSIZE * 19, 50, GRAY);
}

void DrawPiece()
{
    for(int i = 0; i < 4; i++)
    {
        DrawRectangle(Board[(int)GhostPiece.Coords[i].x][(int)GhostPiece.Coords[i].y].Coords.x,
                      Board[(int)GhostPiece.Coords[i].x][(int)GhostPiece.Coords[i].y].Coords.y,
                      BOXSIZE, BOXSIZE, GhostPiece.color);

        if(MainPiece.Coords[i].y > 0)
            DrawRectangle(Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].Coords.x,
                        Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].Coords.y,
                        BOXSIZE, BOXSIZE, MainPiece.color);
    }
}

void DrawNextPiece()
{
    for(int i = 0; i < 5; i++)
    {
        for(int n = 0; n < 4; n++)
        {
            DrawRectangle((BOARDWIDTH + 7 + NxtPiece[i].Coords[n].x) * BOXSIZE,
                          (3 + (3 * i) + NxtPiece[i].Coords[n].y) * BOXSIZE,
                           BOXSIZE, BOXSIZE, NxtPiece[i].color);
        }
    }

    for(int i = 0; i < 4; i++)
        DrawRectangle((HoldPiece.Coords[i].x + 1) * BOXSIZE,
                      (HoldPiece.Coords[i].y + 3) * BOXSIZE,
                       BOXSIZE, BOXSIZE, HoldPiece.color);
}


// ----- CONTROL-RELATED FUNCTIONS ----- //

void PlayerControl()
{
    //rotate and swap
    if(IsKeyPressed(KEY_UP))
        Rotate();
    else if((IsKeyPressed(KEY_C) || IsKeyPressed(KEY_D)) && CanPieceRotate)
        Swap();

    //moving left and right
    if((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)))
        MoveX();
    else if(IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_RIGHT))
        click_move_Timer.counter = 0;
    
    //drop and down
    if(IsKeyPressed(KEY_SPACE))
        Drop();

    if(IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        //changes the time frame in each action
        slow_Timer.max = 120 / 60;
        drop_Timer.max = 120 / 4;
    }
    else if(IsKeyReleased(KEY_DOWN) || IsKeyReleased(KEY_S))
    {
        //returns them to default
        slow_Timer.max = 120 / 3;
        drop_Timer.max = 120 / 40;
    }
}

void MoveX()
{
    if(IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))
    {
        MoveX_();
    }
    else if(hold_move_Timer.counter >= hold_move_Timer.max && click_move_Timer.counter >= click_move_Timer.max)
    {
        hold_move_Timer.counter = 0;
        MoveX_();
    }

    if(click_move_Timer.counter < click_move_Timer.max)
        click_move_Timer.counter++;

    hold_move_Timer.counter++;
    drop_Timer.counter = 0;
}

void MoveX_()
{
    int temp = (IsKeyDown(KEY_LEFT))? -1 : 1;

    MovePiece(&MainPiece, temp, 0);
    if(CheckCollision(&MainPiece))
        MovePiece(&MainPiece, -temp, 0);
}

void Drop()
{
    for(int i = 0; i < 4; i++)
        MainPiece.Coords[i] = GhostPiece.Coords[i];

    slow_Timer.counter = 0;
    drop_Timer.counter = 0;

    WhenCollideVertically();
}

void Down()
{
    
}

void Swap()
{
    drop_Timer.counter = 0;
    slow_Timer.counter = 0;

    TempPiece = HoldPiece;
    HoldPiece = GetNewPiece(MainPiece.shape);
    MainPiece = TempPiece;

    GetMainPiece(false);
}

void Rotate()
{
    RotatePiece = MainPiece;
    int tempX = 0, tempY = 0;

    if(PieceRotation >= 4) PieceRotation = 1;
    else PieceRotation++;

    switch(RotatePiece.shape)
    {
        case O_PIECE: break;
        case I_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? 1 : -1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, -tempY);
                MoveBlock(&RotatePiece.Coords[2], tempX, tempY);
                MoveBlock(&RotatePiece.Coords[3], tempX * 2, tempY * 2);
            break;

        case L_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? 1 : -1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, -tempY);
                MoveBlock(&RotatePiece.Coords[2], tempX, tempY);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], -tempY * 2, 0);
            else MoveBlock(&RotatePiece.Coords[3], 0, -tempY * 2);
            break;

        case RL_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1: -1;
            tempY = (PieceRotation <= 2)? 1 : -1;
                MoveBlock(&RotatePiece.Coords[0], tempX, tempY);
                MoveBlock(&RotatePiece.Coords[2], -tempX, -tempY);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], 0, tempY * 2);
            else MoveBlock(&RotatePiece.Coords[3], -tempY * 2, 0);
            break;

        case S_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? -1 : 1;
                MoveBlock(&RotatePiece.Coords[0], tempX, tempY);
                MoveBlock(&RotatePiece.Coords[2], -tempY, tempX);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], -tempY * 2, 0);
            else MoveBlock(&RotatePiece.Coords[3], 0, tempX * 2);
            break;

        case RS_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? -1 : 1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, tempY);
                MoveBlock(&RotatePiece.Coords[2], tempY, tempX);
            if(PieceRotation % 2 == 0) MoveBlock(&RotatePiece.Coords[3], tempY * 2, 0);
            else MoveBlock(&RotatePiece.Coords[3], 0, tempX * 2);
            break;

        case T_PIECE:
            tempX = (PieceRotation == 1 || PieceRotation == 4)? 1 : -1;
            tempY = (PieceRotation <= 2)? -1 : 1;
                MoveBlock(&RotatePiece.Coords[0], -tempX, -tempY);
                MoveBlock(&RotatePiece.Coords[2], tempY, -tempX);
                MoveBlock(&RotatePiece.Coords[3], tempX, tempY);
            break;
    }

    if(CheckBorderKicks())
    {
        for(int i = 0; i < 4; i++)
            MainPiece.Coords[i] = RotatePiece.Coords[i];
    }
    else
    {
        PieceRotation--;
        if(PieceRotation <= 0) PieceRotation = 4;
    }

    drop_Timer.counter = 0;
}


// ----- PIECE-MANAGEMENT FUNCTIONS ----- //

struct piece GetRandomPiece()
{
    enum pieceShapes Shape = GetRandomValue(0, 6);
    return GetNewPiece(Shape);
}

struct piece GetNewPiece(enum pieceShapes Shape)
{
    struct piece NewPiece;
    NewPiece.shape = Shape;

    switch(NewPiece.shape)
    {
        case O_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 1; NewPiece.Coords[3].x = 2;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 1;  
            NewPiece.color = YELLOW;
            break;
        case I_PIECE:
            NewPiece.Coords[0].x = 0; NewPiece.Coords[1].x = 1; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 0; NewPiece.Coords[3].y = 0;  
            NewPiece.color = SKYBLUE;
            break;
        case L_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 3; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 1; NewPiece.Coords[1].y = 1; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 0;  
            NewPiece.color = ORANGE;
            break;
        case RL_PIECE:
            NewPiece.Coords[0].x = 3; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 1; NewPiece.Coords[3].x = 1;
            NewPiece.Coords[0].y = 1; NewPiece.Coords[1].y = 1; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 0;  
            NewPiece.color = BLUE;
            break;
        case S_PIECE:
            NewPiece.Coords[0].x = 3; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 1;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 1;  
            NewPiece.color = LIME;
            break;
        case RS_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 0; NewPiece.Coords[1].y = 0; NewPiece.Coords[2].y = 1; NewPiece.Coords[3].y = 1;  
            NewPiece.color = RED;
            break;
        case T_PIECE:
            NewPiece.Coords[0].x = 1; NewPiece.Coords[1].x = 2; NewPiece.Coords[2].x = 2; NewPiece.Coords[3].x = 3;
            NewPiece.Coords[0].y = 1; NewPiece.Coords[1].y = 1; NewPiece.Coords[2].y = 0; NewPiece.Coords[3].y = 1;  
            NewPiece.color = PURPLE;
            break;
        default: break;
    }

    return NewPiece;
}

void GetMainPiece(bool canRotate)
{
    for(int i = 0; i < 4; i++)
        MainPiece.Coords[i].x += 3;
    CanPieceRotate = canRotate;

    GhostPiece = MainPiece;
    GhostPiece.color.a = 90;

    PieceRotation = 1;
    Stats.PiecesDrop++;
}

void NextPiece()
{
    MainPiece = NxtPiece[0];
    for(int i = 0; i < 4; i++)
        NxtPiece[i] = NxtPiece[i + 1];
    NxtPiece[4] = GetRandomPiece();

    GetMainPiece(true);
    isGameOver(CheckCollision(&MainPiece));   
}


// ----- PIECE-MOVEMENT FUNCTIONS ---- //

void MovePiece(struct piece* Piece, int MovementX, int MovementY)
{
    for(int i = 0; i < 4; i++)
    {
        Piece->Coords[i].x += MovementX;
        Piece->Coords[i].y += MovementY;
    }
}

void MoveBlock(Vector2* Block, int MovementX, int MovementY)
{
    Block->x += MovementX;
    Block->y += MovementY;
}

void MoveGhostPiece()
{
    for(int i = 0; i < 4; i++)
        GhostPiece.Coords[i] = MainPiece.Coords[i];

    while(!CheckCollision(&GhostPiece))
        MovePiece(&GhostPiece, 0, 1);

    MovePiece(&GhostPiece, 0, -1);
}


// ----- BOOLEAN FUNCTIONS ----- //

bool CheckBorderKicks()
{
    TempPiece = RotatePiece;
    int temp;
    
    for(int i = 0; i < 4; i++)
    {
        temp = (i == 0 || i == 3)? 1 : -1;

        if(CheckCollision(&TempPiece))
        {
            TempPiece = RotatePiece;
            if(i <= 1) MovePiece(&TempPiece, temp, 0);
            else MovePiece(&TempPiece, 0, temp);
        }
        else
        {
            RotatePiece = TempPiece;
            return true;
        }
    }    

    if(!CheckCollision(&TempPiece))
    {
        RotatePiece = TempPiece;
        return true;
    }

    return false;
}

bool CheckCollision(struct piece* Piece)
{
    for(int i = 0; i < 4; i++)
    {
        if(Piece->Coords[i].y >= BOARDHEIGHT ||
           Piece->Coords[i].x >= BOARDWIDTH ||
           Piece->Coords[i].x < 0           ||
           !Board[(int)Piece->Coords[i].x][(int)Piece->Coords[i].y].IsBlank           
          )

        return true;
    }

    return false;
}

void isGameOver(bool GameOver)
{
    if(GameOver)
    {
        CurrentWindow = WINDOWSTATE_GAMEOVER;

        for(int h = 0; h < BOARDHEIGHT; h++)
            for(int w = 0; w < BOARDWIDTH; w++)
                Board[w][h].color = GRAY;

        for(int i = 0; i < 5; i++)
            NxtPiece[i].color = GRAY;
        
        MainPiece.color = GRAY;
        HoldPiece.color = GRAY;
        GhostPiece.color = GRAY;
    }
}


// ---- PIECE-DROP RELATED FUNCTIONS ----- //

void WhenCollideVertically()
{
    int lines = 0;
    bool IsLineClear = true;

    for(int i = 0; i < 4; i++)
    {
        Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].IsBlank = false;
        Board[(int)MainPiece.Coords[i].x][(int)MainPiece.Coords[i].y].color = MainPiece.color;
    }

    for(int h = 0; h < BOARDHEIGHT; h++)
    {
        for(int w = 0; w < BOARDWIDTH; w++)
            if(Board[w][h].IsBlank)
                IsLineClear = false;
        if(IsLineClear)
        {
            ClearLines(h);
            lines++;
        }
        IsLineClear = true;
    }

    Stats.Lines += lines;
    NextPiece();
}

void VerticalDrop()
{
    MovePiece(&MainPiece, 0, 1);

    if(CheckCollision(&MainPiece))
    {
        MovePiece(&MainPiece, 0, -1);
        //This allows piece movement after it dropped
        drop_Timer.counter++;
        if(drop_Timer.counter >= drop_Timer.max)
            WhenCollideVertically();
    }
}


// ----- ERASING RELATED FUNCTIONS ----- //

void ClearBlock(int w, int h)
{
    Board[w][h].IsBlank = true;
    Board[w][h].color = BLANK;
}

void ClearLines(int line)
{
    for(int w = 0; w < BOARDWIDTH; w++)
        ClearBlock(w, line);
    for(int h = line; h > 0; h--)
        for(int w = 0; w < BOARDWIDTH; w++)
        {
            Board[w][h].IsBlank = Board[w][h - 1].IsBlank;
            Board[w][h].color = Board[w][h-1].color;
        }
    for(int w = 0; w < BOARDWIDTH; w++)
        ClearBlock(w, 0);
}


// ----- TIME-RELATED FUNCTIONS ---- //

void AddTime(int frame)
{
    Stats.Time.seconds += frame;

    while(Stats.Time.seconds >= 60)
    {
        Stats.Time.seconds -= 60;
        Stats.Time.minute++;
    }

    while(Stats.Time.minute >= 60)
    {
        Stats.Time.minute -= 60;
        Stats.Time.hour++;
    }
}

int ConvertTime()
{
    int TotalSeconds;
    int TotalMinutes;

    TotalMinutes = (Stats.Time.hour * 60) + Stats.Time.minute;
    TotalSeconds = (TotalMinutes * 60) + Stats.Time.seconds;

    return TotalSeconds;
}
